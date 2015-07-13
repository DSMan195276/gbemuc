#ifndef INCLUDE_GB_CPU_H
#define INCLUDE_GB_CPU_H

#include <stdint.h>

/* Note, the order of the entries in the next two enum's is important - They
 * have to match, and they need to be in this specefic order. */
enum {
    GB_REG_AF,
    GB_REG_BC,
    GB_REG_DE,
    GB_REG_HL,
    GB_REG_SP,
    GB_REG_PC,
    GB_REG_TOTAL
};

enum {
    GB_REG_F,
    GB_REG_A,

    GB_REG_C,
    GB_REG_B,

    GB_REG_E,
    GB_REG_D,

    GB_REG_L,
    GB_REG_H,
};

enum {
    GB_FLAG_ZERO   = 0x80,
    GB_FLAG_SUB    = 0x40,
    GB_FLAG_HCARRY = 0x20,
    GB_FLAG_CARRY  = 0x10
};

#define GB_FLAG_IS_SET(flags, flag) (((flags) & flag) == flag)

struct gb_cpu {
    union {
        uint8_t b[GB_REG_TOTAL * 2];
        uint16_t w[GB_REG_TOTAL];
    } r;

    int m, t;

    unsigned int halted :1;
    unsigned int stopped :1;
    unsigned int ime :1; /* Interrupt master enable */

    /* These are used for the delay required by the DI and EI instructions.
     * The CPU counts down the 'int_count' and then sets 'int_enabled' equal to
     * 'next_int_enabled' when 'int_count' hits zero.
     * 'int_count' is decremented at the end of running an instruction. */
    unsigned int next_ime:1;
    unsigned int int_count :2;
};

struct gb_emu;

uint8_t gb_cpu_int_read8(struct gb_emu *, uint16_t addr, uint16_t low);
uint16_t gb_cpu_int_read16(struct gb_emu *, uint16_t addr, uint16_t low);

void gb_cpu_int_write8(struct gb_emu *, uint16_t addr, uint16_t low, uint8_t byte);
void gb_cpu_int_write16(struct gb_emu *, uint16_t addr, uint16_t low, uint16_t word);

#endif
