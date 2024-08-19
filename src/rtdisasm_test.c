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

    printf("RT_TARGET_NOP %d\n", rtdisasm_find_target((const uint8_t*)test_1, size, RT_TARGET_NOP));
    printf("RT_TARGET_RET %d\n", rtdisasm_find_target((const uint8_t*)test_1, size, RT_TARGET_RET));
    printf("RT_TARGET_RET_N %d\n", rtdisasm_find_target((const uint8_t*)test_1, size, RT_TARGET_RET_N));
    printf("RT_TARGET_INT3 %d\n", rtdisasm_find_target((const uint8_t*)test_1, size, RT_TARGET_INT3));
    printf("RT_TARGET_INT_N %d\n", rtdisasm_find_target((const uint8_t*)test_1, size, RT_TARGET_INT_N));
    printf("RT_TARGET_SYSENTER %d\n", rtdisasm_find_target((const uint8_t*)test_1, size, RT_TARGET_SYSENTER));
    printf("RT_TARGET_SYSCALL %d\n", rtdisasm_find_target((const uint8_t*)test_1, size, RT_TARGET_SYSCALL));
    
    return 0;
}