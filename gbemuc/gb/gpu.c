
#include "common.h"

#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "gb/cpu.h"
#include "gb/gpu.h"
#include "gb.h"
#include "debug.h"

union gb_gpu_color_u gb_colors[][4] = {
    {
        { .i_color = 0xFFFFF77B },
        { .i_color = 0xFFB5AE4A },
        { .i_color = 0xFF6B6931 },
        { .i_color = 0xFF212010 },
    },
    {
        { .i_color = 0xFFFFFFFF },
        { .i_color = 0xFFAAAAAA },
        { .i_color = 0xFF555555 },
        { .i_color = 0xFF000000 },
    },
    {
        { .i_color = 0xFF9BBC0F },
        { .i_color = 0xFF8BAC0F },
        { .i_color = 0xFF305230 },
        { .i_color = 0xFF0F380F },
    },
};

static uint32_t make_cgb_argb_color(uint16_t old_color, int use_real_colors)
{
    uint32_t col = 0xFF000000;
    int r, g, b;
    uint16_t color = use_real_colors? cgb_color_palette[old_color & 0x7FFF]: old_color;

    r = color & 0x1F;
    g = (color >> 5) & 0x1F;
    b = (color >> 10) & 0x1F;

    col |= (r << 3) << 16;
    col |= (g << 3) << 8;
    col |= (b << 3);

    return col;
}

static void render_background_tile(struct gb_emu *emu, struct gb_gpu *gpu, union gb_gpu_color_u *line,
        uint8_t *bkgd_tiles, uint8_t *bkgd_attributes, int x_pix, int y_pix, int tile_offset_byte)
{
    uint8_t c, pal_col;
    uint8_t lo, hi;
    int sprite;
    uint8_t attr;
    uint8_t tile_index;
    int x_flip = 0, y_flip = 0, vbank = 0, priority = 0, cgb_palette = 0;

    if ((gpu->ctl & GB_GPU_CTL_BKGD_SET)) {
        sprite = bkgd_tiles[x_pix / 8];
        attr = bkgd_attributes[x_pix / 8];
    } else {
        sprite = *(int8_t *)&bkgd_tiles[x_pix / 8] + 256;
        attr = *(int8_t *)&bkgd_attributes[x_pix / 8] + 256;
    }

    if (gb_emu_is_cgb(emu)) {
        x_flip = !!(attr & GB_GPU_CGB_BG_ATTR_X_FLIP);
        y_flip = !!(attr & GB_GPU_CGB_BG_ATTR_Y_FLIP);
        vbank = !!(attr & GB_GPU_CGB_BG_ATTR_VBANK);
        priority = !!(attr & GB_GPU_CGB_BG_ATTR_PRIORITY);
        cgb_palette = attr & GB_GPU_CGB_BG_ATTR_PAL_NO;
    }

    if (y_flip)
        tile_offset_byte = 7 - tile_offset_byte;

    tile_index = 7 - (x_pix % 8);
    if (x_flip)
        tile_index = 7 - tile_index;

    lo = gpu->vram[vbank].seg.sprites[sprite][tile_offset_byte * 2] & (1 << tile_index);
    hi = gpu->vram[vbank].seg.sprites[sprite][tile_offset_byte * 2 + 1] & (1 << tile_index);

    c = ((lo)? 1: 0) + ((hi)? 2: 0);

    gpu->bkgd_line_colors[y_pix] = c;
    gpu->bkgd_priority[y_pix] = priority;

    /* Pull the actual color out of the background palette
     * We shift our color on the palette by 'c', moving two bits for every
     * color, so our target color end's up at the bottom, and then mask out
     * the rest */

    if (!gb_emu_is_cgb(emu)) {
        pal_col = (gpu->back_palette >> (c * 2)) & 0x03;
        line[y_pix] = emu->gpu.display->dmg_theme.bg[pal_col];
    } else {
        uint8_t low = gpu->cgb_bkgd_palette[cgb_palette * 8 + c * 2];
        uint8_t high = gpu->cgb_bkgd_palette[cgb_palette * 8 + c * 2 + 1];
        uint16_t color = low | (high << 8);
        line[y_pix].i_color = make_cgb_argb_color(color, emu->config.cgb_real_colors);
    }
}

static void render_background(struct gb_emu *emu, struct gb_gpu *gpu)
{
    union gb_gpu_color_u *line;
    int bkgd_y_pix;
    int tile_offset_byte;
    int i;
    uint8_t *bkgd_tiles;
    uint8_t *bkgd_attributes;
    int tile;

    line = &gpu->screenbuf[gpu->cur_line * GB_SCREEN_WIDTH];

    bkgd_y_pix = (gpu->cur_line + gpu->scroll_y) % (8 * 32);

    tile = (bkgd_y_pix) / 8 * 32;
    bkgd_tiles = gpu->vram[0].seg.bkgd[(gpu->ctl & GB_GPU_CTL_BKGD_MAP)? 1: 0] + tile;
    bkgd_attributes = gpu->vram[1].seg.bkgd[(gpu->ctl & GB_GPU_CTL_BKGD_MAP)? 1: 0] + tile;

    tile_offset_byte = bkgd_y_pix % 8;

    for (i = 0; i < GB_SCREEN_WIDTH; i++) {
        uint8_t x_pix;

        x_pix = (i + gpu->scroll_x) % (8 * 32);

        render_background_tile(emu, gpu, line, bkgd_tiles, bkgd_attributes, x_pix, i, tile_offset_byte);
    }
}

static void render_window(struct gb_emu *emu, struct gb_gpu *gpu)
{
    union gb_gpu_color_u *line;
    int bkgd_y_pix;
    int tile_offset_byte;
    int i;
    uint8_t *bkgd_tiles;
    uint8_t *bkgd_attributes;
    int tile;

    if (gpu->window_y > gpu->cur_line)
        return ;

    line = &gpu->screenbuf[gpu->cur_line * GB_SCREEN_WIDTH];

    bkgd_y_pix = gpu->cur_line - gpu->window_y;

    tile = (bkgd_y_pix / 8) * 32;
    bkgd_tiles      = gpu->vram[0].seg.bkgd[(gpu->ctl & GB_GPU_CTL_WINDOW_MAP)? 1: 0] + tile;
    bkgd_attributes = gpu->vram[1].seg.bkgd[(gpu->ctl & GB_GPU_CTL_WINDOW_MAP)? 1: 0] + tile;

    tile_offset_byte = bkgd_y_pix % 8;

    for (i = 0; i < GB_SCREEN_WIDTH; i++) {
        int x_pix;

        x_pix = i - gpu->window_x + 7;

        if (x_pix < 0 || x_pix >= GB_SCREEN_WIDTH)
            continue;

        render_background_tile(emu, gpu, line, bkgd_tiles, bkgd_attributes, x_pix, i, tile_offset_byte);
    }
}

static void render_single_sprite(struct gb_emu *emu, struct gb_gpu *gpu, union gb_gpu_color_u *line, int x, int y, uint8_t tile_no, uint8_t flags, int *sprite_priority_map)
{
    int flip_x, flip_y, pal_num, behind_bg;
    uint8_t tile_hi, tile_lo;
    uint8_t palette, cgb_palette = 0, vram_bank = 0;
    int y_off = gpu->cur_line - y;
    int x_loc = 0;
    int sprite_size = 8;

    if (gpu->ctl & GB_GPU_CTL_SPRITES_SIZE)
        sprite_size = 16;

    flip_x = !!(flags & GB_GPU_SPRITE_FLAG_X_FLIP);
    flip_y = !!(flags & GB_GPU_SPRITE_FLAG_Y_FLIP);
    pal_num = !!(flags & GB_GPU_SPRITE_FLAG_PAL_NUM);
    behind_bg = !!(flags & GB_GPU_SPRITE_FLAG_BEHIND_BG);

    if (gb_emu_is_cgb(emu)) {
        cgb_palette = flags & GB_GPU_SPRITE_FLAG_CGB_PAL;
        vram_bank = !!(flags & GB_GPU_SPRITE_FLAG_CGB_VBANK);
    }

    if (!flip_y) {
        tile_lo = gpu->vram[vram_bank].seg.sprites[tile_no][y_off * 2];
        tile_hi = gpu->vram[vram_bank].seg.sprites[tile_no][y_off * 2 + 1];
    } else {
        tile_lo = gpu->vram[vram_bank].seg.sprites[tile_no][(sprite_size - 1 - y_off) * 2];
        tile_hi = gpu->vram[vram_bank].seg.sprites[tile_no][(sprite_size - 1 - y_off) * 2 + 1];
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

        if (gb_emu_is_cgb(emu) && gpu->bkgd_priority[x + x_loc])
            continue;

        /* This marks off the places we have already drawn a sprite on for this
         * line. The first sprites take priority over the last sprites, so if
         * we have already put a sprite at this pixel then we don't ever draw
         * over it. */
        if (sprite_priority_map[x + x_loc])
            continue;
        else
            sprite_priority_map[x + x_loc] = 1;

        if (!gb_emu_is_cgb(emu)) {
            col = (palette >> (sel * 2)) & 0x03;
            line[x + x_loc] = emu->gpu.display->dmg_theme.sprites[pal_num][col];
        } else {
            uint8_t low = gpu->cgb_sprite_palette[cgb_palette * 8 + sel * 2];
            uint8_t high = gpu->cgb_sprite_palette[cgb_palette * 8 + sel * 2 + 1];
            uint16_t color = low | (high << 8);
            line[x + x_loc].i_color = make_cgb_argb_color(color, emu->config.cgb_real_colors);
        }
    }
}

static void render_sprites(struct gb_emu *emu, struct gb_gpu *gpu)
{
    union gb_gpu_color_u *line;
    int s;
    int current_line = gpu->cur_line; /* Line being rendered */
    int count = 0;
    int sprite_size = 8;
    int sprite_priority_map[GB_SCREEN_WIDTH] = { 0 };

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

        render_single_sprite(emu, gpu, line, x, y, attr_tile, attr_flags, sprite_priority_map);
    }
}

void gb_gpu_render_line(struct gb_emu *emu, struct gb_gpu *gpu)
{

    if (!(gpu->ctl & GB_GPU_CTL_DISPLAY))
        return ;

    if (gpu->ctl & GB_GPU_CTL_BKGD)
        render_background(emu, gpu);

    if (gpu->ctl & GB_GPU_CTL_WINDOW)
        render_window(emu, gpu);

    if (gpu->ctl & GB_GPU_CTL_SPRITES)
        render_sprites(emu, gpu);
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

    /* This is a convient time to write the save-file if the eram is modified,
     * since it ensure we only do it occasionally and not on every eram write. */
    if (emu->mmu.eram_was_touched) {
        emu->mmu.eram_was_touched = 0;
        gb_emu_write_save(emu);
    }
}

void gb_gpu_ctl_change(struct gb_emu *emu, struct gb_gpu *gpu, uint8_t new_ctl)
{
    uint8_t diff = gpu->ctl ^ new_ctl;

    if (diff & GB_GPU_CTL_DISPLAY) {
        int i, j;
        for (i = 0; i < GB_SCREEN_HEIGHT; i++) {
            for (j = 0; j < GB_SCREEN_WIDTH; j++) {
                if (gb_emu_is_cgb(emu))
                    gpu->screenbuf[i * GB_SCREEN_WIDTH + j].i_color = make_cgb_argb_color(0xFFFF, emu->config.cgb_real_colors);
                else
                    gpu->screenbuf[i * GB_SCREEN_WIDTH + j] = gpu->display->dmg_theme.bg[0];
            }
        }
        gb_gpu_display_screen(emu, gpu);

        gpu->cur_line = 0;
        gpu->mode = GB_GPU_MODE_VBLANK;
    }

    gpu->ctl = new_ctl;
}

static void gb_gpu_inc_line(struct gb_emu *emu)
{
    emu->gpu.cur_line++;

    if ((emu->gpu.status & GB_GPU_STATUS_CONC_INT)
        && emu->gpu.cur_line == emu->gpu.cur_line_cmp)
        emu->cpu.int_flags |= (1 << GB_INT_LCD_STAT);
}


void gb_emu_gpu_tick(struct gb_emu *emu, int cycles)
{
    struct gb_gpu *gpu = &emu->gpu;

    if (!(gpu->ctl & GB_GPU_CTL_DISPLAY)) {
        gpu->mode = GB_GPU_MODE_HBLANK;
        gpu->clock += 4;

        /* We keep counting the cycles even when the display is off to keep the
         * timing right - display the screen also delays the emulation speed */
        if (gpu->clock >= GB_GPU_CLOCK_VBLANK * GB_GPU_VBLANK_LENGTH
                          + (GB_GPU_CLOCK_HBLANK + GB_GPU_CLOCK_OAM + GB_GPU_CLOCK_VRAM) * GB_SCREEN_HEIGHT) {
            gb_gpu_display_screen(emu, gpu);
            gpu->clock = 0;
	        gpu->frame_is_done = 1;
        }
        return ;
    }

    gpu->clock += cycles;

    switch (gpu->mode) {
    case GB_GPU_MODE_HBLANK:
        if (gpu->clock >= GB_GPU_CLOCK_HBLANK) {
            gpu->clock = 0;
            gb_gpu_inc_line(emu);

            if (gpu->cur_line == GB_SCREEN_HEIGHT) {
                gpu->mode = GB_GPU_MODE_VBLANK;
                gb_gpu_display_screen(emu, gpu);
	            gpu->frame_is_done = 1;

                emu->cpu.int_flags |= (1 << GB_INT_VBLANK);

                if (gpu->status & GB_GPU_STATUS_VBLANK_INT)
                    emu->cpu.int_flags |= (1 << GB_INT_LCD_STAT);
            } else {
                gpu->mode = GB_GPU_MODE_OAM;

                if (gpu->status & GB_GPU_STATUS_OAM_INT)
                    emu->cpu.int_flags |= (1 << GB_INT_LCD_STAT);
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

            if (gpu->status & GB_GPU_STATUS_HBLANK_INT)
                emu->cpu.int_flags |= (1 << GB_INT_LCD_STAT);

            gb_gpu_render_line(emu, gpu);
        }
        break;

    case GB_GPU_MODE_VBLANK:
        if (gpu->clock >= GB_GPU_CLOCK_VBLANK) {
            gpu->clock = 0;
            gb_gpu_inc_line(emu);
            if (gpu->cur_line >= GB_SCREEN_HEIGHT + GB_GPU_VBLANK_LENGTH) {
                gpu->mode = GB_GPU_MODE_OAM;
                gpu->cur_line = 0;

                if (gpu->status & GB_GPU_STATUS_OAM_INT)
                    emu->cpu.int_flags |= (1 << GB_INT_LCD_STAT);

                if ((emu->gpu.status & GB_GPU_STATUS_CONC_INT)
                    && emu->gpu.cur_line == emu->gpu.cur_line_cmp)
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

void gb_gpu_init(struct gb_gpu *gpu)
{
    memset(gpu, 0, sizeof(*gpu));
    memset(gpu->screenbuf, 0x00, sizeof(gpu->screenbuf));

    gpu->keypad.key_a = 1;
    gpu->keypad.key_b = 1;
    gpu->keypad.key_up = 1;
    gpu->keypad.key_down = 1;
    gpu->keypad.key_left= 1;
    gpu->keypad.key_right = 1;
    gpu->keypad.key_start = 1;
    gpu->keypad.key_select = 1;
}

uint8_t gb_gpu_vram_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    //if (emu->gpu.mode != GB_GPU_MODE_VRAM)
        return emu->gpu.vram[emu->gpu.cgb_vram_bank_no].mem[addr];
    //else
    //    return 0xFF;
}

void gb_gpu_vram_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t byte)
{
    //if (emu->gpu.mode != GB_GPU_MODE_VRAM)
        emu->gpu.vram[emu->gpu.cgb_vram_bank_no].mem[addr] = byte;
    //else
    //    printf("Invalid VBANK write\n");
}

uint8_t gb_gpu_sprite_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (emu->gpu.mode != GB_GPU_MODE_VRAM && emu->gpu.mode != GB_GPU_MODE_OAM)
        return emu->gpu.oam.mem[addr];
    else
        return 0xFF;
}

void gb_gpu_sprite_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t byte)
{
    if (emu->gpu.mode != GB_GPU_MODE_VRAM && emu->gpu.mode != GB_GPU_MODE_OAM)
        emu->gpu.oam.mem[addr] = byte;
}

