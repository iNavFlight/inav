# List of all the STM32F1xx platform files.
ifeq ($(USE_SMART_BUILD),yes)
HALCONF := $(strip $(shell cat halconf.h | egrep -e "define"))

PLATFORMSRC := $(CHIBIOS)/os/hal/ports/common/ARMCMx/nvic.c \
               $(CHIBIOS)/os/hal/ports/STM32/STM32F1xx/hal_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/DMAv1/stm32_dma.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1/st_lld.c
ifneq ($(findstring HAL_USE_ADC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/STM32F1xx/adc_lld.c
endif
ifneq ($(findstring HAL_USE_CAN TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/CANv1/can_lld.c
endif
ifneq ($(findstring HAL_USE_DAC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/DACv1/dac_lld.c
endif
ifneq ($(findstring HAL_USE_EXT TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/EXTIv1/ext_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/STM32F1xx/ext_lld_isr.c
endif
ifneq ($(findstring HAL_USE_PAL TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/GPIOv1/pal_lld.c
endif
ifneq ($(findstring HAL_USE_I2C TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/I2Cv1/i2c_lld.c
endif
ifneq ($(findstring HAL_USE_RTC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/RTCv1/rtc_lld.c
endif
ifneq ($(findstring HAL_USE_SDC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/SDIOv1/sdc_lld.c
endif
ifneq ($(findstring HAL_USE_SPI TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/SPIv1/spi_lld.c
endif
ifneq ($(findstring HAL_USE_GPT TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1/gpt_lld.c
endif
ifneq ($(findstring HAL_USE_ICU TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1/icu_lld.c
endif
ifneq ($(findstring HAL_USE_PWM TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1/pwm_lld.c
endif
ifneq ($(findstring HAL_USE_SERIAL TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/USARTv1/serial_lld.c
endif
ifneq ($(findstring HAL_USE_UART TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/USARTv1/uart_lld.c
endif
ifneq ($(findstring HAL_USE_USB TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/USBv1/usb_lld.c
endif
ifneq ($(findstring HAL_USE_WDG TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/xWDGv1/wdg_lld.c
endif
else
PLATFORMSRC := $(CHIBIOS)/os/hal/ports/common/ARMCMx/nvic.c \
               $(CHIBIOS)/os/hal/ports/STM32/STM32F1xx/hal_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/STM32F1xx/adc_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/STM32F1xx/ext_lld_isr.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/CANv1/can_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/DACv1/dac_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/DMAv1/stm32_dma.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/EXTIv1/ext_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/GPIOv1/pal_lld.c \
			   $(CHIBIOS)/os/hal/ports/STM32/LLD/I2Cv1/i2c_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/RTCv1/rtc_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/SDIOv1/sdc_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/SPIv1/spi_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1/gpt_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1/icu_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1/pwm_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1/st_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/USARTv1/serial_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/USARTv1/uart_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/USBv1/usb_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/xWDGv1/wdg_lld.c
endif

# Required include directories
PLATFORMINC := $(CHIBIOS)/os/hal/ports/common/ARMCMx \
               $(CHIBIOS)/os/hal/ports/STM32/STM32F1xx \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/CANv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/DACv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/DMAv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/EXTIv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/GPIOv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/I2Cv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/RTCv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/SDIOv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/SPIv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/USARTv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/USBv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/xWDGv1

