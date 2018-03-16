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

#include "ch.h"
#include "hal.h"

#define EEPROM_SIZE 8192 // 64Kb, 8KB
#define EEPROM_PAGE_SIZE 32
#define EEPROM_WRITE_TIME_MS 10 // byte/page write time
#define EEPROM_SPID SPID1
#define EEPROM_SPIDCONFIG spi1cfg

static const SPIConfig EEPROM_SPIDCONFIG = {
  NULL,
  GPIOA,
  12,
  0, // Up to 20Mhz
  SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0
};

static SPIEepromFileConfig eeCfg = {
  0,
  EEPROM_SIZE,
  EEPROM_SIZE,
  EEPROM_PAGE_SIZE,
  MS2ST(EEPROM_WRITE_TIME_MS),
  &EEPROM_SPID,
  &EEPROM_SPIDCONFIG,
};

static SPIEepromFileStream eeFile;
static EepromFileStream *eeFS;

static uint8_t buffer[64];

THD_WORKING_AREA(waThreadEE, 256);
static THD_FUNCTION(ThreadEE, arg)
{
  (void)arg;
  uint8_t len = 64;

  while (TRUE)
  {

    eeFS = SPIEepromFileOpen(&eeFile, &eeCfg, EepromFindDevice(EEPROM_DEV_25XX));
    fileStreamSeek(eeFS, 0);

    fileStreamWrite(eeFS, buffer, len);
    fileStreamRead(eeFS, buffer, len);

    fileStreamClose(eeFS);

    chThdSleepMilliseconds(500);
  }

  return;
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

  spiStart(&EEPROM_SPID, &EEPROM_SPIDCONFIG);

  chThdCreateStatic(waThreadEE, sizeof(waThreadEE), NORMALPRIO, ThreadEE, NULL);

  /*
   * Normal main() thread activity, it resets the watchdog.
   */
  while (true) {
    palToggleLine(LINE_LED4_BLUE);
    chThdSleepMilliseconds(500);
  }
  return 0;
}
