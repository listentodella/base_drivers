#include <...>
/*
 *填充并仿照printk实现myprint函数
 *
 */
#define MYLOG_BUF_LEN 1024

struct proc_dir_entry *myentry;
static char mylog_buf[MYLOG_BUF_LEN];
static char tmp_buf[MYLOG_BUF_LEN];
static int mylog_r = 0;
static int mylog_w = 0;

static int is_mylog_empty(void)
{
  return (mylog_w == mylog_r);
}

static int is_mylog_full(void)
{
  return ((mylog_w + 1) % MYLOG_BUF_LEN == mylog_r);
}

static void mylog_putc(char c)
{
  if (is_mylog_full()) {
    /*丢弃一个数据*/
    mylog_r = (mylog_r + 1) % MYLOG_BUF_LEN;
  }

  mylog_buf[mylog_w] = c;
  mylog_w = (mylog_w + 1) % MYLOG_BUF_LEN;

  return;
}

static int mylog_getc(char *p)
{
  if (is_mylog_empty()) {
    return 0;
  }

  *p = mylog_buf[mylog_r];
  mylog_r = (mylog_r + 1) % MYLOG_BUF_LEN;

  return 1;
}

/*
int sprintf(char * buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i=vsnprintf(buf, INT_MAX, fmt, args);
	va_end(args);
	return i;
}
*/
int myprintk(const char *fmt, ...)
{
	va_list args;
	int i, j;

	va_start(args, fmt);
	i = vsnprintf(tmp_buf, INT_MAX, fmt, args);
	va_end(args);

  for(j = 0; j < i; j++) {
    mylog_putc(tmp_buf[i]);
  }

	return i;
}









static ssize_t kmsg_read(struct file *file, char __user *buf,
			 size_t count, loff_t *ppos)
{
	//printk("mymsg read\n");
	/*把mylog_buf的数据copy_to_user，然后return*/
  //int cnt = min(1024, count);
  copy_to_user(buf, mylog_buf, 10);

	return 10;
}


static struct file_operations proc_mymsg_operations = {
  .read = mymsg_read;
};


static int mymsg_init(void)
{
  snprintf(mylog_buf, "hello mymsg_buf");
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
