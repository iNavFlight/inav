collect (PROJECT_LIB_TESTS helper.c)

set (_test_lib_external "xil")

collect (PROJECT_LIB_DEPS ${_test_lib_external})
collect (PROJECT_LIB_DEPS "c")
collect (PROJECT_LIB_DEPS "m")

find_library (_ext_lib_path ${_test_lib_external})
if (NOT _ext_lib_path)
  message ( "external library ${_test_lib_external} not found" )
  message ( "hint: you may need to pass -DCMAKE_LIBRARY_PATH=<path>" )
  message ( FATAL_ERROR "library ${_test_lib_external} is required to build the tests" )
endif (NOT _ext_lib_path)
get_filename_component (_ext_lib_path ${_ext_lib_path} DIRECTORY)
collect (PROJECT_LIB_DIRS ${_ext_lib_path})

set_property (GLOBAL PROPERTY TEST_LINKER_OPTIONS "-Wl,--gc-sections -Wl,-build-id=none -specs=${CMAKE_CURRENT_SOURCE_DIR}/Xilinx.spec -T\"${CMAKE_CURRENT_SOURCE_DIR}/lscript.ld\"")

# vim: expandtab:ts=2:sw=2:smartindent
