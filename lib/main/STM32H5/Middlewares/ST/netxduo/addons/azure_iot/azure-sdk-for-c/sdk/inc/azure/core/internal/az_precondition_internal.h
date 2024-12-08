// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

/**
 * @file az_precondition_internal.h
 *
 * @brief This header defines the types and functions your application uses
 *        to override the default precondition failure behavior.
 *
 *        Public SDK functions validate the arguments passed to them in an effort
 *        to ensure that calling code is passing valid values. The valid value is
 *        called a contract precondition. If an SDK function detects a precondition
 *        failure (invalid argument value), then by default, it calls a function that
 *        places the calling thread into an infinite sleep state; other threads
 *        continue to run.
 *
 *        To override the default behavior, implement a function matching the
 *        az_precondition_failed_fn function signature and then, in your application's
 *        initialization (before calling any Azure SDK function), call
 *        az_precondition_failed_set_callback passing it the address of your function.
 *        Now, when any Azure SDK function detects a precondition failure, it will invoke
 *        your callback instead. You might override the callback to attach a debugger or
 *        perhaps to reboot the device rather than allowing it to continue running with
 *        unpredictable behavior.
 *
 *        Also, if you define the AZ_NO_PRECONDITION_CHECKING symbol when compiling the SDK
 *        code (or adding option -DPRECONDITIONS=OFF with cmake), all of the Azure SDK
 *        precondition checking will be excluding making the binary code smaller and faster. We
 *        recommend doing this before you ship your code.
 *
 * @note You MUST NOT use any symbols (macros, functions, structures, enums, etc.)
 *       prefixed with an underscore ('_') directly in your application code. These symbols
 *       are part of Azure SDK's internal implementation; we do not document these symbols
 *       and they are subject to change in future versions of the SDK which would break your code.
 */

#ifndef _az_PRECONDITION_INTERNAL_H
#define _az_PRECONDITION_INTERNAL_H

#include <azure/core/az_precondition.h>

#include <azure/core/az_span.h>

#include <stdbool.h>
#include <stddef.h>

#include <azure/core/_az_cfg_prefix.h>

az_precondition_failed_fn az_precondition_failed_get_callback();

// __analysis_assume() tells MSVC's code analysis tool about the assumptions we have, so it doesn't
// emit warnings for the statements that we put into _az_PRECONDITION().
// Code analysis starts to fail with this condition some time around version 19.27; but
// __analysis_assume() does appear at least as early as version 1920 (The first "Visual Studio
// 2019"). So, we do __analysis_assume() for MSVCs starting with 2019. We don't want to go much
// earlier without verifying, because if __analysis_assume() is not available on earlier compiler
// version, there will be a compilation error.
// For more info, see
// https://docs.microsoft.com/windows-hardware/drivers/devtest/using-the--analysis-assume-function-to-suppress-false-defects
#if _MSC_VER >= 1920
#define _az_ANALYSIS_ASSUME(statement) __analysis_assume(statement)
#else
#define _az_ANALYSIS_ASSUME(statement)
#endif

#ifdef AZ_NO_PRECONDITION_CHECKING
#define _az_PRECONDITION(condition)
#else
#define _az_PRECONDITION(condition)            \
  do                                           \
  {                                            \
    if (!(condition))                          \
    {                                          \
      az_precondition_failed_get_callback()(); \
    }                                          \
    _az_ANALYSIS_ASSUME(condition);            \
  } while (0)
#endif // AZ_NO_PRECONDITION_CHECKING

#define _az_PRECONDITION_RANGE(low, arg, max) _az_PRECONDITION((low) <= (arg) && (arg) <= (max))

#define _az_PRECONDITION_NOT_NULL(arg) _az_PRECONDITION((arg) != NULL)
#define _az_PRECONDITION_IS_NULL(arg) _az_PRECONDITION((arg) == NULL)

AZ_NODISCARD AZ_INLINE bool _az_span_is_valid(az_span span, int32_t min_size, bool null_is_valid)
{
  if (min_size < 0)
  {
    return false;
  }

  uint8_t* const ptr = az_span_ptr(span);
  int32_t const span_size = az_span_size(span);

  bool result = false;

  /* Span is valid if:
     The size is greater than or equal to a user defined minimum value AND one of the
     following:
        - If null_is_valid is true and the pointer in the span is null, the size must also be 0.
        - In the case of the pointer not being NULL, the size is greater than or equal to zero.
  */

  // On some platforms, in some compilation configurations (Debug), NULL is not 0x0...0. But if you
  // initialize a span with { 0 } (or if that span is a part of a structure that is initialized with
  // { 0 }) the ptr is not going to be equal to NULL, however the intent of the precondition is to
  // disallow default-initialized and null ptrs, so we should treat them the same.
  uint8_t* const default_init_ptr = az_span_ptr((az_span){ 0 });
  if (null_is_valid)
  {
    result = (ptr == NULL || ptr == default_init_ptr) ? span_size == 0 : span_size >= 0;
  }
  else
  {
    result = (ptr != NULL && ptr != default_init_ptr) && span_size >= 0;
  }

  // Can't wrap over the end of the address space.
  // The biggest theoretical pointer value is "(void*)~0" (0xFFFF...), which is the end of address
  // space. We don't attempt to read/write beyond the end of the address space - it is unlikely a
  // desired behavior, and it is not defined. So, if the span size is greater than the addresses
  // left until the theoretical end of the address space, it is not a valid span.
  // Example: (az_span) { .ptr = (uint8_t*)(~0 - 5), .size = 10 } is not a valid span, because most
  // likely you end up pointing to 0x0000 at .ptr[6], &.ptr[7] is 0x...0001, etc.
  uint8_t* const max_ptr = (uint8_t*)~(uint8_t)0;
  result = result && ((size_t)span_size <= (size_t)(max_ptr - ptr));

  return result && min_size <= span_size;
}

#define _az_PRECONDITION_VALID_SPAN(span, min_size, null_is_valid) \
  _az_PRECONDITION(_az_span_is_valid(span, min_size, null_is_valid))

AZ_NODISCARD AZ_INLINE bool _az_span_overlap(az_span a, az_span b)
{
  uint8_t* const a_ptr = az_span_ptr(a);
  uint8_t* const b_ptr = az_span_ptr(b);

  return a_ptr <= b_ptr ? (a_ptr + az_span_size(a) > b_ptr) : (b_ptr + az_span_size(b) > a_ptr);
}

#define _az_PRECONDITION_NO_OVERLAP_SPANS(a, b) _az_PRECONDITION(!_az_span_overlap(a, b))

#include <azure/core/_az_cfg_suffix.h>

#endif // _az_PRECONDITION_INTERNAL_H
