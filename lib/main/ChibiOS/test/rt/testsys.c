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

#include "ch.h"
#include "test.h"

/**
 * @page test_sys System test
 *
 * File: @ref testsys.c
 *
 * <h2>Description</h2>
 * This module implements the test sequence for the @ref system subsystem.
 *
 * <h2>Objective</h2>
 * Objective of the test module is to cover 100% of the @ref system
 * subsystem code.
 *
 * <h2>Preconditions</h2>
 * None.
 *
 * <h2>Test Cases</h2>
 * - @subpage test_sys_001
 * - @subpage test_sys_002
 * - @subpage test_sys_003
 * .
 * @file testsys.c
 * @brief System test source file
 * @file testsys.h
 * @brief System header file
 */

/**
 * @page test_sys_001 Critical zones check
 *
 * <h2>Description</h2>
 * The critical zones API is invoked for coverage.
 */

static void vtcb(void *p) {
  syssts_t sts;

  (void)p;

  /* Testing normal case.*/
  chSysLockFromISR();
  chSysUnlockFromISR();

  /* Reentrant case.*/
  chSysLockFromISR();
  sts = chSysGetStatusAndLockX();
  chSysRestoreStatusX(sts);
  chSysUnlockFromISR();
}

static void sys1_execute(void) {
  syssts_t sts;
  virtual_timer_t vt;

  /* Testing normal case.*/
  sts = chSysGetStatusAndLockX();
  chSysRestoreStatusX(sts);

  /* Reentrant case.*/
  chSysLock();
  sts = chSysGetStatusAndLockX();
  chSysRestoreStatusX(sts);
  chSysUnlock();

  /* Unconditional lock.*/
  chSysUnconditionalLock();
  chSysUnconditionalLock();
  chSysUnlock();

  /* Unconditional unlock.*/
  chSysLock();
  chSysUnconditionalUnlock();
  chSysUnconditionalUnlock();

  /*/Testing from ISR context using a virtual timer.*/
  chVTObjectInit(&vt);
  chVTSet(&vt, 1, vtcb, NULL);
  chThdSleep(10);

  test_assert(1, chVTIsArmed(&vt) == false, "timer still armed");
}

ROMCONST struct testcase testsys1 = {
  "System, critical zones",
  NULL,
  NULL,
  sys1_execute
};

/**
 * @page test_sys_002 Interrupts handling
 *
 * <h2>Description</h2>
 * The interrupts handling API is invoked for coverage.
 */

static void sys2_execute(void) {

  chSysSuspend();
  chSysDisable();
  chSysSuspend();
  chSysEnable();
}

ROMCONST struct testcase testsys2 = {
  "System, interrupts handling",
  NULL,
  NULL,
  sys2_execute
};

/**
 * @page test_sys_003 System integrity check
 *
 * <h2>Description</h2>
 * The chSysIntegrityCheckI() API is invoked in order to asses the state of the
 * system data structures.
 */

static void sys3_execute(void) {
  bool result;

  chSysLock();
  result = chSysIntegrityCheckI(CH_INTEGRITY_RLIST);
  chSysUnlock();
  test_assert(1, result == false, "ready list check failed");

  chSysLock();
  result = chSysIntegrityCheckI(CH_INTEGRITY_VTLIST);
  chSysUnlock();
  test_assert(2, result == false, "virtual timers list check failed");

  chSysLock();
  result = chSysIntegrityCheckI(CH_INTEGRITY_REGISTRY);
  chSysUnlock();
  test_assert(3, result == false, "registry list check failed");

  chSysLock();
  result = chSysIntegrityCheckI(CH_INTEGRITY_PORT);
  chSysUnlock();
  test_assert(4, result == false, "port layer check failed");
}

ROMCONST struct testcase testsys3 = {
  "System, integrity",
  NULL,
  NULL,
  sys3_execute
};

/**
 * @brief   Test sequence for messages.
 */
ROMCONST struct testcase * ROMCONST patternsys[] = {
  &testsys1,
  &testsys2,
  &testsys3,
  NULL
};
