cmake_minimum_required(VERSION 3.17)

set(TOOLCHAIN_OPTIONS "arm-none-eabi")
set(TOOLCHAIN "arm-none-eabi" CACHE STRING "Toolchain to use. Available: ${TOOLCHAIN_OPTIONS}")
set_property(CACHE TOOLCHAIN PROPERTY STRINGS ${TOOLCHAIN_OPTIONS})
if (NOT ${TOOLCHAIN} IN_LIST TOOLCHAIN_OPTIONS)
    message(FATAL_ERROR "Invalid toolchain ${TOOLCHAIN}")
endif()

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${TOOLCHAIN}.cmake")

project(INAV VERSION 2.5.0)

ENABLE_LANGUAGE(ASM)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_program(RUBY_EXECUTABLE ruby)
if (NOT RUBY_EXECUTABLE)
    message(FATAL_ERROR "Could not find ruby")
endif()

execute_process(COMMAND git rev-parse --short HEAD
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE NO_GIT_HASH
    OUTPUT_VARIABLE GIT_SHORT_HASH)
if (NO_GIT_HASH)
    message(FATAL_ERROR "Could not find git revision. Is git installed?")
endif()

set(INAV_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(INAV_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")
set(INAV_UTILS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/utils")
set(INAV_MAIN_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/main")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(FIRMWARE_VERSION ${PROJECT_VERSION})

include(settings)
include(inav)
include(stm32)

add_subdirectory(src)

collect_targets()

message("-- Build type: ${CMAKE_BUILD_TYPE}")
