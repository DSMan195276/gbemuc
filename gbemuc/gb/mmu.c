
#include "common.h"

#include <stdint.h>
#include <string.h>

#include "slist.h"
#include "gb.h"
#include "gb/io.h"
#include "gb/mmu.h"

/*
 *
 * ROM Bank 0
 *
 */

static uint8_t bank0_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    return emu->rom.data[addr];
}

static uint16_t bank0_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    uint16_t ret;
    /* memcpy avoids aliasing issues */
    memcpy(&ret, emu->rom.data + addr, sizeof(ret));
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
    emu->wram[addr] = val;
}

static void wram_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t val)
{
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
        bank1_read8, bank1_read16, bank1_write8, bank1_write16 },
    { 0x8000, 0x9FFF,
        vram_read8, vram_read16, vram_write8, vram_write16 },
    { 0xA000, 0xBFFF,
        cram_read8, cram_read16, cram_write8, cram_write16 }, */
    { 0xC000, 0xDFFF,
        wram_read8, wram_read16, wram_write8, wram_write16 },
    { 0xE000, 0xFDFF,
        wram_read8, wram_read16, wram_write8, wram_write16 },
    /* { 0xFE00, 0xFE9F,
        sprite_read8, sprite_read16, sprite_write8, sprite_write16 }, */
    { 0xFEA0, 0xFEFF,
        zero_read8, zero_read16, zero_write8, zero_write16 },
    { 0xFF00, 0xFF7F,
        gb_emu_io_read8, zero_read16, gb_emu_io_write8, zero_write16 },
    { 0xFF80, 0xFFFF,
        zram_read8, zram_read16, zram_write8, zram_write16 },
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

    for (i = 0; i < sizeof(mmu_entries)/sizeof(*mmu_entries); i++)
        if (addr >= mmu_entries[i].low && addr <= mmu_entries[i].high)
            return (mmu_entries[i].write8) (emu, addr - mmu_entries[i].low, mmu_entries[i].low, byte);
}

void gb_emu_write16(struct gb_emu *emu, uint16_t addr, uint16_t word)
{
    int i;

    for (i = 0; i < sizeof(mmu_entries)/sizeof(*mmu_entries); i++)
        if (addr >= mmu_entries[i].low && addr <= mmu_entries[i].high)
            return (mmu_entries[i].write16) (emu, addr - mmu_entries[i].low, mmu_entries[i].low, word);
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


