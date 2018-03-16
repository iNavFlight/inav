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
 * @file    SPC5xx/eMIOS200_v1/hal_pwm_lld.h
 * @brief   SPC5xx low level pwm driver header.
 *
 * @addtogroup PWM
 * @{
 */

#ifndef HAL_PWM_LLD_H
#define HAL_PWM_LLD_H

#if HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Number of PWM channels per PWM driver.
 */
#define PWM_CHANNELS                            1

/**
 * @brief   Edge-Aligned PWM functional mode.
 * @note    This is an SPC5-specific setting.
 */
#define PWM_ALIGN_EDGE                          0x00

/**
 * @brief   Center-Aligned PWM functional mode.
 * @note    This is an SPC5-specific setting.
 */
#define PWM_ALIGN_CENTER                        0x01

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   PWMD1 driver enable switch.
 * @details If set to @p TRUE the support for PWMD1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH0) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH0              FALSE
#endif

/**
 * @brief   PWMD2 driver enable switch.
 * @details If set to @p TRUE the support for PWMD2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH1) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH1              FALSE
#endif

/**
 * @brief   PWMD3 driver enable switch.
 * @details If set to @p TRUE the support for PWMD3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH2) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH2              FALSE
#endif

/**
 * @brief   PWMD4 driver enable switch.
 * @details If set to @p TRUE the support for PWMD4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH3) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH3              FALSE
#endif

/**
 * @brief   PWMD5 driver enable switch.
 * @details If set to @p TRUE the support for PWMD5 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH4) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH4              FALSE
#endif

/**
 * @brief   PWMD6 driver enable switch.
 * @details If set to @p TRUE the support for PWMD6 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH5) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH5              FALSE
#endif

/**
 * @brief   PWMD7 driver enable switch.
 * @details If set to @p TRUE the support for PWMD7 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH6) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH6              FALSE
#endif

/**
 * @brief   PWMD8 driver enable switch.
 * @details If set to @p TRUE the support for PWMD8 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH7) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH7              FALSE
#endif

/**
 * @brief   PWMD9 driver enable switch.
 * @details If set to @p TRUE the support for PWMD9 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH8) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH8              FALSE
#endif

/**
 * @brief   PWMD10 driver enable switch.
 * @details If set to @p TRUE the support for PWMD10 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH9) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH9              FALSE
#endif

/**
 * @brief   PWMD11 driver enable switch.
 * @details If set to @p TRUE the support for PWMD11 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH10) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH10             FALSE
#endif

/**
 * @brief   PWMD12 driver enable switch.
 * @details If set to @p TRUE the support for PWMD12 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH11) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH11             FALSE
#endif

/**
 * @brief   PWMD13 driver enable switch.
 * @details If set to @p TRUE the support for PWMD13 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH12) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH12             FALSE
#endif

/**
 * @brief   PWMD14 driver enable switch.
 * @details If set to @p TRUE the support for PWMD14 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH13) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH13             FALSE
#endif

/**
 * @brief   PWMD15 driver enable switch.
 * @details If set to @p TRUE the support for PWMD15 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH14) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH14             FALSE
#endif

/**
 * @brief   PWMD16 driver enable switch.
 * @details If set to @p TRUE the support for PWMD16 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH15) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH15             FALSE
#endif

/**
 * @brief   PWMD17 driver enable switch.
 * @details If set to @p TRUE the support for PWMD17 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH16) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH16             FALSE
#endif

/**
 * @brief   PWMD18 driver enable switch.
 * @details If set to @p TRUE the support for PWMD18 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH17) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH17             FALSE
#endif

/**
 * @brief   PWMD19 driver enable switch.
 * @details If set to @p TRUE the support for PWMD19 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH18) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH18             FALSE
#endif

/**
 * @brief   PWMD20 driver enable switch.
 * @details If set to @p TRUE the support for PWMD20 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH19) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH19             FALSE
#endif

/**
 * @brief   PWMD21 driver enable switch.
 * @details If set to @p TRUE the support for PWMD21 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH20) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH20             FALSE
#endif

/**
 * @brief   PWMD22 driver enable switch.
 * @details If set to @p TRUE the support for PWMD22 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH21) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH21             FALSE
#endif

/**
 * @brief   PWMD23 driver enable switch.
 * @details If set to @p TRUE the support for PWMD23 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH22) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH22             FALSE
#endif

/**
 * @brief   PWMD24 driver enable switch.
 * @details If set to @p TRUE the support for PWMD24 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS_CH23) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS_CH23             FALSE
#endif

/**
 * @brief   PWMD1 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F0_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F0_PRIORITY         7
#endif

/**
 * @brief   PWMD2 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F1_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F1_PRIORITY         7
#endif

/**
 * @brief   PWMD3 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F2_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F2_PRIORITY         7
#endif

/**
 * @brief   PWMD4 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F3_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F3_PRIORITY         7
#endif

/**
 * @brief   PWMD5 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F4_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F4_PRIORITY         7
#endif

/**
 * @brief   PWMD6 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F5_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F5_PRIORITY         7
#endif

/**
 * @brief   PWMD7 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F6_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F6_PRIORITY         7
#endif

/**
 * @brief   PWMD8 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F7_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F7_PRIORITY         7
#endif

/**
 * @brief   PWMD9 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F8_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F8_PRIORITY         7
#endif

/**
 * @brief   PWMD10 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F9_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F9_PRIORITY         7
#endif

/**
 * @brief   PWMD11 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F10_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F10_PRIORITY        7
#endif

/**
 * @brief   PWMD12 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F11_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F11_PRIORITY        7
#endif

/**
 * @brief   PWMD13 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F12_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F12_PRIORITY        7
#endif

/**
 * @brief   PWMD14 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F13_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F13_PRIORITY        7
#endif

/**
 * @brief   PWMD15 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F14_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F14_PRIORITY        7
#endif

/**
 * @brief   PWMD16 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F15_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F15_PRIORITY        7
#endif

/**
 * @brief   PWMD17 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F16_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F16_PRIORITY        7
#endif

/**
 * @brief   PWMD18 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F17_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F17_PRIORITY        7
#endif

/**
 * @brief   PWMD19 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F18_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F18_PRIORITY        7
#endif

/**
 * @brief   PWMD20 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F19_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F19_PRIORITY        7
#endif

/**
 * @brief   PWMD21 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F20_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F20_PRIORITY        7
#endif

/**
 * @brief   PWMD22 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F21_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F21_PRIORITY        7
#endif

/**
 * @brief   PWMD23 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS_FLAG_F22_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS_FLAG_F22_PRIORITY        7
#endif

/**
 * @brief   PWMD24 interrupt priority level setting.
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

#define SPC5_PWM_USE_EMIOS                  (SPC5_PWM_USE_EMIOS_CH0  ||    \
                                             SPC5_PWM_USE_EMIOS_CH1  ||    \
                                             SPC5_PWM_USE_EMIOS_CH2  ||    \
                                             SPC5_PWM_USE_EMIOS_CH3  ||    \
                                             SPC5_PWM_USE_EMIOS_CH4  ||    \
                                             SPC5_PWM_USE_EMIOS_CH5  ||    \
                                             SPC5_PWM_USE_EMIOS_CH6  ||    \
                                             SPC5_PWM_USE_EMIOS_CH7  ||    \
                                             SPC5_PWM_USE_EMIOS_CH8  ||    \
                                             SPC5_PWM_USE_EMIOS_CH9  ||    \
                                             SPC5_PWM_USE_EMIOS_CH10 ||    \
                                             SPC5_PWM_USE_EMIOS_CH11 ||    \
                                             SPC5_PWM_USE_EMIOS_CH12 ||    \
                                             SPC5_PWM_USE_EMIOS_CH13 ||    \
                                             SPC5_PWM_USE_EMIOS_CH14 ||    \
                                             SPC5_PWM_USE_EMIOS_CH15 ||    \
                                             SPC5_PWM_USE_EMIOS_CH16 ||    \
                                             SPC5_PWM_USE_EMIOS_CH17 ||    \
                                             SPC5_PWM_USE_EMIOS_CH18 ||    \
                                             SPC5_PWM_USE_EMIOS_CH19 ||    \
                                             SPC5_PWM_USE_EMIOS_CH20 ||    \
                                             SPC5_PWM_USE_EMIOS_CH21 ||    \
                                             SPC5_PWM_USE_EMIOS_CH22 ||    \
                                             SPC5_PWM_USE_EMIOS_CH23)

#if !SPC5_PWM_USE_EMIOS
#error "PWM driver activated but no Channels assigned"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   PWM mode type.
 */
typedef uint32_t pwmmode_t;

/**
 * @brief   PWM channel type.
 */
typedef uint8_t pwmchannel_t;

/**
 * @brief   PWM counter type.
 */
typedef uint32_t pwmcnt_t;

/**
 * @brief   PWM driver channel configuration structure.
 * @note    Some architectures may not be able to support the channel mode
 *          or the callback, in this case the fields are ignored.
 */
typedef struct {
  /**
   * @brief Channel active logic level.
   */
  pwmmode_t                 mode;
  /**
   * @brief Channel callback pointer.
   * @note  This callback is invoked on the channel compare event. If set to
   *        @p NULL then the callback is disabled.
   */
  pwmcallback_t             callback;
  /* End of the mandatory fields.*/
} PWMChannelConfig;

/**
 * @brief   Driver configuration structure.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct {
  /**
   * @brief Timer clock in Hz.
   * @note  The low level can use assertions in order to catch invalid
   *        frequency specifications.
   */
  uint32_t                  frequency;
  /**
   * @brief PWM period in ticks.
   * @note  The low level can use assertions in order to catch invalid
   *        period specifications.
   */
  pwmcnt_t                  period;
  /**
   * @brief Periodic callback pointer.
   * @note  This callback is invoked on PWM counter reset. If set to
   *        @p NULL then the callback is disabled.
   */
  pwmcallback_t             callback;
  /**
   * @brief Channels configurations.
   */
  PWMChannelConfig          channels[PWM_CHANNELS];
  /* End of the mandatory fields.*/
  /**
   * @brief PWM functional mode.
   */
  pwmmode_t                 mode;
} PWMConfig;

/**
 * @brief   Structure representing an PWM driver.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
struct PWMDriver {
  /**
   * @brief Driver state.
   */
  volatile pwmstate_t       state;
  /**
   * @brief eMIOSx channel number.
   */
  uint8_t                   ch_number;
  /**
   * @brief Current configuration data.
   */
  const PWMConfig           *config;
  /**
   * @brief Current PWM period in ticks.
   */
  pwmcnt_t                  period;
#if defined(PWM_DRIVER_EXT_FIELDS)
  PWM_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the eMIOSx registers block.
   */
  volatile struct EMIOS_tag *emiosp;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if SPC5_PWM_USE_EMIOS_CH0 && !defined(__DOXYGEN__)
extern PWMDriver PWMD1;
#endif

#if SPC5_PWM_USE_EMIOS_CH1 && !defined(__DOXYGEN__)
extern PWMDriver PWMD2;
#endif

#if SPC5_PWM_USE_EMIOS_CH2 && !defined(__DOXYGEN__)
extern PWMDriver PWMD3;
#endif

#if SPC5_PWM_USE_EMIOS_CH3 && !defined(__DOXYGEN__)
extern PWMDriver PWMD4;
#endif

#if SPC5_PWM_USE_EMIOS_CH4 && !defined(__DOXYGEN__)
extern PWMDriver PWMD5;
#endif

#if SPC5_PWM_USE_EMIOS_CH5 && !defined(__DOXYGEN__)
extern PWMDriver PWMD6;
#endif

#if SPC5_PWM_USE_EMIOS_CH6 && !defined(__DOXYGEN__)
extern PWMDriver PWMD7;
#endif

#if SPC5_PWM_USE_EMIOS_CH7 && !defined(__DOXYGEN__)
extern PWMDriver PWMD8;
#endif

#if SPC5_PWM_USE_EMIOS_CH8 && !defined(__DOXYGEN__)
extern PWMDriver PWMD9;
#endif

#if SPC5_PWM_USE_EMIOS_CH9 && !defined(__DOXYGEN__)
extern PWMDriver PWMD10;
#endif

#if SPC5_PWM_USE_EMIOS_CH10 && !defined(__DOXYGEN__)
extern PWMDriver PWMD11;
#endif

#if SPC5_PWM_USE_EMIOS_CH11 && !defined(__DOXYGEN__)
extern PWMDriver PWMD12;
#endif

#if SPC5_PWM_USE_EMIOS_CH12 && !defined(__DOXYGEN__)
extern PWMDriver PWMD13;
#endif

#if SPC5_PWM_USE_EMIOS_CH13 && !defined(__DOXYGEN__)
extern PWMDriver PWMD14;
#endif

#if SPC5_PWM_USE_EMIOS_CH14 && !defined(__DOXYGEN__)
extern PWMDriver PWMD15;
#endif

#if SPC5_PWM_USE_EMIOS_CH15 && !defined(__DOXYGEN__)
extern PWMDriver PWMD16;
#endif

#if SPC5_PWM_USE_EMIOS_CH16 && !defined(__DOXYGEN__)
extern PWMDriver PWMD17;
#endif

#if SPC5_PWM_USE_EMIOS_CH17 && !defined(__DOXYGEN__)
extern PWMDriver PWMD18;
#endif

#if SPC5_PWM_USE_EMIOS_CH18 && !defined(__DOXYGEN__)
extern PWMDriver PWMD19;
#endif

#if SPC5_PWM_USE_EMIOS_CH19 && !defined(__DOXYGEN__)
extern PWMDriver PWMD20;
#endif

#if SPC5_PWM_USE_EMIOS_CH20 && !defined(__DOXYGEN__)
extern PWMDriver PWMD21;
#endif

#if SPC5_PWM_USE_EMIOS_CH21 && !defined(__DOXYGEN__)
extern PWMDriver PWMD22;
#endif

#if SPC5_PWM_USE_EMIOS_CH22 && !defined(__DOXYGEN__)
extern PWMDriver PWMD23;
#endif

#if SPC5_PWM_USE_EMIOS_CH23 && !defined(__DOXYGEN__)
extern PWMDriver PWMD24;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void pwm_lld_init(void);
  void pwm_lld_start(PWMDriver *pwmp);
  void pwm_lld_stop(PWMDriver *pwmp);
  void pwm_lld_change_period(PWMDriver *pwmp, pwmcnt_t period);
  void pwm_lld_enable_channel(PWMDriver *pwmp,
                              pwmchannel_t channel,
                              pwmcnt_t width);
  void pwm_lld_disable_channel(PWMDriver *pwmp, pwmchannel_t channel);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_PWM */

#endif /* HAL_PWM_LLD_H */

/** @} */
