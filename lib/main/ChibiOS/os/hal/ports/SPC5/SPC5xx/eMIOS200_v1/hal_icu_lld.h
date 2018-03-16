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
 * @file    SPC5xx/eMIOS200_v1/hal_icu_lld.h
 * @brief   SPC5xx low level icu driver header.
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
#if !defined(SPC5_ICU_USE_EMIOS_CH0) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH0              FALSE
#endif

/**
 * @brief   ICUD2 driver enable switch.
 * @details If set to @p TRUE the support for ICUD2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH1) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH1              FALSE
#endif

/**
 * @brief   ICUD3 driver enable switch.
 * @details If set to @p TRUE the support for ICUD3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH2) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH2              FALSE
#endif

/**
 * @brief   ICUD4 driver enable switch.
 * @details If set to @p TRUE the support for ICUD4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH3) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH3              FALSE
#endif

/**
 * @brief   ICUD5 driver enable switch.
 * @details If set to @p TRUE the support for ICUD5 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH4) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH4              FALSE
#endif

/**
 * @brief   ICUD6 driver enable switch.
 * @details If set to @p TRUE the support for ICUD6 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH5) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH5              FALSE
#endif

/**
 * @brief   ICUD7 driver enable switch.
 * @details If set to @p TRUE the support for ICUD7 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH6) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH6              FALSE
#endif

/**
 * @brief   ICUD8 driver enable switch.
 * @details If set to @p TRUE the support for ICUD8 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH7) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH7              FALSE
#endif

/**
 * @brief   ICUD9 driver enable switch.
 * @details If set to @p TRUE the support for ICUD9 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH8) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH8              FALSE
#endif

/**
 * @brief   ICUD10 driver enable switch.
 * @details If set to @p TRUE the support for ICUD10 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH9) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH9              FALSE
#endif

/**
 * @brief   ICUD11 driver enable switch.
 * @details If set to @p TRUE the support for ICUD11 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH10) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH10             FALSE
#endif

/**
 * @brief   ICUD12 driver enable switch.
 * @details If set to @p TRUE the support for ICUD12 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH11) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH11             FALSE
#endif

/**
 * @brief   ICUD13 driver enable switch.
 * @details If set to @p TRUE the support for ICUD13 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH12) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH12             FALSE
#endif

/**
 * @brief   ICUD14 driver enable switch.
 * @details If set to @p TRUE the support for ICUD14 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH13) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH13             FALSE
#endif

/**
 * @brief   ICUD15 driver enable switch.
 * @details If set to @p TRUE the support for ICUD15 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH14) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH14             FALSE
#endif

/**
 * @brief   ICUD16 driver enable switch.
 * @details If set to @p TRUE the support for ICUD16 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH15) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH15             FALSE
#endif

/**
 * @brief   ICUD17 driver enable switch.
 * @details If set to @p TRUE the support for ICUD17 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH16) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH16             FALSE
#endif

/**
 * @brief   ICUD18 driver enable switch.
 * @details If set to @p TRUE the support for ICUD18 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH17) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH17             FALSE
#endif

/**
 * @brief   ICUD19 driver enable switch.
 * @details If set to @p TRUE the support for ICUD19 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH18) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH18             FALSE
#endif

/**
 * @brief   ICUD20 driver enable switch.
 * @details If set to @p TRUE the support for ICUD20 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH19) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH19             FALSE
#endif

/**
 * @brief   ICUD21 driver enable switch.
 * @details If set to @p TRUE the support for ICUD21 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH20) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH20             FALSE
#endif

/**
 * @brief   ICUD22 driver enable switch.
 * @details If set to @p TRUE the support for ICUD22 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH21) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH21             FALSE
#endif

/**
 * @brief   ICUD23 driver enable switch.
 * @details If set to @p TRUE the support for ICUD23 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH22) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH22             FALSE
#endif

/**
 * @brief   ICUD24 driver enable switch.
 * @details If set to @p TRUE the support for ICUD24 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_ICU_USE_EMIOS_CH23) || defined(__DOXYGEN__)
#define SPC5_ICU_USE_EMIOS_CH23             FALSE
#endif

/**
 * @brief   ICUD1 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F0_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F0_PRIORITY         7
#endif

/**
 * @brief   ICUD2 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F1_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F1_PRIORITY         7
#endif

/**
 * @brief   ICUD3 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F2_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F2_PRIORITY         7
#endif

/**
 * @brief   ICUD4 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F3_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F3_PRIORITY         7
#endif

/**
 * @brief   ICUD5 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F4_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F4_PRIORITY         7
#endif

/**
 * @brief   ICUD6 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F5_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F5_PRIORITY         7
#endif

/**
 * @brief   ICUD7 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F6_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F6_PRIORITY         7
#endif

/**
 * @brief   ICUD8 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F7_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F7_PRIORITY         7
#endif

/**
 * @brief   ICUD9 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F8_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F8_PRIORITY         7
#endif

/**
 * @brief   ICUD10 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F9_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F9_PRIORITY         7
#endif

/**
 * @brief   ICUD11 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F10_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F10_PRIORITY        7
#endif

/**
 * @brief   ICUD12 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F11_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F11_PRIORITY        7
#endif

/**
 * @brief   ICUD13 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F12_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F12_PRIORITY        7
#endif

/**
 * @brief   ICUD14 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F13_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F13_PRIORITY        7
#endif

/**
 * @brief   ICUD15 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F14_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F14_PRIORITY        7
#endif

/**
 * @brief   ICUD16 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F15_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F15_PRIORITY        7
#endif

/**
 * @brief   ICUD17 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F16_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F16_PRIORITY        7
#endif

/**
 * @brief   ICUD18 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F17_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F17_PRIORITY        7
#endif

/**
 * @brief   ICUD19 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F18_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F18_PRIORITY        7
#endif

/**
 * @brief   ICUD20 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F19_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F19_PRIORITY        7
#endif

/**
 * @brief   ICUD21 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F20_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F20_PRIORITY        7
#endif

/**
 * @brief   ICUD22 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F21_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F21_PRIORITY        7
#endif

/**
 * @brief   ICUD23 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F22_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F22_PRIORITY        7
#endif

/**
 * @brief   ICUD24 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F23_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F23_PRIORITY        7
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !SPC5_HAS_EMIOS
#error "EMIOS not present in the selected device"
#endif

#define SPC5_ICU_USE_EMIOS                  (SPC5_ICU_USE_EMIOS_CH0  ||     \
                                             SPC5_ICU_USE_EMIOS_CH1  ||     \
                                             SPC5_ICU_USE_EMIOS_CH2  ||     \
                                             SPC5_ICU_USE_EMIOS_CH3  ||     \
                                             SPC5_ICU_USE_EMIOS_CH4  ||     \
                                             SPC5_ICU_USE_EMIOS_CH5  ||     \
                                             SPC5_ICU_USE_EMIOS_CH6  ||     \
                                             SPC5_ICU_USE_EMIOS_CH7  ||     \
                                             SPC5_ICU_USE_EMIOS_CH8  ||     \
                                             SPC5_ICU_USE_EMIOS_CH9  ||     \
                                             SPC5_ICU_USE_EMIOS_CH10 ||     \
                                             SPC5_ICU_USE_EMIOS_CH11 ||     \
                                             SPC5_ICU_USE_EMIOS_CH12 ||     \
                                             SPC5_ICU_USE_EMIOS_CH13 ||     \
                                             SPC5_ICU_USE_EMIOS_CH14 ||     \
                                             SPC5_ICU_USE_EMIOS_CH15 ||     \
                                             SPC5_ICU_USE_EMIOS_CH16 ||     \
                                             SPC5_ICU_USE_EMIOS_CH17 ||     \
                                             SPC5_ICU_USE_EMIOS_CH18 ||     \
                                             SPC5_ICU_USE_EMIOS_CH19 ||     \
                                             SPC5_ICU_USE_EMIOS_CH20 ||     \
                                             SPC5_ICU_USE_EMIOS_CH21 ||     \
                                             SPC5_ICU_USE_EMIOS_CH22 ||     \
                                             SPC5_ICU_USE_EMIOS_CH23)

#if !SPC5_ICU_USE_EMIOS
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
typedef uint32_t icucnt_t;

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
  volatile vuint32_t        *wccrp;
  /**
   * @brief CCR register used for period capture.
   */
  volatile vuint32_t        *pccrp;
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

#if SPC5_ICU_USE_EMIOS_CH0 && !defined(__DOXYGEN__)
extern ICUDriver ICUD1;
#endif

#if SPC5_ICU_USE_EMIOS_CH1 && !defined(__DOXYGEN__)
extern ICUDriver ICUD2;
#endif

#if SPC5_ICU_USE_EMIOS_CH2 && !defined(__DOXYGEN__)
extern ICUDriver ICUD3;
#endif

#if SPC5_ICU_USE_EMIOS_CH3 && !defined(__DOXYGEN__)
extern ICUDriver ICUD4;
#endif

#if SPC5_ICU_USE_EMIOS_CH4 && !defined(__DOXYGEN__)
extern ICUDriver ICUD5;
#endif

#if SPC5_ICU_USE_EMIOS_CH5 && !defined(__DOXYGEN__)
extern ICUDriver ICUD6;
#endif

#if SPC5_ICU_USE_EMIOS_CH6 && !defined(__DOXYGEN__)
extern ICUDriver ICUD7;
#endif

#if SPC5_ICU_USE_EMIOS_CH7 && !defined(__DOXYGEN__)
extern ICUDriver ICUD8;
#endif

#if SPC5_ICU_USE_EMIOS_CH8 && !defined(__DOXYGEN__)
extern ICUDriver ICUD9;
#endif

#if SPC5_ICU_USE_EMIOS_CH9 && !defined(__DOXYGEN__)
extern ICUDriver ICUD10;
#endif

#if SPC5_ICU_USE_EMIOS_CH10 && !defined(__DOXYGEN__)
extern ICUDriver ICUD11;
#endif

#if SPC5_ICU_USE_EMIOS_CH11 && !defined(__DOXYGEN__)
extern ICUDriver ICUD12;
#endif

#if SPC5_ICU_USE_EMIOS_CH12 && !defined(__DOXYGEN__)
extern ICUDriver ICUD13;
#endif

#if SPC5_ICU_USE_EMIOS_CH13 && !defined(__DOXYGEN__)
extern ICUDriver ICUD14;
#endif

#if SPC5_ICU_USE_EMIOS_CH14 && !defined(__DOXYGEN__)
extern ICUDriver ICUD15;
#endif

#if SPC5_ICU_USE_EMIOS_CH15 && !defined(__DOXYGEN__)
extern ICUDriver ICUD16;
#endif

#if SPC5_ICU_USE_EMIOS_CH16 && !defined(__DOXYGEN__)
extern ICUDriver ICUD17;
#endif

#if SPC5_ICU_USE_EMIOS_CH17 && !defined(__DOXYGEN__)
extern ICUDriver ICUD18;
#endif

#if SPC5_ICU_USE_EMIOS_CH18 && !defined(__DOXYGEN__)
extern ICUDriver ICUD19;
#endif

#if SPC5_ICU_USE_EMIOS_CH19 && !defined(__DOXYGEN__)
extern ICUDriver ICUD20;
#endif

#if SPC5_ICU_USE_EMIOS_CH20 && !defined(__DOXYGEN__)
extern ICUDriver ICUD21;
#endif

#if SPC5_ICU_USE_EMIOS_CH21 && !defined(__DOXYGEN__)
extern ICUDriver ICUD22;
#endif

#if SPC5_ICU_USE_EMIOS_CH22 && !defined(__DOXYGEN__)
extern ICUDriver ICUD23;
#endif

#if SPC5_ICU_USE_EMIOS_CH23 && !defined(__DOXYGEN__)
extern ICUDriver ICUD24;
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
