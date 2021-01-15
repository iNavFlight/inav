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
#define SPI_BUF_SIZE    512

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
static void spi_end_cb(SPIDriver *spip);

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */
static uint8_t testbuf_ram[SPI_BUF_SIZE];
static const uint8_t testbuf_flash[SPI_BUF_SIZE];

/*
 *
 */
static const SPIConfig spicfg = {
    false,
    spi_end_cb,
    GPIOA,
    GPIOA_SPI1_NSS,
    0, //SPI_CR1_BR_1 | SPI_CR1_BR_0
    0
};

static uint32_t ints;
static binary_semaphore_t sem;
static bool stop = false;

/*
 ******************************************************************************
 ******************************************************************************
 * LOCAL FUNCTIONS
 ******************************************************************************
 ******************************************************************************
 */

static void spi_end_cb(SPIDriver *spip){
  ints++;

  if (stop){
    chSysLockFromISR();
    chBSemSignalI(&sem);
    chSysUnlockFromISR();
    return;
  }
  else{
    chSysLockFromISR();
    spiStartExchangeI(spip, SPI_BUF_SIZE, testbuf_flash, testbuf_ram);
    chSysUnlockFromISR();
  }
}

/*
 ******************************************************************************
 * EXPORTED FUNCTIONS
 ******************************************************************************
 */

void dma_storm_spi_start(void){
  ints = 0;
  stop = false;
  chBSemObjectInit(&sem, true);
  spiStart(&SPID1, &spicfg);
  spiStartExchange(&SPID1, SPI_BUF_SIZE, testbuf_flash, testbuf_ram);
}

uint32_t dma_storm_spi_stop(void){
  stop = true;
  chBSemWait(&sem);
  spiStop(&SPID1);
  return ints;
}

