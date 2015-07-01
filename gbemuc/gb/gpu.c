
#include "common.h"

#include <stdint.h>
#include <string.h>

#include "gb/gpu.h"
#include "gb.h"

union gb_gpu_color_u gb_colors[] =
{
    { .i_color = 0x000000FF },
    { .i_color = 0x808080FF },
    { .i_color = 0xC0C0C0FF },
    { .i_color = 0xFFFFFFFF },
};

void gb_gpu_render_line(struct gb_gpu *gpu)
{

}

void gb_gpu_display_screen(struct gb_gpu *gpu)
{
    (gpu->display->disp_buf) (gpu->display, gpu->screenbuf);
}

void gb_emu_gpu_tick(struct gb_emu *emu)
{
    struct gb_gpu *gpu = &emu->gpu;

    gpu->clock += 4;

    switch (gpu->mode) {
    case GB_GPU_MODE_HBLANK:
        if (gpu->clock >= GB_GPU_CLOCK_HBLANK) {
            gpu->clock = 0;
            gpu->cur_line++;

            if (gpu->cur_line == GB_SCREEN_HEIGHT - 1) {
                gpu->mode = GB_GPU_MODE_VBLANK;
                gb_gpu_display_screen(gpu);
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
            gpu->cur_line++;
            if (gpu->cur_line >= GB_SCREEN_HEIGHT + GB_GPU_VBLANK_LENGTH - 1) {
                gpu->mode = GB_GPU_MODE_OAM;
                gpu->cur_line = 0;
            }
        }
        break;
    }
}

void gb_gpu_init(struct gb_gpu *gpu, struct gb_gpu_display *display)
{
    memset(gpu, 0, sizeof(*gpu));

    memset(gpu->screenbuf, 0x80, sizeof(gpu->screenbuf));
    gpu->display = display;
}

