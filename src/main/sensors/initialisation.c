/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "common/utils.h"

#include "config/config_eeprom.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/compass.h"
#include "sensors/gyro.h"
#include "sensors/initialisation.h"
#include "sensors/irlock.h"
#include "sensors/opflow.h"
#include "sensors/pitotmeter.h"
#include "sensors/rangefinder.h"
#include "sensors/sensors.h"
#include "sensors/temperature.h"
#include "sensors/temperature.h"
#include "sensors/rotor.h"
#include "rx/rx.h"

uint8_t requestedSensors[SENSOR_INDEX_COUNT] = { GYRO_AUTODETECT, ACC_NONE, BARO_NONE, MAG_NONE, RANGEFINDER_NONE, PITOT_NONE, OPFLOW_NONE };
uint8_t detectedSensors[SENSOR_INDEX_COUNT] = { GYRO_NONE, ACC_NONE, BARO_NONE, MAG_NONE, RANGEFINDER_NONE, PITOT_NONE, OPFLOW_NONE };

bool sensorsAutodetect(void)
{
    bool eepromUpdatePending = false;

    if (!gyroInit()) {
        return false;
    }

    accInit(getLooptime());

#ifdef USE_BARO
    baroInit();
#endif

#ifdef USE_PITOT
    pitotInit();
#endif

#ifdef USE_MAG
    compassInit();
#endif

#ifdef USE_TEMPERATURE_SENSOR
    temperatureInit();
#endif

#ifdef USE_RANGEFINDER
    rangefinderInit();
#endif

#ifdef USE_OPFLOW
    opflowInit();
#endif

    if (accelerometerConfig()->acc_hardware == ACC_AUTODETECT) {
        accelerometerConfigMutable()->acc_hardware = detectedSensors[SENSOR_INDEX_ACC];
        eepromUpdatePending = true;
    }

#ifdef USE_BARO
    if (barometerConfig()->baro_hardware == BARO_AUTODETECT) {
        barometerConfigMutable()->baro_hardware = detectedSensors[SENSOR_INDEX_BARO];
        eepromUpdatePending = true;
    }
#endif

#ifdef USE_MAG
    if (compassConfig()->mag_hardware == MAG_AUTODETECT) {
        compassConfigMutable()->mag_hardware = detectedSensors[SENSOR_INDEX_MAG];
        eepromUpdatePending = true;
    }
#endif

#ifdef USE_PITOT
    if (pitotmeterConfig()->pitot_hardware == PITOT_AUTODETECT) {
        pitotmeterConfigMutable()->pitot_hardware = detectedSensors[SENSOR_INDEX_PITOT];
        eepromUpdatePending = true;
    }
#endif

#ifdef USE_IRLOCK
    irlockInit();
#endif

#ifdef USE_ROTOR
    rotorInit();
#endif

    if (eepromUpdatePending) {
        suspendRxSignal();
        writeEEPROM();
        resumeRxSignal();
    }

    return true;
}
