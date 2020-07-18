
cflags-y += -I'$(srctree)'

objs-y += bios.o
objs-y += gb.o
objs-y += rom.o
objs-y += io.o
objs-y += gpu.o
objs-y += debugger.o
objs-y += disasm.o
objs-y += timer.o

objs-y += cgb_colors.o
objs-y += cgb_themes.o

subdir-y += cpu
subdir-y += mmu

# We provide an APU stub in the event we don't want/support the APU
ifeq ($(CONFIG_APU),y)
subdir-y += gb_apu
else
objs-y += gb_apu_null.o
endif

