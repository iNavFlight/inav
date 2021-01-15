include(stm32f4)

set(STM32F446_BRAINFPV_COMPILE_DEFINITIONS
    STM32F446xx
    FLASH_SIZE=512
    USE_CHIBIOS
    CORTEX_USE_FPU=TRUE
    CORTEX_SIMPLIFIED_PRIORITY=TRUE
)

function(target_brainfpv_stm32f446 name)
    target_stm32f4xx(
        NAME ${name}
        STARTUP startup_chibios_stm32f446xx.s
        SOURCES ${STM32F411_OR_F427_STDPERIPH_SRC}
        COMPILE_DEFINITIONS ${STM32F446_BRAINFPV_COMPILE_DEFINITIONS}
        LINKER_SCRIPT stm32_flash_f446_chibios
        SVD STM32F446
        ${ARGN}
    )
endfunction()
