#ifndef __RTDISASM_TABLE_H
#define __RTDISASM_TABLE_H

#include <stdint.h>

#define INSTRUCTION_STD         0
#define INSTRUCTION_VEX         1
#define INSTRUCTION_EVEX        2

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
    
    uint16_t opcode_len;
    uint8_t opcode[MAX_OPCODE_LEN];
} instruction_t;

extern const instruction_t rtdisasm_table[];
extern const unsigned rtdisasm_table_len;

#endif