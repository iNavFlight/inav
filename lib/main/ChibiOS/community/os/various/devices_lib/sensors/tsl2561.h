/*
    TSL2561 for ChibiOS/RT - Copyright (C) 2016 Stephane D'Alu

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
 * @file    tsl2561.h
 * @brief   TSL2561 Light sensor interface module header.
 *
 * @{
 */

#ifndef _SENSOR_TSL2561_H_
#define _SENSOR_TSL2561_H_

#include <math.h>
#include "i2c_helpers.h"
#include "sensor.h"


/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define TSL2561_CONTINUOUS_ACQUISITION_SUPPORTED   TRUE

#define TSL2561_OVERLOADED        (-1)


/* I2C address */
#define TSL2561_I2CADDR_LOW          (0x29)
#define TSL2561_I2CADDR_FLOAT        (0x39)
#define TSL2561_I2CADDR_HIGH         (0x49)

/**
 * @brief Time necessary for the sensor to boot
 */
#define TSL2561_BOOTUP_TIME		0

/**
 * @brief Time necessary for the sensor to start
 */
#define TSL2561_STARTUP_TIME		0



/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

#ifndef TSL2561_WITH_CS
#define TSL2561_WITH_CS		  0
#endif

#ifndef TSL2561_WITH_T_FN_CL
#define TSL2561_WITH_T_FN_CL	  1
#endif


/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/


#define TSL2561_I2CADDR_DEFAULT        TSL2561_I2CADDR_FLOAT


/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   TSL2561 configuration structure.
 */
typedef struct {
    I2CHelper i2c; /* keep it first */
} TSL2561_config;

/**
 * @brief   Available integration time
 *
 * @details Available integration time are:
 *          13.7ms, 101ms, 402ms
 */
typedef enum {
    TSL2561_INTEGRATIONTIME_SHORT      = 0x00,    /**< @brief  13.7ms */
    TSL2561_INTEGRATIONTIME_MEDIUM     = 0x01,    /**< @brief 101.0ms */
    TSL2561_INTEGRATIONTIME_LONG       = 0x02,    /**< @brief 402.0ms */
} TSL2561_integration_time_t;

/**
 * @brief   Available gain
 *
 * @details Available gain are 1x, 16x
 */
typedef enum {
    TSL2561_GAIN_1X                    = 0x00,    /**< @brief  1x gain */
    TSL2561_GAIN_16X                   = 0x10,    /**< @brief 16x gain */
} TSL2561_gain_t;

/**
 * @brief   TSL2561 configuration structure.
 */
typedef struct {
    TSL2561_config  *config;
    sensor_state_t   state;
    TSL2561_gain_t   gain;
    TSL2561_integration_time_t integration_time;
    struct PACKED {
	uint8_t revno  : 4;
	uint8_t partno : 4; }  id;
} TSL2561_drv;

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
TSL2561_init(TSL2561_drv *drv,
	TSL2561_config *config);

/**
 * @brief Start the sensor
 */
msg_t
TSL2561_start(TSL2561_drv *drv);

/**
 * @brief   Stop the sensor
 *
 * @details If the sensor support it, it will be put in low energy mode.
 */
msg_t
TSL2561_stop(TSL2561_drv *drv);

/**
 * @brief   Check that the sensor is really present
 */
msg_t
TSL2561_check(TSL2561_drv *drv);

/**
 * @brief Time in milli-seconds necessary for acquiring a naw measure
 *
 * @returns
 *   unsigned int   time in millis-seconds
 */
unsigned int
TSL2561_getAcquisitionTime(TSL2561_drv *drv);

/**
 * @brief Trigger a mesure acquisition
 */
static inline msg_t
TSL2561_startMeasure(TSL2561_drv *drv) {
    (void)drv;
    return MSG_OK;
};

/**
 * @brief Read the newly acquiered measure
 *
 * @note  According the the sensor design the measure read
 *        can be any value acquired after the acquisition time
 *        and the call to readMeasure.
 */
msg_t
TSL2561_readMeasure(TSL2561_drv *drv,
	unsigned int illuminance);

msg_t
TSL2561_setGain(TSL2561_drv *drv,
	TSL2561_gain_t gain);

msg_t
TSL2561_setIntegrationTime(TSL2561_drv *drv,
	TSL2561_integration_time_t time);

/**
 * @brief   Read temperature and humidity
 *
 * @details According to the sensor specification/configuration
 *          (see #TSL2561_CONTINUOUS_ACQUISITION_SUPPORTED), 
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
TSL2561_readIlluminance(TSL2561_drv *drv,
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
TSL2561_getIlluminance(TSL2561_drv *drv) {
    unsigned int illuminance = -1;
    TSL2561_readIlluminance(drv, &illuminance);
    return illuminance;
}


#endif

