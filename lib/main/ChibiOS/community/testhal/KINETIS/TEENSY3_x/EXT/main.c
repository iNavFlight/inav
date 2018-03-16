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

static void ledoff(void *p) {

  (void)p;
  palClearPad(TEENSY_PIN13_IOPORT, TEENSY_PIN13);
}

static virtual_timer_t vt;

/* Triggered when the Teensy's pin 12 (PC7) changes; The LED is set to ON.*/
static void extcb1(EXTDriver *extp, expchannel_t channel) {
  (void)extp;
  (void)channel;
  palSetPad(TEENSY_PIN13_IOPORT, TEENSY_PIN13);
  chSysLockFromISR();
  chVTResetI(&vt);
  /* LED set to OFF after 500mS.*/
  chVTSetI(&vt, MS2ST(500), ledoff, NULL);
  chSysUnlockFromISR();
}

static const EXTConfig extcfg = {
  {
   {EXT_CH_MODE_BOTH_EDGES|EXT_CH_MODE_AUTOSTART, extcb1, PORTC, 7}
  }
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
   * Activates the EXT driver 1.
   */
  extStart(&EXTD1, &extcfg);
  /*
   * Normal main() thread activity, in this demo it enables and disables the
   * button EXT channel using 10 seconds intervals.
   */
  while (!chThdShouldTerminateX()) {
    extChannelDisable(&EXTD1, 0);
    chThdSleepMilliseconds(10000);
    extChannelEnable(&EXTD1, 0);
    chThdSleepMilliseconds(10000);
  }
  return 0;
}
