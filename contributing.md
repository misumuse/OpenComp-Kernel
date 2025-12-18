# Contributing to OpenComp

Thank you for your interest in contributing to OpenComp! This document provides guidelines for contributing to the project.

## Getting Started

1. Fork the repository on GitHub
2. Clone your fork locally
3. Create a new branch for your feature or bugfix
4. Make your changes
5. Test thoroughly in QEMU
6. Submit a pull request

## Development Environment Setup

### Required Tools

- `x86_64-elf-gcc` (cross-compiler)
- `x86_64-elf-ld` (linker)
- `grub-mkrescue` (ISO creation)
- `qemu-system-x86_64` (testing)

### Building and Testing

```bash
make clean
make
make run
```

## Coding Guidelines

### Style

- Use 4 spaces for indentation (no tabs)
- Keep lines under 100 characters when possible
- Use descriptive variable names
- Add comments for complex logic

### Component Guidelines

1. **Self-Contained**: Components should be independent
2. **Single Responsibility**: Each component does one thing well
3. **Minimal Dependencies**: Avoid tight coupling between components
4. **Documentation**: Document your component's purpose and API

### Example Component Template

```c
/* mycomponent.c
 *
 * Brief description of what this component does
 * Copyright (C) 2025 Your Name
 * Licensed under GNU GPLv2
 */

#include "kernel.h"

// Private state (static)
static int my_state = 0;

// Helper functions (static)
static void helper_function(void) {
    // ...
}

// Public API (if needed)
void my_component_do_something(void) {
    // ...
}

// Component init function
static void mycomponent_init(void) {
    puts("[mycomponent] Initializing...\n");
    // Initialize state here
}

// Component tick function
static void mycomponent_tick(void) {
    // Called repeatedly by kernel main loop
}

// Component registration
__attribute__((section(".compobjs"))) static struct component mycomponent_component = {
    .name = "mycomponent",
    .init = mycomponent_init,
    .tick = mycomponent_tick
};

__attribute__((section(".comps"))) struct component *p_mycomponent_component = &mycomponent_component;
```

## Pull Request Process

1. **Update Documentation**: Update README.md and ARCHITECTURE.md if needed
2. **Add Tests**: Ensure your changes work in QEMU
3. **Clean Commits**: Use clear, descriptive commit messages
4. **One Feature Per PR**: Keep pull requests focused on one feature/fix
5. **Update Makefile**: Add new source files to the build system

### Commit Message Format

```
[component] Brief description (50 chars or less)

Detailed explanation of what changed and why.
Can be multiple paragraphs.

Fixes #123
```

Examples:
```
[desktop] Add window minimize functionality

[keyboard] Fix buffer overflow in scancode handler

[memory] Improve page allocation performance
```

## What to Contribute

### High Priority

- Timer interrupt support
- Mouse driver (PS/2)
- Better memory management (virtual memory)
- Process/task management
- System call interface

### Medium Priority

- Serial port debugging output
- More desktop applications
- Better error handling
- Documentation improvements

### Ideas Welcome

- Network stack basics
- Simple filesystem support
- Audio support
- Additional hardware drivers

## Reporting Bugs

Use GitHub Issues to report bugs. Include:

1. **Description**: Clear description of the bug
2. **Steps to Reproduce**: How to trigger the bug
3. **Expected Behavior**: What should happen
4. **Actual Behavior**: What actually happens
5. **Environment**: QEMU version, build environment, etc.

### Bug Report Template

```markdown
**Description**
Brief description of the bug

**To Reproduce**
1. Boot kernel
2. Type command 'xyz'
3. Observe crash

**Expected Behavior**
Command should execute normally

**Actual Behavior**
Kernel crashes with page fault

**Environment**
- QEMU version: 6.2.0
- GCC version: 11.2.0
- Host OS: Ubuntu 22.04
```

## Feature Requests

Feature requests are welcome! Please use GitHub Issues and include:

1. **Use Case**: Why is this feature needed?
2. **Proposed Solution**: How might it work?
3. **Alternatives**: Other approaches considered?
4. **Additional Context**: Any other relevant information

## Code Review Process

All submissions require review. We look for:

- **Correctness**: Does it work as intended?
- **Style**: Does it follow project conventions?
- **Documentation**: Is it properly documented?
- **Testing**: Has it been tested?
- **Integration**: Does it fit the architecture?

## License

By contributing, you agree that your contributions will be licensed under the GNU General Public License v2.0.

## Questions?

Feel free to open an issue with the "question" label if you need help or clarification.

## Community

Be respectful and constructive. We're all here to learn and build something cool together!

---

Thank you for contributing to OpenComp! 🚀
