include(cortex-m33)

set(PICO_SDK_DIR "${MAIN_LIB_DIR}/main/pico-sdk")
set(RP2350_STARTUP_DIR "${MAIN_SRC_DIR}/startup")
set(RP2350_LINKER_DIR "${MAIN_SRC_DIR}/target/link")

# CMSIS directories (shared with STM32/AT32)
set(RP2350_CMSIS_DIR "${MAIN_LIB_DIR}/main/CMSIS")
set(RP2350_CMSIS_INCLUDE_DIR "${RP2350_CMSIS_DIR}/Core/Include")
set(RP2350_CMSIS_DSP_DIR "${RP2350_CMSIS_DIR}/DSP")
set(RP2350_CMSIS_DSP_INCLUDE_DIR "${RP2350_CMSIS_DSP_DIR}/Include")

# NOTE: CMSIS DSP sources are NOT compiled for RP2350.
# The bundled CMSIS DSP (arm_math.h) predates ARMv8-M and conflicts with
# the newer CMSIS Core headers. DSP functions will be needed for M2+ when
# real sensor filtering is implemented - at that point, update to CMSIS-DSP 1.10+.
# The DSP include dir is still needed for arm_math.h type definitions used
# by headers like sensors/gyro.h.

# Sources to exclude from COMMON_SRC (same as SITL + additional STM32-specific)
main_sources(RP2350_COMMON_SRC_EXCLUDES
    build/atomic.h
    drivers/system.c
    drivers/time.c
    drivers/timer.c
    drivers/rcc.c
    drivers/persistent.c
    drivers/accgyro/accgyro_mpu.c
    drivers/display_ug2864hsweg01.c
    io/displayport_oled.c
)

# Platform-specific sources
main_sources(RP2350_SRC
    config/config_streamer_ram.c
)

set(RP2350_DEFINITIONS
    RP2350
    PICO_RP2350
    PICO_32BIT=1
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
)

function(target_rp2350 name)
    if(NOT arm-none-eabi STREQUAL TOOLCHAIN)
        return()
    endif()

    exclude(COMMON_SRC "${RP2350_COMMON_SRC_EXCLUDES}")

    set(target_sources)
    # Startup and boot2
    list(APPEND target_sources
        ${RP2350_STARTUP_DIR}/startup_rp2350.s
        ${RP2350_STARTUP_DIR}/boot2_rp2350.S
    )
    # Platform sources
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
    add_custom_target(${name}.bin
        cmake -E env PATH="$ENV{PATH}"
        ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${exe_target}> ${bin_filename}
        BYPRODUCTS ${bin_filename}
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
