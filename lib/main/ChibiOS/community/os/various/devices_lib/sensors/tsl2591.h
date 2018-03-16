/*
    TSL2591 for ChibiOS/RT - Copyright (C) 2016 Stephane D'Alu

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
 * @file    tsl2591.h
 * @brief   TSL2591 Light sensor interface module header.
 *
 * @{
 */

#ifndef _SENSOR_TSL2591_H_
#define _SENSOR_TSL2591_H_

#include <math.h>
#include "i2c_helpers.h"
#include "sensor.h"


/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief Device sensor continuous acquisition support.
 */
#define TSL2591_CONTINUOUS_ACQUISITION_SUPPORTED   TRUE

/**
 * @brief I2C address. 
 */
#define TSL2591_I2CADDR_FIXED           0x29

/**
 * @brief Time necessary for the sensor to boot
 */
#define TSL2591_BOOTUP_TIME		0

/**
 * @brief Time necessary for the sensor to start
 */
#define TSL2591_STARTUP_TIME		0



/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/**
 * @brief Default I2C address (when pin unconfigured)
 */
#define TSL2591_I2CADDR_DEFAULT     TSL2591_I2CADDR_FIXED


/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   TSL2591 configuration structure.
 */
typedef struct {
    I2CHelper i2c; /* keep it first */
} TSL2591_config;

/**
 * @brief   Available integration time
 *
 * @details Available integration time are:
 *          100ms, 200ms, 300ms, 400ms, 500ms and 600ms
 */
typedef enum {
    TSL2591_INTEGRATIONTIME_100MS     = 0x00, /**< @brief 100ms */
    TSL2591_INTEGRATIONTIME_200MS     = 0x01, /**< @brief 200ms */
    TSL2591_INTEGRATIONTIME_300MS     = 0x02, /**< @brief 300ms */
    TSL2591_INTEGRATIONTIME_400MS     = 0x03, /**< @brief 400ms */
    TSL2591_INTEGRATIONTIME_500MS     = 0x04, /**< @brief 500ms */
    TSL2591_INTEGRATIONTIME_600MS     = 0x05, /**< @brief 600ms */
} TSL2591_integration_time_t;

/**
 * @brief   Available gain
 *
 * @details Available gain are 1x, 25x, 415x, 10000x
 */
typedef enum {
    TSL2591_GAIN_1X                  = 0x00, /**< @brief     1x gain */
    TSL2591_GAIN_25X                 = 0x10, /**< @brief    25x gain */
    TSL2591_GAIN_415X                = 0x20, /**< @brief   415x gain */
    TSL2591_GAIN_10000X              = 0x30, /**< @brief 10000x gain */
} TSL2591_gain_t;

/**
 * @brief   TSL2591 configuration structure.
 */
typedef struct {
    TSL2591_config  *config;
    sensor_state_t   state;
    TSL2591_gain_t gain;
    TSL2591_integration_time_t integration_time;
} TSL2591_drv;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/**
 * @brief Initialize the sensor driver
 */
void
TSL2591_init(TSL2591_drv *drv,
	TSL2591_config *config);

/**
 * @brief Start the sensor
 */
msg_t
TSL2591_start(TSL2591_drv *drv);

/**
 * @brief   Stop the sensor
 *
 * @details If the sensor support it, it will be put in low energy mode.
 */
msg_t
TSL2591_stop(TSL2591_drv *drv);

/**
 * @brief   Check that the sensor is really present
 */
msg_t
TSL2591_check(TSL2591_drv *drv);

/**
 * @brief Time in milli-seconds necessary for acquiring a naw measure
 *
 * @returns
 *   unsigned int   time in millis-seconds
 */
unsigned int
TSL2591_getAcquisitionTime(TSL2591_drv *drv);

/**
 * @brief Trigger a mesure acquisition
 */
static inline msg_t
TSL2591_startMeasure(TSL2591_drv *drv) {
    (void)drv;
    return MSG_OK;
};


msg_t
TSL2591_setGain(TSL2591_drv *drv,
	TSL2591_gain_t gain);

msg_t
TSL2591_setIntegrationTime(TSL2591_drv *drv,
	TSL2591_integration_time_t time);

/**
 * @brief Read the newly acquiered measure
 *
 * @note  According the the sensor design the measure read
 *        can be any value acquired after the acquisition time
 *        and the call to readMeasure.
 */
msg_t
TSL2591_readMeasure(TSL2591_drv *drv,
	unsigned int illuminance);


/**
 * @brief   Read temperature and humidity
 *
 * @details According to the sensor specification/configuration
 *          (see #TSL2591_CONTINUOUS_ACQUISITION_SUPPORTED), 
 *          if the sensor is doing continuous measurement
 *          it's value will be requested and returned immediately.
 *          Otherwise a measure is started, the necessary amount of
 *          time for acquiring the value is spend sleeping (not spinning),
 *          and finally the measure is read.
 *
 * @note    In continuous measurement mode, if you just started
 *          the sensor, you will need to wait getAcquisitionTime()
 *          in addition to the usual getStartupTime()

 * @note    If using several sensors, it is better to start all the
 *          measure together, wait for the sensor having the longuest
 *          aquisition time, and finally read all the values
 */
msg_t
TSL2591_readIlluminance(TSL2591_drv *drv,
	unsigned int *illuminance);

/**
 * @brief   Return the illuminance value in Lux
 *
 * @details Use readIlluminance() for returning the humidity value.
 *
 * @note    Prefere readIlluminance()if you need better error handling.
 *
 * @return Illuminance in Lux
 * @retval  unsigned int illuminace value 
 * @retval  -1           on failure
 */
static inline unsigned int
TSL2591_getIlluminance(TSL2591_drv *drv) {
    unsigned int illuminance = -1;
    TSL2591_readIlluminance(drv, &illuminance);
    return illuminance;
}


#endif

