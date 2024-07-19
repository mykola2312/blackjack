#include <unistd.h>
#include <stdio.h>

extern char *program_invocation_name;
extern char *program_invocation_short_name;

int main()
{
    printf("pid: %d\n", getpid());
    printf("program_invocation_name: %s\n", program_invocation_name);
    printf("program_invocation_short_name: %s\n", program_invocation_short_name);

    puts("waiting for any key...");
    getc(stdin);
    return 0;
}