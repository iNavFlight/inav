cmake_minimum_required(VERSION 3.10)

project(INAVTests)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GCC/Clang on Windows default to the Microsoft bit-field allocation layout, but
# INAV sources assume the ARM/Linux ABI (e.g. the STATIC_ASSERT on the size of
# packed protocol structs). Ask the compiler for that layout explicitly.
if(WIN32 AND CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-mno-ms-bitfields)
endif()

include(cmake/gtest.cmake)

enable_testing()
include(GoogleTest)
add_subdirectory(unit)
