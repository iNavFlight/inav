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
#include <stdbool.h>
#include <stdint.h>

#include <asc_config.h>

#include "nx_azure_iot_hub_client.h"
#include "nx_azure_iot_security_module.h"

#include "asc_security_core/logger.h"
#include "asc_security_core/components_manager.h"
#include "asc_security_core/utils/containerof.h"
#include "asc_security_core/utils/notifier.h"
#include "asc_security_core/utils/ievent_loop.h"
#include "asc_security_core/utils/itime.h"
#include "asc_security_core/utils/collection/bit_vector.h"

#include "iot_security_module/mti.h"

BIT_VECTOR_DECLARATIONS(hubs_t, ASC_SECURITY_MODULE_MAX_HUB_DEVICES)

#define AZURE_IOT_SECURITY_MODULE_NAME      "Azure IoT Security Module"
#define AZURE_IOT_SECURITY_MODULE_EVENTS    (NX_CLOUD_MODULE_AZURE_ISM_EVENT | NX_CLOUD_COMMON_PERIODIC_EVENT)

#define MAX_PROPERTY_COUNT  2
static const CHAR *telemetry_headers[MAX_PROPERTY_COUNT][2] = {{MTI_KEY, MTI_VALUE},
                                                               {"%24.ifid", "urn%3Aazureiot%3ASecurity%3ASecurityAgent%3A1"}};

static unsigned long _security_module_unix_time_get(unsigned long *unix_time);
static VOID _security_module_event_process(VOID *security_module_ptr, ULONG common_events, ULONG module_own_events);
static void _security_module_event_process_state_pending(NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr);
static void _security_module_event_process_state_active(NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr);
static void _security_module_event_process_state_suspended(NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr);
static UINT _security_module_message_send(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, security_message_t *security_message_ptr);
static void _security_module_update_state(NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr, security_module_state_t state);
static bool _security_module_exists_connected_iot_hub(NX_AZURE_IOT *nx_azure_iot_ptr);
static void _security_module_state_machine(NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr);
static void _security_module_state_machine_cb(event_loop_timer_handler h, void *ctx);
static void _security_module_state_machine_schedule(NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr, unsigned long delay);
static UINT _security_module_get_nx_status(UINT current);
static void _security_module_message_ready_cb(notifier_t *notifier, int msg, void *payload);
static NX_AZURE_IOT_HUB_CLIENT *_security_module_get_connected_hub_client(NX_AZURE_IOT_RESOURCE *resource_ptr);
static bool _is_skip_resource(
    NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr,
    NX_AZURE_IOT_RESOURCE *resource_ptr,
    NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
    bit_vector_hubs_t *send_bitmap_vector,
    bool send_failed_once_over_map_limit);

static bool _initialized;

static asc_result_t _cm_init(component_id_t id);
static asc_result_t _cm_deinit(component_id_t id);
static asc_result_t _cm_start(component_id_t id);
static component_ops_t _ops = {
    .init = _cm_init,
    .start = _cm_start,
    .deinit = _cm_deinit
};

COMPONENTS_FACTORY_DEFINITION(SecurityModule, &_ops)

static asc_result_t _cm_start(component_id_t id)
{
    NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr = components_manager_get_self_ctx();

    if (!security_module_ptr) {
        return ASC_RESULT_MEMORY_EXCEPTION;
    }
    /* Set security module state as pending. */
    _security_module_update_state(security_module_ptr, SECURITY_MODULE_STATE_PENDING);
    _security_module_state_machine(security_module_ptr);

    return ASC_RESULT_OK;
}

static asc_result_t _cm_init(component_id_t id)
{
    asc_result_t result = ASC_RESULT_OK;
    component_info_t *info;
    UINT status = NX_AZURE_IOT_SUCCESS;
    static NX_AZURE_IOT_SECURITY_MODULE _security_module;

    memset(&_security_module, 0, sizeof(NX_AZURE_IOT_SECURITY_MODULE));
    components_manager_set_self_ctx(&_security_module);

    /* Persist the nx_azure_iot_ptr. */
    _security_module.nx_azure_iot_ptr = (NX_AZURE_IOT *)components_manager_global_context_get();

    /* Register Azure IoT Security Module on cloud helper.  */
    if ((status = nx_cloud_module_register(
        &(_security_module.nx_azure_iot_ptr->nx_azure_iot_cloud),
        &(_security_module.nx_azure_iot_security_module_cloud),
        AZURE_IOT_SECURITY_MODULE_NAME,
        AZURE_IOT_SECURITY_MODULE_EVENTS,
        _security_module_event_process,
        &_security_module
    )))
    {
        LogError(LogLiteralArgs("Security module register fail, error=%d"), status);
        goto cleanup;
    }

    _security_module.message_ready.notify = _security_module_message_ready_cb;
    result = notifier_subscribe(NOTIFY_TOPIC_COLLECT, &_security_module.message_ready);
    _security_module.state = SECURITY_MODULE_STATE_NOT_INITIALIZED;

cleanup:
    /* Store NX status to be able return right result from nx_azure_iot_security_module_enable() */
    info = components_manager_get_info(components_manager_get_self_id());
    if (info)
    {
        info->ext_ctx = (uintptr_t)status;
    }
    if (status != NX_AZURE_IOT_SUCCESS)
    {
        LogError(LogLiteralArgs("Failed to init Azure IoT Security Module component, error=%d"), status);
        return ASC_RESULT_EXCEPTION;
    }
    return result;
}

static asc_result_t _cm_deinit(component_id_t id)
{
    UINT status = NX_AZURE_IOT_SUCCESS;
    NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr = components_manager_get_self_ctx();

    if (!security_module_ptr) {
        return ASC_RESULT_MEMORY_EXCEPTION;
    }
    ievent_loop_get_instance()->timer_delete(security_module_ptr->h_state_machine);

    notifier_unsubscribe(NOTIFY_TOPIC_COLLECT, &security_module_ptr->message_ready);

    /* Deregister Azure IoT Security Module from cloud helper.  */
    if (security_module_ptr->nx_azure_iot_ptr != NULL)
    {
        if ((status = nx_cloud_module_deregister(
                    &(security_module_ptr->nx_azure_iot_ptr->nx_azure_iot_cloud),
                    &(security_module_ptr->nx_azure_iot_security_module_cloud)
        )))
        {
            LogError(LogLiteralArgs("Failed to deregister Azure IoT Security Module, error=%d"), status);
            status = NX_AZURE_IOT_FAILURE;
        }
    }
    components_manager_set_self_ctx(NULL);
    if (status != NX_AZURE_IOT_SUCCESS)
    {
        LogError(LogLiteralArgs("Failed to deinit Azure IoT Security Module component, error=%d"), status);
        return ASC_RESULT_EXCEPTION;
    }
    return ASC_RESULT_OK;
}

UINT nx_azure_iot_security_module_enable(NX_AZURE_IOT *nx_azure_iot_ptr)
{
    UINT status = NX_AZURE_IOT_SUCCESS;
    asc_result_t result = ASC_RESULT_OK;
    ievent_loop_t *event_loop = ievent_loop_get_instance();
    ULONG t = (ULONG)-1;
    NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr = NULL;

    /* Check if Security Module instance is already been initialized. */
    if (_initialized)
    {
        security_module_ptr = components_manager_get_self_ctx();
        if (security_module_ptr == NULL)
        {
            LogError(LogLiteralArgs("Security Module is not running"));
            status = NX_AZURE_IOT_FAILURE;
            goto cleanup;
        }
        if (security_module_ptr->nx_azure_iot_ptr != nx_azure_iot_ptr)
        {
            LogError(LogLiteralArgs("Multiple initializing with different nx_azure_iot_ptr"));
            status = NX_AZURE_IOT_INVALID_PARAMETER;
            goto cleanup;
        }
        goto cleanup;
    }

    if (nx_azure_iot_ptr == NULL)
    {
        status = NX_AZURE_IOT_INVALID_PARAMETER;
        goto cleanup;
    }

    if (nx_azure_iot_ptr->nx_azure_iot_unix_time_get(&t) != NX_AZURE_IOT_SUCCESS)
    {
        status = NX_AZURE_IOT_FAILURE;
        LogError(LogLiteralArgs("Failed to retrieve UNIX time"));
        goto cleanup;
    }
    if (t == (ULONG)-1)
    {
        status = NX_AZURE_IOT_FAILURE;
        LogError(LogLiteralArgs("Failed to retrieve UNIX time"));
        goto cleanup;
    }

    itime_init((unix_time_callback_t)_security_module_unix_time_get);

    if (event_loop == NULL)
    {
        /* Should never happen */
        status = NX_AZURE_IOT_FAILURE;
        LogError(LogLiteralArgs("Failed to retrieve event loop"));
        goto cleanup;
    }

    event_loop->init();
    components_manager_global_context_set((uintptr_t)nx_azure_iot_ptr);

    result = components_manager_init();
    switch (result)
    {
    case ASC_RESULT_INITIALIZED:
        goto cleanup;
        break;
    case ASC_RESULT_OK:
        break;
    default:
        status = NX_AZURE_IOT_FAILURE;
        LogError(LogLiteralArgs("Failed to init component manager"));
        goto cleanup;
        break;
    }
    
    security_module_ptr = components_manager_get_self_ctx();
    if (security_module_ptr == NULL)
    {
        LogError(LogLiteralArgs("Failed to init Security Module"));
        status = NX_AZURE_IOT_FAILURE;
        goto cleanup;
    }
    status = _security_module_get_nx_status(status);
    
cleanup:
    if (status != NX_AZURE_IOT_SUCCESS)
    {
        LogError(LogLiteralArgs("Failed to enable Azure IoT Security Module, error=%d"), status);

        /* Destroy Security Module instance */
        nx_azure_iot_security_module_disable(nx_azure_iot_ptr);
    }
    else
    {
        _initialized = true;
        LogInfo(LogLiteralArgs("Azure IoT Security Module has been enabled, status=%d"), status);
    }

    return status;
}

UINT nx_azure_iot_security_module_disable(NX_AZURE_IOT *nx_azure_iot_ptr)
{
    UINT status = NX_AZURE_IOT_SUCCESS;
    ievent_loop_t *event_loop = ievent_loop_get_instance();
    NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr = components_manager_get_self_ctx();

    if (security_module_ptr == NULL)
    {
        LogInfo(LogLiteralArgs("Security Module is not runnung"));
        goto cleanup;
    }
    if (security_module_ptr->nx_azure_iot_ptr != nx_azure_iot_ptr && nx_azure_iot_ptr != NULL)
    {
        LogError(LogLiteralArgs("Disabling with wrong nx_azure_iot_ptr"));
        status = NX_AZURE_IOT_INVALID_PARAMETER;
        goto cleanup;
    }
    _initialized = false;
    security_module_ptr->state = SECURITY_MODULE_STATE_NOT_INITIALIZED;
    components_manager_deinit();
    if (event_loop != NULL) /* (event_loop == NULL) - Should never happen */
    {
        event_loop->stop();
        event_loop->deinit(true);
    }

cleanup:
    if (status != NX_AZURE_IOT_SUCCESS)
    {
        LogError(LogLiteralArgs("Failed to disable Azure IoT Security Module, error=%d"), status);
    }
    else
    {
        LogInfo(LogLiteralArgs("Azure IoT Security Module has been disabled, status=%d"), status);
    }

    return status;
}

static void _security_module_message_ready_cb(notifier_t *notifier, int msg, void *payload)
{
    NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr = containerof(notifier, NX_AZURE_IOT_SECURITY_MODULE, message_ready);
 
    if (security_module_ptr == NULL)
    {
        /* Should never happen */
        LogError(LogLiteralArgs("Azure IoT Security Module component is not initialized"));
        return;
    }
    _security_module_state_machine(security_module_ptr);
}

static unsigned long _security_module_unix_time_get(unsigned long *unix_time)
{
    ULONG t;
    NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr = components_manager_get_self_ctx();

    if (security_module_ptr == NULL || security_module_ptr->nx_azure_iot_ptr == NULL)
    {
        return ITIME_FAILED;
    }

    if (security_module_ptr->nx_azure_iot_ptr->nx_azure_iot_unix_time_get(&t) == NX_AZURE_IOT_SUCCESS)
    {
        if (unix_time != NULL)
        {
            *unix_time = (unsigned long)t;
        }

        return (unsigned long)t;
    }

    return ITIME_FAILED;
}

static UINT _security_module_get_nx_status(UINT current)
{
    UINT status = current;
    asc_result_t result = ASC_RESULT_OK;

    if (current != NX_AZURE_IOT_SUCCESS)
    {
        return current;
    }
    result = components_manager_get_last_result(components_manager_get_self_id());
    if (result != ASC_RESULT_OK)
    {
        status = NX_AZURE_IOT_FAILURE;
        component_info_t *info = components_manager_get_info(components_manager_get_self_id());
        if (info && (UINT)info->ext_ctx != NX_AZURE_IOT_SUCCESS)
        {
            status = (UINT)info->ext_ctx;
        }
    }
    return status;
}

static void _security_module_state_machine(NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr)
{
    ievent_loop_get_instance()->timer_delete(security_module_ptr->h_state_machine);
    switch(security_module_ptr->state)
    {
        case SECURITY_MODULE_STATE_NOT_INITIALIZED:
            /* Cannot occurred. */
            break;
        case SECURITY_MODULE_STATE_PENDING:
            _security_module_event_process_state_pending(security_module_ptr);
            break;
        case SECURITY_MODULE_STATE_ACTIVE:
            _security_module_event_process_state_active(security_module_ptr);
            break;
        case SECURITY_MODULE_STATE_SUSPENDED:
            _security_module_event_process_state_suspended(security_module_ptr);
            break;
        default:
            LogError(LogLiteralArgs("Unsupported Security Module state=%d"), security_module_ptr->state);
            break;
    }
}

static void _security_module_state_machine_cb(event_loop_timer_handler h, void *ctx)
{
    NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr = (NX_AZURE_IOT_SECURITY_MODULE *)ctx;

    ievent_loop_get_instance()->timer_delete(h);
    _security_module_state_machine(security_module_ptr);
}

static VOID _security_module_event_process(VOID *ctx, ULONG common_events, ULONG module_own_events)
{
    UINT status = NX_AZURE_IOT_SUCCESS;
    NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr = (NX_AZURE_IOT_SECURITY_MODULE*)ctx;

    NX_PARAMETER_NOT_USED(module_own_events);

    /* Process common events. */
    if (common_events & NX_CLOUD_COMMON_PERIODIC_EVENT)
    {
        if (security_module_ptr == NULL)
        {
            /* Periodic events must use instance of security module. */
            status = NX_AZURE_IOT_INVALID_PARAMETER;
            LogError(LogLiteralArgs("Security Module process periodic events must receive an instance, status=%d"), status);
            goto error;
        }
        ievent_loop_get_instance()->run_once();
    }

error:
    if (status != NX_AZURE_IOT_SUCCESS)
    {
        LogError(LogLiteralArgs("Security Module process periodic events finished with an error, status=%d"), status);
    }
}

static void _security_module_state_machine_schedule(NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr, unsigned long delay)
{
    ievent_loop_get_instance()->timer_delete(security_module_ptr->h_state_machine);
    security_module_ptr->h_state_machine = ievent_loop_get_instance()->timer_create(
        _security_module_state_machine_cb, security_module_ptr,
        delay,
        0, 
        &security_module_ptr->h_state_machine
    );
}

static NX_AZURE_IOT_HUB_CLIENT *_security_module_get_connected_hub_client(NX_AZURE_IOT_RESOURCE *resource_ptr)
{
    NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr = NULL;

    hub_client_ptr = (NX_AZURE_IOT_HUB_CLIENT *)resource_ptr->resource_data_ptr;
    if (hub_client_ptr == NULL)
    {
        return NULL;
    }
    /* Filter only connected IoT Hubs. */
    if (hub_client_ptr->nx_azure_iot_hub_client_state != NX_AZURE_IOT_HUB_CLIENT_STATUS_CONNECTED)
    {
        return NULL;
    }
    return hub_client_ptr;
}

static bool _is_skip_resource(
    NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr,
    NX_AZURE_IOT_RESOURCE *resource_ptr,
    NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr,
    bit_vector_hubs_t *send_bitmap_vector,
    bool send_failed_once_over_map_limit)
{
    int prev_hub_index = -1;

    /* Iterate over previous seen IoT Hub resources and send Security Message only for unique devices. */
    for (NX_AZURE_IOT_RESOURCE *prev_resource_ptr = security_module_ptr->nx_azure_iot_ptr->nx_azure_iot_resource_list_header;
            prev_resource_ptr != resource_ptr;
            prev_resource_ptr = prev_resource_ptr->resource_next)
    {
        if (prev_resource_ptr->resource_type != NX_AZURE_IOT_RESOURCE_IOT_HUB)
        {
            continue;
        }
        NX_AZURE_IOT_HUB_CLIENT *prev_hub_client_ptr = (NX_AZURE_IOT_HUB_CLIENT *)prev_resource_ptr->resource_data_ptr;
        prev_hub_index++;
        bool was_send;
        if (prev_hub_index < bit_vector_size(hubs_t))
        {
            was_send = bit_vector_get(hubs_t, send_bitmap_vector, prev_hub_index);
        }
        else
        {
            LogError(LogLiteralArgs("Hub index %d is over max supported history hubs %d - broadcast will send - change ASC_SECURITY_MODULE_MAX_HUB_DEVICES config"), prev_hub_index+1, ASC_SECURITY_MODULE_MAX_HUB_DEVICES);
            was_send = !send_failed_once_over_map_limit;
        }

        if (was_send &&
            az_span_is_content_equal(
                hub_client_ptr->iot_hub_client_core._internal.iot_hub_hostname,
                prev_hub_client_ptr->iot_hub_client_core._internal.iot_hub_hostname) &&
            az_span_is_content_equal(
                hub_client_ptr->iot_hub_client_core._internal.device_id,
                prev_hub_client_ptr->iot_hub_client_core._internal.device_id
        ))
        {
            return true;
        }
    }
    return false;
}

static void _security_module_event_process_state_pending(NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr)
{
    unsigned long now_timestamp;
    unsigned long delay = ASC_SECURITY_MODULE_SEND_MESSAGE_RETRY_TIME;

    /* Reevaluate Security Module State */
    if (_security_module_exists_connected_iot_hub(security_module_ptr->nx_azure_iot_ptr))
    {
        /* Security Module is able to send security messages. */

        /* Update security Module state to active. */
        delay = 0;
        _security_module_update_state(security_module_ptr, SECURITY_MODULE_STATE_ACTIVE);
        goto cleanup;
    }

     /* Get current timestamp. */
    if (itime_time(&now_timestamp) == ITIME_FAILED)
    {
        LogError(LogLiteralArgs("Failed to retrieve timestamp"));
    }

    if (security_module_ptr->state_timestamp != ITIME_FAILED &&
        now_timestamp != ITIME_FAILED &&
        now_timestamp - security_module_ptr->state_timestamp > ASC_SECURITY_MODULE_PENDING_TIME)
    {
        /* Security Module pending state time expired. */

        /* Update security Module state to suspend. */
        _security_module_update_state(security_module_ptr, SECURITY_MODULE_STATE_SUSPENDED);
    }

cleanup:
    _security_module_state_machine_schedule(security_module_ptr, delay);
}

static void _security_module_event_process_state_active(NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr)
{
    asc_result_t result = ASC_RESULT_OK;
    security_message_t security_message = { 0 };
    NX_AZURE_IOT_RESOURCE *resource_ptr;

    result = core_message_get(&security_message);
    if (result == ASC_RESULT_EMPTY)
    {
        LogInfo(LogLiteralArgs("Azure IoT Security Module message is empty"));
        return;
    }
    if (result != ASC_RESULT_OK)
    {
        LogError(LogLiteralArgs("Fail to get security message result=%d"), result);
        core_message_deinit();
        return;
    }

    /* If exists at least one connected IoT Hub, Security Module will remain in active state. */
    bool exists_connected_iot_hub = false;
    /* If was failure on send message on hub number > 64, we want to try to send to all connected hubs. */
    bool send_failed_once_over_map_limit = false;
    /* If was not passed on send message, we want to retry to send in short time. */
    bool send_passed_once = false;

    bit_vector_hubs_t send_bitmap_vector;
    memset(&send_bitmap_vector, 0, sizeof(bit_vector_hubs_t));

    int hub_index = -1;

    /* Iterate over all NX_AZURE_IOT_HUB_CLIENT instances. */
    for (resource_ptr = security_module_ptr->nx_azure_iot_ptr->nx_azure_iot_resource_list_header;
        resource_ptr != NX_NULL;
        resource_ptr = resource_ptr->resource_next)
    {
        if (resource_ptr->resource_type != NX_AZURE_IOT_RESOURCE_IOT_HUB)
        {
            continue;
        }
        hub_index++;
        NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr = _security_module_get_connected_hub_client(resource_ptr);
        if (hub_client_ptr == NULL)
        {
            continue;
        }
        exists_connected_iot_hub = true;

        /*
            Skip resource iff a security message is already been sent to this specific device. Avoid
            sending security message to a Device Identity and to his Module Identities if both connected.
        */
        bool skip_resource = _is_skip_resource(security_module_ptr, resource_ptr, hub_client_ptr, &send_bitmap_vector, send_failed_once_over_map_limit);

        if (!skip_resource)
        {
            UINT status = _security_module_message_send(hub_client_ptr, &security_message);
            if (status != NX_AZURE_IOT_SUCCESS)
            {
                if (hub_index >= bit_vector_size(hubs_t))
                {
                    send_failed_once_over_map_limit = true;
                }
                LogError(LogLiteralArgs("Failed to send security message, error=%d"), status);
            }
            else
            {
                bit_vector_set(hubs_t, &send_bitmap_vector, hub_index, true);
                send_passed_once = true;
            }
        }
    }
    if (!exists_connected_iot_hub)
    {
        _security_module_update_state(security_module_ptr, SECURITY_MODULE_STATE_PENDING);
        _security_module_state_machine_schedule(security_module_ptr, ASC_SECURITY_MODULE_SEND_MESSAGE_RETRY_TIME);
    }
    else
    {
        if (!send_passed_once)
        {
            _security_module_state_machine_schedule(security_module_ptr, ASC_SECURITY_MODULE_SEND_MESSAGE_RETRY_TIME);
        }
        else
        {
            core_message_deinit();
        }
    }
}

static void _security_module_event_process_state_suspended(NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr)
{
    unsigned long delay = ASC_SECURITY_MODULE_SEND_MESSAGE_RETRY_TIME;

    /* Reevaluate Security Module State */
    if (_security_module_exists_connected_iot_hub(security_module_ptr->nx_azure_iot_ptr))
    {
        /* Security Module is able to send security messages. */

        /* Update security Module state to active. */
        delay = 0;
        _security_module_update_state(security_module_ptr, SECURITY_MODULE_STATE_ACTIVE);
        goto cleanup;
    }

cleanup:
    _security_module_state_machine_schedule(security_module_ptr, delay);
}

static UINT _security_module_message_send(NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr, security_message_t *security_message_ptr)
{
    UINT status = NX_AZURE_IOT_SUCCESS;
    NX_PACKET *packet_ptr = NULL;

    /* Create a telemetry message packet. */
    if ((status = nx_azure_iot_hub_client_telemetry_message_create(hub_client_ptr,
                                                                &packet_ptr,
                                                                NX_NO_WAIT)))
    {
        LogError(LogLiteralArgs("Security Message telemetry message create failed, error=%d"), status);
        return status;
    }

    /* Add properties to telemetry message. */
    for (int index = 0; index < MAX_PROPERTY_COUNT; index++)
    {
        if ((status =
                nx_azure_iot_hub_client_telemetry_property_add(packet_ptr,
                                                            (UCHAR*)telemetry_headers[index][0],
                                                            (USHORT)strlen(telemetry_headers[index][0]),
                                                            (UCHAR *)telemetry_headers[index][1],
                                                            (USHORT)strlen(telemetry_headers[index][1]),
                                                            NX_NO_WAIT)))
        {
            LogError(LogLiteralArgs("Failed to add telemetry property, error=%d"), status);

            /* Remove resources. */
            nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);

            return status;
        }
    }

    UCHAR *data = security_message_ptr->data;
    UINT data_length = (UINT)security_message_ptr->size;

    if ((status = nx_azure_iot_hub_client_telemetry_send(hub_client_ptr,
                                                         packet_ptr,
                                                         data,
                                                         data_length,
                                                         NX_NO_WAIT)))
    {
        LogError(LogLiteralArgs("Failed to send Security Message, error=%d"), status);

        /* Delete telemetry message */
        nx_azure_iot_hub_client_telemetry_message_delete(packet_ptr);
    }
    else
    {
        /* packet_ptr will be released `nx_azure_iot_hub_client_telemetry_send`. */
        LogDebug(LogLiteralArgs("Security Message has been sent successfully"));
    }

    return status;
}

static int _state2notify(security_module_state_t state)
{
    switch(state)
    {
        case SECURITY_MODULE_STATE_PENDING:
            return NOTIFY_SECURITY_MODULE_PENDING;
        case SECURITY_MODULE_STATE_ACTIVE:
            return NOTIFY_SECURITY_MODULE_CONNECTED;
        case SECURITY_MODULE_STATE_SUSPENDED:
            return NOTIFY_SECURITY_MODULE_SUSPENDED;
        default:
            LogError(LogLiteralArgs("Unsupported Security Module state=%d"), state);
            return -1;
    }
}

static void _security_module_update_state(NX_AZURE_IOT_SECURITY_MODULE *security_module_ptr, security_module_state_t state)
{
    unsigned long now_timestamp;

    if (security_module_ptr->state == state)
    {
        /* Security Module is already set to given state. */
        return;
    }

    /* Set security module state timestamp. */
    if (itime_time(&now_timestamp) == ITIME_FAILED)
    {
        LogError(LogLiteralArgs("Failed to retrive time"));
    }
    else
    {
        security_module_ptr->state_timestamp = now_timestamp;
    }

    /* Set security module state. */
    security_module_ptr->state = state;

    int notification_enum = _state2notify(state);
    if (notification_enum >= 0)
    {
        notifier_notify(NOTIFY_TOPIC_SECURITY_MODULE_STATE, _state2notify(state), security_module_ptr);
    }
}


static bool _security_module_exists_connected_iot_hub(NX_AZURE_IOT *nx_azure_iot_ptr)
{
    NX_AZURE_IOT_RESOURCE *resource_ptr;

    /* Iterate over all NX_AZURE_IOT_HUB_CLIENT instances. */
    for (resource_ptr = nx_azure_iot_ptr->nx_azure_iot_resource_list_header;
        resource_ptr != NX_NULL;
        resource_ptr = resource_ptr->resource_next)
    {
        if (resource_ptr->resource_type == NX_AZURE_IOT_RESOURCE_IOT_HUB)
        {
            /* Check IoT Hub client connectivity. */
            NX_AZURE_IOT_HUB_CLIENT *hub_client_ptr = _security_module_get_connected_hub_client(resource_ptr);

            if (hub_client_ptr != NULL)
            {
                return true;
            }
        }
    }

    return false;
}