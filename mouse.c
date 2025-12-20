/* mouse.c
 *
 * PS/2 Mouse driver component
 * Copyright (C) 2025 B."Nova" J.
 * Licensed under GNU GPLv2
 */

#include <stdint.h>
#include "kernel.h"

#define MOUSE_PORT 0x60
#define MOUSE_STATUS 0x64
#define MOUSE_ABIT 0x02
#define MOUSE_BBIT 0x01

static int mouse_x = 160;  // Center of 320x200
static int mouse_y = 100;
static uint8_t mouse_buttons = 0;
static uint8_t mouse_cycle = 0;
static uint8_t mouse_byte[3];

// Mouse smoothing
static float mouse_x_float = 160.0f;
static float mouse_y_float = 100.0f;
static float velocity_x = 0.0f;
static float velocity_y = 0.0f;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;
    if (type == 0) {
        while (timeout--) {
            if ((inb(MOUSE_STATUS) & MOUSE_BBIT) == 1) return;
        }
    } else {
        while (timeout--) {
            if ((inb(MOUSE_STATUS) & MOUSE_ABIT) == 0) return;
        }
    }
}

static void mouse_write(uint8_t data) {
    mouse_wait(1);
    outb(MOUSE_STATUS, 0xD4);
    mouse_wait(1);
    outb(MOUSE_PORT, data);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(MOUSE_PORT);
}

void mouse_get_position(int *x, int *y) {
    *x = mouse_x;
    *y = mouse_y;
}

uint8_t mouse_get_buttons(void) {
    return mouse_buttons;
}

static void mouse_init(void) {
    uint8_t status;
    
    // Enable auxiliary mouse device
    mouse_wait(1);
    outb(MOUSE_STATUS, 0xA8);
    
    // Enable interrupts
    mouse_wait(1);
    outb(MOUSE_STATUS, 0x20);
    mouse_wait(0);
    status = inb(MOUSE_PORT) | 2;
    mouse_wait(1);
    outb(MOUSE_STATUS, 0x60);
    mouse_wait(1);
    outb(MOUSE_PORT, status);
    
    // Use default settings
    mouse_write(0xF6);
    mouse_read();
    
    // Enable the mouse
    mouse_write(0xF4);
    mouse_read();
    
    // Reset cycle
    mouse_cycle = 0;
    
    puts("[mouse] PS/2 mouse driver initialized\n");
}

static void mouse_tick(void) {
    // Check if mouse has data
    uint8_t status = inb(MOUSE_STATUS);
    if ((status & MOUSE_BBIT) == 0) return;
    
    uint8_t data = inb(MOUSE_PORT);
    
    switch (mouse_cycle) {
        case 0:
            // First byte must have bit 3 set
            if ((data & 0x08) == 0) {
                // Invalid packet, stay at cycle 0
                break;
            }
            mouse_byte[0] = data;
            mouse_cycle = 1;
            break;
            
        case 1:
            mouse_byte[1] = data;
            mouse_cycle = 2;
            break;
            
        case 2:
            mouse_byte[2] = data;
            mouse_cycle = 0;
            
            // Process complete packet
            mouse_buttons = mouse_byte[0] & 0x07;
            
            // Extract movement (convert from uint8_t to signed int)
            int8_t dx = (int8_t)mouse_byte[1];
            int8_t dy = (int8_t)mouse_byte[2];
            
            // Check for overflow flags
            if (mouse_byte[0] & 0x40) dx = 0;  // X overflow
            if (mouse_byte[0] & 0x80) dy = 0;  // Y overflow
            
            // Apply acceleration curve
            float move_x = (float)dy;
            float move_y = (float)dx;
            
            // Acceleration: faster movement = more speed
            float speed = move_x * move_x + move_y * move_y;
            if (speed > 100.0f) {
                float mult = 1.5f;
                move_x *= mult;
                move_y *= mult;
            }
            
            // Smoothing: blend old velocity with new
            velocity_x = velocity_x * 0.3f + move_x * 0.7f;
            velocity_y = velocity_y * 0.3f + move_y * 0.7f;
            
            // Update float position
            mouse_x_float += velocity_x;
            mouse_y_float += velocity_y;
            
            // Clamp float position
            if (mouse_x_float < 0.0f) mouse_x_float = 0.0f;
            if (mouse_x_float > 319.0f) mouse_x_float = 319.0f;
            if (mouse_y_float < 0.0f) mouse_y_float = 0.0f;
            if (mouse_y_float > 199.0f) mouse_y_float = 199.0f;
            
            // Convert to integer
            mouse_x = (int)mouse_x_float;
            mouse_y = (int)mouse_y_float;
            
            break;
    }
}

__attribute__((section(".compobjs"))) static struct component mouse_component = {
    .name = "mouse",
    .init = mouse_init,
    .tick = mouse_tick
};

__attribute__((section(".comps"))) struct component *p_mouse_component = &mouse_component;
