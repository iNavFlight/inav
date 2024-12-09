cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Set up the project
project(levelx
    LANGUAGES C ASM
)

option(LX_STANDALONE_ENABLE "Enable LevelX in standalone mode" OFF)
option(LX_ENABLE_FILE_SERVERS "Includes a dependency on FileX" ON)

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
if(NOT LX_STANDALONE_ENABLE)
    target_link_libraries(${PROJECT_NAME} PUBLIC 
        "azrtos::threadx")
endif()

if(LX_ENABLE_FILE_SERVERS)
    message(STATUS "LX_ENABLE_FILE_SERVERS - defined")
    target_link_libraries(${PROJECT_NAME} PUBLIC "azrtos::filex")
endif()

# A place for generated/copied include files (no need to change)
set(CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/custom_inc)

# Pick up the common stuff
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/common)

# Include the user's override file if required
if (NOT LX_USER_FILE)
    message(STATUS "Using default lx_user.h file")
    set(LX_USER_FILE ${CMAKE_CURRENT_LIST_DIR}/common/inc/lx_user_sample.h)
else()
    message(STATUS "Using custom lx_user.h file from ${LX_USER_FILE}")
endif()
configure_file(${LX_USER_FILE} ${CUSTOM_INC_DIR}/lx_user.h COPYONLY)
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
    ${CUSTOM_INC_DIR}
)

if(NOT LX_STANDALONE_ENABLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC "LX_INCLUDE_USER_DEFINE_FILE" )
else()
    # Enable LevelX and FileX standalone support (No Azure RTOS support)
    set(FX_STANDALONE_ENABLE ON CACHE BOOL "Standalone enable")
    target_compile_definitions(${PROJECT_NAME} PUBLIC "LX_INCLUDE_USER_DEFINE_FILE" -DLX_STANDALONE_ENABLE -DFX_STANDALONE_ENABLE)
endif()

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

