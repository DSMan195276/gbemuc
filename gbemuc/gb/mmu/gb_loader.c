
#include "common.h"

#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "gb.h"
#include "gb/mmu.h"
#include "gb/bios.h"

static char game_list_data[][MAX_FILE_PACKET_NAME] = {
    "Pokemon RED.gb",
    "Pokemon Blue.gb",
    "Pokemon Gold.gb",
    "Link's Awakening",
    "Mario - Six GoldCo",
    "Flappy Boy.gb",
    "FaceBall 2000.gb",
    "Kirby.gb",
    "donkey_kong.gb",
    "warioland.gb",
    "Pokemon RED.gb",
    "Pokemon Blue.gb",
    "Pokemon Gold.gb",
    "Link's Awakening",
    "Mario - Six GoldCo",
};

static void get_game_name(int game_number, struct file_packet *packet)
{
    DIR *curdir = opendir(".");
    struct dirent *dent;
    int i;

    for (i = 0; i < game_number; i++) {
        dent = readdir(curdir);
        if (!dent) {
            memset(packet, 0, sizeof(*packet));
            goto close_dir;
        }
    }

    dent = readdir(curdir);
    if (!dent) {
        memset(packet, 0, sizeof(*packet));
        goto close_dir;
    }

    struct stat stbuf;

    stat(dent->d_name, &stbuf);

    memset(packet, 0, sizeof(*packet));

    if (S_ISDIR(stbuf.st_mode))
        packet->flags |= FILE_PACKET_FLAG_IS_DIR;

    strncpy(packet->name, dent->d_name, sizeof(packet->name));
    packet->name[sizeof(packet->name) - 1] = '\0';

  close_dir:
    closedir(curdir);
}

/*
 *
 * MBC0 ROM
 *
 */

static uint8_t gb_loader_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    if (!emu->mmu.bios_flag && addr < 0x0100)
        return gb_bios[addr];

    if (addr == 0x7FFF) {
        char *buffer = &emu->mmu.gb_loader.cur_game;
        return buffer[emu->mmu.gb_loader.cur_game_pos++];
    }

    if (addr == 0x7FFE) {
        int sig = emu->mmu.gb_loader.ready_signal++;

        if (sig < 5)
            return 0xF0;
        else if (sig < 10)
            return 0x0F;
        else
            return 0xAA;
    }

    return emu->rom.data[addr];
}

static void gb_loader_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    if (addr == 0x7FFF) {
        emu->mmu.gb_loader.cur_game_loc = val;
        emu->mmu.gb_loader.cur_game_pos = 0;
        get_game_name(emu->mmu.gb_loader.cur_game_loc, &emu->mmu.gb_loader.cur_game);
    }

    if (addr == 0x7FFE) {
        printf("Hit chdir\n");
        struct file_packet current;
        get_game_name(val, &current);

        chdir(current.name);
    }

    if (addr == 0x7FFD)
        emu->mmu.gb_loader.ready_signal = 0;
}

static uint8_t gb_loader_eram_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    return 0;
}

static void gb_loader_eram_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t val)
{
    /* NOP */
}

static int gb_loader_get_bank(struct gb_emu *emu, uint16_t addr)
{
    return 0;
}

struct gb_mmu_entry gb_loader_mmu_entry = {
    .low = 0x0000,
    .high = 0x7FFF,
    .read8 = gb_loader_read8,
    .write8 = gb_loader_write8,
    .get_bank = gb_loader_get_bank,
};

struct gb_mmu_entry gb_loader_eram_mmu_entry = {
    .low = 0xA000,
    .high = 0xBFFF,
    .read8 = gb_loader_eram_read8,
    .write8 = gb_loader_eram_write8,
    .get_bank = gb_loader_get_bank,
};

