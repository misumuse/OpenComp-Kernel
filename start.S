/* start.S - minimal 64-bit entry used by GRUB multiboot (flat) */
.global _start

_start:
    /* set up a simple stack */
    mov $0x7C00, %rsp     /* a temporary stack - when using a proper bootloader adjust this */
    
    /* Call kernel main (C) */
    call kernel_main
    
1:  
    jmp 1b
