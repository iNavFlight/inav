/*
    Copyright (C) 2015 Stephen Caudle

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
 * GPT callback for GPTD2.
 */
static void gptcallback1(GPTDriver *gptp) {

  (void)gptp;
  palTogglePad(IOPORT1, LED1);

  /*
   * Start a one-shot timer (@ 250ms)
   */
  chSysLockFromISR();
  gptStartOneShotI(&GPTD3, 15625);
  chSysUnlockFromISR();
}

/*
 * GPT callback for GPTD3.
 */
static void gptcallback2(GPTDriver *gptp) {

  (void)gptp;
  palTogglePad(IOPORT1, LED2);
}

/*
 * GPT configuration
 * Frequency: 31250Hz (32us period)
 * Resolution: 16 bits
 */
static const GPTConfig gptcfg1 = {
  31250,
  gptcallback1,
  16,
};

/*
 * GPT configuration
 * Frequency: 62500Hz (16us period)
 * Resolution: 16 bits
 */
static const GPTConfig gptcfg2 = {
  62500,
  gptcallback2,
  16,
};

/*
 * LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palTogglePad(IOPORT1, LED0);
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
   * Sets up the GPT timers
   */
  gptStart(&GPTD2, &gptcfg1);
  gptStart(&GPTD3, &gptcfg2);

  /*
   * Start a continuous timer
   */
  gptStartContinuous(&GPTD2, 15625);

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while (true) {
    if (palReadPad(IOPORT1, KEY1) == 0) {
      gptStopTimer(&GPTD2);
      gptStop(&GPTD2);
      gptStopTimer(&GPTD3);
      gptStop(&GPTD3);
    }
    chThdSleepMilliseconds(500);
  }
}
