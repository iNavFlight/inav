cmake_minimum_required (VERSION 3.14)

project(CMSISDSPDistance)

include(configLib)
include(configDsp)

file(GLOB SRC "./*_*.c")

add_library(CMSISDSPDistance STATIC)

target_sources(CMSISDSPDistance PRIVATE arm_boolean_distance.c)
target_sources(CMSISDSPDistance PRIVATE arm_braycurtis_distance_f32.c)
target_sources(CMSISDSPDistance PRIVATE arm_canberra_distance_f32.c)
target_sources(CMSISDSPDistance PRIVATE arm_chebyshev_distance_f32.c)
target_sources(CMSISDSPDistance PRIVATE arm_chebyshev_distance_f64.c)
target_sources(CMSISDSPDistance PRIVATE arm_cityblock_distance_f32.c)
target_sources(CMSISDSPDistance PRIVATE arm_cityblock_distance_f64.c)
target_sources(CMSISDSPDistance PRIVATE arm_correlation_distance_f32.c)
target_sources(CMSISDSPDistance PRIVATE arm_cosine_distance_f32.c)
target_sources(CMSISDSPDistance PRIVATE arm_cosine_distance_f64.c)
target_sources(CMSISDSPDistance PRIVATE arm_dice_distance.c)
target_sources(CMSISDSPDistance PRIVATE arm_euclidean_distance_f32.c)
target_sources(CMSISDSPDistance PRIVATE arm_euclidean_distance_f64.c)
target_sources(CMSISDSPDistance PRIVATE arm_hamming_distance.c)
target_sources(CMSISDSPDistance PRIVATE arm_jaccard_distance.c)
target_sources(CMSISDSPDistance PRIVATE arm_jensenshannon_distance_f32.c)
target_sources(CMSISDSPDistance PRIVATE arm_kulsinski_distance.c)
target_sources(CMSISDSPDistance PRIVATE arm_minkowski_distance_f32.c)
target_sources(CMSISDSPDistance PRIVATE arm_rogerstanimoto_distance.c)
target_sources(CMSISDSPDistance PRIVATE arm_russellrao_distance.c)
target_sources(CMSISDSPDistance PRIVATE arm_sokalmichener_distance.c)
target_sources(CMSISDSPDistance PRIVATE arm_sokalsneath_distance.c)
target_sources(CMSISDSPDistance PRIVATE arm_yule_distance.c)


configLib(CMSISDSPDistance ${ROOT})
configDsp(CMSISDSPDistance ${ROOT})

### Includes
target_include_directories(CMSISDSPDistance PUBLIC "${DSP}/Include")
target_include_directories(CMSISDSPDistance PRIVATE ".")

if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
target_sources(CMSISDSPDistance PRIVATE arm_braycurtis_distance_f16.c)
target_sources(CMSISDSPDistance PRIVATE arm_canberra_distance_f16.c)
target_sources(CMSISDSPDistance PRIVATE arm_chebyshev_distance_f16.c)
target_sources(CMSISDSPDistance PRIVATE arm_cityblock_distance_f16.c)
target_sources(CMSISDSPDistance PRIVATE arm_correlation_distance_f16.c)
target_sources(CMSISDSPDistance PRIVATE arm_cosine_distance_f16.c)
target_sources(CMSISDSPDistance PRIVATE arm_euclidean_distance_f16.c)
target_sources(CMSISDSPDistance PRIVATE arm_jensenshannon_distance_f16.c)
target_sources(CMSISDSPDistance PRIVATE arm_minkowski_distance_f16.c)
endif()

 