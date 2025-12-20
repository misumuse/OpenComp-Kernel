/* kernel.c
 *
 * Tiny pluggable-kernel example (educational).
 * Copyright (C) 2025 B."Nova" J.
 * Licensed under GNU GPLv2
 *
 * What it does:
 *  - VGA text output with extended functions
 *  - a "component" API: components provide a name and init/tick functions
 *  - desktop environment support
 *  - keyboard input
 *  - memory management
 *
 * Note: This is a minimal 64-bit kernel meant to be loaded by GRUB (multiboot2).
 */

#include <stdint.h>
#include <stddef.h>
#include "kernel.h"

/* ------------------------------
   VGA Display Functions
   ------------------------------
*/

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
static uint16_t *vga = (uint16_t *)0xB8000;
static size_t vga_row = 0, vga_col = 0;
static uint8_t vga_color = 0x0f;

void vga_putchar_at(int x, int y, char c, uint8_t color) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        size_t index = y * VGA_WIDTH + x;
        vga[index] = ((uint16_t)color << 8) | (uint8_t)c;
    }
}

void vga_clear(uint8_t color) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            size_t index = y * VGA_WIDTH + x;
            vga[index] = ((uint16_t)color << 8) | ' ';
        }
    }
    vga_row = 0;
    vga_col = 0;
}

void vga_putchar(char c) {
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

void puts(const char *s) {
    while (*s) vga_putchar(*s++);
}

void itoa_u(uint64_t v, char *buf) {
    char tmp[32];
    int i = 0;
    if (v == 0) { buf[0]='0'; buf[1]=0; return; }
    while (v) { tmp[i++] = '0' + (v % 10); v /= 10; }
    int j = 0;
    while (i > 0) buf[j++] = tmp[--i];
    buf[j]=0;
}

void str_append(char *dest, const char *src) {
    while (*dest) dest++;
    while (*src) *dest++ = *src++;
    *dest = 0;
}

/* ------------------------------
   Component API
   ------------------------------
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
        for (volatile uint64_t i=0;i<1000000ULL;i++);
    }
}

/* Entry point called from assembly stub */
void kernel_main(void) {
    // basic welcome
    vga_clear(0x0F);
    puts("OpenComp Kernel - Component-Based OS (GPLv2)\n");
    puts("============================================\n\n");
    
    // init components
    register_components_and_init();
    puts("\nEntering main loop...\n");
    
    // Small delay to show init messages
    for (volatile uint64_t i=0;i<50000000ULL;i++);
    
    kernel_main_loop();
    // never returns
}

/* ------------------------------
   Minimal stubs for required symbols
   ------------------------------
*/
void _start_crt_stub(void) { kernel_main(); for(;;); }
