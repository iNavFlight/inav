cmake_minimum_required (VERSION 3.14)

project(CMSISDSPInterpolation)

include(configLib)
include(configDsp)


add_library(CMSISDSPInterpolation STATIC)

target_sources(CMSISDSPInterpolation PRIVATE arm_bilinear_interp_f32.c)
target_sources(CMSISDSPInterpolation PRIVATE arm_bilinear_interp_q15.c)
target_sources(CMSISDSPInterpolation PRIVATE arm_bilinear_interp_q31.c)
target_sources(CMSISDSPInterpolation PRIVATE arm_bilinear_interp_q7.c)
target_sources(CMSISDSPInterpolation PRIVATE arm_linear_interp_f32.c)
target_sources(CMSISDSPInterpolation PRIVATE arm_linear_interp_q15.c)
target_sources(CMSISDSPInterpolation PRIVATE arm_linear_interp_q31.c)
target_sources(CMSISDSPInterpolation PRIVATE arm_linear_interp_q7.c)
target_sources(CMSISDSPInterpolation PRIVATE arm_spline_interp_f32.c)
target_sources(CMSISDSPInterpolation PRIVATE arm_spline_interp_init_f32.c)


configLib(CMSISDSPInterpolation ${ROOT})
configDsp(CMSISDSPInterpolation ${ROOT})

### Includes
target_include_directories(CMSISDSPInterpolation PUBLIC "${DSP}/Include")



if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
target_sources(CMSISDSPInterpolation PRIVATE arm_bilinear_interp_f16.c)
target_sources(CMSISDSPInterpolation PRIVATE arm_linear_interp_f16.c)
endif()