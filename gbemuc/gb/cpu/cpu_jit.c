
#include "common.h"

#include <stdint.h>
#include <time.h>
#include <signal.h>

#include <jit/jit.h>

#include "debug.h"
#include "gb_internal.h"
#include "gb/cpu.h"
#include "cpu_internal.h"
#include "cpu_jit_helpers.h"

/* Handles the special case of (HL).
 * Returns the value that should be used, and modifies cycles if necessary. */
static inline jit_value_t read_8bit_reg(struct gb_cpu_jit_context *ctx, int reg)
{
    if (IS_HL(reg)) {
        gb_jit_clock_tick(ctx);
        jit_value_t hl = gb_jit_load_reg16(ctx, GB_REG_HL);
        return gb_jit_read8(ctx, hl);
    } else {
        return gb_jit_load_reg8(ctx, gb_reg_map_8bit[reg]);
    }
}

static void write_8bit_reg(struct gb_cpu_jit_context *ctx, int reg, jit_value_t val)
{
    if (IS_HL(reg)) {
        gb_jit_clock_tick(ctx);
        jit_value_t hl = gb_jit_load_reg16(ctx, GB_REG_HL);
        gb_jit_write8(ctx, hl, val);
    } else {
        gb_jit_store_reg8(ctx, gb_reg_map_8bit[reg], val);
    }
}

/*
 *
 * 8-bit loads
 *
 */

/* LD reg, imm */
static void load8_reg_imm(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    /* Pull Register number out of bits 4 through 6. */
    uint8_t reg = ((opcode & b8(00111000)) >> 3);

    /* Reading PC takes a clock-tick */
    gb_jit_clock_tick(ctx);
    jit_value_t imm = gb_jit_next_pc8(ctx);
    ctx->addr++;

    write_8bit_reg(ctx, reg, imm);

    return ;
}

/* LD reg, reg. */
static void load8_reg_reg(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    /* Dest is bits 4 through 6
     * Src is bits 1 through 3 */
    uint8_t dest = ((opcode & b8(00111000)) >> 3);
    uint8_t src = (opcode & b8(00000111));
    jit_value_t val;

    val = read_8bit_reg(ctx, src);
    write_8bit_reg(ctx, dest, val);

    return ;
}

/* LD A, (n)
 * LD A, #
 * LD A, (C)
 * LDD A, (HL)
 * LDI A, (HL)
 */
static void load8_a_extra(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int reg;
    jit_value_t val = NULL;
    jit_value_t src;

    switch (opcode) {
    case 0x0A: /* LD A, (BC) */
    case 0x1A: /* LD A, (DE) */
        if (opcode & 0x10)
            reg = GB_REG_DE;
        else
            reg = GB_REG_BC;

        val = gb_jit_read8(ctx, gb_jit_load_reg16(ctx, reg));
        break;

    case 0xFA: /* LD A, (nn) */
        /* Two clock ticks for reading a 16-bit from PC */
        gb_jit_clock_tick(ctx);
        gb_jit_clock_tick(ctx);

        src = gb_jit_next_pc16(ctx);
        val = gb_jit_read8(ctx, src);
        ctx->addr += 2;
        break;

    case 0x3E: /* LD A, # */
        val = gb_jit_next_pc8(ctx);
        ctx->addr++;
        break;

    case 0xF2: /* LD A, (C) */
        src = jit_insn_add(ctx->func, gb_jit_load_reg8(ctx, GB_REG_C),
                                 GB_JIT_CONST_USHORT(ctx->func, 0xFF00));
        val = gb_jit_read8(ctx, src);
        break;

    case 0xF0:
        /* 8-bit read from PC */
        gb_jit_clock_tick(ctx);
        src = jit_insn_add(ctx->func, gb_jit_next_pc8(ctx),
                                 GB_JIT_CONST_USHORT(ctx->func, 0xFF00));
        val = gb_jit_read8(ctx, src);
        ctx->addr++;
        break;

    case 0x3A: /* LDD A, (HL) */
    case 0x2A: /* LDI A, (HL) */
        src = gb_jit_load_reg16(ctx, GB_REG_HL);
        if (opcode == 0x3A) {
            gb_jit_store_reg16(ctx, GB_REG_HL,
                jit_insn_sub(ctx->func, src, GB_JIT_CONST_USHORT(ctx->func, 1)));
        } else {
            gb_jit_store_reg16(ctx, GB_REG_HL,
                jit_insn_add(ctx->func, src, GB_JIT_CONST_USHORT(ctx->func, 1)));
        }

        val = gb_jit_read8(ctx, src);
        break;
    }

    /* Accounts for 8-bit read in every path. */
    gb_jit_clock_tick(ctx);

    gb_jit_store_reg8(ctx, GB_REG_A, val);

    return ;
}

/* LD (n), A
 * LDD (HL), A
 * LDI (HL), A
 */
static void load8_extra_a(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int reg;
    jit_value_t dest = NULL;

    switch (opcode) {
    case 0x02: /* LD (BC), A */
    case 0x12: /* LD (DE), A */
        if (opcode & 0x10)
            reg = GB_REG_DE;
        else
            reg = GB_REG_BC;

        dest = gb_jit_load_reg16(ctx, reg);
        break;

    case 0xEA: /* LD (nn), A */
        gb_jit_clock_tick(ctx);
        gb_jit_clock_tick(ctx);
        dest = gb_jit_next_pc16(ctx);
        ctx->addr += 2;
        break;

    case 0xE2: /* LD (0xFF00 + C), A */
        dest = jit_insn_add(ctx->func, gb_jit_load_reg8(ctx, GB_REG_C),
                                  GB_JIT_CONST_USHORT(ctx->func, 0xFF00));
        break;

    case 0xE0: /* LD (0xFF00 + n), A */
        gb_jit_clock_tick(ctx);
        jit_value_t val = gb_jit_next_pc8(ctx);
        dest = jit_insn_add(ctx->func, val,
                                  GB_JIT_CONST_USHORT(ctx->func, 0xFF00));
        ctx->addr++;
        break;

    case 0x32: /* LDD (HL), A */
    case 0x22: /* LDI (HL), A */
        dest = gb_jit_load_reg16(ctx, GB_REG_HL);
        if (opcode == 0x32) {
            gb_jit_store_reg16(ctx, GB_REG_HL,
                    jit_insn_sub(ctx->func, dest, GB_JIT_CONST_USHORT(ctx->func, 1)));
        } else {
            gb_jit_store_reg16(ctx, GB_REG_HL,
                    jit_insn_add(ctx->func, dest, GB_JIT_CONST_USHORT(ctx->func, 1)));
        }
        break;
    }

    gb_jit_clock_tick(ctx);
    gb_jit_write8(ctx, dest, gb_jit_load_reg8(ctx, GB_REG_A));

    return ;
}

/*
 *
 * 16-bit Loads
 *
 */

/* LD n, nn */
static void load16_reg_imm(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int dest = gb_reg_map_16bit_sp[(opcode & 0x30) >> 4];

    gb_jit_clock_tick(ctx);
    gb_jit_clock_tick(ctx);
    jit_value_t val = gb_jit_next_pc16(ctx);
    ctx->addr += 2;

    gb_jit_store_reg16(ctx, dest, val);

    return ;
}

/* LD SP, HL */
static void load_sp_hl(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    /* Extra clock-tick for 16-bit loads */
    gb_jit_clock_tick(ctx);

    gb_jit_store_reg16(ctx, GB_REG_SP, gb_jit_load_reg16(ctx, GB_REG_HL));

    return ;
}

/* LDHL SP, n */
static void load_hl_sp_n(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_value_t flags;
    jit_value_t val;
    jit_label_t tmp_label;

    /* Read 8-bit from PC */
    gb_jit_clock_tick(ctx);
    jit_value_t off = gb_jit_next_pc8(ctx);
    ctx->addr++;

    /* Extra clock tick for 16-bit load */
    gb_jit_clock_tick(ctx);

    val = jit_insn_add(ctx->func, gb_jit_load_reg16(ctx, GB_REG_SP), off);

    flags = jit_value_create(ctx->func, jit_type_ubyte);

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    jit_value_t flag_check_value;

    jit_value_t tmp;
    tmp = gb_jit_load_reg16(ctx, GB_REG_SP);
    tmp = jit_insn_xor(ctx->func, tmp, off);
    flag_check_value = jit_insn_xor(ctx->func, tmp, val);

    tmp = jit_insn_and(ctx->func, flag_check_value, GB_JIT_CONST_USHORT(ctx->func, 0x100));
    tmp_label = jit_label_undefined;

    tmp = jit_insn_eq(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 0));
    jit_insn_branch_if(ctx->func, tmp, &tmp_label);
    jit_insn_store(ctx->func, flags, jit_insn_or(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY)));
    jit_insn_label(ctx->func, &tmp_label);

    tmp = jit_insn_and(ctx->func, flag_check_value, GB_JIT_CONST_UBYTE(ctx->func, 0x10));
    tmp_label = jit_label_undefined;

    tmp = jit_insn_eq(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 0));
    jit_insn_branch_if(ctx->func, tmp, &tmp_label);
    jit_insn_store(ctx->func, flags, jit_insn_or(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_HCARRY)));
    jit_insn_label(ctx->func, &tmp_label);

    gb_jit_store_reg16(ctx, GB_REG_HL, val);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* LD (nn), SP */
static void load_mem_sp(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    /* 16-bit read */
    gb_jit_clock_tick(ctx);
    gb_jit_clock_tick(ctx);
    jit_value_t dest = gb_jit_next_pc16(ctx);
    ctx->addr += 2;

    /* 16-bit write */
    gb_jit_clock_tick(ctx);
    gb_jit_clock_tick(ctx);
    gb_jit_write16(ctx, dest, gb_jit_load_reg16(ctx, GB_REG_SP));

    return ;
}

static void push_val(struct gb_cpu_jit_context *ctx, jit_value_t val)
{
    /* Extra clock-tick for 16-bit */
    gb_jit_clock_tick(ctx);
    jit_value_t tmp = gb_jit_load_reg16(ctx, GB_REG_SP);
    tmp = jit_insn_sub(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 2));
    gb_jit_store_reg16(ctx, GB_REG_SP, tmp);

    /* 16-bit write */
    gb_jit_clock_tick(ctx);
    gb_jit_clock_tick(ctx);
    gb_jit_write16(ctx, tmp, val);

    return ;
}

static jit_value_t pop_val(struct gb_cpu_jit_context *ctx)
{
    /* 16-bit read */
    gb_jit_clock_tick(ctx);
    gb_jit_clock_tick(ctx);
    jit_value_t dest = gb_jit_read16(ctx, gb_jit_load_reg16(ctx, GB_REG_SP));

    /* Extra clock-tick for 16-bit */
    gb_jit_clock_tick(ctx);
    jit_value_t tmp = gb_jit_load_reg16(ctx, GB_REG_SP);
    tmp = jit_insn_add(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 2));
    gb_jit_store_reg16(ctx, GB_REG_SP, tmp);

    return dest;
}

/* PUSH reg
 * reg = AF, BC, DE, HL */
static void push(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int dest = gb_reg_map_16bit_af[(opcode & 0x30) >> 4];

    jit_value_t tmp = gb_jit_load_reg16(ctx, dest);

    push_val(ctx, tmp);

    return ;
}

/* POP reg
 * reg = AF, BC, DE, HL */
static void pop(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int dest = gb_reg_map_16bit_af[(opcode & 0x30) >> 4];

    jit_value_t tmp = pop_val(ctx);
    gb_jit_store_reg16(ctx, dest, tmp);

    return ;
}

/*
 *
 * 8-bit ALU
 *
 */

/* Performs an 8-bit addition, with optional carry.
 *
 * Returns the result of the add */
static jit_value_t adc(struct gb_cpu_jit_context *ctx, jit_value_t val1, jit_value_t val2, jit_value_t carry)
{
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    jit_value_t val1_result = jit_insn_and(ctx->func, val1, GB_JIT_CONST_UBYTE(ctx->func, 0x0F));
    jit_value_t val2_result = jit_insn_and(ctx->func, val2, GB_JIT_CONST_UBYTE(ctx->func, 0x0F));
    jit_value_t add_result = jit_insn_add(ctx->func, val1_result, jit_insn_add(ctx->func, val2_result, carry));

    gb_jit_set_flag_if_gt(ctx->func, add_result, GB_JIT_CONST_UBYTE(ctx->func, 0x0F), flags, GB_FLAG_HCARRY);

    val1_result = jit_insn_convert(ctx->func, val1, jit_type_ushort, 0);
    val2_result = jit_insn_convert(ctx->func, val2, jit_type_ushort, 0);
    add_result = jit_insn_add(ctx->func, val1_result, jit_insn_add(ctx->func, val2_result, carry));

    gb_jit_set_flag_if_gt(ctx->func, add_result, GB_JIT_CONST_USHORT(ctx->func, 0xFF), flags, GB_FLAG_CARRY);

    jit_value_t result = jit_insn_and(ctx->func, jit_insn_add(ctx->func, val1_result, jit_insn_add(ctx->func, val2_result, carry)), GB_JIT_CONST_USHORT(ctx->func, 0xFF));

    gb_jit_set_flag_if_zero(ctx->func, result, flags, GB_FLAG_ZERO);

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return result;
}

/* ADD A, reg
 * ADD A, #
 *
 * ADC A, reg
 * ADC A, #
 */
static void add_a(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = opcode & 0x07;
    jit_value_t carry = GB_JIT_CONST_UBYTE(ctx->func, 0);
    jit_value_t tmp;

    /* 0xC0 means it's an immediate value */
    if ((opcode & 0xF0) != 0xC0) {
        tmp = read_8bit_reg(ctx, src);
    } else {
        gb_jit_clock_tick(ctx);
        tmp = gb_jit_next_pc8(ctx);
        ctx->addr++;
    }

    /* ADC commands add the carry flag */
    if ((opcode & 0x0F) >= 0x08 || opcode == 0xCE) {
        carry = gb_jit_load_reg8(ctx, GB_REG_F);
        carry = jit_insn_and(ctx->func, carry, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
        /* Shifts carry from 0x10 to 0x01 */
        carry = jit_insn_shr(ctx->func, carry, GB_JIT_CONST_UBYTE(ctx->func, 4));
    }

    gb_jit_clock_tick(ctx);
    tmp = adc(ctx, gb_jit_load_reg8(ctx, GB_REG_A), tmp, carry);
    gb_jit_store_reg8(ctx, GB_REG_A, tmp);

    return ;
}

static jit_value_t sbc(struct gb_cpu_jit_context *ctx, jit_value_t val1, jit_value_t val2, jit_value_t carry)
{
    jit_value_t result;
    jit_value_t tmp;
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_SUB));

    tmp = jit_insn_sub(ctx->func, val1, val2);
    tmp = jit_insn_sub(ctx->func, tmp, carry);
    result = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0xFF));

    jit_value_t val1_16 = jit_insn_convert(ctx->func, val1, jit_type_ushort, 0);
    jit_value_t val2_16 = jit_insn_convert(ctx->func, val2, jit_type_ushort, 0);
    tmp = jit_insn_sub(ctx->func, val1_16, val2_16);
    tmp = jit_insn_sub(ctx->func, tmp, carry);
    tmp = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 0xFF00));

    gb_jit_set_flag_if_nonzero(ctx->func, tmp, flags, GB_FLAG_CARRY);

    tmp = jit_insn_xor(ctx->func, val1, val2);
    tmp = jit_insn_xor(ctx->func, tmp, result);
    tmp = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x10));

    gb_jit_set_flag_if_nonzero(ctx->func, tmp, flags, GB_FLAG_HCARRY);

    gb_jit_set_flag_if_zero(ctx->func, result, flags, GB_FLAG_ZERO);

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return result;
}

/* SUB reg
 * SUB #
 */
static void sub_a(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = opcode & 0x07;
    jit_value_t tmp, carry;

    /* 0xD0 means it's am immediate value */
    if ((opcode & 0xF0) != 0xD0) {
        tmp = read_8bit_reg(ctx, src);
    } else {
        gb_jit_clock_tick(ctx);
        tmp = gb_jit_next_pc8(ctx);
        ctx->addr++;
    }

    /* SBC commands add the carry flag to the front of 'a'. */
    if ((opcode & 0x0F) >= 0x08 || opcode == 0xDE) {
        carry = gb_jit_load_reg8(ctx, GB_REG_F);
        carry = jit_insn_and(ctx->func, carry, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
        /* Shifts carry from 0x10 to 0x01 */
        carry = jit_insn_shr(ctx->func, carry, GB_JIT_CONST_UBYTE(ctx->func, 4));
    } else {
        carry = GB_JIT_CONST_UBYTE(ctx->func, 0);
    }

    jit_value_t a = gb_jit_load_reg8(ctx, GB_REG_A);
    gb_jit_store_reg8(ctx, GB_REG_A, sbc(ctx, a, tmp, carry));

    return ;
}

/* AND reg
 * AND n */
static void and_a(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = opcode & 0x07;
    jit_value_t tmp;
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_HCARRY));

    /* Check if this is not the immediate value opcode 0xE6 */
    if ((opcode & 0xF0) != 0xE0) {
        tmp = read_8bit_reg(ctx, src);
    } else {
        gb_jit_clock_tick(ctx);
        tmp = gb_jit_next_pc8(ctx);
        ctx->addr++;
    }

    jit_value_t a = gb_jit_load_reg8(ctx, GB_REG_A);
    a = jit_insn_and(ctx->func, a, tmp);
    gb_jit_store_reg8(ctx, GB_REG_A, a);

    gb_jit_set_flag_if_zero(ctx->func, a, flags, GB_FLAG_ZERO);

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* OR reg
 * OR n */
static void or_a(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = opcode & 0x07;
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_value_t tmp;

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    /* Check if this is not the immediate value opcode 0xF6 */
    if ((opcode & 0xF0) != 0xF0) {
        tmp = read_8bit_reg(ctx, src);
    } else {
        gb_jit_clock_tick(ctx);
        tmp = gb_jit_next_pc8(ctx);
        ctx->addr++;
    }

    jit_value_t a = gb_jit_load_reg8(ctx, GB_REG_A);
    a = jit_insn_or(ctx->func, a, tmp);
    gb_jit_store_reg8(ctx, GB_REG_A, a);

    gb_jit_set_flag_if_zero(ctx->func, a, flags, GB_FLAG_ZERO);

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* XOR reg
 * XOR n */
static void xor_a(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = opcode & 0x07;
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_value_t tmp;

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    /* Check if this is not the immediate value opcode 0xEE */
    if ((opcode & 0xF0) != 0xE0) {
        tmp = read_8bit_reg(ctx, src);
    } else {
        gb_jit_clock_tick(ctx);
        tmp = gb_jit_next_pc8(ctx);
        ctx->addr++;
    }

    jit_value_t a = gb_jit_load_reg8(ctx, GB_REG_A);
    a = jit_insn_xor(ctx->func, a, tmp);
    gb_jit_store_reg8(ctx, GB_REG_A, a);

    gb_jit_set_flag_if_zero(ctx->func, a, flags, GB_FLAG_ZERO);

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* CP reg
 * CP n */
static void cp_a(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = opcode & 0x07;
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_value_t tmp;

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    if ((opcode & 0xF0) != 0xF0) {
        tmp = read_8bit_reg(ctx, src);
    } else {
        gb_jit_clock_tick(ctx);
        tmp = gb_jit_next_pc8(ctx);
        ctx->addr++;
    }

    jit_value_t a = gb_jit_load_reg8(ctx, GB_REG_A);

    gb_jit_set_flag_if_eq(ctx->func, a, tmp, flags, GB_FLAG_ZERO);

    jit_value_t a_low = jit_insn_and(ctx->func, a, GB_JIT_CONST_UBYTE(ctx->func, 0xF));
    jit_value_t tmp_low = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0xF));
    gb_jit_set_flag_if_lt(ctx->func, a_low, tmp_low, flags, GB_FLAG_HCARRY);

    gb_jit_set_flag_if_lt(ctx->func, a, tmp, flags, GB_FLAG_CARRY);

    jit_insn_store(ctx->func, flags, jit_insn_or(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_SUB)));

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* INC reg */
static void inc_reg(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = (opcode & 0x38) >> 3;
    jit_value_t flags, tmp;

    flags = gb_jit_load_reg8(ctx, GB_REG_F);
    flags = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));

    tmp = read_8bit_reg(ctx, src);

    jit_value_t tmp_low = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x0F));
    gb_jit_set_flag_if_eq(ctx->func, tmp_low, GB_JIT_CONST_UBYTE(ctx->func, 0x0F), flags, GB_FLAG_HCARRY);

    gb_jit_set_flag_if_eq(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0xFF), flags, GB_FLAG_ZERO);

    jit_value_t tmp_16bit = jit_insn_convert(ctx->func, tmp, jit_type_ushort, 0);
    tmp_16bit = jit_insn_add(ctx->func, tmp_16bit, GB_JIT_CONST_USHORT(ctx->func, 1));
    tmp = jit_insn_convert(ctx->func, tmp_16bit, jit_type_ubyte, 0);

    write_8bit_reg(ctx, src, tmp);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* DEC reg */
static void dec_reg(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = (opcode & 0x38) >> 3;
    jit_value_t flags, tmp;

    flags = gb_jit_load_reg8(ctx, GB_REG_F);
    flags = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
    flags = jit_insn_or(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_SUB));

    tmp = read_8bit_reg(ctx, src);

    jit_value_t tmp_low = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x0F));
    gb_jit_set_flag_if_zero(ctx->func, tmp_low, flags, GB_FLAG_HCARRY);

    gb_jit_set_flag_if_eq(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 1), flags, GB_FLAG_ZERO);

    jit_value_t tmp_16bit = jit_insn_convert(ctx->func, tmp, jit_type_ushort, 0);
    tmp_16bit = jit_insn_sub(ctx->func, tmp_16bit, GB_JIT_CONST_USHORT(ctx->func, 1));
    tmp = jit_insn_convert(ctx->func, tmp_16bit, jit_type_ubyte, 0);

    write_8bit_reg(ctx, src, tmp);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/*
 *
 * 16-bit ALU
 *
 */

/* ADD HL, reg */
static void add_hl_reg16(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = (opcode & 0x30) >> 4;
    int reg = gb_reg_map_16bit_sp[src];
    jit_value_t flags;

    flags = gb_jit_load_reg8(ctx, GB_REG_F);
    flags = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_ZERO));

    jit_value_t hl = gb_jit_load_reg16(ctx, GB_REG_HL);
    jit_value_t reg16 = gb_jit_load_reg16(ctx, reg);
    jit_value_t hl_low = jit_insn_and(ctx->func, hl, GB_JIT_CONST_USHORT(ctx->func, 0x0FFF));
    jit_value_t reg16_low = jit_insn_and(ctx->func, reg16, GB_JIT_CONST_USHORT(ctx->func, 0x0FFF));
    jit_value_t tmp = jit_insn_add(ctx->func, hl_low, reg16_low);

    gb_jit_set_flag_if_gt(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 0x0FFF), flags, GB_FLAG_HCARRY);

    jit_value_t hl_32bit = jit_insn_convert(ctx->func, hl, jit_type_uint, 0);
    jit_value_t reg_32bit = jit_insn_convert(ctx->func, reg16, jit_type_uint, 0);
    jit_value_t add_32bit = jit_insn_add(ctx->func, hl_32bit, reg_32bit);

    gb_jit_set_flag_if_gt(ctx->func, add_32bit, GB_JIT_CONST_UINT(ctx->func, 0xFFFF), flags, GB_FLAG_CARRY);

    gb_jit_clock_tick(ctx);
    jit_value_t add_16bit = jit_insn_convert(ctx->func, add_32bit, jit_type_ushort, 0);
    gb_jit_store_reg16(ctx, GB_REG_HL, add_16bit);

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* ADD SP, # */
static void add_sp_imm(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_value_t tmp;

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    gb_jit_clock_tick(ctx);
    tmp = gb_jit_next_pc8(ctx);
    ctx->addr++;

    jit_value_t sp = gb_jit_load_reg16(ctx, GB_REG_SP);
    jit_value_t result = jit_insn_add(ctx->func, sp, tmp);

    /* From Mednafem - magic to do the carry and hcarry flags */
    jit_value_t xor_value = jit_insn_xor(ctx->func, sp, jit_insn_xor(ctx->func, tmp, result));

    gb_jit_set_flag_if_nonzero(ctx->func, jit_insn_and(ctx->func, xor_value, GB_JIT_CONST_USHORT(ctx->func, 0x100)), flags, GB_FLAG_CARRY);
    gb_jit_set_flag_if_nonzero(ctx->func, jit_insn_and(ctx->func, xor_value, GB_JIT_CONST_USHORT(ctx->func, 0x10)), flags, GB_FLAG_HCARRY);

    gb_jit_clock_tick(ctx);
    gb_jit_clock_tick(ctx);
    gb_jit_store_reg16(ctx, GB_REG_SP, result);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* INC nn */
static void inc_reg16(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = (opcode & 0x30) >> 4;
    int reg = gb_reg_map_16bit_sp[src];

    gb_jit_clock_tick(ctx);
    jit_value_t reg_val = gb_jit_load_reg16(ctx, reg);
    reg_val = jit_insn_add(ctx->func, reg_val, GB_JIT_CONST_USHORT(ctx->func, 1));
    gb_jit_store_reg16(ctx, reg, reg_val);

    return ;
}

/* DEC nn */
static void dec_reg16(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = (opcode & 0x30) >> 4;
    int reg = gb_reg_map_16bit_sp[src];

    gb_jit_clock_tick(ctx);
    jit_value_t reg_val = gb_jit_load_reg16(ctx, reg);
    reg_val = jit_insn_sub(ctx->func, reg_val, GB_JIT_CONST_USHORT(ctx->func, 1));
    gb_jit_store_reg16(ctx, reg, reg_val);

    return ;
}

/*
 *
 * Misc ops
 *
 */

/* SWAP reg */
static void swap(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int src = (opcode & 0x7);
    jit_value_t flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_value_t tmp;

    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    tmp = read_8bit_reg(ctx, src);

    gb_jit_set_flag_if_zero(ctx->func, tmp, flags, GB_FLAG_ZERO);

    jit_value_t left_half = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0xF0));
    jit_value_t right_half = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x0F));

    left_half = jit_insn_shr(ctx->func, left_half, GB_JIT_CONST_UBYTE(ctx->func, 4));
    right_half = jit_insn_shl(ctx->func, right_half, GB_JIT_CONST_UBYTE(ctx->func, 4));

    tmp = jit_insn_or(ctx->func, left_half, right_half);

    write_8bit_reg(ctx, src, tmp);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* DAA */
static void daa(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_value_t lookup;

    lookup = gb_jit_load_reg8(ctx, GB_REG_A);
    lookup = jit_insn_convert(ctx->func, lookup, jit_type_ushort, 0);

    jit_value_t flags = gb_jit_load_reg8(ctx, GB_REG_F);
    flags = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY | GB_FLAG_HCARRY | GB_FLAG_SUB));
    flags = jit_insn_convert(ctx->func, flags, jit_type_ushort, 0);
    flags = jit_insn_shl(ctx->func, flags, GB_JIT_CONST_USHORT(ctx->func, 4));
    lookup = jit_insn_or(ctx->func, lookup, flags);

    jit_value_t af = jit_insn_load_elem(ctx->func, GB_JIT_CONST_PTR(ctx->func, gb_daa_table), lookup, jit_type_ushort);
    gb_jit_store_reg16(ctx, GB_REG_AF, af);

    return ;
}

/* CPL */
static void cpl(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_value_t a = gb_jit_load_reg8(ctx, GB_REG_A);
    a = jit_insn_not(ctx->func, a);
    gb_jit_store_reg8(ctx, GB_REG_A, a);

    jit_value_t flags = gb_jit_load_reg8(ctx, GB_REG_F);
    flags = jit_insn_or(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_SUB | GB_FLAG_HCARRY));
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* CCF */
static void ccf(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_value_t flags = gb_jit_load_reg8(ctx, GB_REG_F);

    flags = jit_insn_xor(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
    flags = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, ~(GB_FLAG_SUB | GB_FLAG_HCARRY)));

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* SCF */
static void scf(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_value_t flags = gb_jit_load_reg8(ctx, GB_REG_F);

    flags = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_ZERO));
    flags = jit_insn_or(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* NOP */
static void nop(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_insn_nop(ctx->func);
    return ;
}

/* HALT */
static void halt(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.halted), GB_JIT_CONST_UBYTE(ctx->func, 1));
    return ;
}

/* STOP */
static void stop(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    if (!gb_emu_is_cgb(ctx->gb_emu)) {
        jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.stopped), GB_JIT_CONST_UBYTE(ctx->func, 1));
        return ;
    }

    jit_label_t tmp_label = jit_label_undefined;
    jit_label_t end_label = jit_label_undefined;

    jit_value_t do_speed = jit_insn_load_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.do_speed_switch), jit_type_ubyte);
    jit_value_t cmp = jit_insn_eq(ctx->func, do_speed, GB_JIT_CONST_UBYTE(ctx->func, 0));

    jit_insn_branch_if_not(ctx->func, cmp, &tmp_label);
    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.stopped), GB_JIT_CONST_UBYTE(ctx->func, 1));
    jit_insn_branch(ctx->func, &end_label);

    jit_insn_label(ctx->func, &tmp_label);
    jit_value_t ds = jit_insn_load_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.double_speed), jit_type_ubyte);
    ds = jit_insn_xor(ctx->func, ds, GB_JIT_CONST_UBYTE(ctx->func, 1));
    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.double_speed), ds);

    jit_insn_label(ctx->func, &end_label);

    return ;
}

/* DI */
static void di(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.next_ime), GB_JIT_CONST_UBYTE(ctx->func, 0));
    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.int_count), GB_JIT_CONST_UBYTE(ctx->func, 1));

    return ;
}

/* EI */
static void ei(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.next_ime), GB_JIT_CONST_UBYTE(ctx->func, 1));
    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.int_count), GB_JIT_CONST_UBYTE(ctx->func, 2));

    return ;
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
static void rla(struct gb_cpu_jit_context *ctx, uint8_t opcode, int is_cb)
{
    int reg = opcode & 0x07;
    jit_value_t flags, carry, tmp;

    tmp = read_8bit_reg(ctx, reg);

    flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    /* RLA - use carry flag */
    if ((opcode & 0x10) == 0x10) {
        jit_value_t f = gb_jit_load_reg8(ctx, GB_REG_F);
        f = jit_insn_and(ctx->func, f, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
        carry = jit_insn_shr(ctx->func, f, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_SHIFT_CARRY));
    } else {
        carry = jit_insn_shr(ctx->func, jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x80)), GB_JIT_CONST_UBYTE(ctx->func, 7));
    }

    jit_value_t res = jit_insn_convert(ctx->func, tmp, jit_type_ushort, 0);
    res = jit_insn_shl(ctx->func, res, GB_JIT_CONST_USHORT(ctx->func, 1));
    res = jit_insn_or(ctx->func, res, carry);

    tmp = jit_insn_and(ctx->func, res, GB_JIT_CONST_USHORT(ctx->func, 0x100));
    gb_jit_set_flag_if_nonzero(ctx->func, tmp, flags, GB_FLAG_CARRY);

    res = jit_insn_convert(ctx->func, res, jit_type_ubyte, 0);

    if (is_cb)
        gb_jit_set_flag_if_zero(ctx->func, res, flags, GB_FLAG_ZERO);

    write_8bit_reg(ctx, reg, res);

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* RRA
 * RRCA
 * RR reg
 * RRC reg */
static void rra(struct gb_cpu_jit_context *ctx, uint8_t opcode, int is_cb)
{
    int reg = opcode & 0x07;
    jit_value_t flags, carry, tmp;

    tmp = read_8bit_reg(ctx, reg);

    flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    /* RRA uses carry */
    if ((opcode & 0x10) == 0x10) {
        jit_value_t f = gb_jit_load_reg8(ctx, GB_REG_F);
        f = jit_insn_and(ctx->func, f, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
        carry = jit_insn_shr(ctx->func, f, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_SHIFT_CARRY));
        carry = jit_insn_shl(ctx->func, carry, GB_JIT_CONST_UBYTE(ctx->func, 7));
    } else {
        carry = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x01));
        carry = jit_insn_shl(ctx->func, carry, GB_JIT_CONST_UBYTE(ctx->func, 7));
    }

    jit_value_t res = jit_insn_shr(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 1));
    res = jit_insn_or(ctx->func, res, carry);

    tmp = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x01));
    gb_jit_set_flag_if_nonzero(ctx->func, tmp, flags, GB_FLAG_CARRY);

    if (is_cb)
        gb_jit_set_flag_if_zero(ctx->func, res, flags, GB_FLAG_ZERO);

    write_8bit_reg(ctx, reg, res);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* SLA reg */
static void sla(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int reg = opcode & 0x07;
    jit_value_t tmp, flags;

    flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    tmp = read_8bit_reg(ctx, reg);

    jit_value_t test = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x80));
    gb_jit_set_flag_if_nonzero(ctx->func, test, flags, GB_FLAG_CARRY);

    tmp = jit_insn_shl(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 1));

    gb_jit_set_flag_if_zero(ctx->func, tmp, flags, GB_FLAG_ZERO);

    write_8bit_reg(ctx, reg, tmp);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* SRA reg
 * SRL reg */
static void sra(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int reg = opcode & 0x07;
    jit_value_t tmp, flags;

    flags = jit_value_create(ctx->func, jit_type_ubyte);
    jit_insn_store(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, 0));

    tmp = read_8bit_reg(ctx, reg);

    jit_value_t test = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0x01));
    gb_jit_set_flag_if_nonzero(ctx->func, test, flags, GB_FLAG_CARRY);

    /* Replicate the top bit if this is a SRA */
    if ((opcode & 0x10) == 0x00)
        tmp = jit_insn_sshr(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 1));
    else
        tmp = jit_insn_shr(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 1));

    gb_jit_set_flag_if_zero(ctx->func, tmp, flags, GB_FLAG_ZERO);

    write_8bit_reg(ctx, reg, tmp);
    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    return ;
}

/* BIT b, reg */
static void bit(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int reg = opcode & 0x07;
    int bit = (opcode & 0x38) >> 3;
    jit_value_t flags, tmp;

    flags = gb_jit_load_reg8(ctx, GB_REG_F);
    flags = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));

    tmp = read_8bit_reg(ctx, reg);

    jit_value_t test = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, (1 << bit)));
    gb_jit_set_flag_if_zero(ctx->func, test, flags, GB_FLAG_ZERO);

    jit_insn_or(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_HCARRY));

    gb_jit_store_reg8(ctx, GB_REG_F, flags);

    /* The (HL) version takes twice as long as normal */
    if (IS_HL(reg))
        gb_jit_clock_tick(ctx);

    return ;
}

/* SET b, reg */
static void set(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int reg = opcode & 0x07;
    int bit = (opcode & 0x38) >> 3;
    jit_value_t tmp;

    tmp = read_8bit_reg(ctx, reg);
    tmp = jit_insn_or(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, (1 << bit)));

    write_8bit_reg(ctx, reg, tmp);

    return ;
}

/* RES b, reg */
static void res(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int reg = opcode & 0x07;
    int bit = (opcode & 0x38) >> 3;
    jit_value_t tmp;

    tmp = read_8bit_reg(ctx, reg);
    tmp = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, ~(1 << bit)));

    write_8bit_reg(ctx, reg, tmp);

    return ;
}

/*
 *
 * jumps
 *
 */

enum jump_type {
    JUMP_TYPE_DIRECT,
    JUMP_TYPE_RELATIVE,
    JUMP_TYPE_CALL,
    JUMP_TYPE_RET,
};

static void jp_all(struct gb_cpu_jit_context *ctx, uint8_t opcode, enum jump_type jump_type)
{
    jit_value_t flags, test;
    jit_value_t jump = jit_value_create(ctx->func, jit_type_ubyte);;
    jit_label_t tmp_label, end_label;

    jit_insn_store(ctx->func, jump, GB_JIT_CONST_UBYTE(ctx->func, 0));

    if (jump_type != JUMP_TYPE_RET) {
        gb_jit_clock_tick(ctx);
        if (jump_type != JUMP_TYPE_RELATIVE)
            gb_jit_clock_tick(ctx);
    }

    flags = gb_jit_load_reg8(ctx, GB_REG_F);

    tmp_label = jit_label_undefined;
    switch (opcode) {
    case 0xC3: /* unconditional */
    case 0x18:
    case 0xCD:
    case 0xC9:
        jit_insn_store(ctx->func, jump, GB_JIT_CONST_UBYTE(ctx->func, 1));
        break;

    case 0xC2: /* Not-zero */
    case 0x20:
    case 0xC4:
    case 0xC0:
        test = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_ZERO));
        jit_insn_branch_if(ctx->func, test, &tmp_label);
        jit_insn_store(ctx->func, jump, GB_JIT_CONST_UBYTE(ctx->func, 1));
        jit_insn_label(ctx->func, &tmp_label);
        break;

    case 0xCA: /* zero */
    case 0x28:
    case 0xCC:
    case 0xC8:
        test = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_ZERO));
        jit_insn_branch_if_not(ctx->func, test, &tmp_label);
        jit_insn_store(ctx->func, jump, GB_JIT_CONST_UBYTE(ctx->func, 1));
        jit_insn_label(ctx->func, &tmp_label);
        break;

    case 0xD2: /* not-carry */
    case 0x30:
    case 0xD4:
    case 0xD0:
        test = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
        jit_insn_branch_if(ctx->func, test, &tmp_label);
        jit_insn_store(ctx->func, jump, GB_JIT_CONST_UBYTE(ctx->func, 1));
        jit_insn_label(ctx->func, &tmp_label);
        break;

    case 0xDA: /* carry */
    case 0x38:
    case 0xDC:
    case 0xD8:
        test = jit_insn_and(ctx->func, flags, GB_JIT_CONST_UBYTE(ctx->func, GB_FLAG_CARRY));
        jit_insn_branch_if_not(ctx->func, test, &tmp_label);
        jit_insn_store(ctx->func, jump, GB_JIT_CONST_UBYTE(ctx->func, 1));
        jit_insn_label(ctx->func, &tmp_label);
        break;
    }

    tmp_label = jit_label_undefined;
    end_label = jit_label_undefined;

    jit_insn_branch_if_not(ctx->func, jump, &tmp_label);
    {
        jit_value_t tmp;
        switch (jump_type) {
        case JUMP_TYPE_DIRECT:
            gb_jit_clock_tick(ctx);
            tmp = gb_jit_next_pc16(ctx);
            gb_jit_store_reg16(ctx, GB_REG_PC, tmp);
            ctx->addr += 2;
            break;

        case JUMP_TYPE_RELATIVE:
            gb_jit_clock_tick(ctx);
            jit_value_t tmp = gb_jit_next_pc8(ctx);
            tmp = jit_insn_convert(ctx->func, tmp, jit_type_sbyte, 0);
            tmp = jit_insn_add(ctx->func, gb_jit_load_reg16(ctx, GB_REG_PC), tmp);
            gb_jit_store_reg16(ctx, GB_REG_PC, tmp);
            ctx->addr++;
            break;

        case JUMP_TYPE_CALL:
            tmp = gb_jit_next_pc16(ctx);
            push_val(ctx, gb_jit_load_reg16(ctx, GB_REG_PC));
            gb_jit_store_reg16(ctx, GB_REG_PC, tmp);
            ctx->addr += 2;
            break;

        case JUMP_TYPE_RET:
            gb_jit_store_reg16(ctx, GB_REG_PC, pop_val(ctx));
            break;
        }
    }
    jit_insn_branch(ctx->func, &end_label);
    jit_insn_label(ctx->func, &tmp_label);
    if (jump_type != JUMP_TYPE_RET) {
        jit_value_t tmp = gb_jit_load_reg16(ctx, GB_REG_PC);

        switch (jump_type) {
        case JUMP_TYPE_DIRECT:
        case JUMP_TYPE_CALL:
            tmp = jit_insn_add(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 2));
            break;

        case JUMP_TYPE_RELATIVE:
            tmp = jit_insn_add(ctx->func, tmp, GB_JIT_CONST_USHORT(ctx->func, 1));
            break;

        case JUMP_TYPE_RET:
            /* Can't happen */
            break;
        }

        gb_jit_store_reg16(ctx, GB_REG_PC, tmp);
    }
    jit_insn_label(ctx->func, &end_label);
}

/* JP nn
 * JP cc, nn */
static void jp(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jp_all(ctx, opcode, JUMP_TYPE_DIRECT);
}

/* JP (HL) */
static void jp_hl(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    gb_jit_store_reg16(ctx, GB_REG_PC, gb_jit_load_reg16(ctx, GB_REG_HL));
}

/* JP n (8-bits).
 * JP cc, n */
static void jp_rel(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jp_all(ctx, opcode, JUMP_TYPE_RELATIVE);
}

/*
 *
 * calls
 *
 */

/* CALL nn,
 * CALL cc, nn */
static void call(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jp_all(ctx, opcode, JUMP_TYPE_CALL);
}

/*
 *
 * Restarts
 *
 */

/* RST n
 * n = 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38 */
static void rst(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    uint16_t addr = ((opcode & 0x38) >> 3);
    addr *= 0x08;

    push_val(ctx, gb_jit_load_reg16(ctx, GB_REG_PC));
    gb_jit_store_reg16(ctx, GB_REG_PC, GB_JIT_CONST_USHORT(ctx->func, addr));
}

/*
 *
 * Returns
 *
 */

/* RET
 * RET cc */
static void ret(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    jp_all(ctx, opcode, JUMP_TYPE_RET);
}

/* RETI */
static void reti(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    gb_jit_store_reg16(ctx, GB_REG_PC, pop_val(ctx));

    jit_insn_store_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.ime), GB_JIT_CONST_UBYTE(ctx->func, 1));
}

/* 0xCB prefix */
static void z80_emu_run_cb_inst(struct gb_cpu_jit_context *ctx)
{
    uint8_t opcode;

    gb_jit_clock_tick(ctx);
    gb_jit_next_pc8(ctx);
    opcode = gb_emu_read8(ctx->gb_emu, ctx->addr);
    ctx->addr++;

    switch (opcode) {
    case 0x30 ... 0x37:
        swap(ctx, opcode);
        break;

    case 0x00 ... 0x07:
    case 0x10 ... 0x17:
        rla(ctx, opcode, 1);
        break;

    case 0x08 ... 0x0F:
    case 0x18 ... 0x1F:
        rra(ctx, opcode, 1);
        break;

    case 0x20 ... 0x27:
        sla(ctx, opcode);
        break;

    case 0x28 ... 0x2F:
    case 0x38 ... 0x3F:
        sra(ctx, opcode);
        break;

    case 0x40 ... 0x7F:
        bit(ctx, opcode);
        break;

    case 0xC0 ... 0xFF:
        set(ctx, opcode);
        break;

    case 0x80 ... 0xBF:
        res(ctx, opcode);
        break;
    }

    return ;
}

static void check_int_count(struct gb_emu *emu)
{
    if (emu->cpu.int_count > 0) {
        emu->cpu.int_count--;
        if (emu->cpu.int_count == 0)
            emu->cpu.ime = emu->cpu.next_ime;
    }
}

/* Returns true if instruction was a jump */
int gb_emu_jit_run_inst(struct gb_cpu_jit_context *ctx, uint8_t opcode)
{
    int jump = 0;

    switch (opcode) {
    case 0x06:
    case 0x0E:
    case 0x16:
    case 0x1E:
    case 0x26:
    case 0x2E:
    case 0x36:
        load8_reg_imm(ctx, opcode);
        break;

    case 0x40 ... 0x75: /* 0x76 isn't valid */
    case 0x77 ... 0x7F: /* It's used as HALT down below */
        load8_reg_reg(ctx, opcode);
        break;

    case 0x0A:
    case 0x1A:
    case 0xFA:
    case 0x3E:
    case 0xF2:
    case 0x3A:
    case 0x2A:
    case 0xF0:
        load8_a_extra(ctx, opcode);
        break;

    case 0x02:
    case 0x12:
    case 0xEA:
    case 0xE2:
    case 0x32:
    case 0x22:
    case 0xE0:
        load8_extra_a(ctx, opcode);
        break;

    case 0x01:
    case 0x11:
    case 0x21:
    case 0x31:
        load16_reg_imm(ctx, opcode);
        break;

    case 0xF9:
        load_sp_hl(ctx, opcode);
        break;

    case 0xF8:
        load_hl_sp_n(ctx, opcode);
        break;

    case 0x08:
        load_mem_sp(ctx, opcode);
        break;

    case 0xC5:
    case 0xD5:
    case 0xE5:
    case 0xF5:
        push(ctx, opcode);
        break;

    case 0xC1:
    case 0xD1:
    case 0xE1:
    case 0xF1:
        pop(ctx, opcode);
        break;

    case 0x80 ... 0x87:
    case 0xC6:
    case 0x88 ... 0x8F:
    case 0xCE:
        add_a(ctx, opcode);
        break;

    case 0x90 ... 0x97:
    case 0xD6:
    case 0x98 ... 0x9F:
    case 0xDE:
        sub_a(ctx, opcode);
        break;

    case 0xA0 ... 0xA7:
    case 0xE6:
        and_a(ctx, opcode);
        break;

    case 0xB0 ... 0xB7:
    case 0xF6:
        or_a(ctx, opcode);
        break;

    case 0xA8 ... 0xAF:
    case 0xEE:
        xor_a(ctx, opcode);
        break;

    case 0xB8 ... 0xBF:
    case 0xFE:
        cp_a(ctx, opcode);
        break;

    case 0x04:
    case 0x0C:
    case 0x14:
    case 0x1C:
    case 0x24:
    case 0x2C:
    case 0x34:
    case 0x3C:
        inc_reg(ctx, opcode);
        break;

    case 0x05:
    case 0x0D:
    case 0x15:
    case 0x1D:
    case 0x25:
    case 0x2D:
    case 0x35:
    case 0x3D:
        dec_reg(ctx, opcode);
        break;

    case 0x09:
    case 0x19:
    case 0x29:
    case 0x39:
        add_hl_reg16(ctx, opcode);
        break;

    case 0xE8:
        add_sp_imm(ctx, opcode);
        break;

    case 0x03:
    case 0x13:
    case 0x23:
    case 0x33:
        inc_reg16(ctx, opcode);
        break;

    case 0x0B:
    case 0x1B:
    case 0x2B:
    case 0x3B:
        dec_reg16(ctx, opcode);
        break;

    case 0x27:
        daa(ctx, opcode);
        break;

    case 0x2F:
        cpl(ctx, opcode);
        break;

    case 0x3F:
        ccf(ctx, opcode);
        break;

    case 0x37:
        scf(ctx, opcode);
        break;

    case 0x00:
        nop(ctx, opcode);
        break;

    case 0x76:
        halt(ctx, opcode);
        break;

    case 0x10:
        stop(ctx, opcode);
        break;

    case 0xF3:
        di(ctx, opcode);
        break;

    case 0xFB:
        ei(ctx, opcode);
        break;

    case 0x07:
    case 0x17:
        rla(ctx, opcode, 0);
        break;

    case 0x0F:
    case 0x1F:
        rra(ctx, opcode, 0);
        break;

    case 0xC3:
    case 0xC2:
    case 0xCA:
    case 0xD2:
    case 0xDA:
        jp(ctx, opcode);
        jump = 1;
        break;

    case 0xE9:
        jp_hl(ctx, opcode);
        jump = 1;
        break;

    case 0x18:
    case 0x20:
    case 0x28:
    case 0x30:
    case 0x38:
        jp_rel(ctx, opcode);
        jump = 1;
        break;

    case 0xCD:
    case 0xC4:
    case 0xCC:
    case 0xD4:
    case 0xDC:
        call(ctx, opcode);
        jump = 1;
        break;

    case 0xC7:
    case 0xCF:
    case 0xD7:
    case 0xDF:
    case 0xE7:
    case 0xEF:
    case 0xF7:
    case 0xFF:
        rst(ctx, opcode);
        jump = 1;
        break;

    case 0xC9:
    case 0xC0:
    case 0xC8:
    case 0xD0:
    case 0xD8:
        ret(ctx, opcode);
        jump = 1;
        break;

    case 0xD9:
        reti(ctx, opcode);
        jump = 1;
        break;

    case 0xCB:
        z80_emu_run_cb_inst(ctx);
        break;
    }

    /* No matter what the code does, the lower bits of F are always zero. */
    jit_value_t tmp = gb_jit_load_reg8(ctx, GB_REG_F);
    tmp = jit_insn_and(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, 0xF0));
    gb_jit_store_reg8(ctx, GB_REG_F, tmp);

    /* Check if we should enable interrupts */
    jit_type_t params[] = { jit_type_void_ptr };
    jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, params, ARRAY_SIZE(params), 1);

    jit_value_t args[] = { ctx->emu };
    jit_insn_call_native(ctx->func, "check_int_count", check_int_count, signature, args, ARRAY_SIZE(args), JIT_CALL_NOTHROW);

    return jump;
}

/* Returns true when we hit a jump - and the end of a block */
int gb_emu_cpu_jit_run_next_inst(struct gb_cpu_jit_context *ctx)
{
    jit_type_t check_int_params[] = { jit_type_void_ptr };
    jit_type_t check_int_signature = jit_type_create_signature(jit_abi_cdecl, jit_type_sys_int, check_int_params, ARRAY_SIZE(check_int_params), 1);
    jit_value_t check_int_args[] = { ctx->emu };

    jit_type_t dispatch_params[] = { jit_type_void_ptr, jit_type_void_ptr };
    jit_type_t dispatch_signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, dispatch_params, ARRAY_SIZE(dispatch_params), 1);
    jit_value_t dispatch_args[] = { GB_JIT_CONST_PTR(ctx->func, ctx->dispatcher), ctx->emu };
    jit_value_t interrupt_check;

    jit_label_t inst_end = jit_label_undefined;
    jit_label_t not_halted = jit_label_undefined;
    jit_label_t run_again = jit_label_undefined;

    jit_insn_label(ctx->func, &run_again);

    jit_value_t halted = jit_insn_load_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.halted), jit_type_ubyte);

    /* When we're halted, we keep running the instruction again until we're not longer halted */
    jit_insn_branch_if_not(ctx->func, halted, &not_halted);

        /* When we're halted, the clock still ticks */
        gb_jit_clock_tick(ctx);
        interrupt_check = jit_insn_call_native(ctx->func, "gb_emu_check_interrupt", gb_emu_check_interrupt, check_int_signature, check_int_args, ARRAY_SIZE(check_int_args), JIT_CALL_NOTHROW);
        jit_value_t halt_check_int = jit_insn_eq(ctx->func, interrupt_check, GB_JIT_CONST_INT(ctx->func, 0));

        /* 
         * FIXME: We currently return back to the dispatcher here and exit the JIT code. This works, but is not optimal.
         */
        jit_insn_branch_if(ctx->func, halt_check_int, &run_again);
        gb_emu_jit_func_exit(ctx);
        /* jit_insn_call_native(ctx->func, "gb_emu_run_dispatcher", gb_emu_run_dispatcher, dispatch_signature, dispatch_args, ARRAY_SIZE(dispatch_args), JIT_CALL_NOTHROW);
        jit_insn_branch(ctx->func, &run_again); */
    jit_insn_label(ctx->func, &not_halted);

    jit_type_t hdma_check_params[] = { jit_type_void_ptr };
    jit_type_t hdma_check_signature = jit_type_create_signature(jit_abi_cdecl, jit_type_sys_int, hdma_check_params, ARRAY_SIZE(hdma_check_params), 1);

    jit_value_t hdma_check_args[] = { ctx->emu };
    jit_value_t hdma_check_flag = jit_insn_call_native(ctx->func, "gb_emu_hdma_check", gb_emu_hdma_check, hdma_check_signature, hdma_check_args, ARRAY_SIZE(hdma_check_args), JIT_CALL_NOTHROW);

    jit_insn_branch_if(ctx->func, hdma_check_flag, &run_again);

    /* Insert hook_flag check */

    gb_jit_clock_tick(ctx);

    gb_jit_next_pc8(ctx);
    uint8_t opcode = gb_emu_read8(ctx->gb_emu, ctx->addr);
    ctx->addr++;

    int jump = gb_emu_jit_run_inst(ctx, opcode);

    /* Insert hook_flag check here */

    jit_insn_label(ctx->func, &inst_end);

    interrupt_check = jit_insn_call_native(ctx->func, "gb_emu_check_interrupt", gb_emu_check_interrupt, check_int_signature, check_int_args, ARRAY_SIZE(check_int_args), JIT_CALL_NOTHROW);

    /* Insert break_flag hook here */

    jit_label_t dispatch_label = jit_label_undefined;

    jit_insn_branch_if(ctx->func, jit_insn_eq(ctx->func, interrupt_check, GB_JIT_CONST_INT(ctx->func, 0)), &dispatch_label);
    jit_insn_default_return(ctx->func);
    /* jit_insn_call_native(ctx->func, "gb_emu_run_dispatcher", gb_emu_run_dispatcher, dispatch_signature, dispatch_args, ARRAY_SIZE(dispatch_args), JIT_CALL_NOTHROW); */

    jit_insn_label(ctx->func, &dispatch_label);

    return jump;
}

static struct gb_emu *signal_emu;

static void debugger_run_sigint(int signum)
{
    signal_emu->stop_emu = 1;
    signal_emu->reason = GB_EMU_STOP;
}

enum gb_emu_stop gb_run_jit(struct gb_emu *emu)
{
    int i;
    uint64_t cycles = 0;
    struct sigaction new_act, old_act;

    emu->stop_emu = 0;

    if (emu->sound.driver) {
        gb_sound_start(&emu->sound);
        gb_sound_set_sound_rate(&emu->sound, GB_APU_SAMPLE_RATE);

        (emu->sound.driver->play) (emu->sound.driver);
    }

    memset(&new_act, 0, sizeof(new_act));
    new_act.sa_handler = debugger_run_sigint;

    sigaction(SIGINT, &new_act, &old_act);

    signal_emu = emu;

    while (!emu->stop_emu) {
        cycles = 0;

        for (i = 0; i < 20000; i++) {
            cycles += gb_emu_cpu_run_next_inst(emu);
            if (emu->stop_emu)
                break;
        }
    }

    sigaction(SIGINT, &old_act, NULL);

    if (emu->sound.driver) {
        (emu->sound.driver->pause) (emu->sound.driver);

        gb_sound_finish(&emu->sound);
    }

    return emu->reason;
}

