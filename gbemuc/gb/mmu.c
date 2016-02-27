
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

/*
 *
 * Working RAM
 *
 */

static uint8_t wram_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    return emu->mmu.wram[addr];
}

static uint16_t wram_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    uint16_t ret;
    memcpy(&ret, emu->mmu.wram + addr, sizeof(ret));
    return ret;
}

static void wram_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    emu->mmu.wram[addr] = val;
}

static void wram_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t val)
{
    memcpy(emu->mmu.wram + addr, &val, sizeof(val));
}

/*
 *
 * Z-RAM
 *
 */

static uint8_t zram_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    return emu->mmu.zram[addr];
}

static uint16_t zram_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    uint16_t ret;
    memcpy(&ret, emu->mmu.zram + addr, sizeof(ret));
    return ret;
}

static void zram_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    emu->mmu.zram[addr] = val;
}

static void zram_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t val)
{
    memcpy(emu->mmu.zram + addr, &val, sizeof(val));
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

enum {
    GB_MMU_BANK0,
    GB_MMU_BANK1,
    GB_MMU_VRAM,
    GB_MMU_WRAM,
    GB_MMU_WRAM2,
    GB_MMU_SPRITE,
    GB_MMU_EMPTY,
    GB_MMU_IO,
    GB_MMU_ZRAM,
    GB_MMU_INT
};

static struct gb_mmu_entry mmu_entries[] = {
    [GB_MMU_VRAM] =
        { 0x8000, 0x9FFF,
            gb_gpu_vram_read8, gb_gpu_vram_read16, gb_gpu_vram_write8, gb_gpu_vram_write16 },
    /* { 0xA000, 0xBFFF,
        cram_read8, cram_read16, cram_write8, cram_write16 }, */
    [GB_MMU_WRAM] =
        { 0xC000, 0xDFFF,
            wram_read8, wram_read16, wram_write8, wram_write16 },
    [GB_MMU_WRAM2] =
        { 0xE000, 0xFDFF,
            wram_read8, wram_read16, wram_write8, wram_write16 },
    [GB_MMU_SPRITE] =
        { 0xFE00, 0xFE9F,
            gb_gpu_sprite_read8, gb_gpu_sprite_read16, gb_gpu_sprite_write8, gb_gpu_sprite_write16 },
    [GB_MMU_EMPTY] =
        { 0xFEA0, 0xFEFF,
            zero_read8, zero_read16, zero_write8, zero_write16 },
    [GB_MMU_IO] =
        { 0xFF00, 0xFF7F,
            gb_emu_io_read8, zero_read16, gb_emu_io_write8, zero_write16 },
    [GB_MMU_ZRAM] =
        { 0xFF80, 0xFFFE,
            zram_read8, zram_read16, zram_write8, zram_write16 },
    [GB_MMU_INT] =
        { 0xFFFF, 0xFFFF,
            gb_cpu_int_read8, zero_read16, gb_cpu_int_write8, zero_write16 },
};

static inline struct gb_mmu_entry *get_mmu_entry(struct gb_emu *emu, uint16_t addr)
{
    uint8_t a;

    /* The switch-case is much faster then a loop */

    /* Get the high nibble */
    a = addr >> 12;

    switch (a) {
    case 0x0:
    case 0x1:
    case 0x2:
    case 0x3:
    case 0x4:
    case 0x5:
    case 0x6:
    case 0x7:
        return emu->mmu.mbc_controller;

    case 0x8:
    case 0x9:
        return mmu_entries + GB_MMU_VRAM;

    case 0xA:
    case 0xB:
        return emu->mmu.eram_controller;

    case 0xC:
    case 0xD:
        return mmu_entries + GB_MMU_WRAM;

    case 0xE:
        return mmu_entries + GB_MMU_WRAM2;

    case 0xF: {
        uint8_t b = (addr >> 8) & 0xF;
        switch (b) {
        case 0x0 ... 0xD:
            return mmu_entries + GB_MMU_WRAM2;

        case 0xE: {
            uint8_t c = (addr >> 4) & 0xF;
            if (c < 0xa)
                return mmu_entries + GB_MMU_SPRITE;
            else
                return mmu_entries + GB_MMU_EMPTY;
        }

        case 0xF: {
            uint8_t c = (addr >> 4) & 0xF;
            if (c < 0x8)
                return mmu_entries + GB_MMU_IO;
            else if (addr != 0xFFFF)
                return mmu_entries + GB_MMU_ZRAM;
            else
                return mmu_entries + GB_MMU_INT;
        }
        }
        break;
    }

    }

    return NULL;
}

uint8_t gb_emu_read8(struct gb_emu *emu, uint16_t addr)
{
    struct gb_mmu_entry *entry = get_mmu_entry(emu, addr);

    if (entry)
        return (entry->read8) (emu, addr - entry->low, entry->low);
    else
        return 0;
}

uint16_t gb_emu_read16(struct gb_emu *emu, uint16_t addr)
{
    struct gb_mmu_entry *entry = get_mmu_entry(emu, addr);

    if (entry)
        return (entry->read16) (emu, addr - entry->low, entry->low);
    else
        return 0;
}

void gb_emu_write8(struct gb_emu *emu, uint16_t addr, uint8_t byte)
{
    struct gb_mmu_entry *entry = get_mmu_entry(emu, addr);

    if (entry)
        (entry->write8) (emu, addr - entry->low, entry->low, byte);
}

void gb_emu_write16(struct gb_emu *emu, uint16_t addr, uint16_t word)
{
    struct gb_mmu_entry *entry = get_mmu_entry(emu, addr);

    if (entry)
        (entry->write16) (emu, addr - entry->low, entry->low, word);
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


