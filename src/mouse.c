#include "mouse.h"
#include "io.h"
#include "idt.h"
#include "pic.h"

#define MOUSE_DATA_PORT    0x60
#define MOUSE_STATUS_PORT  0x64
#define MOUSE_COMMAND_PORT 0x64

static volatile struct mouse_state state = {0, 0, false, false, false};
static volatile uint8_t mouse_cycle = 0;
static volatile int8_t mouse_bytes[3];
static int32_t max_x = 1280;
static int32_t max_y = 800;

static void mouse_wait_write(void) {
    int timeout = 100000;
    while (timeout-- && (inb(MOUSE_STATUS_PORT) & 2));
}

static void mouse_wait_read(void) {
    int timeout = 100000;
    while (timeout-- && !(inb(MOUSE_STATUS_PORT) & 1));
}

static void mouse_write(uint8_t data) {
    mouse_wait_write();
    outb(MOUSE_COMMAND_PORT, 0xD4);
    mouse_wait_write();
    outb(MOUSE_DATA_PORT, data);
}

static uint8_t mouse_read(void) {
    mouse_wait_read();
    return inb(MOUSE_DATA_PORT);
}

static void mouse_handler(struct interrupt_frame *frame) {
    (void)frame;
    
    uint8_t status = inb(MOUSE_STATUS_PORT);
    if (!(status & 0x20)) {
        pic_send_eoi(12);
        return;
    }
    
    int8_t data = inb(MOUSE_DATA_PORT);
    
    switch (mouse_cycle) {
        case 0:
            if (data & 0x08) {
                mouse_bytes[0] = data;
                mouse_cycle++;
            }
            break;
        case 1:
            mouse_bytes[1] = data;
            mouse_cycle++;
            break;
        case 2:
            mouse_bytes[2] = data;
            mouse_cycle = 0;
            
            state.left_button = mouse_bytes[0] & 0x01;
            state.right_button = mouse_bytes[0] & 0x02;
            state.middle_button = mouse_bytes[0] & 0x04;
            
            int32_t rel_x = mouse_bytes[1];
            int32_t rel_y = mouse_bytes[2];
            
            if (mouse_bytes[0] & 0x10) rel_x |= 0xFFFFFF00;
            if (mouse_bytes[0] & 0x20) rel_y |= 0xFFFFFF00;
            
            state.x += rel_x;
            state.y -= rel_y;
            
            if (state.x < 0) state.x = 0;
            if (state.y < 0) state.y = 0;
            if (state.x >= max_x) state.x = max_x - 1;
            if (state.y >= max_y) state.y = max_y - 1;
            break;
    }
    
    pic_send_eoi(12);
}

void mouse_init(void) {
    mouse_wait_write();
    outb(MOUSE_COMMAND_PORT, 0xA8);
    
    mouse_wait_write();
    outb(MOUSE_COMMAND_PORT, 0x20);
    mouse_wait_read();
    uint8_t status = inb(MOUSE_DATA_PORT);
    status |= 2;
    status &= ~0x20;
    mouse_wait_write();
    outb(MOUSE_COMMAND_PORT, 0x60);
    mouse_wait_write();
    outb(MOUSE_DATA_PORT, status);
    
    mouse_write(0xF6);
    mouse_read();
    
    mouse_write(0xF4);
    mouse_read();
    
    isr_register_handler(44, mouse_handler);
    pic_clear_mask(12);
    
    state.x = max_x / 2;
    state.y = max_y / 2;
}

struct mouse_state mouse_get_state(void) {
    return state;
}

void mouse_set_bounds(int32_t mx, int32_t my) {
    max_x = mx;
    max_y = my;
}
