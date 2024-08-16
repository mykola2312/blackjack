.section .rodata
.globl test_1
.globl test_1_end

test_1:
    lock
    mov %gs:(%rbx), %rax

    nop # target that rtdisasm must reach
test_1_end: