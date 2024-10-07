cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0079 NEW)

set(PROJECT_NAME "iot_security_module")
project(${PROJECT_NAME})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(${CMAKE_CURRENT_LIST_DIR}/iot-security-module-core/configs/functions.cmake)

# `IOT_SECURITY_MODULE_DIST_TARGET` A Compile flag that specifies the name of the distribution
# to be built, the value needs to be one of the *.dist file names (excluding extension)
# in 'netxduo/addons/azure_iot/azure_iot_security_module/configs' directory.
# The relevant 'asc_config.h' header file will then be generated with the property values
# specified in the chosen distribution.
if(NOT DEFINED IOT_SECURITY_MODULE_DIST_TARGET)
    message("Target Distribution Undefined, setting default 'RTOS_BASE'")
    set(IOT_SECURITY_MODULE_DIST_TARGET RTOS_BASE)
endif()

CONF_INC_CLEAN()

include_directories(${CMAKE_CURRENT_LIST_DIR}/inc/configs/${IOT_SECURITY_MODULE_DIST_TARGET})

CONF_LOG_LEVEL()

CONF_CREATE_DIST(
        ${IOT_SECURITY_MODULE_DIST_TARGET}
        ${CMAKE_CURRENT_LIST_DIR}/iot-security-module-core/configs/
        ${CMAKE_CURRENT_LIST_DIR}/configs/
        ${CMAKE_CURRENT_LIST_DIR}/inc/configs/${IOT_SECURITY_MODULE_DIST_TARGET}/asc_config.h
        __ASC_CONFIG_H__
        ON
        ON
    )

if (DEFINED asc_config_h_only)
    if (${asc_config_h_only})
        message("asc_config_h_only is ${asc_config_h_only} - configuration finished")
        return()
    endif()
endif()

# Azure IoT Security Module core
if(UNIX)
    set(build_as_32 ON CACHE BOOL "build as 32 bit")
endif()

add_subdirectory(iot-security-module-core)
# FlatBUffers
target_include_directories(flatccrt
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/inc
        ${CMAKE_CURRENT_LIST_DIR}/inc/configs/${IOT_SECURITY_MODULE_DIST_TARGET}
)

# Define our target library and an alias for consumers
add_library(${PROJECT_NAME}
    ${CMAKE_CURRENT_LIST_DIR}/nx_azure_iot_security_module.c

    $<$<BOOL:${ASC_COLLECTOR_NETWORK_ACTIVITY_ENABLED}>:${CMAKE_CURRENT_LIST_DIR}/src/collectors/collector_network_activity.c>
    $<$<BOOL:${ASC_COLLECTOR_SYSTEM_INFORMATION_ENABLED}>:${CMAKE_CURRENT_LIST_DIR}/src/collectors/collector_system_information.c>
    ${CMAKE_CURRENT_LIST_DIR}/src/model/objects/object_network_activity_ext.c
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/irand.c
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/itime.c
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/os_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/src/utils/ievent_loop.c
)
add_library("azrtos::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${AZURE_IOT_SECURITY_MODULE}
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/inc
        ${CMAKE_CURRENT_LIST_DIR}/inc/configs/${IOT_SECURITY_MODULE_DIST_TARGET}
)


target_link_libraries(${PROJECT_NAME}
    PUBLIC
        asc_security_core
        az::core
        netxduo
)
target_link_libraries(asc_security_core
    PUBLIC
        netxduo
)

target_include_directories(asc_security_core
    PRIVATE
        inc
        ${CMAKE_CURRENT_LIST_DIR}/inc/configs/${IOT_SECURITY_MODULE_DIST_TARGET}
)

# Define any required dependencies between this library and others
target_link_libraries(asc_security_core
    PRIVATE
        ${PROJECT_NAME}
)

setTargetCompileOptions(${PROJECT_NAME})
compileTargetAsC99(${PROJECT_NAME})

set_target_properties(asc_security_core PROPERTIES FOLDER "azure_iot_security_module")
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "azure_iot_security_module")
set_target_properties(flatccrt PROPERTIES FOLDER "azure_iot_security_module")
