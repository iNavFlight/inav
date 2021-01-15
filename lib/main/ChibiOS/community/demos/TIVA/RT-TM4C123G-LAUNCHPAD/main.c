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
#include "rt_test_root.h"
#include "oslib_test_root.h"

typedef struct led_config
{
  ioline_t line;
  uint32_t sleep;
} led_config_t;

/*
 * LED blinker thread.
 */
static THD_WORKING_AREA(waBlinkLedRed, 128);
static THD_WORKING_AREA(waBlinkLedGreen, 128);
static THD_WORKING_AREA(waBlinkLedBlue, 128);
static THD_FUNCTION(blinkLed, arg) {
  led_config_t *ledConfig = (led_config_t*) arg;

  chRegSetThreadName("Blinker");

  palSetLineMode(ledConfig->line, PAL_MODE_OUTPUT_PUSHPULL);

  while (TRUE) {
    chThdSleepMilliseconds(ledConfig->sleep);
    palToggleLine(ledConfig->line);
  }
}

/*
 * Application entry point.
 */
int main(void)
{
  led_config_t ledRed, ledGreen, ledBlue;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /* Configure RX and TX pins for UART0.*/
  palSetLineMode(LINE_UART0_RX, PAL_MODE_INPUT | PAL_MODE_ALTERNATE(1));
  palSetLineMode(LINE_UART0_TX, PAL_MODE_INPUT | PAL_MODE_ALTERNATE(1));

  /* Start the serial driver with the default configuration.*/
  sdStart(&SD1, NULL);

  if (!palReadLine(LINE_SW2)) {
    test_execute((BaseSequentialStream *)&SD1, &rt_test_suite);
    test_execute((BaseSequentialStream *)&SD1, &oslib_test_suite);
  }

  ledRed.line    = LINE_LED_RED;
  ledRed.sleep   = 100;

  ledGreen.line  = LINE_LED_GREEN;
  ledGreen.sleep = 101;

  ledBlue.line   = LINE_LED_BLUE;
  ledBlue.sleep  = 102;

  /* Creating the blinker threads.*/
  chThdCreateStatic(waBlinkLedRed,
                    sizeof(waBlinkLedRed),
                    NORMALPRIO,
                    blinkLed,
                    &ledRed);

  chThdCreateStatic(waBlinkLedGreen,
                    sizeof(waBlinkLedGreen),
                    NORMALPRIO,
                    blinkLed,
                    &ledGreen);

  chThdCreateStatic(waBlinkLedBlue,
                    sizeof(waBlinkLedBlue),
                    NORMALPRIO,
                    blinkLed,
                    &ledBlue);

  /* Normal main() thread activity.*/
  while (TRUE) {
    chThdSleepMilliseconds(100);
  }

  return 0;
}
