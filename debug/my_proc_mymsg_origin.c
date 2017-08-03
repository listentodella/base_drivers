#include <...>
/*
 *尽管fops并没有进行任何的操作，但是这里已经可以在proc文件系统里创建mymsg了
 *当然还不可以进行读
 */

static proc_dir_entry *myentry;

static struct file_operations proc_mymsg_operations = {

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
