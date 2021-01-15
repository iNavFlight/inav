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

/*
    This demo:
    1) Writes bytes to the EEPROM
    2) Reads the same bytes back
    3) Inverts the byte values
    4) Writes them
    5) Reads them back
 */

#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#define I2C_ADDR 0x50
#define I2C_FAKE_ADDR 0x4C
#define EEPROM_START_ADDR 0x00

/*
 * EEPROM thread.
 */
static THD_WORKING_AREA(PollEepromThreadWA, 1024);
static THD_FUNCTION(PollEepromThread, arg) {

  unsigned i;
  uint8_t tx_data[6];
  uint8_t rx_data[4];
  msg_t status;

  (void)arg;

  chRegSetThreadName("PollEeprom");

  /* set initial data to write */
  tx_data[0] = EEPROM_START_ADDR;
  tx_data[1] = EEPROM_START_ADDR;
  tx_data[2] = 0xA0;
  tx_data[3] = 0xA1;
  tx_data[4] = 0xA2;
  tx_data[5] = 0xA3;

  while (true) {

    /* write out initial data */
    i2cAcquireBus(&I2CD1);
    status = i2cMasterTransmitTimeout(&I2CD1, I2C_ADDR, tx_data, sizeof(tx_data), NULL, 0, TIME_INFINITE);
    i2cReleaseBus(&I2CD1);
    osalDbgCheck(MSG_OK == status);

    /* read back inital data */
    osalThreadSleepMilliseconds(2);
    i2cAcquireBus(&I2CD1);
    status = i2cMasterTransmitTimeout(&I2CD1, I2C_ADDR, tx_data, 2, rx_data, sizeof(rx_data), TIME_INFINITE);
    i2cReleaseBus(&I2CD1);
    osalDbgCheck(MSG_OK == status);

    /* invert the data */
    for (i = 2; i < sizeof(tx_data); i++)
      tx_data[i] ^= 0xff;

    /* write out inverted data */
    osalThreadSleepMilliseconds(2);
    i2cAcquireBus(&I2CD1);
    status = i2cMasterTransmitTimeout(&I2CD1, I2C_ADDR, tx_data, sizeof(tx_data), NULL, 0, TIME_INFINITE);
    i2cReleaseBus(&I2CD1);
    osalDbgCheck(MSG_OK == status);

    /* read back inverted data */
    osalThreadSleepMilliseconds(2);
    i2cAcquireBus(&I2CD1);
    status = i2cMasterTransmitTimeout(&I2CD1, I2C_ADDR, tx_data, 2, rx_data, sizeof(rx_data), TIME_INFINITE);
    i2cReleaseBus(&I2CD1);
    osalDbgCheck(MSG_OK == status);

    osalThreadSleepMilliseconds(TIME_INFINITE);
  }
}

/*
 * Fake polling thread.
 */
static THD_WORKING_AREA(PollFakeThreadWA, 256);
static THD_FUNCTION(PollFakeThread, arg) {

  (void)arg;

  chRegSetThreadName("PollFake");
  while (true) {

    msg_t status;
    uint8_t rx_data[2];
    i2cflags_t errors;

    i2cAcquireBus(&I2CD1);
    status = i2cMasterReceiveTimeout(&I2CD1, I2C_FAKE_ADDR, rx_data, 2, TIME_MS2I(4));
    i2cReleaseBus(&I2CD1);

    if (status == MSG_RESET){
      errors = i2cGetErrors(&I2CD1);
      osalDbgCheck(I2C_ACK_FAILURE == errors);
    }

    palTogglePad(IOPORT1, LED1); /* on */
    osalThreadSleepMilliseconds(1000);
  }
}

/*
 * I2C1 config.
 */
static const I2CConfig i2cfg = {
    100000,
    I2C_SCL,
    I2C_SDA,
};

/*
 * Entry point, note, the main() function is already a thread in the system
 * on entry.
 */
int main(void) {

  halInit();
  chSysInit();

  i2cStart(&I2CD1, &i2cfg);

  /* Create EEPROM thread. */
  chThdCreateStatic(PollEepromThreadWA,
          sizeof(PollEepromThreadWA),
          NORMALPRIO,
          PollEepromThread,
          NULL);

  /* Create not responding thread. */
  chThdCreateStatic(PollFakeThreadWA,
          sizeof(PollFakeThreadWA),
          NORMALPRIO,
          PollFakeThread,
          NULL);

  /* main loop handles LED */
  while (true) {
    palTogglePad(IOPORT1, LED2); /* on */
    osalThreadSleepMilliseconds(500);
  }

  return 0;
}
