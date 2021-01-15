/*
    ChibiOS - Copyright (C) 2015 RedoX https://github.com/RedoXyde

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
#include "hal.h"

#define ADC_GRP1_NUM_CHANNELS   2
#define ADC_GRP1_BUF_DEPTH      1

static adcsample_t samples1[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
static virtual_timer_t vt;

static void ledoff(void *p) {

  (void)p;
  palClearPad(TEENSY_PIN13_IOPORT, TEENSY_PIN13);
}

static void adc_end_cb(ADCDriver *adcp, adcsample_t *buffer, size_t n) {

  (void)adcp;
  (void)n;

  /*
   * The bandgap value represents the ADC reading for 1.0V
   */
  uint16_t sensor = buffer[0];
  uint16_t bandgap = buffer[1];

  /*
   * The v25 value is the voltage reading at 25C, it comes from the ADC
   * electricals table in the processor manual. V25 is in millivolts.
   */
  int32_t v25 = 716;

  /*
   * The m value is slope of the temperature sensor values, again from
   * the ADC electricals table in the processor manual.
   * M in microvolts per degree.
   */
  int32_t m = 1620;

  /*
   * Divide the temperature sensor reading by the bandgap to get
   * the voltage for the ambient temperature in millivolts.
   */
  int32_t vamb = (sensor * 1000) / bandgap;

  /*
   * This formula comes from the reference manual.
   * Temperature is in millidegrees C.
   */
  int32_t delta = (((vamb - v25) * 1000000) / m);
  int32_t temp = 25000 - delta;

  palSetPad(TEENSY_PIN13_IOPORT, TEENSY_PIN13);
  chSysLockFromISR();
  chVTResetI(&vt);
  if (temp < 19000) {
    chVTSetI(&vt, TIME_MS2I(10), ledoff, NULL);
  } else if (temp > 28000) {
    chVTSetI(&vt, TIME_MS2I(20), ledoff, NULL);
  } else {
    chVTSetI(&vt, TIME_MS2I(40), ledoff, NULL);
  }
  chSysUnlockFromISR();
}


/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 1 channel, SW triggered.
 */
static const ADCConversionGroup adcgrpcfg1 = {
  false,
  ADC_GRP1_NUM_CHANNELS,
  adc_end_cb,
  NULL,
  ADC_TEMP_SENSOR | ADC_BANDGAP,
  /* CFG1 Regiser - ADCCLK = SYSCLK / 16, 16 bits per sample */
  ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_8) |
    ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2) |
    ADCx_CFG1_MODE(ADCx_CFG1_MODE_16_BITS),
  /* SC3 Register - Average 32 readings per sample */
  ADCx_SC3_AVGE |
    ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_32_SAMPLES)
};

static const ADCConfig adccfg1 = {
  /* Perform initial calibration */
  true
};

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
   * Activates the ADC1 driver.
   */
  adcStart(&ADCD1, &adccfg1);

  while (!chThdShouldTerminateX()) {
    /*
       * ADC linear conversion.
       */
    adcConvert(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);

    chThdSleepMilliseconds(1000);
  }

  return 0;
}
