
#include "common.h"

#include "gb/gpu.h"
#include "gb/cgb_themes.h"

struct cgb_dmg_theme {
    uint8_t chksum;
    uint8_t title_4;
    struct gb_dmg_theme theme;
};

struct cgb_dmg_theme cgb_themes2[] = {
    {
        .chksum = 0xFF,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFF9C00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF9C00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF9C00, 0xFFFF0000, 0xFF000000),
    },
    {
        .chksum = 0xFF,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFF9C00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF9C00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF9C00, 0xFFFF0000, 0xFF000000),
    },
    {
        .chksum = 0x71,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFF9C00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF9C00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF9C00, 0xFFFF0000, 0xFF000000),
    },
    {
        .chksum = 0xDB,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000),
    },
    {
        .chksum = 0x15,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000),
    },
    {
        .chksum = 0x88,
        .theme = GB_DEFINE_THEME(0xFFA59CFF, 0xFFFFFF00, 0xFF006300, 0xFF000000,
                                 0xFFA59CFF, 0xFFFFFF00, 0xFF006300, 0xFF000000,
                                 0xFFA59CFF, 0xFFFFFF00, 0xFF006300, 0xFF000000),
    },
    {
        .chksum = 0x16,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000),
    },
    {
        .chksum = 0x92,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000),
    },
    {
        .chksum = 0x35,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000),
    },
    {
        .chksum = 0x75,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000),
    },
    {
        .chksum = 0x99,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000),
    },
    {
        .chksum = 0x0C,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000),
    },
    {
        .chksum = 0xB7,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000),
    },
    {
        .chksum = 0x67,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000),
    },
    {
        .chksum = 0xE8,
        .theme = GB_DEFINE_THEME(0xFF000000, 0xFF008484, 0xFFFFDE00, 0xFFFFFFFF,
                                 0xFF000000, 0xFF008484, 0xFFFFDE00, 0xFFFFFFFF,
                                 0xFF000000, 0xFF008484, 0xFFFFDE00, 0xFFFFFFFF),
    },
    {
        .chksum = 0x28,
        .title_4 = 0x41,
        .theme = GB_DEFINE_THEME(0xFF000000, 0xFF008484, 0xFFFFDE00, 0xFFFFFFFF,
                                 0xFF000000, 0xFF008484, 0xFFFFDE00, 0xFFFFFFFF,
                                 0xFF000000, 0xFF008484, 0xFFFFDE00, 0xFFFFFFFF),
    },
    {
        .chksum = 0xA5,
        .title_4 = 0x41,
        .theme = GB_DEFINE_THEME(0xFF000000, 0xFF008484, 0xFFFFDE00, 0xFFFFFFFF,
                                 0xFF000000, 0xFF008484, 0xFFFFDE00, 0xFFFFFFFF,
                                 0xFF000000, 0xFF008484, 0xFFFFDE00, 0xFFFFFFFF),
    },
    {
        .chksum = 0x58,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFA5A5A5, 0xFF525252, 0xFF000000,
                                 0xFFFFFFFF, 0xFFA5A5A5, 0xFF525252, 0xFF000000,
                                 0xFFFFFFFF, 0xFFA5A5A5, 0xFF525252, 0xFF000000),
    },
    {
        .chksum = 0x6F,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFCE00, 0xFF9C6300, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFCE00, 0xFF9C6300, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFCE00, 0xFF9C6300, 0xFF000000),
    },
    {
        .chksum = 0x8C,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFADAD84, 0xFF42737B, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF7300, 0xFF944200, 0xFF000000,
                                 0xFFFFFFFF, 0xFFADAD84, 0xFF42737B, 0xFF000000),
    },
    {
        .chksum = 0x61,
        .title_4 = 0x45,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0xD3,
        .title_4 = 0x52,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF8C8CDE, 0xFF52528C, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFF8C8CDE, 0xFF52528C, 0xFF000000),
    },
    {
        .chksum = 0x14,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0xAA,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF0063C5, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF0063C5, 0xFF000000),
    },
    {
        .chksum = 0x3C,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x9C,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF8C8CDE, 0xFF52528C, 0xFF000000,
                                 0xFFFFFFFF, 0xFF8C8CDE, 0xFF52528C, 0xFF000000,
                                 0xFFFFC542, 0xFFFFD600, 0xFF943A00, 0xFF4A0000),
    },
    {
        .chksum = 0xB3,
        .title_4 = 0x55,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFADAD84, 0xFF42737B, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF7300, 0xFF944200, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF7300, 0xFF944200, 0xFF000000),
    },
    {
        .chksum = 0x34,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF00, 0xFFB57300, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x66,
        .title_4 = 0x45,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF00, 0xFFB57300, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0xF4,
        .title_4 = 0x20,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF00, 0xFFB57300, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x3D,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF52FF00, 0xFFFF4200, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x6A,
        .title_4 = 0x49,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF52FF00, 0xFFFF4200, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x19,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFF9C00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x1D,
        .theme = GB_DEFINE_THEME(0xFFA59CFF, 0xFFFFFF00, 0xFF006300, 0xFF000000,
                                 0xFFFF6352, 0xFFD60000, 0xFF630000, 0xFF000000,
                                 0xFFFF6352, 0xFFD60000, 0xFF630000, 0xFF000000),
    },
    {
        .chksum = 0x46,
        .title_4 = 0x45,
        .theme = GB_DEFINE_THEME(0xFFB5B5FF, 0xFFFFFF94, 0xFFAD5A42, 0xFF000000,
                                 0xFF000000, 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A,
                                 0xFF000000, 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A),
    },
    {
        .chksum = 0x0D,
        .title_4 = 0x45,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF8C8CDE, 0xFF52528C, 0xFF000000,
                                 0xFFFFC542, 0xFFFFD600, 0xFF943A00, 0xFF4A0000,
                                 0xFFFFC542, 0xFFFFD600, 0xFF943A00, 0xFF4A0000),
    },
    {
        .chksum = 0xBF,
        .title_4 = 0x20,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF8C8CDE, 0xFF52528C, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x4B,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x90,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x9A,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0xBD,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x28,
        .title_4 = 0x46,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x97,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000),
    },
    {
        .chksum = 0x39,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000),
    },
    {
        .chksum = 0x43,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000),
    },
    {
        .chksum = 0xA5,
        .title_4 = 0x52,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000),
    },
    {
        .chksum = 0x00,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF0063C5, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x3F,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF0063C5, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0xC6,
        .title_4 = 0x20,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF0063C5, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x18,
        .title_4 = 0x49,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF0063C5, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x66,
        .title_4 = 0x66,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF0063C5, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x95,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF52FF00, 0xFFFF4200, 0xFF000000,
                                 0xFFFFFFFF, 0xFF52FF00, 0xFFFF4200, 0xFF000000,
                                 0xFFFFFFFF, 0xFF5ABDFF, 0xFFFF0000, 0xFF0000FF),
    },
    {
        .chksum = 0xB3,
        .title_4 = 0x52,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF52FF00, 0xFFFF4200, 0xFF000000,
                                 0xFFFFFFFF, 0xFF52FF00, 0xFFFF4200, 0xFF000000,
                                 0xFFFFFFFF, 0xFF5ABDFF, 0xFFFF0000, 0xFF0000FF),
    },
    {
        .chksum = 0x3E,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFF9C00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF9C00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFF5ABDFF, 0xFFFF0000, 0xFF0000FF),
    },
    {
        .chksum = 0xE0,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFF9C00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF9C00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFF5ABDFF, 0xFFFF0000, 0xFF0000FF),
    },
    {
        .chksum = 0xF2,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFF5ABDFF, 0xFFFF0000, 0xFF0000FF),
    },
    {
        .chksum = 0x69,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFF5ABDFF, 0xFFFF0000, 0xFF0000FF),
    },
    {
        .chksum = 0x0D,
        .title_4 = 0x52,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFFF00, 0xFFFF0000, 0xFF000000,
                                 0xFFFFFFFF, 0xFF5ABDFF, 0xFFFF0000, 0xFF0000FF),
    },
    {
        .chksum = 0x59,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFADAD84, 0xFF42737B, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF7300, 0xFF944200, 0xFF000000,
                                 0xFFFFFFFF, 0xFF5ABDFF, 0xFFFF0000, 0xFF0000FF),
    },
    {
        .chksum = 0xC6,
        .title_4 = 0x41,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFADAD84, 0xFF42737B, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF7300, 0xFF944200, 0xFF000000,
                                 0xFFFFFFFF, 0xFF5ABDFF, 0xFFFF0000, 0xFF0000FF),
    },
    {
        .chksum = 0xA8,
        .theme = GB_DEFINE_THEME(0xFFFFFF9C, 0xFF94B5FF, 0xFF639473, 0xFF003A3A,
                                 0xFFFFC542, 0xFFFFD600, 0xFF943A00, 0xFF4A0000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x86,
        .theme = GB_DEFINE_THEME(0xFFFFFF9C, 0xFF94B5FF, 0xFF639473, 0xFF003A3A,
                                 0xFFFFC542, 0xFFFFD600, 0xFF943A00, 0xFF4A0000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0xD1,
        .theme = GB_DEFINE_THEME(0xFF6BFF00, 0xFFFFFFFF, 0xFFFF524A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000),
    },
    {
        .chksum = 0xF0,
        .theme = GB_DEFINE_THEME(0xFF6BFF00, 0xFFFFFFFF, 0xFFFF524A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000),
    },
    {
        .chksum = 0xCE,
        .theme = GB_DEFINE_THEME(0xFF6BFF00, 0xFFFFFFFF, 0xFFFF524A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000),
    },
    {
        .chksum = 0xBF,
        .title_4 = 0x43,
        .theme = GB_DEFINE_THEME(0xFF6BFF00, 0xFFFFFFFF, 0xFFFF524A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000),
    },
    {
        .chksum = 0x36,
        .theme = GB_DEFINE_THEME(0xFF52DE00, 0xFFFF8400, 0xFFFFFF00, 0xFFFFFFFF,
                                 0xFFFFFFFF, 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000),
    },
    {
        .chksum = 0x5C,
        .theme = GB_DEFINE_THEME(0xFFA59CFF, 0xFFFFFF00, 0xFF006300, 0xFF000000,
                                 0xFFFF6352, 0xFFD60000, 0xFF630000, 0xFF000000,
                                 0xFF0000FF, 0xFFFFFFFF, 0xFFFFFF7B, 0xFF0084FF),
    },
    {
        .chksum = 0x49,
        .theme = GB_DEFINE_THEME(0xFFA59CFF, 0xFFFFFF00, 0xFF006300, 0xFF000000,
                                 0xFFFF6352, 0xFFD60000, 0xFF630000, 0xFF000000,
                                 0xFF0000FF, 0xFFFFFFFF, 0xFFFFFF7B, 0xFF0084FF),
    },
    {
        .chksum = 0xB3,
        .title_4 = 0x42,
        .theme = GB_DEFINE_THEME(0xFFA59CFF, 0xFFFFFF00, 0xFF006300, 0xFF000000,
                                 0xFFFF6352, 0xFFD60000, 0xFF630000, 0xFF000000,
                                 0xFF0000FF, 0xFFFFFFFF, 0xFFFFFF7B, 0xFF0084FF),
    },
    {
        .chksum = 0x27,
        .title_4 = 0x42,
        .theme = GB_DEFINE_THEME(0xFFA59CFF, 0xFFFFFF00, 0xFF006300, 0xFF000000,
                                 0xFFFF6352, 0xFFD60000, 0xFF630000, 0xFF000000,
                                 0xFF0000FF, 0xFFFFFFFF, 0xFFFFFF7B, 0xFF0084FF),
    },
    {
        .chksum = 0xC9,
        .theme = GB_DEFINE_THEME(0xFFFFFFCE, 0xFF63EFEF, 0xFF9C8431, 0xFF5A5A5A,
                                 0xFFFFFFFF, 0xFFFF7300, 0xFF944200, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000),
    },
    {
        .chksum = 0x4E,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFFF7B, 0xFF0084FF, 0xFFFF0000),
    },
    {
        .chksum = 0x6B,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF8C8CDE, 0xFF52528C, 0xFF000000,
                                 0xFFFFC542, 0xFFFFD600, 0xFF943A00, 0xFF4A0000,
                                 0xFFFFFFFF, 0xFF5ABDFF, 0xFFFF0000, 0xFF0000FF),
    },
    {
        .chksum = 0x18,
        .title_4 = 0x4B,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF8C8CDE, 0xFF52528C, 0xFF000000,
                                 0xFFFFC542, 0xFFFFD600, 0xFF943A00, 0xFF4A0000,
                                 0xFFFFFFFF, 0xFF5ABDFF, 0xFFFF0000, 0xFF0000FF),
    },
    {
        .chksum = 0x6A,
        .title_4 = 0x4B,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF8C8CDE, 0xFF52528C, 0xFF000000,
                                 0xFFFFC542, 0xFFFFD600, 0xFF943A00, 0xFF4A0000,
                                 0xFFFFFFFF, 0xFF5ABDFF, 0xFFFF0000, 0xFF0000FF),
    },
    {
        .chksum = 0x9D,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF8C8CDE, 0xFF52528C, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000),
    },
    {
        .chksum = 0x17,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000),
    },
    {
        .chksum = 0x8B,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000),
    },
    {
        .chksum = 0x27,
        .title_4 = 0x4E,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000),
    },
    {
        .chksum = 0x61,
        .title_4 = 0x41,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000),
    },
    {
        .chksum = 0x10,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000),
    },
    {
        .chksum = 0xF6,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000),
    },
    {
        .chksum = 0x68,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000),
    },
    {
        .chksum = 0x29,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000),
    },
    {
        .chksum = 0x52,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000),
    },
    {
        .chksum = 0x01,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000),
    },
    {
        .chksum = 0x5D,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000),
    },
    {
        .chksum = 0x6D,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000),
    },
    {
        .chksum = 0x70,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFF00FF00, 0xFF318400, 0xFF004A00,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000),
    },
    {
        .chksum = 0xF7,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000),
    },
    {
        .chksum = 0xA2,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000),
    },
    {
        .chksum = 0x46,
        .title_4 = 0x52,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000,
                                 0xFFFFFF00, 0xFFFF0000, 0xFF630000, 0xFF000000,
                                 0xFFFFFFFF, 0xFF7BFF31, 0xFF008400, 0xFF000000),
    },
    {
        .chksum = 0xD3,
        .title_4 = 0x49,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFFADAD84, 0xFF42737B, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFFAD63, 0xFF843100, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000),
    },
    {
        .chksum = 0xF4,
        .title_4 = 0x2D,
        .theme = GB_DEFINE_THEME(0xFFFFFFFF, 0xFF7BFF31, 0xFF0063C5, 0xFF000000,
                                 0xFFFFFFFF, 0xFFFF8484, 0xFF943A3A, 0xFF000000,
                                 0xFFFFFFFF, 0xFF63A5FF, 0xFF0000FF, 0xFF000000),
    },
};

int cgb_find_theme(struct gb_dmg_theme *theme, char *title, uint8_t chksum)
{
    uint8_t title_4 = title[3]; /* Character 4 - zero based */
    struct cgb_dmg_theme *cur_theme = NULL;
    int i;

    for (i = 0; i < sizeof(cgb_themes2) / sizeof(*cgb_themes2); i++) {
        if (cgb_themes2[i].chksum == chksum &&
            ((!cur_theme && cgb_themes2[i].title_4 == 0)
             || (cgb_themes2[i].title_4 == title_4)))
            cur_theme = cgb_themes2 + i;
    }

    if (cur_theme) {
        *theme = cur_theme->theme;
        return 1;
    }

    return 0;
}

