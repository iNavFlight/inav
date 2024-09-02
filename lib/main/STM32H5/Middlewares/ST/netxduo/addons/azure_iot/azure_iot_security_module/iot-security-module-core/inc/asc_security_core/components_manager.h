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

#ifndef __COMPONENTS_MANAGER_H__
#define __COMPONENTS_MANAGER_H__
#include <asc_config.h>

#include "asc_security_core/asc_result.h"
#include "asc_security_core/components_factory.h"

/**
 * @brief Call init and start function of all declarated built-in components.
 *
 * @return A @c asc_result_t result.
 */
asc_result_t components_manager_init(void);

/**
 * @brief Set global context.
 *
 * @param ctx Global Context
 *
 * @return none.
 */
void components_manager_global_context_set(uintptr_t ctx);

/**
 * @brief Get global context.
 *
 * @return The global context.
 */
uintptr_t components_manager_global_context_get(void);

/**
 * @brief Call stop and deinit function of all declarated built-in components.
 *
 * @return A @c asc_result_t result.
 */
asc_result_t components_manager_deinit(void);

/**
 * @brief Get internal component struct by it's ID.
 *
 * @param id Component ID provided by component manager on init function
 *
 * @return A @c component_t* representing component.
 */
component_t *components_manager_get_component(component_id_t id);

/**
 * @brief Get internal info of specific component by it's ID.
 *
 * @param id Component ID provided by component manager on init function
 *
 * @return A @c component_info_t* representing internal component info.
 */
component_info_t *components_manager_get_info(component_id_t id);

/**
 * @brief Get user component context by it's ID. Avoid using __components_manager_get_ctx() - use private components_manager_get_self_ctx() macro
 *
 * @param id Component ID provided by component manager on init function
 *
 * @return A @c void* user context.
 */
void *__components_manager_get_ctx(component_id_t id);
#define components_manager_get_self_ctx() __components_manager_get_ctx(components_manager_get_self_id())

/**
 * @brief Set user component context by it's ID. Avoid using __components_manager_set_ctx() - use private components_manager_set_self_ctx() macro
 *
 * @param id Component ID provided by component manager on init function
 *
 * @param ctx User context to store
 *
 * @return none.
 */
void __components_manager_set_ctx(component_id_t id, void *ctx);
#define components_manager_set_self_ctx(_ctx) __components_manager_set_ctx(components_manager_get_self_id(), _ctx)

/**
 * @brief Get last result of component.
 *
 * @param id Component ID provided by component manager on init function
 *
 * @return A @c asc_result_t result.
 */
asc_result_t components_manager_get_last_result(component_id_t id);

/**
 * @brief Get name of component.
 *
 * @param id Component ID provided by component manager on init function
 *
 * @return A component name.
 */
const char *components_manager_get_name(component_id_t id);

/**
 * @brief Set log level to specified component.
 *
 * @param id    Component ID provided by component manager on init function
 * @param set   Requested log level, if (-1) - reset to default 'ASC_LOG_LEVEL'
 *
 * @return true on seccess, otherwise false..
 */
bool components_manager_set_log_level(component_id_t id, int set);

/**
 * @brief Get log level to specified component.
 *
 * @param id    Component ID provided by component manager on init function
 *
 * @return log level.
 */
unsigned int components_manager_get_log_level(component_id_t id);

/**
 * @brief Set log level to all components.
 *
 * @param set   Requested log level, if (-1) - reset to default 'ASC_LOG_LEVEL'
 *
 * @return true on seccess, otherwise false..
 */
bool components_manager_set_log_level_all(int set);

/**
 * @brief Get ID of specific component by it's generated name.
 *
 * @param name Component name in string format
 * @paran len  The length of the name
 *
 * @return A @c component_id_t component ID.
 */
component_id_t  components_manager_get_id_by_name(const char *name, size_t len);

/**
 * @brief Get ID of specific component by it's generated name.
 *
 * @param _component Component generated name from @c component_enum_t or @c collector_enum_t
 *
 * @return A @c component_id_t component ID.
 */
#define components_manager_get_id(_component) (g_component_factory[_component].component.info.id)

/**
 * @brief Start component.
 *
 * @param id        Component ID provided by component manager on init function
 * @param owner_id  Component ID that asked the action
 *
 * @return A @c asc_result_t result.
 */
asc_result_t components_manager_start(component_id_t id, component_id_t owner_id);

/**
 * @brief Stop component.
 *
 * @param id                Component ID provided by component manager on init function
 * @param owner_id          Component ID that asked the action
 * @param ignore_self_owner If component has only self owner stop it
 * @param force             Force stop and clean owners
 *
 * @return A @c asc_result_t result.
 */
asc_result_t components_manager_stop(component_id_t id, component_id_t owner_id, bool ignore_self_owner, bool force);

#endif //__COMPONENTS_MANAGER_H__