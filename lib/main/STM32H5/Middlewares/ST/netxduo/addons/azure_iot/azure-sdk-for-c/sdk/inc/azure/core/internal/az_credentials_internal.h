// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Defines internals used by credentials.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_CREDENTIALS_INTERNAL_H
#define _az_CREDENTIALS_INTERNAL_H

#include <azure/core/az_credentials.h>
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>

#include <stddef.h>

#include <azure/core/_az_cfg_prefix.h>

AZ_INLINE AZ_NODISCARD az_result
_az_credential_set_scopes(_az_credential* credential, az_span scopes)
{
  return (credential == NULL || credential->_internal.set_scopes == NULL)
      ? AZ_OK
      : (credential->_internal.set_scopes)(credential, scopes);
}

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_CREDENTIALS_INTERNAL_H
