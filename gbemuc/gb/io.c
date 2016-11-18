
#include "common.h"

#include <stdint.h>
#include <string.h>

#include "slist.h"
#include "gb.h"
#include "gb/gpu.h"
#include "gb/io.h"
#include "gb/sound.h"
#include "debug.h"

uint8_t gb_emu_io_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    uint8_t ret = 0xFF;

    switch (addr + low) {
    case GB_IO_CPU_IF:
        ret = emu->cpu.int_flags;
        break;

    case GB_IO_GPU_CTL:
        ret = emu->gpu.ctl;
        break;

    case GB_IO_GPU_STATUS:
        ret = emu->gpu.status & 0xF8;
        ret |= emu->gpu.mode;
        ret |= (emu->gpu.cur_line == emu->gpu.cur_line_cmp)? 4: 0;
        break;

    case GB_IO_GPU_SCRY:
        ret = emu->gpu.scroll_y;
        break;

    case GB_IO_GPU_SCRX:
        ret = emu->gpu.scroll_x;
        break;

    case GB_IO_GPU_LY:
        ret = emu->gpu.cur_line;
        break;

    case GB_IO_GPU_LYC:
        ret = emu->gpu.cur_line_cmp;
        break;

    case GB_IO_GPU_PALETTE:
        ret = emu->gpu.back_palette;
        break;

    case GB_IO_GPU_WX:
        ret = emu->gpu.window_x;
        break;

    case GB_IO_GPU_WY:
        ret = emu->gpu.window_y;
        break;

    case GB_IO_OBJ_PAL1:
        ret = emu->gpu.obj_pal[0];
        break;

    case GB_IO_OBJ_PAL2:
        ret = emu->gpu.obj_pal[1];
        break;

    case GB_IO_KEYPAD:
        ret = emu->gpu.key_line;
        break;

    case GB_IO_TIMER_DIV:
        ret = emu->timer.div;
        break;

    case GB_IO_TIMER_TIMA:
        ret = emu->timer.tima;
        break;

    case GB_IO_TIMER_TMA:
        ret = emu->timer.tma;
        break;

    case GB_IO_TIMER_TAC:
        ret = emu->timer.tac;
        break;

    case 0xFF10 ... 0xFF3F:
        ret = gb_sound_read(&emu->sound, emu->sound.apu_cycles, addr + low);
        break;
    }

    return ret;
}

void gb_emu_io_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t byte)
{
    switch (addr + low) {
    case GB_IO_BIOS_FLAG:
        if (byte == 1)
            emu->mmu.bios_flag = 1;
        DEBUG_ON();
        break;

    case GB_IO_CPU_IF:
        emu->cpu.int_flags = byte;
        break;

    case GB_IO_GPU_CTL:
        gb_gpu_ctl_change(emu, &emu->gpu, byte);
        break;

    case GB_IO_GPU_STATUS:
        emu->gpu.status = byte & 0x78;
        break;

    case GB_IO_GPU_SCRY:
        emu->gpu.scroll_y = byte;
        break;

    case GB_IO_GPU_SCRX:
        emu->gpu.scroll_x = byte;
        break;

    case GB_IO_GPU_LY:
        emu->gpu.cur_line = 0;

        if ((emu->gpu.status & GB_GPU_STATUS_CONC_INT)
            && emu->gpu.cur_line == emu->gpu.cur_line_cmp)
            emu->cpu.int_flags |= (1 << GB_INT_LCD_STAT);
        break;

    case GB_IO_GPU_LYC:
        emu->gpu.cur_line_cmp = byte;

        break;

    case GB_IO_GPU_PALETTE:
        emu->gpu.back_palette = byte;
        break;

    case GB_IO_GPU_WX:
        emu->gpu.window_x = byte;
        break;

    case GB_IO_GPU_WY:
        emu->gpu.window_y = byte;
        break;

    case GB_IO_GPU_DMA:
        gb_gpu_dma(emu, byte);
        break;

    case GB_IO_OBJ_PAL1:
        emu->gpu.obj_pal[0] = byte;
        break;

    case GB_IO_OBJ_PAL2:
        emu->gpu.obj_pal[1] = byte;
        break;

    case GB_IO_KEYPAD:
        emu->gpu.key_select = byte & 0x30;
        gb_gpu_update_key_line(emu);
        break;

    case GB_IO_TIMER_DIV:
        emu->timer.div = 0;
        emu->timer.div_count = 0;
        break;

    case GB_IO_TIMER_TIMA:
        emu->timer.tima = byte;
        emu->timer.tima_count = 0;
        break;

    case GB_IO_TIMER_TMA:
        emu->timer.tma = byte;
        break;

    case GB_IO_TIMER_TAC:
        gb_timer_update_tac(emu, byte);
        break;

    case 0xFF10 ... 0xFF3F:
        gb_sound_write(&emu->sound, emu->sound.apu_cycles, addr + low, byte);
        break;
    }
}

