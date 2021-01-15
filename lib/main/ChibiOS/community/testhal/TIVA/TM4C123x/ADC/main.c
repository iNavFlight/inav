/*
    Copyright (C) 2014..2016 Marco Veeneman

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
#include "chprintf.h"

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
uint32_t temp = 0;
static void adccallback(ADCDriver *adcp, adcsample_t *buffer, size_t n) {

  (void)adcp;
  if (samples2 == buffer) {
    nx += n;

    uint8_t i, j;
    adcsample_t avg = 0;

    for (j = 0; j < n; j++) {
      for (i = 0; i < ADC_GRP2_NUM_CHANNELS; i++) {
        avg += *buffer++;
      }
    }

    avg /= (n * ADC_GRP2_NUM_CHANNELS);

    temp = (uint32_t)(147.5f - ((75.0f * 3.3f * (float)avg)) / 4096.0f);
  }
  else {
    ny += n;

    uint8_t i, j;
    adcsample_t avg = 0;

    for (j = 0; j < n; j++) {
      for (i = 0; i < ADC_GRP2_NUM_CHANNELS; i++) {
        avg += *buffer++;
      }
    }

    avg /= (n * ADC_GRP2_NUM_CHANNELS);

    temp = (uint32_t)(147.5f - ((75.0f * 3.3f * (float)avg)) / 4096.0f);
  }
}

static void adcerrorcallback(ADCDriver *adcp, adcerror_t err) {

  (void)adcp;
  (void)err;
}

/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 2 channels, Always triggered.
 * Channels:    TS, TS.
 */
static const ADCConversionGroup adcgrpcfg1 = {
  FALSE,
  ADC_GRP1_NUM_CHANNELS,
  NULL,
  adcerrorcallback,
  0xF,                    /* EMUX */ /* Always trigger */
  0,                      /* SSMUX */
  (1 << 7) | (1 << 5) |
  (1 << 3)                /* SSCTL */ /* 2 times TS, 2nd has end bit set */
};

/*
 * ADC conversion group.
 * Mode:        Continuous, 16 samples of 8 channels, Always triggered.
 * Channels:    TS, TS, TS, TS, TS, TS, TS, TS.
 */
static const ADCConversionGroup adcgrpcfg2 = {
  TRUE,
  ADC_GRP2_NUM_CHANNELS,
  adccallback,
  adcerrorcallback,
  0xF,                    /* EMUX */ /* Always trigger */
  0,                      /* SSMUX */
  (1 << 31) | (1 << 29) |
  (1 << 27) |
  (1 << 23) |
  (1 << 19) |
  (1 << 15) |
  (1 << 11) |
  (1 << 7) |
  (1 << 3)                /* SSCTL */ /* 8 times TS, 8th has end bit set */
};

/*
 * Application entry point.
 */
int main(void)
{
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /* Configure RX and TX pins for UART0.*/
  palSetLineMode(LINE_UART0_RX, PAL_MODE_INPUT | PAL_MODE_ALTERNATE(1));
  palSetLineMode(LINE_UART0_TX, PAL_MODE_INPUT | PAL_MODE_ALTERNATE(1));

  sdStart(&SD1, NULL);

  chprintf((BaseSequentialStream *)&SD1, "Starting ADC0...");
  chThdSleepMilliseconds(500);

  /*
   * Activates the ADC0 driver.
   */
  adcStart(&ADCD1, NULL);

  /*
   * Linear conversion.
   */
  adcConvert(&ADCD1, &adcgrpcfg1, samples1, ADC_GRP1_BUF_DEPTH);

  /*
   * Starts an ADC continuous conversion.
   */
  adcStartConversion(&ADCD1, &adcgrpcfg2, samples2, ADC_GRP2_BUF_DEPTH);

  /*
   * Normal main() thread activity
   */
  while (TRUE) {
    chThdSleepMilliseconds(500);

    chprintf((BaseSequentialStream *)&SD1, "A:%d\tB:%d\ttmp:%d\r\n", nx, ny, temp);
  }
  
  return 0;
}
