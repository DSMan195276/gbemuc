# Program wide settings
EXE       := gbemuc
EXEC      := GBEMUC
GBEMUC_VERSION   := 0
GBEMUC_SUBLEVEL  := 1
GBEMUC_PATCH     := 0
GBEMUC_VERSION_N := $(GBEMUC_VERSION).$(GBEMUC_SUBLEVEL).$(GBEMUC_PATCH)

GBEMUC_LIBFLAGS := -pthread
GBEMUC_CFLAGS  += -I'./include'                           \
				  -Wall -Wextra -Wno-unused-parameter     \
                   -DGBEMUC_VERSION=$(GBEMUC_VERSION)     \
      			 -DGBEMUC_SUBLEVEL=$(GBEMUC_SUBLEVEL)     \
      			 -DGBEMUC_PATCH=$(GBEMUC_PATCH)           \
      			 -DGBEMUC_VERSION_N="$(GBEMUC_VERSION_N)"

ifeq ($(CONFIG_EMSCRIPTEN),y)

GBEMUC_CFLAGS := -s USE_SDL=2 \
				-s TOTAL_MEMORY=67108864

GBEMUC_LIBFLAGS += --preload-file ./preload

GBEMUC_LIBFLAGS += -s EXPORTED_FUNCTIONS='["_main","_gb_write_save"]' \
				-s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]'

CFLAGS += -s EXPORTED_FUNCTIONS='["_main","_gb_write_save"]' \
				-s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]'
# -s SAFE_HEAP=1
# -s ASSERTIONS=2
# -s ALLOW_MEMORY_GROWTH=1
else
GBEMUC_LIBFLAGS += `pkg-config --libs sdl2`
GBEMUC_CFLAGS += `pkg-config --cflags sdl2`
GBEMUC_CXXFLAGS += `pkg-config --cflags sdl2`
endif

ifeq ($(CONFIG_JIT),y)
	GBEMUC_LIBFLAGS += -ljit
	GBEMUC_CFLAGS += -DCONFIG_JIT
endif

GBEMUC_OBJS += ./gbemuc.o

