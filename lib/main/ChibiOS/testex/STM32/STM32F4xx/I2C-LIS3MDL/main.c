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
#include "lis3mdl.h"

#define cls(chp)  chprintf(chp, "\033[2J\033[1;1H")

/*===========================================================================*/
/* LIS3MDL related.                                                           */
/*===========================================================================*/

/* LIS3MDL Driver: This object represent an LIS3MDL instance.*/
static LIS3MDLDriver LIS3MDLD1;

static int32_t compraw[LIS3MDL_COMP_NUMBER_OF_AXES];

static float compcooked[LIS3MDL_COMP_NUMBER_OF_AXES];

static char axisID[LIS3MDL_COMP_NUMBER_OF_AXES] = {'X', 'Y', 'Z'};
static uint32_t i;

static const I2CConfig i2ccfg = {
  OPMODE_I2C,
  400000,
  FAST_DUTY_CYCLE_2,
};

static LIS3MDLConfig lis3mdlcfg = {
  &I2CD1,
  &i2ccfg,
  LIS3MDL_SAD_VCC,
  NULL,
  NULL,
  LIS3MDL_COMP_FS_4GA,
  LIS3MDL_COMP_ODR_40HZ,
#if LIS3MDL_USE_ADVANCED
  LIS3MDL_COMP_LP_ENABLED,
  LIS3MDL_COMP_MD_CONTINUOUS,
  LIS3MDL_COMP_OMXY_LP,
  LIS3MDL_COMP_OMZ_LP,
  LIS3MDL_BDU_CONTINUOUS,
  LIS3MDL_END_LITTLE
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

  /* Activates the serial driver 1 using the driver default configuration.*/
  sdStart(&SD2, NULL);

  /* Creates the blinker thread.*/
  chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

  /* LIS3MDL Object Initialization.*/
  lis3mdlObjectInit(&LIS3MDLD1);

  /* Activates the LIS3MDL driver.*/
  lis3mdlStart(&LIS3MDLD1, &lis3mdlcfg);

  /* Normal main() thread activity, printing MEMS data on the SD2. */
  while (true) {
    lis3mdlCompassReadRaw(&LIS3MDLD1, compraw);
    chprintf(chp, "LIS3MDL Compass raw data...\r\n");
    for(i = 0; i < LIS3MDL_COMP_NUMBER_OF_AXES; i++) {
      chprintf(chp, "%c-axis: %d\r\n", axisID[i], compraw[i]);
    }

    lis3mdlCompassReadCooked(&LIS3MDLD1, compcooked);
    chprintf(chp, "LIS3MDL Compass cooked data...\r\n");
    for(i = 0; i < LIS3MDL_COMP_NUMBER_OF_AXES; i++) {
      chprintf(chp, "%c-axis: %.3f\r\n", axisID[i], compcooked[i]);
    }
    chThdSleepMilliseconds(100);
    cls(chp);
  }
  lis3mdlStop(&LIS3MDLD1);
}
