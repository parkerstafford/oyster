#include "shell.h"
#include "framebuffer.h"
#include "keyboard.h"
#include "timer.h"
#include <stdint.h>
#include <stdbool.h>

static char input_buffer[256];
static int input_pos = 0;

static int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

static int strncmp(const char *s1, const char *s2, int n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

static void cmd_help(void) {
    fb_set_color(0x0081a2be);
    fb_puts("Available commands:\n");
    fb_set_color(0x00c5c8c6);
    fb_puts("  help     - Show this help message\n");
    fb_puts("  clear    - Clear the screen\n");
    fb_puts("  info     - Show system information\n");
    fb_puts("  time     - Show uptime\n");
    fb_puts("  echo     - Echo text back\n");
    fb_puts("  color    - Test colors\n");
}

static void cmd_clear(void) {
    fb_clear(0x001a1a2e);
}

static void cmd_info(void) {
    fb_set_color(0x0081a2be);
    fb_puts("System Information:\n");
    fb_set_color(0x00c5c8c6);
    fb_puts("  OS: Oyster OS v0.1\n");
    fb_puts("  Architecture: x86_64\n");
    fb_puts("  Bootloader: Limine\n");
}

static void cmd_time(void) {
    uint64_t ticks = timer_get_ticks();
    uint64_t seconds = ticks / 100;
    uint64_t minutes = seconds / 60;
    seconds %= 60;
    
    fb_set_color(0x00c5c8c6);
    fb_printf("Uptime: %lu minutes, %lu seconds\n", minutes, seconds);
}

static void cmd_echo(const char *text) {
    fb_set_color(0x00c5c8c6);
    fb_puts(text);
    fb_puts("\n");
}

static void cmd_color(void) {
    fb_set_color(0x00ff0000);
    fb_puts("Red ");
    fb_set_color(0x0000ff00);
    fb_puts("Green ");
    fb_set_color(0x000000ff);
    fb_puts("Blue ");
    fb_set_color(0x00ffff00);
    fb_puts("Yellow ");
    fb_set_color(0x00ff00ff);
    fb_puts("Magenta ");
    fb_set_color(0x0000ffff);
    fb_puts("Cyan ");
    fb_set_color(0x00ffffff);
    fb_puts("White\n");
}

static void process_command(void) {
    if (input_pos == 0) return;
    
    input_buffer[input_pos] = '\0';
    
    if (strcmp(input_buffer, "help") == 0) {
        cmd_help();
    } else if (strcmp(input_buffer, "clear") == 0) {
        cmd_clear();
    } else if (strcmp(input_buffer, "info") == 0) {
        cmd_info();
    } else if (strcmp(input_buffer, "time") == 0) {
        cmd_time();
    } else if (strncmp(input_buffer, "echo ", 5) == 0) {
        cmd_echo(input_buffer + 5);
    } else if (strcmp(input_buffer, "color") == 0) {
        cmd_color();
    } else {
        fb_set_color(0x00ff6666);
        fb_printf("Unknown command: %s\n", input_buffer);
        fb_set_color(0x00c5c8c6);
        fb_puts("Type 'help' for available commands.\n");
    }
}

static void print_prompt(void) {
    fb_set_color(0x00b5bd68);
    fb_puts("oyster");
    fb_set_color(0x00c5c8c6);
    fb_puts("> ");
}

void shell_init(void) {
    fb_set_color(0x0081a2be);
    fb_puts("\nOyster Shell v0.1\n");
    fb_set_color(0x00c5c8c6);
    fb_puts("Type 'help' for available commands.\n\n");
}

void shell_run(void) {
    print_prompt();
    input_pos = 0;
    
    while (true) {
        char c = keyboard_getchar();
        
        if (c == '\n') {
            fb_putchar('\n');
            process_command();
            input_pos = 0;
            print_prompt();
        } else if (c == '\b') {
            if (input_pos > 0) {
                input_pos--;
                fb_puts("\b \b");
            }
        } else if (input_pos < 255) {
            input_buffer[input_pos++] = c;
            fb_putchar(c);
        }
    }
}
