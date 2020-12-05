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
