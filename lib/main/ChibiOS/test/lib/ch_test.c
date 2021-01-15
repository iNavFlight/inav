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
 * @file    ch_test.c
 * @brief   Unit Tests Engine module code.
 *
 * @addtogroup CH_TEST
 * @{
 */

#include "hal.h"
#include "ch_test.h"

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

/**
 * @brief   Test result flag.
 */
bool test_global_fail;

/*===========================================================================*/
/* Module local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Module local variables.                                                   */
/*===========================================================================*/

static bool test_local_fail;
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
  test_local_fail = false;

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

static void print_fat_line(void) {
  unsigned i;

  for (i = 0; i < 76; i++)
    streamPut(test_chp, '=');
  streamWrite(test_chp, (const uint8_t *)"\r\n", 2);
}

/*===========================================================================*/
/* Module exported functions.                                                */
/*===========================================================================*/

bool _test_fail(const char *msg) {

  test_local_fail      = true;
  test_global_fail     = true;
  test_failure_message = msg;
  return true;
}

bool _test_assert(bool condition, const char *msg) {

  if (!condition)
    return _test_fail(msg);
  return false;
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

  return false;
}

bool _test_assert_time_window(systime_t start,
                              systime_t end,
                              const char *msg) {

  return _test_assert(osalTimeIsInRangeX(osalOsGetSystemTimeX(), start, end),
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
 * @param[in] tsp       test suite to execute
 * @return              A failure boolean value casted to @p msg_t.
 * @retval false        if no errors occurred.
 * @retval true         if one or more tests failed.
 *
 * @api
 */
msg_t test_execute(BaseSequentialStream *stream, const testsuite_t *tsp) {
  int tseq, tcase;

  test_chp = stream;
  test_println("");
  if (tsp->name != NULL) {
    test_print("*** ");
    test_println(tsp->name);
  }
  else {
    test_println("*** Test Suite");
  }
  test_println("***");
  test_print("*** Compiled:     ");
  test_println(__DATE__ " - " __TIME__);
#if defined(PLATFORM_NAME)
  test_print("*** Platform:     ");
  test_println(PLATFORM_NAME);
#endif
#if defined(BOARD_NAME)
  test_print("*** Test Board:   ");
  test_println(BOARD_NAME);
#endif
#if defined(TEST_SIZE_REPORT)
  {
    extern uint8_t __text_base, __text_end,
                   _data_start, _data_end,
                   _bss_start, _bss_end;
    test_println("***");
    test_print("*** Text size:    ");
    test_printn((uint32_t)(&__text_end - &__text_base));
    test_println(" bytes");
    test_print("*** Data size:    ");
    test_printn((uint32_t)(&_data_end - &_data_start));
    test_println(" bytes");
    test_print("*** BSS size:     ");
    test_printn((uint32_t)(&_bss_end - &_bss_start));
    test_println(" bytes");
  }
#endif
#if defined(TEST_REPORT_HOOK_HEADER)
  TEST_REPORT_HOOK_HEADER
#endif
  test_println("");

  test_global_fail = false;
  tseq = 0;
  while (tsp->sequences[tseq] != NULL) {
#if TEST_SHOW_SEQUENCES == TRUE
    print_fat_line();
    test_print("=== Test Sequence ");
    test_printn(tseq + 1);
    test_print(" (");
    test_print(tsp->sequences[tseq]->name);
    test_println(")");
#endif
    tcase = 0;
    while (tsp->sequences[tseq]->cases[tcase] != NULL) {
      print_line();
      test_print("--- Test Case ");
      test_printn(tseq + 1);
      test_print(".");
      test_printn(tcase + 1);
      test_print(" (");
      test_print(tsp->sequences[tseq]->cases[tcase]->name);
      test_println(")");
#if TEST_DELAY_BETWEEN_TESTS > 0
      osalThreadSleepMilliseconds(TEST_DELAY_BETWEEN_TESTS);
#endif
      execute_test(tsp->sequences[tseq]->cases[tcase]);
      if (test_local_fail) {
        test_print("--- Result: FAILURE (#");
        test_printn(test_step);
        test_print(" [");
        print_tokens();
        test_print("] \"");
        test_print(test_failure_message);
        test_println("\")");
      }
      else {
        test_println("--- Result: SUCCESS");
      }
      tcase++;
    }
    tseq++;
  }
  print_line();
  test_println("");
  test_print("Final result: ");
  if (test_global_fail)
    test_println("FAILURE");
  else
    test_println("SUCCESS");

#if defined(TEST_REPORT_HOOK_END)
  TEST_REPORT_HOOK_END
#endif

  return (msg_t)test_global_fail;
}

/** @} */
