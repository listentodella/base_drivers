#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/mutex.h>

static unsigned short ignore[] = { I2C_CLIENT_END };
static unsigned short normal_addr[] = { 0x50, I2C_CLIENT_END };/*地址是7bit,改为0x60的话，detect不会被调用，因为该地址的设备不存在*/

static unsigned short force_addr[] = { ANY_I2C_BUS, 0x60, I2C_CLIENT_END };
static unsigned short *forces[] = { force_addr, NULL };

static struct i2c_client_address_data addr_data = {
	.normal_i2c		= ignore,/*要发出S信号和设备地址并得到ACK信号才能确定存在这个设备*/
	.probe			= ignore,
	.ignore			= ignore,
	.force			= forces,/*强制认为存在这个设备*/
};


static int at24cxx_detect(struct i2c_adapter *adapter,  int address, int kind)
{
	printk("%s\n", __func__);
	return 0;
}


static int at24cxx_attach(struct i2c_adapter *adapter)
{
	return i2c_probe(adapter, &addr_data, at24cxx_detect);
}

static int at24cxx_detach(struct i2c_client *client)
{
	printk("%s\n", __func__);
	return 0;
}


/*1.分配一个i2c_driver结构体*/
/*2.设置i2c_driver结构体*/

static struct i2c_driver at24cxx_driver = {
	.driver = {
		.name = "at24cxx",
	},
	.attach_adapter = at24cxx_attach,
	.detach_client = at24cxx_detach,
};


static int at24cxx_init(void)
{
	i2c_add_driver(&at24cxx_driver);
	return 0;
}
module_init(at24cxx_init);

static void at24cxx_exit(void)
{
	i2c_del_driver(&at24cxx_driver);
}
module_exit(at24cxx_exit);

MODULE_LICENSE("GPL");