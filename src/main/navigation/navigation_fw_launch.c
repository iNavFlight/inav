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
#define UNUSED(x) ((void)(x))

typedef enum {
    FW_LAUNCH_EVENT_NONE = 0,
    FW_LAUNCH_EVENT_SUCCESS,
    FW_LAUNCH_EVENT_GOTO_DETECTION,
    FW_LAUNCH_EVENT_ABORT,
    FW_LAUNCH_EVENT_COUNT
} fixedWingLaunchEvent_t;

typedef enum {
    FW_LAUNCH_STATE_IDLE = 0,
    FW_LAUNCH_STATE_INIT,
    FW_LAUNCH_STATE_WAIT_THROTTLE,
    FW_LAUNCH_STATE_MOTOR_IDLE,
    FW_LAUNCH_STATE_WAIT_DETECTION,
    FW_LAUNCH_STATE_DETECTED,
    FW_LAUNCH_STATE_MOTOR_DELAY,
    FW_LAUNCH_STATE_MOTOR_SPINUP,
    FW_LAUNCH_STATE_IN_PROGRESS,
    FW_LAUNCH_STATE_FINISH,
    FW_LAUNCH_STATE_COUNT
} fixedWingLaunchState_t;

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_IDLE(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_INIT(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_WAIT_THROTTLE(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_MOTOR_IDLE(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_WAIT_DETECTION(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_DETECTED(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_MOTOR_DELAY(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_MOTOR_SPINUP(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_IN_PROGRESS(timeUs_t currentTimeUs);
static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_FINISH(timeUs_t currentTimeUs);

typedef struct fixedWingLaunchStateDescriptor_s {

    fixedWingLaunchEvent_t              (*onEntry)(timeUs_t currentTimeUs);
    fixedWingLaunchState_t              onEvent[FW_LAUNCH_EVENT_COUNT];

} fixedWingLaunchStateDescriptor_t;

static EXTENDED_FASTRAM timeUs_t fwStateTimeUs;
static EXTENDED_FASTRAM fixedWingLaunchState_t fwState;

static const fixedWingLaunchStateDescriptor_t launchSM[FW_LAUNCH_STATE_COUNT] = {

    [FW_LAUNCH_STATE_IDLE] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_IDLE,
        .onEvent = {

        }
    },

    [FW_LAUNCH_STATE_INIT] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_INIT,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_WAIT_THROTTLE,
            [FW_LAUNCH_EVENT_GOTO_DETECTION]        = FW_LAUNCH_STATE_WAIT_DETECTION
        }
    },
    [FW_LAUNCH_STATE_WAIT_THROTTLE] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_WAIT_THROTTLE,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_MOTOR_IDLE
        }
    },

    [FW_LAUNCH_STATE_MOTOR_IDLE] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_MOTOR_IDLE,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_WAIT_DETECTION
        }
    },

    [FW_LAUNCH_STATE_WAIT_DETECTION] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_WAIT_DETECTION,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_DETECTED
        }
    },

    [FW_LAUNCH_STATE_DETECTED] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_DETECTED,
        .onEvent = {
            // waiting for the navigation to move on the next state FW_LAUNCH_STATE_MOTOR_DELAY
        }
    },

    [FW_LAUNCH_STATE_MOTOR_DELAY] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_MOTOR_DELAY,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_MOTOR_SPINUP,
            [FW_LAUNCH_EVENT_ABORT]                 = FW_LAUNCH_STATE_FINISH
        }
    },

    [FW_LAUNCH_STATE_MOTOR_SPINUP] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_MOTOR_SPINUP,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_IN_PROGRESS,
            [FW_LAUNCH_EVENT_ABORT]                 = FW_LAUNCH_STATE_FINISH
        }
    },

    [FW_LAUNCH_STATE_IN_PROGRESS] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_IN_PROGRESS,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_FINISH,
        }
    },

    [FW_LAUNCH_STATE_FINISH] = {
        .onEntry                                    = fwLaunchState_FW_LAUNCH_STATE_FINISH,
        .onEvent = {
            [FW_LAUNCH_EVENT_SUCCESS]               = FW_LAUNCH_STATE_IDLE
        }
    }
};

/* Current State Handlers */

static timeMs_t currentStateElapsedMs(timeUs_t currentTimeUs) {
    return US2MS(currentTimeUs - fwStateTimeUs);
}

static void setCurrentState(fixedWingLaunchState_t nextState, timeUs_t currentTimeUs) {
    fwState = nextState;
    fwStateTimeUs = currentTimeUs;
}

/* Wing control Helpers */

static void updateFixedWingLaunchPitchAngle(uint8_t pitchAngle) {
    rcCommand[ROLL] = 0;
    rcCommand[PITCH] = pidAngleToRcCommand(-DEGREES_TO_DECIDEGREES(pitchAngle), pidProfile()->max_angle_inclination[FD_PITCH]);
    rcCommand[YAW] = 0;
}

static bool isFixedWingLaunchIdleEnabled(void) {
    return navConfig()->fw.launch_idle_throttle > getThrottleIdleValue();
}

static void applyLaunchPitchAngle(void) {
    updateFixedWingLaunchPitchAngle(navConfig()->fw.launch_climb_angle);
}

static void applyLaunchThrottleIdle(void) {
    if (isFixedWingLaunchIdleEnabled()) {
        rcCommand[THROTTLE] = navConfig()->fw.launch_idle_throttle;
    } else {
      ENABLE_STATE(NAV_MOTOR_STOP_OR_IDLE);                              // If MOTOR_STOP is enabled mixer will keep motor stopped
      rcCommand[THROTTLE] = getThrottleIdleValue();                      // If MOTOR_STOP is disabled, motors will spin at minthrottle
    }
}

static void fixedWingLaunchResetPids(void) {
    // Until motors are started don't use PID I-term and reset TPA filter
    if (fwState < FW_LAUNCH_STATE_MOTOR_SPINUP) {
        pidResetErrorAccumulators();
        pidResetTPAFilter();
    }
}

static inline bool isFixedWingLaunchMaxAltitudeReached(void) {
    return (navConfig()->fw.launch_max_altitude > 0) && (getEstimatedActualPosition(Z) >= navConfig()->fw.launch_max_altitude);
}

static inline bool fwLaunchShouldAbort(timeMs_t initialTime, timeUs_t currentTimeUs) {
    return initialTime + currentStateElapsedMs(currentTimeUs) > navConfig()->fw.launch_min_time && areSticksDeflectedMoreThanPosHoldDeadband();
}

/* onEntry state handlers */

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_IDLE(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_INIT(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    if (isFixedWingLaunchIdleEnabled()) {
        return FW_LAUNCH_EVENT_SUCCESS;
    } else {
        return FW_LAUNCH_EVENT_GOTO_DETECTION;
    }
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_WAIT_THROTTLE(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    if (calculateThrottleStatus(THROTTLE_STATUS_TYPE_RC) != THROTTLE_LOW) {
        return FW_LAUNCH_EVENT_SUCCESS;
    }
    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_MOTOR_IDLE(timeUs_t currentTimeUs) {
    const timeMs_t elapsedTimeMs = currentStateElapsedMs(currentTimeUs);
    if (elapsedTimeMs > LAUNCH_MOTOR_IDLE_SPINUP_TIME) {
        applyLaunchThrottleIdle();
        applyLaunchPitchAngle();
        return FW_LAUNCH_EVENT_SUCCESS;
    } else {
        rcCommand[THROTTLE] = scaleRangef(elapsedTimeMs, 0.0f, LAUNCH_MOTOR_IDLE_SPINUP_TIME, getThrottleIdleValue(), navConfig()->fw.launch_idle_throttle);
        updateFixedWingLaunchPitchAngle(scaleRangef(elapsedTimeMs, 0.0f, LAUNCH_MOTOR_IDLE_SPINUP_TIME, 0, navConfig()->fw.launch_climb_angle));
    }
    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_WAIT_DETECTION(timeUs_t currentTimeUs) {
    const float swingVelocity = (fabsf(imuMeasuredRotationBF.z) > SWING_LAUNCH_MIN_ROTATION_RATE) ? (imuMeasuredAccelBF.y / imuMeasuredRotationBF.z) : 0;
    const bool isForwardAccelerationHigh = (imuMeasuredAccelBF.x > navConfig()->fw.launch_accel_thresh);
    const bool isAircraftAlmostLevel = (calculateCosTiltAngle() >= cos_approx(DEGREES_TO_RADIANS(navConfig()->fw.launch_max_angle)));

    const bool isBungeeLaunched = isForwardAccelerationHigh && isAircraftAlmostLevel;
    const bool isSwingLaunched = (swingVelocity > navConfig()->fw.launch_velocity_thresh) && (imuMeasuredAccelBF.x > 0);

    applyLaunchThrottleIdle();
    applyLaunchPitchAngle();

    if (isBungeeLaunched || isSwingLaunched) {
        if (currentStateElapsedMs(currentTimeUs) > navConfig()->fw.launch_time_thresh) {
            return FW_LAUNCH_EVENT_SUCCESS;
        }
    } else {
        fwStateTimeUs = currentTimeUs; // reset the state counter
    }
    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_DETECTED(timeUs_t currentTimeUs) {
    UNUSED(currentTimeUs);

    applyLaunchThrottleIdle();
    applyLaunchPitchAngle();
    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_MOTOR_DELAY(timeUs_t currentTimeUs) {
    applyLaunchThrottleIdle();
    applyLaunchPitchAngle();

    if (fwLaunchShouldAbort(0, currentTimeUs)) {
        return FW_LAUNCH_EVENT_ABORT;
    }
    if (currentStateElapsedMs(currentTimeUs) > navConfig()->fw.launch_motor_timer) {
        return FW_LAUNCH_EVENT_SUCCESS;
    }
    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_MOTOR_SPINUP(timeUs_t currentTimeUs) {
    applyLaunchPitchAngle();

    const timeMs_t elapsedTimeMs = currentStateElapsedMs(currentTimeUs);

    if (fwLaunchShouldAbort(navConfig()->fw.launch_motor_timer, currentTimeUs)) {
        return FW_LAUNCH_EVENT_ABORT;
    }

    const uint16_t motorSpinUpMs = navConfig()->fw.launch_motor_spinup_time;
    const uint16_t minIdleThrottle = MAX(getThrottleIdleValue(), navConfig()->fw.launch_idle_throttle);
    const uint16_t launchThrottle = navConfig()->fw.launch_throttle;

    if (elapsedTimeMs > motorSpinUpMs) {
        rcCommand[THROTTLE] = launchThrottle;
        return FW_LAUNCH_EVENT_SUCCESS;
    } else {
        rcCommand[THROTTLE] = scaleRangef(elapsedTimeMs, 0.0f, motorSpinUpMs,  minIdleThrottle, launchThrottle);
    }
    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_IN_PROGRESS(timeUs_t currentTimeUs) {
    rcCommand[THROTTLE] = navConfig()->fw.launch_throttle;
    applyLaunchPitchAngle();

    if (isFixedWingLaunchMaxAltitudeReached()) {
        return FW_LAUNCH_EVENT_SUCCESS; // We're on air, cancel the launch and do the FW_LAUNCH_STATE_FINISH state
    }
    if (fwLaunchShouldAbort(navConfig()->fw.launch_motor_timer + navConfig()->fw.launch_motor_spinup_time, currentTimeUs)) {
        return FW_LAUNCH_EVENT_SUCCESS; // cancel the launch and do the FW_LAUNCH_STATE_FINISH state
    }
    if (currentStateElapsedMs(currentTimeUs) > navConfig()->fw.launch_timeout) {
        return FW_LAUNCH_EVENT_SUCCESS;
    }
    return FW_LAUNCH_EVENT_NONE;
}

static fixedWingLaunchEvent_t fwLaunchState_FW_LAUNCH_STATE_FINISH(timeUs_t currentTimeUs) {
    const timeMs_t elapsedTimeMs = currentStateElapsedMs(currentTimeUs);
    const timeMs_t endTimeMs = navConfig()->fw.launch_end_time;

    if (elapsedTimeMs > endTimeMs) {
        return FW_LAUNCH_EVENT_SUCCESS;
    } else {
        rcCommand[THROTTLE] = scaleRangef(elapsedTimeMs, 0.0f, endTimeMs,  navConfig()->fw.launch_throttle, rcCommand[THROTTLE]);
        updateFixedWingLaunchPitchAngle(scaleRangef(elapsedTimeMs, 0.0f, endTimeMs, navConfig()->fw.launch_climb_angle, rcCommand[PITCH]));
    }
    return FW_LAUNCH_EVENT_NONE;
}

// Public methods ---------------------------------------------------------------

void applyFixedWingLaunchController(timeUs_t currentTimeUs) {
    // Called at PID rate

    fixedWingLaunchResetPids();

    while (launchSM[fwState].onEntry) {
        fixedWingLaunchEvent_t newEvent = launchSM[fwState].onEntry(currentTimeUs);
        if (newEvent == FW_LAUNCH_EVENT_NONE) {
            break;
        }
        setCurrentState(launchSM[fwState].onEvent[newEvent], currentTimeUs);
    }

    // Control beeper
    beeper(BEEPER_LAUNCH_MODE_ENABLED);
}

void resetFixedWingLaunchController(timeUs_t currentTimeUs) {
    setCurrentState(FW_LAUNCH_STATE_INIT, currentTimeUs);
}

bool isFixedWingLaunchDetected(void) {
    return fwState == FW_LAUNCH_STATE_DETECTED;
}

void enableFixedWingLaunchController(timeUs_t currentTimeUs) {
    setCurrentState(FW_LAUNCH_STATE_MOTOR_DELAY, currentTimeUs);
}

bool isFixedWingLaunchFinishedOrAborted(void) {
    return fwState == FW_LAUNCH_STATE_IDLE;
}

void abortFixedWingLaunch(void) {
    setCurrentState(FW_LAUNCH_STATE_IDLE, 0);
}

const char * fwLaunchStateMessage(void) {
    if (fwState < FW_LAUNCH_STATE_WAIT_DETECTION) {
        return "RAISE THE THROTTLE";
    } else if (fwState == FW_LAUNCH_STATE_WAIT_DETECTION) {
        return "READY";
    } else if (fwState > FW_LAUNCH_STATE_DETECTED) {
        return "MOVE STICKS TO TAKE CONTROL"; // conforming to OSD_MESSAGE_LENGTH = 28 from osd.c
    } else {
        return NULL;
    }
}

#endif
