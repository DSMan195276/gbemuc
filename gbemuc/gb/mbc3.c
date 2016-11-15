
#include "common.h"

#include <string.h>

#include "gb.h"
#include "gb/mmu.h"
#include "gb/bios.h"
#include "debug.h"

/*
 *
 * MBC3 ROM
 *
 */

static inline int bank_number(struct gb_emu *emu)
{
    int bank_no = 0;

    bank_no = emu->mmu.mbc3.rom_bank;

    if (bank_no == 0)
        bank_no++;

    return bank_no;
}

static uint8_t mbc3_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (!emu->mmu.bios_flag && addr < 0x0100) {
        return gb_bios[addr];
    } else {
        uint16_t bank_no = 0;
        int data_offset = addr;

        if (addr >= 0x4000) {
            bank_no = bank_number(emu);
            data_offset &= 0x3FFF;
            data_offset += bank_no * 0x4000;
        }

        return emu->rom.data[data_offset];
    }
}

static uint16_t mbc3_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    uint16_t ret;

    if (!emu->mmu.bios_flag && addr < 0x0100) {
        memcpy(&ret, gb_bios + addr, sizeof(ret));
    } else {
        int bank_no = 0;
        int data_offset = addr;

        if (addr >= 0x4000) {
            bank_no = bank_number(emu);
            data_offset &= 0x3FFF;
            data_offset += bank_no * 0x4000;
        }

        memcpy(&ret, emu->rom.data + data_offset, sizeof(ret));
    }

    return ret;
}

static void mbc3_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    uint16_t a;

    a = (addr >> 12);

    switch (a) {
    case 0x0:
    case 0x1:
        emu->mmu.mbc3.ram_timer_enable = val;
        break;

    case 0x2:
    case 0x3:
        emu->mmu.mbc3.rom_bank = val & 0x7F;
        break;

    case 0x4:
    case 0x5:
        emu->mmu.mbc3.ram_bank = val;
        break;

    case 0x6:
    case 0x7:
        emu->mmu.mbc3.timer_latch = val;
        break;
    }
}

static void mbc3_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t val)
{
    printf("MBC3 WRITE16: 0x%04x, 0x%04x\n", addr, low);
    /* NOP */
}

static uint8_t mbc3_eram_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (!((emu->mmu.mbc3.ram_timer_enable & 0x0A) == 0x0A))
        return 0;

    if (emu->mmu.mbc3.ram_bank < 0x04)
        return emu->mmu.eram[addr][emu->mmu.mbc3.ram_bank];
    else
        return 0; /* Timer */
}

static uint16_t mbc3_eram_read16(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    uint16_t ret;

    if (!((emu->mmu.mbc3.ram_timer_enable & 0x0A) == 0x0A))
        return 0;

    if (emu->mmu.mbc3.ram_bank < 0x04)
        memcpy(&ret, &emu->mmu.eram[addr][emu->mmu.mbc3.ram_bank], sizeof(ret));
    else
        return 0; /* Timer */

    return ret;
}

static void mbc3_eram_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    if (!((emu->mmu.mbc3.ram_timer_enable & 0x0A) == 0x0A))
        return ;

    if (emu->mmu.mbc3.ram_bank < 0x04)
        emu->mmu.eram[addr][emu->mmu.mbc3.ram_bank] = val;
    else
        return ; /* Timer */
}

static void mbc3_eram_write16(struct gb_emu *emu, uint16_t addr, uint16_t low, uint16_t val)
{
    if (!((emu->mmu.mbc3.ram_timer_enable & 0x0A) == 0x0A))
        return ;

    if (emu->mmu.mbc3.ram_bank < 0x04)
        emu->mmu.eram[addr][emu->mmu.mbc3.ram_bank] = val;
    else
        return ; /* Timer */
}

struct gb_mmu_entry gb_mbc3_mmu_entry = {
    .low = 0x0000,
    .high = 0x7FFF,
    .read8 = mbc3_read8,
    .read16 = mbc3_read16,
    .write8 = mbc3_write8,
    .write16 = mbc3_write16,
};

struct gb_mmu_entry gb_mbc3_eram_mmu_entry = {
    .low = 0xA000,
    .high = 0xBFFF,
    .read8 = mbc3_eram_read8,
    .read16 = mbc3_eram_read16,
    .write8 = mbc3_eram_write8,
    .write16 = mbc3_eram_write16,
};

