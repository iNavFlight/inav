// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#include "az_test_definitions.h"
#include <azure/core/internal/az_span_internal.h>

#include <az_test_precondition.h>

#include <stdarg.h>
#include <stddef.h>

#include <limits.h>
#include <setjmp.h>
#include <stdint.h>

#include <cmocka.h>

#include <azure/core/_az_cfg.h>

static void test_url_encode_basic(void** state)
{
  (void)state;
  {
    // Empty (null) input span, empty non-null output span.
    uint8_t buf20[20] = {
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    };

    az_span const buffer0 = az_span_slice(AZ_SPAN_FROM_BUFFER(buf20), 0, 0);

    int32_t url_length = 0xFF;
    assert_true(az_result_succeeded(_az_span_url_encode(buffer0, AZ_SPAN_EMPTY, &url_length)));
    assert_int_equal(url_length, 0);
    assert_true(az_span_is_content_equal(
        AZ_SPAN_FROM_BUFFER(buf20), AZ_SPAN_FROM_STR("********************")));
  }
  {
    // Empty (non-null) input span, empty non-null output span.
    uint8_t buf20[20] = {
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    };

    az_span const buffer0 = az_span_slice(AZ_SPAN_FROM_BUFFER(buf20), 0, 0);

    uint8_t buf1[1] = { 'A' };
    az_span const buffer0input = az_span_slice(AZ_SPAN_FROM_BUFFER(buf1), 0, 0);

    int32_t url_length = 0xFF;
    assert_true(az_result_succeeded(_az_span_url_encode(buffer0, buffer0input, &url_length)));
    assert_int_equal(url_length, 0);
    assert_true(az_span_is_content_equal(
        AZ_SPAN_FROM_BUFFER(buf20), AZ_SPAN_FROM_STR("********************")));
  }
  {
    // Just enough to succeed, but not percent-encode.
    uint8_t buf20[20] = {
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    };

    az_span const buffer5 = az_span_slice(AZ_SPAN_FROM_BUFFER(buf20), 0, 5);

    int32_t url_length = 0xFF;
    assert_true(
        az_result_succeeded(_az_span_url_encode(buffer5, AZ_SPAN_FROM_STR("AbCdE"), &url_length)));

    assert_int_equal(url_length, sizeof("AbCdE") - 1);
    assert_true(az_span_is_content_equal(
        AZ_SPAN_FROM_BUFFER(buf20), AZ_SPAN_FROM_STR("AbCdE***************")));
  }
  {
    // Percent-encode single character.
    uint8_t buf20[20] = {
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    };

    az_span const buffer7 = az_span_slice(AZ_SPAN_FROM_BUFFER(buf20), 0, 7);

    int32_t url_length = 0xFF;
    assert_true(
        az_result_succeeded(_az_span_url_encode(buffer7, AZ_SPAN_FROM_STR("aBc/g"), &url_length)));

    assert_int_equal(url_length, sizeof("aBc%2Fg") - 1);
    assert_true(az_span_is_content_equal(
        AZ_SPAN_FROM_BUFFER(buf20), AZ_SPAN_FROM_STR("aBc%2Fg*************")));
  }
  {
    // Could've been enough space to encode, but the character needs percent-encoding.
    uint8_t buf20[20] = {
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    };

    az_span const buffer2 = az_span_slice(AZ_SPAN_FROM_BUFFER(buf20), 0, 2);

    int32_t url_length = 0xFF;
    assert_true(
        _az_span_url_encode(buffer2, AZ_SPAN_FROM_STR("/"), &url_length)
        == AZ_ERROR_NOT_ENOUGH_SPACE);

    assert_int_equal(url_length, 0);
    assert_true(az_span_is_content_equal(
        AZ_SPAN_FROM_BUFFER(buf20), AZ_SPAN_FROM_STR("********************")));
  }
  {
    // Single character needs encoding, and there's enough space.
    uint8_t buf20[20] = {
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    };

    az_span const buffer3 = az_span_slice(AZ_SPAN_FROM_BUFFER(buf20), 0, 3);

    int32_t url_length = 0xFF;
    assert_true(
        az_result_succeeded(_az_span_url_encode(buffer3, AZ_SPAN_FROM_STR("/"), &url_length)));

    assert_int_equal(url_length, sizeof("%2F") - 1);
    assert_true(az_span_is_content_equal(
        AZ_SPAN_FROM_BUFFER(buf20), AZ_SPAN_FROM_STR("%2F*****************")));
  }
  {
    // Enough space to encode 3 characters, regardless of input.
    uint8_t buf20[20] = {
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    };

    az_span const buffer9 = az_span_slice(AZ_SPAN_FROM_BUFFER(buf20), 0, 9);

    int32_t url_length = 0xFF;
    assert_true(
        az_result_succeeded(_az_span_url_encode(buffer9, AZ_SPAN_FROM_STR("///"), &url_length)));

    assert_int_equal(url_length, sizeof("%2F%2F%2F") - 1);
    assert_true(az_span_is_content_equal(
        AZ_SPAN_FROM_BUFFER(buf20), AZ_SPAN_FROM_STR("%2F%2F%2F***********")));
  }
  {
    // More than enough space to encode 3 characters, regardless of input.
    uint8_t buf20[20] = {
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    };

    az_span const buffer12 = az_span_slice(AZ_SPAN_FROM_BUFFER(buf20), 0, 12);

    int32_t url_length = 0xFF;
    assert_true(
        az_result_succeeded(_az_span_url_encode(buffer12, AZ_SPAN_FROM_STR("///"), &url_length)));

    assert_int_equal(url_length, sizeof("%2F%2F%2F") - 1);
    assert_true(az_span_is_content_equal(
        AZ_SPAN_FROM_BUFFER(buf20), AZ_SPAN_FROM_STR("%2F%2F%2F***********")));
  }
  {
    // Could've been enough space to encode 3 characters, but there's only space for two.
    uint8_t buf20[20] = {
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    };

    az_span const buffer10 = az_span_slice(AZ_SPAN_FROM_BUFFER(buf20), 0, 10);

    int32_t url_length = 0xFF;
    assert_true(
        _az_span_url_encode(buffer10, AZ_SPAN_FROM_STR("AbC///"), &url_length)
        == AZ_ERROR_NOT_ENOUGH_SPACE);

    assert_int_equal(url_length, 0);
    assert_true(az_span_is_content_equal(
        AZ_SPAN_FROM_BUFFER(buf20), AZ_SPAN_FROM_STR("AbC%2F%2F***********")));
  }
  {
    // Could've been enough space to encode 3 characters, but there's only space for two
    // Slightly bigger buffer, still not big enough.
    uint8_t buf20[20] = {
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    };

    az_span const buffer11 = az_span_slice(AZ_SPAN_FROM_BUFFER(buf20), 0, 11);

    int32_t url_length = 0xFF;
    assert_true(
        _az_span_url_encode(buffer11, AZ_SPAN_FROM_STR("AbC///"), &url_length)
        == AZ_ERROR_NOT_ENOUGH_SPACE);

    assert_int_equal(url_length, 0);
    assert_true(az_span_is_content_equal(
        AZ_SPAN_FROM_BUFFER(buf20), AZ_SPAN_FROM_STR("AbC%2F%2F***********")));
  }
  {
    // Could've been enough space to encode 3 characters, and there's just enough space.
    uint8_t buf20[20] = {
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
      '*', '*', '*', '*', '*', '*', '*', '*', '*', '*',
    };

    az_span const buffer12 = az_span_slice(AZ_SPAN_FROM_BUFFER(buf20), 0, 12);

    int32_t url_length = 0xFF;
    assert_true(az_result_succeeded(
        _az_span_url_encode(buffer12, AZ_SPAN_FROM_STR("AbC///"), &url_length)));

    assert_int_equal(url_length, sizeof("AbC%2F%2F%2F") - 1);
    assert_true(az_span_is_content_equal(
        AZ_SPAN_FROM_BUFFER(buf20), AZ_SPAN_FROM_STR("AbC%2F%2F%2F********")));
  }
  {
    int32_t url_length = _az_span_url_encode_calc_length(AZ_SPAN_FROM_STR("aBc/g"));
    assert_int_equal(url_length, sizeof("aBc%2Fg") - 1);
  }
  {
    // Empty span
    int32_t url_length = _az_span_url_encode_calc_length(AZ_SPAN_EMPTY);
    assert_int_equal(url_length, 0);
    url_length = _az_span_url_encode_calc_length(AZ_SPAN_FROM_STR(""));
    assert_int_equal(url_length, 0);
  }
  {
    // Nothing gets encoded
    int32_t url_length = _az_span_url_encode_calc_length(AZ_SPAN_FROM_STR("123"));
    assert_int_equal(url_length, 3);
  }
  {
    // first last encoded
    int32_t url_length = _az_span_url_encode_calc_length(AZ_SPAN_FROM_STR(" 123 "));
    assert_int_equal(url_length, 9);
  }
  {
    // every character is encoded
    int32_t url_length = _az_span_url_encode_calc_length(AZ_SPAN_FROM_STR("   "));
    assert_int_equal(url_length, 9);
  }
  {
    // % character, which in itself is encoded as %25
    int32_t url_length = _az_span_url_encode_calc_length(AZ_SPAN_FROM_STR("%"));
    assert_int_equal(url_length, 3);
  }
  {
    // a single length span with the \0 NULL character
    int32_t url_length = _az_span_url_encode_calc_length(AZ_SPAN_FROM_STR("\0"));
    assert_int_equal(url_length, 3);
  }
}

#ifndef AZ_NO_PRECONDITION_CHECKING
ENABLE_PRECONDITION_CHECK_TESTS()
#endif

static void test_url_encode_preconditions(void** state)
{
  (void)state;
#ifdef AZ_NO_PRECONDITION_CHECKING
  {
    {
      // URL encode wouldn't succeed, but encode/copy what would fit.
      uint8_t buf5[5] = { '*', '*', '*', '*', '*' };
      az_span const buffer5 = AZ_SPAN_FROM_BUFFER(buf5);

      int32_t url_length = 0xFF;
      assert_true(
          _az_span_url_encode(buffer5, AZ_SPAN_FROM_STR("1234567890"), &url_length)
          == AZ_ERROR_NOT_ENOUGH_SPACE);

      assert_int_equal(url_length, 0);
      assert_true(az_span_is_content_equal(buffer5, AZ_SPAN_FROM_STR("12345")));
    }
    {
      // Input is empty, so the output is also empty BUT the output span is null.
      int32_t url_length = 0xFF;
      assert_true(
          az_result_succeeded(_az_span_url_encode(AZ_SPAN_EMPTY, AZ_SPAN_EMPTY, &url_length)));
      assert_int_equal(url_length, 0);
    }
    { // Overlapping buffers, same pointer.
      uint8_t buf[13] = { 'a', 'B', 'c', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*' };
      az_span const in_buffer = az_span_slice(AZ_SPAN_FROM_BUFFER(buf), 0, 3);
      az_span const out_buffer = az_span_slice(AZ_SPAN_FROM_BUFFER(buf), 0, 3);

      int32_t url_length = 0xFF;
      assert_true(az_result_succeeded(_az_span_url_encode(out_buffer, in_buffer, &url_length)));
      assert_int_equal(url_length, sizeof("aBc") - 1);
      assert_true(
          az_span_is_content_equal(AZ_SPAN_FROM_BUFFER(buf), AZ_SPAN_FROM_STR("aBc**********")));
    }
    {
      // Overlapping buffers, different pointers.
      uint8_t buf[13] = { 'a', 'B', 'c', '/', '/', '/', '*', '*', '*', '*', '*', '*', '*' };

      az_span const in_buffer = az_span_slice(AZ_SPAN_FROM_BUFFER(buf), 0, 6);
      az_span const out_buffer = az_span_slice(AZ_SPAN_FROM_BUFFER(buf), 1, 13);

      int32_t url_length = 0xFF;
      assert_true(az_result_succeeded(_az_span_url_encode(out_buffer, in_buffer, &url_length)));
      assert_int_equal(url_length, sizeof("aaaaaa") - 1);
      assert_true(
          az_span_is_content_equal(AZ_SPAN_FROM_BUFFER(buf), AZ_SPAN_FROM_STR("aaaaaaa******")));
    }
    {
      // Overlapping buffers, writing before reading.
      uint8_t buf[12] = { '/', '/', '/', '/', '*', '*', '*', '*', '*', '*', '*', '*' };
      az_span const in_buffer = az_span_slice(AZ_SPAN_FROM_BUFFER(buf), 0, 4);
      az_span const out_buffer = az_span_slice(AZ_SPAN_FROM_BUFFER(buf), 0, 12);

      int32_t url_length = 0xFF;
      assert_true(az_result_succeeded(_az_span_url_encode(out_buffer, in_buffer, &url_length)));
      assert_int_equal(url_length, sizeof("%2F2F2") - 1);
      assert_true(
          az_span_is_content_equal(AZ_SPAN_FROM_BUFFER(buf), AZ_SPAN_FROM_STR("%2F2F2******")));
    }
  }
#else
  {
    SETUP_PRECONDITION_CHECK_TESTS();

    {
      // URL encode could never succeed.
      uint8_t buf5[5] = { '*', '*', '*', '*', '*' };
      az_span const buffer5 = AZ_SPAN_FROM_BUFFER(buf5);

      int32_t url_length = 0xFF;
      ASSERT_PRECONDITION_CHECKED(
          _az_span_url_encode(buffer5, AZ_SPAN_FROM_STR("1234567890"), &url_length));

      assert_int_equal(url_length, 0xFF);
      assert_true(az_span_is_content_equal(buffer5, AZ_SPAN_FROM_STR("*****")));
    }

    {
      // Input is empty, so the output is also empty BUT the output span is null.
      // This precondition assert relies on the ptr of an empty span be null, which is not
      // guaranteed. However, it is a reasonable assumption for tests as part of span validation,
      // only for precondition checking.
      int32_t url_length = 0xFF;
      ASSERT_PRECONDITION_CHECKED(_az_span_url_encode(AZ_SPAN_EMPTY, AZ_SPAN_EMPTY, &url_length));
      assert_int_equal(url_length, 0xFF);
    }
    {
      // Overlapping buffers, same pointer.
      uint8_t buf[13] = { 'a', 'B', 'c', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*' };
      az_span const in_buffer = az_span_slice(AZ_SPAN_FROM_BUFFER(buf), 0, 3);
      az_span const out_buffer = az_span_slice(AZ_SPAN_FROM_BUFFER(buf), 0, 3);

      int32_t url_length = 0xFF;
      ASSERT_PRECONDITION_CHECKED(_az_span_url_encode(out_buffer, in_buffer, &url_length));
      assert_int_equal(url_length, 0xFF);
      assert_true(
          az_span_is_content_equal(AZ_SPAN_FROM_BUFFER(buf), AZ_SPAN_FROM_STR("aBc**********")));
    }
    {
      // Overlapping buffers, different pointers.
      uint8_t buf[13] = { 'a', 'B', 'c', '/', '/', '/', '*', '*', '*', '*', '*', '*', '*' };
      az_span const in_buffer = az_span_slice(AZ_SPAN_FROM_BUFFER(buf), 0, 6);
      az_span const out_buffer = az_span_slice(AZ_SPAN_FROM_BUFFER(buf), 1, 13);

      int32_t url_length = 0xFF;
      ASSERT_PRECONDITION_CHECKED(_az_span_url_encode(in_buffer, out_buffer, &url_length));
      assert_int_equal(url_length, 0xFF);
      assert_true(
          az_span_is_content_equal(AZ_SPAN_FROM_BUFFER(buf), AZ_SPAN_FROM_STR("aBc///*******")));
    }
    {
      // Overlapping buffers, writing before reading.
      uint8_t buf[14] = { '/', '/', '/', '/', '*', '*', '*', '*', '*', '*', '*', '*', '*', '*' };
      az_span const in_buffer = az_span_slice(AZ_SPAN_FROM_BUFFER(buf), 0, 4);
      az_span const out_buffer = az_span_slice(AZ_SPAN_FROM_BUFFER(buf), 1, 14);

      int32_t url_length = 0xFF;
      ASSERT_PRECONDITION_CHECKED(_az_span_url_encode(out_buffer, in_buffer, &url_length));
      assert_int_equal(url_length, 0xFF);
      assert_true(
          az_span_is_content_equal(AZ_SPAN_FROM_BUFFER(buf), AZ_SPAN_FROM_STR("////**********")));
    }
    {
      // NULL out_size parameter.
      uint8_t buf1[1] = { '*' };
      az_span const buffer0 = az_span_slice(AZ_SPAN_FROM_BUFFER(buf1), 0, 0);
      ASSERT_PRECONDITION_CHECKED(_az_span_url_encode(buffer0, AZ_SPAN_EMPTY, NULL));
      assert_true(az_span_is_content_equal(AZ_SPAN_FROM_BUFFER(buf1), AZ_SPAN_FROM_STR("*")));
    }
    {
      // precondition -> bigger than INT32_MAX / 3
      az_span simulate_span = { ._internal = { .ptr = NULL, .size = INT32_MAX / 3 + 1 } };
      ASSERT_PRECONDITION_CHECKED(_az_span_url_encode_calc_length(simulate_span));
    }
    {
      // precondition -> less than 0
      az_span simulate_span = { ._internal = { .ptr = NULL, .size = -1 } };
      ASSERT_PRECONDITION_CHECKED(_az_span_url_encode_calc_length(simulate_span));
    }
  }
#endif // AZ_NO_PRECONDITION_CHECKING
}

static void test_url_encode_usage(void** state)
{
  (void)state;

  // Typical use case.
  uint8_t buf[100] = { 0 };
  az_span const buffer = AZ_SPAN_FROM_BUFFER(buf);

  int32_t url_length = 0xFF;
  assert_true(az_result_succeeded(
      _az_span_url_encode(buffer, AZ_SPAN_FROM_STR("https://vault.azure.net"), &url_length)));

  assert_true(az_span_is_content_equal(
      az_span_slice(buffer, 0, url_length), AZ_SPAN_FROM_STR("https%3A%2F%2Fvault.azure.net")));

  assert_int_equal(url_length, sizeof("https%3A%2F%2Fvault.azure.net") - 1);
}

static void test_url_encode_full(void** state)
{
  // Go through all 256 values.
  (void)state;

  uint8_t values256[256] = { 0 };
  for (size_t i = 0; i < _az_COUNTOF(values256); ++i)
  {
    values256[i] = (uint8_t)i;
  }

  uint8_t buf[256 * 3] = { 0 };
  for (size_t i = 0; i < _az_COUNTOF(buf); ++i)
  {
    buf[i] = '*';
  }

  az_span const buffer = AZ_SPAN_FROM_BUFFER(buf);

  int32_t url_length = 0xFF;
  assert_true(az_result_succeeded(
      _az_span_url_encode(buffer, AZ_SPAN_FROM_BUFFER(values256), &url_length)));

  int const unreserved = sizeof("-_.~"
                                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                "abcdefghijklmnopqrstuvwxyz"
                                "0123456789")
      - 1;

  assert_int_equal(url_length, unreserved + (256 - unreserved) * 3);

  assert_true(az_span_is_content_equal(
      buffer,
      AZ_SPAN_FROM_STR("%00%01%02%03%04%05%06%07%08%09%0A%0B%0C%0D%0E%0F"
                       "%10%11%12%13%14%15%16%17%18%19%1A%1B%1C%1D%1E%1F"
                       "%20%21%22%23%24%25%26%27%28%29%2A%2B%2C-.%2F"
                       "0123456789%3A%3B%3C%3D%3E%3F"
                       "%40ABCDEFGHIJKLMNO"
                       "PQRSTUVWXYZ%5B%5C%5D%5E_"
                       "%60abcdefghijklmno"
                       "pqrstuvwxyz%7B%7C%7D~%7F"
                       "%80%81%82%83%84%85%86%87%88%89%8A%8B%8C%8D%8E%8F"
                       "%90%91%92%93%94%95%96%97%98%99%9A%9B%9C%9D%9E%9F"
                       "%A0%A1%A2%A3%A4%A5%A6%A7%A8%A9%AA%AB%AC%AD%AE%AF"
                       "%B0%B1%B2%B3%B4%B5%B6%B7%B8%B9%BA%BB%BC%BD%BE%BF"
                       "%C0%C1%C2%C3%C4%C5%C6%C7%C8%C9%CA%CB%CC%CD%CE%CF"
                       "%D0%D1%D2%D3%D4%D5%D6%D7%D8%D9%DA%DB%DC%DD%DE%DF"
                       "%E0%E1%E2%E3%E4%E5%E6%E7%E8%E9%EA%EB%EC%ED%EE%EF"
                       "%F0%F1%F2%F3%F4%F5%F6%F7%F8%F9%FA%FB%FC%FD%FE%FF"
                       "****************"
                       "****************"
                       "****************"
                       "****************"
                       "****************"
                       "****************"
                       "****************"
                       "****************"
                       "****")));
}

int test_az_url_encode()
{
  struct CMUnitTest const tests[] = {
    cmocka_unit_test(test_url_encode_basic),
    cmocka_unit_test(test_url_encode_preconditions),
    cmocka_unit_test(test_url_encode_usage),
    cmocka_unit_test(test_url_encode_full),
  };

  return cmocka_run_group_tests_name("az_core_encode", tests, NULL, NULL);
}
