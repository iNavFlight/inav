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
#endif

static void setupRTHController(void);
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

    if (posControl.navProfile->nav_pterm_cut_hz)
        newPterm = navApplyFilter(newPterm, posControl.navProfile->nav_pterm_cut_hz, dt, &pid->pterm_filter_state);

#if defined(NAV_BLACKBOX)
    pid->lastP = newPterm;
#endif

    return newPterm;
}

float navPidGetI(float error, float dt, pidController_t *pid)
{
    pid->integrator += ((float)error * pid->param.kI) * dt;
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

    if (posControl.navProfile->nav_dterm_cut_hz)
        newDerivative = pid->param.kD * navApplyFilter(newDerivative, posControl.navProfile->nav_dterm_cut_hz, dt, &pid->dterm_filter_state);
    else
        newDerivative = pid->param.kD * newDerivative;

#if defined(NAV_BLACKBOX)
    pid->lastD = newDerivative;
#endif

    return newDerivative;
}

float navPidGetPID(float error, float dt, pidController_t *pid)
{
    return navPidGetP(error, dt, pid) + navPidGetI(error, dt, pid) + navPidGetD(error, dt, pid);
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
    setupRTHController();

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
void calculateDistanceAndBearingToDestination(t_fp_vector * currentPos, t_fp_vector * destinationPos, uint32_t *dist, int32_t *bearing)
{
    float deltaX = destinationPos->V.X - currentPos->V.X;
    float deltaY = destinationPos->V.Y - currentPos->V.Y;

    if (dist) {
        *dist = sqrtf(sq(deltaX) + sq(deltaY));
    }

    if (bearing) {
        *bearing = wrap_36000(constrain((int32_t)(atan2_approx(deltaY, deltaX) * TAN_89_99_DEGREES), -18000, 18000));
    }
}

/*-----------------------------------------------------------
 * Check if waypoint is/was reached
 *-----------------------------------------------------------*/
static bool navIsWaypointReached(navWaypointPosition_t *waypoint)
{
    // We consider waypoint reached if within specified radius
    uint32_t wpDistance;
    calculateDistanceAndBearingToDestination(&posControl.actualState.pos, &waypoint->pos, &wpDistance, NULL);
    return (wpDistance <= posControl.navProfile->nav_wp_radius);
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
    gpsConvertLocalToGeodetic(&posControl.gpsOrigin, &posControl.homeWaypoint.pos, &GPS_home);
    GPS_distanceToHome = posControl.homeDistance / 100;
    GPS_directionToHome = posControl.homeDirection / 100;
}

/*-----------------------------------------------------------
 * Reset home position to current position
 *-----------------------------------------------------------*/
void resetHomePosition(void)
{
    if (STATE(GPS_FIX) && GPS_numSat >= 5) {
        posControl.homeWaypoint.pos = posControl.actualState.pos;
        posControl.homeWaypoint.yaw = posControl.actualState.yaw;
        posControl.homeDistance = 0;
        posControl.homeDirection = 0;

        // Update target RTH altitude as a waypoint above home
        posControl.homeWaypointAbove = posControl.homeWaypoint;
        setupRTHController();

        updateHomePositionCompatibility();
        ENABLE_STATE(GPS_FIX_HOME);
    }
}

/*-----------------------------------------------------------
 * Update home position, calculate distance and bearing to home
 *-----------------------------------------------------------*/
void updateHomePosition(void)
{
    // Disarmed, reset home position
    if (!ARMING_FLAG(ARMED))
        DISABLE_STATE(GPS_FIX_HOME);

    // Arming and don't have home position set - do it now
    if (!STATE(GPS_FIX_HOME) && ARMING_FLAG(ARMED) && STATE(GPS_FIX) && GPS_numSat >= 5) {
        resetHomePosition();
    }

    // Update distance and direction to home
    if (STATE(GPS_FIX_HOME)) {
        calculateDistanceAndBearingToDestination(&posControl.actualState.pos, &posControl.homeWaypoint.pos, &posControl.homeDistance, &posControl.homeDirection);
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
        int32_t wpBearing;
        calculateDistanceAndBearingToDestination(&posControl.actualState.pos, pos, NULL, &wpBearing);
        posControl.desiredState.yaw = wpBearing;
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

static void applyHeadingController(uint32_t currentTime)
{
    if (STATE(FIXED_WING)) {
        applyFixedWingHeadingController(currentTime);
    }
    else {
        applyMulticopterHeadingController(currentTime);
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
 * RTH controller
 *-----------------------------------------------------------*/
static void setupRTHController(void)
{
    if (ARMING_FLAG(ARMED)) {
        if (!navShouldApplyRTH()) {
            switch (posControl.navProfile->flags.rth_alt_control_style) {
            case NAV_RTH_NO_ALT:
                posControl.homeWaypointAbove.pos.V.Z = posControl.actualState.pos.V.Z;
                break;
            case NAX_RTH_EXTRA_ALT: // Maintain current altitude + predefined safety margin
                posControl.homeWaypointAbove.pos.V.Z = posControl.actualState.pos.V.Z + posControl.navProfile->nav_rth_altitude;
                break;
            case NAV_RTH_MAX_ALT:
                posControl.homeWaypointAbove.pos.V.Z = MAX(posControl.homeWaypointAbove.pos.V.Z, posControl.actualState.pos.V.Z);
                break;
            case NAV_RTH_AT_LEAST_ALT:  // Climb to at least some predefined altitude above home
                posControl.homeWaypointAbove.pos.V.Z = MAX(posControl.homeWaypoint.pos.V.Z + posControl.navProfile->nav_rth_altitude, posControl.actualState.pos.V.Z);
                break;
            case NAV_RTH_CONST_ALT:     // Climb/descend to predefined altitude above home
            default:
                posControl.homeWaypointAbove.pos.V.Z = posControl.homeWaypoint.pos.V.Z + posControl.navProfile->nav_rth_altitude;
                break;
            }
        }
    }
    else {
        posControl.homeWaypointAbove.pos.V.Z = posControl.actualState.pos.V.Z;
    }
}

static void resetRTHController(void)
{
    posControl.navRthState = NAV_RTH_STATE_INIT;
}

static void applyRTHController(void)
{
    /* RTH control logic is the same for multicopter and fixed-wing */
    if (posControl.mode == NAV_MODE_RTH) {
        // 3D RTH state machine
        switch (posControl.navRthState) {
            case NAV_RTH_STATE_INIT:
                if (posControl.homeDistance < posControl.navProfile->nav_min_rth_distance) {
                    // Close to original home - reset home to currect position
                    resetHomePosition();
                    // Set position lock on home and heading to original heading, altitude to current altitude
                    setDesiredPosition(&posControl.homeWaypoint.pos, posControl.homeWaypoint.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                    setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);
                    posControl.navRthState = NAV_RTH_STATE_HOME_AUTOLAND;
                }
                else {
                    // Climb/descend to safe altitude if needed
                    setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                    setDesiredPosition(&posControl.homeWaypointAbove.pos, 0, NAV_POS_UPDATE_Z);
                    posControl.navRthState = NAV_RTH_STATE_CLIMB_TO_SAVE_ALTITUDE;
                }
                break;
            case NAV_RTH_STATE_CLIMB_TO_SAVE_ALTITUDE:
                if (fabsf(posControl.actualState.pos.V.Z - posControl.homeWaypointAbove.pos.V.Z) < 50.0f) {
                    // Set target position to home and calculate original bearing
                    setDesiredPosition(&posControl.homeWaypoint.pos, posControl.homeWaypoint.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_BEARING);
                    setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);
                    posControl.navRthState = NAV_RTH_STATE_HEAD_HOME;
                }
                else {
                    // TODO: a safeguard that would emergency land us if can not climb to desired altitude
                }
                break;
            case NAV_RTH_STATE_HEAD_HOME:
                // Stay at this state until home reached
                if (navIsWaypointReached(&posControl.homeWaypoint)) {
                    // Set position lock on home and heading to original heading when lauched
                    setDesiredPosition(&posControl.homeWaypoint.pos, posControl.homeWaypoint.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                    setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);
                    resetLandingDetector();
                    posControl.navRthState = NAV_RTH_STATE_HOME_AUTOLAND;
                }
                else {
                    // TODO: a safeguard that would emergency land us in case of unexpected situation - battery low, etc
                }
                break;
            case NAV_RTH_STATE_HOME_AUTOLAND:
                if (!ARMING_FLAG(ARMED)) {
                    posControl.navRthState = NAV_RTH_STATE_FINISHED;
                }
                else if (isLandingDetected()) {
                    posControl.navRthState = NAV_RTH_STATE_LANDED;
                }
                else {
                    // Still landing. Altitude controller takes care of gradual descent.
                    // TODO: A safeguard in case of emergency
                }
                break;
            case NAV_RTH_STATE_LANDED:
                // RTH is a non-normal flight mode. Engaging RTH likely means that pilot cannot or don't want to control aircraft.
                // Craft in RTH mode should return home, land, disarm and lock out rearming to prevent accidental takeoff
                //ENABLE_ARMING_FLAG(PREVENT_ARMING);
                //mwDisarm();
                posControl.navRthState = NAV_RTH_STATE_FINISHED;
                break;
            case NAV_RTH_STATE_FINISHED:
                // Stay in this state forever
                break;
        }
    }
    else if (posControl.mode == NAV_MODE_RTH_2D) {
        // 2D RTH state machine (when no ALTHOLD is available)
        switch (posControl.navRthState) {
            case NAV_RTH_STATE_INIT:
                if (posControl.homeDistance < posControl.navProfile->nav_min_rth_distance) {
                    resetHomePosition();
                    setDesiredPosition(&posControl.homeWaypoint.pos, posControl.homeWaypoint.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                    posControl.navRthState = NAV_RTH_STATE_FINISHED;
                }
                else {
                    // In case of 2D RTH - head home immediately
                    setDesiredPosition(&posControl.homeWaypoint.pos, posControl.homeWaypoint.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_BEARING);
                    posControl.navRthState = NAV_RTH_STATE_HEAD_HOME;
                }
                break;
            case NAV_RTH_STATE_HEAD_HOME:
                // Stay at this state until home reached
                if (navIsWaypointReached(&posControl.homeWaypoint)) {
                    // Set position lock on home and heading to original heading when lauched
                    setDesiredPosition(&posControl.homeWaypoint.pos, posControl.homeWaypoint.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                    posControl.navRthState = NAV_RTH_STATE_FINISHED;
                }
                else {
                    // TODO: a safeguard that would emergency land us in case of unexpected situation - battery low, etc
                }
                break;
            case NAV_RTH_STATE_FINISHED:
            default:
                // Stay in this state forever
                break;
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

    // No navigation when disarmed
    if (!ARMING_FLAG(ARMED)) {
        posControl.enabled = false;
        return;
    }

    if (!posControl.enabled) {
        if (posControl.navProfile->flags.lock_nav_until_takeoff) {
            if (posControl.navProfile->flags.use_midrc_for_althold) {
                if (rcCommand[THROTTLE] > (masterConfig.rxConfig.midrc + posControl.navProfile->nav_rc_deadband)) {
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
        if ((selectNavModeFromBoxModeInput() != NAV_MODE_NONE) && posControl.navProfile->flags.lock_nav_until_takeoff) { // && posControl.navProfile->flags.use_midrc_for_althold
            rcCommand[THROTTLE] = masterConfig.escAndServoConfig.minthrottle;
        }
        return;
    }

    // Apply navigation adjustments
    if (navShouldApplyAltHold() && isThrustFacingDownwards(&inclination)) {
        applyAltitudeController(currentTime);
    }

    if (navShouldApplyPosHold() || navShouldApplyWaypoint() || navShouldApplyRTH()) {
        applyPositionController(currentTime);
    }

    if (navShouldApplyHeadingControl()) {
        applyHeadingController(currentTime);
    }
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
static navigationMode_t selectNavModeFromBoxModeInput(void)
{
    // Flags if we can activate certain nav modes (check if we have required sensors and they provide valid data)
    bool canActivateAltHold = sensors(SENSOR_BARO) || sensors(SENSOR_SONAR);
    bool canActivatePosHold = posControl.gpsOrigin.valid && sensors(SENSOR_ACC) && (sensors(SENSOR_GPS) && STATE(GPS_FIX) && GPS_numSat >= 5) && 
                                (sensors(SENSOR_MAG) && persistentFlag(FLAG_MAG_CALIBRATION_DONE));

    // Figure out, what mode pilot want to activate, also check if it is possible
    if (IS_RC_MODE_ACTIVE(BOXNAVRTH) && canActivatePosHold && canActivateAltHold && STATE(GPS_FIX_HOME)) {
        return NAV_MODE_RTH;
    }
    else if (IS_RC_MODE_ACTIVE(BOXNAVRTH) && canActivatePosHold && STATE(GPS_FIX_HOME)) {
        // Fall back to 2D RTH if can not activate altitude hold
        return NAV_MODE_RTH_2D;
    }
    else if (IS_RC_MODE_ACTIVE(BOXNAVWP) && canActivatePosHold && canActivateAltHold) {
        return NAV_MODE_WP;
    }
    else if (IS_RC_MODE_ACTIVE(BOXNAVPOSHOLD) && IS_RC_MODE_ACTIVE(BOXNAVALTHOLD) && canActivatePosHold && canActivateAltHold) {
        return NAV_MODE_POSHOLD_3D;
    }
    else if (IS_RC_MODE_ACTIVE(BOXNAVPOSHOLD) && canActivatePosHold) {
        return NAV_MODE_POSHOLD_2D;
    }
    else if (IS_RC_MODE_ACTIVE(BOXNAVALTHOLD) && canActivateAltHold) {
        return NAV_MODE_ALTHOLD;
    }

    return NAV_MODE_NONE;
}

/*-----------------------------------------------------------
 * An indicator that throttle tilt compensation for multirotors is controlled by NAV
 *-----------------------------------------------------------*/
bool navigationControlsThrottleAngleCorrection(void)
{
    return posControl.navProfile->flags.throttle_tilt_comp && navShouldApplyAltHold();
}

/*-----------------------------------------------------------
 * An indicator that ANGLE mode must be forced per NAV requirement
 *-----------------------------------------------------------*/
bool naivationRequiresAngleMode(void)
{
    return navShouldApplyPosHold() || navShouldApplyWaypoint() || navShouldApplyRTH();
}

/*-----------------------------------------------------------
 * An indicator that NAV is in charge of heading control (a signal to disable other heading controllers)
 *-----------------------------------------------------------*/
bool naivationControlsHeadingNow(void)
{
    return navShouldApplyHeadingControl();
}

/*-----------------------------------------------------------
 * Process NAV mode transition and RTH state machine
 *-----------------------------------------------------------*/
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
                // Check if previous mode was using ALTHOLD, re-use target altitude if necessary
                if (navShouldApplyAltHold() && !navShouldApplyRTHLandingLogic()) {
                    // We were already applying ALTHOLD, don't update anything
                    setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_NONE);
                }
                else {
                    // Update desired Z-position to actual position
                    setupAltitudeController();
                    setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);
                }
                posControl.mode = NAV_MODE_ALTHOLD;
                break;
            case NAV_MODE_POSHOLD_2D:
                if (navShouldApplyPosHold() && !navShouldApplyRTH()) {
                    // XY-controller already active, update nothing
                    setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_NONE);
                }
                else {
                    // Update XY-controller and Heading controller target
                    setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                }
                posControl.mode = NAV_MODE_POSHOLD_2D;
                break;
            case NAV_MODE_POSHOLD_3D:
                // Depending on current navMode we can re-use target position and/or altitude
                if (navShouldApplyAltHold() && !navShouldApplyRTHLandingLogic()) {
                    if (navShouldApplyPosHold() && !navShouldApplyRTH()) {
                        // Already applying XY and Z-controllers
                        setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_NONE);
                    }
                    else {
                        // Already applying Z-controller, update only XY-controller
                        setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                    }
                }
                else {
                    setupAltitudeController();
                    if (navShouldApplyPosHold() && !navShouldApplyRTH()) {
                        // No Z-controlller yet, update it
                        setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);
                    }
                    else {
                        // Update XY and Z-controller target
                        setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING);
                    }
                }
                posControl.mode = NAV_MODE_POSHOLD_3D;
                break;
            case NAV_MODE_WP:
                // TODO
                posControl.mode = NAV_MODE_NONE;
                break;
            case NAV_MODE_RTH:
            case NAV_MODE_RTH_2D:
                // We fix @ current position and climb to safe altitude
                setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING);
                posControl.mode = newNavMode;
                resetRTHController();
                break;
            default: // NAV_MODE_NONE
                resetNavigation();
                posControl.mode = newNavMode;
                break;
        }
    }

    // Map navMode back to enabled flight modes
    swithNavigationFlightModes(posControl.mode);

    // Apply RTH sequence controller
    applyRTHController();

#if defined(NAV_BLACKBOX)
    navCurrentMode = (int16_t)posControl.mode;
#endif
}

/*-----------------------------------------------------------
 * NAV main control functions
 *-----------------------------------------------------------*/
void navigationUseProfile(navProfile_t *navProfileToUse)
{
    posControl.navProfile = navProfileToUse;
}

void navigationUseBarometerConfig(barometerConfig_t * intialBarometerConfig)
{
    posControl.barometerConfig = intialBarometerConfig;
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
                                               200.0);
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

void navigationInit(navProfile_t *initialNavProfile,
                    pidProfile_t *initialPidProfile,
                    barometerConfig_t *intialBarometerConfig,
                    rcControlsConfig_t *initialRcControlsConfig)
{
    /* Initial state */
    posControl.enabled = 0;
    posControl.mode = NAV_MODE_NONE;
    posControl.flags.verticalPositionNewData = 0;
    posControl.flags.horizontalPositionNewData = 0;
    posControl.flags.headingNewData = 0;

    posControl.baroOffset = 0.0f;

    /* Use system config */
    navigationUseProfile(initialNavProfile);
    navigationUsePIDs(initialPidProfile);
    navigationUseBarometerConfig(intialBarometerConfig);
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

#endif  // NAV
