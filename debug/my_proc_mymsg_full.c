#include <...>
/*
 *填充并仿照printk实现myprint函数
 *并且完善，使得每次cat都可以输出完整的log
 */
#define MYLOG_BUF_LEN 1024

struct proc_dir_entry *myentry;
static char mylog_buf[MYLOG_BUF_LEN];
static char tmp_buf[MYLOG_BUF_LEN];
static int mylog_r = 0;
static int mylog_r_for_read = 0;
static int mylog_w = 0;

static DECLARE_WAIT_QUEUE_HEAD(mymsg_waitq);

static int is_mylog_empty(void)
{
  return (mylog_w == mylog_r);
}

static int is_mylog_empty_for_read(void)
{
  return (mylog_r_for_read == mylog_r);
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
    if((mylog_r_for_read + 1) % MYLOG_BUF_LEN == mylog_r)
      mylog_r_for_read = mylog_r;
  }

  mylog_buf[mylog_w] = c;
  mylog_w = (mylog_w + 1) % MYLOG_BUF_LEN;
  /*唤醒等待数据的进程*/
  wait_up_interruptible(&mymsg_waitq);
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

static int mylog_getc_for_read(char *p)
{
  if (is_mylog_empty_for_read()) {
    return 0;
  }

  *p = mylog_buf[mylog_r_for_read];
  mylog_r_for_read = (mylog_r + 1) % MYLOG_BUF_LEN;

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


static ssize_t mymsg_read(struct file *file, char __user *buf,
			 size_t count, loff_t *ppos)
{
	int error = 0;
  int i = 0;
  char c;
	/*把mylog_buf的数据copy_to_user，然后return*/
  //int cnt = min(1024, count);
  //如果以非阻塞方式打开并且是空的，立即返回
  if((file->f_flags & O_NOBLOCK) && is_mylog_empty_for_read())
    return -EAGAIN;
  //否则休眠
  error = wait_event_interruptible(mymsg_waitq, !is_mylog_emtpy_for_read());
  //直至唤醒,copy_to_user
  while (!error && (mylog_getc_for_read(&c)) && i < count) {
			error = __put_user(c, buf);
      buf++;
      i++;
		}
    if(!error)
      error = i;

	return error;
}


static int mymsg_open(struct inode *inode, struct file *file)
{
  mylog_r_for_read = mylog_r;//这样可以使得每次cat都可以输出全部的log
  return 0;
}

static struct file_operations proc_mymsg_operations = {
  .open = mymsg_open,
  .read = mymsg_read;
};


static int mymsg_init(void)
{
//  snprintf(mylog_buf, "hello mymsg_buf");
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
EXPORT_SYMBOL(myprintk);

MODULE_LICENSE("GPL");
