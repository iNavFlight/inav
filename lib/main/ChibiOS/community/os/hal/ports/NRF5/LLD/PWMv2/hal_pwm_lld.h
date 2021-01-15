/*
    ChibiOS/HAL - Copyright (C) 2018 Andru

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
 * @file    hal_pwm_lld.h
 * @brief   NRF52 PWM subsystem low level driver header.
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
#define PWM_CHANNELS      4

typedef enum {
	PWM_FREQUENCY_16MHZ = 16000000,	 /** @brief   16MHz */
	PWM_FREQUENCY_8MHZ  =  8000000,	 /** @brief    8MHz */
	PWM_FREQUENCY_4MHZ  =  4000000,	 /** @brief    4MHz */
	PWM_FREQUENCY_2MHZ  =  2000000,	 /** @brief    2MHz */
	PWM_FREQUENCY_1MHZ  =  1000000,	 /** @brief    1MHz */
	PWM_FREQUENCY_500KHZ =  500000,	 /** @brief  500kHz */
	PWM_FREQUENCY_250KHZ =  250000,	 /** @brief  250kHz */
	PWM_FREQUENCY_125KHZ =  125000,	 /** @brief  125kHz */
} pwm_frequency_t;

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */

/** @} */

/*===========================================================================*/
/* Configuration checks.                                                     */
/*===========================================================================*/


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

  /**
   * @brief PAL line to toggle.
   * @note  Only used if mode is PWM_OUTPUT_HIGH or PWM_OUTPUT_LOW.
   * @note  When channel enabled it wont be possible to access this PAL line using the PAL
   *        driver.
   */
  ioline_t ioline;

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
  pwm_frequency_t           frequency;
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
   * @brief Pointer to the PWM registers block.
   */
  NRF_PWM_Type           *pwm;
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
#define pwm_lld_change_period(pwmp, period) \
  do { \
    (pwmp)->pwm->COUNTERTOP = period & PWM_COUNTERTOP_COUNTERTOP_Msk;  \
  } while(0)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if NRF5_PWM_USE_PWM0 || defined(__DOXYGEN__)
extern PWMDriver PWMD1;
#endif
#if NRF5_PWM_USE_PWM1 || defined(__DOXYGEN__)
extern PWMDriver PWMD2;
#endif
#if NRF5_PWM_USE_PWM2 || defined(__DOXYGEN__)
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
