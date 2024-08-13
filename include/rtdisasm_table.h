#ifndef __RTDISASM_TABLE_H
#define __RTDISASM_TABLE_H

#include <stdint.h>

#define INSTRUCTION_STANDARD    0
#define INSTRUCTION_VEX         1
#define INSTRUCTION_EVEX        2

#define MAX_OPCODE_LEN          4

typedef struct {
    struct {
        uint16_t type       : 4;
        uint16_t has_rex    : 1;
        uint16_t has_digit  : 1;
        uint16_t has_modrm  : 1;
        uint16_t has_imm    : 1;
        uint16_t has_value  : 1;
        uint16_t has_opreg  : 1;
    } info;
    
    uint16_t opcode_len;
    uint8_t opcode[MAX_OPCODE_LEN];
} instruction_t;

extern const instruction_t rtdisasm_table[];
extern const unsigned rtdisasm_table_len;

#endif