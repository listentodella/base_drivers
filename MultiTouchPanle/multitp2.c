/*多触点触摸屏，即电容屏，一般是i2c与input驱动的混合
*测试：
*a.先把原有的ft5x06_ts.c驱动程序去掉
* i2c驱动有i2c_driver, i2c_device, ft5x06_ts.c只是i2c_driver,因此我们要找到i2c_device并
* 把它去掉,方法：
* 修改同目录下的Makefile：
* obj-$(CONFIG_TOUCHSCREEN_FT5X0X) += ft5x06_ts.o
* 改为
* obj-$(CONFIG_TOUCHSCREEN_FT5X0X) += multitp2.o
*b.去掉i2c_device
* 找到\linux-3.0.86\arch\arm\mach-exynos\Mach-tiny4412.c，并注释掉
*   i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));
*
*/

#include <...>

#define MTP_ADDR (0x70 << 1)
#define MTP_MAX_X 800
#define MTP_MAX_Y 480

#define MTP_NAME "ft5x0x_ts"
/*支持多少点触控由硬件决定
*由spec可得，一个触点ID要用4bit表示，则最大值为1111 B,即15个
**/
#define MTP_MAX_ID 15


#define MTP_IRQ gpio_to_irq(EXYNOS4_GPX1(6))//中断号要对

struct input_dev *ts_dev;
static struct work_struct mtp_work;
static struct i2c_client *mtp_client;

struct mtp_event {
  int x;
  int y;
  int id;
};

static mtp_event mtp_events[16];
static int mtp_points;

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

static int mtp_ft5x0x_i2c_rxdata(struct i2c_client *client,
  char *rxdata, int length)
{
  int ret;
  struct i2c_msg msgs[] = {
    {
      .addr = client->addr,
      .flags = 0,
      .len = 1,
      .buf = rxdata,
    },{
      .addr = client->addr,
      .flags = I2C_M_RD,
      .len = length,
      .buf = rxdata,
    },
  };

  ret = i2c_transfer(client->adapter, msgs, 2);
  if (ret < 0)
    pr_err("%s: i2c read error: %d\n", __func__, ret);

  return ret;

}


static int mtp_ft5x0x_read_data(void)
{
  //struct ft5x0x_ts_event *event = &ts->event;
  u8 buf[32] = {0};
  int ret;

  ret = mtp_ft5x0x_i2c_rxdata(mtp_client, 31);

  if(ret < 0) {
    printk("%s: read touch data failed, %d\n", __func__, ret);
    return ret;
  }
  mtp_points = buf[2] & 0x0f;

  switch (mtp_points) {
  case 5:
    mtp_events[4].x = (s16)(buf[0x1b] & 0x0F) << 8 | (s16)buf[0x1c];
    mtp_events[4].y = (s16)(buf[0x1d] & 0x0F) << 8 | (s16)buf[0x1e];
    mtp_events[4].id = buf[0x1d] >> 4;
  case 4:
    mtp_events[3].x = (s16)(buf[0x15] & 0x0F) << 8 | (s16)buf[0x16];
    mtp_events[3].y = (s16)(buf[0x17] & 0x0F) << 8 | (s16)buf[0x18];
    mtp_events[3].id = buf[0x17] >> 4;
  case 3:
    mtp_events[2].x = (s16)(buf[0x0F] & 0x0F) << 8 | (s16)buf[0x10];
    mtp_events[2].y = (s16)(buf[0x11] & 0x0F) << 8 | (s16)buf[0x12];
    mtp_events[2].id = buf[0x11] >> 4;
  case 2:
    mtp_events[1].x = (s16)(buf[0x09] & 0x0F) << 8 | (s16)buf[0x0a];
    mtp_events[1].y = (s16)(buf[0x0b] & 0x0F) << 8 | (s16)buf[0x0c];
    mtp_events[1].id = buf[0x0b] >> 4;
  case 1:
    mtp_events[0].x = (s16)(buf[0x03] & 0x0F) << 8 | (s16)buf[0x1c];
    mtp_events[0].y = (s16)(buf[0x05] & 0x0F) << 8 | (s16)buf[0x1c];
    mtp_events[0].id = buf[0x05] >> 4;
    break;
  case 0:
    return 0;
  default:
  //  printk("%s: invalid touch data, %d\n", __func__, e);
    return -1;
  }

  return 0;
}


static void mtp_work_func(void)
{
  int i;
  int ret;
	/*读取i2c设备获取触点数据并上报*/
  /*读取*/
  ret = mtp_ft5x0x_read_data();
  if (ret < 0)
    return;
  /*上报*/
  if (!mtp_points) {//如果没有触点
    input_mt_sync(ts_dev);
    input_sync(ts_dev);
    return 1;
  }
  for(i = 0; i < mtp_points; i++) {/*每一个点*/
    input_report_abs(ts_dev, ABS_MT_POSITION_X, mtp_events[i].x);
    input_report_abs(ts_dev, ABS_MT_POSITION_Y, mtp_events[i].y);
    input_report_abs(ts_dev, ABS_MT_TRACKING_ID, mtp_events[i].id);
    input_mt_sync(ts_dev);
  }
  input_sync(ts_dev);
}


static int __devinit mtp_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk("%s %s %d\n", __FILE__, __func__, __line__);
  /*终于进入了输入子系统*/
  mtp_client = client;
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

static int mtp_ft5x06_valid(struct i2c_client *client)
{
  u8 buf[32] = {0};
  int ret;

  printk("mtp_ft5x06_valid : addr = 0x%x\n", client->addr);
  /*进一步判断设备的合法性*/
    buf[0] = 0xa3;//读id的寄存器
    ret = mtp_ft5x0x_i2c_rxdata(client, buf, 1);

    if(ret < 0) {
      printk("%s: there is no real device, i2c read err\n", __func__);
      return ret;
    }
  //  printk("chip vendor id is 0x%x\n", buf[0]);
    /*如果返回值是0，则读的是00寄存器，根据spec，00寄存器只有这3个合法值*/
    if (buf[0] != 0x55) {//如果spec里没有给，可以先直接读出来id并打印，如上面的Printk
      printk("%s: there is no real device, val err\n", __func__);
      return -1;
    }
    return 0;
}

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
  if(mtp_ft5x06_valid() < 0)
    return -1;

  strlcpy(info->type, "100ask_mtp", I2C_NAME_SIZE);
    return 0;
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
