# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.10)

project (az_iot_provisioning_test LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include(AddCMockaTest)

add_cmocka_test(az_iot_provisioning_test SOURCES
                main.c
                test_az_iot_provisioning_client.c
                test_az_iot_provisioning_client_sas.c
                test_az_iot_provisioning_client_parser.c
                test_az_iot_proviosining_client_payload.c
                COMPILE_OPTIONS ${DEFAULT_C_COMPILE_FLAGS} ${NO_CLOBBERED_WARNING}
                LINK_LIBRARIES ${CMOCKA_LIB}
                    az_iot_common
                    az_iot_provisioning
                    az_core
                INCLUDE_DIRECTORIES ${CMOCKA_INCLUDE_DIR}
                )

create_map_file(az_iot_provisioning_test az_iot_provisioning_test.map)

add_cmocka_test_environment(az_iot_provisioning_test)
