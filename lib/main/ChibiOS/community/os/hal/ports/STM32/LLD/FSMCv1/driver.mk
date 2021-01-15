ifeq ($(USE_SMART_BUILD),yes)
ifneq ($(findstring HAL_USE_FSMC TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/FSMCv1/hal_fsmc.c \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/FSMCv1/hal_fsmc_sram.c \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/FSMCv1/hal_fsmc_sdram.c
endif
ifneq ($(findstring HAL_USE_NAND TRUE,$(HALCONF)),)
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/FSMCv1/hal_nand_lld.c
endif
else
PLATFORMSRC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/FSMCv1/hal_fsmc.c \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/FSMCv1/hal_fsmc_sram.c \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/FSMCv1/hal_fsmc_sdram.c \
                       ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/FSMCv1/hal_nand_lld.c
endif

PLATFORMINC_CONTRIB += ${CHIBIOS_CONTRIB}/os/hal/ports/STM32/LLD/FSMCv1
