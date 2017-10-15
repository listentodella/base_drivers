/**
 * 参考
 * /drivers/mtd/nand/s3c2410.c
 * /drivers/mtd/nand/at91_nand.c
 */

#include<...>

static struct nand_chip *s3c_nand;
static struct mtd_info *s3c_mtd;

static void	s3c2440_select_chip(struct mtd_info *mtd, int chipnr)
{
  if(chipnr == -1) {
    /*取消选中： NFCONT[1]设为 1*/
  } else {
    /*选中：NFCONT[1]设为 1*/
  }
}

static void s3c2440_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
  if(ctrl & NAND_CLE) {
    /*发命令：NFCMMD = dat*/
  } else {
    /*发地址：NFADDR = dat*/
  }
}

static int s3c2440_dev_ready(struct mtd_info *mtd)
{
  return "NFSTAT 的 bit[0]"
}

static int s3c_nand_init(void)
{
  /*1.分配一个nand_chip结构体*/
  s3c_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);

  /*2.设置 nand_chip*/
    /*设置 nand_chip 是给nand_scan函数使用的
     *如果不知道怎么设置，先看nand_scan怎么使用
     *它应该提供 选中、发命令、发地址、发数据、读数据、判断状态的功能
    */
  s3c_nand->select_chip = s3c2440_select_chip;
  s3c_nand->cmd_ctrl = s3c2440_cmd_ctrl;
  s3c_nand->IO_ADDR_R = "NFDATA 的虚拟地址";
  s3c_nand->IO_ADDR_W = "NFDATA 的虚拟地址";
  s3c_nand->dev_ready = s3c2440_dev_ready;
  /*3.硬件相关的操作*/

  /*4.使用 nand_scan*/
  s3c_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
  s3c_mtd->owner = THIS_MODULE;
  s3c_mtd->priv = s3c_nand;

  nand_scan(s3c_mtd, 1);//扫描识别NAND FLASH，构造 mtd_info
  /*5. add_mtd_partition*/
}
module_init(s3c_nand_init);

static void s3c_nand_exit(void)
{

}
module_exit(s3c_nand_exit);

MODULE_LICENSE("GPL");
