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
 * @file    ch_test.c
 * @brief   Unit Tests Engine module code.
 *
 * @addtogroup CH_TEST
 * @{
 */

#include "hal.h"
#include "ch_test.h"
#include "test_root.h"

/*===========================================================================*/
/* Module local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   Test step being executed.
 */
unsigned test_step;

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

static bool test_local_fail;
static bool test_global_fail;
static const char *test_failure_message;
static char test_tokens_buffer[TEST_MAX_TOKENS];
static char *test_tokp;
static BaseSequentialStream *test_chp;

/*===========================================================================*/
/* Module local functions.                                                   */
/*===========================================================================*/

static void clear_tokens(void) {

  test_tokp = test_tokens_buffer;
}

static void print_tokens(void) {
  char *cp = test_tokens_buffer;

  while (cp < test_tokp)
    streamPut(test_chp, *cp++);
}

static void execute_test(const testcase_t *tcp) {

  /* Initialization */
  clear_tokens();
  test_local_fail = FALSE;

  if (tcp->setup != NULL)
    tcp->setup();
  tcp->execute();
  if (tcp->teardown != NULL)
    tcp->teardown();
}

static void print_line(void) {
  unsigned i;

  for (i = 0; i < 76; i++)
    streamPut(test_chp, '-');
  streamWrite(test_chp, (const uint8_t *)"\r\n", 2);
}

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

bool _test_fail(const char *msg) {

  test_local_fail = TRUE;
  test_global_fail = TRUE;
  test_failure_message = msg;
  return TRUE;
}

bool _test_assert(bool condition, const char *msg) {

  if (!condition)
    return _test_fail(msg);
  return FALSE;
}

bool _test_assert_sequence(char *expected, const char *msg) {
  char *cp = test_tokens_buffer;

  while (cp < test_tokp) {
    if (*cp++ != *expected++)
     return _test_fail(msg);
  }

  if (*expected)
    return _test_fail(msg);

  clear_tokens();

  return FALSE;
}

bool _test_assert_time_window(systime_t start,
                              systime_t end,
                              const char *msg) {

  return _test_assert(osalOsIsTimeWithinX(osalOsGetSystemTimeX(), start, end),
                      msg);
}

/**
 * @brief   Prints a decimal unsigned number.
 *
 * @param[in] n         the number to be printed
 *
 * @api
 */
void test_printn(uint32_t n) {
  char buf[16], *p;

  if (!n)
    streamPut(test_chp, '0');
  else {
    p = buf;
    while (n)
      *p++ = (n % 10) + '0', n /= 10;
    while (p > buf)
      streamPut(test_chp, *--p);
  }
}

/**
 * @brief   Prints a line without final end-of-line.
 *
 * @param[in] msgp      the message
 *
 * @api
 */
void test_print(const char *msgp) {

  while (*msgp)
    streamPut(test_chp, *msgp++);
}

/**
 * @brief   Prints a line.
 *
 * @param[in] msgp      the message
 *
 * @api
 */
void test_println(const char *msgp) {

  test_print(msgp);
  streamWrite(test_chp, (const uint8_t *)"\r\n", 2);
}

/**
 * @brief   Emits a token into the tokens buffer.
 *
 * @param[in] token     the token as a char
 *
 * @api
 */
void test_emit_token(char token) {

  osalSysLock();
  if (test_tokp < &test_tokens_buffer[TEST_MAX_TOKENS])
    *test_tokp++ = token;
  osalSysUnlock();
}

/**
 * @brief   Emits a token into the tokens buffer from a critical zone.
 *
 * @param[in] token     the token as a char
 *
 * @iclass
 */
void test_emit_token_i(char token) {

  if (test_tokp < &test_tokens_buffer[TEST_MAX_TOKENS])
    *test_tokp++ = token;
}

/**
 * @brief   Test execution thread function.
 *
 * @param[in] stream    pointer to a @p BaseSequentialStream object for test
 *                      output
 * @return              A failure boolean value casted to @p msg_t.
 * @retval FALSE        if no errors occurred.
 * @retval TRUE         if one or more tests failed.
 *
 * @api
 */
msg_t test_execute(BaseSequentialStream *stream) {
  int i, j;

  test_chp = stream;
  test_println("");
#if defined(TEST_SUITE_NAME)
  test_println("*** " TEST_SUITE_NAME);
#else
  test_println("*** ChibiOS test suite");
#endif
  test_println("***");
  test_print("*** Compiled:     ");
  test_println(__DATE__ " - " __TIME__);
#ifdef PLATFORM_NAME
  test_print("*** Platform:     ");
  test_println(PLATFORM_NAME);
#endif
#ifdef BOARD_NAME
  test_print("*** Test Board:   ");
  test_println(BOARD_NAME);
#endif
  test_println("");

  test_global_fail = FALSE;
  i = 0;
  while (test_suite[i]) {
    j = 0;
    while (test_suite[i][j]) {
      print_line();
      test_print("--- Test Case ");
      test_printn(i + 1);
      test_print(".");
      test_printn(j + 1);
      test_print(" (");
      test_print(test_suite[i][j]->name);
      test_println(")");
#if TEST_DELAY_BETWEEN_TESTS > 0
      osalThreadSleepMilliseconds(TEST_DELAY_BETWEEN_TESTS);
#endif
      execute_test(test_suite[i][j]);
      if (test_local_fail) {
        test_print("--- Result: FAILURE (#");
        test_printn(test_step);
        test_print(" [");
        print_tokens();
        test_print("] \"");
        test_print(test_failure_message);
        test_println("\")");
      }
      else
        test_println("--- Result: SUCCESS");
      j++;
    }
    i++;
  }
  print_line();
  test_println("");
  test_print("Final result: ");
  if (test_global_fail)
    test_println("FAILURE");
  else
    test_println("SUCCESS");

  return (msg_t)test_global_fail;
}

/** @} */
