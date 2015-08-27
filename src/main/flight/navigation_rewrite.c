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

// Navigation PosControl
navigationPosControl_t   posControl;

// Desired pitch/roll/yaw/throttle adjustments
static int16_t rcCommandAdjustedThrottle;

// Current navigation mode & profile
static barometerConfig_t *barometerConfig;

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
 * Update desired RTH altitude, depends on rth_alt_control_style parameter
 *-----------------------------------------------------------*/
static void updateTargetRTHAltitude(void)
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

/*-----------------------------------------------------------
 * Processes an update to XY-position and velocity
 *-----------------------------------------------------------*/
static void updateActualHorizontalPositionAndVelocity(float newX, float newY, float newVelX, float newVelY)
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
static void updateActualAltitudeAndClimbRate(float newAltitude, float newVelocity)
{
    posControl.actualState.pos.V.Z = newAltitude;
    posControl.actualState.vel.V.Z = newVelocity;

    // Update altitude that would be used when executing RTH
    updateTargetRTHAltitude();

#if defined(NAV_BLACKBOX)
    navLatestActualPosition[Z] = lrintf(newAltitude);
    navActualVelocity[Z] = constrain(lrintf(newVelocity), -32678, 32767);
#endif

    posControl.flags.verticalPositionNewData = 1;
}

/*-----------------------------------------------------------
 * Processes an update to estimated heading
 *-----------------------------------------------------------*/
static void updateActualHeading(int32_t newHeading)
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
void gpsConvertGeodeticToLocal(gpsLocation_t * origin, bool originValid, gpsLocation_t * llh, t_fp_vector * pos)
{
    if (originValid) {
        float gpsScaleLonDown = constrainf(cos_approx((ABS(origin->lat) / 10000000.0f) * 0.0174532925f), 0.01f, 1.0f);
        pos->V.X = (llh->lat - origin->lat) * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
        pos->V.Y = (llh->lon - origin->lon) * (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * gpsScaleLonDown);
        pos->V.Z = (llh->alt - origin->alt);
    }
    else {
        pos->V.X = 0;
        pos->V.Y = 0;
        pos->V.Z = 0;
    }
}

void gpsConvertLocalToGeodetic(gpsLocation_t * origin, bool originValid, t_fp_vector * pos, gpsLocation_t * llh)
{
    if (originValid) {
        float gpsScaleLonDown = constrainf(cos_approx((ABS(origin->lat) / 10000000.0f) * 0.0174532925f), 0.01f, 1.0f);
        llh->lat = origin->lat + lrintf(pos->V.X / DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR);
        llh->lon = origin->lon + lrintf(pos->V.Y / (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * gpsScaleLonDown));
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
    gpsConvertLocalToGeodetic(&posControl.gpsOrigin, posControl.gpsOriginValid, &posControl.homeWaypoint.pos, &GPS_home);
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
        updateTargetRTHAltitude();

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

// FIXME: Make this configurable, default to about 5% highet than minthrottle
#define minFlyableThrottle  (masterConfig.escAndServoConfig.minthrottle + (masterConfig.escAndServoConfig.maxthrottle - masterConfig.escAndServoConfig.minthrottle) * 5 / 100)

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
    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else {
        resetMulticopterAltitudeController();
    }
}

static void setupAltitudeController(void)
{
    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else {
        setupMulticopterAltitudeController();
    }
}

static void applyAltitudeController(uint32_t currentTime)
{
    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
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
    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else {
        resetMulticopterHeadingController();
    }
}

static void applyHeadingController(uint32_t currentTime)
{
    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
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
    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else {
        resetMulticopterPositionController();
    }
}

static void applyPositionController(uint32_t currentTime)
{
    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else {
        applyMulticopterPositionController(currentTime);
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
static navigationMode_t selectNavModeFromBoxModeInput(void);
void applyWaypointNavigationAndAltitudeHold(void)
{
    uint32_t currentTime = micros();

    // No navigation when disarmed
    if (!ARMING_FLAG(ARMED)) {
        posControl.enabled = false;
        return;
    }

    // FIXME: Make this compliant with FIXED_WING
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

    // Save processed throttle for future use
    rcCommandAdjustedThrottle = rcCommand[THROTTLE];
}

/*-----------------------------------------------------------
 * NAV land detector
 *-----------------------------------------------------------*/
static bool isLandingDetected(bool resetDetector)
{
    static uint32_t landingTimer;
    uint32_t currentTime = micros();

    if (resetDetector) {
        landingTimer = currentTime;
        return false;
    }

    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
        return false;
    }
    else {
        // Average climb rate should be low enough
        bool verticalMovement = fabsf(posControl.actualState.vel.V.Z) > 25.0f;

        // check if we are moving horizontally
        bool horizontalMovement = sqrtf(sq(posControl.actualState.vel.V.X) + sq(posControl.actualState.vel.V.Y)) > 100.0f;

        // Throttle should be low enough
        // We use rcCommandAdjustedThrottle to keep track of NAV corrected throttle (isLandingDetected is executed
        // from processRx() and rcCommand at that moment holds rc input, not adjusted values from NAV core)
        bool minimalThrust = rcCommandAdjustedThrottle <= (masterConfig.escAndServoConfig.minthrottle + (masterConfig.escAndServoConfig.maxthrottle - masterConfig.escAndServoConfig.minthrottle) * 0.25f);

        if (!minimalThrust || !navShouldApplyRTHLandingLogic() || verticalMovement || horizontalMovement) {
            landingTimer = currentTime;
            return false;
        }
        else {
            return ((currentTime - landingTimer) > LAND_DETECTOR_TRIGGER_TIME) ? true : false;
        }
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
    bool canActivatePosHold = posControl.gpsOriginValid && sensors(SENSOR_ACC) && (sensors(SENSOR_GPS) && STATE(GPS_FIX) && GPS_numSat >= 5) && (sensors(SENSOR_MAG) && persistentFlag(FLAG_MAG_CALIBRATION_DONE));

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
                posControl.navRthState = NAV_RTH_STATE_INIT;
                break;
            default: // NAV_MODE_NONE
                resetNavigation();
                posControl.mode = newNavMode;
                break;
        }
    }

    swithNavigationFlightModes(posControl.mode);

#if defined(NAV_BLACKBOX)
    navCurrentMode = (int16_t)posControl.mode;
#endif

    // Process RTH state machine
    if (STATE(FIXED_WING)) { // FIXED_WING
        // FIXME: Multirotor only, fixed-wing logic must be different
    }
    else {
        if (posControl.mode == NAV_MODE_RTH) {
            // 3D RTH state machine
            switch (posControl.navRthState) {
                case NAV_RTH_STATE_INIT:
                    if (posControl.homeDistance < posControl.navProfile->nav_min_rth_distance) {
                        // Prevent RTH jump in your face, when arming copter accidentally activating RTH (or RTH on failsafe)
                        // Inspired by CrashPilot1000's TestCode3
                        // Reset home to currect position
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
                        // Reset landing detector
                        isLandingDetected(true);
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
                    else if (isLandingDetected(false)) {
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
    barometerConfig = intialBarometerConfig;
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
    if (axis == Z)
        return posControl.actualState.pos.V.Z;
    else
        return 0;
}

/*-----------------------------------------------------------
 * NAV data collection and pre-processing code
 * This is the largest sensor-dependent part of nav-rewrite.
 * Adding new sensors, implementing EKF, etc. should modify
 * this part of code and do not touch the above code (if possible)
 *-----------------------------------------------------------*/
// Why is this here: Because GPS will be sending at quiet a nailed rate (if not overloaded by junk tasks at the brink of its specs)
// but we might read out with timejitter because Irq might be off by a few us so we do a +-10% margin around the time between GPS
// datasets representing the most common Hz-rates today. You might want to extend the list or find a smarter way.
// Don't overload your GPS in its config with trash, choose a Hz rate that it can deliver at a sustained rate.
// (c) CrashPilot1000
static uint32_t getGPSDeltaTimeFilter(uint32_t dTus)
{
    if (dTus >= 225000 && dTus <= 275000) return HZ2US(4);       //  4Hz Data 250ms
    if (dTus >= 180000 && dTus <= 220000) return HZ2US(5);       //  5Hz Data 200ms
    if (dTus >=  90000 && dTus <= 110000) return HZ2US(10);      // 10Hz Data 100ms
    if (dTus >=  45000 && dTus <=  55000) return HZ2US(20);      // 20Hz Data  50ms
    if (dTus >=  30000 && dTus <=  36000) return HZ2US(30);      // 30Hz Data  33ms
    if (dTus >=  23000 && dTus <=  27000) return HZ2US(40);      // 40Hz Data  25ms
    if (dTus >=  18000 && dTus <=  22000) return HZ2US(50);      // 50Hz Data  20ms
    return dTus;                                                 // Filter failed. Set GPS Hz by measurement
}

/*
 * newLat, newLon - new coordinates
 * newAlt - new MSL altitude (cm)
 * newVel - new velocity (cm/s)
 * newCOG - new course over ground (degrees * 10)
 */
void onNewGPSData(int32_t newLat, int32_t newLon, int32_t newAlt)
{
    static uint32_t previousTime;
    static bool isFirstUpdate = true;
    static int32_t previousLat;
    static int32_t previousLon;

    static int32_t latFilterTable[3];
    static int32_t lonFilterTable[3];
    static int32_t altFilterTable[3];
    static int8_t  gpsFilterIndex;

    static float gpsVelocityX = 0.0f, gpsVelocityY = 0.0f;

    gpsLocation_t newLLH;
    t_fp_vector newGPSPos;

    // Don't have a valid GPS 3D fix, do nothing and restart
    if (!(STATE(GPS_FIX) && GPS_numSat >= 5)) {
        isFirstUpdate = true;
        return;
    }

    // Set GPS origin
    if (!posControl.gpsOriginValid) {
        posControl.gpsOrigin.lat = newLat;
        posControl.gpsOrigin.lon = newLon;
        posControl.gpsOrigin.alt = newAlt;
        posControl.gpsOriginValid = true;
    }

    uint32_t currentTime = micros();

    // If not first update - calculate velocities
    if (!isFirstUpdate) {
        float dT = US2S(getGPSDeltaTimeFilter(currentTime - previousTime));
        float gpsScaleLonDown = constrainf(cos_approx((ABS(newLat) / 10000000.0f) * 0.0174532925f), 0.01f, 1.0f);

        // Update filter table
        latFilterTable[gpsFilterIndex] = newLat;
        lonFilterTable[gpsFilterIndex] = newLon;
        altFilterTable[gpsFilterIndex] = newAlt;
        if (++gpsFilterIndex >= 3) gpsFilterIndex = 0;

        // Apply median filter
        newLat = quickMedianFilter3(latFilterTable);
        newLon = quickMedianFilter3(lonFilterTable);
        newAlt = quickMedianFilter3(altFilterTable);

        // Calculate NEU velocities
        gpsVelocityX = (gpsVelocityX + (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * (newLat - previousLat) / dT)) / 2.0f;
        gpsVelocityY = (gpsVelocityY + (gpsScaleLonDown * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * (newLon - previousLon) / dT)) / 2.0f;

        // Update IMU velocities with complementary filter to keep them close to real velocities (as given by GPS)
        imuApplyFilterToActualVelocity(X, posControl.navProfile->nav_gps_cf, gpsVelocityX);
        imuApplyFilterToActualVelocity(Y, posControl.navProfile->nav_gps_cf, gpsVelocityY);

        // Convert to local coordinates
        newLLH.lat = newLat;
        newLLH.lon = newLon;
        newLLH.alt = newAlt;
        gpsConvertGeodeticToLocal(&posControl.gpsOrigin, posControl.gpsOriginValid, &newLLH, &newGPSPos);

#if defined(BARO)
        if (sensors(SENSOR_BARO)) {
            // Adjust barometer offset to compensate for barometric drift
            float gpsPosCorrectionZ = newGPSPos.V.Z - posControl.latestBaroAlt;
            posControl.baroOffset -= gpsPosCorrectionZ * 0.01f * dT;   // FIXME: Explain 0.01f
        }
#endif

        updateActualHorizontalPositionAndVelocity(newGPSPos.V.X, newGPSPos.V.Y, imuAverageVelocity.V.X, imuAverageVelocity.V.Y);
    }
    else {
        int i;

        // Initialize GPS velocity
        gpsVelocityX = 0.0f;
        gpsVelocityY = 0.0f;

        // Initialize GPS filter table
        gpsFilterIndex = 0;
        for (i = 0; i < 3; i++) {
            latFilterTable[i] = newLat;
            lonFilterTable[i] = newLon;
            altFilterTable[i] = newAlt;
        }

        // Convert to local coordinates (isFirstUpdate might be set, but this might also occur after GPS_FIX was lost and re-acquired)
        newLLH.lat = newLat;
        newLLH.lon = newLon;
        newLLH.alt = newAlt;
        gpsConvertGeodeticToLocal(&posControl.gpsOrigin, posControl.gpsOriginValid, &newLLH, &newGPSPos);

        updateActualHorizontalPositionAndVelocity(newGPSPos.V.X, newGPSPos.V.Y, 0, 0);
    }

    previousLat = newLat;
    previousLon = newLon;

    isFirstUpdate = false;
    previousTime = currentTime;

    updateHomePosition();
}

void updateEstimatedHeading(void)
{
    // NAV uses heading in centidegrees
    updateActualHeading((int32_t)heading * 100);
}

void updateAltitudeAndClimbRate(void)
{
    float estimatedAlt, estimatedClimbRate, imuFilterWeight;

#if defined(BARO)
    float newBaroAlt, baroClimbRate;
#endif

#if defined(SONAR)
    float newSonarAlt, sonarClimbRate;
#endif

#if defined(BARO)
    static filterWithBufferSample_t baroClimbRateFilterBuffer[NAV_BARO_CLIMB_RATE_FILTER_SIZE];
    static filterWithBufferState_t baroClimbRateFilter;
#endif
    static bool climbRateFiltersInitialized = false;

    static uint32_t previousTimeUpdate = 0;
    uint32_t currentTime = micros();
    uint32_t deltaMicros = currentTime - previousTimeUpdate;

    // too fast, likely no new data available
    if (deltaMicros < HZ2US(ALTITUDE_UPDATE_FREQUENCY_HZ))
        return;

    previousTimeUpdate = currentTime;

    // Initialize climb rate filter
    if (!climbRateFiltersInitialized) {
#if defined(BARO)
        // If BARO compiled in - initialize filter
        filterWithBufferInit(&baroClimbRateFilter, &baroClimbRateFilterBuffer[0], NAV_BARO_CLIMB_RATE_FILTER_SIZE);
#endif
        climbRateFiltersInitialized = true;
    }

#if defined(BARO)
    // Calculate barometric altitude and climb rate
    // For NAV to work good baro altitude must not be delayed much. Large delay means slow response, means low PID gains to avoid oscillations
    // One should keep baro_tab_size small, but this will lead to high noise. NAV is OK with noisy measures, LPFs in altitude control
    // code and new CLT fusion will handle that just fine
    newBaroAlt = baroCalculateAltitude() - posControl.baroOffset;

    if (sensors(SENSOR_BARO) && isBaroCalibrationComplete()) {
        filterWithBufferUpdate(&baroClimbRateFilter, newBaroAlt, currentTime);
        baroClimbRate = filterWithBufferApply_Derivative(&baroClimbRateFilter) * 1e6f;

        baroClimbRate = constrainf(baroClimbRate, -1500, 1500);  // constrain baro velocity +/- 1500cm/s
        baroClimbRate = applyDeadband(baroClimbRate, 10);       // to reduce noise near zero
    }
    else {
        newBaroAlt = 0;
        baroClimbRate = 0.0f;
    }

    // Save baro altitude for valid baro offset correction via GPS
    posControl.latestBaroAlt = newBaroAlt;
#endif

#if defined(SONAR)
    // Calculate sonar altitude above surface and climb rate above surface
    if (sensors(SENSOR_SONAR)) {
        static uint32_t lastValidSonarUpdateTime = 0;
        static float lastValidSonarAlt;

        // Read sonar
        newSonarAlt = sonarRead();

        // FIXME: Add sonar tilt compensation

        if (newSonarAlt >= 0) {
            // We have a valid reading

            if ((currentTime - lastValidSonarUpdateTime) < HZ2US(MIN_SONAR_UPDATE_FREQUENCY_HZ)) {
                // Sonar updated within valid time, use this measurement to calculate climb rate
                sonarClimbRate = (newSonarAlt - lastValidSonarAlt) / ((currentTime - lastValidSonarUpdateTime) * 1e-6f);
            }
            else {
                // Previous sonar update was delayed too much - we can't trust sonarClimbRate yet
                sonarClimbRate = 0.0f;
            }

            lastValidSonarAlt = newSonarAlt;
            lastValidSonarUpdateTime = currentTime;
        }
    }
    else {
        // No sonar
        newSonarAlt = -1;
        sonarClimbRate = 0.0f;
    }
#endif

#if defined(BARO) && defined(SONAR)
    if (newSonarAlt < 0) {
        // Can't trust sonar - rely on baro
        estimatedAlt = newBaroAlt;
        estimatedClimbRate = baroClimbRate;
        imuFilterWeight = barometerConfig->baro_cf_vel;
    }
    else {
        // Fuse altitude
        if (newSonarAlt <= (SONAR_MAX_RANGE * 2 / 3)) {
            // If within 2/3 sonar range - use only sonar
            estimatedAlt = newSonarAlt;
        }
        else if (newSonarAlt <= SONAR_MAX_RANGE) {
            // Squeze difference between sonar and baro into upper 1/3 sonar range.
            // FIXME: this will give us totally wrong altitude in the upper 
            //        1/3 sonar range but will allow graceful transition from SONAR to BARO
            float sonarToBaroTransition = constrainf((SONAR_MAX_RANGE - newSonarAlt) / (SONAR_MAX_RANGE / 3.0f), 0, 1);
            estimatedAlt = newSonarAlt * sonarToBaroTransition + newBaroAlt * (1.0f - sonarToBaroTransition);
        }

        // FIXME: Make all this configurable
        // Fuse velocity - trust sonar more and baro less
        estimatedClimbRate = 0.333f * baroClimbRate + 0.667f * sonarClimbRate;
        imuFilterWeight = barometerConfig->baro_cf_vel;
    }
#elif defined(BARO)
    estimatedAlt = newBaroAlt;
    estimatedClimbRate = baroClimbRate;
    imuFilterWeight = barometerConfig->baro_cf_vel;
#elif defined(SONAR)
    // If sonar reading is not valid, assume that we are outside valid sonar range and set measured altitude to SONAR_MAX_RANGE
    // This will allow us to enable althold above sonar range and not hit the ground when going within range
    if (newSonarAlt < 0)
        estimatedAlt = SONAR_MAX_RANGE;
    else
        estimatedAlt = newSonarAlt;

    estimatedClimbRate = sonarClimbRate;
    imuFilterWeight = 0.900f; // FIXME: make configurable
#else
    #error "FIXME - Shouldn't happen (no baro or sonar)"
#endif

    // By using CF it's possible to correct the drift of integrated accZ (velocity) without loosing the phase, i.e without delay
    imuApplyFilterToActualVelocity(Z, imuFilterWeight, estimatedClimbRate);

    updateActualAltitudeAndClimbRate(estimatedAlt, imuAverageVelocity.V.Z);
}

#endif  // NAV
