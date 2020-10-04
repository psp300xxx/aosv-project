#pragma once

#include <linux/ioctl.h>

#define MYDEV_IOC_MAGIC 'R'
#define DRIVER_NAME  "thread_manager_spowner"

typedef struct _ioctl_info {
	int gt;
} ioctl_info;

// #define IOCTL_PRINTK	 		_IO(MYDEV_IOC_MAGIC, 1)
#define IOCTL_INSTALL_GROUP_T 			_IOR(MYDEV_IOC_MAGIC, 1, ioctl_info *)
// #define IOCTL_POPULATE	 		_IOW(MYDEV_IOC_MAGIC, 3, ioctl_info*)
