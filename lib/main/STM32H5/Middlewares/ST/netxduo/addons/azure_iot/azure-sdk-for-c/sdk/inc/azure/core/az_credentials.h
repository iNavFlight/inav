// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file
 *
 * @brief Credentials used for authentication with many (not all) Azure SDK client libraries.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_CREDENTIALS_H
#define _az_CREDENTIALS_H

#include <azure/core/az_http_transport.h>
#include <azure/core/az_result.h>
#include <azure/core/az_span.h>

#include <stddef.h>

#include <azure/core/_az_cfg_prefix.h>

/**
 * @brief Equivalent to no credential (`NULL`).
 */
#define AZ_CREDENTIAL_ANONYMOUS NULL

/**
 * @brief Function callback definition as a contract to be implemented for a credential to set
 * authentication scopes when it is supported by the type of the credential.
 */
typedef AZ_NODISCARD az_result (
    *_az_credential_set_scopes_fn)(void* ref_credential, az_span scopes);

/**
 * @brief Credential definition. It is used internally to authenticate an SDK client with Azure.
 * All types of credentials must contain this structure as their first member.
 */
typedef struct
{
  struct
  {
    _az_http_policy_process_fn apply_credential_policy;

    /// If the credential doesn't support scopes, this function pointer is `NULL`.
    _az_credential_set_scopes_fn set_scopes;
  } _internal;
} _az_credential;

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_CREDENTIALS_H
