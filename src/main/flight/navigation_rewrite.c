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

float actualAverageVerticalVelocity;    // average climb rate (updated every 250ms)

static int16_t altholdInitialThrottle;  // Throttle input when althold was activated
static int16_t lastAdjustedThrottle = 0;
int32_t targetRTHAltitude;

// Desired pitch/roll/yaw/throttle adjustments
static int16_t rcAdjustment[4];

// Current navigation mode & profile
static navProfile_t *navProfile;
static barometerConfig_t *barometerConfig;
static rcControlsConfig_t *rcControlsConfig;
static pidProfile_t *pidProfile;
static navRthState_t navRthState;

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
#define NAV_BLACKBOX_DEBUG(x,y) navDebug[x] = constrain((y), -32678, 32767)
#else
#define NAV_BLACKBOX_DEBUG(x,y)
#endif

/*-----------------------------------------------------------
 * A simple 1-st order LPF filter implementation
 *-----------------------------------------------------------*/
static float navApplyFilter(float input, float fCut, float dT, float * state)
{
    float RC = 1.0f / (2.0f * (float)M_PI * fCut);

    *state = *state + dT / (RC + dT) * (input - *state);

    return *state;
}

/*-----------------------------------------------------------
 * Float point PID-controller implementation
 *-----------------------------------------------------------*/
static float pidGetP(float error, float dt, pidController_t *pid)
{
    float newPterm = error * pid->param.kP;

    if (navProfile->nav_pterm_cut_hz)
        newPterm = navApplyFilter(newPterm, navProfile->nav_pterm_cut_hz, dt, &pid->pterm_filter_state);

#if defined(NAV_BLACKBOX)
    pid->lastP = newPterm;
#endif

    return newPterm;
}

static float pidGetI(float error, float dt, pidController_t *pid)
{
    pid->integrator += ((float)error * pid->param.kI) * dt;
    pid->integrator = constrainf(pid->integrator, -pid->param.Imax, pid->param.Imax);

#if defined(NAV_BLACKBOX)
    pid->lastI = pid->integrator;
#endif

    return pid->integrator;
}

static float pidGetD(float error, float dt, pidController_t *pid)
{
    float newDerivative = (error - pid->last_error) / dt;
    pid->last_error = error;

    if (navProfile->nav_dterm_cut_hz)
        newDerivative = pid->param.kD * navApplyFilter(newDerivative, navProfile->nav_dterm_cut_hz, dt, &pid->dterm_filter_state);
    else
        newDerivative = pid->param.kD * newDerivative;

#if defined(NAV_BLACKBOX)
    pid->lastD = newDerivative;
#endif

    return newDerivative;
}

static float pidGetPID(float error, float dt, pidController_t *pid)
{
    return pidGetP(error, dt, pid) + pidGetI(error, dt, pid) + pidGetD(error, dt, pid);
}

static void pidReset(pidController_t *pid)
{
    pid->integrator = 0;
    pid->last_error = 0;
    pid->pterm_filter_state = 0;
    pid->dterm_filter_state = 0;
}

static void pidInit(pidController_t *pid, float _kP, float _kI, float _kD, float _Imax)
{
    pid->param.kP = _kP;
    pid->param.kI = _kI;
    pid->param.kD = _kD;
    pid->param.Imax = _Imax;
    pidReset(pid);
}

/*-----------------------------------------------------------
 * Float point P-controller implementation
 *-----------------------------------------------------------*/
static void pInit(pController_t *p, float _kP)
{
    p->param.kP = _kP;
}

/*-----------------------------------------------------------
 * Functions to wrap angles in centidegrees (deg*100)
 *-----------------------------------------------------------*/
static int32_t wrap_18000(int32_t error)
{
    if (error > 18000)
        error -= 36000;
    if (error < -18000)
        error += 36000;
    return error;
}

static int32_t wrap_36000(int32_t angle)
{
    if (angle > 36000)
        angle -= 36000;
    if (angle < 0)
        angle += 36000;
    return angle;
}

/*-----------------------------------------------------------
 * Detects if thrust vector is facing downwards
 *-----------------------------------------------------------*/
#define DEGREES_80_IN_DECIDEGREES 800
bool navIsThrustFacingDownwards(rollAndPitchInclination_t *inclination)
{
    return ABS(inclination->values.rollDeciDegrees) < DEGREES_80_IN_DECIDEGREES && ABS(inclination->values.pitchDeciDegrees) < DEGREES_80_IN_DECIDEGREES;
}

/*-----------------------------------------------------------
 * A function to reset navigation PIDs and states
 *-----------------------------------------------------------*/
void resetNavigation(void)
{
    int i;

    pidReset(&posControl.pids.accz);
    pidReset(&posControl.pids.vel[Z]);

    for (i = 0; i < 2; i++) {
        pidReset(&posControl.pids.vel[i]);
    }

    for (i = 0; i < 4; i++) {
        rcAdjustment[i] = 0;
    }
}

/*-----------------------------------------------------------
 * Update desired RTH altitude, depends on rth_alt_control_style parameter
 *-----------------------------------------------------------*/
static void updateTargetRTHAltitude(void)
{
    if (ARMING_FLAG(ARMED)) {
        if (!navShouldApplyRTH()) {
            switch (navProfile->flags.rth_alt_control_style) {
            case NAV_RTH_NO_ALT:
                targetRTHAltitude = posControl.actualState.pos.V.Z;
                break;
            case NAX_RTH_EXTRA_ALT: // Maintain current altitude + predefined safety margin
                targetRTHAltitude = posControl.actualState.pos.V.Z + navProfile->nav_rth_altitude;
                break;
            case NAV_RTH_CONST_ALT: // Climb to predefined altitude
                targetRTHAltitude = posControl.homeWaypoint.pos.V.Z + navProfile->nav_rth_altitude;
                break;
            case NAV_RTH_MAX_ALT:
                targetRTHAltitude = MAX(targetRTHAltitude, posControl.actualState.pos.V.Z);
                break;
            default: // same as NAV_RTH_CONST_ALT
                targetRTHAltitude = posControl.homeWaypoint.pos.V.Z + navProfile->nav_rth_altitude;
                break;
            }
        }
    }
    else {
        targetRTHAltitude = navProfile->nav_rth_altitude;
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
#define AVERAGE_VERTICAL_VEL_INTERVAL   250000      // 250ms, 4Hz
static void updateActualAltitudeAndClimbRate(uint32_t currentTime, float newAltitude, float newVelocity)
{
    static uint32_t averageVelocityLastUpdateTime = 0;
    static float averageVelocityAccumulator = 0;
    static uint32_t averageVelocitySampleCount = 1;

    posControl.actualState.pos.V.Z = newAltitude;
    posControl.actualState.vel.V.Z = newVelocity;

    averageVelocityAccumulator += newVelocity;
    averageVelocitySampleCount += 1;

    if ((currentTime - averageVelocityLastUpdateTime) >= AVERAGE_VERTICAL_VEL_INTERVAL) {
        if (averageVelocitySampleCount) {
            actualAverageVerticalVelocity = averageVelocityAccumulator / averageVelocitySampleCount;
            averageVelocityLastUpdateTime = currentTime;
        }
        else {
            actualAverageVerticalVelocity = 0;
        }

        averageVelocityAccumulator = 0;
        averageVelocitySampleCount = 0;
    }

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
#define GPS_RAW_TO_RAD (0.0000001f * (float)M_PI / 180.0f)
// Took this from https://github.com/Crashpilot1000/TestCode3/blob/master/src/navigation.c#L113
// Get distance between two points in cm Get bearing from pos1 to pos2, returns an 1deg = 100 precision
// Now with more correct BEARING calclation according to this: http://www.movable-type.co.uk/scripts/latlong.html
// var coslat2 = cos(lat2);
// var y = sin(dLon) * coslat2;
// var x = cos(lat1) * sin(lat2) - sin(lat1) * coslat2 * cos(dLon);
// var brng = atan2(y, x).toDeg(); -180  0 +180
// bearing is in deg*100!
static void calculateDistanceAndBearingToDestination(t_fp_vector * currentPos, t_fp_vector * destinationPos, uint32_t *dist, int32_t *bearing)
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
    return (wpDistance <= navProfile->nav_wp_radius);
}

void navConvertGeodeticToLocal(navLocation_t * llh, t_fp_vector * pos)
{
    if (posControl.gpsOriginValid) {
        float gpsScaleLonDown = constrainf(cos_approx((ABS(posControl.gpsOrigin.lat) / 10000000.0f) * 0.0174532925f), 0.01f, 1.0f);
        pos->V.X = (llh->lat - posControl.gpsOrigin.lat) * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
        pos->V.Y = (llh->lon - posControl.gpsOrigin.lon) * (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * gpsScaleLonDown);
        pos->V.Z = (llh->alt - posControl.gpsOrigin.alt);
    }
    else {
        pos->V.X = 0;
        pos->V.Y = 0;
        pos->V.Z = 0;
    }
}

void navConvertLocalToGeodetic(t_fp_vector * pos, navLocation_t * llh)
{
    if (posControl.gpsOriginValid) {
        float gpsScaleLonDown = constrainf(cos_approx((ABS(posControl.gpsOrigin.lat) / 10000000.0f) * 0.0174532925f), 0.01f, 1.0f);
        llh->lat = posControl.gpsOrigin.lat + pos->V.X / DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
        llh->lon = posControl.gpsOrigin.lon + pos->V.Y / (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * gpsScaleLonDown);
        llh->alt = posControl.gpsOrigin.alt + pos->V.Z;
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
navLocation_t GPS_home;
uint16_t      GPS_distanceToHome;        // distance to home point in meters
int16_t       GPS_directionToHome;       // direction to home point in degrees

static void updateHomePositionCompatibility(void)
{
    navConvertLocalToGeodetic(&posControl.homeWaypoint.pos, &GPS_home);
    GPS_distanceToHome = posControl.homeDistance / 100;
    GPS_directionToHome = posControl.homeDirection / 100;
}

/*-----------------------------------------------------------
 * Reset home position to current position
 *-----------------------------------------------------------*/
void resetHomePosition(void)
{
    if (STATE(GPS_FIX) && GPS_numSat >= 5) {
        posControl.homeWaypoint.pos.V.X = posControl.actualState.pos.V.X;
        posControl.homeWaypoint.pos.V.Y = posControl.actualState.pos.V.Y;
        posControl.homeWaypoint.pos.V.Z = posControl.actualState.pos.V.Z;
        posControl.homeWaypoint.yaw = posControl.actualState.yaw;
        posControl.homeDistance = 0;
        posControl.homeDirection = 0;
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
 * Calculate rcAdjustment for YAW
 *-----------------------------------------------------------*/
static void calculateHeadingAdjustment(float dTnav)
{
    UNUSED(dTnav);

    // FIXME: Account for fixed-wing config without rudder (flying wing)

    // Calculate yaw correction
    int32_t headingError = wrap_18000(posControl.actualState.yaw - posControl.desiredState.yaw) * masterConfig.yaw_control_direction;
    headingError = constrain(headingError, -3000, +3000); // limit error to +- 30 degrees to avoid fast rotation

    // FIXME: SMALL_ANGLE might prevent NAV from adjusting yaw when banking is too high (i.e. nav in high wind)
    if (STATE(SMALL_ANGLE)) {
        // Heading PID controller takes degrees, not centidegrees (this pid duplicates MAGHOLD)
        rcAdjustment[YAW] = (headingError / 100.0f) * posControl.pids.heading.param.kP;
    }
}

// FIXME: Make this configurable, default to about 5% highet than minthrottle
#define minFlyableThrottle  (masterConfig.escAndServoConfig.minthrottle + (masterConfig.escAndServoConfig.maxthrottle - masterConfig.escAndServoConfig.minthrottle) * 5 / 100)
/*-----------------------------------------------------------
 * Set altitude hold throttle midpoint
 *-----------------------------------------------------------*/
static void setAltHoldInitialThrottle(int16_t throttle)
{
    // FIXME: Account for fixed wing aircraft, throttle DOES NOT control altitude there
    if (navProfile->flags.use_midrc_for_althold) {
        altholdInitialThrottle = masterConfig.rxConfig.midrc;
    }
    else {
        altholdInitialThrottle = throttle;
    }
}

/*-----------------------------------------------------------
 * Adjusts desired heading from pilot's input
 *-----------------------------------------------------------*/
static void adjustHeadingFromRCInput()
{
    // In some cases pilot has no control over flight direction
    if (navCanAdjustHeadingFromRCInput()) {
        if (STATE(FIXED_WING)) { // FIXED_WING
            // TODO
        }
        else { // MULTIROTOR
            // Passthrough yaw input if stick is moved
            int16_t rcYawAdjustment = applyDeadband(rcCommand[YAW], navProfile->nav_rc_deadband);

            if (rcYawAdjustment) {
                rcAdjustment[YAW] = rcYawAdjustment;

                // Can only allow pilot to set the new heading if doing PH, during RTH copter will target itself to home
                if (navShouldApplyPosHold()) {
                    posControl.desiredState.yaw = posControl.actualState.yaw;
                }
            }
        }
    }
}

/*-----------------------------------------------------------
 * NAV updates
 *-----------------------------------------------------------*/
static navigationMode_t selectNavModeFromBoxModeInput(void);

/*-----------------------------------------------------------
 * Z-position controller
 *-----------------------------------------------------------*/
static void resetAltitudeController()
{
    // TODO
    if (STATE(FIXED_WING)) { // FIXED_WING
    }
    else {
        pidReset(&posControl.pids.vel[Z]);
        pidReset(&posControl.pids.accz);
        rcAdjustment[THROTTLE] = 0;
    }
}

static void updateAltitudeTargetFromClimbRate(uint32_t deltaMicros, float climbRate)
{
    UNUSED(deltaMicros);

    // Calculate new altitude target
    posControl.desiredState.pos.V.Z = posControl.actualState.pos.V.Z + (climbRate / posControl.pids.pos[Z].param.kP);
}

static void updateAltitudeTargetFromRCInput(uint32_t deltaMicros)
{
    // In some cases pilot has no control over flight direction
    if (!navCanAdjustAltitudeFromRCInput())
        return;

    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
        int16_t rcThrottleAdjustment = applyDeadband(rcCommand[THROTTLE] - altholdInitialThrottle, rcControlsConfig->alt_hold_deadband);

        if (rcThrottleAdjustment) {
            // set velocity proportional to stick movement
            float rcClimbRate = rcThrottleAdjustment * navProfile->nav_manual_speed_vertical / (500.0f - rcControlsConfig->alt_hold_deadband);
            updateAltitudeTargetFromClimbRate(deltaMicros, rcClimbRate);
        }
    }
}

// Position to velocity controller for Z axis
static void updateAltitudeVelocityController(uint32_t deltaMicros)
{
    UNUSED(deltaMicros);

    float altitudeError = posControl.desiredState.pos.V.Z - posControl.actualState.pos.V.Z;

    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
        // Use only P term for PH velocity calculation
        altitudeError = constrainf(altitudeError, -500, 500);
        //altitudeError = applyDeadband(altitudeError, 10); // remove small P parameter to reduce noise near zero position
        posControl.desiredState.vel.V.Z = altitudeError * posControl.pids.pos[Z].param.kP;
        posControl.desiredState.vel.V.Z = constrainf(posControl.desiredState.vel.V.Z, -300, 300); // hard limit velocity to +/- 3 m/s
    }

#if defined(NAV_BLACKBOX)
    navDesiredVelocity[Z] = constrain(lrintf(posControl.desiredState.vel.V.Z), -32678, 32767);
    navLatestPositionError[Z] = constrain(lrintf(altitudeError), -32678, 32767);
    navTargetPosition[Z] = constrain(lrintf(posControl.desiredState.pos.V.Z), -32678, 32767);
#endif
}

static void updateAltitudeAccelController(uint32_t deltaMicros)
{
    static float velFilterState;

    // calculate rate error and filter with cut off frequency of 2 Hz
    float velError = posControl.desiredState.vel.V.Z - posControl.actualState.vel.V.Z;
    velError = navApplyFilter(velError, NAV_VEL_ERROR_CUTOFF_FREQENCY_HZ, US2S(deltaMicros), &velFilterState);
    posControl.desiredState.acc.V.Z = velError * posControl.pids.vel[Z].param.kP;
}

static void updateAltitudeThrottleController(uint32_t deltaMicros)
{
    static float throttleFilterState;
    float accError = posControl.desiredState.acc.V.Z - imuAverageAcceleration.V.Z;

    rcAdjustment[THROTTLE] = pidGetPID(accError, US2S(deltaMicros), &posControl.pids.accz);
    rcAdjustment[THROTTLE] = navApplyFilter(rcAdjustment[THROTTLE], NAV_THROTTLE_CUTOFF_FREQENCY_HZ, US2S(deltaMicros), &throttleFilterState);
    rcAdjustment[THROTTLE] = constrain(rcAdjustment[THROTTLE], -500, 500);
}

static void applyAltitudeController(uint32_t currentTime)
{
    static uint32_t previousTimeTargetPositionUpdate;   // Occurs @ POSITION_TARGET_UPDATE_RATE_HZ
    static uint32_t previousTimePositionUpdate;         // Occurs @ altitude sensor update rate (max MAX_ALTITUDE_UPDATE_FREQUENCY_HZ)
    static uint32_t previousTimeUpdate;                 // Occurs @ looptime rate

    uint32_t deltaMicros = currentTime - previousTimeUpdate;
    previousTimeUpdate = currentTime;

    // If last position update was too long in the past - ignore it (likely restarting altitude controller)
    if (deltaMicros > HZ2US(MIN_ALTITUDE_UPDATE_FREQUENCY_HZ)) {
        previousTimeUpdate = currentTime;
        previousTimeTargetPositionUpdate = currentTime;
        previousTimePositionUpdate = currentTime;
        resetAltitudeController();
        return;
    }

    // Update altitude target from RC input or RTL controller
    if (currentTime - previousTimeTargetPositionUpdate >= HZ2US(POSITION_TARGET_UPDATE_RATE_HZ)) {
        uint32_t deltaMicrosPositionTargetUpdate = currentTime - previousTimeTargetPositionUpdate;
        previousTimeTargetPositionUpdate = currentTime;

        if (navShouldApplyRTHAltitudeLogic()) {
            // Gradually reduce descent speed depending on actual altitude.
            if (posControl.actualState.pos.V.Z > (posControl.homeWaypoint.pos.V.Z + 1000)) {
                updateAltitudeTargetFromClimbRate(deltaMicrosPositionTargetUpdate, -150.0f);
            }
            else if (posControl.actualState.pos.V.Z > (posControl.homeWaypoint.pos.V.Z + 250)) {
                updateAltitudeTargetFromClimbRate(deltaMicrosPositionTargetUpdate, -75.0f);
            }
            else {
                updateAltitudeTargetFromClimbRate(deltaMicrosPositionTargetUpdate, -35.0f);
            }
        }

        updateAltitudeTargetFromRCInput(deltaMicrosPositionTargetUpdate);
    }

    // If we have an update on vertical position data - update velocity and accel targets
    if (posControl.flags.verticalPositionNewData) {
        uint32_t deltaMicrosPositionUpdate = currentTime - previousTimePositionUpdate;
        previousTimePositionUpdate = currentTime;

        // Check if last correction was too log ago - ignore this update
        if (deltaMicrosPositionUpdate < HZ2US(MIN_ALTITUDE_UPDATE_FREQUENCY_HZ)) {
            updateAltitudeVelocityController(deltaMicrosPositionUpdate);
            updateAltitudeAccelController(deltaMicrosPositionUpdate);
        }
        else {
            // due to some glitch position update has not occurred in time, reset altitude controller
            resetAltitudeController();
        }

        // Indicate that information is no longer usable
        posControl.flags.verticalPositionNewData = 0;
    }

    // Update throttle controller
    // We are controlling acceleration here, IMU updates accel every loop so this step is executed at full loop rate,
    // regardless of available altitude and velocity data
    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else {
        updateAltitudeThrottleController(deltaMicros);

        uint16_t newThrottle = constrain(altholdInitialThrottle + rcAdjustment[THROTTLE], masterConfig.escAndServoConfig.minthrottle, masterConfig.escAndServoConfig.maxthrottle);
        if (navProfile->flags.throttle_tilt_comp && navIsThrustFacingDownwards(&inclination)) {
            float tiltCompFactor = 1.0f / constrainf(calculateCosTiltAngle(), 0.6f, 1.0f);  // max tilt about 50 deg
            newThrottle *= tiltCompFactor;
        }

        rcCommand[THROTTLE] = constrain(newThrottle, masterConfig.escAndServoConfig.minthrottle, masterConfig.escAndServoConfig.maxthrottle);
    }
}

/*-----------------------------------------------------------
 * XY position controller
 *-----------------------------------------------------------*/
static void resetPositionController()
{
    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else {
        int axis;
        for (axis = 0; axis < 2; axis++) {
            pidReset(&posControl.pids.vel[axis]);
            rcAdjustment[axis] = 0;
        }
    }
}

static void updatePositionTargetFromRCInput(uint32_t deltaMicros)
{
    UNUSED(deltaMicros);

    // In some cases pilot has no control over flight direction
    if (!navCanAdjustHorizontalVelocityAndAttitudeFromRCInput())
        return;

    if (navProfile->flags.user_control_mode != NAV_GPS_CRUISE)
        return;

    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
        int16_t rcPitchAdjustment = applyDeadband(rcCommand[PITCH], navProfile->nav_rc_deadband);
        int16_t rcRollAdjustment = applyDeadband(rcCommand[ROLL], navProfile->nav_rc_deadband);

        if (rcPitchAdjustment || rcRollAdjustment) {
            float rcVelX = rcPitchAdjustment * navProfile->nav_manual_speed_horizontal / (500.0f - navProfile->nav_rc_deadband);
            float rcVelY = rcRollAdjustment * navProfile->nav_manual_speed_horizontal / (500.0f - navProfile->nav_rc_deadband);

            // Calculate rotation coefficients
            float sinYaw = sin_approx(posControl.actualState.yaw * RADX100);
            float cosYaw = cos_approx(posControl.actualState.yaw * RADX100);

            // Rotate these velocities from body frame to to earth frame
            float neuVelX = rcVelX * cosYaw - rcVelY * sinYaw;
            float neuVelY = rcVelX * sinYaw + rcVelY * cosYaw;

            // Calculate new position target, so Pos-to-Vel P-controller would yield desired velocity
            posControl.desiredState.pos.V.X = posControl.actualState.pos.V.X + (neuVelX / posControl.pids.pos[X].param.kP);
            posControl.desiredState.pos.V.Y = posControl.actualState.pos.V.Y + (neuVelY / posControl.pids.pos[Y].param.kP);
        }
    }
}

static void updatePositionLeanAngleFromRCInput(uint32_t deltaMicros)
{
    UNUSED(deltaMicros);

    // In some cases pilot has no control over flight direction
    if (!navCanAdjustHorizontalVelocityAndAttitudeFromRCInput())
        return;

    if (navProfile->flags.user_control_mode != NAV_GPS_ATTI)
        return;

    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
        int16_t rcPitchAdjustment = applyDeadband(rcCommand[PITCH], navProfile->nav_rc_deadband);
        int16_t rcRollAdjustment = applyDeadband(rcCommand[ROLL], navProfile->nav_rc_deadband);

        if (rcPitchAdjustment || rcRollAdjustment) {
            // Direct attitude control
            rcAdjustment[PITCH] = rcPitchAdjustment;
            rcAdjustment[ROLL] = rcRollAdjustment;

            // If we are in position hold mode, so adjust poshold position
            if (navShouldApplyPosHold()) {
                posControl.desiredState.pos.V.X = posControl.actualState.pos.V.X;
                posControl.desiredState.pos.V.Y = posControl.actualState.pos.V.Y;
            }

            // When sticks are released we should restart velocity PIDs
            pidReset(&posControl.pids.vel[X]);
            pidReset(&posControl.pids.vel[Y]);
        }
    }
}

static void updatePositionVelocityController(uint32_t deltaMicros)
{
    UNUSED(deltaMicros);

    float posErrorX = posControl.desiredState.pos.V.X - posControl.actualState.pos.V.X;
    float posErrorY = posControl.desiredState.pos.V.Y - posControl.actualState.pos.V.Y;

    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
       //TODO: Apply some non-linear approach here. If we are close - go linear, if we are far, increase slower
        float newVelX = posErrorX * posControl.pids.pos[X].param.kP;
        float newVelY = posErrorY * posControl.pids.pos[Y].param.kP;
        float newVelTotal = sqrtf(sq(newVelX) + sq(newVelY));

        if (newVelTotal > navProfile->nav_speed_max) {
            newVelX = navProfile->nav_speed_max * (newVelX / newVelTotal);
            newVelY = navProfile->nav_speed_max * (newVelY / newVelTotal);
        }

        posControl.desiredState.vel.V.X = newVelX;
        posControl.desiredState.vel.V.Y = newVelY;
    }

#if defined(NAV_BLACKBOX)
    navDesiredVelocity[X] = constrain(lrintf(posControl.desiredState.vel.V.X), -32678, 32767);
    navDesiredVelocity[Y] = constrain(lrintf(posControl.desiredState.vel.V.Y), -32678, 32767);
    navLatestPositionError[X] = constrain(lrintf(posErrorX), -32678, 32767);
    navLatestPositionError[Y] = constrain(lrintf(posErrorY), -32678, 32767);
    navTargetPosition[X] = constrain(lrintf(posControl.desiredState.pos.V.X), -32678, 32767);
    navTargetPosition[Y] = constrain(lrintf(posControl.desiredState.pos.V.Y), -32678, 32767);
#endif
}

static void updatePositionAccelController(uint32_t deltaMicros, float maxAccelLimit)
{
    static float accFilterStateX = 0.0f, accFilterStateY = 0.0f;
    float velError, newAccelX, newAccelY;

    // Calculate acceleration target on X-axis
    velError = constrainf(posControl.desiredState.vel.V.X - posControl.actualState.vel.V.X, -500.0f, 500.0f); // limit error to 5 m/s
    newAccelX = pidGetPID(velError, US2S(deltaMicros), &posControl.pids.vel[X]);

#if defined(NAV_BLACKBOX)
    NAV_BLACKBOX_DEBUG(0, velError);
    NAV_BLACKBOX_DEBUG(1, posControl.pids.vel[X].lastP);
    NAV_BLACKBOX_DEBUG(2, posControl.pids.vel[X].lastI);
    NAV_BLACKBOX_DEBUG(3, posControl.pids.vel[X].lastD);
#endif

    // Calculate acceleration target on Y-axis
    velError = constrainf(posControl.desiredState.vel.V.Y - posControl.actualState.vel.V.Y, -500.0f, 500.0f); // limit error to 5 m/s
    newAccelY = pidGetPID(velError, US2S(deltaMicros), &posControl.pids.vel[Y]);

    // Check if required acceleration exceeds maximum allowed accel
    float newAccelTotal = sqrtf(sq(newAccelX) + sq(newAccelY));

    // Recalculate acceleration
    if (newAccelTotal > maxAccelLimit) {
        newAccelX = maxAccelLimit * (newAccelX / newAccelTotal);
        newAccelY = maxAccelLimit * (newAccelY / newAccelTotal);
    }

    // Apply LPF to acceleration target
    posControl.desiredState.acc.V.X = navApplyFilter(newAccelX, NAV_ACCEL_CUTOFF_FREQUENCY_HZ, US2S(deltaMicros), &accFilterStateX);
    posControl.desiredState.acc.V.Y = navApplyFilter(newAccelY, NAV_ACCEL_CUTOFF_FREQUENCY_HZ, US2S(deltaMicros), &accFilterStateY);
}

static void updatePositionLeanAngleController(uint32_t deltaMicros)
{
    UNUSED(deltaMicros);

    // Calculate rotation matrix coefficients
    float sinYaw = sin_approx(posControl.actualState.yaw * RADX100);
    float cosYaw = cos_approx(posControl.actualState.yaw * RADX100);

    // Rotate acceleration target into forward-right frame (aircraft)
    float accelForward = posControl.desiredState.acc.V.X * cosYaw + posControl.desiredState.acc.V.Y * sinYaw;
    float accelRight = -posControl.desiredState.acc.V.X * sinYaw + posControl.desiredState.acc.V.Y * cosYaw;

    // Calculate banking angles
    float desiredPitch = atan2_approx(accelForward, NAV_GRAVITY_CMSS) / RADX10;
    float desiredRoll = atan2_approx(accelRight * cos_approx(desiredPitch * RADX10), NAV_GRAVITY_CMSS) / RADX10;

    rcAdjustment[ROLL] = constrainf(desiredRoll, -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX);
    rcAdjustment[PITCH] = constrainf(desiredPitch, -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX);
}

static void applyPositionController(uint32_t currentTime)
{
    static uint32_t previousTimeTargetPositionUpdate;   // Occurs @ POSITION_TARGET_UPDATE_RATE_HZ
    static uint32_t previousTimePositionUpdate;         // Occurs @ GPS update rate
    static uint32_t previousTimeUpdate;                 // Occurs @ looptime rate

    uint32_t deltaMicros = currentTime - previousTimeUpdate;
    previousTimeUpdate = currentTime;

    // If last position update was too long in the past - ignore it (likely restarting altitude controller)
    if (deltaMicros > HZ2US(MIN_POSITION_UPDATE_FREQUENCY_HZ)) {
        previousTimeUpdate = currentTime;
        previousTimeTargetPositionUpdate = currentTime;
        previousTimePositionUpdate = currentTime;
        resetPositionController();
        return;
    }

    // Update altitude target from RC input
    if (currentTime - previousTimeTargetPositionUpdate >= HZ2US(POSITION_TARGET_UPDATE_RATE_HZ)) {
        uint32_t deltaMicrosPositionTargetUpdate = currentTime - previousTimeTargetPositionUpdate;
        previousTimeTargetPositionUpdate = currentTime;
        updatePositionTargetFromRCInput(deltaMicrosPositionTargetUpdate);
    }

    // If we have new position - update velocity and acceleration controllers
    if (posControl.flags.horizontalPositionNewData) {
        uint32_t deltaMicrosPositionUpdate = currentTime - previousTimePositionUpdate;
        previousTimePositionUpdate = currentTime;

        if (deltaMicrosPositionUpdate < HZ2US(MIN_POSITION_UPDATE_FREQUENCY_HZ)) {
            updatePositionVelocityController(deltaMicrosPositionUpdate);

            if (navShouldApplyWaypoint() || navShouldApplyRTH()) {
                // In case of waypoint navigation and RTH limit maximum acceleration to lower value
                updatePositionAccelController(deltaMicrosPositionUpdate, NAV_ACCEL_SLOW_XY_MAX);
            }
            else {
                // In case of PH limit acceleration to some high value
                updatePositionAccelController(deltaMicrosPositionUpdate, NAV_ACCELERATION_XY_MAX);
            }
        }
        else {
            resetPositionController();
        }

        // Indicate that information is no longer usable
        posControl.flags.horizontalPositionNewData = 0;
    }

    // Update lean angle controller. This update occurs at loop rate
    // TODO: Investigate if PositionLeanAngle controller need to be run @ looprate
    // All updates occur at GPS rate and lean angle should probably be recalculated at that rate as well
    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else {
        updatePositionLeanAngleController(deltaMicros);
        updatePositionLeanAngleFromRCInput(deltaMicros);

        // Convert target angle (rcAdjustment) to rcCommand, account for the way PID controllers treat the value
        if (pidProfile->pidController == PID_CONTROLLER_LUX_FLOAT) {
            // LuxFloat is the only PID controller that uses raw rcCommand as target angle
            rcCommand[PITCH] = constrain(rcAdjustment[PITCH], -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX);
            rcCommand[ROLL] = constrain(rcAdjustment[ROLL], -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX);
        }
        else {
            // Most PID controllers use 2 * rcCommand as target angle for ANGLE mode
            rcCommand[PITCH] = constrain(rcAdjustment[PITCH], -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX) / 2;
            rcCommand[ROLL] = constrain(rcAdjustment[ROLL], -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX) / 2;
        }
    }
}

/*-----------------------------------------------------------
 * Heading controller
 *-----------------------------------------------------------*/
static void applyHeadingController(uint32_t currentTime)
{
    static uint32_t previousTime;

    if (posControl.flags.headingNewData) {
        float dTnav = US2S(currentTime - previousTime);
        previousTime = currentTime;

        if (navShouldKeepHeadingToBearing()) {
            int32_t wpBearing;

            calculateDistanceAndBearingToDestination(&posControl.actualState.pos, &posControl.desiredState.pos, NULL, &wpBearing);
            posControl.desiredState.yaw = wpBearing;
        }

#if defined(NAV_BLACKBOX)
            navDesiredHeading = constrain(lrintf(posControl.desiredState.yaw), -32678, 32767);
#endif

        calculateHeadingAdjustment(dTnav);
        adjustHeadingFromRCInput();

        // Indicate that information is no longer usable
        posControl.flags.headingNewData = 0;
    }

    // Control yaw by NAV PID
    rcCommand[YAW] = constrain(rcAdjustment[YAW], -500, 500);
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
        if (navProfile->flags.lock_nav_until_takeoff) {
            if (navProfile->flags.use_midrc_for_althold) {
                if (rcCommand[THROTTLE] > (masterConfig.rxConfig.midrc + navProfile->nav_rc_deadband)) {
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
        if ((selectNavModeFromBoxModeInput() != NAV_MODE_NONE) && navProfile->flags.lock_nav_until_takeoff) { // && navProfile->flags.use_midrc_for_althold
            rcCommand[THROTTLE] = masterConfig.escAndServoConfig.minthrottle;
        }
        return;
    }

    // Apply navigation adjustments
    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
        if (navShouldApplyAltHold() && navIsThrustFacingDownwards(&inclination)) {
            applyAltitudeController(currentTime);
        }

        if (navShouldApplyPosHold() || navShouldApplyWaypoint() || navShouldApplyRTH()) {
            applyPositionController(currentTime);
        }

        if (navShouldApplyHeadingControl()) {
            applyHeadingController(currentTime);
        }
    }

    // Save processed throttle for future use
    lastAdjustedThrottle = rcCommand[THROTTLE];
}

/*-----------------------------------------------------------
 * NAV land detector
 *-----------------------------------------------------------*/
static bool isLandingDetected(bool resetDetector)
{
    static uint32_t landingConditionsNotSatisfiedTime;
    bool landingConditionsSatisfied = true;
    uint32_t currentTime = micros();

    if (resetDetector) {
        landingConditionsNotSatisfiedTime = currentTime;
        return false;
    }

    // land detector can not use the following sensors because they are unreliable during landing
    // calculated vertical velocity or altitude : poor barometer and large acceleration from ground impact, ground effect
    // earth frame angle or angle error :         landing on an uneven surface will force the airframe to match the ground angle
    // gyro output :                              on uneven surface the airframe may rock back an forth after landing
    // input throttle :                           in slow land the input throttle may be only slightly less than hover

    // TODO

    // Throttle should be less than 25%. We use lastAdjustedThrottle to keep track of NAV corrected throttle (isLandingDetected is executed
    // from processRx() and rcCommand holds rc input, not adjusted values from NAV core)
    if (lastAdjustedThrottle >= (masterConfig.escAndServoConfig.minthrottle + (masterConfig.escAndServoConfig.maxthrottle - masterConfig.escAndServoConfig.minthrottle) / 4)) {
        landingConditionsSatisfied = false;
    }

    // Average climb rate should be less than 20 cm/s
    if (fabsf(actualAverageVerticalVelocity) > 20) {
        landingConditionsSatisfied = false;
    }

    if (landingConditionsSatisfied) {
        if ((currentTime - landingConditionsNotSatisfiedTime) > LANDING_DETECTION_TIMEOUT) {
            return true;
        }
    }
    else {
        landingConditionsNotSatisfiedTime = currentTime;
    }

    return false;
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
    return navProfile->flags.throttle_tilt_comp && navShouldApplyAltHold();
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
                if (STATE(FIXED_WING)) { // FIXED_WING
                    // TODO
                }
                else {
                    // Check if previous mode was using ALTHOLD, re-use target altitude if necessary
                    if (navShouldApplyAltHold() && !navShouldApplyRTHAltitudeLogic()) {
                        // We were already applying ALTHOLD, don't update anything
                        setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_NONE);
                    }
                    else {
                        // Update desired Z-position to actual position
                        setAltHoldInitialThrottle(rcCommand[THROTTLE]);
                        setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);
                    }
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
                if (STATE(FIXED_WING)) { // FIXED_WING
                    // TODO
                }
                else {
                    // Depending on current navMode we can re-use target position and/or altitude
                    if (navShouldApplyAltHold() && !navShouldApplyRTHAltitudeLogic()) {
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
                        setAltHoldInitialThrottle(rcCommand[THROTTLE]);
                        if (navShouldApplyPosHold() && !navShouldApplyRTH()) {
                            // No Z-controlller yet, update it
                            setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);
                        }
                        else {
                            // Update XY and Z-controller target
                            setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING);
                        }
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
                navRthState = NAV_RTH_STATE_INIT;
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
            switch (navRthState) {
                case NAV_RTH_STATE_INIT:
                    if (posControl.homeDistance < navProfile->nav_min_rth_distance) {
                        // Prevent RTH jump in your face, when arming copter accidentally activating RTH (or RTH on failsafe)
                        // Inspired by CrashPilot1000's TestCode3
                        // Reset home to currect position
                        resetHomePosition();
                        // Set position lock on home and heading to original heading, altitude to current altitude
                        setDesiredPosition(&posControl.homeWaypoint.pos, posControl.homeWaypoint.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                        setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);
                        navRthState = NAV_RTH_STATE_HOME_AUTOLAND;
                    }
                    else {
                        // Climb to safe altitude if needed
                        if (posControl.actualState.pos.V.Z <= targetRTHAltitude) {
                            t_fp_vector rthAltPos;
                            rthAltPos.V.Z = targetRTHAltitude + 50.0f;
                            setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                            setDesiredPosition(&rthAltPos, 0, NAV_POS_UPDATE_Z);
                        }
                        navRthState = NAV_RTH_STATE_CLIMB_TO_SAVE_ALTITUDE;
                    }
                    break;
                case NAV_RTH_STATE_CLIMB_TO_SAVE_ALTITUDE:
                    if (posControl.actualState.pos.V.Z > targetRTHAltitude) {
                        // Set target position to home and calculate original bearing
                        setDesiredPosition(&posControl.homeWaypoint.pos, posControl.homeWaypoint.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_BEARING);
                        setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);
                        navRthState = NAV_RTH_STATE_HEAD_HOME;
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
                        navRthState = NAV_RTH_STATE_HOME_AUTOLAND;
                    }
                    else {
                        // TODO: a safeguard that would emergency land us in case of unexpected situation - battery low, etc
                    }
                    break;
                case NAV_RTH_STATE_HOME_AUTOLAND:
                    if (!ARMING_FLAG(ARMED)) {
                        navRthState = NAV_RTH_STATE_FINISHED;
                    }
                    else if (isLandingDetected(false)) {
                        navRthState = NAV_RTH_STATE_LANDED;
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
                    navRthState = NAV_RTH_STATE_FINISHED;
                    break;
                case NAV_RTH_STATE_FINISHED:
                    // Stay in this state forever
                    break;
            }
        }
        else if (posControl.mode == NAV_MODE_RTH_2D) {
            // 2D RTH state machine (when no ALTHOLD is available)
            switch (navRthState) {
                case NAV_RTH_STATE_INIT:
                    if (posControl.homeDistance < navProfile->nav_min_rth_distance) {
                        resetHomePosition();
                        setDesiredPosition(&posControl.homeWaypoint.pos, posControl.homeWaypoint.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                        navRthState = NAV_RTH_STATE_FINISHED;
                    }
                    else {
                        // In case of 2D RTH - head home immediately
                        setDesiredPosition(&posControl.homeWaypoint.pos, posControl.homeWaypoint.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_BEARING);
                        navRthState = NAV_RTH_STATE_HEAD_HOME;
                    }
                    break;
                case NAV_RTH_STATE_HEAD_HOME:
                    // Stay at this state until home reached
                    if (navIsWaypointReached(&posControl.homeWaypoint)) {
                        // Set position lock on home and heading to original heading when lauched
                        setDesiredPosition(&posControl.homeWaypoint.pos, posControl.homeWaypoint.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                        navRthState = NAV_RTH_STATE_FINISHED;
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
    navProfile = navProfileToUse;
}

void navigationUseBarometerConfig(barometerConfig_t * intialBarometerConfig)
{
    barometerConfig = intialBarometerConfig;
}

void navigationUseRcControlsConfig(rcControlsConfig_t *initialRcControlsConfig)
{
    rcControlsConfig = initialRcControlsConfig;
}

void navigationUsePIDs(pidProfile_t *initialPidProfile)
{
    int axis;

    pidProfile = initialPidProfile;

    // Initialize position hold PI-controller
    for (axis = 0; axis < 2; axis++) {
        pInit(&posControl.pids.pos[axis], (float)pidProfile->P8[PIDPOS] / 100.0f);

        pidInit(&posControl.pids.vel[axis], (float)pidProfile->P8[PIDPOSR] / 100.0f,
                                            (float)pidProfile->I8[PIDPOSR] / 100.0f,
                                            (float)pidProfile->D8[PIDPOSR] / 1000.0f,
                                            200.0);
    }

    // Initialize altitude hold PID-controllers (pos_z, vel_z, acc_z
    pInit(&posControl.pids.pos[Z], (float)pidProfile->P8[PIDALT] / 100.0f);

    pidInit(&posControl.pids.vel[Z], (float)pidProfile->I8[PIDALT] / 100.0f, 0, 0, 0);

    pidInit(&posControl.pids.accz, (float)pidProfile->P8[PIDVEL] / 100.0f,
                                   (float)pidProfile->I8[PIDVEL] / 100.0f,
                                   (float)pidProfile->D8[PIDVEL] / 1000.0f,
                                   300.0);

    // Heading PID (duplicates maghold)
    pInit(&posControl.pids.heading, (float)pidProfile->P8[PIDMAG] / 30.0f);
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
static float gpsVelocity[XYZ_AXIS_COUNT] = {0.0f, 0.0f, 0.0f};

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
    static int32_t previousAlt;

    static int32_t latFilterTable[3];
    static int32_t lonFilterTable[3];
    static int32_t altFilterTable[3];
    static int8_t  gpsFilterIndex;

    navLocation_t newLLH;
    t_fp_vector newPos;

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
        gpsVelocity[X] = (gpsVelocity[X] + (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * (newLat - previousLat) / dT)) / 2.0f;
        gpsVelocity[Y] = (gpsVelocity[Y] + (gpsScaleLonDown * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * (newLon - previousLon) / dT)) / 2.0f;
        gpsVelocity[Z] = (gpsVelocity[Z] + (newAlt - previousAlt) / dT) / 2.0f;

        // Update IMU velocities with complementary filter to keep them close to real velocities (as given by GPS)
        imuApplyFilterToActualVelocity(X, navProfile->nav_gps_cf, gpsVelocity[X]);
        imuApplyFilterToActualVelocity(Y, navProfile->nav_gps_cf, gpsVelocity[Y]);

        // Convert to local coordinates
        newLLH.lat = newLat;
        newLLH.lon = newLon;
        newLLH.alt = newAlt;
        navConvertGeodeticToLocal(&newLLH, &newPos);

        updateActualHorizontalPositionAndVelocity(newPos.V.X, newPos.V.Y, imuAverageVelocity.V.X, imuAverageVelocity.V.Y);
    }
    else {
        int i;

        // Initialize GPS velocity
        for (i = 0; i < 3; i++) {
            gpsVelocity[i] = 0.0f;
        }

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
        navConvertGeodeticToLocal(&newLLH, &newPos);

        updateActualHorizontalPositionAndVelocity(newPos.V.X, newPos.V.Y, 0, 0);
    }

    previousLat = newLat;
    previousLon = newLon;
    previousAlt = newAlt;

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
    static filterWithBufferSample_t baroClimbRateFilterBuffer[NAV_BARO_CLIMB_RATE_FILTER_SIZE];
    static filterWithBufferState_t baroClimbRateFilter;
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
        filterWithBufferInit(&baroClimbRateFilter, &baroClimbRateFilterBuffer[0], NAV_BARO_CLIMB_RATE_FILTER_SIZE);
        climbRateFiltersInitialized = true;
    }

#ifdef BARO
    // Calculate barometric altitude and climb rate
    // For NAV to work good baro altitude must not be delayed much. Large delay means slow response, means low PID gains to avoid oscillations
    // One should keep baro_tab_size small, but this will lead to high noise. NAV is OK with noisy measures, LPFs in altitude control
    // code and new CLT fusion will handle that just fine
    int32_t newBaroAlt = baroCalculateAltitude();
    float baroClimbRate;

    if (isBaroCalibrationComplete()) {
        filterWithBufferUpdate(&baroClimbRateFilter, newBaroAlt, currentTime);
        baroClimbRate = filterWithBufferApply_Derivative(&baroClimbRateFilter) * 1e6f;
    }
    else {
        newBaroAlt = 0;
        baroClimbRate = 0.0f;
    }
#else
    int32_t newBaroAlt = 0;
    baroClimbRate = 0.0f;
#endif

    baroClimbRate = constrainf(baroClimbRate, -1500, 1500);  // constrain baro velocity +/- 1500cm/s
    baroClimbRate = applyDeadband(baroClimbRate, 10);       // to reduce noise near zero

    // By using CF it's possible to correct the drift of integrated accZ (velocity) without loosing the phase, i.e without delay
    imuApplyFilterToActualVelocity(Z, barometerConfig->baro_cf_vel, baroClimbRate);

    updateActualAltitudeAndClimbRate(currentTime, newBaroAlt, imuAverageVelocity.V.Z);
}

#endif  // NAV
