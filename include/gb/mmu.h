#ifndef INCLUDE_MMU_H
#define INCLUDE_MMU_H

#include <stdint.h>
#include "slist.h"

struct gb_emu;

struct gb_mmu_entry {
    uint16_t low, high;

    uint8_t (*read8) (struct gb_emu *, uint16_t addr, uint16_t low);
    uint16_t (*read16) (struct gb_emu *, uint16_t addr, uint16_t low);

    void (*write8) (struct gb_emu *, uint16_t addr, uint16_t low, uint8_t val);
    void (*write16) (struct gb_emu *, uint16_t addr, uint16_t low, uint16_t val);
};

struct gb_mmu {
    struct slist_head entry_list;

    int bios_flag;
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
