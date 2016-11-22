#ifndef INCLUDE_MMU_H
#define INCLUDE_MMU_H

#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

struct gb_emu;

struct gb_mmu_entry {
    uint16_t low, high;

    uint8_t (*read8) (struct gb_emu *, uint16_t addr, uint16_t low);
    void (*write8) (struct gb_emu *, uint16_t addr, uint16_t low, uint8_t val);
};

extern struct gb_mmu_entry gb_mbc0_mmu_entry, gb_mbc0_eram_mmu_entry;
extern struct gb_mmu_entry gb_mbc1_mmu_entry, gb_mbc1_eram_mmu_entry;
extern struct gb_mmu_entry gb_mbc3_mmu_entry, gb_mbc3_eram_mmu_entry;
extern struct gb_mmu_entry gb_mbc5_mmu_entry, gb_mbc5_eram_mmu_entry;

struct gb_mmu_mbc1 {
    uint8_t ram_enable;
    uint8_t rom_ram_mode;

    uint8_t rom_bank;
    uint8_t ram_rom_bank_upper;
};

struct gb_mmu_mbc3_time {
    int seconds;
    int minutes;
    int hours;
    int days;
    int control;
};

struct gb_mmu_mbc3 {
    uint8_t rom_bank;

    uint8_t ram_timer_enable;
    uint8_t ram_bank;
    uint8_t timer_latch;

    struct gb_mmu_mbc3_time cur, latched;
    time_t last_time_update;
};

struct gb_mmu_mbc5 {
    uint16_t rom_bank;
    uint8_t  ram_bank;

    uint8_t  ram_bank_enable;
};

#define GB_IO_CGB_WRAM_BANK_NO 0xFF70

#define GB_IO_CGB_HDMA_SOURCE_HIGH 0xFF51
#define GB_IO_CGB_HDMA_SOURCE_LOW  0xFF52
#define GB_IO_CGB_HDMA_DEST_HIGH   0xFF53
#define GB_IO_CGB_HDMA_DEST_LOW    0xFF54
#define GB_IO_CGB_HDMA_MODE        0xFF55

#define GB_CGB_HDMA_DMA_TYPE (1 << 7)

#define GB_CGB_HDMA_DMA_HBLANK (1 << 7)
#define GB_CGB_HDMA_DMA_GENERAL (0)

struct gb_mmu {
    int bios_flag;
    struct gb_mmu_mbc1 mbc1;
    struct gb_mmu_mbc3 mbc3;
    struct gb_mmu_mbc5 mbc5;

    struct gb_mmu_entry *mbc_controller, *eram_controller;

    char eram[16][8 * 1024]; /* External RAM */

    int cgb_wram_bank_no;
    char wram[8][4 * 1024]; /* Working RAM space */

    char zram[128]; /* Zero-page RAM */

    uint16_t hdma_source, hdma_dest;

    int hdma_active;
    int hdma_type;
    int hdma_length_left;
};

void gb_mmu_add_mmu_entry(struct gb_mmu *mmu, struct gb_mmu_entry *entry);

/* Returns the current byte that the PC reg points too, and increments the PC
 * register by one */
uint8_t gb_emu_next_pc8(struct gb_emu *);
uint16_t gb_emu_next_pc16(struct gb_emu *);

uint8_t gb_emu_read8(struct gb_emu *, uint16_t addr);
void gb_emu_write8(struct gb_emu *, uint16_t addr, uint8_t byte);

uint16_t gb_emu_read16(struct gb_emu *, uint16_t addr);
void gb_emu_write16(struct gb_emu *, uint16_t addr, uint16_t word);

#endif
