include(cortex-m7)
include(stm32f7-usb)

set(STM32F7_HAL_DIR "${MAIN_LIB_DIR}/main/STM32F7/Drivers/STM32F7xx_HAL_Driver")

set(STM32F7_HAL_SRC
    stm32f7xx_hal.c
    stm32f7xx_hal_adc.c
    stm32f7xx_hal_adc_ex.c
    stm32f7xx_hal_cortex.c
    stm32f7xx_hal_dac.c
    stm32f7xx_hal_dac_ex.c
    stm32f7xx_hal_dma.c
    stm32f7xx_hal_dma_ex.c
    stm32f7xx_hal_flash.c
    stm32f7xx_hal_flash_ex.c
    stm32f7xx_hal_gpio.c
    stm32f7xx_hal_i2c.c
    stm32f7xx_hal_i2c_ex.c
    stm32f7xx_hal_pcd.c
    stm32f7xx_hal_pcd_ex.c
    stm32f7xx_hal_pwr.c
    stm32f7xx_hal_pwr_ex.c
    stm32f7xx_hal_rcc.c
    stm32f7xx_hal_rcc_ex.c
    stm32f7xx_hal_rtc.c
    stm32f7xx_hal_rtc_ex.c
    stm32f7xx_hal_spi.c
    stm32f7xx_hal_tim.c
    stm32f7xx_hal_tim_ex.c
    stm32f7xx_hal_uart.c
    stm32f7xx_hal_usart.c
    stm32f7xx_ll_dma.c
    stm32f7xx_ll_dma2d.c
    stm32f7xx_ll_gpio.c
    stm32f7xx_ll_rcc.c
    stm32f7xx_ll_spi.c
    stm32f7xx_ll_tim.c
    stm32f7xx_ll_usb.c
    stm32f7xx_ll_utils.c
)
list(TRANSFORM STM32F7_HAL_SRC PREPEND "${STM32F7_HAL_DIR}/src/")

set(STM32F7_CMSIS_DEVICE_DIR "${MAIN_LIB_DIR}/main/STM32F7/Drivers/CMSIS/Device/ST/STM32F7xx")

set(STM32F7_VCP_DIR "${MAIN_SRC_DIR}/vcp_hal")

set(STM32F7_VCP_SRC
    usbd_desc.c
    usbd_conf.c
    usbd_cdc_interface.c
)
list(TRANSFORM STM32F7_VCP_SRC PREPEND "${STM32F7_VCP_DIR}/")

set(STM32F7_INCLUDE_DIRS
    ${STM32F7_HAL_DIR}/inc
    ${STM32F7_CMSIS_DEVICE_DIR}/Include
)

main_sources(STM32F7_SRC
    target/system_stm32f7xx.c
    drivers/adc_stm32f7xx.c
    drivers/bus_i2c_hal.c
    drivers/dma_stm32f7xx.c
    drivers/bus_spi_hal.c
    drivers/timer.c
    drivers/timer_impl_hal.c
    drivers/timer_stm32f7xx.c
    drivers/system_stm32f7xx.c
    drivers/serial_uart_stm32f7xx.c
    drivers/serial_uart_hal.c
    drivers/sdcard/sdmmc_sdio_f7xx.c
)

main_sources(STM32F7_MSC_SRC
    drivers/usb_msc_f7xx.c
)

set(STM32F7_DEFINITIONS
    ${CORTEX_M7_DEFINITIONS}
    USE_HAL_DRIVER
    USE_FULL_LL_DRIVER
)

function(target_stm32f7xx name startup ldscript)
    target_stm32(${name} ${startup} ${ldscript} OPENOCD_TARGET stm32f7x ${ARGN})
    if (IS_RELEASE_BUILD)
        target_compile_options(${name} PRIVATE "-O2")
        target_link_options(${name} PRIVATE "-O2")
    endif()
    target_sources(${name} PRIVATE ${STM32F7_HAL_SRC} ${STM32F7_SRC})
    target_compile_options(${name} PRIVATE ${CORTEX_M7_COMMON_OPTIONS} ${CORTEX_M7_COMPILE_OPTIONS})
    target_include_directories(${name} PRIVATE ${STM32F7_INCLUDE_DIRS})
    target_compile_definitions(${name} PRIVATE ${STM32F7_DEFINITIONS})
    target_link_options(${name} PRIVATE ${CORTEX_M7_COMMON_OPTIONS} ${CORTEX_M7_LINK_OPTIONS})

    get_property(features TARGET ${name} PROPERTY FEATURES)
    if(VCP IN_LIST features)
        target_include_directories(${name} PRIVATE ${STM32F7_USB_INCLUDE_DIRS} ${STM32F7_VCP_DIR})
        target_sources(${name} PRIVATE ${STM32F7_USB_SRC} ${STM32F7_VCP_SRC})
    endif()
    if(MSC IN_LIST features)
        target_sources(${name} PRIVATE ${STM32F7_USBMSC_SRC} ${STM32F7_MSC_SRC})
    endif()
endfunction()

macro(define_target_stm32f7 suffix flash_size)
    function(target_stm32f7${suffix} name)
        target_stm32f7xx(${name} startup_stm32f7${suffix}xx.s stm32_flash_f7${suffix}.ld ${ARGN})
        set(definitions
            STM32F7
            STM32F7${suffix}xx
            FLASH_SIZE=${flash_size}
        )
        target_compile_definitions(${name} PRIVATE ${definitions})
        setup_firmware_target(${name})
    endfunction()
endmacro()

define_target_stm32f7("22" 512)
define_target_stm32f7("45" 2048)
define_target_stm32f7("46" 2048)
define_target_stm32f7("65" 2048)