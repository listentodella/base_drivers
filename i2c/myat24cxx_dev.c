#include <...>

static struct i2c_board_info *at24cxx_info = {
	I2C_BOARD_INFO("at24c08", 0x50),//这里的名字很重要，要和drv里的idtable里的名字匹配；地址值需要参考数据手册

};

static struct i2c_client *at24cxx_client;

static int at24cxx_dev_init(void)
{
	struct i2c_adapter *i2c_adap;
	i2c_adap = i2c_get_adapter(0);//0是指第几个适配器
	at24cxx_client = i2c_new_device(i2c_adap, &at24cxx_info);
	i2c_put_adapter(i2c_adap);
	return 0;
}
module_init(at24cxx_dev_init);



static void at24cxx_dev_exit(void)
{
	i2c_unregister_device(&at24cxx_driver);
}
module_exit(at24cxx_dev_exit);

MODULE_LICENSE("GPL");