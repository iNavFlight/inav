main_sources(BOOTLOADER_SOURCES
    common/log.c
    common/log.h
    common/printf.c
    common/printf.h
    common/string_light.c
    common/string_light.h
    common/typeconversion.c
    common/typeconversion.h

    drivers/bus.c
    drivers/bus_busdev_i2c.c
    drivers/bus_busdev_spi.c
    drivers/bus_i2c_soft.c
    drivers/io.c
    drivers/light_led.c
    drivers/persistent.c
    drivers/rcc.c
    drivers/serial.c
    drivers/system.c
    drivers/time.c
    drivers/timer.c

    fc/firmware_update_common.c
    fc/firmware_update_common.h

    target/common_hardware.c
)

list(APPEND BOOTLOADER_SOURCES ${MAIN_DIR}/src/bl/bl_main.c)
