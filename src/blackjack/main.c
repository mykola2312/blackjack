#define _DEFAULT_SOURCE
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "blackjack/procstat.h"
#include "blackjack/process.h"

void print_status(procstat_status_t* proc)
{
    puts("Process:");
    printf("name: %s\n", proc->name);
    printf("umask: %d\n", proc->umask);
    printf("state: %d\n", proc->state);
    printf("tgid: %d\n", proc->tgid);
    printf("ngid: %d\n", proc->ngid);
    printf("pid: %d\n", proc->pid);
    printf("ppid: %d\n", proc->ppid);
    printf("tracer_pid: %d\n", proc->tracer_pid);
    printf("uid: %d\n", proc->uid);
    printf("gid: %d\n", proc->gid);
}

int main(int argc, char** argv)
{
    procstat_status_t* list = NULL;
    size_t count = 0;

    // find process
    procstat_by_name("dummy_target", &list, &count);
    // get real parent
    procstat_status_t* parent;
    if (procstat_determine_parent(list, count, &parent))
    {
        fputs("unable to determine parent process. exiting\n", stderr);
        free(list);
        return 1;
    }

    print_status(parent);

    // find active thread
    puts("Looking for active thread..");
    
    procstat_status_t* threads = NULL;
    size_t thread_count = 0;
    
    procstat_status_t* active;
    while (1)
    {
        if (procstat_get_threads(parent->pid, &threads, &thread_count))
        {
            fputs("failed to obtain process threads\n", stderr);
            free(list);
            return 1;
        }

        if (procstat_find_active(threads, thread_count, &active))
        {
            // no active threads - free list and continue
            free(threads);
            usleep(500*1000);
        }
        else
        {
            // we got active thread!
            break;
        }
    }

    puts("Active thread:");
    print_status(active);

    if (!process_ptrace_permissions())
    {
        fputs("this process doesn't have permission to ptrace.\n", stderr);
        fputs("either run as root or set caps.\n", stderr);
        return 1;
    }

    // attach
    if (process_attach_all(threads, thread_count))
    {
        fprintf(stderr, "failed to attach: %s\n", strerror(errno));
        
        free(threads);
        free(list);
        
        return 1;
    }
    
    puts("attached to all threads. please enter address of destination");
    void* destination;
    scanf("%p", &destination);

    struct user_regs_struct regs;
    if (process_read_registers(active, &regs))
        fprintf(stderr, "failed to read registers: %s\n", strerror(errno));
    // hijack instruction pointer to our destination
    regs.rip = process_calculate_ip(active, (uintptr_t)destination);
    if (process_write_registers(active, &regs))
        fprintf(stderr, "failed to write registers: %s\n", strerror(errno));

    process_detach_all(threads, thread_count);
    
    free(threads);
    free(list);
    
    return 0;
}