cmake_minimum_required (VERSION 3.14)

project(CMSISDSPQuaternionMath)

include(configLib)
include(configDsp)



add_library(CMSISDSPQuaternionMath STATIC arm_quaternion_norm_f32.c)
target_sources(CMSISDSPQuaternionMath PRIVATE arm_quaternion_inverse_f32.c)
target_sources(CMSISDSPQuaternionMath PRIVATE arm_quaternion_conjugate_f32.c)
target_sources(CMSISDSPQuaternionMath PRIVATE arm_quaternion_normalize_f32.c)
target_sources(CMSISDSPQuaternionMath PRIVATE arm_quaternion_product_single_f32.c)
target_sources(CMSISDSPQuaternionMath PRIVATE arm_quaternion_product_f32.c)
target_sources(CMSISDSPQuaternionMath PRIVATE arm_quaternion2rotation_f32.c)
target_sources(CMSISDSPQuaternionMath PRIVATE arm_rotation2quaternion_f32.c)


if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
endif()


configLib(CMSISDSPQuaternionMath ${ROOT})
configDsp(CMSISDSPQuaternionMath ${ROOT})

### Includes
target_include_directories(CMSISDSPQuaternionMath PUBLIC "${DSP}/Include")



