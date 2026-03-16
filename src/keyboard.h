#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

void keyboard_init(void);
char keyboard_getchar(void);
bool keyboard_has_key(void);
void keyboard_get_arrows(int *up, int *down, int *left, int *right);

#endif
