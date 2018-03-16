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

#include "ch.h"
#include "hal.h"

/*===========================================================================*/
/* GPT driver related.                                                       */
/*===========================================================================*/

/*
 * GPT4 configuration. This timer is used as trigger for the ADC.
 */
static const GPTConfig gpt4cfg1 = {
  .frequency =  1000000U,
  .callback  =  NULL,
  .cr2       =  TIM_CR2_MMS_1,  /* MMS = 010 = TRGO on Update Event.        */
  .dier      =  0U
};

/*===========================================================================*/
/* ADC driver related.                                                       */
/*===========================================================================*/

#define ADC_GRP1_NUM_CHANNELS   2
#define ADC_GRP1_BUF_DEPTH      64

adcsample_t samples1[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];

/*
 * ADC streaming callback.
 */
size_t nx = 0, ny = 0;
static void adccallback(ADCDriver *adcp, adcsample_t *buffer, size_t n) {

  (void)adcp;

  /* Updating counters.*/
  if (samples1 == buffer) {
    nx += n;
  }
  else {
    ny += n;
  }
}

/*
 * ADC errors callback, should never happen.
 */
static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {

  (void)adcp;
  (void)err;
}

/*
 * ADC conversion group.
 * Mode:        Continuous, 16 samples of 2 channels, HS triggered by
 *              GPT4-TRGO.
 * Channels:    VRef, PC1.
 */
static const ADCConversionGroup adcgrpcfg1 = {
  true,
  ADC_GRP1_NUM_CHANNELS,
  adccallback,
  adcerrorcallback,
  ADC_CFGR_CONT | ADC_CFGR_EXTEN_RISING | ADC_CFGR_EXTSEL_SRC(12), /* CFGR   */
  ADC_TR(0, 4095),                                                 /* TR1    */
  {                                                                /* SMPR[2]*/
    ADC_SMPR1_SMP_AN0(ADC_SMPR_SMP_247P5),
    ADC_SMPR1_SMP_AN2(ADC_SMPR_SMP_247P5)
  },
  {                                                                /* SQR[4] */
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN0) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN2),
    0,
    0,
    0
  }
};

/*===========================================================================*/
/* Application code.                                                         */
/*===========================================================================*/

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED attached to TP1.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  palSetLineMode(LINE_ARD_D13, PAL_MODE_OUTPUT_PUSHPULL);
  while (true) {
    palSetLine(LINE_ARD_D13);
    chThdSleepMilliseconds(500);
    palClearLine(LINE_ARD_D13);
    chThdSleepMilliseconds(500);
  }
}

/*
 * Application entry point.
 */
int main(void) {

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /*
   * Activates the serial driver 2 using the driver default configuration.
   */
  sdStart(&SD2, NULL);

  /*
   * Starting GPT4 driver, it is used for triggering the ADC.
   */
  gptStart(&GPTD4, &gpt4cfg1);

  /*
   * Activates the ADC1 driver and the temperature sensor.
   */
  adcStart(&ADCD1, NULL);
  adcSTM32EnableVREF(&ADCD1);
  adcSTM32EnableTS(&ADCD1);

  /*
   * Starts an ADC continuous conversion triggered with a period of
   * 1/1000000 second.
   */
  adcStartConversion(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);
  gptStartContinuous(&GPTD4, 100);

  /*
   * Creates the example thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while (true) {
    chThdSleepMilliseconds(500);
  }
}
