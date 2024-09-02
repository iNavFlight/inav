# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.10)

project(az_platform LANGUAGES C)
set(CMAKE_C_STANDARD 99)

if(AZ_PLATFORM_IMPL STREQUAL "WIN32")
  # build windows platform
  add_library(az_win32 STATIC
    ${CMAKE_CURRENT_LIST_DIR}/az_win32.c
  )

  target_link_libraries(az_win32
    PRIVATE
      az_core
  )
elseif(AZ_PLATFORM_IMPL STREQUAL "POSIX")
  # build linux platform
  add_library(az_posix STATIC
      ${CMAKE_CURRENT_LIST_DIR}/az_posix.c
  )

  target_link_libraries(az_posix
    PRIVATE
      az_core
  )
else()
  #noplatform
  add_library(az_noplatform STATIC
    ${CMAKE_CURRENT_LIST_DIR}/az_noplatform.c
  )

  target_link_libraries(az_noplatform
    PRIVATE
      az_core
  )
endif()

# HTTP Client
add_library (
  az_nohttp
    STATIC
      ${CMAKE_CURRENT_LIST_DIR}/az_nohttp.c
)

target_link_libraries(az_nohttp PRIVATE az_core)

# make sure that users can consume the project as a library.
add_library (az::nohttp ALIAS az_nohttp)

# Curl Platform
if (TRANSPORT_CURL)
  set(CURL_MIN_REQUIRED_VERSION 7.1) #Min curl version to support CURLOPT_HTTPHEADER option
  find_package(CURL ${CURL_MIN_REQUIRED_VERSION} CONFIG)
  if(NOT CURL_FOUND)
    find_package(CURL ${CURL_MIN_REQUIRED_VERSION} REQUIRED)
  endif()

  add_library (
    az_curl
      STATIC
      ${CMAKE_CURRENT_LIST_DIR}/az_curl.c
  )

  target_link_libraries(az_curl PRIVATE az_core)

  # make sure that users can consume the project as a library.
  add_library (az::curl ALIAS az_curl)

  target_link_libraries(az_curl PUBLIC CURL::libcurl)
  target_include_directories(az_curl INTERFACE ${CURL_INCLUDE_DIR})

endif()
