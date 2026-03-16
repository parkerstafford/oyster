#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>
#include <stdbool.h>

struct mouse_state {
    int32_t x;
    int32_t y;
    bool left_button;
    bool right_button;
    bool middle_button;
};

void mouse_init(void);
struct mouse_state mouse_get_state(void);
void mouse_set_bounds(int32_t max_x, int32_t max_y);

#endif
