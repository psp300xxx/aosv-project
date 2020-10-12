#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
// #include "../common.h"
#include "../userspace_library/thread_msn.h"
#define STRING_LEN 20

groupt info;

int main(void) {
	int fd;
	int ret;
	char * message;
	info.group = 2;
	fd = open_group(&info);
	if(fd<0){
		perror("group not opened");
		return -1;
	} 
	printf("fd is %d\n", fd);
	message = malloc(sizeof(char)*STRING_LEN);
	sprintf(message, " ");
	ret = read_message(fd, message, STRING_LEN);
	if(ret<0){
		printf("Error in reading");
		return -1;
	}
	printf("%s\n", message);
	if(message==NULL){
		perror("mem error");
		return -1;
	}
	sprintf(message, "test msg");
	ret = write_message(fd, message, STRING_LEN);
	if(ret<0){
		perror("Error in writing");
		return -1;
	}
	sprintf(message, " ");
	ret = read_message(fd, message, STRING_LEN);
	if(ret<0){
		printf("Error in reading");
		return -1;
	}
	printf("END %s\n", message);
	// ioctl(fd, IOCTL_POPULATE, &info);
	// printf("%s\n", info.payload);

	return 0;

}
