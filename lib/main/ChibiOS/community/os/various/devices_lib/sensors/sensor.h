/*
    Copyright (C) 2016 Stephane D'Alu
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
 * 
 * Example of function calls.
 *
 * @code
 * static SENSOR_config sensor_config = {
 * };
 * static SENSOR_drv    sensor_drv;
 * @endcode
 *
 * 
 * @code
 * osalThreadSleepMilliseconds(SENSOR_BOOTUP_TIME);
 * SENSOR_init(&sensor_drv);
 * @endcode
 *
 * @code
 * SENSOR_start(&sensor_drv, &sensor_config);
 * osalThreadSleepMilliseconds(SENSOR_STARTUP_TIME);
 * @endcode
 *
 * If using SENSOR_startMeasure()/SENSOR_readMeasure()
 * @code
 * while(true) {
 *   SENSOR_startMeasure(&sensor_drv);
 *   osalThreadSleepMilliseconds(SENSOR_getAcquisitionTime());
 *   SENSOR_readMeasure(&sensor_drv, ...);
 * }
 * @endcode
 *
 * If using SENSOR_readValue() or SENSOR_getValue()
 * @code
 * #if SENSOR_CONTINUOUS_ACQUISITION_SUPPORTED == TRUE
 * osalThreadSleepMilliseconds(SENSOR_getAcquisitionTime())
 * #endif
 * 
 * while(true) {
 *   SENSOR_readValue(&sensor_drv, ...);
 * }
 * @encode
 */
#ifndef _SENSOR_H_
#define _SENSOR_H_

#define SENSOR_OK        MSG_OK         /**< @brief Operation successful. */
#define SENSOR_TIMEOUT   MSG_TIMEOUT    /**< @brief Communication timeout */
#define SENSOR_RESET     MSG_REST       /**< @brief Communication error.  */
#define SENSOR_NOTFOUND  (msg_t)-20     /**< @brief Sensor not found.     */


/**
 * @brief   Driver state machine possible states.
 */
typedef enum __attribute__ ((__packed__)) {
    SENSOR_UNINIT    = 0,            /**< Not initialized.                */
    SENSOR_INIT      = 1,            /**< Initialized.                    */
    SENSOR_STARTED   = 2,            /**< Started.                        */
    SENSOR_MEASURING = 4,            /**< Measuring.                      */
    SENSOR_READY     = 3,            /**< Ready.                          */
    SENSOR_STOPPED   = 5,            /**< Stopped.                        */
    SENSOR_ERROR     = 6,            /**< Error.                          */
} sensor_state_t;

#endif


