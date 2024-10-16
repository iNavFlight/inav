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

#ifndef OBJECTS_H
#define OBJECTS_H
#include <asc_config.h>

#ifdef ASC_COLLECTOR_LOG_ENABLED
#include "asc_security_core/model/objects/log.h"
#endif

#ifdef ASC_COLLECTOR_SYSTEM_INFORMATION_ENABLED
#include "asc_security_core/model/objects/system_information.h"
#endif

#ifdef ASC_COLLECTOR_LISTENING_PORTS_ENABLED
#include "asc_security_core/model/objects/listening_ports.h"
#endif

#ifdef ASC_COLLECTOR_NETWORK_ACTIVITY_ENABLED
#include "asc_security_core/model/objects/network_activity.h"
#endif

#ifdef ASC_COLLECTOR_BASELINE_ENABLED
#include "asc_security_core/model/objects/baseline.h"
#include "asc_security_core/model/objects/object_baseline_ext.h"
#endif

#ifdef ASC_COLLECTOR_PROCESS_ENABLED
#include "asc_security_core/model/objects/process.h"
#endif

#endif /* OBJECTS_H */
