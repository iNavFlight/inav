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
#include "common/color.h"
#include "common/maths.h"

#include "drivers/sensor.h"
#include "drivers/system.h"
#include "drivers/gpio.h"
#include "drivers/timer.h"
#include "drivers/serial.h"
#include "drivers/accgyro.h"
#include "drivers/compass.h"
#include "drivers/pwm_rx.h"

#include "rx/rx.h"

#include "sensors/sensors.h"
#include "sensors/sonar.h"
#include "sensors/barometer.h"
#include "sensors/compass.h"
#include "sensors/acceleration.h"
#include "sensors/gyro.h"
#include "sensors/battery.h"
#include "sensors/boardalignment.h"

#include "io/serial.h"
#include "io/gps.h"
#include "io/gimbal.h"
#include "io/ledstrip.h"

#include "telemetry/telemetry.h"
#include "blackbox/blackbox.h"

#include "flight/pid.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/failsafe.h"
#include "flight/gps_conversion.h"
#include "flight/navigation_rewrite.h"
#include "flight/navigation_rewrite_private.h"

#include "config/runtime_config.h"
#include "config/config.h"
#include "config/config_profile.h"
#include "config/config_master.h"

#if defined(NAV)

navigationPosControl_t   posControl;

#if defined(NAV_BLACKBOX)
int16_t navCurrentMode;
int16_t navActualVelocity[3];
int16_t navDesiredVelocity[3];
int16_t navLatestPositionError[3];
int16_t navActualHeading;
int16_t navDesiredHeading;
int16_t navTargetPosition[3];
int32_t navLatestActualPosition[3];
int16_t navDebug[4];
uint16_t navFlags;
#endif

static navigationMode_t selectNavModeFromBoxModeInput(void);

/*-----------------------------------------------------------
 * A simple 1-st order LPF filter implementation
 *-----------------------------------------------------------*/
float navApplyFilter(float input, float fCut, float dT, float * state)
{
    float RC = 1.0f / (2.0f * (float)M_PI * fCut);

    *state = *state + dT / (RC + dT) * (input - *state);

    return *state;
}

/*-----------------------------------------------------------
 * Float point PID-controller implementation
 *-----------------------------------------------------------*/
float navPidGetP(float error, float dt, pidController_t *pid)
{
    float newPterm = error * pid->param.kP;

    if (posControl.navConfig->pterm_cut_hz)
        newPterm = navApplyFilter(newPterm, posControl.navConfig->pterm_cut_hz, dt, &pid->pterm_filter_state);

#if defined(NAV_BLACKBOX)
    pid->lastP = newPterm;
#endif

    return newPterm;
}

float navPidGetI(float error, float dt, pidController_t *pid, bool onlyShrinkI)
{
    float newIntegrator = pid->integrator + ((float)error * pid->param.kI) * dt;

    if (onlyShrinkI) {
        // Only allow I to shrink
        if (fabsf(newIntegrator) < fabsf(pid->integrator)) {
            pid->integrator = newIntegrator;
        }
    }
    else {
        pid->integrator = newIntegrator;
    }

    pid->integrator = constrainf(pid->integrator, -pid->param.Imax, pid->param.Imax);

#if defined(NAV_BLACKBOX)
    pid->lastI = pid->integrator;
#endif

    return pid->integrator;
}

float navPidGetD(float error, float dt, pidController_t *pid)
{
    float newDerivative = (error - pid->last_error) / dt;
    pid->last_error = error;

    if (posControl.navConfig->dterm_cut_hz)
        newDerivative = pid->param.kD * navApplyFilter(newDerivative, posControl.navConfig->dterm_cut_hz, dt, &pid->dterm_filter_state);
    else
        newDerivative = pid->param.kD * newDerivative;

#if defined(NAV_BLACKBOX)
    pid->lastD = newDerivative;
#endif

    return newDerivative;
}

float navPidGetPID(float error, float dt, pidController_t *pid, bool onlyShrinkI)
{
    return navPidGetP(error, dt, pid) + navPidGetI(error, dt, pid, onlyShrinkI) + navPidGetD(error, dt, pid);
}

void navPidReset(pidController_t *pid)
{
    pid->integrator = 0;
    pid->last_error = 0;
    pid->pterm_filter_state = 0;
    pid->dterm_filter_state = 0;
}

void navPidInit(pidController_t *pid, float _kP, float _kI, float _kD, float _Imax)
{
    pid->param.kP = _kP;
    pid->param.kI = _kI;
    pid->param.kD = _kD;
    pid->param.Imax = _Imax;
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
#define DEGREES_80_IN_DECIDEGREES 800
bool isThrustFacingDownwards(rollAndPitchInclination_t *inclination)
{
    return ABS(inclination->values.rollDeciDegrees) < DEGREES_80_IN_DECIDEGREES && ABS(inclination->values.pitchDeciDegrees) < DEGREES_80_IN_DECIDEGREES;
}

/*-----------------------------------------------------------
 * Processes an update to XY-position and velocity
 *-----------------------------------------------------------*/
void updateActualHorizontalPositionAndVelocity(float newX, float newY, float newVelX, float newVelY)
{
    posControl.actualState.pos.V.X = newX;
    posControl.actualState.pos.V.Y = newY;

    posControl.actualState.vel.V.X = newVelX;
    posControl.actualState.vel.V.Y = newVelY;

#if defined(NAV_BLACKBOX)
    navLatestActualPosition[X] = newX;
    navLatestActualPosition[Y] = newY;
    navActualVelocity[X] = constrain(lrintf(newVelX), -32678, 32767);
    navActualVelocity[Y] = constrain(lrintf(newVelY), -32678, 32767);
#endif

    posControl.flags.horizontalPositionNewData = 1;
}

/*-----------------------------------------------------------
 * Processes an update to Z-position and velocity
 *-----------------------------------------------------------*/
void updateActualAltitudeAndClimbRate(float newAltitude, float newVelocity)
{
    posControl.actualState.pos.V.Z = newAltitude;
    posControl.actualState.vel.V.Z = newVelocity;

    // Update altitude that would be used when executing RTH
    setupAutonomousControllerRTH();

#if defined(NAV_BLACKBOX)
    navLatestActualPosition[Z] = lrintf(newAltitude);
    navActualVelocity[Z] = constrain(lrintf(newVelocity), -32678, 32767);
#endif

    posControl.flags.verticalPositionNewData = 1;
}

/*-----------------------------------------------------------
 * Processes an update to estimated heading
 *-----------------------------------------------------------*/
void updateActualHeading(int32_t newHeading)
{
    /* Update heading */
    posControl.actualState.yaw = newHeading;

#if defined(NAV_BLACKBOX)
    navActualHeading = constrain(lrintf(posControl.actualState.yaw), -32678, 32767);
#endif

    posControl.flags.headingNewData = 1;
}

/*-----------------------------------------------------------
 * Calculates distance and bearing to destination point
 *-----------------------------------------------------------*/
#define TAN_89_99_DEGREES 5729.577951308f
uint32_t calculateDistanceToDestination(t_fp_vector * destinationPos)
{
    float deltaX = destinationPos->V.X - posControl.actualState.pos.V.X;
    float deltaY = destinationPos->V.Y - posControl.actualState.pos.V.Y;

    return lrintf(sqrtf(sq(deltaX) + sq(deltaY)));
}

int32_t calculateBearingToDestination(t_fp_vector * destinationPos)
{
    float deltaX = destinationPos->V.X - posControl.actualState.pos.V.X;
    float deltaY = destinationPos->V.Y - posControl.actualState.pos.V.Y;

    return wrap_36000(constrain((int32_t)(atan2_approx(deltaY, deltaX) * TAN_89_99_DEGREES), -18000, 18000));
}

/*-----------------------------------------------------------
 * Check if waypoint is/was reached
 *-----------------------------------------------------------*/
bool isWaypointReached(navWaypointPosition_t *waypoint)
{
    // We consider waypoint reached if within specified radius
    uint32_t wpDistance = calculateDistanceToDestination(&waypoint->pos);
    return (wpDistance <= posControl.navConfig->waypoint_radius);
}

/*-----------------------------------------------------------
 * Coordinate conversions
 *-----------------------------------------------------------*/
void gpsConvertGeodeticToLocal(gpsOrigin_s * origin, gpsLocation_t * llh, t_fp_vector * pos)
{
    if (!origin->valid) {
        origin->valid = true;
        origin->lat = llh->lat;
        origin->lon = llh->lon;
        origin->alt = llh->alt;
        origin->scale = constrainf(cos_approx((ABS(origin->lat) / 10000000.0f) * 0.0174532925f), 0.01f, 1.0f);
    }

    pos->V.X = (llh->lat - origin->lat) * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
    pos->V.Y = (llh->lon - origin->lon) * (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * origin->scale);
    pos->V.Z = (llh->alt - origin->alt);
}

void gpsConvertLocalToGeodetic(gpsOrigin_s * origin, t_fp_vector * pos, gpsLocation_t * llh)
{
    if (origin->valid) {
        llh->lat = origin->lat + lrintf(pos->V.X / DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR);
        llh->lon = origin->lon + lrintf(pos->V.Y / (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * origin->scale));
        llh->alt = origin->alt + lrintf(pos->V.Z);
    }
    else {
        llh->lat = 0;
        llh->lon = 0;
        llh->alt = 0;
    }
}

/*-----------------------------------------------------------
 * Compatibility for home position
 *-----------------------------------------------------------*/
gpsLocation_t GPS_home;
uint16_t      GPS_distanceToHome;        // distance to home point in meters
int16_t       GPS_directionToHome;       // direction to home point in degrees

static void updateHomePositionCompatibility(void)
{
    gpsConvertLocalToGeodetic(&posControl.gpsOrigin, &posControl.homePosition.pos, &GPS_home);
    GPS_distanceToHome = posControl.homeDistance / 100;
    GPS_directionToHome = posControl.homeDirection / 100;
}

/*-----------------------------------------------------------
 * Reset home position to current position
 *-----------------------------------------------------------*/
void setHomePosition(t_fp_vector * pos, int32_t yaw)
{
    if (STATE(GPS_FIX) && GPS_numSat >= 5) {
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
}

/*-----------------------------------------------------------
 * Update home position, calculate distance and bearing to home
 *-----------------------------------------------------------*/
void updateHomePosition(void)
{
    // Disarmed and have a valid position, constantly update home
    if (!ARMING_FLAG(ARMED) && posControl.flags.hasValidPositionSensor) {
        setHomePosition(&posControl.actualState.pos, posControl.actualState.yaw);
    }

    // Update distance and direction to home
    if (STATE(GPS_FIX_HOME)) {
        posControl.homeDistance = calculateDistanceToDestination(&posControl.homePosition.pos);
        posControl.homeDirection = calculateBearingToDestination(&posControl.homePosition.pos);
        updateHomePositionCompatibility();
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
void updateAltitudeTargetFromClimbRate(uint32_t deltaMicros, float climbRate)
{
    UNUSED(deltaMicros);

    // Calculate new altitude target
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

    wpLLH.lat = 0;
    wpLLH.lon = 0;
    wpLLH.alt = 0;

    // WP #0 - special waypoint - HOME
    if (wpNumber == 0) {
        if (STATE(GPS_FIX_HOME)) {
            wpLLH = GPS_home;
        }
    }
    // WP #255 - special waypoint - directly get actualPosition
    else if (wpNumber == 255) {
        gpsConvertLocalToGeodetic(&posControl.gpsOrigin, &posControl.actualState.pos, &wpLLH);
    }
    // WP #1 - #15 - common waypoints - pre-programmed mission
    else if ((wpNumber >= 1) && (wpNumber <= NAV_MAX_WAYPOINTS)) {
        if (wpNumber <= posControl.waypointCount) {
            gpsConvertLocalToGeodetic(&posControl.gpsOrigin, &posControl.waypointList[wpNumber - 1].pos, &wpLLH);
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
    if (!(STATE(GPS_FIX) && GPS_numSat >= 5))
        return;

    // Convert to local coordinates
    wpLLH.lat = wpLat;
    wpLLH.lon = wpLon;
    wpLLH.alt = wpAlt;
    gpsConvertGeodeticToLocal(&posControl.gpsOrigin, &wpLLH, &wpPos.pos);
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
                if (rcCommand[THROTTLE] > (masterConfig.rxConfig.midrc + posControl.navConfig->alt_hold_deadband)) {
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
            rcCommand[THROTTLE] = masterConfig.escAndServoConfig.minthrottle;
        }
        return;
    }

    if (navShouldApplyEmergencyLanding()) {
        // Check for emergency landing sequence
        applyEmergencyLandingController();
    }
    else {
        // Apply navigation adjustments
        if (navShouldApplyAltHold() && isThrustFacingDownwards(&inclination)) {
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
    navFlags = 0;
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
 * An indicator that throttle tilt compensation for multirotors is controlled by NAV
 *-----------------------------------------------------------*/
bool navigationControlsThrottleAngleCorrection(void)
{
    return posControl.navConfig->flags.throttle_tilt_comp && navShouldApplyAltHold();
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

/**
 * Process NAV mode transition and WP/RTH state machine
 *  Update rate: RX (data driven or 50Hz)
 */
void updateWaypointsAndNavigationMode(void)
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

void navigationUsePIDs(pidProfile_t *initialPidProfile)
{
    int axis;

    posControl.pidProfile = initialPidProfile;

    // Initialize position hold PI-controller
    for (axis = 0; axis < 2; axis++) {
        navPInit(&posControl.pids.pos[axis], (float)posControl.pidProfile->P8[PIDPOS] / 100.0f);

        navPidInit(&posControl.pids.vel[axis], (float)posControl.pidProfile->P8[PIDPOSR] / 100.0f,
                                               (float)posControl.pidProfile->I8[PIDPOSR] / 100.0f,
                                               (float)posControl.pidProfile->D8[PIDPOSR] / 1000.0f,
                                               400.0);
    }

    // Initialize altitude hold PID-controllers (pos_z, vel_z, acc_z
    navPInit(&posControl.pids.pos[Z], (float)posControl.pidProfile->P8[PIDALT] / 100.0f);

    navPidInit(&posControl.pids.vel[Z], (float)posControl.pidProfile->I8[PIDALT] / 100.0f, 0, 0, 0);

    navPidInit(&posControl.pids.accz, (float)posControl.pidProfile->P8[PIDVEL] / 100.0f,
                                      (float)posControl.pidProfile->I8[PIDVEL] / 100.0f,
                                      (float)posControl.pidProfile->D8[PIDVEL] / 1000.0f,
                                      400.0);

    // Heading PID (duplicates maghold)
    navPInit(&posControl.pids.heading, (float)posControl.pidProfile->P8[PIDMAG] / 30.0f);
}

void navigationInit(navConfig_t *initialnavConfig,
                    pidProfile_t *initialPidProfile,
                    rcControlsConfig_t *initialRcControlsConfig)
{
    /* Initial state */
    posControl.enabled = 0;
    posControl.mode = NAV_MODE_NONE;
    posControl.flags.verticalPositionNewData = 0;
    posControl.flags.horizontalPositionNewData = 0;
    posControl.flags.headingNewData = 0;
    posControl.flags.hasValidAltitudeSensor = 0;
    posControl.flags.hasValidPositionSensor = 0;
    posControl.flags.forcedRTHActivated = 0;
    posControl.waypointCount = 0;

    /* Use system config */
    navigationUseConfig(initialnavConfig);
    navigationUsePIDs(initialPidProfile);
    navigationUseRcControlsConfig(initialRcControlsConfig);
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
 * Ability to execute RTH on external event
 *-----------------------------------------------------------*/
bool canActivateForcedRTH(void)
{
    return canActivatePosHoldMode() && STATE(GPS_FIX_HOME);
}

void activateForcedRTH(void)
{
    posControl.flags.forcedRTHActivated = true;
}

void abortForcedRTH(void)
{
    posControl.flags.forcedRTHActivated = false;
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
