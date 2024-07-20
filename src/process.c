#define _DEFAULT_SOURCE
#include "process.h"
#include "debug.h"
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

int processes_by_name(const char* name, process_status_t** list, size_t* count)
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

int determine_parent_process(process_status_t* list, size_t count, process_status_t** parent)
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

int is_considered_active(process_state_t state)
{
    return state == INTERRUPTIBLE_SLEEP || state == RUNNING;
}

int find_active_thread(process_status_t* list, size_t count, process_status_t** thread)
{
    for (size_t i = 0; i < count; i++)
    {
        TRACE("task %d state %d\n", list[i].pid, list[i].state);
        if (is_considered_active(list[i].state))
        {
            *thread = &list[i];
            return 0;
        }
    }
    return 1;
}