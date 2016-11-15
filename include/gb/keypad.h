#ifndef INCLUDE_KEYPAD_H
#define INCLUDE_KEYPAD_H

#include <stdint.h>

struct gb_keypad {
    uint8_t key_a :1;
    uint8_t key_b :1;
    uint8_t key_up :1;
    uint8_t key_down :1;
    uint8_t key_left :1;
    uint8_t key_right :1;
    uint8_t key_start :1;
    uint8_t key_select :1;

    uint8_t key_out;
    uint8_t key_line_active;
};

#endif
