#include "rtdisasm.h"
#include <stdio.h>

extern void test_1();
extern void test_1_end();

int main()
{
    size_t size = (uintptr_t)test_1_end - (uintptr_t)test_1;
    int len = rtdisasm_analyze_single((const uint8_t*)test_1, size);

    printf("len %d\n", len);
    return 0;
}