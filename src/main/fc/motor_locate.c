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

#include "platform.h"

#ifdef USE_DSHOT

#include "drivers/pwm_output.h"
#include "drivers/time.h"
#include "flight/mixer.h"
#include "fc/motor_locate.h"
#include "fc/runtime_config.h"

// Timing constants (in microseconds)
#define LOCATE_JERK_DURATION_US     100000  // 100ms motor jerk
#define LOCATE_JERK_PAUSE_US        100000  // 100ms pause after jerk
#define LOCATE_BEEP_ON_US            80000  // 80ms beep on
#define LOCATE_BEEP_OFF_US           80000  // 80ms beep off
#define LOCATE_CYCLE_DURATION_US   2000000  // 2 seconds total cycle

// Motor throttle for jerk (~12% throttle)
// DShot range: 48 (0%) to 2047 (100%)
#define LOCATE_JERK_THROTTLE    288  // 48 + (1999 * 0.12) ≈ 288

typedef enum {
    LOCATE_STATE_IDLE,
    LOCATE_STATE_JERK,
    LOCATE_STATE_JERK_PAUSE,
    LOCATE_STATE_BEEP1_ON,
    LOCATE_STATE_BEEP1_OFF,
    LOCATE_STATE_BEEP2_ON,
    LOCATE_STATE_BEEP2_OFF,
    LOCATE_STATE_BEEP3_ON,
    LOCATE_STATE_BEEP3_PAUSE,
} motorLocateState_e;

// Global flag for fast inline check in FAST_CODE path
bool motorLocateActive = false;

static struct {
    motorLocateState_e state;
    uint8_t motorIndex;
    timeUs_t stateStartTime;
    timeUs_t cycleStartTime;
} locateState = {
    .state = LOCATE_STATE_IDLE,
    .motorIndex = 0,
    .stateStartTime = 0,
    .cycleStartTime = 0,
};

// Forward declarations
static void transitionToState(motorLocateState_e newState, timeUs_t now);
static timeUs_t getStateDuration(motorLocateState_e state);

bool motorLocateStart(uint8_t motorIndex)
{
    // Don't allow if already running
    if (locateState.state != LOCATE_STATE_IDLE) {
        return false;
    }

    // Validate motor index
    if (motorIndex >= getMotorCount()) {
        return false;
    }

    // Only allow when disarmed
    if (ARMING_FLAG(ARMED)) {
        return false;
    }

    // Only allow with DShot protocol
    if (!isMotorProtocolDshot()) {
        return false;
    }

    timeUs_t now = micros();
    locateState.motorIndex = motorIndex;
    locateState.cycleStartTime = now;
    transitionToState(LOCATE_STATE_JERK, now);
    motorLocateActive = true;

    return true;
}

void motorLocateStop(void)
{
    locateState.state = LOCATE_STATE_IDLE;
    motorLocateActive = false;
}

bool motorLocateIsActive(void)
{
    return locateState.state != LOCATE_STATE_IDLE;
}

static void transitionToState(motorLocateState_e newState, timeUs_t now)
{
    locateState.state = newState;
    locateState.stateStartTime = now;
}

static timeUs_t getStateDuration(motorLocateState_e state)
{
    switch (state) {
        case LOCATE_STATE_JERK:
            return LOCATE_JERK_DURATION_US;
        case LOCATE_STATE_JERK_PAUSE:
            return LOCATE_JERK_PAUSE_US;
        case LOCATE_STATE_BEEP1_ON:
        case LOCATE_STATE_BEEP2_ON:
        case LOCATE_STATE_BEEP3_ON:
            return LOCATE_BEEP_ON_US;
        case LOCATE_STATE_BEEP1_OFF:
        case LOCATE_STATE_BEEP2_OFF:
            return LOCATE_BEEP_OFF_US;
        case LOCATE_STATE_BEEP3_PAUSE:
            return LOCATE_JERK_PAUSE_US;
        default:
            return 0;
    }
}

static motorLocateState_e getNextState(motorLocateState_e state)
{
    switch (state) {
        case LOCATE_STATE_JERK:        return LOCATE_STATE_JERK_PAUSE;
        case LOCATE_STATE_JERK_PAUSE:  return LOCATE_STATE_BEEP1_ON;
        case LOCATE_STATE_BEEP1_ON:    return LOCATE_STATE_BEEP1_OFF;
        case LOCATE_STATE_BEEP1_OFF:   return LOCATE_STATE_BEEP2_ON;
        case LOCATE_STATE_BEEP2_ON:    return LOCATE_STATE_BEEP2_OFF;
        case LOCATE_STATE_BEEP2_OFF:   return LOCATE_STATE_BEEP3_ON;
        case LOCATE_STATE_BEEP3_ON:    return LOCATE_STATE_BEEP3_PAUSE;
        case LOCATE_STATE_BEEP3_PAUSE: return LOCATE_STATE_JERK;  // Loop back
        default:                       return LOCATE_STATE_IDLE;
    }
}

static uint16_t getMotorValueForState(motorLocateState_e state, uint8_t motorIdx, uint8_t targetMotorIdx)
{
    // All motors except target get MOTOR_STOP
    if (motorIdx != targetMotorIdx) {
        return DSHOT_CMD_MOTOR_STOP;
    }

    // Target motor value depends on state
    switch (state) {
        case LOCATE_STATE_JERK:
            return LOCATE_JERK_THROTTLE;
        case LOCATE_STATE_BEEP1_ON:
        case LOCATE_STATE_BEEP2_ON:
        case LOCATE_STATE_BEEP3_ON:
            return DSHOT_CMD_BEACON1;
        default:
            return DSHOT_CMD_MOTOR_STOP;
    }
}

bool motorLocateUpdate(void)
{
    if (locateState.state == LOCATE_STATE_IDLE) {
        return false;
    }

    timeUs_t now = micros();

    // Check if total cycle time exceeded
    if (cmpTimeUs(now, locateState.cycleStartTime) >= LOCATE_CYCLE_DURATION_US) {
        motorLocateStop();
        return false;
    }

    // Check for state transition
    timeUs_t stateDuration = getStateDuration(locateState.state);
    if (cmpTimeUs(now, locateState.stateStartTime) >= stateDuration) {
        transitionToState(getNextState(locateState.state), now);
    }

    // Apply motor values for current state
    uint8_t motorCount = getMotorCount();
    for (uint8_t i = 0; i < motorCount; i++) {
        uint16_t value = getMotorValueForState(locateState.state, i, locateState.motorIndex);
        pwmWriteMotor(i, value);
    }

    return true;
}

#endif // USE_DSHOT
