/*
    SPC5 HAL - Copyright (C) 2013 STMicroelectronics

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
 * @file    SPC5xx/eMIOS_v1/hal_icu_lld.h
 * @brief   SPC5xx low level ICU driver header.
 *
 * @addtogroup ICU
 * @{
 */

#ifndef HAL_ICU_LLD_H
#define HAL_ICU_LLD_H

#if HAL_USE_ICU || defined(__DOXYGEN__)

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
 * @brief   ICUD1 driver enable switch.
 * @details If set to @p TRUE the support for ICUD1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS0_CH0) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS0_CH0             FALSE
#endif

/**
 * @brief   ICUD2 driver enable switch.
 * @details If set to @p TRUE the support for ICUD2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS0_CH1) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS0_CH1             FALSE
#endif

/**
 * @brief   ICUD3 driver enable switch.
 * @details If set to @p TRUE the support for ICUD3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS0_CH2) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS0_CH2             FALSE
#endif

/**
 * @brief   ICUD4 driver enable switch.
 * @details If set to @p TRUE the support for ICUD4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS0_CH3) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS0_CH3             FALSE
#endif

/**
 * @brief   ICUD5 driver enable switch.
 * @details If set to @p TRUE the support for ICUD5 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS0_CH4) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS0_CH4             FALSE
#endif

/**
 * @brief   ICUD6 driver enable switch.
 * @details If set to @p TRUE the support for ICUD6 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS0_CH5) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS0_CH5             FALSE
#endif

/**
 * @brief   ICUD7 driver enable switch.
 * @details If set to @p TRUE the support for ICUD7 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS0_CH6) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS0_CH6             FALSE
#endif

/**
 * @brief   ICUD8 driver enable switch.
 * @details If set to @p TRUE the support for ICUD8 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS0_CH7) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS0_CH7             FALSE
#endif

/**
 * @brief   ICUD9 driver enable switch.
 * @details If set to @p TRUE the support for ICUD9 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS0_CH24) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS0_CH24            FALSE
#endif

/**
 * @brief   ICUD1 and ICUD2 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS0_GFR_F0F1_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_GFR_F0F1_PRIORITY       7
#endif

/**
 * @brief   ICUD3 and ICUD4 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS0_GFR_F2F3_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_GFR_F2F3_PRIORITY       7
#endif

/**
 * @brief   ICUD5 and ICUD6 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS0_GFR_F4F5_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_GFR_F4F5_PRIORITY       7
#endif

/**
 * @brief   ICUD7 and ICUD8 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS0_GFR_F6F7_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_GFR_F6F7_PRIORITY       7
#endif

/**
 * @brief   ICUD9 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS0_GFR_F24F25_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_GFR_F24F25_PRIORITY     7
#endif

/**
 * @brief   ICUD10 driver enable switch.
 * @details If set to @p TRUE the support for ICUD10 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS1_CH24) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS1_CH24            FALSE
#endif

/**
 * @brief   ICUD10 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS1_GFR_F24F25_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_GFR_F24F25_PRIORITY     7
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#define SPC5_ICU_USE_EMIOS0                 (SPC5_ICU_USE_EMIOS0_CH0 ||     \
                                             SPC5_ICU_USE_EMIOS0_CH1 ||     \
                                             SPC5_ICU_USE_EMIOS0_CH2 ||     \
                                             SPC5_ICU_USE_EMIOS0_CH3 ||     \
                                             SPC5_ICU_USE_EMIOS0_CH4 ||     \
                                             SPC5_ICU_USE_EMIOS0_CH5 ||     \
                                             SPC5_ICU_USE_EMIOS0_CH6 ||     \
                                             SPC5_ICU_USE_EMIOS0_CH7 ||     \
                                             SPC5_ICU_USE_EMIOS0_CH24)

#define SPC5_ICU_USE_EMIOS1                  SPC5_ICU_USE_EMIOS1_CH24

#if !SPC5_HAS_EMIOS0 && SPC5_ICU_USE_EMIOS0
#error "EMIOS0 not present in the selected device"
#endif

#if !SPC5_HAS_EMIOS1 && SPC5_ICU_USE_EMIOS1
#error "EMIOS1 not present in the selected device"
#endif

#if !SPC5_ICU_USE_EMIOS0 && !SPC5_ICU_USE_EMIOS1
#error "ICU driver activated but no Channels assigned"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief ICU driver mode.
 */
typedef enum {
  ICU_INPUT_ACTIVE_HIGH = 0,        /**< Trigger on rising edge.            */
  ICU_INPUT_ACTIVE_LOW = 1,         /**< Trigger on falling edge.           */
} icumode_t;

/**
 * @brief   ICU frequency type.
 */
typedef uint32_t icufreq_t;

/**
 * @brief   ICU counter type.
 */
typedef uint16_t icucnt_t;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief   Driver mode.
   */
  icumode_t     mode;
  /**
   * @brief   Timer clock in Hz.
   * @note    The low level can use assertions in order to catch invalid
   *          frequency specifications.
   */
  icufreq_t     frequency;
  /**
   * @brief   Callback for pulse width measurement.
   */
  icucallback_t width_cb;
  /**
   * @brief   Callback for cycle period measurement.
   */
  icucallback_t period_cb;
  /**
   * @brief   Callback for timer overflow.
   */
  icucallback_t overflow_cb;
  /* End of the mandatory fields.*/
} ICUConfig;

/**
 * @brief   Structure representing an ICU driver.
 */
struct ICUDriver {
  /**
   * @brief Driver state.
   */
  volatile icustate_t       state;
  /**
   * @brief eMIOSx channel number.
   */
  uint32_t                  ch_number;
  /**
   * @brief Current configuration data.
   */
  const ICUConfig           *config;
  /**
   * @brief CH Counter clock.
   */
  uint32_t clock;
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the eMIOSx registers block.
   */
  volatile struct EMIOS_tag *emiosp;
  /**
   * @brief CCR register used for width capture.
   */
  volatile vint16_t        *wccrp;
  /**
   * @brief CCR register used for period capture.
   */
  volatile vint16_t        *pccrp;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Returns the width of the latest pulse.
 * @details The pulse width is defined as number of ticks between the start
 *          edge and the stop edge.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 * @return              The number of ticks.
 *
 * @notapi
 */
#define icu_lld_get_width(icup) (*((icup)->wccrp) + 1)

/**
 * @brief   Returns the width of the latest cycle.
 * @details The cycle width is defined as number of ticks between a start
 *          edge and the next start edge.
 *
 * @param[in] icup      pointer to the @p ICUDriver object
 * @return              The number of ticks.
 *
 * @notapi
 */
#define icu_lld_get_period(icup) (*((icup)->pccrp) + 1)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if SPC5_ICU_USE_EMIOS0_CH0 && !defined(__DOXYGEN__)
extern ICUDriver ICUD1;
#endif

#if SPC5_ICU_USE_EMIOS0_CH1 && !defined(__DOXYGEN__)
extern ICUDriver ICUD2;
#endif

#if SPC5_ICU_USE_EMIOS0_CH2 && !defined(__DOXYGEN__)
extern ICUDriver ICUD3;
#endif

#if SPC5_ICU_USE_EMIOS0_CH3 && !defined(__DOXYGEN__)
extern ICUDriver ICUD4;
#endif

#if SPC5_ICU_USE_EMIOS0_CH4 && !defined(__DOXYGEN__)
extern ICUDriver ICUD5;
#endif

#if SPC5_ICU_USE_EMIOS0_CH5 && !defined(__DOXYGEN__)
extern ICUDriver ICUD6;
#endif

#if SPC5_ICU_USE_EMIOS0_CH6 && !defined(__DOXYGEN__)
extern ICUDriver ICUD7;
#endif

#if SPC5_ICU_USE_EMIOS0_CH7 && !defined(__DOXYGEN__)
extern ICUDriver ICUD8;
#endif

#if SPC5_ICU_USE_EMIOS0_CH24 && !defined(__DOXYGEN__)
extern ICUDriver ICUD9;
#endif

#if SPC5_ICU_USE_EMIOS1_CH24 && !defined(__DOXYGEN__)
extern ICUDriver ICUD10;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void icu_lld_init(void);
  void icu_lld_start(ICUDriver *icup);
  void icu_lld_stop(ICUDriver *icup);
  void icu_lld_enable(ICUDriver *icup);
  void icu_lld_disable(ICUDriver *icup);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_ICU */

#endif /* HAL_ICU_LLD_H */

/** @} */
