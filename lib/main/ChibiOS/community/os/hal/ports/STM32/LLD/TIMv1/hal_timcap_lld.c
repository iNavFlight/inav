/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

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
   This file was derived from the ICU subsystem code, modified to achieve
   timing measurements on 2 and/or 4 channel STM32 timers by Dave Camarillo.
 */
/*
   Concepts and parts of this file have been contributed by Fabio Utzig and
   Xo Wang.
 */


/**
 * @file    STM32/hal_timcap_lld.c
 * @brief   STM32 TIMCAP subsystem low level driver header.
 *
 * @addtogroup TIMCAP
 * @{
 */

#include "hal.h"

#if HAL_USE_TIMCAP || defined(__DOXYGEN__)

#include "stm32_tim.h"
#include "hal_timcap.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   TIMCAPD1 driver identifier.
 * @note    The driver TIMCAPD1 allocates the complex timer TIM1 when enabled.
 */
#if STM32_TIMCAP_USE_TIM1 || defined(__DOXYGEN__)
TIMCAPDriver TIMCAPD1;
#endif

/**
 * @brief   TIMCAPD2 driver identifier.
 * @note    The driver TIMCAPD1 allocates the timer TIM2 when enabled.
 */
#if STM32_TIMCAP_USE_TIM2 || defined(__DOXYGEN__)
TIMCAPDriver TIMCAPD2;
#endif

/**
 * @brief   TIMCAPD3 driver identifier.
 * @note    The driver TIMCAPD1 allocates the timer TIM3 when enabled.
 */
#if STM32_TIMCAP_USE_TIM3 || defined(__DOXYGEN__)
TIMCAPDriver TIMCAPD3;
#endif

/**
 * @brief   TIMCAPD4 driver identifier.
 * @note    The driver TIMCAPD4 allocates the timer TIM4 when enabled.
 */
#if STM32_TIMCAP_USE_TIM4 || defined(__DOXYGEN__)
TIMCAPDriver TIMCAPD4;
#endif

/**
 * @brief   TIMCAPD5 driver identifier.
 * @note    The driver TIMCAPD5 allocates the timer TIM5 when enabled.
 */
#if STM32_TIMCAP_USE_TIM5 || defined(__DOXYGEN__)
TIMCAPDriver TIMCAPD5;
#endif

/**
 * @brief   TIMCAPD8 driver identifier.
 * @note    The driver TIMCAPD8 allocates the timer TIM8 when enabled.
 */
#if STM32_TIMCAP_USE_TIM8 || defined(__DOXYGEN__)
TIMCAPDriver TIMCAPD8;
#endif

/**
 * @brief   TIMCAPD9 driver identifier.
 * @note    The driver TIMCAPD9 allocates the timer TIM9 when enabled.
 */
#if STM32_TIMCAP_USE_TIM9 || defined(__DOXYGEN__)
TIMCAPDriver TIMCAPD9;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/


/**
 * @brief   Returns the maximum channel number for the respective TIMCAP driver.
 *          Note: different timer perepherials on the STM32 have between 1 and 4
 *          CCR registers.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 */
static timcapchannel_t timcap_get_max_timer_channel(const TIMCAPDriver *timcapp) {
  //Choose a sane default value
#if STM32_TIMCAP_USE_TIM1 || defined(__DOXYGEN__)
  if( timcapp == &TIMCAPD1 ) {
    return(TIMCAP_CHANNEL_4);
  }
#endif

#if STM32_TIMCAP_USE_TIM2 || defined(__DOXYGEN__)
  if( timcapp == &TIMCAPD2 ) {
    return(TIMCAP_CHANNEL_4);
  }
#endif

#if STM32_TIMCAP_USE_TIM3 || defined(__DOXYGEN__)
  if( timcapp == &TIMCAPD3 ) {
    return(TIMCAP_CHANNEL_4);
  }
#endif

#if STM32_TIMCAP_USE_TIM4 || defined(__DOXYGEN__)
  if( timcapp == &TIMCAPD4 ) {
    return(TIMCAP_CHANNEL_4);
  }
#endif

#if STM32_TIMCAP_USE_TIM5 || defined(__DOXYGEN__)
  if( timcapp == &TIMCAPD5 ) {
    return(TIMCAP_CHANNEL_4);
  }
#endif

#if STM32_TIMCAP_USE_TIM8 || defined(__DOXYGEN__)
  if( timcapp == &TIMCAPD8 ) {
    return(TIMCAP_CHANNEL_4);
  }
#endif

#if STM32_TIMCAP_USE_TIM9 || defined(__DOXYGEN__)
  if( timcapp == &TIMCAPD9 ) {
    return(TIMCAP_CHANNEL_2);
  }
#endif

  /*Return a conservative default value.*/
  return(TIMCAP_CHANNEL_1);
}


/**
 * @brief   Returns the maximum value for the ARR register of a given timer.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 */
static uint32_t timcap_get_max_arr(const TIMCAPDriver *timcapp) {
  //Choose a sane default value
#if STM32_TIMCAP_USE_TIM1 || defined(__DOXYGEN__)
  if( timcapp == &TIMCAPD1 ) {
    return(UINT16_MAX);
  }
#endif

#if STM32_TIMCAP_USE_TIM2 || defined(__DOXYGEN__)
  if( timcapp == &TIMCAPD2 ) {
    return(UINT32_MAX);
  }
#endif

#if STM32_TIMCAP_USE_TIM3 || defined(__DOXYGEN__)
  if( timcapp == &TIMCAPD3 ) {
    return(UINT16_MAX);
  }
#endif

#if STM32_TIMCAP_USE_TIM4 || defined(__DOXYGEN__)
  if( timcapp == &TIMCAPD4 ) {
    return(UINT16_MAX);
  }
#endif

#if STM32_TIMCAP_USE_TIM5 || defined(__DOXYGEN__)
  if( timcapp == &TIMCAPD5 ) {
    return(UINT32_MAX);
  }
#endif

#if STM32_TIMCAP_USE_TIM8 || defined(__DOXYGEN__)
  if( timcapp == &TIMCAPD8 ) {
    return(UINT16_MAX);
  }
#endif

#if STM32_TIMCAP_USE_TIM9 || defined(__DOXYGEN__)
  if( timcapp == &TIMCAPD9 ) {
    return(UINT16_MAX);
  }
#endif

  /*Return a conservative default value.*/
  return(UINT16_MAX);
}

/**
 * @brief   Shared IRQ handler.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 */
static void timcap_lld_serve_interrupt(TIMCAPDriver *timcapp) {
  uint16_t sr;

  sr  = timcapp->tim->SR;
  sr &= timcapp->tim->DIER & STM32_TIM_DIER_IRQ_MASK;
  timcapp->tim->SR = ~sr;

  if ((sr & STM32_TIM_SR_CC1IF) != 0 && timcapp->config->capture_cb_array[TIMCAP_CHANNEL_1] != NULL )
    _timcap_isr_invoke_channel1_cb(timcapp);

  if ((sr & STM32_TIM_SR_CC2IF) != 0 && timcapp->config->capture_cb_array[TIMCAP_CHANNEL_2] != NULL )
    _timcap_isr_invoke_channel2_cb(timcapp);

  if ((sr & STM32_TIM_SR_CC3IF) != 0 && timcapp->config->capture_cb_array[TIMCAP_CHANNEL_3] != NULL )
    _timcap_isr_invoke_channel3_cb(timcapp);

  if ((sr & STM32_TIM_SR_CC4IF) != 0 && timcapp->config->capture_cb_array[TIMCAP_CHANNEL_4] != NULL )
    _timcap_isr_invoke_channel4_cb(timcapp);

  if ((sr & STM32_TIM_SR_UIF) != 0 && timcapp->config->overflow_cb != NULL)
    _timcap_isr_invoke_overflow_cb(timcapp);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if STM32_TIMCAP_USE_TIM1
#if !defined(STM32_TIM1_UP_HANDLER)
#error "STM32_TIM1_UP_HANDLER not defined"
#endif
/**
 * @brief   TIM1 compare interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_TIM1_UP_HANDLER) {

  CH_IRQ_PROLOGUE();

  timcap_lld_serve_interrupt(&TIMCAPD1);

  CH_IRQ_EPILOGUE();
}

#if !defined(STM32_TIM1_CC_HANDLER)
#error "STM32_TIM1_CC_HANDLER not defined"
#endif
/**
 * @brief   TIM1 compare interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_TIM1_CC_HANDLER) {

  CH_IRQ_PROLOGUE();

  timcap_lld_serve_interrupt(&TIMCAPD1);

  CH_IRQ_EPILOGUE();
}
#endif /* STM32_TIMCAP_USE_TIM1 */

#if STM32_TIMCAP_USE_TIM2
#if !defined(STM32_TIM2_HANDLER)
#error "STM32_TIM2_HANDLER not defined"
#endif
/**
 * @brief   TIM2 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_TIM2_HANDLER) {

  CH_IRQ_PROLOGUE();

  timcap_lld_serve_interrupt(&TIMCAPD2);

  CH_IRQ_EPILOGUE();
}
#endif /* STM32_TIMCAP_USE_TIM2 */

#if STM32_TIMCAP_USE_TIM3
#if !defined(STM32_TIM3_HANDLER)
#error "STM32_TIM3_HANDLER not defined"
#endif
/**
 * @brief   TIM3 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_TIM3_HANDLER) {

  CH_IRQ_PROLOGUE();

  timcap_lld_serve_interrupt(&TIMCAPD3);

  CH_IRQ_EPILOGUE();
}
#endif /* STM32_TIMCAP_USE_TIM3 */

#if STM32_TIMCAP_USE_TIM4
#if !defined(STM32_TIM4_HANDLER)
#error "STM32_TIM4_HANDLER not defined"
#endif
/**
 * @brief   TIM4 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_TIM4_HANDLER) {

  CH_IRQ_PROLOGUE();

  timcap_lld_serve_interrupt(&TIMCAPD4);

  CH_IRQ_EPILOGUE();
}
#endif /* STM32_TIMCAP_USE_TIM4 */

#if STM32_TIMCAP_USE_TIM5
#if !defined(STM32_TIM5_HANDLER)
#error "STM32_TIM5_HANDLER not defined"
#endif
/**
 * @brief   TIM5 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_TIM5_HANDLER) {

  CH_IRQ_PROLOGUE();

  timcap_lld_serve_interrupt(&TIMCAPD5);

  CH_IRQ_EPILOGUE();
}
#endif /* STM32_TIMCAP_USE_TIM5 */

#if STM32_TIMCAP_USE_TIM8
#if !defined(STM32_TIM8_UP_HANDLER)
#error "STM32_TIM8_UP_HANDLER not defined"
#endif
/**
 * @brief   TIM8 compare interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_TIM8_UP_HANDLER) {

  CH_IRQ_PROLOGUE();

  timcap_lld_serve_interrupt(&TIMCAPD8);

  CH_IRQ_EPILOGUE();
}

#if !defined(STM32_TIM8_CC_HANDLER)
#error "STM32_TIM8_CC_HANDLER not defined"
#endif
/**
 * @brief   TIM8 compare interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_TIM8_CC_HANDLER) {

  CH_IRQ_PROLOGUE();

  timcap_lld_serve_interrupt(&TIMCAPD8);

  CH_IRQ_EPILOGUE();
}
#endif /* STM32_TIMCAP_USE_TIM8 */

#if STM32_TIMCAP_USE_TIM9
#if !defined(STM32_TIM9_HANDLER)
#error "STM32_TIM9_HANDLER not defined"
#endif
/**
 * @brief   TIM9 interrupt handler.
 * @note    It is assumed that the various sources are only activated if the
 *          associated callback pointer is not equal to @p NULL in order to not
 *          perform an extra check in a potentially critical interrupt handler.
 *
 * @isr
 */
CH_IRQ_HANDLER(STM32_TIM9_HANDLER) {

  CH_IRQ_PROLOGUE();

  timcap_lld_serve_interrupt(&TIMCAPD9);

  CH_IRQ_EPILOGUE();
}
#endif /* STM32_TIMCAP_USE_TIM9 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level TIMCAP driver initialization.
 *
 * @notapi
 */
void timcap_lld_init(void) {

#if STM32_TIMCAP_USE_TIM1
  /* Driver initialization.*/
  timcapObjectInit(&TIMCAPD1);
  TIMCAPD1.tim = STM32_TIM1;
#endif

#if STM32_TIMCAP_USE_TIM2
  /* Driver initialization.*/
  timcapObjectInit(&TIMCAPD2);
  TIMCAPD2.tim = STM32_TIM2;
#endif

#if STM32_TIMCAP_USE_TIM3
  /* Driver initialization.*/
  timcapObjectInit(&TIMCAPD3);
  TIMCAPD3.tim = STM32_TIM3;
#endif

#if STM32_TIMCAP_USE_TIM4
  /* Driver initialization.*/
  timcapObjectInit(&TIMCAPD4);
  TIMCAPD4.tim = STM32_TIM4;
#endif

#if STM32_TIMCAP_USE_TIM5
  /* Driver initialization.*/
  timcapObjectInit(&TIMCAPD5);
  TIMCAPD5.tim = STM32_TIM5;
#endif

#if STM32_TIMCAP_USE_TIM8
  /* Driver initialization.*/
  timcapObjectInit(&TIMCAPD8);
  TIMCAPD8.tim = STM32_TIM8;
#endif

#if STM32_TIMCAP_USE_TIM9
  /* Driver initialization.*/
  timcapObjectInit(&TIMCAPD9);
  TIMCAPD9.tim = STM32_TIM9;
#endif
}

/**
 * @brief   Configures and activates the TIMCAP peripheral.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 *
 * @notapi
 */
void timcap_lld_start(TIMCAPDriver *timcapp) {
  uint32_t psc;

  const timcapchannel_t tim_max_channel = timcap_get_max_timer_channel(timcapp);

  if (timcapp->state == TIMCAP_STOP) {
    /* Clock activation and timer reset.*/
#if STM32_TIMCAP_USE_TIM1
    if (&TIMCAPD1 == timcapp) {
      rccEnableTIM1(FALSE);
      rccResetTIM1();
      nvicEnableVector(STM32_TIM1_UP_NUMBER, STM32_TIMCAP_TIM1_IRQ_PRIORITY);
      nvicEnableVector(STM32_TIM1_CC_NUMBER, STM32_TIMCAP_TIM1_IRQ_PRIORITY);
#if defined(STM32_TIM1CLK)
      timcapp->clock = STM32_TIM1CLK;
#else
      timcapp->clock = STM32_TIMCLK2;
#endif
    }
#endif
#if STM32_TIMCAP_USE_TIM2
    if (&TIMCAPD2 == timcapp) {
      rccEnableTIM2(FALSE);
      rccResetTIM2();
      nvicEnableVector(STM32_TIM2_NUMBER, STM32_TIMCAP_TIM2_IRQ_PRIORITY);
      timcapp->clock = STM32_TIMCLK1;
    }
#endif
#if STM32_TIMCAP_USE_TIM3
    if (&TIMCAPD3 == timcapp) {
      rccEnableTIM3(FALSE);
      rccResetTIM3();
      nvicEnableVector(STM32_TIM3_NUMBER, STM32_TIMCAP_TIM3_IRQ_PRIORITY);
      timcapp->clock = STM32_TIMCLK1;
    }
#endif
#if STM32_TIMCAP_USE_TIM4
    if (&TIMCAPD4 == timcapp) {
      rccEnableTIM4(FALSE);
      rccResetTIM4();
      nvicEnableVector(STM32_TIM4_NUMBER, STM32_TIMCAP_TIM4_IRQ_PRIORITY);
      timcapp->clock = STM32_TIMCLK1;
    }
#endif
#if STM32_TIMCAP_USE_TIM5
    if (&TIMCAPD5 == timcapp) {
      rccEnableTIM5(FALSE);
      rccResetTIM5();
      nvicEnableVector(STM32_TIM5_NUMBER, STM32_TIMCAP_TIM5_IRQ_PRIORITY);
      timcapp->clock = STM32_TIMCLK1;
    }
#endif
#if STM32_TIMCAP_USE_TIM8
    if (&TIMCAPD8 == timcapp) {
      rccEnableTIM8(FALSE);
      rccResetTIM8();
      nvicEnableVector(STM32_TIM8_UP_NUMBER, STM32_TIMCAP_TIM8_IRQ_PRIORITY);
      nvicEnableVector(STM32_TIM8_CC_NUMBER, STM32_TIMCAP_TIM8_IRQ_PRIORITY);
#if defined(STM32_TIM8CLK)
      timcapp->clock = STM32_TIM8CLK;
#else
      timcapp->clock = STM32_TIMCLK2;
#endif
    }
#endif
#if STM32_TIMCAP_USE_TIM9
    if (&TIMCAPD9 == timcapp) {
      rccEnableTIM9(FALSE);
      rccResetTIM9();
      nvicEnableVector(STM32_TIM9_NUMBER, STM32_TIMCAP_TIM9_IRQ_PRIORITY);
      timcapp->clock = STM32_TIMCLK1;
    }
#endif
  }
  else {
    /* Driver re-configuration scenario, it must be stopped first.*/
    timcapp->tim->CR1    = 0;                  /* Timer disabled.              */
    timcapp->tim->DIER   = timcapp->config->dier &/* DMA-related DIER settings.   */
                        ~STM32_TIM_DIER_IRQ_MASK;
    timcapp->tim->SR     = 0;                  /* Clear eventual pending IRQs. */
    timcapp->tim->CCR[0] = 0;                  /* Comparator 1 disabled.       */
    timcapp->tim->CCR[1] = 0;                  /* Comparator 2 disabled.       */
    if( tim_max_channel >= TIMCAP_CHANNEL_3 )
      timcapp->tim->CCR[2] = 0;                /* Comparator 3 disabled.       */
    if( tim_max_channel >= TIMCAP_CHANNEL_4 )
      timcapp->tim->CCR[3] = 0;                /* Comparator 4 disabled.       */
    timcapp->tim->CNT    = 0;                  /* Counter reset to zero.       */
  }

  /* Timer configuration.*/
  psc = (timcapp->clock / timcapp->config->frequency) - 1;
  osalDbgAssert((psc <= 0xFFFF) &&
              ((psc + 1) * timcapp->config->frequency) == timcapp->clock,
              "invalid frequency");
  timcapp->tim->PSC  = (uint16_t)psc;
  timcapp->tim->ARR = timcap_get_max_arr(timcapp);

  timcapp->tim->CCMR1 = 0;
  timcapp->tim->CCMR2 = 0;
  timcapp->tim->CCER = 0;

  timcapchannel_t chan = TIMCAP_CHANNEL_1;

  /*go through each non-NULL callback channel and enable the capture register on rising/falling edge*/
  for( chan = TIMCAP_CHANNEL_1; chan <= tim_max_channel; chan++ ) {
    if( timcapp->config->capture_cb_array[chan] == NULL ) {
      continue;
    }

    switch (chan) {
      case TIMCAP_CHANNEL_1:
        /*CCMR1_CC1S = 01 = CH1 Input on TI1.*/
        timcapp->tim->CCMR1 |= STM32_TIM_CCMR1_CC1S(1);
        break;
      case TIMCAP_CHANNEL_2:
        /*CCMR1_CC2S = 10 = CH2 Input on TI1.*/
        timcapp->tim->CCMR1 |= STM32_TIM_CCMR1_CC2S(1);
        break;
      case TIMCAP_CHANNEL_3:
        timcapp->tim->CCMR2 |= STM32_TIM_CCMR2_CC3S(1);
        break;
      case TIMCAP_CHANNEL_4:
        timcapp->tim->CCMR2 |= STM32_TIM_CCMR2_CC4S(1);
        break;
    }

    /* The CCER settings depend on the selected trigger mode.
       TIMCAP_INPUT_DISABLED: Input not used.
       TIMCAP_INPUT_ACTIVE_HIGH: Active on rising edge, idle on falling edge.
       TIMCAP_INPUT_ACTIVE_LOW:  Active on falling edge, idle on rising edge.*/
    if (timcapp->config->modes[chan] == TIMCAP_INPUT_ACTIVE_HIGH) {
      switch (chan) {
        case TIMCAP_CHANNEL_1:
          timcapp->tim->CCER |= STM32_TIM_CCER_CC1E;
          break;
        case TIMCAP_CHANNEL_2:
          timcapp->tim->CCER |= STM32_TIM_CCER_CC2E;
          break;
        case TIMCAP_CHANNEL_3:
          timcapp->tim->CCER |= STM32_TIM_CCER_CC3E;
          break;
        case TIMCAP_CHANNEL_4:
          timcapp->tim->CCER |= STM32_TIM_CCER_CC4E;
          break;
      }
    }
    else if (timcapp->config->modes[chan] == TIMCAP_INPUT_ACTIVE_LOW) {
      switch (chan) {
        case TIMCAP_CHANNEL_1:
          timcapp->tim->CCER |= STM32_TIM_CCER_CC1E | STM32_TIM_CCER_CC1P;
          break;
        case TIMCAP_CHANNEL_2:
          timcapp->tim->CCER |= STM32_TIM_CCER_CC2E | STM32_TIM_CCER_CC2P;
          break;
        case TIMCAP_CHANNEL_3:
          timcapp->tim->CCER |= STM32_TIM_CCER_CC3E | STM32_TIM_CCER_CC3P;
          break;
        case TIMCAP_CHANNEL_4:
          timcapp->tim->CCER |= STM32_TIM_CCER_CC4E | STM32_TIM_CCER_CC4P;
          break;
      }
    }
    else {
      switch (chan) {
        case TIMCAP_CHANNEL_1:
          timcapp->tim->CCER &= ~STM32_TIM_CCER_CC1E;
          break;
        case TIMCAP_CHANNEL_2:
          timcapp->tim->CCER &= ~STM32_TIM_CCER_CC2E;
          break;
        case TIMCAP_CHANNEL_3:
          timcapp->tim->CCER &= ~STM32_TIM_CCER_CC3E;
          break;
        case TIMCAP_CHANNEL_4:
          timcapp->tim->CCER &= ~STM32_TIM_CCER_CC4E;
          break;
      }
    }
    /* Direct pointers to the capture registers in order to make reading
         data faster from within callbacks.*/
    timcapp->ccr_p[chan] = &timcapp->tim->CCR[chan];
  }

  /* SMCR_TS  = 101, input is TI1FP1.*/
  timcapp->tim->SMCR  = STM32_TIM_SMCR_TS(5);
}

/**
 * @brief   Deactivates the TIMCAP peripheral.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 *
 * @notapi
 */
void timcap_lld_stop(TIMCAPDriver *timcapp) {

  if (timcapp->state == TIMCAP_READY) {
    /* Clock deactivation.*/
    timcapp->tim->CR1  = 0;                    /* Timer disabled.              */
    timcapp->tim->DIER = 0;                    /* All IRQs disabled.           */
    timcapp->tim->SR   = 0;                    /* Clear eventual pending IRQs. */

#if STM32_TIMCAP_USE_TIM1
    if (&TIMCAPD1 == timcapp) {
      nvicDisableVector(STM32_TIM1_UP_NUMBER);
      nvicDisableVector(STM32_TIM1_CC_NUMBER);
      rccDisableTIM1();
    }
#endif
#if STM32_TIMCAP_USE_TIM2
    if (&TIMCAPD2 == timcapp) {
      nvicDisableVector(STM32_TIM2_NUMBER);
      rccDisableTIM2();
    }
#endif
#if STM32_TIMCAP_USE_TIM3
    if (&TIMCAPD3 == timcapp) {
      nvicDisableVector(STM32_TIM3_NUMBER);
      rccDisableTIM3();
    }
#endif
#if STM32_TIMCAP_USE_TIM4
    if (&TIMCAPD4 == timcapp) {
      nvicDisableVector(STM32_TIM4_NUMBER);
      rccDisableTIM4();
    }
#endif
#if STM32_TIMCAP_USE_TIM5
    if (&TIMCAPD5 == timcapp) {
      nvicDisableVector(STM32_TIM5_NUMBER);
      rccDisableTIM5();
    }
#endif
#if STM32_TIMCAP_USE_TIM8
    if (&TIMCAPD8 == timcapp) {
      nvicDisableVector(STM32_TIM8_UP_NUMBER);
      nvicDisableVector(STM32_TIM8_CC_NUMBER);
      rccDisableTIM8();
    }
#endif
#if STM32_TIMCAP_USE_TIM9
    if (&TIMCAPD9 == timcapp) {
      nvicDisableVector(STM32_TIM9_NUMBER);
      rccDisableTIM9();
    }
#endif
  }
}

/**
 * @brief   Enables the input capture.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 *
 * @notapi
 */
void timcap_lld_enable(TIMCAPDriver *timcapp) {

  timcapp->tim->EGR |= STM32_TIM_EGR_UG;
  timcapp->tim->SR = 0;                        /* Clear pending IRQs (if any). */

  timcapchannel_t chan = TIMCAP_CHANNEL_1;
  const timcapchannel_t tim_max_channel = timcap_get_max_timer_channel(timcapp);
  for( chan = TIMCAP_CHANNEL_1; chan <= tim_max_channel; chan++ ) {
    if( timcapp->config->capture_cb_array[chan] != NULL 
      && timcapp->config->modes[chan] != TIMCAP_INPUT_DISABLED ) {
      switch (chan) {
        case TIMCAP_CHANNEL_1:
          timcapp->tim->DIER |= STM32_TIM_DIER_CC1IE;
          break;
        case TIMCAP_CHANNEL_2:
          timcapp->tim->DIER |= STM32_TIM_DIER_CC2IE;
          break;
        case TIMCAP_CHANNEL_3:
          timcapp->tim->DIER |= STM32_TIM_DIER_CC3IE;
          break;
        case TIMCAP_CHANNEL_4:
          timcapp->tim->DIER |= STM32_TIM_DIER_CC4IE;
          break;
      }
    }
  }

  if (timcapp->config->overflow_cb != NULL)
    timcapp->tim->DIER |= STM32_TIM_DIER_UIE;
  
  timcapp->tim->CR1 = STM32_TIM_CR1_URS | STM32_TIM_CR1_CEN | timcapp->config->cr1;
}

/**
 * @brief   Disables the input capture.
 *
 * @param[in] timcapp      pointer to the @p TIMCAPDriver object
 *
 * @notapi
 */
void timcap_lld_disable(TIMCAPDriver *timcapp) {

  timcapp->tim->CR1   = 0;                     /* Initially stopped.           */
  timcapp->tim->SR    = 0;                     /* Clear pending IRQs (if any). */

  /* All interrupts disabled.*/
  timcapp->tim->DIER &= ~STM32_TIM_DIER_IRQ_MASK;
}

#endif /* HAL_USE_TIMCAP */

/** @} */
