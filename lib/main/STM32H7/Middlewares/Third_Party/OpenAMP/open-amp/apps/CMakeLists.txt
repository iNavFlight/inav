collector_create (APP_COMMON_SOURCES "")
collector_create (APP_LIB_DIRS "")
collector_create (APP_INC_DIRS "")
collector_create (APP_LIB_DEPS "")

set (APPS_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

set (APPS_SHARE_DIR "${CMAKE_CURRENT_BINARY_DIR}/share")

add_subdirectory (machine)
add_subdirectory (system)
add_subdirectory (tests)
add_subdirectory (examples)
