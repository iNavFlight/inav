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

#include "irq_storm.h"

/*
 * GPT4 configuration.
 */
static const GPTConfig gpt4cfg = {
  1000000,              /* 1MHz timer clock.*/
  irq_storm_gpt1_cb,    /* Timer callback.*/
  0,
  0
};

/*
 * GPT3 configuration.
 */
static const GPTConfig gpt3cfg = {
  1000000,              /* 1MHz timer clock.*/
  irq_storm_gpt2_cb,    /* Timer callback.*/
  0,
  0
};

/*
 * IRQ Storm configuration.
 */
static const irq_storm_config_t irq_storm_config = {
  (BaseSequentialStream  *)&SD2,
  GPIOD,
  GPIOD_LED5,
  &GPTD4,
  &GPTD3,
  &gpt4cfg,
  &gpt3cfg,
  STM32_SYSCLK
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

  /* Prepares the Serial driver 2.*/
  sdStart(&SD2, NULL);          /* Default is 38400-8-N-1.*/
  palSetPadMode(GPIOA, 2, PAL_MODE_ALTERNATE(7));
  palSetPadMode(GPIOA, 3, PAL_MODE_ALTERNATE(7));

  /* Running the test.*/
  irq_storm_execute(&irq_storm_config);

  /* Normal main() thread activity, nothing in this test.*/
  while (true) {
    chThdSleepMilliseconds(5000);
  }
}
