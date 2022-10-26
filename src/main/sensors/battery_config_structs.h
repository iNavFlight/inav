/*
 * This file is part of INAV
 *
 * INAV free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

typedef enum {
    CURRENT_SENSOR_NONE = 0,
    CURRENT_SENSOR_ADC,
    CURRENT_SENSOR_VIRTUAL,
    CURRENT_SENSOR_ESC,
    CURRENT_SENSOR_MAX = CURRENT_SENSOR_VIRTUAL
} currentSensor_e;

typedef enum {
    VOLTAGE_SENSOR_NONE = 0,
    VOLTAGE_SENSOR_ADC,
    VOLTAGE_SENSOR_ESC
} voltageSensor_e;

typedef enum {
    BAT_CAPACITY_UNIT_MAH,
    BAT_CAPACITY_UNIT_MWH,
} batCapacityUnit_e;

typedef enum {
    BAT_VOLTAGE_RAW,
    BAT_VOLTAGE_SAG_COMP
} batVoltageSource_e;

typedef struct batteryMetersConfig_s {

#ifdef USE_ADC
    struct {
        uint16_t scale;
        voltageSensor_e type;
    } voltage;
#endif

    struct {
        int16_t scale;          // scale the current sensor output voltage to milliamps. Value in 1/10th mV/A
        int16_t offset;         // offset of the current sensor in millivolt steps
        currentSensor_e type;   // type of current meter used, either ADC or virtual
    } current;

    batVoltageSource_e voltageSource;

    uint32_t cruise_power;      // power drawn by the motor(s) at cruise throttle/speed (cW)
    uint16_t idle_power;        // power drawn by the system when the motor(s) are stopped (cW)
    uint8_t rth_energy_margin;  // Energy that should be left after RTH (%), used for remaining time/distance before RTH

    float throttle_compensation_weight;

} batteryMetersConfig_t;

typedef struct batteryProfile_s {

#ifdef USE_ADC
    uint8_t cells;

    struct {
        uint16_t cellDetect;    // maximum voltage per cell, used for auto-detecting battery cell count in 0.01V units, default is 430 (4.3V)
        uint16_t cellMax;       // maximum voltage per cell, used for auto-detecting battery voltage in 0.01V units, default is 421 (4.21V)
        uint16_t cellMin;       // minimum voltage per cell, this triggers battery critical alarm, in 0.01V units, default is 330 (3.3V)
        uint16_t cellWarning;   // warning voltage per cell, this triggers battery warning alarm, in 0.01V units, default is 350 (3.5V)
    } voltage;
#endif

    struct {
        uint32_t value;                     // mAh or mWh (see capacity.unit)
        uint32_t warning;                   // mAh or mWh (see capacity.unit)
        uint32_t critical;                  // mAh or mWh (see capacity.unit)
        batCapacityUnit_e unit;             // Describes unit of capacity.value, capacity.warning and capacity.critical
    } capacity;

    uint8_t controlRateProfile;

    struct {
        float throttleIdle;                 // Throttle IDLE value based on min_command, max_throttle, in percent
        float throttleScale;                // Scaling factor for throttle.
#ifdef USE_DSHOT
        uint8_t turtleModePowerFactor;      // Power factor from 0 to 100% of flip over after crash
#endif
    } motor;

    uint16_t failsafe_throttle;             // Throttle level used for landing - specify value between 1000..2000 (pwm pulse width for slightly below hover). center throttle = 1500.

    uint16_t fwMinThrottleDownPitchAngle;

    struct {

        struct {
            bool thr_hover_learn_enabled;  // Enable/Disabled hover thrust learning 
            uint16_t hover_throttle;       // multicopter hover throttle
        } mc;

        struct {
            uint16_t cruise_throttle;       // Cruise throttle
            uint16_t min_throttle;          // Minimum allowed throttle in auto mode
            uint16_t max_throttle;          // Maximum allowed throttle in auto mode
            uint8_t  pitch_to_throttle;     // Pitch angle (in deg) to throttle gain (in 1/1000's of throttle) (*10)
            uint16_t launch_idle_throttle;  // Throttle to keep at launch idle
            uint16_t launch_throttle;       // Launch throttle
        } fw;

    } nav;

#if defined(USE_POWER_LIMITS)
    struct {
        uint16_t continuousCurrent;         // dA
        uint16_t burstCurrent;              // dA
        uint16_t burstCurrentTime;          // ds
        uint16_t burstCurrentFalldownTime;  // ds

#ifdef USE_ADC
        uint16_t continuousPower;           // dW
        uint16_t burstPower;                // dW
        uint16_t burstPowerTime;            // ds
        uint16_t burstPowerFalldownTime;    // ds
#endif // USE_ADC
    } powerLimits;
#endif // USE_POWER_LIMITS

} batteryProfile_t;
