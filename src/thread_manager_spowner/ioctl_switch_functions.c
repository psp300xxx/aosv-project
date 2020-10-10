#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/fs.h>
#include "ioctl_switch_functions.h"
#include "thread_manager_spowner.h"
#include "../group_message_manager/group_message_manager.h"
#include <linux/kdev_t.h>
#include <linux/types.h>


struct device *device_group = NULL;
EXPORT_SYMBOL(device_group);


char * driver_name = NULL;
int major_number;
static dev_t  current_devt = 1;
static int number_devices = 0;
struct devices_created devices;
int list_devices_created =0;

int current_open(struct inode *inode, struct file *filp);
static ssize_t current_read(struct file * file, char * buffer, size_t lenght, loff_t * offset);


int unregister_and_destroy_all_devices(){
		struct devices_created * iter;
		char * current_driver_name;
		current_driver_name = kmalloc(sizeof(char)*20,0);
		list_for_each_entry(iter, &devices.list, list){
				sprintf(current_driver_name, DRIVER_NAME_NUMB, iter->group);
				device_destroy(dev_cl_group, iter->devt);
		}
		class_unregister(dev_cl_group);
		class_destroy(dev_cl_group);
		list_for_each_entry(iter, &devices.list, list){
				sprintf(current_driver_name, DRIVER_NAME_NUMB, iter->group);
				unregister_chrdev(iter->major_number, current_driver_name);
		}
		kfree(current_driver_name);
}

long set_new_driver( int group ){
    int ret;
    struct devices_created * iter;
	if(list_devices_created){
		list_for_each_entry(iter, &devices.list, list){
				if(iter->group == group){
					return 0;
				}
		}
	}
	printk(KERN_ERR "dev cl gourp %d", dev_cl_group);
    ret = init_new_device(group);
    if (!ret){
        if (!number_devices){
            INIT_LIST_HEAD(&devices.list);
			list_devices_created=1;
        }
        ret = add_group_list(&devices, group, current_devt, major_number);
		number_devices++;
		printk(KERN_DEBUG "number devices %d ", number_devices);
        return 0;
    }
    return -1;
}

int add_group_list(struct devices_created * devs, int group, dev_t new_devt, int major){
	struct devices_created *tmp;
	tmp = (struct devices_created *)kmalloc(sizeof(struct devices_created), 0);
	if (!tmp) {
		return -1;
	}
	tmp -> group = group;
	tmp -> devt = new_devt;
	tmp -> major_number = major;
	list_add_tail(&(tmp->list), &(devs->list));
    return group;
}


static ssize_t current_read(struct file * file, char * buffer, size_t lenght, loff_t * offset){
	char * total;
	struct devices_created * iter;
	total = kmalloc(sizeof(char)*40,0);
	list_for_each_entry(iter, &devices.list, list){
			sprintf(total, "%s%d",total, iter->group);
	}
	copy_to_user(buffer, total, lenght);
	kfree(total);
	return lenght;
}

int current_open(struct inode *inode, struct file *filp) {

	return 0;
}



int  init_new_device(int group)
{
	int err;
	char * driver_name ;
	driver_name = kmalloc(sizeof(char) * 20,0 );
	sprintf(driver_name, DRIVER_NAME_NUMB, group);
	major_number = register_chrdev(0, driver_name, &file_ops_gmm_origin);
    printk(KERN_DEBUG "%s major number %d",driver_name, major_number);

	// Dynamically allocate a major_number for the device
	if (major_number < 0) {
		printk(KERN_ERR "%s: Failed registering char device\n", driver_name);
		err = major_number;
		goto finish;
	}

	// Create a device in the previously created class
    printk(KERN_ERR "%d number devices", number_devices);
	// if(current_devt==-1){
	current_devt = MKDEV(major_number,0);
	// }
	// else {
    // 	current_devt = MKDEV(MAJOR(current_devt), MINOR(current_devt)+1);
	// }
	device_group = device_create(dev_cl_group, NULL, current_devt, NULL, driver_name);
	if (IS_ERR(device_group)) {
		printk(KERN_ERR "%s: failed to create device group %ld\n", driver_name, dev_cl_group);
		pr_err("%s:%d error code %ld \n", __func__, __LINE__, PTR_ERR(device_group));
		err = PTR_ERR(device_group);
		goto failed_classreg;
	}

	printk(KERN_INFO "%s: special device registered with major_number number %d\n", driver_name, major_number);

	return 0;
failed_classreg:
	unregister_chrdev(major_number, driver_name);
finish:
	kfree(driver_name);
	return err;
}
