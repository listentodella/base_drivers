/*TP驱动 硬件操作
  测试：make menuconfig 去掉原本的tp的功能
  insmod

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



static void enter_wait_pen_down_mode(void)
{
  s3c_ts_regs->adctsc = 0xd3;//数据手册
}
static void enter_wait_pen_up_mode(void)
{
  s
}

static irqreturn_t pen_down_up_irq(int irq, void *dev_id)
{
  if(s3c_ts_regs->adcdat0 & (1 << 15)){//该寄存器的第15bit表示按下或松开
    printk("pen up\n");
    enter_wait_pen_down_mode();
  } else {
    printk("pen down\n");
    enter_wait_pen_up_mode();
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
    enter_wait_pen_down_mode();

  return 0;
}

static void s3c_ts_exit(void)
{
  free_irq(IRQ_TC, NULL);
  iounmap(s3c_ts_regs);
  input_unregister_device(s3c_ts_dev);
  input_free_device(s3c_ts_dev);
}
module_init(s3c_ts_init);
module_exit(s3c_ts_exit);
MODULE_LICENSE("GPL");
