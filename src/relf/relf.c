#include "relf/relf.h"
#include "relf/relf_debug.h"
#include "blackjack/debug.h"
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

// returns 1 if size not suitable for 32 bit host
static int check_32bit_limit(off_t st_size)
{
#if __x86_64__
    return 0;
#else
    // on 32bit hosts we need to check if 64bit file size
    // does not exceed 32bit size_t, which mmap/munmap uses
    return (st_size > SIZE_MAX);
#endif
}

static void relf_unmap(relf_t* relf)
{
    munmap(relf->image, relf->image_size);
    relf->image = NULL;
    relf->image_size = 0;
}

relf_value_t relf_open(relf_t* relf, const char* path)
{
    // reset struct
    memset(relf, '\0', sizeof(relf_t));

    // try stat file
    struct stat st = {0};
    if (stat(path, &st))
        return RELF_ERROR(RELF_FAILED_OPEN);
    if (check_32bit_limit(st.st_size))
        return RELF_ERROR(RELF_TOO_BIG);

    relf->image_size = (size_t)st.st_size;
    TRACE("image_size %lu\n", relf->image_size);
    // open file and read ELF header
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return RELF_ERROR(RELF_FAILED_OPEN);
    
    // read ELF's ident header, which contains magic and type
    uint8_t e_ident[EI_NIDENT];
    if (read(fd, e_ident, EI_NIDENT) < EI_NIDENT)
    {
        close(fd);
        return RELF_ERROR(RELF_FAILED_OPEN);
    }

    // check magic and decide ELF type
    // we operate here ELF64 variant since it same as in ELF32
    if (!memcmp(e_ident, ELFMAG, sizeof(ELFMAG)))
    {
        // not an ELF file at all
        close(fd);
        return RELF_ERROR(RELF_NOT_AN_ELF);
    }

    // 32 bit or 64 bit
    switch (e_ident[EI_CLASS])
    {
        case ELFCLASS32: relf->type = RELF_32BIT; break;
        case ELFCLASS64: relf->type = RELF_64BIT; break;
        default:
            close(fd);
            return RELF_ERROR(RELF_UNSUPPORTED);
    }

    if (e_ident[EI_DATA] != ELFDATA2LSB)
    {
        // not little endian, we can't work with that
        close(fd);
        return RELF_ERROR(RELF_UNSUPPORTED);
    }

    // we don't care about ABI, OS, machine type or ELF type,
    // as long as we got little endian we're good to go, so
    // let's map file to memory
    relf->image = mmap(NULL, relf->image_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd); // we can close file after mmap
    // but still check for errors
    if (relf->image == MAP_FAILED)
    {
        TRACE("mmap failed errno %d %s\n", errno, strerror(errno));
        return RELF_ERROR(RELF_MMAP_FAILED);
    }

    // now we need to parse segments and section headers

    // get segment and section numbers
    if (relf->type == RELF_64BIT)
    {
        Elf64_Ehdr* elf = (Elf64_Ehdr*)relf->image;
        relf->segment_num = elf->e_phnum;
        relf->section_num = elf->e_shnum;
    }
    else
    {
        Elf32_Ehdr* elf = (Elf32_Ehdr*)relf->image;
        relf->segment_num = elf->e_phnum;
        relf->section_num = elf->e_shnum;
    }
    TRACE("segment_num %u section_num %u\n", relf->segment_num, relf->section_num);

    if (relf->segment_num)
        relf->segments = (relf_segment_t*)calloc(relf->segment_num, sizeof(relf_segment_t));
    if (relf->section_num)
        relf->sections = (relf_section_t*)calloc(relf->section_num, sizeof(relf_section_t));
    
    // load segment info
    if (relf->type == RELF_64BIT)
    {
        Elf64_Ehdr* elf = (Elf64_Ehdr*)relf->image;
        for (unsigned i = 0; i < relf->segment_num; i++)
        {
            const Elf64_Phdr* hdr = (const Elf64_Phdr*)
                ((uint8_t*)relf->image + elf->e_phoff + elf->e_phentsize * i);
            relf_segment_t* segment = &relf->segments[i];

            segment->type = hdr->p_type;
            segment->flags = hdr->p_flags;
            
            segment->f_offset = hdr->p_offset;
            segment->f_size = hdr->p_filesz;

            segment->v_addr = hdr->p_vaddr;
            segment->v_size = hdr->p_memsz;

            TRACE_SEGMENT(segment);
        }
    }
    else
    {
        Elf32_Ehdr* elf = (Elf32_Ehdr*)relf->image;
        for (unsigned i = 0; i < relf->segment_num; i++)
        {
            const Elf32_Phdr* hdr = (const Elf32_Phdr*)
                ((uint8_t*)relf->image + elf->e_phoff + elf->e_phentsize * i);
            relf_segment_t* segment = &relf->segments[i];

            segment->type = hdr->p_type;
            segment->flags = hdr->p_flags;
            
            segment->f_offset = hdr->p_offset;
            segment->f_size = hdr->p_filesz;

            segment->v_addr = hdr->p_vaddr;
            segment->v_size = hdr->p_memsz;

            TRACE_SEGMENT(segment);
        }
    }

    // load section info
    if (relf->type == RELF_64BIT)
    {
        Elf64_Ehdr* elf = (Elf64_Ehdr*)relf->image;
        for (unsigned i = 0; i < relf->section_num; i++)
        {
            const Elf64_Shdr* hdr = (const Elf64_Shdr*)
                ((uint8_t*)relf->image + elf->e_shoff + elf->e_shentsize * i);
            relf_section_t* section = &relf->sections[i];

            section->type = hdr->sh_type;
            section->flags = hdr->sh_flags;

            // we will resolve names when string table is resolved
            section->name = NULL;

            section->f_offset = hdr->sh_offset;
            section->f_size = hdr->sh_size;

            section->v_addr = hdr->sh_addr;

            section->link = hdr->sh_link;
            section->info = hdr->sh_info;

            section->entsize = hdr->sh_entsize;

            TRACE_SECTION(section);
        }
    }
    else
    {
        Elf32_Ehdr* elf = (Elf32_Ehdr*)relf->image;
        for (unsigned i = 0; i < relf->section_num; i++)
        {
            const Elf32_Shdr* hdr = (const Elf32_Shdr*)
                ((uint8_t*)relf->image + elf->e_shoff + elf->e_shentsize * i);
            relf_section_t* section = &relf->sections[i];

            section->type = hdr->sh_type;
            section->flags = hdr->sh_flags;

            // we will resolve names when string table is resolved
            section->name = NULL;

            section->f_offset = hdr->sh_offset;
            section->f_size = hdr->sh_size;

            section->v_addr = hdr->sh_addr;

            section->link = hdr->sh_link;
            section->info = hdr->sh_info;

            section->entsize = hdr->sh_entsize;

            TRACE_SECTION(section);
        }
    }

    // load string table
    unsigned e_shstrndx = SHN_UNDEF;
    if (relf->type == RELF_64BIT)
        e_shstrndx = ((const Elf64_Ehdr*)relf->image)->e_shstrndx;
    else
        e_shstrndx = ((const Elf32_Ehdr*)relf->image)->e_shstrndx;
    
    // NOTE: I should handle SHN_LORESERVE in e_phnum, e_shnum and e_shstrndx
    // but that's non-existent use case when ELF has number of segments
    // and sections that are exceeding uint16_t limit, so I don't care.
    if (e_shstrndx != SHN_UNDEF)
    {
        // we have string table, so to ease parsing
        // we gonna allocate array of char pointers
        // but first we need enumerate and find out
        // how many strings there are
        const char* cur = (const char*)
            relf->image + relf->sections[e_shstrndx].f_offset;
        const char* end = cur + relf->sections[e_shstrndx].f_size;
        while (cur < end)
        {
            TRACE("str %s\n", cur);
            cur += strlen(cur) + 1;
            relf->string_num++;
        }

        TRACE("string_num %u\n", relf->string_num);
    }
    
    return RELF_ERROR(RELF_OK);
}
