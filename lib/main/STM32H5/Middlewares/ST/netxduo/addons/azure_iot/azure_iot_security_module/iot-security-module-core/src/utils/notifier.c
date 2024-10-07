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
#include "asc_security_core/utils/collection/list.h"
#include "asc_security_core/object_pool.h"
#include "asc_security_core/utils/notifier.h"
typedef struct notifier_item_t {
    /* This macro must be first in object */
    COLLECTION_INTERFACE(struct notifier_item_t);
    notifier_t *notifier;
} notifier_item_t;

OBJECT_POOL_DECLARATIONS(notifier_item_t)
OBJECT_POOL_DEFINITIONS(notifier_item_t, NOTIFIERS_POOL_ENTRIES)

static linked_list_t _notify_arr[NOTIFY_TOPICS_NUMBER] = {0};

static void object_pool_notifier_item_t_free(void *item)
{
    object_pool_free(notifier_item_t, item);
}

static bool _linked_list_find_condition(void *item, void *condition_input)
{
    notifier_item_t *handle = item;
    notifier_t *notifier = condition_input;

    return (notifier == handle->notifier);
}

int32_t notifier_notify(notify_topic_t topic, int msg_num, void *payload)
{
    linked_list_t *linked_list_handle = NULL;
    notifier_item_t *curr = NULL;

    if (topic >= NOTIFY_TOPICS_NUMBER) {
        log_error("Failed to remove notifier due to bad argument");
        goto error;
    }

    linked_list_handle = &_notify_arr[topic];
    linked_list_foreach(linked_list_handle, curr) {
        if (curr->notifier->notify) {
            curr->notifier->notify(curr->notifier, msg_num, payload);
        }
    }

    return (int32_t)linked_list_get_size(linked_list_handle);

error:
    return -1;
}

asc_result_t notifier_subscribe(notify_topic_t topic, notifier_t *notifier)
{
    asc_result_t result = ASC_RESULT_OK;
    notifier_item_t *add;
    linked_list_t *linked_list_handle;

    if (topic >= NOTIFY_TOPICS_NUMBER) {
        log_error("Failed to initialize notifier due to bad argument");
        result = ASC_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }
    add = object_pool_get(notifier_item_t);
    if (add == NULL) {
        log_error("Failed to allocate notifier");
        result = ASC_RESULT_MEMORY_EXCEPTION;
        goto cleanup;
    }
    add->notifier = notifier;
    linked_list_handle = &_notify_arr[topic];
    if (linked_list_handle->initialized == false) {
        linked_list_init(linked_list_handle);
    }
    linked_list_add_first(linked_list_handle, add);

cleanup:
	return result;
}

asc_result_t notifier_unsubscribe(notify_topic_t topic, notifier_t *notifier)
{
    asc_result_t result = ASC_RESULT_OK;
    linked_list_t *linked_list_handle;
    notifier_item_t *handle;

    if (topic >= NOTIFY_TOPICS_NUMBER) {
        log_error("Failed to remove notifier due to bad argument topic");
        result = ASC_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    linked_list_handle = &_notify_arr[topic];
    handle = linked_list_find(linked_list_handle, _linked_list_find_condition, notifier);
    if (handle == NULL) {
        log_error("Failed to remove notifier due to bad argument notifier");
        result = ASC_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }

    linked_list_remove(linked_list_handle, handle, object_pool_notifier_item_t_free);

cleanup:
	return result;
}

asc_result_t notifier_deinit(notify_topic_t topic)
{
    asc_result_t result = ASC_RESULT_OK;
    linked_list_t *linked_list_handle;

    if (topic >= NOTIFY_TOPICS_NUMBER) {
        log_error("Failed to remove notifier due to bad argument topic");
        result = ASC_RESULT_BAD_ARGUMENT;
        goto cleanup;
    }
    linked_list_handle = &_notify_arr[topic];

    linked_list_flush(linked_list_handle, object_pool_notifier_item_t_free);

cleanup:
    return result;
}