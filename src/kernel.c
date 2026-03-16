#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"
#include "framebuffer.h"

static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static void hcf(void) {
    for (;;) {
        __asm__ volatile ("hlt");
    }
}

void kmain(void) {
    if (framebuffer_request.response == NULL 
        || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    
    fb_init(fb);
    fb_clear(0x001a1a2e);
    
    fb_set_color(0x00f0c674);
    fb_puts("=====================================\n");
    fb_puts("       Welcome to Oyster OS!\n");
    fb_puts("=====================================\n\n");
    
    fb_set_color(0x00c5c8c6);
    fb_puts("Kernel loaded successfully.\n");
    fb_puts("Framebuffer initialized.\n\n");
    
    fb_set_color(0x0081a2be);
    fb_puts("System Information:\n");
    fb_set_color(0x00c5c8c6);
    fb_puts("  Architecture: x86_64\n");
    fb_puts("  Bootloader: Limine\n");
    
    fb_printf("  Resolution: %dx%d\n", fb->width, fb->height);
    fb_printf("  Bits per pixel: %d\n", fb->bpp);
    
    fb_puts("\n");
    fb_set_color(0x00b5bd68);
    fb_puts("Oyster OS is ready.\n");
    
    hcf();
}

void _start(void) {
    kmain();
    hcf();
}
