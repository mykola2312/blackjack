#ifndef __RELF_DEBUG_H
#define __RELF_DEBUG_H

#include "blackjack/debug.h"

#ifdef DEBUG
#define TRACE_SEGMENT(segment)                                                              \
    TRACE("segment type %u flags %u f_offset 0x%lx f_size %lu v_addr 0x%lx v_size %lu\n",   \
        segment->type, segment->flags,                                                      \
        segment->f_offset, segment->f_size,                                                 \
        segment->v_addr, segment->v_size                                                    \
    )

#define TRACE_SECTION(section)                                                                                      \
    TRACE("section type %x flags %x name %s f_offset 0x%lx f_size %lu v_addr 0x%lx link %x info %x entsize %lu\n",  \
        section->type, section->flags,                                                                              \
        section->name,                                                                                              \
        section->f_offset, section->f_size,                                                                         \
        section->v_addr,                                                                                            \
        section->link, section->info,                                                                               \
        section->entsize                                                                                            \
    )

#define TRACE_SYMBOL(symbol)                                                                                        \
    TRACE("symbol name %s value 0x%lx size %lu info %x other %x\n",                                                 \
        symbol->name,                                                                                               \
        symbol->value, symbol->size,                                                                                \
        symbol->info, symbol->other                                                                                 \
    )
#else
#define TRACE_SEGMENT(segment)
#define TRACE_SECTION(section)
#define TRACE_SYMBOL(symbol)
#endif

#endif