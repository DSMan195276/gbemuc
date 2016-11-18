
#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gb/rom.h"

const char gb_nintendo_logo[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC,
    0x0D, 0x00, 0x0B, 0x03, 0x73,
    0x00, 0x83, 0x00, 0x0C, 0x00,
    0x0D, 0x00, 0x08, 0x11, 0x1F,
    0x88, 0x89, 0x00, 0x0E, 0xDC,
    0xCC, 0x6E, 0xE6, 0xDD, 0xDD,
    0xD9, 0x99, 0xBB, 0xBB, 0x67,
    0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB,
    0xB9, 0x33, 0x3E,
};

int gb_cart_type_bitmap[] = {
    [0x00] = GB_CART_FLAG(ROM),
    [0x01] = GB_CART_FLAG(MBC1),
    [0x02] = GB_CART_FLAG(MBC1) | GB_CART_FLAG(RAM),
    [0x03] = GB_CART_FLAG(MBC1) | GB_CART_FLAG(RAM) | GB_CART_FLAG(BATTERY),
    [0x05] = GB_CART_FLAG(MBC2),
    [0x06] = GB_CART_FLAG(MBC2) | GB_CART_FLAG(BATTERY),
    [0x08] = GB_CART_FLAG(ROM) | GB_CART_FLAG(RAM),
    [0x09] = GB_CART_FLAG(ROM) | GB_CART_FLAG(RAM) | GB_CART_FLAG(BATTERY),
    [0x0B] = GB_CART_FLAG(MMM01),
    [0x0C] = GB_CART_FLAG(MMM01) | GB_CART_FLAG(RAM),
    [0x0D] = GB_CART_FLAG(MMM01) | GB_CART_FLAG(RAM) | GB_CART_FLAG(BATTERY),
    [0x0F] = GB_CART_FLAG(MBC3) | GB_CART_FLAG(TIMER) | GB_CART_FLAG(BATTERY),
    [0x10] = GB_CART_FLAG(MBC3) | GB_CART_FLAG(TIMER) | GB_CART_FLAG(RAM) | GB_CART_FLAG(BATTERY),
    [0x11] = GB_CART_FLAG(MBC3),
    [0x12] = GB_CART_FLAG(MBC3) | GB_CART_FLAG(RAM),
    [0x13] = GB_CART_FLAG(MBC3) | GB_CART_FLAG(RAM) | GB_CART_FLAG(BATTERY),
    [0x15] = GB_CART_FLAG(MBC4),
    [0x16] = GB_CART_FLAG(MBC4) | GB_CART_FLAG(RAM),
    [0x17] = GB_CART_FLAG(MBC4) | GB_CART_FLAG(RAM) | GB_CART_FLAG(BATTERY),
    [0x19] = GB_CART_FLAG(MBC5),
    [0x1A] = GB_CART_FLAG(MBC5) | GB_CART_FLAG(RAM),
    [0x1B] = GB_CART_FLAG(MBC5) | GB_CART_FLAG(RAM) | GB_CART_FLAG(BATTERY),
    [0x1C] = GB_CART_FLAG(MBC5) | GB_CART_FLAG(RUMBLE),
    [0x1D] = GB_CART_FLAG(MBC5) | GB_CART_FLAG(RUMBLE) | GB_CART_FLAG(RAM),
    [0x1E] = GB_CART_FLAG(MBC5) | GB_CART_FLAG(RUMBLE) | GB_CART_FLAG(RAM) | GB_CART_FLAG(BATTERY),
    [0xFC] = GB_CART_FLAG(POCKET_CAMERA),
    [0xFD] = GB_CART_FLAG(BANDAI_TAMA5),
    [0xFE] = GB_CART_FLAG(HuC3),
    [0xFF] = GB_CART_FLAG(HuC1) | GB_CART_FLAG(RAM) | GB_CART_FLAG(BATTERY),
};

const char *gb_cart_types_str[] = {
    [GB_CART_ROM] = "ROM",
    [GB_CART_MBC1] = "MBC1",
    [GB_CART_MBC2] = "MBC2",
    [GB_CART_MBC3] = "MBC3",
    [GB_CART_MBC4] = "MBC4",
    [GB_CART_MBC5] = "MBC5",
    [GB_CART_RAM] = "RAM",
    [GB_CART_TIMER] = "TIMER",
    [GB_CART_BATTERY] = "BATTERY",
    [GB_CART_MMM01] = "MMM01",
    [GB_CART_RUMBLE] = "RUMBLE",
    [GB_CART_POCKET_CAMERA] = "POCKET CAMERA",
    [GB_CART_BANDAI_TAMA5] = "BANDAI TAMA5",
    [GB_CART_HuC3]= "HuC3",
    [GB_CART_HuC1] = "HuC1",
};

size_t gb_rom_size[] = {
    [0x00] = 32,
    [0x01] = 64,
    [0x02] = 128,
    [0x03] = 256,
    [0x04] = 512,
    [0x05] = 1024,
    [0x06] = 2048,
    [0x07] = 4096,
    [0x52] = 576,
    [0x53] = 640,
    [0x54] = 768,
};

size_t gb_ram_size[] = {
    [0x00] = 0,
    [0x01] = 2,
    [0x02] = 8,
    [0x03] = 32,
    [0x04] = 128,
    [0x05] = 64,
};

void gb_rom_open(struct gb_rom *rom, const char *filename)
{
    char sav_file[strlen(filename) + 5];
    FILE *f = fopen(filename, "r");

    if (!f)
        return ;

    /* Read length */
    fseek(f, 0, SEEK_END);
    rom->length = ftell(f);
    fseek(f, 0, SEEK_SET);

    /* Read file into 'data' */
    rom->data = malloc(rom->length);
    fread(rom->data, rom->length, 1, f);

    fclose(f);

    /* Read header information */
    memcpy(&rom->ninten_logo,      rom->data + 0x104, 48);
    memcpy(&rom->title,            rom->data + 0x134, 15);
    memcpy(&rom->man_code,         rom->data + 0x13F,  4);
    memcpy(&rom->cgb_flag,         rom->data + 0x143,  1);
    memcpy(&rom->new_license_code, rom->data + 0x144,  2);
    memcpy(&rom->sgb_code,         rom->data + 0x146,  1);
    memcpy(&rom->cart_type,        rom->data + 0x147,  1);
    memcpy(&rom->rom_size,         rom->data + 0x148,  1);
    memcpy(&rom->ram_size,         rom->data + 0x149,  1);
    memcpy(&rom->dest_code,        rom->data + 0x14A,  1);
    memcpy(&rom->old_license_code, rom->data + 0x14B,  1);
    memcpy(&rom->rom_version,      rom->data + 0x14C,  1);
    memcpy(&rom->header_checksum,  rom->data + 0x14D,  1);
    memcpy(&rom->global_checksum,  rom->data + 0x14E,  2);

    snprintf(sav_file, sizeof(sav_file), "%s.sav", filename);
    printf("Sav: %s\n", sav_file);

    rom->sav_file = fopen(sav_file, "r+");
    if (!rom->sav_file)
        rom->sav_file = fopen(sav_file, "w+");
}

void gb_rom_dump_header(struct gb_rom *rom, FILE *file)
{
    uint8_t hchecksum;
    int i, flag = 0;

    if (rom->cgb_flag & 0x80)
        fprintf(file, "TITLE: %.11s\n", rom->title);
    else
        fprintf(file, "TITLE: %.15s\n", rom->title);

    fprintf(file, "SGB: %s\n", (rom->sgb_code)? "Yes": "No");

    fprintf(file, "Cart: (0x%02x)", rom->cart_type);

    /* Print out all the matching cart type options in the bitmap for this
     * cart's type. */
    for (i = 0; i < GB_CART_TOTAL; i++) {
        if (gb_cart_type_bitmap[rom->cart_type] & (1 << i)) {
            if (flag)
                fputc('+', file);

            fprintf(file, "%s", gb_cart_types_str[i]);
            flag = 1;
        }
    }

    fputc('\n', file);

    fprintf(file, "ROM size: %zdKb\n", gb_rom_size[rom->rom_size]);
    fprintf(file, "RAM size: %zdKb\n", gb_ram_size[rom->ram_size]);
    fprintf(file, "Country: %s\n", (rom->dest_code)? "Non-Japan": "Japan");
    fprintf(file, "License: 0x%02x\n", rom->old_license_code);

    if (rom->old_license_code == GB_HAS_NEW_LICENSE)
        fprintf(file, "New License: %.2s\n", rom->new_license_code);

    if (rom->cgb_flag & 0x80) {
        fprintf(file, "Manufacturer code: %.4s\n", rom->man_code);
        fprintf(file, "CGB: %s\n", (rom->cgb_flag == GB_CGB_OPTIONAL)? "Optional": "Required");
    } else {
        fprintf(file, "CGB: No\n");
    }

    fprintf(file, "ROM version: 0x%02x\n", rom->rom_version);

    hchecksum = gb_rom_header_checksum(rom);
    fprintf(file, "ROM header checksum: 0x%02x - %s\n", rom->header_checksum, (hchecksum == rom->header_checksum)? "CORRECT": "WRONG");

    fprintf(file, "ROM global checksum: 0x%04x\n", rom->global_checksum);

    if (memcmp(rom->ninten_logo, gb_nintendo_logo, sizeof(gb_nintendo_logo)) == 0)
        fprintf(file, "Nintendo logo: CORRECT\n");
    else
        fprintf(file, "Nintendo logo: WRONG\n");
}

int gb_rom_header_checksum(struct gb_rom *rom)
{
    uint16_t sum = 0;
    int x;
    for (x = 0x134; x < 0x14D; x++)
        sum = (sum - rom->data[x] - 1) & 0xFF;

    return sum;
}

void gb_rom_clear(struct gb_rom *rom)
{
    if (rom->data)
        free(rom->data);

    if (rom->sav_file)
        fclose(rom->sav_file);
}

