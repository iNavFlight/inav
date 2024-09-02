# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT
#
# Cmake module used by any project's target for creating a code coverage target for it.
#

# codeCoverage
macro(create_code_coverage_targets code_cov_target)
    if(DEFINED ENV{AZ_SDK_CODE_COV} AND CMAKE_C_COMPILER_ID MATCHES "GNU")
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            APPEND_COVERAGE_COMPILER_FLAGS()

            # Basic coverage using lcov (gcc integrated)
            setup_target_for_coverage_lcov(NAME ${code_cov_target}_cov
                                        EXECUTABLE ${code_cov_target}_test
                                        EXCLUDE ${COV_EXCLUDE}
                                        LCOV_ARGS --rc lcov_branch_coverage=1)

            # HTML and XML - Coverage using gcovr (Needs to be installed into system)
            setup_target_for_coverage_gcovr_html(NAME ${code_cov_target}_cov_html EXECUTABLE ${code_cov_target}_test)
            setup_target_for_coverage_gcovr_xml(NAME ${code_cov_target}_cov_xml EXECUTABLE ${code_cov_target}_test)

            # add project to coverage projects for printing
            file(APPEND ${CMAKE_BINARY_DIR}/coverage_targets.txt " ${code_cov_target}_cov_xml")
        endif()
    endif()
endmacro()
