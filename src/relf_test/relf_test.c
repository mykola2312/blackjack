#include "relf/relf.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    relf_t dummy;
    relf_open(&dummy, argv[1]);

    printf("image %p\n", dummy.image);
    printf("type %u\n", dummy.type);
    return 0;
}
