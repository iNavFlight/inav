include(cortex-m4f)
include(at32-stdperiph)
include(at32f4-usb)

set(AT32F4_STDPERIPH_DIR "${MAIN_LIB_DIR}/main/AT32F43x/Drivers/AT32F43x_StdPeriph_Driver")
set(AT32F4_CMSIS_DEVICE_DIR "${MAIN_LIB_DIR}/main/AT32F43x/Drivers/CMSIS/Device/ST/AT32F43x")
set(AT32F4_CMSIS_DRIVERS_DIR "${MAIN_LIB_DIR}/main/AT32F43x/Drivers/CMSIS")

  
set(AT32F4_STDPERIPH_SRC_EXCLUDES
        at32f435_437_can.c
        at32f435_437_dvp.c
        at32f435_437_emac
        at32f435_437_xmc.c
)

set(AT32F4_STDPERIPH_SRC_DIR "${AT32F4_STDPERIPH_DIR}/src")
glob_except(AT32F4_STDPERIPH_SRC "${AT32F4_STDPERIPH_SRC_DIR}/*.c" "${AT32F4_STDPERIPH_SRC_EXCLUDES}")
 
list(APPEND AT32F4_STDPERIPH_SRC "${AT32F4_CMSIS_DEVICE_DIR}/at32f435_437_clock.c" )

main_sources(AT32F4_SRC
    target/system_at32f435_437.c
    config/config_streamer_at32f43x.c
    config/config_streamer_ram.c
    config/config_streamer_extflash.c 
    drivers/adc_at32f43x.c
    drivers/i2c_application.c
    drivers/bus_i2c_at32f43x.c
    drivers/bus_spi_at32f43x
    drivers/serial_uart_hal_at32f43x.c
    drivers/serial_uart_at32f43x.c

    drivers/system_at32f43x.c
    drivers/timer.c
    drivers/timer_impl_stdperiph_at32.c
    drivers/timer_at32f43x.c
    drivers/uart_inverter.c
    drivers/dma_at32f43x.c
)
 
set(AT32F4_INCLUDE_DIRS
    ${CMSIS_INCLUDE_DIR}
    ${CMSIS_DSP_INCLUDE_DIR}
    ${AT32F4_CMSIS_DRIVERS_DIR}
    ${AT32F4_STDPERIPH_DIR}/inc
    ${AT32F4_CMSIS_DEVICE_DIR}
    #"${AT32F4_I2C_DIR}"
)

set(AT32F4_DEFINITIONS
    ${CORTEX_M4F_DEFINITIONS}
    AT32F43x
    USE_STDPERIPH_DRIVER
)

function(target_at32f43x)
    target_at32(
        SOURCES ${AT32_STDPERIPH_SRC} ${AT32F4_SRC}
        COMPILE_DEFINITIONS ${AT32F4_DEFINITIONS}
        COMPILE_OPTIONS ${CORTEX_M4F_COMMON_OPTIONS} ${CORTEX_M4F_COMPILE_OPTIONS}
        INCLUDE_DIRECTORIES ${AT32F4_INCLUDE_DIRS}
        LINK_OPTIONS ${CORTEX_M4F_COMMON_OPTIONS} ${CORTEX_M4F_LINK_OPTIONS}

        MSC_SOURCES ${AT32F4_USBMSC_SRC} ${AT32F4_MSC_SRC}
        VCP_SOURCES ${AT32F4_USB_SRC} ${AT32F4_VCP_SRC}
        VCP_INCLUDE_DIRECTORIES ${AT32F4_USB_INCLUDE_DIRS}

        OPTIMIZATION -O2

        OPENOCD_TARGET at32f437xx

        ${ARGN}
    )
endfunction()

#target_at32f43x_xMT7
#target_at32f43x_xGT7

set(at32f43x_xMT7_COMPILE_DEFINITIONS
    AT32F437VMT7
    MCU_FLASH_SIZE=4032
)

function(target_at32f43x_xMT7 name)
    target_at32f43x(
        NAME ${name}
        STARTUP startup_at32f435_437.s
        SOURCES ${AT32F4_STDPERIPH_SRC}
        COMPILE_DEFINITIONS ${at32f43x_xMT7_COMPILE_DEFINITIONS}
        LINKER_SCRIPT at32_flash_f43xM
        #BOOTLOADER
        SVD at32f43x_xMT7
        ${ARGN}
    )
endfunction()

set(at32f43x_xGT7_COMPILE_DEFINITIONS
    AT32F435RGT7
    MCU_FLASH_SIZE=1024
)

function(target_at32f43x_xGT7 name)
    target_at32f43x(
        NAME ${name}
        STARTUP startup_at32f435_437.s
        SOURCES ${AT32F4_STDPERIPH_SRC}
        COMPILE_DEFINITIONS ${at32f43x_xGT7_COMPILE_DEFINITIONS}
        LINKER_SCRIPT at32_flash_f43xG
        #BOOTLOADER
        SVD at32f43x_xGT7
        ${ARGN}
    )
endfunction()
