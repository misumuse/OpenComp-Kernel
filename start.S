/* start.S - multiboot2 entry with 32-bit to 64-bit transition */

/* Multiboot2 header */
.section .multiboot
.align 8
multiboot2_header_start:
    .long 0xE85250D6                /* magic number */
    .long 0                         /* architecture: 0 = i386 */
    .long multiboot2_header_end - multiboot2_header_start
    .long -(0xE85250D6 + 0 + (multiboot2_header_end - multiboot2_header_start))
    
    /* end tag */
    .align 8
    .short 0
    .short 0
    .long 8
multiboot2_header_end:

/* Start in 32-bit mode (multiboot requirement) */
.code32
.section .text
.global _start
.type _start, @function

_start:
    /* Disable interrupts */
    cli
    
    /* Set up temporary stack */
    movl $stack_top, %esp
    
    /* Clear direction flag */
    cld
    
    /* Call kernel (still 32-bit for now) */
    call kernel_main
    
    /* Hang if kernel returns */
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
