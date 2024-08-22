#include "rtdisasm.h"
#include <immintrin.h>
#include <stdio.h>

extern void test_1();
extern void test_1_end();

// TEST 2 - CRC calculation function to put rtdisasm to test

static unsigned int test_2(unsigned char *message)
{
    int i, j;
    unsigned int byte, crc, mask;

    i = 0;
    crc = 0xFFFFFFFF;
    while (message[i] != 0)
    {
        byte = message[i];
        crc = crc ^ byte;
        for (j = 7; j >= 0; j--)
        {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
        i = i + 1;
    }
    __asm__("nop"); // TARGET
    return ~crc;
}
static void test_2_end() {}

// TEST 3 - VEX instructins
static void test_3()
{
    __m256 evens = _mm256_set_ps(2.0, 4.0, 6.0, 8.0, 10.0, 12.0, 14.0, 16.0);
    __m256 odds = _mm256_set_ps(1.0, 3.0, 5.0, 7.0, 9.0, 11.0, 13.0, 15.0);

    __m256 result = _mm256_sub_ps(evens, odds);
    (void)result;
    __asm__("nop"); // TARGET
}
static void test_3_end() {}

int main()
{
    printf("== TEST 1 ==\n");
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
    
    printf("\n== TEST 2 ==\n");
    size = (uintptr_t)test_2_end - (uintptr_t)test_2;
    printf("size %lu\n", size);

    printf("test2 %d\n", rtdisasm_find_target((const uint8_t*)test_2, size, RT_TARGET_NOP));

    printf("\n== TEST 3 ==\n");
    size = (uintptr_t)test_3_end - (uintptr_t)test_3;
    printf("size %lu\n", size);

    printf("test3 %d\n", rtdisasm_find_target((const uint8_t*)test_3, size, RT_TARGET_NOP));

    printf("\n== TEST 4 ==\n");
    printf("test1 patch %d\n", rtdisasm_estimate_patch((const uint8_t*)test_1, 16, 8));
    printf("test2 patch %d\n", rtdisasm_estimate_patch((const uint8_t*)test_2, 16, 8));
    printf("test3 patch %d\n", rtdisasm_estimate_patch((const uint8_t*)test_3, 16, 8));


    return 0;
}