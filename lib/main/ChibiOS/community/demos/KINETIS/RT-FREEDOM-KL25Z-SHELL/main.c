/*
    Copyright (C) 2016 Jonathan Struebel

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

#include "shellcfg.h"

SerialConfig s0cfg = {
  115200
};

/*
 * Blue LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waBlinkThread, 128);
static THD_FUNCTION(BlinkThread, arg) {
  systime_t time = 500;

  (void)arg;

  chRegSetThreadName("blinker");
  while (true) {
    palTogglePad(GPIO_LED_BLUE, PIN_LED_BLUE);
    chThdSleepMilliseconds(time);
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
   * Turn off the RGB LED.
   */
  palSetPad(GPIO_LED_RED, PIN_LED_RED); /* red */
  palSetPad(GPIO_LED_GREEN, PIN_LED_GREEN); /* green */
  palSetPad(GPIO_LED_BLUE, PIN_LED_BLUE); /* blue */

  sdStart(&SD1, &s0cfg);

  /*
   * Shell manager initialization.
   */
  shellInit();

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waBlinkThread, sizeof(waBlinkThread), NORMALPRIO, BlinkThread, NULL);

  while (true) {
    if (SD1.state == SD_READY) {
      thread_t *shelltp = chThdCreateFromHeap(NULL, SHELL_WA_SIZE,
                                              "shell", NORMALPRIO + 1,
                                              shellThread, (void *)&shell_cfg);
      chThdWait(shelltp);               /* Waiting termination.             */
    }
    palTogglePad(GPIO_LED_RED, PIN_LED_RED);
    chThdSleepMilliseconds(1000);
  }
}
