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
#include "asc_security_core/object_pool.h"
#include "asc_security_core/utils/uuid.h"

#include "asc_security_core/serializer.h"
#include "serializer_private.h"

OBJECT_POOL_DECLARATIONS(serializer_t)
OBJECT_POOL_DEFINITIONS(serializer_t, 1)

#ifdef ASC_SERIALIZER_USE_CUSTOM_ALLOCATOR
#include "asc_security_core/serializer/custom_builder_allocator.h"
#define ASC_SERIALIZER_CUSTOM_ALLOCATOR serializer_custom_allocator
#else
#define ASC_SERIALIZER_CUSTOM_ALLOCATOR NULL
#endif

serializer_t *serializer_init(void)
{
    log_debug("serializer_init");

    bool failed = false;
    serializer_t *serializer_ptr = object_pool_get(serializer_t);
    if (serializer_ptr == NULL) {
        log_error("Could not allocate new serializer_t");
        failed = true;
        goto cleanup;
    }

    if (flatbuffers_failed(flatcc_builder_custom_init(&serializer_ptr->builder, NULL, NULL, ASC_SERIALIZER_CUSTOM_ALLOCATOR, NULL))) {
        log_error("Could not initialize flatcc_builder");
        failed = true;
        goto cleanup;
    }

    serializer_ptr->state = SERIALIZER_STATE_INITIALIZED;

cleanup:
    if (failed && serializer_ptr != NULL) {
        object_pool_free(serializer_t, serializer_ptr);
        serializer_ptr = NULL;
    }

    return serializer_ptr;
}

void serializer_deinit(serializer_t *serializer)
{
    log_debug("serializer_deinit, serializer=[%p]", (void*)serializer);

    if (serializer == NULL) {
        log_warn("Tried to deinit a NULL serializer");
        return;
    }

    flatcc_builder_clear(&serializer->builder);
#ifdef ASC_SERIALIZER_USE_CUSTOM_ALLOCATOR
    serializer_custom_allocator_reset();
#endif
    object_pool_free(serializer_t, serializer);
}

serializer_state_t serializer_get_state(serializer_t *serializer)
{
    log_debug("serializer_get_state, serializer=[%p]", (void*)serializer);

    if (serializer == NULL) {
        log_error("failed, bad argument");
        return SERIALIZER_STATE_UNINITIALIZED;
    }

    return serializer->state;
}

asc_result_t serializer_message_begin(serializer_t *serializer, const char *security_module_id, uint32_t security_module_version)
{
    log_debug("serializer_message_begin, serializer=[%p]", (void*)serializer);

    if (serializer == NULL) {
        log_error("failed, bad argument");
        return ASC_RESULT_BAD_ARGUMENT;
    }

    if (serializer->state != SERIALIZER_STATE_INITIALIZED) {
        log_error("failed, state=[%d]", serializer->state);
        return ASC_RESULT_EXCEPTION;
    }

    if (flatbuffers_failed(AzureIoTSecurity_Message_start_as_root(&serializer->builder))) {
        log_error("failed in AzureIoTSecurity_Message_start_as_root");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    if (flatbuffers_failed(AzureIoTSecurity_Message_security_module_id_create_str(&serializer->builder, security_module_id))) {
        log_error("failed in AzureIoTSecurity_Message_security_module_id_create_str");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    if (flatbuffers_failed(AzureIoTSecurity_Message_security_module_version_add(&serializer->builder, security_module_version))) {
        log_error("failed in AzureIoTSecurity_Message_security_module_version_add");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    // FIXME: add time zone here

    serializer->state = SERIALIZER_STATE_MESSAGE_EMPTY;

    return ASC_RESULT_OK;
}

asc_result_t serializer_message_end(serializer_t *serializer)
{
    log_debug("serializer_message_end, serializer=[%p]", (void*)serializer);

    if (serializer == NULL) {
        log_error("failed, bad argument");
        return ASC_RESULT_BAD_ARGUMENT;
    }

    if (serializer->state != SERIALIZER_STATE_MESSAGE_PROCESSING) {
        log_error("failed, state=[%d]", serializer->state);
        return ASC_RESULT_EXCEPTION;
    }

    if (flatbuffers_failed(AzureIoTSecurity_Message_events_end(&serializer->builder))) {
        log_error("failed in AzureIoTSecurity_Message_events_end");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }


    if (AzureIoTSecurity_Message_end_as_root(&serializer->builder) == 0) {
        log_error("failed in AzureIoTSecurity_Message_end_as_root");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }

    serializer->state = SERIALIZER_STATE_MESSAGE_READY;

    return ASC_RESULT_OK;
}

asc_result_t serializer_buffer_get(serializer_t *serializer, uint8_t **buffer, size_t *size)
{
    log_debug("serializer_buffer_get, serializer=[%p], buffer=[%p], size=[%p]", (void*)serializer, (void*)buffer, (void*)size);

    if (serializer == NULL || buffer == NULL || size == NULL) {
        log_error("serializer_get_buffer failed, bad argument");
        return ASC_RESULT_BAD_ARGUMENT;
    }

    if (serializer->state != SERIALIZER_STATE_MESSAGE_READY) {
        log_error("serializer_get_buffer failed, state=[%d]", serializer->state);
        return ASC_RESULT_EXCEPTION;
    }

    *buffer = flatcc_builder_get_direct_buffer(&serializer->builder, size);
    if (*buffer == NULL) {
        log_debug("failed in flatcc_builder_get_direct_buffer, message too big");
        return ASC_RESULT_IMPOSSIBLE;
    }

    return ASC_RESULT_OK;
}

asc_result_t serializer_buffer_get_size(serializer_t *serializer, size_t *size)
{
    log_debug("serializer_buffer_get_size, serializer=[%p], size=[%p]", (void*)serializer, (void*)size);

    if (serializer == NULL || size == NULL) {
        log_error("serializer_get_buffer failed, bad argument");
        return ASC_RESULT_BAD_ARGUMENT;
    }

    if (serializer->state != SERIALIZER_STATE_MESSAGE_READY) {
        log_error("serializer_get_buffer failed, state=[%d]", serializer->state);
        return ASC_RESULT_EXCEPTION;
    }

    *size = flatcc_builder_get_buffer_size(&serializer->builder);

    return ASC_RESULT_OK;
}

asc_result_t serializer_buffer_get_copy(serializer_t *serializer, uint8_t *buffer, size_t size)
{
    log_debug("serializer_buffer_get_copy, serializer=[%p], buffer=[%p], size=[%lu]", (void*)serializer, (void*)buffer, (long unsigned int)size);

    if (serializer == NULL || buffer == NULL) {
        log_error("serializer_get_buffer failed, bad argument");
        return ASC_RESULT_BAD_ARGUMENT;
    }

    if (serializer->state != SERIALIZER_STATE_MESSAGE_READY) {
        log_error("serializer_get_buffer failed, state=[%d]", serializer->state);
        return ASC_RESULT_EXCEPTION;
    }

    if (flatcc_builder_copy_buffer(&serializer->builder, buffer, size) == NULL) {
        log_error("failed in flatcc_builder_copy_buffer, target buffer too small");
        return ASC_RESULT_EXCEPTION;
    }

    return ASC_RESULT_OK;
}

asc_result_t serializer_reset(serializer_t *serializer)
{
    log_debug("serializer_reset, serializer=[%p]", (void*)serializer);

    if (serializer == NULL) {
        log_error("serializer_reset failed, bad argument");
        return ASC_RESULT_BAD_ARGUMENT;
    }

    if (serializer->state == SERIALIZER_STATE_UNINITIALIZED) {
        log_error("serializer_reset failed, state=[%d]", serializer->state);
        return ASC_RESULT_EXCEPTION;
    }

    if (flatbuffers_failed(flatcc_builder_reset(&serializer->builder))) {
        log_error("serializer_reset failed in flatcc_builder_reset");
        serializer->state = SERIALIZER_STATE_EXCEPTION;
        return ASC_RESULT_EXCEPTION;
    }
#ifdef ASC_SERIALIZER_USE_CUSTOM_ALLOCATOR
    serializer_custom_allocator_reset();
#endif
    serializer->state = SERIALIZER_STATE_INITIALIZED;

    return ASC_RESULT_OK;
}
