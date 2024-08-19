.text
.globl test_1
.globl test_1_end

test_1:
    push %rax
    push (%rbp)

    # rt targets
    nop
    
    ret
    ret $0x1234

    int3
    int $0x80
    sysenter
    syscall
test_1_end:
