#include "timer.h"
#include "io.h"
#include "idt.h"
#include "pic.h"

#define PIT_CHANNEL0 0x40
#define PIT_COMMAND  0x43

static volatile uint64_t ticks = 0;
static uint32_t tick_frequency = 0;

static void timer_handler(struct interrupt_frame *frame) {
    (void)frame;
    ticks++;
    pic_send_eoi(0);
}

void timer_init(uint32_t frequency) {
    tick_frequency = frequency;
    
    uint32_t divisor = 1193180 / frequency;
    
    outb(PIT_COMMAND, 0x36);
    outb(PIT_CHANNEL0, divisor & 0xFF);
    outb(PIT_CHANNEL0, (divisor >> 8) & 0xFF);
    
    isr_register_handler(32, timer_handler);
    pic_clear_mask(0);
}

uint64_t timer_get_ticks(void) {
    return ticks;
}

void timer_sleep(uint64_t ms) {
    uint64_t target = ticks + (ms * tick_frequency / 1000);
    while (ticks < target) {
        __asm__ volatile ("hlt");
    }
}
