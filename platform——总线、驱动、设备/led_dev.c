/*
	分配、设置、注册 一个platform_device

 */

#include <...>



static struct resource led_resource[] = {
	[0] = {
		.start = 0x56000050,//起始物理地址
		.end = 0x56000050 + 8 - 1,
		.flags = IORESOURCE_MEM,//资源类型
	},
	[1] = {
		.start = 4,
		.end = 4,
		.flags = IORESOURCE_IRQ,
	}
};


static void led_release(struct device *dev)
{

}

static struct platform_device led_dev = {
	.name = "myled",
	.id = -1,
	.num_resource = ARRAY_SIZE(led_resource),
	.resource = led_resource,
	.dev = {
		.release = led_release,
	},
};


stati int led_dev_init(void)
{
	platform_device_register(&led_dev);
	return 0;
}
module_init(led_dev_init);

static void led_dev_exit(void)
{
	platform_device_unregister(&led_dev);
}
module_exit(led_dev_exit);

MODULE_LICENSE("GPL");