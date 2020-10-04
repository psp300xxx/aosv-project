#include <linux/kernel.h>
#include <linux/module.h>
#include "ioctl_switch_functions.h"

long set_new_driver( int group ){
    printk(KERN_ALERT "INSTALLING GROUP %d", group);
    return 0;
}