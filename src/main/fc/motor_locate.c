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
#define LOCATE_JERK_DURATION_US       10000 // 10ms motor jerk (safety: minimal movement)
#define LOCATE_JERK_PAUSE_US          10000 // 10ms pause after jerk
#define LOCATE_BEEP_ON_US             80000 // 80ms beep on
#define LOCATE_BEEP_OFF_US            80000 // 80ms beep off
#define LOCATE_CYCLE_DURATION_US     2000000 // 2000ms total cycle

// Motor throttle for jerk (~12% throttle)
// DShot range: 48 (0%) to 2047 (100%)
#define LOCATE_JERK_THROTTLE    288  // 48 + (1999 * 0.12) ≈ 288

typedef enum {
    LOCATE_STATE_IDLE,
    LOCATE_STATE_JERK,
    LOCATE_STATE_JERK_PAUSE,
    LOCATE_STATE_BEEP_ON,
    LOCATE_STATE_BEEP_OFF,
} motorLocateState_e;

#define LOCATE_NUM_BEEPS 4

// Global flag for fast inline check in FAST_CODE path
bool motorLocateActive = false;

static struct {
    motorLocateState_e state;
    uint8_t motorIndex;
    uint8_t beepCount;
    timeUs_t stateStartTime;
    timeUs_t cycleStartTime;
} locateState = {
    .state = LOCATE_STATE_IDLE,
    .motorIndex = 0,
    .beepCount = 0,
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
    locateState.beepCount = 0;
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

    // Send beacon command once when entering BEEP_ON state
    if (newState == LOCATE_STATE_BEEP_ON) {
        dshotCommands_e beaconCmd = DSHOT_CMD_BEACON1 + locateState.beepCount;
        sendDShotCommandToMotor(locateState.motorIndex, beaconCmd);
    }
}

static timeUs_t getStateDuration(motorLocateState_e state)
{
    switch (state) {
        case LOCATE_STATE_JERK:
            return LOCATE_JERK_DURATION_US;
        case LOCATE_STATE_JERK_PAUSE:
            return LOCATE_JERK_PAUSE_US;
        case LOCATE_STATE_BEEP_ON:
            return LOCATE_BEEP_ON_US;
        case LOCATE_STATE_BEEP_OFF:
            return (locateState.beepCount >= LOCATE_NUM_BEEPS - 1) ? LOCATE_JERK_PAUSE_US : LOCATE_BEEP_OFF_US;
        default:
            return 0;
    }
}

static motorLocateState_e advanceToNextState(motorLocateState_e state)
{
    switch (state) {
        case LOCATE_STATE_JERK:
            return LOCATE_STATE_JERK_PAUSE;
        case LOCATE_STATE_JERK_PAUSE:
            locateState.beepCount = 0;
            return LOCATE_STATE_BEEP_ON;
        case LOCATE_STATE_BEEP_ON:
            return LOCATE_STATE_BEEP_OFF;
        case LOCATE_STATE_BEEP_OFF:
            if (++locateState.beepCount >= LOCATE_NUM_BEEPS) {
                return LOCATE_STATE_JERK;
            }
            return LOCATE_STATE_BEEP_ON;
        default:
            return LOCATE_STATE_IDLE;
    }
}

bool motorLocateUpdate(void)
{
    if (locateState.state == LOCATE_STATE_IDLE) {
        return false;
    }

    // Immediately stop if aircraft becomes armed
    if (ARMING_FLAG(ARMED)) {
        motorLocateStop();
        return false;
    }

    timeUs_t now = micros();

    // Check if total cycle time exceeded
    if (cmpTimeUs(now, locateState.cycleStartTime) >= LOCATE_CYCLE_DURATION_US) {
        motorLocateStop();
        return false;
    }

    // Check for state transition
    timeDelta_t stateDuration = getStateDuration(locateState.state);
    if (cmpTimeUs(now, locateState.stateStartTime) >= stateDuration) {
        transitionToState(advanceToNextState(locateState.state), now);
    }

    // Apply motor values for current state
    uint8_t motorCount = getMotorCount();

    if (locateState.state == LOCATE_STATE_JERK) {
        // For jerk state, use direct PWM write with throttle value
        pwmWriteMotor(locateState.motorIndex, LOCATE_JERK_THROTTLE);
        // Set all other motors to stop
        for (uint8_t i = 0; i < motorCount; i++) {
            if (i != locateState.motorIndex) {
                pwmWriteMotor(i, DSHOT_CMD_MOTOR_STOP);
            }
        }
    } else if (locateState.state == LOCATE_STATE_BEEP_ON || locateState.state == LOCATE_STATE_BEEP_OFF) {
        // For beep states, beacon command already sent, just keep motors stopped
        for (uint8_t i = 0; i < motorCount; i++) {
            pwmWriteMotor(i, DSHOT_CMD_MOTOR_STOP);
        }
    } else {
        // For other states (pause, etc.), ensure all motors stopped
        for (uint8_t i = 0; i < motorCount; i++) {
            pwmWriteMotor(i, DSHOT_CMD_MOTOR_STOP);
        }
    }

    return true;
}

#endif // USE_DSHOT
