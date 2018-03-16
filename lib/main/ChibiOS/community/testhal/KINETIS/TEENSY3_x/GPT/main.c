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

/*
 * GPT1 callback.
 */
static void gpt1cb(GPTDriver *gptp) {

  (void)gptp;
  palTogglePad(TEENSY_PIN13_IOPORT, TEENSY_PIN13);
}

static const GPTConfig gpt1cfg = {
  10000,    /* 10kHz timer clock.*/
  gpt1cb    /* Timer callback.*/
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
   * Activates the GPT driver 1.
   */
  gptStart(&GPTD1, &gpt1cfg);
  gptStartContinuous(&GPTD1, 5000);   /* 500ms */
  //~ gptPolledDelay(&GPTD1, 10); /* Small delay.*/

  /*
   * Normal main() thread activity, it changes the GPT1 period every
   * five seconds.
   */
  while (!chThdShouldTerminateX()) {
    chThdSleepMilliseconds(5000);
    gptChangeInterval(&GPTD1,gptGetIntervalX(&GPTD1)/2); /* 25ms */
    chThdSleepMilliseconds(5000);
    gptChangeInterval(&GPTD1,gptGetIntervalX(&GPTD1)*2); /* 50ms */
  }
  return 0;
}
