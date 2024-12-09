cmake_minimum_required (VERSION 3.14)

project(CMSISDSPComplexMath)

include(configLib)
include(configDsp)

add_library(CMSISDSPComplexMath STATIC)

configLib(CMSISDSPComplexMath ${ROOT})
configDsp(CMSISDSPComplexMath ${ROOT})


include(interpol)
interpol(CMSISDSPComplexMath)


if (CONFIGTABLE AND ALLFAST)
    target_compile_definitions(CMSISDSPComplexMath PUBLIC ARM_ALL_FAST_TABLES)  
endif()

# Vectorized code is defining sqrt
# so fast tables required even if Fast Math not built.
if (CONFIGTABLE AND (HELIUM OR MVEF OR MVEI))
    target_compile_definitions(CMSISDSPComplexMath PUBLIC ARM_FAST_ALLOW_TABLES)  
endif()


# MVE code is using a table for computing the fast sqrt arm_cmplx_mag_q31
# There is the possibility of not compiling this function and not including
# the table.
if (NOT CONFIGTABLE OR ALLFAST OR ARM_CMPLX_MAG_Q31 OR (NOT HELIUM AND NOT MVEI))
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mag_q31.c)
endif()

if (NOT CONFIGTABLE OR ALLFAST OR ARM_CMPLX_MAG_Q15 OR (NOT HELIUM AND NOT MVEI))
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mag_q15.c)
endif()

if (NOT CONFIGTABLE OR ALLFAST OR ARM_CMPLX_MAG_FAST_Q15 OR (NOT HELIUM AND NOT MVEI))
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mag_fast_q15.c)
endif()

target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_conj_f32.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_conj_q15.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_conj_q31.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_dot_prod_f32.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_dot_prod_q15.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_dot_prod_q31.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mag_f32.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mag_f64.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mag_squared_f32.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mag_squared_f64.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mag_squared_q15.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mag_squared_q31.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mult_cmplx_f32.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mult_cmplx_f64.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mult_cmplx_q15.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mult_cmplx_q31.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mult_real_f32.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mult_real_q15.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mult_real_q31.c)

if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_conj_f16.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_dot_prod_f16.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mag_f16.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mag_squared_f16.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mult_cmplx_f16.c)
target_sources(CMSISDSPComplexMath PRIVATE arm_cmplx_mult_real_f16.c)
endif()

### Includes
target_include_directories(CMSISDSPComplexMath PUBLIC "${DSP}/Include")
