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
#include <stdint.h>

#include <asc_config.h>

#include "asc_security_core/logger.h"
#include "asc_security_core/model/schema/message_builder.h"
#include "asc_security_core/utils/uuid.h"

#include "serializer_private.h"

asc_result_t serializer_event_start(serializer_t *serializer, unsigned long timestamp, unsigned long collection_interval)
{
    if (serializer->state == SERIALIZER_STATE_MESSAGE_EMPTY && flatbuffers_failed(AzureIoTSecurity_Message_events_start(&serializer->builder))) {
        log_error("failed in AzureIoTSecurity_Message_events_start");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    if (flatbuffers_failed(AzureIoTSecurity_Message_events_push_start(&serializer->builder))) {
        log_error("failed in AzureIoTSecurity_Event_vec_push_start");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    uint8_t uuid[16] = { 0 };
    if (uuid_generate(uuid) < 0) {
        log_error("failed in uuid_generate");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    if (flatbuffers_failed(AzureIoTSecurity_Event_id_create(&serializer->builder, uuid))) {
        log_error("failed in AzureIoTSecurity_Event_id_create");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;

    }

    if (flatbuffers_failed(AzureIoTSecurity_Event_time_add(&serializer->builder, (uint32_t)timestamp))) {
        log_error("failed in AzureIoTSecurity_Event_time_add");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    if (flatbuffers_failed(AzureIoTSecurity_Event_collection_interval_add(&serializer->builder, (uint32_t)collection_interval))) {
        log_error("failed in AzureIoTSecurity_Event_collection_interval_add");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    return ASC_RESULT_OK;
}
