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

#include <asc_config.h>

#include "asc_security_core/components_factory_declarations.h"
#include "asc_security_core/components_manager.h"

components_factory_t g_component_factory[COMPONENTS_COUNT];

static component_load_function_t *component_load_function_array[] = {
    COMPONENTS_FACTORY_LOAD(ManagerCore),
    COMPONENTS_FACTORY_LOAD(Logger),
#ifdef ASC_COMPONENT_SECURITY_MODULE
    COMPONENTS_FACTORY_LOAD(SecurityModule),
#endif
    COMPONENTS_FACTORY_LOAD(CollectorsCore),
#ifdef ASC_COMPONENT_CONFIGURATION
    COMPONENTS_FACTORY_LOAD(Configuration),
#endif
#ifdef ASC_COMPONENT_CONFIGURATION_PLAT
    COMPONENTS_FACTORY_LOAD(ConfigurationPlatform),
#endif
#ifdef ASC_COMPONENT_COMMAND_EXECUTOR_PLAT
    COMPONENTS_FACTORY_LOAD(CommandExecutorPlatform),
#endif
#ifdef ASC_COLLECTOR_HEARTBEAT_ENABLED
    COMPONENTS_FACTORY_LOAD(Heartbeat),
#endif
#ifdef ASC_COLLECTOR_BASELINE_ENABLED
    COMPONENTS_FACTORY_LOAD(Baseline),
#endif
#ifdef ASC_COMPONENT_BASELINE_PLAT
    COMPONENTS_FACTORY_LOAD(BaselinePlatform),
#endif
#ifdef ASC_COMPONENT_IPC_PLAT
    COMPONENTS_FACTORY_LOAD(IpcPlatform),
#endif
#ifdef ASC_COMPONENT_CLI_PLAT
    #ifdef ASC_COMPONENT_DEMO_CLI_PLAT
        COMPONENTS_FACTORY_LOAD(CliDemoPlatform),
    #endif
    COMPONENTS_FACTORY_LOAD(CliPlatform),
#endif
#ifdef ASC_COLLECTOR_SYSTEM_INFORMATION_ENABLED
    COMPONENTS_FACTORY_LOAD(SystemInformation),
#endif
#ifdef ASC_COLLECTOR_NETWORK_ACTIVITY_ENABLED
    COMPONENTS_FACTORY_LOAD(NetworkActivity),
#endif
#ifdef ASC_COLLECTOR_LISTENING_PORTS_ENABLED
    COMPONENTS_FACTORY_LOAD(ListeningPorts),
#endif
#ifdef ASC_COLLECTOR_PROCESS_ENABLED
    COMPONENTS_FACTORY_LOAD(Process),
#endif
    NULL
};

component_load_function_t **components_factory_get_load_array(void)
{
    return component_load_function_array;
}

component_id_t components_factory_create_id(int index)
{
    return (component_id_t)(&g_component_factory[index].component);
}

asc_result_t components_factory_set(const char *name, int index, component_ops_t *ops, bool auto_disable)
{
    if (name == NULL || str_str(name, "_")) {
        log_fatal("Component name=[%s] is wrong (NULL or has '_')", name ? name : "NULL");
        return ASC_RESULT_BAD_ARGUMENT;
    }
    if (components_manager_get_id_by_name(name, str_len(name))) {
        log_fatal("Component name=[%s] already exists", name);
        return ASC_RESULT_BAD_ARGUMENT;
    }
    g_component_factory[index].component.info.state = COMPONENT_LOADED;
    g_component_factory[index].component.info.last_result = ASC_RESULT_OK;
    g_component_factory[index].component.info.log_level = ASC_LOG_LEVEL;
    g_component_factory[index].component.ops = ops;
    g_component_factory[index].component.info.id = components_factory_create_id(index);
    g_component_factory[index].component.info.name = name;
    g_component_factory[index].component.info.enumerator = index;
    g_component_factory[index].component.info.auto_start_disable = auto_disable;
    bit_vector_clean(component_owners_t, &g_component_factory[index].component.info.owners);
    //__auto_generated_self_id = g_component_factory[_component].component.info.id;
    return ASC_RESULT_OK;
}

#ifdef ASC_DYNAMIC_FACTORY_ENABLED
asc_result_t components_factory_set_dynamic_collector(const char *name, int index, component_ops_t *ops, bool auto_disable)
{
    if (index < (__COLLECTOR_COUNT + ASC_EXTRA_COLLECTORS_COUNT) || index >= COLLECTORS_COUNT) {
        log_fatal("Index=[%d] is out of dynamic collectors area", index);
        return ASC_RESULT_MEMORY_EXCEPTION;
    }
    return components_factory_set(name, index, ops, auto_disable);
}

asc_result_t components_factory_set_dynamic_component(const char *name, int index, component_ops_t *ops, bool auto_disable)
{
    if (index < (__COMPONENT_COUNT + ASC_EXTRA_COMPONENTS_COUNT) || index >= COMPONENTS_COUNT) {
        log_fatal("Index=[%d] is out of dynamic components area", index);
        return ASC_RESULT_MEMORY_EXCEPTION;
    }
    return components_factory_set(name, index, ops, auto_disable);
}
#endif

void component_factory_unload(int index)
{
    g_component_factory[index].component.ops = NULL;
    g_component_factory[index].component.info.state = COMPONENT_UNLOADED;
    g_component_factory[index].component.info.id = 0;
    g_component_factory[index].component.info.name = NULL;
    g_component_factory[index].component.info.enumerator = COMPONENTS_COUNT;
    g_component_factory[index].component.info.log_level = ASC_LOG_LEVEL;
    bit_vector_clean(component_owners_t, &g_component_factory[index].component.info.owners);
}

void components_factory_unload(void)
{
    int index;

    for (index = 0; index < COMPONENTS_COUNT; index++) {
        component_factory_unload(index);
    }
}
