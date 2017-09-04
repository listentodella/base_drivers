/*TP驱动
各种优化措施
*/

#include <...>

struct s3c_ts_regs {
  unsigned long adccon;
  unsigned long adctsc;
  unsigned long adcdly;
  unsigned long adcdat0;
  unsigned long adcdat1;
  unsigned long adcupdn;
};

static struct input_dev *s3c_ts_dev;
static volatile struct s3c_ts_regs *s3c_ts_regs;

static struct timer_list ts_timer;

static void enter_wait_pen_down_mode(void)
{
  s3c_ts_regs->adctsc = 0xd3;//数据手册
}
static void enter_wait_pen_up_mode(void)
{
  s3c_ts_regs->adctsc = 0x1d3;//数据手册
}

static void enter_measure_xy_mode(void)
{
  s3c_ts_regs->adctsc = (1 << 3) | (1 << 2);//数据手册,adc寄存器
}

static void start_adc(void)
{
  s3c_ts_regs->adccon |= (1 << 0);
}

static int filter_ts(int x[], int y[])
{
#define ERR_LIMIT 10

  int avr_x, avr_y;
  int det_x, det_y;
  avr_x = (x[0] + x[1]) / 2;
  avr_y = (y[0] + y[1]) / 2;

  det_x = (x[2] > avr_x) ? (x[2] - avr_x) : (avr_x - x[2]);
  det_y = (y[2] > avr_y) ? (y[2] - avr_y) : (avr_y - y[2]);

  if ((det_x > ERR_LIMIT) || (det_y > ERR_LIMIT))
    return 0;

  avr_x = (x[1] + x[2]) / 2;
  avr_y = (y[1] + y[2]) / 2;

  det_x = (x[3] > avr_x) ? (x[3] - avr_x) : (avr_x - x[3]);
  det_y = (y[3] > avr_y) ? (y[3] - avr_y) : (avr_y - y[3]);

  if ((det_x > ERR_LIMIT) || (det_y > ERR_LIMIT))
    return 0;

  return 1;
}

static void s3c_ts_timer_function(unsigned long data)
{
  if(s3c_ts_regs->adcdat0 & (1 << 15)){//该寄存器的第15bit表示按下或松开,1为松开
    /*已经松开*/
    enter_wait_pen_down_mode();
  } else {
    /*测量 x y坐标*/
    enter_measure_xy_mode();
    start_adc();
  }
}

static irqreturn_t pen_down_up_irq(int irq, void *dev_id)
{
  if(s3c_ts_regs->adcdat0 & (1 << 15)){//该寄存器的第15bit表示按下或松开,1为松开
    printk("pen up\n");
    enter_wait_pen_down_mode();
  } else {
    //printk("pen down\n");
    //enter_wait_pen_up_mode();
    enter_measure_xy_mode();
    start_adc();
  }
  return IRQ_HANDLED;
}

static irqreturn_t adc_irq(int irq, void *dev_id)
{
  static int cnt = 0;
  static int x[4], y[4];
  int adcdat0, adcdat1;
  /*优化措施2：如果adc完成时，发现触摸笔已经松开，则丢弃此次结果*/
  adcdat0 = s3c_ts_regs->adcdat0;
  adcdat1 = s3c_ts_regs->adcdat1;

  if(s3c_ts_regs->adcdat0 & (1 << 15)) {//如果已经松开
    cnt = 0;
    enter_wait_pen_down_mode()
  } else {//按下
    //printk("adc_irq cnt = %d, x = %d, y = %d\n", ++cnt, adcdat0 & 0x3ff,
      //      adcdat1 & 0x3ff);//这里的x y其实是电压值而不是坐标

    /*优化措施3：多次测量求平均值*/
    x[cnt] = adcdat0 & 0x3ff;
    x[cnt] = adcdat1 & 0x3ff;
    ++cnt;
    if (cnt == 4) {
      /*优化措施4：软件过滤*/
      if (filter_ts(x, y)) {
        printk("x = %d, y = %d\n", (x[0] + x[1] + x[2] + x[3]) / 4,
        (y[0] + y[1] + y[2] + y[3]) / 4);
      }

      cnt = 0;
      enter_wait_pen_up_mode();
      /*启动定时器处理长按、滑动的情况*/
      mod_timer(&ts_timer, jiffies + HZ / 100);//10ms

    } else {
      enter_measure_xy_mode();
      start_adc();
    }
  }

  return IRQ_HANDLED;
}

static int s3c_ts_init(void)
{
  /*1.分配一个input_dev结构体*/
  s3c_ts_dev = input_allocate_device();
  struct clk *clk;

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
  /*4.硬件相关的操作*/
    /*4.1 使能时钟（CLKCON[15]）*/
  clk = clk_get(NULL, "adc");
  clk_enable(clk);
    /*4.2 设置S3C2440的ADC/TP 寄存器*/
  s3c_ts_regs = ioremap(0x58000000, sizeof(struct s3c_ts_regs));
    /*4.3 50MHz
    *bit[14] : AD转换器 预分频使能
  *bit[13:6]:AD转换器 预分频值，49，ADCCLK = PCLK/(49+1) = 50MHz/50 = 1MHz
  *bit[1:0]:先设为0
  */
  s3c_ts_regs->adccon = (1 << 14) | (49 << 6);

  request_irq(IRQ_TC, pen_down_up_irq, IRQF_SAMPLE_RANDOM, "ts_pen", NULL);
  request_irq(IRQ_ADC, adc_irq, IRQF_SAMPLE_RANDOM, "adc", NULL);

  /*优化措施1：
  *设置ADCDLY为最大值，这样可以使得电压稳定后再发出中断IRQ_TC
  */
  s3c_ts_regs->adcdly = 0xffff;

  /*优化措施5： 使用定时器处理长按，滑动的情况*/
  init_timer(&ts_timer);
  ts_timer.function = s3c_ts_timer_function;
  add_timer(&ts_timer);

  enter_wait_pen_down_mode();

  return 0;
}

static void s3c_ts_exit(void)
{

  free_irq(IRQ_TC, NULL);
  free_irq(IRQ_ADC, NULL);

  iounmap(s3c_ts_regs);
  input_unregister_device(s3c_ts_dev);
  input_free_device(s3c_ts_dev);
  del_timer(&ts_timer);
}
module_init(s3c_ts_init);
module_exit(s3c_ts_exit);
MODULE_LICENSE("GPL");
