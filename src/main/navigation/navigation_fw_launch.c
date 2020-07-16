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
#include <math.h>
#include <string.h>

#include "platform.h"

#if defined(USE_NAV)

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"

#include "config/feature.h"

#include "drivers/time.h"

#include "io/gps.h"
#include "io/beeper.h"

#include "flight/pid.h"
#include "flight/imu.h"
#include "flight/mixer.h"

#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/runtime_config.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#define SWING_LAUNCH_MIN_ROTATION_RATE      DEGREES_TO_RADIANS(100)     // expect minimum 100dps rotation rate
#define LAUNCH_MOTOR_IDLE_SPINUP_TIME 1500                              // ms

enum {
    FW_LAUNCH_STATE_WAIT_THROTTLE = 0,
    FW_LAUNCH_STATE_MOTOR_IDLE = 1,
    FW_LAUNCH_STATE_WAIT_DETECTION = 2,
    FW_LAUNCH_STATE_DETECTED = 3,
    FW_LAUNCH_STATE_MOTOR_DELAY = 4,
    FW_LAUNCH_STATE_MOTOR_SPIN_UP = 5,
    FW_LAUNCH_STATE_IN_PROGRESS = 6,
    FW_LAUNCH_STATE_FINISHING = 7,
    FW_LAUNCH_STATE_FINISHED = 8
};

typedef struct FixedWingLaunchState_s {
    /* Launch detection */
    timeUs_t launchDetectorPreviousUpdate;
    timeUs_t launchDetectionTimeAccum;

    int state;
    timeUs_t stateStartedTimeUs;
    timeUs_t launchStartedTimeUs;
} FixedWingLaunchState_t;

static EXTENDED_FASTRAM FixedWingLaunchState_t   launchState;


static void setLaunchState(int state, timeUs_t currentTimeUs)
{
    launchState.state = state;
    launchState.stateStartedTimeUs = currentTimeUs;
}

static timeMs_t currentStateElapsedMs(timeUs_t currentTimeUs)
{
    return US2MS(currentTimeUs - launchState.stateStartedTimeUs);
}

static timeMs_t getElapsedMsAndSetNextState(uint16_t input, timeUs_t currentTimeUs, int nextState)
{
    const timeMs_t elapsedTimeMs = currentStateElapsedMs(currentTimeUs);

    if (input == 0 || input < elapsedTimeMs) {
        setLaunchState(nextState, currentTimeUs);
    }
    return elapsedTimeMs;
}

static inline bool isFixedWingLaunchMaxAltitudeReached(void)
{
    return (navConfig()->fw.launch_max_altitude > 0) && (getEstimatedActualPosition(Z) >= navConfig()->fw.launch_max_altitude);
}

static inline bool isLaunchModeMinTimeElapsed(float currentTimeUs)
{
    return US2MS(currentTimeUs - launchState.launchStartedTimeUs) > navConfig()->fw.launch_min_time;
}

static void updateFixedWingLaunchPitchAngle(uint8_t pitchAngle)
{
    rcCommand[ROLL] = 0;
    rcCommand[PITCH] = pidAngleToRcCommand(-DEGREES_TO_DECIDEGREES(pitchAngle), pidProfile()->max_angle_inclination[FD_PITCH]);
    rcCommand[YAW] = 0;
}

static void lockLaunchPitchAngle(void)
{
    updateFixedWingLaunchPitchAngle(navConfig()->fw.launch_climb_angle);
}

static void lockLaunchThrottleIdle(void)
{
    rcCommand[THROTTLE] = MAX(getThrottleIdleValue(), navConfig()->fw.launch_idle_throttle);
}

static void fixedWingLaunchResetPids(void)
{
    // Until motors are started don't use PID I-term and reset TPA filter
    if (launchState.state < FW_LAUNCH_STATE_MOTOR_SPIN_UP) {
        pidResetErrorAccumulators();
        pidResetTPAFilter();
    }
}

static void updateFixedWingLaunchFinishingTriggers(timeUs_t currentTimeUs)
{
    if (launchState.state < FW_LAUNCH_STATE_MOTOR_DELAY || launchState.state > FW_LAUNCH_STATE_IN_PROGRESS) {
        return;
    }
    if (isFixedWingLaunchMaxAltitudeReached() || (areSticksDeflectedMoreThanPosHoldDeadband() && isLaunchModeMinTimeElapsed(currentTimeUs))) {
        setLaunchState(FW_LAUNCH_STATE_FINISHING, currentTimeUs);
    }
}

static void updateFixedWingLaunchThrottleWait(timeUs_t currentTimeUs)
{
    if (calculateThrottleStatus(THROTTLE_STATUS_TYPE_RC) != THROTTLE_LOW) {
        setLaunchState(FW_LAUNCH_STATE_MOTOR_IDLE, currentTimeUs);
    }
    rcCommand[THROTTLE] = getThrottleIdleValue();
}

static void updateFixedWingLaunchDetector(timeUs_t currentTimeUs)
{
    const float swingVelocity = (fabsf(imuMeasuredRotationBF.z) > SWING_LAUNCH_MIN_ROTATION_RATE) ? (imuMeasuredAccelBF.y / imuMeasuredRotationBF.z) : 0;
    const bool isForwardAccelerationHigh = (imuMeasuredAccelBF.x > navConfig()->fw.launch_accel_thresh);
    const bool isAircraftAlmostLevel = (calculateCosTiltAngle() >= cos_approx(DEGREES_TO_RADIANS(navConfig()->fw.launch_max_angle)));

    const bool isBungeeLaunched = isForwardAccelerationHigh && isAircraftAlmostLevel;
    const bool isSwingLaunched = (swingVelocity > navConfig()->fw.launch_velocity_thresh) && (imuMeasuredAccelBF.x > 0);

    if (isBungeeLaunched || isSwingLaunched) {
        launchState.launchDetectionTimeAccum += (currentTimeUs - launchState.launchDetectorPreviousUpdate);
        launchState.launchDetectorPreviousUpdate = currentTimeUs;
        if (launchState.launchDetectionTimeAccum >= MS2US((uint32_t)navConfig()->fw.launch_time_thresh)) {
            setLaunchState(FW_LAUNCH_STATE_DETECTED, currentTimeUs);
        }
    }
    else {
        launchState.launchDetectorPreviousUpdate = currentTimeUs;
        launchState.launchDetectionTimeAccum = 0;
    }
}

static void updateFixedWingLaunchThrottleIdle(timeMs_t currentTimeUs)
{
    const int throttleIdleValue = getThrottleIdleValue();

    // Throttle control logic
    if (navConfig()->fw.launch_idle_throttle <= throttleIdleValue) {
        ENABLE_STATE(NAV_MOTOR_STOP_OR_IDLE);                              // If MOTOR_STOP is enabled mixer will keep motor stopped
        rcCommand[THROTTLE] = throttleIdleValue;                           // If MOTOR_STOP is disabled, motors will spin at minthrottle
        setLaunchState(FW_LAUNCH_STATE_WAIT_DETECTION, currentTimeUs);
        lockLaunchPitchAngle();
    }
    else {
        timeMs_t elapsedTimeMs = elapsedTimeMs = getElapsedMsAndSetNextState(LAUNCH_MOTOR_IDLE_SPINUP_TIME, currentTimeUs, FW_LAUNCH_STATE_WAIT_DETECTION);
        rcCommand[THROTTLE] = scaleRangef(elapsedTimeMs, 0.0f, LAUNCH_MOTOR_IDLE_SPINUP_TIME, throttleIdleValue, navConfig()->fw.launch_idle_throttle);
        updateFixedWingLaunchPitchAngle(scaleRangef(elapsedTimeMs, 0.0f, LAUNCH_MOTOR_IDLE_SPINUP_TIME, 0, navConfig()->fw.launch_climb_angle));
    }
}

static void updateFixedWingLaunchMotorDelay(timeUs_t currentTimeUs) {
    getElapsedMsAndSetNextState(navConfig()->fw.launch_motor_timer, currentTimeUs, FW_LAUNCH_STATE_MOTOR_SPIN_UP);
    lockLaunchThrottleIdle();
    lockLaunchPitchAngle();
}

static void updateFixedWingLaunchSpinUp(timeUs_t currentTimeUs)
{
    const uint16_t motorSpinUpMs = navConfig()->fw.launch_motor_spinup_time;
    const timeMs_t elapsedTimeMs = getElapsedMsAndSetNextState(motorSpinUpMs, currentTimeUs, FW_LAUNCH_STATE_IN_PROGRESS);
    const uint16_t minIdleThrottle = MAX(getThrottleIdleValue(), navConfig()->fw.launch_idle_throttle);

    rcCommand[THROTTLE] = scaleRangef(elapsedTimeMs, 0.0f, motorSpinUpMs,  minIdleThrottle, navConfig()->fw.launch_throttle);
    lockLaunchPitchAngle();
}

static void updateFixedWingLaunchProgress(timeUs_t currentTimeUs)
{
    getElapsedMsAndSetNextState(navConfig()->fw.launch_timeout, currentTimeUs, FW_LAUNCH_STATE_FINISHING);
    rcCommand[THROTTLE] = navConfig()->fw.launch_throttle;
    lockLaunchPitchAngle();
}

static void updateFixedWingLaunchFinishing(timeUs_t currentTimeUs)
{
    const uint16_t endTimeMs = navConfig()->fw.launch_end_time;
    const timeMs_t elapsedTimeMs = getElapsedMsAndSetNextState(endTimeMs, currentTimeUs, FW_LAUNCH_STATE_FINISHED);

    rcCommand[THROTTLE] = scaleRangef(elapsedTimeMs, 0.0f, endTimeMs,  navConfig()->fw.launch_throttle, rcCommand[THROTTLE]);
    updateFixedWingLaunchPitchAngle(scaleRangef(elapsedTimeMs, 0.0f, endTimeMs, navConfig()->fw.launch_climb_angle, rcCommand[PITCH]));
}

// Public methods ---------------------------------------------------------------

void applyFixedWingLaunchController(timeUs_t currentTimeUs)
{
    // Called at PID rate

    updateFixedWingLaunchFinishingTriggers(currentTimeUs);
    fixedWingLaunchResetPids();

    switch (launchState.state) {
        case FW_LAUNCH_STATE_WAIT_THROTTLE:
            updateFixedWingLaunchThrottleWait(currentTimeUs); // raise throttle stick to jump to FW_LAUNCH_STATE_MOTOR_IDLE and activate detection
            break;

        case FW_LAUNCH_STATE_MOTOR_IDLE:
            updateFixedWingLaunchDetector(currentTimeUs); // if the launch is detected, the throttle idle will be skipped and jump to FW_LAUNCH_STATE_DETECTED
            updateFixedWingLaunchThrottleIdle(currentTimeUs);
            break;

        case FW_LAUNCH_STATE_WAIT_DETECTION:
            updateFixedWingLaunchDetector(currentTimeUs); // when the launch is detected, we jump to FW_LAUNCH_STATE_DETECTED
            lockLaunchThrottleIdle();
            lockLaunchPitchAngle();
            break;

        case FW_LAUNCH_STATE_DETECTED:
            lockLaunchThrottleIdle();
            lockLaunchPitchAngle();
            // nothing else to do, wait for the navigation to switch to NAV_LAUNCH_STATE_MOTOR_DELAY state by calling enableFixedWingLaunchController()
            break;

        case FW_LAUNCH_STATE_MOTOR_DELAY:
            updateFixedWingLaunchMotorDelay(currentTimeUs); // when finish jump to FW_LAUNCH_STATE_MOTOR_SPIN_UP
            break;

        case FW_LAUNCH_STATE_MOTOR_SPIN_UP:
            updateFixedWingLaunchSpinUp(currentTimeUs); // when finish jump to FW_LAUNCH_STATE_IN_PROGRESS
            break;

        case FW_LAUNCH_STATE_IN_PROGRESS:
            updateFixedWingLaunchProgress(currentTimeUs); // when finish jump to FW_LAUNCH_STATE_FINISHING
            break;

        case FW_LAUNCH_STATE_FINISHING:
            updateFixedWingLaunchFinishing(currentTimeUs); // when finish jump to FW_LAUNCH_STATE_FINISHED
            break;

        case FW_LAUNCH_STATE_FINISHED:
        default:
            return;
    }

    // Control beeper
    beeper(BEEPER_LAUNCH_MODE_ENABLED);
}

void resetFixedWingLaunchController(timeUs_t currentTimeUs)
{
    launchState.launchDetectorPreviousUpdate = currentTimeUs;
    launchState.launchDetectionTimeAccum = 0;
    launchState.launchStartedTimeUs = 0;
    setLaunchState(FW_LAUNCH_STATE_WAIT_THROTTLE, currentTimeUs);
}

bool isFixedWingLaunchDetected(void)
{
    return launchState.state == FW_LAUNCH_STATE_DETECTED;
}

void enableFixedWingLaunchController(timeUs_t currentTimeUs)
{
    setLaunchState(FW_LAUNCH_STATE_MOTOR_DELAY, currentTimeUs);
    launchState.launchStartedTimeUs = currentTimeUs;
}

bool isFixedWingLaunchFinishedOrAborted(void)
{
    return launchState.state == FW_LAUNCH_STATE_FINISHED;
}

void abortFixedWingLaunch(void)
{
    launchState.state = FW_LAUNCH_STATE_FINISHED;
}

#endif
