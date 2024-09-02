cmake_minimum_required (VERSION 3.14)

project(CMSISDSPCommon)

include(configLib)
include(configDsp)

add_library(CMSISDSPCommon STATIC arm_common_tables.c arm_common_tables_f16.c)

configLib(CMSISDSPCommon ${ROOT})
configDsp(CMSISDSPCommon ${ROOT})

if (CONFIGTABLE AND ALLFFT)
    target_compile_definitions(CMSISDSPCommon PUBLIC ARM_ALL_FFT_TABLES) 
endif()

if (CONFIGTABLE AND ALLFAST)
    target_compile_definitions(CMSISDSPCommon PUBLIC ARM_ALL_FAST_TABLES) 
endif()

include(fft)
fft(CMSISDSPCommon)

include(interpol)
interpol(CMSISDSPCommon)

target_sources(CMSISDSPCommon PRIVATE arm_const_structs.c)
target_sources(CMSISDSPCommon PRIVATE arm_const_structs_f16.c)


### Includes
target_include_directories(CMSISDSPCommon PUBLIC "${DSP}/Include")

if (NEON OR NEONEXPERIMENTAL)
    target_sources(CMSISDSPCommon PRIVATE "${DSP}/ComputeLibrary/Source/arm_cl_tables.c")
endif()

if (HELIUM OR MVEF)
    target_sources(CMSISDSPCommon PRIVATE "${DSP}/Source/CommonTables/arm_mve_tables.c")
    target_sources(CMSISDSPCommon PRIVATE "${DSP}/Source/CommonTables/arm_mve_tables_f16.c")
endif()


if (WRAPPER)
    target_compile_definitions(CMSISDSPCommon PUBLIC ARM_TABLE_BITREV_1024)
    target_compile_definitions(CMSISDSPCommon PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096)
    target_compile_definitions(CMSISDSPCommon PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_4096)
    target_compile_definitions(CMSISDSPCommon PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_4096)
    if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
    target_compile_definitions(CMSISDSPCommon PUBLIC ARM_TABLE_TWIDDLECOEF_F16_4096)
    endif()
endif()