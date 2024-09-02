// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "az_span_private.h"
#include <azure/core/az_config.h>
#include <azure/core/az_http.h>
#include <azure/core/az_http_transport.h>
#include <azure/core/az_log.h>
#include <azure/core/az_span.h>
#include <azure/core/internal/az_http_internal.h>
#include <azure/core/internal/az_log_internal.h>

#include <stddef.h>

#include <azure/core/_az_cfg.h>

#ifndef AZ_NO_LOGGING

// Only using volatile here, not for thread safety, but so that the compiler does not optimize what
// it falsely thinks are stale reads.
static az_log_message_fn volatile _az_log_message_callback = NULL;
static az_log_classification_filter_fn volatile _az_message_filter_callback = NULL;

void az_log_set_message_callback(az_log_message_fn log_message_callback)
{
  // We assume assignments are atomic for the supported platforms and compilers.
  _az_log_message_callback = log_message_callback;
}

void az_log_set_classification_filter_callback(
    az_log_classification_filter_fn message_filter_callback)
{
  // We assume assignments are atomic for the supported platforms and compilers.
  _az_message_filter_callback = message_filter_callback;
}

AZ_INLINE az_log_message_fn _az_log_get_message_callback(az_log_classification classification)
{
  _az_PRECONDITION(classification > 0);

  // Copy the volatile fields to local variables so that they don't change within this function.
  az_log_message_fn const message_callback = _az_log_message_callback;
  az_log_classification_filter_fn const message_filter_callback = _az_message_filter_callback;

  // If the user hasn't registered a message_filter_callback, then we log everything, as long as a
  // message_callback method was provided.
  // Otherwise, we log only what that filter allows.
  if (message_callback != NULL
      && (message_filter_callback == NULL || message_filter_callback(classification)))
  {
    return message_callback;
  }

  // This message's classification is either not allowed by the filter, or there is no callback
  // function registered to receive the message. In both cases, we should not log it.
  return NULL;
}

// This function returns whether or not the passed-in message should be logged.
bool _az_log_should_write(az_log_classification classification)
{
  return _az_log_get_message_callback(classification) != NULL;
}

// This function attempts to log the passed-in message.
void _az_log_write(az_log_classification classification, az_span message)
{
  _az_PRECONDITION_VALID_SPAN(message, 0, true);

  az_log_message_fn const message_callback = _az_log_get_message_callback(classification);

  if (message_callback != NULL)
  {
    message_callback(classification, message);
  }
}

#endif // AZ_NO_LOGGING
