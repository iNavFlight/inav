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

#include "io/gps.h"

#include "sensors/battery.h"
#include "sensors/gyro.h"

#define SWING_LAUNCH_MIN_ROTATION_RATE      DEGREES_TO_RADIANS(100)     // expect minimum 100dps rotation rate
#define LAUNCH_MOTOR_IDLE_SPINUP_TIME 1500                              // ms
#if !defined(UNUSED)
#define UNUSED(x) ((void)(x))
#endif
#define FW_LAUNCH_MESSAGE_TEXT_WAIT_THROTTLE "RAISE THE THROTTLE"
#define FW_LAUNCH_MESSAGE_TEXT_WAIT_IDLE "WAITING FOR IDLE"
#define FW_LAUNCH_MESSAGE_TEXT_WAIT_DETECTION "READY TO LAUNCH"
#define FW_LAUNCH_MESSAGE_TEXT_IN_PROGRESS "MOVE THE STICKS TO ABORT"
#define FW_LAUNCH_MESSAGE_TEXT_FINISHING "FINISHING"

typedef enum {
    FW_LAUNCH_MESSAGE_TYPE_NONE = 0,
    FW_LAUNCH_MESSAGE_TYPE_WAIT_THROTTLE,
    FW_LAUNCH_MESSAGE_TYPE_WAIT_IDLE,
    FW_LAUNCH_MESSAGE_TYPE_WAIT_DETECTION,
    FW_LAUNCH_MESSAGE_TYPE_IN_PROGRESS,
    FW_LAUNCH_MESSAGE_TYPE_FINISHING
} fixedWingLaunchMessage_t;

typedef enum {
    FW_LAUNCH_EVENT_NONE = 0,
    FW_LAUNCH_EVENT_SUCCESS,
    FW_LAUNCH_EVENT_GOTO_DETECTION,
    FW_LAUNCH_EVENT_ABORT,
    FW_LAUNCH_EVENT_THROTTLE_LOW,
    FW_LAUNCH_EVENT_COUNT
} fixedWingLaunchEvent_t;

typedef enum {  // if changed update navFwLaunchStatus_e
    FW_LAUNCH_STATE_WAIT_THROTTLE = 0,
    FW_LAUNCH_STATE_IDLE_WIGGLE_WAIT,
    FW_LAUNCH_STATE_IDLE_MOTOR_DELAY,
    FW_LAUNCH_STATE_MOTOR_IDLE,
    FW_LAUNCH_STATE_WAIT_DETECTION,
    FW_LAUNCH_STATE_DETECTED,           // FW_LAUNCH_DETECTED = 5
    FW_LAUNCH_STATE_MOTOR_DELAY,
    FW_LAUNCH_STATE_MOTOR_SPINUP,
    FW_LAUNCH_STATE_IN_PROGRESS,
    FW_LAUNCH_STATE_FINISH,
    FW_LAUNCH_STATE_ABORTED,            // FW_LAUNCH_ABORTED = 10
    FW_LAUNCH_STATE_FLYING,             // FW_LAUNCH_FLYING = 11
    FW_LAUNCH_STATE_COUNT
} fixedWingLaunchState_t;

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_WAIT_THROTTLE(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_IDLE_WIGGLE_WAIT(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_IDLE_MOTOR_DELAY(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_MOTOR_IDLE(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_WAIT_DETECTION(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_DETECTED(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_MOTOR_DELAY(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_MOTOR_SPINUP(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_IN_PROGRESS(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_FINISH(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_ABORTED(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_FLYING(timeUs_t currentTimeUs);

typedef struct fixedWingLaunchStateDescriptor_s {
    fixedWingLaunchEvent_t (*onEntry)(timeUs_t currentTimeUs);
    fixedWingLaunchState_t onEvent[FW_LAUNCH_EVENT_COUNT];
    fixedWingLaunchMessage_t messageType;
} fixedWingLaunchStateDescriptor_t;

typedef struct fixedWingLaunchData_s {
    timeUs_t currentStateTimeUs;
    fixedWingLaunchState_t currentState;
    uint8_t pitchAngle; // used to smooth the transition of the pitch angle
} fixedWingLaunchData_t;

static EXTENDED_FASTRAM fixedWingLaunchData_t fwLaunch;
static bool idleMotorAboutToStart;

static const fixedWingLaunchStateDescriptor_t launchStateMachine[FW_LAUNCH_STATE_COUNT] = {

    [FW_LAUNCH_STATE_WAIT_THROTTLE] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_WAIT_THROTTLE,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_IDLE_WIGGLE_WAIT,
            [FW_LAUNCH_EVENT_GOTO_DETECTION]        = FW_LAUNCH_STATE_WAIT_DETECTION
        },
        .messageType                                = FW_LAUNCH_MESSAGE_TYPE_WAIT_THROTTLE
    },

    [FW_LAUNCH_STATE_IDLE_WIGGLE_WAIT] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_IDLE_WIGGLE_WAIT,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_MOTOR_IDLE,
            [FW_LAUNCH_EVENT_GOTO_DETECTION]        = FW_LAUNCH_STATE_IDLE_MOTOR_DELAY,
            [FW_LAUNCH_EVENT_THROTTLE_LOW]          = FW_LAUNCH_STATE_WAIT_THROTTLE,
        },
        .messageType                                = FW_LAUNCH_MESSAGE_TYPE_WAIT_IDLE
    },

    [FW_LAUNCH_STATE_IDLE_MOTOR_DELAY] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_IDLE_MOTOR_DELAY,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_MOTOR_IDLE,
            [FW_LAUNCH_EVENT_THROTTLE_LOW]          = FW_LAUNCH_STATE_WAIT_THROTTLE
        },
        .messageType                                = FW_LAUNCH_MESSAGE_TYPE_WAIT_IDLE
    },

    [FW_LAUNCH_STATE_MOTOR_IDLE] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_MOTOR_IDLE,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_WAIT_DETECTION,
            [FW_LAUNCH_EVENT_THROTTLE_LOW]          = FW_LAUNCH_STATE_WAIT_THROTTLE
        },
        .messageType                                = FW_LAUNCH_MESSAGE_TYPE_WAIT_IDLE
    },

    [FW_LAUNCH_STATE_WAIT_DETECTION] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_WAIT_DETECTION,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_DETECTED,
            [FW_LAUNCH_EVENT_THROTTLE_LOW]          = FW_LAUNCH_STATE_WAIT_THROTTLE
        },
        .messageType                                = FW_LAUNCH_MESSAGE_TYPE_WAIT_DETECTION
    },

    [FW_LAUNCH_STATE_DETECTED] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_DETECTED,
        .onEvent = {
            // waiting for the navigation to move on the next state FW_LAUNCH_STATE_MOTOR_DELAY
        },
        .messageType                                = FW_LAUNCH_MESSAGE_TYPE_WAIT_DETECTION
    },

    [FW_LAUNCH_STATE_MOTOR_DELAY] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_MOTOR_DELAY,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_MOTOR_SPINUP,
            [FW_LAUNCH_EVENT_ABORT]                 = FW_LAUNCH_STATE_ABORTED
        },
        .messageType                                = FW_LAUNCH_MESSAGE_TYPE_IN_PROGRESS
    },

    [FW_LAUNCH_STATE_MOTOR_SPINUP] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_MOTOR_SPINUP,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_IN_PROGRESS,
            [FW_LAUNCH_EVENT_ABORT]                 = FW_LAUNCH_STATE_ABORTED
        },
        .messageType                                = FW_LAUNCH_MESSAGE_TYPE_IN_PROGRESS
    },

    [FW_LAUNCH_STATE_IN_PROGRESS] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_IN_PROGRESS,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_FINISH,
            [FW_LAUNCH_EVENT_ABORT]                 = FW_LAUNCH_STATE_ABORTED
        },
        .messageType                                = FW_LAUNCH_MESSAGE_TYPE_IN_PROGRESS
    },

    [FW_LAUNCH_STATE_FINISH] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_FINISH,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_FLYING
        },
        .messageType                                = FW_LAUNCH_MESSAGE_TYPE_FINISHING
    },

        [FW_LAUNCH_STATE_ABORTED] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_ABORTED,
        .onEvent = {

        },
        .messageType                                = FW_LAUNCH_MESSAGE_TYPE_NONE
    },

        [FW_LAUNCH_STATE_FLYING] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_FLYING,
        .onEvent = {

        },
        .messageType                                = FW_LAUNCH_MESSAGE_TYPE_NONE
    }
};

/* Current State Handlers */

static timeMs_t currentStateElapsedMs(timeUs_t currentTimeUs)
{
    return US2MS(currentTimeUs - fwLaunch.currentStateTimeUs);
}

static void setCurrentState(fixedWingLaunchState_t nextState, timeUs_t currentTimeUs)
{
    fwLaunch.currentState = nextState;
    fwLaunch.currentStateTimeUs = currentTimeUs;
}

/* Wing control Helpers */

static void applyThrottleIdleLogic(bool forceMixerIdle)
{
    if (forceMixerIdle) {
        ENABLE_STATE(NAV_MOTOR_STOP_OR_IDLE);   // If MOTOR_STOP is enabled mixer will keep motor stopped, otherwise motor will run at idle
    } else {
        rcCommand[THROTTLE] = setDesiredThrottle(currentBatteryProfile->nav.fw.launch_idle_throttle, true);
    }
}

static bool hasIdleWakeWiggleSucceeded(timeUs_t currentTimeUs) {
    static timeMs_t wiggleTime = 0;
    static timeMs_t wigglesTime = 0;
    static int8_t   wiggleStageOne = 0;
    static uint8_t  wiggleCount = 0;
    const bool      isAircraftWithinLaunchAngle = (calculateCosTiltAngle() >= cos_approx(DEGREES_TO_RADIANS(navConfig()->fw.launch_max_angle)));
    const uint8_t   wiggleStrength = (navConfig()->fw.launch_wiggle_wake_idle == 1) ? 50 : 40;
    int8_t wiggleDirection = 0;
    int16_t yawRate = (int16_t)(gyroRateDps(YAW) * (4 / 16.4));

    // Check to see if yaw rate has exceeded 50 dps. If so proceed to the next stage or continue to idle
    if ((yawRate < -wiggleStrength || yawRate > wiggleStrength) && isAircraftWithinLaunchAngle) {
        wiggleDirection = (yawRate > 0) ? 1 : -1;

        if (wiggleStageOne == 0) {
            wiggleStageOne = wiggleDirection;
            wigglesTime = US2MS(currentTimeUs);
        } else if (wiggleStageOne != wiggleDirection) {
            wiggleStageOne = 0;
            wiggleCount++;

            if (wiggleCount == navConfig()->fw.launch_wiggle_wake_idle) {
                return true;
            }
        }

        wiggleTime = US2MS(currentTimeUs);
    }

    // If time between wiggle stages is > 100ms, or the time between two wiggles is > 1s. Reset the wiggle
    if (
        ((wiggleStageOne != 0) && (US2MS(currentTimeUs) > (wiggleTime + 100))) ||
        ((wiggleCount != 0) && (US2MS(currentTimeUs) > (wigglesTime + 500)))
    ) {
        wiggleStageOne = 0;
        wiggleCount = 0;
    }

    return false;
}

static inline bool isLaunchMaxAltitudeReached(void)
{
    return (navConfig()->fw.launch_max_altitude > 0) && (getEstimatedActualPosition(Z) >= navConfig()->fw.launch_max_altitude);
}

static inline bool areSticksMoved(timeMs_t initialTime, timeUs_t currentTimeUs)
{
    return (initialTime + currentStateElapsedMs(currentTimeUs)) >= navConfig()->fw.launch_min_time &&
            isRollPitchStickDeflected(navConfig()->fw.launch_land_abort_deadband);
}

static inline bool isProbablyNotFlying(void)
{
    // Check flight status but only if GPS lock valid
    return posControl.flags.estPosStatus == EST_TRUSTED && !isFixedWingFlying();
}

static void resetPidsIfNeeded(void) {
    // Don't use PID I-term and reset TPA filter until motors are started or until flight is detected
    if (isProbablyNotFlying() || fwLaunch.currentState < FW_LAUNCH_STATE_MOTOR_SPINUP || (navConfig()->fw.launch_manual_throttle && throttleStickIsLow())) {
        pidResetErrorAccumulators();
        pidResetTPAFilter();
    }
}

static void updateRcCommand(void)
{
    // lock roll and yaw and apply needed pitch angle
    rcCommand[ROLL] = 0;
    rcCommand[PITCH] = pidAngleToRcCommand(-DEGREES_TO_DECIDEGREES(fwLaunch.pitchAngle), pidProfile()->max_angle_inclination[FD_PITCH]);
    rcCommand[YAW] = 0;
}

/* onEntry state handlers */

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_WAIT_THROTTLE(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    if (!throttleStickIsLow()) {
        if (currentBatteryProfile->nav.fw.launch_idle_throttle > getThrottleIdleValue()) {
            return FW_LAUNCH_EVENT_SUCCESS;
        }
        else {
            fwLaunch.pitchAngle = navConfig()->fw.launch_climb_angle;
            return FW_LAUNCH_EVENT_GOTO_DETECTION;
        }
    }
    else {
        applyThrottleIdleLogic(true);   // Stick low, force mixer idle (motor stop or low rpm)
    }

    fwLaunch.pitchAngle = 0;

    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_IDLE_WIGGLE_WAIT(timeUs_t currentTimeUs) {
    if (throttleStickIsLow()) {
        return FW_LAUNCH_EVENT_THROTTLE_LOW; // go back to FW_LAUNCH_STATE_WAIT_THROTTLE
    }

    if (navConfig()->fw.launch_wiggle_wake_idle == 0 || navConfig()->fw.launch_idle_motor_timer > 0 ) {
        return FW_LAUNCH_EVENT_GOTO_DETECTION;
    }

    applyThrottleIdleLogic(true);

    if (hasIdleWakeWiggleSucceeded(currentTimeUs)) {
        idleMotorAboutToStart = false;
        return FW_LAUNCH_EVENT_SUCCESS;
    }

    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_IDLE_MOTOR_DELAY(timeUs_t currentTimeUs)
{
    if (throttleStickIsLow()) {
        return FW_LAUNCH_EVENT_THROTTLE_LOW; // go back to FW_LAUNCH_STATE_WAIT_THROTTLE
    }

    applyThrottleIdleLogic(true);

    if ((currentStateElapsedMs(currentTimeUs) > navConfig()->fw.launch_idle_motor_timer) || (navConfig()->fw.launch_wiggle_wake_idle > 0 && hasIdleWakeWiggleSucceeded(currentTimeUs))) {
        idleMotorAboutToStart = false;
        return FW_LAUNCH_EVENT_SUCCESS;
    }

    // 5 second warning motor about to start at idle, changes Beeper sound
    idleMotorAboutToStart = navConfig()->fw.launch_idle_motor_timer - currentStateElapsedMs(currentTimeUs) < 5000;

    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_MOTOR_IDLE(timeUs_t currentTimeUs)
{
    if (throttleStickIsLow()) {
        return FW_LAUNCH_EVENT_THROTTLE_LOW; // go back to FW_LAUNCH_STATE_WAIT_THROTTLE
    }

    const timeMs_t elapsedTimeMs = currentStateElapsedMs(currentTimeUs);
    if (elapsedTimeMs > LAUNCH_MOTOR_IDLE_SPINUP_TIME) {
        applyThrottleIdleLogic(false);
        return FW_LAUNCH_EVENT_SUCCESS;
    }
    else {
        rcCommand[THROTTLE] = scaleRangef(elapsedTimeMs, 0.0f, LAUNCH_MOTOR_IDLE_SPINUP_TIME, getThrottleIdleValue(), currentBatteryProfile->nav.fw.launch_idle_throttle);
        fwLaunch.pitchAngle = scaleRangef(elapsedTimeMs, 0.0f, LAUNCH_MOTOR_IDLE_SPINUP_TIME, 0, navConfig()->fw.launch_climb_angle);
    }

    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_WAIT_DETECTION(timeUs_t currentTimeUs)
{
    if (throttleStickIsLow()) {
        return FW_LAUNCH_EVENT_THROTTLE_LOW; // go back to FW_LAUNCH_STATE_WAIT_THROTTLE
    }

    const float swingVelocity = (fabsf(imuMeasuredRotationBF.z) > SWING_LAUNCH_MIN_ROTATION_RATE) ? (imuMeasuredAccelBF.y / imuMeasuredRotationBF.z) : 0;
    const bool isForwardAccelerationHigh = (imuMeasuredAccelBF.x > navConfig()->fw.launch_accel_thresh);
    const bool isAircraftAlmostLevel = (calculateCosTiltAngle() >= cos_approx(DEGREES_TO_RADIANS(navConfig()->fw.launch_max_angle)));

    const bool isBungeeLaunched = isForwardAccelerationHigh && isAircraftAlmostLevel;
    const bool isSwingLaunched = (swingVelocity > navConfig()->fw.launch_velocity_thresh) && (imuMeasuredAccelBF.x > 0);
    const bool isForwardLaunched = isGPSHeadingValid() && (gpsSol.groundSpeed > navConfig()->fw.launch_velocity_thresh) && (imuMeasuredAccelBF.x > 0);

    applyThrottleIdleLogic(false);

    if (isBungeeLaunched || isSwingLaunched || isForwardLaunched) {
        if (currentStateElapsedMs(currentTimeUs) > navConfig()->fw.launch_time_thresh) {
            return FW_LAUNCH_EVENT_SUCCESS; // the launch is detected now, go to FW_LAUNCH_STATE_DETECTED
        }
    } else {
        fwLaunch.currentStateTimeUs = currentTimeUs; // reset the state counter
    }

    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_DETECTED(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    // waiting for the navigation to move it to next step FW_LAUNCH_STATE_MOTOR_DELAY
    applyThrottleIdleLogic(false);

    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_MOTOR_DELAY(timeUs_t currentTimeUs)
{
    applyThrottleIdleLogic(false);

    if (areSticksMoved(0, currentTimeUs)) {
        return FW_LAUNCH_EVENT_ABORT; // jump to FW_LAUNCH_STATE_ABORTED
    }

    if (currentStateElapsedMs(currentTimeUs) > navConfig()->fw.launch_motor_timer) {
        return FW_LAUNCH_EVENT_SUCCESS;
    }

    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_MOTOR_SPINUP(timeUs_t currentTimeUs)
{
    if (areSticksMoved(navConfig()->fw.launch_motor_timer, currentTimeUs)) {
        return FW_LAUNCH_EVENT_ABORT; // jump to FW_LAUNCH_STATE_ABORTED
    }

    const timeMs_t elapsedTimeMs = currentStateElapsedMs(currentTimeUs);
    const uint16_t motorSpinUpMs = navConfig()->fw.launch_motor_spinup_time;
    const uint16_t launchThrottle = setDesiredThrottle(currentBatteryProfile->nav.fw.launch_throttle, false);

    if (elapsedTimeMs > motorSpinUpMs) {
        rcCommand[THROTTLE] = launchThrottle;
        return FW_LAUNCH_EVENT_SUCCESS;
    }
    else {
        const uint16_t minIdleThrottle = MAX(getThrottleIdleValue(), currentBatteryProfile->nav.fw.launch_idle_throttle);
        rcCommand[THROTTLE] = scaleRangef(elapsedTimeMs, 0.0f, motorSpinUpMs,  minIdleThrottle, launchThrottle);
    }

    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_IN_PROGRESS(timeUs_t currentTimeUs)
{
    uint16_t initialTime = 0;

    if (navConfig()->fw.launch_manual_throttle) {
        // reset timers when throttle is low or until flight detected and abort launch regardless of launch settings
        if (throttleStickIsLow()) {
            fwLaunch.currentStateTimeUs = currentTimeUs;
            fwLaunch.pitchAngle = 0;
            if (isRollPitchStickDeflected(navConfig()->fw.launch_land_abort_deadband)) {
                return FW_LAUNCH_EVENT_ABORT;
            }
        } else {
            if (isProbablyNotFlying()) {
                fwLaunch.currentStateTimeUs = currentTimeUs;
            }
            fwLaunch.pitchAngle = navConfig()->fw.launch_climb_angle;
        }
    } else {
        initialTime = navConfig()->fw.launch_motor_timer + navConfig()->fw.launch_motor_spinup_time;
        rcCommand[THROTTLE] = setDesiredThrottle(currentBatteryProfile->nav.fw.launch_throttle, false);
    }

    if (isLaunchMaxAltitudeReached()) {
        return FW_LAUNCH_EVENT_SUCCESS; // cancel the launch and do the FW_LAUNCH_STATE_FINISH state
    }

    if (areSticksMoved(initialTime, currentTimeUs)) {
        return FW_LAUNCH_EVENT_ABORT; // cancel the launch and do the FW_LAUNCH_STATE_ABORTED state
    }

    if (currentStateElapsedMs(currentTimeUs) > navConfig()->fw.launch_timeout) {
        return FW_LAUNCH_EVENT_SUCCESS; // launch timeout. go to FW_LAUNCH_STATE_FINISH
    }

    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_FINISH(timeUs_t currentTimeUs)
{
    const timeMs_t elapsedTimeMs = currentStateElapsedMs(currentTimeUs);
    const timeMs_t endTimeMs = navConfig()->fw.launch_end_time;

    if (elapsedTimeMs > endTimeMs || isRollPitchStickDeflected(navConfig()->fw.launch_land_abort_deadband)) {
        return FW_LAUNCH_EVENT_SUCCESS;     // End launch go to FW_LAUNCH_STATE_FLYING state
    }
    else {
        // Make a smooth transition from the launch state to the current state for pitch angle
        // Do the same for throttle when manual launch throttle isn't used
        if (!navConfig()->fw.launch_manual_throttle) {
            const uint16_t launchThrottle = setDesiredThrottle(currentBatteryProfile->nav.fw.launch_throttle, false);
            rcCommand[THROTTLE] = scaleRangef(elapsedTimeMs, 0.0f, endTimeMs, launchThrottle, rcCommand[THROTTLE]);
        }
        fwLaunch.pitchAngle = scaleRangef(elapsedTimeMs, 0.0f, endTimeMs, navConfig()->fw.launch_climb_angle, rcCommand[PITCH]);
    }

    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_ABORTED(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_FLYING(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    return FW_LAUNCH_EVENT_NONE;
}

// Public methods ---------------------------------------------------------------

void applyFixedWingLaunchController(timeUs_t currentTimeUs)
{
    // Called at PID rate

    // process the current state, set the next state or exit if FW_LAUNCH_EVENT_NONE
    while (launchStateMachine[fwLaunch.currentState].onEntry) {
        fixedWingLaunchEvent_t newEvent = launchStateMachine[fwLaunch.currentState].onEntry(currentTimeUs);
        if (newEvent == FW_LAUNCH_EVENT_NONE) {
            break;
        }
        setCurrentState(launchStateMachine[fwLaunch.currentState].onEvent[newEvent], currentTimeUs);
    }

    resetPidsIfNeeded();
    updateRcCommand();

    // Control beeper
    if (fwLaunch.currentState == FW_LAUNCH_STATE_WAIT_THROTTLE) {
        beeper(BEEPER_LAUNCH_MODE_LOW_THROTTLE);
    }
    else {
        if (idleMotorAboutToStart) {
            beeper(BEEPER_LAUNCH_MODE_IDLE_START);
        } else {
            beeper(BEEPER_LAUNCH_MODE_ENABLED);
        }
    }
}

void resetFixedWingLaunchController(timeUs_t currentTimeUs)
{
    if (navConfig()->fw.launch_manual_throttle) {
        // no detection or motor control required with manual launch throttle
        // so start at launch in progress
        setCurrentState(FW_LAUNCH_STATE_IN_PROGRESS, currentTimeUs);
    } else {
        setCurrentState(FW_LAUNCH_STATE_WAIT_THROTTLE, currentTimeUs);
    }
}

void enableFixedWingLaunchController(timeUs_t currentTimeUs)
{
    setCurrentState(FW_LAUNCH_STATE_MOTOR_DELAY, currentTimeUs);
}

uint8_t fixedWingLaunchStatus(void)
{
    return fwLaunch.currentState;
}

void abortFixedWingLaunch(void)
{
    setCurrentState(FW_LAUNCH_STATE_ABORTED, 0);
    DISABLE_FLIGHT_MODE(NAV_LAUNCH_MODE);
}

const char * fixedWingLaunchStateMessage(void)
{
    switch (launchStateMachine[fwLaunch.currentState].messageType) {
        case FW_LAUNCH_MESSAGE_TYPE_WAIT_THROTTLE:
            return FW_LAUNCH_MESSAGE_TEXT_WAIT_THROTTLE;

        case FW_LAUNCH_MESSAGE_TYPE_WAIT_IDLE:
            return FW_LAUNCH_MESSAGE_TEXT_WAIT_IDLE;

        case FW_LAUNCH_MESSAGE_TYPE_WAIT_DETECTION:
            return FW_LAUNCH_MESSAGE_TEXT_WAIT_DETECTION;

        case FW_LAUNCH_MESSAGE_TYPE_IN_PROGRESS:
            return FW_LAUNCH_MESSAGE_TEXT_IN_PROGRESS;

        case FW_LAUNCH_MESSAGE_TYPE_FINISHING:
            return FW_LAUNCH_MESSAGE_TEXT_FINISHING;

        default:
            return NULL;
    }
}
