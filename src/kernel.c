#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"
#include "framebuffer.h"
#include "gdt.h"
#include "idt.h"
#include "pic.h"
#include "timer.h"
#include "keyboard.h"
#include "shell.h"

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
    fb_puts("Initializing kernel...\n");
    
    fb_puts("  [");
    fb_set_color(0x00b5bd68);
    fb_puts("OK");
    fb_set_color(0x00c5c8c6);
    fb_puts("] GDT\n");
    gdt_init();
    
    fb_puts("  [");
    fb_set_color(0x00b5bd68);
    fb_puts("OK");
    fb_set_color(0x00c5c8c6);
    fb_puts("] IDT\n");
    idt_init();
    
    fb_puts("  [");
    fb_set_color(0x00b5bd68);
    fb_puts("OK");
    fb_set_color(0x00c5c8c6);
    fb_puts("] PIC\n");
    pic_init();
    
    fb_puts("  [");
    fb_set_color(0x00b5bd68);
    fb_puts("OK");
    fb_set_color(0x00c5c8c6);
    fb_puts("] Timer (100 Hz)\n");
    timer_init(100);
    
    fb_puts("  [");
    fb_set_color(0x00b5bd68);
    fb_puts("OK");
    fb_set_color(0x00c5c8c6);
    fb_puts("] Keyboard\n");
    keyboard_init();
    
    __asm__ volatile ("sti");
    
    fb_puts("\n");
    fb_set_color(0x0081a2be);
    fb_puts("System Information:\n");
    fb_set_color(0x00c5c8c6);
    fb_puts("  Architecture: x86_64\n");
    fb_puts("  Bootloader: Limine\n");
    fb_printf("  Resolution: %dx%d\n", fb->width, fb->height);
    fb_printf("  Bits per pixel: %d\n", fb->bpp);
    
    shell_init();
    shell_run();
    
    hcf();
}

void _start(void) {
    kmain();
    hcf();
}
