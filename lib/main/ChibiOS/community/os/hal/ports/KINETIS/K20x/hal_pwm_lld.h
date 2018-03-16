/*
    ChibiOS/HAL - Copyright (C) 2014 Adam J. Porter

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
 * @file    K20x7/pwm_lld.h
 * @brief   KINETIS PWM subsystem low level driver header.
 *
 * @addtogroup PWM
 * @{
 */

#ifndef HAL_PWM_LLD_H_
#define HAL_PWM_LLD_H_

#if HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Number of PWM channels per PWM driver.
 */
#define PWM_CHANNELS                            8

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

#if !defined(KINETIS_PWM_USE_FTM0)
  #define KINETIS_PWM_USE_FTM0 FALSE
#endif

#if !defined(KINETIS_PWM_USE_FTM1)
  #define KINETIS_PWM_USE_FTM1 FALSE
#endif

#if !defined(KINETIS_PWM_USE_FTM2)
  #define KINETIS_PWM_USE_FTM2 FALSE
#endif

/**
 * @brief   FTM0 interrupt priority level setting.
 */
#if !defined(KINETIS_PWM_FTM0_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_PWM_FTM0_PRIORITY        12
#endif

/**
 * @brief   FTM1 interrupt priority level setting.
 */
#if !defined(KINETIS_PWM_FTM1_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_PWM_FTM1_PRIORITY        12
#endif

/**
 * @brief   FTM2 interrupt priority level setting.
 */
#if !defined(KINETIS_PWM_FTM2_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_PWM_FTM2_PRIORITY        12
#endif

/** @} */

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   If advanced timer features switch.
 * @details If set to @p TRUE the advanced features for TIM1 and TIM8 are
 *          enabled.
 * @note    The default is @p TRUE.
 */
#if !defined(KINETIS_PWM_USE_ADVANCED) || defined(__DOXYGEN__)
#define KINETIS_PWM_USE_ADVANCED              FALSE
#endif
/** @} */

/*===========================================================================*/
/* Configuration checks.                                                     */
/*===========================================================================*/

#if !KINETIS_PWM_USE_FTM0 && !KINETIS_PWM_USE_FTM1 && !KINETIS_PWM_USE_FTM2
#error "PWM driver activated but no FTM peripheral assigned"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a PWM mode.
 */
typedef uint32_t pwmmode_t;

/**
 * @brief   Type of a PWM channel.
 */
typedef uint8_t pwmchannel_t;

/**
 * @brief   Type of a channels mask.
 */
typedef uint32_t pwmchnmsk_t;

/**
 * @brief   Type of a PWM counter.
 */
typedef uint16_t pwmcnt_t;

/**
 * @brief   Type of a PWM driver channel configuration structure.
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
 * @brief   Type of a PWM driver configuration structure.
 */
typedef struct {
  /**
   * @brief   Timer clock in Hz.
   * @note    The low level can use assertions in order to catch invalid
   *          frequency specifications.
   */
  uint32_t                  frequency;
  /**
   * @brief   PWM period in ticks.
   * @note    The low level can use assertions in order to catch invalid
   *          period specifications.
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
} PWMConfig;

/**
 * @brief   Structure representing a PWM driver.
 */
struct PWMDriver {
  /**
   * @brief Driver state.
   */
  pwmstate_t                state;
  /**
   * @brief Current driver configuration data.
   */
  const PWMConfig           *config;
  /**
   * @brief   Current PWM period in ticks.
   */
  pwmcnt_t                  period;
  /**
   * @brief   Mask of the enabled channels.
   */
  pwmchnmsk_t               enabled;
  /**
   * @brief   Number of channels in this instance.
   */
  pwmchannel_t              channels;
#if defined(PWM_DRIVER_EXT_FIELDS)
  PWM_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the FTM registers block.
   */
  FTM_TypeDef               *ftm;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Changes the period the PWM peripheral.
 * @details This function changes the period of a PWM unit that has already
 *          been activated using @p pwmStart().
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The PWM unit period is changed to the new value.
 * @note    The function has effect at the next cycle start.
 * @note    If a period is specified that is shorter than the pulse width
 *          programmed in one of the channels then the behavior is not
 *          guaranteed.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] period    new cycle time in ticks
 *
 * @notapi
 */
#define pwm_lld_change_period(pwmp, period)                                 \
  do { \
    (pwmp)->ftm->MOD = ((period) - 1); \
    pwmp->ftm->PWMLOAD = FTM_PWMLOAD_LDOK_MASK;\
  } while(0)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if KINETIS_PWM_USE_FTM0 || defined(__DOXYGEN__)
extern PWMDriver PWMD1;
#endif
#if KINETIS_PWM_USE_FTM1 || defined(__DOXYGEN__)
extern PWMDriver PWMD2;
#endif
#if KINETIS_PWM_USE_FTM2 || defined(__DOXYGEN__)
extern PWMDriver PWMD3;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void pwm_lld_init(void);
  void pwm_lld_start(PWMDriver *pwmp);
  void pwm_lld_stop(PWMDriver *pwmp);
  void pwm_lld_enable_channel(PWMDriver *pwmp,
                              pwmchannel_t channel,
                              pwmcnt_t width);
  void pwm_lld_disable_channel(PWMDriver *pwmp, pwmchannel_t channel);
  void pwm_lld_enable_periodic_notification(PWMDriver *pwmp);
  void pwm_lld_disable_periodic_notification(PWMDriver *pwmp);
  void pwm_lld_enable_channel_notification(PWMDriver *pwmp,
                                           pwmchannel_t channel);
  void pwm_lld_disable_channel_notification(PWMDriver *pwmp,
                                            pwmchannel_t channel);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_PWM */

#endif /* HAL_PWM_LLD_H_ */

/** @} */
