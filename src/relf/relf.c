#include "relf/relf.h"
#include "blackjack/debug.h"
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

relf_value_t relf_open(relf_t* relf, const char* path)
{
    // reset struct
    memset(relf, '\0', sizeof(relf_t));

    // try stat file
    struct stat st = {0};
    if (stat(path, &st))
        return RELF_ERROR(RELF_FAILED_OPEN);
    
    TRACE("st_size %lu\n", st.st_size);
    // open file and read ELF header
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return RELF_ERROR(RELF_FAILED_OPEN);
    
    union {
        Elf64_Ehdr hdr64;
        Elf32_Ehdr hdr32;
    } e;

    // read biggest value by default
    if (read(fd, &e.hdr64, sizeof(e.hdr64)) < sizeof(e.hdr64))
    {
        close(fd);
        return RELF_ERROR(RELF_FAILED_OPEN);
    }

    // check magic and decide ELF type
    // we operate here ELF64 variant since it same as in ELF32
    if (!memcmp(e.hdr64.e_ident, ELFMAG, sizeof(ELFMAG)))
    {
        // not an ELF file at all
        close(fd);
        return RELF_ERROR(RELF_NOT_AN_ELF);
    }

    // 32 bit or 64 bit
    switch (e.hdr64.e_ident[EI_CLASS])
    {
        case ELFCLASS32: relf->type = RELF_32BIT; break;
        case ELFCLASS64: relf->type = RELF_64BIT; break;
        default:
            close(fd);
            return RELF_ERROR(RELF_UNSUPPORTED);
    }

    if (e.hdr64.e_ident[EI_DATA] != ELFDATA2LSB)
    {
        // not little endian, we can't work with that
        close(fd);
        return RELF_ERROR(RELF_UNSUPPORTED);
    }

    // we don't care about ABI, OS, machine type or ELF type,
    // as long as we got little endian we're good to go, so
    // let's map file to memory
    relf->image = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd); // we can close file after mmap
    // but still check for errors
    if (relf->image == MAP_FAILED)
    {
        TRACE("mmap failed errno %d %s\n", errno, strerror(errno));
        return RELF_ERROR(RELF_MMAP_FAILED);
    }
    
    return RELF_ERROR(RELF_OK);
}
