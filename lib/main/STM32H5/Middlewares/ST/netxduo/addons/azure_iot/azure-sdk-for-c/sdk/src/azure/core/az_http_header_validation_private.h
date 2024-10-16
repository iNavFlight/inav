// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file az_http_header_validation_private.h
 *
 * @brief This header defines a bit array that is used to validate whenever or not an ASCII char is
 * valid within an http header name.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 * prefixed with an underscore ('_') directly in your application code. These symbols
 * are part of Azure SDK's internal implementation; we do not document these symbols
 * and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_HTTP_HEADER_VALIDATION_PRIVATE_H
#define _az_HTTP_HEADER_VALIDATION_PRIVATE_H

#include <azure/core/az_span.h>

#include <stdbool.h>
#include <stdint.h>

#include <azure/core/_az_cfg_prefix.h>

// Bit array from ASCII to valid. Every zero in array means invalid character, while non-zeros
// return the valid character.
static const uint8_t az_http_valid_token[256] = {
  0, // 0 ->  null
  0, // 1 ->  start of heading
  0, // 2 ->  start of text
  0, // 3 ->  end of text
  0, // 4 ->  end of transmission
  0, // 5 ->  enquiry
  0, // 6 ->  acknowledge
  0, // 7 ->  bell
  0, // 8 ->  backspace
  0, // 9 ->  horizontal tab
  0, // 10 ->  new line
  0, // 11 ->  vertical tab
  0, // 12 ->  new page
  0, // 13 ->  carriage return
  0, // 14 ->  shift out
  0, // 15 ->  shift in
  0, // 16 ->  data link escape
  0, // 17 ->  device control 1
  0, // 18 ->  device control 2
  0, // 19 ->  device control 3
  0, // 20 ->  device control 4
  0, // 21 ->  negative acknowledge
  0, // 22 ->  synchronous idle
  0, // 23 ->  end of trans. block
  0, // 24 ->  cancel
  0, // 25 ->  end of medium
  0, // 26 ->  substitute
  0, // 27 ->  escape
  0, // 28 ->  file separator
  0, // 29 ->  group separator
  0, // 30 ->  record separator
  0, // 31 ->  unit separator
  ' ', // 32 ->  space
  '!', // 33 ->  !
  0, // 34 ->  "
  '#', // 35 ->  #
  '$', // 36 ->  $
  '%', // 37 ->  %
  '&', // 38 ->  &
  '\'', // 39 -> '
  0, // 40 ->  (
  0, // 41 ->  )
  '*', // 42 ->  *
  '+', // 43 ->  +
  0, // 44 ->  ,
  '-', // 45 ->  -
  '.', // 46 ->  .
  0, // 47 ->  /
  '0', // 48 ->  0
  '1', // 49 ->  1
  '2', // 50 ->  2
  '3', // 51 ->  3
  '4', // 52 ->  4
  '5', // 53 ->  5
  '6', // 54 ->  6
  '7', // 55 ->  7
  '8', // 56 ->  8
  '9', // 57 ->  9
  0, // 58 ->  :
  0, // 59 ->  ;
  0, // 60 ->  <
  0, // 61 ->  =
  0, // 62 ->  >
  0, // 63 ->  ?
  0, // 64 ->  @
  'a', // 65 ->  A
  'b', // 66 ->  B
  'c', // 67 ->  C
  'd', // 68 ->  D
  'e', // 69 ->  E
  'f', // 70 ->  F
  'g', // 71 ->  G
  'h', // 72 ->  H
  'i', // 73 ->  I
  'j', // 74 ->  J
  'k', // 75 ->  K
  'l', // 76 ->  L
  'm', // 77 ->  M
  'n', // 78 ->  N
  'o', // 79 ->  O
  'p', // 80 ->  P
  'q', // 81 ->  Q
  'r', // 82 ->  R
  's', // 83 ->  S
  't', // 84 ->  T
  'u', // 85 ->  U
  'v', // 86 ->  V
  'w', // 87 ->  W
  'x', // 88 ->  X
  'y', // 89 ->  Y
  'z', // 90 ->  Z
  0, // 91 ->  [
  0, // 92 ->  comment
  0, // 93 ->  ]
  '^', // 94 ->  ^
  '_', // 95 ->  _
  '`', // 96 ->  `
  'a', // 97 ->  a
  'b', // 98 ->  b
  'c', // 99 ->  c
  'd', // 100 ->  d
  'e', // 101 ->  e
  'f', // 102 ->  f
  'g', // 103 ->  g
  'h', // 104 ->  h
  'i', // 105 ->  i
  'j', // 106 ->  j
  'k', // 107 ->  k
  'l', // 108 ->  l
  'm', // 109 ->  m
  'n', // 110 ->  n
  'o', // 111 ->  o
  'p', // 112 ->  p
  'q', // 113 ->  q
  'r', // 114 ->  r
  's', // 115 ->  s
  't', // 116 ->  t
  'u', // 117 ->  u
  'v', // 118 ->  v
  'w', // 119 ->  w
  'x', // 120 ->  x
  'y', // 121 ->  y
  'z', // 122 ->  z
  0, // 123 ->  {
  '|', // 124 ->  |
  0, // 125 ->  }
  '~', // 126 ->  ~
  0 // 127 ->  DEL
  // ...128-255 is all zeros (not valid) characters
};

#ifndef AZ_NO_PRECONDITION_CHECKING

/* This function is for httpRequest only to check header names are valid.
 * Validation is a Precondition so this code would compile away if preconditions are OFF
 */
AZ_NODISCARD AZ_INLINE bool az_http_is_valid_header_name(az_span name)
{
  uint8_t* name_ptr = az_span_ptr(name);
  for (int32_t i = 0; i < az_span_size(name); i++)
  {
    uint8_t c = name_ptr[i];
    if (az_http_valid_token[c] == 0)
    {
      return false;
    }
  }
  return true;
}

#endif // AZ_NO_PRECONDITION_CHECKING

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_HTTP_HEADER_VALIDATION_PRIVATE_H
