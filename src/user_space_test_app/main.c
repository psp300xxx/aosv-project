#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include "../thread_manager_spowner/thread_manager_spowner.h"

ioctl_info info;

int main(void) {
	int fd = open("/dev/thread_manager_spowner", O_RDONLY);
	char * buff = (char *) malloc( sizeof(char) * 3 );
	int count = read(fd, buff, 3);
	printf("%s %d\n", buff, count);
	if(fd < 0) {
		perror("Error opening /dev/thread_manager_spowner");
		exit(EXIT_FAILURE);
	}
	// ioctl(fd, IOCTL_PRINTK);

	strcpy((char *__restrict)&info.payload, "From STOCAZZO");
	info.gt = 29;
	ioctl(fd, IOCTL_INSTALL_GROUP_T , &info);
	// ioctl(fd, IOCTL_POPULATE, &info);
	// printf("%s\n", info.payload);

	return 0;

}
