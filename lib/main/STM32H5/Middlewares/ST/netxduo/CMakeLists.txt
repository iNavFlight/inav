cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Set up the project
project(netxduo
    LANGUAGES C ASM
)    

option(NXD_ENABLE_FILE_SERVERS "Includes a dependency on FileX to support 'server' protocol handlers" ON)
option(NXD_ENABLE_AZURE_IOT "Enable Azure IoT" OFF)
option(NX_AZURE_DISABLE_IOT_SECURITY_MODULE "Disable Azure IoT Security Module" OFF)

if(NOT DEFINED THREADX_ARCH)
    message(FATAL_ERROR "Error: THREADX_ARCH not defined")
endif()
if(NOT DEFINED THREADX_TOOLCHAIN)
    message(FATAL_ERROR "Error: THREADX_TOOLCHAIN not defined")
endif()

# Define our target library and an alias for consumers
add_library(${PROJECT_NAME})
add_library("azrtos::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

# Define any required dependencies between this library and others
target_link_libraries(${PROJECT_NAME} PUBLIC "azrtos::threadx")

if(NXD_ENABLE_FILE_SERVERS)
    message(STATUS "NXD_ENABLE_FILE_SERVERS - defined")
    target_link_libraries(${PROJECT_NAME} PUBLIC "azrtos::filex")
endif()

if(NXD_ENABLE_AZURE_IOT)
    message(STATUS "NXD_ENABLE_AZURE_IOT - defined")
    target_link_libraries(${PROJECT_NAME} PUBLIC az_iot_hub az_iot_provisioning)
    if(NOT NX_AZURE_DISABLE_IOT_SECURITY_MODULE)
        target_link_libraries(${PROJECT_NAME} PUBLIC iot_security_module)
    endif()
endif()

# A place for generated/copied include files (no need to change)
set(CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/custom_inc)

# Pick up the port specific stuff first
if(DEFINED NETXDUO_CUSTOM_PORT)
    add_subdirectory(${NETXDUO_CUSTOM_PORT} netxduo_port)
else()
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ports/${THREADX_ARCH}/${THREADX_TOOLCHAIN})
endif()

# Then the common files
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/common)

# Pick up the apps directory containing the protocol and app-layer components
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/addons)

# Network security and crypto components
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/crypto_libraries)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/nx_secure)

# Utility components
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/utility)

# If the user provided an override, copy it to the custom directory
if (NOT NX_USER_FILE)
    message(STATUS "Using default nx_user.h file")
    set(NX_USER_FILE ${CMAKE_CURRENT_LIST_DIR}/common/inc/nx_user_sample.h)
else()
    message(STATUS "Using custom nx_user.h file from ${NX_USER_FILE}")
endif()
configure_file(${NX_USER_FILE} ${CUSTOM_INC_DIR}/nx_user.h COPYONLY)
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
    ${CUSTOM_INC_DIR}
)
target_compile_definitions(${PROJECT_NAME} PUBLIC "NX_INCLUDE_USER_DEFINE_FILE" )

# Enable a build target that produces a ZIP file of all sources
set(CPACK_SOURCE_GENERATOR "ZIP")
set(CPACK_SOURCE_IGNORE_FILES
  \\.git/
  \\.github/
  _build/
  \\.git
  \\.gitattributes
  \\.gitignore
  ".*~$"
)
set(CPACK_VERBATIM_VARIABLES YES)
include(CPack)