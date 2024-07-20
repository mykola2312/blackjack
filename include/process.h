#ifndef __PROCESS_H
#define __PROCESS_H

#include <sys/types.h>

typedef enum {
    UNINTERRUPTABLE_SLEEP   = 'D',
    IDLE_KERNEL_THREAD      = 'I',
    RUNNING                 = 'R',
    INTERRUPTIBLE_SLEEP     = 'S',
    STOPPED                 = 'T',
    STOPPED_BY_DEBUGGER     = 't',
    DEAD                    = 'X',
    ZOMBIE                  = 'Z'
} process_state_t;

#define MAX_PROCESS_NAME 256
typedef struct {
    char name[MAX_PROCESS_NAME];
    mode_t umask;
    process_state_t state;
    pid_t tgid;
    pid_t ngid;
    pid_t pid;
    pid_t ppid;
    pid_t tracer_pid;
    uid_t uid;
    gid_t gid;
} process_status_t;

// parse process status from procfs. returns 0 no errors and 1 on any kind of error
// error information obtain from errno
int process_parse_status(pid_t pid, process_status_t* status);

// find any process that matches name, case insensitive.
// list pointer must point to NULL-initialized pointer, and count pointer must pount to initialized 0
// will skip any process which status couldn't be parsed 
int processes_by_name(const char* name, process_status_t** list, size_t* count);

#endif