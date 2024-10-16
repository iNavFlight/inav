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
#include <asc_config.h>

#include "asc_security_core/logger.h"
#include "asc_security_core/model/schema/event_builder.h"
#include "asc_security_core/serializer.h"

#include "protocol_serialize_enum.h"
#include "serializer_private.h"

/**
 *
 **/
static asc_result_t _serializer_network_activity_add_ipv4_activity(serializer_t *serializer, network_activity_ipv4_t *ipv4_payload);
static asc_result_t _serializer_network_activity_add_ipv6_activity(serializer_t *serializer, network_activity_ipv6_t *ipv6_payload);
static asc_result_t _serializer_network_activity_add_common(serializer_t *serializer, network_activity_common_t *common);

asc_result_t serializer_event_add_network_activity(serializer_t *serializer, unsigned long timestamp, unsigned long collection_interval,
                                                         network_activity_ipv4_t *ipv4_payload, network_activity_ipv6_t *ipv6_payload)
{
    log_debug("serializer_event_add_network_activity, serializer=[%p], timestamp=[%lu], collection_interval=[%lu], ipv4_payload=[%p], ipv6_payload=[%p]",
              (void*)serializer, timestamp, collection_interval, (void*)ipv4_payload, (void*)ipv6_payload);

#ifndef ASC_COLLECTOR_NETWORK_ACTIVITY_SEND_EMPTY_EVENTS
    if (ipv4_payload == NULL && ipv6_payload == NULL) {
        log_debug("skipping empty network activity event");
        return ASC_RESULT_OK;
    }
#endif

    if (serializer == NULL) {
        log_error("failed, bad argument");
        return ASC_RESULT_BAD_ARGUMENT;
    }

    if (serializer->state != SERIALIZER_STATE_MESSAGE_EMPTY && serializer->state != SERIALIZER_STATE_MESSAGE_PROCESSING) {
        log_error("failed, state=[%d]", serializer->state);
        return ASC_RESULT_EXCEPTION;
    }

    if (serializer_event_start(serializer, timestamp, collection_interval) != ASC_RESULT_OK) {
        return ASC_RESULT_EXCEPTION;
    }

    if (flatbuffers_failed(AzureIoTSecurity_Event_payload_NetworkActivity_start(&serializer->builder))) {
        log_error("failed in AzureIoTSecurity_Event_payload_NetworkActivity_start");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    if (ipv4_payload != NULL && _serializer_network_activity_add_ipv4_activity(serializer, ipv4_payload) != ASC_RESULT_OK) {
        log_error("failed in _serializer_network_activity_add_ipv4_activity");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    if (ipv6_payload != NULL && _serializer_network_activity_add_ipv6_activity(serializer, ipv6_payload) != ASC_RESULT_OK) {
        log_error("failed in _serializer_network_activity_add_ipv6_activity");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    if (flatbuffers_failed(AzureIoTSecurity_Event_payload_NetworkActivity_end(&serializer->builder))) {
        log_error("failed in AzureIoTSecurity_Event_payload_NetworkActivity_end");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    if (AzureIoTSecurity_Event_vec_push_end(&serializer->builder) == NULL) {
        log_error("failed in AzureIoTSecurity_Event_vec_push_end");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    serializer->state = SERIALIZER_STATE_MESSAGE_PROCESSING;

    return ASC_RESULT_OK;
}


static asc_result_t _serializer_network_activity_add_ipv4_activity(serializer_t *serializer, network_activity_ipv4_t *ipv4_payload)
{
    if (flatbuffers_failed(AzureIoTSecurity_NetworkActivity_ipv4_activity_start(&serializer->builder))) {
        log_error("failed in AzureIoTSecurity_NetworkActivity_ipv4_activity_start");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    do {
        if (flatbuffers_failed(AzureIoTSecurity_NetworkActivityV4_vec_push_start(&serializer->builder))) {
            log_error("failed in AzureIoTSecurity_NetworkActivityV4_vec_push_start");
            serializer->state = SERIALIZER_STATE_EXCEPTION;
            return ASC_RESULT_EXCEPTION;
        }

        if (flatbuffers_failed(AzureIoTSecurity_NetworkActivityV4_addresses_create(&serializer->builder,
                                                                ipv4_payload->local_address, ipv4_payload->remote_address))) {
            log_error("failed in AzureIoTSecurity_NetworkActivityV4_addresses_create");
            serializer->state = SERIALIZER_STATE_EXCEPTION;
            return ASC_RESULT_EXCEPTION;

        }

        if (flatbuffers_failed(AzureIoTSecurity_NetworkActivityV4_common_start(&serializer->builder))) {
            log_error("failed in AzureIoTSecurity_NetworkActivityV4_common_start");
            serializer->state = SERIALIZER_STATE_EXCEPTION;
            return ASC_RESULT_EXCEPTION;
        }

        if (_serializer_network_activity_add_common(serializer, &ipv4_payload->common) != ASC_RESULT_OK) {
            return ASC_RESULT_EXCEPTION;
        }

        if (flatbuffers_failed(AzureIoTSecurity_NetworkActivityV4_common_end(&serializer->builder))) {
            log_error("failed in AzureIoTSecurity_NetworkActivityV4_common_end");
            serializer->state = SERIALIZER_STATE_EXCEPTION;
            return ASC_RESULT_EXCEPTION;
        }

        if (AzureIoTSecurity_NetworkActivityV4_vec_push_end(&serializer->builder) == NULL) {
            log_error("failed in AzureIoTSecurity_NetworkActivityV4_vec_push_end");
            serializer->state = SERIALIZER_STATE_EXCEPTION;
            return ASC_RESULT_EXCEPTION;
        }

        ipv4_payload = ipv4_payload->next;
    } while (ipv4_payload != NULL);

    if (flatbuffers_failed(AzureIoTSecurity_NetworkActivity_ipv4_activity_end(&serializer->builder))) {
        log_error("failed in AzureIoTSecurity_NetworkActivity_ipv4_activity_end");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    return ASC_RESULT_OK;
}


static asc_result_t _serializer_network_activity_add_ipv6_activity(serializer_t *serializer, network_activity_ipv6_t *ipv6_payload)
{
    if (flatbuffers_failed(AzureIoTSecurity_NetworkActivity_ipv6_activity_start(&serializer->builder))) {
        log_error("failed in AzureIoTSecurity_NetworkActivity_ipv6_activity_start");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    do {
        if (flatbuffers_failed(AzureIoTSecurity_NetworkActivityV6_vec_push_start(&serializer->builder))) {
            log_error("failed in AzureIoTSecurity_NetworkActivityV6_vec_push_start");
            serializer->state = SERIALIZER_STATE_EXCEPTION;
            return ASC_RESULT_EXCEPTION;
        }

        if (flatbuffers_failed(AzureIoTSecurity_NetworkActivityV6_addresses_create(&serializer->builder,
                                                                ipv6_payload->local_address, ipv6_payload->remote_address))) {
            log_error("failed in AzureIoTSecurity_NetworkActivityV6_addresses_create");
            serializer->state = SERIALIZER_STATE_EXCEPTION;
            return ASC_RESULT_EXCEPTION;

        }

        if (flatbuffers_failed(AzureIoTSecurity_NetworkActivityV6_common_start(&serializer->builder))) {
            log_error("failed in AzureIoTSecurity_NetworkActivityV6_common_start");
            serializer->state = SERIALIZER_STATE_EXCEPTION;
            return ASC_RESULT_EXCEPTION;
        }

        if (_serializer_network_activity_add_common(serializer, &ipv6_payload->common) != ASC_RESULT_OK) {
            return ASC_RESULT_EXCEPTION;
        }

        if (flatbuffers_failed(AzureIoTSecurity_NetworkActivityV6_common_end(&serializer->builder))) {
            log_error("failed in AzureIoTSecurity_NetworkActivityV6_common_end");
            serializer->state = SERIALIZER_STATE_EXCEPTION;
            return ASC_RESULT_EXCEPTION;
        }

        if (AzureIoTSecurity_NetworkActivityV6_vec_push_end(&serializer->builder) == NULL) {
            log_error("failed in AzureIoTSecurity_NetworkActivityV6_vec_push_end");
            serializer->state = SERIALIZER_STATE_EXCEPTION;
            return ASC_RESULT_EXCEPTION;
        }

        ipv6_payload = ipv6_payload->next;
    } while (ipv6_payload != NULL);

    if (flatbuffers_failed(AzureIoTSecurity_NetworkActivity_ipv6_activity_end(&serializer->builder))) {
        log_error("failed in AzureIoTSecurity_NetworkActivity_ipv6_activity_end");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    return ASC_RESULT_OK;
}


static asc_result_t _serializer_network_activity_add_common(serializer_t *serializer, network_activity_common_t *common)
{
    if (flatbuffers_failed(AzureIoTSecurity_NetworkActivityCommon_local_port_add(&serializer->builder, common->local_port))) {
        log_error("failed in AzureIoTSecurity_NetworkActivityCommon_local_port_add");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    if (flatbuffers_failed(AzureIoTSecurity_NetworkActivityCommon_remote_port_add(&serializer->builder, common->remote_port))) {
        log_error("failed in AzureIoTSecurity_NetworkActivityCommon_remote_port_add");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    if (flatbuffers_failed(AzureIoTSecurity_NetworkActivityCommon_bytes_in_add(&serializer->builder, common->bytes_in))) {
        log_error("failed in AzureIoTSecurity_NetworkActivityCommon_bytes_in_add");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    if (flatbuffers_failed(AzureIoTSecurity_NetworkActivityCommon_bytes_out_add(&serializer->builder, common->bytes_out))) {
        log_error("failed in AzureIoTSecurity_NetworkActivityCommon_bytes_out_add");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    AzureIoTSecurity_Protocol_enum_t protocol = protocol_serialize_enum(common->transport_protocol);
    if (flatbuffers_failed(AzureIoTSecurity_NetworkActivityCommon_protocol_add(&serializer->builder, protocol))) {
        log_error("failed in AzureIoTSecurity_NetworkActivityCommon_protocol_add");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    return ASC_RESULT_OK;
}

