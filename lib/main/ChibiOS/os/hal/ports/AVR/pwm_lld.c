/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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

/*
   This driver is based on the work done by Matteo Serva available at
   http://github.com/matteoserva/ChibiOS-AVR
*/

/**
 * @file    AVR/pwm_lld.c
 * @brief   AVR PWM driver subsystem low level driver.
 *
 * @addtogroup PWM
 * @{
 */

#include "hal.h"

#if HAL_USE_PWM || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

typedef struct {
  volatile uint8_t *tccra;
  volatile uint8_t *tccrb;
  volatile uint8_t *ocrah;
  volatile uint8_t *ocral;
  volatile uint8_t *ocrbh;
  volatile uint8_t *ocrbl;
  volatile uint8_t *ocrch;
  volatile uint8_t *ocrcl;
  volatile uint8_t *tifr;
  volatile uint8_t *timsk;
} timer_registers_t;

static timer_registers_t regs_table[]=
{
#if AVR_PWM_USE_TIM1 || defined(__DOXYGEN__)
#if defined(OCR1C)
  {&TCCR1A, &TCCR1B, &OCR1AH, &OCR1AL, &OCR1BH, &OCR1BL, &OCR1CH, &OCR1CL, &TIFR1, &TIMSK1},
#else
  {&TCCR1A, &TCCR1B, &OCR1AH, &OCR1AL, &OCR1BH, &OCR1BL, NULL, NULL, &TIFR1, &TIMSK1},
#endif
#endif
#if AVR_PWM_USE_TIM2 || defined(__DOXYGEN__)
  {&TCCR2A, &TCCR2B, &OCR2A, &OCR2A, &OCR2B, &OCR2B, NULL, NULL, &TIFR2, &TIMSK2},
#endif
#if AVR_PWM_USE_TIM3 || defined(__DOXYGEN__)
  {&TCCR3A, &TCCR3B, &OCR3AH, &OCR3AL, &OCR3BH, &OCR3BL, &OCR3CH, &OCR3CL, &TIFR3, &TIMSK3},
#endif
#if AVR_PWM_USE_TIM4 || defined(__DOXYGEN__)
  {&TCCR4A, &TCCR4B, &OCR4AH, &OCR4AL, &OCR4CH, &OCR4CL, &OCR4CH, &OCR4CL, &TIFR4, &TIMSK4},
#endif
#if AVR_PWM_USE_TIM5 || defined(__DOXYGEN__)
  {&TCCR5A, &TCCR5B, &OCR5AH, &OCR5AL, &OCR5BH, &OCR5BL, &OCR5CH, &OCR5CL, &TIFR5, &TIMSK5},
#endif
};

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief PWM driver identifiers.*/
#if AVR_PWM_USE_TIM1 || defined(__DOXYGEN__)
PWMDriver PWMD1;
#endif
#if AVR_PWM_USE_TIM2 || defined(__DOXYGEN__)
PWMDriver PWMD2;
#endif
#if AVR_PWM_USE_TIM3 || defined(__DOXYGEN__)
PWMDriver PWMD3;
#endif
#if AVR_PWM_USE_TIM4 || defined(__DOXYGEN__)
PWMDriver PWMD4;
#endif
#if AVR_PWM_USE_TIM5 || defined(__DOXYGEN__)
PWMDriver PWMD5;
#endif

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static void config_channel(volatile uint8_t *tccra,
                           uint8_t com1,
                           uint8_t com0,
                           pwmmode_t mode)
{
  *tccra &= ~((1 << com1) | (1 << com0));
  if (mode == PWM_OUTPUT_ACTIVE_HIGH)
    *tccra |= ((1 << com1) | (0 << com0)); /* non inverting mode */
  else if (mode == PWM_OUTPUT_ACTIVE_LOW)
    *tccra |= (1 << com1) | (1 << com0);   /* inverting mode */
}

static uint8_t timer_index(PWMDriver *pwmp)
{
  uint8_t index = 0;

#if AVR_PWM_USE_TIM1 || defined(__DOXYGEN__)
  if (pwmp == &PWMD1) return index;
  else index++;
#endif
#if AVR_PWM_USE_TIM2 || defined(__DOXYGEN__)
  if (pwmp == &PWMD2) return index;
  else index++;
#endif
#if AVR_PWM_USE_TIM3 || defined(__DOXYGEN__)
  if (pwmp == &PWMD3) return index;
  else index++;
#endif
#if AVR_PWM_USE_TIM4 || defined(__DOXYGEN__)
  if (pwmp == &PWMD4) return index;
  else index++;
#endif
#if AVR_PWM_USE_TIM5 || defined(__DOXYGEN__)
  if (pwmp == &PWMD5) return index;
  else index++;
#endif

  /* This is an error! */
  return index;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*
 * interrupt for compare1&2 and clock overflow. pwmd1 & pwmd2
 */
#if AVR_PWM_USE_TIM1 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(TIMER1_OVF_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD1.config->callback(&PWMD1);
  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(TIMER1_COMPA_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD1.config->channels[0].callback(&PWMD1);
  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(TIMER1_COMPB_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD1.config->channels[1].callback(&PWMD1);
  OSAL_IRQ_EPILOGUE();
}
#if PWM_CHANNELS > 2
OSAL_IRQ_HANDLER(TIMER1_COMPC_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD1.config->channels[2].callback(&PWMD1);
  OSAL_IRQ_EPILOGUE();
}
#endif
#endif

#if AVR_PWM_USE_TIM2 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(TIMER2_OVF_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD2.config->callback(&PWMD2);
  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(TIMER2_COMPA_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD2.config->channels[0].callback(&PWMD2);
  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(TIMER2_COMPB_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD2.config->channels[1].callback(&PWMD2);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if AVR_PWM_USE_TIM3 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(TIMER3_OVF_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD3.config->callback(&PWMD3);
  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(TIMER3_COMPA_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD3.config->channels[0].callback(&PWMD3);
  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(TIMER3_COMPB_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD3.config->channels[1].callback(&PWMD3);
  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(TIMER3_COMPC_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD3.config->channels[2].callback(&PWMD3);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if AVR_PWM_USE_TIM4 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(TIMER4_OVF_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD4.config->callback(&PWMD4);
  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(TIMER4_COMPA_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD4.config->channels[0].callback(&PWMD4);
  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(TIMER4_COMPB_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD4.config->channels[1].callback(&PWMD4);
  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(TIMER4_COMPC_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD4.config->channels[2].callback(&PWMD4);
  OSAL_IRQ_EPILOGUE();
}
#endif

#if AVR_PWM_USE_TIM5 || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(TIMER5_OVF_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD5.config->callback(&PWMD5);
  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(TIMER5_COMPA_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD5.config->channels[0].callback(&PWMD5);
  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(TIMER5_COMPB_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD5.config->channels[1].callback(&PWMD5);
  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(TIMER5_COMPC_vect)
{
  OSAL_IRQ_PROLOGUE();
  PWMD5.config->channels[2].callback(&PWMD5);
  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level PWM driver initialization.
 *
 * @notapi
 */
void pwm_lld_init(void)
{
#if AVR_PWM_USE_TIM1 || defined(__DOXYGEN__)
  pwmObjectInit(&PWMD1);
  PWMD1.channels = PWM_CHANNELS;
  TCCR1A = (1 << WGM11) | (1 << WGM10);
  TCCR1B = (0 << WGM13) | (1 << WGM12);
#endif

#if AVR_PWM_USE_TIM2 || defined(__DOXYGEN__)
  pwmObjectInit(&PWMD2);
  PWMD2.channels = PWM_CHANNELS;
  TCCR2A = (1 << WGM21) | (1 << WGM20);
  TCCR2B = (0 << WGM22);
#endif

#if AVR_PWM_USE_TIM3 || defined(__DOXYGEN__)
  pwmObjectInit(&PWMD3);
  PWMD3.channels = PWM_CHANNELS;
  TCCR3A = (1 << WGM31) | (1 << WGM30);
  TCCR3B = (0 << WGM33) | (1 << WGM32);
#endif

#if AVR_PWM_USE_TIM4 || defined(__DOXYGEN__)
  pwmObjectInit(&PWMD4);
  PWMD4.channels = PWM_CHANNELS;
  TCCR4A = (1 << WGM41) | (1 << WGM40);
  TCCR4B = (0 << WGM43) | (1 << WGM42);
#endif

#if AVR_PWM_USE_TIM5 || defined(__DOXYGEN__)
  pwmObjectInit(&PWMD5);
  PWMD5.channels = PWM_CHANNELS;
  TCCR5A = (1 << WGM51) | (1 << WGM50);
  TCCR5B = (0 << WGM53) | (1 << WGM52);
#endif
}

/**
 * @brief   Configures and activates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to the @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_start(PWMDriver *pwmp)
{
  if (pwmp->state == PWM_STOP) {

#if AVR_PWM_USE_TIM2 || defined(__DOXYGEN__)
    if (pwmp == &PWMD2) {
      TCCR2B &= ~((1 << CS22) | (1 << CS21));
      TCCR2B |= (1 << CS20);
      if (pwmp->config->callback != NULL)
        TIMSK2 |= (1 << TOIE2);
      return;
    }
#endif

    uint8_t i = timer_index(pwmp);

    /* TODO: support other prescaler options */

    *regs_table[i].tccrb &= ~(1 << CS11);
    *regs_table[i].tccrb |= (1 << CS12) | (1 << CS10);
    if (pwmp->config->callback != NULL)
      *regs_table[i].timsk = (1 << TOIE1);
  }
}

/**
 * @brief   Deactivates the PWM peripheral.
 *
 * @param[in] pwmp      pointer to the @p PWMDriver object
 *
 * @notapi
 */
void pwm_lld_stop(PWMDriver *pwmp)
{
  uint8_t i = timer_index(pwmp);
  *regs_table[i].tccrb &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
  *regs_table[i].timsk = 0;
}

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
void pwm_lld_change_period(PWMDriver *pwmp, pwmcnt_t period)
{
}

/**
 * @brief   Enables a PWM channel.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The channel is active using the specified configuration.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...PWM_CHANNELS-1)
 * @param[in] width     PWM pulse width as clock pulses number
 *
 * @notapi
 */
void pwm_lld_enable_channel(PWMDriver *pwmp,
                            pwmchannel_t channel,
                            pwmcnt_t width)
{
  uint16_t val = width;
  if (val > MAX_PWM_VALUE)
    val = MAX_PWM_VALUE;

#if AVR_PWM_USE_TIM2 || defined(__DOXYGEN__)
  if (pwmp == &PWMD2) {
    config_channel(&TCCR2A,
                   7 - 2*channel,
                   6 - 2*channel,
                   pwmp->config->channels[channel].mode);
    TIMSK2 |= (1 << (channel + 1));
    /* Timer 2 is 8 bit */
    if (val > 0xFF)
      val = 0xFF;
    if (pwmp->config->channels[channel].callback) {
      switch (channel) {
      case 0: OCR2A = val; break;
      case 1: OCR2B = val; break;
      }
    }
    return;
  }
#endif

  uint8_t i = timer_index(pwmp);
  config_channel(regs_table[i].tccra,
                 7 - 2*channel,
                 6 - 2*channel,
                 pwmp->config->channels[channel].mode);
  volatile uint8_t *ocrh, *ocrl;
  switch (channel) {
  case 1:
    ocrh = regs_table[i].ocrbh;
    ocrl = regs_table[i].ocrbl;
    break;
  case 2:
    ocrh = regs_table[i].ocrch;
    ocrl = regs_table[i].ocrcl;
    break;
  default:
    ocrh = regs_table[i].ocrah;
    ocrl = regs_table[i].ocral;
  }
  *ocrh = val >> 8;
  *ocrl = val & 0xFF;
  *regs_table[i].tifr |= (1 << (channel + 1));
  if (pwmp->config->channels[channel].callback != NULL)
    *regs_table[i].timsk |= (1 << (channel + 1));
}

/**
 * @brief   Disables a PWM channel.
 * @pre     The PWM unit must have been activated using @p pwmStart().
 * @post    The channel is disabled and its output line returned to the
 *          idle state.
 * @note    Depending on the hardware implementation this function has
 *          effect starting on the next cycle (recommended implementation)
 *          or immediately (fallback implementation).
 *
 * @param[in] pwmp      pointer to a @p PWMDriver object
 * @param[in] channel   PWM channel identifier (0...PWM_CHANNELS-1)
 *
 * @notapi
 */
void pwm_lld_disable_channel(PWMDriver *pwmp, pwmchannel_t channel)
{
  uint8_t i = timer_index(pwmp);
  config_channel(regs_table[i].tccra,
                 7 - 2*channel,
                 6 - 2*channel,
                 PWM_OUTPUT_DISABLED);
  *regs_table[i].timsk &= ~(1 << (channel + 1));
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
  uint8_t i = timer_index(pwmp);
  *regs_table[i].timsk |= (1 << TOIE1);
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
  uint8_t i = timer_index(pwmp);
  *regs_table[i].timsk &= ~(1 << TOIE1);
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
  uint8_t i = timer_index(pwmp);
  *regs_table[i].timsk |= (1 << (channel + 1));
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

  uint8_t i = timer_index(pwmp);
  *regs_table[i].timsk &= ~(1 << (channel + 1));
}


#endif /* HAL_USE_PWM */

/** @} */
