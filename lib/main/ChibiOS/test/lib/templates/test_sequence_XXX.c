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
 * @page test_sequence_XXX Sequence brief description
 *
 * File: @ref test_sequence_XXX.c
 *
 * <h2>Description</h2>
 * Sequence detailed description.
 *
 * <h2>Test Cases</h2>
 * - @subpage test_XXX_001
 * .
 */

/****************************************************************************
 * Shared code.
 ****************************************************************************/


/****************************************************************************
 * Test cases.
 ****************************************************************************/

#if TEST_XXX_000_CONDITION || defined(__DOXYGEN__)
/**
 * @page test_XXX_001 Brief description
 *
 * <h2>Description</h2>
 * Detailed description.
 *
 * <h2>Conditions</h2>
 * This test is only executed if the following preprocessor condition
 * evaluates to true:
 * - TEST_XXX_001_CONDITION
 * .
 *
 * <h2>Test Steps</h2>
 * - Step description.
 * .
 */

static void test_XXX_001_setup(void) {

}

static void test_XXX_001_teardown(void) {

}

static void test_XXX_001_execute(void) {

  /* Step description.*/
  test_set_step(1);
  {
  }
}

static const testcase_t test_XXX_001 = {
  "Brief description",
  test_XXX_001_setup,
  test_XXX_001_teardown,
  test_XXX_001_execute
};
#endif /* TEST_XXX_001_CONDITION */

 /****************************************************************************
 * Exported data.
 ****************************************************************************/

/**
 * @brief   Sequence brief description.
 */
const testcase_t * const test_sequence_XXX[] = {
#if TEST_XXX_001_CONDITION || defined(__DOXYGEN__)
  &test_XXX_001,
#endif
  NULL
};
