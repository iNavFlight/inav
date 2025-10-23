collect (APP_COMMON_SOURCES helper.c)

set (_linker_script "${CMAKE_CURRENT_SOURCE_DIR}/linker_remote.ld")

set_property (GLOBAL PROPERTY APP_LINKER_OPT "-Wl,-build-id=none -specs=${CMAKE_CURRENT_SOURCE_DIR}/Xilinx.spec -T\"${_linker_script}\"")

find_library(LIBXIL_LIB NAMES xil PATHS ${CMAKE_FIND_ROOT_PATH})
get_filename_component(LIBXIL_LIB_DIR ${LIBXIL_LIB} DIRECTORY)
collect(PROJECT_LIB_DIRS ${LIBXIL_LIB_DIR})

collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS c)
collect(PROJECT_LIB_DEPS m)
