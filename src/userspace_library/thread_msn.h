#include "../thread_manager_spowner/thread_manager_spowner.h"


// this functions tries to open a group, if it exists it returns 
// a File Descriptor of the file we are operating to.
// If the group does not exists, the function tries to install it.
// if the operation fails, a number < 0 is returned.
int open_group(groupt * group_descriptor);

// tries to write a message into the group managed by the given file descriptor
int write_message(int file_descriptor, char * message, int max_length);

// read a message(if any) from the group managed by the given file descriptor
int read_message(int file_descriptor, char * buffer, int max_length);