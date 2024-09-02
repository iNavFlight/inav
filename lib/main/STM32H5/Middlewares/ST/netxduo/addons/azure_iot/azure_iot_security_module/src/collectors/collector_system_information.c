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

#include <asc_config.h>

#include "nx_api.h"
#include "tx_api.h"

#include "asc_security_core/logger.h"
#include "asc_security_core/collector.h"
#include "asc_security_core/components_factory_declarations.h"
#include "asc_security_core/components_manager.h"
#include "asc_security_core/serializer.h"
#include "asc_security_core/utils/itime.h"
#include "asc_security_core/utils/notifier.h"
#include "asc_security_core/utils/num2str.h"

#define OS_NAME "Azure RTOS "
#define OS_INFO OS_NAME NUM_2_STR(THREADX_MAJOR_VERSION) "." NUM_2_STR(THREADX_MINOR_VERSION)

static asc_result_t _collector_system_information_serialize_events(collector_internal_t *collector_internal_ptr, serializer_t *serializer);
static asc_result_t _collect_operation_system_information(collector_internal_t *collector_internal_ptr, system_information_t *data_ptr);

static asc_result_t _cm_init(component_id_t id);

COLLECTOR_OPS_DEFINITIONS(, _cm_init, collector_default_deinit,
    collector_default_subscribe, collector_default_unsubscribe, collector_default_start, collector_default_stop);

COMPONENTS_FACTORY_DEFINITION(SystemInformation, &_ops)

static asc_result_t _cm_init(component_id_t id)
{
    return collector_default_create(id, SystemInformation, COLLECTOR_PRIORITY_LOW,
        _collector_system_information_serialize_events, ASC_LOW_PRIORITY_INTERVAL, NULL);
}

static asc_result_t _collector_system_information_serialize_events(collector_internal_t *collector_internal_ptr, serializer_t *serializer)
{
    asc_result_t result = ASC_RESULT_OK;
    system_information_t system_information;
    unsigned long current_time;
    
    memset(&system_information, 0, sizeof(system_information_t));

    log_debug("Start _collector_system_information_serialize_events");

    result = _collect_operation_system_information(collector_internal_ptr, &system_information);
    if (result != ASC_RESULT_OK)
    {
        log_error("Failed to collect Operation System information, result=[%d]", result);
        goto cleanup;
    }

    current_time = itime_time(NULL);
    result = serializer_event_add_system_information(serializer, current_time, collector_internal_ptr->interval, &system_information);

cleanup:
    if (result != ASC_RESULT_OK)
    {
        log_error("failed to collect events");
    }

    log_debug("Done _collector_system_information_serialize_events, result=[%d]", result);
    return result;
}

static asc_result_t _collect_operation_system_information(collector_internal_t *collector_internal_ptr, system_information_t *data_ptr)
{
    log_debug("Start _collect_operation_system_information");
    data_ptr->os_info = OS_INFO;

    log_debug("Done _collect_operation_system_information");

    return ASC_RESULT_OK;
}
