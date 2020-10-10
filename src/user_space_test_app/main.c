#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>
#include <sys/types.h>
// #include "../common.h"
#include "../thread_manager_spowner/thread_manager_spowner.h"
#define STRING_LEN 20

groupt info;

int main(void) {
	int fd = open("/dev/thread_manager_spowner", O_RDONLY);
	if(fd < 0) {
		perror("Error opening /dev/thread_manager_spowner");
		exit(EXIT_FAILURE);
	}
	info.group = 29;
	int ret = ioctl(fd, IOCTL_INSTALL_GROUP_T , &info);
	info.group = 29;
	printf("ioctl29 first %d\n", ret);
	ret = ioctl(fd, IOCTL_INSTALL_GROUP_T , &info);
	printf("ioctl29 second ret %d\n", ret);
	fd = open("/dev/synch/GROUP_MESSAGE_MANAGER29", O_RDONLY);
	if(fd<0){
		printf("not opened first \n");
		return 0;
	}
	char * buf;
	buf = malloc(sizeof(char)*STRING_LEN);
	int readen = read(fd, buf, STRING_LEN);
	printf("read first %s\n", buf);
	ret = close(fd);
	printf("before second open, close ret %d\n", ret);
	fd = open("/dev/synch/GROUP_MESSAGE_MANAGER29", O_RDONLY);
	if(fd<0){
		printf("not opened second \n");
		return 0;
	}
	printf("before second read\n");
	readen = read(fd, buf, STRING_LEN);
	printf("read second %s\n", buf);
	close(fd);
	free(buf);
	// ioctl(fd, IOCTL_POPULATE, &info);
	// printf("%s\n", info.payload);

	return 0;

}
