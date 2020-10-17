#include <linux/ioctl.h>

#include "../common.h"

#define HASH_BITS_USED 20

#define MYDEV_IOC_MAGIC 'R'
#define DRIVER_NAME  "thread_manager_spowner"



extern struct class * dev_cl_group ;


#define IOCTL_INSTALL_GROUP_T 			_IOR(MYDEV_IOC_MAGIC, 1, groupt *)
