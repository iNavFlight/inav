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
 * @file    STM32L0xx/stm32_isr.h
 * @brief   STM32L0xx ISR handler header.
 *
 * @addtogroup STM32L0xx_ISR
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
 * @brief   EXTI0..1 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI0_1_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI0_1_PRIORITY          3
#endif

/**
 * @brief   EXTI2..3 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI2_3_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI2_3_PRIORITY          3
#endif

/**
 * @brief   EXTI4..15 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI4_15_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI4_15_PRIORITY         3
#endif

/**
 * @brief   EXTI16 (PVD) interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI16_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI16_PRIORITY           3
#endif

/**
 * @brief   EXTI17,19,20 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI17_20_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI17_20_PRIORITY        3
#endif

/**
 * @brief   EXTI21,22 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI21_22_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI21_22_PRIORITY        3
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
