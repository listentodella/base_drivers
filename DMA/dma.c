
static int major = 0;

#define MEM_CPY_NO_DMA 0
#define MEM_CPY_DMA 1

static char *src;
static u32 src_phys;
static char *dst;
static u32 dst_phys;

static struct class *cls;

#define BUF_SIZE (512 * 1024)

static int s3c_dma_ioctl(struct inode *inode, struct file *file,
    unsigned int cmd, unsigned long arg)
{
  switch (cmd) {
    case MEM_CPY_NO_DMA:{
      break;
    }
    case MEM_CPY_DMA:{
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

static int s3c_dma_init(void)
{
  /*分配SRC DST对应的缓冲区*/
  src = dma_alloc_writecombine(NULL, BUF_SIZE, &src_phys, GFP_KERNEL);
  if (NULL == src) {
    printk("can't alloc buffer for src\n");
    return -ENOMEM;
  }
  dst = dma_alloc_writecombine(NULL, BUF_SIZE, &dst_phys, GFP_KERNEL);
  if (NULL == dst) {
    /*如果这里分配失败，那么上面分配成功的也应该释放掉，避免内存泄漏
    *个人认为在出错处理环节做free操作更有规范性，即内核常用的goto
    */
    dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
    printk("can't alloc buffer for dst\n");
    return -ENOMEM;
  }
  major = register_chrdev(0, "s3c_dma", &dma_fops);
  /*为了自动创建设备节点*/
  cls = class_create(THIS_MODULE, "s3c_dma");
  class_device_create(cls, NULL, MKDEV(major, 0), NULL, "dma");// /dev/dma节点
  return 0;
}

static void s3c_dma_exit(void)
{
  class_device_destroy(cls, MKDEV(major, 0));
  class_destroy(cls);
  unregister_chrdev(major, "s3c_dma");
  dma_free_writecombine(NULL, BUF_SIZE, src, src_phys);
  dma_free_writecombine(NULL, BUF_SIZE, dst, dst_phys);
}

module_init(s3c_dma_init);
module_exit(s3c_dma_exit);
MODULE_LICENSE("GPL");
