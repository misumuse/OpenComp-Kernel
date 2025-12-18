# OpenComp Kernel

OpenComp is an educational component-based kernel featuring a simple text-mode desktop environment. It's designed to help developers understand operating system fundamentals while maintaining a clean, modular architecture.

![License](https://img.shields.io/badge/license-GPLv2-blue.svg)
![Platform](https://img.shields.io/badge/platform-x86--64-lightgrey.svg)
![Build](https://img.shields.io/badge/build-multiboot2-green.svg)

## Features

- **Component Architecture**: Modular design where features are self-contained components
- **Memory Management**: Bitmap-based physical page allocator managing 16MB
- **Keyboard Driver**: PS/2 keyboard input with scancode translation
- **Desktop Environment**: Text-mode windowing system with command interface
- **VGA Display**: Enhanced text-mode graphics with color support
- **Multiboot2 Compliant**: Boots with GRUB2 bootloader

## Quick Start

```bash
# Clone the repository
git clone https://github.com/misumuse/OpenComp-Kernel.git
cd OpenComp-Kernel

# Build and run
make
make run
```

**First time building?** See [QUICKSTART.md](QUICKSTART.md) for detailed setup instructions.

## Screenshots

*Text-mode desktop with windowing system*

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                      OpenComp Desktop Environment                             │
├──────────────────────────────────────────────────────────────────────────────┤
│                                                                                │
│     ┌────────────────────────────────────┐                                    │
│     │    Welcome to OpenComp             │                                    │
│     ├────────────────────────────────────┤                                    │
│     │                                    │                                    │
│     │ Welcome to OpenComp!               │                                    │
│     │                                    │                                    │
│     │ Type 'help' for commands.          │                                    │
│     │                                    │                                    │
│     └────────────────────────────────────┘                                    │
│                                                                                │
│                                                                                │
├──────────────────────────────────────────────────────────────────────────────┤
│ CMD> help_                                                                     │
└──────────────────────────────────────────────────────────────────────────────┘
```

## Building

### Prerequisites

- `x86_64-elf-gcc` cross-compiler
- `x86_64-elf-ld` linker
- `grub-mkrescue` (for creating bootable ISO)
- `qemu-system-x86_64` (for testing)

#### Installing Prerequisites (Ubuntu/Debian)

```bash
sudo apt-get install build-essential grub-pc-bin xorriso qemu-system-x86

# For cross-compiler, you may need to build from source or install via:
sudo apt-get install gcc-x86-64-linux-gnu
```

### Compilation

```bash
make              # Build the kernel
make run          # Build and run in QEMU
make clean        # Clean build artifacts
```

## Usage

Once booted, you'll see the desktop environment with a command prompt at the bottom.

### Available Commands

- `help` - Display available commands
- `about` - Show kernel information
- `mem` - Display memory statistics
- `clear` - Close all windows

### Keyboard Support

Type commands at the bottom prompt and press Enter to execute them.

## Architecture

OpenComp uses a component-based architecture where each major feature is a self-contained component with `init()` and `tick()` functions.

### Current Components

1. **Memory Component** - Physical memory management
2. **Keyboard Component** - PS/2 keyboard driver
3. **Desktop Component** - Windowing system and UI

### Adding New Components

Create a new `.c` file with this structure:

```c
#include "kernel.h"

static void my_component_init(void) {
    // Initialize your component
}

static void my_component_tick(void) {
    // Called repeatedly in main loop
}

__attribute__((section(".compobjs"))) static struct component my_component = {
    .name = "my_component",
    .init = my_component_init,
    .tick = my_component_tick
};

__attribute__((section(".comps"))) struct component *p_my_component = &my_component;
```

Add your component to the Makefile `OBJS` list and recompile.

## Project Structure

```
├── LICENSE           - GNU GPLv2 license
├── README.md         - This file
├── Makefile          - Build configuration
├── kernel.h          - Main kernel header
├── kernel.c          - Core kernel code
├── memory.c          - Memory management component
├── keyboard.c        - Keyboard driver component
├── desktop.c         - Desktop environment component
├── start.S           - Assembly entry point
└── linker.ld         - Linker script
```

## Roadmap

Future features planned:

- [ ] Mouse support
- [ ] Simple filesystem (initrd)
- [ ] Process/task management
- [ ] System call interface
- [ ] Network stack (basic)
- [ ] More desktop applications
- [ ] Timer interrupts
- [ ] Better memory management (virtual memory)

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for bugs and feature requests.

### Development Guidelines

1. Follow the existing component architecture
2. Keep components self-contained
3. Document your code
4. Test in QEMU before submitting
5. Update README with new features

## License

OpenComp is licensed under the GNU General Public License v2.0. See [LICENSE](LICENSE) for details.

## Credits

Copyright (C) 2025 B."Nova" J.

OpenComp is meant to help development of other kernels, such as GNU/Linux and BSD, by providing educational examples of kernel development techniques.

## Resources

- [OSDev Wiki](https://wiki.osdev.org/)
- [GRUB Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/)
- [x86-64 Architecture](https://en.wikipedia.org/wiki/X86-64)