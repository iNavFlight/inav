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
 * @file    STM32F4xx/hal_lld.h
 * @brief   STM32F4xx/STM32F2xx HAL subsystem low level driver header.
 * @pre     This module requires the following macros to be defined in the
 *          @p board.h file:
 *          - STM32_LSECLK.
 *          - STM32_LSE_BYPASS (optionally).
 *          - STM32_HSECLK.
 *          - STM32_HSE_BYPASS (optionally).
 *          - STM32_VDD (as hundredths of Volt).
 *          .
 *          One of the following macros must also be defined:
 *          - STM32F2XX for High-performance STM32 F-2 devices.
 *          - STM32F405xx, STM32F415xx, STM32F407xx, STM32F417xx for
 *            High-performance STM32 F-4 devices.
 *          - STM32F427xx, STM32F437xx, STM32F429xx, STM32F439xx for
 *            High-performance STM32 F-4 devices.
 *          - STM32F401xC, STM32F401xE for High-performance STM32 F-4 devices.
 *          - STM32F411xE for High-performance STM32 F-4 devices.
 *          - STM32F446xx for High-performance STM32 F-4 devices.
 *          .
 *
 * @addtogroup HAL
 * @{
 */

#ifndef _HAL_LLD_H_
#define _HAL_LLD_H_

#include "stm32_registry.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Defines the support for realtime counters in the HAL.
 */
#define HAL_IMPLEMENTS_COUNTERS TRUE

/**
 * @name    Platform identification macros
 * @{
 */
#if defined(STM32F439xx) || defined(__DOXYGEN__)
#define PLATFORM_NAME           "STM32F439 High Performance with DSP and FPU"

#elif defined(STM32F446xx)
#define PLATFORM_NAME           "STM32F446 High Performance with DSP and FPU"

#elif defined(STM32F429xx)
#define PLATFORM_NAME           "STM32F429 High Performance with DSP and FPU"

#elif defined(STM32F437xx)
#define PLATFORM_NAME           "STM32F437 High Performance with DSP and FPU"

#elif defined(STM32F427xx)
#define PLATFORM_NAME           "STM32F427 High Performance with DSP and FPU"

#elif defined(STM32F405xx)
#define PLATFORM_NAME           "STM32F405 High Performance with DSP and FPU"

#elif defined(STM32F415xx)
#define PLATFORM_NAME           "STM32F415 High Performance with DSP and FPU"

#elif defined(STM32F407xx)
#define PLATFORM_NAME           "STM32F407 High Performance with DSP and FPU"

#elif defined(STM32F417xx)
#define PLATFORM_NAME           "STM32F417 High Performance with DSP and FPU"

#elif defined(STM32F401xC)
#define PLATFORM_NAME           "STM32F401xC High Performance with DSP and FPU"

#elif defined(STM32F401xE)
#define PLATFORM_NAME           "STM32F401xE High Performance with DSP and FPU"

#elif defined(STM32F411xE)
#define PLATFORM_NAME           "STM32F411xE High Performance with DSP and FPU"

#elif defined(STM32F205xx)
#define PLATFORM_NAME           "STM32F405 High Performance"

#elif defined(STM32F215xx)
#define PLATFORM_NAME           "STM32F415 High Performance"

#elif defined(STM32F207xx)
#define PLATFORM_NAME           "STM32F407 High Performance"

#elif defined(STM32F217xx)
#define PLATFORM_NAME           "STM32F417 High Performance"

#else
#error "STM32F2xx/F4xx device not specified"
#endif
/** @} */

/**
 * @name    Absolute Maximum Ratings
 * @{
 */
#if defined(STM32F427xx) || defined(STM32F437xx) ||                         \
    defined(STM32F429xx) || defined(STM32F439xx) ||                         \
    defined(STM32F446xx) || defined(__DOXYGEN__)
/**
 * @brief   Absolute maximum system clock.
 */
#define STM32_SYSCLK_MAX        180000000

/**
 * @brief   Maximum HSE clock frequency.
 */
#define STM32_HSECLK_MAX        26000000

/**
 * @brief   Maximum HSE clock frequency using an external source.
 */
#define STM32_HSECLK_BYP_MAX    50000000

/**
 * @brief   Minimum HSE clock frequency.
 */
#define STM32_HSECLK_MIN        4000000

/**
 * @brief   Minimum HSE clock frequency.
 */
#define STM32_HSECLK_BYP_MIN    1000000

/**
 * @brief   Maximum LSE clock frequency.
 */
#define STM32_LSECLK_MAX        32768

/**
 * @brief   Maximum LSE clock frequency.
 */
#define STM32_LSECLK_BYP_MAX    1000000

/**
 * @brief   Minimum LSE clock frequency.
 */
#define STM32_LSECLK_MIN        32768

/**
 * @brief   Maximum PLLs input clock frequency.
 */
#define STM32_PLLIN_MAX         2100000

/**
 * @brief   Minimum PLLs input clock frequency.
 */
#define STM32_PLLIN_MIN         950000

/**
 * @brief   Maximum PLLs VCO clock frequency.
 */
#define STM32_PLLVCO_MAX        432000000

/**
 * @brief   Minimum PLLs VCO clock frequency.
 */
#define STM32_PLLVCO_MIN        192000000

/**
 * @brief   Maximum PLL output clock frequency.
 */
#define STM32_PLLOUT_MAX        180000000

/**
 * @brief   Minimum PLL output clock frequency.
 */
#define STM32_PLLOUT_MIN        24000000

/**
 * @brief   Maximum APB1 clock frequency.
 */
#define STM32_PCLK1_MAX         (STM32_PLLOUT_MAX /4)

/**
 * @brief   Maximum APB2 clock frequency.
 */
#define STM32_PCLK2_MAX         (STM32_PLLOUT_MAX / 2)

/**
 * @brief   Maximum SPI/I2S clock frequency.
 */
#define STM32_SPII2S_MAX        45000000
#endif

#if defined(STM32F40_41xxx) || defined(__DOXYGEN__)
#define STM32_SYSCLK_MAX        168000000
#define STM32_HSECLK_MAX        26000000
#define STM32_HSECLK_BYP_MAX    50000000
#define STM32_HSECLK_MIN        4000000
#define STM32_HSECLK_BYP_MIN    1000000
#define STM32_LSECLK_MAX        32768
#define STM32_LSECLK_BYP_MAX    1000000
#define STM32_LSECLK_MIN        32768
#define STM32_PLLIN_MAX         2100000
#define STM32_PLLIN_MIN         950000
#define STM32_PLLVCO_MAX        432000000
#define STM32_PLLVCO_MIN        192000000
#define STM32_PLLOUT_MAX        168000000
#define STM32_PLLOUT_MIN        24000000
#define STM32_PCLK1_MAX         42000000
#define STM32_PCLK2_MAX         84000000
#define STM32_SPII2S_MAX        42000000
#endif

#if defined(STM32F401xx) || defined(__DOXYGEN__)
#define STM32_SYSCLK_MAX        84000000
#define STM32_HSECLK_MAX        26000000
#define STM32_HSECLK_BYP_MAX    50000000
#define STM32_HSECLK_MIN        4000000
#define STM32_HSECLK_BYP_MIN    1000000
#define STM32_LSECLK_MAX        32768
#define STM32_LSECLK_BYP_MAX    1000000
#define STM32_LSECLK_MIN        32768
#define STM32_PLLIN_MAX         2100000
#define STM32_PLLIN_MIN         950000
#define STM32_PLLVCO_MAX        432000000
#define STM32_PLLVCO_MIN        192000000
#define STM32_PLLOUT_MAX        84000000
#define STM32_PLLOUT_MIN        24000000
#define STM32_PCLK1_MAX         42000000
#define STM32_PCLK2_MAX         84000000
#define STM32_SPII2S_MAX        42000000
#endif

#if defined(STM32F411xx)
#define STM32_SYSCLK_MAX        100000000
#define STM32_HSECLK_MAX        26000000
#define STM32_HSECLK_BYP_MAX    50000000
#define STM32_HSECLK_MIN        4000000
#define STM32_HSECLK_BYP_MIN    1000000
#define STM32_LSECLK_MAX        32768
#define STM32_LSECLK_BYP_MAX    1000000
#define STM32_LSECLK_MIN        32768
#define STM32_PLLIN_MAX         2100000
#define STM32_PLLIN_MIN         950000
#define STM32_PLLVCO_MAX        432000000
#define STM32_PLLVCO_MIN        100000000
#define STM32_PLLOUT_MAX        100000000
#define STM32_PLLOUT_MIN        24000000
#define STM32_PCLK1_MAX         50000000
#define STM32_PCLK2_MAX         100000000
#define STM32_SPII2S_MAX        50000000
#endif

#if defined(STM32F2XX)
#define STM32_SYSCLK_MAX        120000000
#define STM32_HSECLK_MAX        26000000
#define STM32_HSECLK_BYP_MAX    26000000
#define STM32_HSECLK_MIN        1000000
#define STM32_HSECLK_BYP_MIN    1000000
#define STM32_LSECLK_MAX        32768
#define STM32_LSECLK_BYP_MAX    1000000
#define STM32_LSECLK_MIN        32768
#define STM32_PLLIN_MAX         2000000
#define STM32_PLLIN_MIN         950000
#define STM32_PLLVCO_MAX        432000000
#define STM32_PLLVCO_MIN        192000000
#define STM32_PLLOUT_MAX        120000000
#define STM32_PLLOUT_MIN        24000000
#define STM32_PCLK1_MAX         30000000
#define STM32_PCLK2_MAX         60000000
#define STM32_SPII2S_MAX        30000000
#endif
/** @} */

/**
 * @name    Internal clock sources
 * @{
 */
#define STM32_HSICLK            16000000    /**< High speed internal clock. */
#define STM32_LSICLK            32000       /**< Low speed internal clock.  */
/** @} */

/**
 * @name    PWR_CR register bits definitions
 * @{
 */
#define STM32_VOS_SCALE3        0x00004000
#define STM32_VOS_SCALE2        0x00008000
#define STM32_VOS_SCALE1        0x0000C000
#define STM32_PLS_MASK          (7 << 5)    /**< PLS bits mask.             */
#define STM32_PLS_LEV0          (0 << 5)    /**< PVD level 0.               */
#define STM32_PLS_LEV1          (1 << 5)    /**< PVD level 1.               */
#define STM32_PLS_LEV2          (2 << 5)    /**< PVD level 2.               */
#define STM32_PLS_LEV3          (3 << 5)    /**< PVD level 3.               */
#define STM32_PLS_LEV4          (4 << 5)    /**< PVD level 4.               */
#define STM32_PLS_LEV5          (5 << 5)    /**< PVD level 5.               */
#define STM32_PLS_LEV6          (6 << 5)    /**< PVD level 6.               */
#define STM32_PLS_LEV7          (7 << 5)    /**< PVD level 7.               */
/** @} */

/**
 * @name    RCC_PLLCFGR register bits definitions
 * @{
 */
#define STM32_PLLP_MASK         (3 << 16)   /**< PLLP mask.                 */
#define STM32_PLLP_DIV2         (0 << 16)   /**< PLL clock divided by 2.    */
#define STM32_PLLP_DIV4         (1 << 16)   /**< PLL clock divided by 4.    */
#define STM32_PLLP_DIV6         (2 << 16)   /**< PLL clock divided by 6.    */
#define STM32_PLLP_DIV8         (3 << 16)   /**< PLL clock divided by 8.    */

#define STM32_PLLSRC_HSI        (0 << 22)   /**< PLL clock source is HSI.   */
#define STM32_PLLSRC_HSE        (1 << 22)   /**< PLL clock source is HSE.   */
/** @} */

/**
 * @name    RCC_CFGR register bits definitions
 * @{
 */
#define STM32_SW_MASK           (3 << 0)    /**< SW mask.                   */
#define STM32_SW_HSI            (0 << 0)    /**< SYSCLK source is HSI.      */
#define STM32_SW_HSE            (1 << 0)    /**< SYSCLK source is HSE.      */
#define STM32_SW_PLL            (2 << 0)    /**< SYSCLK source is PLL.      */

#define STM32_HPRE_MASK         (15 << 4)   /**< HPRE mask.                 */
#define STM32_HPRE_DIV1         (0 << 4)    /**< SYSCLK divided by 1.       */
#define STM32_HPRE_DIV2         (8 << 4)    /**< SYSCLK divided by 2.       */
#define STM32_HPRE_DIV4         (9 << 4)    /**< SYSCLK divided by 4.       */
#define STM32_HPRE_DIV8         (10 << 4)   /**< SYSCLK divided by 8.       */
#define STM32_HPRE_DIV16        (11 << 4)   /**< SYSCLK divided by 16.      */
#define STM32_HPRE_DIV64        (12 << 4)   /**< SYSCLK divided by 64.      */
#define STM32_HPRE_DIV128       (13 << 4)   /**< SYSCLK divided by 128.     */
#define STM32_HPRE_DIV256       (14 << 4)   /**< SYSCLK divided by 256.     */
#define STM32_HPRE_DIV512       (15 << 4)   /**< SYSCLK divided by 512.     */

#define STM32_PPRE1_MASK        (7 << 10)   /**< PPRE1 mask.                */
#define STM32_PPRE1_DIV1        (0 << 10)   /**< HCLK divided by 1.         */
#define STM32_PPRE1_DIV2        (4 << 10)   /**< HCLK divided by 2.         */
#define STM32_PPRE1_DIV4        (5 << 10)   /**< HCLK divided by 4.         */
#define STM32_PPRE1_DIV8        (6 << 10)   /**< HCLK divided by 8.         */
#define STM32_PPRE1_DIV16       (7 << 10)   /**< HCLK divided by 16.        */

#define STM32_PPRE2_MASK        (7 << 13)   /**< PPRE2 mask.                */
#define STM32_PPRE2_DIV1        (0 << 13)   /**< HCLK divided by 1.         */
#define STM32_PPRE2_DIV2        (4 << 13)   /**< HCLK divided by 2.         */
#define STM32_PPRE2_DIV4        (5 << 13)   /**< HCLK divided by 4.         */
#define STM32_PPRE2_DIV8        (6 << 13)   /**< HCLK divided by 8.         */
#define STM32_PPRE2_DIV16       (7 << 13)   /**< HCLK divided by 16.        */

#define STM32_RTCPRE_MASK       (31 << 16)  /**< RTCPRE mask.               */

#define STM32_MCO1SEL_MASK      (3 << 21)   /**< MCO1 mask.                 */
#define STM32_MCO1SEL_HSI       (0 << 21)   /**< HSI clock on MCO1 pin.     */
#define STM32_MCO1SEL_LSE       (1 << 21)   /**< LSE clock on MCO1 pin.     */
#define STM32_MCO1SEL_HSE       (2 << 21)   /**< HSE clock on MCO1 pin.     */
#define STM32_MCO1SEL_PLL       (3 << 21)   /**< PLL clock on MCO1 pin.     */

#define STM32_I2SSRC_MASK       (1 << 23)   /**< I2CSRC mask.               */
#define STM32_I2SSRC_PLLI2S     (0 << 23)   /**< I2SSRC is PLLI2S.          */
#define STM32_I2SSRC_CKIN       (1 << 23)   /**< I2S_CKIN is PLLI2S.        */

#define STM32_MCO1PRE_MASK      (7 << 24)   /**< MCO1PRE mask.              */
#define STM32_MCO1PRE_DIV1      (0 << 24)   /**< MCO1 divided by 1.         */
#define STM32_MCO1PRE_DIV2      (4 << 24)   /**< MCO1 divided by 2.         */
#define STM32_MCO1PRE_DIV3      (5 << 24)   /**< MCO1 divided by 3.         */
#define STM32_MCO1PRE_DIV4      (6 << 24)   /**< MCO1 divided by 4.         */
#define STM32_MCO1PRE_DIV5      (7 << 24)   /**< MCO1 divided by 5.         */

#define STM32_MCO2PRE_MASK      (7 << 27)   /**< MCO2PRE mask.              */
#define STM32_MCO2PRE_DIV1      (0 << 27)   /**< MCO2 divided by 1.         */
#define STM32_MCO2PRE_DIV2      (4 << 27)   /**< MCO2 divided by 2.         */
#define STM32_MCO2PRE_DIV3      (5 << 27)   /**< MCO2 divided by 3.         */
#define STM32_MCO2PRE_DIV4      (6 << 27)   /**< MCO2 divided by 4.         */
#define STM32_MCO2PRE_DIV5      (7 << 27)   /**< MCO2 divided by 5.         */

#define STM32_MCO2SEL_MASK      (3 << 30)   /**< MCO2 mask.                 */
#define STM32_MCO2SEL_SYSCLK    (0 << 30)   /**< SYSCLK clock on MCO2 pin.  */
#define STM32_MCO2SEL_PLLI2S    (1 << 30)   /**< PLLI2S clock on MCO2 pin.  */
#define STM32_MCO2SEL_HSE       (2 << 30)   /**< HSE clock on MCO2 pin.     */
#define STM32_MCO2SEL_PLL       (3 << 30)   /**< PLL clock on MCO2 pin.     */

/**
 * @name    RCC_PLLI2SCFGR register bits definitions
 * @{
 */
#define STM32_PLLI2SN_MASK      (511 << 6)  /**< PLLI2SN mask.              */
#define STM32_PLLI2SR_MASK      (7 << 28)   /**< PLLI2SR mask.              */
/** @} */

/**
 * @name    RCC_BDCR register bits definitions
 * @{
 */
#define STM32_RTCSEL_MASK       (3 << 8)    /**< RTC source mask.           */
#define STM32_RTCSEL_NOCLOCK    (0 << 8)    /**< No RTC source.             */
#define STM32_RTCSEL_LSE        (1 << 8)    /**< RTC source is LSE.         */
#define STM32_RTCSEL_LSI        (2 << 8)    /**< RTC source is LSI.         */
#define STM32_RTCSEL_HSEDIV     (3 << 8)    /**< RTC source is HSE divided. */
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   Disables the PWR/RCC initialization in the HAL.
 */
#if !defined(STM32_NO_INIT) || defined(__DOXYGEN__)
#define STM32_NO_INIT               FALSE
#endif

/**
 * @brief   Enables or disables the programmable voltage detector.
 */
#if !defined(STM32_PVD_ENABLE) || defined(__DOXYGEN__)
#define STM32_PVD_ENABLE            FALSE
#endif

/**
 * @brief   Sets voltage level for programmable voltage detector.
 */
#if !defined(STM32_PLS) || defined(__DOXYGEN__)
#define STM32_PLS                   STM32_PLS_LEV0
#endif

/**
 * @brief   Enables the backup RAM regulator.
 */
#if !defined(STM32_BKPRAM_ENABLE) || defined(__DOXYGEN__)
#define STM32_BKPRAM_ENABLE         FALSE
#endif

/**
 * @brief   Enables or disables the HSI clock source.
 */
#if !defined(STM32_HSI_ENABLED) || defined(__DOXYGEN__)
#define STM32_HSI_ENABLED           TRUE
#endif

/**
 * @brief   Enables or disables the LSI clock source.
 */
#if !defined(STM32_LSI_ENABLED) || defined(__DOXYGEN__)
#define STM32_LSI_ENABLED           FALSE
#endif

/**
 * @brief   Enables or disables the HSE clock source.
 */
#if !defined(STM32_HSE_ENABLED) || defined(__DOXYGEN__)
#define STM32_HSE_ENABLED           TRUE
#endif

/**
 * @brief   Enables or disables the LSE clock source.
 */
#if !defined(STM32_LSE_ENABLED) || defined(__DOXYGEN__)
#define STM32_LSE_ENABLED           FALSE
#endif

/**
 * @brief   USB/SDIO clock setting.
 */
#if !defined(STM32_CLOCK48_REQUIRED) || defined(__DOXYGEN__)
#define STM32_CLOCK48_REQUIRED      TRUE
#endif

/**
 * @brief   Main clock source selection.
 * @note    If the selected clock source is not the PLL then the PLL is not
 *          initialized and started.
 * @note    The default value is calculated for a 168MHz system clock from
 *          an external 8MHz HSE clock.
 */
#if !defined(STM32_SW) || defined(__DOXYGEN__)
#define STM32_SW                    STM32_SW_PLL
#endif

#if defined(STM32F4XX) || defined(__DOXYGEN__)
/**
 * @brief   Clock source for the PLLs.
 * @note    This setting has only effect if the PLL is selected as the
 *          system clock source.
 * @note    The default value is calculated for a 168MHz system clock from
 *          an external 8MHz HSE clock.
 */
#if !defined(STM32_PLLSRC) || defined(__DOXYGEN__)
#define STM32_PLLSRC                STM32_PLLSRC_HSE
#endif

/**
 * @brief   PLLM divider value.
 * @note    The allowed values are 2..63.
 * @note    The default value is calculated for a 168MHz system clock from
 *          an external 8MHz HSE clock.
 */
#if !defined(STM32_PLLM_VALUE) || defined(__DOXYGEN__)
#define STM32_PLLM_VALUE            8
#endif

/**
 * @brief   PLLN multiplier value.
 * @note    The allowed values are 192..432.
 * @note    The default value is calculated for a 168MHz system clock from
 *          an external 8MHz HSE clock.
 */
#if !defined(STM32_PLLN_VALUE) || defined(__DOXYGEN__)
#define STM32_PLLN_VALUE            336
#endif

/**
 * @brief   PLLP divider value.
 * @note    The allowed values are 2, 4, 6, 8.
 * @note    The default value is calculated for a 168MHz system clock from
 *          an external 8MHz HSE clock.
 */
#if !defined(STM32_PLLP_VALUE) || defined(__DOXYGEN__)
#define STM32_PLLP_VALUE            2
#endif

/**
 * @brief   PLLQ multiplier value.
 * @note    The allowed values are 2..15.
 * @note    The default value is calculated for a 168MHz system clock from
 *          an external 8MHz HSE clock.
 */
#if !defined(STM32_PLLQ_VALUE) || defined(__DOXYGEN__)
#define STM32_PLLQ_VALUE            7
#endif

#else /* !defined(STM32F4XX) */
/**
 * @brief   Clock source for the PLLs.
 * @note    This setting has only effect if the PLL is selected as the
 *          system clock source.
 * @note    The default value is calculated for a 120MHz system clock from
 *          an external 8MHz HSE clock.
 */
#if !defined(STM32_PLLSRC) || defined(__DOXYGEN__)
#define STM32_PLLSRC                STM32_PLLSRC_HSE
#endif

/**
 * @brief   PLLM divider value.
 * @note    The allowed values are 2..63.
 * @note    The default value is calculated for a 120MHz system clock from
 *          an external 8MHz HSE clock.
 */
#if !defined(STM32_PLLM_VALUE) || defined(__DOXYGEN__)
#define STM32_PLLM_VALUE            8
#endif

/**
 * @brief   PLLN multiplier value.
 * @note    The allowed values are 192..432.
 * @note    The default value is calculated for a 120MHz system clock from
 *          an external 8MHz HSE clock.
 */
#if !defined(STM32_PLLN_VALUE) || defined(__DOXYGEN__)
#define STM32_PLLN_VALUE            240
#endif

/**
 * @brief   PLLP divider value.
 * @note    The allowed values are 2, 4, 6, 8.
 * @note    The default value is calculated for a 120MHz system clock from
 *          an external 8MHz HSE clock.
 */
#if !defined(STM32_PLLP_VALUE) || defined(__DOXYGEN__)
#define STM32_PLLP_VALUE            2
#endif

/**
 * @brief   PLLQ multiplier value.
 * @note    The allowed values are 2..15.
 * @note    The default value is calculated for a 120MHz system clock from
 *          an external 8MHz HSE clock.
 */
#if !defined(STM32_PLLQ_VALUE) || defined(__DOXYGEN__)
#define STM32_PLLQ_VALUE            5
#endif
#endif /* !defined(STM32F4XX) */

/**
 * @brief   AHB prescaler value.
 */
#if !defined(STM32_HPRE) || defined(__DOXYGEN__)
#define STM32_HPRE                  STM32_HPRE_DIV1
#endif

/**
 * @brief   APB1 prescaler value.
 */
#if !defined(STM32_PPRE1) || defined(__DOXYGEN__)
#define STM32_PPRE1                 STM32_PPRE1_DIV4
#endif

/**
 * @brief   APB2 prescaler value.
 */
#if !defined(STM32_PPRE2) || defined(__DOXYGEN__)
#define STM32_PPRE2                 STM32_PPRE2_DIV2
#endif

/**
 * @brief   RTC clock source.
 */
#if !defined(STM32_RTCSEL) || defined(__DOXYGEN__)
#define STM32_RTCSEL                STM32_RTCSEL_LSE
#endif

/**
 * @brief   RTC HSE prescaler value.
 */
#if !defined(STM32_RTCPRE_VALUE) || defined(__DOXYGEN__)
#define STM32_RTCPRE_VALUE          8
#endif

/**
 * @brief   MCO1 clock source value.
 * @note    The default value outputs HSI clock on MCO1 pin.
 */
#if !defined(STM32_MCO1SEL) || defined(__DOXYGEN__)
#define STM32_MCO1SEL               STM32_MCO1SEL_HSI
#endif

/**
 * @brief   MCO1 prescaler value.
 * @note    The default value outputs HSI clock on MCO1 pin.
 */
#if !defined(STM32_MCO1PRE) || defined(__DOXYGEN__)
#define STM32_MCO1PRE               STM32_MCO1PRE_DIV1
#endif

/**
 * @brief   MCO2 clock source value.
 * @note    The default value outputs SYSCLK / 5 on MCO2 pin.
 */
#if !defined(STM32_MCO2SEL) || defined(__DOXYGEN__)
#define STM32_MCO2SEL               STM32_MCO2SEL_SYSCLK
#endif

/**
 * @brief   MCO2 prescaler value.
 * @note    The default value outputs SYSCLK / 5 on MCO2 pin.
 */
#if !defined(STM32_MCO2PRE) || defined(__DOXYGEN__)
#define STM32_MCO2PRE               STM32_MCO2PRE_DIV5
#endif

/**
 * @brief   I2S clock source.
 */
#if !defined(STM32_I2SSRC) || defined(__DOXYGEN__)
#define STM32_I2SSRC                STM32_I2SSRC_CKIN
#endif

/**
 * @brief   PLLI2SN multiplier value.
 * @note    The allowed values are 192..432.
 */
#if !defined(STM32_PLLI2SN_VALUE) || defined(__DOXYGEN__)
#define STM32_PLLI2SN_VALUE         192
#endif

/**
 * @brief   PLLI2SR multiplier value.
 * @note    The allowed values are 2..7.
 */
#if !defined(STM32_PLLI2SR_VALUE) || defined(__DOXYGEN__)
#define STM32_PLLI2SR_VALUE         5
#endif

/**
 * @brief   PLLSAIQ value.
 * @note    The allowed values are 2..15.
 */
#if !defined(STM32_PLLSAIQ_VALUE) || defined(__DOXYGEN__)
#define STM32_PLLSAIQ_VALUE         8
#endif

/**
 * @brief   PLLSAIQ value.
 * @note    The allowed values are 49..432.
 */
#if !defined(STM32_PLLSAIN_VALUE) || defined(__DOXYGEN__)
#define STM32_PLLSAIN_VALUE         120
#endif

/**
 * @brief   PLLSAIQ value.
 * @note    The allowed values are 2..7.
 */
#if !defined(STM32_PLLSAIR_VALUE) || defined(__DOXYGEN__)
#define STM32_PLLSAIR_VALUE         4
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if defined(STM32F4XX) || defined(__DOXYGEN__)
/*
 * Configuration-related checks.
 */
#if !defined(STM32F4xx_MCUCONF)
#error "Using a wrong mcuconf.h file, STM32F4xx_MCUCONF not defined"
#endif

#else /* !defined(STM32F4XX) */
/*
 * Configuration-related checks.
 */
#if !defined(STM32F2xx_MCUCONF)
#error "Using a wrong mcuconf.h file, STM32F2xx_MCUCONF not defined"
#endif
#endif /* !defined(STM32F4XX) */

/**
 * @brief   Maximum frequency thresholds and wait states for flash access.
 * @note    The values are valid for 2.7V to 3.6V supply range.
 */
#if defined(STM32F429_439xx) || defined(STM32F427_437xx) ||                 \
    defined(STM32F40_41xxx)  || defined(STM32F446xx)     ||                 \
    defined(__DOXYGEN__)
#if ((STM32_VDD >= 270) && (STM32_VDD <= 360)) || defined(__DOXYGEN__)
#define STM32_0WS_THRESHOLD         30000000
#define STM32_1WS_THRESHOLD         60000000
#define STM32_2WS_THRESHOLD         90000000
#define STM32_3WS_THRESHOLD         120000000
#define STM32_4WS_THRESHOLD         150000000
#define STM32_5WS_THRESHOLD         180000000
#define STM32_6WS_THRESHOLD         0
#define STM32_7WS_THRESHOLD         0
#define STM32_8WS_THRESHOLD         0
#elif (STM32_VDD >= 240) && (STM32_VDD < 270)
#define STM32_0WS_THRESHOLD         24000000
#define STM32_1WS_THRESHOLD         48000000
#define STM32_2WS_THRESHOLD         72000000
#define STM32_3WS_THRESHOLD         96000000
#define STM32_4WS_THRESHOLD         120000000
#define STM32_5WS_THRESHOLD         144000000
#define STM32_6WS_THRESHOLD         168000000
#define STM32_7WS_THRESHOLD         180000000
#define STM32_8WS_THRESHOLD         0
#elif (STM32_VDD >= 210) && (STM32_VDD < 240)
#define STM32_0WS_THRESHOLD         22000000
#define STM32_1WS_THRESHOLD         44000000
#define STM32_2WS_THRESHOLD         66000000
#define STM32_3WS_THRESHOLD         88000000
#define STM32_4WS_THRESHOLD         110000000
#define STM32_5WS_THRESHOLD         132000000
#define STM32_6WS_THRESHOLD         154000000
#define STM32_7WS_THRESHOLD         176000000
#define STM32_8WS_THRESHOLD         180000000
#elif (STM32_VDD >= 180) && (STM32_VDD < 210)
#define STM32_0WS_THRESHOLD         20000000
#define STM32_1WS_THRESHOLD         40000000
#define STM32_2WS_THRESHOLD         60000000
#define STM32_3WS_THRESHOLD         80000000
#define STM32_4WS_THRESHOLD         100000000
#define STM32_5WS_THRESHOLD         120000000
#define STM32_6WS_THRESHOLD         140000000
#define STM32_7WS_THRESHOLD         168000000
#define STM32_8WS_THRESHOLD         0
#else
#error "invalid VDD voltage specified"
#endif

#elif defined(STM32F401xx)
#if (STM32_VDD >= 270) && (STM32_VDD <= 360)
#define STM32_0WS_THRESHOLD         30000000
#define STM32_1WS_THRESHOLD         60000000
#define STM32_2WS_THRESHOLD         84000000
#define STM32_3WS_THRESHOLD         0
#define STM32_4WS_THRESHOLD         0
#define STM32_5WS_THRESHOLD         0
#define STM32_6WS_THRESHOLD         0
#define STM32_7WS_THRESHOLD         0
#define STM32_8WS_THRESHOLD         0
#elif (STM32_VDD >= 240) && (STM32_VDD < 270)
#define STM32_0WS_THRESHOLD         24000000
#define STM32_1WS_THRESHOLD         48000000
#define STM32_2WS_THRESHOLD         72000000
#define STM32_3WS_THRESHOLD         84000000
#define STM32_4WS_THRESHOLD         0
#define STM32_5WS_THRESHOLD         0
#define STM32_6WS_THRESHOLD         0
#define STM32_7WS_THRESHOLD         0
#define STM32_8WS_THRESHOLD         0
#elif (STM32_VDD >= 210) && (STM32_VDD < 240)
#define STM32_0WS_THRESHOLD         18000000
#define STM32_1WS_THRESHOLD         36000000
#define STM32_2WS_THRESHOLD         54000000
#define STM32_3WS_THRESHOLD         72000000
#define STM32_4WS_THRESHOLD         84000000
#define STM32_5WS_THRESHOLD         0
#define STM32_6WS_THRESHOLD         0
#define STM32_7WS_THRESHOLD         0
#define STM32_8WS_THRESHOLD         0
#elif (STM32_VDD >= 180) && (STM32_VDD < 210)
#define STM32_0WS_THRESHOLD         16000000
#define STM32_1WS_THRESHOLD         32000000
#define STM32_2WS_THRESHOLD         48000000
#define STM32_3WS_THRESHOLD         64000000
#define STM32_4WS_THRESHOLD         80000000
#define STM32_5WS_THRESHOLD         84000000
#define STM32_6WS_THRESHOLD         0
#define STM32_7WS_THRESHOLD         0
#define STM32_8WS_THRESHOLD         0
#else
#error "invalid VDD voltage specified"
#endif

#elif defined(STM32F411xx)
#if (STM32_VDD >= 270) && (STM32_VDD <= 360)
#define STM32_0WS_THRESHOLD         30000000
#define STM32_1WS_THRESHOLD         64000000
#define STM32_2WS_THRESHOLD         90000000
#define STM32_3WS_THRESHOLD         100000000
#define STM32_4WS_THRESHOLD         0
#define STM32_5WS_THRESHOLD         0
#define STM32_6WS_THRESHOLD         0
#define STM32_7WS_THRESHOLD         0
#define STM32_8WS_THRESHOLD         0
#elif (STM32_VDD >= 240) && (STM32_VDD < 270)
#define STM32_0WS_THRESHOLD         24000000
#define STM32_1WS_THRESHOLD         48000000
#define STM32_2WS_THRESHOLD         72000000
#define STM32_3WS_THRESHOLD         96000000
#define STM32_4WS_THRESHOLD         100000000
#define STM32_5WS_THRESHOLD         0
#define STM32_6WS_THRESHOLD         0
#define STM32_7WS_THRESHOLD         0
#define STM32_8WS_THRESHOLD         0
#elif (STM32_VDD >= 210) && (STM32_VDD < 240)
#define STM32_0WS_THRESHOLD         18000000
#define STM32_1WS_THRESHOLD         36000000
#define STM32_2WS_THRESHOLD         54000000
#define STM32_3WS_THRESHOLD         72000000
#define STM32_4WS_THRESHOLD         90000000
#define STM32_5WS_THRESHOLD         100000000
#define STM32_6WS_THRESHOLD         0
#define STM32_7WS_THRESHOLD         0
#define STM32_8WS_THRESHOLD         0
#elif (STM32_VDD >= 171) && (STM32_VDD < 210)
#define STM32_0WS_THRESHOLD         16000000
#define STM32_1WS_THRESHOLD         32000000
#define STM32_2WS_THRESHOLD         48000000
#define STM32_3WS_THRESHOLD         64000000
#define STM32_4WS_THRESHOLD         80000000
#define STM32_5WS_THRESHOLD         96000000
#define STM32_6WS_THRESHOLD         100000000
#define STM32_7WS_THRESHOLD         0
#define STM32_8WS_THRESHOLD         0
#else
#error "invalid VDD voltage specified"
#endif

#else /* STM32F2XX */
#if (STM32_VDD >= 270) && (STM32_VDD <= 360)
#define STM32_0WS_THRESHOLD         30000000
#define STM32_1WS_THRESHOLD         60000000
#define STM32_2WS_THRESHOLD         90000000
#define STM32_3WS_THRESHOLD         120000000
#define STM32_4WS_THRESHOLD         0
#define STM32_5WS_THRESHOLD         0
#define STM32_6WS_THRESHOLD         0
#define STM32_7WS_THRESHOLD         0
#elif (STM32_VDD >= 240) && (STM32_VDD < 270)
#define STM32_0WS_THRESHOLD         24000000
#define STM32_1WS_THRESHOLD         48000000
#define STM32_2WS_THRESHOLD         72000000
#define STM32_3WS_THRESHOLD         96000000
#define STM32_4WS_THRESHOLD         120000000
#define STM32_5WS_THRESHOLD         0
#define STM32_6WS_THRESHOLD         0
#define STM32_7WS_THRESHOLD         0
#elif (STM32_VDD >= 210) && (STM32_VDD < 240)
#define STM32_0WS_THRESHOLD         18000000
#define STM32_1WS_THRESHOLD         36000000
#define STM32_2WS_THRESHOLD         54000000
#define STM32_3WS_THRESHOLD         72000000
#define STM32_4WS_THRESHOLD         90000000
#define STM32_5WS_THRESHOLD         108000000
#define STM32_6WS_THRESHOLD         120000000
#define STM32_7WS_THRESHOLD         0
#elif (STM32_VDD >= 180) && (STM32_VDD < 210)
#define STM32_0WS_THRESHOLD         16000000
#define STM32_1WS_THRESHOLD         32000000
#define STM32_2WS_THRESHOLD         48000000
#define STM32_3WS_THRESHOLD         64000000
#define STM32_4WS_THRESHOLD         80000000
#define STM32_5WS_THRESHOLD         96000000
#define STM32_6WS_THRESHOLD         112000000
#define STM32_7WS_THRESHOLD         120000000
#else
#error "invalid VDD voltage specified"
#endif
#endif /* STM32F2XX */

/*
 * HSI related checks.
 */
#if STM32_HSI_ENABLED
#else /* !STM32_HSI_ENABLED */

#if STM32_SW == STM32_SW_HSI
#error "HSI not enabled, required by STM32_SW"
#endif

#if (STM32_SW == STM32_SW_PLL) && (STM32_PLLSRC == STM32_PLLSRC_HSI)
#error "HSI not enabled, required by STM32_SW and STM32_PLLSRC"
#endif

#if (STM32_MCO1SEL == STM32_MCO1SEL_HSI) ||                                 \
    ((STM32_MCO1SEL == STM32_MCO1SEL_PLL) &&                                \
     (STM32_PLLSRC == STM32_PLLSRC_HSI))
#error "HSI not enabled, required by STM32_MCO1SEL"
#endif

#if (STM32_MCO2SEL == STM32_MCO2SEL_PLL) && (STM32_PLLSRC == STM32_PLLSRC_HSI)
#error "HSI not enabled, required by STM32_MCO2SEL"
#endif

#if (STM32_I2SSRC == STM32_I2SSRC_PLLI2S) &&                                \
    (STM32_PLLSRC == STM32_PLLSRC_HSI)
#error "HSI not enabled, required by STM32_I2SSRC"
#endif

#endif /* !STM32_HSI_ENABLED */

/*
 * HSE related checks.
 */
#if STM32_HSE_ENABLED

#if STM32_HSECLK == 0
#error "HSE frequency not defined"
#elif (STM32_HSECLK < STM32_HSECLK_MIN) || (STM32_HSECLK > STM32_HSECLK_MAX)
#error "STM32_HSECLK outside acceptable range (STM32_HSECLK_MIN...STM32_HSECLK_MAX)"
#endif

#else /* !STM32_HSE_ENABLED */

#if STM32_SW == STM32_SW_HSE
#error "HSE not enabled, required by STM32_SW"
#endif

#if (STM32_SW == STM32_SW_PLL) && (STM32_PLLSRC == STM32_PLLSRC_HSE)
#error "HSE not enabled, required by STM32_SW and STM32_PLLSRC"
#endif

#if (STM32_MCO1SEL == STM32_MCO1SEL_HSE) ||                                 \
    ((STM32_MCO1SEL == STM32_MCO1SEL_PLL) &&                                \
     (STM32_PLLSRC == STM32_PLLSRC_HSE))
#error "HSE not enabled, required by STM32_MCO1SEL"
#endif

#if (STM32_MCO2SEL == STM32_MCO2SEL_HSE) ||                                 \
    ((STM32_MCO2SEL == STM32_MCO2SEL_PLL) &&                                \
     (STM32_PLLSRC == STM32_PLLSRC_HSE))
#error "HSE not enabled, required by STM32_MCO2SEL"
#endif

#if (STM32_I2SSRC == STM32_I2SSRC_PLLI2S) &&                                \
    (STM32_PLLSRC == STM32_PLLSRC_HSE)
#error "HSE not enabled, required by STM32_I2SSRC"
#endif

#if STM32_RTCSEL == STM32_RTCSEL_HSEDIV
#error "HSE not enabled, required by STM32_RTCSEL"
#endif

#endif /* !STM32_HSE_ENABLED */

/*
 * LSI related checks.
 */
#if STM32_LSI_ENABLED
#else /* !STM32_LSI_ENABLED */

#if STM32_RTCSEL == STM32_RTCSEL_LSI
#error "LSI not enabled, required by STM32_RTCSEL"
#endif

#endif /* !STM32_LSI_ENABLED */

/*
 * LSE related checks.
 */
#if STM32_LSE_ENABLED

#if (STM32_LSECLK == 0)
#error "LSE frequency not defined"
#endif

#if (STM32_LSECLK < STM32_LSECLK_MIN) || (STM32_LSECLK > STM32_LSECLK_MAX)
#error "STM32_LSECLK outside acceptable range (STM32_LSECLK_MIN...STM32_LSECLK_MAX)"
#endif

#else /* !STM32_LSE_ENABLED */

#if STM32_RTCSEL == STM32_RTCSEL_LSE
#error "LSE not enabled, required by STM32_RTCSEL"
#endif

#endif /* !STM32_LSE_ENABLED */

/**
 * @brief   STM32_PLLM field.
 */
#if ((STM32_PLLM_VALUE >= 2) && (STM32_PLLM_VALUE <= 63)) ||                \
    defined(__DOXYGEN__)
#define STM32_PLLM                  (STM32_PLLM_VALUE << 0)
#else
#error "invalid STM32_PLLM_VALUE value specified"
#endif

/**
 * @brief   PLLs input clock frequency.
 */
#if (STM32_PLLSRC == STM32_PLLSRC_HSE) || defined(__DOXYGEN__)
#define STM32_PLLCLKIN              (STM32_HSECLK / STM32_PLLM_VALUE)
#elif STM32_PLLSRC == STM32_PLLSRC_HSI
#define STM32_PLLCLKIN              (STM32_HSICLK / STM32_PLLM_VALUE)
#else
#error "invalid STM32_PLLSRC value specified"
#endif

/*
 * PLLs input frequency range check.
 */
#if (STM32_PLLCLKIN < STM32_PLLIN_MIN) || (STM32_PLLCLKIN > STM32_PLLIN_MAX)
#error "STM32_PLLCLKIN outside acceptable range (STM32_PLLIN_MIN...STM32_PLLIN_MAX)"
#endif

/*
 * PLL enable check.
 */
#if STM32_CLOCK48_REQUIRED ||                                               \
    (STM32_SW == STM32_SW_PLL) ||                                           \
    (STM32_MCO1SEL == STM32_MCO1SEL_PLL) ||                                 \
    (STM32_MCO2SEL == STM32_MCO2SEL_PLL) ||                                 \
    defined(__DOXYGEN__)
/**
 * @brief   PLL activation flag.
 */
#define STM32_ACTIVATE_PLL          TRUE
#else
#define STM32_ACTIVATE_PLL          FALSE
#endif

/**
 * @brief   STM32_PLLN field.
 */
#if ((STM32_PLLN_VALUE >= 64) && (STM32_PLLN_VALUE <= 432)) ||              \
    defined(__DOXYGEN__)
#define STM32_PLLN                  (STM32_PLLN_VALUE << 6)
#else
#error "invalid STM32_PLLN_VALUE value specified"
#endif

/**
 * @brief   STM32_PLLP field.
 */
#if (STM32_PLLP_VALUE == 2) || defined(__DOXYGEN__)
#define STM32_PLLP                  (0 << 16)
#elif STM32_PLLP_VALUE == 4
#define STM32_PLLP                  (1 << 16)
#elif STM32_PLLP_VALUE == 6
#define STM32_PLLP                  (2 << 16)
#elif STM32_PLLP_VALUE == 8
#define STM32_PLLP                  (3 << 16)
#else
#error "invalid STM32_PLLP_VALUE value specified"
#endif

/**
 * @brief   STM32_PLLQ field.
 */
#if ((STM32_PLLQ_VALUE >= 2) && (STM32_PLLQ_VALUE <= 15)) ||                \
    defined(__DOXYGEN__)
#define STM32_PLLQ                  (STM32_PLLQ_VALUE << 24)
#else
#error "invalid STM32_PLLQ_VALUE value specified"
#endif

/**
 * @brief   PLL VCO frequency.
 */
#define STM32_PLLVCO                (STM32_PLLCLKIN * STM32_PLLN_VALUE)

/*
 * PLL VCO frequency range check.
 */
#if (STM32_PLLVCO < STM32_PLLVCO_MIN) || (STM32_PLLVCO > STM32_PLLVCO_MAX)
#error "STM32_PLLVCO outside acceptable range (STM32_PLLVCO_MIN...STM32_PLLVCO_MAX)"
#endif

/**
 * @brief   PLL output clock frequency.
 */
#define STM32_PLLCLKOUT             (STM32_PLLVCO / STM32_PLLP_VALUE)

/*
 * PLL output frequency range check.
 */
#if (STM32_PLLCLKOUT < STM32_PLLOUT_MIN) || (STM32_PLLCLKOUT > STM32_PLLOUT_MAX)
#error "STM32_PLLCLKOUT outside acceptable range (STM32_PLLOUT_MIN...STM32_PLLOUT_MAX)"
#endif

/**
 * @brief   System clock source.
 */
#if STM32_NO_INIT || defined(__DOXYGEN__)
#define STM32_SYSCLK                STM32_HSICLK
#elif (STM32_SW == STM32_SW_HSI)
#define STM32_SYSCLK                STM32_HSICLK
#elif (STM32_SW == STM32_SW_HSE)
#define STM32_SYSCLK                STM32_HSECLK
#elif (STM32_SW == STM32_SW_PLL)
#define STM32_SYSCLK                STM32_PLLCLKOUT
#else
#error "invalid STM32_SW value specified"
#endif

/* Check on the system clock.*/
#if STM32_SYSCLK > STM32_SYSCLK_MAX
#error "STM32_SYSCLK above maximum rated frequency (STM32_SYSCLK_MAX)"
#endif

/* Calculating VOS settings, it is different for each sub-platform.*/
#if defined(STM32F429_439xx) || defined(STM32F427_437xx) ||                 \
    defined(STM32F446xx)     || defined(__DOXYGEN__)
#if STM32_SYSCLK <= 120000000
#define STM32_VOS                   STM32_VOS_SCALE3
#define STM32_OVERDRIVE_REQUIRED    FALSE
#elif STM32_SYSCLK <= 144000000
#define STM32_VOS                   STM32_VOS_SCALE2
#define STM32_OVERDRIVE_REQUIRED    FALSE
#elif STM32_SYSCLK <= 168000000
#define STM32_VOS                   STM32_VOS_SCALE1
#define STM32_OVERDRIVE_REQUIRED    FALSE
#else
#define STM32_VOS                   STM32_VOS_SCALE1
#define STM32_OVERDRIVE_REQUIRED    TRUE
#endif

#elif defined(STM32F40_41xxx)
#if STM32_SYSCLK <= 144000000
#define STM32_VOS                   STM32_VOS_SCALE2
#else
#define STM32_VOS                   STM32_VOS_SCALE1
#endif
#define STM32_OVERDRIVE_REQUIRED    FALSE

#elif defined(STM32F401xx)
#if STM32_SYSCLK <= 60000000
#define STM32_VOS                   STM32_VOS_SCALE3
#else
#define STM32_VOS                   STM32_VOS_SCALE2
#endif
#define STM32_OVERDRIVE_REQUIRED    FALSE

#elif defined(STM32F411xx)
#if STM32_SYSCLK <= 64000000
#define STM32_VOS                   STM32_VOS_SCALE3
#elif STM32_SYSCLK <= 84000000
#define STM32_VOS                   STM32_VOS_SCALE2
#else
#define STM32_VOS                   STM32_VOS_SCALE1
#endif
#define STM32_OVERDRIVE_REQUIRED    FALSE

#else /* STM32F2XX */
#define STM32_OVERDRIVE_REQUIRED    FALSE
#endif

/**
 * @brief   AHB frequency.
 */
#if (STM32_HPRE == STM32_HPRE_DIV1) || defined(__DOXYGEN__)
#define STM32_HCLK                  (STM32_SYSCLK / 1)
#elif STM32_HPRE == STM32_HPRE_DIV2
#define STM32_HCLK                  (STM32_SYSCLK / 2)
#elif STM32_HPRE == STM32_HPRE_DIV4
#define STM32_HCLK                  (STM32_SYSCLK / 4)
#elif STM32_HPRE == STM32_HPRE_DIV8
#define STM32_HCLK                  (STM32_SYSCLK / 8)
#elif STM32_HPRE == STM32_HPRE_DIV16
#define STM32_HCLK                  (STM32_SYSCLK / 16)
#elif STM32_HPRE == STM32_HPRE_DIV64
#define STM32_HCLK                  (STM32_SYSCLK / 64)
#elif STM32_HPRE == STM32_HPRE_DIV128
#define STM32_HCLK                  (STM32_SYSCLK / 128)
#elif STM32_HPRE == STM32_HPRE_DIV256
#define STM32_HCLK                  (STM32_SYSCLK / 256)
#elif STM32_HPRE == STM32_HPRE_DIV512
#define STM32_HCLK                  (STM32_SYSCLK / 512)
#else
#error "invalid STM32_HPRE value specified"
#endif

/*
 * AHB frequency check.
 */
#if STM32_HCLK > STM32_SYSCLK_MAX
#error "STM32_HCLK exceeding maximum frequency (STM32_SYSCLK_MAX)"
#endif

/**
 * @brief   APB1 frequency.
 */
#if (STM32_PPRE1 == STM32_PPRE1_DIV1) || defined(__DOXYGEN__)
#define STM32_PCLK1                 (STM32_HCLK / 1)
#elif STM32_PPRE1 == STM32_PPRE1_DIV2
#define STM32_PCLK1                 (STM32_HCLK / 2)
#elif STM32_PPRE1 == STM32_PPRE1_DIV4
#define STM32_PCLK1                 (STM32_HCLK / 4)
#elif STM32_PPRE1 == STM32_PPRE1_DIV8
#define STM32_PCLK1                 (STM32_HCLK / 8)
#elif STM32_PPRE1 == STM32_PPRE1_DIV16
#define STM32_PCLK1                 (STM32_HCLK / 16)
#else
#error "invalid STM32_PPRE1 value specified"
#endif

/*
 * APB1 frequency check.
 */
#if STM32_PCLK1 > STM32_PCLK1_MAX
#error "STM32_PCLK1 exceeding maximum frequency (STM32_PCLK1_MAX)"
#endif

/**
 * @brief   APB2 frequency.
 */
#if (STM32_PPRE2 == STM32_PPRE2_DIV1) || defined(__DOXYGEN__)
#define STM32_PCLK2                 (STM32_HCLK / 1)
#elif STM32_PPRE2 == STM32_PPRE2_DIV2
#define STM32_PCLK2                 (STM32_HCLK / 2)
#elif STM32_PPRE2 == STM32_PPRE2_DIV4
#define STM32_PCLK2                 (STM32_HCLK / 4)
#elif STM32_PPRE2 == STM32_PPRE2_DIV8
#define STM32_PCLK2                 (STM32_HCLK / 8)
#elif STM32_PPRE2 == STM32_PPRE2_DIV16
#define STM32_PCLK2                 (STM32_HCLK / 16)
#else
#error "invalid STM32_PPRE2 value specified"
#endif

/*
 * APB2 frequency check.
 */
#if STM32_PCLK2 > STM32_PCLK2_MAX
#error "STM32_PCLK2 exceeding maximum frequency (STM32_PCLK2_MAX)"
#endif

/*
 * PLLI2S enable check.
 */
#if (STM32_I2SSRC == STM32_I2SSRC_PLLI2S) || defined(__DOXYGEN__)
/**
 * @brief   PLLI2S activation flag.
 */
#define STM32_ACTIVATE_PLLI2S       TRUE
#else
#define STM32_ACTIVATE_PLLI2S       FALSE
#endif

/**
 * @brief   STM32_PLLI2SN field.
 */
#if ((STM32_PLLI2SN_VALUE >= 192) && (STM32_PLLI2SN_VALUE <= 432)) ||       \
    defined(__DOXYGEN__)
#define STM32_PLLI2SN               (STM32_PLLI2SN_VALUE << 6)
#else
#error "invalid STM32_PLLI2SN_VALUE value specified"
#endif

/**
 * @brief   STM32_PLLI2SR field.
 */
#if ((STM32_PLLI2SR_VALUE >= 2) && (STM32_PLLI2SR_VALUE <= 7)) ||           \
    defined(__DOXYGEN__)
#define STM32_PLLI2SR               (STM32_PLLI2SR_VALUE << 28)
#else
#error "invalid STM32_PLLI2SR_VALUE value specified"
#endif

/**
 * @brief   PLLSAI activation flag.
 */
//#define STM32_ACTIVATE_PLLSAI       FALSE

/**
 * @brief   STM32_PLLSAIN field.
 */
#if ((STM32_PLLSAIN_VALUE >= 49) && (STM32_PLLSAIN_VALUE <= 432)) ||        \
    defined(__DOXYGEN__)
#define STM32_PLLSAIN               (STM32_PLLSAIN_VALUE << 6)
#else
#error "invalid STM32_PLLSAIN_VALUE value specified"
#endif

/**
 * @brief   STM32_PLLSAIQ field.
 */
#if ((STM32_PLLSAIQ_VALUE >= 2) && (STM32_PLLSAIQ_VALUE <= 15)) ||          \
    defined(__DOXYGEN__)
#define STM32_PLLSAIQ               (STM32_PLLSAIQ_VALUE << 24)
#else
#error "invalid STM32_PLLSAIR_VALUE value specified"
#endif

/**
 * @brief   STM32_PLLSAIR field.
 */
#if ((STM32_PLLSAIR_VALUE >= 2) && (STM32_PLLSAIR_VALUE <= 7)) ||           \
    defined(__DOXYGEN__)
#define STM32_PLLSAIR               (STM32_PLLSAIR_VALUE << 28)
#else
#error "invalid STM32_PLLSAIR_VALUE value specified"
#endif

/**
 * @brief   PLL VCO frequency.
 */
#define STM32_PLLI2SVCO             (STM32_PLLCLKIN * STM32_PLLI2SN_VALUE)

/*
 * PLLI2S VCO frequency range check.
 */
#if (STM32_PLLI2SVCO < STM32_PLLVCO_MIN) ||                                 \
    (STM32_PLLI2SVCO > STM32_PLLVCO_MAX)
#error "STM32_PLLI2SVCO outside acceptable range (STM32_PLLVCO_MIN...STM32_PLLVCO_MAX)"
#endif

/**
 * @brief   PLLI2S output clock frequency.
 */
#define STM32_PLLI2SCLKOUT          (STM32_PLLI2SVCO / STM32_PLLI2SR_VALUE)

/**
 * @brief   MCO1 divider clock.
 */
#if (STM32_MCO1SEL == STM32_MCO1SEL_HSI) || defined(__DOXYGEN__)
#define STM32_MCO1DIVCLK             STM32_HSICLK
#elif STM32_MCO1SEL == STM32_MCO1SEL_LSE
#define STM32_MCO1DIVCLK             STM32_LSECLK
#elif STM32_MCO1SEL == STM32_MCO1SEL_HSE
#define STM32_MCO1DIVCLK             STM32_HSECLK
#elif STM32_MCO1SEL == STM32_MCO1SEL_PLL
#define STM32_MCO1DIVCLK             STM32_PLLCLKOUT
#else
#error "invalid STM32_MCO1SEL value specified"
#endif

/**
 * @brief   MCO1 output pin clock.
 */
#if (STM32_MCO1PRE == STM32_MCO1PRE_DIV1) || defined(__DOXYGEN__)
#define STM32_MCO1CLK                STM32_MCO1DIVCLK
#elif STM32_MCO1PRE == STM32_MCO1PRE_DIV2
#define STM32_MCO1CLK                (STM32_MCO1DIVCLK / 2)
#elif STM32_MCO1PRE == STM32_MCO1PRE_DIV3
#define STM32_MCO1CLK                (STM32_MCO1DIVCLK / 3)
#elif STM32_MCO1PRE == STM32_MCO1PRE_DIV4
#define STM32_MCO1CLK                (STM32_MCO1DIVCLK / 4)
#elif STM32_MCO1PRE == STM32_MCO1PRE_DIV5
#define STM32_MCO1CLK                (STM32_MCO1DIVCLK / 5)
#else
#error "invalid STM32_MCO1PRE value specified"
#endif

/**
 * @brief   MCO2 divider clock.
 */
#if (STM32_MCO2SEL == STM32_MCO2SEL_HSE) || defined(__DOXYGEN__)
#define STM32_MCO2DIVCLK             STM32_HSECLK
#elif STM32_MCO2SEL == STM32_MCO2SEL_PLL
#define STM32_MCO2DIVCLK             STM32_PLLCLKOUT
#elif STM32_MCO2SEL == STM32_MCO2SEL_SYSCLK
#define STM32_MCO2DIVCLK             STM32_SYSCLK
#elif STM32_MCO2SEL == STM32_MCO2SEL_PLLI2S
#define STM32_MCO2DIVCLK             STM32_PLLI2S
#else
#error "invalid STM32_MCO2SEL value specified"
#endif

/**
 * @brief   MCO2 output pin clock.
 */
#if (STM32_MCO2PRE == STM32_MCO2PRE_DIV1) || defined(__DOXYGEN__)
#define STM32_MCO2CLK                STM32_MCO2DIVCLK
#elif STM32_MCO2PRE == STM32_MCO2PRE_DIV2
#define STM32_MCO2CLK                (STM32_MCO2DIVCLK / 2)
#elif STM32_MCO2PRE == STM32_MCO2PRE_DIV3
#define STM32_MCO2CLK                (STM32_MCO2DIVCLK / 3)
#elif STM32_MCO2PRE == STM32_MCO2PRE_DIV4
#define STM32_MCO2CLK                (STM32_MCO2DIVCLK / 4)
#elif STM32_MCO2PRE == STM32_MCO2PRE_DIV5
#define STM32_MCO2CLK                (STM32_MCO2DIVCLK / 5)
#else
#error "invalid STM32_MCO2PRE value specified"
#endif

/**
 * @brief   RTC HSE divider setting.
 */
#if ((STM32_RTCPRE_VALUE >= 2) && (STM32_RTCPRE_VALUE <= 31)) ||            \
    defined(__DOXYGEN__)
#define STM32_RTCPRE                (STM32_RTCPRE_VALUE << 16)
#else
#error "invalid STM32_RTCPRE value specified"
#endif

/**
 * @brief   HSE divider toward RTC clock.
 */
#if ((STM32_RTCPRE_VALUE >= 2) && (STM32_RTCPRE_VALUE <= 31))  ||           \
    defined(__DOXYGEN__)
#define STM32_HSEDIVCLK             (STM32_HSECLK / STM32_RTCPRE_VALUE)
#else
#error "invalid STM32_RTCPRE value specified"
#endif

/**
 * @brief   RTC clock.
 */
#if (STM32_RTCSEL == STM32_RTCSEL_NOCLOCK) || defined(__DOXYGEN__)
#define STM32_RTCCLK                0
#elif STM32_RTCSEL == STM32_RTCSEL_LSE
#define STM32_RTCCLK                STM32_LSECLK
#elif STM32_RTCSEL == STM32_RTCSEL_LSI
#define STM32_RTCCLK                STM32_LSICLK
#elif STM32_RTCSEL == STM32_RTCSEL_HSEDIV
#define STM32_RTCCLK                STM32_HSEDIVCLK
#else
#error "invalid STM32_RTCSEL value specified"
#endif

/**
 * @brief   48MHz frequency.
 */
#if STM32_CLOCK48_REQUIRED || defined(__DOXYGEN__)
#define STM32_PLL48CLK              (STM32_PLLVCO / STM32_PLLQ_VALUE)
#else
#define STM32_PLL48CLK              0
#endif

/**
 * @brief   Clock of timers connected to APB1
 *          (Timers 2, 3, 4, 5, 6, 7, 12, 13, 14).
 */
#if (STM32_PPRE1 == STM32_PPRE1_DIV1) || defined(__DOXYGEN__)
#define STM32_TIMCLK1               (STM32_PCLK1 * 1)
#else
#define STM32_TIMCLK1               (STM32_PCLK1 * 2)
#endif

/**
 * @brief   Clock of timers connected to APB2 (Timers 1, 8, 9, 10, 11).
 */
#if (STM32_PPRE2 == STM32_PPRE2_DIV1) || defined(__DOXYGEN__)
#define STM32_TIMCLK2               (STM32_PCLK2 * 1)
#else
#define STM32_TIMCLK2               (STM32_PCLK2 * 2)
#endif

/**
 * @brief   Flash settings.
 */
#if (STM32_HCLK <= STM32_0WS_THRESHOLD) || defined(__DOXYGEN__)
#define STM32_FLASHBITS             0x00000000
#elif STM32_HCLK <= STM32_1WS_THRESHOLD
#define STM32_FLASHBITS             0x00000001
#elif STM32_HCLK <= STM32_2WS_THRESHOLD
#define STM32_FLASHBITS             0x00000002
#elif STM32_HCLK <= STM32_3WS_THRESHOLD
#define STM32_FLASHBITS             0x00000003
#elif STM32_HCLK <= STM32_4WS_THRESHOLD
#define STM32_FLASHBITS             0x00000004
#elif STM32_HCLK <= STM32_5WS_THRESHOLD
#define STM32_FLASHBITS             0x00000005
#elif STM32_HCLK <= STM32_6WS_THRESHOLD
#define STM32_FLASHBITS             0x00000006
#elif STM32_HCLK <= STM32_7WS_THRESHOLD
#define STM32_FLASHBITS             0x00000007
#else
#define STM32_FLASHBITS             0x00000008
#endif

/* There are differences in vector names in the various sub-families,
   normalizing.*/
#if 0
#define TIM1_BRK_IRQn       TIM1_BRK_TIM9_IRQn
#define TIM1_UP_IRQn        TIM1_UP_TIM10_IRQn
#define TIM1_TRG_COM_IRQn   TIM1_TRG_COM_TIM11_IRQn
#define TIM8_BRK_IRQn       TIM8_BRK_TIM12_IRQn
#define TIM8_UP_IRQn        TIM8_UP_TIM13_IRQn
#define TIM8_TRG_COM_IRQn   TIM8_TRG_COM_TIM14_IRQn
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

/* Various helpers.*/
#include "nvic.h"
#include "stm32_isr.h"
#include "stm32_dma.h"
#include "stm32_rcc.h"

#ifdef __cplusplus
extern "C" {
#endif
  void hal_lld_init(void);
  void stm32_clock_init(void);
#ifdef __cplusplus
}
#endif

#endif /* _HAL_LLD_H_ */

/** @} */
