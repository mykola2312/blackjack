#include "process.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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