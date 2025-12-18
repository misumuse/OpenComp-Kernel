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

### Windows

#### Option 1: WSL2 (Recommended)

Windows Subsystem for Linux provides a native Linux environment:

```bash
# 1. Install WSL2 (PowerShell as Administrator)
wsl --install

# 2. Restart your computer

# 3. Open Ubuntu from Start Menu and create a user account

# 4. Install dependencies in WSL
sudo apt-get update
sudo apt-get install -y build-essential grub-pc-bin xorriso qemu-system-x86

# 5. Clone and build OpenComp
git clone https://github.com/YOUR_USERNAME/opencomp.git
cd opencomp
make
make run
```

#### Option 2: MSYS2/MinGW

Use MSYS2 for a Windows-native build environment:

```bash
# 1. Download and install MSYS2 from https://www.msys2.org/

# 2. Open MSYS2 MINGW64 terminal

# 3. Install dependencies
pacman -S base-devel mingw-w64-x86_64-gcc make git

# 4. Install QEMU for Windows
# Download from: https://qemu.weilnetz.de/w64/
# Or use: pacman -S mingw-w64-x86_64-qemu

# 5. For GRUB, you'll need to use WSL or a Linux VM to create the ISO
# Alternatively, test with raw ELF in QEMU:
qemu-system-x86_64 -kernel tinykernel.elf -m 256M
```

#### Option 3: Cygwin

```bash
# 1. Download Cygwin installer from https://www.cygwin.com/

# 2. Run setup and select these packages:
#    - gcc-core
#    - gcc-g++
#    - make
#    - git
#    - grub2
#    - xorriso

# 3. Download QEMU for Windows from https://qemu.weilnetz.de/

# 4. Build OpenComp in Cygwin terminal
git clone https://github.com/YOUR_USERNAME/opencomp.git
cd opencomp
make
# Run QEMU from Windows install location
/c/Program\ Files/qemu/qemu-system-x86_64.exe -cdrom opencomp.iso -m 256M
```

#### Option 4: Virtual Machine

If other options don't work, use a Linux VM:

```bash
# 1. Install VirtualBox or VMware
# 2. Create Ubuntu VM
# 3. Follow Ubuntu/Debian instructions above
```

#### Windows Tips

- **WSL2 GUI**: Install WSLg for graphical QEMU window:
  ```bash
  # In WSL2
  sudo apt-get install qemu-system-gui
  ```

- **File Access**: Access WSL files from Windows at `\\wsl$\Ubuntu\home\username\opencomp`

- **Performance**: WSL2 offers near-native Linux performance

- **VS Code Integration**: Use VS Code with WSL extension for seamless editing

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

**Windows WSL2**: Make sure you installed build-essential:
```bash
sudo apt-get install build-essential
```

### ISO creation fails

**Problem**: `grub-mkrescue: command not found`

**Solution**: Install GRUB tools:

**Linux**:
```bash
sudo apt-get install grub-pc-bin xorriso
```

**Windows WSL2**: Same as Linux above

**Windows MSYS2**: ISO creation not fully supported. Test with:
```bash
qemu-system-x86_64 -kernel tinykernel.elf -m 256M
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

**Windows**: If using QEMU GUI, press `Ctrl+Alt+G` to grab/release mouse and keyboard.

### "Permission denied" on WSL2

**Problem**: Can't execute commands or access files

**Solution**:
```bash
# Make sure files have execute permissions
chmod +x script_name.sh

# Or for the whole project
cd opencomp
chmod -R 755 .
```

### QEMU window doesn't appear on Windows

**Problem**: QEMU runs but no window shows

**Solution**: 
- Install WSLg for GUI support in WSL2
- Or use `-nographic` flag and interact via terminal:
  ```bash
  qemu-system-x86_64 -cdrom opencomp.iso -m 256M -nographic
  ```
- Or use VNC: `-vnc :1` then connect with VNC client to `localhost:5901`

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
