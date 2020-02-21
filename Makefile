###############################################################################
# "THE BEER-WARE LICENSE" (Revision 42):
# <msmith@FreeBSD.ORG> wrote this file. As long as you retain this notice you
# can do whatever you want with this stuff. If we meet some day, and you think
# this stuff is worth it, you can buy me a beer in return
###############################################################################
#
# Makefile for building the iNav firmware.
#
# Invoke this with 'make help' to see the list of supported targets.
#
###############################################################################

# Root directory
ROOT            := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))

# developer preferences, edit these at will, they'll be gitignored
-include $(ROOT)/make/local.mk

# Things that the user might override on the commandline
#

# The target to build, see VALID_TARGETS below
TARGET    ?= REVO

# Compile-time options
OPTIONS   ?=

# Debugger optons, must be empty or GDB
DEBUG     ?=

SEMIHOSTING ?=

# Build suffix
BUILD_SUFFIX ?=

# Serial port/Device for flashing
SERIAL_DEVICE   ?= $(firstword $(wildcard /dev/ttyUSB*) no-port-found)

# Flash size (KB).  Some low-end chips actually have more flash than advertised, use this to override.
FLASH_SIZE ?=

## V                 : Set verbosity level based on the V= parameter
##                     V=0 Low
##                     V=1 High
export AT := @

ifndef V
export V0    :=
export V1    := $(AT)
export STDOUT   :=
else ifeq ($(V), 0)
export V0    := $(AT)
export V1    := $(AT)
export STDOUT:= "> /dev/null"
export MAKE  := $(MAKE) --no-print-directory
else ifeq ($(V), 1)
export V0    :=
export V1    :=
export STDOUT   :=
endif

###############################################################################
# Things that need to be maintained as the source changes
#

FORKNAME      = inav

# Working directories
SRC_DIR         := $(ROOT)/src/main
OBJECT_DIR      := $(ROOT)/obj/main
BIN_DIR         := $(ROOT)/obj
CMSIS_DIR       := $(ROOT)/lib/main/CMSIS
INCLUDE_DIRS    := $(SRC_DIR) \
                   $(ROOT)/src/main/target
LINKER_DIR      := $(ROOT)/src/main/target/link

# import macros common to all supported build systems
include $(ROOT)/make/system-id.mk

# default xtal value for F4 targets
HSE_VALUE       = 8000000
MHZ_VALUE      ?=

# used for turning on features like VCP and SDCARD
FEATURES        =

include $(ROOT)/make/targets.mk

REVISION = $(shell git rev-parse --short HEAD)

FC_VER_MAJOR := $(shell grep " FC_VERSION_MAJOR" src/main/build/version.h | awk '{print $$3}' )
FC_VER_MINOR := $(shell grep " FC_VERSION_MINOR" src/main/build/version.h | awk '{print $$3}' )
FC_VER_PATCH := $(shell grep " FC_VERSION_PATCH" src/main/build/version.h | awk '{print $$3}' )

FC_VER := $(FC_VER_MAJOR).$(FC_VER_MINOR).$(FC_VER_PATCH)
FC_VER_SUFFIX ?=

BUILD_DATE = $(shell date +%Y%m%d)

# Search path for sources
FATFS_DIR       = $(ROOT)/lib/main/FatFS
FATFS_SRC       = $(notdir $(wildcard $(FATFS_DIR)/*.c))

VPATH           := $(SRC_DIR):$(SRC_DIR)/startup
VPATH 			:= $(VPATH):$(ROOT)/make/mcu
VPATH 			:= $(VPATH):$(ROOT)/make

CSOURCES        := $(shell find $(SRC_DIR) -name '*.c')

# start specific includes
include $(ROOT)/make/mcu/STM32.mk
include $(ROOT)/make/mcu/$(TARGET_MCU_GROUP).mk

# Configure default flash sizes for the targets (largest size specified gets hit first) if flash not specified already.
ifeq ($(FLASH_SIZE),)
ifneq ($(TARGET_FLASH),)
FLASH_SIZE := $(TARGET_FLASH)
else
$(error FLASH_SIZE not configured for target $(TARGET))
endif
endif

# Configure devide and target-specific defines and compiler flags
DEVICE_FLAGS    := $(DEVICE_FLAGS) -DFLASH_SIZE=$(FLASH_SIZE)
TARGET_FLAGS    := $(TARGET_FLAGS) -D$(TARGET_MCU) -D$(TARGET_MCU_GROUP) -D$(TARGET)

ifneq ($(HSE_VALUE),)
DEVICE_FLAGS    := $(DEVICE_FLAGS) -DHSE_VALUE=$(HSE_VALUE)
endif

ifneq ($(MHZ_VALUE),)
DEVICE_FLAGS    := $(DEVICE_FLAGS) -DMHZ_VALUE=$(MHZ_VALUE)
endif

ifneq ($(BASE_TARGET), $(TARGET))
TARGET_FLAGS    := $(TARGET_FLAGS) -D$(BASE_TARGET)
endif

TARGET_DIR     = $(ROOT)/src/main/target/$(BASE_TARGET)
TARGET_DIR_SRC = $(notdir $(wildcard $(TARGET_DIR)/*.c))

INCLUDE_DIRS    := $(INCLUDE_DIRS) \
                   $(ROOT)/lib/main/MAVLink

INCLUDE_DIRS    := $(INCLUDE_DIRS) \
                   $(TARGET_DIR)

VPATH           := $(VPATH):$(TARGET_DIR)

.DEFAULT_GOAL   := hex

include $(ROOT)/make/source.mk
include $(ROOT)/make/release.mk

###############################################################################
#
# Toolchain installer
#

TOOLS_DIR := $(ROOT)/tools
DL_DIR    := $(ROOT)/downloads

include $(ROOT)/make/tools.mk

#
# Tool names
#
CROSS_CC    = $(ARM_SDK_PREFIX)gcc
OBJCOPY     = $(ARM_SDK_PREFIX)objcopy
SIZE        = $(ARM_SDK_PREFIX)size

#
# Tool options.
#

ifeq ($(DEBUG),GDB)
OPTIMIZE    = -O0
LTO_FLAGS   = $(OPTIMIZE)
else
OPTIMIZE    = -Os
LTO_FLAGS   = -flto -fuse-linker-plugin $(OPTIMIZE)
endif

ifneq ($(SEMIHOSTING),)
SEMIHOSTING_CFLAGS	= -DSEMIHOSTING
SEMIHOSTING_LDFLAGS	= --specs=rdimon.specs -lc -lrdimon
SYSLIB			:=
else
SEMIHOSTING_CFLAGS	=
SEMIHOSTING_LDFLAGS	=
SYSLIB			:= -lnosys
endif

DEBUG_FLAGS = -ggdb3 -DDEBUG

CFLAGS      += $(ARCH_FLAGS) \
              $(LTO_FLAGS) \
              $(addprefix -D,$(OPTIONS)) \
              $(addprefix -I,$(INCLUDE_DIRS)) \
              $(DEBUG_FLAGS) \
              $(SEMIHOSTING_CFLAGS) \
              -std=gnu99 \
              -Wall -Wextra -Wunsafe-loop-optimizations -Wdouble-promotion \
              -Wstrict-prototypes \
              -Werror=switch \
              -ffunction-sections \
              -fdata-sections \
              -fno-common \
              $(DEVICE_FLAGS) \
              -DUSE_STDPERIPH_DRIVER \
              $(TARGET_FLAGS) \
              -D'__FORKNAME__="$(FORKNAME)"' \
              -D'__TARGET__="$(TARGET)"' \
              -D'__REVISION__="$(REVISION)"' \
              -save-temps=obj \
              -MMD -MP

ASFLAGS     = $(ARCH_FLAGS) \
              -x assembler-with-cpp \
              $(addprefix -I,$(INCLUDE_DIRS)) \
              -D$(TARGET) \
              -MMD -MP

LDFLAGS     = -lm \
              -nostartfiles \
              --specs=nano.specs \
              -lc \
              $(SYSLIB) \
              $(ARCH_FLAGS) \
              $(LTO_FLAGS) \
              $(DEBUG_FLAGS) \
              $(SEMIHOSTING_LDFLAGS) \
              -static \
              -Wl,-gc-sections,-Map,$(TARGET_MAP) \
              -Wl,-L$(LINKER_DIR) \
              -Wl,--cref \
              -Wl,--no-wchar-size-warning \
              -Wl,--print-memory-usage \
              -T$(LD_SCRIPT)

###############################################################################
# No user-serviceable parts below
###############################################################################

CPPCHECK        = cppcheck $(CSOURCES) --enable=all --platform=unix64 \
                  --std=c99 --inline-suppr --quiet --force \
                  $(addprefix -I,$(INCLUDE_DIRS)) \
                  -I/usr/include -I/usr/include/linux

#
# Things we will build
#
TARGET_BIN	:= $(BIN_DIR)/$(FORKNAME)_$(FC_VER)
ifneq ($(FC_VER_SUFFIX),)
    TARGET_BIN	:= $(TARGET_BIN)-$(FC_VER_SUFFIX)
endif
TARGET_BIN	:= $(TARGET_BIN)_$(TARGET)
ifneq ($(BUILD_SUFFIX),)
    TARGET_BIN	:= $(TARGET_BIN)_$(BUILD_SUFFIX)
endif
TARGET_BIN	:= $(TARGET_BIN).bin
TARGET_HEX	= $(TARGET_BIN:.bin=.hex)

TARGET_OBJ_DIR  = $(OBJECT_DIR)/$(TARGET)
TARGET_ELF      = $(OBJECT_DIR)/$(FORKNAME)_$(TARGET).elf
TARGET_OBJS     = $(addsuffix .o,$(addprefix $(TARGET_OBJ_DIR)/,$(basename $(TARGET_SRC))))
TARGET_DEPS     = $(addsuffix .d,$(addprefix $(TARGET_OBJ_DIR)/,$(basename $(TARGET_SRC))))
TARGET_MAP      = $(OBJECT_DIR)/$(FORKNAME)_$(TARGET).map


CLEAN_ARTIFACTS := $(TARGET_BIN)
CLEAN_ARTIFACTS += $(TARGET_HEX)
CLEAN_ARTIFACTS += $(TARGET_ELF)
CLEAN_ARTIFACTS += $(TARGET_OBJS) $(TARGET_MAP)

include $(ROOT)/make/stamp.mk
include $(ROOT)/make/settings.mk
include $(ROOT)/make/svd.mk

# Make sure build date and revision is updated on every incremental build
$(TARGET_OBJ_DIR)/build/version.o : $(TARGET_SRC)

# CFLAGS used for ASM generation. These can't include the LTO related options
# since they prevent proper ASM generation. Since $(LTO_FLAGS) includes the
# optization level, we have to add it back. -g is required to make interleaved
# source/ASM work.
ASM_CFLAGS=-g $(OPTIMZE) $(filter-out $(LTO_FLAGS) -save-temps=obj, $(CFLAGS))

# List of buildable ELF files and their object dependencies.
# It would be nice to compute these lists, but that seems to be just beyond make.

$(TARGET_HEX): $(TARGET_ELF)
	$(V0) $(OBJCOPY) -O ihex --set-start $(FLASH_ORIGIN) $< $@

$(TARGET_BIN): $(TARGET_ELF)
	$(V0) $(OBJCOPY) -O binary $< $@

$(TARGET_ELF): $(TARGET_OBJS)
	$(V1) echo Linking $(TARGET)
	$(V1) $(CROSS_CC) -o $@ $(filter %.o, $^) $(LDFLAGS)
	$(V0) $(SIZE) $(TARGET_ELF)

# Compile
$(TARGET_OBJ_DIR)/%.o: %.c
	$(V1) mkdir -p $(dir $@)
	$(V1) echo %% $(notdir $<) "$(STDOUT)"
	$(V1) $(CROSS_CC) -c -o $@ $(CFLAGS) $<
ifeq ($(GENERATE_ASM), 1)
	$(V1) $(CROSS_CC) -S -fverbose-asm -Wa,-aslh -o $(patsubst %.o,%.txt.S,$@) -g $(ASM_CFLAGS) $<
endif


# Assemble
$(TARGET_OBJ_DIR)/%.o: %.s
	$(V1) mkdir -p $(dir $@)
	$(V1) echo %% $(notdir $<) "$(STDOUT)"
	$(V1) $(CROSS_CC) -c -o $@ $(ASFLAGS) $<

$(TARGET_OBJ_DIR)/%.o: %.S
	$(V1) mkdir -p $(dir $@)
	$(V1) echo %% $(notdir $<) "$(STDOUT)"
	$(V1) $(CROSS_CC) -c -o $@ $(ASFLAGS) $<


# mkdirs
$(DL_DIR):
	mkdir -p $@

$(TOOLS_DIR):
	mkdir -p $@


## all               : Build all valid targets
all: $(VALID_TARGETS)

## targets-group-rest: build targets specified in release-targets list
release: $(RELEASE_TARGETS)

$(VALID_TARGETS):
	$(V0) echo "" && \
	echo "Building $@" && \
	$(MAKE) -j 8 TARGET=$@ && \
	echo "Building $@ succeeded."

## clean             : clean up all temporary / machine-generated files
clean:
	$(V0) echo "Cleaning $(TARGET)"
	$(V0) rm -f $(CLEAN_ARTIFACTS)
	$(V0) rm -rf $(TARGET_OBJ_DIR)
	$(V0) echo "Cleaning $(TARGET) succeeded."

## clean_test        : clean up all temporary / machine-generated files (tests)
clean_test:
	$(V0) cd src/test && $(MAKE) clean

## clean_<TARGET>    : clean up one specific target
$(CLEAN_TARGETS) :
	$(V0) $(MAKE) -j 8 TARGET=$(subst clean_,,$@) clean

## <TARGET>_clean    : clean up one specific target (alias for above)
$(TARGETS_CLEAN) :
	$(V0) $(MAKE) -j 8 TARGET=$(subst _clean,,$@) clean

## clean_all         : clean all valid targets
clean_all:$(CLEAN_TARGETS)

## all_clean         : clean all valid targets (alias for above)
all_clean:$(TARGETS_CLEAN)

flash_$(TARGET): $(TARGET_HEX)
	$(V0) stty -F $(SERIAL_DEVICE) raw speed 115200 -crtscts cs8 -parenb -cstopb -ixon
	$(V0) echo -n 'R' >$(SERIAL_DEVICE)
	$(V0) stm32flash -w $(TARGET_HEX) -v -g 0x0 -b 115200 $(SERIAL_DEVICE)

## flash             : flash firmware (.hex) onto flight controller
flash: flash_$(TARGET)

$(STFLASH_TARGETS) :
	$(V0) $(MAKE) -j 8 TARGET=$(subst st-flash_,,$@) st-flash

## st-flash          : flash firmware (.bin) onto flight controller
st-flash: $(TARGET_BIN)
	$(V0) st-flash --reset write $< $(FLASH_ORIGIN)

elf:	$(TARGET_ELF)
binary: $(TARGET_BIN)
hex:    $(TARGET_HEX)

unbrick_$(TARGET): $(TARGET_HEX)
	$(V0) stty -F $(SERIAL_DEVICE) raw speed 115200 -crtscts cs8 -parenb -cstopb -ixon
	$(V0) stm32flash -w $(TARGET_HEX) -v -g 0x0 -b 115200 $(SERIAL_DEVICE)

## unbrick           : unbrick flight controller
unbrick: unbrick_$(TARGET)

## cppcheck          : run static analysis on C source code
cppcheck: $(CSOURCES)
	$(V0) $(CPPCHECK)

cppcheck-result.xml: $(CSOURCES)
	$(V0) $(CPPCHECK) --xml-version=2 2> cppcheck-result.xml

## help              : print this help message and exit
help: Makefile
	$(V0) @echo ""
	$(V0) @echo "Makefile for the $(FORKNAME) firmware"
	$(V0) @echo ""
	$(V0) @echo "Usage:"
	$(V0) @echo "        make [TARGET=<target>] [OPTIONS=\"<options>\"]"
	$(V0) @echo "Or:"
	$(V0) @echo "        make <target> [OPTIONS=\"<options>\"]"
	$(V0) @echo ""
	$(V0) @echo "Valid TARGET values are: $(VALID_TARGETS)"
	$(V0) @echo ""
	$(V0) @sed -n 's/^## //p' $<

## test              : run the cleanflight test suite
test:
	$(V0) cd src/test && $(MAKE) test

# rebuild everything when makefile changes
# Make the generated files and the build stamp order only prerequisites,
# so they will be generated before TARGET_OBJS but regenerating them
# won't cause all TARGET_OBJS to be rebuilt.
$(TARGET_OBJS) : Makefile | $(GENERATED_FILES) $(STAMP)

# include auto-generated dependencies
-include $(TARGET_DEPS)

# Developer tools
include $(ROOT)/make/openocd.mk
include $(ROOT)/make/gdb.mk
