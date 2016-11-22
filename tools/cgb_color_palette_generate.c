
/* 
 * This utility program generates the table of CGB colors used to render the CGB screen.
 *
 * The CGB screen does weird color mixing which results in a non-linear colorspace.
 *
 * IE. Simply taking the 5-bit color values and directly scaling them to range
 * from 0 to 255 does not result in an accurate looking render.
 *
 * Tihs generates 16-bit colors that do map to a linear colorspace, making them
 * easy to render.
 */

#include <stdint.h>
#include <stdio.h>

/*
 * Color Palette mixing code comes from VBA-M source code (GPLv2)
 *
 * This takes the linear 15-bit colors from the CGB, and mixes them to appear
 * as they did on the original CGb. Without this, colors appear wrong.
 */

uint16_t cgb_color_palette[32 * 32 * 32];

static inline int gbGetValue(int min,int max,int v)
{
  return (int)(min + (float)(max - min) * (2.0 * (v / 31.0) - ( v / 31.0) * ( v / 31.0)));
}

int main(int argc, char **argv)
{
    int r, g, b;

    for (r = 0; r < 32; r++) {
        for (g = 0; g < 32; g++) {
            for (b = 0; b < 32; b++) {
                int nr, ng, nb;

                nr = gbGetValue(gbGetValue(4, 14, g),
                                gbGetValue(24, 29, g), r) - 4;

                ng = gbGetValue(gbGetValue(4  + gbGetValue(0, 5, r),
                                           14 + gbGetValue(0, 3, r), b),
                                gbGetValue(24 + gbGetValue(0, 3, r),
                                           29 + gbGetValue(0, 1, r), b), g) - 4;

                nb = gbGetValue(gbGetValue(4  + gbGetValue(0, 5, r),
                                           14 + gbGetValue(0, 3, r), g),
                                gbGetValue(24 + gbGetValue(0, 3, r),
                                           29 + gbGetValue(0, 1, r), g), b) - 4;

                cgb_color_palette[(b << 10) | (g << 5) | r] = (nb << 10) | (ng << 5) | nr;
            }
        }
    }

    printf("const uint16_t cgb_color_palette[32 * 32 * 32] = {\n");
    for (r = 0; r < 32 * 32; r++) {
        printf("    ");
        for (g = 0; g < 32; g++) {
            printf("0x%04x,", cgb_color_palette[r * 32 + g]);
            if ((g % 4) == 3 && g != 31)
                printf("\n    ");
            else if ((g % 4) == 3)
                printf("\n");
            else
                printf(" ");
        }
        printf("\n");
    }

    printf("};\n");

    return 0;
}

