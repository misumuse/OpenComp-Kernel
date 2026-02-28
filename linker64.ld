/* linker64.ld - linker script for experimental x86_64 kernel */
ENTRY(_start)

SECTIONS
{
  . = 1M;

  .multiboot ALIGN(8) : {
    KEEP(*(.multiboot))
  }

  .text ALIGN(4K) : {
    *(.text*)
  }

  .rodata ALIGN(4K) : {
    *(.rodata*)
  }

  .data ALIGN(4K) : {
    *(.data*)
  }

  .bss ALIGN(4K) : {
    *(COMMON)
    *(.bss*)
  }

  /DISCARD/ : {
    *(.comment)
    *(.note*)
    *(.eh_frame*)
  }
}
