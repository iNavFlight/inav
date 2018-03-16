/*
    ChibiOS - (C) 2015 RedoX https://github.com/RedoXyde
              (C) 2015-2016 flabbergast <s3+flabbergast@sdfeu.org>

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

#define POLLED_TEST     FALSE

void gptcb(GPTDriver *gptp) {
  (void)gptp;
  palTogglePad(GPIO_LED_GREEN, PIN_LED_GREEN);
}

/*
 * GPT configuration structure.
 */
static const GPTConfig gpt1cfg = {
  4,
  gptcb
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
   * Turn off the RGB LED.
   */
  palSetPad(GPIO_LED_RED, PIN_LED_RED); /* red */
  palSetPad(GPIO_LED_GREEN, PIN_LED_GREEN); /* green */
  palSetPad(GPIO_LED_BLUE, PIN_LED_BLUE); /* blue */

  /*
   *  Initializes the GPT driver 1.
   */
  gptStart(&GPTD1, &gpt1cfg);

#if !POLLED_TEST
  gptStartContinuous(&GPTD1, 2);
#endif

  while (1) {
#if POLLED_TEST
    gpt_lld_polled_delay(&GPTD1, 1) ;
    palTogglePad(GPIO_LED_GREEN, PIN_LED_GREEN);
#else
    chThdSleepMilliseconds(500);
#endif
  }
}
