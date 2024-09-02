// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT
#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#define strdup _strdup
#endif

#include "az_test_definitions.h"
#include <azure/core/az_json.h>
#include <azure/core/internal/az_result_internal.h>
#include <azure/core/internal/az_span_internal.h>

#include <math.h>
#include <setjmp.h>
#include <stdarg.h>

#include <cmocka.h>

#include <azure/core/_az_cfg.h>
#include <stdlib.h>
#define TEST_EXPECT_SUCCESS(exp) assert_true(az_result_succeeded(exp))

az_result test_allocator(
    az_span_allocator_context* allocator_context,
    az_span* out_next_destination);

az_result test_allocator_never_called(
    az_span_allocator_context* allocator_context,
    az_span* out_next_destination);

az_result test_allocator_always_null(
    az_span_allocator_context* allocator_context,
    az_span* out_next_destination);

az_result test_allocator_chunked(
    az_span_allocator_context* allocator_context,
    az_span* out_next_destination);

#define TEST_JSON_TOKEN_HELPER(token, expected_token_kind, expected_token_slice) \
  do                                                                             \
  {                                                                              \
    assert_int_equal(token.kind, expected_token_kind);                           \
    assert_true(az_span_is_content_equal(token.slice, expected_token_slice));    \
  } while (0)

static void test_json_reader_init(void** state)
{
  (void)state;

  az_json_reader_options options = az_json_reader_options_default();

  az_json_reader reader = { 0 };

  assert_int_equal(reader.current_depth, 0);

  assert_int_equal(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("{}"), NULL), AZ_OK);
  assert_int_equal(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("{}"), &options), AZ_OK);

  assert_int_equal(reader.current_depth, 0);

  // Verify that initialization doesn't process any JSON text, even if it is invalid or incomplete.
  assert_int_equal(az_json_reader_init(&reader, AZ_SPAN_FROM_STR(" "), NULL), AZ_OK);
  assert_int_equal(az_json_reader_init(&reader, AZ_SPAN_FROM_STR(" "), &options), AZ_OK);
  assert_int_equal(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("a"), NULL), AZ_OK);
  assert_int_equal(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("a"), &options), AZ_OK);
  assert_int_equal(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("\""), NULL), AZ_OK);
  assert_int_equal(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("\""), &options), AZ_OK);

  TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NONE, AZ_SPAN_EMPTY);

  assert_int_equal(reader.current_depth, 0);
}

static void test_json_reader_current_depth_array(void** state)
{
  (void)state;

  az_json_reader reader = { 0 };

  assert_int_equal(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("[ ]"), NULL), AZ_OK);
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader.current_depth, 0);
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader.current_depth, 0);

  assert_int_equal(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("[ [ 1, 2, 3] ]"), NULL), AZ_OK);
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 0);
  assert_int_equal(reader.current_depth, 0);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 1);
  assert_int_equal(reader.current_depth, 0);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 2);
  assert_int_equal(reader.current_depth, 1);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 2);
  assert_int_equal(reader.current_depth, 2);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 2);
  assert_int_equal(reader.current_depth, 2);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 2);
  assert_int_equal(reader.current_depth, 2);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 1);
  assert_int_equal(reader.current_depth, 1);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 0);
  assert_int_equal(reader.current_depth, 0);

  assert_int_equal(az_json_reader_next_token(&reader), AZ_ERROR_JSON_READER_DONE);
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 0);
  assert_int_equal(reader.current_depth, 0);
}

static void test_json_reader_current_depth_object(void** state)
{
  (void)state;

  az_json_reader reader = { 0 };

  assert_int_equal(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("{}"), NULL), AZ_OK);
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader.current_depth, 0);
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader.current_depth, 0);

  assert_int_equal(
      az_json_reader_init(&reader, AZ_SPAN_FROM_STR("{\"array\": [1,2,3,{}]}"), NULL), AZ_OK);
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 0);
  assert_int_equal(reader.current_depth, 0);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 1);
  assert_int_equal(reader.current_depth, 0);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 1);
  assert_int_equal(reader.current_depth, 1);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 2);
  assert_int_equal(reader.current_depth, 1);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 2);
  assert_int_equal(reader.current_depth, 2);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 2);
  assert_int_equal(reader.current_depth, 2);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 2);
  assert_int_equal(reader.current_depth, 2);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 3);
  assert_int_equal(reader.current_depth, 2);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 2);
  assert_int_equal(reader.current_depth, 2);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 1);
  assert_int_equal(reader.current_depth, 1);

  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 0);
  assert_int_equal(reader.current_depth, 0);

  assert_int_equal(az_json_reader_next_token(&reader), AZ_ERROR_JSON_READER_DONE);
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 0);
  assert_int_equal(reader.current_depth, 0);
}

/**  Json writer **/
static void test_json_writer(void** state)
{
  (void)state;
  {
    uint8_t array[200] = { 0 };
    az_json_writer writer = { 0 };

    TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));

    // 0___________________________________________________________________________________________________1
    // 0_________1_________2_________3_________4_________5_________6_________7_________8_________9_________0
    // 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456
    // {"name":true,"foo":["bar",null,0,-12,12,9007199254740991],"int-max":2147483647,"esc":"_\"_\\_\b\f\n\r\t_","u":"a\u001Fb"}
    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("name")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_bool(&writer, true));

    {
      TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("foo")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_array(&writer));
      az_result e = az_json_writer_append_string(&writer, AZ_SPAN_FROM_STR("bar"));
      TEST_EXPECT_SUCCESS(e);
      TEST_EXPECT_SUCCESS(az_json_writer_append_null(&writer));
      TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 0));
      TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, -12));
      TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, 12.1, 0));
      TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, 9007199254740991ull, 0));
      TEST_EXPECT_SUCCESS(az_json_writer_append_end_array(&writer));
    }

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("int-max")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 2147483647));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("esc")));
    TEST_EXPECT_SUCCESS(
        az_json_writer_append_string(&writer, AZ_SPAN_FROM_STR("_\"_\\_\b\f\n\r\t_")));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("u")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_string(
        &writer,
        AZ_SPAN_FROM_STR( //
            "a"
            "\x1f"
            "b")));

    TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));

    az_span_to_str((char*)array, 200, az_json_writer_get_bytes_used_in_destination(&writer));

    assert_string_equal(
        array,
        "{"
        "\"name\":true,"
        "\"foo\":[\"bar\",null,0,-12,12,9007199254740991],"
        "\"int-max\":2147483647,"
        "\"esc\":\"_\\\"_\\\\_\\b\\f\\n\\r\\t_\","
        "\"u\":\"a\\u001Fb\""
        "}");
  }
  {
    uint8_t array[33] = { 0 };
    az_json_writer writer = { 0 };
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));

      TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, 0.000000000000001, 15));

      az_span_to_str((char*)array, 33, az_json_writer_get_bytes_used_in_destination(&writer));
      assert_string_equal(array, "0.000000000000001");
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));

      TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, 1e-300, 15));

      az_span_to_str((char*)array, 33, az_json_writer_get_bytes_used_in_destination(&writer));
      assert_string_equal(array, "0");
    }
  }
  {
    // json with AZ_JSON_TOKEN_STRING
    uint8_t array[200] = { 0 };
    az_json_writer writer = { 0 };
    TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));

    // this json { "span": "\" } would be scaped to { "span": "\\"" }
    uint8_t single_char[1] = { '\\' }; // char = '\'
    az_span single_span = AZ_SPAN_FROM_BUFFER(single_char);

    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("span")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_string(&writer, single_span));

    TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));

    az_span expected = AZ_SPAN_FROM_STR("{"
                                        "\"span\":\"\\\\\""
                                        "}");

    assert_true(
        az_span_is_content_equal(az_json_writer_get_bytes_used_in_destination(&writer), expected));
  }
  {
    // json with array and object inside
    uint8_t array[200] = { 0 };
    az_json_writer writer = { 0 };
    TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));

    // this json { "array": [1, 2, {}, 3, -12.3 ] }
    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("array")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_array(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 1));
    TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 2));

    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));
    TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 3));

    TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, -1.234e1, 1));

    TEST_EXPECT_SUCCESS(az_json_writer_append_end_array(&writer));
    TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));

    assert_true(az_span_is_content_equal(
        az_json_writer_get_bytes_used_in_destination(&writer),
        AZ_SPAN_FROM_STR( //
            "{"
            "\"array\":[1,2,{},3,-12.3]"
            "}")));
  }
  {
    uint8_t nested_object_array[200] = { 0 };
    az_json_writer nested_object_writer = { 0 };
    {
      // 0___________________________________________________________________________________________________1
      // 0_________1_________2_________3_________4_________5_________6_________7_________8_________9_________0
      // 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456
      // {"bar":true}
      TEST_EXPECT_SUCCESS(az_json_writer_init(
          &nested_object_writer, AZ_SPAN_FROM_BUFFER(nested_object_array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&nested_object_writer));
      TEST_EXPECT_SUCCESS(
          az_json_writer_append_property_name(&nested_object_writer, AZ_SPAN_FROM_STR("bar")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_bool(&nested_object_writer, true));
      TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&nested_object_writer));

      assert_true(az_span_is_content_equal(
          az_json_writer_get_bytes_used_in_destination(&nested_object_writer),
          AZ_SPAN_FROM_STR( //
              "{"
              "\"bar\":true"
              "}")));
    }
  }
  {
    az_json_writer writer = { 0 };
    TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_EMPTY, NULL));
    assert_int_equal(az_json_writer_append_int32(&writer, 1), AZ_ERROR_NOT_ENOUGH_SPACE);
  }
}

static void test_json_writer_append_nested(void** state)
{
  (void)state;
  {
    uint8_t array[200] = { 0 };
    az_json_writer writer = { 0 };

    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));

      // Intentionally include non-normalized spacing and character escaping to validate
      // the data is copied as is, after validation.
      TEST_EXPECT_SUCCESS(az_json_writer_append_json_text(
          &writer, AZ_SPAN_FROM_STR("{\"name\":  \"f\\u0065o\", \"values\": [1, 2, 3,{}]}")));

      az_span_to_str((char*)array, 200, az_json_writer_get_bytes_used_in_destination(&writer));
      assert_string_equal(array, "{\"name\":  \"f\\u0065o\", \"values\": [1, 2, 3,{}]}");
    }

    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_array(&writer));
      TEST_EXPECT_SUCCESS(az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("1")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("2")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("3")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_end_array(&writer));

      az_span_to_str((char*)array, 200, az_json_writer_get_bytes_used_in_destination(&writer));
      assert_int_equal(7, az_span_size(az_json_writer_get_bytes_used_in_destination(&writer)));
      assert_int_equal(7, writer.total_bytes_written);
      assert_int_equal(7, writer._internal.bytes_written);
      assert_string_equal(array, "[1,2,3]");
    }

    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_array(&writer));
      TEST_EXPECT_SUCCESS(az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("123  ")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_end_array(&writer));

      az_span_to_str((char*)array, 200, az_json_writer_get_bytes_used_in_destination(&writer));
      assert_string_equal(array, "[123  ]");
    }

    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));
      TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("name")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_bool(&writer, true));
      TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("foo")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_json_text(
          &writer, AZ_SPAN_FROM_STR("[\"bar\",null,0,-12,12,9007199254740991]")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));

      az_span_to_str((char*)array, 200, az_json_writer_get_bytes_used_in_destination(&writer));
      assert_string_equal(
          array,
          "{"
          "\"name\":true,"
          "\"foo\":[\"bar\",null,0,-12,12,9007199254740991]"
          "}");
    }

    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_array(&writer));
      TEST_EXPECT_SUCCESS(az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("{\"a\": 1}")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("{\"b\": 2}")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_end_array(&writer));

      az_span_to_str((char*)array, 200, az_json_writer_get_bytes_used_in_destination(&writer));
      assert_string_equal(
          array,
          "["
          "{\"a\": 1},"
          "{\"b\": 2}"
          "]");
    }

    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));
      TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("foo")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("[]")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("bar")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("[1]")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));

      az_span_to_str((char*)array, 200, az_json_writer_get_bytes_used_in_destination(&writer));
      assert_string_equal(
          array,
          "{"
          "\"foo\":[],"
          "\"bar\":[1]"
          "}");
    }

    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_array(&writer));
      TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 42));
      TEST_EXPECT_SUCCESS(az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("\"a\"  ")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("\"b\"")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 24));
      TEST_EXPECT_SUCCESS(az_json_writer_append_end_array(&writer));

      az_span_to_str((char*)array, 200, az_json_writer_get_bytes_used_in_destination(&writer));
      assert_string_equal(array, "[42,\"a\"  ,\"b\",24]");
    }
  }
}

static void test_json_writer_append_nested_invalid(void** state)
{
  (void)state;
  {
    uint8_t array[200] = { 0 };
    az_json_writer writer = { 0 };

    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      assert_int_equal(
          az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("\"name\":  \"f\\u0065o\"")),
          AZ_ERROR_UNEXPECTED_CHAR);
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      assert_int_equal(
          az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("{]")),
          AZ_ERROR_UNEXPECTED_CHAR);
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));
      TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("name")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_bool(&writer, true));

      TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("foo")));
      assert_int_equal(
          az_json_writer_append_json_text(
              &writer, AZ_SPAN_FROM_STR("\"bar\",null,0,-12,12,9007199254740991")),
          AZ_ERROR_UNEXPECTED_CHAR);
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      assert_int_equal(
          az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("fals ")),
          AZ_ERROR_UNEXPECTED_CHAR);
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_array(&writer));
      assert_int_equal(
          az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("{\"a\": 1},")),
          AZ_ERROR_UNEXPECTED_CHAR);
      TEST_EXPECT_SUCCESS(az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("{\"b\": 2}")));
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      assert_int_equal(
          az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("  ")),
          AZ_ERROR_UNEXPECTED_END);
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      assert_int_equal(
          az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("{\"name")),
          AZ_ERROR_UNEXPECTED_END);
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      assert_int_equal(
          az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("{\"name\":  ")),
          AZ_ERROR_UNEXPECTED_END);
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("1")));
      assert_int_equal(
          az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("2")),
          AZ_ERROR_JSON_INVALID_STATE);
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));
      assert_int_equal(
          az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("{}")),
          AZ_ERROR_JSON_INVALID_STATE);
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));
      assert_int_equal(
          az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("1")),
          AZ_ERROR_JSON_INVALID_STATE);
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 123));
      assert_int_equal(
          az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("123")),
          AZ_ERROR_JSON_INVALID_STATE);
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));
      TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));
      assert_int_equal(
          az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("123")),
          AZ_ERROR_JSON_INVALID_STATE);
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_init(&writer, AZ_SPAN_FROM_BUFFER(array), NULL));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_array(&writer));
      TEST_EXPECT_SUCCESS(az_json_writer_append_end_array(&writer));
      assert_int_equal(
          az_json_writer_append_json_text(&writer, AZ_SPAN_FROM_STR("true")),
          AZ_ERROR_JSON_INVALID_STATE);
    }
  }
}

static uint8_t json_array[200] = { 0 };

typedef struct
{
  int32_t* current_index;
} _az_user_context;

az_result test_allocator(
    az_span_allocator_context* allocator_context,
    az_span* out_next_destination)
{
  _az_user_context* user_context = (_az_user_context*)allocator_context->user_context;
  int32_t current_index = *user_context->current_index + allocator_context->bytes_used;

  if (current_index + allocator_context->minimum_required_size > 200)
  {
    current_index = 0;
  }
  assert_true(current_index + allocator_context->minimum_required_size <= 200);

  *out_next_destination = az_span_slice(
      AZ_SPAN_FROM_BUFFER(json_array),
      current_index,
      current_index + allocator_context->minimum_required_size);

  *user_context->current_index = current_index;

  return AZ_OK;
}

az_result test_allocator_always_null(
    az_span_allocator_context* allocator_context,
    az_span* out_next_destination)
{
  (void)allocator_context;
  *out_next_destination = AZ_SPAN_EMPTY;

  return AZ_OK;
}

static void test_json_writer_chunked(void** state)
{
  (void)state;
  {
    az_json_writer writer = { 0 };
    az_span_allocator_fn allocator = &test_allocator;
    int32_t previous = 0;
    _az_user_context user_context = { .current_index = &previous };

    TEST_EXPECT_SUCCESS(
        az_json_writer_chunked_init(&writer, AZ_SPAN_EMPTY, allocator, (void*)&user_context, NULL));

    // 0___________________________________________________________________________________________________1
    // 0_________1_________2_________3_________4_________5_________6_________7_________8_________9_________0
    // 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456
    // {"name":true,"foo":["bar",null,0,-12,12,9007199254740991],"int-max":2147483647,"esc":"_\"_\\_\b\f\n\r\t_","u":"a\u001Fb"}
    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("name")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_bool(&writer, true));

    {
      TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("foo")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_array(&writer));
      az_result e = az_json_writer_append_string(&writer, AZ_SPAN_FROM_STR("bar"));
      TEST_EXPECT_SUCCESS(e);
      TEST_EXPECT_SUCCESS(az_json_writer_append_null(&writer));
      TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 0));
      TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, -12));
      TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, 12.1, 0));
      TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, 9007199254740991ull, 0));
      TEST_EXPECT_SUCCESS(az_json_writer_append_end_array(&writer));
    }

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("int-max")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 2147483647));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("esc")));
    TEST_EXPECT_SUCCESS(
        az_json_writer_append_string(&writer, AZ_SPAN_FROM_STR("_\"_\\_\b\f\n\r\t_")));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("u")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_string(
        &writer,
        AZ_SPAN_FROM_STR( //
            "a"
            "\x1f"
            "b")));

    TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));

    uint8_t array[200] = { 0 };

    az_span_to_str((char*)array, 200, az_json_writer_get_bytes_used_in_destination(&writer));
    assert_string_equal(array, "\",\"u\":\"a\\u001Fb\"}");

    az_span_to_str(
        (char*)array,
        200,
        az_span_slice(AZ_SPAN_FROM_BUFFER(json_array), 0, writer.total_bytes_written));

    assert_string_equal(
        array,
        "{"
        "\"name\":true,"
        "\"foo\":[\"bar\",null,0,-12,12,9007199254740991],"
        "\"int-max\":2147483647,"
        "\"esc\":\"_\\\"_\\\\_\\b\\f\\n\\r\\t_\","
        "\"u\":\"a\\u001Fb\""
        "}");
  }
  {
    az_json_writer writer = { 0 };
    az_span_allocator_fn allocator = &test_allocator;
    int32_t previous = 0;
    _az_user_context user_context = { .current_index = &previous };

    {
      TEST_EXPECT_SUCCESS(az_json_writer_chunked_init(
          &writer, AZ_SPAN_EMPTY, allocator, (void*)&user_context, NULL));

      TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, 0.000000000000001, 15));

      uint8_t array[200] = { 0 };

      az_span_to_str((char*)array, 200, az_json_writer_get_bytes_used_in_destination(&writer));
      assert_string_equal(array, "0.000000000000001");

      az_span_to_str(
          (char*)array,
          200,
          az_span_slice(AZ_SPAN_FROM_BUFFER(json_array), 0, writer.total_bytes_written));

      assert_string_equal(array, "0.000000000000001");
    }
    {
      TEST_EXPECT_SUCCESS(az_json_writer_chunked_init(
          &writer, AZ_SPAN_EMPTY, allocator, (void*)&user_context, NULL));

      TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, 1e-300, 15));

      uint8_t array[200] = { 0 };

      az_span_to_str((char*)array, 200, az_json_writer_get_bytes_used_in_destination(&writer));
      assert_string_equal(array, "0");

      az_span_to_str(
          (char*)array,
          200,
          az_span_slice(AZ_SPAN_FROM_BUFFER(json_array), 0, writer.total_bytes_written));

      assert_string_equal(array, "0");
    }
  }
  {
    // json with AZ_JSON_TOKEN_STRING
    az_json_writer writer = { 0 };
    az_span_allocator_fn allocator = &test_allocator;
    int32_t previous = 0;
    _az_user_context user_context = { .current_index = &previous };

    TEST_EXPECT_SUCCESS(
        az_json_writer_chunked_init(&writer, AZ_SPAN_EMPTY, allocator, (void*)&user_context, NULL));

    // this json { "span": "\" } would be scaped to { "span": "\\"" }
    uint8_t single_char[1] = { '\\' }; // char = '\'
    az_span single_span = AZ_SPAN_FROM_BUFFER(single_char);

    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("span")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_string(&writer, single_span));

    TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));

    uint8_t array[200] = { 0 };

    az_span_to_str((char*)array, 200, az_json_writer_get_bytes_used_in_destination(&writer));
    assert_string_equal(array, "}");

    az_span_to_str(
        (char*)array,
        200,
        az_span_slice(AZ_SPAN_FROM_BUFFER(json_array), 0, writer.total_bytes_written));

    assert_string_equal(
        array,
        "{"
        "\"span\":\"\\\\\""
        "}");
  }
  {
    // json with array and object inside
    az_json_writer writer = { 0 };
    az_span_allocator_fn allocator = &test_allocator;
    int32_t previous = 0;
    _az_user_context user_context = { .current_index = &previous };

    TEST_EXPECT_SUCCESS(
        az_json_writer_chunked_init(&writer, AZ_SPAN_EMPTY, allocator, (void*)&user_context, NULL));

    // this json { "array": [1, 2, {}, 3, -12.3 ] }
    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("array")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_array(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 1));
    TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 2));

    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));
    TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 3));

    TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, -1.234e1, 1));

    TEST_EXPECT_SUCCESS(az_json_writer_append_end_array(&writer));
    TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));

    uint8_t array[200] = { 0 };

    az_span_to_str((char*)array, 200, az_json_writer_get_bytes_used_in_destination(&writer));
    assert_string_equal(array, ",-12.3]}");

    az_span_to_str(
        (char*)array,
        200,
        az_span_slice(AZ_SPAN_FROM_BUFFER(json_array), 0, writer.total_bytes_written));

    assert_string_equal(
        array,
        "{"
        "\"array\":[1,2,{},3,-12.3]"
        "}");
  }
  {
    az_json_writer nested_object_builder = { 0 };
    az_span_allocator_fn allocator = &test_allocator;
    int32_t previous = 0;
    _az_user_context user_context = { .current_index = &previous };

    TEST_EXPECT_SUCCESS(az_json_writer_chunked_init(
        &nested_object_builder, AZ_SPAN_EMPTY, allocator, (void*)&user_context, NULL));

    {
      // 0___________________________________________________________________________________________________1
      // 0_________1_________2_________3_________4_________5_________6_________7_________8_________9_________0
      // 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456
      // {"bar":true}
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&nested_object_builder));
      TEST_EXPECT_SUCCESS(
          az_json_writer_append_property_name(&nested_object_builder, AZ_SPAN_FROM_STR("bar")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_bool(&nested_object_builder, true));
      TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&nested_object_builder));

      uint8_t array[200] = { 0 };

      az_span_to_str(
          (char*)array, 200, az_json_writer_get_bytes_used_in_destination(&nested_object_builder));
      assert_string_equal(array, "}");

      az_span_to_str(
          (char*)array,
          200,
          az_span_slice(
              AZ_SPAN_FROM_BUFFER(json_array), 0, nested_object_builder.total_bytes_written));

      assert_string_equal(
          array,
          "{"
          "\"bar\":true"
          "}");
    }
  }
  {
    az_json_writer writer = { 0 };
    az_span_allocator_fn allocator = &test_allocator_always_null;

    TEST_EXPECT_SUCCESS(az_json_writer_chunked_init(&writer, AZ_SPAN_EMPTY, allocator, NULL, NULL));
    assert_int_equal(az_json_writer_append_int32(&writer, 1), AZ_ERROR_NOT_ENOUGH_SPACE);
  }
}

az_result test_allocator_never_called(
    az_span_allocator_context* allocator_context,
    az_span* out_next_destination)
{
  assert_true(false);
  (void)allocator_context;
  (void)out_next_destination;

  return AZ_ERROR_NOT_SUPPORTED;
}

static void test_json_writer_chunked_no_callback(void** state)
{
  (void)state;
  {
    uint8_t array[200] = { 0 };
    az_json_writer writer = { 0 };

    az_span_allocator_fn allocator = &test_allocator_never_called;

    TEST_EXPECT_SUCCESS(
        az_json_writer_chunked_init(&writer, AZ_SPAN_FROM_BUFFER(array), allocator, NULL, NULL));

    // 0___________________________________________________________________________________________________1
    // 0_________1_________2_________3_________4_________5_________6_________7_________8_________9_________0
    // 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456
    // {"name":true,"foo":["bar",null,0,-12,12,9007199254740991],"int-max":9007199254740991,"esc":"_\"_\\_\b\f\n\r\t_","u":"a\u001Fb"}
    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("name")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_bool(&writer, true));

    {
      TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("foo")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_array(&writer));
      az_result e = az_json_writer_append_string(&writer, AZ_SPAN_FROM_STR("bar"));
      TEST_EXPECT_SUCCESS(e);
      TEST_EXPECT_SUCCESS(az_json_writer_append_null(&writer));
      TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 0));
      TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, -12));
      TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, 12.1, 0));
      TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, 9007199254740991ull, 0));
      TEST_EXPECT_SUCCESS(az_json_writer_append_end_array(&writer));
    }

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("int-max")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 2147483647));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("esc")));
    TEST_EXPECT_SUCCESS(
        az_json_writer_append_string(&writer, AZ_SPAN_FROM_STR("_\"_\\_\b\f\n\r\t_")));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("u")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_string(
        &writer,
        AZ_SPAN_FROM_STR( //
            "a"
            "\x1f"
            "b")));

    TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));

    az_span_to_str((char*)array, 200, az_json_writer_get_bytes_used_in_destination(&writer));

    assert_string_equal(
        array,
        "{"
        "\"name\":true,"
        "\"foo\":[\"bar\",null,0,-12,12,9007199254740991],"
        "\"int-max\":2147483647,"
        "\"esc\":\"_\\\"_\\\\_\\b\\f\\n\\r\\t_\","
        "\"u\":\"a\\u001Fb\""
        "}");
  }
  {
    uint8_t array[200] = { 0 };
    az_json_writer writer = { 0 };

    az_span_allocator_fn allocator = &test_allocator_never_called;

    {
      TEST_EXPECT_SUCCESS(
          az_json_writer_chunked_init(&writer, AZ_SPAN_FROM_BUFFER(array), allocator, NULL, NULL));

      TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, 0.000000000000001, 15));

      az_span_to_str((char*)array, 33, az_json_writer_get_bytes_used_in_destination(&writer));
      assert_string_equal(array, "0.000000000000001");
    }
    {
      TEST_EXPECT_SUCCESS(
          az_json_writer_chunked_init(&writer, AZ_SPAN_FROM_BUFFER(array), allocator, NULL, NULL));

      TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, 1e-300, 15));

      az_span_to_str((char*)array, 33, az_json_writer_get_bytes_used_in_destination(&writer));
      assert_string_equal(array, "0");
    }
  }
  {
    // json with AZ_JSON_TOKEN_STRING
    uint8_t array[200] = { 0 };
    az_json_writer writer = { 0 };

    az_span_allocator_fn allocator = &test_allocator_never_called;

    TEST_EXPECT_SUCCESS(
        az_json_writer_chunked_init(&writer, AZ_SPAN_FROM_BUFFER(array), allocator, NULL, NULL));

    // this json { "span": "\" } would be scaped to { "span": "\\"" }
    uint8_t single_char[1] = { '\\' }; // char = '\'
    az_span single_span = AZ_SPAN_FROM_BUFFER(single_char);

    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("span")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_string(&writer, single_span));

    TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));

    az_span expected = AZ_SPAN_FROM_STR("{"
                                        "\"span\":\"\\\\\""
                                        "}");

    assert_true(
        az_span_is_content_equal(az_json_writer_get_bytes_used_in_destination(&writer), expected));
  }
  {
    // json with array and object inside
    uint8_t array[200] = { 0 };
    az_json_writer writer = { 0 };

    az_span_allocator_fn allocator = &test_allocator_never_called;

    TEST_EXPECT_SUCCESS(
        az_json_writer_chunked_init(&writer, AZ_SPAN_FROM_BUFFER(array), allocator, NULL, NULL));

    // this json { "array": [1, 2, {}, 3, -12.3 ] }
    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_property_name(&writer, AZ_SPAN_FROM_STR("array")));
    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_array(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 1));
    TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 2));

    TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&writer));
    TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));

    TEST_EXPECT_SUCCESS(az_json_writer_append_int32(&writer, 3));

    TEST_EXPECT_SUCCESS(az_json_writer_append_double(&writer, -1.234e1, 1));

    TEST_EXPECT_SUCCESS(az_json_writer_append_end_array(&writer));
    TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&writer));

    assert_true(az_span_is_content_equal(
        az_json_writer_get_bytes_used_in_destination(&writer),
        AZ_SPAN_FROM_STR( //
            "{"
            "\"array\":[1,2,{},3,-12.3]"
            "}")));
  }
  {
    uint8_t nested_object_array[200] = { 0 };
    az_json_writer nested_object_builder = { 0 };

    az_span_allocator_fn allocator = &test_allocator_never_called;

    TEST_EXPECT_SUCCESS(az_json_writer_chunked_init(
        &nested_object_builder, AZ_SPAN_FROM_BUFFER(nested_object_array), allocator, NULL, NULL));

    {
      // 0___________________________________________________________________________________________________1
      // 0_________1_________2_________3_________4_________5_________6_________7_________8_________9_________0
      // 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456
      // {"bar":true}
      TEST_EXPECT_SUCCESS(az_json_writer_append_begin_object(&nested_object_builder));
      TEST_EXPECT_SUCCESS(
          az_json_writer_append_property_name(&nested_object_builder, AZ_SPAN_FROM_STR("bar")));
      TEST_EXPECT_SUCCESS(az_json_writer_append_bool(&nested_object_builder, true));
      TEST_EXPECT_SUCCESS(az_json_writer_append_end_object(&nested_object_builder));

      assert_true(az_span_is_content_equal(
          az_json_writer_get_bytes_used_in_destination(&nested_object_builder),
          AZ_SPAN_FROM_STR( //
              "{"
              "\"bar\":true"
              "}")));
    }
  }
}

static uint8_t json_chunked_array_256[10][256] = { 0 };
static az_span json_buffers[10] = { 0 };

az_result test_allocator_chunked(
    az_span_allocator_context* allocator_context,
    az_span* out_next_destination)
{
  _az_user_context* user_context = (_az_user_context*)allocator_context->user_context;
  int32_t current_index = *user_context->current_index;

  if (current_index > 0)
  {
    json_buffers[current_index - 1] = az_span_slice(
        AZ_SPAN_FROM_BUFFER(json_chunked_array_256[current_index - 1]),
        0,
        allocator_context->bytes_used);
  }
  if (current_index > 9)
  {
    current_index = 0;
  }
  assert_true(current_index <= 10);

  *out_next_destination = AZ_SPAN_FROM_BUFFER(json_chunked_array_256[current_index]);

  *user_context->current_index = current_index + 1;

  return AZ_OK;
}

static void test_json_writer_large_string_chunked(void** state)
{
  (void)state;
  {
    uint8_t expected[1300] = { 0 };
    for (int32_t i = 0; i < 1300; i++)
    {
      expected[i] = 'a';
    }
    expected[0] = '"';
    expected[1299] = '"';

    az_json_writer writer = { 0 };
    az_span_allocator_fn allocator = &test_allocator_chunked;
    int32_t previous = 0;
    _az_user_context user_context = { .current_index = &previous };

    TEST_EXPECT_SUCCESS(
        az_json_writer_chunked_init(&writer, AZ_SPAN_EMPTY, allocator, (void*)&user_context, NULL));

    TEST_EXPECT_SUCCESS(az_json_writer_append_string(
        &writer, az_span_slice(AZ_SPAN_FROM_BUFFER(expected), 1, 1299)));

    uint8_t array[1300] = { 0 };
    az_span entire_json = AZ_SPAN_FROM_BUFFER(array);
    for (int32_t i = 0; i < 10; i++)
    {
      az_span next_span = json_buffers[i];
      entire_json = az_span_copy(entire_json, next_span);
    }
    az_span leftover = az_json_writer_get_bytes_used_in_destination(&writer);
    assert_int_equal(az_span_size(leftover), 20);
    entire_json = az_span_copy(entire_json, leftover);

    assert_true(
        az_span_is_content_equal(AZ_SPAN_FROM_BUFFER(array), AZ_SPAN_FROM_BUFFER(expected)));
  }
}

/** Json reader **/
az_result read_write(az_span input, az_span* output, int32_t* o);
az_result read_write_token(
    az_span* output,
    int32_t* written,
    int32_t* o,
    az_json_reader* state,
    az_json_token token);
az_result write_str(az_span span, az_span s, az_span* out, int32_t* written);

static az_span const sample1 = AZ_SPAN_LITERAL_FROM_STR( //
    "{\n"
    "  \"parameters\": {\n"
    "    \"subscriptionId\": \"{subscription-id}\",\n"
    "      \"resourceGroupName\" : \"res4303\",\n"
    "      \"accountName\" : \"sto7280\",\n"
    "      \"containerName\" : \"container8723\",\n"
    "      \"api-version\" : \"2019-04-01\",\n"
    "      \"monitor\" : \"true\",\n"
    "      \"LegalHold\" : {\n"
    "      \"tags\": [\n"
    "        \"tag1\",\n"
    "          \"tag2\",\n"
    "          \"tag3\"\n"
    "      ]\n"
    "    }\n"
    "  },\n"
    "    \"responses\": {\n"
    "    \"200\": {\n"
    "      \"body\": {\n"
    "        \"hasLegalHold\": false,\n"
    "          \"tags\" : []\n"
    "      }\n"
    "    }\n"
    "  }\n"
    "}\n");

static bool _is_double_equal(double actual, double expected, double error)
{
  return fabs(actual - expected) < error;
}

static void test_json_reader(void** state)
{
  (void)state;
  {
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("    "), NULL));
    assert_true(az_json_reader_next_token(&reader) == AZ_ERROR_UNEXPECTED_END);
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NONE, AZ_SPAN_EMPTY);
  }
  {
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("  null  "), NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NULL, AZ_SPAN_FROM_STR("null"));
  }
  {
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("  nul"), NULL));
    assert_true(az_json_reader_next_token(&reader) == AZ_ERROR_UNEXPECTED_END);
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NONE, AZ_SPAN_EMPTY);
  }
  {
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("  false"), NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_FALSE, AZ_SPAN_FROM_STR("false"));
  }
  {
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("  falsx  "), NULL));
    assert_true(az_json_reader_next_token(&reader) == AZ_ERROR_UNEXPECTED_CHAR);
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NONE, AZ_SPAN_EMPTY);
  }
  {
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("true "), NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_TRUE, AZ_SPAN_FROM_STR("true"));
  }
  {
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("  truem"), NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_TRUE, AZ_SPAN_FROM_STR("true"));
  }
  {
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("  123a"), NULL));
    assert_true(az_json_reader_next_token(&reader) == AZ_ERROR_UNEXPECTED_CHAR);
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NONE, AZ_SPAN_EMPTY);
  }
  {
    az_span const s = AZ_SPAN_FROM_STR(" \"tr\\\"ue\\t\" ");
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, s, NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_STRING, AZ_SPAN_FROM_STR("tr\\\"ue\\t"));
    assert_true(az_span_ptr(reader.token.slice) == (az_span_ptr(s) + 2));
  }
  {
    az_span const s = AZ_SPAN_FROM_STR("\"\\uFf0F\"");
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, s, NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_STRING, AZ_SPAN_FROM_STR("\\uFf0F"));
    assert_true(az_span_ptr(reader.token.slice) == az_span_ptr(s) + 1);
  }
  {
    az_span const s = AZ_SPAN_FROM_STR("\"\\uFf0\"");
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, s, NULL));
    assert_int_equal(reader.current_depth, 0);
    assert_true(az_json_reader_next_token(&reader) == AZ_ERROR_UNEXPECTED_CHAR);
  }
  /* Testing reading number and converting to double */
  {
    // no exp number, no decimal, integer only
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR(" 23 "), NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NUMBER, AZ_SPAN_FROM_STR("23"));

    uint64_t actual_u64 = 0;
    TEST_EXPECT_SUCCESS(az_json_token_get_uint64(&reader.token, &actual_u64));
    assert_int_equal(actual_u64, 23);

    uint32_t actual_u32 = 0;
    TEST_EXPECT_SUCCESS(az_json_token_get_uint32(&reader.token, &actual_u32));
    assert_int_equal(actual_u32, 23);

    int64_t actual_i64 = 0;
    TEST_EXPECT_SUCCESS(az_json_token_get_int64(&reader.token, &actual_i64));
    assert_int_equal(actual_i64, 23);

    int32_t actual_i32 = 0;
    TEST_EXPECT_SUCCESS(az_json_token_get_int32(&reader.token, &actual_i32));
    assert_int_equal(actual_i32, 23);

    double actual_d = 0;
    TEST_EXPECT_SUCCESS(az_json_token_get_double(&reader.token, &actual_d));
    assert_true(_is_double_equal(actual_d, 23, 1e-2));
  }
  {
    // no exp number, no decimal, negative integer only
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR(" -23 "), NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NUMBER, AZ_SPAN_FROM_STR("-23"));

    int64_t actual_i64 = 0;
    TEST_EXPECT_SUCCESS(az_json_token_get_int64(&reader.token, &actual_i64));
    assert_int_equal(actual_i64, -23);

    int32_t actual_i32 = 0;
    TEST_EXPECT_SUCCESS(az_json_token_get_int32(&reader.token, &actual_i32));
    assert_int_equal(actual_i32, -23);

    double actual_d = 0;
    TEST_EXPECT_SUCCESS(az_json_token_get_double(&reader.token, &actual_d));
    assert_true(_is_double_equal(actual_d, -23, 1e-2));
  }
  {
    // negative number with decimals
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR(" -23.56"), NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NUMBER, AZ_SPAN_FROM_STR("-23.56"));

    double actual_d = 0;
    TEST_EXPECT_SUCCESS(az_json_token_get_double(&reader.token, &actual_d));
    assert_true(_is_double_equal(actual_d, -23.56, 1e-2));
  }
  {
    // negative + decimals + exp
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR(" -23.56e-3"), NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NUMBER, AZ_SPAN_FROM_STR("-23.56e-3"));

    double actual_d = 0;
    TEST_EXPECT_SUCCESS(az_json_token_get_double(&reader.token, &actual_d));
    assert_true(_is_double_equal(actual_d, -0.02356, 1e-5));
  }
  {
    // exp
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("1e50"), NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NUMBER, AZ_SPAN_FROM_STR("1e50"));

    double actual_d = 0;
    TEST_EXPECT_SUCCESS(az_json_token_get_double(&reader.token, &actual_d));
    assert_true(_is_double_equal(actual_d, 1e50, 1e-2));
  }
  {
    // big decimal + exp
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(
        az_json_reader_init(&reader, AZ_SPAN_FROM_STR("10000000000000000000000e17"), NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    TEST_JSON_TOKEN_HELPER(
        reader.token, AZ_JSON_TOKEN_NUMBER, AZ_SPAN_FROM_STR("10000000000000000000000e17"));

    double actual_d = 0;
    TEST_EXPECT_SUCCESS(az_json_token_get_double(&reader.token, &actual_d));
    assert_true(_is_double_equal(actual_d, 10000000000000000000000e17, 1e-2));
  }
  {
    // exp inf -> Any value above double MAX range which would be translated to positive inf, is not
    // supported
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("1e309"), NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NUMBER, AZ_SPAN_FROM_STR("1e309"));

    double actual_d = 0;
    assert_int_equal(az_json_token_get_double(&reader.token, &actual_d), AZ_ERROR_UNEXPECTED_CHAR);
  }
  {
    // exp negative inf -> Any value below double MIN range would be translated 0
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("1e-400"), NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NUMBER, AZ_SPAN_FROM_STR("1e-400"));

    double actual_d = 0;
    TEST_EXPECT_SUCCESS(az_json_token_get_double(&reader.token, &actual_d));
    assert_true(_is_double_equal(actual_d, 0.0, 1e-2));
  }
  {
    // negative exp
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("1e-18"), NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NUMBER, AZ_SPAN_FROM_STR("1e-18"));

    double actual_d = 0;
    TEST_EXPECT_SUCCESS(az_json_token_get_double(&reader.token, &actual_d));
    assert_true(_is_double_equal(actual_d, 0.000000000000000001, 1e-17));
  }
  /* end of Testing reading number and converting to double */
  {
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR(" [ true, 0.25 ]"), NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_BEGIN_ARRAY, AZ_SPAN_FROM_STR("["));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 1);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_TRUE, AZ_SPAN_FROM_STR("true"));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 1);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_NUMBER, AZ_SPAN_FROM_STR("0.25"));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_END_ARRAY, AZ_SPAN_FROM_STR("]"));
    assert_true(az_json_reader_next_token(&reader) == AZ_ERROR_JSON_READER_DONE);
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_END_ARRAY, AZ_SPAN_FROM_STR("]"));
  }
  {
    az_span const json = AZ_SPAN_FROM_STR("{\"a\":\"Hello world!\"}");
    az_json_reader reader = { 0 };
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, json, NULL));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_BEGIN_OBJECT, AZ_SPAN_FROM_STR("{"));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 1);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_PROPERTY_NAME, AZ_SPAN_FROM_STR("a"));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 1);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_STRING, AZ_SPAN_FROM_STR("Hello world!"));
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_END_OBJECT, AZ_SPAN_FROM_STR("}"));
    assert_true(az_json_reader_next_token(&reader) == AZ_ERROR_JSON_READER_DONE);
    assert_int_equal(reader.current_depth, 0);
    TEST_JSON_TOKEN_HELPER(reader.token, AZ_JSON_TOKEN_END_OBJECT, AZ_SPAN_FROM_STR("}"));
  }
  {
    uint8_t buffer[1000] = { 0 };
    az_span output = AZ_SPAN_FROM_BUFFER(buffer);
    {
      int32_t o = 0;
      assert_true(
          read_write(AZ_SPAN_FROM_STR("{ \"a\" : [ true, { \"b\": [{}]}, 15 ] }"), &output, &o)
          == AZ_OK);

      assert_true(
          az_span_is_content_equal(output, AZ_SPAN_FROM_STR("{\"a\":[true,{\"b\":[{}]},0]}")));
    }
    {
      int32_t o = 0;
      output = AZ_SPAN_FROM_BUFFER(buffer);
      az_span const json = AZ_SPAN_FROM_STR(
          // 0           1           2           3           4           5 6
          // 01234 56789 01234 56678 01234 56789 01234 56789 01234 56789 01234
          // 56789 0123
          "[[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ "
          "[[[[[ [[[[[");
      az_result const result = read_write(json, &output, &o);
      assert_true(result == AZ_ERROR_JSON_NESTING_OVERFLOW);
    }
    {
      int32_t o = 0;
      output = AZ_SPAN_FROM_BUFFER(buffer);
      az_span const json = AZ_SPAN_FROM_STR(
          // 0           1           2           3           4           5 6 01234
          // 56789 01234 56678 01234 56789 01234 56789 01234 56789 01234 56789 012
          "[[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ "
          "[[[[[ [[[[");
      az_result const result = read_write(json, &output, &o);
      assert_int_equal(result, AZ_ERROR_UNEXPECTED_END);
    }
    {
      int32_t o = 0;
      az_span const json = AZ_SPAN_FROM_STR(
          // 0           1           2           3           4           5 6 01234
          // 56789 01234 56678 01234 56789 01234 56789 01234 56789 01234 56789 012
          "[[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ [[[[[ "
          "[[[[[ [[{"
          "   \"\\t\\n\": \"\\u0abc\"   "
          "}]]]] ]]]]] ]]]]] ]]]]] ]]]]] ]]]]] ]]]]] ]]]]] ]]]]] ]]]]] ]]]]] "
          "]]]]] ]]]");
      output = AZ_SPAN_FROM_BUFFER(buffer);
      az_result const result = read_write(json, &output, &o);
      assert_true(result == AZ_OK);

      assert_true(az_span_is_content_equal(
          output,
          AZ_SPAN_FROM_STR( //
              "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[{"
              "\"\\t\\n\":\"\\u0abc\""
              "}]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]"
              "]")));
    }
    //
    {
      int32_t o = 0;
      output = AZ_SPAN_FROM_BUFFER(buffer);
      az_result const result = read_write(sample1, &output, &o);
      assert_true(result == AZ_OK);
    }
  }
}

// Aux funtions
az_result read_write_token(
    az_span* output,
    int32_t* written,
    int32_t* o,
    az_json_reader* state,
    az_json_token token)
{
  switch (token.kind)
  {
    case AZ_JSON_TOKEN_NULL:
    {
      _az_RETURN_IF_NOT_ENOUGH_SIZE(*output, 4);
      *output = az_span_copy(*output, AZ_SPAN_FROM_STR("null"));
      *written += 4;
      return AZ_OK;
    }
    case AZ_JSON_TOKEN_TRUE:
    {
      int32_t required_length = 4;
      _az_RETURN_IF_NOT_ENOUGH_SIZE(*output, required_length);
      *output = az_span_copy(*output, AZ_SPAN_FROM_STR("true"));
      *written += required_length;
      return AZ_OK;
    }
    case AZ_JSON_TOKEN_FALSE:
    {
      int32_t required_length = 5;
      _az_RETURN_IF_NOT_ENOUGH_SIZE(*output, required_length);
      *output = az_span_copy(*output, AZ_SPAN_FROM_STR("false"));
      *written += required_length;
      return AZ_OK;
    }
    case AZ_JSON_TOKEN_NUMBER:
    {
      _az_RETURN_IF_NOT_ENOUGH_SIZE(*output, 1);
      *output = az_span_copy_u8(*output, '0');
      *written += 1;
      return AZ_OK;
    }
    case AZ_JSON_TOKEN_STRING:
    {
      return write_str(*output, token.slice, output, written);
    }
    case AZ_JSON_TOKEN_BEGIN_OBJECT:
    {
      _az_RETURN_IF_NOT_ENOUGH_SIZE(*output, 1);
      *output = az_span_copy_u8(*output, '{');
      *written += 1;
      bool need_comma = false;
      while (true)
      {
        az_result const result = az_json_reader_next_token(state);
        _az_RETURN_IF_FAILED(result);
        if (state->token.kind != AZ_JSON_TOKEN_PROPERTY_NAME)
        {
          break;
        }
        if (need_comma)
        {
          _az_RETURN_IF_NOT_ENOUGH_SIZE(*output, 1);
          *output = az_span_copy_u8(*output, ',');
          *written += 1;
        }
        else
        {
          need_comma = true;
        }
        _az_RETURN_IF_FAILED(write_str(*output, state->token.slice, output, written));
        _az_RETURN_IF_NOT_ENOUGH_SIZE(*output, 1);
        *output = az_span_copy_u8(*output, ':');
        *written += 1;

        _az_RETURN_IF_FAILED(az_json_reader_next_token(state));
        _az_RETURN_IF_FAILED(read_write_token(output, written, o, state, state->token));
      }
      _az_RETURN_IF_NOT_ENOUGH_SIZE(*output, 1);
      *output = az_span_copy_u8(*output, '}');
      *written += 1;
      return AZ_OK;
    }
    case AZ_JSON_TOKEN_BEGIN_ARRAY:
    {
      _az_RETURN_IF_NOT_ENOUGH_SIZE(*output, 1);
      *output = az_span_copy_u8(*output, '[');
      *written += 1;
      bool need_comma = false;
      while (true)
      {
        az_result const result = az_json_reader_next_token(state);
        _az_RETURN_IF_FAILED(result);
        if (state->token.kind == AZ_JSON_TOKEN_END_ARRAY)
        {
          break;
        }
        if (need_comma)
        {
          _az_RETURN_IF_NOT_ENOUGH_SIZE(*output, 1);
          *output = az_span_copy_u8(*output, ',');
          *written += 1;
        }
        else
        {
          need_comma = true;
        }
        _az_RETURN_IF_FAILED(read_write_token(output, written, o, state, state->token));
      }
      _az_RETURN_IF_NOT_ENOUGH_SIZE(*output, 1);
      *output = az_span_copy_u8(*output, ']');
      *written += 1;
      return AZ_OK;
    }
    default:
      break;
  }
  return AZ_ERROR_JSON_INVALID_STATE;
}

az_result read_write(az_span input, az_span* output, int32_t* o)
{
  az_json_reader reader = { 0 };
  TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, input, NULL));
  _az_RETURN_IF_FAILED(az_json_reader_next_token(&reader));
  int32_t written = 0;
  az_span output_copy = *output;
  _az_RETURN_IF_FAILED(read_write_token(&output_copy, &written, o, &reader, reader.token));
  *output = az_span_slice(*output, 0, written);
  return AZ_OK;
}

az_result write_str(az_span span, az_span s, az_span* out, int32_t* written)
{
  *out = span;
  int32_t required_length = az_span_size(s) + 2;

  _az_RETURN_IF_NOT_ENOUGH_SIZE(*out, required_length);
  *out = az_span_copy_u8(*out, '"');
  *out = az_span_copy(*out, s);
  *out = az_span_copy_u8(*out, '"');

  *written += required_length;
  return AZ_OK;
}

// Using a macro instead of a helper function to retain line number
// in call stack to help debug which line/test case failed.
#define TEST_JSON_READER_INVALID_HELPER(json, expected_result)     \
  do                                                               \
  {                                                                \
    az_json_reader reader = { 0 };                                 \
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, json, NULL)); \
    az_result result = AZ_OK;                                      \
    while (result == AZ_OK)                                        \
    {                                                              \
      result = az_json_reader_next_token(&reader);                 \
    }                                                              \
    assert_int_equal(result, expected_result);                     \
  } while (0)

static void test_json_reader_invalid(void** state)
{
  (void)state;

  // Invalid nesting
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{]"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("[}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{[]}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{[,]}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("[[{,}]]"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{{}}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("[[{{}}]]"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(
      AZ_SPAN_FROM_STR("{\"age\":30,\"ints\":[1, 2, 3}}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(
      AZ_SPAN_FROM_STR("[[[[{\r\n\"a\":[[[[{\"b\":[}]]]]}]]]]"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(
      AZ_SPAN_FROM_STR("[[[[{\r\n\"a\":[[[[{\"b\":[]},[}]]]]}]]]]"), AZ_ERROR_UNEXPECTED_CHAR);

  // Invalid trailing commas
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR(","), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("   ,   "), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{},"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("[],"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("1,"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("true,"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("false,"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("null,"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{,}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"a\": 1,,}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(
      AZ_SPAN_FROM_STR("{\"a\": 1,,\"b\":2,}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("[,]"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("[1,2,]"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("[1,,]"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("[1,,2,]"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(
      AZ_SPAN_FROM_STR("{\"a\":1,\"b\":[],}"), AZ_ERROR_UNEXPECTED_CHAR);

  // Invalid literals and single tokens
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("nulz"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("truz"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("falsz"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("nul "), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("tru "), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("fals "), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("NULL"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("trUe"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("False"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("age"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"age\":"), AZ_ERROR_UNEXPECTED_CHAR);

  // Invalid numbers
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("12345.1."), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("-f"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("1.f"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("0.1f"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("0.1e1f"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("123f"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("0-"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("1-"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("1.1-"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("123,"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("+0"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("+1"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("01"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("-01"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("0.e"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("-0.e"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("0.1e+,"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("-0.1e- "), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("0.1e+}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("-0.1e-]"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("1, 2"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("1, \"age\":"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("001"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("00h"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("[01"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("10.5e-f"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("10.5e-0.2"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(
      AZ_SPAN_FROM_STR("{\"age\":30, \"ints\":[1, 2, 3, 4, 5.1e7.3]}"), AZ_ERROR_UNEXPECTED_CHAR);

  // Invalid strings
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"hel\rlo\""), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"hel\nlo\""), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"\\uABCX\""), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"\\uXABC\""), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"hel\\uABCXlo\""), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"hel\\lo\""), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"hel\\\\\\lo\""), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"hel\\\tlo\""), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"hello\\\\\"\""), AZ_ERROR_UNEXPECTED_CHAR);

  //  Invalid property names
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"hel\rlo\":1}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"hel\nlo\":1}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"\\uABCX\":1}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"\\uXABC\":1}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(
      AZ_SPAN_FROM_STR("{\"hel\\uABCXlo\":1}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"hel\\lo\":1}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(
      AZ_SPAN_FROM_STR("{\"hel\\\\\\lo\":1}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"hel\\\tlo\":1}"), AZ_ERROR_UNEXPECTED_CHAR);
  TEST_JSON_READER_INVALID_HELPER(
      AZ_SPAN_FROM_STR("{\"hello\\\\\"\":1}"), AZ_ERROR_UNEXPECTED_CHAR);

  TEST_JSON_READER_INVALID_HELPER(
      AZ_SPAN_FROM_STR("{\r\n\"isActive\":false \"\r\n}"), AZ_ERROR_UNEXPECTED_CHAR);
}

// Using a macro instead of a helper function to retain line number
// in call stack to help debug which line/test case failed.
#define TEST_JSON_READER_INVALID_HELPER(json, expected_result)     \
  do                                                               \
  {                                                                \
    az_json_reader reader = { 0 };                                 \
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, json, NULL)); \
    az_result result = AZ_OK;                                      \
    while (result == AZ_OK)                                        \
    {                                                              \
      result = az_json_reader_next_token(&reader);                 \
    }                                                              \
    assert_int_equal(result, expected_result);                     \
  } while (0)

static void test_json_reader_incomplete(void** state)
{
  (void)state;

  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("["), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("[1, 2,"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("[1, 2, "), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{  "), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("[ "), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("t"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("tru"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("n"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("nu"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("f"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("fals"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"name"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"name\\"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"name\\u"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"name\\u1"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"name\\u12"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"name\\u123"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("\"name\\u1234"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("-"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("-123."), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("-123.1e"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("-123e+"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("-123.1e+"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("0."), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"name"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{  \"name"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{  \"name\"  "), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"name\" :"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"name\":  "), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"name\": 123"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"name\": 123  "), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"name\": 123,"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"name\": 123,  "), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"name\": 123 ,  "), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"name\":\"value}"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"name\":\"value\""), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(
      AZ_SPAN_FROM_STR("{\"name\":\"value\","), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"name\":{}"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"name\":[]"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(AZ_SPAN_FROM_STR("{\"name\":[[]"), AZ_ERROR_UNEXPECTED_END);
  TEST_JSON_READER_INVALID_HELPER(
      AZ_SPAN_FROM_STR("{\"name\":[1, 2, [], 3] "), AZ_ERROR_UNEXPECTED_END);
}

static void test_json_skip_children(void** state)
{
  (void)state;

  az_json_reader reader = { 0 };

  TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("{\"foo\":1}"), NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader.token.kind, AZ_JSON_TOKEN_BEGIN_OBJECT);
  TEST_EXPECT_SUCCESS(az_json_reader_skip_children(&reader));
  assert_int_equal(reader.token.kind, AZ_JSON_TOKEN_END_OBJECT);
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 0);
  assert_int_equal(reader.current_depth, 0);

  TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("{\"foo\":{}}"), NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader.token.kind, AZ_JSON_TOKEN_BEGIN_OBJECT);
  TEST_EXPECT_SUCCESS(az_json_reader_skip_children(&reader));
  assert_int_equal(reader.token.kind, AZ_JSON_TOKEN_END_OBJECT);
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 0);
  assert_int_equal(reader.current_depth, 0);

  TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("{\"foo\":{}}"), NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader.token.kind, AZ_JSON_TOKEN_PROPERTY_NAME);
  TEST_EXPECT_SUCCESS(az_json_reader_skip_children(&reader));
  assert_int_equal(reader.token.kind, AZ_JSON_TOKEN_END_OBJECT);
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 1);
  assert_int_equal(reader.current_depth, 1);

  TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, AZ_SPAN_FROM_STR("{\"foo\":{}}"), NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  assert_int_equal(reader.token.kind, AZ_JSON_TOKEN_BEGIN_OBJECT);
  TEST_EXPECT_SUCCESS(az_json_reader_skip_children(&reader));
  assert_int_equal(reader.token.kind, AZ_JSON_TOKEN_END_OBJECT);
  assert_int_equal(reader._internal.bit_stack._internal.current_depth, 1);
  assert_int_equal(reader.current_depth, 1);
}

/** Json Value **/
static void test_json_value(void** state)
{
  (void)state;

  az_json_token const json_boolean = (az_json_token){
      .kind = AZ_JSON_TOKEN_TRUE,
      .slice = AZ_SPAN_FROM_STR("true"),
      ._internal = {
        .string_has_escaped_chars = false,
      },
    };
  az_json_token const json_number = (az_json_token){
      .kind = AZ_JSON_TOKEN_NUMBER,
      .slice = AZ_SPAN_FROM_STR("42"),
      ._internal = {
        .string_has_escaped_chars = false,
      },
    };
  az_json_token const json_negative_number = (az_json_token){
      .kind = AZ_JSON_TOKEN_NUMBER,
      .slice = AZ_SPAN_FROM_STR("-42"),
      ._internal = {
        .string_has_escaped_chars = false,
      },
    };
  az_json_token const json_decimal_number = (az_json_token){
      .kind = AZ_JSON_TOKEN_NUMBER,
      .slice = AZ_SPAN_FROM_STR("123.456"),
      ._internal = {
        .string_has_escaped_chars = false,
      },
    };
  az_json_token const json_string = (az_json_token){
      .kind = AZ_JSON_TOKEN_STRING,
      .slice = AZ_SPAN_FROM_STR("Hello"),
      ._internal = {
        .string_has_escaped_chars = false,
      },
    };
  az_json_token const json_property_name = (az_json_token){
      .kind = AZ_JSON_TOKEN_PROPERTY_NAME,
      .slice = AZ_SPAN_FROM_STR("Name"),
      ._internal = {
        .string_has_escaped_chars = false,
      },
    };

  // boolean from boolean
  {
    bool boolean_value = false;
    TEST_EXPECT_SUCCESS(az_json_token_get_boolean(&json_boolean, &boolean_value));
    assert_true(boolean_value);
  }
  // boolean from number
  {
    bool boolean_value = false;
    assert_true(
        az_json_token_get_boolean(&json_number, &boolean_value) == AZ_ERROR_JSON_INVALID_STATE);
  }
  // unsigned number from negative number
  {
    uint32_t number_value_u32 = 0;
    assert_int_equal(
        az_json_token_get_uint32(&json_negative_number, &number_value_u32),
        AZ_ERROR_UNEXPECTED_CHAR);
    uint64_t number_value_u64 = 0;
    assert_int_equal(
        az_json_token_get_uint64(&json_negative_number, &number_value_u64),
        AZ_ERROR_UNEXPECTED_CHAR);
  }
  // integer number from double number
  {
    uint32_t number_value_u32 = 0;
    assert_int_equal(
        az_json_token_get_uint32(&json_decimal_number, &number_value_u32),
        AZ_ERROR_UNEXPECTED_CHAR);
    uint64_t number_value_u64 = 0;
    assert_int_equal(
        az_json_token_get_uint64(&json_decimal_number, &number_value_u64),
        AZ_ERROR_UNEXPECTED_CHAR);
    int32_t number_value_i32 = 0;
    assert_int_equal(
        az_json_token_get_int32(&json_decimal_number, &number_value_i32), AZ_ERROR_UNEXPECTED_CHAR);
    int64_t number_value_i64 = 0;
    assert_int_equal(
        az_json_token_get_int64(&json_decimal_number, &number_value_i64), AZ_ERROR_UNEXPECTED_CHAR);
  }
  // string from string
  {
    char string_value[10] = { 0 };
    TEST_EXPECT_SUCCESS(az_json_token_get_string(&json_string, string_value, 10, NULL));
    assert_true(
        az_span_is_content_equal(az_span_create_from_str(string_value), AZ_SPAN_FROM_STR("Hello")));

    TEST_EXPECT_SUCCESS(az_json_token_get_string(&json_property_name, string_value, 10, NULL));
    assert_true(
        az_span_is_content_equal(az_span_create_from_str(string_value), AZ_SPAN_FROM_STR("Name")));
  }
  // string from boolean
  {
    char string_value[10] = { 0 };
    assert_true(
        az_json_token_get_string(&json_boolean, string_value, 10, NULL)
        == AZ_ERROR_JSON_INVALID_STATE);
  }
  // number from number
  {
    uint64_t number_value = 1;
    TEST_EXPECT_SUCCESS(az_json_token_get_uint64(&json_number, &number_value));

    uint64_t const expected_value_bin_rep_view = 42;
    assert_true(number_value == expected_value_bin_rep_view);
  }
  // double number from decimal and negative number
  {
    double number_value = 1;
    TEST_EXPECT_SUCCESS(az_json_token_get_double(&json_number, &number_value));
    assert_true(_is_double_equal(number_value, 42, 1e-2));
    TEST_EXPECT_SUCCESS(az_json_token_get_double(&json_negative_number, &number_value));
    assert_true(_is_double_equal(number_value, -42, 1e-2));
    TEST_EXPECT_SUCCESS(az_json_token_get_double(&json_decimal_number, &number_value));
    assert_true(_is_double_equal(number_value, 123.456, 1e-2));

    int int32_number = -1;
    TEST_EXPECT_SUCCESS(az_json_token_get_int32(&json_negative_number, &int32_number));
    assert_int_equal(int32_number, -42);
  }
  // number from string
  {
    uint64_t number_u64 = 1;
    assert_true(az_json_token_get_uint64(&json_string, &number_u64) == AZ_ERROR_JSON_INVALID_STATE);
    int64_t number_i64 = 1;
    assert_true(az_json_token_get_int64(&json_string, &number_i64) == AZ_ERROR_JSON_INVALID_STATE);
    uint32_t number_u32 = 1;
    assert_true(az_json_token_get_uint32(&json_string, &number_u32) == AZ_ERROR_JSON_INVALID_STATE);
    int32_t number_i32 = 1;
    assert_true(az_json_token_get_int32(&json_string, &number_i32) == AZ_ERROR_JSON_INVALID_STATE);
    double number_d = 1;
    assert_true(az_json_token_get_double(&json_string, &number_d) == AZ_ERROR_JSON_INVALID_STATE);
  }
  // az_json_token_is_text_equal
  {
    assert_true(az_json_token_is_text_equal(&json_boolean, AZ_SPAN_FROM_STR("true")) == false);
    assert_true(az_json_token_is_text_equal(&json_number, AZ_SPAN_FROM_STR("42")) == false);
    assert_true(
        az_json_token_is_text_equal(&json_negative_number, AZ_SPAN_FROM_STR("-42")) == false);
    assert_true(
        az_json_token_is_text_equal(&json_decimal_number, AZ_SPAN_FROM_STR("123.456")) == false);
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("hello")) == false);
    assert_true(
        az_json_token_is_text_equal(&json_property_name, AZ_SPAN_FROM_STR("name")) == false);

    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("Hello")));
    assert_true(az_json_token_is_text_equal(&json_property_name, AZ_SPAN_FROM_STR("Name")));
  }
}

#define _az_JSON_TOKEN_IS_TEXT_EQUAL_HELLO_HELPER(token)                                           \
  do                                                                                               \
  {                                                                                                \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_EMPTY) == false);                \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("")) == false);         \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("a")) == false);        \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("hello")) == false);    \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("HELLO")) == false);    \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("HEllo")) == false);    \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("HeLlo")) == false);    \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("HelLo")) == false);    \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("HellO")) == false);    \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("Hell")) == false);     \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("Helloo")) == false);   \
    assert_true(                                                                                   \
        az_json_token_is_text_equal(                                                               \
            &json_string, AZ_SPAN_FROM_STR("\\u0048\\u0065\\u006C\\u006C\\u006F"))                 \
        == false);                                                                                 \
    assert_true(                                                                                   \
        az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("abcdefghijklmnop")) == false); \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("Hello")));             \
  } while (0)

#define _az_JSON_TOKEN_IS_TEXT_EQUAL_NAME_HELPER(token)                                            \
  do                                                                                               \
  {                                                                                                \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_EMPTY) == false);                \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("")) == false);         \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("a")) == false);        \
    assert_true(                                                                                   \
        az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("My name is \\\\\"Ahson\\\"!")) \
        == false);                                                                                 \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("My name")) == false);  \
    assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("!")) == false);        \
    assert_true(                                                                                   \
        az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("My name  is")) == false);      \
    assert_true(                                                                                   \
        az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("My name is \"Ahson\"!"))       \
        == false);                                                                                 \
    assert_true(                                                                                   \
        az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("My name is \\\"Ahson\"! "))    \
        == false);                                                                                 \
    assert_true(                                                                                   \
        az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("My name is \\\"Ahson\"!")));   \
  } while (0)

static void test_az_json_token_get_string_and_text_equal(void** state)
{
  (void)state;

  char dest[128] = { 0 };
  int32_t str_length = 0;

  az_json_token json_string = (az_json_token){
      .kind = AZ_JSON_TOKEN_STRING,
      .slice = AZ_SPAN_FROM_STR(""),
      .size = 0,
      ._internal = {
        .string_has_escaped_chars = false,
      },
    };
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 0);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("a")) == false);
  assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("aaaaaa")) == false);
  assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("       ")) == false);
  assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_EMPTY));
  assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("")));

  json_string.kind = AZ_JSON_TOKEN_PROPERTY_NAME;
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 0);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("a")) == false);
  assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("aaaaaa")) == false);
  assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("       ")) == false);
  assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_EMPTY));
  assert_true(az_json_token_is_text_equal(&json_string, AZ_SPAN_FROM_STR("")));

  json_string = (az_json_token){
      .kind = AZ_JSON_TOKEN_STRING,
      .slice = AZ_SPAN_FROM_STR("Hello"),
      .size = 5,
      ._internal = {
        .string_has_escaped_chars = false,
      },
    };
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 5);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_HELLO_HELPER(json_string);

  json_string.kind = AZ_JSON_TOKEN_PROPERTY_NAME;
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 5);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_HELLO_HELPER(json_string);

  json_string = (az_json_token){
      .kind = AZ_JSON_TOKEN_STRING,
      .slice = AZ_SPAN_FROM_STR("My name is \\\\\\\"Ahson\\\"!"),
      .size = 23,
      ._internal = {
        .string_has_escaped_chars = true,
      },
    };
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 20);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_NAME_HELPER(json_string);

  json_string.kind = AZ_JSON_TOKEN_PROPERTY_NAME;
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 20);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_NAME_HELPER(json_string);
}

static void _az_split_buffers(az_span input, az_span* output)
{
  output[0] = az_span_slice(input, 0, az_span_size(input) / 2);
  output[1] = az_span_slice_to_end(input, az_span_size(input) / 2);
}

static void _az_split_buffers_single_byte(az_span input, az_span* output)
{
  for (int32_t i = 0; i < az_span_size(input); i++)
  {
    output[i] = az_span_slice(input, i, i + 1);
  }
}

static void test_az_json_token_get_string_and_text_equal_discontiguous(void** state)
{
  (void)state;

  char dest[128] = { 0 };
  int32_t str_length = 0;

  az_span json = AZ_SPAN_FROM_STR("\"Hello\"");

  az_json_reader reader = { 0 };
  TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, json, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  az_json_token json_string = reader.token;
  assert_int_equal(json_string.size, 5);
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 5);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_HELLO_HELPER(json_string);

  az_span buffers_half[2] = { 0 };
  _az_split_buffers(json, buffers_half);

  TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers_half, 2, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_string = reader.token;
  assert_int_equal(json_string.size, 5);
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 5);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_HELLO_HELPER(json_string);

  az_span buffers7_one[7] = { 0 };
  _az_split_buffers_single_byte(json, buffers7_one);

  TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers7_one, 7, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_string = reader.token;
  assert_int_equal(json_string.size, 5);
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 5);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_HELLO_HELPER(json_string);

  json = AZ_SPAN_FROM_STR("{\"Hello\":5}");

  TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, json, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_string = reader.token;
  assert_int_equal(json_string.size, 5);
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 5);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_HELLO_HELPER(json_string);

  _az_split_buffers(json, buffers_half);

  TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers_half, 2, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_string = reader.token;
  assert_int_equal(json_string.size, 5);
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 5);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_HELLO_HELPER(json_string);

  az_span buffers11_one[11] = { 0 };
  _az_split_buffers_single_byte(json, buffers11_one);

  TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers11_one, 11, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_string = reader.token;
  assert_int_equal(json_string.size, 5);
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 5);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_HELLO_HELPER(json_string);

  json = AZ_SPAN_FROM_STR("\"My name is \\\\\\\"Ahson\\\"!\"");

  TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, json, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_string = reader.token;
  assert_int_equal(json_string.size, 23);
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 20);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_NAME_HELPER(json_string);

  _az_split_buffers(json, buffers_half);

  TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers_half, 2, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_string = reader.token;
  assert_int_equal(json_string.size, 23);
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 20);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_NAME_HELPER(json_string);

  az_span buffers25_one[25] = { 0 };
  _az_split_buffers_single_byte(json, buffers25_one);

  TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers25_one, 25, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_string = reader.token;
  assert_int_equal(json_string.size, 23);
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 20);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_NAME_HELPER(json_string);

  json = AZ_SPAN_FROM_STR("{\"My name is \\\\\\\"Ahson\\\"!\":5}");

  TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, json, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_string = reader.token;
  assert_int_equal(json_string.size, 23);
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 20);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_NAME_HELPER(json_string);

  _az_split_buffers(json, buffers_half);

  TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers_half, 2, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_string = reader.token;
  assert_int_equal(json_string.size, 23);
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 20);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_NAME_HELPER(json_string);

  az_span buffers29_one[29] = { 0 };
  _az_split_buffers_single_byte(json, buffers29_one);

  TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers29_one, 29, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_string = reader.token;
  assert_int_equal(json_string.size, 23);
  assert_int_equal(az_json_token_get_string(&json_string, dest, 128, &str_length), AZ_OK);
  assert_int_equal(str_length, 20);
  assert_true(az_json_token_is_text_equal(&json_string, az_span_create_from_str(dest)));
  _az_JSON_TOKEN_IS_TEXT_EQUAL_NAME_HELPER(json_string);
}

static az_span _az_buffers64_one[64] = { 0 };
static uint8_t _az_buffer_for_complex_json[64] = { 0 };

#define _az_JSON_READER_DOUBLE_HELPER(json, expected)                                              \
  do                                                                                               \
  {                                                                                                \
                                                                                                   \
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, json, NULL));                                 \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
                                                                                                   \
    actual_d = 0;                                                                                  \
    json_number = reader.token;                                                                    \
    assert_int_equal(json_number.size, az_span_size(json));                                        \
    assert_int_equal(az_json_token_get_double(&json_number, &actual_d), AZ_OK);                    \
    assert_true(_is_double_equal(actual_d, expected, 1e-2));                                       \
                                                                                                   \
    _az_split_buffers(json, buffers_half);                                                         \
                                                                                                   \
    TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers_half, 2, NULL));              \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
                                                                                                   \
    actual_d = 0;                                                                                  \
    json_number = reader.token;                                                                    \
    assert_int_equal(json_number.size, az_span_size(json));                                        \
    assert_int_equal(az_json_token_get_double(&json_number, &actual_d), AZ_OK);                    \
    assert_true(_is_double_equal(actual_d, expected, 1e-2));                                       \
                                                                                                   \
    _az_split_buffers_single_byte(json, _az_buffers64_one);                                        \
                                                                                                   \
    TEST_EXPECT_SUCCESS(                                                                           \
        az_json_reader_chunked_init(&reader, _az_buffers64_one, az_span_size(json), NULL));        \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
                                                                                                   \
    actual_d = 0;                                                                                  \
    json_number = reader.token;                                                                    \
    assert_int_equal(json_number.size, az_span_size(json));                                        \
    assert_int_equal(az_json_token_get_double(&json_number, &actual_d), AZ_OK);                    \
    assert_true(_is_double_equal(actual_d, expected, 1e-2));                                       \
                                                                                                   \
    json_nested_array = AZ_SPAN_FROM_BUFFER(_az_buffer_for_complex_json);                          \
    remainder = az_span_copy(json_nested_array, AZ_SPAN_FROM_STR("["));                            \
    remainder = az_span_copy(remainder, json);                                                     \
    remainder = az_span_copy_u8(remainder, ']');                                                   \
                                                                                                   \
    json_nested_array                                                                              \
        = az_span_slice(json_nested_array, 0, _az_span_diff(remainder, json_nested_array));        \
                                                                                                   \
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, json_nested_array, NULL));                    \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
                                                                                                   \
    actual_d = 0;                                                                                  \
    json_number = reader.token;                                                                    \
    assert_int_equal(json_number.size, az_span_size(json));                                        \
    assert_int_equal(az_json_token_get_double(&json_number, &actual_d), AZ_OK);                    \
    assert_true(_is_double_equal(actual_d, expected, 1e-2));                                       \
                                                                                                   \
    _az_split_buffers(json_nested_array, buffers_half);                                            \
                                                                                                   \
    TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers_half, 2, NULL));              \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
                                                                                                   \
    actual_d = 0;                                                                                  \
    json_number = reader.token;                                                                    \
    assert_int_equal(json_number.size, az_span_size(json));                                        \
    assert_int_equal(az_json_token_get_double(&json_number, &actual_d), AZ_OK);                    \
    assert_true(_is_double_equal(actual_d, expected, 1e-2));                                       \
                                                                                                   \
    _az_split_buffers_single_byte(json_nested_array, _az_buffers64_one);                           \
                                                                                                   \
    TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(                                               \
        &reader, _az_buffers64_one, az_span_size(json_nested_array), NULL));                       \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
                                                                                                   \
    actual_d = 0;                                                                                  \
    json_number = reader.token;                                                                    \
    assert_int_equal(json_number.size, az_span_size(json));                                        \
    assert_int_equal(az_json_token_get_double(&json_number, &actual_d), AZ_OK);                    \
    assert_true(_is_double_equal(actual_d, expected, 1e-2));                                       \
                                                                                                   \
    json_object = AZ_SPAN_FROM_BUFFER(_az_buffer_for_complex_json);                                \
    remainder = az_span_copy(json_object, AZ_SPAN_FROM_STR("{\"name\":"));                         \
    remainder = az_span_copy(remainder, json);                                                     \
    remainder = az_span_copy_u8(remainder, '}');                                                   \
                                                                                                   \
    json_object = az_span_slice(json_object, 0, _az_span_diff(remainder, json_object));            \
                                                                                                   \
    TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, json_object, NULL));                          \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
                                                                                                   \
    actual_d = 0;                                                                                  \
    json_number = reader.token;                                                                    \
    assert_int_equal(json_number.size, az_span_size(json));                                        \
    assert_int_equal(az_json_token_get_double(&json_number, &actual_d), AZ_OK);                    \
    assert_true(_is_double_equal(actual_d, expected, 1e-2));                                       \
                                                                                                   \
    _az_split_buffers(json_object, buffers_half);                                                  \
                                                                                                   \
    TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers_half, 2, NULL));              \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
                                                                                                   \
    actual_d = 0;                                                                                  \
    json_number = reader.token;                                                                    \
    assert_int_equal(json_number.size, az_span_size(json));                                        \
    assert_int_equal(az_json_token_get_double(&json_number, &actual_d), AZ_OK);                    \
    assert_true(_is_double_equal(actual_d, expected, 1e-2));                                       \
                                                                                                   \
    _az_split_buffers_single_byte(json_object, _az_buffers64_one);                                 \
                                                                                                   \
    TEST_EXPECT_SUCCESS(                                                                           \
        az_json_reader_chunked_init(&reader, _az_buffers64_one, az_span_size(json_object), NULL)); \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
    TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));                                       \
                                                                                                   \
    actual_d = 0;                                                                                  \
    json_number = reader.token;                                                                    \
    assert_int_equal(json_number.size, az_span_size(json));                                        \
    assert_int_equal(az_json_token_get_double(&json_number, &actual_d), AZ_OK);                    \
    assert_true(_is_double_equal(actual_d, expected, 1e-2));                                       \
  } while (0)

static void test_az_json_reader_double(void** state)
{
  (void)state;

  az_json_reader reader = { 0 };
  double actual_d = 0;
  az_json_token json_number = reader.token;
  az_span json_nested_array = { 0 };
  az_span json_object = { 0 };
  az_span remainder = { 0 };
  az_span buffers_half[2] = { 0 };

  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("-0"), 0);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("2147483647"), 2147483647);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("-2147483648"), -2147483647 - 1);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("4294967295"), 4294967295);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("-4294967296"), -4294967296);
  _az_JSON_READER_DOUBLE_HELPER(
      AZ_SPAN_FROM_STR("9223372036854775807"), (double)9223372036854775807);
  _az_JSON_READER_DOUBLE_HELPER(
      AZ_SPAN_FROM_STR("-9223372036854775808"), -2147483647 * (double)4294967298);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("1.23e3"), 1.23e3);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("-123.456e-78"), -123.456e-78);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("123.456e+78"), 123.456e+78);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("0.0"), 0);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("0.0e-1"), 0);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("0.0e+1"), 0);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("-1"), -1);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("123.123"), 123.123);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("123.1230"), 123.1230);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("123.0100"), 123.0100);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("123.001"), 123.001);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("0.000000000000001"), 0.000000000000001);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("1.0000000001"), 1.0000000001);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("-1.0000000001"), -1.0000000001);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("100.001"), 100.001);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("100.00100"), 100.00100);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("0.001"), 0.001);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("0.0012"), 0.0012);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("1.2e4"), 1.2e4);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("1.2e-4"), 1.2e-4);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("1.2e+4"), 1.2e+4);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("-1.2e4"), -1.2e4);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("-1.2e-4"), -1.2e-4);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("9876.54321"), 9876.54321);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("-9876.54321"), -9876.54321);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("987654.0000321"), 987654.0000321);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("9007199254740991"), 9007199254740991);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("4503599627370496.2"), 4503599627370496.2);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("1e15"), 1e15);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("0.1234567890123456"), 0.1234567890123456);
  _az_JSON_READER_DOUBLE_HELPER(
      AZ_SPAN_FROM_STR("123456789012345.123456789012340000"), 123456789012345.123456789012340000);
  _az_JSON_READER_DOUBLE_HELPER(
      AZ_SPAN_FROM_STR("1000000000000.123456789012340000"), 1000000000000.123456789012340000);
  _az_JSON_READER_DOUBLE_HELPER(
      AZ_SPAN_FROM_STR("123456789012345.1234567890123400001"), 123456789012345.1234567890123400001);
  _az_JSON_READER_DOUBLE_HELPER(
      AZ_SPAN_FROM_STR("1000000000000.1234567890123400001"), 1000000000000.1234567890123400001);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("12345.12300000010e5"), 12345.12300000010e5);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("1e-300"), 1e-300);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("9007199254740993"), (double)9007199254740993);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("45035996273704961"), (double)45035996273704961);
  _az_JSON_READER_DOUBLE_HELPER(
      AZ_SPAN_FROM_STR("1844674407370955100"), (double)1844674407370955100);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("1.844674407370955e+19"), 1.844674407370955e+19);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("1.8446744073709551e+19"), 1.8446744073709551e+19);
  _az_JSON_READER_DOUBLE_HELPER(
      AZ_SPAN_FROM_STR("18446744073709551615"), (double)18446744073709551615UL);
  _az_JSON_READER_DOUBLE_HELPER(
      AZ_SPAN_FROM_STR("18446744073709551615.18446744073709551615"),
      18446744073709551615.18446744073709551615);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("1e16"), 1e16);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("-1e300"), -1e300);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("1.7e308"), 1.7e308);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("2.22507e-308"), 2.22507e-308);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("-2.22507e-308"), -2.22507e-308);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("4.94e-325"), 0);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("1e-400"), 0);
  _az_JSON_READER_DOUBLE_HELPER(AZ_SPAN_FROM_STR("-1e-400"), 0);
}

static void test_az_json_token_number_too_large(void** state)
{
  (void)state;

  az_json_reader reader = { 0 };
  az_span json = AZ_SPAN_FROM_STR("9999999999999999999999999999999999999999999999999999999999999999"
                                  "999999999999999999999999999999999999");
  TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, json, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  az_json_token json_number = reader.token;
  assert_int_equal(json_number.size, az_span_size(json));

  int32_t actual_i32 = 0;
  assert_int_equal(az_json_token_get_int32(&json_number, &actual_i32), AZ_ERROR_UNEXPECTED_CHAR);

  uint32_t actual_u32 = 0;
  assert_int_equal(az_json_token_get_uint32(&json_number, &actual_u32), AZ_ERROR_UNEXPECTED_CHAR);

  int64_t actual_i64 = 0;
  assert_int_equal(az_json_token_get_int64(&json_number, &actual_i64), AZ_ERROR_UNEXPECTED_CHAR);

  uint64_t actual_u64 = 0;
  assert_int_equal(az_json_token_get_uint64(&json_number, &actual_u64), AZ_ERROR_UNEXPECTED_CHAR);

  // double actual_d = 0;
  // assert_int_equal(az_json_token_get_double(&json_number, &actual_d), AZ_ERROR_UNEXPECTED_CHAR);

  az_span buffers_half[2] = { 0 };
  _az_split_buffers(json, buffers_half);

  TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers_half, 2, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_number = reader.token;
  assert_int_equal(json_number.size, az_span_size(json));

  assert_int_equal(az_json_token_get_int32(&json_number, &actual_i32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_json_token_get_uint32(&json_number, &actual_u32), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_json_token_get_int64(&json_number, &actual_i64), AZ_ERROR_UNEXPECTED_CHAR);
  assert_int_equal(az_json_token_get_uint64(&json_number, &actual_u64), AZ_ERROR_UNEXPECTED_CHAR);
  // assert_int_equal(az_json_token_get_double(&json_number, &actual_d), AZ_ERROR_UNEXPECTED_CHAR);
}

static void _az_json_token_literal_helper(az_span json, bool expected)
{
  az_json_reader reader = { 0 };
  az_json_token json_literal = { 0 };
  bool value = false;
  az_span json_nested_array = { 0 };
  az_span json_object = { 0 };
  az_span remainder = { 0 };

  TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, json, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_literal = reader.token;
  assert_int_equal(json_literal.size, az_span_size(json));

  if (json_literal.kind != AZ_JSON_TOKEN_NULL)
  {
    value = false;
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_OK);
    assert_true(value == expected);
  }
  else
  {
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_ERROR_JSON_INVALID_STATE);
  }

  az_span buffers_half[2] = { 0 };
  _az_split_buffers(json, buffers_half);

  TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers_half, 2, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_literal = reader.token;
  assert_int_equal(json_literal.size, az_span_size(json));

  if (json_literal.kind != AZ_JSON_TOKEN_NULL)
  {
    value = false;
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_OK);
    assert_true(value == expected);
  }
  else
  {
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_ERROR_JSON_INVALID_STATE);
  }

  _az_split_buffers_single_byte(json, _az_buffers64_one);

  TEST_EXPECT_SUCCESS(
      az_json_reader_chunked_init(&reader, _az_buffers64_one, az_span_size(json), NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_literal = reader.token;
  assert_int_equal(json_literal.size, az_span_size(json));

  if (json_literal.kind != AZ_JSON_TOKEN_NULL)
  {
    value = false;
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_OK);
    assert_true(value == expected);
  }
  else
  {
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_ERROR_JSON_INVALID_STATE);
  }

  json_nested_array = AZ_SPAN_FROM_BUFFER(_az_buffer_for_complex_json);
  remainder = az_span_copy(json_nested_array, AZ_SPAN_FROM_STR("["));
  remainder = az_span_copy(remainder, json);
  remainder = az_span_copy_u8(remainder, ']');

  json_nested_array
      = az_span_slice(json_nested_array, 0, _az_span_diff(remainder, json_nested_array));

  TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, json_nested_array, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_literal = reader.token;
  assert_int_equal(json_literal.size, az_span_size(json));

  if (json_literal.kind != AZ_JSON_TOKEN_NULL)
  {
    value = false;
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_OK);
    assert_true(value == expected);
  }
  else
  {
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_ERROR_JSON_INVALID_STATE);
  }

  _az_split_buffers(json_nested_array, buffers_half);

  TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers_half, 2, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_literal = reader.token;
  assert_int_equal(json_literal.size, az_span_size(json));

  if (json_literal.kind != AZ_JSON_TOKEN_NULL)
  {
    value = false;
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_OK);
    assert_true(value == expected);
  }
  else
  {
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_ERROR_JSON_INVALID_STATE);
  }

  _az_split_buffers_single_byte(json_nested_array, _az_buffers64_one);

  TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(
      &reader, _az_buffers64_one, az_span_size(json_nested_array), NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_literal = reader.token;
  assert_int_equal(json_literal.size, az_span_size(json));

  if (json_literal.kind != AZ_JSON_TOKEN_NULL)
  {
    value = false;
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_OK);
    assert_true(value == expected);
  }
  else
  {
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_ERROR_JSON_INVALID_STATE);
  }

  json_object = AZ_SPAN_FROM_BUFFER(_az_buffer_for_complex_json);
  remainder = az_span_copy(json_object, AZ_SPAN_FROM_STR("{\"name\":"));
  remainder = az_span_copy(remainder, json);
  remainder = az_span_copy_u8(remainder, '}');

  json_object = az_span_slice(json_object, 0, _az_span_diff(remainder, json_object));

  TEST_EXPECT_SUCCESS(az_json_reader_init(&reader, json_object, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_literal = reader.token;
  assert_int_equal(json_literal.size, az_span_size(json));

  if (json_literal.kind != AZ_JSON_TOKEN_NULL)
  {
    value = false;
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_OK);
    assert_true(value == expected);
  }
  else
  {
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_ERROR_JSON_INVALID_STATE);
  }

  _az_split_buffers(json_object, buffers_half);

  TEST_EXPECT_SUCCESS(az_json_reader_chunked_init(&reader, buffers_half, 2, NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_literal = reader.token;
  assert_int_equal(json_literal.size, az_span_size(json));

  if (json_literal.kind != AZ_JSON_TOKEN_NULL)
  {
    value = false;
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_OK);
    assert_true(value == expected);
  }
  else
  {
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_ERROR_JSON_INVALID_STATE);
  }

  _az_split_buffers_single_byte(json_object, _az_buffers64_one);

  TEST_EXPECT_SUCCESS(
      az_json_reader_chunked_init(&reader, _az_buffers64_one, az_span_size(json_object), NULL));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));
  TEST_EXPECT_SUCCESS(az_json_reader_next_token(&reader));

  json_literal = reader.token;
  assert_int_equal(json_literal.size, az_span_size(json));

  if (json_literal.kind != AZ_JSON_TOKEN_NULL)
  {
    value = false;
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_OK);
    assert_true(value == expected);
  }
  else
  {
    assert_int_equal(az_json_token_get_boolean(&json_literal, &value), AZ_ERROR_JSON_INVALID_STATE);
  }
}

static void test_az_json_token_literal(void** state)
{
  (void)state;

  _az_json_token_literal_helper(AZ_SPAN_FROM_STR("true"), true);
  _az_json_token_literal_helper(AZ_SPAN_FROM_STR("false"), false);
  _az_json_token_literal_helper(AZ_SPAN_FROM_STR("null"), false);
}

static void test_az_json_token_copy(void** state)
{
  (void)state;

  az_span json = AZ_SPAN_FROM_STR("abcdefghijkl");
  uint8_t dest_buffer[128] = { 0 };
  az_span destination = AZ_SPAN_FROM_BUFFER(dest_buffer);

  az_json_token json_string = (az_json_token){
    .slice = json,
    .size = 12,
  };

  az_span remainder = az_json_token_copy_into_span(&json_string, destination);
  assert_int_equal(az_span_size(remainder), 128 - 12);
  assert_true(az_span_is_content_equal(json, az_span_slice(destination, 0, 12)));

  // Split in 2
  {
    uint8_t split1[7] = { 0 };
    uint8_t split2[13] = { 0 };
    az_span buffers[2] = { AZ_SPAN_FROM_BUFFER(split1), AZ_SPAN_FROM_BUFFER(split2) };
    az_span_copy(buffers[0], az_span_slice(json, 0, 7));
    az_span_copy(buffers[1], az_span_slice_to_end(json, 7));

    json_string = (az_json_token){
      .slice = AZ_SPAN_EMPTY,
      .size = 12,
      ._internal = {
        .is_multisegment = true,
        .pointer_to_first_buffer = buffers,
        .start_buffer_index = 0,
        .start_buffer_offset = 0,
        .end_buffer_index = 1,
        .end_buffer_offset = 5,
      },
    };

    remainder = az_json_token_copy_into_span(&json_string, destination);
    assert_int_equal(az_span_size(remainder), 128 - 12);
    assert_true(az_span_is_content_equal(json, az_span_slice(destination, 0, 12)));
  }
  // Split in 3
  {

    uint8_t split1[7] = { 0 };
    uint8_t split2[3] = { 0 };
    uint8_t split3[5] = { 0 };
    az_span buffers[3]
        = { AZ_SPAN_FROM_BUFFER(split1), AZ_SPAN_FROM_BUFFER(split2), AZ_SPAN_FROM_BUFFER(split3) };
    az_span_copy(az_span_slice_to_end(buffers[0], 1), az_span_slice(json, 0, 6));
    az_span_copy(buffers[1], az_span_slice(json, 6, 9));
    az_span_copy(buffers[2], az_span_slice_to_end(json, 9));

    json_string = (az_json_token){
      .slice = AZ_SPAN_EMPTY,
      .size = 12,
      ._internal = {
        .is_multisegment = true,
        .pointer_to_first_buffer = buffers,
        .start_buffer_index = 0,
        .start_buffer_offset = 1,
        .end_buffer_index = 2,
        .end_buffer_offset = 3,
      },
    };

    remainder = az_json_token_copy_into_span(&json_string, destination);
    assert_int_equal(az_span_size(remainder), 128 - 12);
    assert_true(az_span_is_content_equal(json, az_span_slice(destination, 0, 12)));
  }
  // Split in n
  {
    _az_split_buffers_single_byte(json, _az_buffers64_one);

    json_string = (az_json_token){
      .kind = AZ_JSON_TOKEN_STRING,
      .slice = AZ_SPAN_EMPTY,
      .size = 12,
      ._internal = {
        .is_multisegment = true,
        .string_has_escaped_chars = false,
        .pointer_to_first_buffer = _az_buffers64_one,
        .start_buffer_index = 0,
        .start_buffer_offset = 0,
        .end_buffer_index = az_span_size(json) - 1,
        .end_buffer_offset = 1,
      },
    };

    remainder = az_json_token_copy_into_span(&json_string, destination);
    assert_int_equal(az_span_size(remainder), 128 - 12);
    assert_true(az_span_is_content_equal(json, az_span_slice(destination, 0, 12)));
  }
}

// Imagine your JSON input to parse is " { \"name\": \"some value string\" , \"code\" : 123456 } "
// Either in one contiguous buffer, or split up within multiple non-contiguous ones.
typedef struct
{
  char* name_string;
  int32_t name_length;
  az_span name_value_span; // optional, for demonstration purposes only
  int32_t code;
} model;

static uint8_t available_scratch[64] = { 0 };

static az_result _az_process_json(az_span* input, int32_t number_of_buffers, model* output)
{
  az_span scratch_span = AZ_SPAN_FROM_BUFFER(available_scratch);

  az_json_reader jr = { 0 };
  _az_RETURN_IF_FAILED(az_json_reader_chunked_init(&jr, input, number_of_buffers, NULL));

  _az_RETURN_IF_FAILED(az_json_reader_next_token(&jr));
  if (jr.token.kind != AZ_JSON_TOKEN_BEGIN_OBJECT)
  {
    return AZ_ERROR_UNEXPECTED_CHAR;
  }

  while (az_result_succeeded(az_json_reader_next_token(&jr))
         && jr.token.kind != AZ_JSON_TOKEN_END_OBJECT)
  {
    if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR("name")))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(&jr));
      if (jr.token.kind != AZ_JSON_TOKEN_STRING)
      {
        return AZ_ERROR_ITEM_NOT_FOUND;
      }

      az_json_token_copy_into_span(&jr.token, az_span_slice(scratch_span, 0, jr.token.size));
      output->name_value_span = az_span_slice(scratch_span, 0, jr.token.size);

      _az_RETURN_IF_FAILED(az_json_token_get_string(
          &jr.token, output->name_string, output->name_length, &output->name_length));
    }
    else if (az_json_token_is_text_equal(&jr.token, AZ_SPAN_FROM_STR("code")))
    {
      _az_RETURN_IF_FAILED(az_json_reader_next_token(&jr));
      if (jr.token.kind != AZ_JSON_TOKEN_NUMBER)
      {
        return AZ_ERROR_ITEM_NOT_FOUND;
      }
      _az_RETURN_IF_FAILED(az_json_token_get_int32(&jr.token, &output->code));
    }
    else
    {
      // ignore other tokens
      _az_RETURN_IF_FAILED(az_json_reader_skip_children(&jr));
    }
  }

  return AZ_OK;
}

static void test_az_json_reader_chunked(void** state)
{
  (void)state;

  char property_value[32] = { 0 };
  az_span expected = AZ_SPAN_FROM_STR("some value string");

  model original = (model){
    .name_string = property_value,
    .name_length = 32,
    .name_value_span = AZ_SPAN_EMPTY,
    .code = 0,
  };

  model m = original;
  az_span json = AZ_SPAN_FROM_STR(" { \"name\": \"some value string\" , \"code\" : 123456 } ");

  _az_process_json(&json, 1, &m);

  assert_int_equal(m.code, 123456);
  assert_int_equal(m.name_length, 17);
  assert_true(az_span_is_content_equal(expected, m.name_value_span));
  assert_true(az_span_is_content_equal(expected, az_span_create_from_str(m.name_string)));

  m = original;
  property_value[0] = 0;
  az_span buffers_half[2] = { 0 };
  _az_split_buffers(json, buffers_half);

  _az_process_json(buffers_half, 2, &m);

  assert_int_equal(m.code, 123456);
  assert_int_equal(m.name_length, 17);
  assert_true(az_span_is_content_equal(expected, m.name_value_span));
  assert_true(az_span_is_content_equal(expected, az_span_create_from_str(m.name_string)));

  m = original;
  property_value[0] = 0;
  _az_split_buffers_single_byte(json, _az_buffers64_one);

  _az_process_json(_az_buffers64_one, az_span_size(json), &m);

  assert_int_equal(m.code, 123456);
  assert_int_equal(m.name_length, 17);
  assert_true(az_span_is_content_equal(expected, m.name_value_span));
  assert_true(az_span_is_content_equal(expected, az_span_create_from_str(m.name_string)));
}

static void _az_span_free(az_span* p)
{
  if (p == NULL)
  {
    return;
  }
  free(az_span_ptr(*p));
  *p = AZ_SPAN_EMPTY;
}

static void test_az_json_string_unescape(void** state)
{
  (void)state;

  // no escapes
  {
    az_span json = AZ_SPAN_FROM_STR(" { \"name\": \"some value string\" , \"code\" : 123456 } ");
    az_span expected
        = AZ_SPAN_FROM_STR(" { \"name\": \"some value string\" , \"code\" : 123456 } ");

    char destination[59] = { 0 };
    az_span destination_span = AZ_SPAN_FROM_BUFFER(destination);

    destination_span = az_json_string_unescape(json, destination_span);

    assert_true(az_span_is_content_equal(expected, destination_span));
  }

  // nothing is written past the end (including null terminator)
  {
    uint8_t buffer[3];
    az_span test_span = AZ_SPAN_FROM_BUFFER(buffer);
    test_span._internal.ptr[0] = 'a';
    test_span._internal.ptr[1] = 'b';
    test_span._internal.ptr[2] = 'c';

    az_span expected = AZ_SPAN_FROM_STR("ab");

    char destination[59] = { 0 };
    destination[2] = 'd'; // verify that 'd' is not overwritten with 0
    az_span destination_span = AZ_SPAN_FROM_BUFFER(destination);

    destination_span = az_json_string_unescape(az_span_slice(test_span, 0, 2), destination_span);

    assert_int_equal((int)'d', (int)destination[2]);
    assert_true(az_span_is_content_equal(expected, destination_span));
  }

  // only escapes
  {
    az_span original = AZ_SPAN_FROM_STR("\\b\\f\\n\\r\\t\\\\");
    az_span expected = AZ_SPAN_FROM_STR("\b\f\n\r\t\\");

    char destination[59] = { 0 };
    az_span destination_span = AZ_SPAN_FROM_BUFFER(destination);

    destination_span = az_json_string_unescape(original, destination_span);

    assert_true(az_span_is_content_equal(expected, destination_span));
  }

  // mix and match
  {
    az_span original
        = AZ_SPAN_FROM_STR("Hello \\b My \\f Name \\n Is \\r Doctor \\t Green \\\\ Thumb");
    az_span expected = AZ_SPAN_FROM_STR("Hello \b My \f Name \n Is \r Doctor \t Green \\ Thumb");

    char destination[59] = { 0 };
    az_span destination_span = AZ_SPAN_FROM_BUFFER(destination);

    destination_span = az_json_string_unescape(original, destination_span);

    assert_true(az_span_is_content_equal(expected, destination_span));
  }

  // single char
  {
    az_span original = AZ_SPAN_FROM_STR("a");
    az_span expected = AZ_SPAN_FROM_STR("a");

    char destination[59] = { 0 };
    az_span destination_span = AZ_SPAN_FROM_BUFFER(destination);

    destination_span = az_json_string_unescape(original, destination_span);

    assert_true(az_span_is_content_equal(expected, destination_span));
  }

  // magic test
  {
    az_span original = AZ_SPAN_FROM_STR("A\\\"\"Z");
    az_span expected = AZ_SPAN_FROM_STR("A\"\"Z");
    char destination[59] = { 0 };
    az_span destination_span = AZ_SPAN_FROM_BUFFER(destination);

    destination_span = az_json_string_unescape(original, destination_span);

    assert_true(az_span_is_content_equal(expected, destination_span));
  }

  // magic test2
  {
    az_span original = AZ_SPAN_FROM_STR("A\\\\\\\\\\\"\\\"Z");
    az_span expected = AZ_SPAN_FROM_STR("A\\\\\"\"Z");
    char destination[59] = { 0 };
    az_span destination_span = AZ_SPAN_FROM_BUFFER(destination);

    destination_span = az_json_string_unescape(original, destination_span);

    assert_true(az_span_is_content_equal(expected, destination_span));
  }

  // magic test 2
  {
    az_span original = AZ_SPAN_FROM_STR("\\\"\\\"");
    az_span expected = AZ_SPAN_FROM_STR("\"\"");

    char destination[59] = { 0 };
    az_span destination_span = AZ_SPAN_FROM_BUFFER(destination);

    destination_span = az_json_string_unescape(original, destination_span);

    assert_true(az_span_is_content_equal(expected, destination_span));
  }

  // magic test 3
  {
    az_span original = AZ_SPAN_FROM_STR("\" \" \\\" \\\"");
    az_span expected = AZ_SPAN_FROM_STR("\" \" \" \"");

    char destination[59] = { 0 };
    az_span destination_span = AZ_SPAN_FROM_BUFFER(destination);

    destination_span = az_json_string_unescape(original, destination_span);

    assert_true(az_span_is_content_equal(expected, destination_span));
  }

  // magic test 4
  {
    az_span original = AZ_SPAN_FROM_STR("My name is \\\\\\\"Ahson\\\"!");
    az_span expected = AZ_SPAN_FROM_STR("My name is \\\"Ahson\"!");

    char destination[59] = { 0 };
    az_span destination_span = AZ_SPAN_FROM_BUFFER(destination);

    destination_span = az_json_string_unescape(original, destination_span);

    assert_true(az_span_is_content_equal(expected, destination_span));
  }

  // magic test 5
  {
    az_span original = AZ_SPAN_FROM_STR("My name is \\\\\\\"Ahson\\\"!");
    az_span expected = AZ_SPAN_FROM_STR("My name is \\\"Ahson\"!");

    char destination[59] = { 0 };
    az_span destination_span = AZ_SPAN_FROM_BUFFER(destination);
    destination_span = az_json_string_unescape(original, destination_span);

    assert_string_equal((char*)az_span_ptr(expected), destination);
  }

  // glorious test
  {
    az_span original = AZ_SPAN_FROM_STR("\\/\\/\\\"");
    az_span expected = AZ_SPAN_FROM_STR("//\"");

    char destination[59] = { 0 };
    az_span destination_span = AZ_SPAN_FROM_BUFFER(destination);
    destination_span = az_json_string_unescape(original, destination_span);

    assert_string_equal((char*)az_span_ptr(expected), destination);
  }
}

static void test_az_json_string_unescape_same_buffer(void** state)
{
  (void)state;

  // no escapes
  {
    az_span json = az_span_create_from_str(
        strdup(" { \"name\": \"some value string\" , \"code\" : 123456 } "));

    az_span expected
        = AZ_SPAN_FROM_STR(" { \"name\": \"some value string\" , \"code\" : 123456 } ");

    json = az_json_string_unescape(json, json);

    assert_true(az_span_is_content_equal(expected, json));
    _az_span_free(&json);
  }

  // nothing is written past the end (including null terminator)
  {
    uint8_t buffer[3];
    az_span test_span = AZ_SPAN_FROM_BUFFER(buffer);
    test_span._internal.ptr[0] = 'a';
    test_span._internal.ptr[1] = 'b';
    test_span._internal.ptr[2] = 'c'; // verify that 'c' is not overwritten with 0

    az_span expected = AZ_SPAN_FROM_STR("ab");

    test_span = az_json_string_unescape(az_span_slice(test_span, 0, 2), test_span);

    assert_int_equal((int)'c', buffer[2]);
    assert_true(az_span_is_content_equal(expected, az_span_slice(test_span, 0, 2)));
  }

  // only escapes
  {
    az_span original = az_span_create_from_str(strdup("\\b\\f\\n\\r\\t\\\\"));
    az_span expected = AZ_SPAN_FROM_STR("\b\f\n\r\t\\");

    original = az_json_string_unescape(original, original);

    assert_true(az_span_is_content_equal(expected, original));
    _az_span_free(&original);
  }

  // some other escapes
  {
    az_span original = az_span_create_from_str(strdup("\\/\\/\\\""));
    az_span expected = AZ_SPAN_FROM_STR("//\"");

    original = az_json_string_unescape(original, original);

    assert_true(az_span_is_content_equal(expected, original));
    _az_span_free(&original);
  }

  // mix and match
  {
    az_span original = az_span_create_from_str(
        strdup("Hello \\b My \\f Name \\n Is \\r Doctor \\t Green \\\\ Thumb"));
    az_span expected = AZ_SPAN_FROM_STR("Hello \b My \f Name \n Is \r Doctor \t Green \\ Thumb");

    original = az_json_string_unescape(original, original);

    assert_true(az_span_is_content_equal(expected, original));
    _az_span_free(&original);
  }
}

int test_az_json()
{
  const struct CMUnitTest tests[]
      = { cmocka_unit_test(test_json_reader_init),
          cmocka_unit_test(test_json_reader_current_depth_array),
          cmocka_unit_test(test_json_reader_current_depth_object),
          cmocka_unit_test(test_json_writer),
          cmocka_unit_test(test_json_writer_append_nested),
          cmocka_unit_test(test_json_writer_append_nested_invalid),
          cmocka_unit_test(test_json_writer_chunked),
          cmocka_unit_test(test_json_writer_chunked_no_callback),
          cmocka_unit_test(test_json_writer_large_string_chunked),
          cmocka_unit_test(test_json_reader),
          cmocka_unit_test(test_json_reader_invalid),
          cmocka_unit_test(test_json_reader_incomplete),
          cmocka_unit_test(test_json_skip_children),
          cmocka_unit_test(test_json_value),
          cmocka_unit_test(test_az_json_token_get_string_and_text_equal),
          cmocka_unit_test(test_az_json_token_get_string_and_text_equal_discontiguous),
          cmocka_unit_test(test_az_json_reader_double),
          cmocka_unit_test(test_az_json_token_number_too_large),
          cmocka_unit_test(test_az_json_token_literal),
          cmocka_unit_test(test_az_json_token_copy),
          cmocka_unit_test(test_az_json_reader_chunked),
          cmocka_unit_test(test_az_json_string_unescape),
          cmocka_unit_test(test_az_json_string_unescape_same_buffer) };
  return cmocka_run_group_tests_name("az_core_json", tests, NULL, NULL);
}
