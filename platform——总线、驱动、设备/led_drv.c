/*
	分配、设置、注册 一个platform_driver

 */

#include <...>

static int major;
static struct class *cls;
static volatile unsigned long *gpio_con;
static volatile unsigned long *gpio_dat;
static int pin;


static int led_open(struct inode *inode, struct file *file)
{
	/*
	原：
	 配置GPF4,5,6为输出
			*gpio_con &= ~((0x3 << (4 * 2)) | (0x3 << (5 * 2)) | (0x3 << (6 * 2)));
			*gpio_con |= ((0x1 << (4 * 2)) | (0x1 << (5 * 2)) | (0x1 << (6 * 2)));
	*/

	/*配置为输出*/
	*gpio_con &= ~(0x3 << (pin * 2));
	*gpio_con |= (0x1 << (pin * 2)); 
	return 0;
}


static ssize_t led_write(struct file *file, const char __user *buf,
							size_t count, loff_t *offset)
{
	int val;
	copy_from_user(&val, buf, count);
	if (val == 1) 
		*gpio_dat &= ~(1 << pin);//点灯
	else
		*gpio_dat |= (1 << pin);//灭灯

	return 0;
}


static struct file_operations led_fops = {
	.owner = THIS_MODULE,//这是一个宏，指向编译模块时自动创建的__this_module变量
	.open = led_open,
	.write = led_write,
	//.
};



static int led_probe(struct platform_device *pdev)
{
	struct resource *res;
	/*根据platform_device的资源进行ioremap*/
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);//这里的0表示dev下的resource结构体里的第一个
	gpio_con = ioremap(res->start, res->end - res->start + 1);
	gpio_dat = gpio_con + 1;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	pin = res->start;

	/*注册字符设备驱动程序*/

	printk("led_probe, found led\n");

	major =	register_chrdev(0, "myled", &led_fops);
	cls = class_create(THIS_MODULE, "myled");
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "led");//自动创建设备文件节点

	return 0;
}

/*如果dev中没有release函数，则卸载会出问题*/
static int led_remove(struct platform_device *pdev)
{
	/*卸载字符设备驱动程序*/

	/*根据platform_device的资源进行iounmap*/

	printk("led_remove, remove led\n");
	class_device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregisger_chrdev(major, "myled");
	iounmap(gpio_con);

	return 0;
}


static struct platform_driver led_drv = {
	.probe = led_probe,
	.remove = led_remove,
	.driver = {
		.name = "myled",//platform_driver要通过名字进行匹配
	}
};



static int led_drv_init(void)
{
	platform_driver_register(&led_drv);
	return 0;
}
module_init(led_drv_init);

static void led_drv_exit(void)
{
	platform_driver_unregister(&led_drv);
}
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");