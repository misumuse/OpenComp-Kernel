/* start.S - minimal 64-bit entry with multiboot2 header */

/* Multiboot2 header */
.section .multiboot
.align 8
multiboot2_header_start:
    .long 0xE85250D6
    .long 0
    .long multiboot2_header_end - multiboot2_header_start
    .long -(0xE85250D6 + 0 + (multiboot2_header_end - multiboot2_header_start))
    
    .align 8
    .short 0
    .short 0
    .long 8
multiboot2_header_end:

/* Entry point */
.section .text
.global _start

_start:
    cli
    mov $0x90000, %rsp
    cld
    call kernel_main
    
1:  
    hlt
    jmp 1b
