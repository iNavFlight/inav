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
 * @file    K20x/pwm_lld.c
 * @brief   KINETIS PWM subsystem low level driver source.
 *
 * @addtogroup PWM
 * @{
 */

#include "hal.h"

#if HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/


/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   PWMD1 driver identifier.
 * @note    The driver PWMD1 allocates the timer FTM0 when enabled.
 */
#if KINETIS_PWM_USE_FTM0 || defined(__DOXYGEN__)
PWMDriver PWMD1;
#endif

/**
 * @brief   PWMD2 driver identifier.
 * @note    The driver PWMD2 allocates the timer FTM1 when enabled.
 */
#if KINETIS_PWM_USE_FTM1 || defined(__DOXYGEN__)
PWMDriver PWMD2;
#endif

/**
 * @brief   PWMD3 driver identifier.
 * @note    The driver PWMD3 allocates the timer FTM2 when enabled.
 */
#if KINETIS_PWM_USE_FTM2 || defined(__DOXYGEN__)
PWMDriver PWMD3;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static void pwm_lld_serve_interrupt(PWMDriver *pwmp) {
  uint32_t sr;

  sr = pwmp->ftm->SC;
  pwmp->ftm->SC = sr&(~FTM_SC_TOF);

  if (((sr & FTM_SC_TOF) != 0) &&                          /* Timer Overflow */
      ((sr & FTM_SC_TOIE) != 0) &&
      (pwmp->config->callback != NULL)) {
    pwmp->config->callback(pwmp);
  }

  uint8_t n=0;
  for(n=0;n<pwmp->channels;n++) {
    sr = pwmp->ftm->CHANNEL[n].CnSC;
    pwmp->ftm->CHANNEL[n].CnSC = sr&(~FTM_CnSC_CHF);
    if (((sr & FTM_CnSC_CHF) != 0) &&
        ((sr & FTM_CnSC_CHIE) != 0) &&
        (pwmp->config->channels[n].callback != NULL)) {
      pwmp->config->channels[n].callback(pwmp);
    }
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if KINETIS_PWM_USE_FTM0
/**
 * @brief   FTM0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(KINETIS_FTM0_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  pwm_lld_serve_interrupt(&PWMD1);
  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_PWM_USE_FTM0 */

#if KINETIS_PWM_USE_FTM1
/**
 * @brief   FTM1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(KINETIS_FTM1_IRQ_VECTOR) {

  OSAL_IRQ_PROLOGUE();
  pwm_lld_serve_interrupt(&PWMD2);
  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_PWM_USE_FTM1 */

#if KINETIS_PWM_USE_FTM2
/**
 * @brief   FTM2 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(KINETIS_FTM2_IRQ_VECTOR) {

  OSAL_IRQ_PROLOGUE();
  pwm_lld_serve_interrupt(&PWMD3);
  OSAL_IRQ_EPILOGUE();
}
#endif /* KINETIS_PWM_USE_FTM2 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level PWM driver initialization.
 *
 * @notapi
 */
void pwm_lld_init(void) {

#if KINETIS_PWM_USE_FTM0
  pwmObjectInit(&PWMD1);
  PWMD1.channels = KINETIS_FTM0_CHANNELS;
  PWMD1.ftm = FTM0;
#endif

#if KINETIS_PWM_USE_FTM1
  pwmObjectInit(&PWMD2);
  PWMD2.channels = KINETIS_FTM1_CHANNELS;
  PWMD2.ftm = FTM1;
#endif

#if KINETIS_PWM_USE_FTM2
  pwmObjectInit(&PWMD3);
  PWMD3.channels = KINETIS_FTM2_CHANNELS;
  PWMD3.ftm = FTM2;
#endif
}

/**
 * @brief   Configures and activates the PWM peripheral.
 * @note    Starting a driver that is already in the @p PWM_READY state
 *          disables all the active channels.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_start(PWMDriver *pwmp) {
  uint16_t psc;
  uint8_t i=0;

  if (pwmp->state == PWM_STOP) {
    /* Clock activation and timer reset.*/
#if KINETIS_PWM_USE_FTM0
    if (&PWMD1 == pwmp) {
      SIM->SCGC6 |= SIM_SCGC6_FTM0;
      nvicEnableVector(FTM0_IRQn, KINETIS_PWM_FTM0_PRIORITY);
    }
#endif

#if KINETIS_PWM_USE_FTM1
    if (&PWMD2 == pwmp) {
      SIM->SCGC6 |= SIM_SCGC6_FTM1;
      nvicEnableVector(FTM1_IRQn, KINETIS_PWM_FTM1_PRIORITY);
    }
#endif

#if KINETIS_PWM_USE_FTM2
    if (&PWMD3 == pwmp) {
      SIM->SCGC3 |= SIM_SCGC3_FTM2;
      nvicEnableVector(FTM2_IRQn, KINETIS_PWM_FTM2_PRIORITY);
    }
#endif
  }
  pwmp->ftm->MODE =  FTM_MODE_FTMEN_MASK|FTM_MODE_PWMSYNC_MASK;
  pwmp->ftm->SYNC =  FTM_SYNC_CNTMIN_MASK|FTM_SYNC_CNTMAX_MASK
                    |FTM_SYNC_SWSYNC_MASK;
  pwmp->ftm->COMBINE =  FTM_COMBINE_SYNCEN3_MASK | FTM_COMBINE_SYNCEN2_MASK
                      | FTM_COMBINE_SYNCEN1_MASK | FTM_COMBINE_SYNCEN0_MASK;
  pwmp->ftm->SYNCONF =  FTM_SYNCONF_SYNCMODE_MASK;

  pwmp->ftm->CNTIN = 0x0000;
  //~ pwmp->ftm->SC = 0;       /* Disable FTM counter.*/
  pwmp->ftm->CNT = 0x0000; /* Clear count register.*/

  /* Prescaler value calculation.*/
  psc = (KINETIS_SYSCLK_FREQUENCY / pwmp->config->frequency);
  //~ /* Prescaler must be power of two between 1 and 128.*/
  osalDbgAssert(psc <= 128 && !(psc & (psc - 1)), "invalid frequency");
  //~ /* Prescaler register value determination.
     //~ Prescaler register value conveniently corresponds to bit position,
     //~ i.e., register value for prescaler CLK/64 is 6 ((1 << 6) == 64).*/
  for (i = 0; i < 8; i++) {
    if (psc == (unsigned)(1 << i)) {
      break;
    }
  }

  /* Set prescaler and clock mode.
     This also sets the following:
          CPWMS up-counting mode
          Timer overflow interrupt disabled
          DMA disabled.*/
  pwmp->ftm->SC = FTM_SC_CLKS(1) | FTM_SC_PS(i);
  /* Configure period */
  pwmp->ftm->MOD = pwmp->period-1;
  pwmp->ftm->PWMLOAD = FTM_PWMLOAD_LDOK_MASK;
}

/**
 * @brief   Deactivates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_stop(PWMDriver *pwmp) {

  /* If in ready state then disables the PWM clock.*/
  if (pwmp->state == PWM_READY) {
#if KINETIS_PWM_USE_FTM0
    if (&PWMD1 == pwmp) {
      SIM->SCGC6 &= ~SIM_SCGC6_FTM0;
      nvicDisableVector(FTM0_IRQn);
    }
#endif

#if KINETIS_PWM_USE_FTM1
    if (&PWMD2 == pwmp) {
      SIM->SCGC6 &= ~SIM_SCGC6_FTM1;
      nvicDisableVector(FTM1_IRQn);
    }
#endif

#if KINETIS_PWM_USE_FTM2
    if (&PWMD3 == pwmp) {
      SIM->SCGC3 &= ~SIM_SCGC3_FTM2;
      nvicDisableVector(FTM2_IRQn);
    }
#endif
    /* Disable FTM counter.*/
    pwmp->ftm->SC = 0;
    pwmp->ftm->MOD = 0;
  }
}

/**
 * @brief   Enables a PWM channel.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The channel is active using the specified configuration.
 * @note    The function has effect at the next cycle start.
 * @note    Channel notification is not enabled.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...channels-1)
 * @param[in] width     PWM pulse width as clock pulses number
 *
 * @notapi
 */
void pwm_lld_enable_channel(PWMDriver *pwmp,
                            pwmchannel_t channel,
                            pwmcnt_t width) {
  uint32_t mode = FTM_CnSC_MSB; /* Edge-aligned PWM mode.*/

  switch (pwmp->config->channels[channel].mode & PWM_OUTPUT_MASK) {
  case PWM_OUTPUT_ACTIVE_HIGH:
    mode |= FTM_CnSC_ELSB;
    break;
  case PWM_OUTPUT_ACTIVE_LOW:
    mode |= FTM_CnSC_ELSA;
    break;
  }

  if (pwmp->ftm->CHANNEL[channel].CnSC & FTM_CnSC_CHIE)
    mode |= FTM_CnSC_CHIE;

  pwmp->ftm->CHANNEL[channel].CnSC = mode;
  pwmp->ftm->CHANNEL[channel].CnV = width;
  pwmp->ftm->PWMLOAD = FTM_PWMLOAD_LDOK_MASK;
}

/**
 * @brief   Disables a PWM channel and its notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The channel is disabled and its output line returned to the
 *          idle state.
 * @note    The function has effect at the next cycle start.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...channels-1)
 *
 * @notapi
 */
void pwm_lld_disable_channel(PWMDriver *pwmp, pwmchannel_t channel) {

  pwmp->ftm->CHANNEL[channel].CnSC = 0;
  pwmp->ftm->CHANNEL[channel].CnV = 0;
}

/**
 * @brief   Enables the periodic activation edge notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @note    If the notification is already enabled then the call has no effect.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_enable_periodic_notification(PWMDriver *pwmp) {
  pwmp->ftm->SC |= FTM_SC_TOIE;
}

/**
 * @brief   Disables the periodic activation edge notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @note    If the notification is already disabled then the call has no effect.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_disable_periodic_notification(PWMDriver *pwmp) {
  pwmp->ftm->SC &= ~FTM_SC_TOIE;
}

/**
 * @brief   Enables a channel de-activation edge notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @pre     The channel must have been activated using @p pwmEnableChannel().
 * @note    If the notification is already enabled then the call has no effect.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...channels-1)
 *
 * @notapi
 */
void pwm_lld_enable_channel_notification(PWMDriver *pwmp,
                                         pwmchannel_t channel) {
  pwmp->ftm->CHANNEL[channel].CnSC |= FTM_CnSC_CHIE;
}

/**
 * @brief   Disables a channel de-activation edge notification.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @pre     The channel must have been activated using @p pwmEnableChannel().
 * @note    If the notification is already disabled then the call has no effect.
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...channels-1)
 *
 * @notapi
 */
void pwm_lld_disable_channel_notification(PWMDriver *pwmp,
                                          pwmchannel_t channel) {
  pwmp->ftm->CHANNEL[channel].CnSC &= ~FTM_CnSC_CHIE;
}

#endif /* HAL_USE_PWM */

/** @} */
