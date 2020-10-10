#pragma once
#include <linux/fs.h>


typedef struct {
    // message_queue
    // message_waiting_queue
    // sleeping_threads_list
    // reading semaphore
    // writing semaphore
    unsigned long open_count;
    unsigned long control_number;
} node_information;

void destroy_map(void);


int gmm_open(struct inode *inode, struct file *filp);

ssize_t gmm_read(struct file * file, char * buffer, size_t lenght, loff_t * offset);

ssize_t gmm_write(struct file * file, const char __user * buffer, size_t lenght, loff_t * offset );

int gmm_release(struct inode * inode, struct file * filp);

extern struct file_operations file_ops_gmm_origin;

