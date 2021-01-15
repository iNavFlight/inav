/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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

#include "ccportab.h"
#include "portab.h"

/*
 * SPI TX and RX buffers.
 */
CC_ALIGN(32) static uint8_t txbuf[512];
CC_ALIGN(32) static uint8_t rxbuf[512];

#if SPI_SUPPORTS_CIRCULAR == TRUE
/*
 * SPI callback for circular operations.
 */
void spi_circular_cb(SPIDriver *spip) {

  if (spiIsBufferComplete(spip)) {
    /* 2nd half.*/
    palWriteLine(PORTAB_LINE_LED1, PORTAB_LED_OFF);
  }
  else {
    /* 1st half.*/
    palWriteLine(PORTAB_LINE_LED1, PORTAB_LED_ON);
  }
}
#endif

/*
 * SPI bus contender 1.
 */
static THD_WORKING_AREA(spi_thread_1_wa, 256);
static THD_FUNCTION(spi_thread_1, p) {

  (void)p;
  chRegSetThreadName("SPI thread 1");
  while (true) {
    spiAcquireBus(&PORTAB_SPI1);        /* Acquire ownership of the bus.    */
    palWriteLine(PORTAB_LINE_LED1, PORTAB_LED_ON);
    spiStart(&PORTAB_SPI1, &hs_spicfg); /* Setup transfer parameters.       */
    spiSelect(&PORTAB_SPI1);            /* Slave Select assertion.          */
    spiExchange(&PORTAB_SPI1, 512,
                txbuf, rxbuf);          /* Atomic transfer operations.      */
    spiUnselect(&PORTAB_SPI1);          /* Slave Select de-assertion.       */
    cacheBufferInvalidate(&txbuf[0],    /* Cache invalidation over the      */
                          sizeof txbuf);/* buffer.                          */
    spiReleaseBus(&PORTAB_SPI1);        /* Ownership release.               */
  }
}

/*
 * SPI bus contender 2.
 */
static THD_WORKING_AREA(spi_thread_2_wa, 256);
static THD_FUNCTION(spi_thread_2, p) {

  (void)p;
  chRegSetThreadName("SPI thread 2");
  while (true) {
    spiAcquireBus(&PORTAB_SPI1);        /* Acquire ownership of the bus.    */
    palWriteLine(PORTAB_LINE_LED1, PORTAB_LED_OFF);
    spiStart(&PORTAB_SPI1, &ls_spicfg); /* Setup transfer parameters.       */
    spiSelect(&PORTAB_SPI1);            /* Slave Select assertion.          */
    spiExchange(&PORTAB_SPI1, 512,
                txbuf, rxbuf);          /* Atomic transfer operations.      */
    spiUnselect(&PORTAB_SPI1);          /* Slave Select de-assertion.       */
    cacheBufferInvalidate(&txbuf[0],    /* Cache invalidation over the      */
                          sizeof txbuf);/* buffer.                          */
    spiReleaseBus(&PORTAB_SPI1);        /* Ownership release.               */
  }
}

/*
 * LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {
  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    bool key_pressed = palReadLine(PORTAB_LINE_BUTTON) == PORTAB_BUTTON_PRESSED;
    systime_t time = key_pressed ? 250 : 500;
#if SPI_SUPPORTS_CIRCULAR == TRUE
    if (key_pressed) {
      spiAbort(&PORTAB_SPI1);
    }
#endif
#if defined(PORTAB_LINE_LED2)
    palToggleLine(PORTAB_LINE_LED2);
#endif
    chThdSleepMilliseconds(time);
  }
}

/*
 * Application entry point.
 */
int main(void) {
  unsigned i;

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
   * Board-dependent GPIO setup code.
   */
  portab_setup();

  /*
   * Creates the blinker thread.
   */
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /*
   * Prepare transmit pattern.
   */
  for (i = 0; i < sizeof(txbuf); i++)
    txbuf[i] = (uint8_t)i;
  cacheBufferFlush(&txbuf[0], sizeof txbuf);

#if SPI_SUPPORTS_CIRCULAR == TRUE
  /*
   * Starting a continuous operation for test.
   */
  spiStart(&PORTAB_SPI1, &c_spicfg);  /* Setup transfer parameters.       */
  spiSelect(&PORTAB_SPI1);            /* Slave Select assertion.          */
  spiExchange(&PORTAB_SPI1, 512,
              txbuf, rxbuf);          /* Atomic transfer operations.      */
  spiUnselect(&PORTAB_SPI1);          /* Slave Select de-assertion.       */
  cacheBufferInvalidate(&txbuf[0],    /* Cache invalidation over the      */
                        sizeof txbuf);/* buffer.                          */
#endif

  /*
   * Starting the transmitter and receiver threads.
   */
  chThdCreateStatic(spi_thread_1_wa, sizeof(spi_thread_1_wa),
                    NORMALPRIO + 1, spi_thread_1, NULL);
  chThdCreateStatic(spi_thread_2_wa, sizeof(spi_thread_2_wa),
                    NORMALPRIO + 1, spi_thread_2, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while (true) {
    chThdSleepMilliseconds(500);
  }
  return 0;
}
