#ifndef INCLUDE_GB_GPU_H
#define INCLUDE_GB_GPU_H

#include <stdint.h>

struct gb_emu;

enum {
    GB_SCREEN_WIDTH = 160,
    GB_SCREEN_HEIGHT = 144,
};

#define GB_GPU_CTL_BKGD         0x01
#define GB_GPU_CTL_SPRITES      0x02
#define GB_GPU_CTL_SPRITES_SIZE 0x04
#define GB_GPU_CTL_BKGD_MAP     0x08
#define GB_GPU_CTL_BKGD_SET     0x10
#define GB_GPU_CTL_WINDOW       0x20
#define GB_GPU_CTL_WINDOW_MAP   0x40
#define GB_GPU_CTL_DISPLAY      0x80

#define GB_IO_GPU_CTL 0xFF40

#define GB_IO_GPU_STATUS 0xFF41

#define GB_IO_GPU_SCRY 0xFF42
#define GB_IO_GPU_SCRX 0xFF43

#define GB_GPU_CLOCK_HBLANK 204
#define GB_GPU_CLOCK_VBLANK 456
#define GB_GPU_CLOCK_OAM    80
#define GB_GPU_CLOCK_VRAM   172

#define GB_GPU_VBLANK_LENGTH 10

struct gb_gpu_color {
    uint8_t r, b, g, a;
};

union gb_gpu_color_u {
    struct gb_gpu_color color;
    uint32_t i_color;
};


struct gb_gpu_display {
    void (*disp_buf) (struct gb_gpu_display *, union gb_gpu_color_u *buf);
};

extern union gb_gpu_color_u gb_colors[];

enum gb_gpu_mode {
    GB_GPU_MODE_HBLANK = 0,
    GB_GPU_MODE_VBLANK = 1,
    GB_GPU_MODE_OAM = 2,
    GB_GPU_MODE_VRAM = 3,
};

struct gb_gpu {
    union gb_gpu_color_u screenbuf[GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH];

    int clock;

    enum gb_gpu_mode mode;

    uint8_t ctl, status;
    uint8_t scroll_x, scroll_y;
    uint8_t cur_line;
    uint8_t back_palette;

    struct gb_gpu_display *display;
};

void gb_emu_gpu_tick(struct gb_emu *);

void gb_gpu_init(struct gb_gpu *, struct gb_gpu_display *display);
void gb_gpu_display_screen(struct gb_gpu *gpu);

#endif
