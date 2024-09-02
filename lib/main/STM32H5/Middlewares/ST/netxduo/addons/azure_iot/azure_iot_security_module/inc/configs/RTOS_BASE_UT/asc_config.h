/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


#ifdef __cplusplus
extern "C" {
#endif

/* Distribution RTOS_BASE_UT */
#ifndef __ASC_CONFIG_H__
#define __ASC_CONFIG_H__

/********************
* Core configuration
*********************/
/* Based on 2022.07.19-3.5.3 core tag */

/* ID and version */
#define ASC_SECURITY_MODULE_ID "defender-iot-micro-agent"
#define SECURITY_MODULE_VERSION_MAJOR 3
#define SECURITY_MODULE_VERSION_MINOR 5
#define SECURITY_MODULE_VERSION_PATCH 4
#ifndef SECURITY_MODULE_VERSION_MAJOR
#define SECURITY_MODULE_VERSION_MAJOR 0
#endif
#ifndef SECURITY_MODULE_VERSION_MINOR
#define SECURITY_MODULE_VERSION_MINOR 0
#endif
#ifndef SECURITY_MODULE_VERSION_PATCH
#define SECURITY_MODULE_VERSION_PATCH 0
#endif

/* Collectors definitions */
/* #undef ASC_DYNAMIC_COLLECTORS_MAX */
#ifndef ASC_DYNAMIC_COLLECTORS_MAX
#define ASC_DYNAMIC_COLLECTORS_MAX 0
#endif
#define ASC_COLLECTOR_HEARTBEAT_ENABLED

/* #undef ASC_COLLECTOR_BASELINE_ENABLED */
/* #undef ASC_BASELINE_REPORT_POOL_ENTRIES */

/* #undef ASC_COLLECTOR_LOG_ENABLED */
/* #undef ASC_LOG_REPORT_POOL_ENTRIES */

#define ASC_COLLECTOR_NETWORK_ACTIVITY_ENABLED
/* #undef ASC_COLLECTOR_NETWORK_ACTIVITY_SEND_EMPTY_EVENTS */
#define ASC_COLLECTOR_NETWORK_ACTIVITY_MAX_IPV4_OBJECTS_IN_CACHE 4
#define ASC_COLLECTOR_NETWORK_ACTIVITY_MAX_IPV6_OBJECTS_IN_CACHE 4
#ifndef ASC_COLLECTOR_NETWORK_ACTIVITY_MAX_IPV4_OBJECTS_IN_CACHE
#define ASC_COLLECTOR_NETWORK_ACTIVITY_MAX_IPV4_OBJECTS_IN_CACHE 0
#endif
#ifndef ASC_COLLECTOR_NETWORK_ACTIVITY_MAX_IPV6_OBJECTS_IN_CACHE
#define ASC_COLLECTOR_NETWORK_ACTIVITY_MAX_IPV6_OBJECTS_IN_CACHE 0
#endif

/* #undef ASC_COLLECTOR_PROCESS_ENABLED */
/* #undef ASC_COLLECTOR_PROCESS_SEND_EMPTY_EVENTS */
/* #undef ASC_COLLECTOR_PROCESS_IN_CACHE */
#ifndef ASC_COLLECTOR_PROCESS_IN_CACHE
#define ASC_COLLECTOR_PROCESS_IN_CACHE 0
#endif

#define ASC_COLLECTOR_SYSTEM_INFORMATION_ENABLED

/* #undef ASC_COLLECTOR_LISTENING_PORTS_ENABLED */

/* Components definitions */
/* #undef ASC_DYNAMIC_FACTORY_ENABLED */
/* #undef ASC_DYNAMIC_FACTORY_PATH */
/* #undef ASC_DYNAMIC_FACTORY_PATH_RUNTIME_SET */
/* #undef ASC_DYNAMIC_COMPONENTS_MAX */
#ifndef ASC_DYNAMIC_COMPONENTS_MAX
#define ASC_DYNAMIC_COMPONENTS_MAX 0
#endif

/* #undef ASC_COMPONENT_COMMAND_EXECUTOR */
/* #undef ASC_OPERATIONS_POOL_ENTRIES */

/* #undef ASC_COMPONENT_CONFIGURATION */
/* #undef ASC_COMPONENT_CONFIGURATION_PLAT */

#define ASC_COMPONENT_SECURITY_MODULE

/* Collection definitions */
#define ASC_FIRST_COLLECTION_INTERVAL 30
#define ASC_HIGH_PRIORITY_INTERVAL 10
#define ASC_MEDIUM_PRIORITY_INTERVAL 30
#define ASC_LOW_PRIORITY_INTERVAL 60

/* Dynamic/Static memory */
/* #undef ASC_DYNAMIC_MEMORY_ENABLED */

/* ROM reduce */
/* #undef ASC_COMPONENT_CORE_SUPPORTS_RESTART */
/* #undef ASC_COLLECTORS_INFO_SUPPORT */
/* #undef ASC_LINKED_LIST_NODE_SUPPORT */

/* Log */
// Highest compiled log level
/* #undef ASC_LOG_LEVEL */
/* #undef ASC_TIME_H_SUPPORT */
/* #undef ASC_LOG_TIMESTAMP_DEFAULT */

/* Notifier definitions */
#define ASC_NOTIFIERS_OBJECT_POOL_ENTRIES 2

/* Event loop best effort */
#define ASC_BEST_EFFORT_EVENT_LOOP
#define ASC_BE_TIMERS_OBJECT_POOL_ENTRIES 2

/* Flat buffer serializer */
#define ASC_SERIALIZER_USE_CUSTOM_ALLOCATOR
#define ASC_FLATCC_JSON_PRINTER_OVERWRITE
#define ASC_EMITTER_PAGE_CACHE_SIZE 1
/* #undef FLATCC_NO_ASSERT */
/* FLATCC_ASSERT might be redefined in platform area */
/* #undef FLATCC_ASSERT */
#define FLATCC_USE_GENERIC_ALIGNED_ALLOC
/* #undef FLATCC_EMITTER_PAGE_SIZE */

/* Tests definitions */
// Set ASC_FIRST_FORCE_COLLECTION_INTERVAL to '-1' to force immediatly collecting
#define ASC_FIRST_FORCE_COLLECTION_INTERVAL 2
/* #undef ASC_EXTRA_BE_TIMERS_OBJECT_POOL_ENTRIES */
#define ASC_EXTRA_NOTIFIERS_OBJECT_POOL_ENTRIES 1
/* #undef ASC_EXTRA_COMPONENTS_COUNT */
#ifndef ASC_EXTRA_COMPONENTS_COUNT
#define ASC_EXTRA_COMPONENTS_COUNT 0
#endif
/* #undef ASC_EXTRA_COLLECTORS_COUNT */
#ifndef ASC_EXTRA_COLLECTORS_COUNT
#define ASC_EXTRA_COLLECTORS_COUNT 0
#endif

#define LOOP_ASSERT_FAIL for (;;) { }
#define LOOP_ASSERT(s) if (!(s)) {LOOP_ASSERT_FAIL}

/************************
* Platform configuration
*************************/
#ifdef FLATCC_ASSERT
#undef FLATCC_ASSERT
#endif

#ifndef __ASC_CONFIG_EXCLUDE_PORT__H__
#include "tx_port.h"
#include "nx_port.h"
#include "nx_api.h"

/* Flat buffer serializer platform */
/* #undef FLATCC_ASSERT */

#endif /* __ASC_CONFIG_EXCLUDE_PORT__H__ */

/* Security Module pending time, in seconds */
#define ASC_SECURITY_MODULE_PENDING_TIME 60*5
#define ASC_SECURITY_MODULE_SEND_MESSAGE_RETRY_TIME 3
#define ASC_SECURITY_MODULE_MAX_HUB_DEVICES 128
#ifndef ASC_SECURITY_MODULE_MAX_HUB_DEVICES
#define ASC_SECURITY_MODULE_MAX_HUB_DEVICES 64
#endif

/* Collector network activity. */
/* #undef ASC_COLLECTOR_NETWORK_ACTIVITY_TCP_DISABLED */
/* #undef ASC_COLLECTOR_NETWORK_ACTIVITY_UDP_DISABLED */
/* #undef ASC_COLLECTOR_NETWORK_ACTIVITY_ICMP_DISABLED */
#define ASC_COLLECTOR_NETWORK_ACTIVITY_CAPTURE_UNICAST_ONLY

/* The maximum number of IPv4 network events to store in memory. */
#ifdef NX_DISABLE_IPV6
#undef ASC_COLLECTOR_NETWORK_ACTIVITY_MAX_IPV6_OBJECTS_IN_CACHE
#define ASC_COLLECTOR_NETWORK_ACTIVITY_MAX_IPV6_OBJECTS_IN_CACHE 0
#endif

#endif /* __ASC_CONFIG_H__ */

#ifdef __cplusplus
}
#endif
