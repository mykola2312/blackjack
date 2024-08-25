#include "relf/relf.h"
#include "blackjack/debug.h"
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

relf_value_t relf_open(relf_t* relf, const char* path)
{
    // reset struct
    memset(relf, '\0', sizeof(relf_t));

    // try stat file
    struct stat st = {0};
    if (stat(path, &stat))
        return RELF_ERROR(RELF_FAILED_OPEN);
    
    // open file and read ELF header
    relf->fd = open(path, O_RDONLY);
    if (relf->fd < 0)
        return RELF_ERROR(RELF_FAILED_OPEN);
    
    union {
        Elf64_Ehdr hdr64;
        Elf32_Ehdr hdr32;
    } e;

    // read biggest value by default
    if (read(relf->fd, &e.hdr64, sizeof(e.hdr64)) < sizeof(e.hdr64))
    {
        close(relf->fd);
        return RELF_ERROR(RELF_FAILED_OPEN);
    }

    // check magic and decide ELF type
    // we operate here ELF64 variant since it same as in ELF32
    if (!memcmp(e.hdr64.e_ident, ELFMAG, sizeof(ELFMAG)))
    {
        // not an ELF file at all
        close(relf->fd);
        return RELF_ERROR(RELF_NOT_AN_ELF);
    }

    // 32 bit or 64 bit
    switch (e.hdr64.e_ident[EI_CLASS])
    {
        case ELFCLASS32: relf->type = RELF_32BIT; break;
        case ELFCLASS64: relf->type = RELF_64BIT; break;
        default:
            close(relf->fd);
            return RELF_ERROR(RELF_UNSUPPORTED);
    }

    if (e.hdr64.e_ident[EI_DATA] != ELFDATA2LSB)
    {
        // not little endian, we can't work with that
        close(relf->fd);
        return RELF_ERROR(RELF_UNSUPPORTED);
    }
}
