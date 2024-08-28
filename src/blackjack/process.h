#ifndef __PROCESS_H
#define __PROCESS_H

#include "blackjack/procstat.h"
#include <sys/types.h>
#include <sys/user.h>
#include <stdint.h>

// check if this process has any capability or is ran as root to be able to ptrace attach
int process_ptrace_permissions();

// attach to all threads of the process. on error returns 1 and detaches from already attached
int process_attach_all(procstat_status_t* threads, size_t thread_count);

// detaches from all threads
void process_detach_all(procstat_status_t* threads, size_t thread_count);

// calculate correct instruction pointer address for hijacking
uintptr_t process_calculate_ip(procstat_status_t* thread, uintptr_t addr);

// read registers of thread. returns 0 on success, 1 on error
int process_read_registers(procstat_status_t* thread, struct user_regs_struct* regs);

// write registers for thread. for return value same rules apply as read registers function
int process_write_registers(procstat_status_t* thread, const struct user_regs_struct* regs);

#endif