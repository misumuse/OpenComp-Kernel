/* kernel.h
 *
 * Main kernel header for OpenComp
 * Copyright (C) 2025 B."Nova" J.
 * Licensed under GNU GPLv2
 */

#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

/* Component structure */
struct component {
    const char *name;
    void (*init)(void);
    void (*tick)(void);
};

/* VGA Text Mode functions */
void vga_putchar(char c);
void vga_putchar_at(int x, int y, char c, uint8_t color);
void vga_clear(uint8_t color);
void puts(const char *s);

/* VGA Graphics Mode functions */
void vga_setpixel(int x, int y, uint8_t color);
void vga_clear_screen(uint8_t color);
void vga_fill_rect(int x, int y, int w, int h, uint8_t color);
void vga_draw_rect(int x, int y, int w, int h, uint8_t color);
void vga_draw_line(int x0, int y0, int x1, int y1, uint8_t color);
void vga_draw_char(int x, int y, char c, uint8_t color);
void vga_draw_string(int x, int y, const char *str, uint8_t color);

/* Utility functions */
void itoa_u(uint64_t v, char *buf);
void str_append(char *dest, const char *src);

/* Memory management */
void *kalloc_page(void);
void kfree_page(void *addr);
uint64_t get_free_pages(void);

/* Keyboard */
int keyboard_has_key(void);
char keyboard_get_key(void);

/* Mouse */
void mouse_get_position(int *x, int *y);
uint8_t mouse_get_buttons(void);

/* Filesystem */
void fs_set_initrd(uint8_t *addr, uint32_t size);
int fs_get_file_count(void);
int fs_get_file_info(int index, char *name, uint32_t *size, int *is_dir);
int fs_read_file(const char *filename, uint8_t **data, uint32_t *size);
int fs_read_file_by_index(int index, uint8_t **data, uint32_t *size);

#endif
