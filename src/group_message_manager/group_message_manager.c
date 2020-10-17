#include "group_message_manager.h"
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/sched.h>

#include <linux/hashtable.h>
#include <linux/types.h>


#define CONTROL_NUMBER 633443


static int table_initialized = 0;
static DEFINE_HASHTABLE(hash_table, 16);
// node_information * get_device_data(dev_t i_ino);
int is_in_sleeping_tids(int tid, node_information * node_info);
node_information * create_node_info(void);
struct h_node {
    void * data;
    struct hlist_node node_info;
};


inline node_information * create_node_info(){
    node_information * node_info;
    node_info = kmalloc(sizeof(node_information), GFP_KERNEL);
    if(node_info==NULL){
        return NULL;
    }
    node_info -> control_number = CONTROL_NUMBER;
    init_rwsem(&node_info->delivering_semaphore);
    init_rwsem(&node_info->publishing_semaphore);
    init_rwsem(&node_info->sleeping_tid_semaphore);
    INIT_LIST_HEAD(&node_info->delivering_queue.list);
    INIT_LIST_HEAD(&node_info->publishing_queue.list);
    INIT_LIST_HEAD(&node_info->sleeping_tid_list.list);
    node_info->msg_in_delivering = 0;
    node_info->msg_in_publishing = 0;
    node_info ->number_of_sleeping_tid = 0;
    node_info -> open_count = 0;
    return node_info;
}

node_information * get_device_data(dev_t i_ino){
    struct h_node * cur ;
    struct hlist_node * tmp;
    node_information * node_info;
    u32 key;
    key = (u32)MAJOR(i_ino);
    cur = NULL;
    node_info = NULL;
    tmp = NULL;
    hash_for_each_possible_safe(hash_table, cur,tmp, node_info, key) {
        node_info = (node_information *) cur->data;
        if(node_info->control_number == CONTROL_NUMBER){
            return node_info;
        }
    }
    return 0;
}

int is_in_sleeping_tids(int tid, node_information * node_info){
        struct sleeping_tid * sleeper;
        down_read(&node_info->sleeping_tid_semaphore);
        list_for_each_entry(sleeper, &node_info->sleeping_tid_list.list, list){
            if(sleeper->tid == tid){
                up_read(&node_info->sleeping_tid_semaphore);
                return 1;
            }
        }
        up_read(&node_info->sleeping_tid_semaphore);
        return 0;
}



int gmm_open(struct inode *inode, struct file *filp){
    node_information * node_info;
    struct h_node * my_node;
    u32 key;
    key = (u32)MAJOR(inode->i_rdev);
    // INIT HASHTABLE, THIS IS THE FIRST TIME USER OPENS A DEVICE
    if(table_initialized<=0){
        hash_init(hash_table);
        table_initialized++;
    }
    // already inizialized structure for this device
    node_info = get_device_data(inode->i_rdev);
    if ( node_info == NULL ){
        node_info = create_node_info();
        if(node_info==NULL){
            return -1;
        }
        my_node = kmalloc(sizeof(struct h_node), GFP_KERNEL);
        if(my_node==NULL){
            return -1;
        }
        my_node->data = node_info;
        hash_add(hash_table, &my_node->node_info, key);
    }
    filp->private_data = node_info;
    return 0;
}

int gmm_release(struct inode * inode, struct file * filp){
    return 0;
}

void destroy_map(){

}

inline void publish_message_from_publishing_queue(node_information * node_info,struct message_queue ** msg, int length){
    int i;
    i=0;
    for (; i<length; i++){
        list_del(&msg[i]->list);
        list_add_tail(&msg[i]->list, &node_info->delivering_queue.list);
    }
    node_info -> msg_in_publishing -= length;
    node_info -> msg_in_delivering += length;
}

long gmm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
    long ret;
    long delay;
    node_information * node_info;
    struct sleeping_tid * sleeper;
    struct sleeping_tid ** sleepers_ptrs;
    int count = 0;
    ret = 0;
	switch (cmd) {
		case IOCTL_GMM_SET_DELAY:
            delay = arg;
			// copy_from_user(info, (long *) arg, sizeof(long));
            node_info = filp->private_data;
			node_info -> sending_delay = delay;
			goto out;
        case IOCTL_GMM_SLEEP_TID:
            node_info = filp -> private_data;
            down_write(&node_info->sleeping_tid_semaphore);
            // write_lock_irqsave(&node_info->sleeping_tid_lock,node_info->sleeping_lock_flags);
            sleeper = kmalloc(sizeof(struct sleeping_tid), GFP_KERNEL);
            if(sleeper==NULL){
                ret = -ENOMEM;
                goto out;
            }
            sleeper -> tid = arg;
            list_add_tail(&sleeper->list, &node_info->sleeping_tid_list.list);   
            node_info->number_of_sleeping_tid ++;
            // write_unlock_irqrestore(&node_info->sleeping_tid_lock, node_info->sleeping_lock_flags);
            up_write(&node_info->sleeping_tid_semaphore);
            goto out;
        case IOCTL_GMM_AWAKE_TIDS:
            node_info = filp -> private_data;
            down_write(&node_info->sleeping_tid_semaphore);
            // I look for the current entry in sleeping threads.
            sleepers_ptrs = vmalloc(sizeof(struct sleeping_tid *)*(node_info->number_of_sleeping_tid));
            if(sleepers_ptrs==NULL){
                ret = -ENOMEM;
                goto out;
            }
            list_for_each_entry(sleeper, &node_info->sleeping_tid_list.list, list){
                sleepers_ptrs[count++] = sleeper;
            }
            for( ; count>0 ; count--){
                list_del(&sleepers_ptrs[count-1]->list);
            }
            node_info->number_of_sleeping_tid = 0;
            vfree(sleepers_ptrs);
            // write_unlock_irqrestore(&node_info->sleeping_tid_lock, node_info->sleeping_lock_flags);
            up_write(&node_info->sleeping_tid_semaphore);
            goto out;	
	}
    out:
	return ret;
}


inline struct message_queue * get_minimum_message(node_information * node_info){
    struct message_queue * curr;
    struct message_queue * min;
    // read_lock_irqsave(&node_info->publishing_lock,node_info->publishing_lock_flags);
    curr = NULL;
    min = NULL;
    down_read(&node_info->publishing_semaphore);
    list_for_each_entry(curr, &node_info->publishing_queue.list, list){
			if(curr->publishing_time >0 && curr->publishing_time <= ktime_get_boottime()){
                    if (min==NULL){
                        min = curr;
                    }
                    else {
                        if(curr->publishing_time < min->publishing_time){
                            min = curr;
                        }
                    }
			}
	}
    // read_unlock_irqrestore(&node_info->publishing_lock, node_info->publishing_lock_flags);
    up_read(&node_info->publishing_semaphore);
    return min; 
}

int gmm_flush (struct file * filp, fl_owner_t id){
    node_information * node_info;
    struct message_queue * curr;
    node_info = filp->private_data;
    // write_lock_irqsave(&node_info->publishing_lock, node_info->publishing_lock_flags);
    // write_lock_irqsave(&node_info->lock, node_info->lock_flags);
    down_write(&node_info->publishing_semaphore);
    down_write(&node_info->delivering_semaphore);
    while( !list_empty(&node_info->publishing_queue.list) ){
        curr = get_minimum_message(node_info);
        list_del(&curr->list);
        list_add_tail(&curr->list, &node_info->delivering_queue.list);
        node_info->msg_in_delivering++;
        node_info->msg_in_publishing--;
    }
    up_write(&node_info->publishing_semaphore);
    up_write(&node_info->delivering_semaphore);
    // write_unlock_irqrestore(&node_info->lock,node_info->lock_flags);
    // write_unlock_irqrestore(&node_info->publishing_lock,node_info->publishing_lock_flags);
    return 0;
}


ssize_t gmm_read(struct file * file, char * buffer, size_t lenght, loff_t * offset){
    struct message_queue * iter;
    ktime_t current_time;
    int list_is_empty;
    char * err_msg;
    int ret;
    node_information * node_info;
    node_info = file->private_data;
    if(lenght>bytes_per_message){
        return -ENOMEM;
    }
    if ( is_in_sleeping_tids(current->pid, node_info) ){
        return -EACCES;
    }
    current_time = ktime_get_boottime();
    iter =NULL;
    list_is_empty = list_empty(&node_info->publishing_queue.list);
    if( !list_is_empty ){
        iter = get_minimum_message(node_info);
        if(iter==NULL){
            // I have no valid messages, so it's like having an empty queue
            goto empty_queue;
        }
        printk( "min is %s", iter->message->text );
        down_write(&node_info->publishing_semaphore);
        down_write(&node_info->delivering_semaphore);
        list_del(&iter->list);
        list_add_tail(&iter->list, &node_info->delivering_queue.list);
        node_info->msg_in_publishing--;
        node_info->msg_in_delivering++;
        up_write(&node_info->publishing_semaphore);
        up_write(&node_info->delivering_semaphore);
    }
    // print_messages(current_iter, count);
    // I deliver the next message to the user
    // read_lock_irqsave(&node_info->lock, node_info->lock_flags);
    list_is_empty = list_empty(&node_info->delivering_queue.list);
    // read_unlock_irqrestore(&node_info->lock, node_info->lock_flags);
    if( !list_is_empty ){
        // write_lock_irqsave(&node_info->lock,node_info->lock_flags);
        down_write(&node_info->delivering_semaphore);
        iter = list_first_entry(&node_info->delivering_queue.list, struct message_queue, list);
        ret = strlen(iter->message->text);
        copy_to_user(buffer, iter->message->text, ret);
        list_del(&iter->list);
        vfree(iter->message->text);
        kfree(iter->message);
        node_info -> msg_in_delivering--;
        up_write(&node_info->delivering_semaphore);
        goto end;
    }
    else {
        // queue is empty
    empty_queue:
        ret = strlen(QUEUE_EMPTY_MESSAGE);
        err_msg = kmalloc(sizeof(char)*ret, GFP_KERNEL);
        if(err_msg==NULL){
            return -ENOMEM;
        }
        sprintf(err_msg, "%s", QUEUE_EMPTY_MESSAGE);
        copy_to_user(buffer, err_msg, strlen(err_msg));
        ret = 0;
        kfree(err_msg);
    }
    end:
        return ret;
}

ssize_t gmm_write(struct file * file, const char __user * buffer, size_t length, loff_t * offset ){
    int bytes_used;
    struct message_queue * current_message;
    node_information* node_info;
    ktime_t current_time, sending_time;
    node_info = file->private_data;
    bytes_used = (node_info->msg_in_delivering + node_info->msg_in_publishing)*bytes_per_message;
    if(length > bytes_per_message || bytes_used>=total_bytes_in_queue){
        return -ENOMEM;
    }
    if( is_in_sleeping_tids(current->pid, node_info) ){
        return -EACCES;
    }
    if( (current_message = kmalloc(sizeof(struct message_queue), GFP_KERNEL)) == NULL ){
        return -ENOMEM;
    }
    if( ( current_message->message = kmalloc(sizeof(thread_message), GFP_KERNEL) ) == NULL ){
        return -ENOMEM;
    }
    if( ( current_message->message->text = kmalloc(sizeof(char)*bytes_per_message, GFP_KERNEL) ) == NULL ){
        return -ENOMEM;
    }
    if( node_info->sending_delay <=0 ){
        // direct publish in delivering queue
        copy_from_user(current_message->message->text, buffer, length);
        current_message->message->sender = current->pid;
        down_write(&node_info->delivering_semaphore);
        list_add_tail( &current_message->list, &node_info->delivering_queue.list );
        node_info->msg_in_delivering++;
        // write_unlock_irqrestore(&node_info->lock, node_info->lock_flags);
        up_write(&node_info->delivering_semaphore);
        return strlen(current_message->message->text);
    }
    else {
        current_time = ktime_get_boottime();
        sending_time = current_time + node_info -> sending_delay;
        current_message -> message->sender = current->pid;
        copy_from_user(current_message->message->text, buffer, length);
        current_message->publishing_time = sending_time;
        down_write(&node_info->publishing_semaphore);
        list_add_tail( &(current_message->list), &(node_info->publishing_queue.list) );
        node_info -> msg_in_publishing ++;
        up_write(&node_info->publishing_semaphore);
        return length;
    }
}


struct file_operations file_ops_gmm_origin = {
	.open= gmm_open,
    .read= gmm_read,
    .write= gmm_write,
    .release= gmm_release,
    .unlocked_ioctl= gmm_ioctl,
	.compat_ioctl= gmm_ioctl,
    .flush = gmm_flush,
	// unlocked_ioctl: mydev_ioctl,
	// compat_ioctl: mydev_ioctl,
	// release: mydev_release
};
