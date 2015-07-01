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
    GB_REG_A,
    GB_REG_F,

    GB_REG_B,
    GB_REG_C,

    GB_REG_D,
    GB_REG_E,

    GB_REG_H,
    GB_REG_L,
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
    unsigned int int_enabled :1;

    /* These are used for the delay required by the DI and EI instructions.
     * The CPU counts down the 'int_count' and then sets 'int_enabled' equal to
     * 'next_int_enabled' when 'int_count' hits zero.
     * 'int_count' is decremented at the end of running an instruction. */
    unsigned int next_int_enabled :1;
    unsigned int int_count :2;
};

#endif
