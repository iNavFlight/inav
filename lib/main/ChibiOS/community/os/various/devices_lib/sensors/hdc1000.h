/*
    HDC100x for ChibiOS/RT - Copyright (C) 2016 Stephane D'Alu

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
 * @file    hdc1000.h
 * @brief   HDC1000 Temperature/Humidiry sensor interface module header.
 *
 * When changing sensor settings, you generally need to wait
 * for 2 * getAquisitionTime(), as usually the first acquisition
 * will be corrupted by the change of settings.
 *
 * No locking is done.
 *
 * @{
 */

#ifndef _SENSOR_HDC1000_H_
#define _SENSOR_HDC1000_H_

#include <math.h>
#include <stdbool.h>
#include "i2c_helpers.h"
#include "sensor.h"


/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define HDC1000_CONTINUOUS_ACQUISITION_SUPPORTED   FALSE

/* I2C address */
#define HDC1000_I2CADDR_1           0x40
#define HDC1000_I2CADDR_2           0x41
#define HDC1000_I2CADDR_3           0x42
#define HDC1000_I2CADDR_4           0x43

#define HDC1000_SERIAL_SIZE         5  /**< @brief Size of serial (40bits) */

/**
 * @brief Time necessary for the sensor to boot
 */
#define HDC1000_BOOTUP_TIME		15

/**
 * @brief Time necessary for the sensor to start
 */
#define HDC1000_STARTUP_TIME		0


/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#define HDC1000_I2CADDR_DEFAULT     HDC1000_I2CADDR_1


/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   HDC1000 configuration structure.
 */
typedef struct {
    I2CHelper i2c; /* keep it first */
} HDC1000_config;

/**
 * @brief   HDC1000 configuration structure.
 */
typedef struct {
    HDC1000_config  *config;
    sensor_state_t   state;
    unsigned int     delay;    
    uint16_t         cfg;
} HDC1000_drv;

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
HDC1000_init(HDC1000_drv *drv,
	HDC1000_config *config);

/**
 * @brief Start the sensor
 */
msg_t
HDC1000_start(HDC1000_drv *drv);

/**
 * @brief   Stop the sensor
 *
 * @details If the sensor support it, it will be put in low energy mode.
 */
msg_t
HDC1000_stop(HDC1000_drv *drv);

/**
 * @brief   Check that the sensor is really present
 */
msg_t
HDC1000_check(HDC1000_drv *drv);


msg_t
HDC1000_readSerial(HDC1000_drv *drv, uint8_t *serial);

/**
 * @brief   Control the HD1000 heater.
 */
msg_t
HDC1000_setHeater(HDC1000_drv *drv,
	bool on);



/**
 * @brief Time in milli-seconds necessary for acquiring a naw measure
 *
 * @returns
 *   unsigned int   time in millis-seconds
 */
static inline unsigned int
HDC1000_getAcquisitionTime(HDC1000_drv *drv) {
    return drv->delay;
}

/**
 * @brief Trigger a mesure acquisition
 */
msg_t
HDC1000_startMeasure(HDC1000_drv *drv);

/**
 * @brief Read the newly acquiered measure
 *
 * @note  According the the sensor design the measure read
 *        can be any value acquired after the acquisition time
 *        and the call to readMeasure.
 */
msg_t
HDC1000_readMeasure(HDC1000_drv *drv,
	float *temperature, float *humidity);


/**
 * @brief   Read temperature and humidity
 *
 * @details According to the sensor specification/configuration
 *          (see #HDC1000_CONTINUOUS_ACQUISITION_SUPPORTED), 
 *          if the sensor is doing continuous measurement
 *          it's value will be requested and returned immediately.
 *          Otherwise a measure is started, the necessary amount of
 *          time for acquiring the value is spend sleeping (not spinning),
 *          and finally the measure is read.
 *
 * @note    In continuous measurement mode, if you just started
 *          the sensor, you will need to wait getAcquisitionTime()
 *          in addition to the usual #HDC1000_STARTUP_TIME

 * @note    If using several sensors, it is better to start all the
 *          measure together, wait for the sensor having the longuest
 *          aquisition time, and finally read all the values
 */
msg_t
HDC1000_readTemperatureHumidity(HDC1000_drv *drv,
	float *temperature, float *humidity);

/**
 * @brief   Return the humidity value in percent.
 *
 * @details Use readTemperatureHumidity() for returning the humidity value.
 *
 * @note    Prefere readTemperatureHumidity(), if you need both temperature
 *          and humidity, or if you need better error handling.
 *
 * @returns
 *   float  humidity percent
 *   NAN    on failure
 */
static inline float
HDC1000_getHumidity(HDC1000_drv *drv) {
    float humidity   = NAN;
    HDC1000_readTemperatureHumidity(drv, NULL, &humidity);
    return humidity;
}

/**
 * @brief   Return the temperature value in °C.
 *
 * @details Use readTemperatureHumidity() for returning the humidity value.
 *
 * @note    Prefere readTemperatureHumidity(), if you need both temperature
 *          and humidity, or if you need better error handling.
 *
 * @returns
 *   float  humidity percent
 *   NAN    on failure
 */
static inline float
HDC1000_getTemperature(HDC1000_drv *drv) {
    float temperature = NAN;
    HDC1000_readTemperatureHumidity(drv, &temperature, NULL);
    return temperature;
}


#endif

/**
 * @}
 */
