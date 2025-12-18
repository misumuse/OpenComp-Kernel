/* desktop.c
 *
 * Simple text-mode desktop environment
 * Copyright (C) 2025 B."Nova" J.
 * Licensed under GNU GPLv2
 *
 * Provides a simple window manager and desktop interface using VGA text mode
 */

#include <stdint.h>
#include "kernel.h"

#define MAX_WINDOWS 8
#define WIN_MIN_WIDTH 20
#define WIN_MIN_HEIGHT 5

typedef struct {
    int active;
    int x, y;
    int width, height;
    char title[32];
    char content[256];
    uint8_t color;
} Window;

static Window windows[MAX_WINDOWS];
static int active_window = -1;
static int cursor_x = 0, cursor_y = 0;
static char command_buffer[64];
static size_t command_len = 0;

extern int keyboard_has_key(void);
extern char keyboard_get_key(void);
extern uint64_t get_free_pages(void);

static void draw_box(int x, int y, int w, int h, uint8_t color) {
    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            int screen_x = x + col;
            int screen_y = y + row;
            if (screen_x >= 0 && screen_x < 80 && screen_y >= 0 && screen_y < 25) {
                char c = ' ';
                // Draw borders
                if (row == 0 || row == h - 1) {
                    if (col == 0) c = '+';
                    else if (col == w - 1) c = '+';
                    else c = '-';
                } else if (col == 0 || col == w - 1) {
                    c = '|';
                }
                vga_putchar_at(screen_x, screen_y, c, color);
            }
        }
    }
}

static void draw_window(int idx) {
    Window *w = &windows[idx];
    if (!w->active) return;
    
    uint8_t color = (idx == active_window) ? 0x1F : 0x17; // Bright if active
    draw_box(w->x, w->y, w->width, w->height, color);
    
    // Draw title
    int title_len = 0;
    while (w->title[title_len] && title_len < 30) title_len++;
    int title_x = w->x + (w->width - title_len) / 2;
    for (int i = 0; i < title_len; i++) {
        vga_putchar_at(title_x + i, w->y, w->title[i], color);
    }
    
    // Draw content
    int content_idx = 0;
    for (int row = 1; row < w->height - 1; row++) {
        for (int col = 1; col < w->width - 1; col++) {
            char c = w->content[content_idx];
            if (c == 0) break;
            if (c == '\n') {
                content_idx++;
                break;
            }
            vga_putchar_at(w->x + col, w->y + row, c, color);
            content_idx++;
        }
        if (w->content[content_idx] == 0) break;
    }
}

static void draw_desktop(void) {
    // Clear screen
    vga_clear(0x01); // Blue background
    
    // Draw title bar
    const char *title = " OpenComp Desktop Environment ";
    int title_x = (80 - 31) / 2;
    for (int i = 0; i < 80; i++) {
        char c = ' ';
        if (i >= title_x && i < title_x + 31) {
            c = title[i - title_x];
        }
        vga_putchar_at(i, 0, c, 0x70); // White on black
    }
    
    // Draw all windows
    for (int i = 0; i < MAX_WINDOWS; i++) {
        draw_window(i);
    }
    
    // Draw command line at bottom
    const char *prompt = "CMD> ";
    for (int i = 0; i < 5; i++) {
        vga_putchar_at(i, 24, prompt[i], 0x0F);
    }
    for (size_t i = 0; i < command_len; i++) {
        vga_putchar_at(5 + i, 24, command_buffer[i], 0x0F);
    }
    // Draw cursor
    vga_putchar_at(5 + command_len, 24, '_', 0x0F);
}

static int create_window(const char *title, int x, int y, int w, int h) {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (!windows[i].active) {
            windows[i].active = 1;
            windows[i].x = x;
            windows[i].y = y;
            windows[i].width = w;
            windows[i].height = h;
            
            // Copy title
            int j = 0;
            while (title[j] && j < 31) {
                windows[i].title[j] = title[j];
                j++;
            }
            windows[i].title[j] = 0;
            
            // Clear content
            windows[i].content[0] = 0;
            
            if (active_window == -1) active_window = i;
            return i;
        }
    }
    return -1;
}

static void set_window_content(int idx, const char *content) {
    if (idx < 0 || idx >= MAX_WINDOWS || !windows[idx].active) return;
    
    int i = 0;
    while (content[i] && i < 255) {
        windows[i].content[i] = content[i];
        i++;
    }
    windows[idx].content[i] = 0;
}

static void handle_command(void) {
    if (command_len == 0) return;
    
    command_buffer[command_len] = 0;
    
    // Parse command
    if (command_buffer[0] == 'h' && command_buffer[1] == 'e' && 
        command_buffer[2] == 'l' && command_buffer[3] == 'p') {
        int win = create_window("Help", 10, 5, 60, 15);
        if (win >= 0) {
            set_window_content(win, 
                "Commands:\n"
                "help - Show this help\n"
                "about - About OpenComp\n"
                "mem - Memory info\n"
                "time - Show uptime\n"
                "clear - Close all windows");
        }
    } else if (command_buffer[0] == 'a' && command_buffer[1] == 'b' &&
               command_buffer[2] == 'o' && command_buffer[3] == 'u' && 
               command_buffer[4] == 't') {
        int win = create_window("About", 15, 8, 50, 10);
        if (win >= 0) {
            set_window_content(win,
                "OpenComp Kernel v0.1\n"
                "A component-based OS\n"
                "Licensed under GPLv2\n"
                "Copyright 2025 B.Nova J.");
        }
    } else if (command_buffer[0] == 'm' && command_buffer[1] == 'e' && 
               command_buffer[2] == 'm') {
        int win = create_window("Memory", 20, 10, 40, 8);
        if (win >= 0) {
            char buf[128];
            char num[32];
            buf[0] = 0;
            str_append(buf, "Free pages: ");
            itoa_u(get_free_pages(), num);
            str_append(buf, num);
            str_append(buf, "\nFree memory: ");
            itoa_u(get_free_pages() * 4, num);
            str_append(buf, num);
            str_append(buf, " KB");
            set_window_content(win, buf);
        }
    } else if (command_buffer[0] == 'c' && command_buffer[1] == 'l' &&
               command_buffer[2] == 'e' && command_buffer[3] == 'a' &&
               command_buffer[4] == 'r') {
        // Close all windows
        for (int i = 0; i < MAX_WINDOWS; i++) {
            windows[i].active = 0;
        }
        active_window = -1;
    }
    
    command_len = 0;
}

static void desktop_init(void) {
    // Initialize windows
    for (int i = 0; i < MAX_WINDOWS; i++) {
        windows[i].active = 0;
    }
    
    // Create welcome window
    int win = create_window("Welcome to OpenComp", 15, 6, 50, 12);
    if (win >= 0) {
        set_window_content(win,
            "Welcome to OpenComp!\n\n"
            "Type 'help' for commands.\n\n"
            "This is a simple text-mode\n"
            "desktop environment.\n");
    }
    
    puts("[desktop] Desktop environment initialized\n");
}

static int tick_counter = 0;

static void desktop_tick(void) {
    // Redraw every few ticks
    if ((tick_counter++ % 10) == 0) {
        draw_desktop();
    }
    
    // Handle keyboard input
    if (keyboard_has_key()) {
        char c = keyboard_get_key();
        
        if (c == '\n') {
            handle_command();
        } else if (c == '\b') {
            if (command_len > 0) command_len--;
        } else if (command_len < 63) {
            command_buffer[command_len++] = c;
        }
    }
}

__attribute__((section(".compobjs"))) static struct component desktop_component = {
    .name = "desktop",
    .init = desktop_init,
    .tick = desktop_tick
};

__attribute__((section(".comps"))) struct component *p_desktop_component = &desktop_component;
