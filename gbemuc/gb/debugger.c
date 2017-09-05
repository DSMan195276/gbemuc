
#include "common.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>

#include "container_of.h"
#include "cmd_parser.h"
#include "gb.h"
#include "gb/cpu.h"
#include "gb/gpu.h"
#include "gb/disasm.h"
#include "gb/debugger.h"
#include "debug.h"

struct gb_debugger {
    int exit_flag;
};

static struct cmd_desc debugger_cmds[];
static const char *prompt = " > ";

static void debugger_breakpoint(int argc, char **argv, va_list args)
{
    struct gb_debugger *__unused debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);
    uint16_t addr;

    if (argc != 1) {
        int i;
        for (i = 0; i < emu->breakpoint_count; i++)
            printf("%d - 0x%04x\n", i + 1, emu->breakpoints[i]);
        return;
    }

    sscanf(argv[0], "%hi", &addr);

    gb_emu_add_breakpoint(emu, addr);
}

static void debugger_breakpoint_on(int argc, char **argv, va_list args)
{
    struct gb_debugger *__unused debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);

    emu->break_flag = 1;
}

static void debugger_breakpoint_off(int argc, char **argv, va_list args)
{
    struct gb_debugger *__unused debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);

    emu->break_flag = 0;
}

static void debugger_run(int argc, char **argv, va_list args)
{
    struct gb_debugger *__unused debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);
    enum gb_emu_stop result;

    result = gb_run(emu, GB_CPU_INTERPRETER);

    if (result == GB_EMU_STOP)
        printf("PC: 0x%04x\n", emu->cpu.r.w[GB_REG_PC]);
    else
        printf("BREAK: 0x%04x\n", emu->cpu.r.w[GB_REG_PC]);
}

static void debugger_step(int argc, char **argv, va_list args)
{
    struct gb_debugger *__unused debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);
    int count = 1;

    if (argc == 1)
        sscanf(argv[0], "%i", &count);

    while (count--)
        gb_emu_cpu_run_next_inst(emu);

    printf("PC: 0x%04x\n", emu->cpu.r.w[GB_REG_PC]);
}

static void debugger_dump_regs(int argc, char **argv, va_list args)
{
    struct gb_debugger *__unused debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);
    char reg_buf[263];

    gb_emu_dump_regs(emu, reg_buf);
    printf("%s", reg_buf);
}

static void debugger_print_addr(int argc, char **argv, va_list args)
{
    struct gb_debugger *__unused debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);
    uint8_t val;
    uint16_t addr;

    if (argc != 1) {
        printf("Please supply a 16-bit address\n");
        return;
    }

    sscanf(argv[0], "%hi", &addr);

    val = gb_emu_read8(emu, addr);

    printf("Value at 0x%04x: 0x%02x (%d, %d)\n", addr, val, val, (int8_t)val);

    return ;
}

static void debugger_exit(int argc, char **argv, va_list args)
{
    struct gb_debugger *debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *__unused emu = va_arg(args, struct gb_emu *);

    debugger->exit_flag = 1;
}

static void debugger_help(int argc, char **argv, va_list args)
{
    printf("Command      - Help\n");
    struct cmd_desc *desc;

    for (desc = debugger_cmds; desc->cmd_id; desc++)
        printf("%-12s%s%s - %s\n", desc->cmd_id, desc->args? " - ": "", desc->args? desc->args: "", desc->help);
}

static void debugger_debug_off(int argc, char **argv, va_list args)
{
    struct gb_debugger *__unused debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);

    emu->hook_flag = 0;
}

static void debugger_debug_on(int argc, char **argv, va_list args)
{
    struct gb_debugger *__unused debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);

    emu->hook_flag = 1;
}

static void display_tile(struct gb_gpu *gpu, int spriten, union gb_gpu_color_u *screenbuf, int x, int y)
{
    int i, k;
    for (i = 0; i < 8; i++) {
        uint8_t low = gpu->vram[0].seg.sprites[spriten][i * 2];
        uint8_t high = gpu->vram[0].seg.sprites[spriten][i * 2 + 1];
        for (k = 0; k < 8; k++) {
            int lo = !!(low & (1 << (7 - k)));
            int hi = !!(high & (1 << (7 - k)));
            int pix = lo | (hi << 1);
            screenbuf[(x + i) * GB_SCREEN_WIDTH + (y + k)] = gpu->display->dmg_theme.bg[pix];
        }
    }
}

static void debugger_show_tiles(int argc, char **argv, va_list args)
{
    struct gb_debugger *__unused debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);
    struct gb_gpu *gpu = &emu->gpu;
    union gb_gpu_color_u screenbuf[GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH] = { 0 };
    int spriten = 0, soff = 0;

    for (spriten = 0; spriten < 20 * 18; spriten++) {
        int x, y;
        x = (spriten / 20) * 8;
        y = (spriten % 20) * 8;

        display_tile(gpu, spriten + soff, screenbuf, x, y);
    }

    (gpu->display->disp_buf) (gpu->display, screenbuf);
}

static void debugger_show_sprites(int argc, char **argv, va_list args)
{
    struct gb_debugger *__unused debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);
    struct gb_gpu *gpu = &emu->gpu;
    union gb_gpu_color_u screenbuf[GB_SCREEN_HEIGHT * GB_SCREEN_WIDTH] = { 0 };
    int spriten = 0;

    for (spriten = 0; spriten < 40; spriten++) {
        int tile_no = emu->gpu.oam.s_attrs[spriten][GB_GPU_SPRITE_ATTR_TILE_NUM];
        int x, y;

        x = (spriten / 20) * 8;
        y = (spriten % 20) * 8;

        display_tile(gpu, tile_no, screenbuf, x, y);
    }

    (gpu->display->disp_buf) (gpu->display, screenbuf);
}

static void debugger_show_palettes(int argc, char **argv, va_list args)
{
    struct gb_debugger *__unused debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);
    struct gb_gpu *gpu = &emu->gpu;

    printf("Back pal: 0x%02x\n", gpu->back_palette);
    printf("Obj pal0: 0x%02x\n", gpu->obj_pal[0]);
    printf("Obj pal1: 0x%02x\n", gpu->obj_pal[1]);
}

static struct cmd_desc debugger_cmds[] = {
    { 'b', "breakpoint", debugger_breakpoint,
        "Set a breakpoint at an address",
        "<addr>" },
    { '\0', "breakpoint-on", debugger_breakpoint_on,
        "Turn on breakpoints",
        NULL },
    { '\0', "breakpoint-off", debugger_breakpoint_off,
        "Turn on breakpoints",
        NULL },
    { 'r', "run", debugger_run,
        "Start emulator",
        NULL },
    { 's', "step", debugger_step,
        "Step <count> instructions - default is one",
        "<count>" },
    { 'd', "dump-regs", debugger_dump_regs,
        "Display current value of registers",
        NULL },
    { 'p', "print", debugger_print_addr,
        "Display the value at an address in memory",
        "<addr>" },
    { '\0', "debug-on", debugger_debug_on,
        "Turns on Debugging output",
        NULL },
    { '\0', "debug-off", debugger_debug_off,
        "Turns off debugging output",
        NULL },
    { '\0', "sprites", debugger_show_sprites,
        "Display current loaded sprites",
        NULL },
    { '\0', "tiles", debugger_show_tiles,
        "Display current loaded tiles",
        NULL },
    { '\0', "palettes", debugger_show_palettes,
        "Display current loaded palettes",
        NULL },
    { '\0', "exit", debugger_exit,
        "Exit the debugger",
        NULL },
    { '\0', "quit", debugger_exit,
        "Exit the debugger",
        NULL },
    { 'h', "help", debugger_help,
        "Display help",
        NULL },
    { '\0', NULL, NULL, NULL, NULL }
};

static void debugger_print_next_inst(struct gb_cpu_hooks *hooks, struct gb_emu *emu, uint8_t *inst)
{
    char buf[30] = { 0 };

    gb_disasm_inst(buf, inst);
    printf("(0x%02x)%s\n", inst[0], buf);
    DEBUG_PRINTF("(0x%02x)%s\n", inst[0], buf);
}

static void debugger_print_end_inst(struct gb_cpu_hooks *hooks, struct gb_emu *emu)
{
    char reg_buf[263];

    gb_emu_dump_regs(emu, reg_buf);
    printf("%s", reg_buf);
    DEBUG_PRINTF("%s", reg_buf);
}

static struct gb_cpu_hooks debugger_cpu_hooks = { .next_inst = debugger_print_next_inst,
                                                  .end_inst  = debugger_print_end_inst };

void gb_debugger_run(struct gb_emu *emu)
{
    struct gb_debugger debugger;
    int cur_buff = 0;
    ssize_t line_len[2];
    size_t line_max[2] = { 0, 0 };
    char *line[2] = { NULL, NULL };

    memset(&debugger, 0, sizeof(debugger));

    emu->cpu.hooks = &debugger_cpu_hooks;

    while (printf("%s", prompt),
           line_len[cur_buff] = getline(line + cur_buff, line_max + cur_buff, stdin)) {
        int sel = cur_buff;

        /* We do some double buffering to allow entering a blank line to repeat
         * the last instruction.
         *
         * If a blank line is entered, then we switch the command-buffer to be
         * the last-used buffer, which contains the last instruction.
         *
         * If not, then we switch the current-buffer, so we overwrite our
         * second buffer next time and the current buffer is saved as the
         * 'last' instruction. */
        if (line_len[cur_buff] != 1)
            cur_buff ^= 1;
        else
            sel ^= 1;

        run_cmd(debugger_cmds, line[sel], &debugger, emu);

        if (debugger.exit_flag)
            break;
    }

    free(line[0]);
    free(line[1]);

    return ;
}

