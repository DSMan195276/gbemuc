#ifndef INCLUDE_GB_DISASM_H
#define INCLUDE_GB_DISASM_H

#include <stdint.h>

struct opcode_format {
    const char *format;
    enum {
        OPCODE_NONE,
        OPCODE_16BIT,
        OPCODE_8BIT,
    } type;
    int is_jmp;
};

struct opcode_format opcode_decode_format_str[256];

void gb_disasm_inst(char *buf, uint8_t *bytes);

#endif
