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
list(TRANSFORM STM32F7_HAL_SRC PREPEND "${STM32F7_HAL_DIR}/Src/")

set(STM32F7_CMSIS_DEVICE_DIR "${MAIN_LIB_DIR}/main/STM32F7/Drivers/CMSIS/Device/ST/STM32F7xx")

set(STM32F7_VCP_DIR "${MAIN_SRC_DIR}/vcp_hal")

set(STM32F7_VCP_SRC
    usbd_desc.c
    usbd_conf.c
    usbd_cdc_interface.c
)
list(TRANSFORM STM32F7_VCP_SRC PREPEND "${STM32F7_VCP_DIR}/")

set(STM32F7_INCLUDE_DIRS
    ${STM32F7_HAL_DIR}/Inc
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

function(target_stm32f7xx)
    target_stm32(
        SOURCES ${STM32F7_HAL_SRC} ${STM32F7_SRC}
        COMPILE_DEFINITIONS ${STM32F7_DEFINITIONS}
        COMPILE_OPTIONS ${CORTEX_M7_COMMON_OPTIONS} ${CORTEX_M7_COMPILE_OPTIONS}
        INCLUDE_DIRECTORIES ${STM32F7_INCLUDE_DIRS}
        LINK_OPTIONS ${CORTEX_M7_COMMON_OPTIONS} ${CORTEX_M7_LINK_OPTIONS}

        MSC_SOURCES ${STM32F7_USBMSC_SRC} ${STM32F7_MSC_SRC}
        VCP_SOURCES ${STM32F7_USB_SRC} ${STM32F7_VCP_SRC}
        VCP_INCLUDE_DIRECTORIES ${STM32F7_USB_INCLUDE_DIRS} ${STM32F7_VCP_DIR}

        OPTIMIZATION -O2

        OPENOCD_TARGET stm32f7x

        BOOTLOADER

        ${ARGN}
    )
endfunction()

macro(define_target_stm32f7 subfamily size)
    function(target_stm32f7${subfamily}x${size} name)
        set(func_ARGV ARGV)
        string(TOUPPER ${size} upper_size)
        get_stm32_flash_size(flash_size ${size})
        set(definitions
            STM32F7
            STM32F7${subfamily}xx
            STM32F7${subfamily}x${upper_size}
            FLASH_SIZE=${flash_size}
        )
        target_stm32f7xx(
            NAME ${name}
            STARTUP startup_stm32f7${subfamily}xx.s
            COMPILE_DEFINITIONS ${definitions}
            LINKER_SCRIPT stm32_flash_f7${subfamily}x${size}
            ${${func_ARGV}}
        )
    endfunction()
endmacro()

define_target_stm32f7(22 e)
define_target_stm32f7(45 g)
define_target_stm32f7(46 g)
define_target_stm32f7(65 g)
define_target_stm32f7(65 i)
