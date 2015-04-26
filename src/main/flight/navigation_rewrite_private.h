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

#define LAND_DETECTOR_TRIGGER_TIME_MS       2000
#define POSITION_SENSOR_TIMEOUT_MS          500    // If GPS updates do not happen for this much time, consider position sensor lost

#define RADX100                             0.000174532925f
#define NAV_ROLL_PITCH_MAX                  (30 * 100) // Max control input from NAV (30 deg)
#define NAV_ROLL_PITCH_MAX_FW               (15 * 100) // Max control input from NAV-FW (15 deg)

// Size of barometer derivative filter
#define NAV_BARO_CLIMB_RATE_FILTER_SIZE     7

#define POSITION_TARGET_UPDATE_RATE_HZ      5       // Rate manual position target update (minumum possible speed in cms will be this value)
#define ALTITUDE_UPDATE_RATE_HZ             10      // Rate at which altitude sensors would be read and updated

#define MIN_SONAR_UPDATE_RATE_HZ            3       // Sonar velocity won't be calculated if readings updated slower than this
#define MIN_ALTITUDE_UPDATE_RATE_HZ         5       // Althold will not be applied if update rate is less than this constant
#define MIN_POSITION_UPDATE_FREQUENCY_HZ    2       // GPS navigation (PH/WP/RTH) won't be applied unless update rate is above this

#define NAV_VEL_ERROR_CUTOFF_FREQENCY_HZ    4       // low-pass filter on Z-velocity error
#define NAV_THROTTLE_CUTOFF_FREQENCY_HZ     2       // low-pass filter on throttle output
#define NAV_ACCEL_CUTOFF_FREQUENCY_HZ       2       // low-pass filter on XY-acceleration target

#define NAV_FW_VEL_CUTOFF_FREQENCY_HZ       1       // low-pass filter on Z-velocity for fixed wing

#define NAV_ACCELERATION_XY_MAX             980.0f  // cm/s/s       // approx 45 deg lean angle
#define NAV_ACCEL_SLOW_XY_MAX               550.0f  // cm/s/s       // approx 29 deg lean angle

#define HZ2US(hz)   (1000000 / (hz))
#define US2S(us)    ((us) * 1e-6f)
#define MS2US(ms)   ((ms) * 1000)

// FIXME: Make this configurable, default to about 5% highet than minthrottle
#define minFlyableThrottle  (masterConfig.escAndServoConfig.minthrottle + (masterConfig.escAndServoConfig.maxthrottle - masterConfig.escAndServoConfig.minthrottle) * 5 / 100)

#define IS_NAV_MODE_ALTHOLD     (posControl.mode == NAV_MODE_ALTHOLD)
#define IS_NAV_MODE_POSHOLD_2D  (posControl.mode == NAV_MODE_POSHOLD_2D)
#define IS_NAV_MODE_POSHOLD_3D  (posControl.mode == NAV_MODE_POSHOLD_3D)
#define IS_NAV_MODE_WP          (posControl.mode == NAV_MODE_WP)
#define IS_NAV_MODE_RTH         (posControl.mode == NAV_MODE_RTH)
#define IS_NAV_MODE_RTH_2D      (posControl.mode == NAV_MODE_RTH_2D)

#define IS_NAV_AUTO_AUTOLAND    (posControl.navMissionState == NAV_AUTO_AUTOLAND)
#define IS_NAV_AUTO_LANDED      (posControl.navMissionState == NAV_AUTO_LANDED)
#define IS_NAV_AUTO_FINISHED    (posControl.navMissionState == NAV_AUTO_FINISHED)
#define IS_NAV_EMERG_LANDING    (posControl.navMissionState == NAV_AUTO_EMERGENCY_LANDING)
#define IS_RTH_HEAD_HOME        (posControl.navMissionState == NAV_AUTO_RTH_HEAD_HOME)

// Should apply position hold logic
#define navShouldApplyPosHold()  (IS_NAV_MODE_POSHOLD_2D || IS_NAV_MODE_POSHOLD_3D)
// Should apply waypoint navigation logic (WP/RTH)
#define navShouldApplyWaypoint() (IS_NAV_MODE_WP)
#define navShouldApplyRTH()      (IS_NAV_MODE_RTH || IS_NAV_MODE_RTH_2D)
// Should apply altitude PID controller
#define navShouldApplyAltHold()  (IS_NAV_MODE_ALTHOLD || IS_NAV_MODE_POSHOLD_2D || IS_NAV_MODE_POSHOLD_3D || IS_NAV_MODE_WP || IS_NAV_MODE_RTH )
// Should apply RTH-specific logic
#define navShouldApplyAutonomousLandingLogic()  ((IS_NAV_MODE_RTH || IS_NAV_MODE_WP) && \
                                                (IS_NAV_AUTO_AUTOLAND || IS_NAV_AUTO_LANDED || IS_NAV_AUTO_FINISHED))
// Should NAV apply emergency landing sequence
#define navShouldApplyEmergencyLanding() ((IS_NAV_MODE_RTH || IS_NAV_MODE_RTH_2D || IS_NAV_MODE_WP) && IS_NAV_EMERG_LANDING)

// Should apply heading control logic
#define navShouldApplyHeadingControl() (IS_NAV_MODE_POSHOLD_2D || IS_NAV_MODE_POSHOLD_3D || IS_NAV_MODE_WP || IS_NAV_MODE_RTH || IS_NAV_MODE_RTH_2D)

// Should NAV continuously adjust heading towards destination
#define navShouldKeepHeadingToBearing() (IS_NAV_MODE_WP || ((IS_NAV_MODE_RTH || IS_NAV_MODE_RTH_2D) && IS_RTH_HEAD_HOME))

// Define conditions when pilot can adjust NAV behaviour
#define navCanAdjustAltitudeFromRCInput() ((IS_NAV_MODE_ALTHOLD || IS_NAV_MODE_POSHOLD_3D) || \
                                           (IS_NAV_MODE_RTH && IS_RTH_HEAD_HOME))
#define navCanAdjustHorizontalVelocityAndAttitudeFromRCInput() (IS_NAV_MODE_POSHOLD_2D || IS_NAV_MODE_POSHOLD_3D || IS_NAV_MODE_RTH || IS_NAV_MODE_RTH_2D)
#define navCanAdjustHeadingFromRCInput() (IS_NAV_MODE_POSHOLD_2D || IS_NAV_MODE_POSHOLD_3D)

typedef enum navAutonomousMissionState_e {
    // RTH sequence
    NAV_AUTO_RTH_INIT = 0,
    NAV_AUTO_RTH_CLIMB_TO_SAVE_ALTITUDE,
    NAV_AUTO_RTH_HEAD_HOME,
    // Waypoint mission mode
    NAV_AUTO_WP_INIT,
    NAV_AUTO_WP,
    // Autoland sequence
    NAV_AUTO_AUTOLAND_INIT,
    NAV_AUTO_AUTOLAND,
    NAV_AUTO_EMERGENCY_LANDING_INIT,
    NAV_AUTO_EMERGENCY_LANDING,
    NAV_AUTO_LANDED,
    NAV_AUTO_FINISHED,
} navAutonomousMissionState_t;

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
    bool hasValidPositionSensor;        // Indicates that GPS is working (or not)
    bool isAdjustingPosition;
    bool isAdjustingAltitude;
    bool isAdjustingHeading;
    bool forcedRTHActivated;
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
    navWaypointPosition_t       homePosition;       // Special waypoint, stores original yaw (heading when launched)
    navWaypointPosition_t       homeWaypointAbove;  // NEU-coordinates and initial bearing + desired RTH altitude

    uint32_t                    homeDistance;   // cm
    int32_t                     homeDirection;  // deg*100

    /* Waypoint list */
    navWaypointPosition_t       waypointList[NAV_MAX_WAYPOINTS];
    int8_t                      waypointCount;

    /* Internals */
    int16_t                     rcAdjustment[4];

    navConfig_t *               navConfig;
    rcControlsConfig_t *        rcControlsConfig;
    navAutonomousMissionState_t navMissionState;
    pidProfile_t *              pidProfile;
} navigationPosControl_t;

extern navigationPosControl_t posControl;

/* Internally used functions */
float navApplyFilter(float input, float fCut, float dT, float * state);
float navPidGetP(float error, float dt, pidController_t *pid);
float navPidGetI(float error, float dt, pidController_t *pid, bool onlyShrinkI);
float navPidGetD(float error, float dt, pidController_t *pid);
float navPidGetPID(float error, float dt, pidController_t *pid, bool onlyShrinkI);
void navPidReset(pidController_t *pid);
void navPidInit(pidController_t *pid, float _kP, float _kI, float _kD, float _Imax);
void navPInit(pController_t *p, float _kP);

bool isThrustFacingDownwards(rollAndPitchInclination_t *inclination);
void updateAltitudeTargetFromClimbRate(uint32_t deltaMicros, float climbRate);
uint32_t calculateDistanceToDestination(t_fp_vector * destinationPos);
int32_t calculateBearingToDestination(t_fp_vector * destinationPos);
void resetLandingDetector(void);
bool isLandingDetected(void);
void setHomePosition(t_fp_vector * pos, int32_t yaw);
void setDesiredPosition(t_fp_vector * pos, int32_t yaw, navSetWaypointFlags_t useMask);
bool isWaypointReached(navWaypointPosition_t *waypoint);

void updateActualHorizontalPositionAndVelocity(float newX, float newY, float newVelX, float newVelY);
void updateActualAltitudeAndClimbRate(float newAltitude, float newVelocity);
void updateActualHeading(int32_t newHeading);

/* Autonomous navigation functions */
void setupAutonomousControllerRTH(void);
void resetAutonomousControllerForWP(void);
void resetAutonomousControllerForRTH(void);
void applyAutonomousController(void);

/* Multicopter-specific functions */
void setupMulticopterAltitudeController(void);
void resetMulticopterAltitudeController();
void applyMulticopterAltitudeController(uint32_t currentTime);
void resetMulticopterHeadingController(void);
void applyMulticopterHeadingController(void);
void resetMulticopterPositionController(void);
void applyMulticopterPositionController(uint32_t currentTime);
void applyMulticopterEmergencyLandingController(void);
bool isMulticopterLandingDetected(uint32_t * landingTimer);

/* Fixed-wing specific functions */
void setupFixedWingAltitudeController(void);
void resetFixedWingAltitudeController();
void applyFixedWingAltitudeController(uint32_t currentTime);
void resetFixedWingHeadingController(void);
void applyFixedWingHeadingController(void);
void resetFixedWingPositionController(void);
void applyFixedWingPositionController(uint32_t currentTime);
void applyFixedWingEmergencyLandingController(void);
bool isFixedWingLandingDetected(uint32_t * landingTimer);
