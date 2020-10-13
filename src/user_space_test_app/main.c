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
	long delay;
	long ret_ioctl;
	char * message;
	info.group = 2;
	fd = open_group(&info);
	if(fd<0){
		perror("group not opened");
		return -1;
	} 
	printf("fd is %d\n", fd);
	// 1 sec
	delay = 1000;
	ret_ioctl = set_message_delay(fd, delay);
	// ret_ioctl = sleep_tid(fd, gettid());
	message = malloc(sizeof(char)*STRING_LEN);
	sprintf(message, "Ciao mundo!");
	ret = write_message(fd, message, strlen("Ciao mundo!"));
	if(ret<0){
		printf("Error in writing\n");
		return -1;
	}
	sleep(2);
	sprintf(message, " ");
	ret = read_message(fd, message, STRING_LEN);
	if(ret<0){
		printf("Error in reading\n");
		return -1;
	}
	printf("END %s\n", message);
	// ioctl(fd, IOCTL_POPULATE, &info);
	// printf("%s\n", info.payload);

	return 0;

}
