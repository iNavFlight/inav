cmake_minimum_required (VERSION 3.14)

project(CMSISDSPSVM)

include(configLib)
include(configDsp)


add_library(CMSISDSPSVM STATIC)

target_sources(CMSISDSPSVM PRIVATE arm_svm_linear_init_f32.c) 
target_sources(CMSISDSPSVM PRIVATE arm_svm_rbf_init_f32.c)
target_sources(CMSISDSPSVM PRIVATE arm_svm_linear_predict_f32.c)
target_sources(CMSISDSPSVM PRIVATE arm_svm_rbf_predict_f32.c)
target_sources(CMSISDSPSVM PRIVATE arm_svm_polynomial_init_f32.c)
target_sources(CMSISDSPSVM PRIVATE arm_svm_sigmoid_init_f32.c)
target_sources(CMSISDSPSVM PRIVATE arm_svm_polynomial_predict_f32.c)  
target_sources(CMSISDSPSVM PRIVATE arm_svm_sigmoid_predict_f32.c)



configLib(CMSISDSPSVM ${ROOT})
configDsp(CMSISDSPSVM ${ROOT})

### Includes
target_include_directories(CMSISDSPSVM PUBLIC "${DSP}/Include")

if ((NOT ARMAC5) AND (NOT DISABLEFLOAT16))
target_sources(CMSISDSPSVM PRIVATE arm_svm_linear_init_f16.c) 
target_sources(CMSISDSPSVM PRIVATE arm_svm_rbf_init_f16.c)
target_sources(CMSISDSPSVM PRIVATE arm_svm_linear_predict_f16.c)
target_sources(CMSISDSPSVM PRIVATE arm_svm_rbf_predict_f16.c)
target_sources(CMSISDSPSVM PRIVATE arm_svm_polynomial_init_f16.c)
target_sources(CMSISDSPSVM PRIVATE arm_svm_sigmoid_init_f16.c)
target_sources(CMSISDSPSVM PRIVATE arm_svm_polynomial_predict_f16.c)  
target_sources(CMSISDSPSVM PRIVATE arm_svm_sigmoid_predict_f16.c)
endif()




