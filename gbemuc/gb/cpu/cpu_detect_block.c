
#include "common.h"

#include <stdint.h>
#include <time.h>
#include <signal.h>

#include "debug.h"
#include "gb_internal.h"
#include "gb/cpu.h"
#include "cpu_internal.h"
#include "cpu_jit_helpers.h"
#include "cpu_dispatcher.h"
#include "hashtable.h"

int gb_emu_detect_block_length(struct gb_emu *emu)
{
    uint16_t pc = emu->cpu.r.w[GB_REG_PC];

    while (1) {
        uint8_t inst = gb_emu_read8(emu, pc);

    }
}

