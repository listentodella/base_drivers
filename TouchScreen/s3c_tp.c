/*TP驱动雏形*/



#include <...>

static struct input_dev *s3c_ts_dev;

static int s3c_ts_init(void)
{
  /*1.分配一个input_dev结构体*/
  s3c_ts_dev = input_allocate_device();
  /*2.设置*/
    /*2.1 能够产生哪类事件*/
  set_bit(EV_KEY, s3c_ts_dev->evbit);//按键类
  set_bit(EV_REP, s3c_ts_dev->evbit);//触摸绝对位移类事件
    /*2.2能够产生这类事件里的哪些事件*/
  set_bit(BTN_TOUCH, s3c_ts_dev->keybit);//按键类里的触摸屏事件
  input_set_abs_params(&s3c_ts_dev, ABS_X, 0, 0x3FF, 0, 0);//x方向
  input_set_abs_params(&s3c_ts_dev, ABS_Y, 0, 0x3E8, 0, 0);//y方向
  input_set_abs_params(&s3c_ts_dev, ABS_PRESSURE, 0, 1, 0, 0);  //压力方向，按下、松开，有的还有压力程度的识别


  /*3.注册*/
  input_register_device(s3c_ts_dev);
  /*硬件相关的操作*/

  return 0;
}

static void s3c_ts_exit(void)
{

}
module_init(s3c_ts_init);
module_exit(s3c_ts_exit);
MODULE_LICENSE("GPL");
