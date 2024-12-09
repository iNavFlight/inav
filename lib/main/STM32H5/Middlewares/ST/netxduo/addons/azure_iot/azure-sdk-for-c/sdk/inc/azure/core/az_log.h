// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief This header defines the types and functions your application uses to be notified of Azure
 * SDK client library log messages.
 *
 * @details If you define the `AZ_NO_LOGGING` symbol when compiling the SDK code (or adding option
 * `-DLOGGING=OFF` with cmake), all of the Azure SDK logging functionality will be excluded, making
 * the resulting compiled code smaller and faster.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_LOG_H
#define _az_LOG_H

#include <azure/core/az_result.h>
#include <azure/core/az_span.h>

#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

/**
 * @brief Identifies the classifications of log messages produced by the SDK.
 *
 * @note See the following `az_log_classification` values from various headers:
 * - #az_log_classification_core
 * - #az_log_classification_iot
 */
typedef int32_t az_log_classification;

// az_log_classification Bits:
//   - 31 Always 0.
//   - 16..30 Facility.
//   - 0..15 Code.

#define _az_LOG_MAKE_CLASSIFICATION(facility, code) \
  ((az_log_classification)(((uint32_t)(facility) << 16U) | (uint32_t)(code)))

/**
 * @brief Identifies the #az_log_classification produced by the SDK Core.
 */
enum az_log_classification_core
{
  AZ_LOG_HTTP_REQUEST
  = _az_LOG_MAKE_CLASSIFICATION(_az_FACILITY_CORE_HTTP, 1), ///< HTTP request is about to be sent.

  AZ_LOG_HTTP_RESPONSE
  = _az_LOG_MAKE_CLASSIFICATION(_az_FACILITY_CORE_HTTP, 2), ///< HTTP response was received.

  AZ_LOG_HTTP_RETRY = _az_LOG_MAKE_CLASSIFICATION(
      _az_FACILITY_CORE_HTTP,
      3), ///< First HTTP request did not succeed and will be retried.
};

/**
 * @brief Defines the signature of the callback function that application developers must provide to
 * receive Azure SDK log messages.
 *
 * @param[in] classification The log message's #az_log_classification.
 * @param[in] message The log message.
 */
typedef void (*az_log_message_fn)(az_log_classification classification, az_span message);

/**
 * @brief Defines the signature of the callback function that application developers must provide
 * which will be used to check whether a particular log classification should be logged.
 *
 * @param[in] classification The log message's #az_log_classification.
 *
 * @return Whether or not a log message with the provided classification should be logged.
 */
typedef bool (*az_log_classification_filter_fn)(az_log_classification classification);

/**
 * @brief Sets the functions that will be invoked to report an SDK log message.
 *
 * @param[in] log_message_callback __[nullable]__ A pointer to the function that will be invoked
 * when the SDK reports a log message that should be logged according to the result of the
 * #az_log_classification_filter_fn provided to #az_log_set_classification_filter_callback(). If
 * `NULL`, no function will be invoked.
 *
 * @remarks By default, this is `NULL`, which means, no function is invoked.
 */
#ifndef AZ_NO_LOGGING
void az_log_set_message_callback(az_log_message_fn log_message_callback);
#else
AZ_INLINE void az_log_set_message_callback(az_log_message_fn log_message_callback)
{
  (void)log_message_callback;
}
#endif // AZ_NO_LOGGING

/**
 * @brief Sets the functions that will be invoked to check whether an SDK log message should be
 * reported.
 *
 * @param[in] message_filter_callback __[nullable]__ A pointer to the function that will be invoked
 * when the SDK checks whether a log message of a particular #az_log_classification should be
 * logged. If `NULL`, log messages for all classifications will be logged, by passing them to the
 * #az_log_message_fn provided to #az_log_set_message_callback().
 *
 * @remarks By default, this is `NULL`, in which case no function is invoked to check whether a
 * classification should be logged or not. The SDK assumes true, passing messages with any log
 * classification to the #az_log_message_fn provided to #az_log_set_message_callback().
 */
#ifndef AZ_NO_LOGGING
void az_log_set_classification_filter_callback(
    az_log_classification_filter_fn message_filter_callback);
#else
AZ_INLINE void az_log_set_classification_filter_callback(
    az_log_classification_filter_fn message_filter_callback)
{
  (void)message_filter_callback;
}
#endif // AZ_NO_LOGGING

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_LOG_H
