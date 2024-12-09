# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.10)

project (az_core_test LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include(AddCMockaTest)

# -ld link option is only available for gcc
if(UNIT_TESTING_MOCKS)
    set(WRAP_FUNCTIONS "-Wl,--wrap=az_platform_clock_msec -Wl,--wrap=az_platform_sleep_msec")
else()
    set(WRAP_FUNCTIONS "")
endif()

set(MATH_LIB_UNIX "")
if (UNIX)
    set(MATH_LIB_UNIX "m")
endif()

add_cmocka_test(az_core_test SOURCES
                main.c
                test_az_base64.c
                test_az_context.c
                test_az_http.c
                test_az_json.c
                test_az_logging.c
                test_az_pipeline.c
                test_az_policy.c
                test_az_span.c
                test_az_url_encode.c
                COMPILE_OPTIONS ${DEFAULT_C_COMPILE_FLAGS} ${NO_CLOBBERED_WARNING}
                LINK_LIBRARIES ${CMOCKA_LIB} ${MATH_LIB_UNIX} az_core ${PAL} az_nohttp
                LINK_OPTIONS ${WRAP_FUNCTIONS}  
                # include cmoka headers and private folder headers
                INCLUDE_DIRECTORIES ${CMOCKA_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/sdk/src/azure/core/
                )

create_map_file(az_core_test az_core_test.map)

add_cmocka_test_environment(az_core_test)
