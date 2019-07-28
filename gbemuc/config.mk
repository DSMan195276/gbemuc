# Program wide settings
EXE       := gbemuc
EXEC      := GBEMUC
GBEMUC_VERSION   := 0
GBEMUC_SUBLEVEL  := 1
GBEMUC_PATCH     := 0
GBEMUC_VERSION_N := $(GBEMUC_VERSION).$(GBEMUC_SUBLEVEL).$(GBEMUC_PATCH)

GBEMUC_LIBFLAGS := -pthread `pkg-config --libs sdl2`
GBEMUC_CFLAGS  += -I'./include'                           \
				  -Wall -Wextra -Wno-unused-parameter     \
                   -DGBEMUC_VERSION=$(GBEMUC_VERSION)     \
      			 -DGBEMUC_SUBLEVEL=$(GBEMUC_SUBLEVEL)     \
      			 -DGBEMUC_PATCH=$(GBEMUC_PATCH)           \
      			 -DGBEMUC_VERSION_N="$(GBEMUC_VERSION_N)" \
				 `pkg-config --cflags sdl2`

CXXFLAGS += `pkg-config --cflags sdl2`

ifeq ($(CONFIG_JIT),y)
	GBEMUC_LIBFLAGS += -ljit
	GBEMUC_CFLAGS += -DCONFIG_JIT
endif

GBEMUC_OBJS += ./gbemuc.o

