/**
 * 参考
 * /drivers/mtd/nand/s3c2410.c
 * /drivers/mtd/nand/at91_nand.c
 *
 * 添加分区
 * 测试方法：
 * 1.make menuconfig 去掉内核自带的nand flash驱动
 * 2.make uImage
 * 3.使用新内核启动并且使用NFS作为根文件系统
 * 4.格式化 百度工具
 * 5.挂接 mount ...
 * 6.在 /mnt 下建文件
 */

#include<...>

struct s3c_nand_regs {
  unsigned long nfcont  ;
  unsigned long nfcmd   ;
  unsigned long nfaddr  ;
  unsigned long nfdata  ;
  unsigned long nfeccd0 ;
  unsigned long nfeccd1 ;
  unsigned long nfeccd  ;
  unsigned long nfstat  ;
  unsigned long nfestat0;
  unsigned long nfestat1;
  unsigned long nfmecc0 ;
  unsigned long nfmecc1 ;
  unsigned long nfsecc  ;
  unsigned long nfsblk  ;
  unsigned long nfeblk  ;
}

static struct nand_chip *s3c_nand;
static struct mtd_info *s3c_mtd;
static struct s3c_nand_regs *s3c_nand_regs;

static struct mtd_partitions s3c_nand_parts[] = {
  [0] = {
    .name = "bootloader",
    .size = 0x00040000,
    .offset = 0,
  },
  [1] = {
    .name = "params",
    .offset = MTDPART_OFS_APPEND,
    .size = 0x00020000,
  },
  [2] = {
    .name = "kernel",
    .offset = MTDPART_OFS_APPEND,
    .size = 0x00020000,
  },
  [3] = {
    .name = "root",
    .offset = MTDPART_OFS_APPEND,
    .size = MTDPART_SIZ_FULL,
  }
};


static void	s3c2440_select_chip(struct mtd_info *mtd, int chipnr)
{
  if(chipnr == -1) {
    /*取消选中： NFCONT[1]设为 1*/
    s3c_nand_regs->nfcont |= (1 << 1);
  } else {
    /*选中：NFCONT[1]设为 0*/
    s3c_nand_regs->nfcont &= ~(1 << 1);
  }
}

static void s3c2440_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
  if(ctrl & NAND_CLE) {
    /*发命令：NFCMMD = dat*/
    s3c_nand_regs->nfcmmd = dat;
  } else {
    /*发地址：NFADDR = dat*/
    s3c_nand_regs->nfaddr = dat;
  }
}

static int s3c2440_dev_ready(struct mtd_info *mtd)
{
  //return "NFSTAT 的 bit[0]"
  return (s3c_nand_regs->nfstat & (1 << 0));
}

static int s3c_nand_init(void)
{
  struct clk *clk;
  /*1.分配一个nand_chip结构体*/
  s3c_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
  s3c_nand_regs = ioremap(0x4E000000, sizoef(struct s3c_nand_regs));
  /*2.设置 nand_chip*/
    /*设置 nand_chip 是给nand_scan函数使用的
     *如果不知道怎么设置，先看nand_scan怎么使用
     *它应该提供 选中、发命令、发地址、发数据、读数据、判断状态的功能
    */
  s3c_nand->select_chip = s3c2440_select_chip;
  s3c_nand->cmd_ctrl = s3c2440_cmd_ctrl;
  // s3c_nand->IO_ADDR_R = "NFDATA 的虚拟地址";
  s3c_nand->IO_ADDR_R = s3c_nand_regs->nfdata;
  // s3c_nand->IO_ADDR_W = "NFDATA 的虚拟地址";
  s3c_nand->IO_ADDR_R = s3c_nand_regs->nfdata;
  s3c_nand->dev_ready = s3c2440_dev_ready;
  s3c_nand->ecc.mode = NAND_ECC_SOFT;//ECC校验

  /*3.硬件相关的操作:根据nand flash的手册设置时间参数*/
  /*使能NAND Flash 控制器的时钟*/
  clk_get(NULL, "nand");
  clk_enable(clk); /*实际上相当于设置nand flash的CLKCON寄存器的bit4为1*/
  /**
   * HCLK = 100 MHz
   * TACLS = 0    TACLS 发出CLE/ALE之后多长时间才发出nWE信号
   *              从NAND手册可知CLE/ALE与nWE可以同时发出，
   * TWRPH0       nWE的脉冲宽度，HCLK * (TWRPHO + 1),从spec可知要 ≥ 12ns,故 >= 1
   * TWRPH1       nWE变为高电平后多长时间CLE/ALE才能变为低电平，从spec手册可知要 ≥ 5ns，故 ≥ 0
   */
#define TACLS   0
#define TWRPH0  1
#define TWRPH1  0
  s3c_nand_regs->nfconf = (TACLS << 12) | (TWRPH0 << 8) | (TWRPH1 << 4);

  /*NFCONT
  * bit1设为1，取消片选
  * bit0设为1，使能NAND Flash控制器
  */
  s3c_nand_regs->nfcont = (1 << 1) | (1 << 0);


  /*4.使用 nand_scan*/
  s3c_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
  s3c_mtd->owner = THIS_MODULE;
  s3c_mtd->priv = s3c_nand;

  nand_scan(s3c_mtd, 1);//扫描识别NAND FLASH，构造 mtd_info
  /*5. add_mtd_partition*/
  add_mtd_partitions(s3c_mtd, s3c_nand_parts, 4);//根据数组的配置可知有4个分区
  //add_mtd_device(s3c_mtd);如果不需要构造分区，只需这一句

  return 0;
}
module_init(s3c_nand_init);

static void s3c_nand_exit(void)
{
  del_mtd_partitions(s3c_mtd);
  kfree(s3c_mtd);
  iounmap(s3c_nand_regs);
  kfree(s3c_nand_regs);
}
module_exit(s3c_nand_exit);

MODULE_LICENSE("GPL");
