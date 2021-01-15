/*
    Copyright (C) 2014..2017 Marco Veeneman

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
 * @file    PWM/hal_pwm_lld.c
 * @brief   TM4C123x/TM4C129x PWM subsystem low level driver header.
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
#define PWM_CHANNELS                            8

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
#if !defined(TIVA_PWM_USE_PWM0) || defined(__DOXYGEN__)
#define TIVA_PWM_USE_PWM0                       FALSE
#endif

/**
 * @brief   PWMD2 driver enable switch.
 * @details If set to @p TRUE the support for PWMD2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_PWM_USE_PWM1) || defined(__DOXYGEN__)
#define TIVA_PWM_USE_PWM1                       FALSE
#endif

/**
 * @brief   PWMD1 fault interrupt priority level setting.
 */
#if !defined(TIVA_PWM_PWM0_FAULT_IRQ_PRIORITY) || defined (__DOXYGEN__)
#define TIVA_PWM_PWM0_FAULT_IRQ_PRIORITY
#endif

/**
 * @brief   PWMD1 channel 0 & 1 interrupt priority level setting.
 */
#if !defined(TIVA_PWM_PWM0_0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PWM_PWM0_0_IRQ_PRIORITY            4
#endif

/**
 * @brief   PWMD1 channel 2 & 3 interrupt priority level setting.
 */
#if !defined(TIVA_PWM_PWM0_1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PWM_PWM0_1_IRQ_PRIORITY            4
#endif

/**
 * @brief   PWMD1 channel 4 & 5 interrupt priority level setting.
 */
#if !defined(TIVA_PWM_PWM0_2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PWM_PWM0_2_IRQ_PRIORITY            4
#endif

/**
 * @brief   PWMD1 channel 6 & 7 interrupt priority level setting.
 */
#if !defined(TIVA_PWM_PWM0_3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PWM_PWM0_3_IRQ_PRIORITY            4
#endif

/**
 * @brief   PWMD2 fault interrupt priority level setting.
 */
#if !defined(TIVA_PWM_PWM1_FAULT_IRQ_PRIORITY) || defined (__DOXYGEN__)
#define TIVA_PWM_PWM1_FAULT_IRQ_PRIORITY
#endif

/**
 * @brief   PWMD2 channel 0 & 1 interrupt priority level setting.
 */
#if !defined(TIVA_PWM_PWM1_0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PWM_PWM1_0_IRQ_PRIORITY            4
#endif

/**
 * @brief   PWMD2 channel 2 & 3 interrupt priority level setting.
 */
#if !defined(TIVA_PWM_PWM1_1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PWM_PWM1_1_IRQ_PRIORITY            4
#endif

/**
 * @brief   PWMD2 channel 4 & 5 interrupt priority level setting.
 */
#if !defined(TIVA_PWM_PWM1_2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PWM_PWM1_2_IRQ_PRIORITY            4
#endif

/**
 * @brief   PWMD2 channel 6 & 7 interrupt priority level setting.
 */
#if !defined(TIVA_PWM_PWM1_3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_PWM_PWM1_3_IRQ_PRIORITY            4
#endif

/**
 * @}
 */

/*===========================================================================*/
/* Configuration checks.                                                     */
/*===========================================================================*/

#if TIVA_PWM_USE_PWM0 && !TIVA_HAS_PWM0
#error "PWM0 not present in the selected device"
#endif

#if TIVA_PWM_USE_PWM1 && !TIVA_HAS_PWM1
#error "PWM1 not present in the selected device"
#endif

#if !TIVA_PWM_USE_PWM0 && !TIVA_PWM_USE_PWM1
#error "PWM driver activated but no PWM peripheral assigned"
#endif

#if TIVA_PWM_USE_PWM0 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PWM_PWM0_FAULT_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PWM0 FAULT"
#endif

#if TIVA_PWM_USE_PWM0 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PWM_PWM0_0_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PWM0 GEN0"
#endif

#if TIVA_PWM_USE_PWM0 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PWM_PWM0_1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PWM0 GEN1"
#endif

#if TIVA_PWM_USE_PWM0 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PWM_PWM0_2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PWM0 GEN2"
#endif

#if TIVA_PWM_USE_PWM0 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PWM_PWM0_3_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PWM0 GEN3"
#endif

#if TIVA_PWM_USE_PWM1 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PWM_PWM1_FAULT_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PWM1 FAULT"
#endif

#if TIVA_PWM_USE_PWM1 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PWM_PWM1_0_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PWM1 GEN0"
#endif

#if TIVA_PWM_USE_PWM1 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PWM_PWM1_1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PWM1 GEN1"
#endif

#if TIVA_PWM_USE_PWM1 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PWM_PWM1_2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PWM1 GEN2"
#endif

#if TIVA_PWM_USE_PWM1 &&                                                    \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_PWM_PWM1_3_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to PWM1 GEN3"
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
   * @brief Pointer to the PWMx registers block.
   */
  uint32_t                  pwm;
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
#define pwm_lld_change_period(pwmp, period)                                  \
  HWREG((pwmp)->pwm + PWM_O_0_LOAD) = (uint16_t)((period) - 1);             \
  HWREG((pwmp)->pwm + PWM_O_1_LOAD) = (uint16_t)((period) - 1);             \
  HWREG((pwmp)->pwm + PWM_O_2_LOAD) = (uint16_t)((period) - 1);             \
  HWREG((pwmp)->pwm + PWM_O_3_LOAD) = (uint16_t)((period) - 1)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if TIVA_PWM_USE_PWM0 && !defined(__DOXYGEN__)
extern PWMDriver PWMD1;
#endif

#if TIVA_PWM_USE_PWM1 && !defined(__DOXYGEN__)
extern PWMDriver PWMD2;
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

#endif /* HAL_PWM_LLD_H */

/** @} */
