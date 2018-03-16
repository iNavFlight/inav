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
 * @file    test_root.c
 * @brief   Test Suite root structures code.
 *
 * @addtogroup CH_TEST_ROOT
 * @{
 */

#include "hal.h"
#include "ch_test.h"
#include "test_root.h"

/*===========================================================================*/
/* Module exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   Array of all the test sequences.
 */
const testcase_t * const *test_suite[] = {
  test_sequence_001,
  test_sequence_002,
  NULL
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
#if NIL_CFG_USE_EVENTS == TRUE
  thread_t *tp = (thread_t *)arg;
#else
  (void)arg;
#endif

  /* Initializing global resources.*/
  chSemObjectInit(&gsem1, 0);
  chSemObjectInit(&gsem2, 0);

  /* Waiting for button push and activation of the test suite.*/
  while (true) {
    chSysLock();
    if (chSemGetCounterI(&gsem1) < 0)
      chSemSignalI(&gsem1);
    chSemResetI(&gsem2, 0);
    chThdResumeI(&gtr1, MSG_OK);
#if NIL_CFG_USE_EVENTS == TRUE
    chEvtSignalI(tp, 0x55);
#endif
    chSchRescheduleS();
    chSysUnlock();

    chThdSleepMilliseconds(250);
  }
}

/** @} */
