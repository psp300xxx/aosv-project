
#define MESSAGE_LENGTH 50

typedef struct {
    int group;
    int open_times;
} groupt;

typedef struct {
    char message [MESSAGE_LENGTH];
    unsigned long time_of_deliver;
    pid_t sender;
} thread_message;