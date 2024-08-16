.section .rodata
.globl test_1

test_1:
    lock
    mov %gs:(%rbx), %rax

    nop # target that rtdisasm must reach
