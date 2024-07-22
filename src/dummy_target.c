#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdio.h>

extern char *program_invocation_name;
extern char *program_invocation_short_name;

void status(const char* tag)
{
    printf("[%s] pid: %d\n", tag, getpid());
    printf("[%s] program_invocation_name: %s\n", tag, program_invocation_name);
    printf("[%s] program_invocation_short_name: %s\n", tag, program_invocation_short_name);
}

__attribute__((noreturn)) void slave1_job()
{
    status("slave1");

    puts("[slave1] waiting for any key...");
    getc(stdin);
    
    _exit(0);
}

__attribute__((noreturn)) void slave2_job()
{
    status("slave2");

    puts("[slave2] will do something each second");
    while (1)
    {
        __asm__("nop");
        sleep(1);
    }
}

__attribute__((noreturn)) void* slave3_job(void*)
{
    status("slave3");

    puts("[slave3] will do something each second but in a thread");
    while (1)
    {
        unsigned a = 3;
        for (unsigned i = 0; i < 100000; i++)
            a = a * 3 - a;
        sleep(1);
    }
}

__attribute__((noreturn)) void* hijack_destination(void*)
{
    puts("thread has been redirected to this function! cool!");
    while (1) sleep(1);
}

int main()
{
    status("master");
    printf("hijack_destination: %p\n", hijack_destination);

    pid_t slave1 = fork();
    if (!slave1) slave1_job();

    pid_t slave2 = fork();
    if (!slave2) slave2_job();

    pthread_t slave3;
    pthread_create(&slave3, NULL, slave3_job, NULL);

    waitpid(slave1, NULL, 0);
    waitpid(slave2, NULL, 0);
    pthread_join(slave3, NULL);
    return 0;
}