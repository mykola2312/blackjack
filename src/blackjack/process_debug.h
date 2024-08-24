#ifndef __PROCESS_DEBUG_H
#define __PROCESS_DEBUG_H

#ifdef DEBUG
#include <stdio.h>
#include <sys/user.h>
static void print_registers(const struct user_regs_struct* regs)
{
    fprintf(stderr,
                    "r15\t%p\n"
                    "r14\t%p\n"
                    "r13\t%p\n"
                    "r12\t%p\n"
                    "rbp\t%p\n"
                    "rbx\t%p\n"
                    "r11\t%p\n"
                    "r10\t%p\n"
                    "r9\t%p\n"
                    "r8\t%p\n"
                    "rax\t%p\n"
                    "rcx\t%p\n"
                    "rdx\t%p\n"
                    "rsi\t%p\n"
                    "rdi\t%p\n"
                    "orig_rax\t%p\n"
                    "rip\t%p\n"
                    "cs\t%p\n"
                    "eflags\t%p\n"
                    "rsp\t%p\n"
                    "ss\t%p\n"
                    "fs_base\t%p\n"
                    "gs_base\t%p\n"
                    "ds\t%p\n"
                    "es\t%p\n"
                    "fs\t%p\n"
                    "gs\t%p\n",
                    (void*)regs->r15,
                    (void*)regs->r14,
                    (void*)regs->r13,
                    (void*)regs->r12,
                    (void*)regs->rbp,
                    (void*)regs->rbx,
                    (void*)regs->r11,
                    (void*)regs->r10,
                    (void*)regs->r9,
                    (void*)regs->r8,
                    (void*)regs->rax,
                    (void*)regs->rcx,
                    (void*)regs->rdx,
                    (void*)regs->rsi,
                    (void*)regs->rdi,
                    (void*)regs->orig_rax,
                    (void*)regs->rip,
                    (void*)regs->cs,
                    (void*)regs->eflags,
                    (void*)regs->rsp,
                    (void*)regs->ss,
                    (void*)regs->fs_base,
                    (void*)regs->gs_base,
                    (void*)regs->ds,
                    (void*)regs->es,
                    (void*)regs->fs,
                    (void*)regs->gs
    );
}
#else
#define print_registers(regs)
#endif

#endif