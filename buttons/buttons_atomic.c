/*
 * 原子操作
 * 同时只有一个app能打开驱动
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

//static int flag = 1;
atomic_t flag = ATOMIC_INIT(1);//定义原子变量并初始化为1

//中断处理函数，读出按键值

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	
	struct pin_desc *pindesc = (struct pin_desc *)dev_id;
	unsigned int pinval;

	pinval = s3c2410_gpio_getpin(pindesc->pin);
	if (pinval) {
		key_val = 0x80 | pindesc->key_val;//松开
	} else {
		key_val = pindesc->key_val;//按下
	}

	wake_up_interruptible(&button_waitq);//唤醒
	ev_press = 1；
	/*void kill_fasync(struct fasync_struct **fa, int sig, int band);
	*  资源可获得时，应调用kill_fasync()释放SIGIO信号
	*/
	kill_fasync(&button_async, SIGIO, POLLIN);

	return IRQ_RETVAL(IRQ_HANDLED);
}

/*open config pins*/
static int button_open(struct inode *inode, struct file *file)
{
	if (!atomic_dec_and_test(&flag)) {
		atomic_inc(&flag);
		return -EBUSY;
	}
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
	/*如果没有按键动作发生，则休眠->wait_event
	  如果有按键动作发生，返回键值
	*/
	//会判断ev_press是否满足条件，不满足就休眠,等待wakeup触；满足接着往下跑
	wait_event_interruptible(button_waitq, ev_press);


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
	atomic_inc(&flag);
	free_irq(IRQ_EINT0, &pins_desc);
	free_irq(IRQ_EINT2, &pins_desc);
	free_irq(IRQ_EINT11, &pins_desc);
	free_irq(IRQ_EINT19, &pins_desc);
	/*
	 * 文件关闭时，应在release中调用驱动的fasync()
	 * 将文件从异步通知的列表中删除
	 */
	button_fasync(-1, filp, 0);

	return 0;
}


static unsigned int button_poll(struct file *filp, struct poll_table *wait)
{
	unsigned int mask = 0;
	/*
	 *void poll_wait(struct file *filp, wait_queue_head_t *queue, struct poll_table *wait) 
	 *并不立即会休眠，而是将其加入等待队列，如果各个条件都不满足才会进入休眠(而不是阻塞)等待唤醒
	 *wait的名称容易让人误解，实际并不会因此阻塞。将当前进程添加到wait参数指定的等待列表（poll_table)
	 *实际作用是让唤醒参数queue对应的等待队列可以印唤醒因select、poll而睡眠的进程
	 */
	poll_wait(filp, &button_waitq, wait);

	if (ev_press)
		mask |= POLLIN | POLLRDNORM;

    return mask;
}

/*
 *该函数会在app调用fcntl（有FSYNC标志时）调用
*/
static int button_fasync(int fd, struct file *filp, int on)
{
	printk("buttons fsync\n");
	/*
	int fasync_helper(int fd, struct file *filp, int on, struct fasync_struct **fapp)
	处理FASYNC标志变更的函数
	fasync_helper作用就是初始化fasync这个东西，包括分配内存和设置属性
	最后在驱动的release里把fasync_helper初始化的东西free掉。
	*/
	return fasync_helper(fd, filp, on, &button_fasync);
}


static struct file_operations button_fops = {
	.owner = THIS_MODULE;
	.open = button_open;
	.read = button_read;
	.write = button_write;
	.release = button_close;
	/*通过查看file_operations结构体可以发现并没有设定select成员
	 * 实际上select和poll最终都会调用到这里
	 */
	.poll = button_poll;
	.fasync = button_fasync;
}



MODULE_LICENSE("GPL");
