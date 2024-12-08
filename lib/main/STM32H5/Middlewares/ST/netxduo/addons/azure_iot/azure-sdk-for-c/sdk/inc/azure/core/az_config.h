// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Configurable constants used by the Azure SDK.
 *
 * @remarks Typically, these constants do not need to be modified but depending on how your
 * application uses an Azure service, they can be adjusted.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_CONFIG_H
#define _az_CONFIG_H

#include <azure/core/_az_cfg_prefix.h>

enum
{
  /// The maximum buffer size for a URL.
  AZ_HTTP_REQUEST_URL_BUFFER_SIZE = 2 * 1024,

  /// The maximum buffer size for an HTTP request body.
  AZ_HTTP_REQUEST_BODY_BUFFER_SIZE = 1024,

  /// The maximum buffer size for a log message.
  AZ_LOG_MESSAGE_BUFFER_SIZE = 1024,
};

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_CONFIG_H
