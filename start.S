/* start.S - minimal 64-bit entry with multiboot2 header */

/* Multiboot2 header - MUST be in first 32KB of kernel */
.section .multiboot
.align 8
multiboot2_header_start:
    .long 0xE85250D6                /* magic number (multiboot2) */
    .long 0                         /* architecture: 0 = 32-bit i386 */
    .long multiboot2_header_end - multiboot2_header_start  /* header length */
    /* checksum */
    .long -(0xE85250D6 + 0 + (multiboot2_header_end - multiboot2_header_start))
    
    /* information request tag */
    .align 8
    .short 1    /* type = information request */
    .short 0    /* flags */
    .long 12    /* size */
    .long 3     /* request basic memory info */
    
    /* end tag */
    .align 8
    .short 0    /* type = end */
    .short 0    /* flags */
    .long 8     /* size */
multiboot2_header_end:

/* Entry point */
.section .text
.global _start
.type _start, @function

_start:
    /* Disable interrupts */
    cli
    
    /* Set up stack (64-bit) */
    movq $stack_top, %rsp
    
    /* Call kernel main */
    call kernel_main
    
    /* Hang if kernel returns */
.hang:
    cli
    hlt
    jmp .hang

/* Reserve space for stack */
.section .bss
.align 16
stack_bottom:
    .skip 16384  /* 16 KB stack */
stack_top:
