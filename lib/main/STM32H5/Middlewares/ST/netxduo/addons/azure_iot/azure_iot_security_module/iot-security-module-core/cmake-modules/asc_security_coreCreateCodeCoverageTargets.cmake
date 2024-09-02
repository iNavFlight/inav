# asc_security_coreCodeCoverage
if(CMAKE_C_COMPILER_ID MATCHES "GNU")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        APPEND_COVERAGE_COMPILER_FLAGS()
        
        # Basic coverage using lcov (gcc integrated)
        setup_target_for_coverage_lcov(NAME ${TARGET_NAME}_cov
                                       EXECUTABLE ${TARGET_NAME}_test
                                       EXCLUDE ${COV_EXCLUDE})
        
        # HTML and XML - Coverage using gcovr (Needs to be installed into system)
        setup_target_for_coverage_gcovr_html(NAME ${TARGET_NAME}_cov_html EXECUTABLE ${TARGET_NAME}_test)
        setup_target_for_coverage_gcovr_xml(NAME ${TARGET_NAME}_cov_xml EXECUTABLE ${TARGET_NAME}_test)
    endif() 
endif()