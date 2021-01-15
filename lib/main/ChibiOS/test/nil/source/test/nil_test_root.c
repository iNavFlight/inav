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
 * @mainpage Test Suite Specification
 * Test suite for ChibiOS/NIL. The purpose of this suite is to perform
 * unit tests on the NIL modules and to converge to 100% code coverage
 * through successive improvements.
 *
 * <h2>Test Sequences</h2>
 * - @subpage nil_test_sequence_001
 * - @subpage nil_test_sequence_002
 * - @subpage nil_test_sequence_003
 * - @subpage nil_test_sequence_004
 * .
 */

/**
 * @file    nil_test_root.c
 * @brief   Test Suite root structures code.
 */

#include "hal.h"
#include "nil_test_root.h"

#if !defined(__DOXYGEN__)

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   Array of test sequences.
 */
const testsequence_t * const nil_test_suite_array[] = {
  &nil_test_sequence_001,
  &nil_test_sequence_002,
#if (CH_CFG_USE_SEMAPHORES) || defined(__DOXYGEN__)
  &nil_test_sequence_003,
#endif
  &nil_test_sequence_004,
  NULL
};

/**
 * @brief   Test suite root structure.
 */
const testsuite_t nil_test_suite = {
  "ChibiOS/NIL Test Suite",
  nil_test_suite_array
};

/*===========================================================================*/
/* Shared code.                                                              */
/*===========================================================================*/

semaphore_t gsem1, gsem2;
thread_reference_t gtr1;

/*
 * Support thread.
 */
THD_WORKING_AREA(wa_test_support, 128);
THD_FUNCTION(test_support, arg) {
#if CH_CFG_USE_EVENTS == TRUE
  thread_t *tp = (thread_t *)arg;
#else
  (void)arg;
#endif

  /* Initializing global resources.*/
  chSemObjectInit(&gsem1, 0);
  chSemObjectInit(&gsem2, 0);

  while (true) {
    chSysLock();
    if (chSemGetCounterI(&gsem1) < 0)
      chSemSignalI(&gsem1);
    chSemResetI(&gsem2, 0);
    chThdResumeI(&gtr1, MSG_OK);
#if CH_CFG_USE_EVENTS == TRUE
    chEvtSignalI(tp, 0x55);
#endif
    chSchRescheduleS();
    chSysUnlock();

    chThdSleepMilliseconds(250);
  }
}

#endif /* !defined(__DOXYGEN__) */
