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
 * @file    test_root.h
 * @brief   Test Suite root structures header.
 *
 * @addtogroup CH_TEST_ROOT
 * @{
 */

#ifndef _TEST_ROOT_H_
#define _TEST_ROOT_H_

#include "nil.h"

#include "test_sequence_001.h"
#include "test_sequence_002.h"

/*===========================================================================*/
/* Default definitions.                                                      */
/*===========================================================================*/

/* Global test suite name, it is printed on top of the test
   report header.*/
#define TEST_SUITE_NAME                     "ChibiOS/NIL Test Suite"

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern const testcase_t * const *test_suite[];

#ifdef __cplusplus
extern "C" {
#endif
  extern semaphore_t gsem1, gsem2;
  extern thread_reference_t gtr1;
  extern THD_WORKING_AREA(wa_test_support, 128);
  THD_FUNCTION(test_support, arg);
#ifdef __cplusplus
}
#endif

/*===========================================================================*/
/* Shared definitions.                                                       */
/*===========================================================================*/

#endif /* _TEST_ROOT_H_ */

/** @} */
