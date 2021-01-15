/*
    Copyright (C) 2016 Stephane D'Alu
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

void timeout_callback(void) {
  palTogglePad(IOPORT1, LED2);
  palTogglePad(IOPORT1, LED3);
  palTogglePad(IOPORT1, LED4);
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

  palSetPad(IOPORT1, LED1);

  WDGConfig WDG_config = {
    .pause_on_sleep = 0,
    .pause_on_halt  = 0,
    .timeout_ms     = 5000,
    .callback       = timeout_callback
  };

  wdgStart(&WDGD1, &WDG_config);

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while (true) {
    if (palReadPad(IOPORT1, BTN1) == 0) {
      palTogglePad(IOPORT1, LED1);
      wdgReset(&WDGD1);
    }
    chThdSleepMilliseconds(500);
  }
}
