/*******************************************************************************/
/*                                                                             */
/* Copyright (c) Microsoft Corporation. All rights reserved.                   */
/*                                                                             */
/* This software is licensed under the Microsoft Software License              */
/* Terms for Microsoft Azure Defender for IoT. Full text of the license can be */
/* found in the LICENSE file at https://aka.ms/AzureDefenderForIoT_EULA        */
/* and in the root directory of this software.                                 */
/*                                                                             */
/*******************************************************************************/

#ifndef __COMPONENT_INFO_H__
#define __COMPONENT_INFO_H__

#include <stdint.h>

#include <asc_config.h>

#include "asc_security_core/asc_result.h"

#include "asc_security_core/components_factory_enum.h"
#include "asc_security_core/utils/collection/bit_vector.h"

BIT_VECTOR_DECLARATIONS(component_owners_t, COMPONENTS_COUNT)

/**
 * @brief Type definition of component ID.
 */
typedef uintptr_t component_id_t;

/**
 * @brief Type definition of life cycle management functions: init(), deinit(), start(), stop().
 */
typedef asc_result_t (*lcm_func_t)(component_id_t id);

/**
 * @brief State enumerator of built-in component.
 */
typedef enum {
    COMPONENT_UNLOADED = 0,
    COMPONENT_LOADED,
    COMPONENT_FAIL,
    COMPONENT_INITIALIZED,
    COMPONENT_RUNNING,
    COMPONENT_STOPED,
    COMPONENT_SUBSCRIBED,
    COMPONENT_UNSUBSCRIBED,
    COMPONENT_UNDEFINED
} component_state_enum_t;

/**
 * @brief Struct of component internal info.
 */
typedef struct {
/* The current component's state from component_state_enum_t */
    component_state_enum_t state;

/* The last component's result (like erno but for each component) */
    asc_result_t last_result;

/* The component's ID */
    component_id_t id;

/* The component's name */
    const char *name;

/* The component's enumerator */
    int enumerator;

/* The component's internal context */
    void *ctx;

/* The component's external context */
    uintptr_t ext_ctx;

/* Disable auto start configuration */
    bool auto_start_disable;

/* The bit vector of component's owners - who started and stopped it */
    bit_vector_component_owners_t owners;

/* The log level */
    unsigned int log_level;

} component_info_t;

/**
 * @brief Struct of component's operations.
 */
typedef struct {
/* The 'init()' function is for internal allocations and subscriptions. Don't use here API from another components,
 * don't set timers, don't send notifications etc.
 */
    lcm_func_t init;

/* The 'subscribe()' is place to registrate and subscribe to APIs from another components. */
    lcm_func_t subscribe;

/* The 'start()' is place to set timers, send notifications and start the component's logic. */
    lcm_func_t start;

/* The 'deinit()' function is for free internal memory. Don't use here API from another components,
 * don't set/delete timers, don't send notifications etc.
 */
    lcm_func_t deinit;

/* The 'unsubscribe()' is place to unregistrate and unsubscribe from APIs from another components. */
    lcm_func_t unsubscribe;

/* The 'stop()' is place to delete timers, send notifications and finalize all running sequence of component. */
    lcm_func_t stop;

} component_ops_t;

/**
 * @brief Struct of component's.
 */
typedef struct {
    component_info_t info;
    component_ops_t *ops;
} component_t;

#endif