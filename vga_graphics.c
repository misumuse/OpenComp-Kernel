/* vga_graphics.c
 *
 * VGA Mode 13h graphics driver (320x200, 256 colors)
 * Copyright (C) 2025 B."Nova" J.
 * Licensed under GNU GPLv2
 */

#include <stdint.h>
#include "kernel.h"

#define VGA_WIDTH 320
#define VGA_HEIGHT 200
#define VGA_MEMORY 0xA0000

static uint8_t *framebuffer = (uint8_t *)VGA_MEMORY;

// VGA registers
#define VGA_MISC_WRITE 0x3C2
#define VGA_SEQ_INDEX 0x3C4
#define VGA_SEQ_DATA 0x3C5
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA 0x3D5
#define VGA_GC_INDEX 0x3CE
#define VGA_GC_DATA 0x3CF

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Set VGA Mode 13h (320x200, 256 colors)
static void set_mode_13h(void) {
    // Switch to mode 13h using BIOS interrupt would require real mode
    // Instead we'll set registers directly
    
    outb(VGA_MISC_WRITE, 0x63);
    
    // Sequencer registers
    outb(VGA_SEQ_INDEX, 0x00); outb(VGA_SEQ_DATA, 0x03);
    outb(VGA_SEQ_INDEX, 0x01); outb(VGA_SEQ_DATA, 0x01);
    outb(VGA_SEQ_INDEX, 0x02); outb(VGA_SEQ_DATA, 0x0F);
    outb(VGA_SEQ_INDEX, 0x03); outb(VGA_SEQ_DATA, 0x00);
    outb(VGA_SEQ_INDEX, 0x04); outb(VGA_SEQ_DATA, 0x0E);
    
    // Unlock CRTC registers
    outb(VGA_CRTC_INDEX, 0x11);
    uint8_t val = inb(VGA_CRTC_DATA);
    outb(VGA_CRTC_DATA, val & 0x7F);
    
    // CRTC registers for 320x200
    uint8_t crtc_regs[] = {
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3
    };
    
    for (int i = 0; i < 24; i++) {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, crtc_regs[i]);
    }
    
    // Graphics controller
    outb(VGA_GC_INDEX, 0x05); outb(VGA_GC_DATA, 0x40);
    outb(VGA_GC_INDEX, 0x06); outb(VGA_GC_DATA, 0x05);
}

// Set a pixel at (x, y) with color
void vga_setpixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        framebuffer[y * VGA_WIDTH + x] = color;
    }
}

// Clear screen with color
void vga_clear_screen(uint8_t color) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        framebuffer[i] = color;
    }
}

// Draw a filled rectangle
void vga_fill_rect(int x, int y, int w, int h, uint8_t color) {
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            vga_setpixel(x + dx, y + dy, color);
        }
    }
}

// Draw a rectangle outline
void vga_draw_rect(int x, int y, int w, int h, uint8_t color) {
    // Top and bottom
    for (int dx = 0; dx < w; dx++) {
        vga_setpixel(x + dx, y, color);
        vga_setpixel(x + dx, y + h - 1, color);
    }
    // Left and right
    for (int dy = 0; dy < h; dy++) {
        vga_setpixel(x, y + dy, color);
        vga_setpixel(x + w - 1, y + dy, color);
    }
}

// Draw a line (Bresenham's algorithm)
void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    
    while (1) {
        vga_setpixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        
        int e2 = err;
        if (e2 > -dx) { err -= dy; x0 += sx; }
        if (e2 < dy) { err += dx; y0 += sy; }
    }
}

// Simple 8x8 font (subset for now)
static uint8_t font_8x8[128][8] = {
    // Space (32)
    [32] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    // A (65)
    [65] = {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00},
    // B
    [66] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    // C
    [67] = {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00},
    // Add more letters as needed...
};

// Draw a character at (x, y)
void vga_draw_char(int x, int y, char c, uint8_t color) {
    if (c < 0 || c >= 128) return;
    
    for (int row = 0; row < 8; row++) {
        uint8_t line = font_8x8[(int)c][row];
        for (int col = 0; col < 8; col++) {
            if (line & (0x80 >> col)) {
                vga_setpixel(x + col, y + row, color);
            }
        }
    }
}

// Draw a string
void vga_draw_string(int x, int y, const char *str, uint8_t color) {
    int cx = x;
    while (*str) {
        vga_draw_char(cx, y, *str, color);
        cx += 8;
        str++;
    }
}

static void vga_graphics_init(void) {
    puts("[vga_graphics] Switching to Mode 13h (320x200)...\n");
    set_mode_13h();
    vga_clear_screen(0x00); // Black
    
    // Test pattern
    vga_fill_rect(10, 10, 100, 50, 0x0F); // White
    vga_draw_rect(120, 10, 100, 50, 0x0C); // Red outline
    vga_draw_line(10, 70, 310, 70, 0x0A); // Green line
    
    puts("[vga_graphics] Graphics mode initialized\n");
}

static void vga_graphics_tick(void) {
    // Nothing to do each tick for now
}

__attribute__((section(".compobjs"))) static struct component vga_graphics_component = {
    .name = "vga_graphics",
    .init = vga_graphics_init,
    .tick = vga_graphics_tick
};

__attribute__((section(".comps"))) struct component *p_vga_graphics_component = &vga_graphics_component;
