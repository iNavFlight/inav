cmake_minimum_required (VERSION 3.14)

project(CMSISDSPBasicMath)

include(configLib)
include(configDsp)

file(GLOB SRCF64 "./*_f64.c")
file(GLOB SRCF32 "./*_f32.c")
file(GLOB SRCF16 "./*_f16.c")
file(GLOB SRCQ31 "./*_q31.c")
file(GLOB SRCQ15 "./*_q15.c")
file(GLOB SRCQ7  "./*_q7.c")

file(GLOB SRCU32 "./*_u32.c")
file(GLOB SRCU16 "./*_u16.c")
file(GLOB SRCU8  "./*_u8.c")

add_library(CMSISDSPBasicMath STATIC ${SRCF64})
target_sources(CMSISDSPBasicMath PRIVATE ${SRCF32})

if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
target_sources(CMSISDSPBasicMath PRIVATE ${SRCF16})
endif()

target_sources(CMSISDSPBasicMath PRIVATE ${SRCQ31})
target_sources(CMSISDSPBasicMath PRIVATE ${SRCQ15})
target_sources(CMSISDSPBasicMath PRIVATE ${SRCQ7})

target_sources(CMSISDSPBasicMath PRIVATE ${SRCU32})
target_sources(CMSISDSPBasicMath PRIVATE ${SRCU16})
target_sources(CMSISDSPBasicMath PRIVATE ${SRCU8})

configLib(CMSISDSPBasicMath ${ROOT})
configDsp(CMSISDSPBasicMath ${ROOT})

### Includes
target_include_directories(CMSISDSPBasicMath PUBLIC "${DSP}/Include")



