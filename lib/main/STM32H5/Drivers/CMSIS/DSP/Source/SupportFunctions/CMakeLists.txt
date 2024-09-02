cmake_minimum_required (VERSION 3.14)

project(CMSISDSPSupport)

include(configLib)
include(configDsp)

file(GLOB SRC "./*_*.c")

add_library(CMSISDSPSupport STATIC ${SRC})

configLib(CMSISDSPSupport ${ROOT})
configDsp(CMSISDSPSupport ${ROOT})

### Includes
target_include_directories(CMSISDSPSupport PUBLIC "${DSP}/Include")

if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
target_sources(CMSISDSPSupport PRIVATE arm_copy_f16.c)
target_sources(CMSISDSPSupport PRIVATE arm_fill_f16.c)
target_sources(CMSISDSPSupport PRIVATE arm_f16_to_q15.c)
target_sources(CMSISDSPSupport PRIVATE arm_q15_to_f16.c)
target_sources(CMSISDSPSupport PRIVATE arm_float_to_f16.c)
target_sources(CMSISDSPSupport PRIVATE arm_f16_to_float.c)
target_sources(CMSISDSPSupport PRIVATE arm_weighted_sum_f16.c)
target_sources(CMSISDSPSupport PRIVATE arm_barycenter_f16.c)
endif()

