set (_cflags "${CMAKE_C_FLAGS} ${APP_EXTRA_C_FLAGS}")
set (_fw_dir "${APPS_SHARE_DIR}")

collector_list (_list PROJECT_INC_DIRS)
collector_list (_app_list APP_INC_DIRS)
include_directories (${_list} ${_app_list} ${CMAKE_CURRENT_SOURCE_DIR})

collector_list (_list PROJECT_LIB_DIRS)
collector_list (_app_list APP_LIB_DIRS)
link_directories (${_list} ${_app_list})

get_property (_linker_opt GLOBAL PROPERTY APP_LINKER_LARGE_TEXT_OPT)
if (NOT _linker_opt)
  get_property (_linker_opt GLOBAL PROPERTY APP_LINKER_OPT)
endif (NOT _linker_opt)
collector_list (_deps PROJECT_LIB_DEPS)

set (OPENAMP_LIB open_amp)

if (${PROJECT_SYSTEM} STREQUAL "linux")
  set (app_list rpc_demod)
else (${PROJECT_SYSTEM})
  set (app_list rpc_demo)
endif (${PROJECT_SYSTEM} STREQUAL "linux")

foreach (_app ${app_list})
  collector_list (_sources APP_COMMON_SOURCES)
  list (APPEND _sources "${CMAKE_CURRENT_SOURCE_DIR}/${_app}.c")

  if (WITH_SHARED_LIB)
    add_executable (${_app}-shared ${_sources})
    target_link_libraries (${_app}-shared ${OPENAMP_LIB}-shared ${_deps})
    install (TARGETS ${_app}-shared RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif (WITH_SHARED_LIB)

  if (WITH_STATIC_LIB)
    if (${PROJECT_SYSTEM} STREQUAL "linux")
      add_executable (${_app}-static ${_sources})
      target_link_libraries (${_app}-static ${OPENAMP_LIB}-static ${_deps})
      install (TARGETS ${_app}-static RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    else (${PROJECT_SYSTEM})
      add_executable (${_app}.out ${_sources})
      set_source_files_properties(${_sources} PROPERTIES COMPILE_FLAGS "${_cflags}")

      target_link_libraries(${_app}.out -Wl,-Map=${_app}.map -Wl,--gc-sections ${_linker_opt} -Wl,--start-group ${OPENAMP_LIB}-static ${_deps} -Wl,--end-group)
      install (TARGETS ${_app}.out RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif (${PROJECT_SYSTEM} STREQUAL "linux" )
  endif (WITH_STATIC_LIB)
endforeach(_app)
