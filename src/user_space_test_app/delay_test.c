#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include "../userspace_library/thread_msn.h"
#include <pthread.h>

const long millis_delay = 2000;
char * message;
const int message_len = 20;
#define MESSAGE_TEXT "TEST DELAY"

int main(void){
        // set up data for tests
    srand(getpid());
    groupt * group_descriptor;
    int val;
    int ret;
    group_descriptor = malloc(sizeof(groupt));
    message = malloc(sizeof(char)*message_len);
    if(group_descriptor==NULL || message==NULL){
        perror("No memory!");
        return -1;
    }
    group_descriptor->group = rand()%(100);
    int fd = open_group(group_descriptor);
    printf("FD is %d\n", fd);
    if(fd<0){
        perror("Group not opened");
    }
    // setting delay
    printf("Setting delay\n");
    ret = set_message_delay(fd, millis_delay);
    // I'm writing a message to the queueS
    if(ret<0){
        perror("Problems in delaying");
        return -1;
    }
    printf("Delay set\n");
    sprintf(message, MESSAGE_TEXT);
    ret = write_message(fd,message, strlen(message));
    if(ret<0){
        perror("writing");
        return -1;
    }
    // I read the message, since the delay has not passed, I should not read the previous message
    sprintf(message, " ");
    ret = read_message(fd, message, message_len);
    printf("Before waiting :%s\n", message);
    sleep(millis_delay/1000);
    // I read again the messages
    ret = read_message(fd, message, message_len);
    printf("After waiting: %s\n",message);
    close_group(fd);
    free(group_descriptor);
    free(message);
}