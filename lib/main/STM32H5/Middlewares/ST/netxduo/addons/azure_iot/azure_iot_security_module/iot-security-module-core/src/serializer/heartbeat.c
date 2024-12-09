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
#include "asc_security_core/model/schema/message_builder.h"

#include "asc_security_core/serializer.h"
#include "serializer_private.h"

asc_result_t serializer_event_add_heartbeat(serializer_t *serializer, unsigned long timestamp, unsigned long collection_interval)
{
    log_debug("serializer_event_add_heartbeat, serializer=[%p], timestamp=[%lu], collection_interval=[%lu]",
              (void*)serializer, timestamp, collection_interval);
    
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

    if (flatbuffers_failed(AzureIoTSecurity_Event_payload_Heartbeat_create(&serializer->builder))) {
        log_error("failed in AzureIoTSecurity_Event_payload_SystemInformation_start");
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
