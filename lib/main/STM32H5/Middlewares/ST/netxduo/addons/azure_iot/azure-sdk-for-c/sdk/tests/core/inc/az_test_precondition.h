// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef _az_TEST_PRECONDITION_H
#define _az_TEST_PRECONDITION_H

// These headers must be included prior to including cmocka.h.
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>

#include <assert.h>
#include <stdint.h>

#include <cmocka.h>

// This block defines the resources needed to verify precondition checking.
// Macro ENABLE_PRECONDITION_CHECK_TESTS() (no semi-colon at end) shall be invoked in the unit test
// module right after includes. Macro SETUP_PRECONDITION_CHECK_TESTS() shall be invoked in the unit
// test module entry function, before any test is executed. Macro ASSERT_PRECONDITION_CHECKED(func)
// shall be used in each test to assert if a precondition is verified. If a precondition is not
// verified within a function the result will be either:
// - A crash in the test, as the the target function will continue to execute with an invalid
// argument, or
// - An assert failure indicating a precondition was not tested.
// Notice that:
// - If ASSERT_PRECONDITION_CHECKED(func) is used, the module must include <setjmp.h>;
// - If a function has two precondition checks and both are supposed to fail on a given test,
// ASSERT_PRECONDITION_CHECKED(func)
//   is unable to distinguish which precondition has failed first. Testing precondition checking
//   separately is advised.
// - Tests using ASSERT_PRECONDITION_CHECKED(func) currently must not be run in parallel (!).

#define ENABLE_PRECONDITION_CHECK_TESTS()          \
  static jmp_buf g_precond_test_jmp_buf;           \
  static unsigned int precondition_test_count = 0; \
  static void az_precondition_test_failed_fn()     \
  {                                                \
    precondition_test_count++;                     \
    longjmp(g_precond_test_jmp_buf, 0);            \
  }

#define SETUP_PRECONDITION_CHECK_TESTS() \
  az_precondition_failed_set_callback(az_precondition_test_failed_fn);

// In release builds, the compiler optimizes out 'ASSERT_PRECONDITION_CHECKED' which could result in
// function parameters not being used. Explicitly storing the function result as a bool and using
// (void) to cast it away so that we don't get a warning related to unused variables, particularly
// in release configurations.
#define ASSERT_PRECONDITION_CHECKED(fn)           \
  do                                              \
  {                                               \
    precondition_test_count = 0;                  \
    (void)setjmp(g_precond_test_jmp_buf);         \
    if (precondition_test_count == 0)             \
    {                                             \
      bool const result = (fn);                   \
      assert(result);                             \
      (void)result;                               \
    }                                             \
    assert_int_equal(1, precondition_test_count); \
  } while (0)

#endif // _az_TEST_PRECONDITION_H
