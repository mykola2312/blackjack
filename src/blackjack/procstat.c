#define _DEFAULT_SOURCE
#include "blackjack/procstat.h"
#include "blackjack/debug.h"
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <ctype.h>

int procstat_parse_status(pid_t pid, procstat_status_t* status)
{
    char statusPath[256] = {0};
    snprintf(statusPath, sizeof(statusPath), "/proc/%d/status", pid);

    int statusFd = open(statusPath, O_RDONLY);
    if (statusFd < 0) return 1;

    char buffer[4096] = {0};
    int rd = read(statusFd, buffer, sizeof(buffer));
    close(statusFd);

    if (rd < 1) return 1;

    memset(status, '\0', sizeof(procstat_status_t));

    char* lineptr = NULL, *line = strtok_r(buffer, "\n", &lineptr);
    while (line != NULL)
    {
        char* fieldptr = NULL;
        const char* key = (const char*)strtok_r(line, ":\t", &fieldptr);
        const char* value = (const char*)strtok_r(NULL, ":\t", &fieldptr);

        if (!strcmp(key, "Name"))
        {
            if (value) status->name = strdup(value);
        }
        else if (!strcmp(key, "Umask"))
            status->umask = (unsigned int)strtoul(value, NULL, 8);
        else if (!strcmp(key, "State"))
            status->state = (procstat_state_t)value[0];
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

void procstat_free_status_list(procstat_status_t* list, size_t count)
{
    for (unsigned i = 0; i < count; i++)
        if (list[i].name) free(list[i].name);
    free(list);
}

static int is_numeric(const char* str)
{
    for (const char* p = str; *p; p++)
        if (!isdigit(*p)) return 0;

    return 1;
}

int procstat_by_name(const char* name, procstat_status_t** list, size_t* count)
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
        procstat_status_t status = {};
        if (procstat_parse_status(pid, &status))
        {
            TRACE("process parse status failed for %d\n", pid);
            continue;
        }

        if (!strcasecmp(status.name, name))
        {
            // we have match! lets add it to list
            size_t _count = *count;
            procstat_status_t* _list = *list;
            
            if (_count) _list = (procstat_status_t*)realloc(_list, ++_count * sizeof(procstat_status_t));
            else _list = (procstat_status_t*)malloc(++_count * sizeof(procstat_status_t));

            if (!_list)
            {
                TRACE("out of memory for process status!\n");
                closedir(proc);
                return 1;
            }

            // copy process status to list
            memcpy(&_list[_count - 1], &status, sizeof(procstat_status_t));

            // update pointers
            *list = _list;
            *count = _count;
        }
    }

    closedir(proc);
    return 0;
}

static procstat_status_t* procstat_by_pid_in_list(pid_t pid, procstat_status_t* list, size_t count)
{
    for (size_t i = 0; i < count; i++)
        if (list[i].pid == pid) return &list[i];
    
    return NULL;
}

int procstat_determine_parent(procstat_status_t* list, size_t count, procstat_status_t** parent)
{
    // we're gonna find any process that doesnt have parent in this list,
    // that means we hit real parent, not descendant
    for (size_t i = 0; i < count; i++)
    {
        if (!procstat_by_pid_in_list(list[i].ppid, list, count))
        {
            // that's real parent
            *parent = &list[i];
            return 0;
        }
    }

    return 1;
}

int procstat_get_threads(pid_t pid, procstat_status_t** list, size_t* count)
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
        procstat_status_t status = {};
        if (procstat_parse_status(pid, &status))
        {
            // if we can't some threads, we should fail completely
            // failure is better than incomplete info
            TRACE("failed to parse %d task status\n", pid);
            closedir(taskDir);
            return 1;
        }

        // add thread to list
        procstat_status_t* _list = *list;
        size_t _count = *count;

        if (_count) _list = (procstat_status_t*)realloc(_list, ++_count * sizeof(procstat_status_t));
        else _list = (procstat_status_t*)malloc(++_count * sizeof(procstat_status_t));

        if (!_list)
        {
            TRACE("out of memory for process status!\n");
            closedir(taskDir);
            return 1;
        }

        // copy thread to list
        memcpy(&_list[_count - 1], &status, sizeof(procstat_status_t));

        *list = _list;
        *count = _count;
    }

    closedir(taskDir);
    return 0;
}

int procstat_is_considered_active(procstat_state_t state)
{
    return state == INTERRUPTIBLE_SLEEP || state == RUNNING;
}

int procstat_find_active(procstat_status_t* list, size_t count, procstat_status_t** thread)
{
    for (size_t i = 0; i < count; i++)
    {
        TRACE("task %d state %d\n", list[i].pid, list[i].state);
        if (procstat_is_considered_active(list[i].state))
        {
            *thread = &list[i];
            return 0;
        }
    }
    return 1;
}

void procstat_free_map(procstat_map_t* map)
{
    if (map->path) free(map->path);
}

procstat_maps_t* procstat_parse_maps(pid_t pid)
{
    // open proc maps
    char mapsPath[256] = {0};
    snprintf(mapsPath, sizeof(mapsPath), "/proc/%d/maps", pid);

    int fd = open(mapsPath, O_RDONLY);
    if (fd < 0) return NULL;
    
    // now, block by block read contents
    const unsigned blockSize = 512;
    
    char* buffer = (char*)malloc(blockSize);
    unsigned bufferOffset = 0;

    while (1)
    {
        ssize_t rd = read(fd, buffer + bufferOffset, blockSize);
        if (rd == -1)
        {
            // error
            free(buffer);
            close(fd);
            return NULL;
        }
        else if (rd < blockSize)
        {
            // we've encountered last block, so set bufferOffset
            // and terminate last line with zero byte
            bufferOffset += rd;
            buffer[bufferOffset] = '\0';
            // finish reading
            break;
        }
        else
        {
            // so rd is blockSize precisely, that means
            // we need go further and read next block
            bufferOffset += blockSize;
            buffer = (char*)realloc(buffer, bufferOffset + blockSize);
        }
    }
    // we got our maps buffer, now we can close file
    close(fd);
    
    procstat_maps_t* maps;
    MLIST_ALLOC(procstat_maps_t, maps);

    char* lineptr = NULL, *line = strtok_r(buffer, "\n", &lineptr);
    while (line != NULL)
    {
        // now we need to parse fields
        char* fieldptr = NULL;
        const char* v_range = strtok_r(line, " ", &fieldptr);
        const char* perms = strtok_r(NULL, " ", &fieldptr);
        const char* offset = strtok_r(NULL, " ", &fieldptr);
        const char* device = strtok_r(NULL, " ", &fieldptr);
        const char* inode = strtok_r(NULL, " ", &fieldptr);
        const char* pathname = strtok_r(NULL, " ", &fieldptr);

        // allocate new map entry
        MLIST_ADD(procstat_maps_t, maps);
        
        procstat_map_t* map = (procstat_map_t*)MLIST_LAST(procstat_maps_t, maps);
        memset(map, '\0', sizeof(procstat_map_t));
        // v_start and v_end
        sscanf(v_range, "%"SCNx64 "-" "%"SCNx64, &map->v_start, &map->v_end);
        // prot and flags
        for (const char* c = perms; *c; c++)
        {
            switch (*c)
            {
                // prot
                case 'r': map->prot |= PROT_READ; break;
                case 'w': map->prot |= PROT_WRITE; break;
                case 'x': map->prot |= PROT_EXEC; break;
                // flags
                case 'p': map->flags |= MAP_PRIVATE; break;
                case 's': map->flags |= MAP_SHARED; break;
                case '-': continue;
                default:
                    TRACE("unknown perm %c\n", *c);
                    break;
            }
        }
        // f_offset
        map->f_offset = strtoull(offset, NULL, 16);
        // dev_major and dev_minor
        sscanf(device, "%d:%d", &map->dev_major, &map->dev_minor);
        // inode
        map->inode = strtoull(inode, NULL, 10);
        // path
        if (pathname) map->path = strdup(pathname);

        TRACE("map v_start %lx v_end %lx prot %x flags %x f_offset %lx dev_major %u dev_minor %u inode %lu path %s\n",
            map->v_start, map->v_end,
            map->prot, map->flags,
            map->f_offset,
            map->dev_major, map->dev_minor,
            map->inode,
            map->path
        );

        line = strtok_r(NULL, "\n", &lineptr);
    }

    free(buffer);

    return maps;
}

void procstat_free_maps(procstat_maps_t* maps)
{
    MLIST_FREE(procstat_maps_t, maps);
}
