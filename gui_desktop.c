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
#define COLOR_CURSOR 0x0F        // White

typedef struct {
    int active;
    int x, y;
    int width, height;
    char title[32];
    char content[512];
    int dragging;
    int drag_offset_x;
    int drag_offset_y;
} GUIWindow;

static GUIWindow windows[MAX_WINDOWS];
static int active_window = -1;
static int mouse_x = 160, mouse_y = 100;
static uint8_t last_mouse_buttons = 0;

extern void vga_setpixel(int x, int y, uint8_t color);
extern void vga_clear_screen(uint8_t color);
extern void vga_fill_rect(int x, int y, int w, int h, uint8_t color);
extern void vga_draw_rect(int x, int y, int w, int h, uint8_t color);
extern void vga_draw_string(int x, int y, const char *str, uint8_t color);
extern void mouse_get_position(int *x, int *y);
extern uint8_t mouse_get_buttons(void);

// Draw mouse cursor (larger, more visible)
static void draw_cursor(int x, int y) {
    // Larger arrow cursor (15x18) with border for visibility
    const int cursor_data[18] = {
        0b111100000000000,
        0b110110000000000,
        0b110011000000000,
        0b110001100000000,
        0b110000110000000,
        0b110000011000000,
        0b110000001100000,
        0b110000000110000,
        0b110000000011000,
        0b110000110001100,
        0b110001111000110,
        0b110011001100011,
        0b110110000110001,
        0b111100000011000,
        0b110000000001100,
        0b000000000000110,
        0b000000000000011,
        0b000000000000000,
    };
    
    for (int dy = 0; dy < 18 && y + dy < 200; dy++) {
        for (int dx = 0; dx < 15 && x + dx < 320; dx++) {
            if (cursor_data[dy] & (1 << (14 - dx))) {
                // Draw white pixel
                vga_setpixel(x + dx, y + dy, COLOR_CURSOR);
            }
        }
    }
    
    // Draw black border around cursor for visibility
    for (int dy = 0; dy < 18 && y + dy < 200; dy++) {
        for (int dx = 0; dx < 15 && x + dx < 320; dx++) {
            int is_cursor = cursor_data[dy] & (1 << (14 - dx));
            if (is_cursor) {
                // Check if edge pixel (has non-cursor neighbor)
                int is_edge = 0;
                if (dx == 0 || !(cursor_data[dy] & (1 << (15 - dx)))) is_edge = 1;
                if (dx == 14 || !(cursor_data[dy] & (1 << (13 - dx)))) is_edge = 1;
                if (dy == 0 || !(cursor_data[dy-1] & (1 << (14 - dx)))) is_edge = 1;
                if (dy == 17 || !(cursor_data[dy+1] & (1 << (14 - dx)))) is_edge = 1;
                
                if (is_edge) {
                    vga_setpixel(x + dx, y + dy, COLOR_BORDER);
                }
            }
        }
    }
}

// Draw taskbar
static void draw_taskbar(void) {
    vga_fill_rect(0, 200 - TASKBAR_HEIGHT, 320, TASKBAR_HEIGHT, COLOR_TASKBAR);
    vga_draw_string(4, 200 - TASKBAR_HEIGHT + 4, "OpenComp", COLOR_TITLEBAR_TEXT);
    
    // Draw window buttons in taskbar
    int btn_x = 80;
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].active) {
            vga_fill_rect(btn_x, 200 - TASKBAR_HEIGHT + 2, 40, 12, 
                         i == active_window ? COLOR_TITLEBAR : COLOR_BUTTON);
            vga_draw_rect(btn_x, 200 - TASKBAR_HEIGHT + 2, 40, 12, COLOR_BORDER);
            
            // Draw first few chars of title
            char short_title[6];
            for (int j = 0; j < 5 && windows[i].title[j]; j++) {
                short_title[j] = windows[i].title[j];
            }
            short_title[5] = 0;
            vga_draw_string(btn_x + 2, 200 - TASKBAR_HEIGHT + 4, short_title, COLOR_TEXT);
            
            btn_x += 44;
        }
    }
}

// Draw a window
static void draw_window(int idx) {
    GUIWindow *w = &windows[idx];
    if (!w->active) return;
    
    int available_height = 200 - TASKBAR_HEIGHT;
    if (w->y + w->height > available_height) {
        w->y = available_height - w->height;
    }
    if (w->y < 0) w->y = 0;
    if (w->x < 0) w->x = 0;
    if (w->x + w->width > 320) w->x = 320 - w->width;
    
    // Draw title bar
    uint8_t titlebar_color = (idx == active_window) ? COLOR_TITLEBAR : COLOR_BUTTON;
    vga_fill_rect(w->x, w->y, w->width, TITLEBAR_HEIGHT, titlebar_color);
    vga_draw_rect(w->x, w->y, w->width, TITLEBAR_HEIGHT, COLOR_BORDER);
    
    // Draw title text
    vga_draw_string(w->x + 4, w->y + 2, w->title, COLOR_TITLEBAR_TEXT);
    
    // Draw close button
    int close_x = w->x + w->width - 12;
    vga_fill_rect(close_x, w->y + 2, 10, 8, COLOR_BUTTON);
    vga_draw_rect(close_x, w->y + 2, 10, 8, COLOR_BORDER);
    vga_draw_string(close_x + 2, w->y + 2, "X", COLOR_TEXT);
    
    // Draw window content area
    vga_fill_rect(w->x, w->y + TITLEBAR_HEIGHT, w->width, 
                  w->height - TITLEBAR_HEIGHT, COLOR_WINDOW_BG);
    vga_draw_rect(w->x, w->y + TITLEBAR_HEIGHT, w->width, 
                  w->height - TITLEBAR_HEIGHT, COLOR_BORDER);
    
    // Draw content text
    int cx = w->x + 4;
    int cy = w->y + TITLEBAR_HEIGHT + 4;
    for (int i = 0; w->content[i] && cy < w->y + w->height - 8; i++) {
        if (w->content[i] == '\n') {
            cy += 10;
            cx = w->x + 4;
        } else {
            if (cx + 8 < w->x + w->width - 4) {
                vga_draw_char(cx, cy, w->content[i], COLOR_TEXT);
                cx += 8;
            }
        }
    }
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
            windows[i].dragging = 0;
            
            // Copy title
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

// Check if point is in rectangle
static int point_in_rect(int px, int py, int x, int y, int w, int h) {
    return px >= x && px < x + w && py >= y && py < y + h;
}

// Handle mouse input
static void handle_mouse(void) {
    mouse_get_position(&mouse_x, &mouse_y);
    uint8_t buttons = mouse_get_buttons();
    int left_click = (buttons & 0x01) && !(last_mouse_buttons & 0x01);
    int left_held = buttons & 0x01;
    int left_released = !(buttons & 0x01) && (last_mouse_buttons & 0x01);
    
    // Handle window dragging
    if (active_window >= 0 && windows[active_window].dragging) {
        if (left_held) {
            windows[active_window].x = mouse_x - windows[active_window].drag_offset_x;
            windows[active_window].y = mouse_y - windows[active_window].drag_offset_y;
        } else {
            windows[active_window].dragging = 0;
        }
    }
    
    // Handle clicks
    if (left_click) {
        // Check windows from top to bottom
        for (int i = MAX_WINDOWS - 1; i >= 0; i--) {
            if (!windows[i].active) continue;
            
            GUIWindow *w = &windows[i];
            
            // Check close button
            int close_x = w->x + w->width - 12;
            if (point_in_rect(mouse_x, mouse_y, close_x, w->y + 2, 10, 8)) {
                w->active = 0;
                if (active_window == i) active_window = -1;
                break;
            }
            
            // Check title bar for dragging
            if (point_in_rect(mouse_x, mouse_y, w->x, w->y, w->width, TITLEBAR_HEIGHT)) {
                active_window = i;
                w->dragging = 1;
                w->drag_offset_x = mouse_x - w->x;
                w->drag_offset_y = mouse_y - w->y;
                break;
            }
            
            // Check window body
            if (point_in_rect(mouse_x, mouse_y, w->x, w->y, w->width, w->height)) {
                active_window = i;
                break;
            }
        }
    }
    
    last_mouse_buttons = buttons;
}

// Redraw everything
static void redraw_desktop(void) {
    // Clear background
    vga_clear_screen(COLOR_DESKTOP_BG);
    
    // Draw windows (back to front)
    for (int i = 0; i < MAX_WINDOWS; i++) {
        if (windows[i].active && i != active_window) {
            draw_window(i);
        }
    }
    
    // Draw active window on top
    if (active_window >= 0) {
        draw_window(active_window);
    }
    
    // Draw taskbar
    draw_taskbar();
    
    // Draw cursor last
    draw_cursor(mouse_x, mouse_y);
}

static void gui_desktop_init(void) {
    // Initialize windows
    for (int i = 0; i < MAX_WINDOWS; i++) {
        windows[i].active = 0;
    }
    
    // Create welcome window
    int win = create_window("Welcome", 60, 40, 200, 100);
    if (win >= 0) {
        set_window_content(win, 
            "OpenComp GUI Desktop\n\n"
            "Drag windows by title\n"
            "Click X to close\n\n"
            "Right click for menu");
    }
    
    // Create info window
    win = create_window("System", 20, 80, 160, 80);
    if (win >= 0) {
        set_window_content(win,
            "Graphics: 320x200\n"
            "Mode: VGA 13h\n"
            "Mouse: PS/2\n\n"
            "Components loaded!");
    }
    
    puts("[gui_desktop] Graphical desktop initialized\n");
}

static int tick_counter = 0;

static void gui_desktop_tick(void) {
    handle_mouse();
    
    // Redraw at 20 FPS
    if ((tick_counter++ % 5) == 0) {
        redraw_desktop();
    }
}

__attribute__((section(".compobjs"))) static struct component gui_desktop_component = {
    .name = "gui_desktop",
    .init = gui_desktop_init,
    .tick = gui_desktop_tick
};

__attribute__((section(".comps"))) struct component *p_gui_desktop_component = &gui_desktop_component;
