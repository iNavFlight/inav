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

#include "ch.h"
#include "hal.h"

/*
 * GPT1 callback
 */
static void gpt1cb(GPTDriver *gptp)
{
  (void)gptp;
  palClearLine(LINE_LED_RED);
}

/*
 * GPT7 callback
 */
static void gpt7cb(GPTDriver *gptp)
{
  (void)gptp;
  palSetLine(LINE_LED_RED);
  chSysLockFromISR();
  gptStartOneShotI(&GPTD1, 31250);  /* 0.1 second pulse.*/
  chSysUnlockFromISR();
}

/*
 * GPT1 configuration.
 */
static const GPTConfig gpt1cfg =
{
 312500,    /* 312500 kHz timer clock.*/
 gpt1cb,    /* Timer callback.*/
};

/*
 * GPT7 configuration.
 */
static const GPTConfig gpt7cfg =
{
 10000,     /* 10000 kHz timer clock.*/
 gpt7cb,    /* Timer callback.*/
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

  palSetLineMode(LINE_LED_RED, PAL_MODE_OUTPUT_PUSHPULL);

  /*
   * Start the gpt drivers with the custom configurations.
   */
  gptStart(&GPTD1, &gpt1cfg);
  gptStart(&GPTD7, &gpt7cfg);

  /*
   * Normal main() thread activity
   */
  while (TRUE) {
    gptStartContinuous(&GPTD7, 5000);
    chThdSleepMilliseconds(5000);
    gptStopTimer(&GPTD7);
    gptStartContinuous(&GPTD7, 2500);
    chThdSleepMilliseconds(5000);
    gptStopTimer(&GPTD7);
  }
  
  return 0;
}
