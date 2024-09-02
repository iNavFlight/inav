# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.10)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake-modules")

option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)
option(TRANSPORT_CURL "Build internal http transport implementation with CURL for HTTP Pipeline" OFF)
option(UNIT_TESTING "Build unit test projects" OFF)
option(UNIT_TESTING_MOCKS "wrap PAL functions with mock implementation for tests" OFF)
option(TRANSPORT_PAHO "Build IoT Samples with Paho MQTT support" OFF)
option(PRECONDITIONS "Build SDK with preconditions enabled" ON)
option(LOGGING "Build SDK with logging support" ON)
option(ADDRESS_SANITIZER "Build with address sanitizer" OFF)

# vcpkg integration
include(AzureVcpkg)
az_vcpkg_integrate()

# disable preconditions when it's set to OFF
if (NOT PRECONDITIONS)
  add_compile_definitions(AZ_NO_PRECONDITION_CHECKING)
endif()

if (NOT LOGGING)
  add_compile_definitions(AZ_NO_LOGGING)
endif()

# enable mock functions with link option -ld
if(UNIT_TESTING_MOCKS)
  add_compile_definitions(_az_MOCK_ENABLED)
endif()

# make libcurl option enabled to be visible to code
if(TRANSPORT_CURL)
  add_compile_definitions(TRANSPORT_CURL)
endif()

project(az LANGUAGES C)
enable_testing ()

include(eng/cmake/global_compile_options.cmake)
include(create_map_file)

# Include function for creating code coverage targets
include(CreateCodeCoverageTargets)

# List of projects that generate coverage
# This write empty makes sure that if file is already there, we replace it for an empty one
# Then each project will APPEND to this file
# At the end of cmake generate, this file will list the targets for code cov
file(WRITE ${CMAKE_BINARY_DIR}/coverage_targets.txt "")

# Determine platform to build
if(AZ_PLATFORM_IMPL STREQUAL "WIN32")
  set(PAL az_win32)
elseif(AZ_PLATFORM_IMPL STREQUAL "POSIX")
  set(PAL az_posix)
elseif(AZ_PLATFORM_IMPL STREQUAL "CUSTOM")
  if(NOT DEFINED AZ_CUSTOM_PLATFORM_IMPL_NAME)
    message(FATAL_ERROR "When using AZ_PLATFORM_IMPL=CUSTOM, AZ_CUSTOM_PLATFORM_IMPL_NAME must be defined as well. See Readme.")
  endif()
  set(PAL ${AZ_CUSTOM_PLATFORM_IMPL_NAME})
else()
  #noplatform
  set(PAL az_noplatform)
endif()

add_subdirectory(sdk/src/azure/core)

# SDK Clients
add_subdirectory(sdk/src/azure/iot)

#PAL (Hardware + HTTP)
add_subdirectory(sdk/src/azure/platform)

# User can disable samples generation by setting env variable AZ_SDK_C_NO_SAMPLES
if(NOT DEFINED ENV{AZ_SDK_C_NO_SAMPLES})
  if(TRANSPORT_PAHO)
    add_subdirectory(sdk/samples/iot)
  endif()
endif()

# default for Unit testing with cmocka is OFF, however, this will be ON on CI and tests must
# pass before committing changes
if (UNIT_TESTING)
  # make generate step fail if cmocka dependency is not found
  find_package(cmocka CONFIG REQUIRED)

  # Old versions of cmocka (including the latest stable) don't define an imported target
  if(NOT TARGET cmocka::cmocka)
    set(CMOCKA_LIB ${CMOCKA_LIBRARIES})
  else()
    set(CMOCKA_LIB cmocka::cmocka)
  endif()

  # for gcc, we need to add no-clobbered compile opt to avoid warning about set-jump function
  set(NO_CLOBBERED_WARNING "")
  if (CMAKE_C_COMPILER_ID MATCHES "GNU")
    set(NO_CLOBBERED_WARNING "-Wno-clobbered")
  endif()

  # Core
  add_subdirectory(sdk/tests/core)

  # IoT
  add_subdirectory(sdk/tests/iot/adu)
  add_subdirectory(sdk/tests/iot/common)
  add_subdirectory(sdk/tests/iot/hub)
  add_subdirectory(sdk/tests/iot/provisioning)

endif()

# Fail generation when setting MOCKS ON without GCC
if(UNIT_TESTING_MOCKS)
  if(UNIT_TESTING)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "GNU")
      # Fail generation when asking for MOCK without GCC
      message(FATAL_ERROR "Unsupported Compiler Option.\nOption `UNIT_TESTING_MOCKS` is not supported on ${CMAKE_C_COMPILER_ID}. It is only supported by GNU gcc.\nRemove option `UNIT_TESTING_MOCKS` to build tests without using MOCK.")
    endif()
  else()
    # Mention MOCK is ignored
    message(WARNING "UNIT_TESTING_MOCKS will be ignored because UNIT_TESTING is not enabled.")
  endif()
endif()
