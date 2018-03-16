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
 * Jump to bootloader on MCHCK.
 */
#define SCB_AIRCR_VECTKEY_WRITEMAGIC 0x05FA0000
const uint8_t sys_reset_to_loader_magic[] = "\xff\x00\x7fRESET TO LOADER\x7f\x00\xff";

void jump_to_bootloader(void) {
  __builtin_memcpy((void *)VBAT, (const void *)sys_reset_to_loader_magic, sizeof(sys_reset_to_loader_magic));
  // request reset
  SCB->AIRCR = SCB_AIRCR_VECTKEY_WRITEMAGIC | SCB_AIRCR_SYSRESETREQ_Msk;
}

/*
 * Blink thread.
 */

static THD_WORKING_AREA(waBlinkThread, 128);
static THD_FUNCTION(BlinkThread, arg) {
  (void)arg;
  uint8_t i;

  // while(TRUE) {
  for(i=0; i<10; i++) {
    palTogglePad(GPIOB, GPIOB_LED);
    chThdSleepMilliseconds(700);
  }
  jump_to_bootloader();
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
   * Normal main() thread activity, in this demo it does nothing.
   */
  while(TRUE) {
    chThdSleepMilliseconds(500);
  }

  return 0;
}
