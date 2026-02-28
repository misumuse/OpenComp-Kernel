/* start64.S - experimental x86_64 multiboot2 entry */

.section .multiboot
.align 8
multiboot2_header_start64:
    .long 0xE85250D6
    .long 0
    .long 24
    .long -(0xE85250D6 + 0 + 24)

    .align 8
    .short 0
    .short 0
    .long 8

.section .text
.global _start
.type _start, @function

_start:
    cli
    movq $stack_top64, %rsp
    call kernel64_main

.hang:
    hlt
    jmp .hang

.section .bss
.align 16
stack_bottom64:
    .skip 32768
stack_top64:
