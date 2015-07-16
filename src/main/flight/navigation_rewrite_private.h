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
#define NAV_ROLL_PITCH_MAX              300 // Max control input from NAV
#define NAV_THROTTLE_CORRECTION_ANGLE   450

// Should apply position-to-velocity PID controller for POS_HOLD
#define navShouldApplyPosHold() ((navMode & (NAV_MODE_POSHOLD_2D | NAV_MODE_POSHOLD_3D)) != 0)
// Should apply position-to-velocity PID controller for waypoint navigation (WP/RTH)
#define navShouldApplyWaypoint() ((navMode & (NAV_MODE_WP | NAV_MODE_RTH)) != 0)
// Should apply altitude PID controller
#define navShouldApplyAltHold() ((navMode & (NAV_MODE_ALTHOLD | NAV_MODE_POSHOLD_3D | NAV_MODE_WP | NAV_MODE_RTH)) != 0)

// 
#define navShouldApplyHeadingControl() ((navMode & (NAV_MODE_WP | NAV_MODE_RTH | NAV_MODE_POSHOLD_2D | NAV_MODE_POSHOLD_3D)) != 0)
#define navShouldAdjustHeading() ((navMode & (NAV_MODE_WP | NAV_MODE_RTH)) != 0)

#define navCanAdjustVerticalVelocityFromRCInput() (((navMode & (NAV_MODE_ALTHOLD | NAV_MODE_POSHOLD_3D)) != 0) || ((navMode == NAV_MODE_RTH) && (navRthState == NAV_RTH_STATE_HEAD_HOME)))
#define navCanAdjustHorizontalVelocityAndAttitudeFromRCInput() ((navMode & (NAV_MODE_POSHOLD_2D | NAV_MODE_POSHOLD_3D | NAV_MODE_RTH)) != 0)
#define navCanAdjustHeadingFromRCInput() ((navMode & (NAV_MODE_POSHOLD_2D | NAV_MODE_POSHOLD_3D | NAV_MODE_RTH)) != 0)

typedef enum {
    NAV_RTH_STATE_INIT = 0,
    NAV_RTH_STATE_CLIMB_TO_SAVE_ALTITUDE,
    NAV_RTH_STATE_HEAD_HOME,
    NAV_RTH_STATE_HOME_AUTOLAND,
    NAV_RTH_STATE_LANDED,
    NAV_RTH_STATE_FINISHED,
} navRthState_t;

typedef struct {
    bool verticalPositionNewData;
    bool horizontalPositionNewData;
    bool headingNewData;
} navigationFlags_s;

typedef struct {
    float kP;
    float kI;
    float kD;
    float Imax;
} PID_PARAM;

typedef struct {
    PID_PARAM param;
    float integrator;       // integrator value
    float last_error;       // last input for derivative
    float pterm_filter_state;
    float dterm_filter_state;  // last derivative for low-pass filter

#if defined(NAV_BLACKBOX)
    float lastP, lastI, lastD;
#endif
} PID;

typedef struct {
    bool available;
    float variance;
    int32_t value;
} navCLTAxisPos_s;

typedef struct {
    bool available;
    float variance;
    float value;
} navCLTAxisVel_s;

typedef struct {
    struct {
        // For GPS we are doing only altitude
        navCLTAxisPos_s alt;
        navCLTAxisVel_s vel[XYZ_AXIS_COUNT];
    } gps;

    struct {
        navCLTAxisPos_s alt;
        navCLTAxisVel_s vel;
    } baro;

    struct {
        navCLTAxisPos_s alt;
        navCLTAxisVel_s vel;
    } sonar;

    struct {
        navCLTAxisVel_s vel[XYZ_AXIS_COUNT];
    } imu;

    struct {
        navCLTAxisPos_s alt;
        navCLTAxisVel_s vel[XYZ_AXIS_COUNT];
    } estimated;
} navCLTState_s;
