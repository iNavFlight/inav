/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    ch_test.h
 * @brief   Unit Tests Engine Module macros and structures.
 *
 * @addtogroup CH_TEST
 * @{
 */

#ifndef _CH_TEST_H_
#define _CH_TEST_H_

/*===========================================================================*/
/* Module constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Module pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Maximum number of entries in the tokens buffer.
 */
#if !defined(TEST_MAX_TOKENS) || defined(__DOXYGEN__)
#define TEST_MAX_TOKENS                     16
#endif

/**
 * @brief   Delay inserted between test cases.
 */
#if !defined(TEST_DELAY_BETWEEN_TESTS) || defined(__DOXYGEN__)
#define TEST_DELAY_BETWEEN_TESTS            200
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Structure representing a test case.
 */
typedef struct {
  const char *name;             /**< @brief Test case name.                 */
  void (*setup)(void);          /**< @brief Test case preparation function. */
  void (*teardown)(void);       /**< @brief Test case clean up function.    */
  void (*execute)(void);        /**< @brief Test case execution function.   */
} testcase_t;

/*===========================================================================*/
/* Module macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Sets the step identifier.
 *
 * @param[in] step      the step number
 */
#define test_set_step(step) test_step = (step)

/**
 * @brief   Test failure enforcement.
 * @note    This function can only be called from test_case execute context.
 *
 * @param[in] msg       failure message as string
 *
 * @api
 */
#define test_fail(msg) {                                                    \
  _test_fail(msg);                                                          \
  return;                                                                   \
}

/**
 * @brief   Test assertion.
 * @note    This function can only be called from test_case execute context.
 *
 * @param[in] condition a boolean expression that must be verified to be true
 * @param[in] msg       failure message as string
 *
 * @api
 */
#define test_assert(condition, msg) {                                       \
  if (_test_assert(condition, msg))                                         \
    return;                                                                 \
}

/**
 * @brief   Test assertion with lock.
 * @note    This function can only be called from test_case execute context.
 *
 * @param[in] condition a boolean expression that must be verified to be true
 * @param[in] msg       failure message as string
 *
 * @api
 */
#define test_assert_lock(condition, msg) {                                  \
  osalSysLock();                                                            \
  if (_test_assert(condition, msg)) {                                       \
    osalSysUnlock();                                                        \
    return;                                                                 \
  }                                                                         \
  osalSysUnlock();                                                          \
}

/**
 * @brief   Test sequence assertion.
 * @note    This function can only be called from test_case execute context.
 *
 * @param[in] expected  string to be matched with the tokens buffer
 * @param[in] msg       failure message as string
 *
 * @api
 */
#define test_assert_sequence(expected, msg) {                               \
  if (_test_assert_sequence(expected, msg))                                 \
    return;                                                                 \
}

/**
 * @brief   Test time window assertion.
 * @note    This function can only be called from test_case execute context.
 *
 * @param[in] start     initial time in the window (included)
 * @param[in] end       final time in the window (not included)
 * @param[in] msg       failure message as string
 *
 * @api
 */
#define test_assert_time_window(start, end, msg) {                          \
  if (_test_assert_time_window(start, end, msg))                            \
    return;                                                                 \
}

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
extern unsigned test_step;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  bool _test_fail(const char *message);
  bool _test_assert(bool condition, const char *msg);
  bool _test_assert_sequence(char *expected, const char *msg);
  bool _test_assert_time_window(systime_t start,
                                systime_t end,
                                const char *msg);
  void test_printn(uint32_t n);
  void test_print(const char *msgp);
  void test_println(const char *msgp);
  void test_emit_token(char token);
  void test_emit_token_i(char token);
  msg_t test_execute(BaseSequentialStream *stream);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

/*===========================================================================*/
/* Late inclusions.                                                          */
/*===========================================================================*/

#include "test_root.h"

#endif /* _CH_TEST_H_ */

/** @} */
