/*
    Copyright (C) 2015 Fabio Utzig

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

static THD_WORKING_AREA(waThread1, 64);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  uint8_t led = LED0;
  chRegSetThreadName("Blinker");
  while (1) {
    palSetPad(IOPORT1, led);
    chThdSleepMilliseconds(100);
    palClearPad(IOPORT1, led);
    if (++led > LED4) led = LED0;
  }
}

/*
 * Application entry point.
 */
int main(void) {

  SerialConfig serial_config = {
    .speed = 38400,
    .tx_pad = UART_TX,
    .rx_pad = UART_RX,
  };

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
   * Activates UART0 using the driver default configuration.
   */
  sdStart(&SD1, &serial_config);

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  test_execute((BaseSequentialStream *)&SD1, &rt_test_suite);
  test_execute((BaseSequentialStream *)&SD1, &oslib_test_suite);
  while (1) {
    chThdSleepMilliseconds(500);
  }
}
