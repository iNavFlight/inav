/*
    ChibiOS/HAL - Copyright (C) 2016 Stéphane D'Alu

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
 * @file    PWMv1/hal_pwm_lld.h
 * @brief   NRF51 PWM subsystem low level driver header.
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
#if NRF5_PWM_USE_GPIOTE_PPI
#define PWM_CHANNELS                            2
#else
#define PWM_CHANNELS                            3
#endif

#define PWM_FREQUENCY_16MHZ  16000000 /** @brief   16MHz */
#define PWM_FREQUENCY_8MHZ    8000000 /** @brief    8MHz */
#define PWM_FREQUENCY_4MHZ    4000000 /** @brief    4MHz */
#define PWM_FREQUENCY_2MHZ    2000000 /** @brief    2MHz */
#define PWM_FREQUENCY_1MHZ    1000000 /** @brief    1MHz */
#define PWM_FREQUENCY_500KHZ   500000 /** @brief  500kHz */
#define PWM_FREQUENCY_250KHZ   250000 /** @brief  250kHz */
#define PWM_FREQUENCY_125KHZ   125000 /** @brief  125kHz */
#define PWM_FREQUENCY_62500HZ   62500 /** @brief 62500Hz */
#define PWM_FREQUENCY_31250HZ   31250 /** @brief 31250Hz */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */

/**
 * @brief   TIMER0 as driver implementation
 */
#if !defined(NRF5_PWM_USE_TIMER0)
#define NRF5_PWM_USE_TIMER0 FALSE
#endif

/**
 * @brief   TIMER1 as driver implementation
 */
#if !defined(NRF5_PWM_USE_TIMER1)
#define NRF5_PWM_USE_TIMER1 FALSE
#endif

/**
 * @brief   TIMER2 as driver implementation
 */
#if !defined(NRF5_PWM_USE_TIMER2)
#define NRF5_PWM_USE_TIMER2 FALSE
#endif

/**
 * @brief   TIMER0 interrupt priority level setting.
 */
#if !defined(NRF5_PWM_TIMER0_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_PWM_TIMER0_PRIORITY        3
#endif

/**
 * @brief   TIMER1 interrupt priority level setting.
 */
#if !defined(NRF5_PWM_TIMER1_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_PWM_TIMER1_PRIORITY        3
#endif

/**
 * @brief   TIMER2 interrupt priority level setting.
 */
#if !defined(NRF5_PWM_TIMER2_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_PWM_TIMER2_PRIORITY        3
#endif

/**
 * @brief   Allow driver to use GPIOTE/PPI to control PAL line
 */
#if !defined(NRF5_PWM_USE_GPIOTE_PPI)
#define NRF5_PWM_USE_GPIOTE_PPI TRUE
#endif

/** @} */

/*===========================================================================*/
/* Configuration checks.                                                     */
/*===========================================================================*/

#if !NRF5_PWM_USE_TIMER0 && !NRF5_PWM_USE_TIMER1 && !NRF5_PWM_USE_TIMER2
#error "PWM driver activated but no TIMER peripheral assigned"
#endif

#if (NRF5_ST_USE_TIMER0 == TRUE) && (NRF5_PWM_USE_TIMER0 == TRUE)
#error "TIMER0 used for ST and PWM"
#endif

#if NRF5_PWM_USE_TIMER0 &&						    \
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_PWM_TIMER0_PRIORITY)
#error "Invalid IRQ priority assigned to TIMER0"
#endif

#if NRF5_PWM_USE_TIMER1 &&						    \
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_PWM_TIMER1_PRIORITY)
#error "Invalid IRQ priority assigned to TIMER1"
#endif

#if NRF5_PWM_USE_TIMER2 &&						    \
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_PWM_TIMER2_PRIORITY)
#error "Invalid IRQ priority assigned to TIMER2"
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

#if NRF5_PWM_USE_GPIOTE_PPI || defined(__DOXYGEN__)
  /**
   * @brief PAL line to toggle.
   * @note  Only used if mode is PWM_OUTPUT_HIGH or PWM_OUTPUT_LOW.
   * @note  When NRF5_PWM_USE_GPIOTE_PPI is used and channel enabled,
   *        it wont be possible to access this PAL line using the PAL
   *        driver.
   */
  ioline_t ioline;

  /**
   * @brief Unique GPIOTE channel to use. (1 channel)
   * @note  Only 4 GPIOTE channels are available on nRF51.
   */
  uint8_t gpiote_channel;

  /**
   * @brief Unique PPI channels to use. (2 channels)
   * @note  Only 16 PPI channels are available on nRF51
   *        (When Softdevice is enabled, only channels 0-7 are available)
   */
  uint8_t ppi_channel[2];
#endif
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
   * @brief Pointer to the TIMER registers block.
   */
  NRF_TIMER_Type           *timer;
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
#define pwm_lld_change_period(pwmp, period)                             \
  do { 									\
    (pwmp)->timer->CC[(pwmp)->channels] = ((period) - 1);		\
  } while(0)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if NRF5_PWM_USE_TIMER0 || defined(__DOXYGEN__)
extern PWMDriver PWMD1;
#endif
#if NRF5_PWM_USE_TIMER1 || defined(__DOXYGEN__)
extern PWMDriver PWMD2;
#endif
#if NRF5_PWM_USE_TIMER2 || defined(__DOXYGEN__)
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
