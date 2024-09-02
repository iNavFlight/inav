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

#ifndef __COMPONENTS_FACTORY_DECLARATIONS_H__
#define __COMPONENTS_FACTORY_DECLARATIONS_H__
#include <asc_config.h>

#include "asc_security_core/components_factory.h"

COMPONENTS_FACTORY_DECLARATION(ManagerCore)
#ifdef ASC_COMPONENT_SECURITY_MODULE
    COMPONENTS_FACTORY_DECLARATION(SecurityModule)
#endif
COMPONENTS_FACTORY_DECLARATION(CollectorsCore)
COMPONENTS_FACTORY_DECLARATION(Logger)
#ifdef ASC_COMPONENT_CONFIGURATION
    COMPONENTS_FACTORY_DECLARATION(Configuration)
#endif
#ifdef ASC_COMPONENT_CONFIGURATION_PLAT
    COMPONENTS_FACTORY_DECLARATION(ConfigurationPlatform)
#endif
#ifdef ASC_COMPONENT_COMMAND_EXECUTOR_PLAT
    COMPONENTS_FACTORY_DECLARATION(CommandExecutorPlatform)
#endif
#ifdef ASC_COLLECTOR_HEARTBEAT_ENABLED
    COMPONENTS_FACTORY_DECLARATION(Heartbeat)
#endif
#ifdef ASC_COLLECTOR_BASELINE_ENABLED
    COMPONENTS_FACTORY_DECLARATION(Baseline)
#endif
#ifdef ASC_COMPONENT_BASELINE_PLAT
    COMPONENTS_FACTORY_DECLARATION(BaselinePlatform)
#endif
#ifdef ASC_COMPONENT_IPC_PLAT
    COMPONENTS_FACTORY_DECLARATION(IpcPlatform)
#endif
#ifdef ASC_COMPONENT_CLI_PLAT
    #ifdef ASC_COMPONENT_DEMO_CLI_PLAT
        COMPONENTS_FACTORY_DECLARATION(CliDemoPlatform)
    #endif
    COMPONENTS_FACTORY_DECLARATION(CliPlatform)
#endif
#ifdef ASC_COLLECTOR_SYSTEM_INFORMATION_ENABLED
    COMPONENTS_FACTORY_DECLARATION(SystemInformation)
#endif
#ifdef ASC_COLLECTOR_NETWORK_ACTIVITY_ENABLED
    COMPONENTS_FACTORY_DECLARATION(NetworkActivity)
#endif
#ifdef ASC_COLLECTOR_LISTENING_PORTS_ENABLED
    COMPONENTS_FACTORY_DECLARATION(ListeningPorts)
#endif
#ifdef ASC_COLLECTOR_PROCESS_ENABLED
    COMPONENTS_FACTORY_DECLARATION(Process)
#endif

#endif // __COMPONENTS_FACTORY_DECLARATIONS_H__