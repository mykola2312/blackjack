#ifndef __RELF_H
#define __RELF_H

#include <stdint.h>
#include <stddef.h>

// composite error type
typedef enum {
    RELF_MMAP_FAILED    = -5,   // file memory mapping failed
    RELF_UNSUPPORTED    = -4,   // big endian or not x86/x86-64 architecture
    RELF_NOT_AN_ELF     = -3,   // wrong magic
    RELF_TOO_BIG        = -2,   // file is over size_t limit
    RELF_FAILED_OPEN    = -1,   // failed to stat or open file
    RELF_OK             = 0,
} relf_error_t;

typedef union {
    relf_error_t error;

    // yeah, we're going to always use 64bit value so
    // we won't get any undefined behavior regardless
    // host and target architectures
    uint64_t value;
} relf_value_t;

// supply relf_value_t type here
#define RELF_IS_ERROR(v)    (v.error < 0)
#define RELF_ERROR(e)       ((relf_value_t) {.error = e})

typedef enum {
    RELF_32BIT,
    RELF_64BIT
} relf_type_t;

// we're using our own structures so parsing
// logic wouldn't be cluttered with 32 and 64 bit branching

// prefix convention
// f_   - file positioning aka file offsets
// v_   - virtual memory in runtime

// ELF program segment
typedef struct {
    uint32_t type;
    uint32_t flags;

    // file offset and size
    uint64_t f_offset;
    uint64_t f_size;

    // address and size in virtual memory in runtime
    uint64_t v_addr;
    uint64_t v_size;
} relf_segment_t;

// ELF section
typedef struct {
    uint32_t type;
    uint32_t flags;

    union {
        uint32_t si_name; // index in string table
        const char* name;
    };

    uint64_t f_offset;
    uint64_t f_size;

    uint64_t v_addr;

    uint32_t link;
    uint32_t info;

    // for symbol table will tell size of symbol entry
    uint64_t entsize;
} relf_section_t;

// ELF symbol
typedef struct {
    const char* name;
    
    uint64_t value;
    uint64_t size;

    unsigned info;
    unsigned other;

    const relf_section_t* section;
} relf_symbol_t;

// relf instance
typedef struct {
    void* image;
    size_t image_size;

    // is it 64 or 32 bit mode
    relf_type_t type;

    relf_segment_t* segments;
    unsigned segment_num;

    relf_section_t* sections;
    unsigned section_num;

    relf_symbol_t* symbols;
    unsigned symbol_num;
} relf_t;

// opens ELF file, checks ELF magic and maps it into memory
// may load additional info like string table
relf_value_t relf_open(relf_t* relf, const char* path);

// closes mapping and file, frees any allocated memory in relf instance
void relf_close(relf_t* relf);

#endif