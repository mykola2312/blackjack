#define _DEFAULT_SOURCE
#include "process.h"
#include "debug.h"
#include "process_debug.h"
#include <sys/capability.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <linux/elf.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

int process_parse_status(pid_t pid, process_status_t* status)
{
    char statusPath[256] = {0};
    snprintf(statusPath, sizeof(statusPath), "/proc/%d/status", pid);

    int statusFd = open(statusPath, O_RDONLY);
    if (statusFd < 0) return 1;

    char buffer[4096] = {0};
    int rd = read(statusFd, buffer, sizeof(buffer));
    close(statusFd);

    if (rd < 1) return 1;

    char* lineptr = NULL, *line = strtok_r(buffer, "\n", &lineptr);
    while (line != NULL)
    {
        char* fieldptr = NULL;
        const char* key = (const char*)strtok_r(line, ":\t", &fieldptr);
        const char* value = (const char*)strtok_r(NULL, ":\t", &fieldptr);

        if (!strcmp(key, "Name"))
            strncpy(status->name, value, MAX_PROCESS_NAME);
        else if (!strcmp(key, "Umask"))
            status->umask = (unsigned int)strtoul(value, NULL, 8);
        else if (!strcmp(key, "State"))
            status->state = (process_state_t)value[0];
        else if (!strcmp(key, "Tgid"))
            status->tgid = atoi(value);
        else if (!strcmp(key, "Ngid"))
            status->ngid = atoi(value);
        else if (!strcmp(key, "Pid"))
            status->pid = atoi(value);
        else if (!strcmp(key, "PPid"))
            status->ppid = atoi(value);
        else if (!strcmp(key, "TracerPid"))
            status->tracer_pid = atoi(value);
        else if (!strcmp(key, "Uid"))
            status->uid = atoi(value);
        else if (!strcmp(key, "Gid"))
            status->gid = atoi(value);

        line = strtok_r(NULL, "\n", &lineptr);
    }

    return 0;
}

static int is_numeric(const char* str)
{
    for (const char* p = str; *p; p++)
        if (!isdigit(*p)) return 0;

    return 1;
}

int process_by_name(const char* name, process_status_t** list, size_t* count)
{
    *list = NULL;
    *count = 0;

    DIR* proc = opendir("/proc");
    if (!proc) return 1;

    struct dirent* entry;
    while ((entry = readdir(proc)))
    {
        if (entry->d_type != DT_DIR) continue;
        if (!is_numeric(entry->d_name)) continue;

        pid_t pid = strtod(entry->d_name, NULL);
        process_status_t status = {};
        if (process_parse_status(pid, &status))
        {
            TRACE("process parse status failed for %d\n", pid);
            continue;
        }

        if (!strcasecmp(status.name, name))
        {
            // we have match! lets add it to list
            size_t _count = *count;
            process_status_t* _list = *list;
            
            if (_count) _list = (process_status_t*)realloc(_list, ++_count * sizeof(process_status_t));
            else _list = (process_status_t*)malloc(++_count * sizeof(process_status_t));

            if (!_list)
            {
                TRACE("out of memory for process status!\n");
                closedir(proc);
                return 1;
            }

            // copy process status to list
            memcpy(&_list[_count - 1], &status, sizeof(process_status_t));

            // update pointers
            *list = _list;
            *count = _count;
        }
    }

    closedir(proc);
    return 0;
}

static process_status_t* process_by_pid_in_list(pid_t pid, process_status_t* list, size_t count)
{
    for (size_t i = 0; i < count; i++)
        if (list[i].pid == pid) return &list[i];
    
    return NULL;
}

int process_determine_parent(process_status_t* list, size_t count, process_status_t** parent)
{
    // we're gonna find any process that doesnt have parent in this list,
    // that means we hit real parent, not descendant
    for (size_t i = 0; i < count; i++)
    {
        if (!process_by_pid_in_list(list[i].ppid, list, count))
        {
            // that's real parent
            *parent = &list[i];
            return 0;
        }
    }

    return 1;
}

int process_get_threads(pid_t pid, process_status_t** list, size_t* count)
{
    *list = NULL;
    *count = 0;

    char taskPath[256] = {0};
    snprintf(taskPath, sizeof(taskPath), "/proc/%d/task", pid);

    DIR* taskDir = opendir(taskPath);
    if (!taskDir) return 1;

    struct dirent* entry;
    while ((entry = readdir(taskDir)))
    {
        if (entry->d_type != DT_DIR) continue;
        if (!is_numeric(entry->d_name)) continue;

        pid_t pid = strtod(entry->d_name, NULL);
        process_status_t status = {};
        if (process_parse_status(pid, &status))
        {
            // if we can't some threads, we should fail completely
            // failure is better than incomplete info
            TRACE("failed to parse %d task status\n", pid);
            closedir(taskDir);
            return 1;
        }

        // add thread to list
        process_status_t* _list = *list;
        size_t _count = *count;

        if (_count) _list = (process_status_t*)realloc(_list, ++_count * sizeof(process_status_t));
        else _list = (process_status_t*)malloc(++_count * sizeof(process_status_t));

        if (!_list)
        {
            TRACE("out of memory for process status!\n");
            closedir(taskDir);
            return 1;
        }

        // copy thread to list
        memcpy(&_list[_count - 1], &status, sizeof(process_status_t));

        *list = _list;
        *count = _count;
    }

    closedir(taskDir);
    return 0;
}

int process_is_considered_active(process_state_t state)
{
    return state == INTERRUPTIBLE_SLEEP || state == RUNNING;
}

int process_find_active(process_status_t* list, size_t count, process_status_t** thread)
{
    for (size_t i = 0; i < count; i++)
    {
        TRACE("task %d state %d\n", list[i].pid, list[i].state);
        if (process_is_considered_active(list[i].state))
        {
            *thread = &list[i];
            return 0;
        }
    }
    return 1;
}

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

int process_attach_all(process_status_t* threads, size_t thread_count)
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

void process_detach_all(process_status_t* threads, size_t thread_count)
{
    while (thread_count--) ptrace(PTRACE_DETACH, threads[thread_count].pid, NULL, NULL);
}

// hardcoded syscall instruction size
#define BJ_PTRACE_CONT_OFFSET 2

uintptr_t process_calculate_ip(process_status_t* thread, uintptr_t addr)
{
    return addr + BJ_PTRACE_CONT_OFFSET;
}

int process_read_registers(process_status_t* thread, struct user_regs_struct* regs)
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

int process_write_registers(process_status_t* thread, const struct user_regs_struct* regs)
{
    struct iovec data = {
        .iov_base = (void*)regs,
        .iov_len = sizeof(struct user_regs_struct)
    };

    long ret = ptrace(PTRACE_SETREGSET, thread->pid, NT_PRSTATUS, &data);
    print_registers(regs);
    return ret < 0;
}