/* linker.ld - simple script for flat kernel image */
ENTRY(_start)
SECTIONS
{
  . = 0x100000;

  .text : {
    *(.multiboot)
    *(.text*)
  }

  .rodata : { *(.rodata*) }

  .data : { *(.data*) }

  .compobjs : { *(.compobjs*) }    /* component objects */
  .comps : { *(.comps*) }          /* pointers to components */

  __start_comps = ADDR(.comps);
  __stop_comps  = __start_comps + SIZEOF(.comps);

  .bss (NOLOAD) : { *(.bss*) }

  /DISCARD/ : { *(.note*) *(.eh_frame*) }
}
