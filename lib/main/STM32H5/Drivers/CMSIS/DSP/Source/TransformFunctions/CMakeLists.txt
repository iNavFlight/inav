cmake_minimum_required (VERSION 3.14)

project(CMSISDSPTransform)

include(configLib)
include(configDsp)

add_library(CMSISDSPTransform STATIC)
configLib(CMSISDSPTransform ${ROOT})
configDsp(CMSISDSPTransform ${ROOT})

include(fft)
fft(CMSISDSPTransform)

if (CONFIGTABLE AND ALLFFT)
target_compile_definitions(CMSISDSPTransform PUBLIC ARM_ALL_FFT_TABLES) 
endif() 

target_sources(CMSISDSPTransform PRIVATE arm_bitreversal.c)
target_sources(CMSISDSPTransform PRIVATE arm_bitreversal2.c)

if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
target_sources(CMSISDSPTransform PRIVATE arm_bitreversal_f16.c)
endif()

if (NOT CONFIGTABLE OR ALLFFT OR CFFT_F32_16 OR CFFT_F32_32 OR CFFT_F32_64 OR CFFT_F32_128 OR CFFT_F32_256 OR CFFT_F32_512 
    OR CFFT_F32_1024 OR CFFT_F32_2048 OR CFFT_F32_4096)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix2_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix8_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_init_f32.c)
endif()

if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
if (NOT CONFIGTABLE OR ALLFFT OR CFFT_F16_16 OR CFFT_F16_32 OR CFFT_F16_64 OR CFFT_F16_128 OR CFFT_F16_256 OR CFFT_F16_512 
    OR CFFT_F16_1024 OR CFFT_F16_2048 OR CFFT_F16_4096)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix2_f16.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_f16.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_f16.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_init_f16.c)
endif()
endif()

if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
if (NOT CONFIGTABLE OR ALLFFT OR RFFT_F16_128 OR RFFT_F16_512 OR RFFT_F16_2048 OR RFFT_F16_8192)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_init_f16.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_f16.c)
endif()
endif()

if (NOT CONFIGTABLE OR ALLFFT OR CFFT_F64_16 OR CFFT_F64_32 OR CFFT_F64_64 OR CFFT_F64_128 OR CFFT_F64_256 OR CFFT_F64_512 
    OR CFFT_F64_1024 OR CFFT_F64_2048 OR CFFT_F64_4096)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_f64.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_init_f64.c)
endif()

if (NOT CONFIGTABLE OR ALLFFT OR CFFT_Q15_16 OR CFFT_Q15_32 OR CFFT_Q15_64 OR CFFT_Q15_128 OR CFFT_Q15_256 OR CFFT_Q15_512 
    OR CFFT_Q15_1024 OR CFFT_Q15_2048 OR CFFT_Q15_4096)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix2_q15.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_q15.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_q15.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_init_q15.c)
endif()

if (NOT CONFIGTABLE OR ALLFFT OR CFFT_Q31_16 OR CFFT_Q31_32 OR CFFT_Q31_64 OR CFFT_Q31_128 OR CFFT_Q31_256 OR CFFT_Q31_512 
    OR CFFT_Q31_1024 OR CFFT_Q31_2048 OR CFFT_Q31_4096)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix2_q31.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_q31.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_q31.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_init_q31.c)
endif()


if (NOT CONFIGTABLE OR ALLFFT OR DCT4_F32_128 OR DCT4_F32_512 OR DCT4_F32_2048 OR DCT4_F32_8192)
target_sources(CMSISDSPTransform PRIVATE arm_dct4_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_dct4_init_f32.c)

target_sources(CMSISDSPTransform PRIVATE arm_rfft_init_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_rfft_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_init_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_f32.c)
endif()

if (NOT CONFIGTABLE OR ALLFFT OR DCT4_Q31_128 OR DCT4_Q31_512 OR DCT4_Q31_2048 OR DCT4_Q31_8192)
target_sources(CMSISDSPTransform PRIVATE arm_dct4_q31.c)
target_sources(CMSISDSPTransform PRIVATE arm_dct4_init_q31.c)

target_sources(CMSISDSPTransform PRIVATE arm_rfft_init_q31.c)
target_sources(CMSISDSPTransform PRIVATE arm_rfft_q31.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_q31.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_init_q31.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_init_q31.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_q31.c)
endif()

if (NOT CONFIGTABLE OR ALLFFT OR ALLFFT OR DCT4_Q15_128 OR DCT4_Q15_512 OR DCT4_Q15_2048 OR DCT4_Q15_8192)
target_sources(CMSISDSPTransform PRIVATE arm_dct4_init_q15.c)
target_sources(CMSISDSPTransform PRIVATE arm_dct4_q15.c)

target_sources(CMSISDSPTransform PRIVATE arm_rfft_init_q15.c)
target_sources(CMSISDSPTransform PRIVATE arm_rfft_q15.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_q15.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_init_q15.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_init_q15.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_q15.c)
endif()

if (NOT CONFIGTABLE OR ALLFFT OR RFFT_FAST_F32_32 OR RFFT_FAST_F32_64 OR RFFT_FAST_F32_128
   OR RFFT_FAST_F32_256 OR RFFT_FAST_F32_512 OR RFFT_FAST_F32_1024 OR RFFT_FAST_F32_2048
   OR RFFT_FAST_F32_4096 )
target_sources(CMSISDSPTransform PRIVATE arm_rfft_fast_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_rfft_fast_init_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_init_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix8_f32.c)
endif()

if (NOT CONFIGTABLE OR ALLFFT OR RFFT_FAST_F64_32 OR RFFT_FAST_F64_64 OR RFFT_FAST_F64_128
   OR RFFT_FAST_F64_256 OR RFFT_FAST_F64_512 OR RFFT_FAST_F64_1024 OR RFFT_FAST_F64_2048
   OR RFFT_FAST_F64_4096 )
target_sources(CMSISDSPTransform PRIVATE arm_rfft_fast_f64.c)
target_sources(CMSISDSPTransform PRIVATE arm_rfft_fast_init_f64.c)
endif()

if ((NOT DISABLEFLOAT16))
if (NOT CONFIGTABLE OR ALLFFT OR RFFT_FAST_F16_32 OR RFFT_FAST_F16_64 OR RFFT_FAST_F16_128
   OR RFFT_FAST_F16_256 OR RFFT_FAST_F16_512 OR RFFT_FAST_F16_1024 OR RFFT_FAST_F16_2048
   OR RFFT_FAST_F16_4096 )
target_sources(CMSISDSPTransform PRIVATE arm_rfft_fast_f16.c)
target_sources(CMSISDSPTransform PRIVATE arm_rfft_fast_init_f16.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_f16.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_init_f16.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix8_f16.c)
endif()
endif()

if (NOT CONFIGTABLE OR ALLFFT OR RFFT_F32_128 OR RFFT_F32_512 OR RFFT_F32_2048 OR RFFT_F32_8192)
target_sources(CMSISDSPTransform PRIVATE arm_rfft_init_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_rfft_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_init_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_f32.c)
endif()

if (NOT CONFIGTABLE OR ALLFFT OR RFFT_Q15_32 OR RFFT_Q15_64 OR RFFT_Q15_128 OR RFFT_Q15_256
     OR RFFT_Q15_512 OR RFFT_Q15_1024 OR RFFT_Q15_2048 OR RFFT_Q15_4096 OR RFFT_Q15_8192)
target_sources(CMSISDSPTransform PRIVATE arm_rfft_init_q15.c)
target_sources(CMSISDSPTransform PRIVATE arm_rfft_q15.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_q15.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_init_q15.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_q15.c)
endif()

if (NOT CONFIGTABLE OR ALLFFT OR RFFT_Q31_32 OR RFFT_Q31_64 OR RFFT_Q31_128 OR RFFT_Q31_256
     OR RFFT_Q31_512 OR RFFT_Q31_1024 OR RFFT_Q31_2048 OR RFFT_Q31_4096 OR RFFT_Q31_8192)
target_sources(CMSISDSPTransform PRIVATE arm_rfft_init_q31.c)
target_sources(CMSISDSPTransform PRIVATE arm_rfft_q31.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_q31.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_init_q31.c)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_q31.c)
endif()

if (WRAPPER OR ARM_CFFT_RADIX2_Q15)
  target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix2_init_q15.c)
endif()

if (NOT CONFIGTABLE  OR ALLFFT OR ARM_CFFT_RADIX4_Q15)
  target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_init_q15.c)
endif()

if (WRAPPER OR ARM_CFFT_RADIX2_Q31)
  target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix2_init_q31.c)
endif()

if (NOT CONFIGTABLE  OR ALLFFT OR ARM_CFFT_RADIX4_Q31)
  target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix4_init_q31.c)
endif()

# For scipy or wrappers or benchmarks
if (WRAPPER)
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix2_init_f32.c)
if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
target_sources(CMSISDSPTransform PRIVATE arm_cfft_radix2_init_f16.c)
endif()

    target_compile_definitions(CMSISDSPTransform PUBLIC ARM_TABLE_BITREV_1024)
    target_compile_definitions(CMSISDSPTransform PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096)
    target_compile_definitions(CMSISDSPTransform PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_4096)
    target_compile_definitions(CMSISDSPTransform PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_4096)
if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
    target_compile_definitions(CMSISDSPTransform PUBLIC ARM_TABLE_TWIDDLECOEF_F16_4096)
endif()
endif()

target_sources(CMSISDSPTransform PRIVATE arm_mfcc_init_f32.c)
target_sources(CMSISDSPTransform PRIVATE arm_mfcc_f32.c)

target_sources(CMSISDSPTransform PRIVATE arm_mfcc_init_q31.c)
target_sources(CMSISDSPTransform PRIVATE arm_mfcc_q31.c)

target_sources(CMSISDSPTransform PRIVATE arm_mfcc_init_q15.c)
target_sources(CMSISDSPTransform PRIVATE arm_mfcc_q15.c)

if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
target_sources(CMSISDSPTransform PRIVATE arm_mfcc_init_f16.c)
target_sources(CMSISDSPTransform PRIVATE arm_mfcc_f16.c)
endif()

### Includes
target_include_directories(CMSISDSPTransform PUBLIC "${DSP}/Include")



