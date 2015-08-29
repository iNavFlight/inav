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

#define DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR    1.113195f  // MagicEarthNumber from APM
#define NAV_GRAVITY_CMSS                                    980.665f

#define LAND_DETECTOR_TRIGGER_TIME      2000000

#define RADX10                          0.00174532925f
#define RADX100                         0.000174532925f
#define NAV_ROLL_PITCH_MAX              300 // Max control input from NAV (30 deg)

// Size of barometer derivative filter
#define NAV_BARO_CLIMB_RATE_FILTER_SIZE  7

//Update rate for position target update (minumum possible speed in cms will be this value)
#define POSITION_TARGET_UPDATE_RATE_HZ      5
#define ALTITUDE_UPDATE_FREQUENCY_HZ        10      // max 30hz update rate (almost maximum possible rate for BMP085)

#define MIN_SONAR_UPDATE_FREQUENCY_HZ       3       // Sonar velocity won't be calculated if readings updated slower than this
#define MIN_ALTITUDE_UPDATE_FREQUENCY_HZ    5       // althold will not be applied if update rate is less than this constant
#define MIN_POSITION_UPDATE_FREQUENCY_HZ    5       // GPS navigation (PH/WP/RTH) won't be applied unless update rate is above this

#define NAV_VEL_ERROR_CUTOFF_FREQENCY_HZ    4       // low-pass filter on Z-velocity error
#define NAV_THROTTLE_CUTOFF_FREQENCY_HZ     2       // low-pass filter on throttle output
#define NAV_ACCEL_CUTOFF_FREQUENCY_HZ       2       // low-pass filter on XY-acceleration target

#define NAV_ACCELERATION_XY_MAX             980.0f  // cm/s/s       // approx 45 deg lean angle
#define NAV_ACCEL_SLOW_XY_MAX               550.0f  // cm/s/s       // approx 29 deg lean angle

#define HZ2US(hz)   (1000000 / (hz))
#define US2S(us)    ((us) * 1e-6f)

// FIXME: Make this configurable, default to about 5% highet than minthrottle
#define minFlyableThrottle  (masterConfig.escAndServoConfig.minthrottle + (masterConfig.escAndServoConfig.maxthrottle - masterConfig.escAndServoConfig.minthrottle) * 5 / 100)

// Should apply position hold logic
#define navShouldApplyPosHold() ((posControl.mode & (NAV_MODE_POSHOLD_2D | NAV_MODE_POSHOLD_3D)) != 0)
// Should apply waypoint navigation logic (WP/RTH)
#define navShouldApplyWaypoint() ((posControl.mode & (NAV_MODE_WP)) != 0)
#define navShouldApplyRTH() ((posControl.mode & (NAV_MODE_RTH | NAV_MODE_RTH_2D)) != 0)
// Should apply altitude PID controller
#define navShouldApplyAltHold() ((posControl.mode & (NAV_MODE_ALTHOLD | NAV_MODE_POSHOLD_3D | NAV_MODE_WP | NAV_MODE_RTH)) != 0)
// Should apply RTH-specific logic
#define navShouldApplyRTHLandingLogic() (((posControl.mode & NAV_MODE_RTH) != 0) && (posControl.navRthState == NAV_RTH_STATE_HOME_AUTOLAND || posControl.navRthState == NAV_RTH_STATE_LANDED || posControl.navRthState == NAV_RTH_STATE_FINISHED))

// Should apply heading control logic
#define navShouldApplyHeadingControl() ((posControl.mode & (NAV_MODE_POSHOLD_2D | NAV_MODE_POSHOLD_3D  | NAV_MODE_WP | NAV_MODE_RTH | NAV_MODE_RTH_2D)) != 0)

// Should NAV continuously adjust heading towards destination
#define navShouldKeepHeadingToBearing() (((posControl.mode & NAV_MODE_WP) != 0) || (((posControl.mode & (NAV_MODE_RTH | NAV_MODE_RTH_2D)) != 0) && (posControl.navRthState == NAV_RTH_STATE_HEAD_HOME)))

// Define conditions when pilot can adjust NAV behaviour
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
    NAV_POS_UPDATE_NONE     = 0,
    NAV_POS_UPDATE_XY       = 1 << 0,
    NAV_POS_UPDATE_Z        = 1 << 1,
    NAV_POS_UPDATE_HEADING  = 1 << 2,
    NAV_POS_UPDATE_BEARING  = 1 << 3
} navSetWaypointFlags_t;

typedef struct navigationFlags_s {
    bool verticalPositionNewData;
    bool horizontalPositionNewData;
    bool headingNewData;
    bool hasValidAltitudeSensor;        // Indicates that we have a working altitude sensor (got at least one valid reading from it)
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
    pController_t   heading;
} navigationPIDControllers_t;

typedef struct {
    t_fp_vector pos;
    t_fp_vector vel;
    int32_t     yaw;
} navigationEstimatedState_t;

typedef struct {
    t_fp_vector pos;
    t_fp_vector vel;
    t_fp_vector acc;
    int32_t     yaw;
} navigationDesiredState_t;

typedef struct {
    /* Flags and navigation system state */
    bool                enabled;
    navigationMode_t    mode;
    navigationFlags_t   flags;

    /* Navigation PID controllers */
    navigationPIDControllers_t  pids;

    /* Local system state, both actual (estimated) and desired (target setpoint)*/
    navigationEstimatedState_t  actualState;
    navigationDesiredState_t    desiredState;   // waypoint coordinates + velocity

    /* INAV GPS origin (position where GPS fix was first acquired) */
    gpsOrigin_s                 gpsOrigin;

    /* Home parameters (NEU coordinated), geodetic position of home (LLH) is stores in GPS_home variable */
    navWaypointPosition_t       homeWaypoint;       // NEU-coordinates and original yaw (heading when launched)
    navWaypointPosition_t       homeWaypointAbove;  // NEU-coordinates and original yaw (heading when launched + desired RTH altitude)

    uint32_t                    homeDistance;   // cm
    int32_t                     homeDirection;  // deg*100

    /* Internals */
    int16_t                     rcAdjustment[4];

    navProfile_t *              navProfile;
    rcControlsConfig_t *        rcControlsConfig;
    navRthState_t               navRthState;
    pidProfile_t *              pidProfile;
    barometerConfig_t *         barometerConfig;
} navigationPosControl_t;

extern navigationPosControl_t posControl;

/* Internally used functions */
bool isThrustFacingDownwards(rollAndPitchInclination_t *inclination);
void updateAltitudeTargetFromClimbRate(uint32_t deltaMicros, float climbRate);
void calculateDistanceAndBearingToDestination(t_fp_vector * currentPos, t_fp_vector * destinationPos, uint32_t *dist, int32_t *bearing);
float navApplyFilter(float input, float fCut, float dT, float * state);
float navPidGetP(float error, float dt, pidController_t *pid);
float navPidGetI(float error, float dt, pidController_t *pid);
float navPidGetD(float error, float dt, pidController_t *pid);
float navPidGetPID(float error, float dt, pidController_t *pid);
void navPidReset(pidController_t *pid);
void navPidInit(pidController_t *pid, float _kP, float _kI, float _kD, float _Imax);
void navPInit(pController_t *p, float _kP);

void updateActualHorizontalPositionAndVelocity(float newX, float newY, float newVelX, float newVelY);
void updateActualAltitudeAndClimbRate(float newAltitude, float newVelocity);
void updateActualHeading(int32_t newHeading);

/* Multicopter-specific functions */
void setupMulticopterAltitudeController(void);
void resetMulticopterAltitudeController();
void applyMulticopterAltitudeController(uint32_t currentTime);

void resetMulticopterHeadingController(void);
void applyMulticopterHeadingController(uint32_t currentTime);

void resetMulticopterPositionController(void);
void applyMulticopterPositionController(uint32_t currentTime);

bool isMulticopterLandingDetected(uint32_t * landingTimer);

/* Fixed-wing specific functions */
void setupFixedWingAltitudeController(void);
void resetFixedWingAltitudeController();
void applyFixedWingAltitudeController(uint32_t currentTime);

void resetFixedWingHeadingController(void);
void applyFixedWingHeadingController(uint32_t currentTime);

void resetFixedWingPositionController(void);
void applyFixedWingPositionController(uint32_t currentTime);

bool isFixedWingLandingDetected(uint32_t * landingTimer);
