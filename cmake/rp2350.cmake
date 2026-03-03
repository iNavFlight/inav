include(cortex-m33)

set(PICO_SDK_DIR "${MAIN_LIB_DIR}/main/pico-sdk")
set(PICO_SDK_SRC_DIR "${PICO_SDK_DIR}/src")
set(TINYUSB_SRC_DIR "${PICO_SDK_DIR}/lib/tinyusb/src")
set(RP2350_STARTUP_DIR "${MAIN_SRC_DIR}/startup")
set(RP2350_LINKER_DIR "${MAIN_SRC_DIR}/target/link")

# CMSIS directories (shared with STM32/AT32)
set(RP2350_CMSIS_DIR "${MAIN_LIB_DIR}/main/CMSIS")
set(RP2350_CMSIS_INCLUDE_DIR "${RP2350_CMSIS_DIR}/Core/Include")
set(RP2350_CMSIS_DSP_DIR "${RP2350_CMSIS_DIR}/DSP")
set(RP2350_CMSIS_DSP_INCLUDE_DIR "${RP2350_CMSIS_DSP_DIR}/Include")

# NOTE: CMSIS DSP sources are NOT compiled for RP2350.
# The bundled CMSIS DSP (arm_math.h) predates ARMv8-M and conflicts with
# the newer CMSIS Core headers. DSP functions are stubbed in system_rp2350.c.
# The DSP include dir is still needed for arm_math.h type definitions.

# Sources to exclude from COMMON_SRC (same as SITL + additional STM32-specific)
main_sources(RP2350_COMMON_SRC_EXCLUDES
    build/atomic.h
    drivers/system.c
    drivers/time.c
    drivers/timer.c
    drivers/rcc.c
    drivers/persistent.c
    drivers/io.c
    drivers/accgyro/accgyro_mpu.c
    drivers/display_ug2864hsweg01.c
    io/displayport_oled.c
    # LED strip: replaced by light_ws2811strip_rp2350.c (PIO2+DMA, no timer/DMA abstraction)
    drivers/light_ws2811strip.c
    # I2C: replaced by bus_i2c_rp2350.c (Pico SDK blocking API; HAL version needs STM32 HAL)
    drivers/bus_i2c_hal.c
    # SPI: replaced by bus_spi_rp2350.c (Pico SDK blocking API; HAL/LL version needs STM32 HAL)
    drivers/bus_spi_hal_ll.c
)

# INAV platform-specific sources
main_sources(RP2350_SRC
    config/config_streamer_rp2350.c
    drivers/io_rp2350.c
    drivers/system_rp2350.c
    drivers/timer_rp2350.c
    drivers/serial_usb_vcp_rp2350.c
    drivers/serial_uart_rp2350.c
    drivers/uart_pio_rp2350.c
    drivers/pwm_output_rp2350.c
    drivers/adc_rp2350.c
    drivers/light_ws2811strip_rp2350.c
    drivers/bus_i2c_rp2350.c
    drivers/bus_spi_rp2350.c
    drivers/persistent_rp2350.c
)

# --- Pico SDK sources ---
set(PICO_SDK_SOURCES
    # Startup (SDK crt0 replaces our custom startup_rp2350.s)
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_crt0/crt0.S

    # Platform & runtime
    ${PICO_SDK_SRC_DIR}/rp2350/pico_platform/platform.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_platform_panic/panic.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_runtime/runtime.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_runtime_init/runtime_init.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_runtime_init/runtime_init_clocks.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_runtime_init/runtime_init_stack_guard.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_standard_binary_info/standard_binary_info.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_bootrom/bootrom.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_bootrom/bootrom_lock.c

    # Hardware drivers
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_gpio/gpio.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_clocks/clocks.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_pll/pll.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_xosc/xosc.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_ticks/ticks.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_timer/timer.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_watchdog/watchdog.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_irq/irq.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_irq/irq_handler_chain.S
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_sync/sync.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_sync_spin_lock/sync_spin_lock.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_vreg/vreg.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_xip_cache/xip_cache.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_uart/uart.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_pio/pio.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_adc/adc.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_dma/dma.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_i2c/i2c.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_spi/spi.c

    # Sync / time / util
    ${PICO_SDK_SRC_DIR}/common/hardware_claim/claim.c
    ${PICO_SDK_SRC_DIR}/common/pico_sync/mutex.c
    ${PICO_SDK_SRC_DIR}/common/pico_sync/lock_core.c
    ${PICO_SDK_SRC_DIR}/common/pico_sync/critical_section.c
    ${PICO_SDK_SRC_DIR}/common/pico_sync/sem.c
    ${PICO_SDK_SRC_DIR}/common/pico_time/time.c
    ${PICO_SDK_SRC_DIR}/common/pico_time/timeout_helper.c
    ${PICO_SDK_SRC_DIR}/common/pico_util/datetime.c
    ${PICO_SDK_SRC_DIR}/common/pico_util/pheap.c
    ${PICO_SDK_SRC_DIR}/common/pico_util/queue.c

    # C library interface / stdlib / atomic
    # NOTE: pico_malloc is excluded — it uses --wrap=malloc which conflicts with
    # INAV's existing allocation. newlib's malloc is used directly instead.
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_clib_interface/newlib_interface.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_stdlib/stdlib.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_atomic/atomic.c

    # stdio / printf / stdio_usb
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_stdio/stdio.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_printf/printf.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_stdio_usb/stdio_usb.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_stdio_usb/stdio_usb_descriptors.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_stdio_usb/reset_interface.c

    # Unique ID
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_unique_id/unique_id.c

    # Divider (compiler builtins)
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_divider/divider_compiler.c
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_divider/divider.c

    # Flash (needed for unique_id)
    ${PICO_SDK_SRC_DIR}/rp2_common/hardware_flash/flash.c
    ${PICO_SDK_SRC_DIR}/rp2_common/pico_flash/flash.c
)

# --- TinyUSB sources ---
set(TINYUSB_SOURCES
    ${TINYUSB_SRC_DIR}/tusb.c
    ${TINYUSB_SRC_DIR}/common/tusb_fifo.c
    ${TINYUSB_SRC_DIR}/device/usbd.c
    ${TINYUSB_SRC_DIR}/device/usbd_control.c
    ${TINYUSB_SRC_DIR}/class/cdc/cdc_device.c
    ${TINYUSB_SRC_DIR}/portable/raspberrypi/rp2040/dcd_rp2040.c
    ${TINYUSB_SRC_DIR}/portable/raspberrypi/rp2040/rp2040_usb.c
)

# --- Pico SDK include directories ---
set(PICO_SDK_INCLUDE_DIRS
    # Common headers
    "${PICO_SDK_SRC_DIR}/common/pico_base_headers/include"
    "${PICO_SDK_SRC_DIR}/common/pico_binary_info/include"
    "${PICO_SDK_SRC_DIR}/common/pico_bit_ops_headers/include"
    "${PICO_SDK_SRC_DIR}/common/pico_stdlib_headers/include"
    "${PICO_SDK_SRC_DIR}/common/pico_sync/include"
    "${PICO_SDK_SRC_DIR}/common/pico_time/include"
    "${PICO_SDK_SRC_DIR}/common/pico_util/include"
    "${PICO_SDK_SRC_DIR}/common/pico_usb_reset_interface_headers/include"
    "${PICO_SDK_SRC_DIR}/common/hardware_claim/include"
    "${PICO_SDK_SRC_DIR}/common/boot_picobin_headers/include"
    "${PICO_SDK_SRC_DIR}/common/boot_picoboot_headers/include"
    "${PICO_SDK_SRC_DIR}/common/boot_uf2_headers/include"
    "${PICO_SDK_SRC_DIR}/common/pico_divider_headers/include"

    # RP2350-specific headers
    "${PICO_SDK_SRC_DIR}/rp2350/hardware_regs/include"
    "${PICO_SDK_SRC_DIR}/rp2350/hardware_structs/include"
    "${PICO_SDK_SRC_DIR}/rp2350/pico_platform/include"
    "${PICO_SDK_SRC_DIR}/rp2350/boot_stage2/include"

    # rp2_common hardware
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_base/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_gpio/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_clocks/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_irq/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_pll/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_sync/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_sync_spin_lock/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_ticks/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_timer/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_uart/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_resets/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_watchdog/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_xosc/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_vreg/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_xip_cache/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_divider/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_flash/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_boot_lock/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_exception/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_dma/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_spi/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_i2c/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_adc/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_pwm/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_pio/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/hardware_powman/include"

    # rp2_common pico libraries
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_platform_compiler/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_platform_panic/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_platform_sections/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_stdio/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_stdio_usb/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_printf/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_runtime/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_runtime_init/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_bootrom/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_unique_id/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_stdlib/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_clib_interface/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_malloc/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_atomic/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_flash/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_float/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_double/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_int64_ops/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_mem_ops/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_multicore/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_rand/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_standard_binary_info/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_aon_timer/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_time_adapter/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/boot_bootrom_headers/include"
    "${PICO_SDK_SRC_DIR}/rp2_common/pico_divider/include"

    # CMSIS
    "${PICO_SDK_SRC_DIR}/rp2_common/cmsis/include"

    # TinyUSB
    "${TINYUSB_SRC_DIR}"
    "${PICO_SDK_SRC_DIR}/rp2_common/tinyusb/include"
)

set(RP2350_DEFINITIONS
    RP2350
    PICO_RP2350=1
    PICO_RP2350A=1
    PICO_32BIT=1
    # Do NOT define PICO_RP2040 at all — even PICO_RP2040=0 counts as "defined"
    # for #ifdef/#elif defined() checks in the Pico SDK assembly (embedded_start_block.inc.S),
    # which would cause the IMAGE_DEF to declare RP2040 chip type instead of RP2350.
    PICO_CONFIG_HEADER=pico_sdk_config.h
    PICO_NO_FPGA_CHECK=1
    PICO_FLASH_SIZE_BYTES=4194304
    # Tell pico_stdio_usb that TinyUSB device mode is already linked by the application.
    # Without this, stdio_usb_init() would call tusb_init() a second time AND set up its
    # own low-priority IRQ + alarm to call tud_task(), racing with INAV's own 1ms timer.
    # The race corrupts TinyUSB internal state, causing USB CDC to drop after a few seconds.
    LIB_TINYUSB_DEVICE=1
    ${CORTEX_M33_DEFINITIONS}
)

set(RP2350_COMPILE_OPTIONS
    ${CORTEX_M33_COMMON_OPTIONS}
    -ffunction-sections
    -fdata-sections
    -fno-common
    -fno-builtin-memcpy
    -funsigned-char
)

set(RP2350_LINK_LIBRARIES
    -lm
    -lc
    -lnosys
)

set(RP2350_LINK_OPTIONS
    ${CORTEX_M33_COMMON_OPTIONS}
    -nostartfiles
    --specs=nano.specs
    -static
    -Wl,-gc-sections
    -Wl,-L${RP2350_LINKER_DIR}
    -Wl,--cref
    -Wl,--no-wchar-size-warning
    -Wl,--print-memory-usage
)

set(RP2350_INCLUDE_DIRS
    "${RP2350_CMSIS_INCLUDE_DIR}"
    "${RP2350_CMSIS_DSP_INCLUDE_DIR}"
    "${MAIN_SRC_DIR}/target"
    ${PICO_SDK_INCLUDE_DIRS}
)

function(target_rp2350 name)
    if(NOT arm-none-eabi STREQUAL TOOLCHAIN)
        return()
    endif()

    exclude(COMMON_SRC "${RP2350_COMMON_SRC_EXCLUDES}")

    set(target_sources)
    # Pico SDK startup (crt0.S) and SDK sources
    list(APPEND target_sources ${PICO_SDK_SOURCES})
    # TinyUSB sources
    list(APPEND target_sources ${TINYUSB_SOURCES})
    # Boot2 stage 2 bootloader
    list(APPEND target_sources
        ${RP2350_STARTUP_DIR}/boot2_rp2350.S
    )
    # INAV platform sources
    list(APPEND target_sources ${RP2350_SRC})
    # Target directory sources
    file(GLOB target_c_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
    file(GLOB target_h_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    list(APPEND target_sources ${target_c_sources} ${target_h_sources})

    set(target_definitions ${RP2350_DEFINITIONS} ${COMMON_COMPILE_DEFINITIONS})

    math(EXPR hse_value "12 * 1000000")
    list(APPEND target_definitions "HSE_VALUE=${hse_value}")

    if (MSP_UART)
        list(APPEND target_definitions "MSP_UART=${MSP_UART}")
    endif()

    string(TOLOWER ${PROJECT_NAME} lowercase_project_name)
    set(binary_name ${lowercase_project_name}_${FIRMWARE_VERSION}_${name})
    if(DEFINED BUILD_SUFFIX AND NOT "" STREQUAL "${BUILD_SUFFIX}")
        set(binary_name "${binary_name}_${BUILD_SUFFIX}")
    endif()

    set(exe_target ${name}.elf)
    add_executable(${exe_target})
    target_sources(${exe_target} PRIVATE ${target_sources} ${COMMON_SRC})
    target_include_directories(${exe_target} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${RP2350_INCLUDE_DIRS}
    )
    target_compile_definitions(${exe_target} PRIVATE ${target_definitions})
    target_compile_options(${exe_target} PRIVATE ${RP2350_COMPILE_OPTIONS})

    # Pico SDK requires C11 (static_assert). Override INAV's global C99 setting.
    set_target_properties(${exe_target} PROPERTIES C_STANDARD 11)

    if(WARNINGS_AS_ERRORS)
        target_compile_options(${exe_target} PRIVATE -Werror)
    endif()

    target_link_libraries(${exe_target} PRIVATE ${RP2350_LINK_LIBRARIES})
    target_link_options(${exe_target} PRIVATE ${RP2350_LINK_OPTIONS})

    # Linker script
    set(script_path ${RP2350_LINKER_DIR}/rp2350_flash.ld)
    if(NOT EXISTS ${script_path})
        message(FATAL_ERROR "linker script ${script_path} doesn't exist")
    endif()
    set_target_properties(${exe_target} PROPERTIES LINK_DEPENDS ${script_path})
    target_link_options(${exe_target} PRIVATE -T${script_path})

    # Map file
    if(CMAKE_VERSION VERSION_LESS 3.15)
        set(map "$<TARGET_FILE:${exe_target}>.map")
    else()
        set(map "$<TARGET_FILE_DIR:${exe_target}>/$<TARGET_FILE_BASE_NAME:${exe_target}>.map")
    endif()
    target_link_options(${exe_target} PRIVATE "-Wl,-Map,${map}")

    # Generate .hex output
    set(hex_filename ${CMAKE_BINARY_DIR}/${binary_name}.hex)
    add_custom_target(${name} ALL
        cmake -E env PATH="$ENV{PATH}"
        ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${exe_target}> ${hex_filename}
        BYPRODUCTS ${hex_filename}
    )

    # Generate .bin output
    set(bin_filename ${CMAKE_BINARY_DIR}/${binary_name}.bin)
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${exe_target}> ${bin_filename}
        BYPRODUCTS ${bin_filename}
        COMMENT "Generating ${binary_name}.bin"
    )

    # Generate .uf2 output (for BOOTSEL drag-and-drop flashing)
    # Use picotool which correctly handles the RP2350 IMAGE_DEF metadata block.
    set(uf2_filename ${CMAKE_BINARY_DIR}/${binary_name}.uf2)
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND picotool uf2 convert
            $<TARGET_FILE:${exe_target}>
            ${uf2_filename}
            --family rp2350-arm-s
        BYPRODUCTS ${uf2_filename}
        COMMENT "Generating ${binary_name}.uf2 via picotool"
    )

    setup_firmware_target(${exe_target} ${name} ${ARGN})

    # clean_<target>
    set(generator_cmd "")
    if (CMAKE_GENERATOR STREQUAL "Unix Makefiles")
        set(generator_cmd "make")
    elseif(CMAKE_GENERATOR STREQUAL "Ninja")
        set(generator_cmd "ninja")
    endif()
    if (NOT generator_cmd STREQUAL "")
        set(clean_target "clean_${name}")
        add_custom_target(${clean_target}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${generator_cmd} clean
            COMMENT "Removing intermediate files for ${name}")
        set_property(TARGET ${clean_target} PROPERTY
            EXCLUDE_FROM_ALL 1
            EXCLUDE_FROM_DEFAULT_BUILD 1)
    endif()
endfunction()
