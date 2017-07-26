#include <...>


static int major;
static struct class *class;
static struct i2c_client *at24cxx_client;


/*传入：buf[0]: addr
  输出的数据存于：buf[1]: data
*/
static ssize_t at24cxx_read(struct file *file, char __user *buf, 
								size_t count, loff_t *offset)
{
	unsigned char addr, data;
	copy_from_user(&addr, buf, 1);
	/*查看内核document下的smbus协议，对比硬件手册上的时序，找到对应的可用函数*/
	data = i2c_smbus_read_byte_data(at24cxx_client, addr);
	copy_to_user(buf[1], &data, 1);
	return 1;
}


/*buf[0]: addr
  buf[1]: data
*/
static ssize_t at24cxx_write(struct file *file, char __user *buf, 
								size_t count, loff_t *offset)
{
	unsigned char ker_buf[2];
	unsigned char addr, data;

	copy_from_user(ker_buf, buf, 2);
	addr = ker_buf[0];
	data = ker_buf[1];
	/*查看内核document下的smbus协议，对比硬件手册上的时序，找到对应的可用函数*/
	if (!i2c_smbus_write_byte_data(at24cxx_client, addr, data))
		return 2;
	else
		return -EIO;
}


static struct file_operations at24cxx_fops = {
	.owner = THIS_MODULE,
	.read = at24cxx_read,
	.write = at24cxx_write,
};


static int __devinit at24cxx_probe(struct i2c_client *client,
									const struct i2c_device_id *id)
{
	printk("%s %s %d\n", __FILE__, __func__, __line__);
	at24cxx_client = client;
	major = register_chrdev(0, "at24cxx", &at24cxx_fops);
	class = class_create(THIS_MODULE, "at24cxx");
	device_create(class, NULL， MKDEV(major, 0), NULL, "at24cxx");// dev/at24cxx

	return 0;
}


static int __devinit at24cxx_remove(struct i2c_client *client,
									const struct i2c_device_id *id)
{
	printk("%s %s %d\n", __FILE__, __func__, __line__);
	device_destroy(clas, MKDEV(major, 0));
	class_destroy(class);
	unregister_chrdev(major, "at24cxx");

	return 0;
}


static const struct i2c_device_id at24cxx_id_table[] = {
		{"at24c08", 0},
		{}
};


/*1.分配、设置i2c_driver*/
static struct i2c_driver at24cxx_driver = {
	.driver = {
		.name = "xxxx",//实际是跟id_table匹配的，与名字并无关系，但是为了好懂建议写和设备相关
		.owner = THIS_MODULE,
	},
	.probe = at24cxx_probe,
	.remove = __devexit_p(at24cxx_remove),
	.id_table =	at24cxx_id_table,
};


static int at24cxx_drv_init(void)
{
	/*2.注册 i2c_driver*/ 
	i2c_add_driver(&at24cxx_driver);
	return 0;
}
module_init(at24cxx_drv_init);


static void at24cxx_drv_exit(void)
{
	i2c_del_driver(&at24cxx_driver);
}
module_exit(at24cxx_drv_exit);

MODULE_LICENSE("GPL");
