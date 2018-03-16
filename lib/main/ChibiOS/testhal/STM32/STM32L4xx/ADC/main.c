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

#define ADC_GRP1_NUM_CHANNELS   2
#define ADC_GRP1_BUF_DEPTH      8

#define ADC_GRP2_NUM_CHANNELS   8
#define ADC_GRP2_BUF_DEPTH      16

static adcsample_t samples1[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
static adcsample_t samples2[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];

/*
 * ADC streaming callback.
 */
size_t nx = 0, ny = 0;
static void adccallback(ADCDriver *adcp, adcsample_t *buffer, size_t n) {

  (void)adcp;
  if (samples2 == buffer) {
    nx += n;
  }
  else {
    ny += n;
  }
}

static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {

  (void)adcp;
  (void)err;
}

/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 2 channels, SW triggered.
 * Channels:    IN1, IN11.
 * Notes:       IN1 = PC0, IN2 = PC1
 */
static const ADCConversionGroup adcgrpcfg1 = {
  FALSE,
  ADC_GRP1_NUM_CHANNELS,
  NULL,
  adcerrorcallback,
  ADC_CFGR_CONT,            /* CFGR    */
  ADC_TR(0, 4095),          /* TR1     */
  {                         /* SMPR[2] */
    0,
    0
  },
  {                         /* SQR[4]  */
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1) | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN2),
    0,
    0,
    0
  }
};

/*
 * ADC conversion group.
 * Mode:        Continuous, 16 samples of 8 channels, SW triggered.
 * Channels:    IN5, IN6, IN5, IN6, IN5, IN6, Sensor, VBat/2.
 * Notes:       IN5 = PA0, IN6 = PA1
 */
static const ADCConversionGroup adcgrpcfg2 = {
  TRUE,
  ADC_GRP2_NUM_CHANNELS,
  adccallback,
  adcerrorcallback,
  ADC_CFGR_CONT,                    /* CFGR    */
  ADC_TR(0, 4095),                  /* TR1     */
  {                                 /* SMPR[2] */
    ADC_SMPR1_SMP_AN5(ADC_SMPR_SMP_24P5)
    | ADC_SMPR1_SMP_AN6(ADC_SMPR_SMP_24P5),
    ADC_SMPR2_SMP_AN17(ADC_SMPR_SMP_47P5)
    | ADC_SMPR2_SMP_AN18(ADC_SMPR_SMP_47P5),
  },
  {                                 /* SQR[4]  */
    ADC_SQR1_SQ1_N(ADC_CHANNEL_IN5)  | ADC_SQR1_SQ2_N(ADC_CHANNEL_IN6) |
    ADC_SQR1_SQ3_N(ADC_CHANNEL_IN5)  | ADC_SQR1_SQ4_N(ADC_CHANNEL_IN6),
    ADC_SQR2_SQ5_N(ADC_CHANNEL_IN5)  | ADC_SQR2_SQ6_N(ADC_CHANNEL_IN6) |
    ADC_SQR2_SQ7_N(ADC_CHANNEL_IN17) | ADC_SQR2_SQ8_N(ADC_CHANNEL_IN18),
    0,
    0
  }
};

/*
 * Red LEDs blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palSetPad(GPIOA, GPIOA_LED_GREEN);
    chThdSleepMilliseconds(500);
    palClearPad(GPIOA, GPIOA_LED_GREEN);
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
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Activates the ADC1 driver and the temperature sensor.
   */
  adcStart(&ADCD1, NULL);
  adcSTM32EnableTS(&ADCD1);
  adcSTM32EnableVBAT(&ADCD1);

  /*
   * Linear conversion.
   */
  adcConvert(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);
  chThdSleepMilliseconds(1000);

  /*
   * Starts an ADC continuous conversion.
   */
  adcStartConversion(&ADCD1, &adcgrpcfg2, samples2, ADC_GRP2_BUF_DEPTH);

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while (true) {
    if (palReadPad(GPIOC, GPIOC_BUTTON)) {
      adcStopConversion(&ADCD1);
    }
    chThdSleepMilliseconds(500);
  }
}
