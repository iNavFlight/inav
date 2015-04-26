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

#pragma once

#include "flight/pid.h"
#include "sensors/barometer.h"
#include "io/rc_controls.h"
#include "io/escservo.h"

// Undefine this to use CF's native magHold PID and MAG mode to control heading, if defined, NAV will control YAW by itself
#define NAV_HEADING_CONTROL_PID

#define DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR 1.113195f  // MagicEarthNumber from APM

#define LANDING_DETECTION_TIMEOUT       10000000     // 10 second timeout
#define RADX10                          0.00174532925f
#define RADX100                         0.000174532925f
#define CROSSTRACK_GAIN                 1
#define NAV_ROLL_PITCH_MAX              300 // Max control input from NAV (30 deg)
#define NAV_THROTTLE_CORRECTION_ANGLE   450

// Size of barometer derivative filter
#define NAV_BARO_CLIMB_RATE_FILTER_SIZE  7

//Update rate for position target update (minumum possible speed in cms will be this value)
#define POSITION_TARGET_UPDATE_RATE_HZ      5
#define ALTITUDE_UPDATE_FREQUENCY_HZ        10      // max 30hz update rate (almost maximum possible rate for BMP085)

#define MIN_ALTITUDE_UPDATE_FREQUENCY_HZ    5       // althold will not be applied if update rate is less than this constant
#define MIN_POSITION_UPDATE_FREQUENCY_HZ    5       // GPS navigation (PH/WP/RTH) won't be applied unless update rate is above this

#define NAV_VEL_ERROR_CUTOFF_FREQENCY_HZ    4       // low-pass filter on Z-velocity error
#define NAV_THROTTLE_CUTOFF_FREQENCY_HZ     2       // low-pass filter on throttle output
#define NAV_ACCEL_CUTOFF_FREQUENCY_HZ       2       // low-pass filter on XY-acceleration target

#define NAV_GRAVITY_CMSS                    980.665f
#define NAV_ACCELERATION_XY_MAX             980.0f  // cm/s/s
#define NAV_ACCEL_SLOW_XY_MAX               150.0f  // cm/s/s

#define HZ2US(hz)   (1000000 / (hz))
#define US2S(us)    ((us) * 1e-6f)

// Should apply position hold logic
#define navShouldApplyPosHold() ((posControl.mode & (NAV_MODE_POSHOLD_2D | NAV_MODE_POSHOLD_3D)) != 0)
// Should apply waypoint navigation logic (WP/RTH)
#define navShouldApplyWaypoint() ((posControl.mode & (NAV_MODE_WP)) != 0)
#define navShouldApplyRTH() ((posControl.mode & (NAV_MODE_RTH | NAV_MODE_RTH_2D)) != 0)
// Should apply altitude PID controller
#define navShouldApplyAltHold() ((posControl.mode & (NAV_MODE_ALTHOLD | NAV_MODE_POSHOLD_3D | NAV_MODE_WP | NAV_MODE_RTH)) != 0)
// Should apply RTH-specific logic
#define navShouldApplyRTHAltitudeLogic() (((posControl.mode & NAV_MODE_RTH) != 0) && (navRthState == NAV_RTH_STATE_HOME_AUTOLAND || navRthState == NAV_RTH_STATE_LANDED || navRthState == NAV_RTH_STATE_FINISHED))

// 
#define navShouldApplyHeadingControl() ((posControl.mode & (NAV_MODE_POSHOLD_2D | NAV_MODE_POSHOLD_3D  | NAV_MODE_WP | NAV_MODE_RTH | NAV_MODE_RTH_2D)) != 0)
#define navShouldKeepHeadingToBearing() (((posControl.mode & NAV_MODE_WP) != 0) || (((posControl.mode & (NAV_MODE_RTH | NAV_MODE_RTH_2D)) != 0) && (navRthState == NAV_RTH_STATE_HEAD_HOME)))

#define navCanAdjustAltitudeFromRCInput() ((posControl.mode & (NAV_MODE_ALTHOLD | NAV_MODE_POSHOLD_3D | NAV_MODE_RTH)) != 0)
#define navCanAdjustHorizontalVelocityAndAttitudeFromRCInput() ((posControl.mode & (NAV_MODE_POSHOLD_2D | NAV_MODE_POSHOLD_3D | NAV_MODE_RTH | NAV_MODE_RTH_2D)) != 0)
#define navCanAdjustHeadingFromRCInput() ((posControl.mode & (NAV_MODE_POSHOLD_2D | NAV_MODE_POSHOLD_3D | NAV_MODE_RTH | NAV_MODE_RTH_2D)) != 0)

typedef enum navRthState_e {
    NAV_RTH_STATE_INIT = 0,
    NAV_RTH_STATE_CLIMB_TO_SAVE_ALTITUDE,
    NAV_RTH_STATE_HEAD_HOME,
    NAV_RTH_STATE_HOME_AUTOLAND,
    NAV_RTH_STATE_LANDED,
    NAV_RTH_STATE_FINISHED,
} navRthState_t;

typedef enum {
    NAV_WP_NONE     = 0,
    NAV_WP_XY       = 1 << 0,
    NAV_WP_Z        = 1 << 1,
    NAV_WP_HEADING  = 1 << 2,
    NAV_WP_BEARING  = 1 << 3
} navSetWaypointFlags_t;

typedef struct navigationFlags_s {
    bool verticalPositionNewData;
    bool horizontalPositionNewData;
    bool headingNewData;
} navigationFlags_t;

typedef struct {
    float kP;
    float kI;
    float kD;
    float Imax;
} pidControllerParam_t;

typedef struct {
    float kP;
} pControllerParam_t;

typedef struct {
    pidControllerParam_t param;
    float integrator;       // integrator value
    float last_error;       // last input for derivative
    float pterm_filter_state;
    float dterm_filter_state;  // last derivative for low-pass filter

#if defined(NAV_BLACKBOX)
    float lastP, lastI, lastD;
#endif
} pidController_t;

typedef struct {
    pControllerParam_t param;
    float pterm_filter_state;
#if defined(NAV_BLACKBOX)
    float lastP;
#endif
} pController_t;

typedef struct navigationPIDControllers_s {
    pController_t   pos[XYZ_AXIS_COUNT];
    pidController_t vel[XYZ_AXIS_COUNT];
    pidController_t accz;
#if defined(NAV_HEADING_CONTROL_PID)
    pController_t   heading;
#endif
} navigationPIDControllers_t;

typedef struct {
    navPosition3D_t pos;
    float           vel[XYZ_AXIS_COUNT];
} navigationEstimatedState_t;

typedef struct {
    navPosition3D_t pos;
    float           vel[XYZ_AXIS_COUNT];
    float           acc[XYZ_AXIS_COUNT];
    int32_t         heading;
} navigationDesiredState_t;

typedef struct {
    bool                enabled;
    navigationMode_t    mode;
    navigationFlags_t   flags;

    navigationPIDControllers_t  pids;

    navigationEstimatedState_t  actualState;
    navigationDesiredState_t    desiredState;   // waypoint coordinates + velocity
} navigationPosControl_t;
