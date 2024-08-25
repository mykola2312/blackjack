#ifndef __RELF_H
#define __RELF_H

#include <stdint.h>

// composite error type
typedef enum {
    RELF_MMAP_FAILED    = -4,   // file memory mapping failed
    RELF_UNSUPPORTED    = -3,   // big endian or not x86/x86-64 architecture
    RELF_NOT_AN_ELF     = -2,   // wrong magic
    RELF_FAILED_OPEN    = -1,   // failed to stat or open file
    RELF_OK             = 0,
} relf_error_t;

typedef union {
    relf_error_t error;

    uintptr_t value;
} relf_value_t;

// supply relf_value_t type here
#define RELF_IS_ERROR(v)    (v.error < 0)
#define RELF_ERROR(e)       ((relf_value_t) {.error = e})

typedef enum {
    RELF_64BIT,
    RELF_32BIT
} relf_type_t;

// relf instance
typedef struct {
    int fd;
    void* image;

    // is it 64 or 32 bit mode
    relf_type_t type;
} relf_t;

// opens ELF file, checks ELF magic and maps it into memory
// may load additional info like string table
relf_value_t relf_open(relf_t* relf, const char* path);

// closes mapping and file, frees any allocated memory in relf instance
void relf_close(relf_t* relf);

#endif