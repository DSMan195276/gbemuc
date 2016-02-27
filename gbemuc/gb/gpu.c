
#include "common.h"

#include <stdint.h>
#include <string.h>

#include "gb/cpu.h"
#include "gb/gpu.h"
#include "gb.h"
#include "debug.h"

union gb_gpu_color_u gb_colors[] =
{
    { .i_color = 0xFFFFF77B },
    { .i_color = 0xFFB5AE4A },
    { .i_color = 0xFF6B6931 },
    { .i_color = 0xFF212010 },
};

static void render_background(struct gb_gpu *gpu)
{
    union gb_gpu_color_u *line;
    int bkgd_y_pix;
    int tile_offset_byte;
    int i;
    uint8_t *bkgd_tiles;
    int tile;

    line = &gpu->screenbuf[gpu->cur_line * GB_SCREEN_WIDTH];

    bkgd_y_pix = (gpu->cur_line + gpu->scroll_y) % (8 * 32);

    tile = (bkgd_y_pix) / 8 * 32;
    bkgd_tiles = gpu->vram.seg.bkgd[(gpu->ctl & GB_GPU_CTL_BKGD_MAP)? 1: 0] + tile;

    tile_offset_byte = bkgd_y_pix % 8;

    for (i = 0; i < GB_SCREEN_WIDTH; i++) {
        uint8_t c, pal_col;
        uint8_t lo, hi;
        int sprite;
        uint8_t x_pix;

        x_pix = (i + gpu->scroll_x) % (8 * 32);

        if (gpu->ctl & GB_GPU_CTL_BKGD_SET)
            sprite = bkgd_tiles[x_pix / 8];
        else
            sprite = *(int8_t *)&bkgd_tiles[x_pix / 8] + 256;

        lo = gpu->vram.seg.sprites[sprite][tile_offset_byte * 2] & (1 << (7 - (x_pix % 8)));
        hi = gpu->vram.seg.sprites[sprite][tile_offset_byte * 2 + 1] & (1 << (7 - (x_pix % 8)));

        c = ((lo)? 1: 0) + ((hi)? 2: 0);

        /* Pull the actual color out of the background palette
         * We shift our color on the palette by 'c', moving two bits for every
         * color, so our target color end's up at the bottom, and then mask out
         * the rest */

        pal_col = (gpu->back_palette >> (c * 2)) & 0x03;

        line[i] = gb_colors[pal_col];
    }
}

static void render_window(struct gb_gpu *gpu)
{
    union gb_gpu_color_u *line;
    int bkgd_y_pix;
    int tile_offset_byte;
    int i;
    uint8_t *bkgd_tiles;
    int tile;

    line = &gpu->screenbuf[gpu->cur_line * GB_SCREEN_WIDTH];

    bkgd_y_pix = (gpu->cur_line + gpu->window_x - 7) % (8 * 32);

    tile = (bkgd_y_pix) / 8 * 32;
    bkgd_tiles = gpu->vram.seg.bkgd[(gpu->ctl & GB_GPU_CTL_WINDOW_MAP)? 1: 0] + tile;

    tile_offset_byte = bkgd_y_pix % 8;

    for (i = 0; i < GB_SCREEN_WIDTH; i++) {
        uint8_t c, pal_col;
        uint8_t lo, hi;
        int sprite;
        uint8_t x_pix;

        x_pix = (i + gpu->window_y) % (8 * 32);

        if (gpu->ctl & GB_GPU_CTL_BKGD_SET)
            sprite = bkgd_tiles[x_pix / 8];
        else
            sprite = *(int8_t *)&bkgd_tiles[x_pix / 8] + 256;

        lo = gpu->vram.seg.sprites[sprite][tile_offset_byte * 2] & (1 << (7 - (x_pix % 8)));
        hi = gpu->vram.seg.sprites[sprite][tile_offset_byte * 2 + 1] & (1 << (7 - (x_pix % 8)));

        c = ((lo)? 1: 0) + ((hi)? 2: 0);

        /* Pull the actual color out of the background palette
         * We shift our color on the palette by 'c', moving two bits for every
         * color, so our target color end's up at the bottom, and then mask out
         * the rest */

        pal_col = (gpu->back_palette >> (c * 2)) & 0x03;

        line[i] = gb_colors[pal_col];

    }
}

static void render_sprites(struct gb_gpu *gpu)
{
    union gb_gpu_color_u *line;
    int s;
    int c_line;
    int count = 0;

    c_line = gpu->cur_line;

    line = &gpu->screenbuf[gpu->cur_line * GB_SCREEN_WIDTH];

    for (s = 0; s < 40; s++) {
        uint8_t *sprite = &gpu->oam.s_attrs[s][0];
        int y, x;

        y = sprite[GB_GPU_SPRITE_ATTR_Y] - 16;
        x = sprite[GB_GPU_SPRITE_ATTR_X] - 8;

        if (c_line >= y && c_line < y + 8)
            continue;

        count++;

        if (count == 11)
            break;

        if (x + 8 < 0 || x >= GB_SCREEN_WIDTH)
            continue;

        uint8_t *tile = &gpu->vram.seg.sprites[sprite[GB_GPU_SPRITE_ATTR_TILE_NUM]][0];

        uint8_t y_pix = c_line - y, x_pix;
        uint8_t pal;

        pal = gpu->obj_pal[(sprite[GB_GPU_SPRITE_ATTR_FLAGS]
            & GB_GPU_SPRITE_FLAG_PAL_NUM) >> 4];

        for (x_pix = 0; x_pix <  8; x_pix++) {
            if (x + x_pix < 0 || x + x_pix >= GB_SCREEN_WIDTH)
                continue;

            uint8_t lo, hi, c;
            uint8_t pal_col;

            lo = tile[y_pix * 2] & (1 << (7 - (x_pix % 8)));
            hi = tile[y_pix * 2 + 1] & (1 << (7 - (x_pix % 8)));

            c = ((lo)? 1: 0) + ((hi)? 2: 0);

            pal_col = (pal >> (c * 2)) & 0x03;

            line[x_pix + x] = gb_colors[pal_col];
        }
    }
}

void gb_gpu_render_line(struct gb_gpu *gpu)
{

    if (!(gpu->ctl & GB_GPU_CTL_DISPLAY))
        return ;

    if (gpu->ctl & GB_GPU_CTL_BKGD)
        render_background(gpu);

    if (gpu->ctl & GB_GPU_CTL_WINDOW)
        render_window(gpu);

    if (gpu->ctl & GB_GPU_CTL_SPRITES)
        render_sprites(gpu);
}

void gb_gpu_display_screen(struct gb_gpu *gpu)
{
    (gpu->display->disp_buf) (gpu->display, gpu->screenbuf);
}

void gb_gpu_ctl_change(struct gb_gpu *gpu, uint8_t new_ctl)
{
    uint8_t diff = gpu->ctl ^ new_ctl;

    if (diff & GB_GPU_CTL_DISPLAY) {
        memset(gpu->screenbuf, 0x00, sizeof(gpu->screenbuf));
        gb_gpu_display_screen(gpu);

        gpu->cur_line = 0;
        gpu->mode = GB_GPU_MODE_VBLANK;
    }

    gpu->ctl = new_ctl;
}

static void gb_gpu_inc_line(struct gb_emu *emu)
{
    emu->gpu.cur_line++;

    if (emu->gpu.cur_line == emu->gpu.cur_line_cmp)
        emu->cpu.int_flags |= (1 << GB_INT_LCD_STAT);
}

void gb_emu_gpu_tick(struct gb_emu *emu)
{
    struct gb_gpu *gpu = &emu->gpu;

    if (!(gpu->ctl & GB_GPU_CTL_DISPLAY))
        return ;

    gpu->clock += 4;

    switch (gpu->mode) {
    case GB_GPU_MODE_HBLANK:
        if (gpu->clock >= GB_GPU_CLOCK_HBLANK) {
            gpu->clock = 0;
            gb_gpu_inc_line(emu);

            if (gpu->cur_line == GB_SCREEN_HEIGHT - 1) {
                gpu->mode = GB_GPU_MODE_VBLANK;
                gb_gpu_display_screen(gpu);

                emu->cpu.int_flags |= (1 << GB_INT_VBLANK);
            } else {
                gpu->mode = GB_GPU_MODE_OAM;
            }
        }
        break;

    case GB_GPU_MODE_OAM:
        if (gpu->clock >= GB_GPU_CLOCK_OAM) {
            gpu->clock = 0;
            gpu->mode = GB_GPU_MODE_VRAM;
        }
        break;

    case GB_GPU_MODE_VRAM:
        if (gpu->clock >= GB_GPU_CLOCK_VRAM) {
            gpu->clock = 0;
            gpu->mode = GB_GPU_MODE_HBLANK;

            gb_gpu_render_line(gpu);
        }
        break;

    case GB_GPU_MODE_VBLANK:
        if (gpu->clock >= GB_GPU_CLOCK_VBLANK) {
            gpu->clock = 0;
            gb_gpu_inc_line(emu);
            if (gpu->cur_line >= GB_SCREEN_HEIGHT + GB_GPU_VBLANK_LENGTH - 1) {
                gpu->mode = GB_GPU_MODE_OAM;
                gpu->cur_line = 0;

                if (gpu->cur_line == gpu->cur_line_cmp)
                    emu->cpu.int_flags |= (1 << GB_INT_LCD_STAT);
            }
        }
        break;
    }
}

void gb_gpu_dma(struct gb_emu *emu, uint8_t dma_addr)
{
    uint16_t src_start = dma_addr << 16;
    uint16_t dest_start = 0xFE00;
    int i;

    for (i = 0; i < 0x9F; i++)
        gb_emu_write8(emu, dest_start + i, gb_emu_read8(emu, src_start));
}

void gb_gpu_init(struct gb_gpu *gpu, struct gb_gpu_display *display)
{
    memset(gpu, 0, sizeof(*gpu));

    memset(gpu->screenbuf, 0x00, sizeof(gpu->screenbuf));
    gpu->display = display;
}

uint8_t gb_gpu_vram_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (emu->gpu.mode != GB_GPU_MODE_VRAM)
        return emu->gpu.vram.mem[addr];
    else
        return 0;
}

uint16_t gb_gpu_vram_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (emu->gpu.mode != GB_GPU_MODE_VRAM) {
        uint16_t ret;
        memcpy(&ret, emu->gpu.vram.mem + addr, sizeof(ret));
        return ret;
    } else {
        return 0;
    }
}

void gb_gpu_vram_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t byte)
{
    if (emu->gpu.mode != GB_GPU_MODE_VRAM)
        emu->gpu.vram.mem[addr] = byte;
}

void gb_gpu_vram_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t word)
{
    if (emu->gpu.mode != GB_GPU_MODE_VRAM)
        memcpy(emu->gpu.vram.mem + addr, &word, sizeof(word));
}

uint8_t gb_gpu_sprite_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (emu->gpu.mode != GB_GPU_MODE_VRAM && emu->gpu.mode != GB_GPU_MODE_OAM)
        return emu->gpu.oam.mem[addr];
    else
        return 0;
}

uint16_t gb_gpu_sprite_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (emu->gpu.mode != GB_GPU_MODE_VRAM && emu->gpu.mode != GB_GPU_MODE_OAM) {
        uint16_t ret;
        memcpy(&ret, emu->gpu.oam.mem + addr, sizeof(ret));
        return ret;
    } else {
        return 0;
    }
}

void gb_gpu_sprite_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t byte)
{
    if (emu->gpu.mode != GB_GPU_MODE_VRAM && emu->gpu.mode != GB_GPU_MODE_OAM)
        emu->gpu.oam.mem[addr] = byte;
}

void gb_gpu_sprite_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t word)
{
    if (emu->gpu.mode != GB_GPU_MODE_VRAM && emu->gpu.mode != GB_GPU_MODE_OAM)
        memcpy(emu->gpu.oam.mem + addr, &word, sizeof(word));
}

