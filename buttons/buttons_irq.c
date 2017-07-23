/*
 *中断方式获取键值
 *	按键按下
 *	CPU发生中断，跳到异常向量入口执行
 *	跳转到对应的处理函数
 *		a保存中断现场	 
 *	 	b执行中断处理函数	
 *	  	c恢复
 *	
 *	单片机下的中断处理：
 *		分辨是哪一个中断
 *		调用处理函数
 *		清中断
 *
 *  Linux下的中断处理：
 *  	adm_do_IRQ(...)
 *  	struct irq_desc desc[]->handle_irq(...);
 *
 * 		handle_edge_irq
 * 			desc->chip->ack(irq)====清中断
 * 			handle_IRQ_event========处理中断
 * 				取出action链表中的成员，执行action->handler
 *
 * 		request_irq(...)申请中断函数
 * 			分配irqaction结构
 * 			setup_irq(irq, action),判断是否可用，合格便在irq_desc[irq]->action,入链
 * 			desc->chip->startup/enable，设备引脚，使能中断
 *
 * 		free_irq(...)卸载中断函数
 * 			出链
 * 			禁止中断
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/poll.h>
#include <asm/irq.h>
#include <linux/interrupt.h>
#include <asm/uaccess.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>

#define DEVICE_NAME "buttons"

static int major;

volatile unsigned long *gpfcon;
volatile unsigned long *gpfdat;
volatile unsigned long *gpgcon;
volatile unsigned long *gpgdat;

/*
(name) -- 生成一个等待队列头wait_queue_head_t,名字为name
-----------------------------------------------------------------
#define DECLARE_WAIT_QUEUE_HEAD (name)                            \
    wait_queue_head_t name = __WAIT_QUEUE_HEAD_INITIALIZER(name)
    
#define __WAIT_QUEUE_HEAD_INITIALIZER (name) {                    \
    .lock       = __SPIN_LOCK_UNLOCKED(name.lock),               \
    .task_list = { &(name).task_list, &(name).task_list } }

typedef struct __wait_queue_head wait_queue_head_t ;
struct __wait_queue_head {
    spinlock_t lock;
    struct list_head task_list;
};*/
static DECLARE_WAIT_QUEUE_HEAD(button_waitq);
/*中断事件标志，中断服务程序将它置1，read将它清零*/
static volatile int ev_press = 0;

static pin_desc {
	unsigned int pin;
	unsigned int key_val;
};

/*
 *键值：按下时，0x01,0x02,0x03,0x04
 *键值：松开时，0X81,0X82,0X83,0X84
 */
static unsigned char key_val;

struct pin_desc pins_desc[4] = {
	{S3C2440_GPG0, 0x01},
	{S3C2440_GPG2, 0x02},
	{S3C2440_GPG3, 0x03},
	{S3C2440_GPG11, 0x04},

};


//中断处理函数，读出按键值
/*后续的学习将会遇到，中断处理一般分为上半部和下半部，上半部一般用于处理紧急的硬件操作、登记，而将真正的耗时处理工作丢给下半部
 *此时可以在中断处理函数中，使用某种调度方法
 *不过当工作十分简单的时候，就不需要分上下半部了
*/
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	/*switch (irq)-case，该方案建立于，中断号在硬件上是绑定的，所以可以用这种方法
		判断出键值，但是太low	
	*/
	struct pin_desc *pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;

	pinval = s3c2410_gpio_getpin(pindesc->pin);
	if (pinval) 
		key_val = 0x80 | pindesc->key_val;//松开
	else 
		key_val = pindesc->key_val;//按下

	wake_up_interruptible(&button_waitq);//唤醒
	ev_press = 1；
	return IRQ_HANDLED;//正确处理完之后就应该返回IRQ_HANDLED，表明中断已被处理
}


/*open config pins*/
static int button_open(struct inode *inode, struct file *file)
{
	/*
	 *int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags, const char *name, void *dev)
	 *参数为：
	 *中断号，与硬件相关
	 *中断处理函数，是一个回调函数，中断发生时会调用这个函数，dev参数会传递给它
	 *中断处理属性、可以指定中断的触发方式、处理方式。触发：上升沿、下降沿、边沿、高电平、低电平等等；处理：共享中断等
	 *中断名称（自定义的，随便写，可以在加载驱动后通过cat /proc/interrupts看到）
	 *要传递给中断服务程序的私有数据（NULL，或者是该设备的结构体）
	*/
	request_irq(IRQ_EINT0, buttons_irq, IRQT_BOTHEDGE, "s2", &pins_desc);//EINT0根据原理图
	request_irq(IRQ_EINT2, buttons_irq, IRQT_BOTHEDGE, "s3", &pins_desc);//EINT2根据原理图
	request_irq(IRQ_EINT11, buttons_irq, IRQT_BOTHEDGE, "s4", &pins_desc);//EINT11根据原理图
	request_irq(IRQ_EINT19, buttons_irq, IRQT_BOTHEDGE, "s5", &pins_desc);//EINT19根据原理图

	return 0;
}

static size_t button_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{

	if (size != 1)
		return -EINVAL;
	/*如果没有按键动作发生，则休眠->wait_event*/
	//会判断ev_press是否满足条件，不满足就休眠,等待wakeup触；满足接着往下跑
	wait_event_interruptible(button_waitq, ev_press);
	
	/*如果有按键动作发生，返回键值*/
	copy_to_user(buf, &key_val, 1);
	ev_press = 0；
	return 1;
}

/*ioremap*/
static int button_drv_init(void)
{
	major = register_chrdev(0, "button", button_fops);
	if (major < 0) {
		printk("fail to register chrdev\n");
		return ret;
	}
	devfs_mk_cdev(MKDEV(major, 0), S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP, DEVICE_NAME);
	printk(DEVICE_NAME " initialized successfully\n");

	gpfcon = (volatile unsigned long *)ioremap(0x56000050, 16);
	gpfdat = gpfcon + 1;

	gpgcon = (volatile unsigned long *)ioremap(0x56000060, 16);
	gpgdat = gpgcon + 1;


	return 0;
}
module_init(button_drv_init);

static void button_drv_exit(void)
{
	/*firstly iounmap or laterly iounmap?*/
	iounmap(gpfcon);
	iounmap(gpgcon);

	devfs_remove(DEVICE_NAME);
	unregister_chrdev(major, "button");//unregister_chrdev(major, DEVICE_NAME);	
	return;
}
module_exit(button_drv_exit);


static int button_close(struct inode *inode, struct file *fops)
{
       /*
	*void free_irq(unsigned int irq, void *dev_id)
	*/
	free_irq(IRQ_EINT0, &pins_desc);
	free_irq(IRQ_EINT2, &pins_desc);
	free_irq(IRQ_EINT11, &pins_desc);
	free_irq(IRQ_EINT19, &pins_desc);
	return 0;
}


static struct file_operations button_fops = {
	.owner = THIS_MODULE;
	.open = button_open;
	.read = button_read;
	.write = button_write;
	.release = button_close;
}



MODULE_LICENSE("GPL");
