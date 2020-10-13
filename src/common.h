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

// IOCTL INFORMATIONS

#define GMM_IOC_MAGIC 'R'

#define IOCTL_GMM_SET_DELAY _IOW(GMM_IOC_MAGIC, 1, long )

#define IOCTL_GMM_SLEEP_TID _IOW(GMM_IOC_MAGIC, 2, long )

#define IOCTL_GMM_AWAKE_TIDS _IO(GMM_IOC_MAGIC,3 )

#define IOCTL_GMM_REVOKE_DELAYED _IO(GMM_IOC_MAGIC,4)