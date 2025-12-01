/* kernel.c
 *
 * Tiny pluggable-kernel example (educational).
 * Copyright (C) 2025 B."Nova" J.
 * Licensed under GNU GPLv2
 *
 * What it does:
 *  - minimal VGA text output
 *  - a "component" API: components provide a name and init/tick functions
 *  - an example component included at the bottom (hello_component)
 *
 * Note: This is a minimal 64-bit kernel meant to be loaded by GRUB (multiboot2).
 */

#include <stdint.h>
#include <stddef.h>

/* ------------------------------
   Placeholder
   ------------------------------
*/

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint16_t *vga = (uint16_t *)0xB8000;
static size_t vga_row = 0, vga_col = 0;
static uint8_t vga_color = 0x0f;

static void vga_putchar(char c) {
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else {
        size_t index = vga_row * VGA_WIDTH + vga_col;
        vga[index] = ((uint16_t)vga_color << 8) | (uint8_t)c;
        vga_col++;
        if (vga_col >= VGA_WIDTH) {
            vga_col = 0;
            vga_row++;
        }
    }
    if (vga_row >= VGA_HEIGHT) {
        // simple scroll: move lines up one
        for (size_t r = 1; r < VGA_HEIGHT; ++r)
            for (size_t c = 0; c < VGA_WIDTH; ++c)
                vga[(r-1)*VGA_WIDTH + c] = vga[r*VGA_WIDTH + c];
        // clear last line
        size_t start = (VGA_HEIGHT-1)*VGA_WIDTH;
        for (size_t c = 0; c < VGA_WIDTH; ++c)
            vga[start + c] = ((uint16_t)vga_color << 8) | ' ';
        vga_row = VGA_HEIGHT - 1;
        vga_col = 0;
    }
}

static void puts(const char *s) {
    while (*s) vga_putchar(*s++);
}

static void itoa_u(uint64_t v, char *buf) {
    char tmp[32];
    int i = 0;
    if (v == 0) { buf[0]='0'; buf[1]=0; return; }
    while (v) { tmp[i++] = '0' + (v % 10); v /= 10; }
    int j = 0;
    while (i--) buf[j++] = tmp[i];
    buf[j]=0;
}

/* ------------------------------
   Component API
   ------------------------------
   Components are simple C objects that implement this interface:
     struct component {
         const char *name;
         void (*init)(void);
         void (*tick)(void);   // called repeatedly by kernel main loop
     };
   To add a component, create a C file that defines a 'struct component my_comp = { ... }'
   and compile/link it into the kernel. This keeps runtime simple (no dynamic loader).
*/

struct component {
    const char *name;
    void (*init)(void);
    void (*tick)(void);
};

/* Forward declare: the linker will put component pointers into the .comps section.
   Each component should define: struct component compNAME __attribute__((section(".comps"))) = { ... };
*/
extern struct component *__start_comps;
extern struct component *__stop_comps;

static void register_components_and_init(void) {
    struct component **it = (struct component **)&__start_comps;
    struct component **end = (struct component **)&__stop_comps;
    if (it == end) {
        puts("No components found.\n");
        return;
    }
    while (it < end) {
        struct component *c = *it;
        if (c && c->name) {
            puts("Component: ");
            puts(c->name);
            puts(" - init\n");
            if (c->init) c->init();
        }
        ++it;
    }
}

/* Kernel main loop: call tick() on each component in round-robin */
static void kernel_main_loop(void) {
    struct component **it = (struct component **)&__start_comps;
    struct component **end = (struct component **)&__stop_comps;
    while (1) {
        for (struct component **p = it; p < end; ++p) {
            struct component *c = *p;
            if (c && c->tick) c->tick();
        }
        // simple busy-wait delay
        for (volatile uint64_t i=0;i<2000000ULL;i++);
    }
}

/* Entry point called from assembly stub (see linker + assembly). */
void kernel_main(void) {
    // basic welcome
    puts("TinyKernel: component-capable kernel (GPLv3)\n");
    // init components
    register_components_and_init();
    puts("Entering main loop...\n");
    kernel_main_loop();
    // never returns
}

/* ------------------------------
   Example component (hello)
   ------------------------------
   A component compiled into the kernel. Real components should live in
   separate files and be added at compile/link time.
*/
static int hello_state = 0;
static void hello_init(void) {
    puts("[hello] initialized\n");
}
static void hello_tick(void) {
    char buf[32];
    if ((hello_state++ % 100)==0) {
        puts("[hello] tick ");
        itoa_u(hello_state, buf);
        puts(buf);
        puts("\n");
    }
}

/* Place pointer to component into .comps by using a little trick:
   we create a static struct component object and then a pointer to it in .comps.
*/
__attribute__((section(".compobjs"))) static struct component hello_component = {
    .name = "hello",
    .init = hello_init,
    .tick = hello_tick
};

/* We need pointers in .comps. Create an aliased pointer that goes into .comps */
__attribute__((section(".comps"))) struct component *p_hello_component = &hello_component;

/* ------------------------------
   Minimal stubs for required symbols
   ------------------------------
   Nova Says: For this we don't need full libc. Contributors, add as needed. Define minimal symbols.
*/
void _start_crt_stub(void) { kernel_main(); for(;;); }
