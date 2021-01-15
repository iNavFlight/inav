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

#include "chprintf.h"
#include "lsm303agr.h"

#define cls(chp)  chprintf(chp, "\033[2J\033[1;1H")

/*===========================================================================*/
/* LSM303AGR related.                                                        */
/*===========================================================================*/

/* LSM303AGR Driver: This object represent an LSM303AGR instance */
static LSM303AGRDriver LSM303AGRD1;

static int32_t accraw[LSM303AGR_ACC_NUMBER_OF_AXES];
static int32_t compraw[LSM303AGR_COMP_NUMBER_OF_AXES];

static float acccooked[LSM303AGR_ACC_NUMBER_OF_AXES];
static float compcooked[LSM303AGR_COMP_NUMBER_OF_AXES];

static char axisID[LSM303AGR_ACC_NUMBER_OF_AXES] = {'X', 'Y', 'Z'};
static uint32_t i;

static const I2CConfig i2ccfg = {
  OPMODE_I2C,
  400000,
  FAST_DUTY_CYCLE_2,
};

static const LSM303AGRConfig lsm303agrcfg = {
  &I2CD1,
  &i2ccfg,
  NULL,
  NULL,
  LSM303AGR_ACC_FS_4G,
  LSM303AGR_ACC_ODR_100Hz,
#if LSM303AGR_USE_ADVANCED
  LSM303AGR_ACC_MODE_LPOW,
  LSM303AGR_ACC_BDU_BLOCK,
  LSM303AGR_ACC_END_LITTLE,
#endif
  NULL,
  NULL,
  LSM303AGR_COMP_ODR_50HZ,
#if LSM303AGR_USE_ADVANCED
  LSM303AGR_COMP_MODE_NORM,
  LSM303AGR_COMP_LPOW_EN
#endif
};

/*===========================================================================*/
/* Generic code.                                                             */
/*===========================================================================*/

static BaseSequentialStream* chp = (BaseSequentialStream*)&SD2;
/*
 * LED blinker thread, times are in milliseconds.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palToggleLine(LINE_LED_GREEN);
    chThdSleepMilliseconds(500);
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

  /* Configuring I2C SCK and I2C SDA related GPIOs .*/
  palSetLineMode(LINE_ARD_D15, PAL_MODE_ALTERNATE(4) |
                 PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_OPENDRAIN);
  palSetLineMode(LINE_ARD_D14, PAL_MODE_ALTERNATE(4) |
                 PAL_STM32_OSPEED_HIGHEST | PAL_STM32_OTYPE_OPENDRAIN);

  /* Activates the serial driver 2 using the driver default configuration.*/
  sdStart(&SD2, NULL);

  /* Creates the blinker thread.*/
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /* LSM303AGR Object Initialization.*/
  lsm303agrObjectInit(&LSM303AGRD1);

  /* Activates the LSM303AGR driver.*/
  lsm303agrStart(&LSM303AGRD1, &lsm303agrcfg);

  /* Normal main() thread activity, printing MEMS data on the SD2. */
  while (true) {
    lsm303agrAccelerometerReadRaw(&LSM303AGRD1, accraw);
    chprintf(chp, "LSM303AGR Accelerometer raw data...\r\n");
    for(i = 0; i < LSM303AGR_ACC_NUMBER_OF_AXES; i++) {
      chprintf(chp, "%c-axis: %d\r\n", axisID[i], accraw[i]);
    }

    lsm303agrCompassReadRaw(&LSM303AGRD1, compraw);
    chprintf(chp, "LSM303AGR Compass raw data...\r\n");
    for(i = 0; i < LSM303AGR_COMP_NUMBER_OF_AXES; i++) {
      chprintf(chp, "%c-axis: %d\r\n", axisID[i], compraw[i]);
    }

    lsm303agrAccelerometerReadCooked(&LSM303AGRD1, acccooked);
    chprintf(chp, "LSM303AGR Accelerometer cooked data...\r\n");
    for(i = 0; i < LSM303AGR_ACC_NUMBER_OF_AXES; i++) {
      chprintf(chp, "%c-axis: %.3f\r\n", axisID[i], acccooked[i]);
    }

    lsm303agrCompassReadCooked(&LSM303AGRD1, compcooked);
    chprintf(chp, "LSM303AGR Compass cooked data...\r\n");
    for(i = 0; i < LSM303AGR_COMP_NUMBER_OF_AXES; i++) {
      chprintf(chp, "%c-axis: %.3f\r\n", axisID[i], compcooked[i]);
    }
    chThdSleepMilliseconds(100);
    cls(chp);
  }
  lsm303agrStop(&LSM303AGRD1);
}
