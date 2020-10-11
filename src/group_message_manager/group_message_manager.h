#pragma once
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/timekeeping.h>

//list that will contain a queue of messages
struct message_queue {
    char * message;
    pid_t tid_sender;
    ktime_t publishing_time;
    struct list_head list;
};

// list containing the list of pids that are currently sleeping
struct sleeping_tid{
    pid_t tid;
    struct list_head list;
};


typedef struct {
    // message_queue
    // message_waiting_queue
    // sleeping_threads_list
    // reading semaphore
    // writing semaphore
    unsigned long open_count;
    unsigned long control_number;
    rwlock_t lock;
    ktime_t sending_delay;
    struct mutex writing_mutex;
    struct message_queue delivering_queue;
    struct message_queue publishing_queue;
    struct sleeping_tid sleeping_tid_list;
} node_information;




void destroy_map(void);


int gmm_open(struct inode *inode, struct file *filp);

ssize_t gmm_read(struct file * file, char * buffer, size_t lenght, loff_t * offset);

ssize_t gmm_write(struct file * file, const char __user * buffer, size_t lenght, loff_t * offset );

int gmm_release(struct inode * inode, struct file * filp);

extern struct file_operations file_ops_gmm_origin;

