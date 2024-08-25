#include "relf/relf.h"
#include <dlfcn.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    relf_t dummy;
    relf_open(&dummy, argv[1]);

    printf("image %p\n", dummy.image);
    printf("type %u\n", dummy.type);

    relf_close(&dummy);

    // dlopen
    void* so = dlopen(argv[1], RTLD_NOW);
    printf("dummy_function1 %p\n", dlsym(so, "dummy_function1"));
    dlclose(so);
    
    return 0;
}
