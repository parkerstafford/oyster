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

static uint8_t scancode_buffer[512];
static volatile uint16_t sc_buf_start = 0;
static volatile uint16_t sc_buf_end = 0;

static volatile int arrow_up = 0;
static volatile int arrow_down = 0;
static volatile int arrow_left = 0;
static volatile int arrow_right = 0;

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
static bool extended_key = false;

static void keyboard_handler(struct interrupt_frame *frame) {
    (void)frame;
    
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);
    
    uint16_t next_sc = (sc_buf_end + 1) % 512;
    if (next_sc != sc_buf_start) {
        scancode_buffer[sc_buf_end] = scancode;
        sc_buf_end = next_sc;
    }
    
    if (scancode == 0xE0) {
        extended_key = true;
        pic_send_eoi(1);
        return;
    }
    
    if (extended_key) {
        extended_key = false;
        switch (scancode) {
            case 0x48: arrow_up = 1; break;
            case 0x50: arrow_down = 1; break;
            case 0x4B: arrow_left = 1; break;
            case 0x4D: arrow_right = 1; break;
            case 0xC8: arrow_up = 0; break;
            case 0xD0: arrow_down = 0; break;
            case 0xCB: arrow_left = 0; break;
            case 0xCD: arrow_right = 0; break;
        }
        pic_send_eoi(1);
        return;
    }
    
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

void keyboard_get_arrows(int *up, int *down, int *left, int *right) {
    *up = arrow_up;
    *down = arrow_down;
    *left = arrow_left;
    *right = arrow_right;
}

bool keyboard_has_scancode(void) {
    return sc_buf_start != sc_buf_end;
}

uint8_t keyboard_get_scancode(void) {
    if (sc_buf_start == sc_buf_end) return 0;
    uint8_t sc = scancode_buffer[sc_buf_start];
    sc_buf_start = (sc_buf_start + 1) % 512;
    return sc;
}
