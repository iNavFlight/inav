// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Context for canceling long running operations.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_CONTEXT_H
#define _az_CONTEXT_H

#include <azure/core/az_result.h>

#include <stddef.h>
#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

/**
 * @brief A context is a node within a tree that represents expiration times and key/value pairs.
 */
// Definition is below. Defining the typedef first is necessary here since there is a cycle.
typedef struct az_context az_context;

/**
 * @brief A context is a node within a tree that represents expiration times and key/value pairs.
 *
 * @details The root node in the tree (ultimate parent).
 */
struct az_context
{
  struct
  {
    az_context const* parent; // Pointer to parent context (or NULL); immutable after creation
    int64_t expiration; // Time when context expires
    void const* key; // Pointers to the key & value (usually NULL)
    void const* value;
  } _internal;
};

#define _az_CONTEXT_MAX_EXPIRATION 0x7FFFFFFFFFFFFFFF

/**
 * @brief The application root #az_context instances.
 * @details The #az_context_application never expires but you can explicitly cancel it by passing
 * its address to #az_context_cancel() which effectively cancels all its #az_context child nodes.
 */
extern az_context az_context_application;

/**
 * @brief Creates a new expiring #az_context node that is a child of the specified parent.
 *
 * @param[in] parent The #az_context node that is the parent to the new node.
 * @param[in] expiration The time when this new node should be canceled.
 *
 * @return The new child #az_context node.
 */
AZ_NODISCARD az_context
az_context_create_with_expiration(az_context const* parent, int64_t expiration);

/**
 * @brief Creates a new key/value az_context node that is a child of the specified parent.
 *
 * @param[in] parent The #az_context node that is the parent to the new node.
 * @param[in] key A pointer to the key of this new #az_context node.
 * @param[in] value A pointer to the value of this new #az_context node.
 *
 * @return The new child #az_context node.
 */
AZ_NODISCARD az_context
az_context_create_with_value(az_context const* parent, void const* key, void const* value);

/**
 * @brief Cancels the specified #az_context node; this cancels all the child nodes as well.
 *
 * @param[in,out] ref_context A pointer to the #az_context node to be canceled.
 */
void az_context_cancel(az_context* ref_context);

/**
 * @brief Returns the soonest expiration time of this #az_context node or any of its parent nodes.
 *
 * @param[in] context A pointer to an #az_context node.
 * @return The soonest expiration time from this context and its parents.
 */
AZ_NODISCARD int64_t az_context_get_expiration(az_context const* context);

/**
 * @brief Returns `true` if this #az_context node or any of its parent nodes' expiration is before
 * the \p current_time.
 *
 * @param[in] context A pointer to the #az_context node to check.
 * @param[in] current_time The current time.
 */
AZ_NODISCARD bool az_context_has_expired(az_context const* context, int64_t current_time);

/**
 * @brief Walks up this #az_context node's parents until it find a node whose key matches the
 * specified key and returns the corresponding value.
 *
 * @param[in] context The #az_context node in the tree where checking starts.
 * @param[in] key A pointer to the key to be scanned for.
 * @param[out] out_value A pointer to a `void const*` that will receive the key's associated value
 * if the key is found.
 *
 * @return An #az_result value indicating the result of the operation.
 * @retval #AZ_OK The key is found.
 * @retval #AZ_ERROR_ITEM_NOT_FOUND No nodes are found with the specified key.
 */
AZ_NODISCARD az_result
az_context_get_value(az_context const* context, void const* key, void const** out_value);

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_CONTEXT_H
