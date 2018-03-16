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

/* Triggered when the button is pressed. The blue led is toggled. */
static void extcb1(EXTDriver *extp, expchannel_t channel) {
  (void)extp;
  (void)channel;

  palTogglePad(IOPORT4, 4);
}

static const EXTConfig extcfg = {
  {
   {EXT_CH_MODE_FALLING_EDGE | EXT_CH_MODE_AUTOSTART, extcb1, PORTA, 1}
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

  palSetPad(IOPORT3, 3);    // Red
  palSetPad(IOPORT4, 4);    // Green
  palSetPad(IOPORT1, 2);    // Blue

  /*
   * Activates the EXT driver 1.
   */
  palSetPadMode(IOPORT1, 1, PAL_MODE_INPUT_PULLUP);
  extStart(&EXTD1, &extcfg);

  while (1) {
    chThdSleepMilliseconds(500);
  }
}
