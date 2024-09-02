# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Checks if code coverage can be generated for a target. This is true only when:
# 1.- env var AZ_SDK_CODE_COV is set
# 2.- cmake project is set to use gcc
# 3.- cmake project is set to build type Debug
#

if(DEFINED ENV{AZ_SDK_CODE_COV} AND CMAKE_C_COMPILER_ID MATCHES "GNU")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        include(CodeCoverage)
    endif()
endif()
