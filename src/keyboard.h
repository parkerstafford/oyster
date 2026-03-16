#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

void keyboard_init(void);
char keyboard_getchar(void);
bool keyboard_has_key(void);
void keyboard_get_arrows(int *up, int *down, int *left, int *right);

bool keyboard_has_scancode(void);
uint8_t keyboard_get_scancode(void);

#endif
