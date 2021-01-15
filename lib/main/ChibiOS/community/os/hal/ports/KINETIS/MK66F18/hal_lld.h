/*
    ChibiOS - Copyright (C) 2014-2015 Fabio Utzig

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
 * @file    MK66F18/hal_lld.h
 * @brief   Kinetis MK66F18 HAL subsystem low level driver header.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef HAL_LLD_H_
#define HAL_LLD_H_

#include "kinetis_registry.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Defines the support for realtime counters in the HAL.
 */
#define HAL_IMPLEMENTS_COUNTERS FALSE

/**
 * @name    Platform identification
 * @{
 */
#define PLATFORM_NAME           "Kinetis"
/** @} */

/**
 * @name    Internal clock sources
 * @{
 */
#define KINETIS_IRCLK_F         4000000     /**< Fast internal reference clock, factory trimmed. */
#define KINETIS_IRCLK_S         32768       /**< Slow internal reference clock, factory trimmed. */
/** @} */

#define KINETIS_MCG_MODE_FEI    1    /**< FLL Engaged Internal. */
#define KINETIS_MCG_MODE_FEE    2    /**< FLL Engaged External. */
#define KINETIS_MCG_MODE_FBI    3    /**< FLL Bypassed Internal. */
#define KINETIS_MCG_MODE_FBE    4    /**< FLL Bypassed External. */
#define KINETIS_MCG_MODE_PEE    5    /**< PLL Engaged External. */
#define KINETIS_MCG_MODE_PBE    6    /**< PLL Bypassed External. */
#define KINETIS_MCG_MODE_BLPI   7    /**< Bypassed Low Power Internal. */
#define KINETIS_MCG_MODE_BLPE   8    /**< Bypassed Low Power External. */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   Disables the MCG/system clock initialization in the HAL.
 */
#if !defined(KINETIS_NO_INIT) || defined(__DOXYGEN__)
#define KINETIS_NO_INIT             FALSE
#endif

/**
 * @brief   MCG mode selection.
 */
#if !defined(KINETIS_MCG_MODE) || defined(__DOXYGEN__)
#define KINETIS_MCG_MODE            KINETIS_MCG_MODE_PEE
#endif

/**
 * @brief   MCU PLL clock frequency.
 */
#if !defined(KINETIS_PLLCLK_FREQUENCY) || defined(__DOXYGEN__)
#define KINETIS_PLLCLK_FREQUENCY    96000000UL
#endif

/**
 * @brief   Clock divider for core/system clocks (OUTDIV1).
 * @note    The allowed range is 1..16
 * @note    The default value is calculated for a 48 MHz system clock
 *          from a 96 MHz PLL output.
 */
#if !defined(KINETIS_CLKDIV1_OUTDIV1) || defined(__DOXYGEN__)
  #if defined(KINETIS_SYSCLK_FREQUENCY) && KINETIS_SYSCLK_FREQUENCY > 0
    #define KINETIS_CLKDIV1_OUTDIV1     (KINETIS_PLLCLK_FREQUENCY/KINETIS_SYSCLK_FREQUENCY)
  #else
    #define KINETIS_CLKDIV1_OUTDIV1     2
  #endif
#endif

/**
 * @brief   Clock divider for bus clock (OUTDIV2).
 * @note    The allowed range is 1..16
 * @note    The default value is calculated for a 48 MHz bus clock
 *          from a 96 MHz PLL output.
 */
#if !defined(KINETIS_CLKDIV1_OUTDIV2) || defined(__DOXYGEN__)
  #if defined(KINETIS_BUSCLK_FREQUENCY) && KINETIS_BUSCLK_FREQUENCY > 0
    #define KINETIS_CLKDIV1_OUTDIV2     (KINETIS_PLLCLK_FREQUENCY/KINETIS_BUSCLK_FREQUENCY)
  #elif defined(KINETIS_SYSCLK_FREQUENCY) && KINETIS_SYSCLK_FREQUENCY > 0
    #define KINETIS_CLKDIV1_OUTDIV2	KINETIS_CLKDIV1_OUTDIV1
  #else
    #define KINETIS_CLKDIV1_OUTDIV2     2
  #endif
#endif

/**
 * @brief   Clock divider for FlexBus clock (OUTDIV3).
 * @note    The allowed range is 1..16
 * @note    The default value is calculated for a 48 MHz clock
 *          from a 96 MHz PLL output.
 */
#if !defined(KINETIS_CLKDIV1_OUTDIV3) || defined(__DOXYGEN__)
  #if defined(KINETIS_FLEXBUSCLK_FREQUENCY) && KINETIS_FLEXBUSCLK_FREQUENCY > 0
    #define KINETIS_CLKDIV1_OUTDIV3     (KINETIS_PLLCLK_FREQUENCY/KINETIS_FLEXBUSCLK_FREQUENCY)
  #else
    /* If no FlexBus frequency provided, use bus speed divider */
    #define KINETIS_CLKDIV1_OUTDIV3     KINETIS_CLKDIV1_OUTDIV2
  #endif
#endif

/**
 * @brief   Clock divider for flash clock (OUTDIV4).
 * @note    The allowed range is 1..16
 * @note    The default value is calculated for a 24 MHz flash clock
 *          from a 96 MHz PLL output
 */
#if !defined(KINETIS_CLKDIV1_OUTDIV4) || defined(__DOXYGEN__)
  #if defined(KINETIS_FLASHCLK_FREQUENCY) && KINETIS_FLASHCLK_FREQUENCY > 0
    #define KINETIS_CLKDIV1_OUTDIV4     (KINETIS_PLLCLK_FREQUENCY/KINETIS_FLASHCLK_FREQUENCY)
  #elif defined(KINETIS_SYSCLK_FREQUENCY) && KINETIS_SYSCLK_FREQUENCY > 0
    #define KINETIS_CLKDIV1_OUTDIV4	(KINETIS_CLKDIV1_OUTDIV1*2)
  #else
    #define KINETIS_CLKDIV1_OUTDIV4     4
  #endif
#endif

/**
 * @brief   FLL DCO tuning enable for 32.768 kHz reference.
 * @note    Set to 1 for fine-tuning DCO for maximum frequency with
 *          a 32.768 kHz reference.
 * @note    The default value is for a 32.768 kHz external crystal.
 */
#if !defined(KINETIS_MCG_FLL_DMX32) || defined(__DOXYGEN__)
#define KINETIS_MCG_FLL_DMX32       1
#endif

/**
 * @brief   FLL DCO range selection.
 * @note    The allowed range is 0...3.
 * @note    The default value is calculated for 48 MHz FLL output
 *          from a 32.768 kHz external crystal.
 *          (DMX32 && DRST_DRS=1 => F=1464; 32.768 kHz * F ~= 48 MHz.)
 *
 */
#if !defined(KINETIS_MCG_FLL_DRS) || defined(__DOXYGEN__)
#define KINETIS_MCG_FLL_DRS         2
#endif

/**
 * @brief   MCU system/core clock frequency.
 */
#if !defined(KINETIS_SYSCLK_FREQUENCY) || defined(__DOXYGEN__)
#define KINETIS_SYSCLK_FREQUENCY    (KINETIS_PLLCLK_FREQUENCY / KINETIS_CLKDIV1_OUTDIV1)
#endif

/**
 * @brief   MCU bus clock frequency.
 */
#if !defined(KINETIS_BUSCLK_FREQUENCY) || defined(__DOXYGEN__)
#define KINETIS_BUSCLK_FREQUENCY    (KINETIS_PLLCLK_FREQUENCY / KINETIS_CLKDIV1_OUTDIV2)
#endif

/**
 * @brief   MCU flash clock frequency.
 */
#if !defined(KINETIS_FLASHCLK_FREQUENCY) || defined(__DOXYGEN__)
#define KINETIS_FLASHCLK_FREQUENCY    (KINETIS_PLLCLK_FREQUENCY / KINETIS_CLKDIV1_OUTDIV4)
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !defined(KINETIS_SYSCLK_FREQUENCY)
  #error KINETIS_SYSCLK_FREQUENCY must be defined
#endif

#if KINETIS_SYSCLK_FREQUENCY <= 0 || KINETIS_SYSCLK_FREQUENCY > KINETIS_SYSCLK_MAX
  #error KINETIS_SYSCLK_FREQUENCY out of range
#endif

#if !defined(KINETIS_BUSCLK_FREQUENCY)
  #error KINETIS_BUSCLK_FREQUENCY must be defined
#endif

#if KINETIS_BUSCLK_FREQUENCY <= 0 || KINETIS_BUSCLK_FREQUENCY > KINETIS_BUSCLK_MAX
  #error KINETIS_BUSCLK_FREQUENCY out of range
#endif

#if KINETIS_BUSCLK_FREQUENCY > KINETIS_SYSCLK_FREQUENCY
  #error KINETIS_BUSCLK_FREQUENCY must be an integer divide of\
	 KINETIS_SYSCLK_FREQUENCY
#endif

#if !defined(KINETIS_FLASHCLK_FREQUENCY)
  #error KINETIS_FLASHCLK_FREQUENCY must be defined
#endif

#if KINETIS_FLASHCLK_FREQUENCY <= 0 || KINETIS_FLASHCLK_FREQUENCY > KINETIS_FLASHCLK_MAX
  #error KINETIS_FLASHCLK_FREQUENCY out of range
#endif

#if KINETIS_FLASHCLK_FREQUENCY > KINETIS_SYSCLK_FREQUENCY
  #error KINETIS_FLASHCLK_FREQUENCY must be an integer divide of\
	 KINETIS_SYSCLK_FREQUENCY
#endif

#if !(defined(KINETIS_CLKDIV1_OUTDIV1) && \
      KINETIS_CLKDIV1_OUTDIV1 >= 1 && KINETIS_CLKDIV1_OUTDIV1 <= 16)
  #error KINETIS_CLKDIV1_OUTDIV1 must be 1 through 16
#endif

#if !(defined(KINETIS_CLKDIV1_OUTDIV2) && \
      KINETIS_CLKDIV1_OUTDIV2 >= 1 && KINETIS_CLKDIV1_OUTDIV2 <= 16)
#error KINETIS_CLKDIV1_OUTDIV2 must be 1 through 16
#endif

#if !(defined(KINETIS_CLKDIV1_OUTDIV3) && \
      KINETIS_CLKDIV1_OUTDIV3 >= 1 && KINETIS_CLKDIV1_OUTDIV3 <= 16)
#error KINETIS_CLKDIV1_OUTDIV3 must be 1 through 16
#endif

#if !(defined(KINETIS_CLKDIV1_OUTDIV4) && \
      KINETIS_CLKDIV1_OUTDIV4 >= 1 && KINETIS_CLKDIV1_OUTDIV4 <= 16)
#error KINETIS_CLKDIV1_OUTDIV4 must be 1 through 16
#endif

#if !(KINETIS_MCG_FLL_DMX32 == 0 || KINETIS_MCG_FLL_DMX32 == 1)
#error Invalid KINETIS_MCG_FLL_DMX32 value, must be 0 or 1
#endif

#if !(0 <= KINETIS_MCG_FLL_DRS && KINETIS_MCG_FLL_DRS <= 3)
#error Invalid KINETIS_MCG_FLL_DRS value, must be 0...3
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type representing a system clock frequency.
 */
typedef uint32_t halclock_t;

/**
 * @brief   Type of the realtime free counter value.
 */
typedef uint32_t halrtcnt_t;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Returns the current value of the system free running counter.
 * @note    This service is implemented by returning the content of the
 *          DWT_CYCCNT register.
 *
 * @return              The value of the system free running counter of
 *                      type halrtcnt_t.
 *
 * @notapi
 */
#define hal_lld_get_counter_value()         0

/**
 * @brief   Realtime counter frequency.
 * @note    The DWT_CYCCNT register is incremented directly by the system
 *          clock so this function returns STM32_HCLK.
 *
 * @return              The realtime counter frequency of type halclock_t.
 *
 * @notapi
 */
#define hal_lld_get_counter_frequency()     0

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#include "nvic.h"

#ifdef __cplusplus
extern "C" {
#endif
  void hal_lld_init(void);
  void MK66F18_clock_init(void);
#ifdef __cplusplus
}
#endif

#endif /* HAL_LLD_H_ */

/** @} */
