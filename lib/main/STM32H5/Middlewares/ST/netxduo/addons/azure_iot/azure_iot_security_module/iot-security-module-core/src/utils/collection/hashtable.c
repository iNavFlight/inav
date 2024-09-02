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

#include <asc_security_core/logger.h>
#include <asc_security_core/utils/string_utils.h>
#include <asc_security_core/utils/collection/hashtable.h>

static unsigned int _hash_calc(hashtable_t *hashtable, const void *key);
static void *_hashtable_find(hashtable_t *hashtable, const void *key, unsigned int *index);

// API functions
bool hashtable_init(hashtable_t *hashtable, size_t size, hash_func_t hash_func,
    hash_equal_func_t hash_equal_func, hash_update_func_t hash_update_func)
{
    if (!hashtable || !size) {
        log_error("Wrong hashtable init parametrs");
        return false;
    }
    hashtable->ctrl.size = size;
    hashtable->ctrl.hash_func = hash_func;
    hashtable->ctrl.hash_equal_func = hash_equal_func ? hash_equal_func : hashtable_equal_default_func;
    hashtable->ctrl.hash_update_func = hash_update_func ? hash_update_func : hashtable_update_none_func;
    memset(hashtable->table, 0, size * sizeof(void *));
    hashtable->ctrl.initialized = true;
    return true;
}

void *hashtable_insert(hashtable_t *hashtable, const void *key, void *item)
{
    if (!hashtable || !hashtable->ctrl.initialized || !key || !item) {
        log_error("Wrong parametrs");
        return NULL;
    }
    collection_item_t *element = item;
    unsigned int index = 0;
    collection_item_t *existing_element = _hashtable_find(hashtable, key, &index);

    if (existing_element == NULL) {
        collection_item_t *column = hashtable->table[index];
        element->previous = NULL;
        element->next = column;
        if (column != NULL) {
            column->previous = element;
        }
        hashtable->table[index] = element;
    } else {
        hashtable->ctrl.hash_update_func(existing_element, element);
    }
    return item;
}

void *hashtable_remove(hashtable_t *hashtable, const void *key, hash_free_func_t hash_free_func)
{
    if (!hashtable || !hashtable->ctrl.initialized || !key) {
        log_error("Wrong parametrs");
        return NULL;
    }
    unsigned int index = 0;
    collection_item_t *existing_element = _hashtable_find(hashtable, key, &index);

    if (existing_element == NULL) {
        return NULL;
    }
    collection_item_t *previous_element = existing_element->previous;
    collection_item_t *next_element = existing_element->next;

    if (next_element != NULL) {
        next_element->previous = previous_element;
    }
    if (previous_element == NULL) {
        hashtable->table[index] = next_element;
    } else {
        previous_element->next = next_element;
    }
    if (hash_free_func) {
        hash_free_func(existing_element);
        return NULL;
    }
    return existing_element;
}

void *hashtable_find(hashtable_t *hashtable, const void *key)
{
    if (!hashtable || !hashtable->ctrl.initialized || !key) {
        log_error("Wrong parametrs");
        return NULL;
    }
    unsigned int index;
    return _hashtable_find(hashtable, key, &index);
}

size_t hashtable_get_depth(hashtable_t *hashtable, size_t *count)
{
    size_t max = 0;
    if (count) {
        *count = 0;
    }
    if (!hashtable || !hashtable->ctrl.initialized) {
        return 0;
    }
    for (size_t i = 0; i < hashtable->ctrl.size; i++) {
        collection_item_t *current_element = NULL;
        size_t cnt = 0;
        hashtable_column_foreach(hashtable, current_element, i) {
            cnt++;
        }
        if (cnt > 1) {
            if (cnt > max) {
                max = cnt;
            }
            if (count) {
                (*count)++;
            }
        }
    }
    return max;
}

void hashtable_foreach(hashtable_t *hashtable, hash_action_func_t hash_action_func, void *ctx)
{
    if (!hashtable || !hashtable->ctrl.initialized || !hash_action_func) {
        return;
    }
    for (unsigned int i = 0; i < hashtable->ctrl.size; i++) {
        // not using hashtable_column_foreach() because of 'tmp = current_element->next for safe action'
        collection_item_t *current_element = hashtable->table[i]; // get column head
        while (current_element != NULL) {
            collection_item_t *tmp = current_element->next;
            hash_action_func(current_element, ctx);
            current_element = tmp;
        }
    }
}

void hashtable_flush(hashtable_t *hashtable, hash_free_func_t hash_free_func)
{
    if (!hashtable || !hashtable->ctrl.initialized) {
        return;
    }
    if (!hash_free_func) {
        goto cleanup;
    }
    for (size_t i = 0; i < hashtable->ctrl.size; i++) {
        // not using hashtable_column_foreach() because of 'tmp = current_element->next for safe free'
        collection_item_t *current_element = hashtable->table[i]; // get column head
        while (current_element != NULL) {
            collection_item_t *tmp = current_element->next;
            hash_free_func(current_element);
            current_element = tmp;
        }
    }
cleanup:
    memset(hashtable->table, 0, hashtable->ctrl.size * sizeof(void *));
}

bool hashtable_is_empty(hashtable_t *hashtable)
{
    if (!hashtable || !hashtable->ctrl.initialized) {
        return true;
    }
    for (size_t i = 0; i < hashtable->ctrl.size; i++) {
        collection_item_t *column = hashtable->table[i];
        if (column) {
            return false;
        }
    }
    return true;
}

// Service callbacks and functions
void hashtable_update_none_func(void *a, void *b)
{
    log_error("Should never happens");
}

bool hashtable_equal_default_func(const void *key, void *item)
{
    return (key == item);
}

unsigned int hashtable_hash_str_default(const void *key)
{
    const char *buffer = key;
    return hashtable_buffer2hash(buffer, str_len(buffer));
}

unsigned int hashtable_buffer2hash(const char *buffer, size_t len)
{
    unsigned int hash = 5381;

    for (size_t i = 0 ; i < len; i++) {
        hash = ((hash << 5) + hash) + (unsigned int)buffer[i];
    }

    return hash;
}

// Internal functions
static unsigned int _hash_calc(hashtable_t *hashtable, const void *key)
{
    return hashtable->ctrl.hash_func ? (unsigned int)(hashtable->ctrl.hash_func(key) % hashtable->ctrl.size) :
        (unsigned int)((uintptr_t)key % hashtable->ctrl.size);
}

static void *_hashtable_find(hashtable_t *hashtable, const void *key, unsigned int *index)
{
    *index = _hash_calc(hashtable, key);
    collection_item_t *current_element = NULL;

    hashtable_column_foreach(hashtable, current_element, *index) {
        if (hashtable->ctrl.hash_equal_func(key, current_element) != 0) {
            break;
        }
    }

    return current_element;
}

