/*与其他方法不同，detect方法只需要这一个文件，但是这种方法比较复杂，内核文档也
建议优先使用另外的方法*/


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
		{"at24c08", 0},//这里的名字就重要了，与device里的名字匹配
		{}
};


static int at24cxx_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	/*应该去访问硬件，确定设备确实存在
		能运行到这里，表示该addr的设备是存在的
		但是有些设备单凭地址无法分辨
		（A芯片的地址是0x50，B芯片的地址也是0x50，当然并不接在同一条总线）
		还需要进一步读写i2c设备来分辨是哪一款芯片
		detect函数就是用来进一步分辨这个芯片是哪一款，并且设置info->type
	*/
	printk("at24cxx_detect : addr = 0x%x\n", client->addr);
	//进一步判断是哪一款 

	if (client->addr == 0x50) ｛
		strlcpy(info->type, "at24c08", I2C_NAME_SIZE);
		return 0;
	｝else
		return -ENODEV;
}



static const unsigned short addr_list[] = {0x60, 0x50, I2C_CLIENT_END};

/*1.分配、设置i2c_driver*/
static struct i2c_driver at24cxx_driver = {
	.class = I2C_CLASS_HWMON,//表示去哪些适配器上找设备
	.driver = {
		.name = "xxxx",//实际是跟id_table匹配的，与名字并无关系，但是为了好懂建议写和设备相关
		.owner = THIS_MODULE,
	},
	.probe = at24cxx_probe,
	.remove = __devexit_p(at24cxx_remove),
	.id_table =	at24cxx_id_table,
	.detect = at24cxx_detect,//用这个函数来检测设备确实存在
	.address_list = addr_list,//这些设备的地址
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