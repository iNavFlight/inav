include(arm-none-eabi)
include(stm32-usb)

set(CMSIS_DIR "${INAV_LIB_DIR}/main/CMSIS")
set(CMSIS_INCLUDE_DIR "${CMSIS_DIR}/Core/Include")
set(CMSIS_DSP_DIR "${INAV_LIB_DIR}/main/CMSIS/DSP")
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

set(STM32_STARTUP_DIR "${INAV_MAIN_SRC_DIR}/startup")

set(STM32_VCP_SRC
    drivers/serial_usb_vcp.c
    drivers/usb_io.c
)
main_sources(STM32_VCP_SRC)

set(STM32_MSC_SRC
    msc/usbd_msc_desc.c
    msc/usbd_storage.c
)
main_sources(STM32_MSC_SRC)

set(STM32_MSC_FLASH_SRC
    msc/usbd_storage_emfat.c
    msc/emfat.c
    msc/emfat_file.c
)
main_sources(STM32_MSC_FLASH_SRC)

set(STM32F4_STDPERIPH_DIR "${INAV_LIB_DIR}/main/STM32F4/Drivers/STM32F4xx_StdPeriph_Driver")
set(STM32F4_CMSIS_DEVICE_DIR "${INAV_LIB_DIR}/main/STM32F4/Drivers/CMSIS/Device/ST/STM32F4xx")
set(STM32F4_CMSIS_DRIVERS_DIR "${INAV_LIB_DIR}/main/STM32F4/Drivers/CMSIS")
set(STM32F4_VCP_DIR "${INAV_MAIN_SRC_DIR}/vcpf4")

set(STM32F4_STDPERIPH_SRC_EXCLUDES
    stm32f4xx_can.c
    stm32f4xx_cec.c
    stm32f4xx_crc.c
    stm32f4xx_cryp.c
    stm32f4xx_cryp_aes.c
    stm32f4xx_cryp_des.c
    stm32f4xx_cryp_tdes.c
    stm32f4xx_dbgmcu.c
    stm32f4xx_dsi.c
    stm32f4xx_flash_ramfunc.c
    stm32f4xx_fmpi2c.c
    stm32f4xx_fmc.c
    stm32f4xx_hash.c
    stm32f4xx_hash_md5.c
    stm32f4xx_hash_sha1.c
    stm32f4xx_lptim.c
    stm32f4xx_qspi.c
    stm32f4xx_sai.c
    stm32f4xx_spdifrx.c
)

set(STM32F4_STDPERIPH_SRC_DIR "${STM32F4_STDPERIPH_DIR}/Src")
glob_except(STM32F4_STDPERIPH_SRC "${STM32F4_STDPERIPH_SRC_DIR}/*.c" STM32F4_STDPERIPH_SRC_EXCLUDES)

set(STM32F4_VCP_SRC
    stm32f4xx_it.c
    usb_bsp.c
    usbd_desc.c
    usbd_usr.c
    usbd_cdc_vcp.c
)
list(TRANSFORM STM32F4_VCP_SRC PREPEND "${STM32F4_VCP_DIR}/")

set(STM32F4_MSC_SRC
    drivers/usb_msc_f4xx.c
)
main_sources(STM32F4_MSC_SRC)

set(STM32F4_INCLUDE_DIRS
    "${CMSIS_INCLUDE_DIR}"
    "${CMSIS_DSP_INCLUDE_DIR}"
    "${STM32F4_STDPERIPH_DIR}/inc"
    "${STM32F4_CMSIS_DEVICE_DIR}"
    "${STM32F4_CMSIS_DRIVERS_DIR}"
    "${STM32F4_VCP_DIR}"
)

set(STM32_INCLUDE_DIRS
    "${INAV_MAIN_SRC_DIR}/target"
)

set(STM32_LINKER_DIR "${INAV_MAIN_SRC_DIR}/target/link")

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

set(STM32_LINK_OPTIONS
    -nostartfiles
    --specs=nano.specs
    -static
    -Wl,-gc-sections,-Map,target.map
    -Wl,-L${STM32_LINKER_DIR}
    -Wl,--cref
    -Wl,--no-wchar-size-warning
    -Wl,--print-memory-usage
)

set(STM32F4_SRC
    target/system_stm32f4xx.c
    drivers/accgyro/accgyro.c
    drivers/accgyro/accgyro_mpu.c
    drivers/adc_stm32f4xx.c
    drivers/adc_stm32f4xx.c
    drivers/bus_i2c_stm32f40x.c
    drivers/serial_softserial.c
    drivers/serial_uart_stm32f4xx.c
    drivers/system_stm32f4xx.c
    drivers/timer.c
    drivers/timer_impl_stdperiph.c
    drivers/timer_stm32f4xx.c
    drivers/uart_inverter.c
    drivers/dma_stm32f4xx.c
    drivers/sdcard/sdmmc_sdio_f4xx.c
)

main_sources(STM32F4_SRC)

set(STM32F4_DEFINITIONS
    STM32F4
    USE_STDPERIPH_DRIVER
    ARM_MATH_MATRIX_CHECK
    ARM_MATH_ROUNDING
    __FPU_PRESENT=1
    UNALIGNED_SUPPORT_DISABLE
    ARM_MATH_CM4
)

set(STM32F4_COMMON_OPTIONS
    -mthumb
    -mcpu=cortex-m4
    -march=armv7e-m
    -mfloat-abi=hard
    -mfpu=fpv4-sp-d16
    -fsingle-precision-constant
)

set(STM32F4_COMPILE_OPTIONS
)

set(SETM32F4_LINK_OPTIONS
)

set(STM32F411_STDPERIPH_SRC_EXCLUDES "stm32f4xx_fsmc.c")

set(STM32F411_COMPILE_DEFINITIONS
    FLASH_SIZE=512
)

macro(get_stm32_target_features) # out-var dir
    file(READ "${ARGV1}/target.h" _contents)
    string(REGEX MATCH "#define[\t ]+USE_VCP" HAS_VCP ${_contents})
    if(HAS_VCP)
        list(APPEND ${ARGV0} VCP)
    endif()
    string(REGEX MATCH "define[\t ]+USE_FLASHFS" HAS_FLASHFS ${_contents})
    if(HAS_FLASHFS)
        list(APPEND ${ARGV0} FLASHFS)
    endif()
    if (HAS_FLASHFS) # || SDCARD
        list(APPEND ${ARGV0} MSC)
    endif()
endmacro()

function(target_stm32 name)
    # Main .elf target
    add_executable(${name} ${COMMON_SRC} ${CMSIS_DSP_SRC})
    file(GLOB target_c_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
    file(GLOB target_h_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    target_sources(${name} PRIVATE ${target_c_sources} ${target_h_sources})
    target_include_directories(${name} PRIVATE . ${STM32_INCLUDE_DIRS})
    target_link_libraries(${name} PRIVATE ${STM32_LINK_LIBRARIES})
    target_link_options(${name} PRIVATE ${STM32_LINK_OPTIONS})
    get_stm32_target_features(features "${CMAKE_CURRENT_SOURCE_DIR}")
    set_property(TARGET ${name} PROPERTY FEATURES ${features})
    if(VCP IN_LIST features)
        target_sources(${name} PRIVATE ${STM32_VCP_SRC})
    endif()
    if(MSC IN_LIST features)
        target_sources(${name} PRIVATE ${STM32_MSC_SRC})
        if (FLASHFS IN_LIST features)
            target_sources(${name} PRIVATE ${STM32_MSC_FLASH_SRC})
        endif()
    endif()
    # Generate .hex
    set(hexdir "${CMAKE_BINARY_DIR}/hex")
    set(hex "${hexdir}/$<TARGET_FILE_PREFIX:${name}>.hex")
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${hexdir}"
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${name}> "${hex}")
    # clean_<target>
    set(clean_target "clean_${name}")
    add_custom_target(${clean_target}
        COMMAND cmake -E rm -r "${CMAKE_CURRENT_BINARY_DIR}"
        COMMENT "Removeng intermediate files for ${name}")
    set_property(TARGET ${clean_target} PROPERTY TARGET_MESSAGES OFF)
endfunction()

function(target_stm32f4xx name)
    target_stm32(${name})
    target_sources(${name} PRIVATE ${STM32F4_SRC})
    target_compile_options(${name} PRIVATE ${STM32F4_COMMON_OPTIONS} ${STM32F4_COMPILE_OPTIONS})
    target_include_directories(${name} PRIVATE ${STM32_STDPERIPH_USB_INCLUDE_DIRS} ${STM32F4_INCLUDE_DIRS})
    target_compile_definitions(${name} PRIVATE ${STM32F4_DEFINITIONS})
    target_link_options(${name} PRIVATE ${STM32F4_COMMON_OPTIONS} ${STM32F4_LINK_OPTIONS})

    get_property(features TARGET ${name} PROPERTY FEATURES)
    if(VCP IN_LIST features)
        target_sources(${name} PRIVATE ${STM32_STDPERIPH_USB_SRC} ${STM32F4_VCP_SRC})
    endif()
    if(MSC IN_LIST features)
        target_sources(${name} PRIVATE ${STM32F4_MSC_SRC})
    endif()
endfunction()

function(target_stm32f411 name)
    target_stm32f4xx(${name})
    set(STM32F411_STDPERIPH_SRC ${STM32F4_STDPERIPH_SRC})
    exclude_basenames(STM32F411_STDPERIPH_SRC STM32F411_STDPERIPH_SRC_EXCLUDES)
    target_sources(${name} PRIVATE "${STM32_STARTUP_DIR}/startup_stm32f411xe.s" ${STM32F411_STDPERIPH_SRC})
    target_link_options(${name} PRIVATE "-T${STM32_LINKER_DIR}/stm32_flash_f411.ld")
    target_compile_definitions(${name} PRIVATE STM32F411xE ${STM32F411_COMPILE_DEFINITIONS})
    setup_firmware_target(${name})
endfunction()
