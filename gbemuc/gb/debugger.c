
#include "common.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>

#include "container_of.h"
#include "cmd_parser.h"
#include "gb.h"
#include "gb/cpu.h"
#include "gb/debugger.h"
#include "debug.h"

struct gb_debugger_breakpoint {
    uint16_t addr;
};

struct gb_debugger {
    size_t breakpoint_count;
    struct gb_debugger_breakpoint *breakpoints;

    int exit_flag;
};

static struct cmd_desc debugger_cmds[];

static const char *prompt = " > ";

static void debugger_breakpoint(int argc, char **argv, va_list args)
{
    struct gb_debugger *debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);
    uint16_t addr;

    if (argc != 1) {
        int i;
        for (i = 0; i < debugger->breakpoint_count; i++)
            printf("%d - 0x%04x\n", i + 1, debugger->breakpoints[i].addr);
        return;
    }

    sscanf(argv[0], "%hi", &addr);

    debugger->breakpoints = realloc(debugger->breakpoints, (++debugger->breakpoint_count) * sizeof(struct gb_debugger_breakpoint));

    debugger->breakpoints[debugger->breakpoint_count - 1].addr = addr;
}

static volatile int stop_run = 0;

static void debugger_run_sigint(int signum)
{
    stop_run = 1;
}

static void debugger_run(int argc, char **argv, va_list args)
{
    struct gb_debugger *debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);
    struct sigaction new_act, old_act;
    int cycles;
    int i;

    memset(&new_act, 0, sizeof(new_act));
    new_act.sa_handler = debugger_run_sigint;

    sigaction(SIGINT, &new_act, &old_act);

    stop_run = 0;

    do {
        cycles += gb_emu_cpu_run_next_inst(emu);

        for (i = 0; i < debugger->breakpoint_count; i++) {
            if (emu->cpu.r.w[GB_REG_PC] == debugger->breakpoints[i].addr) {
                printf("Breakpoint %d - 0x%04x\n", i + 1, debugger->breakpoints[i].addr);
                stop_run = 2;
            }
        }
    } while (!stop_run);

    if (stop_run == 1)
        printf("PC: 0x%04x\n", emu->cpu.r.w[GB_REG_PC]);

    sigaction(SIGINT, &old_act, NULL);
}

static void debugger_step(int argc, char **argv, va_list args)
{
    struct gb_debugger *debugger = va_arg(args, struct gb_debugger *);
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
    struct gb_debugger *debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);
    char reg_buf[163];

    gb_emu_dump_regs(emu, reg_buf);
    printf("%s", reg_buf);
}

static void debugger_exit(int argc, char **argv, va_list args)
{
    struct gb_debugger *debugger = va_arg(args, struct gb_debugger *);
    struct gb_emu *emu = va_arg(args, struct gb_emu *);

    debugger->exit_flag = 1;
}

static void debugger_help(int argc, char **argv, va_list args)
{
    printf("Command      - Help\n");
    struct cmd_desc *desc;

    for (desc = debugger_cmds; desc->cmd_id; desc++)
        printf("%-12s%s%s - %s\n", desc->cmd_id, desc->args? " - ": "", desc->args? desc->args: "", desc->help);
}

static struct cmd_desc debugger_cmds[] = {
    { 'd', "breakpoint", debugger_breakpoint,
        "Set a breakpoint at an address",
        "<addr>" },
    { 'r', "run", debugger_run,
        "Start emulator",
        NULL },
    { 's', "step", debugger_step,
        "Step <count> instructions - default is one",
        "<count>" },
    { '\0', "dump-regs", debugger_dump_regs,
        "Display current value of registers",
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

void gb_debugger_run(struct gb_emu *emu)
{
    struct gb_debugger debugger;
    int cur_buff = 0;
    ssize_t line_len[2];
    size_t line_max[2] = { 0, 0 };
    char *line[2] = { NULL, NULL };

    memset(&debugger, 0, sizeof(debugger));

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

