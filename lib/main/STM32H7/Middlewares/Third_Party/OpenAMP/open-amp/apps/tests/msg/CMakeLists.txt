
set (_cflags "${CMAKE_C_FLAGS} ${APP_EXTRA_C_FLAGS} -fdata-sections -ffunction-sections")
set (_fw_dir "${APPS_SHARE_DIR}")

collector_list (_list PROJECT_INC_DIRS)
collector_list (_app_list APP_INC_DIRS)
include_directories (${_list} ${_app_list} ${CMAKE_CURRENT_SOURCE_DIR})

collector_list (_list PROJECT_LIB_DIRS)
collector_list (_app_list APP_LIB_DIRS)
link_directories (${_list} ${_app_list})

get_property (_linker_opt GLOBAL PROPERTY APP_LINKER_OPT)
collector_list (_deps PROJECT_LIB_DEPS)

set (OPENAMP_LIB open_amp)

foreach (_app msg-test-rpmsg-ping msg-test-rpmsg-update msg-test-rpmsg-flood-ping)
  collector_list (_sources APP_COMMON_SOURCES)
  if (${_app} STREQUAL "msg-test-rpmsg-ping")
    list (APPEND _sources "${CMAKE_CURRENT_SOURCE_DIR}/rpmsg-ping.c")
  elseif (${_app} STREQUAL "msg-test-rpmsg-update")
    list (APPEND _sources "${CMAKE_CURRENT_SOURCE_DIR}/rpmsg-update.c")
  elseif (${_app} STREQUAL "msg-test-rpmsg-flood-ping")
    list (APPEND _sources "${CMAKE_CURRENT_SOURCE_DIR}/rpmsg-flood-ping.c")
  endif (${_app} STREQUAL "msg-test-rpmsg-ping")

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
