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

#ifndef __COMPONENTS_FACTORY_ENUM_H__
#define __COMPONENTS_FACTORY_ENUM_H__
#include <asc_config.h>

typedef enum {
#ifdef ASC_COLLECTOR_SYSTEM_INFORMATION_ENABLED
    SystemInformation,
#endif
#ifdef ASC_COLLECTOR_NETWORK_ACTIVITY_ENABLED
    NetworkActivity,
#endif
#ifdef ASC_COLLECTOR_LISTENING_PORTS_ENABLED
    ListeningPorts,
#endif
#ifdef ASC_COLLECTOR_HEARTBEAT_ENABLED
    Heartbeat,
#endif
#ifdef ASC_COLLECTOR_BASELINE_ENABLED
    Baseline,
#endif
#ifdef ASC_COLLECTOR_PROCESS_ENABLED
    Process,
#endif
    __COLLECTOR_COUNT // Must be last
} collector_enum_t;

#define COLLECTORS_COUNT (__COLLECTOR_COUNT + ASC_EXTRA_COLLECTORS_COUNT + ASC_DYNAMIC_COLLECTORS_MAX)

/**
 * @brief Factory enumerator of all built-in component.
 * Components factory has following struct:
 * FACTORY_ARRAY[__COLLECTOR_COUNT, ASC_EXTRA_COLLECTORS_COUNT, ASC_DYNAMIC_COLLECTORS_MAX, __COMPONENT_COUNT, ASC_EXTRA_COMPONENTS_COUNT, ASC_DYNAMIC_COMPONENTS_MAX]
 *                                  COLLECTORS_COUNT
 *               |________________________________________________________________________|
 *                                                                                                 COMPONENTS_COUNT
 *               |___________________________________________________________________________________________________________________________________________________|
 */
/**
 * @brief Factory enumerator of collectors. This enumerator comes first in component enum.
 *
 * <Product collectors>:
 * <starts>:    0
 *              CollectorN1
 *              CollectorNx
 * <ends>:      __COLLECTOR_COUNT
 * <Extra collectors>:
 * <starts>:    __COLLECTOR_COUNT
 *              CollectorExtraN1
 *              CollectorExtraNx
 * <ends>:      __COLLECTOR_COUNT + ASC_EXTRA_COLLECTORS_COUNT
 * <Dynamic collectors>:
 * <starts>:    __COLLECTOR_COUNT + ASC_DYNAMIC_COLLECTORS_MAX
 *              CollectorDynamicN1
 *              CollectorDynamicNx
 * <ends>:      COLLECTORS_COUNT = __COLLECTOR_COUNT + ASC_DYNAMIC_COLLECTORS_MAX + ASC_DYNAMIC_COLLECTORS_MAX
 * <Product components>:
 * <starts>:    COLLECTORS_COUNT
 *              ComponentN1
 *              ComponentNx
 * <ends>:      __COMPONENT_COUNT
 * <Extra components>:
 * <starts>:    __COMPONENT_COUNT
 *              ComponentExtraN1
 *              ComponentExtraNx
 * <Dynamic components>:
 * <starts>:    __COMPONENT_COUNT + ASC_EXTRA_COMPONENTS_COUNT
 *              ComponentDynamicN1
 *              ComponentDynamicNx
 * <ends>:      COMPONENTS_COUNT = __COMPONENT_COUNT + ASC_EXTRA_COMPONENTS_COUNT + ASC_DYNAMIC_COMPONENTS_MAX
 *
 * Each component should be defined in 5 places:
 * in @c component_enum_t or @c collector_enum_t enum: ComponentName or CollectorName
 * The enum must contain the following characters only: [a-zA-Z0-9]*
 * in inc/asc_security_core/components_factory_declarations.h: @c COMPONENTS_FACTORY_DECLARATION(ComponentName)
 * in src/component_factory.c:component_load_function_array[]: @c COMPONENTS_FACTORY_LOAD(ComponentName);
 * in component's code: @c COMPONENTS_FACTORY_DEFINITION(ComponentName, &ops)
 * in compilation enviroments: @c -DComponentName
 *
 * @param _component Component generated name from @c component_enum_t
 */
typedef enum {
    ManagerCore = COLLECTORS_COUNT, //first and always exists
#ifdef ASC_COMPONENT_SECURITY_MODULE
    SecurityModule,
#endif
    CollectorsCore,
#ifdef ASC_COMPONENT_CONFIGURATION
    Configuration,
#endif
#ifdef ASC_COMPONENT_CONFIGURATION_PLAT
    ConfigurationPlatform,
#endif
#ifdef ASC_COMPONENT_COMMAND_EXECUTOR_PLAT
    CommandExecutorPlatform,
#endif
#ifdef ASC_COMPONENT_BASELINE_PLAT
    BaselinePlatform,
#endif
#ifdef ASC_COMPONENT_IPC_PLAT
    IpcPlatform,
#endif
#ifdef ASC_COMPONENT_CLI_PLAT
    CliPlatform,
    #ifdef ASC_COMPONENT_DEMO_CLI_PLAT
        CliDemoPlatform,
    #endif
#endif
    Logger,
    __COMPONENT_COUNT // Must be last
} component_enum_t;

#define COMPONENTS_COUNT (__COMPONENT_COUNT + ASC_EXTRA_COMPONENTS_COUNT + ASC_DYNAMIC_COMPONENTS_MAX)

#endif // __COMPONENTS_FACTORY_ENUM_H__
