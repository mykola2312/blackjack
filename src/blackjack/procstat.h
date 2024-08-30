#ifndef __PROCSTAT_H
#define __PROCSTAT_H

#include <stdint.h>
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
} procstat_state_t;

typedef struct {
    char* name;
    mode_t umask;
    procstat_state_t state;
    pid_t tgid;
    pid_t ngid;
    pid_t pid;
    pid_t ppid;
    pid_t tracer_pid;
    uid_t uid;
    gid_t gid;
} procstat_status_t;

// parse process status from procfs. returns 0 no errors and 1 on any kind of error
// error information obtain from errno
int procstat_parse_status(pid_t pid, procstat_status_t* status);

// free all procstat status list entries
void procstat_free_status_list(procstat_status_t* list, size_t count);

// find any process that matches name, case insensitive.
// list pointer must point to NULL-initialized pointer, and count pointer must pount to initialized 0
// will skip any process which status couldn't be parsed 
// deallocate list with free later
int procstat_by_name(const char* name, procstat_status_t** list, size_t* count);

// determine parent process amongst children and set parent pointer to element in list
// process list must consist of parent and children processes,
// obtained from processes_by_name call. of course parent pointer shouldn't be NULL
int procstat_determine_parent(procstat_status_t* list, size_t count, procstat_status_t** parent);

// get all process threads. for list and count same rules applies as for processes_by_name
int procstat_get_threads(pid_t pid, procstat_status_t** list, size_t* count);

// returns 1 if state considered active for a process/thread
int procstat_is_considered_active(procstat_state_t state);

// find any active (running) thread and returns 0 and success, otherwise non zero
int procstat_find_active(procstat_status_t* list, size_t count, procstat_status_t** thread);

typedef struct {
    uint64_t v_start;
    uint64_t v_end;
    
    int prot;
    int flags;

    uint64_t f_offset;
    
    int dev_major;
    int dev_minor;

    uint64_t inode;

    char* path;         // don't forget to free
} procstat_map_t;

// parse process file mappings. return 0 on success and -1 on error
int procstat_parse_maps(pid_t pid, procstat_map_t** maps, size_t* count);

// will take care of freeing everything related to procstat_map_t lists
void procstat_free_maps(procstat_map_t* maps, size_t count);

// list of modules that are mapped into process memory
typedef struct {
    char* path;
    char* name;

    uint64_t v_base;    // address base mapped image

    procstat_map_t* maps;
    unsigned map_count;
} procstat_module_t;

typedef struct {
    procstat_module_t* modules;
    unsigned module_count;
} procstat_modules_t;

// analyze file mappings and parse them into modules
// that way, we can figure where libc.so.6 loaded in memory
procstat_modules_t* procstat_parse_modules(procstat_map_t* maps, size_t count);

// free any allocated memory by procstat_parse_modules
void procstat_free_modules(procstat_modules_t* modules);

#endif