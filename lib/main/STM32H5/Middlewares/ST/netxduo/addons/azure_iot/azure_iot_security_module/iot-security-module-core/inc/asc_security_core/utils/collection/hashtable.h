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

#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <asc_config.h>

#include "asc_security_core/utils/collection/collection.h"

/* Pay your attention, that this implementation is not thread safe. */

#define HASH_KEY(_key) ((const void *)((uintptr_t)(_key)))

/**
 * @brief   Typedef of hash function
 *
 * @param   key         Hash key
 *
 * @return  hash value none normilized to size.
 */
typedef unsigned int (*hash_func_t)(const void *key);

/**
 * @brief   Typedef of comparing function
 *
 * @param   key         Hash key calculated by @c hash_func_t provided to @c hashtable_init(...)
 * @param   item        The item to compare
 *
 * @return  true on equal, otherwise false
 */
typedef bool (*hash_equal_func_t)(const void *key, void *item);

/**
 * @brief   Typedef of updating function
 *
 * @param   a         The old item
 * @param   b         The new item
 *
 * @return  none
 */
typedef void (*hash_update_func_t)(void *a, void *b);

/**
 * @brief   Typedef of free function
 *
 * @param   item       The item to free
 *
 * @return  none
 */
typedef void (*hash_free_func_t)(void *item);

/**
 * @brief   Typedef of action function
 *
 * @param   item       The item for action
 * @param   ctx        User context
 *
 * @return  none
 */
typedef void (*hash_action_func_t)(void *item, void *ctx);

typedef struct {
    bool initialized;                       // single tone initializin flag
    size_t size;                            /* Actual size of hashtable. MUST be between 1 and <size> given in HASHTABLE_DECLARATIONS() macro.
                                             * Warning: there is no size checking - so passing <size>, that above <max_hash_size> will lead to crash */
    hash_func_t hash_func;
    hash_equal_func_t hash_equal_func;
    hash_update_func_t hash_update_func;
} hashtable_ctrl_t;

#define HASHTABLE_DECLARATIONS(type, max_hash_size) \
typedef struct { \
    hashtable_ctrl_t ctrl; \
    type *table[(max_hash_size)]; \
} hashtable_##type;

typedef struct {
    hashtable_ctrl_t ctrl;
    void *table[1];
} hashtable_t;

// API functions
/**
 * @brief   Run time initialization of hashtable object
 *
 * @param   hashtable   Pointer on hashtable for initialization
 * @param   size        Actual size of hashtable. MUST be between 1 and <size> given in HASHTABLE_DECLARATIONS() macro.
 *                      Warning: there is no size checking - so passing size, that above number
 *                      that given in HASHTABLE_DECLARATIONS() will lead to crash
 * @param   hash_func   Hash function. @c hash_func_t If NULL was passed the hashtable will use simple hashing formula (key%size)
 * @param   hash_equal_func Hash object comparing function. @c hash_equal_func_t If NULL was passed the hashtable will use default comparing (key == item)
 * @param   hash_update_func Hash object updating function. @c hash_update_func_t If NULL was passed the hashtable will use @c hashtable_update_none_func(...)
 * 
 * @return  true on success, otherwise false
 */
bool hashtable_init(hashtable_t *hashtable, size_t size, hash_func_t hash_func,
    hash_equal_func_t hash_equal_func, hash_update_func_t hash_update_func);

/**
 * @brief   Insert new hashtable item by key. If equal item was found by @c hash_equal_func_t the @c hash_update_func_t will performed
 *
 * @param   hashtable   A pointer on initialized hashtable
 * @param   key         Hash key calculated by @c hash_func_t provided to @c hashtable_init(...)
 * @param   item        Item to be inserted
 *
 * @return  the pointer on inserted item on success, otherwise NULL.
 */
void *hashtable_insert(hashtable_t *hashtable, const void *key, void *item);

/**
 * @brief   Remove hashtable item by key.
 *
 * @param   hashtable   A pointer on initialized hashtable
 * @param   key         Hash key calculated by @c hash_func_t provided to @c hashtable_init(...)
 * @param   hash_free_func Free data function
 *
 * @return  the pointer on removed item on success and if the hash_free_func == NULL, otherwise NULL.
 */
void *hashtable_remove(hashtable_t *hashtable, const void *key, hash_free_func_t hash_free_func);

/**
 * @brief   Find hashtable item by key.
 *
 * @param   hashtable   A pointer on initialized hashtable
 * @param   key         Hash key calculated by @c hash_func_t provided to @c hashtable_init(...)
 *
 * @return  the pointer on found item on success, otherwise NULL.
 */
void *hashtable_find(hashtable_t *hashtable, const void *key);

/**
 * @brief   Cleanup the hashtable.
 *
 * @param   hashtable   A pointer on initialized hashtable
 * @param   hash_free_func Free data function
 *
 * @return  none
 */
void hashtable_flush(hashtable_t *hashtable, hash_free_func_t hash_free_func);

/**
 * @brief   Runn over all hashtable items.
 *
 * @param   hashtable        A pointer on initialized hashtable
 * @param   hash_action_func Action callback to be called for each item
 * @param   ctx              User contex to be passed to @c hash_action_func(...)
 *
 * @return  none
 */
void hashtable_foreach(hashtable_t *hashtable, hash_action_func_t hash_action_func, void *ctx);

/**
 * @brief   Get depth of hash to exam the hash function.
 *
 * @param   hashtable   A pointer on initialized hashtable
 * @param   count       Count of non null or one element linked lists (out parameter)
 *
 * @return  zero if all hash elements have list with 0 or 1 length - otherwise the longest list length
 */
size_t hashtable_get_depth(hashtable_t *hashtable, size_t *count);

/**
 * @brief   Check if hash table is empty
 *
 * @param   table       A pointer on initialized hashset
 *
 * @return  true if hash table is empty - otherwise false
 */
 bool hashtable_is_empty(hashtable_t *hashtable);

/** @brief Run over specified column of hashtable **/
#define hashtable_column_foreach(hashtable, iter, column) for (iter = (hashtable)->table[(column)]; (iter); iter = (iter)->next)

/** @brief Init hashtable with default callbacks **/
#define hashtable_init_simple(hashtable, size) hashtable_init(hashtable, size, NULL, NULL, NULL)

// Service callbacks
/** @brief The default @c hash_equal_func_t equal function returns true is if key equals to object pointer **/
bool hashtable_equal_default_func(const void *key, void *item);

/** @brief The 'key is NULL terminated string' @c hash_func_t hash generic function **/
unsigned int hashtable_hash_str_default(const void *key);

/** @brief The default @c hash_update_func_t hash update doing nothing function **/
void hashtable_update_none_func(void *a, void *b);

// Service functions
/** @brief The default hash calculating function on char buffer **/
unsigned int hashtable_buffer2hash(const char *buffer, size_t len);

#endif // __HASHTABLE_H__