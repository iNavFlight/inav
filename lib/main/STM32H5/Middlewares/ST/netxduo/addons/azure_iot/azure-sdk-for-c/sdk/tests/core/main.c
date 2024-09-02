// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#include <stdlib.h>

#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <cmocka.h>

#include "az_test_definitions.h"

#include <azure/core/_az_cfg.h>

int main()
{
  int result = 0;

  // every test function returns the number of tests failed, 0 means success (there shouldn't be
  // negative numbers
  result += test_az_base64();
  result += test_az_context();
  result += test_az_http();
  result += test_az_json();
  result += test_az_logging();
  result += test_az_pipeline();
  result += test_az_policy();
  result += test_az_span();
  result += test_az_url_encode();

  return result;
}
