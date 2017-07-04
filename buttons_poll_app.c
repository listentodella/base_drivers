#include <...>//自己去man手册查

int main(void)
{

	int fd = open("/dev/buttons", O_RDWR);
	if (fd < 0) {
		perror("open");
		exit(1);
	}

	struct pollfd fds[1];
	fds[0].fd = fd;
	fds[0].events = POLLIN;


	int ret;
	unsigned char key_val;
	while (1) {
		ret = poll(fds, 1, 5000);
		if (ret == 0) 
			printf("time out\n");
		else {
			read(fd, &key_val, 1);
			printf("key_val = 0x%x\n", key_val);
		}
	}
	
	return 0;
}