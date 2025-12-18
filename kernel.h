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

/* VGA functions */
void vga_putchar(char c);
void vga_putchar_at(int x, int y, char c, uint8_t color);
void vga_clear(uint8_t color);
void puts(const char *s);
void itoa_u(uint64_t v, char *buf);
void str_append(char *dest, const char *src);

/* Memory management */
void *kalloc_page(void);
void kfree_page(void *addr);
uint64_t get_free_pages(void);

/* Keyboard */
int keyboard_has_key(void);
char keyboard_get_key(void);

#endif
