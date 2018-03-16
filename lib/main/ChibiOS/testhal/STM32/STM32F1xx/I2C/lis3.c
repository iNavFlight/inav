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

/**
 * This is device realize "read through write" paradigm. This is not
 * standard, but most of I2C devices use this paradigm.
 * You must write to device reading address, send restart to bus,
 * and then begin reading process.
 */

#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "lis3.h"

#define addr 0b0011101

/* autoincrement bit position. This bit needs to perform reading of
 * multiple bytes at one request */
#define AUTO_INCREMENT_BIT (1<<7)

/* slave specific addresses */
#define ACCEL_STATUS_REG  0x27
#define ACCEL_CTRL_REG1   0x20
#define ACCEL_OUT_DATA    0x28

/* buffers */
static uint8_t accel_rx_data[8];
static uint8_t accel_tx_data[8];

/*
 *
 */
void lis3Start(void){
  msg_t status = MSG_OK;
  systime_t tmo = MS2ST(4);

  /* configure accelerometer */
  accel_tx_data[0] = ACCEL_CTRL_REG1 | AUTO_INCREMENT_BIT;
  accel_tx_data[1] = 0b11100111;
  accel_tx_data[2] = 0b01000001;
  accel_tx_data[3] = 0b00000000;

  /* sending */
  i2cAcquireBus(&I2CD1);
  status = i2cMasterTransmitTimeout(&I2CD1, addr,
                          accel_tx_data, 4, accel_rx_data, 0, tmo);
  i2cReleaseBus(&I2CD1);

  osalDbgCheck(MSG_OK == status);
}

/*
 *
 */
#include <math.h>
void lis3GetAcc(float *result) {
  msg_t status = MSG_OK;
  systime_t tmo = MS2ST(4);
  size_t i = 0;
  int16_t tmp;

  accel_tx_data[0] = ACCEL_OUT_DATA | AUTO_INCREMENT_BIT;
  i2cAcquireBus(&I2CD1);
  status = i2cMasterTransmitTimeout(&I2CD1, addr,
                          accel_tx_data, 1, accel_rx_data, 6, tmo);
  i2cReleaseBus(&I2CD1);

  osalDbgCheck(MSG_OK == status);

  for (i=0; i<3; i++){
    tmp = accel_rx_data[i*2] + (accel_rx_data[i*2+1] << 8);
    result[i] = tmp;
    result[i] /= 16384; /* convert raw value to G */
  }
}

