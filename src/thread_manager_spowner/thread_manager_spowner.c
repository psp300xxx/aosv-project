#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/fs.h>
// #include <linux/device.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <asm-generic/errno-base.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include "ioctl_switch_functions.h"
#include "thread_manager_spowner.h"

MODULE_AUTHOR("Luigi De Marco <demarco.1850504@studenti.uniroma1.it>");
MODULE_DESCRIPTION("ioctl example");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");

static ssize_t my_read(struct file * file, char __user * buffer, size_t lenght, loff_t * offset);
long mydev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
int mydev_open(struct inode *inode, struct file *filp);
int mydev_release(struct inode *inode, struct file *filp);
struct class * dev_cl_group =NULL;
int major_secondary = -1;
// EXPORT_SYMBOL(dev_cl_group);

char * msg;
/// Only one process at a time can interact with this mutex
static DEFINE_MUTEX(mydev_mutex);

/// File operations for the module
struct file_operations fops = {
	open: mydev_open,
    read: my_read,
	unlocked_ioctl: mydev_ioctl,
	compat_ioctl: mydev_ioctl,
	release: mydev_release
};

static ssize_t my_read(struct file * file, char * buffer, size_t lenght, loff_t * offset){
	int i;
	char * tmp;
	int msg_len;
	int written;
    i = 0;
	written = 0;
	tmp = msg;
	msg_len = strlen(msg);
	while(msg_len){
        	put_user(*(tmp++), buffer++);
		msg_len--;	
		written++;	
	}
    return written;
}


long mydev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	groupt info;
	switch (cmd) {
		case IOCTL_INSTALL_GROUP_T:
			copy_from_user(&info, (groupt *) arg, sizeof(groupt));
			ret = set_new_driver(info.group);
			goto out;
	}

    out:
	return ret;
}



// Variables to correctly setup/shutdown the pseudo device file
static int major;
static struct class *dev_cl = NULL;
static struct device *device = NULL;


int mydev_open(struct inode *inode, struct file *filp) {

	// It's meaningless to open this device in write mode
	if (((filp->f_flags & O_ACCMODE) == O_WRONLY) || ((filp->f_flags & O_ACCMODE) == O_RDWR)) {
	 	return -EACCES;
	}

	// Only one access at a time
	if (!mutex_trylock(&mydev_mutex)) {
		printk(KERN_INFO "%s: Trying to open an already-opened special device file\n", DRIVER_NAME);
		return -EBUSY;
	}

	return 0;
}


int mydev_release(struct inode *inode, struct file *filp)
{
	mutex_unlock(&mydev_mutex);

	return 0;
}


static int __init mydev_init(void)
{
	int err;

	major = register_chrdev(0, DRIVER_NAME, &fops);
	msg = kmalloc(sizeof(char)* 20, 0);
	msg = "hola";
	// Dynamically allocate a major for the device
	if (major < 0) {
		printk(KERN_ERR "%s: Failed registering char device\n", DRIVER_NAME);
		err = major;
		goto first_fail;
	}

	// Create a class for the device
	dev_cl = class_create(THIS_MODULE, "mydev");
	if (IS_ERR(dev_cl)) {
		printk(KERN_ERR "%s: failed to register device class\n", DRIVER_NAME);
		err = PTR_ERR(dev_cl);
		goto failed_classreg1;
	}

	dev_cl_group = class_create(THIS_MODULE, "GROUP_MSG_MNGR");
	if (IS_ERR(dev_cl_group)) {
		printk(KERN_ERR "%s: failed to register device class group mgr\n", DRIVER_NAME);
		err = PTR_ERR(dev_cl_group);
		goto failed_classreg2;
	}
	printk(KERN_ERR "ok registered group class\n");

	// Create a device in the previously created class
	device = device_create(dev_cl, NULL, MKDEV(major, 0), NULL, DRIVER_NAME);
	if (IS_ERR(device)) {
		printk(KERN_ERR "%s: failed to create device\n", DRIVER_NAME);
		err = PTR_ERR(device);
		goto failed_devreg;
	}

	printk(KERN_INFO "%s: special device registered with major number %d\n", DRIVER_NAME, major);

	return 0;

failed_devreg:
	class_unregister(dev_cl_group);
	class_destroy(dev_cl_group);
failed_classreg2:
	class_unregister(dev_cl);
	class_destroy(dev_cl);
failed_classreg1:
	unregister_chrdev(major, DRIVER_NAME);
first_fail:
	return err;
}

static void __exit mydev_exit(void)
{
	device_destroy(dev_cl, MKDEV(major, 0));
	class_unregister(dev_cl);
	class_destroy(dev_cl);
	unregister_chrdev(major, DRIVER_NAME);
	unregister_and_destroy_all_devices();
}


module_init(mydev_init)
module_exit(mydev_exit)
