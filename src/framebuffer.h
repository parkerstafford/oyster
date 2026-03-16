#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <stdint.h>
#include <stddef.h>
#include "limine.h"

void fb_init(struct limine_framebuffer *framebuffer);
void fb_clear(uint32_t color);
void fb_putpixel(int x, int y, uint32_t color);
void fb_putchar(char c);
void fb_puts(const char *s);
void fb_printf(const char *fmt, ...);
void fb_set_color(uint32_t color);

#endif /* FRAMEBUFFER_H */
