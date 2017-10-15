/**
 * 参考 drivers/mtd/maps/physmap.c
 * 可识别，有分区
 */

#include "..."

static struct map_info *s3c_nor_map;
static struct mtd_info *s3c_nor_mtd;

static struct mtd_partitions s3c_nor_parts[] = {
  [0] = {
    .name = "bootloader",
    .size = 0x00040000,
    .offset = 0,
  },
  [1] = {
    .name = "root_nor",
    .offset = MTDPART_OFS_APPEND,
    .size = MTDPART_SIZ_FULL,
  }
};

static int s3c_nor_init(void)
{
  /*1.分配map_info结构体*/
  s3c_nor_map = kzalloc(sizeof(struct map_info), GFP_KERNEL);

  /*2.设置：物理基地址（phys） 大小（size） 位宽（bankwidth） 虚拟基地址（virt）*/
  s3c_nor_map->name = "s3d_nor";
  s3c_nor_map->phys = 0;
  s3c_nor_map->size = 0x100000;//16M 一定要 ≥ NOR的真正大小
  s3c_nor_map->bankwidth = 2;
  s3c_nor_map->virt = ioremap(s3c_nor_map->phys, s3c_nor_map->size);
  simple_map_init(s3c_nor_map);

  /*3.使用：调用NOR FLASH协议层提供的函数来识别*/
  printk("use cfi_probe...\n");
  s3c_nor_mtd = do_map_probe("cfi_probe", s3c_nor_map);
  if(!s3c_nor_mtd) {//如果没有识别说明不支持这个方法,使用旧的
    printk("use jedec_probe...\n");
    s3c_nor_mtd = do_map_probe("jedec_probe", s3c_nor_map);
  }
  if(!s3c_nor_mtd) {//如果还是失败，退出
    iounmap(s3c_nor_map->virt);
    kfree("s3c_nor_map");
    return -1;
  }

  /*4.add_mtd_partitions*/
  add_mtd_partitions(s3c_nor_mtd, s3c_nor_parts, 2);

  return 0;
}
module_init(s3c_nor_init);

static void s3c_nor_exit(void)
{
  del_mtd_partitions(s3c_nor_mtd);
  iounmap(s3c_nor_map->virt);
  kfree("s3c_nor_map");
}
module_exit(s3c_nor_exit);

MODULE_LICENSE("GPL");
