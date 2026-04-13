collect (APP_COMMON_SOURCES helper.c)

set (_linker_script "${CMAKE_CURRENT_SOURCE_DIR}/linker_remote.ld")
set_property (GLOBAL PROPERTY APP_LINKER_OPT "-T\"${_linker_script}\"")
set (_linker_large_text_script "${CMAKE_CURRENT_SOURCE_DIR}/linker_large_text.ld")
set_property (GLOBAL PROPERTY APP_LINKER_LARGE_TEXT_OPT "-T\"${_linker_large_text_script}\"")

find_library(LIBXIL_LIB NAMES xil PATHS ${CMAKE_FIND_ROOT_PATH})
get_filename_component(LIBXIL_LIB_DIR ${LIBXIL_LIB} DIRECTORY)
collect(PROJECT_LIB_DIRS ${LIBXIL_LIB_DIR})

collect(PROJECT_LIB_DEPS xil)
collect(PROJECT_LIB_DEPS c)
collect(PROJECT_LIB_DEPS m)
