/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

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
 * @file    STM32/timcap_lld.h
 * @brief   STM32 TIMCAP subsystem low level driver header.
 *
 * @addtogroup TIMCAP
 * @{
 */

#ifndef HAL_TIMCAP_LLD_H_
#define HAL_TIMCAP_LLD_H_

#include "hal.h"
#include "stm32_tim.h"


#if HAL_USE_TIMCAP || defined(__DOXYGEN__)

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
 * @brief   TIMCAPD1 driver enable switch.
 * @details If set to @p TRUE the support for TIMCAPD1 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_TIMCAP_USE_TIM1) || defined(__DOXYGEN__)
#define STM32_TIMCAP_USE_TIM1                  FALSE
#endif

/**
 * @brief   TIMCAPD2 driver enable switch.
 * @details If set to @p TRUE the support for TIMCAPD2 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_TIMCAP_USE_TIM2) || defined(__DOXYGEN__)
#define STM32_TIMCAP_USE_TIM2                  FALSE
#endif

/**
 * @brief   TIMCAPD3 driver enable switch.
 * @details If set to @p TRUE the support for TIMCAPD3 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_TIMCAP_USE_TIM3) || defined(__DOXYGEN__)
#define STM32_TIMCAP_USE_TIM3                  FALSE
#endif

/**
 * @brief   TIMCAPD4 driver enable switch.
 * @details If set to @p TRUE the support for TIMCAPD4 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_TIMCAP_USE_TIM4) || defined(__DOXYGEN__)
#define STM32_TIMCAP_USE_TIM4                  FALSE
#endif

/**
 * @brief   TIMCAPD5 driver enable switch.
 * @details If set to @p TRUE the support for TIMCAPD5 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_TIMCAP_USE_TIM5) || defined(__DOXYGEN__)
#define STM32_TIMCAP_USE_TIM5                  FALSE
#endif

/**
 * @brief   TIMCAPD8 driver enable switch.
 * @details If set to @p TRUE the support for TIMCAPD8 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_TIMCAP_USE_TIM8) || defined(__DOXYGEN__)
#define STM32_TIMCAP_USE_TIM8                  FALSE
#endif

/**
 * @brief   TIMCAPD9 driver enable switch.
 * @details If set to @p TRUE the support for TIMCAPD9 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_TIMCAP_USE_TIM9) || defined(__DOXYGEN__)
#define STM32_TIMCAP_USE_TIM9                  FALSE
#endif

/**
 * @brief   TIMCAPD1 interrupt priority level setting.
 */
#if !defined(STM32_TIMCAP_TIM1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_TIMCAP_TIM1_IRQ_PRIORITY         7
#endif

/**
 * @brief   TIMCAPD2 interrupt priority level setting.
 */
#if !defined(STM32_TIMCAP_TIM2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_TIMCAP_TIM2_IRQ_PRIORITY         7
#endif

/**
 * @brief   TIMCAPD3 interrupt priority level setting.
 */
#if !defined(STM32_TIMCAP_TIM3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_TIMCAP_TIM3_IRQ_PRIORITY         7
#endif

/**
 * @brief   TIMCAPD4 interrupt priority level setting.
 */
#if !defined(STM32_TIMCAP_TIM4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_TIMCAP_TIM4_IRQ_PRIORITY         7
#endif

/**
 * @brief   TIMCAPD5 interrupt priority level setting.
 */
#if !defined(STM32_TIMCAP_TIM5_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_TIMCAP_TIM5_IRQ_PRIORITY         7
#endif

/**
 * @brief   TIMCAPD8 interrupt priority level setting.
 */
#if !defined(STM32_TIMCAP_TIM8_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_TIMCAP_TIM8_IRQ_PRIORITY         7
#endif

/**
 * @brief   TIMCAPD9 interrupt priority level setting.
 */
#if !defined(STM32_TIMCAP_TIM9_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_TIMCAP_TIM9_IRQ_PRIORITY         7
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if STM32_TIMCAP_USE_TIM1 && !STM32_HAS_TIM1
#error "TIM1 not present in the selected device"
#endif

#if STM32_TIMCAP_USE_TIM2 && !STM32_HAS_TIM2
#error "TIM2 not present in the selected device"
#endif

#if STM32_TIMCAP_USE_TIM3 && !STM32_HAS_TIM3
#error "TIM3 not present in the selected device"
#endif

#if STM32_TIMCAP_USE_TIM4 && !STM32_HAS_TIM4
#error "TIM4 not present in the selected device"
#endif

#if STM32_TIMCAP_USE_TIM5 && !STM32_HAS_TIM5
#error "TIM5 not present in the selected device"
#endif

#if STM32_TIMCAP_USE_TIM8 && !STM32_HAS_TIM8
#error "TIM8 not present in the selected device"
#endif

#if STM32_TIMCAP_USE_TIM9 && !STM32_HAS_TIM9
#error "TIM9 not present in the selected device"
#endif

#if !STM32_TIMCAP_USE_TIM1 && !STM32_TIMCAP_USE_TIM2 &&                           \
    !STM32_TIMCAP_USE_TIM3 && !STM32_TIMCAP_USE_TIM4 &&                           \
    !STM32_TIMCAP_USE_TIM5 && !STM32_TIMCAP_USE_TIM8 &&                           \
    !STM32_TIMCAP_USE_TIM9
#error "TIMCAP driver activated but no TIM peripheral assigned"
#endif

#if STM32_TIMCAP_USE_TIM1 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_TIMCAP_TIM1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM1"
#endif

#if STM32_TIMCAP_USE_TIM2 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_TIMCAP_TIM2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM2"
#endif

#if STM32_TIMCAP_USE_TIM3 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_TIMCAP_TIM3_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM3"
#endif

#if STM32_TIMCAP_USE_TIM4 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_TIMCAP_TIM4_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM4"
#endif

#if STM32_TIMCAP_USE_TIM5 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_TIMCAP_TIM5_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM5"
#endif

#if STM32_TIMCAP_USE_TIM8 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_TIMCAP_TIM8_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM8"
#endif

#if STM32_TIMCAP_USE_TIM9 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_TIMCAP_TIM9_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM9"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   TIMCAP driver mode.
 */
typedef enum {
  TIMCAP_INPUT_DISABLED = 0,
  TIMCAP_INPUT_ACTIVE_HIGH = 1,        /**< Trigger on rising edge.            */
  TIMCAP_INPUT_ACTIVE_LOW = 2,         /**< Trigger on falling edge.           */
} timcapmode_t;

/**
 * @brief   TIMCAP frequency type.
 */
typedef uint32_t timcapfreq_t;

/**
 * @brief   TIMCAP channel type.
 */
typedef enum {
  TIMCAP_CHANNEL_1 = 0,              /**< Use TIMxCH1.      */
  TIMCAP_CHANNEL_2 = 1,              /**< Use TIMxCH2.      */
  TIMCAP_CHANNEL_3 = 2,              /**< Use TIMxCH3.      */
  TIMCAP_CHANNEL_4 = 3,              /**< Use TIMxCH4.      */
} timcapchannel_t;


/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief   Driver mode.
   */
  timcapmode_t                 modes[4];
  /**
   * @brief   Timer clock in Hz.
   * @note    The low level can use assertions in order to catch invalid
   *          frequency specifications.
   */
  timcapfreq_t                 frequency;

  /**
   * @brief   Callback when a capture occurs
   */
  timcapcallback_t             capture_cb_array[4];

  /**
   * @brief   Callback for timer overflow.
   */
  timcapcallback_t             overflow_cb;

  /* End of the mandatory fields.*/

  /**
   * @brief TIM DIER register initialization data.
   * @note  The value of this field should normally be equal to zero.
   * @note  Only the DMA-related bits can be specified in this field.
   */
  uint32_t                  dier;
  
  /**
   * @brief TIM CR1 register initialization data.
   * @note  The value of this field should normally be equal to zero.
   */
  uint32_t                  cr1;
} TIMCAPConfig;

/**
 * @brief   Structure representing an TIMCAP driver.
 */
struct TIMCAPDriver {
  /**
   * @brief Driver state.
   */
  timcapstate_t                state;
  /**
   * @brief Current configuration data.
   */
  const TIMCAPConfig           *config;
#if defined(TIMCAP_DRIVER_EXT_FIELDS)
  TIMCAP_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Timer base clock.
   */
  uint32_t                  clock;
  /**
   * @brief Pointer to the TIMx registers block.
   */
  stm32_tim_t               *tim;
  /**
   * @brief CCR register used for capture.
   */
  volatile uint32_t         *ccr_p[4];
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

//FIXME document this
#define timcap_lld_get_ccr(timcapp, channel) (*((timcapp)->ccr_p[channel]) + 1)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_TIMCAP_USE_TIM1 && !defined(__DOXYGEN__)
extern TIMCAPDriver TIMCAPD1;
#endif

#if STM32_TIMCAP_USE_TIM2 && !defined(__DOXYGEN__)
extern TIMCAPDriver TIMCAPD2;
#endif

#if STM32_TIMCAP_USE_TIM3 && !defined(__DOXYGEN__)
extern TIMCAPDriver TIMCAPD3;
#endif

#if STM32_TIMCAP_USE_TIM4 && !defined(__DOXYGEN__)
extern TIMCAPDriver TIMCAPD4;
#endif

#if STM32_TIMCAP_USE_TIM5 && !defined(__DOXYGEN__)
extern TIMCAPDriver TIMCAPD5;
#endif

#if STM32_TIMCAP_USE_TIM8 && !defined(__DOXYGEN__)
extern TIMCAPDriver TIMCAPD8;
#endif

#if STM32_TIMCAP_USE_TIM9 && !defined(__DOXYGEN__)
extern TIMCAPDriver TIMCAPD9;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void timcap_lld_init(void);
  void timcap_lld_start(TIMCAPDriver *timcapp);
  void timcap_lld_stop(TIMCAPDriver *timcapp);
  void timcap_lld_enable(TIMCAPDriver *timcapp);
  void timcap_lld_disable(TIMCAPDriver *timcapp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_TIMCAP */

#endif /* _TIMCAP_LLD_H_ */

/** @} */
