# OpenComp Quick Start Guide

Get OpenComp up and running in minutes!

## Prerequisites Installation

### Ubuntu/Debian

```bash
# Install build tools
sudo apt-get update
sudo apt-get install -y build-essential grub-pc-bin xorriso qemu-system-x86

# Install cross-compiler (you may need to build from source)
# Quick option: use the system compiler for testing
sudo apt-get install gcc
```

### Arch Linux

```bash
sudo pacman -S base-devel grub xorriso qemu-system-x86
```

### macOS

```bash
# Install Homebrew if you haven't already
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install x86_64-elf-gcc
brew install grub xorriso qemu
```

### Building Cross-Compiler (if needed)

If you need to build the cross-compiler:

```bash
# Download and build x86_64-elf-gcc
wget https://ftp.gnu.org/gnu/gcc/gcc-11.2.0/gcc-11.2.0.tar.gz
tar xf gcc-11.2.0.tar.gz
cd gcc-11.2.0
./contrib/download_prerequisites
mkdir build && cd build
../configure --target=x86_64-elf --disable-nls --enable-languages=c --without-headers
make all-gcc all-target-libgcc -j$(nproc)
sudo make install-gcc install-target-libgcc
```

## Building OpenComp

```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/opencomp.git
cd opencomp

# Build the kernel
make

# Run in QEMU
make run
```

## First Boot

When OpenComp boots, you'll see:

1. Boot messages showing component initialization
2. A blue desktop with a title bar
3. A welcome window in the center
4. A command prompt at the bottom: `CMD>`

## Trying Commands

Type these commands at the `CMD>` prompt:

### Display Help
```
help
```
Shows all available commands.

### View System Information
```
about
```
Displays kernel version and license info.

### Check Memory
```
mem
```
Shows free pages and available memory.

### Clear All Windows
```
clear
```
Closes all open windows.

## Understanding the Display

```
┌────────────────────────────────────────────┐
│      OpenComp Desktop Environment          │  ← Title Bar
├────────────────────────────────────────────┤
│                                            │
│  ┌──────────────────────┐                 │
│  │  Welcome Window      │ ← Window Title  │
│  ├──────────────────────┤                 │
│  │  Content goes here   │ ← Window Content│
│  │                      │                 │
│  └──────────────────────┘                 │
│                                            │
├────────────────────────────────────────────┤
│ CMD> _                                     │  ← Command Line
└────────────────────────────────────────────┘
```

## Keyboard Controls

- **Type** to enter commands
- **Enter** to execute commands
- **Backspace** to delete characters

## Troubleshooting

### QEMU won't start

**Problem**: `qemu-system-x86_64: command not found`

**Solution**: Install QEMU:
```bash
sudo apt-get install qemu-system-x86
```

### Build fails with "x86_64-elf-gcc not found"

**Problem**: Cross-compiler not installed

**Solution**: Either build the cross-compiler (see above) or modify Makefile:
```makefile
CC = gcc
LD = ld
```
Note: This may not work for all systems.

### ISO creation fails

**Problem**: `grub-mkrescue: command not found`

**Solution**: Install GRUB tools:
```bash
sudo apt-get install grub-pc-bin xorriso
```

### Black screen in QEMU

**Problem**: Kernel not booting properly

**Solution**: Check build output for errors. Try:
```bash
make clean
make
qemu-system-x86_64 -cdrom opencomp.iso -m 256M -serial stdio
```

### Can't type in QEMU

**Problem**: Window focus or keyboard not working

**Solution**: Click inside the QEMU window to grab keyboard focus.

## Next Steps

### Explore the Code

- `kernel.c` - Main kernel and VGA functions
- `desktop.c` - Desktop environment
- `keyboard.c` - Keyboard driver
- `memory.c` - Memory management

### Add a Component

Create `mycomponent.c`:

```c
#include "kernel.h"

static void mycomponent_init(void) {
    puts("[mycomponent] Hello from my component!\n");
}

static void mycomponent_tick(void) {
    // Called repeatedly
}

__attribute__((section(".compobjs"))) 
static struct component mycomponent_component = {
    .name = "mycomponent",
    .init = mycomponent_init,
    .tick = mycomponent_tick
};

__attribute__((section(".comps"))) 
struct component *p_mycomponent_component = &mycomponent_component;
```

Add to Makefile:
```makefile
OBJS = kernel.o start.o memory.o keyboard.o desktop.o mycomponent.o

mycomponent.o: mycomponent.c kernel.h
	$(CC) $(CFLAGS) -c mycomponent.c -o mycomponent.o
```

Rebuild:
```bash
make clean && make run
```

### Read the Documentation

- `README.md` - Project overview
- `ARCHITECTURE.md` - Technical details
- `CONTRIBUTING.md` - How to contribute

### Join Development

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

## Getting Help

- **Issues**: Open a GitHub issue
- **Questions**: Use the "question" label
- **Bugs**: Use the bug report template

## Resources

- [OSDev Wiki](https://wiki.osdev.org/) - Comprehensive OS development resource
- [Intel Manuals](https://software.intel.com/content/www/us/en/develop/articles/intel-sdm.html) - x86-64 architecture reference
- [GRUB Manual](https://www.gnu.org/software/grub/manual/) - Bootloader documentation

---

Welcome to OpenComp! Happy kernel hacking! 🚀
