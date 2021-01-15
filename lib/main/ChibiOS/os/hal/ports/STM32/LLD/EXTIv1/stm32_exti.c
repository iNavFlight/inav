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
 * @file    EXTIv1/stm32_exti.c
 * @brief   EXTI helper driver code.
 *
 * @addtogroup STM32_EXTI
 * @details EXTI sharing helper driver.
 * @{
 */

#include "hal.h"

/* The following macro is only defined if some driver requiring EXTI services
   has been enabled.*/
#if defined(STM32_EXTI_REQUIRED) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   STM32 EXTI group 1 lines initialization.
 *
 * @param[in] mask      mask of group 1 lines to be initialized
 * @param[in] mode      initialization mode
 *
 * @api
 */
void extiEnableGroup1(uint32_t mask, extimode_t mode) {

  /* Masked out lines must not be touched by this driver.*/
  osalDbgAssert((mask & STM32_EXTI_IMR1_MASK) == 0U, "fixed lines");

  if ((mode & EXTI_MODE_EDGES_MASK) == 0U) {
    /* Disabling channels.*/
    EXTI->IMR1  &= ~mask;
    EXTI->EMR1  &= ~mask;
    EXTI->RTSR1 &= ~mask;
    EXTI->FTSR1 &= ~mask;
    EXTI->PR1    =  mask;
  }
  else {
    /* Programming edge registers.*/
    if (mode & EXTI_MODE_RISING_EDGE) {
      EXTI->RTSR1 |= mask;
    }
    else {
      EXTI->RTSR1 &= ~mask;
    }
    if (mode & EXTI_MODE_FALLING_EDGE) {
      EXTI->FTSR1 |= mask;
    }
    else {
      EXTI->FTSR1 &= ~mask;
    }

    /* Programming interrupt and event registers.*/
    if ((mode & EXTI_MODE_ACTION_MASK) == EXTI_MODE_ACTION_INTERRUPT) {
      EXTI->IMR1 |= mask;
      EXTI->EMR1 &= ~mask;
    }
    else {
      EXTI->EMR1 |= mask;
      EXTI->IMR1 &= ~mask;
    }
  }
}

#if (STM32_EXTI_NUM_LINES > 32) || defined(__DOXYGEN__)
/**
 * @brief   STM32 EXTI group 2 lines initialization.
 *
 * @param[in] mask      mask of group 2 lines to be initialized
 * @param[in] mode      initialization mode
 *
 * @api
 */
void extiEnableGroup2(uint32_t mask, extimode_t mode) {

  /* Masked out lines must not be touched by this driver.*/
  osalDbgAssert((mask & STM32_EXTI_IMR2_MASK) == 0U, "fixed lines");

  if ((mode & EXTI_MODE_EDGES_MASK) == 0U) {
    /* Disabling channels.*/
    EXTI->IMR2  &= ~mask;
    EXTI->EMR2  &= ~mask;
    EXTI->RTSR2 &= ~mask;
    EXTI->FTSR2 &= ~mask;
    EXTI->PR2    =  mask;
  }
  else {
    /* Programming edge registers.*/
    if (mode & EXTI_MODE_RISING_EDGE) {
      EXTI->RTSR2 |= mask;
    }
    else {
      EXTI->RTSR2 &= ~mask;
    }
    if (mode & EXTI_MODE_FALLING_EDGE) {
      EXTI->FTSR2 |= mask;
    }
    else {
      EXTI->FTSR2 &= ~mask;
    }

    /* Programming interrupt and event registers.*/
    if ((mode & EXTI_MODE_ACTION_MASK) == EXTI_MODE_ACTION_INTERRUPT) {
      EXTI->IMR2 |= mask;
      EXTI->EMR2 &= ~mask;
    }
    else {
      EXTI->EMR2 |= mask;
      EXTI->IMR2 &= ~mask;
    }
  }
}
#endif /* STM32_EXTI_NUM_LINES > 32 */

/**
 * @brief   STM32 EXTI line initialization.
 *
 * @param[in] line      line to be initialized
 * @param[in] mode      initialization mode
 *
 * @api
 */
void extiEnableLine(extiline_t line, extimode_t mode) {
  uint32_t mask = (1U << (line & 0x1FU));

  osalDbgCheck(line < STM32_EXTI_NUM_LINES);
  osalDbgCheck((mode & ~EXTI_MODE_MASK) == 0U);

#if STM32_EXTI_NUM_LINES > 32
  if (line < 32) {
#endif
    extiEnableGroup1(mask, mode);
#if STM32_EXTI_NUM_LINES > 32
  }
  else {
    extiEnableGroup2(mask, mode);
  }
#endif
}

/**
 * @brief   STM32 EXTI line IRQ status clearing.
 *
 * @param[in] line      line to be initialized
 *
 * @api
 */
void extiClearLine(extiline_t line) {
  uint32_t mask = (1U << (line & 0x1FU));

  osalDbgCheck(line < STM32_EXTI_NUM_LINES);

#if STM32_EXTI_NUM_LINES > 32
  if (line < 32) {
#endif
    extiClearGroup1(mask);
#if STM32_EXTI_NUM_LINES > 32
  }
  else {
    extiClearGroup2(mask);
  }
#endif
}

#endif /* STM32_EXTI_REQUIRED */

/** @} */
