/*
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
#include "hal.h"

#define BTN_GPIO TEENSY_PIN2_IOPORT
#define BTN_PIN TEENSY_PIN2

/*
 * Jump to bootloader on ARM Teensies.
 */

static void jump_to_bootloader(void) {
   /* __asm__ volatile("bkpt"); */
   /* Same as above, CMSIS notation: */
  __BKPT(0);
}

/*
 * Blink thread.
 */

static THD_WORKING_AREA(waBlinkThread, 128);
static THD_FUNCTION(BlinkThread, arg) {
  (void)arg;
  uint8_t i;

  // while(TRUE) {
  for(i=0; i<5; i++) {
    palSetPad(TEENSY_PIN13_IOPORT, TEENSY_PIN13);
    chThdSleepMilliseconds(700);
    palClearPad(TEENSY_PIN13_IOPORT, TEENSY_PIN13);
    chThdSleepMilliseconds(700);
  }
  jump_to_bootloader();
}

/*
 * Button thread.
 */

static THD_WORKING_AREA(waButtonThread, 128);
static THD_FUNCTION(ButtonThread, arg) {
  (void)arg;

  while(TRUE) {
    if(palReadPad(BTN_GPIO, BTN_PIN) == PAL_LOW) {
      jump_to_bootloader();
    }
    chThdSleepMilliseconds(50);
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
   * Button init.
   */
  palSetPadMode(BTN_GPIO, BTN_PIN, PAL_MODE_INPUT_PULLUP);

  /*
   * Create the blink thread.
   */
  chThdCreateStatic(waBlinkThread, sizeof(waBlinkThread), NORMALPRIO, BlinkThread, NULL);

  /*
   * Create the button thread.
   */
  chThdCreateStatic(waButtonThread, sizeof(waButtonThread), NORMALPRIO, ButtonThread, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while(TRUE) {
    chThdSleepMilliseconds(500);
  }

  return 0;
}
