#include "thread_msn.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#define MAIN_DEVICE "/dev/thread_manager_spowner"

#define SUB_DEVICES "/dev/synch/GROUP_MESSAGE_MANAGER%d"

#define SLEEPING_TIME 0.1 //0.1 seconds

// this functions tries to open a group, if it exists it returns 
// a File Descriptor of the file we are operating to.
// If the group does not exists, the function tries to install it.
// if the operation fails, a number < 0 is returned.
int open_group(groupt * group_descriptor){
    int ret;
    int fd;
    int group_fd;
    int tries;
    char * group_to_open;
    fd = open(MAIN_DEVICE, O_RDWR);
	if(fd < 0) {
		perror("Error opening main device");
		return -1;
	}
    ret = ioctl(fd, IOCTL_INSTALL_GROUP_T , group_descriptor);
    if (ret<0){
        perror("Error installing new group");
		return -1;
    }
    group_to_open = malloc(sizeof(char)* strlen(SUB_DEVICES)*2);
    if(group_to_open==NULL){
        perror("mem error");
        return -1;
    }
    sprintf(group_to_open, SUB_DEVICES, group_descriptor->group);
    // Since I have to wait udev creates the symbolic links, I try to do it
    // 10 times before making the operation fail
    tries = 10;
    while(tries>=0){
        group_fd = open(group_to_open, O_RDWR);
        if(group_fd>0){
            goto end;
        }
        sleep(SLEEPING_TIME);
        tries--;
    }
end:
    free(group_to_open);
    return group_fd;
}

// tries to write a message into the group managed by the given file descriptor
// I assume message has been already allocated
int write_message(int file_descriptor, char * message, int max_length){
    int ret;
    ret = write(file_descriptor,message, max_length);
    return ret;
}

// read a message(if any) from the group managed by the given file descriptor
int read_message(int file_descriptor, char * buffer, int max_length){
    int ret;
    ret = read(file_descriptor,buffer, max_length);
    return ret;
}