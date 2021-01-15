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
 * @file    nil_test_sequence_002.c
 * @brief   Test Sequence 002 code.
 *
 * @page nil_test_sequence_002 [2] Threads Functionality
 *
 * File: @ref nil_test_sequence_002.c
 *
 * <h2>Description</h2>
 * This sequence tests the ChibiOS/NIL functionalities related to
 * threading.
 *
 * <h2>Test Cases</h2>
 * - @subpage nil_test_002_001
 * - @subpage nil_test_002_002
 * .
 */

/****************************************************************************
 * Shared code.
 ****************************************************************************/

#include "ch.h"

/****************************************************************************
 * Test cases.
 ****************************************************************************/

/**
 * @page nil_test_002_001 [2.1] System Tick Counter functionality
 *
 * <h2>Description</h2>
 * The functionality of the API @p chVTGetSystemTimeX() is tested.
 *
 * <h2>Test Steps</h2>
 * - [2.1.1] A System Tick Counter increment is expected, the test
 *   simply hangs if it does not happen.
 * .
 */

static void nil_test_002_001_execute(void) {

  /* [2.1.1] A System Tick Counter increment is expected, the test
     simply hangs if it does not happen.*/
  test_set_step(1);
  {
    systime_t time = chVTGetSystemTimeX();
    while (time == chVTGetSystemTimeX()) {
    }
  }
}

static const testcase_t nil_test_002_001 = {
  "System Tick Counter functionality",
  NULL,
  NULL,
  nil_test_002_001_execute
};

/**
 * @page nil_test_002_002 [2.2] Thread Sleep functionality
 *
 * <h2>Description</h2>
 * The functionality of @p chThdSleep() and derivatives is tested.
 *
 * <h2>Test Steps</h2>
 * - [2.2.1] The current system time is read then a sleep is performed
 *   for 100 system ticks and on exit the system time is verified
 *   again.
 * - [2.2.2] The current system time is read then a sleep is performed
 *   for 100000 microseconds and on exit the system time is verified
 *   again.
 * - [2.2.3] The current system time is read then a sleep is performed
 *   for 100 milliseconds and on exit the system time is verified
 *   again.
 * - [2.2.4] The current system time is read then a sleep is performed
 *   for 1 second and on exit the system time is verified again.
 * - [2.2.5] Function chThdSleepUntil() is tested with a timeline of
 *   "now" + 100 ticks.
 * .
 */

static void nil_test_002_002_execute(void) {
  systime_t time;

  /* [2.2.1] The current system time is read then a sleep is performed
     for 100 system ticks and on exit the system time is verified
     again.*/
  test_set_step(1);
  {
    time = chVTGetSystemTimeX();
    chThdSleep(100);
    test_assert_time_window(chTimeAddX(time, 100),
                            chTimeAddX(time, 100 + 1),
                            "out of time window");
  }

  /* [2.2.2] The current system time is read then a sleep is performed
     for 100000 microseconds and on exit the system time is verified
     again.*/
  test_set_step(2);
  {
    time = chVTGetSystemTimeX();
    chThdSleepMicroseconds(100000);
    test_assert_time_window(chTimeAddX(time, TIME_US2I(100000)),
                            chTimeAddX(time, TIME_US2I(100000) + 1),
                            "out of time window");
  }

  /* [2.2.3] The current system time is read then a sleep is performed
     for 100 milliseconds and on exit the system time is verified
     again.*/
  test_set_step(3);
  {
    time = chVTGetSystemTimeX();
    chThdSleepMilliseconds(100);
    test_assert_time_window(chTimeAddX(time, TIME_MS2I(100)),
                            chTimeAddX(time, TIME_MS2I(100) + 1),
                            "out of time window");
  }

  /* [2.2.4] The current system time is read then a sleep is performed
     for 1 second and on exit the system time is verified again.*/
  test_set_step(4);
  {
    time = chVTGetSystemTimeX();
    chThdSleepSeconds(1);
    test_assert_time_window(chTimeAddX(time, TIME_S2I(1)),
                            chTimeAddX(time, TIME_S2I(1) + 1),
                            "out of time window");
  }

  /* [2.2.5] Function chThdSleepUntil() is tested with a timeline of
     "now" + 100 ticks.*/
  test_set_step(5);
  {
    time = chVTGetSystemTimeX();
    chThdSleepUntil(chTimeAddX(time, 100));
    test_assert_time_window(chTimeAddX(time, 100),
                            chTimeAddX(time, 100 + 1),
                            "out of time window");
  }
}

static const testcase_t nil_test_002_002 = {
  "Thread Sleep functionality",
  NULL,
  NULL,
  nil_test_002_002_execute
};

/****************************************************************************
 * Exported data.
 ****************************************************************************/

/**
 * @brief   Array of test cases.
 */
const testcase_t * const nil_test_sequence_002_array[] = {
  &nil_test_002_001,
  &nil_test_002_002,
  NULL
};

/**
 * @brief   Threads Functionality.
 */
const testsequence_t nil_test_sequence_002 = {
  "Threads Functionality",
  nil_test_sequence_002_array
};
