#pragma once
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/timekeeping.h>
// #include "../common.h"
#include "../thread_manager_spowner/thread_manager_spowner.h"


#define QUEUE_EMPTY_MESSAGE "Error: Queue is empty\n"


//list that will contain a queue of messages
struct message_queue {
    thread_message * message;
    ktime_t publishing_time;
    struct list_head list;
};

// list containing the list of pids that are currently sleeping
struct sleeping_tid{
    pid_t tid;
    struct list_head list;
};


typedef struct {
    unsigned long open_count;
    unsigned long control_number;
    unsigned long lock_flags;
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

