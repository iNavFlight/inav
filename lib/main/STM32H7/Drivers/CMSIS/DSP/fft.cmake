function(fft PROJECT)
#######################################
#
# CFFT F32
#


if (CONFIGTABLE AND CFFT_F32_16)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_16)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_16)
endif()

if (CONFIGTABLE AND CFFT_F32_32)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_32)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_32)
endif()

if (CONFIGTABLE AND CFFT_F32_64)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_64)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_64)
endif()

if (CONFIGTABLE AND CFFT_F32_128)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_128)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_128)
endif()

if (CONFIGTABLE AND CFFT_F32_256)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_256)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_256)
endif()

if (CONFIGTABLE AND CFFT_F32_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_512)
endif()

if (CONFIGTABLE AND CFFT_F32_1024)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_1024)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_1024)
endif()

if (CONFIGTABLE AND CFFT_F32_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_2048)
endif()

if (CONFIGTABLE AND CFFT_F32_4096)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_4096)
endif()

#######################################
#
# CFFT Q31
#

if (CONFIGTABLE AND CFFT_Q31_16)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_16)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_16)
endif()

if (CONFIGTABLE AND CFFT_Q31_32)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_32)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_32)
endif()

if (CONFIGTABLE AND CFFT_Q31_64)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_64)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_64)
endif()

if (CONFIGTABLE AND CFFT_Q31_128)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_128)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_128)
endif()

if (CONFIGTABLE AND CFFT_Q31_256)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_256)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_256)
endif()

if (CONFIGTABLE AND CFFT_Q31_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_512)
endif()

if (CONFIGTABLE AND CFFT_Q31_1024)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_1024)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_1024)
endif()

if (CONFIGTABLE AND CFFT_Q31_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_2048)
endif()

if (CONFIGTABLE AND CFFT_Q31_4096)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_4096)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_4096)
endif()

#######################################
#
# CFFT Q15
#

if (CONFIGTABLE AND CFFT_Q15_16)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_16)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_16)
endif()

if (CONFIGTABLE AND CFFT_Q15_32)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_32)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_32)
endif()

if (CONFIGTABLE AND CFFT_Q15_64)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_64)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_64)
endif()

if (CONFIGTABLE AND CFFT_Q15_128)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_128)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_128)
endif()

if (CONFIGTABLE AND CFFT_Q15_256)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_256)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_256)
endif()

if (CONFIGTABLE AND CFFT_Q15_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_512)
endif()

if (CONFIGTABLE AND CFFT_Q15_1024)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_1024)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_1024)
endif()

if (CONFIGTABLE AND CFFT_Q15_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_2048)
endif()

if (CONFIGTABLE AND CFFT_Q15_4096)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_4096)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_4096)
endif()

#######################################
#
# RFFT FAST F32
#

if (CONFIGTABLE AND RFFT_FAST_F32_32)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_16) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_16) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_RFFT_F32_32) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_16)  
endif()

if (CONFIGTABLE AND RFFT_FAST_F32_64)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_32) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_32) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_RFFT_F32_64) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_32)  
endif()

if (CONFIGTABLE AND RFFT_FAST_F32_128)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_64) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_64) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_RFFT_F32_128) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_64) 
endif()

if (CONFIGTABLE AND RFFT_FAST_F32_256)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_128) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_128) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_RFFT_F32_256)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_128) 
endif()

if (CONFIGTABLE AND RFFT_FAST_F32_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_256) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_256) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_RFFT_F32_512) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_256) 
endif()

if (CONFIGTABLE AND RFFT_FAST_F32_1024)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_512) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_512) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_RFFT_F32_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_512) 
endif()

if (CONFIGTABLE AND RFFT_FAST_F32_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_RFFT_F32_2048) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_1024) 
endif()

if (CONFIGTABLE AND RFFT_FAST_F32_4096)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_2048) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FLT_2048) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_RFFT_F32_4096) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_2048) 
endif()

#######################################
#
# RFFT F32
#

if (CONFIGTABLE AND RFFT_F32_32)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_F32) 
    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096) 
endif()

if (CONFIGTABLE AND RFFT_F32_64)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q31) 
    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096) 
endif()

if (CONFIGTABLE AND RFFT_F32_128)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_F32)
    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096)  
endif()

if (CONFIGTABLE AND RFFT_F32_256)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_F32) 
    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096) 
endif()

if (CONFIGTABLE AND RFFT_F32_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_F32)
    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096)  
endif()

if (CONFIGTABLE AND RFFT_F32_1024)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_F32)
    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096)  
endif()

if (CONFIGTABLE AND RFFT_F32_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_F32) 
    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096) 
endif()

if (CONFIGTABLE AND RFFT_F32_4096)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_F32)
    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096)  
endif()

if (CONFIGTABLE AND RFFT_F32_8192)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_F32) 
    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096) 
endif()

#######################################
#
# RFFT Q31
#

if (CONFIGTABLE AND RFFT_Q31_32)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q31) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_16)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_16)
endif()

if (CONFIGTABLE AND RFFT_Q31_64)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q31) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_32)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_32)
endif()

if (CONFIGTABLE AND RFFT_Q31_128)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q31) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_64)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_64)
endif()

if (CONFIGTABLE AND RFFT_Q31_256)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q31) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_128)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_128)
endif()

if (CONFIGTABLE AND RFFT_Q31_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q31) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_256)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_256)
endif()

if (CONFIGTABLE AND RFFT_Q31_1024)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q31) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_512)
endif()

if (CONFIGTABLE AND RFFT_Q31_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q31) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_1024)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_1024)
endif()

if (CONFIGTABLE AND RFFT_Q31_4096)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q31) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_2048)
endif()

if (CONFIGTABLE AND RFFT_Q31_8192)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q31) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_4096)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_4096)
endif()

#######################################
#
# RFFT FAST Q15
#

if (CONFIGTABLE AND RFFT_Q15_32)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q15)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_16)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_16) 
endif()

if (CONFIGTABLE AND RFFT_Q15_64)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q15) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_32)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_32)
endif()

if (CONFIGTABLE AND RFFT_Q15_128)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q15) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_64)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_64)
endif()

if (CONFIGTABLE AND RFFT_Q15_256)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q15) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_128)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_128)
endif()

if (CONFIGTABLE AND RFFT_Q15_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q15) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_256)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_256)
endif()

if (CONFIGTABLE AND RFFT_Q15_1024)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q15) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_512)
endif()

if (CONFIGTABLE AND RFFT_Q15_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q15)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_1024)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_1024) 
endif()

if (CONFIGTABLE AND RFFT_Q15_4096)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q15) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_2048)
endif()

if (CONFIGTABLE AND RFFT_Q15_8192)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q15) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_4096)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREVIDX_FXT_4096)
endif()

#######################################
#
# DCT4 F32
#

if (CONFIGTABLE AND DCT4_F32_128)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_DCT4_F32_128) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_F32) 

    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096)
endif()

if (CONFIGTABLE AND DCT4_F32_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_DCT4_F32_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_F32) 

    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096) 
endif()

if (CONFIGTABLE AND DCT4_F32_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_DCT4_F32_2048) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_F32) 

    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096)
endif()

if (CONFIGTABLE AND DCT4_F32_8192)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_DCT4_F32_8192) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_F32) 

    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_F32_4096)
endif()

#######################################
#
# DCT4 Q31
#

if (CONFIGTABLE AND DCT4_Q31_128)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_DCT4_Q31_128) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q31) 

    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_4096)
endif()

if (CONFIGTABLE AND DCT4_Q31_512) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_DCT4_Q31_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q31)

    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_4096)  
endif()

if (CONFIGTABLE AND DCT4_Q31_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_DCT4_Q31_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q31) 

    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_4096)
endif()

if (CONFIGTABLE AND DCT4_Q31_8192)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_DCT4_Q31_8192)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q31) 

    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q31_4096) 
endif()

#######################################
#
# DCT4 Q15
#

if (CONFIGTABLE AND DCT4_Q15_128) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_DCT4_Q15_128) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q15) 

    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_4096)
endif()

if (CONFIGTABLE AND DCT4_Q15_512) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_DCT4_Q15_512)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q15) 

    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_4096)
endif()

if (CONFIGTABLE AND DCT4_Q15_2048)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_DCT4_Q15_2048) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q15)

    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_4096)
endif()

if (CONFIGTABLE AND DCT4_Q15_8192)
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_DCT4_Q15_8192) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_REALCOEF_Q15)

    # For cfft_radix4_init
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_BITREV_1024) 
    target_compile_definitions(${PROJECT} PUBLIC ARM_TABLE_TWIDDLECOEF_Q15_4096)
endif()

endfunction()