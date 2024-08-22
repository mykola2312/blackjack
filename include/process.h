#ifndef __PROCESS_H
#define __PROCESS_H

#include <sys/types.h>
#include <sys/user.h>

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
// deallocate list with free later
int process_by_name(const char* name, process_status_t** list, size_t* count);

// determine parent process amongst children and set parent pointer to element in list
// process list must consist of parent and children processes,
// obtained from processes_by_name call. of course parent pointer shouldn't be NULL
int process_determine_parent(process_status_t* list, size_t count, process_status_t** parent);

// get all process threads. for list and count same rules applies as for processes_by_name
int process_get_threads(pid_t pid, process_status_t** list, size_t* count);

// returns 1 if state considered active for a process/thread
int process_is_considered_active(process_state_t state);

// find any active (running) thread and returns 0 and success, otherwise non zero
int process_find_active(process_status_t* list, size_t count, process_status_t** thread);

// check if this process has any capability or is ran as root to be able to ptrace attach
int process_ptrace_permissions();

// attach to all threads of the process. on error returns 1 and detaches from already attached
int process_attach_all(process_status_t* threads, size_t thread_count);

// detaches from all threads
void process_detach_all(process_status_t* threads, size_t thread_count);

// read registers of thread. returns 0 on success, 1 on error
int process_read_registers(process_status_t* thread, struct user_regs_struct* regs);

// write registers for thread. for return value same rules apply as read registers function
int process_write_registers(process_status_t* thread, const struct user_regs_struct* regs);

#endif