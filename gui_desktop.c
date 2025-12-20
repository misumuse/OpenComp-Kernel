/* gui_desktop.c
 *
 * Graphical desktop environment with window manager
 * Copyright (C) 2025 B."Nova" J.
 * Licensed under GNU GPLv2
 */

#include <stdint.h>
#include "kernel.h"

#define MAX_WINDOWS 8
#define TASKBAR_HEIGHT 16
#define TITLEBAR_HEIGHT 12

// Color palette (VGA 256 colors)
#define COLOR_DESKTOP_BG 0x01    // Dark blue
#define COLOR_TASKBAR 0x08       // Dark gray
#define COLOR_WINDOW_BG 0x07     // Light gray
#define COLOR_TITLEBAR 0x09      // Light blue
#define COLOR_TITLEBAR_TEXT 0x0F // White
#define COLOR_BORDER 0x00        // Black
#define COLOR_BUTTON 0x07        // Light gray
#define COLOR_TEXT 0x00          // Black

typedef struct {
    int active;
    int x, y;
    int width, height;
    char title[32];
    char content[512];
} GUIWindow;

static GUIWindow windows[MAX_WINDOWS];
static int active_window = -1;
static int tick_counter = 0;
static int needs_redraw = 1;

extern void vga_clear_screen(uint8_t color);
extern void vga_fill_rect(int x, int y, int w, int h, uint8_t color);
extern void vga_draw_rect(int x, int y, int w, int h, uint8_t color);
extern void vga_draw_string(int x, int y, const char *str, uint8_t color);
extern void vga_draw_char(int x, int y, char c, uint8_t color);

// Helper function to append string safely
static void safe_append(char *dest, const char *src, int max) {
    int len = 0;
    while (dest[len] && len < max - 1) len++;
    int i = 0;
    while (src[i] && len < max - 1) {
        dest[len++] = src[i++];
    }
    dest[len] = 0;
}

// Draw a box
static void draw_box(int x, int y, int w, int h, uint8_t color) {
    vga_fill_rect(x, y, w, h, color);
    vga_draw_rect(x, y, w, h, COLOR_BORDER);
}

// Create a new window
static int create_window(const char *title, int x, int y, int w, int h) {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (!windows[i].active) {
            windows[i].active = 1;
            windows[i].x = x;
            windows[i].y = y;
            windows[i].width = w;
            windows[i].height = h;
            
            int j = 0;
            while (title[j] && j < 31) {
                windows[i].title[j] = title[j];
                j++;
            }
            windows[i].title[j] = 0;
            windows[i].content[0] = 0;
            
            active_window = i;
            return i;
        }
    }
    return -1;
}

// Set window content
static void set_window_content(int idx, const char *content) {
    if (idx < 0 || idx >= MAX_WINDOWS || !windows[idx].active) return;
    
    int i = 0;
    while (content[i] && i < 511) {
        windows[idx].content[i] = content[i];
        i++;
    }
    windows[idx].content[i] = 0;
}

// Draw a window
static void draw_window(int idx) {
    GUIWindow *w = &windows[idx];
    if (!w->active) return;
    
    // Clamp position
    if (w->y < 0) w->y = 0;
    if (w->x < 0) w->x = 0;
    if (w->x + w->width > 320) w->x = 320 - w->width;
    if (w->y + w->height > 200 - TASKBAR_HEIGHT) 
        w->y = 200 - TASKBAR_HEIGHT - w->height;
    
    // Draw title bar
    uint8_t color = (idx == active_window) ? COLOR_TITLEBAR : COLOR_BUTTON;
    vga_fill_rect(w->x, w->y, w->width, TITLEBAR_HEIGHT, color);
    vga_draw_rect(w->x, w->y, w->width, TITLEBAR_HEIGHT, COLOR_BORDER);
    
    // Draw title
    int tx = w->x + 4;
    for (int i = 0; w->title[i] && tx + (i * 8) < w->x + w->width - 16; i++) {
        vga_draw_char(tx + (i * 8), w->y + 2, w->title[i], COLOR_TITLEBAR_TEXT);
    }
    
    // Draw close button
    int close_x = w->x + w->width - 12;
    vga_fill_rect(close_x, w->y + 2, 10, 8, COLOR_BUTTON);
    vga_draw_rect(close_x, w->y + 2, 10, 8, COLOR_BORDER);
    vga_draw_char(close_x + 1, w->y + 2, 'X', COLOR_TEXT);
    
    // Draw content area
    vga_fill_rect(w->x, w->y + TITLEBAR_HEIGHT, w->width, 
                  w->height - TITLEBAR_HEIGHT, COLOR_WINDOW_BG);
    vga_draw_rect(w->x, w->y + TITLEBAR_HEIGHT, w->width, 
                  w->height - TITLEBAR_HEIGHT, COLOR_BORDER);
    
    // Draw content
    int content_x = w->x + 4;
    int content_y = w->y + TITLEBAR_HEIGHT + 4;
    int max_chars = (w->width - 8) / 8;
    
    for (int i = 0; w->content[i] && content_y < w->y + w->height - 8; i++) {
        if (w->content[i] == '\n') {
            content_y += 10;
            content_x = w->x + 4;
        } else {
            int pos = (content_x - (w->x + 4)) / 8;
            if (pos < max_chars) {
                vga_draw_char(content_x, content_y, w->content[i], COLOR_TEXT);
                content_x += 8;
            } else {
                content_y += 10;
                content_x = w->x + 4;
                if (content_y < w->y + w->height - 8) {
                    vga_draw_char(content_x, content_y, w->content[i], COLOR_TEXT);
                    content_x += 8;
                }
            }
        }
    }
}

// Draw taskbar
static void draw_taskbar(void) {
    vga_fill_rect(0, 200 - TASKBAR_HEIGHT, 320, TASKBAR_HEIGHT, COLOR_TASKBAR);
    vga_draw_string(4, 200 - TASKBAR_HEIGHT + 4, "OpenComp", COLOR_TITLEBAR_TEXT);
    vga_draw_string(175, 200 - TASKBAR_HEIGHT + 4, "E:Menu X:Close", COLOR_TITLEBAR_TEXT);
    
    if (active_window >= 0) {
        char info[16] = "Win:";
        char num[8];
        itoa_u(active_window + 1, num);
        safe_append(info, num, 16);
        vga_draw_string(65, 200 - TASKBAR_HEIGHT + 4, info, COLOR_TITLEBAR_TEXT);
    }
}

// Redraw everything
static void redraw_desktop(void) {
    vga_clear_screen(COLOR_DESKTOP_BG);
    
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].active && i != active_window) {
            draw_window(i);
        }
    }
    
    if (active_window >= 0) {
        draw_window(active_window);
    }
    
    draw_taskbar();
}

// Handle keyboard
static void handle_keyboard(void) {
    if (!keyboard_has_key()) return;
    
    char key = keyboard_get_key();
    
    // Tab - switch windows
    if (key == '\t') {
        int start = active_window;
        do {
            active_window++;
            if (active_window >= MAX_WINDOWS) active_window = 0;
            if (windows[active_window].active) break;
        } while (active_window != start);
        needs_redraw = 1;
        return;
    }
    
    // X - close window
    if (key == 'x' || key == 'X') {
        if (active_window >= 0) {
            windows[active_window].active = 0;
            active_window = -1;
            for (int i = 0; i < MAX_WINDOWS; i++) {
                if (windows[i].active) {
                    active_window = i;
                    break;
                }
            }
        }
        needs_redraw = 1;
        return;
    }
    
    if (active_window < 0) return;
    GUIWindow *w = &windows[active_window];
    
    // WASD - move window
    if (key == 'w' || key == 'W') {
        w->y -= 5;
        if (w->y < 0) w->y = 0;
        needs_redraw = 1;
    } else if (key == 's' || key == 'S') {
        w->y += 5;
        needs_redraw = 1;
    } else if (key == 'a' || key == 'A') {
        w->x -= 5;
        if (w->x < 0) w->x = 0;
        needs_redraw = 1;
    } else if (key == 'd' || key == 'D') {
        w->x += 5;
        needs_redraw = 1;
    }
    // E - Start Menu
    else if (key == 'e' || key == 'E') {
        int win = create_window("Start Menu", 10, 140, 140, 90);
        if (win >= 0) {
            set_window_content(win,
                "Applications:\n\n"
                "H - Help\n"
                "M - Memory\n"
                "F - Files\n"
                "C - Calculator\n\n"
                "Press key to open");
        }
        needs_redraw = 1;
    }
    // Space - commands
    else if (key == ' ') {
        int win = create_window("Commands", 80, 60, 160, 100);
        if (win >= 0) {
            set_window_content(win,
                "Keys:\n\n"
                "Tab - Switch\n"
                "X - Close\n"
                "WASD - Move\n"
                "E - Menu\n"
                "H - Help\n"
                "M - Memory\n"
                "F - Files");
        }
        needs_redraw = 1;
    }
    // H - Help
    else if (key == 'h' || key == 'H') {
        int win = create_window("Help", 40, 30, 240, 100);
        if (win >= 0) {
            set_window_content(win,
                "OpenComp Help\n\n"
                "Tab switches windows\n"
                "WASD moves windows\n"
                "X closes windows\n"
                "E opens menu\n\n"
                "Press F for files");
        }
        needs_redraw = 1;
    }
    // M - Memory
    else if (key == 'm' || key == 'M') {
        int win = create_window("Memory", 60, 50, 200, 70);
        if (win >= 0) {
            char buf[256] = "Memory:\n\nFree: ";
            char num[32];
            itoa_u(get_free_pages() * 4, num);
            safe_append(buf, num, 256);
            safe_append(buf, " KB\nUsed: ", 256);
            itoa_u((4096 - get_free_pages()) * 4, num);
            safe_append(buf, num, 256);
            safe_append(buf, " KB", 256);
            set_window_content(win, buf);
        }
        needs_redraw = 1;
    }
    // F - File browser
    else if (key == 'f' || key == 'F') {
        int win = create_window("Files", 30, 20, 260, 140);
        if (win >= 0) {
            char buf[512] = "File Browser\n\n";
            char num[16];
            int count = fs_get_file_count();
            itoa_u(count, num);
            safe_append(buf, "Files: ", 512);
            safe_append(buf, num, 512);
            safe_append(buf, "\n", 512);
            safe_append(buf, "Press 1-8 to open\n\n", 512);
            
            for (int i = 0; i < count && i < 8; i++) {
                char name[128];
                uint32_t size;
                int is_dir;
                
                if (fs_get_file_info(i, name, &size, &is_dir)) {
                    // Add number
                    char idx[8];
                    itoa_u(i + 1, idx);
                    safe_append(buf, idx, 512);
                    safe_append(buf, ". ", 512);
                    
                    safe_append(buf, is_dir ? "[DIR] " : "[   ] ", 512);
                    
                    char short_name[25];
                    int j = 0;
                    while (name[j] && j < 24) {
                        short_name[j] = name[j];
                        j++;
                    }
                    short_name[j] = 0;
                    
                    safe_append(buf, short_name, 512);
                    safe_append(buf, "\n", 512);
                }
            }
            
            if (count > 8) {
                safe_append(buf, "\n...more...", 512);
            }
            
            set_window_content(win, buf);
        }
        needs_redraw = 1;
    }
    // 1-8 keys - open file by number
    else if (key >= '1' && key <= '8') {
        int file_idx = key - '1';
        int count = fs_get_file_count();
        
        if (file_idx < count) {
            char name[128];
            uint32_t size;
            int is_dir;
            
            if (fs_get_file_info(file_idx, name, &size, &is_dir)) {
                if (!is_dir) {
                    // Open file viewer window
                    int win = create_window(name, 20, 15, 280, 160);
                    if (win >= 0) {
                        uint8_t *data;
                        uint32_t fsize;
                        
                        if (fs_read_file_by_index(file_idx, &data, &fsize)) {
                            char content[512];
                            int i = 0;
                            // Copy file contents (limit to 500 chars)
                            while (i < fsize && i < 500 && data[i]) {
                                content[i] = data[i];
                                i++;
                            }
                            content[i] = 0;
                            
                            // Add truncation notice if needed
                            if (fsize > 500) {
                                safe_append(content, "\n\n...truncated...", 512);
                            }
                            
                            set_window_content(win, content);
                        } else {
                            set_window_content(win, "Error: Could not read file");
                        }
                    }
                } else {
                    // It's a directory
                    int win = create_window(name, 60, 50, 200, 80);
                    if (win >= 0) {
                        set_window_content(win, 
                            "Directory\n\n"
                            "Directory browsing\n"
                            "not yet implemented.");
                    }
                }
            }
        }
        needs_redraw = 1;
    }
    // C - Calculator
    else if (key == 'c' || key == 'C') {
        int win = create_window("Calculator", 100, 40, 120, 90);
        if (win >= 0) {
            set_window_content(win,
                "Calculator\n\n"
                "Coming soon!\n\n"
                "Will support:\n"
                "+ - * /");
        }
        needs_redraw = 1;
    }
}

// Init
static void gui_desktop_init(void) {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        windows[i].active = 0;
    }
    
    int win = create_window("Welcome", 15, 6, 200, 100);
    if (win >= 0) {
        set_window_content(win,
            "OpenComp Desktop\n\n"
            "Press E for menu\n"
            "Press H for help\n\n"
            "WASD moves windows");
    }
    
    win = create_window("System", 20, 80, 160, 80);
    if (win >= 0) {
        set_window_content(win,
            "Graphics: 320x200\n"
            "Mode: VGA 13h\n"
            "Keyboard: PS/2\n\n"
            "Press Tab!");
    }
    
    needs_redraw = 1;
    puts("[gui_desktop] GUI initialized\n");
}

// Tick
static void gui_desktop_tick(void) {
    handle_keyboard();
    
    if (needs_redraw) {
        redraw_desktop();
        needs_redraw = 0;
    }
    
    tick_counter++;
}

__attribute__((section(".compobjs"))) static struct component gui_desktop_component = {
    .name = "gui_desktop",
    .init = gui_desktop_init,
    .tick = gui_desktop_tick
};

__attribute__((section(".comps"))) struct component *p_gui_desktop_component = &gui_desktop_component;
