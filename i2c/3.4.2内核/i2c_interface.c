/*不自己写，应用内核提供的万能i2c驱动
 *用户态直接访问
 * 需要i2c-tools--->i2c-dev.h
 * Documentation/i2c/dev-interface里有详细介绍
 * 这是一个上层app，而不是驱动，这里调用的api并不是自己写的
 * 实际上也不推荐直接使用read write，因为i2c的读写操作有混合成分，单纯的read write不合适
 * 如果该i2c设备已经有驱动了，则该用户态访问是无法成功的
*/

#include <...>
#include "i2c-dev.h"

/**
 * i2c_usr_test <bus> <dev_addr> r addr
 * i2c_usr_test <bus> <dev_addr> w addr val
 * exmaple:
 * 	i2c_usr_test <dev/i2c-0> <dev_addr> ...
 */

void print_usage(char *file)
{
	printf("%s <dev/i2c-0> <dev_addr> r addr\n", file);
	printf("%s <dev/i2c-0> <dev_addr> w addr val\n", file);
}

int main(int argc, char **argv)
{
	int fd;
	unsigned char buf[2];
	int dev_addr;
	unsigned char addr, data;

	if ((argc != 5) && (argc != 6)) {
		print_usage(argv[0]);
		return -1;
	}
	fd = open(argv[1], O_RDWR);
	if (fd < 0) {
		printf("can't open %s\n", argv[1]);
		return -1;
	}

	dev_addr = strtoul(argv[0], NULL, 0);
	if (ioctl(fd, I2C_SLAVE, dev_addr) < 0) {
		printf("set addr error!\n");
		return -1;
	}

	if (strcmp(argv[3], "r") == 0) {
		addr = strtoul(argv[4], NULL, 0);
		data = i2c_smbus_read_word_data(fd, addr);
		printf("data: %c, %d, 0x%2x\n", data, data, data);
	} else if ((strcmp(argv[3], "w") == 0) && (argc == 6) {
		addr = strtoul(argv[4], NULL, 0);
		data = strtoul(argv[5], NULL, 0);
		i2c_smbus_write_byte_data(fd, addr, data);
	} else {
		print_usage(argv[0]);
		return -1;
	}

	return 0;
}
