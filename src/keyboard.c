#include "keyboard.h"
#include "io.h"
#include "idt.h"
#include "pic.h"
#include "framebuffer.h"

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64

static char key_buffer[256];
static volatile uint8_t buffer_start = 0;
static volatile uint8_t buffer_end = 0;

static const char scancode_to_ascii[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0
};

static const char scancode_to_ascii_shift[] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0
};

static bool shift_pressed = false;
static bool caps_lock = false;

static void keyboard_handler(struct interrupt_frame *frame) {
    (void)frame;
    
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = true;
    } else if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = false;
    } else if (scancode == 0x3A) {
        caps_lock = !caps_lock;
    } else if (scancode < 0x80 && scancode < sizeof(scancode_to_ascii)) {
        char c;
        bool use_shift = shift_pressed ^ caps_lock;
        
        if (use_shift) {
            c = scancode_to_ascii_shift[scancode];
        } else {
            c = scancode_to_ascii[scancode];
        }
        
        if (c != 0) {
            uint8_t next = (buffer_end + 1) % 256;
            if (next != buffer_start) {
                key_buffer[buffer_end] = c;
                buffer_end = next;
            }
        }
    }
    
    pic_send_eoi(1);
}

void keyboard_init(void) {
    isr_register_handler(33, keyboard_handler);
    pic_clear_mask(1);
}

bool keyboard_has_key(void) {
    return buffer_start != buffer_end;
}

char keyboard_getchar(void) {
    while (!keyboard_has_key()) {
        __asm__ volatile ("hlt");
    }
    
    char c = key_buffer[buffer_start];
    buffer_start = (buffer_start + 1) % 256;
    return c;
}
