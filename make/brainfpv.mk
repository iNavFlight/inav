# ChibiOS support
ifneq ($(filter CHIBIOS,$(FEATURES)),)

ifeq ($(TARGET),$(filter $(TARGET),$(F446_TARGETS)))
LD_SCRIPT       = $(LINKER_DIR)/stm32_flash_f446_chibios.ld
STARTUP_SRC     = startup_chibios_stm32f446xx.s
else
$(error Target not supported for ChibiOS)
endif

EXCLUDES    = main.c
TARGET_SRC += main_chibios.c \

CHIBIOS := $(ROOT)/lib/main/ChibiOS

include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32F4xx/platform.mk
include $(CHIBIOS)/os/hal/osal/rt/osal.mk
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/rt/ports/ARMCMx/compilers/GCC/mk/port_v7m.mk

DEVICE_FLAGS += -DUSE_CHIBIOS -DCORTEX_USE_FPU=TRUE -DCORTEX_SIMPLIFIED_PRIORITY=TRUE $(DDEFS)

TARGET_SRC += $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/crt1.c
TARGET_SRC += $(STARTUPSRC)
TARGET_SRC += $(PLATFORMSRC)
TARGET_SRC += $(HALSRC)
TARGET_SRC += $(PORTSRC)
TARGET_SRC += $(KERNSRC)
TARGET_SRC += $(STARTUPASM)
TARGET_SRC += $(PORTASM)
TARGET_SRC += $(OSALASM)

INCLUDE_DIRS += $(CHIBIOS)/os/ext/CMSIS/ST/STM32F4xx/
INCLUDE_DIRS += $(CHIBIOS)/os/common/ports/ARMCMx/devices/STM32F4xx
INCLUDE_DIRS += $(STARTUPINC)
INCLUDE_DIRS += $(KERNINC)
INCLUDE_DIRS += $(PORTINC)
INCLUDE_DIRS += $(OSALINC)
INCLUDE_DIRS += $(HALINC)
INCLUDE_DIRS += $(PLATFORMINC)


ifneq ($(filter BRAINFPV_OSD,$(FEATURES)),)
TARGET_SRC += brainfpv/brainfpv_osd.c \
              brainfpv/video_quadspi.c \
              brainfpv/osd_utils.c \
              brainfpv/fonts.c \
              brainfpv/images.c \
              brainfpv/video_quadspi.c \
              brainfpv/ir_transponder.c \
              io/displayport_max7456.c \
              cms/cms_menu_brainfpv.c \

TARGET_SRC += $(STDPERIPH_DIR)/src/stm32f4xx_qspi.c

DEVICE_FLAGS += -DUSE_BRAINFPV_OSD
endif

endif
# End ChibiOS 
