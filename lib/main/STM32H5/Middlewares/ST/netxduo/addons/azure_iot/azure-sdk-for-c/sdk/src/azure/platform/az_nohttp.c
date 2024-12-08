// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include <azure/core/az_http_transport.h>

#include <azure/core/_az_cfg.h>

AZ_NODISCARD az_result
az_http_client_send_request(az_http_request const* request, az_http_response* ref_response)
{
  (void)request;
  (void)ref_response;
  return AZ_ERROR_DEPENDENCY_NOT_PROVIDED;
}
