
#include "common.h"

#include <stdint.h>
#include <string.h>

#include "slist.h"
#include "gb.h"
#include "gb/io.h"
#include "gb/cpu.h"
#include "gb/gpu.h"
#include "gb/mmu.h"
#include "debug.h"

static uint8_t gb_bios[] = {
    0x31, 0xfe, 0xff, 0xaf, 0x21, 0xff, 0x9f, 0x32, 0xcb, 0x7c,
    0x20, 0xfb, 0x21, 0x26, 0xff, 0x0e, 0x11, 0x3e, 0x80, 0x32,
    0xe2, 0x0c, 0x3e, 0xf3, 0xe2, 0x32, 0x3e, 0x77, 0x77, 0x3e,
    0xfc, 0xe0, 0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1a,
    0xcd, 0x95, 0x00, 0xcd, 0x96, 0x00, 0x13, 0x7b, 0xfe, 0x34,
    0x20, 0xf3, 0x11, 0xd8, 0x00, 0x06, 0x08, 0x1a, 0x13, 0x22,
    0x23, 0x05, 0x20, 0xf9, 0x3e, 0x19, 0xea, 0x10, 0x99, 0x21,
    0x2f, 0x99, 0x0e, 0x0c, 0x3d, 0x28, 0x08, 0x32, 0x0d, 0x20,
    0xf9, 0x2e, 0x0f, 0x18, 0xf3, 0x67, 0x3e, 0x64, 0x57, 0xe0,
    0x42, 0x3e, 0x91, 0xe0, 0x40, 0x04, 0x1e, 0x02, 0x0e, 0x0c,
    0xf0, 0x44, 0xfe, 0x90, 0x20, 0xfa, 0x0d, 0x20, 0xf7, 0x1d,
    0x20, 0xf2, 0x0e, 0x13, 0x24, 0x7c, 0x1e, 0x83, 0xfe, 0x62,
    0x28, 0x06, 0x1e, 0xc1, 0xfe, 0x64, 0x20, 0x06, 0x7b, 0xe2,
    0x0c, 0x3e, 0x87, 0xe2, 0xf0, 0x42, 0x90, 0xe0, 0x42, 0x15,
    0x20, 0xd2, 0x05, 0x20, 0x4f, 0x16, 0x20, 0x18, 0xcb, 0x4f,
    0x06, 0x04, 0xc5, 0xcb, 0x11, 0x17, 0xc1, 0xcb, 0x11, 0x17,
    0x05, 0x20, 0xf5, 0x22, 0x23, 0x22, 0x23, 0xc9, 0xce, 0xed,
    0x66, 0x66, 0xcc, 0x0d, 0x00, 0x0b, 0x03, 0x73, 0x00, 0x83,
    0x00, 0x0c, 0x00, 0x0d, 0x00, 0x08, 0x11, 0x1f, 0x88, 0x89,
    0x00, 0x0e, 0xdc, 0xcc, 0x6e, 0xe6, 0xdd, 0xdd, 0xd9, 0x99,
    0xbb, 0xbb, 0x67, 0x63, 0x6e, 0x0e, 0xec, 0xcc, 0xdd, 0xdc,
    0x99, 0x9f, 0xbb, 0xb9, 0x33, 0x3e, 0x3c, 0x42, 0xb9, 0xa5,
    0xb9, 0xa5, 0x42, 0x3c, 0x21, 0x04, 0x01, 0x11, 0xa8, 0x00,
    0x1a, 0x13, 0xbe, 0x20, 0xfe, 0x23, 0x7d, 0xfe, 0x34, 0x20,
    0xf5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xfb, 0x86,
    0x20, 0xfe, 0x3e, 0x01, 0xe0, 0x50
};

/*
 *
 * ROM Bank 0
 *
 */

static uint8_t bank0_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (emu->mmu.bios_flag || addr > 0x0100)
        return emu->rom.data[addr];
    else {
        DEBUG_PRINTF("BIOS READ: 0x%04x\n", addr);
        return gb_bios[addr];
    }
}

static uint16_t bank0_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    uint16_t ret;
    /* memcpy avoids aliasing issues */
    if (emu->mmu.bios_flag || addr > 0x0100)
        memcpy(&ret, emu->rom.data + addr, sizeof(ret));
    else {
        DEBUG_PRINTF("BIOS READ16: 0x%04x\n", addr);
        memcpy(&ret, gb_bios + addr, sizeof(ret));
    }

    return ret;
}

static void bank0_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    /* NOP */
}

static void bank0_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t val)
{
    /* NOP */
}

/*
 *
 * Working RAM
 *
 */

static uint8_t wram_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    return emu->wram[addr];
}

static uint16_t wram_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    uint16_t ret;
    memcpy(&ret, emu->wram + addr, sizeof(ret));
    return ret;
}

static void wram_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    DEBUG_PRINTF("Writing 0x%04x to 0x%02x\n", addr, val);
    emu->wram[addr] = val;
}

static void wram_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t val)
{
    DEBUG_PRINTF("Writing 0x%04x to 0x%04x\n", addr, val);
    memcpy(emu->wram + addr, &val, sizeof(val));
}

/*
 *
 * Z-RAM
 *
 */

static uint8_t zram_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    return emu->zram[addr];
}

static uint16_t zram_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    uint16_t ret;
    memcpy(&ret, emu->zram + addr, sizeof(ret));
    return ret;
}

static void zram_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    emu->zram[addr] = val;
}

static void zram_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t val)
{
    DEBUG_PRINTF("Writing 0x%04x to 0x%04x\n", val, addr);
    memcpy(emu->zram + addr, &val, sizeof(val));
}

/*
 *
 * zero-only empty memory
 *
 */

static uint8_t zero_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    return 0;
}

static uint16_t zero_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    return 0;
}

static void zero_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    /* NOP */
}

static void zero_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t val)
{
    /* NOP */
}

static struct gb_mmu_entry mmu_entries[] = {
    { 0x0000, 0x3FFF,
        bank0_read8, bank0_read16, bank0_write8, bank0_write16 }, /*
    { 0x4000, 0x7FFF,
        bank1_read8, bank1_read16, bank1_write8, bank1_write16 }, */
    { 0x8000, 0x9FFF,
        gb_gpu_vram_read8, gb_gpu_vram_read16, gb_gpu_vram_write8, gb_gpu_vram_write16 },
    /* { 0xA000, 0xBFFF,
        cram_read8, cram_read16, cram_write8, cram_write16 }, */
    { 0xC000, 0xDFFF,
        wram_read8, wram_read16, wram_write8, wram_write16 },
    { 0xE000, 0xFDFF,
        wram_read8, wram_read16, wram_write8, wram_write16 },
    { 0xFE00, 0xFE9F,
        gb_gpu_sprite_read8, gb_gpu_sprite_read16, gb_gpu_sprite_write8, gb_gpu_sprite_write16 },
    { 0xFEA0, 0xFEFF,
        zero_read8, zero_read16, zero_write8, zero_write16 },
    { 0xFF00, 0xFF7F,
        gb_emu_io_read8, zero_read16, gb_emu_io_write8, zero_write16 },
    { 0xFF80, 0xFFFE,
        zram_read8, zram_read16, zram_write8, zram_write16 },
    /*{ 0xFFFF, 0xFFFF,
        gb_cpu_int_read8, gb_cpu_int_read16, gb_cpu_int_write8, gb_cpu_int_write16 }, */
};

uint8_t gb_emu_read8(struct gb_emu *emu, uint16_t addr)
{
    int i;

    for (i = 0; i < sizeof(mmu_entries)/sizeof(*mmu_entries); i++)
        if (addr >= mmu_entries[i].low && addr <= mmu_entries[i].high)
            return (mmu_entries[i].read8) (emu, addr - mmu_entries[i].low, mmu_entries[i].low);

    return 0;
}

uint16_t gb_emu_read16(struct gb_emu *emu, uint16_t addr)
{
    int i;

    for (i = 0; i < sizeof(mmu_entries)/sizeof(*mmu_entries); i++)
        if (addr >= mmu_entries[i].low && addr <= mmu_entries[i].high)
            return (mmu_entries[i].read16) (emu, addr - mmu_entries[i].low, mmu_entries[i].low);

    return 0;
}

void gb_emu_write8(struct gb_emu *emu, uint16_t addr, uint8_t byte)
{
    int i;

    DEBUG_PRINTF("WRITING 0x%04x\n", addr);

    for (i = 0; i < sizeof(mmu_entries)/sizeof(*mmu_entries); i++) {
        if (addr >= mmu_entries[i].low && addr <= mmu_entries[i].high) {
            return (mmu_entries[i].write8) (emu, addr - mmu_entries[i].low, mmu_entries[i].low, byte);
        }
    }
}

void gb_emu_write16(struct gb_emu *emu, uint16_t addr, uint16_t word)
{
    int i;

    DEBUG_PRINTF("WRITING 0x%04x\n", addr);

    for (i = 0; i < sizeof(mmu_entries)/sizeof(*mmu_entries); i++) {
        DEBUG_PRINTF("Checking 0x%04x to 0x%04x\n", mmu_entries[i].low, mmu_entries[i].high);
        if (addr >= mmu_entries[i].low && addr <= mmu_entries[i].high) {
            DEBUG_PRINTF("FOUND!\n");
            return (mmu_entries[i].write16) (emu, addr - mmu_entries[i].low, mmu_entries[i].low, word);
        }
    }
}

uint8_t gb_emu_next_pc8(struct gb_emu *emu)
{
    uint16_t addr = emu->cpu.r.w[GB_REG_PC];
    emu->cpu.r.w[GB_REG_PC]++;

    return gb_emu_read8(emu, addr);
}

uint16_t gb_emu_next_pc16(struct gb_emu *emu)
{
    uint16_t addr = emu->cpu.r.w[GB_REG_PC];
    emu->cpu.r.w[GB_REG_PC] += 2;

    return gb_emu_read16(emu, addr);
}


