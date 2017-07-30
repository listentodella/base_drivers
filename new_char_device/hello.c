#include <...>






static 


/*
 * 1.确定主设备号
 */
static int major;


static int hello_open(struct inode *inode, struct file *file)
{
	printk("hello_open");
	return 0;
}


/*
 * 2.构造file_opreations
 */

static file_operations hello_fops = {
	.owner = THIS_MODULE,
	.open = hello_open,
};

#define HELLO_CNT 2
static struct cdev hello_cdev;
static struct class *cls;



static int hello_init(void)
{
	int rc;
	dev_t devid;
	/*3. 告诉内核*/
#if 0
	(major, 0) ~ (major, 255)都对应 hello_fops
	major = register_chrdev(0, "hello", &hello_fops);
#else
	if (major) {
		devid = MKDEV(major, 0);
		/*(major, 0)对应 hello_fops, (major, 1~255)都不对应hello_fops
		 * 如果参数count，即下面的1改为2，则(major, 0~1)对应hello_fops,以此类推
		 * 即MKDEV的第二个参数决定起点，register_chrdev_region的第二个参数决定终点
		 * alloc_chrdev_region则是用于没有分配主设备号时直接使用的，设置2个参数即可
		 * int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name)
		*/
		rc = register_chrdev_region(devid, HELLO_CNT, "hello");
	} else {
		rc = alloc_chrdev_region(&devid, 0，HELLO_CNT, "hello");
		major = MAJOR(devid);
	}

	cdev_init(&hello_cdev, &hello_fops);
	cdev_add(&hello_cdev, devid, HELLO_CNT);

#endif
	cls = class_create(THIS_MODULE, "hello");
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "hello0");//dev/hello0
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "hello1");//dev/hello1
	class_device_create(cls, NULL, MKDEV(major, 0), NULL, "hello2");//dev/hello2,因为只分配了2个，这里肯定打开失败

	return 0;
}
module_init(hello_init);

static void hello_exit(void)
{
	class_device(cls, MKDEV(major, 0));
	class_device(cls, MKDEV(major, 1));
	class_device(cls, MKDEV(major, 2));
	class_destroy(cls);

	cdev_del(&hello_cdev);
	unregister_chrdev_region(MKDEV(major, 0), HELLO_CNT);
}
module_exit(hello_exit);

MODULE_LICENSE("GPL");