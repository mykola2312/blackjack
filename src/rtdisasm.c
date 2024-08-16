#include "rtdisasm.h"

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