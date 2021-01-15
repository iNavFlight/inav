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
#include "rt_test_root.h"
#include "oslib_test_root.h"

/* See: https://lancaster-university.github.io/microbit-docs/ubit/display/
 */

static THD_WORKING_AREA(waThread1, 64);
static THD_FUNCTION(Thread1, arg) {  
  (void)arg;
  chRegSetThreadName("Blinker");

  ioline_t cols[] = {
      LINE_LED_COL_1, LINE_LED_COL_2, LINE_LED_COL_3,
      LINE_LED_COL_4, LINE_LED_COL_5, LINE_LED_COL_6,
      LINE_LED_COL_7, LINE_LED_COL_8, LINE_LED_COL_9,
      PAL_NOLINE
  };
  for (ioline_t *col = cols ; *col != PAL_NOLINE ; col++) 
      palClearLine(col);

  while (1) {
      palSetLine(LINE_LED_ROW_2);
      chThdSleepMilliseconds(100);
      palClearLine(LINE_LED_ROW_2);
      chThdSleepMilliseconds(500);
  }
}


/*
 * Application entry point.
 */
int main(void) {

  SerialConfig serial_config = {
    .speed  = 115200,
    .tx_pad = IOPORT1_UART_TX,
    .rx_pad = IOPORT1_UART_RX,
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
