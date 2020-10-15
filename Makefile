# ioctl-objs := ./ioctl.o .
obj-m += thread_manager_spowner.o

obj-m += thread_manager_spowner.o
thread_manager_spowner-objs := ./src/thread_manager_spowner/thread_manager_spowner.o ./src/thread_manager_spowner/ioctl_switch_functions.o ./src/group_message_manager/group_message_manager.o


CURRENT_PATH = $(shell pwd)
LINUX_KERNEL = $(shell uname -r)
LINUX_KERNEL_PATH = /lib/modules/$(LINUX_KERNEL)/build/
all:
			make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
			gcc $(CURRENT_PATH)/src/user_space_test_app/main.c $(CURRENT_PATH)/src/userspace_library/thread_msn.c -o $(CURRENT_PATH)/main.out
			gcc $(CURRENT_PATH)/src/user_space_test_app/single_thread_fifo_test.c $(CURRENT_PATH)/src/userspace_library/thread_msn.c -o $(CURRENT_PATH)/tests/single_thread_fifo_test.out
			gcc $(CURRENT_PATH)/src/user_space_test_app/multiple_threads.c $(CURRENT_PATH)/src/userspace_library/thread_msn.c -o $(CURRENT_PATH)/tests/two_threads.out -lpthread


clean:
			make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
			rm main.out
			rm  $(CURRENT_PATH)/tests/*

