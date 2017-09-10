/*多触点触摸屏，即电容屏，一般是i2c与input驱动的混合*/

#include <...>

#define MTP_ADDR 0x70

#define MTP_MAX_ID 10
#define MTP_MAX_X 800
#define MTP_MAX_Y 480

#define MTP_IRQ 123

struct input_dev *ts_dev;
static struct work_struct mtp_work;

static irqreturn_t mtp_interrupt(int irq, void *dev_id)
{
  /*本该：
  *获取触点数据，并上报
  *但是i2c是慢速设备，不该放在中断服务程序中操作
  */

 /*使用工作队列，让内核线程来操作*/
  schedule_work(&mtp_work);
  return IRQ_HANDLED;
}

static void mtp_work_func(void)
{
	/*读取i2c设备获取触点数据并上报*/
  /*读取*/
  /*上报*/
  for(/*每一个点*/) {
    input_report_abs(ts_dev, ABS_MT_POSITION_X, x);
    input_report_abs(ts_dev, ABS_MT_POSITION_Y, y);
    input_report_abs(ts_dev, ABS_MT_TRACKING_ID, id);
    input_mt_sync(ts_dev);
  }
  input_sync(ts_dev);
}


static int __devinit mtp_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk("%s %s %d\n", __FILE__, __func__, __line__);
  /*终于进入了输入子系统*/
  /*分配input_dev*/
  ts_dev = input_allocate_device();
  /*设置*/
    /*能产生哪类事件*/
    set_bit(EV_SYN, ts_dev->evbit);
    set_bit(EV_ABS, ts_dev->evbit);
    /*能产生这类事件的哪些事件*/
    set_bit(ABS_MT_TRACKING_ID, ts_dev->absbit);
    set_bit(ABS_MT_POSITION_X, ts_dev->absbit);
    set_bit(ABS_MT_POSITION_Y, ts_dev->absbit);
    /*这些事件的范围*/
    input_set_abs_params(ts_dev, ABS_MT_TRACKING_ID, 0, MTP_MAX_ID, 0, 0);//最大触摸点数，这里设置为10
    input_set_abs_params(ts_dev, ABS_MT_POSITION_X, 0, MTP_MAX_X, 0, 0);
    input_set_abs_params(ts_dev, ABS_MT_POSITION_Y, 0, MTP_MAX_Y, 0, 0);
  /*注册*/
  input_register_device(ts_dev);
  /*硬件相关操作*/
  INIT_WORK(&mtp_work, (void *)mtp_work_func, NULL);

  request_irq(MTP_IRQ, mtp_interrupt, IRQ_TYPE_EDGE_FALLING, "100ask_mtp", ts_dev);

	return 0;
}

static int __devinit mtp_remove(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk("%s %s %d\n", __FILE__, __func__, __line__);
  free_irq(MTP_IRQ, ts_dev);
  cancel_work_sync(&mtp_work);
  input_unregister_device(ts_dev);
  input_free_device(ts_dev);
	return 0;
}

static const struct i2c_device_id mtp_id_table[] = {
		{"100ask_mtp", 0},//这里的名字就重要了，与device里的名字匹配
		{}
};


static int mtp_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	/*应该去访问硬件，确定设备确实存在
		能运行到这里，表示该addr的设备是存在的
		但是有些设备单凭地址无法分辨
		（A芯片的地址是0x50，B芯片的地址也是0x50，当然并不接在同一条总线）
		还需要进一步读写i2c设备来分辨是哪一款芯片
		detect函数就是用来进一步分辨这个芯片是哪一款，并且设置info->type
	*/
	printk("mtp_detect : addr = 0x%x\n", client->addr);
	//进一步判断是哪一款

	if (client->addr == 0x50) ｛
		strlcpy(info->type, "at24c08", I2C_NAME_SIZE);
		return 0;
    /*返回0之后，会创建一个新的i2c设备
    *i2c_new_device(adapter, &info),其中的info->type
    */

	｝else
		return -ENODEV;
}


static const unsigned short addr_list[] = {MTP_ADDR, 0x60, 0x50, I2C_CLIENT_END};

/*1.分配、设置i2c_driver*/
static struct i2c_driver mtp_driver = {
	.class = I2C_CLASS_HWMON,//表示去哪些适配器上找设备
	.driver = {
		.name = "xxxx",//实际是跟id_table匹配的，与名字并无关系，但是为了好懂建议写和设备相关
		.owner = THIS_MODULE,
	},
	.probe = mtp_probe,
	.remove = __devexit_p(mtp_remove),
	.id_table =	mtp_id_table,
	.detect = mtp_detect,//用这个函数来检测设备确实存在
	.address_list = addr_list,//这些设备的地址
};


static int mtp_drv_init(void)
{
	/*2.注册 i2c_driver*/
	i2c_add_driver(&mtp_driver);
	return 0;
}
module_init(mtp_drv_init);


static void mtp_drv_exit(void)
{
	i2c_del_driver(&mtp_driver);
}
module_exit(mtp_drv_exit);

MODULE_LICENSE("GPL");
