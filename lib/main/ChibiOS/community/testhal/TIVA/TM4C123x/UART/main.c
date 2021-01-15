/*
    ChibiOS - Copyright (C) 2006..2017 Giovanni Di Sirio

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

static virtual_timer_t vt1, vt2;

static void restart(void *p) {

  (void)p;

  chSysLockFromISR();
  uartStartSendI(&UARTD1, 14, "Hello World!\r\n");
  chSysUnlockFromISR();
}

static void ledoff(void *p) {

  (void)p;
  palClearLine(LINE_LED_RED);
}

/*
 * This callback is invoked when a transmission buffer has been completely
 * read by the driver.
 */
static void txend1(UARTDriver *uartp) {

  (void)uartp;
  palSetLine(LINE_LED_RED);
}

/*
 * This callback is invoked when a transmission has physically completed.
 */
static void txend2(UARTDriver *uartp) {

  (void)uartp;
  palClearLine(LINE_LED_RED);
  chSysLockFromISR();
  chVTResetI(&vt1);
  chVTSetI(&vt1, OSAL_MS2I(5000), restart, NULL);
  chSysUnlockFromISR();
}

/*
 * This callback is invoked on a receive error, the errors mask is passed
 * as parameter.
 */
static void rxerr(UARTDriver *uartp, uartflags_t e) {

  (void)uartp;
  (void)e;
}

/*
 * This callback is invoked when a character is received but the application
 * was not ready to receive it, the character is passed as parameter.
 */
static void rxchar(UARTDriver *uartp, uint16_t c) {

  (void)uartp;
  (void)c;
  /* Flashing the LED each time a character is received.*/
  palSetLine(LINE_LED_RED);
  chSysLockFromISR();
  chVTResetI(&vt2);
  chVTSetI(&vt2, OSAL_MS2I(200), ledoff, NULL);
  chSysUnlockFromISR();
}

/*
 * This callback is invoked when a receive buffer has been completely written.
 */
static void rxend(UARTDriver *uartp) {

  (void)uartp;
}

/*
 * UART driver configuration structure.
 */
static UARTConfig uart_cfg_1 = {
  txend1,
  txend2,
  rxend,
  rxchar,
  rxerr,
  38400,
  0,
  UART_LCRH_FEN | UART_LCRH_WLEN_8,
  UART_IFLS_TX4_8 | UART_IFLS_RX4_8,
  UART_CC_CS_SYSCLK
};

/*
 * Application entry point.
 */
int main(void)
{
  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  palSetLineMode(LINE_UART0_RX, PAL_MODE_INPUT | PAL_MODE_ALTERNATE(1));
  palSetLineMode(LINE_UART0_TX, PAL_MODE_INPUT | PAL_MODE_ALTERNATE(1));

  palSetLineMode(LINE_LED_RED, PAL_MODE_OUTPUT_PUSHPULL);

  /*
   * Activates the uart driver 1.
   */
  uartStart(&UARTD1, &uart_cfg_1);

  /*
   * Starts the transmission, it will be handled entirely in background.
   */
  uartStartSend(&UARTD1, 13, "Starting...\r\n");

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while (true) {
    chThdSleepMilliseconds(5000);
  }
}
