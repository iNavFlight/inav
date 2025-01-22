
option (WITH_LOAD_FW "Include loading firmware example" OFF)

add_subdirectory (echo)
add_subdirectory (rpmsg_sample_echo)
add_subdirectory (matrix_multiply)
if (WITH_LOAD_FW)
  add_subdirectory (load_fw)
endif (WITH_LOAD_FW)
if (WITH_PROXY_APPS)
add_subdirectory (rpc_demo)
endif (WITH_PROXY_APPS)
