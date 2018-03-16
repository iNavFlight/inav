/*
    ChibiOS/RT - Copyright (C) 2014 Uladzimir Pylinsky aka barthess

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

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#define UART_STORM_BAUDRATE     3000000
#define STORM_BUF_LEN           256

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */

/*
 ******************************************************************************
 * PROTOTYPES
 ******************************************************************************
 */
static void txend1(UARTDriver *uartp);
static void txend2(UARTDriver *uartp);
static void rxerr(UARTDriver *uartp, uartflags_t e);
static void rxchar(UARTDriver *uartp, uint16_t c);
static void rxend(UARTDriver *uartp);

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static uint8_t rxbuf[STORM_BUF_LEN];
static uint8_t txbuf[STORM_BUF_LEN];

/*
 * UART driver configuration structure.
 */
static const UARTConfig uart_cfg = {
  txend1,
  txend2,
  rxend,
  rxchar,
  rxerr,
  UART_STORM_BAUDRATE,
  0,
  0,
  0
};

static uint32_t ints;

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */
/*
 * This callback is invoked when a transmission buffer has been completely
 * read by the driver.
 */
static void txend1(UARTDriver *uartp) {

  ints++;
  chSysLockFromISR();
  uartStartSendI(uartp, STORM_BUF_LEN, txbuf);
  chSysUnlockFromISR();
}

/*
 * This callback is invoked when a transmission has physically completed.
 */
static void txend2(UARTDriver *uartp) {
  (void)uartp;

  chSysLockFromISR();
  chSysUnlockFromISR();
}

/*
 * This callback is invoked on a receive error, the errors mask is passed
 * as parameter.
 */
static void rxerr(UARTDriver *uartp, uartflags_t e) {
  (void)uartp;
  (void)e;
  osalSysHalt("");
}

/*
 * This callback is invoked when a character is received but the application
 * was not ready to receive it, the character is passed as parameter.
 */
static void rxchar(UARTDriver *uartp, uint16_t c) {
  (void)uartp;
  (void)c;
}

/*
 * This callback is invoked when a receive buffer has been completely written.
 */
static void rxend(UARTDriver *uartp) {
  (void)uartp;

  chSysLockFromISR();
  uartStartReceiveI(&UARTD6, STORM_BUF_LEN, rxbuf);
  chSysUnlockFromISR();
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

/**
 *
 */
void dma_storm_uart_start(void){

  uint32_t i;

  for (i=0; i<STORM_BUF_LEN; i++){
    txbuf[i] = 0x55;
    rxbuf[i] = 0;
  }

  ints = 0;
  uartStart(&UARTD6, &uart_cfg);
  uartStartReceive(&UARTD6, STORM_BUF_LEN, rxbuf);
  uartStartSend(&UARTD6, STORM_BUF_LEN, txbuf);
}

uint32_t dma_storm_uart_stop(void){

  uartStopSend(&UARTD6);
  uartStopReceive(&UARTD6);
  uartStop(&UARTD6);

  return ints;
}
