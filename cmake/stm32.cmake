include(arm-none-eabi)
include(stm32f3)
include(stm32f4)
include(stm32f7)

include(CMakeParseArguments)

set(CMSIS_DIR "${MAIN_LIB_DIR}/main/CMSIS")
set(CMSIS_INCLUDE_DIR "${CMSIS_DIR}/Core/Include")
set(CMSIS_DSP_DIR "${MAIN_LIB_DIR}/main/CMSIS/DSP")
set(CMSIS_DSP_INCLUDE_DIR "${CMSIS_DSP_DIR}/Include")

set(CMSIS_DSP_SRC
    BasicMathFunctions/arm_mult_f32.c
    TransformFunctions/arm_rfft_fast_f32.c
    TransformFunctions/arm_cfft_f32.c
    TransformFunctions/arm_rfft_fast_init_f32.c
    TransformFunctions/arm_cfft_radix8_f32.c
    TransformFunctions/arm_bitreversal2.S
    CommonTables/arm_common_tables.c
    ComplexMathFunctions/arm_cmplx_mag_f32.c
    StatisticsFunctions/arm_max_f32.c
)
list(TRANSFORM CMSIS_DSP_SRC PREPEND "${CMSIS_DSP_DIR}/Source/")

set(STM32_STARTUP_DIR "${MAIN_SRC_DIR}/startup")

main_sources(STM32_VCP_SRC
    drivers/serial_usb_vcp.c
    drivers/usb_io.c
)

main_sources(STM32_SDCARD_SRC
    drivers/sdcard/sdcard.c
    drivers/sdcard/sdcard_spi.c
    drivers/sdcard/sdcard_sdio.c
    drivers/sdcard/sdcard_standard.c
)

# XXX: This code is not STM32 specific
main_sources(STM32_ASYNCFATFS_SRC
    io/asyncfatfs/asyncfatfs.c
    io/asyncfatfs/fat_standard.c
)

main_sources(STM32_MSC_SRC
    msc/usbd_msc_desc.c
    msc/usbd_storage.c
)

main_sources(STM32_MSC_FLASH_SRC
    msc/usbd_storage_emfat.c
    msc/emfat.c
    msc/emfat_file.c
)

main_sources(STM32_MSC_SDCARD_SPI_SRC
    msc/usbd_storage_sd_spi.c
)

main_sources(STM32_MSC_SDCARD_SDIO_SRC
    msc/usbd_storage_sdio.c
)

set(STM32_INCLUDE_DIRS
    "${CMSIS_INCLUDE_DIR}"
    "${CMSIS_DSP_INCLUDE_DIR}"
    "${MAIN_SRC_DIR}/target"
)

set(STM32_DEFINITIONS
)

set(STM32_DEFAULT_HSE_MHZ 8)

set(STM32_LINKER_DIR "${MAIN_SRC_DIR}/target/link")

set(STM32_LIBS lnosys)
#if(SEMIHOSTING)
#    set(SEMIHOSTING_DEFINITIONS "SEMIHOSTING")
#    set(SEMIHOSTING_LDFLAGS
#        --specs=rdimon.specs
#        -lc
#        -lrdimon
#    )
#else()
#    set(SYS)
#endif()
#ifneq ($(SEMIHOSTING),)
#SEMIHOSTING_CFLAGS	= -DSEMIHOSTING
#SEMIHOSTING_LDFLAGS	= --specs=rdimon.specs -lc -lrdimon
#SYSLIB			:=
#else
#SEMIHOSTING_LDFLAGS	=
#SEMIHOSTING_CFLAGS	=
#SYSLIB			:= -lnosys
#endif

set(STM32_LINK_LIBRARIES
    -lm
    -lc
)

if(SEMIHOSTING)
    # TODO: Is -lc needed again here due to library order or can it be deleted?
    list(APPEND STM32_LINK_LIBRARIES --specs=rdimon.specs -lc -lrdimon)
    list(APPEND STM32_DEFINITIONS SEMIHOSTING)
else()
    list(APPEND STM32_LINK_LIBRARIES -lnosys)
endif()

set(STM32_LINK_OPTIONS
    -nostartfiles
    --specs=nano.specs
    -static
    -Wl,-gc-sections
    -Wl,-L${STM32_LINKER_DIR}
    -Wl,--cref
    -Wl,--no-wchar-size-warning
    -Wl,--print-memory-usage
)

macro(get_stm32_target_features output_var dir target_name)
    execute_process(COMMAND "${CMAKE_C_COMPILER}" -E -dD -D${ARGV2} "${ARGV1}/target.h"
        ERROR_VARIABLE _errors
        RESULT_VARIABLE _result
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE _contents)

    if(NOT _result EQUAL 0)
        message(FATAL_ERROR "error extracting features for stm32 target ${ARGV2}: ${_errors}")
    endif()

    string(REGEX MATCH "#define[\t ]+USE_VCP" HAS_VCP ${_contents})
    if(HAS_VCP)
        list(APPEND ${ARGV0} VCP)
    endif()
    string(REGEX MATCH "define[\t ]+USE_FLASHFS" HAS_FLASHFS ${_contents})
    if(HAS_FLASHFS)
        list(APPEND ${ARGV0} FLASHFS)
    endif()
    string(REGEX MATCH "define[\t ]+USE_SDCARD" HAS_SDCARD ${_contents})
    if (HAS_SDCARD)
        list(APPEND ${ARGV0} SDCARD)
        string(REGEX MATCH "define[\t ]+USE_SDCARD_SDIO" HAS_SDIO ${_contents})
        if (HAS_SDIO)
            list(APPEND ${ARGV0} SDIO)
        endif()
    endif()
    if(HAS_FLASHFS OR HAS_SDCARD)
        list(APPEND ${ARGV0} MSC)
    endif()
endmacro()

function(target_stm32 name startup ldscript)
    set(target_definitions)
    # Parse keyword arguments
    cmake_parse_arguments(
        PARSED_ARGS
        "DISABLE_MSC"   # Boolean arguments
        "HSE_MHZ"       # Single value arguments
        "DEFINITIONS"   # Multi-value arguments
        ${ARGN}         # Start parsing after the known arguments
    )
    if (PARSED_ARGS_HSE_MHZ)
        set(hse_mhz ${PARSED_ARGS_HSE_MHZ})
    else()
        set(hse_mhz ${STM32_DEFAULT_HSE_MHZ})
    endif()

    # Main .elf target
    add_executable(${name} ${COMMON_SRC} ${CMSIS_DSP_SRC})
    file(GLOB target_c_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
    file(GLOB target_h_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    target_sources(${name} PRIVATE ${target_c_sources} ${target_h_sources})
    target_sources(${name} PRIVATE "${STM32_STARTUP_DIR}/${startup}")
    target_link_options(${name} PRIVATE "-T${STM32_LINKER_DIR}/${ldscript}")
    target_link_options(${name} PRIVATE "-Wl,-Map,$<TARGET_FILE:${name}>.map")
    target_include_directories(${name} PRIVATE . ${STM32_INCLUDE_DIRS})
    target_link_libraries(${name} PRIVATE ${STM32_LINK_LIBRARIES})
    target_link_options(${name} PRIVATE ${STM32_LINK_OPTIONS})

    set(target_definitions ${STM32_DEFINITIONS})
    math(EXPR hse_value "${hse_mhz} * 1000000")
    list(APPEND target_definitions "HSE_VALUE=${hse_value}")
    if(PARSED_ARGS_DEFINITIONS)
        list(APPEND target_definitions ${PARSED_ARGS_DEFINITIONS})
    endif()
    target_compile_definitions(${name} PRIVATE ${target_definitions})

    get_stm32_target_features(features "${CMAKE_CURRENT_SOURCE_DIR}" ${name})
    set_property(TARGET ${name} PROPERTY FEATURES ${features})
    if(VCP IN_LIST features)
        target_sources(${name} PRIVATE ${STM32_VCP_SRC})
    endif()
    if(SDCARD IN_LIST features)
        target_sources(${name} PRIVATE ${STM32_SDCARD_SRC} ${STM32_ASYNCFATFS_SRC})
    endif()
    if(NOT PARSED_ARGS_DISABLE_MSC AND MSC IN_LIST features)
        target_sources(${name} PRIVATE ${STM32_MSC_SRC})
        target_compile_definitions(${name} PRIVATE USE_USB_MSC)
        if (FLASHFS IN_LIST features)
            target_sources(${name} PRIVATE ${STM32_MSC_FLASH_SRC})
        endif()
        if (SDCARD IN_LIST features)
            if (SDIO IN_LIST features)
                target_sources(${name} PRIVATE ${STM32_MSC_SDCARD_SDIO_SRC})
            else()
                target_sources(${name} PRIVATE ${STM32_MSC_SDCARD_SPI_SRC})
            endif()
        endif()
    endif()
    # Generate .hex
    # XXX: Generator expressions are not supported for add_custom_command()
    # OUTPUT nor BYPRODUCTS, so we can't rely of them. Instead, build the filename
    # for the .hex from the target name
    set(hexdir "${CMAKE_BINARY_DIR}/hex")
    set(hex "${hexdir}/${PROJECT_NAME}_${name}_${FIRMWARE_VERSION}.hex")
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${hexdir}
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${name}> ${hex}
        BYPRODUCTS ${hex}
    )
    # clean_<target>
    set(clean_target "clean_${name}")
    add_custom_target(${clean_target}
        COMMAND cmake -E rm -r "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Removing intermediate files for ${name}")
    set_property(TARGET ${clean_target} PROPERTY
        TARGET_MESSAGES OFF
        EXCLUDE_FROM_ALL 1
        EXCLUDE_FROM_DEFAULT_BUILD 1)
endfunction()
