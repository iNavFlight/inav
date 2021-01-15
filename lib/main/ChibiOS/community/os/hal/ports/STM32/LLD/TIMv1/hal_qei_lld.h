/*
    ChibiOS - Copyright (C) 2006..2016 Martino Migliavacca

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
 * @file    TIMv1/hal_qei_lld.h
 * @brief   STM32 QEI subsystem low level driver header.
 *
 * @addtogroup QEI
 * @{
 */

#ifndef HAL_QEI_LLD_H
#define HAL_QEI_LLD_H

#if (HAL_USE_QEI == TRUE) || defined(__DOXYGEN__)

#include "stm32_tim.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief Mininum usable value for defining counter underflow
 */
#define QEI_COUNT_MIN (0)

/**
 * @brief Maximum usable value for defining counter overflow
 */
#define QEI_COUNT_MAX (65535)

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   QEID1 driver enable switch.
 * @details If set to @p TRUE the support for QEID1 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_QEI_USE_TIM1) || defined(__DOXYGEN__)
#define STM32_QEI_USE_TIM1                  FALSE
#endif

/**
 * @brief   QEID2 driver enable switch.
 * @details If set to @p TRUE the support for QEID2 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_QEI_USE_TIM2) || defined(__DOXYGEN__)
#define STM32_QEI_USE_TIM2                  FALSE
#endif

/**
 * @brief   QEID3 driver enable switch.
 * @details If set to @p TRUE the support for QEID3 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_QEI_USE_TIM3) || defined(__DOXYGEN__)
#define STM32_QEI_USE_TIM3                  FALSE
#endif

/**
 * @brief   QEID4 driver enable switch.
 * @details If set to @p TRUE the support for QEID4 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_QEI_USE_TIM4) || defined(__DOXYGEN__)
#define STM32_QEI_USE_TIM4                  FALSE
#endif

/**
 * @brief   QEID5 driver enable switch.
 * @details If set to @p TRUE the support for QEID5 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_QEI_USE_TIM5) || defined(__DOXYGEN__)
#define STM32_QEI_USE_TIM5                  FALSE
#endif

/**
 * @brief   QEID8 driver enable switch.
 * @details If set to @p TRUE the support for QEID8 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(STM32_QEI_USE_TIM8) || defined(__DOXYGEN__)
#define STM32_QEI_USE_TIM8                  FALSE
#endif

/**
 * @brief   QEID1 interrupt priority level setting.
 */
#if !defined(STM32_QEI_TIM1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_QEI_TIM1_IRQ_PRIORITY         7
#endif

/**
 * @brief   QEID2 interrupt priority level setting.
 */
#if !defined(STM32_QEI_TIM2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_QEI_TIM2_IRQ_PRIORITY         7
#endif

/**
 * @brief   QEID3 interrupt priority level setting.
 */
#if !defined(STM32_QEI_TIM3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_QEI_TIM3_IRQ_PRIORITY         7
#endif

/**
 * @brief   QEID4 interrupt priority level setting.
 */
#if !defined(STM32_QEI_TIM4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_QEI_TIM4_IRQ_PRIORITY         7
#endif

/**
 * @brief   QEID5 interrupt priority level setting.
 */
#if !defined(STM32_QEI_TIM5_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_QEI_TIM5_IRQ_PRIORITY         7
#endif

/**
 * @brief   QEID8 interrupt priority level setting.
 */
#if !defined(STM32_QEI_TIM8_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define STM32_QEI_TIM8_IRQ_PRIORITY         7
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if STM32_QEI_USE_TIM1 && !STM32_HAS_TIM1
#error "TIM1 not present in the selected device"
#endif

#if STM32_QEI_USE_TIM2 && !STM32_HAS_TIM2
#error "TIM2 not present in the selected device"
#endif

#if STM32_QEI_USE_TIM3 && !STM32_HAS_TIM3
#error "TIM3 not present in the selected device"
#endif

#if STM32_QEI_USE_TIM4 && !STM32_HAS_TIM4
#error "TIM4 not present in the selected device"
#endif

#if STM32_QEI_USE_TIM5 && !STM32_HAS_TIM5
#error "TIM5 not present in the selected device"
#endif

#if STM32_QEI_USE_TIM8 && !STM32_HAS_TIM8
#error "TIM8 not present in the selected device"
#endif

#if !STM32_QEI_USE_TIM1 && !STM32_QEI_USE_TIM2 &&                           \
    !STM32_QEI_USE_TIM3 && !STM32_QEI_USE_TIM4 &&                           \
    !STM32_QEI_USE_TIM5 && !STM32_QEI_USE_TIM8
#error "QEI driver activated but no TIM peripheral assigned"
#endif

/* Checks on allocation of TIMx units.*/
#if STM32_QEI_USE_TIM1
#if defined(STM32_TIM1_IS_USED)
#error "QEID1 requires TIM1 but the timer is already used"
#else
#define STM32_TIM1_IS_USED
#endif
#endif

#if STM32_QEI_USE_TIM2
#if defined(STM32_TIM2_IS_USED)
#error "QEID2 requires TIM2 but the timer is already used"
#else
#define STM32_TIM2_IS_USED
#endif
#endif

#if STM32_QEI_USE_TIM3
#if defined(STM32_TIM3_IS_USED)
#error "QEID3 requires TIM3 but the timer is already used"
#else
#define STM32_TIM3_IS_USED
#endif
#endif

#if STM32_QEI_USE_TIM4
#if defined(STM32_TIM4_IS_USED)
#error "QEID4 requires TIM4 but the timer is already used"
#else
#define STM32_TIM4_IS_USED
#endif
#endif

#if STM32_QEI_USE_TIM5
#if defined(STM32_TIM5_IS_USED)
#error "QEID5 requires TIM5 but the timer is already used"
#else
#define STM32_TIM5_IS_USED
#endif
#endif

#if STM32_QEI_USE_TIM8
#if defined(STM32_TIM8_IS_USED)
#error "QEID8 requires TIM8 but the timer is already used"
#else
#define STM32_TIM8_IS_USED
#endif
#endif

/* IRQ priority checks.*/
#if STM32_QEI_USE_TIM1 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_QEI_TIM1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM1"
#endif

#if STM32_QEI_USE_TIM2 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_QEI_TIM2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM2"
#endif

#if STM32_QEI_USE_TIM3 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_QEI_TIM3_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM3"
#endif

#if STM32_QEI_USE_TIM4 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_QEI_TIM4_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM4"
#endif

#if STM32_QEI_USE_TIM5 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_QEI_TIM5_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM5"
#endif

#if STM32_QEI_USE_TIM8 &&                                                   \
    !OSAL_IRQ_IS_VALID_PRIORITY(STM32_QEI_TIM8_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to TIM8"
#endif

#if QEI_USE_OVERFLOW_DISCARD
#error "QEI_USE_OVERFLOW_DISCARD not supported by this driver"
#endif

#if QEI_USE_OVERFLOW_MINMAX
#error "QEI_USE_OVERFLOW_MINMAX not supported by this driver"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   QEI count mode.
 */
typedef enum {
  QEI_MODE_QUADRATURE = 0,          /**< Quadrature encoder mode.           */
  QEI_MODE_DIRCLOCK = 1,            /**< Direction/Clock mode.              */
} qeimode_t;

/**
 * @brief   QEI resolution.
 */
typedef enum {
  QEI_SINGLE_EDGE = 0,        /**< Count only on edges from first channel.  */
  QEI_BOTH_EDGES = 1,         /**< Count on both edges (resolution doubles).*/
} qeiresolution_t;

/**
 * @brief   QEI direction inversion.
 */
typedef enum {
  QEI_DIRINV_FALSE = 0,             /**< Do not invert counter direction.   */
  QEI_DIRINV_TRUE = 1,              /**< Invert counter direction.          */
} qeidirinv_t;

/**
 * @brief   QEI counter type.
 */
typedef int16_t qeicnt_t;

/**
 * @brief   QEI delta type.
 */
typedef int32_t qeidelta_t;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief   Count mode.
   */
  qeimode_t                 mode;
  /**
   * @brief   Resolution.
   */
  qeiresolution_t           resolution;
  /**
   * @brief   Direction inversion.
   */
  qeidirinv_t               dirinv;
  /**
   * @brief   Handling of counter overflow/underflow
   *
   * @details When overflow occurs, the counter value is updated
   *          according to:
   *            - QEI_OVERFLOW_DISCARD:
   *                discard the update value, counter doesn't change
   */
  qeioverflow_t             overflow;
  /**
   * @brief   Min count value.
   *
   * @note    If min == max, then QEI_COUNT_MIN is used.
   *
   * @note    Only min set to 0 / QEI_COUNT_MIN is supported.
   */
  qeicnt_t                  min;
  /**
   * @brief   Max count value.
   *
   * @note    If min == max, then QEI_COUNT_MAX is used.
   *
   * @note    Only max set to 0 / QEI_COUNT_MAX is supported.
   */
  qeicnt_t                  max;
  /**
    * @brief  Notify of value change
    *
    * @note   Called from ISR context.
    */
  qeicallback_t             notify_cb;
  /**
   * @brief   Notify of overflow
   *
   * @note    Overflow notification is performed after
   *          value changed notification.
   * @note    Called from ISR context.
   */
  void (*overflow_cb)(QEIDriver *qeip, qeidelta_t delta);
  /* End of the mandatory fields.*/
} QEIConfig;

/**
 * @brief   Structure representing an QEI driver.
 */
struct QEIDriver {
  /**
   * @brief Driver state.
   */
  qeistate_t                state;
  /**
   * @brief Last count value.
   */
  qeicnt_t                  last;
  /**
   * @brief Current configuration data.
   */
  const QEIConfig           *config;
#if defined(QEI_DRIVER_EXT_FIELDS)
  QEI_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the TIMx registers block.
   */
  stm32_tim_t               *tim;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Returns the counter value.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 * @return              The current counter value.
 *
 * @notapi
 */
#define qei_lld_get_count(qeip) ((qeip)->tim->CNT)

/**
 * @brief   Set the counter value.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 * @param[in] qeip      counter value
 *
 * @notapi
 */
#define qei_lld_set_count(qeip, value) ((qeip)->tim->CNT = (value))

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_QEI_USE_TIM1 && !defined(__DOXYGEN__)
extern QEIDriver QEID1;
#endif

#if STM32_QEI_USE_TIM2 && !defined(__DOXYGEN__)
extern QEIDriver QEID2;
#endif

#if STM32_QEI_USE_TIM3 && !defined(__DOXYGEN__)
extern QEIDriver QEID3;
#endif

#if STM32_QEI_USE_TIM4 && !defined(__DOXYGEN__)
extern QEIDriver QEID4;
#endif

#if STM32_QEI_USE_TIM5 && !defined(__DOXYGEN__)
extern QEIDriver QEID5;
#endif

#if STM32_QEI_USE_TIM8 && !defined(__DOXYGEN__)
extern QEIDriver QEID8;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void qei_lld_init(void);
  void qei_lld_start(QEIDriver *qeip);
  void qei_lld_stop(QEIDriver *qeip);
  void qei_lld_enable(QEIDriver *qeip);
  void qei_lld_disable(QEIDriver *qeip);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_QEI */

#endif /* HAL_QEI_LLD_H */

/** @} */
