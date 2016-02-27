
#include "common.h"

#include <stdint.h>
#include <time.h>

#include "debug.h"
#include "gb_internal.h"
#include "gb/cpu.h"

/* A map used by instructions to convert a numeral in an opcode to actual
 * register numbers */
static const uint8_t reg_map_8bit[8] = {
    GB_REG_B,
    GB_REG_C,
    GB_REG_D,
    GB_REG_E,
    GB_REG_H,
    GB_REG_L,
    0, /* (HL) */
    GB_REG_A,
};

/* Returns whether or not the provided value maps to (HL) */
#define IS_HL(x) ((x) == b8(110))

/* Handles the special case of (HL).
 * Returns the value that should be used, and modifies cycles if necessary. */
static inline uint8_t read_8bit_reg(struct gb_emu *emu, int reg, int *cycles)
{
    if (IS_HL(reg)) {
        gb_emu_clock_tick(emu);
        *cycles += 4;
        return gb_emu_read8(emu, emu->cpu.r.w[GB_REG_HL]);
    } else {
        return emu->cpu.r.b[reg_map_8bit[reg]];
    }
}

static inline void write_8bit_reg(struct gb_emu *emu, int reg, uint8_t val, int *cycles)
{
    if (IS_HL(reg)) {
        gb_emu_clock_tick(emu);
        *cycles += 4;
        gb_emu_write8(emu, emu->cpu.r.w[GB_REG_HL], val);
    } else {
        emu->cpu.r.b[reg_map_8bit[reg]] = val;
    }
}

/* A map used by instructions to convert numerals in an opcode into a register
 * number.
 *
 * Some instructions make-use of SP instead of AF, so there are two tables. */
static const uint8_t reg_map_16bit_sp[4] = {
    GB_REG_BC,
    GB_REG_DE,
    GB_REG_HL,
    GB_REG_SP,
};

static const uint8_t reg_map_16bit_af[4] = {
    GB_REG_BC,
    GB_REG_DE,
    GB_REG_HL,
    GB_REG_AF,
};

/*
 *
 * 8-bit loads
 *
 */

/* LD reg, imm */
static int load8_reg_imm(struct gb_emu *emu, uint8_t opcode)
{
    /* Pull Register number out of bits 4 through 6. */
    uint8_t reg = ((opcode & b8(00111000)) >> 3);

    /* Reading PC takes a clock-tick */
    gb_emu_clock_tick(emu);
    uint8_t imm = gb_emu_next_pc8(emu);

    emu->cpu.r.b[reg_map_8bit[reg]] = imm;

    /* 4 cycles for our read from PC */
    return 4;
}

/* LD reg, reg. */
static int load8_reg_reg(struct gb_emu *emu, uint8_t opcode)
{
    /* Dest is bits 4 through 6
     * Src is bits 1 through 3 */
    uint8_t dest = ((opcode & b8(00111000)) >> 3);
    uint8_t src = (opcode & b8(00000111));
    int cycles = 0;
    uint8_t val;

    val = read_8bit_reg(emu, src, &cycles);
    write_8bit_reg(emu, dest, val, &cycles);

    return cycles;
}

/* LD A, (n)
 * LD A, #
 * LD A, (C)
 * LDD A, (HL)
 * LDI A, (HL)
 */
static int load8_a_extra(struct gb_emu *emu, uint8_t opcode)
{
    int reg;
    uint8_t val = 0;
    uint16_t src;
    int cycles = 0;

    switch (opcode) {
    case 0x0A: /* LD A, (BC) */
    case 0x1A: /* LD A, (DE) */
        if (opcode & 0x10)
            reg = GB_REG_DE;
        else
            reg = GB_REG_BC;
        val = gb_emu_read8(emu, emu->cpu.r.w[reg]);
        break;

    case 0xFA: /* LD A, (nn) */
        /* Two clock ticks for reading a 16-bit from PC */
        gb_emu_clock_tick(emu);
        gb_emu_clock_tick(emu);
        src = gb_emu_next_pc16(emu);
        val = gb_emu_read8(emu, src);
        cycles += 8;
        break;

    case 0x3E: /* LD A, # */
        val = gb_emu_next_pc8(emu);
        break;

    case 0xF2: /* LD A, (C) */
        src = 0xFF00 + emu->cpu.r.b[GB_REG_C];
        val = gb_emu_read8(emu, src);
        break;

    case 0xF0:
        /* 8-bit read from PC */
        gb_emu_clock_tick(emu);
        src = 0xFF00 + gb_emu_next_pc8(emu);
        val = gb_emu_read8(emu, src);
        cycles += 4;
        break;

    case 0x3A: /* LDD A, (HL) */
    case 0x2A: /* LDI A, (HL) */
        src = emu->cpu.r.w[GB_REG_HL];
        if (opcode == 0x3A)
            emu->cpu.r.w[GB_REG_HL]--;
        else
            emu->cpu.r.w[GB_REG_HL]++;

        val = gb_emu_read8(emu, src);
        break;
    }

    /* Accounts for 8-bit read in every path. */
    gb_emu_clock_tick(emu);

    emu->cpu.r.b[GB_REG_A] = val;

    return cycles;
}

/* LD (n), A
 * LDD (HL), A
 * LDI (HL), A
 */
static int load8_extra_a(struct gb_emu *emu, uint8_t opcode)
{
    int reg;
    uint16_t dest = 0;
    int cycles = 0;

    switch (opcode) {
    case 0x02: /* LD (BC), A */
    case 0x12: /* LD (DE), A */
        if (opcode & 0x10)
            reg = GB_REG_DE;
        else
            reg = GB_REG_BC;

        dest = emu->cpu.r.w[reg];
        break;

    case 0xEA: /* LD (nn), A */
        gb_emu_clock_tick(emu);
        gb_emu_clock_tick(emu);
        dest = gb_emu_next_pc16(emu);
        cycles += 8;
        break;

    case 0xE2: /* LD (0xFF00 + C), A */
        dest = 0xFF00 + emu->cpu.r.b[GB_REG_C];
        break;

    case 0xE0: /* LD (0xFF00 + n), A */
        gb_emu_clock_tick(emu);
        dest = 0xFF00 + gb_emu_next_pc8(emu);
        cycles += 4;
        break;

    case 0x32: /* LDD (HL), A */
    case 0x22: /* LDI (HL), A */
        dest = emu->cpu.r.w[GB_REG_HL];
        if (opcode == 0x32)
            emu->cpu.r.w[GB_REG_HL]--;
        else
            emu->cpu.r.w[GB_REG_HL]++;
        break;
    }

    gb_emu_clock_tick(emu);
    gb_emu_write8(emu, dest, emu->cpu.r.b[GB_REG_A]);
    cycles += 4;

    return cycles;
}

/*
 *
 * 16-bit Loads
 *
 */

/* LD n, nn */
static int load16_reg_imm(struct gb_emu *emu, uint8_t opcode)
{
    int dest = reg_map_16bit_sp[(opcode & 0x30) >> 4];

    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);
    uint16_t src = gb_emu_next_pc16(emu);

    emu->cpu.r.w[dest] = src;

    /* Always takes 8 cycles */
    return 8;
}

/* LD SP, HL */
static int load_sp_hl(struct gb_emu *emu, uint8_t opcode)
{
    /* Extra clock-tick for 16-bit loads */
    gb_emu_clock_tick(emu);

    emu->cpu.r.w[GB_REG_SP] = emu->cpu.r.w[GB_REG_HL];

    return 4;
}

/* LDHL SP, n */
static int load_hl_sp_n(struct gb_emu *emu, uint8_t opcode)
{

    /* Read 8-bit from PC */
    gb_emu_clock_tick(emu);
    int8_t off = gb_emu_next_pc8(emu);

    /* Extra clock tick for 16-bit load */
    gb_emu_clock_tick(emu);
    emu->cpu.r.w[GB_REG_HL] = emu->cpu.r.w[GB_REG_SP] + off;

    return 8;
}

/* LD (nn), SP */
static int load_mem_sp(struct gb_emu *emu, uint8_t opcode)
{
    /* 16-bit read */
    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);
    uint16_t dest = gb_emu_next_pc16(emu);

    /* 16-bit write */
    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);
    gb_emu_write16(emu, dest, emu->cpu.r.w[GB_REG_SP]);

    return 16;
}

static int push_val(struct gb_emu *emu, uint16_t val)
{
    /* Extra clock-tick for 16-bit */
    gb_emu_clock_tick(emu);
    emu->cpu.r.w[GB_REG_SP] -= 2;

    /* 16-bit write */
    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);
    gb_emu_write16(emu, emu->cpu.r.w[GB_REG_SP], val);

    return 12;
}

static int pop_val(struct gb_emu *emu, uint16_t *dest)
{
    /* 16-bit read */
    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);
    *dest = gb_emu_read16(emu, emu->cpu.r.w[GB_REG_SP]);

    /* Extra clock-tick for 16-bit */
    gb_emu_clock_tick(emu);
    emu->cpu.r.w[GB_REG_SP] += 2;

    return 12;
}

/* PUSH reg
 * reg = AF, BC, DE, HL */
static int push(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int dest = reg_map_16bit_af[(opcode & 0x30) >> 4];

    cycles += push_val(emu, emu->cpu.r.w[dest]);

    return cycles;
}

/* POP reg
 * reg = AF, BC, DE, HL */
static int pop(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int dest = reg_map_16bit_af[(opcode & 0x30) >> 4];

    cycles += pop_val(emu, emu->cpu.r.w + dest);

    return cycles;
}

/*
 *
 * 8-bit ALU
 *
 */

/* Performs an 8-bit addition, with optional carry.
 *
 * Returns the result of the add */
static uint8_t adc(uint8_t val1, uint8_t val2, uint8_t *flags, int carry)
{
    uint8_t result;

    *flags = 0;

    if ((val1 & 0xF) + (val2 & 0xF) + carry > 0xF)
        *flags |= GB_FLAG_HCARRY;

    if ((uint16_t)val1 + (uint16_t)val2 + carry > 0xFF)
        *flags |= GB_FLAG_CARRY;

    /* Overflow is undefined in C - We avoid it by upcasting to uint16_t, and
     * then masking out the higher bits after we do the addition */
    result = ((uint16_t)val1 + (uint16_t)val2 + carry) & 0xFF;

    if (result == 0)
        *flags |= GB_FLAG_ZERO;

    return result;
}

/* ADD A, reg
 * ADD A, #
 *
 * ADC A, reg
 * ADC A, #
 */
static int add_a(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = opcode & 0x07;
    uint8_t carry = 0;
    uint8_t tmp;
    uint8_t flags = 0;

    /* 0xC0 means it's an immediate value */
    if ((opcode & 0xF0) != 0xC0) {
        tmp = read_8bit_reg(emu, src, &cycles);
    } else {
        gb_emu_clock_tick(emu);
        tmp = gb_emu_next_pc8(emu);
        cycles += 4;
    }

    /* ADC commands add the carry flag */
    if ((opcode & 0x0F) >= 0x08 || opcode == 0xCE)
        carry = GB_FLAG_IS_SET(emu->cpu.r.b[GB_REG_F], GB_FLAG_CARRY);

    gb_emu_clock_tick(emu);
    emu->cpu.r.b[GB_REG_A] = adc(emu->cpu.r.b[GB_REG_A], tmp, &flags, carry);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles + 4;
}

/* SUB reg
 * SUB #
 */
static int sub_a(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = opcode & 0x07;
    uint8_t tmp, carry;
    uint8_t flags = 0;

    /* 0xD0 means it's am immediate value */
    if ((opcode & 0xF0) != 0xD0) {
        tmp = read_8bit_reg(emu, src, &cycles);
    } else {
        gb_emu_clock_tick(emu);
        tmp = gb_emu_next_pc8(emu);
        cycles += 4;
    }

    /* SBC commands add the carry flag to the front of 'a'.
     * Since we do the SDC with the ADC command, we invert the carry bit. */
    if ((opcode & 0x0F) >= 0x08 || opcode == 0xDE)
        carry = !GB_FLAG_IS_SET(emu->cpu.r.b[GB_REG_F], GB_FLAG_CARRY);
    else
        carry = 1;

    emu->cpu.r.b[GB_REG_A] = adc(emu->cpu.r.b[GB_REG_A], ~tmp, &flags, carry);
    emu->cpu.r.b[GB_REG_F] = ~flags;

    return cycles;
}

/* AND reg
 * AND n */
static int and_a(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = opcode & 0x07;
    uint8_t flags = GB_FLAG_HCARRY; /* AND always sets HCARRY */
    uint8_t tmp;

    /* Check if this is not the immediate value opcode 0xE6 */
    if ((opcode & 0xF0) != 0xE0) {
        tmp = read_8bit_reg(emu, src, &cycles);
    } else {
        gb_emu_clock_tick(emu);
        tmp = gb_emu_next_pc8(emu);
        cycles += 4;
    }

    emu->cpu.r.b[GB_REG_A] &= tmp;

    if (emu->cpu.r.b[GB_REG_A] == 0)
        flags |= GB_FLAG_ZERO;

    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* OR reg
 * OR n */
static int or_a(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = opcode & 0x07;
    uint8_t flags = 0;
    uint8_t tmp;

    /* Check if this is not the immediate value opcode 0xF6 */
    if ((opcode & 0xF0) != 0xF0) {
        tmp = read_8bit_reg(emu, src, &cycles);
    } else {
        gb_emu_clock_tick(emu);
        tmp = gb_emu_next_pc8(emu);
        cycles += 4;
    }

    emu->cpu.r.b[GB_REG_A] |= tmp;

    if (emu->cpu.r.b[GB_REG_A] == 0)
        flags |= GB_FLAG_ZERO;

    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* XOR reg
 * XOR n */
static int xor_a(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = opcode & 0x07;
    uint8_t flags = 0;
    uint8_t tmp;

    /* Check if this is not the immediate value opcode 0xEE */
    if ((opcode & 0xF0) != 0xE0) {
        tmp = read_8bit_reg(emu, src, &cycles);
    } else {
        gb_emu_clock_tick(emu);
        tmp = gb_emu_next_pc8(emu);
        cycles += 4;
    }

    emu->cpu.r.b[GB_REG_A] ^= tmp;

    if (emu->cpu.r.b[GB_REG_A] == 0)
        flags |= GB_FLAG_ZERO;

    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* CP reg
 * CP n */
static int cp_a(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = opcode & 0x07;
    uint8_t flags = 0;
    uint8_t tmp, a;

    if ((opcode & 0xF0) != 0xF0) {
        tmp = read_8bit_reg(emu, src, &cycles);
    } else {
        gb_emu_clock_tick(emu);
        tmp = gb_emu_next_pc8(emu);
        cycles += 4;
    }

    a = emu->cpu.r.b[GB_REG_A];

    if (a == tmp)
        flags |= GB_FLAG_ZERO;

    if ((a & 0xF) < (tmp & 0xF))
        flags |= GB_FLAG_HCARRY;

    if (a < tmp)
        flags |= GB_FLAG_CARRY;

    flags |= GB_FLAG_SUB;

    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* INC reg */
static int inc_reg(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = (opcode & 0x38) >> 3;
    uint8_t flags = emu->cpu.r.b[GB_REG_F] & GB_FLAG_CARRY;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, src, &cycles);

    if ((tmp & 0x0F) == 0xF)
        flags |= GB_FLAG_HCARRY;

    if (tmp == 0xFF)
        flags |= GB_FLAG_ZERO;

    tmp = ((uint16_t)tmp + 1) & 0xFF;

    write_8bit_reg(emu, src, tmp, &cycles);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* DEC reg */
static int dec_reg(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = (opcode & 0x38) >> 3;
    uint8_t flags = emu->cpu.r.b[GB_REG_F] & GB_FLAG_CARRY;
    uint8_t tmp;


    tmp = read_8bit_reg(emu, src, &cycles);

    if ((tmp & 0x0F) == 0)
        flags |= GB_FLAG_HCARRY;

    if (tmp == 1)
        flags |= GB_FLAG_ZERO;

    tmp = ((uint16_t)tmp - 1) & 0xFF;

    write_8bit_reg(emu, src, tmp, &cycles);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/*
 *
 * 16-bit ALU
 *
 */

/* ADD HL, reg */
static int add_hl_reg16(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = (opcode & 0x30) >> 4;
    int reg = reg_map_16bit_sp[src];
    uint8_t flags = emu->cpu.r.b[GB_REG_F] & GB_FLAG_ZERO;

    if ((emu->cpu.r.w[GB_REG_HL] & 0x0FFF) + (emu->cpu.r.w[reg] & 0x0FFF) > 0x0FFF)
        flags |= GB_FLAG_HCARRY;

    if (((uint32_t)emu->cpu.r.w[GB_REG_HL] + (uint32_t)emu->cpu.r.w[reg]) > 0xFFFF)
        flags |= GB_FLAG_CARRY;

    gb_emu_clock_tick(emu);
    emu->cpu.r.w[GB_REG_HL] = ((uint32_t)emu->cpu.r.w[GB_REG_HL] + (uint32_t)emu->cpu.r.w[reg]) & 0xFFFF;

    emu->cpu.r.b[GB_REG_F] = flags;

    cycles = 4;

    return cycles;
}

/* ADD SP, # */
static int add_sp_imm(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int8_t tmp;
    uint8_t flags = 0;

    gb_emu_clock_tick(emu);
    tmp = gb_emu_next_pc8(emu);

    if ((uint32_t)((int32_t)(emu->cpu.r.w[GB_REG_SP] & 0x0FFF) + tmp) > 0x0FFF)
        flags |= GB_FLAG_HCARRY;

    if ((uint32_t)((int32_t)emu->cpu.r.w[GB_REG_SP] + tmp) > 0xFFFF)
        flags |= GB_FLAG_CARRY;

    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);
    emu->cpu.r.w[GB_REG_SP] = ((int32_t)emu->cpu.r.w[GB_REG_SP] + tmp) & 0xFFFF;
    emu->cpu.r.b[GB_REG_F] = flags;

    cycles = 12;

    return cycles;
}

/* INC nn */
static int inc_reg16(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = (opcode & 0x30) >> 4;
    int reg = reg_map_16bit_sp[src];

    gb_emu_clock_tick(emu);
    emu->cpu.r.w[reg] = ((uint32_t)emu->cpu.r.w[reg] + 1) & 0xFFFF;

    cycles = 4;

    return cycles;
}

/* DEC nn */
static int dec_reg16(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = (opcode & 0x30) >> 4;
    int reg = reg_map_16bit_sp[src];

    gb_emu_clock_tick(emu);
    emu->cpu.r.w[reg] = ((uint32_t)emu->cpu.r.w[reg] - 1) & 0xFFFF;

    cycles = 4;

    return cycles;
}

/*
 *
 * Misc ops
 *
 */

/* SWAP reg */
static int swap(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int src = (opcode & 0x7);
    uint8_t tmp, flags = 0;

    tmp = read_8bit_reg(emu, src, &cycles);

    if (tmp == 0)
        flags |= GB_FLAG_ZERO;

    tmp = ((tmp & 0xF0) >> 4) + ((tmp & 0x0F) << 4);

    write_8bit_reg(emu, src, tmp, &cycles);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* DAA */
static int daa(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    uint8_t high_nib, low_nib;
    uint8_t flags = emu->cpu.r.b[GB_REG_F] & GB_FLAG_SUB;
    uint8_t diff = 0;

    high_nib = emu->cpu.r.b[GB_REG_A] >> 4;
    low_nib = emu->cpu.r.b[GB_REG_A] & 0x0F;

    if ((emu->cpu.r.b[GB_REG_F] & GB_FLAG_HCARRY)
        || low_nib > 0x09)
        diff += 6;

    if ((emu->cpu.r.b[GB_REG_F] & GB_FLAG_CARRY)
        || high_nib > 0x09) {
        diff += 60;
        flags |= GB_FLAG_CARRY;
    }

    if (emu->cpu.r.b[GB_REG_F] & GB_FLAG_SUB)
        emu->cpu.r.b[GB_REG_A] = ((uint16_t)emu->cpu.r.b[GB_REG_A] - diff) & 0xFF;
    else
        emu->cpu.r.b[GB_REG_A] = ((uint16_t)emu->cpu.r.b[GB_REG_A] + diff) & 0xFF;

    if (emu->cpu.r.b[GB_REG_A] == 0)
        flags |= GB_FLAG_ZERO;

    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* CPL */
static int cpl(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;

    emu->cpu.r.b[GB_REG_A] = ~emu->cpu.r.b[GB_REG_A];
    emu->cpu.r.b[GB_REG_F] = GB_FLAG_SUB | GB_FLAG_HCARRY;

    return cycles;
}

/* CCF */
static int ccf(struct gb_emu *emu, uint8_t opcode)
{
    emu->cpu.r.b[GB_REG_F] = emu->cpu.r.b[GB_REG_F] ^ GB_FLAG_CARRY;

    return 0;
}

/* SCF */
static int scf(struct gb_emu *emu, uint8_t opcode)
{
    emu->cpu.r.b[GB_REG_F] |= GB_FLAG_CARRY;

    return 0;
}

/* NOP */
static int nop(struct gb_emu *emu, uint8_t opcode)
{
    return 0;
}

/* HALT */
static int halt(struct gb_emu *emu, uint8_t opcode)
{
    emu->cpu.halted = 1;
    return 0;
}

/* STOP */
static int stop(struct gb_emu *emu, uint8_t opcode)
{
    emu->cpu.stopped = 1;
    return 0;
}

/* DI */
static int di(struct gb_emu *emu, uint8_t opcode)
{
    emu->cpu.next_ime = 0;
    emu->cpu.int_count = 1;

    return 0;
}

/* EI */
static int ei(struct gb_emu *emu, uint8_t opcode)
{
    emu->cpu.next_ime = 1;
    emu->cpu.int_count = 2;

    return 0;
}

/*
 *
 * Rotates and shifts
 *
 */

/* RLA
 * RLCA
 * RL reg
 * RLC reg */
static int rla(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    uint8_t flags = 0;
    uint8_t carry = 0;
    int reg = opcode & 0x07;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, reg, &cycles);

    /* RLA - use carry flag */
    if ((opcode & 0x10) == 0x10)
        carry = GB_FLAG_IS_SET(emu->cpu.r.b[GB_REG_F], GB_FLAG_CARRY);
    else
        carry = (tmp & 0x80) >> 7;

    uint16_t res = ((uint16_t)tmp << 1) + carry;

    if (res & 0x100)
        flags |= GB_FLAG_CARRY;

    if ((res & 0xFF) == 0)
        flags |= GB_FLAG_ZERO;

    write_8bit_reg(emu, reg, res & 0xFF, &cycles);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* RRA
 * RRCA
 * RR reg
 * RRC reg */
static int rra(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    uint8_t flags = 0;
    uint8_t carry = 0;
    int reg = opcode & 0x07;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, reg, &cycles);

    /* RRA uses carry */
    if ((opcode & 0x10) == 0x10)
        carry = GB_FLAG_IS_SET(emu->cpu.r.b[GB_REG_F], GB_FLAG_CARRY) << 7;
    else
        carry = (tmp & 0x01) << 7;

    uint8_t res = carry + (tmp >> 1);

    if (tmp & 0x01)
        flags |= GB_FLAG_CARRY;

    if (res == 0)
        flags |= GB_FLAG_ZERO;

    write_8bit_reg(emu, reg, res & 0xFF, &cycles);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* SLA reg */
static int sla(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    uint8_t flags = 0;
    int reg = opcode & 0x07;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, reg, &cycles);

    if (tmp & 0x80)
        flags |= GB_FLAG_CARRY;

    tmp <<= 1;

    if (tmp == 0)
        flags |= GB_FLAG_ZERO;

    write_8bit_reg(emu, reg, tmp, &cycles);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* SRA reg
 * SRL reg */
static int sra(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    uint8_t flags = 0;
    int reg = opcode & 0x07;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, reg, &cycles);

    if (tmp & 0x01)
        flags |= GB_FLAG_CARRY;

    tmp >>= 1;

    /* Replicate the top bit if this is a SRA */
    if ((opcode & 0x10) == 0x00 && (tmp & 0x40))
        tmp |= 0x80;

    if (tmp == 0)
        flags |= GB_FLAG_ZERO;

    write_8bit_reg(emu, reg, tmp, &cycles);
    emu->cpu.r.b[GB_REG_F] = flags;

    return cycles;
}

/* BIT b, reg */
static int bit(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    uint8_t flags = emu->cpu.r.b[GB_REG_F] & GB_FLAG_CARRY;
    int reg = opcode & 0x07;
    int bit = (opcode & 0x31) >> 3;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, reg, &cycles);

    if (!(tmp & (1 << bit)))
        flags |= GB_FLAG_ZERO;

    flags |= GB_FLAG_HCARRY;

    emu->cpu.r.b[GB_REG_F] = flags;

    /* The (HL) version takes twice as long as normal */
    if (IS_HL(reg)) {
        gb_emu_clock_tick(emu);
        cycles += 4;
    }

    return cycles;
}

/* SET b, reg */
static int set(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int reg = opcode & 0x07;
    int bit = (opcode & 0x31) >> 3;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, reg, &cycles);

    tmp |= (1 << bit);

    write_8bit_reg(emu, reg, tmp, &cycles);

    return cycles;
}

/* RES b, reg */
static int res(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int reg = opcode & 0x07;
    int bit = (opcode & 0x31) >> 3;
    uint8_t tmp;

    tmp = read_8bit_reg(emu, reg, &cycles);

    tmp &= ~(1 << bit);

    write_8bit_reg(emu, reg, tmp, &cycles);

    return cycles;
}

/*
 *
 * jumps
 *
 */

/* JP nn
 * JP cc, nn */
static int jp(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int jump = 0;
    uint8_t flags;

    cycles += 8;
    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);

    flags = emu->cpu.r.b[GB_REG_F];

    switch (opcode) {
    case 0xC3: /* unconditional */
        jump = 1;
        break;

    case 0xC2: /* Not-zero */
        if (!(flags & GB_FLAG_ZERO))
            jump = 1;
        break;

    case 0xCA: /* zero */
        if (flags & GB_FLAG_ZERO)
            jump = 1;
        break;

    case 0xD2: /* not-carry */
        if (!(flags & GB_FLAG_CARRY))
            jump = 1;
        break;

    case 0xDA: /* carry */
        if (flags & GB_FLAG_CARRY)
            jump = 1;
        break;
    }

    if (jump) {
        uint16_t addr = gb_emu_next_pc16(emu);
        gb_emu_clock_tick(emu);
        emu->cpu.r.w[GB_REG_PC] = addr;
        cycles += 4;
    } else {
        emu->cpu.r.w[GB_REG_PC] += 2;
    }

    return cycles;
}

/* JP (HL) */
static int jp_hl(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;

    emu->cpu.r.w[GB_REG_PC] = emu->cpu.r.w[GB_REG_HL];

    return cycles;
}

/* JP n (8-bits).
 * JP cc, n */
static int jp_rel(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int jump = 0;
    uint8_t flags;

    cycles += 4;
    gb_emu_clock_tick(emu);

    flags = emu->cpu.r.b[GB_REG_F];

    switch (opcode) {
    case 0x18:
        jump = 1;
        break;

    case 0x20: /* not-zero */
        if (!(flags & GB_FLAG_ZERO))
            jump = 1;
        break;

    case 0x28: /* zero */
        if (flags & GB_FLAG_ZERO)
            jump = 1;
        break;

    case 0x30: /* not-carry */
        if (!(flags & GB_FLAG_CARRY))
            jump = 1;
        break;

    case 0x38: /* carry */
        if (flags & GB_FLAG_CARRY)
            jump = 1;
        break;
    }

    if (jump) {
        gb_emu_clock_tick(emu);
        int8_t tmp = gb_emu_next_pc8(emu);
        emu->cpu.r.w[GB_REG_PC] += tmp;
        cycles += 4;
    } else {
        emu->cpu.r.w[GB_REG_PC] += 1;
    }

    return cycles;
}

/*
 *
 * calls
 *
 */

/* CALL nn,
 * CALL cc, nn */
static int call(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int jump = 0;
    uint8_t flags;

    cycles += 8;
    gb_emu_clock_tick(emu);
    gb_emu_clock_tick(emu);

    flags = emu->cpu.r.b[GB_REG_F];

    switch (opcode) {
    case 0xCD:
        jump = 1;
        break;

    case 0xC4: /* non-zero */
        if (!(flags & GB_FLAG_ZERO))
            jump = 1;
        break;

    case 0xCC: /* zero */
        if (flags & GB_FLAG_ZERO)
            jump = 1;
        break;

    case 0xD4: /* non-carry */
        if (!(flags & GB_FLAG_CARRY))
            jump = 1;
        break;

    case 0xDC: /* carry */
        if (flags & GB_FLAG_CARRY)
            jump = 1;
        break;
    }

    if (jump) {
        uint16_t addr = gb_emu_next_pc16(emu);

        cycles += push_val(emu, emu->cpu.r.w[GB_REG_PC]);
        emu->cpu.r.w[GB_REG_PC] = addr;
    } else {
        emu->cpu.r.w[GB_REG_PC] += 2;
    }

    return cycles;
}

/*
 *
 * Restarts
 *
 */

/* RST n
 * n = 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38 */
static int rst(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    uint16_t addr = ((opcode & 0x38) >> 3);

    addr *= 0x08;

    cycles += push_val(emu, emu->cpu.r.w[GB_REG_PC]);

    emu->cpu.r.w[GB_REG_PC] = addr;

    return cycles;
}

/*
 *
 * Returns
 *
 */

/* RET
 * RET cc */
static int ret(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;
    int jump = 0;
    uint8_t flags = 0;

    flags = emu->cpu.r.b[GB_REG_F];

    switch (opcode) {
    case 0xC9:
        jump = 1;
        break;

    case 0xC0:
        if (!(flags & GB_FLAG_ZERO))
            jump = 1;
        cycles += 4;
        break;

    case 0xC8:
        if (flags & GB_FLAG_ZERO)
            jump = 1;
        cycles += 4;
        break;

    case 0xD0:
        if (!(flags & GB_FLAG_CARRY))
            jump = 1;
        cycles += 4;
        break;

    case 0xD8:
        if (flags & GB_FLAG_CARRY)
            jump = 1;
        cycles += 4;
        break;
    }

    if (jump) {
        cycles += pop_val(emu, emu->cpu.r.w + GB_REG_PC);
    }

    return cycles;
}

/* RETI */
static int reti(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;

    cycles += pop_val(emu, emu->cpu.r.w + GB_REG_PC);

    emu->cpu.ime = 1;

    return cycles;
}

/* 0xCB prefix */
static int z80_emu_run_cb_inst(struct gb_emu *emu)
{
    int cycles = 0;
    uint8_t opcode;

    gb_emu_clock_tick(emu);
    opcode = gb_emu_next_pc8(emu);

    switch (opcode) {
    case 0x30 ... 0x37:
        cycles = swap(emu, opcode);
        break;

    case 0x00 ... 0x07:
    case 0x10 ... 0x17:
        cycles = rla(emu, opcode);
        break;

    case 0x08 ... 0x0F:
    case 0x18 ... 0x1F:
        cycles = rra(emu, opcode);
        break;

    case 0x20 ... 0x27:
        cycles = sla(emu, opcode);
        break;

    case 0x28 ... 0x2F:
    case 0x38 ... 0x3F:
        cycles = sra(emu, opcode);
        break;

    case 0x40 ... 0x7F:
        cycles = bit(emu, opcode);
        break;

    case 0xC0 ... 0xFF:
        cycles = set(emu, opcode);
        break;

    case 0x80 ... 0xB7:
        cycles = res(emu, opcode);
        break;
    }

    cycles += 4;

    return cycles;
}

/* Returns cycle count for this instruction */
int gb_emu_run_inst(struct gb_emu *emu, uint8_t opcode)
{
    int cycles = 0;

    switch (opcode) {
    case 0x06:
    case 0x0E:
    case 0x16:
    case 0x1E:
    case 0x26:
    case 0x2E:
        cycles = load8_reg_imm(emu, opcode);
        break;

    case 0x40 ... 0x75: /* 0x76 isn't valid */
    case 0x77 ... 0x7F: /* It's used as HALT down below */
    case 0x36:
        cycles = load8_reg_reg(emu, opcode);
        break;

    case 0x0A:
    case 0x1A:
    case 0xFA:
    case 0x3E:
    case 0xF2:
    case 0x3A:
    case 0x2A:
    case 0xF0:
        cycles = load8_a_extra(emu, opcode);
        break;

    case 0x02:
    case 0x12:
    case 0xEA:
    case 0xE2:
    case 0x32:
    case 0x22:
    case 0xE0:
        cycles = load8_extra_a(emu, opcode);
        break;

    case 0x01:
    case 0x11:
    case 0x21:
    case 0x31:
        cycles = load16_reg_imm(emu, opcode);
        break;

    case 0xF9:
        cycles = load_sp_hl(emu, opcode);
        break;

    case 0xF8:
        cycles = load_hl_sp_n(emu, opcode);
        break;

    case 0x08:
        cycles = load_mem_sp(emu, opcode);
        break;

    case 0xC5:
    case 0xD5:
    case 0xE5:
    case 0xF5:
        cycles = push(emu, opcode);
        break;

    case 0xC1:
    case 0xD1:
    case 0xE1:
    case 0xF1:
        cycles = pop(emu, opcode);
        break;

    case 0x80 ... 0x87:
    case 0xC6:
    case 0x88 ... 0x8F:
    case 0xCE:
        cycles = add_a(emu, opcode);
        break;

    case 0x90 ... 0x97:
    case 0xD6:
    case 0x98 ... 0x9F:
    case 0xDE:
        cycles = sub_a(emu, opcode);
        break;

    case 0xA0 ... 0xA7:
    case 0xE6:
        cycles = and_a(emu, opcode);
        break;

    case 0xB0 ... 0xB7:
    case 0xF6:
        cycles = or_a(emu, opcode);
        break;

    case 0xA8 ... 0xAF:
    case 0xEE:
        cycles = xor_a(emu, opcode);
        break;

    case 0xB8 ... 0xBF:
    case 0xFE:
        cycles = cp_a(emu, opcode);
        break;

    case 0x04:
    case 0x0C:
    case 0x14:
    case 0x1C:
    case 0x24:
    case 0x2C:
    case 0x34:
    case 0x3C:
        cycles = inc_reg(emu, opcode);
        break;

    case 0x05:
    case 0x0D:
    case 0x15:
    case 0x1D:
    case 0x25:
    case 0x2D:
    case 0x35:
    case 0x3D:
        cycles = dec_reg(emu, opcode);
        break;

    case 0x09:
    case 0x19:
    case 0x29:
    case 0x39:
        cycles = add_hl_reg16(emu, opcode);
        break;

    case 0xE8:
        cycles = add_sp_imm(emu, opcode);
        break;

    case 0x03:
    case 0x13:
    case 0x23:
    case 0x33:
        cycles = inc_reg16(emu, opcode);
        break;

    case 0x0B:
    case 0x1B:
    case 0x2B:
    case 0x3B:
        cycles = dec_reg16(emu, opcode);
        break;

    case 0x27:
        cycles = daa(emu, opcode);
        break;

    case 0x2F:
        cycles = cpl(emu, opcode);
        break;

    case 0x3F:
        cycles = ccf(emu, opcode);
        break;

    case 0x37:
        cycles = scf(emu, opcode);
        break;

    case 0x00:
        cycles = nop(emu, opcode);
        break;

    case 0x76:
        cycles = halt(emu, opcode);
        break;

    case 0x10:
        cycles = stop(emu, opcode);
        break;

    case 0xF3:
        cycles = di(emu, opcode);
        break;

    case 0xFB:
        cycles = ei(emu, opcode);
        break;

    case 0x07:
    case 0x17:
        cycles = rla(emu, opcode);
        break;

    case 0x0F:
    case 0x1F:
        cycles = rra(emu, opcode);
        break;

    case 0xC3:
    case 0xC2:
    case 0xCA:
    case 0xD2:
    case 0xDA:
        cycles = jp(emu, opcode);
        break;

    case 0xE9:
        cycles = jp_hl(emu, opcode);
        break;

    case 0x18:
    case 0x20:
    case 0x28:
    case 0x30:
    case 0x38:
        cycles = jp_rel(emu, opcode);
        break;

    case 0xCD:
    case 0xC4:
    case 0xCC:
    case 0xD4:
    case 0xDC:
        cycles = call(emu, opcode);
        break;

    case 0xC7:
    case 0xCF:
    case 0xD7:
    case 0xDF:
    case 0xE7:
    case 0xEF:
    case 0xF7:
    case 0xFF:
        cycles = rst(emu, opcode);
        break;

    case 0xC9:
    case 0xC0:
    case 0xC8:
    case 0xD0:
    case 0xD8:
        cycles = ret(emu, opcode);
        break;

    case 0xD9:
        cycles = reti(emu, opcode);
        break;

    case 0xCB:
        cycles = z80_emu_run_cb_inst(emu);
        break;
    }

    /* No matter what the code does, the lower bits of F are always zero. */
    emu->cpu.r.b[GB_REG_F] &= 0xF0;

    /* Check if we should enable interrupts */
    if (emu->cpu.int_count > 0) {
        emu->cpu.int_count--;
        if (emu->cpu.int_count == 0)
            emu->cpu.ime = emu->cpu.next_ime;
    }

    return cycles;
}

static int gb_emu_check_interrupt(struct gb_emu *emu)
{
    int cycles = 0;
    uint8_t int_check = emu->cpu.int_enabled & emu->cpu.int_flags;
    int i;

    /* Lower number interrupts have priority */
    for (i = 0; i < GB_INT_TOTAL; i++) {
        if (int_check & (1 << i)) {
            DEBUG_PRINTF("INTERRUPT: %d - (0x%02x IE, 0x%02x IF)\n", i, emu->cpu.int_enabled, emu->cpu.int_flags);
            cycles += push_val(emu, emu->cpu.r.w[GB_REG_PC]);

            emu->cpu.r.w[GB_REG_PC] = GB_INT_BASE_ADDR + i * 0x8;
            emu->cpu.ime = 0;

            emu->cpu.int_flags &= ~(1 << i); /* Reset this interrupt's bit, since we're servicing it */
            emu->cpu.halted = 0;
            break;
        }
    }

    return cycles;
}

int gb_emu_cpu_run_next_inst(struct gb_emu *emu)
{
    int cycles = 0;

    if (emu->cpu.halted) {
        /* When we're halted, the clock still ticks */
        gb_emu_clock_tick(emu);
        cycles += 4;
        goto inst_end;
    }

    if (emu->hook_flag) {
        uint8_t bytes[3];

        bytes[0] = gb_emu_read8(emu, emu->cpu.r.w[GB_REG_PC]);
        bytes[1] = gb_emu_read8(emu, emu->cpu.r.w[GB_REG_PC] + 1);
        bytes[2] = gb_emu_read8(emu, emu->cpu.r.w[GB_REG_PC] + 2);

        if (emu->cpu.hooks && emu->cpu.hooks->next_inst)
            (emu->cpu.hooks->next_inst) (emu->cpu.hooks, emu, bytes);
    }

    gb_emu_clock_tick(emu);
    uint8_t opcode = gb_emu_next_pc8(emu);

    /* The extra four accounts for the read from PC above */
    cycles = gb_emu_run_inst(emu, opcode) + 4;

    if (emu->hook_flag)
        if (emu->cpu.hooks && emu->cpu.hooks->end_inst)
            (emu->cpu.hooks->end_inst) (emu->cpu.hooks, emu);

inst_end:

    if (emu->cpu.ime)
        cycles += gb_emu_check_interrupt(emu);

    if (emu->break_flag) {
        int k;
        uint16_t pc = emu->cpu.r.w[GB_REG_PC];
        for (k = 0; k < emu->breakpoint_count; k++) {
            if (pc == emu->breakpoints[k]) {
                emu->stop_emu = 1;
                emu->reason = GB_EMU_BREAK;
            }
        }
   }

    return cycles;
}

enum gb_emu_stop gb_run(struct gb_emu *emu)
{
    int i;
    uint64_t cycles = 0;

    emu->stop_emu = 0;

    while (!emu->stop_emu) {
        cycles = 0;

        for (i = 0; i < 20000; i++) {
            cycles += gb_emu_cpu_run_next_inst(emu);
            if (emu->stop_emu)
                break;
        }
    }

    return emu->reason;
}

uint8_t gb_cpu_int_read8(struct gb_emu *emu, uint16_t addr, uint16_t low)
{
    return emu->cpu.int_enabled;
}

void gb_cpu_int_write8(struct gb_emu *emu, uint16_t addr, uint16_t low, uint8_t byte)
{
    emu->cpu.int_enabled = byte;
}

void gb_emu_dump_regs(struct gb_emu *emu, char *output_buf)
{
    struct gb_cpu *cpu = &emu->cpu;

    struct reg {
        char id;
        int reg;
    } regs[] = {
        { 'A', GB_REG_A },
        { 'F', GB_REG_F },
        { 'B', GB_REG_B },
        { 'C', GB_REG_C },
        { 'D', GB_REG_D },
        { 'E', GB_REG_E },
        { 'H', GB_REG_H },
        { 'L', GB_REG_L },
        { '\0', 0 },
    };

    struct reg16 {
        const char *id;
        int reg;
    } regs16[] = {
        { "AF", GB_REG_AF },
        { "BC", GB_REG_BC },
        { "DE", GB_REG_DE },
        { "HL", GB_REG_HL },
        { "SP", GB_REG_SP },
        { "PC", GB_REG_PC },
        { NULL, 0 },
    };

    struct reg *reg;
    struct reg16 *reg16;
    char buf[80];
    size_t len = 0;

    memset(buf, 0, sizeof(buf));

    len = sprintf(buf, "8REG: ");
    for (reg = regs; reg->id; reg++)
        len += sprintf(buf + len, "%c: 0x%02x ", reg->id, cpu->r.b[reg->reg]);
    len += sprintf(buf + len, "\n");

    output_buf += sprintf(output_buf, "%s", buf);

    memset(buf, 0, sizeof(buf));

    len = 0;

    len = sprintf(buf, "16REG: ");
    for (reg16 = regs16; reg16->id; reg16++)
        len += sprintf(buf + len, "%s: 0x%04x ", reg16->id, cpu->r.w[reg16->reg]);
    len += sprintf(buf + len, "\n");

    output_buf += sprintf(output_buf, "%s", buf);

    output_buf += sprintf(output_buf, "IME: %d, IE: 0x%02x, IF: 0x%02x, HALT: %d, STOP: %d\n", cpu->ime, cpu->int_enabled, cpu->int_flags, cpu->halted, cpu->stopped);
}

