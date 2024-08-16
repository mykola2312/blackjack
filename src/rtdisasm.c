#include "rtdisasm.h"
#include "rtdisasm_table.h"
#include <string.h>

// prefix definitions. must be declared with macro in order
// to be readable later in prefix table

#define PREFIX_LOCK             0xF0
#define PREFIX_REPNZ            0xF2 // also BND prefix
#define PREFIX_REPZ             0xF3

#define PREFIX_CS_OVERRIDE      0x2E // also branch-not-taken hint
#define PREFIX_SS_OVERRIDE      0x36
#define PREFIX_DS_OVERRIDE      0x3E // also branch-taken hint
#define PREFIX_ES_OVERRIDE      0x26
#define PREFIX_FS_OVERRIDE      0x64
#define PREFIX_GS_OVERRIDE      0x65

#define PREFIX_OPERAND_OVERRIDE 0x66
#define PREFIX_ADDRESS_OVERRIDE 0x67

static const uint8_t std_prefixes[] = {
    PREFIX_LOCK,
    PREFIX_REPNZ,
    PREFIX_REPZ,

    PREFIX_CS_OVERRIDE,
    PREFIX_SS_OVERRIDE,
    PREFIX_DS_OVERRIDE,
    PREFIX_ES_OVERRIDE,
    PREFIX_FS_OVERRIDE,
    PREFIX_GS_OVERRIDE,

    PREFIX_OPERAND_OVERRIDE,
    PREFIX_ADDRESS_OVERRIDE
};

static const unsigned std_prefixes_len = sizeof(std_prefixes);

static int is_std_prefix(const uint8_t prefix)
{
    for (unsigned i = 0; i < std_prefixes_len; i++)
        if (prefix == std_prefixes[i]) return 1;
    
    return 0;
}

#define VEX_2BYTE       0xC5
#define VEX_3BYTE       0xC4

static int test_vex_prefix(const uint8_t vex_first)
{
    if (vex_first == VEX_2BYTE) return 2;
    else if (vex_first == VEX_3BYTE) return 3;
    else return 0;
}

#define REX_SIG         0b01000000
#define REX_MASK        0b11110000
#define REX_VALUE_MASK  0b00001111
#define REX_B_VALUE     (1<<0)
#define REX_X_VALUE     (1<<1)
#define REX_R_VALUE     (1<<2)
#define REX_W_VALUE     (1<<3)

// returns -1 if not rex, and non-negative is REX_* define
static int test_rex_prefix(const uint8_t rex)
{
    if (rex & REX_MASK != REX_SIG) return -1;
    
    const uint8_t rex_value = rex & REX_VALUE_MASK;
    switch (rex_value)
    {
        case REX_B_VALUE: return REX_B;
        case REX_X_VALUE: return REX_X;
        case REX_R_VALUE: return REX_R;
        case REX_W_VALUE: return REX_W;
    }

    return -1;
}

const instruction_t* find_instruction(const uint8_t* cur, unsigned type, int vex, int rex)
{
    for (unsigned i = 0; i < rtdisasm_table_len; i++)
    {
        const instruction_t* ins = &rtdisasm_table[i];

        if (ins->config.type != type) continue;
        // check rex if instruction does rex, and if provided rex is not -1
        if (rex != -1   && type == INSTRUCTION_STD 
                        && ins->config.has_rex && ins->std.rex != rex)
        {
            // rex doesn't match, skip instruction
            continue;
        }

        // compare opcodes
        if (memcmp(cur, &ins->opcode, ins->opcode_len))
        {
            // opcodes don't match up, skip
            continue;
        }

        // for now, everything looks good, so that's our instruction
        return ins;
    }

    return NULL;
}

int rtdisasm_analyze_single(const uint8_t* code, uint8_t size)
{
    const uint8_t* cur = code;
    const uint8_t* const end = code + size;
    if (cur == end) return 0;

    // skip standard prefixes
    while (is_std_prefix(*cur))
    {
        if (++cur == end) return -1;
    }

    unsigned type = INSTRUCTION_STD;

    // first, we need to test vex prefix, because only then comes the rex
    int vex = test_vex_prefix(*cur);
    if (vex)
    {
        // it's vex, lets advance 2 or 3 bytes
        cur += vex;
        if (cur >= end) return -1;
        type = INSTRUCTION_VEX;
    }

    // test if its rex prefix, if so we will look specifically for
    // instructions with rex prefix
    int rex = test_rex_prefix(*cur);
    if (rex != -1)
    {
        // it's rex, so advance 1 byte
        if (++cur >= end) return -1;
    }

    const instruction_t* ins = find_instruction(cur, type, vex, rex);
}
