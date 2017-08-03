#include <...>
/*
 *简单添加读操作的验证，挂载驱动成功后可以模仿cat /proc/kmsg一样cat它
 *当然结果只能是mymsg read
 */

static proc_dir_entry *myentry;

static ssize_t kmsg_read(struct file *file, char __user *buf,
			 size_t count, loff_t *ppos)
{
	printk("mymsg read\n");
	return 0;
}


static struct file_operations proc_mymsg_operations = {
  .read = mymsg_read;
};


static int mymsg_init(void)
{
  /*仿照，S_ISUSR就是为什么kmsg是只读的原因*/
  myentry = create_proc_entry("mymsg", S_ISUSR, &proc_root);
  if (myentry)
    myentry->proc_fops = &proc_mymsg_operations;
  return 0;
}

static void mymsg_exit(void)
{
  remove_proc_entry("mymsg", &proc_root);
}
module_init(mymsg_init);
module_exit(mymsg_exit);
MODULE_LICENSE("GPL");
