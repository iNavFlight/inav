include(cortex-m4f)
include(stm32-stdperiph)
include(stm32f3-usb)

set(STM32F3_STDPERIPH_DIR "${MAIN_LIB_DIR}/main/STM32F3/Drivers/STM32F30x_StdPeriph_Driver")
set(STM32F3_CMSIS_DEVICE_DIR "${MAIN_LIB_DIR}/main/STM32F3/Drivers/CMSIS/Device/ST/STM32F30x")
set(STM32F3_CMSIS_DRIVERS_DIR "${MAIN_LIB_DIR}/main/STM32F3/Drivers/CMSIS")
set(STM32F3_VCP_DIR "${MAIN_SRC_DIR}/vcp")

set(STM32F3_STDPERIPH_SRC_EXCLUDES
    stm32f30x_crc.c
    stm32f30x_can.c
)
set(STM32F3_STDPERIPH_SRC_DIR "${STM32F3_STDPERIPH_DIR}/src")
glob_except(STM32F3_STDPERIPH_SRC "${STM32F3_STDPERIPH_SRC_DIR}/*.c" "${STM32F3_STDPERIPH_SRC_EXCLUDES}")


main_sources(STM32F3_SRC
    target/system_stm32f30x.c
    drivers/adc_stm32f30x.c
    drivers/bus_i2c_stm32f30x.c
    drivers/dma_stm32f3xx.c
    drivers/serial_uart_stm32f30x.c
    drivers/system_stm32f30x.c
    drivers/timer_impl_stdperiph.c
    drivers/timer_stm32f30x.c
)

set(STM32F3_VCP_SRC
    hw_config.c
    stm32_it.c
    usb_desc.c
    usb_endp.c
    usb_istr.c
    usb_prop.c
    usb_pwr.c
)
list(TRANSFORM STM32F3_VCP_SRC PREPEND "${STM32F3_VCP_DIR}/")

set(STM32F3_INCLUDE_DIRS
    "${CMSIS_INCLUDE_DIR}"
    "${CMSIS_DSP_INCLUDE_DIR}"
    "${STM32F3_STDPERIPH_DIR}/inc"
    "${STM32F3_CMSIS_DEVICE_DIR}"
    "${STM32F3_CMSIS_DRIVERS_DIR}"
    "${STM32F3_VCP_DIR}"
)

set(STM32F3_DEFINITIONS
    ${CORTEX_M4F_DEFINITIONS}
    STM32F3
    USE_STDPERIPH_DRIVER
)

set(STM32F303CC_DEFINITIONS
    STM32F303
    STM32F303xC
    FLASH_SIZE=256
)

function(target_stm32f3xx)
    # F3 targets don't support MSC and use -Os instead of -O2 to save size
    target_stm32(
        SOURCES ${STM32_STDPERIPH_SRC} ${STM32F3_STDPERIPH_SRC} ${STM32F3_SRC}
        COMPILE_DEFINITIONS ${STM32F3_DEFINITIONS}
        COMPILE_OPTIONS ${CORTEX_M4F_COMMON_OPTIONS} ${CORTEX_M4F_COMPILE_OPTIONS}
        INCLUDE_DIRECTORIES ${STM32F3_INCLUDE_DIRS}
        LINK_OPTIONS ${CORTEX_M4F_COMMON_OPTIONS} ${CORTEX_M4F_LINK_OPTIONS}

        VCP_SOURCES ${STM32F3_USB_SRC} ${STM32F3_VCP_SRC}
        VCP_INCLUDE_DIRECTORIES ${STM32F3_USB_INCLUDE_DIRS}

        DISABLE_MSC

        OPTIMIZATION -Os

        OPENOCD_TARGET stm32f3x

        ${ARGN}
    )
endfunction()

function(target_stm32f303xc name)
    target_stm32f3xx(
        NAME ${name}
        STARTUP startup_stm32f30x_md_gcc.S
        COMPILE_DEFINITIONS ${STM32F303CC_DEFINITIONS}
        LINKER_SCRIPT stm32_flash_f303xc
        SVD STM32F303
        ${ARGN}
    )
endfunction()
