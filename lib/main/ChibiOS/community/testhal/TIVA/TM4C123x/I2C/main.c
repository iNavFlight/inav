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
#include "chprintf.h"

/* buffers depth */
#define TEMP_RX_DEPTH 6
#define TEMP_TX_DEPTH 6

/* tmp275 specific addresses */
#define TMP275_TEMP             0x00
#define TMP275_CONF             0x01
#define TMP275_TLOW             0x02
#define TMP275_THIGH            0x03

/* tmp275 config register */
#define TMP275_CONF_SD          0x01
#define TMP275_CONF_TM          0x02
#define TMP275_CONF_POL         0x04
#define TMP275_CONF_F0          0x08
#define TMP275_CONF_F1          0x10
#define TMP275_CONF_R0          0x20
#define TMP275_CONF_R1          0x40
#define TMP275_CONF_OS          0x80

#define TMP275_ADDR             0b01001001

static uint8_t rxbuf[TEMP_RX_DEPTH];
static uint8_t txbuf[TEMP_TX_DEPTH];
static i2cflags_t errors = 0;
static uint16_t temperature;

/* I2C configuration*/
static const I2CConfig i2cfg =
{
    400000
};

/*
 * Application entry point.
 */
int main(void)
{
  msg_t status = MSG_OK;
  systime_t tmo = OSAL_MS2I(100);

  /*
   * System initializations.
   * - HAL initialization, this also initializes the configured device drivers
   *   and performs the board-specific initializations.
   * - Kernel initialization, the main() function becomes a thread and the
   *   RTOS is active.
   */
  halInit();
  chSysInit();

  /* Configure RX and TX pins for UART0.*/
  palSetLineMode(LINE_UART0_RX, PAL_MODE_INPUT | PAL_MODE_ALTERNATE(1));
  palSetLineMode(LINE_UART0_TX, PAL_MODE_INPUT | PAL_MODE_ALTERNATE(1));

  /*
   * Start the serial driver with the default configuration.
   */
  sdStart(&SD1, NULL);

  /* Configure SCK and SCL pins for I2C0.*/
  palSetLineMode(LINE_I2C0_SCL, PAL_MODE_OUTPUT_PUSHPULL | PAL_MODE_ALTERNATE(3));
  palSetLineMode(LINE_I2C0_SDA, PAL_MODE_OUTPUT_OPENDRAIN | PAL_MODE_ALTERNATE(3));

  /*
   * Start the i2c driver with the custom configuration.
   */
  i2cStart(&I2CD1, &i2cfg);

  chprintf((BaseSequentialStream *)&SD1, "\r\n**********************\r\n");
  chprintf((BaseSequentialStream *)&SD1, "* TM4C123x I2C Demo. *\r\n");
  chprintf((BaseSequentialStream *)&SD1, "**********************\r\n\r\n");

  txbuf[0] = TMP275_CONF; // register address
  txbuf[1] = TMP275_CONF_R0 | TMP275_CONF_R1; // set conversion resolution to 12 bits
  i2cAcquireBus(&I2CD1);
  status = i2cMasterTransmitTimeout(&I2CD1, TMP275_ADDR, txbuf, 2, rxbuf, 0, tmo);
  i2cReleaseBus(&I2CD1);

  if (status != MSG_OK){
    errors = i2cGetErrors(&I2CD1);
    chprintf((BaseSequentialStream *)&SD1, "ERROR: errors detected.\r\n");
  }

  /*
   * Normal main() thread activity
   */
  while (TRUE) {
    txbuf[0] = TMP275_TEMP; // register address
    i2cAcquireBus(&I2CD1);
    status = i2cMasterTransmitTimeout(&I2CD1, TMP275_ADDR, txbuf, 1, rxbuf, 2, tmo);
    i2cReleaseBus(&I2CD1);

    if (status != MSG_OK){
      errors = i2cGetErrors(&I2CD1);
      chprintf((BaseSequentialStream *)&SD1, "Status: %i\r\n", status);
    }
    else{
      temperature = (((rxbuf[0] << 8) | rxbuf[1]) >> 4);
      chprintf((BaseSequentialStream *)&SD1, "Temperature: %u,%4u\r\n", temperature >> 4, 625*(temperature & 0x0f));
    }

    chThdSleepMilliseconds(1000);
  }
  
  return 0;
}
