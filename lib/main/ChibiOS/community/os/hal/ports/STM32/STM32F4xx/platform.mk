include ${CHIBIOS}/os/hal/ports/STM32/STM32F4xx/platform.mk

PLATFORMSRC += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/CRCv1/hal_crc_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/DMA2Dv1/hal_stm32_dma2d.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/FSMCv1/hal_fsmc.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/FSMCv1/hal_nand_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/FSMCv1/hal_fsmc_sram.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/LTDCv1/hal_stm32_ltdc.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1/hal_eicu_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1/hal_timcap_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1/hal_qei_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/USBHv1/hal_usbh_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/src/hal_fsmc_sdram.c

PLATFORMINC += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/CRCv1 \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/DMA2Dv1 \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/FSMCv1 \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/LTDCv1 \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1 \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/USBHv1 \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD
