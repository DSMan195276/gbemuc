
#include "common.h"

#include <stdint.h>
#include <string.h>

#include "slist.h"
#include "gb.h"
#include "gb/gpu.h"
#include "gb/io.h"
#include "gb/sound.h"
#include "gb_internal.h"
#include "debug.h"

void gb_emu_io_reset(struct gb_emu *emu)
{
    emu->gpu.ctl = 0x91;

    emu->gpu.cur_line = 0;
    emu->gpu.cur_line_cmp = 0;
    emu->gpu.scroll_x = 0;
    emu->gpu.scroll_y = 0;
    emu->gpu.status = 0;

    emu->gpu.window_x = 0;
    emu->gpu.window_y = 0;

    emu->gpu.back_palette = 0xE4;

    emu->gpu.cgb_vram_bank_no = 0;
    emu->mmu.cgb_wram_bank_no = 1;

    emu->timer.div = 0;
    emu->timer.tima = 0;
    emu->timer.tma = 0;
    emu->timer.tac = 0;
}

uint8_t gb_emu_io_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    int ret = 0x100;

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

    if (!gb_emu_is_cgb(emu) || ret != 0x100) {
        if (ret == 0x100)
            ret = 0xFF;

        return ret;
    }

    switch (addr + low) {
    case GB_IO_CGB_WRAM_BANK_NO:
        ret = emu->mmu.cgb_wram_bank_no;
        break;

    case GB_IO_CGB_VRAM_BANK_NO:
        ret = emu->gpu.cgb_vram_bank_no;
        break;

    case GB_IO_CGB_BG_PAL_INDEX:
        ret = emu->gpu.cgb_bkgd_palette_index;
        break;

    case GB_IO_CGB_BG_PAL_DATA:
        ret = emu->gpu.cgb_bkgd_palette[emu->gpu.cgb_bkgd_palette_index & 0x3F];
        break;

    case GB_IO_CGB_SPRITE_PAL_INDEX:
        ret = emu->gpu.cgb_sprite_palette_index;
        break;

    case GB_IO_CGB_SPRITE_PAL_DATA:
        ret = emu->gpu.cgb_sprite_palette[emu->gpu.cgb_sprite_palette_index & 0x3F];
        break;

    case GB_IO_CGB_HDMA_SOURCE_LOW:
        ret = emu->mmu.hdma_source & 0xFF;
        break;

    case GB_IO_CGB_HDMA_SOURCE_HIGH:
        ret = (emu->mmu.hdma_source >> 8) & 0xFF;
        break;

    case GB_IO_CGB_HDMA_DEST_LOW:
        ret = emu->mmu.hdma_dest & 0xFF;
        break;

    case GB_IO_CGB_HDMA_DEST_HIGH:
        ret = (emu->mmu.hdma_dest >> 8) & 0xFF;
        break;

    case GB_IO_CGB_HDMA_MODE:
        ret = ((!emu->mmu.hdma_active) << 7) | (emu->mmu.hdma_length_left / 0x10 - 1);
        break;

    case GB_IO_CPU_CGB_KEY1:
        ret = (emu->cpu.double_speed << 7) | emu->cpu.do_speed_switch;
        break;
    }

    if (ret == 0x100)
        ret = 0xFF;

    return ret;
}

void gb_emu_io_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t byte)
{
    switch (addr + low) {
    case GB_IO_BIOS_FLAG:
        if (byte == 1) {

            /* Games know they're running on a CGB if register A is set to 0x11
             * on startup. So we do that here, which happens right before the
             * BIOS starts the game */
            if (!emu->mmu.bios_flag && gb_emu_is_cgb(emu))
                emu->cpu.r.b[GB_REG_A] = 0x11;

            emu->mmu.bios_flag = 1;
        }
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

    if (!gb_emu_is_cgb(emu))
        return ;

    switch (addr + low) {
    case GB_IO_CGB_WRAM_BANK_NO:
        emu->mmu.cgb_wram_bank_no = byte & 0x07;
        break;

    case GB_IO_CGB_VRAM_BANK_NO:
        emu->gpu.cgb_vram_bank_no = byte & 0x1;
        break;

    case GB_IO_CGB_BG_PAL_INDEX:
        emu->gpu.cgb_bkgd_palette_index = byte;
        break;

    case GB_IO_CGB_BG_PAL_DATA:
        emu->gpu.cgb_bkgd_palette[emu->gpu.cgb_bkgd_palette_index & 0x3F] = byte;

        if (emu->gpu.cgb_bkgd_palette_index & GB_GPU_CGB_PAL_INDEX_AUTO_INCREMENT)
            emu->gpu.cgb_bkgd_palette_index = (emu->gpu.cgb_bkgd_palette_index + 1) & 0xBF;

        break;

    case GB_IO_CGB_SPRITE_PAL_INDEX:
        emu->gpu.cgb_sprite_palette_index = byte;
        break;

    case GB_IO_CGB_SPRITE_PAL_DATA:
        emu->gpu.cgb_sprite_palette[emu->gpu.cgb_sprite_palette_index & 0x3F] = byte;

        if (emu->gpu.cgb_sprite_palette_index & GB_GPU_CGB_PAL_INDEX_AUTO_INCREMENT)
            emu->gpu.cgb_sprite_palette_index = (emu->gpu.cgb_sprite_palette_index + 1) & 0xBF;

        break;

    case GB_IO_CGB_HDMA_SOURCE_LOW:
        emu->mmu.hdma_source = (emu->mmu.hdma_source & 0xFF00) | (byte & 0xF0);
        break;

    case GB_IO_CGB_HDMA_SOURCE_HIGH:
        emu->mmu.hdma_source = (emu->mmu.hdma_source & 0x00FF) | (byte << 8);
        break;

    case GB_IO_CGB_HDMA_DEST_LOW:
        emu->mmu.hdma_dest = (emu->mmu.hdma_dest & 0xFF00) | (byte & 0xF0);
        break;

    case GB_IO_CGB_HDMA_DEST_HIGH:
        emu->mmu.hdma_dest = (emu->mmu.hdma_dest & 0x00FF) | ((byte & 0x1F) << 8) | 0x8000;
        break;

    case GB_IO_CGB_HDMA_MODE:
        if (emu->mmu.hdma_active) {
            if ((byte & GB_CGB_HDMA_DMA_TYPE) == 0)
                emu->mmu.hdma_active = 0;
            break;
        }

        emu->mmu.hdma_type = byte & GB_CGB_HDMA_DMA_TYPE;
        emu->mmu.hdma_length_left = ((byte & 0x7F) + 1) * 0x10;
        emu->mmu.hdma_active = 1;

        if (emu->mmu.hdma_type == GB_CGB_HDMA_DMA_GENERAL) {
            while (emu->mmu.hdma_length_left) {
                if ((emu->mmu.hdma_length_left % 2) == 0)
                    gb_emu_clock_tick(emu);
                emu->gpu.vram[emu->gpu.cgb_vram_bank_no].mem[emu->mmu.hdma_dest - 0x8000] = gb_emu_read8(emu, emu->mmu.hdma_source);

                emu->mmu.hdma_dest++;
                emu->mmu.hdma_source++;
                emu->mmu.hdma_length_left--;
            }

            emu->mmu.hdma_active = 0;
            emu->mmu.hdma_length_left = 0xFF;
        }
        break;

    case GB_IO_CPU_CGB_KEY1:
        emu->cpu.do_speed_switch = byte & 1;
        break;
    }
}

