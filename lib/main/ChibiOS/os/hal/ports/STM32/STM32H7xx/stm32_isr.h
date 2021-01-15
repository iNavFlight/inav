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
 * @file    STM32H7xx/stm32_isr.h
 * @brief   STM32H7xx ISR handler header.
 *
 * @addtogroup STM32H7xx_ISR
 * @{
 */

#ifndef STM32_ISR_H
#define STM32_ISR_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   EXTI0 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI0_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI0_PRIORITY            6
#endif

/**
 * @brief   EXTI1 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI1_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI1_PRIORITY            6
#endif

/**
 * @brief   EXTI2 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI2_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI2_PRIORITY            6
#endif

/**
 * @brief   EXTI3 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI3_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI3_PRIORITY            6
#endif

/**
 * @brief   EXTI4 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI4_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI4_PRIORITY            6
#endif

/**
 * @brief   EXTI9..5 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI5_9_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI5_9_PRIORITY          6
#endif

/**
 * @brief   EXTI15..10 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI10_15_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI10_15_PRIORITY        6
#endif

/**
 * @brief   EXTI16 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI16_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI16_PRIORITY           6
#endif

/**
 * @brief   EXTI17 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI17_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI17_PRIORITY           6
#endif

/**
 * @brief   EXTI18 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI18_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI18_PRIORITY           6
#endif

/**
 * @brief   EXTI19 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI19_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI19_PRIORITY           6
#endif

/**
 * @brief   EXTI20 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI20_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI20_PRIORITY           6
#endif

/**
 * @brief   EXTI21 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI21_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI21_PRIORITY           6
#endif

/**
 * @brief   EXTI22 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI22_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI22_PRIORITY           6
#endif

/**
 * @brief   EXTI23 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI23_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI23_PRIORITY           6
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void irqInit(void);
  void irqDeinit(void);
#ifdef __cplusplus
}
#endif

#endif /* STM32_ISR_H */

/** @} */
