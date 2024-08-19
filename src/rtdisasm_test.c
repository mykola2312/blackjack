#include "rtdisasm.h"
#include <stdio.h>

extern void test_1();
extern void test_1_end();

int main()
{
    size_t size = (uintptr_t)test_1_end - (uintptr_t)test_1;
    printf("size %lu\n", size);
    // int len = rtdisasm_analyze_single((const uint8_t*)test_1, size, NULL);
    // printf("rtdisasm_analyze_single: len %d\n", len);

    int offset = rtdisasm_find_target((const uint8_t*)test_1, size, RT_TARGET_NOP);
    printf("rtdisasm_find_target: offset %d\n", offset);
    return 0;
}