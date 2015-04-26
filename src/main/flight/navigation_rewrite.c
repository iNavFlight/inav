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

#if !defined(NAV_HEADING_CONTROL_PID)
extern int16_t magHold;
#endif

// Current velocities in 3D space in global (NED) coordinate system
float actualAverageVerticalVelocity;    // average climb rate (updated every 250ms)

// Current position in 3D space for navigation purposes (may be different from GPS output)
static float gpsScaleLonDown = 1.0f;
static float sinNEDtoXYZ = 0.0f;   // rotation matrix from NED (GPS) to XYZ (IMU) frame of reference
static float cosNEDtoXYZ = 1.0f;

// Home & hold position
navPosition3D_t homePosition;      // Home position (NED coordinates)
static int16_t altholdInitialThrottle;  // Throttle input when althold was activated
static int16_t lastAdjustedThrottle = 0;

int32_t targetRTHAltitude;
uint32_t distanceToHome;    // Distance to launch point (meters)
int32_t directionToHome;    // Bearing to launch point (degrees)

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
int16_t navGPSVelocity[3];
int16_t navBaroVelocity;
int16_t navDesiredVelocity[3];
int16_t navLatestPositionError[3];
int16_t navActualHeading;
int16_t navDesiredHeading;
int16_t navThrottleAngleCorrection;
int16_t navTargetAltitude;
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

#if defined(NAV_HEADING_CONTROL_PID)
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
#endif

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
                targetRTHAltitude = posControl.actualState.pos.altitude;
                break;
            case NAX_RTH_EXTRA_ALT: // Maintain current altitude + predefined safety margin
                targetRTHAltitude = posControl.actualState.pos.altitude + navProfile->nav_rth_altitude;
                break;
            case NAV_RTH_CONST_ALT: // Climb to predefined altitude
                targetRTHAltitude = navProfile->nav_rth_altitude;
                break;
            case NAV_RTH_MAX_ALT:
                targetRTHAltitude = MAX(targetRTHAltitude, posControl.actualState.pos.altitude);
                break;
            default: // same as NAV_RTH_CONST_ALT
                targetRTHAltitude = navProfile->nav_rth_altitude;
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
static void updateActualHorizontalPositionAndVelocity(int32_t newLat, int32_t newLon, float newVelX, float newVelY)
{
    posControl.actualState.pos.coordinates[LAT] = newLat;
    posControl.actualState.pos.coordinates[LON] = newLon;

    posControl.actualState.vel[X] = newVelX;
    posControl.actualState.vel[Y] = newVelY;

#if defined(NAV_BLACKBOX)
    navLatestActualPosition[X] = newLat;
    navLatestActualPosition[Y] = newLon;
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

    posControl.actualState.pos.altitude = newAltitude;
    posControl.actualState.vel[Z] = newVelocity;

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
    posControl.actualState.pos.heading = newHeading;

    /* Pre-compute rotation matrix */
    sinNEDtoXYZ = sin_approx(posControl.actualState.pos.heading * RADX100);
    cosNEDtoXYZ = cos_approx(posControl.actualState.pos.heading * RADX100);

#if defined(NAV_BLACKBOX)
    navActualHeading = constrain(lrintf(posControl.actualState.pos.heading), -32678, 32767);
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
static void calculateDistanceAndBearingToDestination(navPosition3D_t *currentPos, navPosition3D_t *destinationPos, uint32_t *dist, int32_t *bearing)
{
    float dLatRAW = destinationPos->coordinates[LAT] - currentPos->coordinates[LAT];
    float dLonRAW = destinationPos->coordinates[LON] - currentPos->coordinates[LON];

    if (dist) {
        *dist = sqrtf(sq(dLatRAW) + sq(dLonRAW * gpsScaleLonDown)) * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
    }

    if (bearing) {
        float lat1RAD = currentPos->coordinates[LAT] * GPS_RAW_TO_RAD;
        float lat2RAD = destinationPos->coordinates[LAT] * GPS_RAW_TO_RAD;
        dLonRAW *= GPS_RAW_TO_RAD;

        float cosLat2RAD = cos_approx(lat2RAD);
        float y = sin_approx(dLonRAW) * cosLat2RAD;
        float x = cos_approx(lat1RAD) * sin_approx(lat2RAD) - sin_approx(lat1RAD) * cosLat2RAD * cos_approx(dLonRAW);
        *bearing = wrap_36000(constrain((int32_t)(atan2_approx(y, x) * TAN_89_99_DEGREES), -18000, 18000));
    }
}

/*-----------------------------------------------------------
 * Check if waypoint is/was reached
 *-----------------------------------------------------------*/
static bool navIsWaypointReached(navPosition3D_t *currentPos, navPosition3D_t *destinationPos)
{
    uint32_t wpDistance;

    // FIXME: Account for missed waypoints (passed on high speed)

    calculateDistanceAndBearingToDestination(currentPos, destinationPos, &wpDistance, NULL);

    // We consider waypoint reached if within specified radius
    return (wpDistance <= navProfile->nav_wp_radius);
}

/*-----------------------------------------------------------
 * Calculate desired heading/bearing, depending on NAV mode
 *-----------------------------------------------------------*/
static void calculateDesiredHeading(navPosition3D_t *currentPos, navPosition3D_t *destinationPos, float dTnav)
{
    UNUSED(dTnav);

    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
        // Depending on flight mode, we rotate out heading towards the next waypoint or keep it as it is
        if (navShouldKeepHeadingToBearing()) {
            // Navigating, rotate head towards next waypoint
            uint32_t wpDistance;
            int32_t wpBearing;

            calculateDistanceAndBearingToDestination(currentPos, destinationPos, &wpDistance, &wpBearing);

            // TODO: Apply crosstrack correction

            /*
            // Calculating cross track error, this tries to keep the copter on a direct line when flying to a waypoint.
            if (ABS(wrap_18000(wpBearing - originalWaypointBearing)) < 4500) {     // If we are too far off or too close we don't do track following
                float temp = (wpBearing - originalWaypointBearing) * RADX100;
                float crosstrackError = sinf(temp) * (wpDistance * CROSSTRACK_GAIN); // Meters we are off track line
                desiredBearing = wpBearing + constrain(crosstrackError, -3000, 3000);
                desiredBearing = wrap_36000(desiredBearing);
            } else {
                desiredBearing = wpBearing;
            }
            */

            posControl.desiredState.heading = wrap_36000(wpBearing);
        }
        else {
            // Keep the heading as it was rewuired when setting the destination
            posControl.desiredState.heading = wrap_36000(destinationPos->heading);
        }
    }

#if defined(NAV_BLACKBOX)
    navDesiredHeading = constrain(lrintf(posControl.desiredState.heading), -32678, 32767);
#endif
}

/*-----------------------------------------------------------
 * Reset home position to current position
 *-----------------------------------------------------------*/
void resetHomePosition(void)
{
    if (STATE(GPS_FIX) && GPS_numSat >= 5) {
        homePosition.coordinates[LON] = posControl.actualState.pos.coordinates[LON];
        homePosition.coordinates[LAT] = posControl.actualState.pos.coordinates[LAT];
        homePosition.altitude = posControl.actualState.pos.altitude;
        homePosition.heading = posControl.actualState.pos.heading;
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
        calculateDistanceAndBearingToDestination(&posControl.actualState.pos, &homePosition, &distanceToHome, &directionToHome);
        distanceToHome = distanceToHome / 100; // back to meters
        directionToHome = directionToHome / 100; // directionToHome should be degrees
    }
}

/*-----------------------------------------------------------
 * Set active XYZ-target and desired heading
 *-----------------------------------------------------------*/
void setNextWaypoint(navPosition3D_t * target, navSetWaypointFlags_t useMask)
{
    // XY-position
    if ((useMask & NAV_WP_XY) != 0) {
        posControl.desiredState.pos.coordinates[LAT] = target->coordinates[LAT];
        posControl.desiredState.pos.coordinates[LON] = target->coordinates[LON];
    }
    else {
        posControl.desiredState.pos.coordinates[LAT] = posControl.actualState.pos.coordinates[LAT];
        posControl.desiredState.pos.coordinates[LON] = posControl.actualState.pos.coordinates[LON];
    }

    // Z-position
    if ((useMask & NAV_WP_Z) != 0) {
        posControl.desiredState.pos.altitude = target->altitude;
    }
    else {
        posControl.desiredState.pos.altitude = posControl.actualState.pos.altitude;
    }

    // Heading
    if ((useMask & NAV_WP_HEADING) != 0) {
        // Heading
        posControl.desiredState.pos.heading = target->heading;
    }
    else if ((useMask & NAV_WP_BEARING) != 0) {
        int32_t wpBearing;
        calculateDistanceAndBearingToDestination(&posControl.actualState.pos, target, NULL, &wpBearing);
        posControl.desiredState.pos.heading = wpBearing;
    }
    else {
        posControl.desiredState.pos.heading = target->heading;
    }
}

#if defined(NAV_HEADING_CONTROL_PID)
/*-----------------------------------------------------------
 * Calculate rcAdjustment for YAW
 *-----------------------------------------------------------*/
static void calculateHeadingAdjustment(float dTnav)
{
    UNUSED(dTnav);

    // FIXME: Account for fixed-wing config without rudder (flying wing)

    // Calculate yaw correction
    int32_t headingError = wrap_18000(posControl.actualState.pos.heading - posControl.desiredState.heading) * masterConfig.yaw_control_direction;
    headingError = constrain(headingError, -3000, +3000); // limit error to +- 30 degrees to avoid fast rotation

    // FIXME: SMALL_ANGLE might prevent NAV from adjusting yaw when banking is too high (i.e. nav in high wind)
    if (STATE(SMALL_ANGLE)) {
        // Heading PID controller takes degrees, not centidegrees (this pid duplicates MAGHOLD)
        rcAdjustment[YAW] = (headingError / 100.0f) * posControl.pids.heading.param.kP;
    }
}
#endif

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

#if defined(NAV_HEADING_CONTROL_PID)
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
                    posControl.desiredState.pos.heading = posControl.actualState.pos.heading;
                }
            }
        }
    }
}
#endif

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
    // Calculate new altitude target
    posControl.desiredState.pos.altitude = lrintf(posControl.desiredState.pos.altitude + climbRate * US2S(deltaMicros));
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

    float altitudeError = posControl.desiredState.pos.altitude - posControl.actualState.pos.altitude;

    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
        // Use only P term for PH velocity calculation
        altitudeError = constrainf(altitudeError, -500, 500);
        //altitudeError = applyDeadband(altitudeError, 10); // remove small P parameter to reduce noise near zero position
        posControl.desiredState.vel[Z] = altitudeError * posControl.pids.pos[Z].param.kP;
        posControl.desiredState.vel[Z] = constrainf(posControl.desiredState.vel[Z], -300, 300); // hard limit velocity to +/- 3 m/s
    }

#if defined(NAV_BLACKBOX)
    navDesiredVelocity[Z] = constrain(lrintf(posControl.desiredState.vel[Z]), -32678, 32767);
    navLatestPositionError[Z] = constrain(lrintf(altitudeError), -32678, 32767);
    navTargetAltitude = constrain(lrintf(posControl.desiredState.pos.altitude), -32678, 32767);
#endif
}

static void updateAltitudeAccelController(uint32_t deltaMicros)
{
    static float velFilterState;

    // calculate rate error and filter with cut off frequency of 2 Hz
    float velError = posControl.desiredState.vel[Z] - posControl.actualState.vel[Z];
    velError = navApplyFilter(velError, NAV_VEL_ERROR_CUTOFF_FREQENCY_HZ, US2S(deltaMicros), &velFilterState);
    posControl.desiredState.acc[Z] = velError * posControl.pids.vel[Z].param.kP;
}

static void updateAltitudeThrottleController(uint32_t deltaMicros)
{
    static float throttleFilterState;
    float accError = posControl.desiredState.acc[Z] - imuAverageAcceleration[Z];

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
            // Gradually reduce descent speed depending on actual altitude. Descent from 20m should take about 50 seconds with default PIDs
            if (posControl.actualState.pos.altitude > 1000) {
                updateAltitudeTargetFromClimbRate(deltaMicrosPositionTargetUpdate, -100.0f);
            }
            else if (posControl.actualState.pos.altitude > 250) {
                updateAltitudeTargetFromClimbRate(deltaMicrosPositionTargetUpdate, -50.0f);
            }
            else {
                updateAltitudeTargetFromClimbRate(deltaMicrosPositionTargetUpdate, -20.0f);
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

            // Rotate these velocities from body frame to to earth frame
            float nedVelX = rcVelX * cosNEDtoXYZ - rcVelY * sinNEDtoXYZ;
            float nedVelY = rcVelX * sinNEDtoXYZ + rcVelY * cosNEDtoXYZ;

            // Calculate new position target
            posControl.desiredState.pos.coordinates[LAT] = lrintf(posControl.desiredState.pos.coordinates[LAT] + (nedVelX * US2S(deltaMicros) / DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR));
            posControl.desiredState.pos.coordinates[LON] = lrintf(posControl.desiredState.pos.coordinates[LON] + (nedVelY * US2S(deltaMicros) / (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * gpsScaleLonDown)));
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
                posControl.desiredState.pos.coordinates[LAT] = posControl.actualState.pos.coordinates[LAT];
                posControl.desiredState.pos.coordinates[LON] = posControl.actualState.pos.coordinates[LON];

                // When sticks are released we should restart PIDs
                pidReset(&posControl.pids.vel[X]);
                pidReset(&posControl.pids.vel[Y]);
            }
            else if (navShouldApplyWaypoint() || navShouldApplyRTH()) {
                pidReset(&posControl.pids.vel[X]);
                pidReset(&posControl.pids.vel[Y]);
            }
        }
    }
}

static void updatePositionVelocityController(uint32_t deltaMicros)
{
    UNUSED(deltaMicros);

    uint32_t wpDistance;

    float posErrorX = (posControl.desiredState.pos.coordinates[LAT] - posControl.actualState.pos.coordinates[LAT]) * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
    float posErrorY = (posControl.desiredState.pos.coordinates[LON] - posControl.actualState.pos.coordinates[LON]) * gpsScaleLonDown * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;

    calculateDistanceAndBearingToDestination(&posControl.actualState.pos, &posControl.desiredState.pos, &wpDistance, NULL);

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

        posControl.desiredState.vel[X] = newVelX;
        posControl.desiredState.vel[Y] = newVelY;
    }

#if defined(NAV_BLACKBOX)
    navDesiredVelocity[X] = constrain(lrintf(posControl.desiredState.vel[X]), -32678, 32767);
    navDesiredVelocity[Y] = constrain(lrintf(posControl.desiredState.vel[Y]), -32678, 32767);
    navLatestPositionError[X] = constrain(lrintf(posErrorX), -32678, 32767);
    navLatestPositionError[Y] = constrain(lrintf(posErrorY), -32678, 32767);
#endif
}

static void updatePositionAccelController(uint32_t deltaMicros, float maxAccelLimit)
{
    static float accFilterState[2];
    float velError, newAccel[2];

    // Calculate acceleration target on X-axis
    velError = constrainf(posControl.desiredState.vel[X] - posControl.actualState.vel[X], -500.0f, 500.0f); // limit error to 5 m/s
    newAccel[X] = pidGetPID(velError, US2S(deltaMicros), &posControl.pids.vel[X]);

#if defined(NAV_BLACKBOX)
    NAV_BLACKBOX_DEBUG(0, velError);
    NAV_BLACKBOX_DEBUG(1, posControl.pids.vel[X].lastP);
    NAV_BLACKBOX_DEBUG(2, posControl.pids.vel[X].lastI);
    NAV_BLACKBOX_DEBUG(3, posControl.pids.vel[X].lastD);
#endif

    // Calculate acceleration target on Y-axis
    velError = constrainf(posControl.desiredState.vel[Y] - posControl.actualState.vel[Y], -500.0f, 500.0f); // limit error to 5 m/s
    newAccel[Y] = pidGetPID(velError, US2S(deltaMicros), &posControl.pids.vel[Y]);

    // Check if required acceleration exceeds maximum allowed accel
    float newAccelTotal = sqrtf(sq(newAccel[X]) + sq(newAccel[Y]));

    // Recalculate acceleration
    if (newAccelTotal > maxAccelLimit) {
        newAccel[X] = maxAccelLimit * (newAccel[X] / newAccelTotal);
        newAccel[Y] = maxAccelLimit * (newAccel[Y] / newAccelTotal);
    }

    // Apply LPF to acceleration target
    posControl.desiredState.acc[X] = navApplyFilter(newAccel[X], NAV_ACCEL_CUTOFF_FREQUENCY_HZ, US2S(deltaMicros), &accFilterState[X]);
    posControl.desiredState.acc[Y] = navApplyFilter(newAccel[Y], NAV_ACCEL_CUTOFF_FREQUENCY_HZ, US2S(deltaMicros), &accFilterState[Y]);
}

static void updatePositionLeanAngleController(uint32_t deltaMicros)
{
    UNUSED(deltaMicros);

    // Rotate acceleration target into forward-right frame (aircraft)
    float accelForward = posControl.desiredState.acc[X] * cosNEDtoXYZ + posControl.desiredState.acc[Y] * sinNEDtoXYZ;
    float accelRight = -posControl.desiredState.acc[X] * sinNEDtoXYZ + posControl.desiredState.acc[Y] * cosNEDtoXYZ;

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

#if defined(NAV_HEADING_CONTROL_PID)
        // Zero adjustments
        calculateDesiredHeading(&posControl.actualState.pos, &posControl.desiredState.pos, dTnav);
        calculateHeadingAdjustment(dTnav);
        adjustHeadingFromRCInput();
#else
        calculateposControl.desiredState.heading(&posControl.actualState.pos, &posControl.desiredState.pos, dTnav);
#endif

        // Indicate that information is no longer usable
        posControl.flags.headingNewData = 0;
    }

#if defined(NAV_HEADING_CONTROL_PID)
    // Control yaw by NAV PID
    rcCommand[YAW] = constrain(rcAdjustment[YAW], -500, 500);
#else
    // Simply set heading for mag heading hold
    magHold = posControl.desiredState.heading / 100;
#endif
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
    bool canActivatePosHold = sensors(SENSOR_ACC) && (sensors(SENSOR_GPS) && STATE(GPS_FIX) && GPS_numSat >= 5) && (sensors(SENSOR_MAG) && persistentFlag(FLAG_MAG_CALIBRATION_DONE));

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
#if defined(NAV_HEADING_CONTROL_PID)
    return navShouldApplyHeadingControl();
#else
    return false;
#endif
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
                        setNextWaypoint(&posControl.desiredState.pos, NAV_WP_Z);
                    }
                    else {
                        setAltHoldInitialThrottle(rcCommand[THROTTLE]);
                        setNextWaypoint(&posControl.desiredState.pos, NAV_WP_NONE);
                    }
                }
                posControl.mode = NAV_MODE_ALTHOLD;
                break;
            case NAV_MODE_POSHOLD_2D:
                if (navShouldApplyPosHold() && !navShouldApplyRTH())
                    setNextWaypoint(&posControl.desiredState.pos, NAV_WP_XY | NAV_WP_HEADING);
                else
                    setNextWaypoint(&posControl.desiredState.pos, 0);
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
                            setNextWaypoint(&posControl.desiredState.pos, NAV_WP_XY | NAV_WP_Z | NAV_WP_HEADING);
                        }
                        else {
                            setNextWaypoint(&posControl.desiredState.pos, NAV_WP_Z);
                        }
                    }
                    else {
                        setAltHoldInitialThrottle(rcCommand[THROTTLE]);
                        if (navShouldApplyPosHold() && !navShouldApplyRTH()) {
                            setNextWaypoint(&posControl.desiredState.pos, NAV_WP_XY | NAV_WP_HEADING);
                        }
                        else {
                            setNextWaypoint(&posControl.desiredState.pos, 0);
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
                setNextWaypoint(&posControl.desiredState.pos, 0);
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
                    if (distanceToHome < (navProfile->nav_min_rth_distance / 100)) {
                        // Prevent RTH jump in your face, when arming copter accidentally activating RTH (or RTH on failsafe)
                        // Inspired by CrashPilot1000's TestCode3
                        // Reset home to currect position
                        resetHomePosition();
                        // Set position lock on home and heading to original heading
                        setNextWaypoint(&homePosition, NAV_WP_XY | NAV_WP_HEADING);
                        navRthState = NAV_RTH_STATE_HOME_AUTOLAND;
                    }
                    else {
                        // Climb to safe altitude if needed
                        if (posControl.actualState.pos.altitude <= targetRTHAltitude) {
                            navPosition3D_t rthAltPos;
                            rthAltPos.altitude = targetRTHAltitude + 50.0f;
                            setNextWaypoint(&rthAltPos, NAV_WP_Z);
                        }
                        navRthState = NAV_RTH_STATE_CLIMB_TO_SAVE_ALTITUDE;
                    }
                    break;
                case NAV_RTH_STATE_CLIMB_TO_SAVE_ALTITUDE:
                    if (posControl.actualState.pos.altitude > targetRTHAltitude) {
                        // Set target position to home and calculate original bearing
                        setNextWaypoint(&homePosition, NAV_WP_XY | NAV_WP_BEARING);
                        navRthState = NAV_RTH_STATE_HEAD_HOME;
                    }
                    else {
                        // TODO: a safeguard that would emergency land us if can not climb to desired altitude
                    }
                    break;
                case NAV_RTH_STATE_HEAD_HOME:
                    // Stay at this state until home reached
                    if (navIsWaypointReached(&posControl.actualState.pos, &homePosition)) {
                        // Set position lock on home and heading to original heading when lauched
                        setNextWaypoint(&homePosition, NAV_WP_XY | NAV_WP_HEADING);
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
                    if (distanceToHome < (navProfile->nav_min_rth_distance / 100)) {
                        resetHomePosition();
                        setNextWaypoint(&homePosition, NAV_WP_XY | NAV_WP_HEADING);
                        navRthState = NAV_RTH_STATE_FINISHED;
                    }
                    else {
                        // In case of 2D RTH - head home immediately
                        setNextWaypoint(&homePosition, NAV_WP_XY | NAV_WP_BEARING);
                        navRthState = NAV_RTH_STATE_HEAD_HOME;
                    }
                    break;
                case NAV_RTH_STATE_HEAD_HOME:
                    // Stay at this state until home reached
                    if (navIsWaypointReached(&posControl.actualState.pos, &homePosition)) {
                        // Set position lock on home and heading to original heading when lauched
                        setNextWaypoint(&homePosition, NAV_WP_XY | NAV_WP_HEADING);
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

#if defined(NAV_HEADING_CONTROL_PID)
    // Heading PID (duplicates maghold)
    pInit(&posControl.pids.heading, (float)pidProfile->P8[PIDMAG] / 30.0f);
#endif
}

void navigationInit(navProfile_t *initialNavProfile,
                    pidProfile_t *initialPidProfile,
                    barometerConfig_t *intialBarometerConfig,
                    rcControlsConfig_t *initialRcControlsConfig)
{
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
    return posControl.actualState.vel[axis];
}

float getEstimatedActualPosition(int axis)
{
    if (axis == Z)
        return posControl.actualState.pos.altitude;
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

    // Don't have a valid GPS 3D fix, do nothing and restart
    if (!(STATE(GPS_FIX) && GPS_numSat >= 5)) {
        isFirstUpdate = true;
        return;
    }

    uint32_t currentTime = micros();

    // this is used to offset the shrinking longitude as we go towards the poles
    gpsScaleLonDown = constrainf(cos_approx((ABS(newLat) / 10000000.0f) * 0.0174532925f), 0.01f, 1.0f);

    // If not first update - calculate velocities
    if (!isFirstUpdate) {
        float dT = US2S(getGPSDeltaTimeFilter(currentTime - previousTime));

        // Update filter table
        latFilterTable[gpsFilterIndex] = newLat;
        lonFilterTable[gpsFilterIndex] = newLon;
        altFilterTable[gpsFilterIndex] = newAlt;
        if (++gpsFilterIndex >= 3) gpsFilterIndex = 0;

        // Apply median filter
        newLat = quickMedianFilter3(latFilterTable);
        newLon = quickMedianFilter3(lonFilterTable);
        newAlt = quickMedianFilter3(altFilterTable);

        // Calculate NED velocities
        gpsVelocity[X] = (gpsVelocity[X] + (DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * (newLat - previousLat) / dT)) / 2.0f;
        gpsVelocity[Y] = (gpsVelocity[Y] + (gpsScaleLonDown * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * (newLon - previousLon) / dT)) / 2.0f;
        gpsVelocity[Z] = (gpsVelocity[Z] + (newAlt - previousAlt) / dT) / 2.0f;

#if defined(NAV_BLACKBOX)
        navGPSVelocity[X] = constrain(lrintf(gpsVelocity[X]), -32678, 32767);
        navGPSVelocity[Y] = constrain(lrintf(gpsVelocity[Y]), -32678, 32767);
        navGPSVelocity[Z] = constrain(lrintf(gpsVelocity[Z]), -32678, 32767);
#endif

        // Update IMU velocities with complementary filter to keep them close to real velocities (as given by GPS)
        imuApplyFilterToActualVelocity(X, navProfile->nav_gps_cf, gpsVelocity[X]);
        imuApplyFilterToActualVelocity(Y, navProfile->nav_gps_cf, gpsVelocity[Y]);

        // Update NAV
        updateActualHorizontalPositionAndVelocity(newLat, newLon, imuAverageVelocity[X], imuAverageVelocity[Y]);
        //updateActualHorizontalVelocity((imuAverageVelocity[X] + gpsVelocity[X]) * 0.5f, (imuAverageVelocity[Y] + gpsVelocity[Y]) * 0.5f);
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

        updateActualHorizontalPositionAndVelocity(newLat, newLon, 0, 0);
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

#if defined(NAV_BLACKBOX)
    navBaroVelocity = constrain(lrintf(baroClimbRate), -32678, 32767);
#endif

    updateActualAltitudeAndClimbRate(currentTime, newBaroAlt, imuAverageVelocity[Z]);
}

#endif  // NAV
