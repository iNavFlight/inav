/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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

#ifndef CH_TEST_H
#define CH_TEST_H

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

/**
 * @brief   Delay inserted between test cases.
 */
#if !defined(TEST_SHOW_SEQUENCES) || defined(__DOXYGEN__)
#define TEST_SHOW_SEQUENCES                 TRUE
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

/**
 * @brief   Structure representing a test sequence.
 */
typedef struct {
  const char        *name;          /**< @brief Name of the test sequence.  */
  const testcase_t * const * cases; /**< @brief Test cases array.           */
} testsequence_t;

/**
 * @brief   Type of a test suite.
 */
typedef struct {
  const char        *name;          /**< @brief Name of the test suite.     */
  const testsequence_t * const * sequences; /**< @brief Test sequences array.           */
} testsuite_t;

/**
 * @brief   Type of a test suite.
 */
//typedef const testcase_t * const *testsuite_t[];

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
 * @brief   End step marker.
 *
 * @param[in] step      the step number
 */
#define test_end_step(step) (void)(step);

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
extern bool test_global_fail;
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
  msg_t test_execute(BaseSequentialStream *stream, const testsuite_t *tsp);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Module inline functions.                                                  */
/*===========================================================================*/

#endif /* CH_TEST_H */

/** @} */
