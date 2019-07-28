#ifndef GBEMUC_GB_CPU_JIT_HELPERS_H
#define GBEMUC_GB_CPU_JIT_HELPERS_H

#include <stdint.h>
#include <jit/jit.h>

#include "cpu_dispatcher.h"

#define GB_REG8_OFFSET(reg)  (offsetof(struct gb_emu, cpu.r.b[(reg)]))
#define GB_REG16_OFFSET(reg) (offsetof(struct gb_emu, cpu.r.w[(reg)]))

#define GB_JIT_CONST_UBYTE(func, val) (jit_value_create_nint_constant((func), jit_type_ubyte, (val)))
#define GB_JIT_CONST_SBYTE(func, val) (jit_value_create_nint_constant((func), jit_type_sbyte, (val)))

#define GB_JIT_CONST_USHORT(func, val) (jit_value_create_nint_constant((func), jit_type_ushort, (val)))
#define GB_JIT_CONST_SHORT(func, val) (jit_value_create_nint_constant((func), jit_type_short, (val)))

#define GB_JIT_CONST_UINT(func, val) (jit_value_create_nint_constant((func), jit_type_uint, (val)))
#define GB_JIT_CONST_INT(func, val) (jit_value_create_nint_constant((func), jit_type_int, (val)))

#define GB_JIT_CONST_PTR(func, val) (jit_value_create_nint_constant((func), jit_type_void_ptr, (jit_nint)(val)))

void gb_jit_clock_tick(struct gb_cpu_jit_context *ctx);

jit_value_t gb_jit_load_reg8(struct gb_cpu_jit_context *ctx, int reg);
void        gb_jit_store_reg8(struct gb_cpu_jit_context *ctx, int reg, jit_value_t val);

jit_value_t gb_jit_load_reg16(struct gb_cpu_jit_context *ctx, int reg);
void        gb_jit_store_reg16(struct gb_cpu_jit_context *ctx, int reg, jit_value_t val);

jit_value_t gb_jit_read8(struct gb_cpu_jit_context *ctx, jit_value_t addr);
void        gb_jit_write8(struct gb_cpu_jit_context *ctx, jit_value_t addr, jit_value_t val);

jit_value_t gb_jit_read16(struct gb_cpu_jit_context *ctx, jit_value_t addr);
void        gb_jit_write16(struct gb_cpu_jit_context *ctx, jit_value_t addr, jit_value_t val);

void        gb_jit_set_flag(struct gb_cpu_jit_context *ctx, uint8_t flag);

void gb_jit_set_flag_if_zero(jit_function_t func, jit_value_t value, jit_value_t flags, uint8_t flag);
void gb_jit_set_flag_if_nonzero(jit_function_t func, jit_value_t value, jit_value_t flags, uint8_t flag);

void gb_jit_set_flag_if_eq(jit_function_t func, jit_value_t val1, jit_value_t val2, jit_value_t flags, uint8_t flag);
void gb_jit_set_flag_if_ne(jit_function_t func, jit_value_t val1, jit_value_t val2, jit_value_t flags, uint8_t flag);
void gb_jit_set_flag_if_gt(jit_function_t func, jit_value_t val1, jit_value_t val2, jit_value_t flags, uint8_t flag);
void gb_jit_set_flag_if_ge(jit_function_t func, jit_value_t val1, jit_value_t val2, jit_value_t flags, uint8_t flag);
void gb_jit_set_flag_if_lt(jit_function_t func, jit_value_t val1, jit_value_t val2, jit_value_t flags, uint8_t flag);
void gb_jit_set_flag_if_le(jit_function_t func, jit_value_t val1, jit_value_t val2, jit_value_t flags, uint8_t flag);

jit_value_t gb_jit_next_pc8(struct gb_cpu_jit_context *ctx);
jit_value_t gb_jit_next_pc16(struct gb_cpu_jit_context *ctx);

int gb_emu_cpu_jit_run_next_inst(struct gb_cpu_jit_context *ctx);

void gb_jit_printf(struct gb_cpu_jit_context *ctx, const char *str, ...);
void gb_jit_dump_regs(struct gb_cpu_jit_context *ctx);
void gb_jit_disasm_next(struct gb_cpu_jit_context *ctx);

jit_value_t gb_jit_load_hook_flag(struct gb_cpu_jit_context *ctx);
jit_value_t gb_jit_load_break_flag(struct gb_cpu_jit_context *ctx);
jit_value_t gb_jit_load_cpu_hooks(struct gb_cpu_jit_context *ctx);
jit_value_t gb_jit_cpu_hooks_load_next_inst(struct gb_cpu_jit_context *ctx, jit_value_t hooks_ptr);
jit_value_t gb_jit_cpu_hooks_load_end_inst(struct gb_cpu_jit_context *ctx, jit_value_t hooks_ptr);

#endif
