    .global hijack_destination

    .text
hijack_destination:
    leaq redirect_msg(%rip), %rdi
    call puts
.sleep_loop:
    mov $0x1, %edi
    call sleep
    jmp .sleep_loop

    .section rodata
redirect_msg:   .string "thread has been redirected to this function! cool!"
