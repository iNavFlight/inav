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
 * @file    STM32L4xx/stm32_isr.h
 * @brief   STM32L4xx ISR handler header.
 *
 * @addtogroup SRM32L4xx_ISR
 * @{
 */

#ifndef STM32_ISR_H
#define STM32_ISR_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    ISRs suppressed in standard drivers
 * @{
 */
#define STM32_TIM1_SUPPRESS_ISR
#define STM32_TIM15_SUPPRESS_ISR
#define STM32_TIM16_SUPPRESS_ISR
#define STM32_TIM17_SUPPRESS_ISR
/** @} */

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
 * @brief   EXTI16-EXTI35..38 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI1635_38_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI1635_38_IRQ_PRIORIT   6
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
 * @brief   EXTI21..22 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_EXTI21_22_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_EXTI21_22_PRIORITY        6
#endif

/**
 * @brief   TIM1-BRK, TIM15 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_TIM1_BRK_TIM15_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_TIM1_BRK_TIM15_PRIORITY   7
#endif

/**
 * @brief   TIM1-UP, TIM16 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_TIM1_UP_TIM16_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_TIM1_UP_TIM16_PRIORITY    7
#endif

/**
 * @brief   TIM1-TRG-COM, TIM17 interrupt priority level setting.
 */
#if !defined(STM32_IRQ_TIM1_TRGCO_TIM17_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_TIM1_TRGCO_TIM17_PRIORITY 7
#endif

/**
 * @brief   TIM1-CC interrupt priority level setting.
 */
#if !defined(STM32_IRQ_TIM1_CC_PRIORITY) || defined(__DOXYGEN__)
#define STM32_IRQ_TIM1_CC_PRIORITY          7
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/* IRQ priority checks.*/
#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_EXTI0_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_EXTI0_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_EXTI1_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_EXTI1_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_EXTI2_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_EXTI2_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_EXTI3_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_EXTI3_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_EXTI4_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_EXTI4_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_EXTI5_9_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_EXTI5_9_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_EXTI10_15_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_EXTI10_15_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_EXTI1635_38_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_EXTI1635_38_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_EXTI18_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_EXTI18_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_EXTI19_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_EXTI19_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_EXTI20_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_EXTI20_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_EXTI21_22_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_EXTI21_22_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_TIM1_BRK_TIM15_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_TIM1_BRK_TIM15_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_TIM1_UP_TIM16_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_TIM1_UP_TIM16_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_TIM1_TRGCO_TIM17_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_TIM1_TRGCO_TIM17_PRIORITY"
#endif

#if !OSAL_IRQ_IS_VALID_PRIORITY(STM32_IRQ_TIM1_CC_PRIORITY)
#error "Invalid IRQ priority assigned to STM32_IRQ_TIM1_CC_PRIORITY"
#endif

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
