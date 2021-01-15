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
 * @file    EXTIv1/stm32_exti.h
 * @brief   EXTI helper driver header.
 *
 * @addtogroup STM32_EXTI
 * @{
 */

#ifndef STM32_EXTI_H
#define STM32_EXTI_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    EXTI channel modes
 * @{
 */
#define EXTI_MODE_MASK              7U  /**< @brief Mode parameter mask.    */
#define EXTI_MODE_EDGES_MASK        3U  /**< @brief Edges field mask.       */
#define EXTI_MODE_DISABLED          0U  /**< @brief Channel disabled.       */
#define EXTI_MODE_RISING_EDGE       1U  /**< @brief Rising edge callback.   */
#define EXTI_MODE_FALLING_EDGE      2U  /**< @brief Falling edge callback.  */
#define EXTI_MODE_BOTH_EDGES        3U  /**< @brief Both edges callback.    */
#define EXTI_MODE_ACTION_MASK       4U  /**< @brief Action field mask.      */
#define EXTI_MODE_ACTION_INTERRUPT  0U  /**< @brief Interrupt mode.         */
#define EXTI_MODE_ACTION_EVENT      4U  /**< @brief Event mode.             */
/** @} */

/* Handling differences in ST headers.*/
#if !defined(STM32H7XX) && !defined(STM32L4XX) && !defined(STM32L4XXP)
#define EMR1    EMR
#define IMR1    IMR
#define PR1     PR
#define RTSR1   RTSR
#define FTSR1   FTSR
#endif

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !defined(STM32_EXTI_NUM_LINES)
#error "STM32_EXTI_NUM_LINES not defined in registry"
#endif

#if (STM32_EXTI_NUM_LINES < 0) || (STM32_EXTI_NUM_LINES > 63)
#error "invalid STM32_EXTI_NUM_LINES value"
#endif

#if !defined(STM32_EXTI_IMR1_MASK)
#error "STM32_EXTI_IMR1_MASK not defined in registry"
#endif

#if STM32_EXTI_NUM_LINES > 32
#if !defined(STM32_EXTI_IMR2_MASK)
#error "STM32_EXTI_IMR2_MASK not defined in registry"
#endif
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of an EXTI line identifier.
 */
typedef uint32_t extiline_t;

/**
 * @brief   Type of an EXTI line mode.
 */
typedef uint32_t extimode_t;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   From group 1 line number to mask.
 *
 * @param[in] line      line number in range 0..31
 */
#define EXTI_MASK1(line) (uint32_t)(1U << (line))

/**
 * @brief   From group 2 line number to mask.
 *
 * @param[in] line      line number in range 32..63
 */
#define EXTI_MASK2(line) (uint32_t)(1U << ((line) - 32U))

/**
 * @brief   STM32 EXTI group 1 IRQ status clearing.
 *
 * @param[in] mask      mask of group 1 lines to be initialized
 *
 * @api
 */
#define extiClearGroup1(mask) do {                                          \
  osalDbgAssert(((mask) & STM32_EXTI_IMR1_MASK) == 0U, "fixed lines");      \
  EXTI->PR1 = (uint32_t)(mask);                                             \
} while (false)

#if (STM32_EXTI_NUM_LINES > 32) || defined(__DOXYGEN__)
/**
 * @brief   STM32 EXTI group 2 IRQ status clearing.
 *
 * @param[in] mask      mask of group 2 lines to be initialized
 *
 * @api
 */
#define extiClearGroup2(mask) do {                                          \
  osalDbgAssert(((mask) & STM32_EXTI_IMR2_MASK) == 0U, "fixed lines");      \
  EXTI->PR2 = (uint32_t)(mask);                                             \
} while (false)
#endif /* STM32_EXTI_NUM_LINES > 32 */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void extiEnableGroup1(uint32_t mask, extimode_t mode);
#if (STM32_EXTI_NUM_LINES > 32) || defined(__DOXYGEN__)
  void extiEnableGroup2(uint32_t mask, extimode_t mode);
#endif /* STM32_EXTI_NUM_LINES > 32 */
  void extiEnableLine(extiline_t line, extimode_t mode);
  void extiClearLine(extiline_t line);
  #ifdef __cplusplus
}
#endif

#endif /* STM32_EXTI_H */

/** @} */
