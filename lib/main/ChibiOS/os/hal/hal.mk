# List of all the ChibiOS/HAL files, there is no need to remove the files
# from this list, you can disable parts of the HAL by editing halconf.h.
ifeq ($(USE_SMART_BUILD),yes)
HALCONF := $(strip $(shell cat halconf.h | egrep -e "define"))

HALSRC := $(CHIBIOS)/os/hal/src/hal.c \
          $(CHIBIOS)/os/hal/src/st.c \
          $(CHIBIOS)/os/hal/src/hal_buffers.c \
          $(CHIBIOS)/os/hal/src/hal_queues.c \
          $(CHIBIOS)/os/hal/src/hal_mmcsd.c
ifneq ($(findstring HAL_USE_ADC TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/adc.c
endif
ifneq ($(findstring HAL_USE_CAN TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/can.c
endif
ifneq ($(findstring HAL_USE_DAC TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/dac.c
endif
ifneq ($(findstring HAL_USE_EXT TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/ext.c
endif
ifneq ($(findstring HAL_USE_GPT TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/gpt.c
endif
ifneq ($(findstring HAL_USE_I2C TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/i2c.c
endif
ifneq ($(findstring HAL_USE_I2S TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/i2s.c
endif
ifneq ($(findstring HAL_USE_ICU TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/icu.c
endif
ifneq ($(findstring HAL_USE_MAC TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/mac.c
endif
ifneq ($(findstring HAL_USE_MMC_SPI TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/mmc_spi.c
endif
ifneq ($(findstring HAL_USE_PAL TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/pal.c
endif
ifneq ($(findstring HAL_USE_PWM TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/pwm.c
endif
ifneq ($(findstring HAL_USE_RTC TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/rtc.c
endif
ifneq ($(findstring HAL_USE_SDC TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/sdc.c
endif
ifneq ($(findstring HAL_USE_SERIAL TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/serial.c
endif
ifneq ($(findstring HAL_USE_SERIAL_USB TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/serial_usb.c
endif
ifneq ($(findstring HAL_USE_SPI TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/spi.c
endif
ifneq ($(findstring HAL_USE_UART TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/uart.c
endif
ifneq ($(findstring HAL_USE_USB TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/usb.c
endif
ifneq ($(findstring HAL_USE_WDG TRUE,$(HALCONF)),)
HALSRC += $(CHIBIOS)/os/hal/src/wdg.c
endif
else
HALSRC = $(CHIBIOS)/os/hal/src/hal.c \
         $(CHIBIOS)/os/hal/src/hal_buffers.c \
         $(CHIBIOS)/os/hal/src/hal_queues.c \
         $(CHIBIOS)/os/hal/src/hal_mmcsd.c \
         $(CHIBIOS)/os/hal/src/adc.c \
         $(CHIBIOS)/os/hal/src/can.c \
         $(CHIBIOS)/os/hal/src/dac.c \
         $(CHIBIOS)/os/hal/src/ext.c \
         $(CHIBIOS)/os/hal/src/gpt.c \
         $(CHIBIOS)/os/hal/src/i2c.c \
         $(CHIBIOS)/os/hal/src/i2s.c \
         $(CHIBIOS)/os/hal/src/icu.c \
         $(CHIBIOS)/os/hal/src/mac.c \
         $(CHIBIOS)/os/hal/src/mmc_spi.c \
         $(CHIBIOS)/os/hal/src/pal.c \
         $(CHIBIOS)/os/hal/src/pwm.c \
         $(CHIBIOS)/os/hal/src/rtc.c \
         $(CHIBIOS)/os/hal/src/sdc.c \
         $(CHIBIOS)/os/hal/src/serial.c \
         $(CHIBIOS)/os/hal/src/serial_usb.c \
         $(CHIBIOS)/os/hal/src/spi.c \
         $(CHIBIOS)/os/hal/src/st.c \
         $(CHIBIOS)/os/hal/src/uart.c \
         $(CHIBIOS)/os/hal/src/usb.c \
         $(CHIBIOS)/os/hal/src/wdg.c
endif

# Required include directories
HALINC = $(CHIBIOS)/os/hal/include
