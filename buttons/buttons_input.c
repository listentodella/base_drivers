/*
 *input 子系统
 *可以参考内核驱动里的kernel/drivers/input/keyboard/gpio_keys.c
 *
 *	
	1.分配一个input_dev结构体
	2.设置
	3.注册
	4.硬件相关的操作（例如中断等）
	 
 *
 * 
 */
#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/workqueue.h>

#include <asm/gpio.h>

static pin_desc {
	int irq;
	char *name;
	unsigned int pin;
	unsigned int key_val;
};

struct pin_desc pins_desc[4] = {
	{IRQ_EINT0, "S2", S3C2440_GPG0, KEY_L},
	{IRQ_EINT2, "S3", S3C2440_GPG2, KEY_S},
	{IRQ_EINT11, "S4", S3C2440_GPG3, KEY_ENTER},
	{IRQ_EINT19, "S5", S3C2440_GPG11, KEY_LEFTSHIFT},
};

static struct input_dev *buttons_dev;
static struct pin_desc *irq_pd;
static struct timer_list buttons_timer;

//中断处理函数，读出按键值
static irqreturn_t buttons_irq(int irq, void *dev_id)
{
	/*修改定时器时间，因为每次抖动都会触发中断，因此这里就可以每次更新
		定时器的值,moo = modify
		现在设定10ms后启动定时器
	*/
	irq_pd = (struct pin_desc *)dev_id;
	mod_timer(&buttons_timer, jiffies + HZ / 100);//jiffies+Hz,hz默认100
	return IRQ_RETVAL(IRQ_HANDLED);
}

//data参数就是在定时器初始化的时候传递给他的（）data成员
static void buttons_timer_function(unsigned long data)
{
	
	struct pin_desc *pindesc = irq_pd;
	unsigned int pinval;
	if (!pindesc)
		return;

	pinval = s3c2410_gpio_getpin(pindesc->pin);
	if (pinval) {
		/*
		松开，参数分别为input_dev结构体、输入事件、按键值，最后一个参数：0松开，1按下

		该函数会上报事件
		 */
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 0);
		input_sync(buttons_dev);
	} else {
		//按下
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 1);
		input_sync(buttons_dev);
	}

	wake_up_interruptible(&button_waitq);//唤醒
	ev_press = 1；
	/*void kill_fasync(struct fasync_struct **fa, int sig, int band);
	*  资源可获得时，应调用kill_fasync()释放SIGIO信号
	*/
	kill_fasync(&button_async, SIGIO, POLLIN);

}


static int buttons_init(void)
{

	int i;
	//1.分配一个input_dev结构体
	buttons_dev = input_allocate_device(buttons_dev);
	//2.设置
	//2.1 能产生哪类事件
	set_bit(EV_KEY, buttons_dev->evbit);
	set_bit(EV_REP, buttons_dev->evbit);//重复，可以达到按住不放，一直产生该键的效果
	
	//2.2 能产生这类操作的哪些事件: 基于2440，令4个按键分别对应L S LeftSHFIT ENTER
	set_bit(KEY_L, buttons_dev->keybit);
	set_bit(KEY_S, buttons_dev->keybit);
	set_bit(KEY_ENTER, buttons_dev->keybit);
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);

	//3.注册
	input_register_device(buttons_dev);

	//4.硬件相关的操作,这里是注册中断,定时器
	init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_function;
	add_timer(&buttons_timer);

	for (i = 0; i < 4; i++) {
		request_irq(pins_desc[i].irq, buttons_irq, IRQT_BOTHEDGE, pins_desc[i].name, &pins_desc[i]);
		//实际项目中，建议判断返回值
	}
	
	
	
	return 0;
}
module_init(buttons_init);

static void buttons_exit(void)
{
	int i;
	for (i = 0; i < 4; i++) {
		free_irq(pins_desc[i].irq, buttons_irq, IRQT_BOTHEDGE, pins_desc[i].name, &pins_desc[i]);
		//实际项目中，建议判断返回值
	}
	del_timer(&buttons_timer);
	input_unregister_device(buttons_dev);
	input_free_device(buttons_dev);
}
module_exit(buttons_exit);

MODULE_LICENSE("GPL");


