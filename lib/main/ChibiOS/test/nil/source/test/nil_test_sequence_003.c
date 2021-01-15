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

#include "hal.h"
#include "nil_test_root.h"

/**
 * @file    nil_test_sequence_003.c
 * @brief   Test Sequence 003 code.
 *
 * @page nil_test_sequence_003 [3] Semaphores
 *
 * File: @ref nil_test_sequence_003.c
 *
 * <h2>Description</h2>
 * This sequence tests the ChibiOS/NIL functionalities related to
 * counter semaphores.
 *
 * <h2>Conditions</h2>
 * This sequence is only executed if the following preprocessor condition
 * evaluates to true:
 * - CH_CFG_USE_SEMAPHORES
 * .
 *
 * <h2>Test Cases</h2>
 * - @subpage nil_test_003_001
 * - @subpage nil_test_003_002
 * - @subpage nil_test_003_003
 * .
 */

#if (CH_CFG_USE_SEMAPHORES) || defined(__DOXYGEN__)

/****************************************************************************
 * Shared code.
 ****************************************************************************/

#include "ch.h"

static semaphore_t sem1;

/****************************************************************************
 * Test cases.
 ****************************************************************************/

/**
 * @page nil_test_003_001 [3.1] Semaphore primitives, no state change
 *
 * <h2>Description</h2>
 * Wait, Signal and Reset primitives are tested. The testing thread
 * does not trigger a state change.
 *
 * <h2>Test Steps</h2>
 * - [3.1.1] The function chSemWait() is invoked, after return the
 *   counter and the returned message are tested.
 * - [3.1.2] The function chSemSignal() is invoked, after return the
 *   counter is tested.
 * - [3.1.3] The function chSemReset() is invoked, after return the
 *   counter is tested.
 * .
 */

static void nil_test_003_001_setup(void) {
  chSemObjectInit(&sem1, 1);
}

static void nil_test_003_001_teardown(void) {
  chSemReset(&sem1, 0);
}

static void nil_test_003_001_execute(void) {

  /* [3.1.1] The function chSemWait() is invoked, after return the
     counter and the returned message are tested.*/
  test_set_step(1);
  {
    msg_t msg;

    msg = chSemWait(&sem1);
    test_assert_lock(chSemGetCounterI(&sem1) == 0, "wrong counter value");
    test_assert(MSG_OK == msg, "wrong returned message");
  }

  /* [3.1.2] The function chSemSignal() is invoked, after return the
     counter is tested.*/
  test_set_step(2);
  {
    chSemSignal(&sem1);
    test_assert_lock(chSemGetCounterI(&sem1) == 1, "wrong counter value");
  }

  /* [3.1.3] The function chSemReset() is invoked, after return the
     counter is tested.*/
  test_set_step(3);
  {
    chSemReset(&sem1, 2);
    test_assert_lock(chSemGetCounterI(&sem1) == 2, "wrong counter value");
  }
}

static const testcase_t nil_test_003_001 = {
  "Semaphore primitives, no state change",
  nil_test_003_001_setup,
  nil_test_003_001_teardown,
  nil_test_003_001_execute
};

/**
 * @page nil_test_003_002 [3.2] Semaphore primitives, with state change
 *
 * <h2>Description</h2>
 * Wait, Signal and Reset primitives are tested. The testing thread
 * triggers a state change.
 *
 * <h2>Test Steps</h2>
 * - [3.2.1] The function chSemWait() is invoked, after return the
 *   counter and the returned message are tested. The semaphore is
 *   signaled by another thread.
 * - [3.2.2] The function chSemWait() is invoked, after return the
 *   counter and the returned message are tested. The semaphore is
 *   reset by another thread.
 * .
 */

static void nil_test_003_002_setup(void) {
  chSemObjectInit(&gsem1, 0);
}

static void nil_test_003_002_teardown(void) {
  chSemReset(&gsem1, 0);
}

static void nil_test_003_002_execute(void) {

  /* [3.2.1] The function chSemWait() is invoked, after return the
     counter and the returned message are tested. The semaphore is
     signaled by another thread.*/
  test_set_step(1);
  {
    msg_t msg;

    msg = chSemWait(&gsem1);
    test_assert_lock(chSemGetCounterI(&gsem1) == 0, "wrong counter value");
    test_assert(MSG_OK == msg, "wrong returned message");
  }

  /* [3.2.2] The function chSemWait() is invoked, after return the
     counter and the returned message are tested. The semaphore is
     reset by another thread.*/
  test_set_step(2);
  {
    msg_t msg;

    msg = chSemWait(&gsem2);
    test_assert_lock(chSemGetCounterI(&gsem2) == 0,"wrong counter value");
    test_assert(MSG_RESET == msg, "wrong returned message");
  }
}

static const testcase_t nil_test_003_002 = {
  "Semaphore primitives, with state change",
  nil_test_003_002_setup,
  nil_test_003_002_teardown,
  nil_test_003_002_execute
};

/**
 * @page nil_test_003_003 [3.3] Semaphores timeout
 *
 * <h2>Description</h2>
 * Timeout on semaphores is tested.
 *
 * <h2>Test Steps</h2>
 * - [3.3.1] The function chSemWaitTimeout() is invoked a first time,
 *   after return the system time, the counter and the returned message
 *   are tested.
 * - [3.3.2] The function chSemWaitTimeout() is invoked again, after
 *   return the system time, the counter and the returned message are
 *   tested.
 * .
 */

static void nil_test_003_003_setup(void) {
  chSemObjectInit(&sem1, 0);
}

static void nil_test_003_003_teardown(void) {
  chSemReset(&sem1, 0);
}

static void nil_test_003_003_execute(void) {
  systime_t time;
  msg_t msg;

  /* [3.3.1] The function chSemWaitTimeout() is invoked a first time,
     after return the system time, the counter and the returned message
     are tested.*/
  test_set_step(1);
  {
    time = chVTGetSystemTimeX();
    msg = chSemWaitTimeout(&sem1, TIME_MS2I(1000));
    test_assert_time_window(chTimeAddX(time, TIME_MS2I(1000)),
                            chTimeAddX(time, TIME_MS2I(1000) + 1),
                            "out of time window");
    test_assert_lock(chSemGetCounterI(&sem1) == 0, "wrong counter value");
    test_assert(MSG_TIMEOUT == msg, "wrong timeout message");
  }

  /* [3.3.2] The function chSemWaitTimeout() is invoked again, after
     return the system time, the counter and the returned message are
     tested.*/
  test_set_step(2);
  {
    time = chVTGetSystemTimeX();
    msg = chSemWaitTimeout(&sem1, TIME_MS2I(1000));
    test_assert_time_window(chTimeAddX(time, TIME_MS2I(1000)),
                            chTimeAddX(time, TIME_MS2I(1000) + 1),
                            "out of time window");
    test_assert_lock(chSemGetCounterI(&sem1) == 0, "wrong counter value");
    test_assert(MSG_TIMEOUT == msg, "wrong timeout message");
  }
}

static const testcase_t nil_test_003_003 = {
  "Semaphores timeout",
  nil_test_003_003_setup,
  nil_test_003_003_teardown,
  nil_test_003_003_execute
};

/****************************************************************************
 * Exported data.
 ****************************************************************************/

/**
 * @brief   Array of test cases.
 */
const testcase_t * const nil_test_sequence_003_array[] = {
  &nil_test_003_001,
  &nil_test_003_002,
  &nil_test_003_003,
  NULL
};

/**
 * @brief   Semaphores.
 */
const testsequence_t nil_test_sequence_003 = {
  "Semaphores",
  nil_test_sequence_003_array
};

#endif /* CH_CFG_USE_SEMAPHORES */
