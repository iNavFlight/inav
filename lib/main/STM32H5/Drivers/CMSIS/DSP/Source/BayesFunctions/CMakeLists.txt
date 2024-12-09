cmake_minimum_required (VERSION 3.14)

project(CMSISDSPBayes)

include(configLib)
include(configDsp)

file(GLOB SRC "./*_*.c")

add_library(CMSISDSPBayes STATIC)

target_sources(CMSISDSPBayes PRIVATE arm_gaussian_naive_bayes_predict_f32.c)

configLib(CMSISDSPBayes ${ROOT})
configDsp(CMSISDSPBayes ${ROOT})

### Includes
target_include_directories(CMSISDSPBayes PUBLIC "${DSP}/Include")

if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
target_sources(CMSISDSPBayes PRIVATE arm_gaussian_naive_bayes_predict_f16.c)
endif()

