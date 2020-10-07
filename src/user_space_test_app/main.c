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

groupt info;

int main(void) {
	int fd = open("/dev/thread_manager_spowner", O_RDONLY);
	printf("fd is %d\n", fd);
	char * buff = (char *) malloc( sizeof(char) * 3 );
	int count = read(fd, buff, 3);
	printf("%s tid is %d %d\n", buff, gettid(),count);
	if(fd < 0) {
		perror("Error opening /dev/thread_manager_spowner");
		exit(EXIT_FAILURE);
	}
	// ioctl(fd, IOCTL_PRINTK);

	// strcpy((char *__restrict)&info.payload, "From STOCAZZO");
	info.group = 29;
	ioctl(fd, IOCTL_INSTALL_GROUP_T , &info);
	info.group = 30;
	printf("ioctl2\n");
	ioctl(fd, IOCTL_INSTALL_GROUP_T , &info);
	printf("ioctl2 ret\n");


	// ioctl(fd, IOCTL_POPULATE, &info);
	// printf("%s\n", info.payload);

	return 0;

}
