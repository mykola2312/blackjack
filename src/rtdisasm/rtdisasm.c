#include "rtdisasm/rtdisasm.h"
#include "rtdisasm/rtdisasm_table.h"
#include "blackjack/debug.h"
#include <stdio.h>
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
    if ((rex & REX_MASK) != REX_SIG) return -1;
    
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

// so we can ignore register encoded in opcode
#define OPREG_MASK      0b11111000

static const instruction_t* find_instruction(const uint8_t* cur, unsigned type, int vex, int rex)
{
    for (unsigned i = 0; i < rtdisasm_table_len; i++)
    {
        const instruction_t* ins = &rtdisasm_table[i];
        // always skip special instructions because
        // they require seoarate parsing
        if (ins->config.type == INSTRUCTION_CUSTOM)
            continue;

        if (ins->config.type != type) continue;
        // check rex if instruction does rex, and if provided rex is not -1
        if (rex != -1   && type == INSTRUCTION_STD 
                        && ins->config.has_rex && ins->std.rex != rex)
        {
            // rex doesn't match, skip instruction
            continue;
        }

        if (ins->config.has_opreg)
        {
            // instruction encoding employs register embedded into last opcode byte
            // so we need to apply bit mask
            
            // plain means opcode bytes that are not affected
            // by opcode register encoding
            uint16_t plain_len = ins->opcode_len - 1;
            if (plain_len)
            {
                if (memcmp(cur, ins->opcode, plain_len))
                    continue;
            }

            // now let's match the opreg encoded byte
            if ((cur[plain_len] & OPREG_MASK) != ins->opcode[plain_len])
                continue;
        }
        else
        {
            // just compare opcodes
            if (memcmp(cur, ins->opcode, ins->opcode_len))
            {
                // opcodes don't match up, skip
                continue;
            }
        }

        // for now, everything looks good, so that's our instruction
        return ins;
    }

    return NULL;
}

typedef struct {
    uint8_t mod;
    uint8_t rm;
    uint8_t has_sib;
    uint8_t disp_len;
} modrm_encoding_t;

static const modrm_encoding_t modrm_encodings[] = {
    { .mod = 0b00, .rm = 0b100, .has_sib = 1, .disp_len = 0 },
    { .mod = 0b00, .rm = 0b101, .has_sib = 0, .disp_len = 4 },

    { .mod = 0b01, .rm = 0b000, .has_sib = 0, .disp_len = 1 },
    { .mod = 0b01, .rm = 0b001, .has_sib = 0, .disp_len = 1 },
    { .mod = 0b01, .rm = 0b010, .has_sib = 0, .disp_len = 1 },
    { .mod = 0b01, .rm = 0b011, .has_sib = 0, .disp_len = 1 },
    { .mod = 0b01, .rm = 0b100, .has_sib = 1, .disp_len = 1 },
    { .mod = 0b01, .rm = 0b101, .has_sib = 0, .disp_len = 1 },
    { .mod = 0b01, .rm = 0b110, .has_sib = 0, .disp_len = 1 },
    { .mod = 0b01, .rm = 0b111, .has_sib = 0, .disp_len = 1 },

    { .mod = 0b10, .rm = 0b000, .has_sib = 0, .disp_len = 4 },
    { .mod = 0b10, .rm = 0b001, .has_sib = 0, .disp_len = 4 },
    { .mod = 0b10, .rm = 0b010, .has_sib = 0, .disp_len = 4 },
    { .mod = 0b10, .rm = 0b011, .has_sib = 0, .disp_len = 4 },
    { .mod = 0b10, .rm = 0b100, .has_sib = 1, .disp_len = 4 },
    { .mod = 0b10, .rm = 0b101, .has_sib = 0, .disp_len = 4 },
    { .mod = 0b10, .rm = 0b110, .has_sib = 0, .disp_len = 4 },
    { .mod = 0b10, .rm = 0b111, .has_sib = 0, .disp_len = 4 },
};
static const unsigned modrm_encodings_len = sizeof(modrm_encodings) / sizeof(modrm_encoding_t);

// analyze ModRM and determine if it employs SIB byte,
// as well as any displacements
static void analyze_modrm(const uint8_t modrm, uint8_t* has_sib, uint8_t* disp_len)
{
    const uint8_t mod = modrm >> 6;
    const uint8_t rm = modrm & 0b111;

    // default values
    *has_sib = 0;
    *disp_len = 0;

    // now lets look up in table and if matches
    // set proper values
    for (unsigned i = 0; i < modrm_encodings_len; i++)
    {
        const modrm_encoding_t* encoding = &modrm_encodings[i];
        if (encoding->mod == mod && encoding->rm == rm)
        {
            *has_sib = encoding->has_sib;
            *disp_len = encoding->disp_len;
        } 
    }
}

static unsigned imm2length(uint8_t imm)
{
    switch (imm)
    {
        case IMM_B: return 1;
        case IMM_W: return 2;
        case IMM_D: return 4;
        case IMM_O: return 8;
        
        default: return 0;
    }
}

static unsigned value2length(uint8_t value)
{
    switch (value)
    {
        case VALUE_B: return 1;
        case VALUE_W: return 2;
        case VALUE_D: return 4;
        case VALUE_P: return 6;
        case VALUE_O: return 8;
        case VALUE_T: return 10;
        
        default: return 0;
    }
}

#ifdef DEBUG
static void print_instruction(const instruction_t* ins)
{
    TRACE("type %u has_rex %u has_digit %u has_modrm %u has_imm %u has_value %u has_opreg %u",
        ins->config.type,
        ins->config.has_rex,
        ins->config.has_digit,
        ins->config.has_modrm,
        ins->config.has_imm,
        ins->config.has_value,
        ins->config.has_opreg
    );

    fprintf(stderr, " opcodes ");
    for (unsigned i = 0; i < ins->opcode_len; i++)
        fprintf(stderr, "%02X ", ins->opcode[i]);
    
    fprintf(stderr, "\n");
}
#else
#define print_instruction(ins)
#endif

// we don't do here size limits because special instruction
// opcode byte sequences are unique, and if they cross boundaris
// the moment it returns back to analyze_single there will be
// size limit checks anyway.
const instruction_t* is_special_instruction(const uint8_t* code)
{
    for (unsigned i = 0; i < rtdisasm_table_len; i++)
    {
        const instruction_t* ins = &rtdisasm_table[i];
        if (ins->config.type != INSTRUCTION_CUSTOM)
            continue;
        
        // just dumb memcmp match
        if (!memcmp(code, ins->opcode, ins->opcode_len))
            return ins;
    }

    return NULL;
}

int rtdisasm_analyze_single(const uint8_t* code, unsigned limit, const instruction_t** found)
{
    const uint8_t* cur = code;
    const uint8_t* const end = code + limit;
    if (cur == end) return -1;

    // look for any special instructions first
    {
        const instruction_t* special = is_special_instruction(code);
        if (special)
        {
            TRACE("found special instruction\n");
            print_instruction(special);

            if (found) *found = special;
            return special->opcode_len;
        }
    }

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
    if (!ins) return 0; // no instruction

    print_instruction(ins);
    TRACE("type %d rex %d vex %d\n", type, rex, vex);

    // since we found instruction, we need advance past opcode bytes
    cur += ins->opcode_len;
    // don't check here for size limit, sicne size could be 1
    // and opcode length 1 byte also

    // if instruction has ModRM, we need to analyze it,
    // since it can lead to SIB byte
    if (ins->config.has_modrm)
    {
        // consume ModRM byte
        uint8_t modrm = *cur++;
        if (cur >= end) return -1;

        uint8_t has_sib, disp_len;
        analyze_modrm(modrm, &has_sib, &disp_len);
        TRACE("modrm %02X has_sib %u disp_len %u\n", modrm, has_sib, disp_len);

        if (has_sib)
        {
            // consume SIB byte
            if (++cur >= end) return -1;
        }

        // add displacement
        cur += disp_len;
        if (cur >= end) return -1;
    }

    // now we need to skip the immediate values
    if (type == INSTRUCTION_STD)
    {
        if (ins->config.has_imm)
        {
            cur += imm2length(ins->std.imm);
            if (cur >= end) return -1;
        }
        else if (ins->config.has_value)
        {
            cur += value2length(ins->std.value);
            if (cur >= end) return -1;
        }
    }

    // set found
    if (found) *found = ins;
    // return length of entire decoded instruction
    return (int)((uintptr_t)cur-(uintptr_t)code);
}

int rtdisasm_find_target(const uint8_t* code, unsigned limit, unsigned rt_target)
{
    const uint8_t* cur = code;
    const uint8_t* const end = code + limit;
    unsigned remaining = limit;
    if (cur == end) return -1;

    do {
        const instruction_t* ins;
        int len = rtdisasm_analyze_single(cur, remaining, &ins);
        // NOTE: this is ret passthru from analyze single,
        // so it must be follow same ret logic as this function
        TRACE("rtdisasm_analyze_single len %d\n", len);
        if (len < 1) return len;

        TRACE("ins->rt_target %u rt_target %u\n", ins->rt_target, rt_target);
        if (ins->rt_target == rt_target)
        {
            // we found target instruction!
            return (int)((uintptr_t)cur-(uintptr_t)code);
        }

        // otherwise, advance further
        cur += len;
        remaining -= len;
        if (cur >= end) return -1;
    } while (1);
}

int rtdisasm_estimate_patch(const uint8_t* code, unsigned limit, int wanted)
{
    const uint8_t* cur = code;
    const uint8_t* const end = code + limit;
    unsigned remaining = limit;
    unsigned patch = 0;
    if (cur == end) return -1;

    do {
        const instruction_t* ins;
        int len = rtdisasm_analyze_single(cur, remaining, &ins);
        TRACE("rtdisasm_analyze_single len %d\n", len);
        if (len < 1) return len;

        // advance past instruction and check if we hit limit
        // cuz we don't want patch on the limit
        cur += len;
        remaining -= len;
        if (cur >= end) return -1;

        // we're good, let's go until we have enough bytes for patch
        patch += len;
    } while(patch < wanted);

    // at this point it should contain patch size
    // aligned with instruction boundaries
    return patch;
}
