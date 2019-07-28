
#include "common.h"

#include <stdint.h>
#include <time.h>
#include <signal.h>
#include <stdarg.h>

#include <jit/jit.h>

#include "debug.h"
#include "gb/disasm.h"
#include "gb_internal.h"
#include "gb/cpu.h"
#include "cpu_jit_helpers.h"

void gb_jit_clock_tick(struct gb_cpu_jit_context *ctx)
{
    jit_type_t params[] = { jit_type_void_ptr };
    jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, params, 1, 1);

    jit_value_t args[] = { ctx->emu };
    jit_insn_call_native(ctx->func, "gb_emu_clock_tick", gb_emu_clock_tick, signature, args, 1, JIT_CALL_NOTHROW);
}

jit_value_t gb_jit_read8(struct gb_cpu_jit_context *ctx, jit_value_t addr)
{
    jit_type_t params[] = { jit_type_void_ptr, jit_type_ushort };
    jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_ubyte, params, ARRAY_SIZE(params), 1);

    jit_value_t args[] = { ctx->emu, addr };
    return jit_insn_call_native(ctx->func, "gb_emu_read8", gb_emu_read8, signature, args, ARRAY_SIZE(args), JIT_CALL_NOTHROW);
}

void gb_jit_write8(struct gb_cpu_jit_context *ctx, jit_value_t addr, jit_value_t val)
{
    jit_type_t params[] = { jit_type_void_ptr, jit_type_ushort, jit_type_ubyte };
    jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, params, ARRAY_SIZE(params), 1);

    jit_value_t args[] = { ctx->emu, addr, jit_insn_convert(ctx->func, val, jit_type_ubyte, 0) };
    jit_insn_call_native(ctx->func, "gb_emu_write8", gb_emu_write8, signature, args, ARRAY_SIZE(args), JIT_CALL_NOTHROW);
}

jit_value_t gb_jit_read16(struct gb_cpu_jit_context *ctx, jit_value_t addr)
{
    jit_type_t params[] = { jit_type_void_ptr, jit_type_ushort };
    jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_ushort, params, ARRAY_SIZE(params), 1);

    jit_value_t args[] = { ctx->emu, addr };
    return jit_insn_call_native(ctx->func, "gb_emu_read16", gb_emu_read16, signature, args, ARRAY_SIZE(args), JIT_CALL_NOTHROW);
}

void gb_jit_write16(struct gb_cpu_jit_context *ctx, jit_value_t addr, jit_value_t val)
{
    jit_type_t params[] = { jit_type_void_ptr, jit_type_ushort, jit_type_ushort };
    jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, params, ARRAY_SIZE(params), 1);

    jit_value_t args[] = { ctx->emu, addr, jit_insn_convert(ctx->func, val, jit_type_ushort, 0) };
    jit_insn_call_native(ctx->func, "gb_emu_write16", gb_emu_write16, signature, args, ARRAY_SIZE(args), JIT_CALL_NOTHROW);
}

void gb_jit_set_flag(struct gb_cpu_jit_context *ctx, uint8_t flag)
{
    jit_value_t tmp = jit_insn_load_relative(ctx->func, ctx->emu, GB_REG8_OFFSET(GB_REG_F), jit_type_ubyte);
    jit_insn_store_relative(ctx->func, ctx->emu, GB_REG8_OFFSET(GB_REG_F), jit_insn_or(ctx->func, tmp, GB_JIT_CONST_UBYTE(ctx->func, flag)));
}

jit_value_t gb_jit_load_reg8(struct gb_cpu_jit_context *ctx, int reg)
{
    //jit_value_t val = jit_insn_load_relative(ctx->func, ctx->emu, GB_REG8_OFFSET(reg), jit_type_ubyte);
    //return val;
    return ctx->regs[reg];
}

void gb_jit_store_reg8(struct gb_cpu_jit_context *ctx, int reg, jit_value_t val)
{
    //jit_insn_store_relative(ctx->func, ctx->emu, GB_REG8_OFFSET(reg), jit_insn_convert(ctx->func, val, jit_type_ubyte, 0));
    jit_insn_store(ctx->func, ctx->regs[reg], val);
}

jit_value_t gb_jit_load_reg16(struct gb_cpu_jit_context *ctx, int reg)
{
    //return jit_insn_load_relative(ctx->func, ctx->emu, GB_REG16_OFFSET(reg), jit_type_ushort);
    int reg8 = reg * 2;
    jit_value_t ret;

    ret = jit_insn_shl(ctx->func, jit_insn_convert(ctx->func, gb_jit_load_reg8(ctx, reg8 + 1), jit_type_ushort, 0), GB_JIT_CONST_USHORT(ctx->func, 8));
    ret = jit_insn_or(ctx->func, ret, gb_jit_load_reg8(ctx, reg8));
    return ret;
}

void gb_jit_store_reg16(struct gb_cpu_jit_context *ctx, int reg, jit_value_t val)
{
    //jit_insn_store_relative(ctx->func, ctx->emu, GB_REG16_OFFSET(reg), jit_insn_convert(ctx->func, val, jit_type_ushort, 0));
    int reg8 = reg * 2;
    gb_jit_store_reg8(ctx, reg8 + 1, jit_insn_shr(ctx->func, val, GB_JIT_CONST_USHORT(ctx->func, 8)));
    gb_jit_store_reg8(ctx, reg8, jit_insn_and(ctx->func, val, GB_JIT_CONST_USHORT(ctx->func, 0x00FF)));
}

//jit_value_t gb_jit_load_hook_flag(struct gb_cpu_jit_context *ctx)
//{
//    return jit_insn_load_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.hook_flag), jit_type_uint);
//}

//jit_value_t gb_jit_load_break_flag(struct gb_cpu_jit_context *ctx)
//{
//    return jit_insn_load_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.break_flag), jit_type_uint);
//}
//
//jit_value_t gb_jit_load_cpu_hooks(struct gb_cpu_jit_context *ctx)
//{
//    return jit_insn_load_relative(ctx->func, ctx->emu, offsetof(struct gb_emu, cpu.hooks), jit_type_void_ptr);
//}
//
//jit_value_t gb_jit_cpu_hooks_load_next_inst(struct gb_cpu_jit_context *ctx, jit_value_t hooks_ptr)
//{
//    return jit_insn_load_relative(ctx->func, hooks_ptr, offsetof(struct gb_cpu_hooks, next_inst), jit_type_void_ptr);
//}
//
//jit_value_t gb_jit_cpu_hooks_load_end_inst(struct gb_cpu_jit_context *ctx, jit_value_t hooks_ptr)
//{
//    return jit_insn_load_relative(ctx->func, hooks_ptr, offsetof(struct gb_cpu_hooks, end_inst), jit_type_void_ptr);
//}

static void gb_jit_set_if_true(jit_function_t func, jit_value_t result, jit_value_t flags, uint8_t flag)
{
    jit_label_t tmp_label;

    tmp_label = jit_label_undefined;
    jit_insn_branch_if_not(func, result, &tmp_label);
    jit_insn_store(func, flags, jit_insn_or(func, flags, GB_JIT_CONST_UBYTE(func, flag)));
    jit_insn_label(func, &tmp_label);
}

void gb_jit_set_flag_if_nonzero(jit_function_t func, jit_value_t val, jit_value_t flags, uint8_t flag)
{
    return gb_jit_set_if_true(func, jit_insn_ne(func, val, GB_JIT_CONST_UBYTE(func, 0)), flags, flag);
}

void gb_jit_set_flag_if_zero(jit_function_t func, jit_value_t val, jit_value_t flags, uint8_t flag)
{
    return gb_jit_set_if_true(func, jit_insn_eq(func, val, GB_JIT_CONST_UBYTE(func, 0)), flags, flag);
}

void gb_jit_set_flag_if_eq(jit_function_t func, jit_value_t val1, jit_value_t val2, jit_value_t flags, uint8_t flag)
{
    return gb_jit_set_if_true(func, jit_insn_eq(func, val1, val2), flags, flag);
}

void gb_jit_set_flag_if_ne(jit_function_t func, jit_value_t val1, jit_value_t val2, jit_value_t flags, uint8_t flag)
{
    return gb_jit_set_if_true(func, jit_insn_ne(func, val1, val2), flags, flag);
}

void gb_jit_set_flag_if_gt(jit_function_t func, jit_value_t val1, jit_value_t val2, jit_value_t flags, uint8_t flag)
{
    return gb_jit_set_if_true(func, jit_insn_gt(func, val1, val2), flags, flag);
}

void gb_jit_set_flag_if_et(jit_function_t func, jit_value_t val1, jit_value_t val2, jit_value_t flags, uint8_t flag)
{
    return gb_jit_set_if_true(func, jit_insn_ge(func, val1, val2), flags, flag);
}

void gb_jit_set_flag_if_lt(jit_function_t func, jit_value_t val1, jit_value_t val2, jit_value_t flags, uint8_t flag)
{
    return gb_jit_set_if_true(func, jit_insn_lt(func, val1, val2), flags, flag);
}

void gb_jit_set_flag_if_le(jit_function_t func, jit_value_t val1, jit_value_t val2, jit_value_t flags, uint8_t flag)
{
    return gb_jit_set_if_true(func, jit_insn_le(func, val1, val2), flags, flag);
}

void gb_jit_printf(struct gb_cpu_jit_context *ctx, const char *str, ...)
{
    int param_count = 0;
    const char *s;
    int i;

    for (s = str; *s; s++)
        if (*s == '%' && *(s + 1) != '%')
            param_count++;

    jit_type_t params[param_count + 1];
    jit_value_t args[param_count + 1];
    va_list va_args;

    va_start(va_args, str);

    params[0] = jit_type_void_ptr;
    args[0] = GB_JIT_CONST_PTR(ctx->func, str);

    for (i = 0; i < param_count; i++) {
        args[i + 1] = va_arg(va_args, jit_value_t);
        params[i + 1] = jit_value_get_type(args[i]);
    }

    jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_sys_int, params, ARRAY_SIZE(params), 1);

    jit_insn_call_native(ctx->func, "printf", printf, signature, args, ARRAY_SIZE(args), JIT_CALL_NOTHROW);
}

static char buffer[4096];

void gb_jit_dump_regs(struct gb_cpu_jit_context *ctx)
{
    jit_type_t params[] = { jit_type_void_ptr, jit_type_void_ptr };
    jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, params, ARRAY_SIZE(params), 1);

    jit_value_t args[] = { ctx->emu, GB_JIT_CONST_PTR(ctx->func, buffer) };

    jit_insn_call_native(ctx->func, "gb_emu_dump_regs", gb_emu_dump_regs, signature, args, ARRAY_SIZE(args), JIT_CALL_NOTHROW);

    gb_jit_printf(ctx, "%s", GB_JIT_CONST_PTR(ctx->func, buffer));
}

static void disasm_next(struct gb_emu *emu)
{
    uint8_t bytes[3];
    char buf[30] = { 0 };

    bytes[0] = gb_emu_read8(emu, emu->cpu.r.w[GB_REG_PC]);
    bytes[1] = gb_emu_read8(emu, emu->cpu.r.w[GB_REG_PC] + 1);
    bytes[2] = gb_emu_read8(emu, emu->cpu.r.w[GB_REG_PC] + 2);

    gb_disasm_inst(buf, bytes);

    if (bytes[0] != 0xCB)
        printf("%s\n", buf);
    else
        printf("%s\n", buf);
}

void gb_jit_disasm_next(struct gb_cpu_jit_context *ctx)
{
    jit_type_t params[] = { jit_type_void_ptr };
    jit_type_t signature = jit_type_create_signature(jit_abi_cdecl, jit_type_void, params, 1, 1);

    jit_value_t args[] = { ctx->emu };
    jit_insn_call_native(ctx->func, "disasm_next", disasm_next, signature, args, 1, JIT_CALL_NOTHROW);
}

jit_value_t gb_jit_next_pc8(struct gb_cpu_jit_context *ctx)
{
    jit_value_t addr = gb_jit_load_reg16(ctx, GB_REG_PC);

    jit_value_t read_result = gb_jit_read8(ctx, addr);

    jit_value_t addr_inc = jit_insn_add(ctx->func, addr, GB_JIT_CONST_UBYTE(ctx->func, 1));
    gb_jit_store_reg16(ctx, GB_REG_PC, addr_inc);

    return read_result;
}

jit_value_t gb_jit_next_pc16(struct gb_cpu_jit_context *ctx)
{
    jit_value_t addr = gb_jit_load_reg16(ctx, GB_REG_PC);

    jit_value_t read_result = gb_jit_read16(ctx, addr);

    jit_value_t addr_inc = jit_insn_add(ctx->func, addr, GB_JIT_CONST_USHORT(ctx->func, 2));
    gb_jit_store_reg16(ctx, GB_REG_PC, addr_inc);

    return read_result;
}

