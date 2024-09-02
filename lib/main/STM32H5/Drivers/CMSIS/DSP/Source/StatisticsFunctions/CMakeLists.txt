cmake_minimum_required (VERSION 3.14)

project(CMSISDSPStatistics)

include(configLib)
include(configDsp)



add_library(CMSISDSPStatistics STATIC)

target_sources(CMSISDSPStatistics PRIVATE arm_entropy_f32.c)
target_sources(CMSISDSPStatistics PRIVATE arm_entropy_f64.c)
target_sources(CMSISDSPStatistics PRIVATE arm_kullback_leibler_f32.c)
target_sources(CMSISDSPStatistics PRIVATE arm_kullback_leibler_f64.c)
target_sources(CMSISDSPStatistics PRIVATE arm_logsumexp_dot_prod_f32.c)
target_sources(CMSISDSPStatistics PRIVATE arm_logsumexp_f32.c)
target_sources(CMSISDSPStatistics PRIVATE arm_max_f32.c)
target_sources(CMSISDSPStatistics PRIVATE arm_max_no_idx_f32.c)
target_sources(CMSISDSPStatistics PRIVATE arm_max_q15.c)
target_sources(CMSISDSPStatistics PRIVATE arm_max_q31.c)
target_sources(CMSISDSPStatistics PRIVATE arm_max_q7.c)
target_sources(CMSISDSPStatistics PRIVATE arm_mean_f32.c)
target_sources(CMSISDSPStatistics PRIVATE arm_mean_q15.c)
target_sources(CMSISDSPStatistics PRIVATE arm_mean_q31.c)
target_sources(CMSISDSPStatistics PRIVATE arm_mean_q7.c)
target_sources(CMSISDSPStatistics PRIVATE arm_min_f32.c)
target_sources(CMSISDSPStatistics PRIVATE arm_min_q15.c)
target_sources(CMSISDSPStatistics PRIVATE arm_min_q31.c)
target_sources(CMSISDSPStatistics PRIVATE arm_min_q7.c)
target_sources(CMSISDSPStatistics PRIVATE arm_power_f32.c)
target_sources(CMSISDSPStatistics PRIVATE arm_power_q15.c)
target_sources(CMSISDSPStatistics PRIVATE arm_power_q31.c)
target_sources(CMSISDSPStatistics PRIVATE arm_power_q7.c)
target_sources(CMSISDSPStatistics PRIVATE arm_rms_f32.c)
target_sources(CMSISDSPStatistics PRIVATE arm_rms_q15.c)
target_sources(CMSISDSPStatistics PRIVATE arm_rms_q31.c)
target_sources(CMSISDSPStatistics PRIVATE arm_std_f32.c)
target_sources(CMSISDSPStatistics PRIVATE arm_std_q15.c)
target_sources(CMSISDSPStatistics PRIVATE arm_std_q31.c)
target_sources(CMSISDSPStatistics PRIVATE arm_var_f32.c)
target_sources(CMSISDSPStatistics PRIVATE arm_var_q15.c)
target_sources(CMSISDSPStatistics PRIVATE arm_var_q31.c)

target_sources(CMSISDSPStatistics PRIVATE arm_absmax_f32.c)
target_sources(CMSISDSPStatistics PRIVATE arm_absmax_q15.c)
target_sources(CMSISDSPStatistics PRIVATE arm_absmax_q31.c)
target_sources(CMSISDSPStatistics PRIVATE arm_absmax_q7.c)

target_sources(CMSISDSPStatistics PRIVATE arm_absmin_f32.c)
target_sources(CMSISDSPStatistics PRIVATE arm_absmin_q15.c)
target_sources(CMSISDSPStatistics PRIVATE arm_absmin_q31.c)
target_sources(CMSISDSPStatistics PRIVATE arm_absmin_q7.c)

configLib(CMSISDSPStatistics ${ROOT})
configDsp(CMSISDSPStatistics ${ROOT})

### Includes
target_include_directories(CMSISDSPStatistics PUBLIC "${DSP}/Include")



if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
target_sources(CMSISDSPStatistics PRIVATE arm_max_f16.c)
target_sources(CMSISDSPStatistics PRIVATE arm_min_f16.c)
target_sources(CMSISDSPStatistics PRIVATE arm_mean_f16.c)
target_sources(CMSISDSPStatistics PRIVATE arm_power_f16.c)
target_sources(CMSISDSPStatistics PRIVATE arm_rms_f16.c)
target_sources(CMSISDSPStatistics PRIVATE arm_std_f16.c)
target_sources(CMSISDSPStatistics PRIVATE arm_var_f16.c)
target_sources(CMSISDSPStatistics PRIVATE arm_entropy_f16.c)
target_sources(CMSISDSPStatistics PRIVATE arm_kullback_leibler_f16.c)
target_sources(CMSISDSPStatistics PRIVATE arm_logsumexp_dot_prod_f16.c)
target_sources(CMSISDSPStatistics PRIVATE arm_logsumexp_f16.c)
target_sources(CMSISDSPStatistics PRIVATE arm_max_no_idx_f16.c)

target_sources(CMSISDSPStatistics PRIVATE arm_absmax_f16.c)
target_sources(CMSISDSPStatistics PRIVATE arm_absmin_f16.c)

endif()