/*
 *目的：按键驱动
 *1.写出框架
 *		先构造file_operations,填充成员
 *	 	入口函数，并通知内核；出口函数，通知内核
 *	  	给sysfs提供更多信息，以让udev机制可以自动创建更多设备节点
 *	
 *2.硬件操作
 *		看原理图，引脚
 *		看手册，怎么操作寄存器
 *		编程，需要通过虚拟地址
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



/*open config pins*/
static int button_open(struct inode *inode, struct file *file)
{
	/*配置GPF0,2为输入引脚*/
	*gpfcon &= ~((0x3 << (0 * 2)) | (0x3 << (2 * 2)));

	/*配置GPG3, 11为输入引脚*/
	*gpgcon &= ~((0x3 << (0 * 2)) | (0x3 << (11 * 2)));

	return 0;
}

static size_t button_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	/*返回4个引脚的电平*/
	unsigned char key_vals[4];
	int regval;

	if (size != sizeof(key_vals))
		return -EINVAL;

	//read pins
	regval = *gpfdat;
	key_vals[0] = (regval & (1 << 0)) ? 1 : 0;
	key_vals[1] = (regval & (1 << 2)) ? 1 : 0;

	regval = *gpgdat;
	key_vals[3] = (regval & (1 << 0)) ? 1 : 0;
	key_vals[11] = (regval & (1 << 2)) ? 1 : 0;

	copy_to_user(buf, key_vals, sizeof(key_vals));

	return sizeof(key_vals);
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



static struct file_operations button_fops = {
	.owner = THIS_MODULE;
	.open = button_open;
	.read = button_read;
	.write = button_write;
}



MODULE_LICENSE("GPL");