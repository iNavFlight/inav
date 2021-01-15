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
 * @file    nil_test_sequence_004.c
 * @brief   Test Sequence 004 code.
 *
 * @page nil_test_sequence_004 [4] Suspend/Resume and Event Flags
 *
 * File: @ref nil_test_sequence_004.c
 *
 * <h2>Description</h2>
 * This sequence tests the ChibiOS/NIL functionalities related to
 * threads suspend/resume and event flags.
 *
 * <h2>Test Cases</h2>
 * - @subpage nil_test_004_001
 * - @subpage nil_test_004_002
 * .
 */

/****************************************************************************
 * Shared code.
 ****************************************************************************/

static thread_reference_t tr1;

/****************************************************************************
 * Test cases.
 ****************************************************************************/

/**
 * @page nil_test_004_001 [4.1] Suspend and Resume functionality
 *
 * <h2>Description</h2>
 * The functionality of chThdSuspendTimeoutS() and chThdResumeI() is
 * tested.
 *
 * <h2>Test Steps</h2>
 * - [4.1.1] The function chThdSuspendTimeoutS() is invoked, the thread
 *   is remotely resumed with message @p MSG_OK. On return the message
 *   and the state of the reference are tested.
 * - [4.1.2] The function chThdSuspendTimeoutS() is invoked, the thread
 *   is not resumed so a timeout must occur. On return the message and
 *   the state of the reference are tested.
 * .
 */

static void nil_test_004_001_setup(void) {
  tr1 = NULL;
}

static void nil_test_004_001_execute(void) {
  systime_t time;
  msg_t msg;

  /* [4.1.1] The function chThdSuspendTimeoutS() is invoked, the thread
     is remotely resumed with message @p MSG_OK. On return the message
     and the state of the reference are tested.*/
  test_set_step(1);
  {
    chSysLock();
    msg = chThdSuspendTimeoutS(&gtr1, TIME_INFINITE);
    chSysUnlock();
    test_assert(NULL == gtr1, "not NULL");
    test_assert(MSG_OK == msg,"wrong returned message");
  }

  /* [4.1.2] The function chThdSuspendTimeoutS() is invoked, the thread
     is not resumed so a timeout must occur. On return the message and
     the state of the reference are tested.*/
  test_set_step(2);
  {
    chSysLock();
    time = chVTGetSystemTimeX();
    msg = chThdSuspendTimeoutS(&tr1, TIME_MS2I(1000));
    chSysUnlock();
    test_assert_time_window(chTimeAddX(time, TIME_MS2I(1000)),
                            chTimeAddX(time, TIME_MS2I(1000) + 1),
                            "out of time window");
    test_assert(NULL == tr1, "not NULL");
    test_assert(MSG_TIMEOUT == msg, "wrong returned message");
  }
}

static const testcase_t nil_test_004_001 = {
  "Suspend and Resume functionality",
  nil_test_004_001_setup,
  NULL,
  nil_test_004_001_execute
};

#if (CH_CFG_USE_EVENTS) || defined(__DOXYGEN__)
/**
 * @page nil_test_004_002 [4.2] Events Flags functionality
 *
 * <h2>Description</h2>
 * Event flags functionality is tested.
 *
 * <h2>Conditions</h2>
 * This test is only executed if the following preprocessor condition
 * evaluates to true:
 * - CH_CFG_USE_EVENTS
 * .
 *
 * <h2>Test Steps</h2>
 * - [4.2.1] A set of event flags are set on the current thread then
 *   the function chEvtWaitAnyTimeout() is invoked, the function is
 *   supposed to return immediately because the event flags are already
 *   pending, after return the events mask is tested.
 * - [4.2.2] The pending event flags mask is cleared then the function
 *   chEvtWaitAnyTimeout() is invoked, after return the events mask is
 *   tested. The thread is signaled by another thread.
 * - [4.2.3] The function chEvtWaitAnyTimeout() is invoked, no event
 *   can wakeup the thread, the function must return because timeout.
 * .
 */

static void nil_test_004_002_execute(void) {
  systime_t time;
  eventmask_t events;

  /* [4.2.1] A set of event flags are set on the current thread then
     the function chEvtWaitAnyTimeout() is invoked, the function is
     supposed to return immediately because the event flags are already
     pending, after return the events mask is tested.*/
  test_set_step(1);
  {
    time = chVTGetSystemTimeX();
    chEvtSignal(chThdGetSelfX(), 0x55);
    events = chEvtWaitAnyTimeout(ALL_EVENTS, TIME_MS2I(1000));
    test_assert((eventmask_t)0 != events, "timed out");
    test_assert((eventmask_t)0x55 == events, "wrong events mask");
  }

  /* [4.2.2] The pending event flags mask is cleared then the function
     chEvtWaitAnyTimeout() is invoked, after return the events mask is
     tested. The thread is signaled by another thread.*/
  test_set_step(2);
  {
    time = chVTGetSystemTimeX();
    chThdGetSelfX()->epmask = 0;
    events = chEvtWaitAnyTimeout(ALL_EVENTS, TIME_MS2I(1000));
    test_assert((eventmask_t)0 != events, "timed out");
    test_assert((eventmask_t)0x55 == events, "wrong events mask");
  }

  /* [4.2.3] The function chEvtWaitAnyTimeout() is invoked, no event
     can wakeup the thread, the function must return because timeout.*/
  test_set_step(3);
  {
    time = chVTGetSystemTimeX();
    events = chEvtWaitAnyTimeout(0, TIME_MS2I(1000));
    test_assert_time_window(chTimeAddX(time, TIME_MS2I(1000)),
                            chTimeAddX(time, TIME_MS2I(1000) + 1),
                            "out of time window");
    test_assert((eventmask_t)0 == events, "wrong events mask");
  }
}

static const testcase_t nil_test_004_002 = {
  "Events Flags functionality",
  NULL,
  NULL,
  nil_test_004_002_execute
};
#endif /* CH_CFG_USE_EVENTS */

/****************************************************************************
 * Exported data.
 ****************************************************************************/

/**
 * @brief   Array of test cases.
 */
const testcase_t * const nil_test_sequence_004_array[] = {
  &nil_test_004_001,
#if (CH_CFG_USE_EVENTS) || defined(__DOXYGEN__)
  &nil_test_004_002,
#endif
  NULL
};

/**
 * @brief   Suspend/Resume and Event Flags.
 */
const testsequence_t nil_test_sequence_004 = {
  "Suspend/Resume and Event Flags",
  nil_test_sequence_004_array
};
