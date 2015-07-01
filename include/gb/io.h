#ifndef INCLUDE_GB_IO_H
#define INCLUDE_GB_IO_H

#include <stdint.h>

struct gb_emu;

uint8_t gb_emu_io_read8(struct gb_emu *, uint16_t addr, uint16_t low);
void gb_emu_io_write8(struct gb_emu *, uint16_t addr, uint16_t low, uint8_t byte);

#endif
