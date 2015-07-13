
#include "common.h"

#include <stdint.h>
#include <string.h>

#include "gb/gpu.h"
#include "gb.h"
#include "debug.h"

union gb_gpu_color_u gb_colors[] =
{
    { .i_color = 0xFF000000 },
    { .i_color = 0xFF808080 },
    { .i_color = 0xFFC0C0C0 },
    { .i_color = 0xFFFFFFFF },
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

    /*
    printf("BKGD MAP: %d\n", gpu->ctl & GB_GPU_CTL_BKGD_MAP);
    printf("Line: %d\n", gpu->cur_line);
    printf("Scroll y: %d\n", gpu->scroll_y);
    printf("BKGD Y-Pixel: %d\n", bkgd_y_pix);
    printf("BKGD tile row: %d\n", tile);
    printf("BKGD addr: 0x%04x\n", 0x9800 + tile); */

    for (i = 0; i < GB_SCREEN_WIDTH; i++) {
        uint8_t c;
        uint8_t lo, hi;
        uint8_t sprite;

        sprite = bkgd_tiles[i / 8];
        lo = (gpu->vram.seg.sprites[sprite][tile_offset_byte * 2]) & (i % 8);
        hi = (gpu->vram.seg.sprites[sprite][tile_offset_byte * 2 + 1]) & (i % 8);

        c = ((lo)? 1: 0) + ((hi)? 2: 0);

        /* printf("%d ", c); */

        line[i] = gb_colors[c];
    }

    /* putchar('\n'); */
}

void gb_gpu_render_line(struct gb_gpu *gpu)
{

    if (!(gpu->ctl & GB_GPU_CTL_DISPLAY))
        return ;

    if (gpu->ctl & GB_GPU_CTL_BKGD)
        render_background(gpu);

    if (gpu->ctl & GB_GPU_CTL_WINDOW) {

    }

    if (gpu->ctl & GB_GPU_CTL_SPRITES) {

    }
}

void gb_gpu_display_screen(struct gb_gpu *gpu)
{
    (gpu->display->disp_buf) (gpu->display, gpu->screenbuf);
}

void gb_gpu_ctl_change(struct gb_gpu *gpu, uint8_t new_ctl)
{
    uint8_t diff = gpu->ctl ^ new_ctl;

    DEBUG_PRINTF("New ctl: 0x%02x\n", new_ctl);

    if (diff & GB_GPU_CTL_DISPLAY) {
        if (!(new_ctl & GB_GPU_CTL_DISPLAY)) {
            memset(gpu->screenbuf, 0x00, sizeof(gpu->screenbuf));
            gb_gpu_display_screen(gpu);
        } else {
            memset(gpu->screenbuf, 0x00, sizeof(gpu->screenbuf));
            gb_gpu_display_screen(gpu);
        }
        printf("VRAM: \n");
        dump_mem(&gpu->vram.mem, 8 * 0x0400, 0x8000);
        gpu->cur_line = 0;
        gpu->mode = GB_GPU_MODE_HBLANK;
    }

    gpu->ctl = new_ctl;
}

void gb_emu_gpu_tick(struct gb_emu *emu)
{
    struct gb_gpu *gpu = &emu->gpu;

    gpu->clock += 4;

    DEBUG_PRINTF("GPU TICK: 0x%2x\n", gpu->ctl);

    if (!(gpu->ctl & GB_GPU_CTL_BKGD))
        return ;

    switch (gpu->mode) {
    case GB_GPU_MODE_HBLANK:
        if (gpu->clock >= GB_GPU_CLOCK_HBLANK) {
            gpu->clock = 0;
            gpu->cur_line++;

            if (gpu->cur_line == GB_SCREEN_HEIGHT - 1) {
                gpu->mode = GB_GPU_MODE_VBLANK;
                DEBUG_PRINTF("VBLANK\n");
                gb_gpu_display_screen(gpu);
            } else {
                DEBUG_PRINTF("OAM\n");
                gpu->mode = GB_GPU_MODE_OAM;
            }
        }
        break;

    case GB_GPU_MODE_OAM:
        if (gpu->clock >= GB_GPU_CLOCK_OAM) {
            gpu->clock = 0;
            gpu->mode = GB_GPU_MODE_VRAM;
            DEBUG_PRINTF("VRAM\n");
        }
        break;

    case GB_GPU_MODE_VRAM:
        if (gpu->clock >= GB_GPU_CLOCK_VRAM) {
            gpu->clock = 0;
            gpu->mode = GB_GPU_MODE_HBLANK;
            DEBUG_PRINTF("HBLANK\n");

            gb_gpu_render_line(gpu);
        }
        break;

    case GB_GPU_MODE_VBLANK:
        if (gpu->clock >= GB_GPU_CLOCK_VBLANK) {
            gpu->clock = 0;
            gpu->cur_line++;
            if (gpu->cur_line >= GB_SCREEN_HEIGHT + GB_GPU_VBLANK_LENGTH - 1) {
                gpu->mode = GB_GPU_MODE_OAM;
                DEBUG_PRINTF("OAM\n");
                gpu->cur_line = 0;
            }
        }
        break;
    }
}

void gb_gpu_init(struct gb_gpu *gpu, struct gb_gpu_display *display)
{
    memset(gpu, 0, sizeof(*gpu));

    memset(gpu->screenbuf, 0x00, sizeof(gpu->screenbuf));
    gpu->display = display;
}

uint8_t gb_gpu_vram_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    return emu->gpu.vram.mem[addr];
}

uint16_t gb_gpu_vram_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    uint16_t ret;
    ret = *(uint16_t *)(emu->gpu.vram.mem + addr);
    return ret;
}

void gb_gpu_vram_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t byte)
{
    emu->gpu.vram.mem[addr] = byte;
}

void gb_gpu_vram_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t word)
{
    *(uint16_t *)(emu->gpu.vram.mem + addr) = word;
}

uint8_t gb_gpu_sprite_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    return emu->gpu.oam.mem[addr];
}

uint16_t gb_gpu_sprite_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    uint16_t ret;
    ret = *(uint16_t *)(emu->gpu.oam.mem + addr);
    return ret;
}

void gb_gpu_sprite_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t byte)
{
    emu->gpu.oam.mem[addr] = byte;
}

void gb_gpu_sprite_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t word)
{
    *(uint16_t *)(emu->gpu.oam.mem + addr) = word;
}

