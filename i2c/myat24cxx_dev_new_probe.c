#include <...>

/*static struct i2c_board_info *at24cxx_info = {
	I2C_BOARD_INFO("at24c08", 0x50),//这里的名字很重要，要和drv里的idtable里的名字匹配；地址值需要参考数据手册

};*/

static struct i2c_client *at24cxx_client;
/*下面的值为i2c设备的地址值表，会遍历匹配*/
static const unsigned short addr_list[] = {0x60, 0x50, I2C_CLIENT_END};


static int at24cxx_dev_init(void)
{
	struct i2c_adapter *i2c_adap;
	struct i2c_board_info at24cxx_info;
	memset(at24cxx_info, 0 , sizeof(struct i2c_board_info));
	strlcpy(at24cxx_info.type, "at24c08", I2C_NAME_SIZE);

	i2c_adap = i2c_get_adapter(0);//0是指第几个适配器
	/*会进行判断是否真实存在，根据判断结果再决定是否new_device*/
	at24cxx_client = i2c_new_probed_device(i2c_adap, &at24cxx_info, addr_list，NULL);
	i2c_put_adapter(i2c_adap);

	if (at24cxx_client)
		return 0;
	else
		return -ENODEV;
}
module_init(at24cxx_dev_init);



static void at24cxx_dev_exit(void)
{
	i2c_unregister_device(&at24cxx_driver);
}
module_exit(at24cxx_dev_exit);

MODULE_LICENSE("GPL");