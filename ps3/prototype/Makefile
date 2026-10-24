# Native PS3 homebrew prototype. Requires the PS3DEV/PSL1GHT toolchain.
.SUFFIXES:

ifeq ($(strip $(PSL1GHT)),)
$(error Set PSL1GHT to your PSL1GHT checkout; see README-PS3.md)
endif

include $(PSL1GHT)/ppu_rules

TARGET      := gdps3proto
BUILD       := build
SOURCES     := source
INCLUDES    := source
TITLE       := Geometry Dash PS3 Prototype
APPID       := GDPS30001
CONTENTID   := UP0001-$(APPID)_00-0000000000000000
CFLAGS      = -O2 -Wall -Wextra -std=gnu11 -mcpu=cell $(MACHDEP) $(INCLUDE)
LDFLAGS     = $(MACHDEP) -Wl,-Map,$(notdir $@).map
LIBS        := -lrsx -lgcm_sys -lio -lsysutil -lrt -llv2 -lm
LIBDIRS     :=
ASSET_BUNDLE := data/ps3_assets.bin
ASSET_INPUTS := ../../gfx/icons/player_01_001.png \
                ../../gfx/icons/player_01_2_001.png \
                ../../gfx/block001_01_001.png ../../gfx/spike_01_001.png \
                ../../gfx/back_and_grounds/bg.png \
                ../../gfx/back_and_grounds/ground.png

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT  := $(CURDIR)/$(TARGET)
export VPATH   := $(CURDIR)/$(SOURCES)
export DEPSDIR := $(CURDIR)/$(BUILD)
export BUILDDIR:= $(CURDIR)/$(BUILD)

CFILES := $(notdir $(wildcard $(SOURCES)/*.c))
SFILES := $(notdir $(wildcard $(SOURCES)/*.S))
OFILES := $(CFILES:.c=.o) $(SFILES:.S=.o)

export LD := $(CC)
export OFILES
export INCLUDE := -I$(CURDIR)/$(INCLUDES) $(LIBPSL1GHT_INC) -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(LIBPSL1GHT_LIB)

.PHONY: all clean $(BUILD)

$(ASSET_BUNDLE): $(ASSET_INPUTS) tools/pack_assets.py
	@mkdir -p data
	python3 tools/pack_assets.py $@

$(BUILD): $(ASSET_BUNDLE)
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

all: $(ASSET_BUNDLE) $(BUILD)

clean:
	@rm -rf $(BUILD) $(OUTPUT).elf $(OUTPUT).self $(OUTPUT).fake.self
else
DEPENDS := $(OFILES:.o=.d)
$(OUTPUT).self: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
ps3_assets.o: ../data/ps3_assets.bin
-include $(DEPENDS)
endif
