# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.10)

project (az_iot_hub_test LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include(AddCMockaTest)

add_cmocka_test(az_iot_hub_test SOURCES
                main.c
                test_az_iot_hub_client_sas.c
                test_az_iot_hub_client_telemetry.c
                test_az_iot_hub_client_c2d.c
                test_az_iot_hub_client.c
                test_az_iot_hub_client_twin.c
                test_az_iot_hub_client_methods.c
                test_az_iot_hub_client_commands.c
                test_az_iot_hub_client_properties.c
                COMPILE_OPTIONS ${DEFAULT_C_COMPILE_FLAGS} ${NO_CLOBBERED_WARNING}
                LINK_LIBRARIES ${CMOCKA_LIB}
                    az_iot_common
                    az_iot_hub
                    az_core
                INCLUDE_DIRECTORIES ${CMOCKA_INCLUDE_DIR}
                )

create_map_file(az_iot_hub_test az_iot_hub_test.map)

add_cmocka_test_environment(az_iot_hub_test)
