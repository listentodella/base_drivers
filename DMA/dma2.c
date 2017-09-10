

#define MEM_CPY_NO_DMA 0
#define MEM_CPY_DMA 1
#define BUF_SIZE (512 * 1024)

#define DMA0_BASE_ADDR 0x4B000000
#define DMA1_BASE_ADDR 0x4B000040
#define DMA2_BASE_ADDR 0x4B000080
#define DMA3_BASE_ADDR 0x4B0000C0

struct s3c_dma_regs {
  unsigned long disrc;
  unsigned long disrcc;
  unsigned long didst;
  unsigned long didstc;
  unsigned long dcon;
  unsigned long dstat;
  unsigned long dcsrc;
  unsigned long dcdst;
  unsigned long dmasktrig;
};
static volatile struct s3c_dma_regs *dma_regs;

static int major = 0;

static char *src;
static u32 src_phys;
static char *dst;
static u32 dst_phys;
static struct class *cls;

static DECLATE_WAIT_QUEUE_HEAD(dma_waitq);
/*中断事件标志，中断服务程序将它置1，ioctl 将它清0*/
static volatile int ev_dma = 0;

static int s3c_dma_ioctl(struct inode *inode, struct file *file,
    unsigned int cmd, unsigned long arg)
{
  switch (cmd) {
    case MEM_CPY_NO_DMA:{
      break;
    }
    case MEM_CPY_DMA:{
      ev_dma = 0;
      /*把源、目的、长度告诉DMA*/
      dma_regs->disrc = src_phys;/*源的物理地址*/
      dma_regs->disrcc = (0 << 1) | (0 << 0);/*源位于AHB总线，源地址递增*/
      dma_regs->didst = dst_phys;/*目的的物理地址*/
      dma_regs->didstc = (0 << 2) | (0 << 1) | (0 << 0);/*目的位于AHB总线，递增*/
      dma_regs->dcon = (1 << 29) | (0 << 28) | (0 << 23) | (0 << 20) | (BUF_SIZE << 0);/*使能中断,单个传输，软件触发*/
      /*启动dma*/
      dma_regs->dmasktrig = (1 << 1) | (1 << 0);
      /*如何知道DMA何时完成？何时结束----通过中断*/
      /*休眠*/
      wait_event_interruptible(dma_waitq, ev_dma);
      for( i = 0; i < BUF_SIZE; i++)
        dst[i] = src[i];
      if (memcmp(src, dst, BUF_SIZE) == 0) {
        printk("MEM_CPY_DMA OK \n");
      } else {
        printk("MEM_CPY_DMA ERROR\n");
      }
      break;
    }
    default:
      break;
  }
}

static struct file_operations dma_fops = {
  .owner = THIS_MODULE,
  .ioctl = s3c_dma_iotcl,
};


static irqreturn_t s3c_dma_irq(int irq, void *devid)
{
  /*唤醒*/
  ev_dma = 1;
  wake_up_interruptible(&dma_waitq);/*唤醒休眠的进程*/
  return IRQ_HANDLED;
}

static int s3c_dma_init(void)
{
  /*注册中断*/
  if(request_irq(IRQ_DMA3, s3c_dma_irq, 0, "s3c_dma", 1)){//cat /proc/interrupts 查看已经被使用的中断，已经被占用的dma无法注册成功的
      printk("can't request irq for dma\n");
      return -EBUSY;
  }

  /*分配SRC DST对应的缓冲区*/
  src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL);
  if (NULL == src) {
    free_irq(IRQ_DMA3, 1);
    printk("can't alloc buffer for src\n");
    return -ENOMEM;
  }
  dst = dma_alloc_writecombine(NULL, BUF_SIZE, &dst_phys, GFP_KERNEL);
  if (NULL == dst) {
    /*如果这里分配失败，那么上面分配成功的也应该释放掉，避免内存泄漏
    *个人认为在出错处理环节做free操作更有规范性，即内核常用的goto
    */
   free_irq(IRQ_DMA3, 1);
    dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
    printk("can't alloc buffer for dst\n");
    return -ENOMEM;
  }
  major = register_chrdev(0, "s3c_dma", &dma_fops);
  /*为了自动创建设备节点*/
  cls = class_create(THIS_MODULE, "s3c_dma");
  class_device_create(cls, NULL, MKDEV(major, 0), NULL, "dma");// /dev/dma节点

  dma_regs = ioremap(DMA3_BASE_ADDR, sizeof(struct s3c_dma_regs));

  return 0;
}

static void s3c_dma_exit(void)
{
  iounmap(dma_regs);
  class_device_destroy(cls, MKDEV(major, 0));
  class_destroy(cls);
  unregister_chrdev(major, "s3c_dma");
  dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
  dma_free_writecombine(NULL, BUF_SIZE, dst, dst_phys);
  free_irq(IRQ_DMA3, 1);
}

module_init(s3c_dma_init);
module_exit(s3c_dma_exit);
MODULE_LICENSE("GPL");
