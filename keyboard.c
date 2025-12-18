/* keyboard.c
 *
 * PS/2 Keyboard driver component
 * Copyright (C) 2025 B."Nova" J.
 * Licensed under GNU GPLv2
 */

#include <stdint.h>
#include "kernel.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

#define KEY_BUFFER_SIZE 64
static uint8_t key_buffer[KEY_BUFFER_SIZE];
static size_t key_read_pos = 0;
static size_t key_write_pos = 0;

static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*',
    0, ' '
};

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

int keyboard_has_key(void) {
    return key_read_pos != key_write_pos;
}

char keyboard_get_key(void) {
    if (!keyboard_has_key()) return 0;
    char c = key_buffer[key_read_pos];
    key_read_pos = (key_read_pos + 1) % KEY_BUFFER_SIZE;
    return c;
}

static void keyboard_init(void) {
    puts("[keyboard] PS/2 keyboard driver initialized\n");
}

static void keyboard_tick(void) {
    // Check if keyboard has data
    uint8_t status = inb(KEYBOARD_STATUS_PORT);
    if (status & 0x01) {
        uint8_t scancode = inb(KEYBOARD_DATA_PORT);
        
        // Ignore release codes (bit 7 set)
        if (scancode & 0x80) return;
        
        // Convert to ASCII
        if (scancode < sizeof(scancode_to_ascii)) {
            char c = scancode_to_ascii[scancode];
            if (c != 0) {
                // Add to buffer
                size_t next = (key_write_pos + 1) % KEY_BUFFER_SIZE;
                if (next != key_read_pos) {
                    key_buffer[key_write_pos] = c;
                    key_write_pos = next;
                }
            }
        }
    }
}

__attribute__((section(".compobjs"))) static struct component keyboard_component = {
    .name = "keyboard",
    .init = keyboard_init,
    .tick = keyboard_tick
};

__attribute__((section(".comps"))) struct component *p_keyboard_component = &keyboard_component;
