
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

        if ((gpu->ctl & GB_GPU_CTL_BKGD_SET)) {
            sprite = bkgd_tiles[x_pix / 8];
        } else {
            sprite = *(int8_t *)&bkgd_tiles[x_pix / 8] + 256;
        }

        lo = gpu->vram.seg.sprites[sprite][tile_offset_byte * 2] & (1 << (7 - (x_pix % 8)));
        hi = gpu->vram.seg.sprites[sprite][tile_offset_byte * 2 + 1] & (1 << (7 - (x_pix % 8)));

        c = ((lo)? 1: 0) + ((hi)? 2: 0);

        gpu->bkgd_line_colors[i] = c;

        /* Pull the actual color out of the background palette
         * We shift our color on the palette by 'c', moving two bits for every
         * color, so our target color end's up at the bottom, and then mask out
         * the rest */

        pal_col = (gpu->back_palette >> (c * 2)) & 0x03;

        /*
        if ((i % 8) == 0)
            line[i].i_color = 0xFF00FF00;
        else */
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

    if (gpu->window_y > gpu->cur_line)
        return ;

    line = &gpu->screenbuf[gpu->cur_line * GB_SCREEN_WIDTH];

    bkgd_y_pix = gpu->cur_line - gpu->window_y;

    tile = (bkgd_y_pix / 8) * 32;
    bkgd_tiles = gpu->vram.seg.bkgd[(gpu->ctl & GB_GPU_CTL_WINDOW_MAP)? 1: 0] + tile;

    tile_offset_byte = bkgd_y_pix % 8;

    for (i = 0; i < GB_SCREEN_WIDTH; i++) {
        uint8_t c, pal_col;
        uint8_t lo, hi;
        int sprite;
        int x_pix;

        x_pix = i - gpu->window_x + 7;

        if (x_pix < 0 || x_pix >= GB_SCREEN_WIDTH)
            continue;

        if (gpu->ctl & GB_GPU_CTL_BKGD_SET)
            sprite = bkgd_tiles[x_pix / 8];
        else
            sprite = *(int8_t *)&bkgd_tiles[x_pix / 8] + 256;

        lo = gpu->vram.seg.sprites[sprite][tile_offset_byte * 2] & (1 << (7 - (x_pix % 8)));
        hi = gpu->vram.seg.sprites[sprite][tile_offset_byte * 2 + 1] & (1 << (7 - (x_pix % 8)));

        c = ((lo)? 1: 0) | ((hi)? 2: 0);

        gpu->bkgd_line_colors[i] = c;

        /* Pull the actual color out of the background palette
         * We shift our color on the palette by 'c', moving two bits for every
         * color, so our target color end's up at the bottom, and then mask out
         * the rest */

        pal_col = (gpu->back_palette >> (c * 2)) & 0x03;

        /*
        if ((i % 8) == 0)
            line[i].i_color = 0xFFFF0000;
        else */
            line[i] = gb_colors[pal_col];
    }
}

static void render_single_sprite(struct gb_gpu *gpu, union gb_gpu_color_u *line, int x, int y, uint8_t tile_no, uint8_t flags)
{
    int flip_x, flip_y, pal_num, behind_bg;
    uint8_t tile_hi, tile_lo;
    uint8_t palette;
    int y_off = gpu->cur_line - y;
    int x_loc = 0;
    int sprite_size = 8;

    if (gpu->ctl & GB_GPU_CTL_SPRITES_SIZE)
        sprite_size = 16;

    flip_x = !!(flags & GB_GPU_SPRITE_FLAG_X_FLIP);
    flip_y = !!(flags & GB_GPU_SPRITE_FLAG_Y_FLIP);
    pal_num = !!(flags & GB_GPU_SPRITE_FLAG_PAL_NUM);
    behind_bg = !!(flags & GB_GPU_SPRITE_FLAG_BEHIND_BG);

    if (!flip_y) {
        tile_lo = gpu->vram.seg.sprites[tile_no][y_off * 2];
        tile_hi = gpu->vram.seg.sprites[tile_no][y_off * 2 + 1];
    } else {
        tile_lo = gpu->vram.seg.sprites[tile_no][(sprite_size - 1 - y_off) * 2];
        tile_hi = gpu->vram.seg.sprites[tile_no][(sprite_size - 1 - y_off) * 2 + 1];
    }

    palette = gpu->obj_pal[pal_num];

    for (x_loc = 0; x_loc < 8; x_loc++) {
        int x_pix = ((!flip_x)? 7 - x_loc: x_loc);
        int sel, col;

        if (x_loc + x >= GB_SCREEN_WIDTH || x_loc + x < 0)
            continue;

        sel = !!(tile_lo & (1 << x_pix));
        sel |= (!!(tile_hi & (1 << x_pix))) << 1;

        if (sel == 0)
            continue;

        if (behind_bg && gpu->bkgd_line_colors[x + x_loc] != 0)
            continue;

        col = (palette >> (sel * 2)) & 0x03;
        line[x + x_loc] = gb_colors[col];
    }
}

static void render_sprites(struct gb_gpu *gpu)
{
    union gb_gpu_color_u *line;
    int s;
    int current_line = gpu->cur_line; /* Line being rendered */
    int count = 0;
    int sprite_size = 8;

    if (gpu->ctl & GB_GPU_CTL_SPRITES_SIZE)
        sprite_size = 16;

    line = &gpu->screenbuf[current_line * GB_SCREEN_WIDTH];

    for (s = 0; s < 40; s++) {
        uint8_t *sprite = &gpu->oam.s_attrs[s][0];
        int x, y;
        uint8_t attr_tile, attr_flags;

        y = (int)sprite[GB_GPU_SPRITE_ATTR_Y] - 16;
        x = (int)sprite[GB_GPU_SPRITE_ATTR_X] - 8;
        attr_tile = sprite[GB_GPU_SPRITE_ATTR_TILE_NUM];
        attr_flags = sprite[GB_GPU_SPRITE_ATTR_FLAGS];

        /* Sprites of size 16 span over two tiles - always an even-odd pair with the last bit.
         * This is true regardless of if the tile number given is the even or
         * odd one, so we mask off the last bit to get the first tile in the pair. */
        if (sprite_size == 16)
            attr_tile &= 0xFE;

        if (y >= GB_SCREEN_HEIGHT || y <= -sprite_size)
            continue;

        if (current_line < y || current_line >= y + sprite_size)
            continue;

        count++;

        if (count == 11)
            break;

        if (x + 8 == 0 || x >= GB_SCREEN_WIDTH)
            continue;

        render_single_sprite(gpu, line, x, y, attr_tile, attr_flags);
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

void gb_gpu_update_key_line(struct gb_emu *emu)
{
    struct gb_gpu *gpu = &emu->gpu;

    if (gpu->old_keypad.key_a != gpu->keypad.key_a
        || gpu->old_keypad.key_b != gpu->keypad.key_b
        || gpu->old_keypad.key_up != gpu->keypad.key_up
        || gpu->old_keypad.key_down != gpu->keypad.key_down
        || gpu->old_keypad.key_right != gpu->keypad.key_right
        || gpu->old_keypad.key_left != gpu->keypad.key_left
        || gpu->old_keypad.key_start != gpu->keypad.key_start
        || gpu->old_keypad.key_select != gpu->keypad.key_select)
        emu->cpu.int_flags |= (1 << GB_INT_JOYPAD);

    gpu->old_keypad = gpu->keypad;

    if (gpu->key_select & 0x10) {
        gpu->key_line = gpu->keypad.key_a
            | (gpu->keypad.key_b << 1)
            | (gpu->keypad.key_select << 2)
            | (gpu->keypad.key_start << 3);
    } else if (gpu->key_select & 0x20) {
        gpu->key_line = gpu->keypad.key_right
            | (gpu->keypad.key_left << 1)
            | (gpu->keypad.key_up << 2)
            | (gpu->keypad.key_down << 3);
    } else {
        gpu->key_line = 0;
    }
}

void gb_gpu_display_screen(struct gb_emu *emu, struct gb_gpu *gpu)
{
    (gpu->display->get_keystate) (gpu->display, &gpu->keypad);
    gb_gpu_update_key_line(emu);

    (gpu->display->disp_buf) (gpu->display, gpu->screenbuf);
}

void gb_gpu_ctl_change(struct gb_emu *emu, struct gb_gpu *gpu, uint8_t new_ctl)
{
    uint8_t diff = gpu->ctl ^ new_ctl;

    if (diff & GB_GPU_CTL_DISPLAY) {
        memset(gpu->screenbuf, 0x00, sizeof(gpu->screenbuf));
        gb_gpu_display_screen(emu, gpu);

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

    if (!(gpu->ctl & GB_GPU_CTL_DISPLAY)) {
        gpu->clock = 0;
        gpu->mode = GB_GPU_MODE_HBLANK;
        return ;
    }

    gpu->clock += 4;

    switch (gpu->mode) {
    case GB_GPU_MODE_HBLANK:
        if (gpu->clock >= GB_GPU_CLOCK_HBLANK) {
            gpu->clock = 0;
            gb_gpu_inc_line(emu);

            if (gpu->cur_line == GB_SCREEN_HEIGHT) {
                gpu->mode = GB_GPU_MODE_VBLANK;
                gb_gpu_display_screen(emu, gpu);

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
            if (gpu->cur_line >= GB_SCREEN_HEIGHT + GB_GPU_VBLANK_LENGTH) {
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
    uint16_t src_start = ((int)dma_addr) << 8;
    int i;

    for (i = 0; i < 0xA0; i++)
        emu->gpu.oam.mem[i] = gb_emu_read8(emu, src_start + i);
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
        return 0xFF;
}

uint16_t gb_gpu_vram_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (emu->gpu.mode != GB_GPU_MODE_VRAM) {
        uint16_t ret;
        memcpy(&ret, emu->gpu.vram.mem + addr, sizeof(ret));
        return ret;
    } else {
        return 0xFFFF;
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
        return 0xFF;
}

uint16_t gb_gpu_sprite_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (emu->gpu.mode != GB_GPU_MODE_VRAM && emu->gpu.mode != GB_GPU_MODE_OAM) {
        uint16_t ret;
        memcpy(&ret, emu->gpu.oam.mem + addr, sizeof(ret));
        return ret;
    } else {
        return 0xFFFF;
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

