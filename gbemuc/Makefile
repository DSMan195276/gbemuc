
objs-y += main.o
objs-y += cmd_parser.o
objs-y += dump_mem.o
objs-y += char_buf.o
objs-y += object_pool.o
objs-y += arg_parser.o

objs-$(CONFIG_DEBUG) += debug.o

subdir-y += gb
ifneq ($(filter $(CONFIG_BACKEND),SDL EMSCRIPTEN),)
subdir-y += sdl
endif

ifeq ($(CONFIG_BACKEND),PROTURA)
subdir-y += protura
endif

