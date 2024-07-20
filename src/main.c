#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "process.h"

void print_process(process_status_t* proc)
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
    process_status_t* list = NULL;
    size_t count = 0;

    processes_by_name("dummy_target", &list, &count);
    for (size_t i = 0; i < count; i++)
        print_process(&list[i]);
    
    free(list);
    return 0;
}