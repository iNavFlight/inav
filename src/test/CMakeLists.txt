cmake_minimum_required(VERSION 3.10)

project(INAVTests)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/gtest.cmake)

enable_testing()
include(GoogleTest)
add_subdirectory(unit)

add_executable(dshot_direction_test dshot_direction_test.c)
target_include_directories(dshot_direction_test PRIVATE ../main)
target_compile_options(dshot_direction_test PRIVATE -UNDEBUG)
add_test(NAME dshot_direction_test COMMAND dshot_direction_test)
add_dependencies(check dshot_direction_test)
