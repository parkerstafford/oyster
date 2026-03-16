#ifndef GUI_H
#define GUI_H

#include <stdint.h>
#include <stdbool.h>
#include "limine.h"

#define COLOR_DESKTOP     0x002d4a6e
#define COLOR_WINDOW_BG   0x00e8e8e8
#define COLOR_TITLEBAR    0x004a90d9
#define COLOR_TITLEBAR_INACTIVE 0x007a7a7a
#define COLOR_TITLE_TEXT  0x00ffffff
#define COLOR_BUTTON      0x00d0d0d0
#define COLOR_BUTTON_HOVER 0x00c0c0c0
#define COLOR_BUTTON_TEXT 0x00000000
#define COLOR_BORDER      0x00505050
#define COLOR_TASKBAR     0x00303030
#define COLOR_CURSOR      0x00ffffff

struct window {
    int32_t x, y;
    int32_t width, height;
    char title[64];
    bool active;
    bool visible;
    bool dragging;
    int32_t drag_offset_x, drag_offset_y;
    void (*draw_content)(struct window *win);
    void (*on_click)(struct window *win, int32_t x, int32_t y);
};

struct button {
    int32_t x, y;
    int32_t width, height;
    char label[32];
    bool hovered;
    void (*on_click)(void);
};

void gui_init(struct limine_framebuffer *fb);
void gui_draw_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);
void gui_draw_rect_outline(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);
void gui_draw_text(int32_t x, int32_t y, const char *text, uint32_t color);
void gui_draw_cursor(int32_t x, int32_t y);
void gui_draw_desktop(void);
void gui_draw_taskbar(void);
void gui_draw_window(struct window *win);
void gui_draw_button(struct button *btn);

struct window *gui_create_window(int32_t x, int32_t y, int32_t w, int32_t h, const char *title);
void gui_destroy_window(struct window *win);

void gui_run(void);

#endif
