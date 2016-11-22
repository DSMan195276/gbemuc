#ifndef INCLUDE_GB_IO_H
#define INCLUDE_GB_IO_H

#include <stdint.h>

struct gb_emu;

#define GB_IO_BIOS_FLAG 0xFF50

void gb_emu_io_reset(struct gb_emu *emu);
uint8_t gb_emu_io_read8(struct gb_emu *, uint16_t addr, uint16_t low);
void gb_emu_io_write8(struct gb_emu *, uint16_t addr, uint16_t low, uint8_t byte);

#endif
