# List of all the STM32F2xx/STM32F4xx platform files.
ifeq ($(USE_SMART_BUILD),yes)
HALCONF := $(strip $(shell cat halconf.h | egrep -e "define"))

PLATFORMSRC := $(CHIBIOS)/os/hal/ports/common/ARMCMx/nvic.c \
               $(CHIBIOS)/os/hal/ports/STM32/STM32F4xx/hal_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/DMAv2/stm32_dma.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1/st_lld.c
ifneq ($(findstring HAL_USE_ADC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/ADCv2/adc_lld.c
endif
ifneq ($(findstring HAL_USE_CAN TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/CANv1/can_lld.c
endif
ifneq ($(findstring HAL_USE_DAC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/DACv1/dac_lld.c
endif
ifneq ($(findstring HAL_USE_EXT TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/EXTIv1/ext_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/STM32F4xx/ext_lld_isr.c
endif
ifneq ($(findstring HAL_USE_PAL TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/GPIOv2/pal_lld.c
endif
ifneq ($(findstring HAL_USE_I2C TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/I2Cv1/i2c_lld.c
endif
ifneq ($(findstring HAL_USE_MAC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/MACv1/mac_lld.c
endif
ifneq ($(findstring HAL_USE_USB TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/OTGv1/usb_lld.c
endif
ifneq ($(findstring HAL_USE_RTC TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/RTCv2/rtc_lld.c
endif
ifneq ($(findstring HAL_USE_I2S TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/SPIv1/i2s_lld.c
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
ifneq ($(findstring HAL_USE_WDG TRUE,$(HALCONF)),)
PLATFORMSRC += $(CHIBIOS)/os/hal/ports/STM32/LLD/xWDGv1/wdg_lld.c
endif
else
PLATFORMSRC := $(CHIBIOS)/os/hal/ports/common/ARMCMx/nvic.c \
               $(CHIBIOS)/os/hal/ports/STM32/STM32F4xx/hal_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/STM32F4xx/ext_lld_isr.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/ADCv2/adc_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/CANv1/can_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/EXTIv1/ext_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/DACv1/dac_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/DMAv2/stm32_dma.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/GPIOv2/pal_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/I2Cv1/i2c_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/MACv1/mac_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/OTGv1/usb_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/RTCv2/rtc_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/SDIOv1/sdc_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/SPIv1/i2s_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/SPIv1/spi_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1/gpt_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1/icu_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1/pwm_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1/st_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/USARTv1/serial_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/USARTv1/uart_lld.c \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/xWDGv1/wdg_lld.c
endif

# Required include directories
PLATFORMINC := $(CHIBIOS)/os/hal/ports/common/ARMCMx \
               $(CHIBIOS)/os/hal/ports/STM32/STM32F4xx \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/ADCv2 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/CANv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/DACv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/DMAv2 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/EXTIv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/GPIOv2 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/I2Cv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/MACv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/OTGv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/RTCv2 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/SDIOv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/SPIv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/TIMv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/USARTv1 \
               $(CHIBIOS)/os/hal/ports/STM32/LLD/xWDGv1
