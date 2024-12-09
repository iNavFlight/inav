// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include <stdlib.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>

#include "test_az_iot_provisioning_client.h"

int main()
{
  int result = 0;

  result += test_az_iot_provisioning_client();
  result += test_az_iot_provisioning_client_sas_token();
  result += test_az_iot_provisioning_client_parser();
  result += test_az_iot_provisioning_client_payload();

  return result;
}
