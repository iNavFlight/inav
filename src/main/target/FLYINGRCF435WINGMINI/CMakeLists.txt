# INAV's generic xGT7 helper selects the correct 1 MiB linker script but
# identifies the package as AT32F435RGT7. Select the actual QFN48 part here.
function(target_at32f435xgu7 name)
    target_at32f43x(
        NAME ${name}
        STARTUP startup_at32f435_437.s
        SOURCES ${AT32F4_STDPERIPH_SRC}
        COMPILE_DEFINITIONS AT32F435CGU7 MCU_RAM_SIZE=192 MCU_FLASH_SIZE=1024
        LINKER_SCRIPT at32_flash_f43xG
        SVD at32f43x_xGT7
        ${ARGN}
    )
endfunction()

function(target_at32f435xmu7 name)
    target_at32f43x(
        NAME ${name}
        STARTUP startup_at32f435_437.s
        SOURCES ${AT32F4_STDPERIPH_SRC}
        COMPILE_DEFINITIONS AT32F435CMU7 MCU_RAM_SIZE=192 MCU_FLASH_SIZE=4032
        LINKER_SCRIPT at32_flash_f43xM
        SVD at32f43x_xMT7
        ${ARGN}
    )
endfunction()

target_at32f435xgu7(FLYINGRCF435WINGMINI)
target_at32f435xmu7(FLYINGRCF435WINGMINI_CMU7)
