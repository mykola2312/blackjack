#include <unistd.h>
#include "process.h"

int main(int argc, char** argv)
{
    process_status_t status = {};
    process_parse_status(getpid(), &status);

    printf("name: %s\n", status.name);
    printf("umask: %d\n", status.umask);
    printf("state: %d\n", status.state);
    printf("tgid: %d\n", status.tgid);
    printf("ngid: %d\n", status.ngid);
    printf("pid: %d\n", status.pid);
    printf("ppid: %d\n", status.ppid);
    printf("tracer_pid: %d\n", status.tracer_pid);
    printf("uid: %d\n", status.uid);
    printf("gid: %d\n", status.gid);

    return 0;
}