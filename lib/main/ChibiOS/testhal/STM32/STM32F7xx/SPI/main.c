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

#include <string.h>

#include "ch.h"
#include "hal.h"

/*===========================================================================*/
/* SPI driver related.                                                       */
/*===========================================================================*/

#define SPI_LOOPBACK

/*
 * Maximum speed SPI configuration (27MHz, CPHA=0, CPOL=0, MSb first).
 */
static const SPIConfig hs_spicfg = {
  NULL,
  GPIOB,
  GPIOB_ARD_D15,
  SPI_CR1_BR_0,
  SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0
};

/*
 * Low speed SPI configuration (421.875kHz, CPHA=0, CPOL=0, MSb first).
 */
static const SPIConfig ls_spicfg = {
  NULL,
  GPIOB,
  GPIOB_ARD_D14,
  SPI_CR1_BR_2 | SPI_CR1_BR_1,
  SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0
};

/*
 * SPI TX and RX buffers.
 * Note, the buffer are aligned to a 32 bytes boundary because limitations
 * imposed by the data cache. Note, this is GNU specific, it must be
 * handled differently for other compilers.
 */
#define SPI_BUFFERS_SIZE    128U

static uint8_t txbuf[SPI_BUFFERS_SIZE];
static uint8_t rxbuf[SPI_BUFFERS_SIZE];

/*===========================================================================*/
/* Application code.                                                         */
/*===========================================================================*/

/*
 * SPI bus contender 1.
 */
static THD_WORKING_AREA(spi_thread_1_wa, 256);
static THD_FUNCTION(spi_thread_1, p) {

  (void)p;
  chRegSetThreadName("SPI thread 1");
  while (true) {
    unsigned i;

    /* Bush acquisition and SPI reprogramming.*/
    spiAcquireBus(&SPID2);
    spiStart(&SPID2, &hs_spicfg);

    /* Preparing data buffer and flushing cache.*/
    for (i = 0; i < SPI_BUFFERS_SIZE; i++)
      txbuf[i] = (uint8_t)i;

    /* Slave selection and data exchange.*/
    spiSelect(&SPID2);
    spiExchange(&SPID2, SPI_BUFFERS_SIZE, txbuf, rxbuf);
    spiUnselect(&SPID2);

#if defined(SPI_LOOPBACK)
    if (memcmp(txbuf, rxbuf, SPI_BUFFERS_SIZE) != 0)
      chSysHalt("loopback failure");
#endif

    /* Releasing the bus.*/
    spiReleaseBus(&SPID2);
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
    unsigned i;

    /* Bush acquisition and SPI reprogramming.*/
    spiAcquireBus(&SPID2);
    spiStart(&SPID2, &ls_spicfg);

    /* Preparing data buffer and flushing cache.*/
    for (i = 0; i < SPI_BUFFERS_SIZE; i++)
      txbuf[i] = (uint8_t)(128U + i);

    /* Slave selection and data exchange.*/
    spiSelect(&SPID2);
    spiExchange(&SPID2, SPI_BUFFERS_SIZE, txbuf, rxbuf);
    spiUnselect(&SPID2);

#if defined(SPI_LOOPBACK)
    if (memcmp(txbuf, rxbuf, SPI_BUFFERS_SIZE) != 0)
      chSysHalt("loopback failure");
#endif

    /* Releasing the bus.*/
    spiReleaseBus(&SPID2);
  }
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

  /*
   * SPI2 I/O pins setup.
   */
  palSetLineMode(LINE_ARD_D13,
                 PAL_MODE_ALTERNATE(5) |
                 PAL_STM32_OSPEED_HIGHEST);         /* SPI SCK.             */
  palSetLineMode(LINE_ARD_D12,
                 PAL_MODE_ALTERNATE(5) |
                 PAL_STM32_OSPEED_HIGHEST);         /* MISO.                */
  palSetLineMode(LINE_ARD_D11,
                 PAL_MODE_ALTERNATE(5) |
                 PAL_STM32_OSPEED_HIGHEST);         /* MOSI.                */
  palSetLine(LINE_ARD_D15);
  palSetLineMode(LINE_ARD_D15,
                 PAL_MODE_OUTPUT_PUSHPULL);         /* CS0.                 */
  palSetLine(LINE_ARD_D14);
  palSetLineMode(LINE_ARD_D14,
                 PAL_MODE_OUTPUT_PUSHPULL);         /* CS1.                 */

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
}
