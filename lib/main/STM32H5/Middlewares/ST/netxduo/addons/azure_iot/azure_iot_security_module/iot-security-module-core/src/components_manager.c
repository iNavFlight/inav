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
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <asc_config.h>

#include "asc_security_core/logger.h"
#include "asc_security_core/utils/string_utils.h"

#include "asc_security_core/components_factory_declarations.h"
#include "asc_security_core/components_manager.h"

static bool _components_manager_initialized = false;
static uintptr_t _components_manager_global_context;

static void components_manager_do_lcm_action(component_state_enum_t to_state, component_state_enum_t from_state);
static int _get_owner_enumerator(component_id_t owner_id);

asc_result_t components_manager_init(void)
{
    asc_result_t result = ASC_RESULT_OK;

    if (_components_manager_initialized) {
        log_debug("Aleady initialized");
        return ASC_RESULT_INITIALIZED;
    }

    log_debug("Init");
    int index = 0;
    component_load_function_t *func_ptr = NULL;
    component_load_function_t **init_array = components_factory_get_load_array();

    if (init_array == NULL) {
        log_fatal("Unable to get components init array");
        return ASC_RESULT_EXCEPTION;
    }
    _components_manager_initialized = true;

    while ((func_ptr = init_array[index++])) {
        component_load_function_t func = *func_ptr;

        if (func) {
            asc_result_t ret = func();
            if (ret != ASC_RESULT_OK) {
                result = ret;
                log_fatal("Unable to load component with index=[%d]", index-1);
            }
        }
    }

#ifdef ASC_DYNAMIC_FACTORY_ENABLED
    asc_result_t result_dyn = components_factory_load_dynamic(NULL);
    if (result_dyn != ASC_RESULT_OK && result_dyn != ASC_RESULT_EMPTY) {
        result = result_dyn;
    }
#endif

    components_manager_do_lcm_action(COMPONENT_INITIALIZED, COMPONENT_LOADED);
    components_manager_do_lcm_action(COMPONENT_SUBSCRIBED, COMPONENT_INITIALIZED);
    components_manager_do_lcm_action(COMPONENT_RUNNING, COMPONENT_SUBSCRIBED);

    return result;
}

asc_result_t components_manager_deinit(void)
{
    if (!_components_manager_initialized) {
        log_debug("Unintialized");
        return ASC_RESULT_IMPOSSIBLE;
    }
    _components_manager_initialized = false;
    log_debug("Deinit");
    components_manager_do_lcm_action(COMPONENT_STOPED, COMPONENT_RUNNING);
    components_manager_do_lcm_action(COMPONENT_UNSUBSCRIBED, COMPONENT_UNDEFINED);
    components_manager_do_lcm_action(COMPONENT_LOADED, COMPONENT_UNDEFINED);

#ifdef ASC_DYNAMIC_FACTORY_ENABLED
    components_factory_unload_dynamic(); // must be before components_factory_unload();
#endif
    components_factory_unload();
    _components_manager_global_context = 0;
    return ASC_RESULT_OK;
}

void components_manager_global_context_set(uintptr_t ctx)
{
    _components_manager_global_context = ctx;
}

uintptr_t components_manager_global_context_get(void)
{
    return _components_manager_global_context;
}

#ifdef ASC_COMPONENT_CONFIGURATION
bool components_manager_set_log_level(component_id_t id, int set)
{
    unsigned int level = (set < 0) ? ASC_LOG_LEVEL : (unsigned int)set;

    if (level > ASC_LOG_LEVEL) {
        log_error("Requested log level=[%u] is above than compiled=[%u]", level, ASC_LOG_LEVEL);
        return false;
    }
    if (!id) {
        id = components_manager_get_id(ManagerCore);
    }
    component_info_t *info = components_manager_get_info(id);

    if (info) {
        info->log_level = level;
        return true;
    }
    return false;
}

unsigned int components_manager_get_log_level(component_id_t id)
{
    if (!id) {
        id = components_manager_get_id(ManagerCore);
    }
    component_info_t *info = components_manager_get_info(id);

    // return debug if no info or not initialized yet to not lost the log print
    if (!info || info->state == COMPONENT_UNLOADED) {
        return LOG_LEVEL_DEBUG;
    }

    return info->log_level; 
}

bool components_manager_set_log_level_all(int set)
{
    unsigned int level = (set < 0) ? ASC_LOG_LEVEL : (unsigned int)set;

    if (level > ASC_LOG_LEVEL) {
        log_error("Requested log level=[%u] is above than compiled=[%u]", level, ASC_LOG_LEVEL);
        return false;
    }
    for (int index = 0; index < COMPONENTS_COUNT; index++) {
        g_component_factory[index].component.info.log_level = level;
    }
    return true;
}
#endif

component_t *components_manager_get_component(component_id_t id)
{
    for (int index = 0; index < COMPONENTS_COUNT; index++) {
        if (g_component_factory[index].component.info.id == id) {
            return &g_component_factory[index].component;
        }
    }
    return NULL;
}

component_id_t components_manager_get_id_by_name(const char *name, size_t len)
{
    if (str_isempty(name)) {
        return 0;
    }

    for (int index = 0; index < COMPONENTS_COUNT; index++) {

        if (!str_ncmp(g_component_factory[index].component.info.name,
                str_len(g_component_factory[index].component.info.name), name, len)) {
            return g_component_factory[index].component.info.id;
        }
    }
    return 0;
}

component_info_t *components_manager_get_info(component_id_t id)
{
    component_t *component = components_manager_get_component(id);

    return component ? &component->info : NULL;
}

void *__components_manager_get_ctx(component_id_t id)
{
    component_t *component = components_manager_get_component(id);

    return component ? component->info.ctx : NULL;
}

void __components_manager_set_ctx(component_id_t id, void *ctx)
{
    component_t *component = components_manager_get_component(id);

    if (component) {
        component->info.ctx = ctx;
    }
}

asc_result_t components_manager_get_last_result(component_id_t id)
{
    component_t *component = components_manager_get_component(id);

    return component ? component->info.last_result : ASC_RESULT_UNINITIALIZED;
}

const char *components_manager_get_name(component_id_t id)
{
    component_t *component = components_manager_get_component(id);

    return component ? component->info.name : "";
}

asc_result_t components_manager_start(component_id_t id, component_id_t owner_id)
{
    asc_result_t result = ASC_RESULT_OK;
    component_t *component = components_manager_get_component(id);
    int owner_enumerator = -1;
    component_state_enum_t cur_state = COMPONENT_UNLOADED;
    component_ops_t *ops = NULL;
    lcm_func_t func = NULL;

    if (!component) {
        return ASC_RESULT_BAD_ARGUMENT;
    }

    owner_enumerator = _get_owner_enumerator(owner_id);
    if (owner_enumerator < 0) {
        return ASC_RESULT_BAD_ARGUMENT;
    }

    cur_state = component->info.state;
    if (cur_state != COMPONENT_SUBSCRIBED && cur_state != COMPONENT_STOPED && cur_state != COMPONENT_RUNNING) {
        result = ASC_RESULT_IMPOSSIBLE;
        goto cleanup;
    }

    /* Handle case already in running state */
    if (cur_state == COMPONENT_RUNNING) {
        bit_vector_set(component_owners_t, &component->info.owners, owner_enumerator, true);
        goto cleanup;
    }

    ops = component->ops;
    func = ops ? ops->start : NULL;

    result = func ? func(id) : ASC_RESULT_OK;
    if (result == ASC_RESULT_OK) {
        bit_vector_set(component_owners_t, &component->info.owners, owner_enumerator, true);
        component->info.state = COMPONENT_RUNNING;
    }
    goto cleanup;

cleanup:
    if (result != ASC_RESULT_OK) {
        component->info.state = COMPONENT_FAIL;
        log_error("failed to start component=[%s]", component->info.name);
    }
    return result;
}

asc_result_t components_manager_stop(component_id_t id, component_id_t owner_id, bool ignore_self_owner, bool force)
{
    asc_result_t result = ASC_RESULT_OK;
    component_t *component = components_manager_get_component(id);
    int owner_enumerator = -1;
    bool owner = false;
    bool self_owner = false;

    if (!component) {
        return ASC_RESULT_BAD_ARGUMENT;
    }

    owner_enumerator = _get_owner_enumerator(owner_id);
    if (owner_enumerator < 0) {
        return ASC_RESULT_BAD_ARGUMENT;
    }

    if (component->info.state == COMPONENT_FAIL) {
        result = ASC_RESULT_IMPOSSIBLE;
        goto cleanup;
    }

    /* Handle case already in stopped state */
    if (component->info.state != COMPONENT_RUNNING) {
        if (force) {
            bit_vector_clean(component_owners_t, &component->info.owners);
        } else {
            if (ignore_self_owner) {
                bit_vector_set(component_owners_t, &component->info.owners, component->info.enumerator, false);
            }
            bit_vector_set(component_owners_t, &component->info.owners, owner_enumerator, false);
        }
        goto cleanup;
    }

    /* Prepare vector */
    if (force) {
        bit_vector_clean(component_owners_t, &component->info.owners);
    } else {
        owner = bit_vector_get(component_owners_t, &component->info.owners, owner_enumerator);
        self_owner = bit_vector_get(component_owners_t, &component->info.owners, component->info.enumerator);
        if (ignore_self_owner) {
            bit_vector_set(component_owners_t, &component->info.owners, component->info.enumerator, false);
        }
        bit_vector_set(component_owners_t, &component->info.owners, owner_enumerator, false);
    }

    /* Performe stop */
    if (is_bit_vector_zero(component_owners_t, &component->info.owners)) {
        component_ops_t *ops = component->ops;
        lcm_func_t func = ops ? ops->stop : NULL;

        result = func ? func(id) : ASC_RESULT_OK;
        if (result == ASC_RESULT_OK) {
            component->info.state = COMPONENT_STOPED;
        } else {
            /* Restore status of owners */
            bit_vector_set(component_owners_t, &component->info.owners, owner_enumerator, owner);
            bit_vector_set(component_owners_t, &component->info.owners, component->info.enumerator, self_owner);
        }
    } else {
        /* Restore status of self owner */
        bit_vector_set(component_owners_t, &component->info.owners, component->info.enumerator, self_owner);
    }

cleanup:
    if (result != ASC_RESULT_OK) {
        component->info.state = COMPONENT_FAIL;
        log_error("failed to stop component=[%s]", component->info.name);
    }
    return result;
}

static lcm_func_t _load_lcm_function(int index, component_state_enum_t to_state, component_state_enum_t from_state)
{
    lcm_func_t func = NULL;
    component_ops_t *ops = g_component_factory[index].component.ops;

    if (ops) {
        switch (to_state) {
        case COMPONENT_INITIALIZED:
            func = ops->init;
            break;
        case COMPONENT_SUBSCRIBED:
            func = ops->subscribe;
            break;
        case COMPONENT_RUNNING:
            func = ops->start;
            break;
        case COMPONENT_STOPED:
            func = ops->stop;
            break;
        case COMPONENT_UNSUBSCRIBED:
            func = ops->unsubscribe;
            break;
        case COMPONENT_LOADED:
            func = ops->deinit;
            break;
        default:
            g_component_factory[index].component.info.last_result = ASC_RESULT_NOT_SUPPORTED_EXCEPTION;
            log_fatal("Unsupported LCM state=[%d]", to_state);
            return NULL;
        }
    }

    log_debug("lcm function component=[%s] ops=[%p] is defined=[%d] to=[%d] from=[%d] current=[%d]",
        g_component_factory[index].component.info.name,
        (void *)ops, func ? 1 : 0, to_state, from_state,
        g_component_factory[index].component.info.state);

    return func;
}

static void _perform_lcm_action(lcm_func_t func, int index, component_state_enum_t to_state)
{
    switch (to_state)
    {
    case COMPONENT_RUNNING:
        if (!g_component_factory[index].component.info.auto_start_disable) {
            g_component_factory[index].component.info.last_result =
                components_manager_start(g_component_factory[index].component.info.id, g_component_factory[index].component.info.id);
        } else {
            log_debug("%d function is disabled by auto_start_disable==true in component=[%s]", to_state, g_component_factory[index].component.info.name);
        }

        break;
    case COMPONENT_STOPED:
        g_component_factory[index].component.info.last_result =
            components_manager_stop(g_component_factory[index].component.info.id, g_component_factory[index].component.info.id, false, true);
        break;
    default:
        if (func == NULL) {
            if (g_component_factory[index].component.info.state != COMPONENT_FAIL) {
                g_component_factory[index].component.info.state = to_state;
            }
            log_debug("%d function function in component=[%s] is not armed", to_state, g_component_factory[index].component.info.name);
            return;
        }
        g_component_factory[index].component.info.last_result = func(g_component_factory[index].component.info.id);

        if (g_component_factory[index].component.info.last_result != ASC_RESULT_OK) {
            g_component_factory[index].component.info.state = COMPONENT_FAIL;
            log_error("failed to %d function component=[%s]", to_state, g_component_factory[index].component.info.name);
        } else {
            g_component_factory[index].component.info.state = to_state;
            log_debug("%d function component=[%s] done", to_state, g_component_factory[index].component.info.name);
        }
        break;
    }
}

static void components_manager_do_lcm_action(component_state_enum_t to_state, component_state_enum_t from_state)
{
    for (int index = 0; index < COMPONENTS_COUNT; index++) {
        if (g_component_factory[index].component.info.state == COMPONENT_UNLOADED) {
            continue;
        }
        g_component_factory[index].component.info.last_result = ASC_RESULT_OK;

        lcm_func_t func = _load_lcm_function(index, to_state, from_state);

        if (g_component_factory[index].component.info.last_result == ASC_RESULT_NOT_SUPPORTED_EXCEPTION) {
            return;
        }

        bool is_stop_start_same_state = false;

        if (g_component_factory[index].component.info.state == to_state && (to_state == COMPONENT_RUNNING || to_state == COMPONENT_STOPED)) {
            is_stop_start_same_state = true;
        }

        if (func && !is_stop_start_same_state && from_state != COMPONENT_UNDEFINED && g_component_factory[index].component.info.state != from_state) {
            log_error("Expected=[%d] state is not match current=[%d] component=[%s]", from_state, g_component_factory[index].component.info.state, g_component_factory[index].component.info.name);
            g_component_factory[index].component.info.last_result = ASC_RESULT_PARSE_EXCEPTION;
            continue;
        }
        _perform_lcm_action(func, index, to_state);
    }
}

static int _get_owner_enumerator(component_id_t owner_id)
{
    component_t *owner = components_manager_get_component(owner_id);

    if (owner == NULL) {
        return -1;
    }
    return owner->info.enumerator;
}

/* Component manager is component itself */
COMPONENTS_FACTORY_DEFINITION(ManagerCore, NULL)
