/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "platform.h"

#include "drivers/time.h"
#include "flight/variable_pitch.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "fc/settings.h"
#include "fc/runtime_config.h"

#include "sensors/esc_sensor.h"

#include "common/maths.h"

#if defined(USE_VARIABLE_PITCH)

// woga65: helicopter specific settings
PG_REGISTER_WITH_RESET_TEMPLATE(helicopterConfig_t, helicopterConfig, PG_HELICOPTER_CONFIG, 0);

PG_RESET_TEMPLATE(helicopterConfig_t, helicopterConfig,
    .nav_hc_hover_collective[0] = 1600,                 // normal
    .nav_hc_hover_collective[1] = 1550,                 // idle-up 1
    .nav_hc_hover_collective[2] = 1525,                 // idle-up 2
    .hc_althold_collective_type = HC_ALT_HOLD_STICK,    // ALTHOLD colective stick behaviour
    .hc_rotor_spoolup_time = 10,                        // time for the rotor(s) to spool up
    .hc_governor_type = HC_GOVERNOR_OFF,                // type of governor to use
    .hc_main_motor_number = 0,                          // the motor that the governor takes care of
    .hc_governor_rpm[0] = 1200,                         // normal
    .hc_governor_rpm[1] = 1400,                         // idle-up 1
    .hc_governor_rpm[2] = 1600,                         // idle-up 2
    .hc_gov_pid_P = 40.0f,                              // PID gains for the governor
    .hc_gov_pid_I = 20.0f,
    .hc_gov_pid_D = 4.0f,
);

// woga65: soft spool-up related variables
bool shallSpoolUp = false;
bool isSpoolingUp = false;
bool hasSpooledUp = false;
bool rpmSettled   = false;

float spoolUpSteps = 0;
float currentThrottle = 1000;

timeMs_t spoolUpEndTime = 0;
timeMs_t spoolUpStartTime = 0;
timeMs_t deltaTime = 0;

static bool setGovernorMaxRpm(void);


uint16_t getHoverCollectivePitch(void) {
    const uint8_t headspeed = FLIGHT_MODE(HC_IDLE_UP_2) ? 2 : FLIGHT_MODE(HC_IDLE_UP_1) ? 1 : 0;
    return helicopterConfig()->nav_hc_hover_collective[headspeed];
}


uint8_t getHelicopterFlightMode(void) {
    return FLIGHT_MODE(HC_IDLE_UP_2) ? 2 : FLIGHT_MODE(HC_IDLE_UP_1) ? 1 : 0;
}


uint16_t getHelicopterDesiredHeadspeed(void) {
    return helicopterConfig()->hc_governor_rpm[getHelicopterFlightMode()];
}


uint8_t getSpoolupTime(void) {
    return helicopterConfig()->hc_rotor_spoolup_time;
}


uint16_t spoolupRotors(uint16_t throttleSetpoint) {

    /* Spool-up has finished before or is not used or throttle has been cut? */
    if (!shallSpoolUp || (!isSpoolingUp && throttleSetpoint == 1000) || getSpoolupTime() == 0) {
        hasSpooledUp = (getSpoolupTime() == 0) ? true : hasSpooledUp;

        /* Is SET governor configured and has reached the desired RPM during spool-up? */
        if (setGovernorMaxRpm()) {
            return currentThrottle + 0.5f;
        }
        /* If not, return throttle as is. */
        return throttleSetpoint;
    }

    /* Setup starting conditions for spool-up. */
    if (!isSpoolingUp) {
        deltaTime = 0;
        currentThrottle = 1000;
        spoolUpStartTime = millis();
        spoolUpEndTime = spoolUpStartTime + (getSpoolupTime() * 1000);
        spoolUpSteps = (throttleSetpoint - currentThrottle) / ((spoolUpEndTime - spoolUpStartTime) * 0.005f);
        isSpoolingUp = true;
    }

    /* Spool-Up has been interrupted. Start over again. */
    if (throttleSetpoint == 1000) {
        isSpoolingUp = false;
        return throttleSetpoint;
    }

    /* Spool-Up is finished because setpoint has been lowered. */ 
    if (throttleSetpoint <= currentThrottle) {
        shallSpoolUp = false;
        isSpoolingUp = false;
        hasSpooledUp = true;
        currentThrottle = 1000;
        return throttleSetpoint;
    }

    /* Increase throttle every 200ms */
    if (millis() >= deltaTime) {
        currentThrottle += spoolUpSteps;
        deltaTime = millis() + 200;
    
        /* Is spool-up finished? */
        if (currentThrottle >= throttleSetpoint || setGovernorMaxRpm()) {
            shallSpoolUp = false;
            isSpoolingUp = false;
            hasSpooledUp = true;
        }
    }

    return (uint16_t)(currentThrottle + 0.5f);
}


void prepareSoftSpoolup(void) {
    currentThrottle = 1000;
    shallSpoolUp = true;
    hasSpooledUp = false;
    rpmSettled = false;
}


bool hasFullySpooledUp(void) {
    if (!rpmSettled) {
        rpmSettled = (millis() >= deltaTime) ? true : false;
    }
    return (rpmSettled) ? hasSpooledUp : false;
}


uint8_t getMainMotorNumber(void) {
    return helicopterConfig()->hc_main_motor_number;
}


static bool setGovernorMaxRpm(void) {
#ifdef USE_ESC_SENSOR
    const escSensorData_t * escSensor = getEscTelemetry(getMainMotorNumber());
    return (
        helicopterConfig()->hc_governor_type == HC_GOVERNOR_SET &&
        escSensor->dataAge <= ESC_DATA_MAX_AGE && 
        escSensor->rpm >= getHelicopterDesiredHeadspeed()
        );
#else
    return false;
#endif
}

#endif