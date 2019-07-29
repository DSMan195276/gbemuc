#ifndef INCLUDE_Z80_ROM_H
#define INCLUDE_Z80_ROM_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Note - The header information is pulled straight out of the `data` field
 * when the ROM is opened. */
struct gb_rom {
    char ninten_logo[48];
    char title[16];
    char man_code[4];
    uint8_t cgb_flag;
    uint8_t title_chksum;

    char new_license_code[2];
    uint8_t sgb_code;
    uint8_t cart_type;

    uint8_t rom_size;
    uint8_t ram_size;

    /* 1 for japan, 0 for everywhere else */
    uint8_t dest_code;

    /* Value of 0x33 means use new_license_code */
    uint8_t old_license_code;

    uint8_t rom_version;

    uint8_t header_checksum;
    uint16_t global_checksum;

    size_t length;
    char *data;

    /* If specified, this is used rather then the default of "game_filename.sav" */
    const char *sav_filename;
};

enum gb_dest_code {
    GB_DEST_JAPAN = 0,
    GB_DEST_EVERWHERE_ELSE = 1
};

enum gb_cgb_code {
    GB_CGB_NONE     = 0x00,
    GB_CGB_OPTIONAL = 0x80,
    GB_CGB_REQUIRED = 0xC0,
};

enum gb_sgb_code {
    GB_SGB_NONE = 0,
    GB_SGB_SUPPORT = 3,
};

enum {
    GB_HAS_NEW_LICENSE = 0x33,
};

enum {
    GB_CART_ROM,
    GB_CART_MBC1,
    GB_CART_MBC2,
    GB_CART_MBC3,
    GB_CART_MBC4,
    GB_CART_MBC5,
    GB_CART_RAM,
    GB_CART_TIMER,
    GB_CART_BATTERY,
    GB_CART_MMM01,
    GB_CART_RUMBLE,
    GB_CART_POCKET_CAMERA,
    GB_CART_BANDAI_TAMA5,
    GB_CART_HuC3,
    GB_CART_HuC1,
    GB_CART_TOTAL
};

#define GB_CART_FLAG(x) (1 << GB_CART_ ## x)

extern const char gb_nintendo_logo[];
extern int gb_cart_type_bitmap[];
extern const char *gb_cart_types_str[];

extern size_t gb_rom_size[];
extern size_t gb_ram_size[];

void gb_rom_open(struct gb_rom *rom, const char *filename);
void gb_rom_dump_header(struct gb_rom *rom, FILE *file);
int gb_rom_header_checksum(struct gb_rom *rom);

static inline void gb_rom_init(struct gb_rom *rom)
{
    memset(rom, 0, sizeof(*rom));
}

void gb_rom_clear(struct gb_rom *rom);

#endif
