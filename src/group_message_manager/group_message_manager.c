#include "group_message_manager.h"
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/hashtable.h>
#include <linux/types.h>
#include "../thread_manager_spowner/thread_manager_spowner.h"

#define CONTROL_NUMBER 633443

static int table_initialized = 0;
static int open_control = 0;
DECLARE_WAIT_QUEUE_HEAD(wait_queue);
static DEFINE_HASHTABLE(hash_table, 16);
node_information * has_device_data(dev_t i_ino);
struct h_node {
    void * data;
    struct hlist_node node_info;
};

node_information * create_node_info(void);

node_information * create_node_info(){
    node_information * node_info;
    node_info = kmalloc(sizeof(node_information), GFP_KERNEL);
    node_info -> control_number = CONTROL_NUMBER;
    node_info -> open_count = 0;
    return node_info;
}

node_information * has_device_data(dev_t i_ino){
    int count;
    struct h_node * cur ;
    struct hlist_node * tmp;
    node_information * node_info;
    u32 key;
    key = (u32)MAJOR(i_ino);
    count=0;
    cur = NULL;
    node_info = NULL;
    tmp = NULL;
    hash_for_each_possible_safe(hash_table, cur,tmp, node_info, key) {
        node_info = (node_information *) cur->data;
        if(node_info->control_number == CONTROL_NUMBER){
            return node_info;
        }
        count++;
    }
    return 0;
}



int gmm_open(struct inode *inode, struct file *filp){
    struct h_node * my_node;
    struct h_node * cur;
    struct hlist_node * tmp;
    node_information * node_info;
    u32 key;
    if(open_control>0){
        return -EBUSY;
    }
    // open_control++;
    key = (u32)MAJOR(inode->i_rdev);
    if(table_initialized<=0){
        hash_init(hash_table);
        printk(KERN_ALERT "INIT HASHMAP\n");
        table_initialized++;
    }
    // already inizialized structure for this device
    node_info = has_device_data(inode->i_rdev);
    if ( node_info == NULL ){
        node_info = create_node_info();
        my_node = kmalloc(sizeof(struct h_node), GFP_KERNEL);
        my_node->data = node_info;
        hash_add(hash_table, &my_node->node_info, key);
    }
    printk(KERN_ALERT "OPENS %ld\n", ++(node_info->open_count));
    filp->private_data = node_info;
    return 0;
}

int gmm_release(struct inode * inode, struct file * filp){
    open_control--;
    printk(KERN_ALERT "CLOSING\n");
    return 0;
}

void destroy_map(){

}


ssize_t gmm_read(struct file * file, char * buffer, size_t lenght, loff_t * offset){
    char * msg;
    node_information * node_info;
    // printk(KERN_ALERT "TEST IF I ARRIVE HERE %p\n", file->private_data);
    node_info = (node_information *)file->private_data;
    msg = kmalloc(sizeof(char)*20,0);
    printk(KERN_ALERT "count val %d\n", node_info->open_count);
    sprintf(msg, "OP %d\n",node_info->open_count);
    copy_to_user(buffer, msg, strlen(msg));
    kfree(msg);
    return strlen(msg);
}

ssize_t gmm_write(struct file * file, const char __user * buffer, size_t lenght, loff_t * offset ){
    return 0;
}

struct file_operations file_ops_gmm_origin = {
	open: gmm_open,
    read: gmm_read,
    write: gmm_write,
    release:gmm_release,
	// unlocked_ioctl: mydev_ioctl,
	// compat_ioctl: mydev_ioctl,
	// release: mydev_release
};
// EXPORT_SYMBOL(file_ops_gmm_origin);