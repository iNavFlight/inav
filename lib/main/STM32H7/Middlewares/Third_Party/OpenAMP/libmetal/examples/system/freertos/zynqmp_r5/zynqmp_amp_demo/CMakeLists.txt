collector_list  (_list PROJECT_INC_DIRS)
include_directories (${_list} ${CMAKE_CURRENT_SOURCE_DIR})

collector_list  (_list PROJECT_LIB_DIRS)
link_directories (${_list})

collector_list (_deps PROJECT_LIB_DEPS)

set (_linker_script ${CMAKE_CURRENT_SOURCE_DIR}/lscript.ld)

set (_src_common ${CMAKE_CURRENT_SOURCE_DIR}/init_${PROJECT_SYSTEM}.c)
set (_app0 libmetal_amp_demod)
set (_src0 ${CMAKE_CURRENT_SOURCE_DIR}/${_app0}.c)
list(APPEND _src0 ${CMAKE_CURRENT_SOURCE_DIR}/sys_init.c)
list(APPEND _src0 ${CMAKE_CURRENT_SOURCE_DIR}/shmem_demod.c)
list(APPEND _src0 ${CMAKE_CURRENT_SOURCE_DIR}/shmem_atomic_demod.c)
list(APPEND _src0 ${CMAKE_CURRENT_SOURCE_DIR}/ipi_shmem_demod.c)
list(APPEND _src0 ${CMAKE_CURRENT_SOURCE_DIR}/ipi_latency_demod.c)
list(APPEND _src0 ${CMAKE_CURRENT_SOURCE_DIR}/shmem_latency_demod.c)
list(APPEND _src0 ${CMAKE_CURRENT_SOURCE_DIR}/shmem_throughput_demod.c)

get_property (_linker_options GLOBAL PROPERTY TEST_LINKER_OPTIONS)
add_executable (${_app0}.elf ${_src0})
if (PROJECT_EC_FLAGS)
  string(REPLACE " " ";" _ec_flgs ${PROJECT_EC_FLAGS})
  target_compile_options (${_app0}.elf PUBLIC ${_ec_flgs})
endif (PROJECT_EC_FLAGS)
target_link_libraries(${_app0}.elf -Wl,-Map=${_app0}.map -Wl,--gc-sections -T\"${_linker_script}\" -Wl,--start-group ${_deps} -Wl,--end-group)
install (TARGETS ${_app0}.elf RUNTIME DESTINATION bin)

# vim: expandtab:ts=2:sw=2:smartindent
