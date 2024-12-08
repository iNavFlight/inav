cmake_minimum_required(VERSION 3.13)

set(TARGET_NAME "asc_security_core")
project(${TARGET_NAME})

# Include the common build rules for the security module core
include(cmake-modules/asc_security_coreBuild.cmake)
include(cmake-modules/asc_security_core_build_rules.cmake)

include(configs/functions.cmake)

CONF_LOG_LEVEL()

CONF_INC_CLEAN()

if(DEFINED dist_test)
    include_directories(${CMAKE_SOURCE_DIR}/inc/configs/${dist_test})
    CONF_CREATE_DIST(
        ${dist_test}
        ${CMAKE_SOURCE_DIR}/configs/test/
        ${CMAKE_SOURCE_DIR}/configs/test/plat/
        ${CMAKE_SOURCE_DIR}/inc/configs/${dist_test}/asc_config_test.h
        __ASC_CONFIG_TEST_H__
        OFF
        ON
    )
endif()

if(NOT DEFINED IOT_SECURITY_MODULE_DIST_TARGET AND NOT DEFINED DEFENDER_IOT_MICRO_AGENT_DIST_CORE)
    message(FATAL_ERROR "Target Distribution Undefined")
endif()

if(DEFINED DEFENDER_IOT_MICRO_AGENT_DIST_CORE)
    include_directories(${CMAKE_SOURCE_DIR}/inc/configs/${DEFENDER_IOT_MICRO_AGENT_DIST_CORE})
    CONF_CREATE_DIST(
        ${DEFENDER_IOT_MICRO_AGENT_DIST_CORE}
        ${CMAKE_SOURCE_DIR}/configs/
        ${CMAKE_SOURCE_DIR}/configs/
        ${CMAKE_SOURCE_DIR}/inc/configs/${DEFENDER_IOT_MICRO_AGENT_DIST_CORE}/asc_config.h
        __ASC_CONFIG_H__
        ON
        OFF
    )
endif()

if (DEFINED asc_config_h_only)
    if (${asc_config_h_only})
        message("asc_config_h_only is ${asc_config_h_only} - configuration finished")
        return()
    endif()
endif()

set(build_pedantic ON CACHE BOOL "use -Wpedantic flag in compilation")

# the following variables are project-wide and can be used with cmake-gui
set(build_as_32 OFF CACHE BOOL "build as 32 bit")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake-modules")

if(${run_core_unittests})
    FILE(REMOVE ${CMAKE_BINARY_DIR}/valgrind_targets.lst)

    if (${run_core_coverage})
        include(asc_security_coreCodeCoverage)
        include(asc_security_coreCreateCodeCoverageTargets)
        include(asc_security_coreCheckAndIncludeCodeCov)
    endif ()
endif()

add_library(${TARGET_NAME} STATIC
    # components
    $<$<BOOL:${ASC_COMPONENT_CLI_PLAT}>:${CMAKE_CURRENT_SOURCE_DIR}/src/cli_plat.c>
    
    $<$<BOOL:${ASC_COMPONENT_COMMAND_EXECUTOR}>:${CMAKE_CURRENT_SOURCE_DIR}/src/command_executor.c>
    $<$<BOOL:${ASC_COMPONENT_CONFIGURATION}>:${CMAKE_CURRENT_SOURCE_DIR}/src/configuration.c>

    $<$<BOOL:${ASC_COLLECTOR_HEARTBEAT_ENABLED}>:${CMAKE_CURRENT_SOURCE_DIR}/src/collectors/collector_heartbeat.c>
    $<$<BOOL:${ASC_COLLECTOR_BASELINE_ENABLED}>:${CMAKE_CURRENT_SOURCE_DIR}/src/collectors/baseline.c>
    $<$<BOOL:${ASC_COLLECTOR_BASELINE_ENABLED}>:${CMAKE_CURRENT_SOURCE_DIR}/src/serializer/baseline.c>
    $<$<BOOL:${ASC_COLLECTOR_BASELINE_ENABLED}>:${CMAKE_CURRENT_SOURCE_DIR}/src/model/objects/object_baseline_ext.c>
    $<$<BOOL:${ASC_COLLECTOR_LOG_ENABLED}>:${CMAKE_CURRENT_SOURCE_DIR}/src/serializer/log.c>
    $<$<BOOL:${ASC_COLLECTOR_HEARTBEAT_ENABLED}>:${CMAKE_CURRENT_SOURCE_DIR}/src/serializer/heartbeat.c>
    $<$<BOOL:${ASC_COLLECTOR_PROCESS_ENABLED}>:${CMAKE_CURRENT_SOURCE_DIR}/src/serializer/process.c>
    $<$<BOOL:${ASC_COLLECTOR_LISTENING_PORTS_ENABLED}>:${CMAKE_CURRENT_SOURCE_DIR}/src/serializer/listening_ports.c>
    $<$<BOOL:${ASC_COLLECTOR_NETWORK_ACTIVITY_ENABLED}>:${CMAKE_CURRENT_SOURCE_DIR}/src/serializer/network_activity.c>
    $<$<BOOL:${ASC_COLLECTOR_SYSTEM_INFORMATION_ENABLED}>:${CMAKE_CURRENT_SOURCE_DIR}/src/serializer/system_information.c>

    $<$<BOOL:${ASC_SERIALIZER_USE_CUSTOM_ALLOCATOR}>:${CMAKE_CURRENT_SOURCE_DIR}/src/serializer/extensions/custom_builder_allocator.c>
    $<$<BOOL:${ASC_SERIALIZER_USE_CUSTOM_ALLOCATOR}>:${CMAKE_CURRENT_SOURCE_DIR}/src/serializer/extensions/page_allocator.c>

    src/components_factory.c
    src/components_manager.c
    src/collector_collection.c
    $<$<BOOL:${ASC_COLLECTORS_INFO_SUPPORT}>:${CMAKE_CURRENT_SOURCE_DIR}/src/collectors_info.c>
    src/core.c
    src/logger.c
    src/model/collector.c
    src/model/security_message.c
    src/serializer/serializer_private.c
    src/serializer/serializer.c
    src/utils/iconv.c
    src/utils/notifier.c
    src/utils/uuid.c
    src/utils/collection/bit_vector.c
    src/utils/collection/stack.c
    src/utils/collection/list.c
    src/utils/collection/hashtable.c
    $<$<BOOL:${ASC_LINKED_LIST_NODE_SUPPORT}>:${CMAKE_CURRENT_SOURCE_DIR}/src/utils/collection/linked_list_node.c>
    $<IF:$<BOOL:${ASC_DYNAMIC_MEMORY_ENABLED}>,src/object_pool_dynamic.c,src/object_pool_static.c>
    $<$<BOOL:${ASC_BEST_EFFORT_EVENT_LOOP}>:${CMAKE_CURRENT_SOURCE_DIR}/src/utils/event_loop_be.c>
    src/utils/string_utils.c
)

target_include_directories(${TARGET_NAME} PUBLIC inc)

add_subdirectory(deps)
target_link_libraries(${TARGET_NAME} PRIVATE
    flatccrt
)

if(${ASC_SERIALIZER_USE_CUSTOM_ALLOCATOR})
    target_link_libraries(flatccrt PRIVATE asc_security_core)
    target_include_directories(flatccrt PRIVATE deps/flatcc/src/runtime)
endif()

if (${run_core_unittests})
    enable_testing()
    include(CTest)
    add_subdirectory(tests)

endif ()

setTargetCompileOptions(${TARGET_NAME})
compileTargetAsC99(${TARGET_NAME})
