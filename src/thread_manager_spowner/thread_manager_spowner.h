#pragma once

#include <linux/ioctl.h>
// #include <linux/device.h>
#include "../common.h"

#define MYDEV_IOC_MAGIC 'R'
#define DRIVER_NAME  "thread_manager_spowner"

extern struct class * dev_cl_group ;
// static struct class * dev_cl_group;
// #define IOCTL_PRINTK	 		_IO(MYDEV_IOC_MAGIC, 1)
#define IOCTL_INSTALL_GROUP_T 			_IOR(MYDEV_IOC_MAGIC, 1, groupt *)
// #define IOCTL_POPULATE	 		_IOW(MYDEV_IOC_MAGIC, 3, ioctl_info*)
