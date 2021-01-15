/*
    Copyright (C) 2015 Stephen Caudle

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
 * 1Mbps speed SPI configuration (1MHz, CPHA=0, CPOL=0, LSb first).
 */

static const SPIConfig hs_spicfg = {
  .end_cb=NULL,
  .freq=NRF5_SPI_FREQ_1MBPS,
  .sckpad=SPI_SCK,
  .mosipad=SPI_MOSI,
  .misopad=SPI_MISO,
  .sspad=SPI_SS,
  .lsbfirst=TRUE,
  .mode=0
};

static const SPIConfig ls_spicfg = {
  .end_cb=NULL,
  .freq=NRF5_SPI_FREQ_250KBPS,
  .sckpad=SPI_SCK,
  .mosipad=SPI_MOSI,
  .misopad=SPI_MISO,
  .sspad=SPI_SS,
  .lsbfirst=TRUE,
  .mode=0
};


/*
 * SPI TX and RX buffers.
 */
static uint8_t txbuf[512];
static uint8_t rxbuf[512];

/*
 * SPI bus contender 1.
 */
static THD_WORKING_AREA(spi_thread_1_wa, 256);
static THD_FUNCTION(spi_thread_1, p) {

  (void)p;
  chRegSetThreadName("SPI thread 1");
  while (true) {
    spiAcquireBus(&SPID1);              /* Acquire ownership of the bus.    */
    palSetPad(SPI_PORT_SPI0, LED0);       /* LED ON.                          */
    spiStart(&SPID1, &hs_spicfg);       /* Setup transfer parameters.       */
    spiSelect(&SPID1);                  /* Slave Select assertion.          */
    spiExchange(&SPID1, 512,
                txbuf, rxbuf);          /* Atomic transfer operations.      */
    spiUnselect(&SPID1);                /* Slave Select de-assertion.       */
    spiReleaseBus(&SPID1);              /* Ownership release.               */
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
    spiAcquireBus(&SPID1);              /* Acquire ownership of the bus.    */
    palClearPad(SPI_PORT_SPI0, LED0);     /* LED OFF.                         */
    spiStart(&SPID1, &ls_spicfg);       /* Setup transfer parameters.       */
    spiSelect(&SPID1);                  /* Slave Select assertion.          */
    spiExchange(&SPID1, 512,
                txbuf, rxbuf);          /* Atomic transfer operations.      */
    spiUnselect(&SPID1);                /* Slave Select de-assertion.       */
    spiReleaseBus(&SPID1);              /* Ownership release.               */
  }
}
/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
static THD_WORKING_AREA(blinker_wa, 128);
static THD_FUNCTION(blinker, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palSetPad(SPI_PORT_SPI0, LED1);
    chThdSleepMilliseconds(500);
    palClearPad(SPI_PORT_SPI0, LED1);
    chThdSleepMilliseconds(500);
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
   * Prepare transmit pattern.
   */
  for (i = 0; i < sizeof(txbuf); i++)
    txbuf[i] = (uint8_t)i;

  /*
   * Starting the transmitter and receiver threads.
   */
  chThdCreateStatic(spi_thread_1_wa, sizeof(spi_thread_1_wa),
                    NORMALPRIO + 1, spi_thread_1, NULL);
  chThdCreateStatic(spi_thread_2_wa, sizeof(spi_thread_2_wa),
                    NORMALPRIO + 1, spi_thread_2, NULL);

  /*
   * Starting the blinker thread.
   */
  chThdCreateStatic(blinker_wa, sizeof(blinker_wa),
                    NORMALPRIO-1, blinker, NULL);

  /*
   * Normal main() thread activity, in this demo it does nothing.
   */
  while (true) {
    chThdSleepMilliseconds(500);
  }
  return 0;
}
