set(gcc_host_req_ver 10)

execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE version)

    message("-- found GCC ${version}")
if (NOT gcc_host_req_ver STREQUAL version)
    message(FATAL_ERROR "-- expecting GCC version ${gcc_host_req_ver}, but got version ${version} instead")
endif()
