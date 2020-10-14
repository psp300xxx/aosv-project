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


const int max_array_elements = 1000;

const int NUMBER_OF_THREADS = 10;

const int number_of_elements = 100;

#define MAX_STRING_lEN 20

void * read_and_write(groupt * group_desc);

// current string val
char num_string [MAX_STRING_lEN];

void * read_and_write(groupt * group_desc){
    printf("ENTERING THREAD %d\n", group_desc->group);
    char ** array;
    int fd;
    int ret;
    char * message;
    message = malloc(sizeof(char)*MAX_STRING_lEN);
    if(message==NULL){
        perror("memory");
        return NULL;
    }
    array = malloc(sizeof(char *)*number_of_elements);
    if(array==NULL){
        perror("memory");
        return NULL;
    }
    for(int i =0; i<number_of_elements;i++){
        array[i] = malloc(sizeof(char)*MAX_STRING_lEN);
        if(array[i]==NULL){
            perror("memory");
            return NULL;
        }
        sprintf(array[i], "From tid %d, %d\n", gettid(), i);
    }
    fd = open_group(group_desc);
    if(fd<0){
        perror("opening group");
        return NULL;
    }
    printf("fd is %d\n", fd);
    for(int i = 0 ; i<number_of_elements; i++){
        ret = write_message(fd, array[i], MAX_STRING_lEN);
        if(ret<0){
            perror("writing");
            return NULL;
        }
        sleep(0.2);
    }  
    for(int i = 0 ; i<number_of_elements; i++){
        ret = read_message(fd, message, MAX_STRING_lEN);
        if(ret<0){
            perror("reading");
            return NULL;
        }
        printf("thread %d read %s\n", gettid(), message);
    }
    // free data
    free(message);
    for(int i =0 ; i<number_of_elements; i++){
        free(array[i]);
    }
    free(array);  
}


int main(void){
    // set up data for tests
    srand(getpid());
    groupt * group_descriptor;
    int * array;
    int number_of_elements;
    int current_int;
    int ret;
    group_descriptor = malloc(sizeof(groupt));
    if(group_descriptor==NULL){
        perror("No memory!");
        return -1;
    }
    group_descriptor->group = 23;
    pthread_t * thread_id;
    thread_id = malloc(sizeof(thread_id)*NUMBER_OF_THREADS);
    if(thread_id==NULL){
        perror("memory");
    }
    int start_id = rand()%1000+100;
    for(int i = 0; i<NUMBER_OF_THREADS; i++){
        thread_id[i] = start_id++;
        if(pthread_create(&thread_id[i], NULL, read_and_write, group_descriptor)) {
            fprintf(stderr, "Error creating thread %d \n", i);
            return 1;
        }
    }

    int finished =0;
    while(!finished){
        sleep(10);
    }
    free(group_descriptor);
    free(thread_id);
    return 0;
}