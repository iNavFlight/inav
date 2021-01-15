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

/*
 * SPI TX and RX buffers.
 */
static uint8_t txbuf[512];
static uint8_t rxbuf[512];

/*
 * High speed SPI configuration (5MHZ, CPHA=0, CPOL=0).
 */
static const SPIConfig hs_spicfg =
{
  NULL,
  GPIOA,
  3,
  SSI_CR0_DSS_8 | /*SSI_CR0_SPH | SSI_CR0_SPO |*/ SSI_CR0_SCR(0),
  16
};

/*
 * Low speed SPI configuration (1MHz, CPHA=0, CPOL=0).
 */
static const SPIConfig ls_spicfg =
{
  NULL,
  GPIOA,
  3,
  SSI_CR0_DSS_8 | /*SSI_CR0_SPH | SSI_CR0_SPO |*/ SSI_CR0_SCR(0),
  80
};

/*
 * SPI bus contender 1.
 */
static THD_WORKING_AREA(spi_thread_1_wa, 256);
static THD_FUNCTION(spi_thread_1, p)
{
  (void)p;
  chRegSetThreadName("SPI thread 1");
  while (TRUE) {
    spiAcquireBus(&SPID1);              /* Acquire ownership of the bus.    */
    palSetPad(GPIOF, GPIOF_LED_GREEN);  /* LED ON.                          */
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
static THD_FUNCTION(spi_thread_2, p)
{
  (void)p;
  chRegSetThreadName("SPI thread 2");
  while (TRUE) {
    spiAcquireBus(&SPID1);              /* Acquire ownership of the bus.    */
    palClearPad(GPIOF, GPIOF_LED_GREEN);/* LED OFF.                         */
    spiStart(&SPID1, &ls_spicfg);       /* Setup transfer parameters.       */
    spiSelect(&SPID1);                  /* Slave Select assertion.          */
    spiExchange(&SPID1, 512,
                txbuf, rxbuf);          /* Atomic transfer operations.      */
    spiUnselect(&SPID1);                /* Slave Select de-assertion.       */
    spiReleaseBus(&SPID1);              /* Ownership release.               */
  }
}

/*
 * Application entry point.
 */
int main(void)
{
  size_t i;

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  palSetLineMode(LINE_SSI0_CLK, PAL_MODE_OUTPUT_PUSHPULL | PAL_MODE_ALTERNATE(2));
  palSetLineMode(LINE_SSI0_RX, PAL_MODE_OUTPUT_PUSHPULL | PAL_MODE_ALTERNATE(2));
  palSetLineMode(LINE_SSI0_TX, PAL_MODE_OUTPUT_PUSHPULL | PAL_MODE_ALTERNATE(2));
  palSetLineMode(LINE_LED_GREEN, PAL_MODE_OUTPUT_PUSHPULL);
  palSetPadMode(GPIOA, GPIOA_PIN3, PAL_MODE_OUTPUT_PUSHPULL);

  /*
   * Prepare transmit pattern.
   */
  for (i = 0; i < sizeof(txbuf); i++) {
    txbuf[i] = (uint8_t)i;
  }

  /*
   * Starting the transmitter and receiver threads.
   */
  chThdCreateStatic(spi_thread_1_wa,
                    sizeof(spi_thread_1_wa),
                    NORMALPRIO + 1,
                    spi_thread_1,
                    NULL);
  chThdCreateStatic(spi_thread_2_wa,
                    sizeof(spi_thread_2_wa),
                    NORMALPRIO + 1,
                    spi_thread_2,
                    NULL);

  /*
   * Normal main() thread activity
   */
  while (TRUE) {
    chThdSleepMilliseconds(500);
  }
  
  return 0;
}
