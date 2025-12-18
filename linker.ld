/* linker.ld - linker script for multiboot2 kernel */
ENTRY(_start)

SECTIONS
{
  /* Start at 1 MB (standard for multiboot) */
  . = 1M;

  /* Multiboot header MUST come first */
  .multiboot ALIGN(8) : {
    KEEP(*(.multiboot))
  }

  /* Code section */
  .text ALIGN(4K) : {
    *(.text*)
  }

  /* Read-only data */
  .rodata ALIGN(4K) : {
    *(.rodata*)
  }

  /* Initialized data */
  .data ALIGN(4K) : {
    *(.data*)
  }

  /* Component objects */
  .compobjs ALIGN(8) : {
    *(.compobjs*)
  }

  /* Component pointers - with symbols for iteration */
  .comps ALIGN(8) : {
    __start_comps = .;
    *(.comps*)
    __stop_comps = .;
  }

  /* Uninitialized data */
  .bss ALIGN(4K) : {
    *(COMMON)
    *(.bss*)
  }

  /* Discard unnecessary sections */
  /DISCARD/ : {
    *(.comment)
    *(.note*)
    *(.eh_frame*)
  }
}
