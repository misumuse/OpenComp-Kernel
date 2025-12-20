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
    
    // Draw content - properly handle each character
    int content_x = w->x + 2;
    int content_y = w->y + 2;
    int max_width = w->width - 4;
    
    for (int i = 0; w->content[i] && content_y < w->y + w->height - 1; i++) {
        if (w->content[i] == '\n') {
            content_y++;
            content_x = w->x + 2;
        } else {
            if (content_x < w->x + max_width) {
                vga_putchar_at(content_x, content_y, w->content[i], color);
                content_x++;
            }
        }
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
        windows[idx].content[i] = content[i];
        i++;
    }
    windows[idx].content[i] = 0;
}

static int uptime_ticks = 0;

static void handle_command(void) {
    if (command_len == 0) return;
    
    command_buffer[command_len] = 0;
    
    // Parse command
    if (command_buffer[0] == 'h' && command_buffer[1] == 'e' && 
        command_buffer[2] == 'l' && command_buffer[3] == 'p') {
        int win = create_window("Help", 10, 5, 60, 16);
        if (win >= 0) {
            set_window_content(win, 
                "Available Commands:\n\n"
                "help   - Show this help\n"
                "about  - About OpenComp\n"
                "mem    - Memory info\n"
                "time   - Show uptime\n"
                "colors - Color test\n"
                "info   - System info\n"
                "echo   - Echo test\n"
                "clear  - Close all windows");
        }
    } else if (command_buffer[0] == 'a' && command_buffer[1] == 'b' &&
               command_buffer[2] == 'o' && command_buffer[3] == 'u' && 
               command_buffer[4] == 't') {
        int win = create_window("About OpenComp", 15, 8, 50, 10);
        if (win >= 0) {
            set_window_content(win,
                "OpenComp Kernel v0.1\n"
                "A component-based OS\n\n"
                "Licensed under GPLv2\n"
                "Copyright 2025 B.Nova J.\n\n"
                "github.com/misumuse");
        }
    } else if (command_buffer[0] == 'm' && command_buffer[1] == 'e' && 
               command_buffer[2] == 'm') {
        int win = create_window("Memory Status", 20, 10, 40, 9);
        if (win >= 0) {
            char buf[128];
            char num[32];
            buf[0] = 0;
            str_append(buf, "Free pages: ");
            itoa_u(get_free_pages(), num);
            str_append(buf, num);
            str_append(buf, "\n\nFree memory: ");
            itoa_u(get_free_pages() * 4, num);
            str_append(buf, num);
            str_append(buf, " KB\n\nUsed: ");
            itoa_u((4096 - get_free_pages()) * 4, num);
            str_append(buf, num);
            str_append(buf, " KB");
            set_window_content(win, buf);
        }
    } else if (command_buffer[0] == 't' && command_buffer[1] == 'i' &&
               command_buffer[2] == 'm' && command_buffer[3] == 'e') {
        int win = create_window("System Uptime", 25, 12, 35, 8);
        if (win >= 0) {
            char buf[128];
            char num[32];
            buf[0] = 0;
            str_append(buf, "Uptime:\n\n");
            itoa_u(uptime_ticks / 100, num);
            str_append(buf, num);
            str_append(buf, " seconds\n\n");
            itoa_u(uptime_ticks, num);
            str_append(buf, num);
            str_append(buf, " ticks");
            set_window_content(win, buf);
        }
    } else if (command_buffer[0] == 'c' && command_buffer[1] == 'o' &&
               command_buffer[2] == 'l' && command_buffer[3] == 'o' &&
               command_buffer[4] == 'r' && command_buffer[5] == 's') {
        int win = create_window("Color Test", 5, 4, 70, 18);
        if (win >= 0) {
            set_window_content(win,
                "VGA Text Mode Colors:\n\n"
                "Black, Blue, Green, Cyan\n"
                "Red, Magenta, Brown, Gray\n\n"
                "Light versions available\n"
                "with high intensity bit\n\n"
                "16 colors total\n"
                "80x25 resolution");
        }
    } else if (command_buffer[0] == 'i' && command_buffer[1] == 'n' &&
               command_buffer[2] == 'f' && command_buffer[3] == 'o') {
        int win = create_window("System Information", 12, 6, 55, 13);
        if (win >= 0) {
            set_window_content(win,
                "OpenComp Kernel\n\n"
                "Architecture: x86 (32-bit)\n"
                "Boot: Multiboot2/GRUB\n"
                "Display: VGA Text 80x25\n"
                "Memory: 16MB managed\n"
                "Components: 3 active\n\n"
                "Keyboard: PS/2 driver\n"
                "Desktop: Active");
        }
    } else if (command_buffer[0] == 'e' && command_buffer[1] == 'c' &&
               command_buffer[2] == 'h' && command_buffer[3] == 'o') {
        int win = create_window("Echo", 18, 9, 45, 7);
        if (win >= 0) {
            char buf[128];
            buf[0] = 0;
            str_append(buf, "You typed:\n\n");
            for (size_t i = 5; i < command_len && i < 100; i++) {
                if (command_buffer[i] == ' ') continue;
                buf[i-3] = command_buffer[i];
                buf[i-2] = 0;
            }
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
    } else {
        // Unknown command
        int win = create_window("Error", 22, 11, 36, 6);
        if (win >= 0) {
            set_window_content(win,
                "Unknown command!\n\n"
                "Type 'help' for list");
        }
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
    // Increment uptime counter
    uptime_ticks++;
    
    // Only redraw every 50 ticks to reduce flicker
    if ((tick_counter++ % 50) == 0) {
        draw_desktop();
    }
    
    // Handle keyboard input every tick
    if (keyboard_has_key()) {
        char c = keyboard_get_key();
        
        if (c == '\n') {
            handle_command();
            draw_desktop(); // Redraw immediately after command
        } else if (c == '\b') {
            if (command_len > 0) command_len--;
            draw_desktop(); // Redraw immediately after backspace
        } else if (command_len < 63) {
            command_buffer[command_len++] = c;
            draw_desktop(); // Redraw immediately after character
        }
    }
}

__attribute__((section(".compobjs"))) static struct component desktop_component = {
    .name = "desktop",
    .init = desktop_init,
    .tick = desktop_tick
};

__attribute__((section(".comps"))) struct component *p_desktop_component = &desktop_component;
