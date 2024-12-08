// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/az_context.h>
#include <azure/core/internal/az_precondition_internal.h>

#include <stddef.h>

#include <azure/core/_az_cfg.h>

// This is a global az_context node representing the entire application. By default, this node
// never expires. Call az_context_cancel passing a pointer to this node to cancel the entire
// application (which cancels all the child nodes).
az_context az_context_application = {
  ._internal
  = { .parent = NULL, .expiration = _az_CONTEXT_MAX_EXPIRATION, .key = NULL, .value = NULL }
};

// Returns the soonest expiration time of this az_context node or any of its parent nodes.
AZ_NODISCARD int64_t az_context_get_expiration(az_context const* context)
{
  _az_PRECONDITION_NOT_NULL(context);

  int64_t expiration = _az_CONTEXT_MAX_EXPIRATION;
  for (; context != NULL; context = context->_internal.parent)
  {
    if (context->_internal.expiration < expiration)
    {
      expiration = context->_internal.expiration;
    }
  }
  return expiration;
}

// Walks up this az_context node's parent until it find a node whose key matches the specified key
// and return the corresponding value. Returns AZ_ERROR_ITEM_NOT_FOUND is there are no nodes
// matching the specified key.
AZ_NODISCARD az_result
az_context_get_value(az_context const* context, void const* key, void const** out_value)
{
  _az_PRECONDITION_NOT_NULL(context);
  _az_PRECONDITION_NOT_NULL(out_value);
  _az_PRECONDITION_NOT_NULL(key);

  for (; context != NULL; context = context->_internal.parent)
  {
    if (context->_internal.key == key)
    {
      *out_value = context->_internal.value;
      return AZ_OK;
    }
  }
  *out_value = NULL;
  return AZ_ERROR_ITEM_NOT_FOUND;
}

AZ_NODISCARD az_context
az_context_create_with_expiration(az_context const* parent, int64_t expiration)
{
  _az_PRECONDITION_NOT_NULL(parent);
  _az_PRECONDITION(expiration >= 0);

  return (az_context){ ._internal = { .parent = parent, .expiration = expiration } };
}

AZ_NODISCARD az_context
az_context_create_with_value(az_context const* parent, void const* key, void const* value)
{
  _az_PRECONDITION_NOT_NULL(parent);
  _az_PRECONDITION_NOT_NULL(key);

  return (az_context){
    ._internal
    = { .parent = parent, .expiration = _az_CONTEXT_MAX_EXPIRATION, .key = key, .value = value }
  };
}

void az_context_cancel(az_context* ref_context)
{
  _az_PRECONDITION_NOT_NULL(ref_context);

  ref_context->_internal.expiration = 0; // The beginning of time
}

AZ_NODISCARD bool az_context_has_expired(az_context const* context, int64_t current_time)
{
  _az_PRECONDITION_NOT_NULL(context);
  _az_PRECONDITION(current_time >= 0);

  return az_context_get_expiration(context) < current_time;
}
