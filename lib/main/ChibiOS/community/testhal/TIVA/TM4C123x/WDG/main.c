/*
    Copyright (C) 2014..2017 Marco Veeneman

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

static bool watchdog_timeout(WDGDriver *wdgp)
{
  (void)wdgp;

  palSetLine(LINE_LED_RED);

  /* Return true to prevent a reset on the next timeout.*/
  return true;
}

/*
 * Watchdog deadline set to one second.
 * Use callback on first timeout.
 * Stall timer if paused by debugger.
 */
static const WDGConfig wdgcfg =
{
  TIVA_SYSCLK,
  watchdog_timeout,
  WDT_TEST_STALL
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

  palSetLineMode(LINE_LED_RED, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_LED_BLUE, PAL_MODE_OUTPUT_PUSHPULL);

  palSetLineMode(LINE_SW1, PAL_MODE_INPUT_PULLUP);

  /*
   * Starting the watchdog driver.
   */
  wdgStart(&WDGD1, &wdgcfg);

  /*
   * Normal main() thread activity, it resets the watchdog.
   */
  while (true) {
    if (palReadLine(LINE_SW1)) {
      /* Only reset the watchdog if the button is not pressed */
      wdgReset(&WDGD1);
      palClearLine(LINE_LED_RED);
    }

    palToggleLine(LINE_LED_BLUE);

    chThdSleepMilliseconds(500);
  }
  return 0;
}
