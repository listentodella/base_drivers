/*
 *“秒”字符设备驱动
 *   它在被打开的时候初始化一个定时器并将其添加到内核定时器链表中，每秒输出一次当前的jiffies（为此定时器处理函数中每次都要修改新的expires
 */
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define SECOND_MAJOR 248

static int second_major = SECOND_MAJOR;
module_param(second_major, int, S_IRUGO);

struct second_dev {
	struct cdev cdev;
	atomic_t counter;
	struct timer_list s_timer;
};
static struct second_dev *second_devp;


static void second_timer_handler(unsigned long arg)
{
	mod_timer(&second_devp->s_timer, jiffies + HZ);//触发下一次定时
	atomic_inc(&second_devp->counter);//增加秒计数

	printk(KERN_INFO "current jiffies is %ld\n", jiffies);
}

static int second_open(struct inode *inode, struct file *filp)
{
 	init_timer(&second_devp->s_timer);
 	second_devp->s_timer.function = &second_timer_handler;
 	second_devp->s_timer.expires = jiffies + HZ;

 	add_timer(&second_devp->s_timer);
 	atomic_set(&second_devp->counter, 0);//初始化秒计数为0

 	return 0;
}


static int second_release(struct inode *inode, struct file *filp)
{
	del_timer(&second_devp->s_timer);
	return 0;
}


static ssize_t second_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	int counter = atomic_read(&second_devp->counter);
	if (put_user(counter, (int *)buf))//复制counter到userspace
		return -EFAULT;
	else
		return sizeof(unsigned int);
}


static const struct file_operations second_fops = {
	.owner = THIS_MODULE,
	.open = second_open,
	.release = second_release;
	.read = second_read,
};


static void second_setup_cdev(struct second_dev *pdev, int index)
{
	int  err, devno = MKDEV(second_major, index);

	cdev_init(&pdev->cdev, &second_fops);
	pdev->cdev.owner = THIS_MODULE;
	err = cdev_add(&pdev->cdev, devno, 1);
	if (err) 
		printk(KERN_ERR "Failed to add second device\n");
}


static __init second_init(void)
{
	int ret;
	dev_t devno = MKDEV(second_major, 0);

	if (second_major) 
		ret = register_chrdev_region(devno, 1, "second");
	else {
		ret = alloc_chrdev_region(&devno, 0, 1, "second");
		second_major = MAJOR(devno);
	}
	if (ret < 0) 
		return ret;

	second_devp = kzalloc(sizeof(*second_devp), GFP_KERNEL);
	if (!second_devp) {
		ret = -ENOMEM;
		goto fail_malloc;
	}

	second_setup_cdev(second_devp, 0);

	return 0;

fail_malloc:
	unregister_chrdev_region(devno, 1);
	return ret;
}
module_init(second_init);

static __exit second_exit(void)
{
	cdev_del(&second_devp->cdev);
	kfree(second_devp);
	unregister_chrdev_region(MKDEV(second_major, 0), 1);
}
module_exit(second_exit);

MODULE_LICENSE("GPL");