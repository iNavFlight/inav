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

#include "hal.h"
#include "ch_test.h"
#include "test_root.h"

/**
 * @page test_sequence_002 Synchronization primitives
 *
 * File: @ref test_sequence_002.c
 *
 * <h2>Description</h2>
 * This sequence tests the ChibiOS/NIL functionalities related to
 * threads synchronization.
 *
 * <h2>Test Cases</h2>
 * - @subpage test_002_001
 * - @subpage test_002_002
 * .
 */

/****************************************************************************
 * Shared code.
 ****************************************************************************/

static semaphore_t sem1;
static thread_reference_t tr1;

/****************************************************************************
 * Test cases.
 ****************************************************************************/

#if TRUE || defined(__DOXYGEN__)
/**
 * @page test_002_001 Semaphore primitives, no state change
 *
 * <h2>Description</h2>
 * Wait, Signal and Reset primitives are tested. The testing thread does not
 * trigger a state change.
 *
 * <h2>Conditions</h2>
 * None.
 *
 * <h2>Test Steps</h2>
 * - The function chSemWait() is invoked, after return the counter and
 *   the returned message are tested.
 * - The function chSemSignal() is invoked, after return the counter
 *   is tested.
 * - The function chSemReset() is invoked, after return the counter
 *   is tested.
 * .
 */

static void test_002_001_setup(void) {

  chSemObjectInit(&sem1, 1);
}

static void test_002_001_teardown(void) {

  chSemReset(&sem1, 0);
}

static void test_002_001_execute(void) {

  /* The function chSemWait() is invoked, after return the counter and
     the returned message are tested.*/
  test_set_step(1);
  {
    msg_t msg;

    msg = chSemWait(&sem1);
    test_assert_lock(chSemGetCounterI(&sem1) == 0,
                     "wrong counter value");
    test_assert(MSG_OK == msg,
                "wrong returned message");
  }

  /* The function chSemSignal() is invoked, after return the counter
     is tested.*/
  test_set_step(2);
  {
    chSemSignal(&sem1);
    test_assert_lock(chSemGetCounterI(&sem1) == 1,
                     "wrong counter value");
  }

  /* The function chSemReset() is invoked, after return the counter
     is tested.*/
  test_set_step(3);
  {
    chSemReset(&sem1, 2);
    test_assert_lock(chSemGetCounterI(&sem1) == 2,
                     "wrong counter value");
  }
}

static const testcase_t test_002_001 = {
  "semaphore primitives, no state change",
  test_002_001_setup,
  test_002_001_teardown,
  test_002_001_execute
};
#endif /* TRUE */

#if TRUE || defined(__DOXYGEN__)
/**
 * @page test_002_002 Semaphore primitives, with state change
 *
 * <h2>Description</h2>
 * Wait, Signal and Reset primitives are tested. The testing thread
 * triggers a state change.
 *
 * <h2>Conditions</h2>
 * None.
 *
 * <h2>Test Steps</h2>
 * - The function chSemWait() is invoked, after return the counter and
 *   the returned message are tested. The semaphore is signaled by another
 *   thread.
 * - The function chSemWait() is invoked, after return the counter and
 *   the returned message are tested. The semaphore is reset by another
 *   thread.
 * .
 */

static void test_002_002_setup(void) {

  chSemObjectInit(&sem1, 0);
}

static void test_002_002_teardown(void) {

  chSemReset(&sem1, 0);
}

static void test_002_002_execute(void) {

  /* The function chSemWait() is invoked, after return the counter and
     the returned message are tested. The semaphore is signaled by another
     thread.*/
  test_set_step(1);
  {
    msg_t msg;

    msg = chSemWait(&gsem1);
    test_assert_lock(chSemGetCounterI(&gsem1) == 0,
                     "wrong counter value");
    test_assert(MSG_OK == msg,
                "wrong returned message");
  }

  /* The function chSemWait() is invoked, after return the counter and
     the returned message are tested. The semaphore is reset by another
     thread.*/
  test_set_step(2);
  {
    msg_t msg;

    msg = chSemWait(&gsem2);
    test_assert_lock(chSemGetCounterI(&gsem2) == 0,
                     "wrong counter value");
    test_assert(MSG_RESET == msg,
                "wrong returned message");
  }
}

static const testcase_t test_002_002 = {
  "semaphore primitives, with state change",
  test_002_002_setup,
  test_002_002_teardown,
  test_002_002_execute
};
#endif /* TRUE */

#if TRUE || defined(__DOXYGEN__)
/**
 * @page test_002_003 Semaphores timeout
 *
 * <h2>Description</h2>
 * Timeout on semaphores is tested.
 *
 * <h2>Conditions</h2>
 * None.
 *
 * <h2>Test Steps</h2>
 * - The function chSemWaitTimeout() is invoked, after return the system
 *   time, the counter and the returned message are tested.
 * .
 */

static void test_002_003_setup(void) {

  chSemObjectInit(&sem1, 0);
}

static void test_002_003_teardown(void) {

  chSemReset(&sem1, 0);
}

static void test_002_003_execute(void) {
  systime_t time;
  msg_t msg;

  /* The function chSemWaitTimeout() is invoked, after return the system
     time, the counter and the returned message are tested.*/
  test_set_step(1);
  {
    time = chVTGetSystemTimeX();
    msg = chSemWaitTimeout(&sem1, MS2ST(1000));
    test_assert_time_window(time + MS2ST(1000),
                            time + MS2ST(1000) + 1,
                            "out of time window");
    test_assert_lock(chSemGetCounterI(&sem1) == 0,
                     "wrong counter value");
    test_assert(MSG_TIMEOUT == msg,
                "wrong timeout message");
  }

  /* The function chSemWaitTimeout() is invoked, after return the system
     time, the counter and the returned message are tested.*/
  test_set_step(2);
  {
    time = chVTGetSystemTimeX();
    msg = chSemWaitTimeout(&sem1, MS2ST(1000));
    test_assert_time_window(time + MS2ST(1000),
                            time + MS2ST(1000) + 1,
                            "out of time window");
    test_assert_lock(chSemGetCounterI(&sem1) == 0,
                     "wrong counter value");
    test_assert(MSG_TIMEOUT == msg,
                "wrong timeout message");
  }
}

static const testcase_t test_002_003 = {
  "semaphores timeout",
  test_002_003_setup,
  test_002_003_teardown,
  test_002_003_execute
};
#endif /* TRUE */

#if TRUE || defined(__DOXYGEN__)
/**
 * @page test_002_004 Suspend and Resume functionality
 *
 * <h2>Description</h2>
 * The functionality of chThdSuspendTimeoutS() and chThdResumeI() is
 * tested.
 *
 * <h2>Conditions</h2>
 * None.
 *
 * <h2>Test Steps</h2>
 * - The function chThdSuspendTimeoutS() is invoked, the thread is
 *   remotely resumed with message @p MSG_OK. On return the message
 *   and the state of the reference are tested.
 * - The function chThdSuspendTimeoutS() is invoked, the thread is
 *   not resumed so a timeout must occur. On return the message
 *   and the state of the reference are tested.
 * .
 */

static void test_002_004_setup(void) {

  tr1 = NULL;
}

static void test_002_004_execute(void) {
  systime_t time;
  msg_t msg;

  /* The function chThdSuspendTimeoutS() is invoked, the thread is
     remotely resumed with message @p MSG_OK. On return the message
     and the state of the reference are tested.*/
  test_set_step(1);
  {
    chSysLock();
    msg = chThdSuspendTimeoutS(&gtr1, TIME_INFINITE);
    chSysUnlock();
    test_assert(NULL == gtr1,
                "not NULL");
    test_assert(MSG_OK == msg,
                "wrong returned message");
  }

  /* The function chThdSuspendTimeoutS() is invoked, the thread is
     not resumed so a timeout must occur. On return the message
     and the state of the reference are tested.*/
  test_set_step(2);
  {
    chSysLock();
    time = chVTGetSystemTimeX();
    msg = chThdSuspendTimeoutS(&tr1, MS2ST(1000));
    chSysUnlock();
    test_assert_time_window(time + MS2ST(1000),
                            time + MS2ST(1000) + 1,
                            "out of time window");
    test_assert(NULL == tr1,
                "not NULL");
    test_assert(MSG_TIMEOUT == msg,
                "wrong returned message");
  }
}

static const testcase_t test_002_004 = {
  "suspend and resume functionality",
  test_002_004_setup,
  NULL,
  test_002_004_execute
};
#endif /* TRUE */

#if (NIL_CFG_USE_EVENTS == TRUE) || defined(__DOXYGEN__)
/**
 * @page test_002_005 Events functionality
 *
 * <h2>Description</h2>
 * Event flags functionality is tested.
 *
 * <h2>Conditions</h2>
 * None.
 *
 * <h2>Test Steps</h2>
 * - A set of event flags are set on the current thread then the
 *   function chVTGetSystemTimeX() is invoked, the function is supposed to
 *   return immediately because the event flags are already pending,
 *   after return the events mask is tested.
 * - The pending event flags mask is cleared then the function
 *   chVTGetSystemTimeX() is invoked, after return the events
 *   mask is tested. The thread is signaled by another thread.
 * -
 * . The function chVTGetSystemTimeX() is invoked, no event can
 *   wakeup the thread, the function must return because timeout.
 */

static void test_002_005_setup(void) {

  chSemObjectInit(&sem1, 0);
}

static void test_002_005_execute(void) {
  systime_t time;
  eventmask_t events;

  /* A set of event flags are set on the current thread then the
     function chVTGetSystemTimeX() is invoked, the function is supposed to
     return immediately because the event flags are already pending,
     after return the events mask is tested.*/
  test_set_step(1);
  {
    time = chVTGetSystemTimeX();
    chEvtSignal(chThdGetSelfX(), 0x55);
    events = chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(1000));
    test_assert((eventmask_t)0 != events,
                "timed out");
    test_assert((eventmask_t)0x55 == events,
                "wrong events mask");
  }

  /* The pending event flags mask is cleared then the function
     chVTGetSystemTimeX() is invoked, after return the events
     mask is tested. The thread is signaled by another thread.*/
  test_set_step(2);
  {
    time = chVTGetSystemTimeX();
    chThdGetSelfX()->epmask = 0;
    events = chEvtWaitAnyTimeout(ALL_EVENTS, MS2ST(1000));
    test_assert((eventmask_t)0 != events,
                "timed out");
    test_assert((eventmask_t)0x55 == events,
                "wrong events mask");
  }

  /* The function chVTGetSystemTimeX() is invoked, no event can
     wakeup the thread, the function must return because timeout.*/
  test_set_step(3);
  {
    time = chVTGetSystemTimeX();
    events = chEvtWaitAnyTimeout(0, MS2ST(1000));
    test_assert_time_window(time + MS2ST(1000),
                            time + MS2ST(1000) + 1,
                            "out of time window");
    test_assert((eventmask_t)0 == events,
                "wrong events mask");
  }
}

static const testcase_t test_002_005 = {
  "events functionality",
  test_002_005_setup,
  NULL,
  test_002_005_execute
};
#endif /* NIL_CFG_USE_EVENTS == TRUE */

 /****************************************************************************
 * Exported data.
 ****************************************************************************/

/**
 * @brief   Sequence brief description.
 */
const testcase_t * const test_sequence_002[] = {
#if TRUE || defined(__DOXYGEN__)
  &test_002_001,
#endif
#if TRUE || defined(__DOXYGEN__)
  &test_002_002,
#endif
#if TRUE || defined(__DOXYGEN__)
  &test_002_003,
#endif
#if TRUE || defined(__DOXYGEN__)
  &test_002_004,
#endif
#if (NIL_CFG_USE_EVENTS == TRUE) || defined(__DOXYGEN__)
  &test_002_005,
#endif
  NULL
};
