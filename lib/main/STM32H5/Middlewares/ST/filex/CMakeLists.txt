cmake_minimum_required(VERSION 3.0.0 FATAL_ERROR)

# Set up the project
project(filex
    LANGUAGES C ASM
)


option(FX_STANDALONE_ENABLE "Enable Filex in standalone mode" OFF)

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
if(NOT FX_STANDALONE_ENABLE)
    target_link_libraries(${PROJECT_NAME} PUBLIC 
        "azrtos::threadx"
    )
endif()

# A place for generated/copied include files (no need to change)
set(CUSTOM_INC_DIR ${CMAKE_CURRENT_BINARY_DIR}/custom_inc)

# Pick up the port specific stuff first
if(DEFINED FILEX_CUSTOM_PORT)
    add_subdirectory(${FILEX_CUSTOM_PORT} filex_port)
else()
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/ports/${THREADX_ARCH}/${THREADX_TOOLCHAIN})
endif()

# Then the common files
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/common)




# Include the user's override file if required
if (NOT FX_USER_FILE)
message(STATUS "Using default fx_user.h file")
set(FX_USER_FILE ${CMAKE_CURRENT_LIST_DIR}/common/inc/fx_user_sample.h)
else()
    message(STATUS "Using custom fx_user.h file from ${FX_USER_FILE}")
endif()
configure_file(${FX_USER_FILE} ${CUSTOM_INC_DIR}/fx_user.h COPYONLY)
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
    ${CUSTOM_INC_DIR}
)

if(NOT FX_STANDALONE_ENABLE)
    target_compile_definitions(${PROJECT_NAME} PUBLIC "FX_INCLUDE_USER_DEFINE_FILE" )
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC "FX_INCLUDE_USER_DEFINE_FILE" -DFX_STANDALONE_ENABLE)
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

