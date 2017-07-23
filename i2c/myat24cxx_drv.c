#include <...>




static int __devinit at24cxx_probe(struct i2c_client *client,
									const struct i2c_device_id *id)
{
	printk("%s %s %d\n", __FILE__, __func__, __line__);
	return 0;
}

static int __devinit at24cxx_remove(struct i2c_client *client,
									const struct i2c_device_id *id)
{
	printk("%s %s %d\n", __FILE__, __func__, __line__);
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
