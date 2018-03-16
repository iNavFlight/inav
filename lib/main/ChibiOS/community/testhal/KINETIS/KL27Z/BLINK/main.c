/*
    ChibiOS/RT KL27 example - Copyright (C) 2015 flabbergast

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
 * Blink thread
 */
static THD_WORKING_AREA(waBlinkThread, 128);
static THD_FUNCTION(BlinkThread, arg) {
  (void)arg;

  while(TRUE) {
    palTogglePad(GPIO_LED, PIN_LED);
    chThdSleepMilliseconds(700);
  }
}

/*
 * Check button thread
 */
static THD_WORKING_AREA(waButtonThread, 128);
static THD_FUNCTION(ButtonThread, arg) {
  (void)arg;
  chRegSetThreadName("buttonThread");

  uint8_t newstate, state = PAL_HIGH;

  while(true) {
    if(palReadPad(GPIO_BUTTON, PIN_BUTTON) != state) {
      chThdSleepMilliseconds(20); /* debounce */
      newstate = palReadPad(GPIO_BUTTON, PIN_BUTTON);
      if(newstate != state) {
        state = newstate;
        if(newstate == PAL_LOW) {
          // palTogglePad(GPIO_LED, PIN_LED);
          /* jump to bootloader */
          /* force boot from ROM */
          RCM->FM = RCM_FM_FORCEROM(2);
          /* request RESET */
          #define SCB_AIRCR_VECTKEY_WRITEMAGIC 0x05FA0000
          SCB->AIRCR = SCB_AIRCR_VECTKEY_WRITEMAGIC | SCB_AIRCR_SYSRESETREQ_Msk;
        }
      }
    }
    chThdSleepMilliseconds(20);
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
   * Create the blink thread.
   */
  chThdCreateStatic(waBlinkThread, sizeof(waBlinkThread), NORMALPRIO, BlinkThread, NULL);

  /*
   * Create the button check thread.
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
