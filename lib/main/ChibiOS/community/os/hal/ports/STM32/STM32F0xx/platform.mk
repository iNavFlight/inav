include ${CHIBIOS}/os/hal/ports/STM32/STM32F0xx/platform.mk

PLATFORMSRC += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/CRCv1/hal_crc_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1/hal_timcap_lld.c \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1/hal_qei_lld.c \

PLATFORMINC += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/CRCv1 \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/TIMv1 \
               ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD
