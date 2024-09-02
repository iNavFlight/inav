# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.10)

project (az_iot LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include(CheckAndIncludeCodeCov)

# Azure IoT Common Library
add_library (az_iot_common
  ${CMAKE_CURRENT_LIST_DIR}/az_iot_common.c
)

target_include_directories (az_iot_common
  PUBLIC
    ${az_SOURCE_DIR}/sdk/inc
)

target_link_libraries(az_iot_common
  PUBLIC
    az::core
)

add_library (az::iot::common ALIAS az_iot_common)

# Azure IoT Hub Library
add_library (az_iot_hub
  ${CMAKE_CURRENT_LIST_DIR}/az_iot_adu_client.c
  ${CMAKE_CURRENT_LIST_DIR}/az_iot_hub_client.c
  ${CMAKE_CURRENT_LIST_DIR}/az_iot_hub_client_sas.c
  ${CMAKE_CURRENT_LIST_DIR}/az_iot_hub_client_telemetry.c
  ${CMAKE_CURRENT_LIST_DIR}/az_iot_hub_client_c2d.c
  ${CMAKE_CURRENT_LIST_DIR}/az_iot_hub_client_twin.c
  ${CMAKE_CURRENT_LIST_DIR}/az_iot_hub_client_methods.c
  ${CMAKE_CURRENT_LIST_DIR}/az_iot_hub_client_commands.c
  ${CMAKE_CURRENT_LIST_DIR}/az_iot_hub_client_properties.c
)

target_include_directories (az_iot_hub
  PUBLIC
    ${az_SOURCE_DIR}/sdk/inc
)

target_link_libraries(az_iot_hub
  PUBLIC
    az::core
    az::iot::common
)

add_library (az::iot::hub ALIAS az_iot_hub)

# Azure IoT Provisioning Service Library
add_library (az_iot_provisioning
  ${CMAKE_CURRENT_LIST_DIR}/az_iot_provisioning_client.c
  ${CMAKE_CURRENT_LIST_DIR}/az_iot_provisioning_client_sas.c
)

target_include_directories (az_iot_provisioning
  PUBLIC
    ${az_SOURCE_DIR}/sdk/inc
)

target_link_libraries(az_iot_provisioning
  PUBLIC
    az::core
    az::iot::common
)

add_library (az::iot::provisioning ALIAS az_iot_provisioning)

add_library(az_iot_adu
  ${CMAKE_CURRENT_LIST_DIR}/az_iot_adu_client.c
)

target_include_directories (az_iot_adu
  PUBLIC
    ${az_SOURCE_DIR}/sdk/inc
)

add_library (az::iot::adu ALIAS az_iot_adu)

# set coverage excluding for az_core. Don't show coverage outside iot
set(COV_EXCLUDE
  ${az_SOURCE_DIR}/sdk/inc/azure/core/*
  ${az_SOURCE_DIR}/sdk/inc/azure/core/internal/*)

create_code_coverage_targets(az_iot_adu)
create_code_coverage_targets(az_iot_common)
create_code_coverage_targets(az_iot_hub)
create_code_coverage_targets(az_iot_provisioning)
