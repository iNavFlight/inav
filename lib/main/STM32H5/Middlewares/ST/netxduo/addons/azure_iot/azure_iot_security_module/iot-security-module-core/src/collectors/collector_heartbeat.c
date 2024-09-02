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
#include <string.h>

#include <asc_config.h>

#include "asc_security_core/logger.h"
#include "asc_security_core/components_manager.h"
#include "asc_security_core/core.h"
#include "asc_security_core/object_pool.h"
#include "asc_security_core/utils/itime.h"

static asc_result_t _collector_heartbeat_get_events(collector_internal_t *collector_internal_ptr, serializer_t *serializer);

static asc_result_t _collector_heartbeat_init(component_id_t id);

COLLECTOR_OPS_DEFINITIONS(, _collector_heartbeat_init, collector_default_deinit,
    collector_default_subscribe, collector_default_unsubscribe, collector_default_start, collector_default_stop);

COMPONENTS_FACTORY_DEFINITION(Heartbeat, &_ops)

static asc_result_t _collector_heartbeat_init(component_id_t id)
{
    return collector_default_create(id, Heartbeat, COLLECTOR_PRIORITY_LOW, _collector_heartbeat_get_events, ASC_LOW_PRIORITY_INTERVAL, NULL);
}

static asc_result_t _collector_heartbeat_get_events(collector_internal_t *collector_internal_ptr, serializer_t *serializer)
{
    unsigned long timestamp = itime_time(NULL);
    return serializer_event_add_heartbeat(serializer, timestamp, collector_internal_ptr->interval);
}
