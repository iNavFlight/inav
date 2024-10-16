

include("${CMAKE_CURRENT_LIST_DIR}/asc_security_coreTargets.cmake")

get_target_property(asc_security_core_INCLUDES asc_security_core INTERFACE_INCLUDE_DIRECTORIES)

set(asc_security_core_INCLUDES ${asc_security_core_INCLUDES} CACHE INTERNAL "")
