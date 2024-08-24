#ifndef __RTDISASM_TABLE_H
#define __RTDISASM_TABLE_H

#include <stdint.h>

#define INSTRUCTION_STD         0
#define INSTRUCTION_VEX         1
#define INSTRUCTION_EVEX        2
// so there is special group for instructions like endbr64
// that are just dumb memcmp matches
#define INSTRUCTION_CUSTOM      3

#define REX_B                   0
#define REX_X                   1
#define REX_R                   2
#define REX_W                   3

#define IMM_B                   0
#define IMM_W                   1
#define IMM_D                   2
#define IMM_O                   3

#define VALUE_B                 0
#define VALUE_W                 1
#define VALUE_D                 2
#define VALUE_P                 3
#define VALUE_O                 4
#define VALUE_T                 5

// these are target instructions that rtdisasm will look for
#define RT_TARGET_NO_MEANING    0
#define RT_TARGET_NOP           1   // 90
#define RT_TARGET_RET           2   // C3
#define RT_TARGET_RET_N         3   // C2 iw
#define RT_TARGET_INT3          4   // CC
#define RT_TARGET_INT_N         5   // CD ib
#define RT_TARGET_SYSENTER      6   // 0F 34
#define RT_TARGET_SYSCALL       7   // 0F 05
#define RT_TARGET_ENDBR32       8   // F3 0F 1E FB
#define RT_TARGET_ENDBR64       9   // F3 0F 1E FA

#define MAX_OPCODE_LEN          4

typedef struct {
    struct {
        uint16_t type           : 4;
        uint16_t has_rex        : 1;
        uint16_t has_digit      : 1;
        uint16_t has_modrm      : 1;
        uint16_t has_imm        : 1;
        uint16_t has_value      : 1;
        uint16_t has_opreg      : 1;
    } config;

    union {
        struct {
            uint16_t rex        : 2;
            uint16_t digit      : 3;
            uint16_t imm        : 3;
            uint16_t value      : 3;
        } std;

        struct {
            uint16_t lig        : 1;
            uint16_t l          : 9;
            uint16_t wig        : 1;
            uint16_t w          : 1;
            uint16_t imm        : 3;
        } vex;

        struct {
            uint16_t lig        : 1;
            uint16_t l          : 10;
            uint16_t wig        : 1;
            uint16_t w          : 1;
            uint16_t imm        : 3;
        } evex;
    };

    uint16_t rt_target;
    
    uint16_t opcode_len;
    uint8_t opcode[MAX_OPCODE_LEN];
} instruction_t;

extern const instruction_t rtdisasm_table[];
extern const unsigned rtdisasm_table_len;

#endif