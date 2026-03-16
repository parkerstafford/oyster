#include "gui.h"
#include "mouse.h"
#include "keyboard.h"
#include "timer.h"
#include "memory.h"
#include "framebuffer.h"
#include "io.h"
#include <stddef.h>

static struct limine_framebuffer *framebuffer;
static uint32_t *screen;
static uint32_t *backbuffer;
static int32_t screen_width;
static int32_t screen_height;
static size_t buffer_size;

#define MAX_WINDOWS 16
static struct window *windows[MAX_WINDOWS];
static int window_count = 0;
static struct window *active_window = NULL;

static const uint8_t cursor_bitmap[16][12] = {
    {1,0,0,0,0,0,0,0,0,0,0,0},
    {1,1,0,0,0,0,0,0,0,0,0,0},
    {1,2,1,0,0,0,0,0,0,0,0,0},
    {1,2,2,1,0,0,0,0,0,0,0,0},
    {1,2,2,2,1,0,0,0,0,0,0,0},
    {1,2,2,2,2,1,0,0,0,0,0,0},
    {1,2,2,2,2,2,1,0,0,0,0,0},
    {1,2,2,2,2,2,2,1,0,0,0,0},
    {1,2,2,2,2,2,2,2,1,0,0,0},
    {1,2,2,2,2,2,2,2,2,1,0,0},
    {1,2,2,2,2,2,1,1,1,1,1,0},
    {1,2,2,1,2,2,1,0,0,0,0,0},
    {1,2,1,0,1,2,2,1,0,0,0,0},
    {1,1,0,0,1,2,2,1,0,0,0,0},
    {0,0,0,0,0,1,2,2,1,0,0,0},
    {0,0,0,0,0,1,1,1,1,0,0,0},
};

extern const uint8_t font_8x16[128][16];

static inline void set_pixel(int32_t x, int32_t y, uint32_t color) {
    if (x >= 0 && x < screen_width && y >= 0 && y < screen_height) {
        backbuffer[y * screen_width + x] = color;
    }
}

static void swap_buffers(void) {
    uint32_t *src = backbuffer;
    uint32_t *dst = screen;
    size_t pixels = screen_width * screen_height;
    for (size_t i = 0; i < pixels; i++) {
        dst[i] = src[i];
    }
}

void gui_init(struct limine_framebuffer *fb) {
    framebuffer = fb;
    screen_width = fb->width;
    screen_height = fb->height;
    buffer_size = screen_width * screen_height * sizeof(uint32_t);
    
    mouse_set_bounds(screen_width, screen_height);
    
    screen = (uint32_t *)fb->address;
}

void gui_draw_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
    for (int32_t py = y; py < y + h; py++) {
        for (int32_t px = x; px < x + w; px++) {
            set_pixel(px, py, color);
        }
    }
}

void gui_draw_rect_outline(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
    for (int32_t px = x; px < x + w; px++) {
        set_pixel(px, y, color);
        set_pixel(px, y + h - 1, color);
    }
    for (int32_t py = y; py < y + h; py++) {
        set_pixel(x, py, color);
        set_pixel(x + w - 1, py, color);
    }
}

void gui_draw_text(int32_t x, int32_t y, const char *text, uint32_t color) {
    int32_t cx = x;
    while (*text) {
        unsigned char c = (unsigned char)*text;
        if (c > 127) c = '?';
        
        const uint8_t *glyph = font_8x16[c];
        for (int row = 0; row < 16; row++) {
            for (int col = 0; col < 8; col++) {
                if (glyph[row] & (0x80 >> col)) {
                    set_pixel(cx + col, y + row, color);
                }
            }
        }
        cx += 8;
        text++;
    }
}

void gui_draw_cursor(int32_t x, int32_t y) {
    for (int row = 0; row < 16; row++) {
        for (int col = 0; col < 12; col++) {
            uint8_t pixel = cursor_bitmap[row][col];
            if (pixel == 1) {
                set_pixel(x + col, y + row, 0x00000000);
            } else if (pixel == 2) {
                set_pixel(x + col, y + row, 0x00ffffff);
            }
        }
    }
}

void gui_draw_desktop(void) {
    gui_draw_rect(0, 0, screen_width, screen_height - 40, COLOR_DESKTOP);
}

void gui_draw_taskbar(void) {
    gui_draw_rect(0, screen_height - 40, screen_width, 40, COLOR_TASKBAR);
    
    gui_draw_rect(5, screen_height - 35, 80, 30, COLOR_BUTTON);
    gui_draw_text(15, screen_height - 30, "Oyster", COLOR_BUTTON_TEXT);
    
    uint64_t ticks = timer_get_ticks();
    uint64_t seconds = ticks / 100;
    uint64_t minutes = (seconds / 60) % 60;
    uint64_t hours = (seconds / 3600) % 24;
    
    char time_str[16];
    time_str[0] = '0' + (hours / 10);
    time_str[1] = '0' + (hours % 10);
    time_str[2] = ':';
    time_str[3] = '0' + (minutes / 10);
    time_str[4] = '0' + (minutes % 10);
    time_str[5] = '\0';
    
    gui_draw_text(screen_width - 50, screen_height - 28, time_str, 0x00ffffff);
}

void gui_draw_window(struct window *win) {
    if (!win->visible) return;
    
    gui_draw_rect(win->x, win->y, win->width, win->height, COLOR_WINDOW_BG);
    
    uint32_t title_color = win->active ? COLOR_TITLEBAR : COLOR_TITLEBAR_INACTIVE;
    gui_draw_rect(win->x, win->y, win->width, 30, title_color);
    
    gui_draw_text(win->x + 10, win->y + 7, win->title, COLOR_TITLE_TEXT);
    
    gui_draw_rect(win->x + win->width - 28, win->y + 5, 20, 20, 0x00e74c3c);
    gui_draw_text(win->x + win->width - 24, win->y + 7, "X", COLOR_TITLE_TEXT);
    
    gui_draw_rect_outline(win->x, win->y, win->width, win->height, COLOR_BORDER);
    
    if (win->draw_content) {
        win->draw_content(win);
    }
}

static int my_strlen(const char *s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

void gui_draw_button(struct button *btn) {
    uint32_t color = btn->hovered ? COLOR_BUTTON_HOVER : COLOR_BUTTON;
    gui_draw_rect(btn->x, btn->y, btn->width, btn->height, color);
    gui_draw_rect_outline(btn->x, btn->y, btn->width, btn->height, COLOR_BORDER);
    
    int text_x = btn->x + (btn->width - 8 * my_strlen(btn->label)) / 2;
    int text_y = btn->y + (btn->height - 16) / 2;
    gui_draw_text(text_x, text_y, btn->label, COLOR_BUTTON_TEXT);
}

struct window *gui_create_window(int32_t x, int32_t y, int32_t w, int32_t h, const char *title) {
    if (window_count >= MAX_WINDOWS) return NULL;
    
    struct window *win = kmalloc(sizeof(struct window));
    if (!win) return NULL;
    
    win->x = x;
    win->y = y;
    win->width = w;
    win->height = h;
    win->active = true;
    win->visible = true;
    win->dragging = false;
    win->draw_content = NULL;
    win->on_click = NULL;
    
    int i = 0;
    while (title[i] && i < 63) {
        win->title[i] = title[i];
        i++;
    }
    win->title[i] = '\0';
    
    if (active_window) active_window->active = false;
    active_window = win;
    
    windows[window_count++] = win;
    return win;
}

void gui_destroy_window(struct window *win) {
    for (int i = 0; i < window_count; i++) {
        if (windows[i] == win) {
            for (int j = i; j < window_count - 1; j++) {
                windows[j] = windows[j + 1];
            }
            window_count--;
            kfree(win);
            
            if (active_window == win) {
                active_window = window_count > 0 ? windows[window_count - 1] : NULL;
                if (active_window) active_window->active = true;
            }
            return;
        }
    }
}

static void draw_about_content(struct window *win) {
    gui_draw_text(win->x + 20, win->y + 50, "Oyster OS v0.1", 0x00000000);
    gui_draw_text(win->x + 20, win->y + 75, "A simple operating system", 0x00505050);
    gui_draw_text(win->x + 20, win->y + 100, "written in C.", 0x00505050);
    gui_draw_text(win->x + 20, win->y + 140, "Use arrow keys + Space!", 0x00008000);
}

static void draw_notepad_content(struct window *win) {
    gui_draw_rect(win->x + 10, win->y + 40, win->width - 20, win->height - 50, 0x00ffffff);
    gui_draw_rect_outline(win->x + 10, win->y + 40, win->width - 20, win->height - 50, 0x00808080);
    gui_draw_text(win->x + 15, win->y + 45, "Type here...", 0x00808080);
}

void gui_run(void) {
    static uint8_t heap[512 * 1024];
    static uint32_t static_backbuffer[1920 * 1200];
    
    memory_init(heap, sizeof(heap));
    backbuffer = static_backbuffer;
    
    struct window *about_win = gui_create_window(100, 100, 300, 200, "About Oyster");
    about_win->draw_content = draw_about_content;
    
    struct window *notepad_win = gui_create_window(450, 150, 350, 250, "Notepad");
    notepad_win->draw_content = draw_notepad_content;
    notepad_win->active = false;
    about_win->active = true;
    active_window = about_win;
    
    int32_t cursor_x = screen_width / 2;
    int32_t cursor_y = screen_height / 2;
    
    uint8_t mouse_cycle = 0;
    int8_t mouse_bytes[3] = {0};
    bool mouse_left = false;
    bool prev_mouse_left = false;
    bool extended = false;
    
    while (1) {
        gui_draw_desktop();
        
        for (int i = 0; i < window_count; i++) {
            gui_draw_window(windows[i]);
        }
        
        gui_draw_taskbar();
        
        /* Poll keyboard and mouse directly */
        for (int polls = 0; polls < 20; polls++) {
            uint8_t status = inb(0x64);
            if (!(status & 1)) break;
            
            uint8_t data = inb(0x60);
            
            if (status & 0x20) {
                /* Mouse data */
                switch (mouse_cycle) {
                    case 0:
                        if (data & 0x08) {
                            mouse_bytes[0] = data;
                            mouse_cycle = 1;
                        }
                        break;
                    case 1:
                        mouse_bytes[1] = data;
                        mouse_cycle = 2;
                        break;
                    case 2:
                        mouse_bytes[2] = data;
                        mouse_cycle = 0;
                        
                        prev_mouse_left = mouse_left;
                        mouse_left = mouse_bytes[0] & 0x01;
                        
                        int32_t dx = mouse_bytes[1];
                        int32_t dy = mouse_bytes[2];
                        if (mouse_bytes[0] & 0x10) dx |= (int32_t)0xFFFFFF00;
                        if (mouse_bytes[0] & 0x20) dy |= (int32_t)0xFFFFFF00;
                        
                        cursor_x += dx;
                        cursor_y -= dy;
                        break;
                }
            } else {
                /* Keyboard data */
                if (data == 0xE0) {
                    extended = true;
                } else if (extended) {
                    extended = false;
                    switch (data) {
                        case 0x48: cursor_y -= 6; break;
                        case 0x50: cursor_y += 6; break;
                        case 0x4B: cursor_x -= 6; break;
                        case 0x4D: cursor_x += 6; break;
                    }
                } else if (data == 0x39) {
                    /* Space pressed */
                    prev_mouse_left = mouse_left;
                    mouse_left = true;
                } else if (data == 0xB9) {
                    /* Space released */
                    mouse_left = false;
                } else if (data == 0x1C) {
                    /* Enter pressed */
                    prev_mouse_left = mouse_left;
                    mouse_left = true;
                } else if (data == 0x9C) {
                    /* Enter released */
                    mouse_left = false;
                }
            }
        }
        
        if (cursor_x < 0) cursor_x = 0;
        if (cursor_y < 0) cursor_y = 0;
        if (cursor_x >= screen_width) cursor_x = screen_width - 1;
        if (cursor_y >= screen_height - 1) cursor_y = screen_height - 2;
        
        bool click = mouse_left && !prev_mouse_left;
        prev_mouse_left = mouse_left;
        
        if (click) {
            for (int i = window_count - 1; i >= 0; i--) {
                struct window *win = windows[i];
                if (!win->visible) continue;
                
                if (cursor_x >= win->x && cursor_x < win->x + win->width &&
                    cursor_y >= win->y && cursor_y < win->y + 30) {
                    
                    if (cursor_x >= win->x + win->width - 28 && cursor_x < win->x + win->width - 8 &&
                        cursor_y >= win->y + 5 && cursor_y < win->y + 25) {
                        win->visible = false;
                    } else {
                        if (active_window) active_window->active = false;
                        win->active = true;
                        active_window = win;
                        
                        for (int j = i; j < window_count - 1; j++) {
                            windows[j] = windows[j + 1];
                        }
                        windows[window_count - 1] = win;
                    }
                    break;
                }
            }
        }
        
        gui_draw_cursor(cursor_x, cursor_y);
        
        gui_draw_text(10, screen_height - 60, "Arrow keys: move | Space: click | Mouse: move+click", 0x00888888);
        
        swap_buffers();
    }
}
