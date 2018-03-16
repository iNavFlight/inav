/*
    MCP9808 for ChibiOS/RT - Copyright (C) 2016 Stephane D'Alu

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

#ifndef _SENSOR_MCP9808_H_
#define _SENSOR_MCP9808_H_

#include <math.h>
#include "i2c_helpers.h"
#include "sensor.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define MCP9808_CONTINUOUS_ACQUISITION_SUPPORTED   TRUE


#define MCP9808_I2CADDR_FIXED           0x18

/**
 * @brief Time necessary for the sensor to boot
 */
#define MCP9808_BOOTUP_TIME		0

/**
 * @brief Time necessary for the sensor to start
 */
#define MCP9808_STARTUP_TIME		0

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#define MCP9808_I2CADDR_DEFAULT       MCP9808_I2CADDR_FIXED

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief  Different possible resolution
 */
typedef enum {
    RES_2  = 0x00,  /**< @brief Resolution of 1/2  = 0.5    */
    RES_4  = 0x01,  /**< @brief Resolution of 1/4  = 0.25   */
    RES_8  = 0x10,  /**< @brief Resolution of 1/8  = 0.125  */
    RES_16 = 0x11,  /**< @brief Resolution of 1/16 = 0.0625 */
} MCP9808_resolution_t;

/**
 * @brief   MCP9808 configuration structure.
 */
typedef struct {
    I2CHelper i2c; /* keep it first */
} MCP9808_config;

/**
 * @brief   MCP9808 configuration structure.
 */
typedef struct {
    MCP9808_config      *config;
    sensor_state_t       state;
    MCP9808_resolution_t resolution;
    uint16_t             cfg;
} MCP9808_drv;

/**
 * @brief   MCP9808 measure reading
 */
typedef struct {
    float temperature;
} MCP9808_measure;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/


/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/**
 * @brief   Initialize the sensor driver
 */
void
MCP9808_init(MCP9808_drv *drv,
	MCP9808_config *config);

/**
 * @brief   Check that the sensor is really present
 */
msg_t
MCP9808_check(MCP9808_drv *drv);

/**
 * @brief   Start the sensor
 */
msg_t
MCP9808_start(MCP9808_drv *drv);

/**
 * @brief   Stop the sensor
 *
 * @details If the sensor support it, it will be put in low energy mode.
 */
msg_t
MCP9808_stop(MCP9808_drv *drv);

/**
 * @brief   Control the MCP9809 resolution.
 */
msg_t
MCP9808_setResolution(MCP9808_drv *drv,
	MCP9808_resolution_t res);

/**
 * @brief Time in milli-seconds necessary for acquiring a naw measure
 *
 * @returns
 *   unsigned int   time in millis-seconds
 */
unsigned int
MCP9808_getAcquisitionTime(MCP9808_drv *drv);

/**
 * @brief Trigger a mesure acquisition
 */
static inline msg_t
MCP9808_startMeasure(MCP9808_drv *drv) {
    (void)drv;
    return MSG_OK;
}

/**
 * @brief Read the newly acquiered measure
 *
 * @note  According the the sensor design the measure read
 *        can be any value acquired after the acquisition time
 *        and the call to readMeasure.
 */
msg_t
MCP9808_readMeasure(MCP9808_drv *drv,
	float *temperature);


/**
 * @brief   Read temperature and humidity
 *
 * @details According to the sensor specification/configuration
 *          (see #MCP9808_CONTINUOUS_ACQUISITION_SUPPORTED), 
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
MCP9808_readTemperature(MCP9808_drv *drv,
	float *temperature);

/**
 * @brief   Return the temperature value in °C.
 *
 * @note    Prefere readTemperature(), if you need better error handling.
 *
 * @return  The temperature in °C
 * @retval  float  humidity percent
 * @retval  NAN    on failure
 */
static inline float
MCP9808_getTemperature(MCP9808_drv *drv) {
    float temperature = NAN;
    MCP9808_readTemperature(drv, &temperature);
    return temperature;
}

#endif

