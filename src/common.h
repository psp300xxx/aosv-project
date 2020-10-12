// #include <sys/types.h>
#define MESSAGE_LENGTH 50

typedef struct {
    int group;
    int open_times;
} groupt;

typedef struct {
    char message [MESSAGE_LENGTH];
    int sender;
} thread_message;