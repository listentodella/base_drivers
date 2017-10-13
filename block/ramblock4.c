/**
 * 参考：
 drivers/block/xd.c
 drivers/block/z2ram.c
 进行读写，根据log可以看出，读写操作并不是立即完成的，要么一直读，要么一直写，而不会读写读写……


fdisk /dev/ramblock








 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/blkpg.h>

#include <asm/system.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/dma.h>


#define RAMBLOCK_SIZE (1024 * 1024)
static unsigned char *ramblock_buf;

static int major;

static struct gendisk *ramblock_disk;
static request_queue_t *ramblock_queue;
static DEFINE_SPINLOCK(ramblock_lock);


/*容量 = heads * cylinders * sectors * 512*/
static int ramblock_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	// geo->heads = unit[drive].type->heads;
	// geo->sectors = unit[drive].dtype->sects * unit[drive].type->sect_mult;
	// geo->cylinders = unit[drive].type->tracks;
	geo->heads = 2;//2面
	geo->cylinders = 32;//32环
	geo->sectors = RAMBLOCK_SIZE / 2 / 32 / 512;//多少分区

	return 0;
}

static struct block_device_operations ramblock_fops = {
	.owner	= THIS_MODULE,
	.getgeo = ramblock_getgeo,
};

static void do_ramblock_request (struct request_queue_t *q)
{
	static int r_cnt = 0;
	static int w_cnt = 0;
	struct request *req;
	// printk("%s cnt = %d\n", __func__, cnt++);
	while((req = elv_next_request(q)) != NULL) {
		/*数据传输三要素：源 目的 长度*/
		/*源/目的：*/
		unsigned long offset = req->sector << 9;//相对于乘以512，因为一个扇区的大小是512
		/*目的/源*/
		//req->buffer
		/*长度*/
		unsigned long len = req->current_nr_sectors << 9;

		if(rq_data_dir(req) == READ) {//如果是读
			printk("%s read_cnt = %d\n", __func__, ++r_cnt);
			memcpy(req->buffer, ramblock_buf + offset, len);
		} else {//否则是写
			printk("%s write_cnt = %d\n", __func__, ++w_cnt);
			memcpy(ramblock_buf + offset, req->buffer, len);
		}

		end_request(req, 1);//1表示成功，0表示失败
	}

}

static int ramblock_init(void)
{
  /*1.分配一个gendisk结构体*/
  ramblock_disk = alloc_disk(16);/*次设备号个数：分区个数+1, 设为16，则实际可以有的分区数为15*/
  /*2.设置*/
    /*2.1 分配 设置队列：提供读写能力*/
  ramblock_queue = blk_init_queue(do_ramblock_request, &ramblock_lock);
  ramblock_disk->queue = ramblock_queue;
    /*2.2 设置其他属性：比如容量*/
  major = register_blkdev(0, "ramblock");//cat /proc/devices
  ramblock_disk->major = major;
	ramblock_disk->first_minor = 0;
	sprintf(ramblock_disk->disk_name, "ramblock");
	ramblock_disk->fops = &ramblock_fops;

  set_capacity(ramblock_disk, RAMBLOCK_SIZE / 512);

	/*3.硬件相关操作*/
	ramblock_buf = kzalloc(RAMBLOCK_SIZE, GFP_KERNEL);


  /*4.注册*/
  add_disk(ramblock_disk);
}

static void ramblock_exit(void)
{
	unregister_blkdev(major, "ramblock");
	del_gendisk(ramblock_disk);
	put_disk(ramblock_disk);
	blk_cleanup_queue(ramblock_queue);

	kfree(ramblock_buf);
}

module_init(ramblock_init);
module_exit(ramblock_exit);
MODULE_LICENSE("GPL");
