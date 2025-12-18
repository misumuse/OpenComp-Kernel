# OpenComp Architecture Documentation

## Overview

OpenComp is built around a component-based architecture that allows features to be added modularly without modifying the core kernel.

## Boot Process

1. **GRUB Loads Kernel** - Multiboot2 bootloader loads `tinykernel.elf` at `0x100000`
2. **Assembly Entry** (`start.S`) - Sets up stack at `0x7C00` and calls `kernel_main()`
3. **Kernel Initialization** (`kernel.c`) - Initializes VGA and scans for components
4. **Component Registration** - Calls `init()` on each component in `.comps` section
5. **Main Loop** - Repeatedly calls `tick()` on all components

## Memory Layout

```
0x000000 - 0x0FFFFF : Reserved (BIOS, VGA, etc.)
0x100000 - 0x1FFFFF : Kernel code and data
0x200000 - 0x1FFFFF : Managed physical memory (16MB)
```

### Memory Management

The memory component (`memory.c`) implements a simple bitmap allocator:

- **Page Size**: 4096 bytes (4KB)
- **Total Pages**: 4096 (16MB total)
- **Bitmap**: 512 bytes tracking page allocation
- **Functions**:
  - `kalloc_page()` - Allocate one 4KB page
  - `kfree_page()` - Free a page
  - `get_free_pages()` - Query available pages

## Component System

### Component Structure

```c
struct component {
    const char *name;      // Component identifier
    void (*init)(void);    // Called once at boot
    void (*tick)(void);    // Called repeatedly in main loop
};
```

### Component Sections

The linker script defines special sections:

- `.compobjs` - Contains actual component structures
- `.comps` - Contains pointers to components

The linker provides symbols:
- `__start_comps` - Start of component pointer array
- `__stop_comps` - End of component pointer array

### Creating a Component

1. Define init and tick functions
2. Create component structure in `.compobjs` section
3. Create pointer to component in `.comps` section
4. Add to Makefile

Example:
```c
static void my_init(void) { /* ... */ }
static void my_tick(void) { /* ... */ }

__attribute__((section(".compobjs"))) 
static struct component my_comp = {
    .name = "my_component",
    .init = my_init,
    .tick = my_tick
};

__attribute__((section(".comps"))) 
struct component *p_my_comp = &my_comp;
```

## VGA Text Mode

### Display Parameters

- **Resolution**: 80x25 characters
- **Memory**: `0xB8000` - `0xB8FA0`
- **Format**: 16-bit values (8-bit character + 8-bit color)

### Color Format

```
Bits 0-3: Foreground color
Bits 4-6: Background color
Bit 7:    Blink
```

### VGA Functions

- `vga_putchar(char c)` - Write character at cursor
- `vga_putchar_at(x, y, c, color)` - Write at specific position
- `vga_clear(color)` - Clear screen with color

## Desktop Environment

### Window System

The desktop component manages up to 8 windows:

```c
typedef struct {
    int active;          // Window enabled
    int x, y;           // Position
    int width, height;  // Dimensions
    char title[32];     // Window title
    char content[256];  // Window content
    uint8_t color;      // Border/text color
} Window;
```

### Command Processing

Commands are entered at the bottom command line (`CMD>` prompt):

1. User types command
2. Keyboard component captures input
3. Desktop component buffers keystrokes
4. Enter key triggers `handle_command()`
5. Command parsed and executed
6. New window created to display results

### Drawing Pipeline

```
tick() called every ~100ms
  ↓
Check if redraw needed (every 10 ticks)
  ↓
Clear screen (blue background)
  ↓
Draw title bar
  ↓
For each active window:
  - Draw border
  - Draw title
  - Draw content
  ↓
Draw command line and cursor
```

## Keyboard Driver

### PS/2 Controller

- **Data Port**: `0x60`
- **Status Port**: `0x64`

### Scancode Translation

The keyboard driver converts scancodes to ASCII:

1. Read status port (`0x64`) - check if data available
2. Read data port (`0x60`) - get scancode
3. Ignore release codes (bit 7 set)
4. Look up ASCII character in translation table
5. Add to ring buffer

### Ring Buffer

- **Size**: 64 characters
- **Read Pointer**: Next character to read
- **Write Pointer**: Next position to write

Functions:
- `keyboard_has_key()` - Check if keys available
- `keyboard_get_key()` - Read and remove one key

## Build System

### Compilation Flags

```
-O2              : Optimize for speed
-ffreestanding   : Freestanding environment (no standard library)
-nostdlib        : Don't link standard library
-fno-builtin     : Don't use built-in functions
-mno-red-zone    : Required for kernel code (x86-64)
```

### Linking Process

1. Compile each `.c` file to `.o` object
2. Link objects using `linker.ld` script
3. Produces `tinykernel.elf` executable
4. Copy to ISO directory structure
5. Create bootable ISO with `grub-mkrescue`

## Future Architecture Considerations

### Virtual Memory

To support virtual memory:
1. Set up page tables in `memory.c`
2. Enable paging in `start.S`
3. Map kernel to higher half (`0xFFFFFFFF80000000`)
4. Update linker script addresses

### Interrupt Handling

To add interrupt support:
1. Create IDT (Interrupt Descriptor Table)
2. Write interrupt handlers in `start.S`
3. Register handlers for keyboard, timer, etc.
4. Enable interrupts with `sti` instruction

### Process Management

To add multitasking:
1. Create process/task structures
2. Implement context switching
3. Add scheduler component
4. Create system call interface

### File System

To add filesystem:
1. Create initrd (initial ramdisk)
2. Parse filesystem format (tar, ext2, custom)
3. Add VFS (Virtual File System) layer
4. Create file operations component

## Performance Considerations

### Current Limitations

- **Polling-based I/O**: Components poll in tight loop (inefficient)
- **No Interrupts**: Can't respond to hardware events asynchronously
- **Single-threaded**: One component blocks all others
- **Fixed Tick Rate**: Hardcoded delay in main loop

### Optimization Strategies

1. **Add Timer Interrupts**: More precise timing control
2. **Event Queue**: Components register for specific events
3. **Priorities**: Give important components more CPU time
4. **Preemption**: Allow high-priority tasks to interrupt others

## Debugging Tips

### QEMU Debug Options

```bash
# Run with serial output
qemu-system-x86_64 -cdrom opencomp.iso -serial stdio

# Run with GDB support
qemu-system-x86_64 -cdrom opencomp.iso -s -S
# In another terminal:
gdb tinykernel.elf
(gdb) target remote localhost:1234
(gdb) continue
```

### Adding Debug Output

Add serial port output for debugging:

```c
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void debug_print(const char *s) {
    while (*s) {
        outb(0x3F8, *s++);  // COM1 serial port
    }
}
```

## References

- [OSDev Wiki - Components](https://wiki.osdev.org/)
- [Intel 64 Architecture Manual](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html)
- [GRUB Multiboot2 Specification](https://www.gnu.org/software/grub/manual/multiboot2/)
