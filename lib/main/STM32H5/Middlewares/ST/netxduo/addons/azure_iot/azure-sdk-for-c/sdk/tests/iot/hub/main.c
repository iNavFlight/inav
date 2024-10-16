// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include <stdlib.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>

#include "test_az_iot_hub_client.h"

int main()
{
  int result = 0;

  result += test_az_iot_hub_client();
  result += test_az_iot_hub_client_c2d();
  result += test_az_iot_hub_client_methods();
  result += test_az_iot_hub_client_sas_token();
  result += test_az_iot_hub_client_telemetry();
  result += test_az_iot_hub_client_twin();
  result += test_az_iot_hub_client_commands();
  result += test_az_iot_hub_client_properties();

  return result;
}
