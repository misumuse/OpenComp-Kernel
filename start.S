/* start.S - multiboot2 entry for 32-bit mode */

/* Multiboot2 header */
.section .multiboot
.align 8
multiboot2_header_start:
    .long 0xE85250D6                                    /* magic */
    .long 0                                             /* architecture (i386) */
    .long multiboot2_header_end - multiboot2_header_start  /* header length */
    .long -(0xE85250D6 + (multiboot2_header_end - multiboot2_header_start))  /* checksum */
    
    /* end tag */
    .align 8
    .short 0    /* type */
    .short 0    /* flags */
    .long 8     /* size */
multiboot2_header_end:

/* Start in 32-bit mode */
.section .text
.global _start
.type _start, @function

_start:
    cli
    movl $stack_top, %esp
    cld
    call kernel_main
    
.hang:
    cli
    hlt
    jmp .hang

/* Stack */
.section .bss
.align 16
stack_bottom:
    .skip 16384
stack_top:
