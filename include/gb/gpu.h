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

#define GB_VRAM_BKGD_START_1 0x9800
#define GB_VRAM_BKGD_START_2 0x9C00

#define GB_IO_GPU_CTL 0xFF40

#define GB_IO_GPU_STATUS 0xFF41

#define GB_IO_GPU_SCRY 0xFF42
#define GB_IO_GPU_SCRX 0xFF43

#define GB_IO_GPU_LY 0xFF44
#define GB_IO_GPU_LYC 0xFF45

#define GB_IO_GPU_PALETTE 0xFF47

#define GB_IO_GPU_WY 0xFF4A
#define GB_IO_GPU_WX 0xFF4B

#define GB_IO_GPU_DMA 0xFF46

#define GB_IO_KEYPAD 0xFF00

#define GB_GPU_CLOCK_HBLANK 204
#define GB_GPU_CLOCK_VBLANK 456
#define GB_GPU_CLOCK_OAM    80
#define GB_GPU_CLOCK_VRAM   172

#define GB_GPU_VBLANK_LENGTH 10

struct gb_gpu_color {
    uint8_t a, r, b, g;
};

union gb_gpu_color_u {
    struct gb_gpu_color color;
    uint32_t i_color;
};

struct gb_dmg_theme {
    union gb_gpu_color_u bg[4];
    union gb_gpu_color_u sprites[2][4];
};

#define GB_DEFINE_THEME(bg1, bg2, bg3, bg4, sp11, sp12, sp13, sp14, sp21, sp22, sp23, sp24) \
    { \
        { \
            { .i_color = bg1 }, \
            { .i_color = bg2 }, \
            { .i_color = bg3 }, \
            { .i_color = bg4 }, \
        }, \
        { \
            { \
                { .i_color = sp11 }, \
                { .i_color = sp12 }, \
                { .i_color = sp13 }, \
                { .i_color = sp14 }, \
            }, \
            { \
                { .i_color = sp21 }, \
                { .i_color = sp22 }, \
                { .i_color = sp23 }, \
                { .i_color = sp24 }, \
            }, \
        }, \
    }

struct gb_keypad {
    int key_a;
    int key_b;
    int key_up;
    int key_down;
    int key_left;
    int key_right;
    int key_start;
    int key_select;
};

struct gb_gpu_display {
    void (*disp_buf) (struct gb_gpu_display *, union gb_gpu_color_u *buf);
    void (*get_keystate) (struct gb_gpu_display *, struct gb_keypad *keys);

    struct gb_dmg_theme dmg_theme;
};

extern union gb_gpu_color_u gb_colors[][4];

enum gb_gpu_mode {
    GB_GPU_MODE_HBLANK = 0,
    GB_GPU_MODE_VBLANK = 1,
    GB_GPU_MODE_OAM = 2,
    GB_GPU_MODE_VRAM = 3,
};

#define GB_GPU_SPRITE_ATTR_Y 0
#define GB_GPU_SPRITE_ATTR_X 1
#define GB_GPU_SPRITE_ATTR_TILE_NUM 2
#define GB_GPU_SPRITE_ATTR_FLAGS 3

#define GB_GPU_SPRITE_FLAG_CGB_PAL   (0x07)
#define GB_GPU_SPRITE_FLAG_CGB_VBANK (1 << 3)

#define GB_GPU_SPRITE_FLAG_PAL_NUM (1 << 4)
#define GB_GPU_SPRITE_FLAG_X_FLIP  (1 << 5)
#define GB_GPU_SPRITE_FLAG_Y_FLIP  (1 << 6)
#define GB_GPU_SPRITE_FLAG_BEHIND_BG (1 << 7)

#define GB_GPU_STATUS_CONC_INT (1 << 6)
#define GB_GPU_STATUS_OAM_INT (1 << 5)
#define GB_GPU_STATUS_VBLANK_INT (1 << 4)
#define GB_GPU_STATUS_HBLANK_INT (1 << 3)

#define GB_GPU_CGB_PAL_INDEX_AUTO_INCREMENT (1 << 7)

#define GB_GPU_CGB_BG_ATTR_PAL_NO   (0x07)
#define GB_GPU_CGB_BG_ATTR_VBANK    (1 << 3)
#define GB_GPU_CGB_BG_ATTR_X_FLIP   (1 << 5)
#define GB_GPU_CGB_BG_ATTR_Y_FLIP   (1 << 6)
#define GB_GPU_CGB_BG_ATTR_PRIORITY (1 << 7)

#define GB_IO_OBJ_PAL1 0xFF48
#define GB_IO_OBJ_PAL2 0xFF49

#define GB_IO_CGB_VRAM_BANK_NO 0xFF4F

#define GB_IO_CGB_BG_PAL_INDEX 0xFF68
#define GB_IO_CGB_BG_PAL_DATA  0xFF69

#define GB_IO_CGB_SPRITE_PAL_INDEX 0xFF6A
#define GB_IO_CGB_SPRITE_PAL_DATA  0xFF6B

struct gb_gpu {
    union gb_gpu_color_u screenbuf[GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH];

    int clock;

    enum gb_gpu_mode mode;

    uint8_t ctl, status;
    uint8_t scroll_x, scroll_y;
    uint8_t window_x, window_y;
    uint8_t cur_line;
    uint8_t cur_line_cmp;

    struct gb_keypad keypad, old_keypad;
    uint8_t key_line;
    uint8_t key_select;
    uint8_t back_palette;
    uint8_t obj_pal[2];

    int cgb_vram_bank_no;
    union {
        uint8_t mem[8 * 0x0400];
        struct {
            uint8_t sprites[384][16]; /* 256 + 128 sprites */
            uint8_t bkgd[2][0x0400]; /* 2K of background info */
        } seg;
    } vram[2];

    union {
        uint8_t mem[0xA0];
        uint8_t s_attrs[40][4]; /* Sprite attributes */
    } oam;

    int cgb_bkgd_palette_index;
    uint8_t cgb_bkgd_palette[64];

    int cgb_sprite_palette_index;
    uint8_t cgb_sprite_palette[64];

    uint8_t bkgd_line_colors[GB_SCREEN_WIDTH]; /* Temporary buffer holding current background colors */
    uint8_t bkgd_priority[GB_SCREEN_WIDTH]; /* 1 bit indicates BG/window priority */

    struct gb_gpu_display *display;
};

void gb_emu_gpu_tick(struct gb_emu *, int cycles);

void gb_gpu_init(struct gb_gpu *);
void gb_gpu_display_screen(struct gb_emu *emu, struct gb_gpu *gpu);
void gb_gpu_ctl_change(struct gb_emu *emu, struct gb_gpu *gpu, uint8_t new_ctl);
void gb_gpu_dma(struct gb_emu *emu, uint8_t dma_addr);
void gb_gpu_update_key_line(struct gb_emu *emu);

uint8_t gb_gpu_vram_read8(struct gb_emu *, uint16_t addr, uint16_t low);
void gb_gpu_vram_write8(struct gb_emu *, uint16_t addr, uint16_t low, uint8_t byte);

uint8_t gb_gpu_sprite_read8(struct gb_emu *, uint16_t addr, uint16_t low);
void gb_gpu_sprite_write8(struct gb_emu *, uint16_t addr, uint16_t low, uint8_t byte);

void gb_apu_end_sound_frame(struct gb_emu *emu);

extern const uint16_t cgb_color_palette[32 * 32 * 32];

#endif
