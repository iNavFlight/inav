cmake_minimum_required (VERSION 3.14)

project(CMSISDSPMatrix)

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

add_library(CMSISDSPMatrix STATIC ${SRCF64})
target_sources(CMSISDSPMatrix PRIVATE ${SRCF32})

if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
target_sources(CMSISDSPMatrix PRIVATE ${SRCF16})
endif()

target_sources(CMSISDSPMatrix PRIVATE ${SRCQ31})
target_sources(CMSISDSPMatrix PRIVATE ${SRCQ15})
target_sources(CMSISDSPMatrix PRIVATE ${SRCQ7})

target_sources(CMSISDSPMatrix PRIVATE ${SRCU32})
target_sources(CMSISDSPMatrix PRIVATE ${SRCU16})
target_sources(CMSISDSPMatrix PRIVATE ${SRCU8})

configLib(CMSISDSPMatrix ${ROOT})
configDsp(CMSISDSPMatrix ${ROOT})

### Includes
target_include_directories(CMSISDSPMatrix PUBLIC "${DSP}/Include")


if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
target_sources(CMSISDSPMatrix PRIVATE arm_mat_add_f16.c)
target_sources(CMSISDSPMatrix PRIVATE arm_mat_sub_f16.c)
target_sources(CMSISDSPMatrix PRIVATE arm_mat_trans_f16.c)
target_sources(CMSISDSPMatrix PRIVATE arm_mat_scale_f16.c)
target_sources(CMSISDSPMatrix PRIVATE arm_mat_mult_f16.c)
target_sources(CMSISDSPMatrix PRIVATE arm_mat_vec_mult_f16.c)
target_sources(CMSISDSPMatrix PRIVATE arm_mat_cmplx_trans_f16.c)
target_sources(CMSISDSPMatrix PRIVATE arm_mat_cmplx_mult_f16.c)
target_sources(CMSISDSPMatrix PRIVATE arm_mat_inverse_f16.c)
target_sources(CMSISDSPMatrix PRIVATE arm_mat_init_f16.c)
target_sources(CMSISDSPMatrix PRIVATE arm_mat_cholesky_f16.c)

endif()
