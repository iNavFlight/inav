# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

if (TRANSPORT_PAHO)

cmake_minimum_required (VERSION 3.10)

project (az_iot_samples LANGUAGES C)

set(CMAKE_C_STANDARD 99)

find_package(eclipse-paho-mqtt-c CONFIG)
if(NOT eclipse-paho-mqtt-c_FOUND)
  find_package(eclipse-paho-mqtt-c REQUIRED)
endif()

find_package(OpenSSL CONFIG)
if(NOT OpenSSL_FOUND)
  find_package(OpenSSL REQUIRED)
endif()

# Azure IoT Samples Library
add_library (az_iot_sample_common
  ${CMAKE_CURRENT_LIST_DIR}/iot_sample_common.c
)

# Internal deps
target_link_libraries(az_iot_sample_common
  PUBLIC
    az::iot::hub
    az::iot::provisioning
)

# External deps
target_link_libraries(az_iot_sample_common
  PUBLIC
    eclipse-paho-mqtt-c::paho-mqtt3cs-static
    OpenSSL::SSL
    OpenSSL::Crypto
)

add_library (az::iot::sample::common ALIAS az_iot_sample_common)

# Azure IoT Samples Executables

# C2D Sample
add_executable (paho_iot_hub_c2d_sample
  ${CMAKE_CURRENT_LIST_DIR}/paho_iot_hub_c2d_sample.c
)

target_link_libraries(paho_iot_hub_c2d_sample
  PRIVATE
    az::iot::sample::common
)

create_map_file(paho_iot_hub_c2d_sample paho_iot_hub_c2d_sample.map)

# Methods Sample
add_executable (paho_iot_hub_methods_sample
  ${CMAKE_CURRENT_LIST_DIR}/paho_iot_hub_methods_sample.c
)

target_link_libraries(paho_iot_hub_methods_sample
  PRIVATE
    az::iot::sample::common
)

create_map_file(paho_iot_hub_methods_sample paho_iot_hub_methods_sample.map)

# Telemetry (Certificates) Sample
add_executable (paho_iot_hub_telemetry_sample
  ${CMAKE_CURRENT_LIST_DIR}/paho_iot_hub_telemetry_sample.c
)

target_link_libraries(paho_iot_hub_telemetry_sample
  PRIVATE
    az::iot::sample::common
)

create_map_file(paho_iot_hub_telemetry_sample paho_iot_hub_telemetry_sample.map)

# Telemetry (SAS) Sample
add_executable (paho_iot_hub_sas_telemetry_sample
  ${CMAKE_CURRENT_LIST_DIR}/paho_iot_hub_sas_telemetry_sample.c
)

target_link_libraries(paho_iot_hub_sas_telemetry_sample
  PRIVATE
    az::iot::sample::common
)

create_map_file(paho_iot_hub_sas_telemetry_sample paho_iot_hub_sas_telemetry_sample.map)

# Twin Sample
add_executable (paho_iot_hub_twin_sample
  ${CMAKE_CURRENT_LIST_DIR}/paho_iot_hub_twin_sample.c
)

target_link_libraries(paho_iot_hub_twin_sample
  PRIVATE
    az::iot::sample::common
)

create_map_file(paho_iot_hub_twin_sample paho_iot_hub_twin_sample.map)

# PnP Component Sample
add_executable (paho_iot_pnp_component_sample
  ${CMAKE_CURRENT_LIST_DIR}/pnp/pnp_mqtt_message.c
  ${CMAKE_CURRENT_LIST_DIR}/pnp/pnp_device_info_component.c
  ${CMAKE_CURRENT_LIST_DIR}/pnp/pnp_thermostat_component.c
  ${CMAKE_CURRENT_LIST_DIR}/pnp/pnp_temperature_controller_component.c
  ${CMAKE_CURRENT_LIST_DIR}/paho_iot_pnp_component_sample.c
)

# SDK deps
target_link_libraries(paho_iot_pnp_component_sample
  PRIVATE
    az::iot::sample::common
)

target_include_directories(paho_iot_pnp_component_sample
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

create_map_file(paho_iot_pnp_component_sample paho_iot_pnp_component_sample.map)

# PnP Sample
add_executable (paho_iot_pnp_sample
  ${CMAKE_CURRENT_LIST_DIR}/paho_iot_pnp_sample.c
  ${CMAKE_CURRENT_LIST_DIR}/paho_iot_pnp_sample_common.c
)

target_link_libraries(paho_iot_pnp_sample
  PRIVATE
    az::iot::sample::common
)

create_map_file(paho_iot_pnp_sample paho_iot_pnp_sample.map)

# PnP Provisioning Sample
add_executable (paho_iot_pnp_with_provisioning_sample
  ${CMAKE_CURRENT_LIST_DIR}/paho_iot_pnp_with_provisioning_sample.c
  ${CMAKE_CURRENT_LIST_DIR}/paho_iot_pnp_sample_common.c
)

target_link_libraries(paho_iot_pnp_with_provisioning_sample
  PRIVATE
    az::iot::sample::common
)

create_map_file(paho_iot_pnp_with_provisioning_sample paho_iot_pnp_with_provisioning_sample.map)

# Provisioning (Certificates) Sample
add_executable (paho_iot_provisioning_sample
  ${CMAKE_CURRENT_LIST_DIR}/paho_iot_provisioning_sample.c
)

target_link_libraries(paho_iot_provisioning_sample
  PRIVATE
    az::iot::sample::common
)

create_map_file(paho_iot_provisioning_sample paho_iot_provisioning_sample.map)

# Provisioning (SAS) Sample
add_executable (paho_iot_provisioning_sas_sample
  ${CMAKE_CURRENT_LIST_DIR}/paho_iot_provisioning_sas_sample.c
)

target_link_libraries(paho_iot_provisioning_sas_sample
  PRIVATE
    az::iot::sample::common
)

create_map_file(paho_iot_provisioning_sas_sample paho_iot_provisioning_sas_sample.map)

endif() # TRANSPORT_PAHO
