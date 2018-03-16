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
 * @file    SPC5xx/eMIOS_v1/hal_pwm_lld.h
 * @brief   SPC5xx low level PWM driver header.
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
#define PWM_CHANNELS                            7

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
#if !defined(SPC5_PWM_USE_EMIOS0_GROUP0) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS0__GROUP0         FALSE
#endif

/**
 * @brief   PWMD2 driver enable switch.
 * @details If set to @p TRUE the support for PWMD2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS0_GROUP1) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS0_GROUP1          FALSE
#endif

/**
 * @brief   PWMD1 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS0_GFR_F8F9_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_GFR_F8F9_PRIORITY       7
#endif

/**
 * @brief   PWMD1 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS0_GFR_F10F11_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_GFR_F10F11_PRIORITY     7
#endif

/**
 * @brief   PWMD1 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS0_GFR_F12F13_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_GFR_F12F13_PRIORITY     7
#endif

/**
 * @brief   PWMD1 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS0_GFR_F14F15_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_GFR_F14F15_PRIORITY     7
#endif

/**
 * @brief   PWMD2 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS0_GFR_F16F17_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_GFR_F16F17_PRIORITY     7
#endif

/**
 * @brief   PWMD2 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS0_GFR_F18F19_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_GFR_F18F19_PRIORITY     7
#endif

/**
 * @brief   PWMD2 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS0_GFR_F20F21_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_GFR_F20F21_PRIORITY     7
#endif

/**
 * @brief   PWMD2 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS0_GFR_F22F23_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS0_GFR_F22F23_PRIORITY     7
#endif

/**
 * @brief   PWMD3 driver enable switch.
 * @details If set to @p TRUE the support for PWMD3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS1_GROUP0) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS1_GROUP0          FALSE
#endif

/**
 * @brief   PWMD4 driver enable switch.
 * @details If set to @p TRUE the support for PWMD4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS1_GROUP1) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS1_GROUP1          FALSE
#endif

/**
 * @brief   PWMD5 driver enable switch.
 * @details If set to @p TRUE the support for PWMD5 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SPC5_PWM_USE_EMIOS1_GROUP2) || defined(__DOXYGEN__)
#define SPC5_PWM_USE_EMIOS1_GROUP2          FALSE
#endif

/**
 * @brief   PWMD3 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS1_GFR_F0F1_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_GFR_F0F1_PRIORITY       7
#endif

/**
 * @brief   PWMD3 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS1_GFR_F2F3_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_GFR_F2F3_PRIORITY       7
#endif

/**
 * @brief   PWMD3 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS1_GFR_F4F5_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_GFR_F4F5_PRIORITY       7
#endif

/**
 * @brief   PWMD3 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS1_GFR_F6F7_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_GFR_F6F7_PRIORITY       7
#endif

/**
 * @brief   PWMD4 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS1_GFR_F8F9_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_GFR_F8F9_PRIORITY       7
#endif

/**
 * @brief   PWMD4 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS1_GFR_F10F11_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_GFR_F10F11_PRIORITY     7
#endif

/**
 * @brief   PWMD4 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS1_GFR_F12F13_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_GFR_F12F13_PRIORITY     7
#endif

/**
 * @brief   PWMD4 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS1_GFR_F14F15_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_GFR_F14F15_PRIORITY     7
#endif

/**
 * @brief   PWMD5 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS1_GFR_F16F17_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_GFR_F16F17_PRIORITY     7
#endif

/**
 * @brief   PWMD5 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS1_GFR_F18F19_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_GFR_F18F19_PRIORITY     7
#endif

/**
 * @brief   PWMD5 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS1_GFR_F20F21_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_GFR_F20F21_PRIORITY     7
#endif

/**
 * @brief   PWMD5 interrupt priority level setting.
 */
#if !defined(SPC5_EMIOS1_GFR_F22F23_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_EMIOS1_GFR_F22F23_PRIORITY     7
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#define SPC5_PWM_USE_EMIOS0                 (SPC5_PWM_USE_EMIOS0_GROUP0 ||  \
                                             SPC5_PWM_USE_EMIOS0_GROUP1)

#define SPC5_PWM_USE_EMIOS1                 (SPC5_PWM_USE_EMIOS1_GROUP0 ||  \
                                             SPC5_PWM_USE_EMIOS1_GROUP1 ||  \
                                             SPC5_PWM_USE_EMIOS1_GROUP2)

#if !SPC5_HAS_EMIOS0 && SPC5_PWM_USE_EMIOS0
#error "EMIOS0 not present in the selected device"
#endif

#if !SPC5_HAS_EMIOS1 && SPC5_PWM_USE_EMIOS1
#error "EMIOS1 not present in the selected device"
#endif

#if !SPC5_PWM_USE_EMIOS0 && !SPC5_PWM_USE_EMIOS1
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

#if SPC5_PWM_USE_EMIOS0_GROUP0 && !defined(__DOXYGEN__)
extern PWMDriver PWMD1;
#endif

#if SPC5_PWM_USE_EMIOS0_GROUP1 && !defined(__DOXYGEN__)
extern PWMDriver PWMD2;
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP0 && !defined(__DOXYGEN__)
extern PWMDriver PWMD3;
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP1 && !defined(__DOXYGEN__)
extern PWMDriver PWMD4;
#endif

#if SPC5_PWM_USE_EMIOS1_GROUP2 && !defined(__DOXYGEN__)
extern PWMDriver PWMD5;
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
