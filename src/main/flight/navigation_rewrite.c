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

#include "build_config.h"
#include "platform.h"
#include "debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/filter.h"

#include "drivers/system.h"
#include "drivers/sensor.h"
#include "drivers/accgyro.h"

#include "sensors/sensors.h"
#include "sensors/acceleration.h"
#include "sensors/boardalignment.h"

#include "flight/pid.h"
#include "flight/imu.h"
#include "flight/navigation_rewrite.h"
#include "flight/navigation_rewrite_private.h"

#include "config/runtime_config.h"
#include "config/config.h"


#if defined(NAV)

navigationPosControl_t   posControl;

#if defined(NAV_BLACKBOX)
int16_t navCurrentMode;
int16_t navActualVelocity[3];
int16_t navDesiredVelocity[3];
int16_t navActualHeading;
int16_t navDesiredHeading;
int16_t navTargetPosition[3];
int32_t navLatestActualPosition[3];
int16_t navDebug[4];
uint16_t navFlags;
#endif

static navigationMode_t selectNavModeFromBoxModeInput(void);

bool updateTimer(navigationTimer_t * tim, uint32_t interval, uint32_t currentTime)
{
    if ((currentTime - tim->lastTriggeredTime) >= interval) {
        tim->deltaTime = currentTime - tim->lastTriggeredTime;
        tim->lastTriggeredTime = currentTime;
        return true;
    }
    else {
        return false;
    }
}

/*-----------------------------------------------------------
 * Float point PID-controller implementation
 *-----------------------------------------------------------*/
// Implementation of PID with back-calculation I-term anti-windup
// Control System Design, Lecture Notes for ME 155A by Karl Johan Åström (p.228)
// http://www.cds.caltech.edu/~murray/courses/cds101/fa02/caltech/astrom-ch6.pdf
float navPidApply2(float setpoint, float measurement, float dt, pidController_t *pid, float outMin, float outMax, bool dTermErrorTracking)
{
    float newProportional, newDerivative;
    float error = setpoint - measurement;

    /* P-term */
    newProportional = error * pid->param.kP;

    /* D-term */
    if (dTermErrorTracking) {
        /* Error-tracking D-term */
        newDerivative = (error - pid->last_input) / dt;
        pid->last_input = error;
    }
    else {
        /* Measurement tracking D-term */
        newDerivative = -(measurement - pid->last_input) / dt;
        pid->last_input = measurement;
    }

    if (posControl.navConfig->dterm_cut_hz)
        newDerivative = pid->param.kD * filterApplyPt1(newDerivative, &pid->dterm_filter_state, posControl.navConfig->dterm_cut_hz, dt);
    else
        newDerivative = pid->param.kD * newDerivative;

    /* Pre-calculate output and limit it if actuator is saturating */
    float outVal = newProportional + pid->integrator + newDerivative;
    float outValConstrained = constrainf(outVal, outMin, outMax);

    /* Update I-term */
    pid->integrator += (error * pid->param.kI * dt) + ((outValConstrained - outVal) * pid->param.kT * dt);

    return outValConstrained;
}

void navPidReset(pidController_t *pid)
{
    pid->integrator = 0.0f;
    pid->last_input = 0.0f;
    pid->dterm_filter_state.state = 0.0f;
    pid->dterm_filter_state.RC = 0.0f;
}

void navPidInit(pidController_t *pid, float _kP, float _kI, float _kD)
{
    pid->param.kP = _kP;
    pid->param.kI = _kI;
    pid->param.kD = _kD;

    if (_kI > 1e-6f && _kP > 1e-6f) {
        float Ti = _kP / _kI;
        float Td = _kD / _kP;
        pid->param.kT = 2.0f / (Ti + Td);
    }
    else {
        pid->param.kI = 0.0;
        pid->param.kT = 0.0;
    }

    navPidReset(pid);
}

/*-----------------------------------------------------------
 * Float point P-controller implementation
 *-----------------------------------------------------------*/
void navPInit(pController_t *p, float _kP)
{
    p->param.kP = _kP;
}

/*-----------------------------------------------------------
 * Detects if thrust vector is facing downwards
 *-----------------------------------------------------------*/
bool isThrustFacingDownwards(void)
{
    // Tilt angle <= 80 deg; cos(80) = 0.17364817766693034885171662676931
    return (calculateCosTiltAngle() >= 0.173648178f);
}

/*-----------------------------------------------------------
 * Processes an update to XY-position and velocity
 *-----------------------------------------------------------*/
void updateActualHorizontalPositionAndVelocity(bool hasValidSensor, float newX, float newY, float newVelX, float newVelY)
{
    posControl.actualState.pos.V.X = newX;
    posControl.actualState.pos.V.Y = newY;

    posControl.actualState.vel.V.X = newVelX;
    posControl.actualState.vel.V.Y = newVelY;

    posControl.flags.hasValidPositionSensor = hasValidSensor;

    if (hasValidSensor) {
        posControl.flags.horizontalPositionNewData = 1;
    }
    else {
        posControl.flags.horizontalPositionNewData = 0;
    }

#if defined(NAV_BLACKBOX)
    navLatestActualPosition[X] = newX;
    navLatestActualPosition[Y] = newY;
    navActualVelocity[X] = constrain(newVelX, -32678, 32767);
    navActualVelocity[Y] = constrain(newVelY, -32678, 32767);
#endif
}

/*-----------------------------------------------------------
 * Processes an update to Z-position and velocity
 *-----------------------------------------------------------*/
void updateActualAltitudeAndClimbRate(bool hasValidSensor, float newAltitude, float newVelocity)
{
    posControl.actualState.pos.V.Z = newAltitude;
    posControl.actualState.vel.V.Z = newVelocity;

    posControl.flags.hasValidAltitudeSensor = hasValidSensor;

    // Update altitude that would be used when executing RTH
    if (hasValidSensor) {
        setupAutonomousControllerRTH();
        posControl.flags.verticalPositionNewData = 1;
    }
    else {
        posControl.flags.verticalPositionNewData = 0;
    }

#if defined(NAV_BLACKBOX)
    navLatestActualPosition[Z] = constrain(newAltitude, -32678, 32767);
    navActualVelocity[Z] = constrain(newVelocity, -32678, 32767);
#endif
}

/*-----------------------------------------------------------
 * Processes an update to surface distance
 *-----------------------------------------------------------*/
void updateActualSurfaceDistance(bool hasValidSensor, float surfaceDistance)
{
    posControl.actualState.surface = surfaceDistance;
    posControl.flags.hasValidSurfaceSensor = hasValidSensor;

    if (hasValidSensor) {
        posControl.flags.surfaceDistanceNewData = 1;
    }
    else {
        posControl.flags.surfaceDistanceNewData = 0;
    }
}

/*-----------------------------------------------------------
 * Processes an update to estimated heading
 *-----------------------------------------------------------*/
void updateActualHeading(int32_t newHeading)
{
    /* Update heading */
    posControl.actualState.yaw = newHeading;

    /* Precompute sin/cos of yaw angle */
    posControl.actualState.sinYaw = sin_approx(CENTIDEGREES_TO_RADIANS(newHeading));
    posControl.actualState.cosYaw = cos_approx(CENTIDEGREES_TO_RADIANS(newHeading));

    posControl.flags.headingNewData = 1;
}

/*-----------------------------------------------------------
 * Calculates distance and bearing to destination point
 *-----------------------------------------------------------*/
uint32_t calculateDistanceToDestination(t_fp_vector * destinationPos)
{
    float deltaX = destinationPos->V.X - posControl.actualState.pos.V.X;
    float deltaY = destinationPos->V.Y - posControl.actualState.pos.V.Y;

    return sqrtf(sq(deltaX) + sq(deltaY));
}

int32_t calculateBearingToDestination(t_fp_vector * destinationPos)
{
    float deltaX = destinationPos->V.X - posControl.actualState.pos.V.X;
    float deltaY = destinationPos->V.Y - posControl.actualState.pos.V.Y;

    return wrap_36000(RADIANS_TO_CENTIDEGREES(atan2_approx(deltaY, deltaX)));
}

/*-----------------------------------------------------------
 * Check if waypoint is/was reached
 *-----------------------------------------------------------*/
bool isWaypointMissed(navWaypointPosition_t * waypoint)
{
    // We only can miss not home waypoint
    if (waypoint->flags.isHomeWaypoint) {
        return false;
    }
    else {
        int32_t bearingError = calculateBearingToDestination(&waypoint->pos) - waypoint->yaw;
        bearingError = wrap_18000(bearingError);

        return ABS(bearingError) > 10000; // TRUE if we passed the waypoint by 100 degrees
    }
}

bool isWaypointReached(navWaypointPosition_t * waypoint)
{
    // We consider waypoint reached if within specified radius
    uint32_t wpDistance = calculateDistanceToDestination(&waypoint->pos);
    return (wpDistance <= posControl.navConfig->waypoint_radius);
}

/*-----------------------------------------------------------
 * Compatibility for home position
 *-----------------------------------------------------------*/
gpsLocation_t GPS_home;
uint16_t      GPS_distanceToHome;        // distance to home point in meters
int16_t       GPS_directionToHome;       // direction to home point in degrees

static void updateHomePositionCompatibility(void)
{
    geoConvertLocalToGeodetic(&posControl.gpsOrigin, &posControl.homePosition.pos, &GPS_home);
    GPS_distanceToHome = posControl.homeDistance / 100;
    GPS_directionToHome = posControl.homeDirection / 100;
}

/*-----------------------------------------------------------
 * Reset home position to current position
 *-----------------------------------------------------------*/
void setHomePosition(t_fp_vector * pos, int32_t yaw)
{
    posControl.homePosition.flags.isHomeWaypoint = true;
    posControl.homePosition.pos = *pos;
    posControl.homePosition.yaw = yaw;

    posControl.homeDistance = 0;
    posControl.homeDirection = 0;

    // Update target RTH altitude as a waypoint above home
    posControl.homeWaypointAbove = posControl.homePosition;
    setupAutonomousControllerRTH();

    updateHomePositionCompatibility();
    ENABLE_STATE(GPS_FIX_HOME);
}

/*-----------------------------------------------------------
 * Update home position, calculate distance and bearing to home
 *-----------------------------------------------------------*/
void updateHomePosition(void)
{
    // Disarmed and have a valid position, constantly update home
    if (!ARMING_FLAG(ARMED)) {
        if (posControl.flags.hasValidPositionSensor) {
            setHomePosition(&posControl.actualState.pos, posControl.actualState.yaw);
        }
        else {
            DISABLE_STATE(GPS_FIX_HOME);
        }
    }
    else {
        // Update distance and direction to home if armed (home is not updated when armed)
        if (STATE(GPS_FIX_HOME)) {
            posControl.homeDistance = calculateDistanceToDestination(&posControl.homePosition.pos);
            posControl.homeDirection = calculateBearingToDestination(&posControl.homePosition.pos);
            updateHomePositionCompatibility();
        }
    }
}

/*-----------------------------------------------------------
 * Set surface tracking target
 *-----------------------------------------------------------*/
void setDesiredSurfaceOffset(float surfaceOffset)
{
    if (surfaceOffset > 0) {
        posControl.desiredState.surface = constrainf(surfaceOffset, 10.0f, 250.0f);
    }
    else {
        posControl.desiredState.surface = -1;
    }
}

/*-----------------------------------------------------------
 * Set active XYZ-target and desired heading
 *-----------------------------------------------------------*/
void setDesiredPosition(t_fp_vector * pos, int32_t yaw, navSetWaypointFlags_t useMask)
{
    // XY-position
    if ((useMask & NAV_POS_UPDATE_XY) != 0) {
        posControl.desiredState.pos.V.X = pos->V.X;
        posControl.desiredState.pos.V.Y = pos->V.Y;
    }

    // Z-position
    if ((useMask & NAV_POS_UPDATE_Z) != 0) {
        posControl.desiredState.surface = -1;           // When we directly set altitude target we must reset surface tracking
        posControl.desiredState.pos.V.Z = pos->V.Z;
    }

    // Heading
    if ((useMask & NAV_POS_UPDATE_HEADING) != 0) {
        // Heading
        posControl.desiredState.yaw = yaw;
    }
    else if ((useMask & NAV_POS_UPDATE_BEARING) != 0) {
        posControl.desiredState.yaw = calculateBearingToDestination(pos);
    }
}

void setDesiredPositionToFarAwayTarget(int32_t yaw, int32_t distance, navSetWaypointFlags_t useMask)
{
    t_fp_vector farAwayPos;

    farAwayPos.V.X = posControl.actualState.pos.V.X + distance * cos_approx(CENTIDEGREES_TO_RADIANS(yaw));
    farAwayPos.V.Y = posControl.actualState.pos.V.Y + distance * sin_approx(CENTIDEGREES_TO_RADIANS(yaw));
    farAwayPos.V.Z = posControl.actualState.pos.V.Z;

    setDesiredPosition(&farAwayPos, yaw, useMask);
}

/*-----------------------------------------------------------
 * NAV land detector
 *-----------------------------------------------------------*/
static uint32_t landingTimer;

void resetLandingDetector(void)
{
    landingTimer = micros();
}

bool isLandingDetected(void)
{
    if (STATE(FIXED_WING)) { // FIXED_WING
        return isFixedWingLandingDetected(&landingTimer);
    }
    else {
        return isMulticopterLandingDetected(&landingTimer);
    }
}

/*-----------------------------------------------------------
 * Z-position controller
 *-----------------------------------------------------------*/
void updateAltitudeTargetFromClimbRate(float climbRate)
{
    // FIXME: On FIXED_WING and multicopter this should work in a different way
    // Calculate new altitude target

    /* Move surface tracking setpoint if it is set */
    if (posControl.desiredState.surface > 0.0f && posControl.actualState.surface > 0.0f && posControl.flags.hasValidSurfaceSensor) {
        posControl.desiredState.surface = constrainf(posControl.actualState.surface + (climbRate / posControl.pids.pos[Z].param.kP), 10.0f, 200.0f);
    }

    posControl.desiredState.pos.V.Z = posControl.actualState.pos.V.Z + (climbRate / posControl.pids.pos[Z].param.kP);
}

static void resetAltitudeController()
{
    if (STATE(FIXED_WING)) {
        resetFixedWingAltitudeController();
    }
    else {
        resetMulticopterAltitudeController();
    }
}

static void setupAltitudeController(void)
{
    if (STATE(FIXED_WING)) {
        setupFixedWingAltitudeController();
    }
    else {
        setupMulticopterAltitudeController();
    }
}

static void applyAltitudeController(uint32_t currentTime)
{
    if (STATE(FIXED_WING)) {
        applyFixedWingAltitudeController(currentTime);
    }
    else {
        applyMulticopterAltitudeController(currentTime);
    }
}

/*-----------------------------------------------------------
 * Heading controller
 *-----------------------------------------------------------*/
static void resetHeadingController()
{
    if (STATE(FIXED_WING)) {
        resetFixedWingHeadingController();
    }
    else {
        resetMulticopterHeadingController();
    }
}

static void applyHeadingController(void)
{
    if (STATE(FIXED_WING)) {
        applyFixedWingHeadingController();
    }
    else {
        applyMulticopterHeadingController();
    }
}

/*-----------------------------------------------------------
 * XY Position controller
 *-----------------------------------------------------------*/
static void resetPositionController()
{
    if (STATE(FIXED_WING)) {
        resetFixedWingPositionController();
    }
    else {
        resetMulticopterPositionController();
    }
}

static void applyPositionController(uint32_t currentTime)
{
    if (STATE(FIXED_WING)) {
        applyFixedWingPositionController(currentTime);
    }
    else {
        applyMulticopterPositionController(currentTime);
    }
}

/*-----------------------------------------------------------
 * WP controller
 *-----------------------------------------------------------*/
static void applyEmergencyLandingController(void)
{
    if (STATE(FIXED_WING)) {
        applyFixedWingEmergencyLandingController();
    }
    else {
        applyMulticopterEmergencyLandingController();
    }
}

/*-----------------------------------------------------------
 * WP controller
 *-----------------------------------------------------------*/
void getWaypoint(uint8_t wpNumber, int32_t * wpLat, int32_t * wpLon, int32_t * wpAlt)
{
    gpsLocation_t wpLLH;

    wpLLH.lat = 0.0f;
    wpLLH.lon = 0.0f;
    wpLLH.alt = 0.0f;

    // WP #0 - special waypoint - HOME
    if (wpNumber == 0) {
        if (STATE(GPS_FIX_HOME)) {
            wpLLH = GPS_home;
        }
    }
    // WP #255 - special waypoint - directly get actualPosition
    else if (wpNumber == 255) {
        geoConvertLocalToGeodetic(&posControl.gpsOrigin, &posControl.actualState.pos, &wpLLH);
    }
    // WP #1 - #15 - common waypoints - pre-programmed mission
    else if ((wpNumber >= 1) && (wpNumber <= NAV_MAX_WAYPOINTS)) {
        if (wpNumber <= posControl.waypointCount) {
            geoConvertLocalToGeodetic(&posControl.gpsOrigin, &posControl.waypointList[wpNumber - 1].pos, &wpLLH);
        }
    }

    *wpLat = wpLLH.lat;
    *wpLon = wpLLH.lon;
    *wpAlt = wpLLH.alt;
}

void setWaypoint(uint8_t wpNumber, int32_t wpLat, int32_t wpLon, int32_t wpAlt)
{
    gpsLocation_t wpLLH;
    navWaypointPosition_t wpPos;

    // Ignore mission updates if position estimator is not ready yet
    if (posControl.flags.hasValidPositionSensor)
        return;

    // Convert to local coordinates
    wpLLH.lat = wpLat;
    wpLLH.lon = wpLon;
    wpLLH.alt = wpAlt;
    geoConvertGeodeticToLocal(&posControl.gpsOrigin, &wpLLH, &wpPos.pos);
    wpPos.yaw = 0;  // FIXME

    // WP #0 - special waypoint - HOME
    if ((wpNumber == 0) && ARMING_FLAG(ARMED)) {
        // Forcibly set home position. Note that this is only valid if already armed, otherwise home will be reset instantly
        setHomePosition(&wpPos.pos, wpPos.yaw);
    }
    // WP #255 - special waypoint - directly set desiredPosition
    // Only valid when armed and in poshold mode
    else if ((wpNumber == 255) && ARMING_FLAG(ARMED) && navShouldApplyPosHold()) {
        // If close to actualPos, use heading, if far - use bearing
        uint32_t wpDistance = calculateDistanceToDestination(&wpPos.pos);
        navSetWaypointFlags_t waypointUpdateFlags = NAV_POS_UPDATE_XY;

        // If we received global altitude == 0, use current altitude
        if (wpAlt != 0) {
            waypointUpdateFlags |= NAV_POS_UPDATE_Z;
        }

        if (wpDistance <= posControl.navConfig->waypoint_radius) {
            waypointUpdateFlags |= NAV_POS_UPDATE_HEADING;
        }
        else {
            waypointUpdateFlags |= NAV_POS_UPDATE_BEARING;
        }

        setDesiredPosition(&wpPos.pos, posControl.actualState.yaw, waypointUpdateFlags);
    }
    // WP #1 - #15 - common waypoints - pre-programmed mission
    else if ((wpNumber >= 1) && (wpNumber <= NAV_MAX_WAYPOINTS)) {
        uint8_t wpIndex = wpNumber - 1;
        /* Sanity check - can set waypoints only sequentially - one by one */
        if (wpIndex <= posControl.waypointCount) {
            wpPos.flags.isHomeWaypoint = false;
            posControl.waypointList[wpIndex] = wpPos;
            posControl.waypointCount = wpIndex + 1;
        }
    }
}

/*-----------------------------------------------------------
 * A function to reset navigation PIDs and states
 *-----------------------------------------------------------*/
void resetNavigation(void)
{
    resetAltitudeController();
    resetHeadingController();
    resetPositionController();
}

/*-----------------------------------------------------------
 * A main function to call position controllers at loop rate
 *-----------------------------------------------------------*/
void applyWaypointNavigationAndAltitudeHold(void)
{
    uint32_t currentTime = micros();

#if defined(NAV_BLACKBOX)
    navFlags = 0;
    if (posControl.flags.verticalPositionNewData)   navFlags |= (1 << 0);
    if (posControl.flags.horizontalPositionNewData) navFlags |= (1 << 1);
    if (posControl.flags.headingNewData)            navFlags |= (1 << 2);
    if (posControl.flags.hasValidAltitudeSensor)    navFlags |= (1 << 3);
    if (posControl.flags.hasValidPositionSensor)    navFlags |= (1 << 7);
#endif

    // No navigation when disarmed
    if (!ARMING_FLAG(ARMED)) {
        posControl.enabled = false;
        return;
    }

    if (!posControl.enabled) {
        if (posControl.navConfig->flags.lock_nav_until_takeoff) {
            if (posControl.navConfig->flags.use_midrc_for_althold) {
                if (rcCommand[THROTTLE] > (posControl.rxConfig->midrc + posControl.navConfig->alt_hold_deadband)) {
                    resetNavigation();
                    posControl.enabled = true;
                }
            }
            else {
                if (rcCommand[THROTTLE] > minFlyableThrottle) {
                    resetNavigation();
                    posControl.enabled = true;
                }
            }
        }
        else {
            resetNavigation();
            posControl.enabled = true;
        }
    }

    // If throttle low don't apply navigation either
    if (!posControl.enabled) {
        // If lock_nav_until_takeoff & some NAV mode enabled, lock throttle to minimum, prevent accidental takeoff
        if ((selectNavModeFromBoxModeInput() != NAV_MODE_NONE) && posControl.navConfig->flags.lock_nav_until_takeoff) { // && posControl.navConfig->flags.use_midrc_for_althold
            rcCommand[THROTTLE] = posControl.escAndServoConfig->minthrottle;
        }
        return;
    }

    if (navShouldApplyEmergencyLanding()) {
        // Check for emergency landing sequence
        applyEmergencyLandingController();
    }
    else {
        // Apply navigation adjustments
        if (navShouldApplyAltHold() && isThrustFacingDownwards()) {
            applyAltitudeController(currentTime);
        }

        if (navShouldApplyPosHold() || navShouldApplyWaypoint() || navShouldApplyRTH()) {
            applyPositionController(currentTime);
        }

        if (navShouldApplyHeadingControl()) {
            applyHeadingController();
        }
    }

#if defined(NAV_BLACKBOX)
    if (posControl.flags.isAdjustingPosition)       navFlags |= (1 << 4);
    if (posControl.flags.isAdjustingAltitude)       navFlags |= (1 << 5);
    if (posControl.flags.isAdjustingHeading)        navFlags |= (1 << 6);
#endif
}

/*-----------------------------------------------------------
 * Set CF's FLIGHT_MODE from current NAV_MODE
 *-----------------------------------------------------------*/
void swithNavigationFlightModes(navigationMode_t navMode)
{
    switch(navMode) {
        case NAV_MODE_ALTHOLD:
            ENABLE_FLIGHT_MODE(NAV_ALTHOLD_MODE);
            DISABLE_FLIGHT_MODE(NAV_POSHOLD_MODE | NAV_RTH_MODE | NAV_WP_MODE);
            break;
        case NAV_MODE_POSHOLD_2D:
            ENABLE_FLIGHT_MODE(NAV_POSHOLD_MODE);
            DISABLE_FLIGHT_MODE(NAV_ALTHOLD_MODE | NAV_RTH_MODE | NAV_WP_MODE);
            break;
        case NAV_MODE_POSHOLD_3D:
            ENABLE_FLIGHT_MODE(NAV_ALTHOLD_MODE | NAV_POSHOLD_MODE);
            DISABLE_FLIGHT_MODE(NAV_RTH_MODE | NAV_WP_MODE);
            break;
        case NAV_MODE_WP:
            ENABLE_FLIGHT_MODE(NAV_WP_MODE);
            DISABLE_FLIGHT_MODE(NAV_ALTHOLD_MODE | NAV_POSHOLD_MODE | NAV_RTH_MODE);
            break;
        case NAV_MODE_RTH:
            ENABLE_FLIGHT_MODE(NAV_RTH_MODE);
            DISABLE_FLIGHT_MODE(NAV_ALTHOLD_MODE | NAV_POSHOLD_MODE | NAV_WP_MODE);
            break;
        case NAV_MODE_RTH_2D:
            ENABLE_FLIGHT_MODE(NAV_RTH_MODE);
            DISABLE_FLIGHT_MODE(NAV_ALTHOLD_MODE | NAV_POSHOLD_MODE | NAV_WP_MODE);
            break;
        case NAV_MODE_NONE:
        default:
            DISABLE_FLIGHT_MODE(NAV_ALTHOLD_MODE | NAV_POSHOLD_MODE | NAV_RTH_MODE | NAV_WP_MODE);
            break;
    }
}

/*-----------------------------------------------------------
 * desired NAV_MODE from combination of FLIGHT_MODE flags
 *-----------------------------------------------------------*/
static bool canActivateAltHoldMode(void)
{
    if (STATE(FIXED_WING)) {
        return posControl.flags.hasValidAltitudeSensor;
    }
    else {
        return posControl.flags.hasValidAltitudeSensor;
    }
}

static bool canActivatePosHoldMode(void)
{
    if (STATE(FIXED_WING)) {
        return false;
    }
    else {
        return posControl.flags.hasValidPositionSensor;
    }
}

static navigationMode_t selectNavModeFromBoxModeInput(void)
{
    // Flags if we can activate certain nav modes (check if we have required sensors and they provide valid data)
    bool canActivateAltHold = canActivateAltHoldMode();
    bool canActivatePosHold = canActivatePosHoldMode();

    /* Figure out, what mode pilot want to activate, also check if it is possible
       Once activated, GPS-aware modes should stay activated to cope with possible GPS loss.
       We don't want to have RTH activated from RATE mode being disabled and quad re-entering RATE mode. We'd rather stay in
       forced ANGLE mode and wait for GPS to re-connect or pilot to control the copter directly. */

    if (IS_RC_MODE_ACTIVE(BOXNAVRTH) || posControl.flags.forcedRTHActivated) {
        if (posControl.mode == NAV_MODE_RTH || posControl.mode == NAV_MODE_RTH_2D)
            return posControl.mode;

        if (canActivatePosHold && canActivateAltHold && STATE(GPS_FIX_HOME))
            return NAV_MODE_RTH;

        if (canActivatePosHold && STATE(GPS_FIX_HOME))
            return NAV_MODE_RTH_2D;
    }

    if (IS_RC_MODE_ACTIVE(BOXNAVWP)) {
/*
        if (posControl.mode == NAV_MODE_WP)
            return posControl.mode;

        if (canActivatePosHold && canActivateAltHold)
            return NAV_MODE_WP;
*/
    }

    if (IS_RC_MODE_ACTIVE(BOXNAVPOSHOLD) && IS_RC_MODE_ACTIVE(BOXNAVALTHOLD)) {
        if (posControl.mode == NAV_MODE_POSHOLD_3D)
            return posControl.mode;

        if (canActivatePosHold && canActivateAltHold)
            return NAV_MODE_POSHOLD_3D;
    }

    if (IS_RC_MODE_ACTIVE(BOXNAVPOSHOLD)) {
        if (posControl.mode == NAV_MODE_POSHOLD_2D)
            return posControl.mode;

        if (canActivatePosHold)
            return NAV_MODE_POSHOLD_2D;
    }

    if (IS_RC_MODE_ACTIVE(BOXNAVALTHOLD) && canActivateAltHold) {
        return NAV_MODE_ALTHOLD;
    }

    return NAV_MODE_NONE;
}

/*-----------------------------------------------------------
 * An indicator that throttle tilt compensation is forced
 *-----------------------------------------------------------*/
bool navigationRequiresThrottleTiltCompensation(void)
{
    return !STATE(FIXED_WING) && navShouldApplyAltHold() && posControl.navConfig->flags.throttle_tilt_comp;
}

/*-----------------------------------------------------------
 * An indicator that ANGLE mode must be forced per NAV requirement
 *-----------------------------------------------------------*/
bool naivationRequiresAngleMode(void)
{
    return navShouldApplyPosHold() || navShouldApplyWaypoint() || navShouldApplyRTH();
}

/**
 * An indicator that NAV is in charge of heading control (a signal to disable other heading controllers)
 */
bool naivationControlsHeadingNow(void)
{
    return navShouldApplyHeadingControl();
}

static void setInitialDesiredPositionForZController(void)
{
    // Check if previous mode was using ALTHOLD, re-use target altitude if necessary
    if (navShouldApplyAltHold() && !navShouldApplyAutonomousLandingLogic()) {
        // We were already applying ALTHOLD, don't update anything
        setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_NONE);
    }
    else {
        // Update desired Z-position to actual position
        resetAltitudeController();
        setupAltitudeController();
        setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);
    }

    /* If we have a valid surface offset and surface sensor is healthy - enter surface tracking mode */
    if (posControl.actualState.surface > 0 && posControl.flags.hasValidSurfaceSensor) {
        setDesiredSurfaceOffset(posControl.actualState.surface);
    }
    else {
        setDesiredSurfaceOffset(-1.0f);
    }
}

static void setInitialDesiredPositionForXYController(void)
{
    if (navShouldApplyPosHold() && !navShouldApplyRTH() && !navShouldApplyWaypoint()) {
        // XY-controller already active, update nothing
        setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_NONE);
    }
    else {
        // Update XY-controller and Heading controller target
        setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
    }
}

static void processNavigationModeSwitch(void)
{
    navigationMode_t newNavMode = NAV_MODE_NONE;

    if (posControl.enabled)
        newNavMode = selectNavModeFromBoxModeInput();

    // Process mode transition
    if (newNavMode != posControl.mode) {
        if (posControl.mode == NAV_MODE_NONE) {
            resetNavigation();
        }

        switch (newNavMode) {
            case NAV_MODE_ALTHOLD:
                setInitialDesiredPositionForZController();
                posControl.mode = NAV_MODE_ALTHOLD;
                break;
            case NAV_MODE_POSHOLD_2D:
                setInitialDesiredPositionForXYController();
                posControl.mode = NAV_MODE_POSHOLD_2D;
                break;
            case NAV_MODE_POSHOLD_3D:
                setInitialDesiredPositionForXYController();
                setInitialDesiredPositionForZController();
                posControl.mode = NAV_MODE_POSHOLD_3D;
                break;
            case NAV_MODE_WP:
                resetAutonomousControllerForWP();
                posControl.mode = newNavMode;
                break;
            case NAV_MODE_RTH:
            case NAV_MODE_RTH_2D:
                //setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING);
                resetAutonomousControllerForRTH();
                posControl.mode = newNavMode;
                break;
            default: // NAV_MODE_NONE
                resetNavigation();
                posControl.mode = newNavMode;
                break;
        }
    }
}

/**
 * Process NAV mode transition and WP/RTH state machine
 *  Update rate: RX (data driven or 50Hz)
 */
void updateWaypointsAndNavigationMode(void)
{
    // Process switch to a different navigation mode (if needed)
    processNavigationModeSwitch();

    // Initiate home position update
    updateHomePosition();

    // Map navMode back to enabled flight modes
    swithNavigationFlightModes(posControl.mode);

    // Apply RTH or WP sequence controller
    applyAutonomousController();

#if defined(NAV_BLACKBOX)
    navCurrentMode = (int16_t)posControl.mode;
#endif
}

/*-----------------------------------------------------------
 * NAV main control functions
 *-----------------------------------------------------------*/
void navigationUseConfig(navConfig_t *navConfigToUse)
{
    posControl.navConfig = navConfigToUse;
}

void navigationUseRcControlsConfig(rcControlsConfig_t *initialRcControlsConfig)
{
    posControl.rcControlsConfig = initialRcControlsConfig;
}

void navigationUseRxConfig(rxConfig_t * initialRxConfig)
{
    posControl.rxConfig = initialRxConfig;
}

void navigationUseEscAndServoConfig(escAndServoConfig_t * initialEscAndServoConfig)
{
    posControl.escAndServoConfig = initialEscAndServoConfig;
}

void navigationUsePIDs(pidProfile_t *initialPidProfile)
{
    int axis;

    posControl.pidProfile = initialPidProfile;

    // Initialize position hold PI-controller
    for (axis = 0; axis < 2; axis++) {
        navPInit(&posControl.pids.pos[axis], (float)posControl.pidProfile->P8[PIDPOS] / 100.0f);

        navPidInit(&posControl.pids.vel[axis], (float)posControl.pidProfile->P8[PIDPOSR] / 100.0f,
                                               (float)posControl.pidProfile->I8[PIDPOSR] / 100.0f,
                                               (float)posControl.pidProfile->D8[PIDPOSR] / 1000.0f);
    }

    // Initialize altitude hold PID-controllers (pos_z, vel_z, acc_z
    navPInit(&posControl.pids.pos[Z], (float)posControl.pidProfile->P8[PIDALT] / 100.0f);

    navPidInit(&posControl.pids.vel[Z], (float)posControl.pidProfile->P8[PIDVEL] / 100.0f,
                                        (float)posControl.pidProfile->I8[PIDVEL] / 100.0f,
                                        (float)posControl.pidProfile->D8[PIDVEL] / 1000.0f);

    // Initialize fixed wing PID controllers
    navPidInit(&posControl.pids.fw_nav, (float)posControl.pidProfile->P8[PIDNAVR] / 100.0f,
                                        (float)posControl.pidProfile->I8[PIDNAVR] / 100.0f,
                                        (float)posControl.pidProfile->D8[PIDNAVR] / 1000.0f);

    navPidInit(&posControl.pids.fw_alt, (float)posControl.pidProfile->P8[PIDALT] / 100.0f,
                                        (float)posControl.pidProfile->I8[PIDALT] / 100.0f,
                                        (float)posControl.pidProfile->D8[PIDALT] / 1000.0f);
}

void navigationInit(navConfig_t *initialnavConfig,
                    pidProfile_t *initialPidProfile,
                    rcControlsConfig_t *initialRcControlsConfig,
                    rxConfig_t * initialRxConfig,
                    escAndServoConfig_t * initialEscAndServoConfig)
{
    /* Initial state */
    posControl.enabled = 0;
    posControl.mode = NAV_MODE_NONE;
    posControl.flags.horizontalPositionNewData = 0;
    posControl.flags.verticalPositionNewData = 0;
    posControl.flags.surfaceDistanceNewData = 0;
    posControl.flags.headingNewData = 0;

    posControl.flags.hasValidAltitudeSensor = 0;
    posControl.flags.hasValidPositionSensor = 0;
    posControl.flags.hasValidSurfaceSensor = 0;

    posControl.flags.forcedRTHActivated = 0;
    posControl.waypointCount = 0;

    /* Use system config */
    navigationUseConfig(initialnavConfig);
    navigationUsePIDs(initialPidProfile);
    navigationUseRcControlsConfig(initialRcControlsConfig);
    navigationUseRxConfig(initialRxConfig);
    navigationUseEscAndServoConfig(initialEscAndServoConfig);
}

/*-----------------------------------------------------------
 * Access to estimated position/velocity data
 *-----------------------------------------------------------*/
float getEstimatedActualVelocity(int axis)
{
    return posControl.actualState.vel.A[axis];
}

float getEstimatedActualPosition(int axis)
{
    return posControl.actualState.pos.A[axis];
}

/*-----------------------------------------------------------
 * Interface with PIDs: Angle-Command transformation
 *-----------------------------------------------------------*/
int16_t rcCommandToLeanAngle(int16_t rcCommand)
{
    if (posControl.pidProfile->pidController == PID_CONTROLLER_LUX_FLOAT) {
        // LuxFloat is the only PID controller that uses raw rcCommand as target angle
        return rcCommand;
    }
    else {
        // Most PID controllers use 2 * rcCommand as target angle for ANGLE mode
        return rcCommand * 2;
    }
}

int16_t leanAngleToRcCommand(int16_t leanAngle)
{
    if (posControl.pidProfile->pidController == PID_CONTROLLER_LUX_FLOAT) {
        // LuxFloat is the only PID controller that uses raw rcCommand as target angle
        return leanAngle;
    }
    else {
        // Most PID controllers use 2 * rcCommand as target angle for ANGLE mode
        return leanAngle / 2;
    }
}

/*-----------------------------------------------------------
 * Ability to execute RTH on external event
 *-----------------------------------------------------------*/
void activateForcedRTH(void)
{
    posControl.flags.forcedRTHActivated = true;
    processNavigationModeSwitch();
}

void abortForcedRTH(void)
{
    posControl.flags.forcedRTHActivated = false;
    processNavigationModeSwitch();
}

rthState_e getStateOfForcedRTH(void)
{
    if (navShouldApplyRTH()) {
        if (posControl.navMissionState == NAV_AUTO_LANDED || posControl.navMissionState == NAV_AUTO_FINISHED) {
            return RTH_HAS_LANDED;
        }
        else if (posControl.flags.hasValidPositionSensor) {
            return RTH_IN_PROGRESS_OK;
        }
        else {
            return RTH_IN_PROGRESS_LOST_GPS;
        }
    }
    else {
        return RTH_IDLE;
    }
}
#endif  // NAV
