#include <iostream>
#include "process.h"

int main(int argc, char** argv)
{
    auto proc = Process::FindByName("dummy_target");
    if (proc) proc = proc.value();
    else 
    {
        fputs("process not found\n", stderr);
        return 1;
    }

    return 0;
}