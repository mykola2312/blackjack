#define _DEFAULT_SOURCE
#include "blackjack/process.h"
#include "blackjack/debug.h"
#include "blackjack/process_debug.h"
#include <sys/capability.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <linux/elf.h>
#include <unistd.h>
#include <string.h>

int process_ptrace_permissions()
{
    if (!geteuid())
    {
        // we're running as root
        return 1;
    }

    // otherwise, check CAPS
    cap_t cap = cap_get_pid(getpid());
    cap_flag_value_t cap_flag_value;
    
    if (cap)
    {
        if (!cap_get_flag(cap, CAP_SYS_ADMIN, CAP_EFFECTIVE, &cap_flag_value))
            if (cap_flag_value == CAP_SET) return 1;
        if (!cap_get_flag(cap, CAP_SYS_ADMIN, CAP_PERMITTED, &cap_flag_value))
            if (cap_flag_value == CAP_SET) return 1;
    }

    return 0;
}

int process_attach_all(procstat_status_t* threads, size_t thread_count)
{
    for (size_t i = 0; i < thread_count; i++)
    {
        pid_t pid = threads[i].pid;
        if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0)
        {
            // we've encountered error. now we must detach from attached and return 1
            process_detach_all(threads, i+1);
            return 1;
        }

        // now wait for thread to be stopped
        int wstatus;
        do {
            waitpid(pid, &wstatus, 0);
#if DEBUG
            if (WIFEXITED(wstatus)) {
                TRACE("exited, status=%d\n", WEXITSTATUS(wstatus));
            } else if (WIFSIGNALED(wstatus)) {
                TRACE("killed by signal %d\n", WTERMSIG(wstatus));
            } else if (WIFSTOPPED(wstatus)) {
                TRACE("stopped by signal %d\n", WSTOPSIG(wstatus));
            } else if (WIFCONTINUED(wstatus)) {
                TRACE("continued\n");
            }

#endif
        } while (!(WIFSTOPPED(wstatus) && WSTOPSIG(wstatus) == SIGSTOP));

        // since thread is properly stopped we can continue to the next one
    }

    return 0;
}

void process_detach_all(procstat_status_t* threads, size_t thread_count)
{
    while (thread_count--) ptrace(PTRACE_DETACH, threads[thread_count].pid, NULL, NULL);
}

// hardcoded syscall instruction size
#define BJ_PTRACE_CONT_OFFSET 2

uintptr_t process_calculate_ip(procstat_status_t* thread, uintptr_t addr)
{
    return addr + BJ_PTRACE_CONT_OFFSET;
}

int process_read_registers(procstat_status_t* thread, struct user_regs_struct* regs)
{
    struct iovec data = {
        .iov_base = regs,
        .iov_len = sizeof(struct user_regs_struct)
    };
    memset(regs, '\0', sizeof(struct user_regs_struct));

    long ret = ptrace(PTRACE_GETREGSET, thread->pid, NT_PRSTATUS, &data);
    print_registers(regs);
    return ret < 0;
}

int process_write_registers(procstat_status_t* thread, const struct user_regs_struct* regs)
{
    struct iovec data = {
        .iov_base = (void*)regs,
        .iov_len = sizeof(struct user_regs_struct)
    };

    long ret = ptrace(PTRACE_SETREGSET, thread->pid, NT_PRSTATUS, &data);
    print_registers(regs);
    return ret < 0;
}