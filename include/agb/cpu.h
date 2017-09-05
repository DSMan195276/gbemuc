#ifndef INCLUDE_AGB_CPU_H
#define INCLUDE_AGB_CPU_H

#include <stdint.h>

struct agb_cpu {
    uint32_t reg[16];

    uint32_t CPSR;
};

#define APU_REG_SP 13
#define APU_REG_LR 14
#define APU_REG_PC 15

#endif
