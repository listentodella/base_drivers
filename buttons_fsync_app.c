#include <...>//自己去man手册查


int fd;

void my_signal_fun(int signum)
{
	unsigned char key_val;
	read(fd, &key_val, 1);
	printf("key_val = 0x%x\n", key_val);
}


int main(void)
{
	//int ret;

	signal(SIGIO, my_signal_fun);

	fd = open("/dev/buttons", O_RDWR);
	if (fd < 0) {
		perror("open");
		exit(1);
	}

	fcntl(fd, F_SETOWN, getpid());
	int Oflags = fcntl(fd, F_GETFL);
	/*改变fasync标记，最终会调用到驱动的faync =》 fasync_helper:
	*初始化 or 释放fasync_struct
	*/
	fcntl(fd, F_SETFL, Oflags | FASYNC);

	while (1) {
		sleep(1000);
	}
	
	return 0;
}