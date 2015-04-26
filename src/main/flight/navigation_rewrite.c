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

// Position hold / waypoint PIDs, take position error (cm) and output desired velocities (cm/s)
// NODE: Velocities are output in global frame of reference (NED coordinates)
static PID altitudePID;     // Used for all except ALTHOLD
static PID positionPID[2];  // Used for NAV_MODE_POSHOLD_2D, NAV_MODE_POSHOLD_3D, NAV_MODE_ASSISTED
                            // NAV_RTH & NAV_WP calculate desired speed without PID controller

#if defined(NAV_HEADING_CONTROL_PID)
// Heading PID
static PID headingRatePID;      // Takes heading error (deg) and outputs yaw rate
#else
extern int16_t magHold;
#endif

// NOTE: Input velocities are in *rotated* frame of reference (aircraft based, LON = roll axis, LAT = pitch axis)
static PID altitudeRatePID;         // Takes desired velocity (cm/s) and outputs raw control adjustment
static PID navigationRatePID[2];   // PID controller for WP and RTH
static PID posholdRatePID[2];  // PID controller for PH

// Current velocities in 3D space in global (NED) coordinate system
float actualVerticalVelocity;        // cm/s
float actualHorizontalVelocity[2];   // cm/s
float actualAverageVerticalVelocity;    // average climb rate (updated every 250ms)

// Throttle angle correction
static float throttleAngleCorrectionValue = 0;

// Current position in 3D space for navigation purposes (may be different from GPS output)
navPosition3D_t actualPosition;
static float gpsScaleLonDown = 1.0f;
static float sinNEDtoXYZ = 0.0f;   // rotation matrix from NED (GPS) to XYZ (IMU) frame of reference
static float cosNEDtoXYZ = 1.0f;

// Home & hold position
navPosition3D_t homePosition;      // Home position (NED coordinates)
static int16_t altholdInitialThrottle;  // Throttle input when althold was activated

// Automatic throttle tilt compensation (also hover throttle detection)
static float hoverThrottleAtZeroTilt = 0;
static bool hoverThrottleAtZeroTiltInitialized = false;
static int16_t lastAdjustedThrottle = 0;

uint32_t distanceToHome;
int32_t directionToHome;

// Current active waypoint and desired heading. For PH, ALTHOLD this is used for heading lock,
// For WP/RTH used for target bearing
navPosition3D_t activeWpOrHoldPosition;    // Used for WP/ALTHOLD/PH/RTH

// Desired velocities, might be set by pilot or by NAV position PIDs (NED coordinates)
static float desiredVerticalVel;
#if defined(NAV_3D)
static float desiredHorizontalVel[2];
static int32_t desiredHeading;
#endif

// Desired pitch/roll/yaw/throttle adjustments
static float lastAxisAdjustment[2] = {0, 0};
static int16_t rcAdjustment[4];

// Current navigation mode & profile
static bool navEnabled = false;
static navigationMode_e navMode = NAV_MODE_NONE;    // Navigation mode
static navProfile_t *navProfile;
static barometerConfig_t *barometerConfig;
static rcControlsConfig_t *rcControlsConfig;

#if defined(NAV_BLACKBOX)
int16_t navActualVelocity[3];
int16_t navGPSVelocity[2];
int16_t navDesiredVelocity[3];
int16_t navLatestPositionError[3];
int16_t navActualHeading;
int16_t navThrottleAngleCorrection;
int16_t navTargetAltitude;
#endif

/*-----------------------------------------------------------
 * PID implementation
 *-----------------------------------------------------------*/
static float pidGetP(float error, PID *pid)
{
    return error * pid->param.kP;
}

static float pidGetI(float error, float dt, PID *pid)
{
    pid->integrator += ((float)error * pid->param.kI) * dt;
    pid->integrator = constrainf(pid->integrator, -pid->param.Imax, pid->param.Imax);
    return pid->integrator;
}

static float pidGetD(float error, float dt, PID *pid)
{
    float pidFilter = 0.5f / (M_PIf * (float)navProfile->nav_lpf);
    pid->last_derivative += (dt / (pidFilter + dt)) * (((error - pid->last_error) / dt) - pid->last_derivative);
    pid->last_error = error;
    return pid->param.kD * pid->last_derivative;
}

static float pidGetPI(float error, float dt, PID *pid)
{
    return pidGetP(error, pid) + pidGetI(error, dt, pid);
}

static float pidGetPID(float error, float dt, PID *pid)
{
    return pidGetP(error, pid) + pidGetI(error, dt, pid) + pidGetD(error, dt, pid);
}

static void pidReset(PID *pid)
{
    pid->integrator = 0;
    pid->last_error = 0;
    pid->last_derivative = 0;
}

static void pidInit(PID *pid, float _kP, float _kI, float _kD, float _Imax)
{
    pid->param.kP = _kP;
    pid->param.kI = _kI;
    pid->param.kD = _kD;
    pid->param.Imax = _Imax;
    pidReset(pid);
}

/*-----------------------------------------------------------
 * Utilities
 *-----------------------------------------------------------*/
#if defined(NAV_3D)
#if defined(NAV_HEADING_CONTROL_PID)
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
#endif

#define DEGREES_80_IN_DECIDEGREES 800
bool navIsThrustFacingDownwards(rollAndPitchInclination_t *inclination)
{
    return ABS(inclination->values.rollDeciDegrees) < DEGREES_80_IN_DECIDEGREES && ABS(inclination->values.pitchDeciDegrees) < DEGREES_80_IN_DECIDEGREES;
}

void resetNavigation(void)
{
    int i;

    pidReset(&altitudePID);
    pidReset(&altitudeRatePID);

#if defined(NAV_3D)
#if defined(NAV_HEADING_CONTROL_PID)
    pidReset(&headingRatePID);
#endif

    for (i = 0; i < 2; i++) {
        pidReset(&navigationRatePID[i]);
        pidReset(&positionPID[i]);
        pidReset(&posholdRatePID[i]);
    }
#endif

    for (i = 0; i < 4; i++) {
        rcAdjustment[i] = 0;
    }

    lastAxisAdjustment[X] = 0.0f;
    lastAxisAdjustment[Y] = 0.0f;
}

/*-----------------------------------------------------------
 * NAV actual position calculations
 *  - position & altitude might be updated using different sensors,
 *    so we update them separately
 *-----------------------------------------------------------*/
#if defined(NAV_3D) 
static void updateActualHorizontalPosition(int32_t newLat, int32_t newLon)
{
    actualPosition.coordinates[LAT] = newLat;
    actualPosition.coordinates[LON] = newLon;
}

static void updateActualHorizontalVelocity(float newVelX, float newVelY)
{
    actualHorizontalVelocity[X] = newVelX;
    actualHorizontalVelocity[Y] = newVelY;

#if defined(NAV_BLACKBOX)
    navActualVelocity[X] = constrain(lrintf(newVelX), -32678, 32767);
    navActualVelocity[Y] = constrain(lrintf(newVelY), -32678, 32767);
#endif
}
#endif

#define AVERAGE_VERTICAL_VEL_INTERVAL   250000

static void updateActualAltitudeAndVelocity(float newAltitude, float newVelocity)
{
    static uint32_t averageVelocityLastUpdateTime = 0;

    static float averageVelocityAccumulator = 0;
    static uint32_t averageVelocitySampleCount = 1;

    averageVelocityAccumulator += newVelocity;
    averageVelocitySampleCount += 1;

    uint32_t currentTime = micros();
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

    actualPosition.altitude = newAltitude;
    actualVerticalVelocity = newVelocity;

#if defined(NAV_BLACKBOX)
    navActualVelocity[Z] = constrain(lrintf(newVelocity), -32678, 32767);
#endif
}

#if defined(NAV_3D)
static void updateActualHeading(int32_t newHeading)
{
    /* Update heading */
    actualPosition.heading = newHeading;

    /* Pre-compute rotation matrix */
    sinNEDtoXYZ = sinf(actualPosition.heading * RADX100);
    cosNEDtoXYZ = cosf(actualPosition.heading * RADX100);

#if defined(NAV_BLACKBOX)
    navActualHeading = constrain(lrintf(actualPosition.heading / 100), -32678, 32767);
#endif
}
#endif

/*-----------------------------------------------------------
 * NAV generic position control
 *-----------------------------------------------------------*/
// Takes current and previous position in centidegrees and calculates error in cm
static void calculatePositionError(navPosition3D_t *currentPos, navPosition3D_t *destinationPos, navPosition3D_t *error)
{
    error->coordinates[LON] = (destinationPos->coordinates[LON] - currentPos->coordinates[LON]) * gpsScaleLonDown * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
    error->coordinates[LAT] = (destinationPos->coordinates[LAT] - currentPos->coordinates[LAT]) * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR;
    error->altitude = destinationPos->altitude - currentPos->altitude;
}

 #if defined(NAV_3D)
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

        float cosLat2RAD = cosf(lat2RAD);
        float y = sinf(dLonRAW) * cosLat2RAD;
        float x = cosf(lat1RAD) * sinf(lat2RAD) - sinf(lat1RAD) * cosLat2RAD * cosf(dLonRAW);
        *bearing = wrap_36000(constrain((int32_t)(atan2f(y, x) * TAN_89_99_DEGREES), -18000, 18000));
    }
}

static bool navIsWaypointReached(navPosition3D_t *currentPos, navPosition3D_t *destinationPos)
{
    uint32_t wpDistance;

    calculateDistanceAndBearingToDestination(currentPos, destinationPos, &wpDistance, NULL);

    // We consider waypoint reached if within specified radius
    return (wpDistance <= navProfile->nav_wp_radius);
}

static void calculateDesiredHorizontalVelocity(navPosition3D_t *currentPos, navPosition3D_t *destinationPos, float dTnav)
{
    navPosition3D_t posError;
    uint32_t wpDistance;

    UNUSED(dTnav);

    // Calculate position error
    calculatePositionError(currentPos, destinationPos, &posError);
    calculateDistanceAndBearingToDestination(currentPos, destinationPos, &wpDistance, NULL);

    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
        // Algorithm depends on navigation mode (WP/RTH or PH)
        // We use PH PID governors if explicitly in NAV_MODE_POSHOLD or within 2*waypoint radius
        if (navShouldApplyPosHold() || (wpDistance < 2 * navProfile->nav_wp_radius)) {
            float newVelX = pidGetPI(posError.coordinates[LAT], dTnav, &positionPID[X]);
            float newVelY = pidGetPI(posError.coordinates[LON], dTnav, &positionPID[Y]);
            float newVel = sqrtf(sq(newVelX) + sq(newVelY));

            if (newVel > navProfile->nav_speed_max) {
                newVelX = newVelX * (navProfile->nav_speed_max / newVel);
                newVelY = newVelY * (navProfile->nav_speed_max / newVel);
            }

            desiredHorizontalVel[X] = newVelX;
            desiredHorizontalVel[Y] = newVelY;
        }
        else if (navShouldApplyWaypoint()) {
            float navCurrentSpeed = sqrtf(sq(actualHorizontalVelocity[X]) + sq(actualHorizontalVelocity[Y]));
            float targetSpeed = MIN(navProfile->nav_speed_max, wpDistance / 2.0f); // if close - navigate to reach a waypoint within 2 sec.

            // Avoid fast acceleration, increase speed in small steps
            if ((navCurrentSpeed + 50.0f) < targetSpeed) {
                targetSpeed = navCurrentSpeed + 50.0f;
            }

            targetSpeed = MAX(navProfile->nav_speed_min, targetSpeed);  // go at least min_speed

            // Calculate desired horizontal velocities
            desiredHorizontalVel[X] = targetSpeed * (posError.coordinates[LAT] / wpDistance);
            desiredHorizontalVel[Y] = targetSpeed * (posError.coordinates[LON] / wpDistance);

            // Reset PH PIDs
            pidReset(&positionPID[X]);
            pidReset(&positionPID[Y]);
        }
        else {
            desiredHorizontalVel[X] = 0;
            desiredHorizontalVel[Y] = 0;
        }
    }

#if defined(NAV_BLACKBOX)
    navDesiredVelocity[X] = constrain(lrintf(desiredHorizontalVel[X]), -32678, 32767);
    navDesiredVelocity[Y] = constrain(lrintf(desiredHorizontalVel[Y]), -32678, 32767);
    navLatestPositionError[X] = constrain(lrintf(posError.coordinates[LAT]), -32678, 32767);
    navLatestPositionError[Y] = constrain(lrintf(posError.coordinates[LON]), -32678, 32767);
#endif
}

static void calculateDesiredHeading(navPosition3D_t *currentPos, navPosition3D_t *destinationPos, float dTnav)
{
    UNUSED(dTnav);

    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
        // Depending on flight mode, we rotate out heading towards the next waypoint or keep it as it is
        if (navShouldAdjustHeading()) {
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

            desiredHeading = wrap_36000(wpBearing);
        }
        else {
            // Keep the heading as it was rewuired when setting the destination
            desiredHeading = wrap_36000(destinationPos->heading);
        }
    }
}
#endif

static void calculateDesiredVerticalVelocity(navPosition3D_t *currentPos, navPosition3D_t *destinationPos, float dTnav)
{
    navPosition3D_t posError;

    UNUSED(dTnav);

    // Calculate position error
    calculatePositionError(currentPos, destinationPos, &posError);

    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
        // Should calculate altitude hold if ALTHOLD, 3D poshold or navigation
        if (navShouldApplyAltHold()) {
            if (navIsThrustFacingDownwards(&inclination)) {
                // Use only P term for PH velocity calculation
                int32_t altitudeError = constrain(posError.altitude, -500, 500);
                altitudeError = applyDeadband(altitudeError, 10); // remove small P parameter to reduce noise near zero position
                desiredVerticalVel = pidGetP(altitudeError, &altitudePID);
                desiredVerticalVel = constrainf(desiredVerticalVel, -300, 300); // hard limit velocity to +/- 3 m/s
            }
            else {
                // don't apply altitude hold if flying upside down
                desiredVerticalVel = 0;
            }
        }
    }

#if defined(NAV_BLACKBOX)
    navDesiredVelocity[Z] = constrain(lrintf(desiredVerticalVel), -32678, 32767);
    navLatestPositionError[Z] = constrain(lrintf(posError.altitude), -32678, 32767);
    navTargetAltitude = constrain(lrintf(destinationPos->altitude), -32678, 32767);
#endif
}

/*-----------------------------------------------------------
 * NAV home position
 *-----------------------------------------------------------*/
#if defined(NAV_3D) 
void resetHomePosition(void)
{
    if (STATE(GPS_FIX) && GPS_numSat >= 5) {
        homePosition.coordinates[LON] = actualPosition.coordinates[LON];
        homePosition.coordinates[LAT] = actualPosition.coordinates[LAT];
        homePosition.altitude = actualPosition.altitude;
        homePosition.heading = actualPosition.heading;
        ENABLE_STATE(GPS_FIX_HOME);
    }
}

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
        calculateDistanceAndBearingToDestination(&actualPosition, &homePosition, &distanceToHome, &directionToHome);
        distanceToHome = distanceToHome / 100; // back to meters
        directionToHome = directionToHome / 100; // directionToHome should be degrees
    }
}
#endif

/*-----------------------------------------------------------
 * NAV set waypoint
 *-----------------------------------------------------------*/
void setNextWaypointAndHeadingLock(uint32_t lat, uint32_t lon, int32_t alt, int32_t head)
{
    activeWpOrHoldPosition.coordinates[LAT] = lat;
    activeWpOrHoldPosition.coordinates[LON] = lon;
    activeWpOrHoldPosition.altitude = alt;
    activeWpOrHoldPosition.heading = head;
}

#if defined(NAV_3D)
void setNextWaypointAndCalculateBearing(uint32_t lat, uint32_t lon, int32_t alt)
{
    int32_t wpBearing;

    activeWpOrHoldPosition.coordinates[LAT] = lat;
    activeWpOrHoldPosition.coordinates[LON] = lon;
    activeWpOrHoldPosition.altitude = alt;

    calculateDistanceAndBearingToDestination(&actualPosition, &activeWpOrHoldPosition, NULL, &wpBearing);
    activeWpOrHoldPosition.heading = wpBearing;
}
#endif

/*-----------------------------------------------------------
 * NAV attitude PID controllers
 *-----------------------------------------------------------*/
#if defined(NAV_3D)
static void calculateAttitudeAdjustment(float dTnav)
{
    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
        int axis;

        // rotate velocities into aircraft's frame of reference
        //float rotationAngle = (9000l - desiredBearing) * RADX100;
        float desiredAircraftVel[2];
        desiredAircraftVel[X] = desiredHorizontalVel[X] * cosNEDtoXYZ + desiredHorizontalVel[Y] * sinNEDtoXYZ;
        desiredAircraftVel[Y] = -desiredHorizontalVel[X] * sinNEDtoXYZ + desiredHorizontalVel[Y] * cosNEDtoXYZ;

        float actualAircraftVel[2];
        actualAircraftVel[X] = actualHorizontalVelocity[X] * cosNEDtoXYZ - actualHorizontalVelocity[Y] * sinNEDtoXYZ;
        actualAircraftVel[Y] = -actualHorizontalVelocity[X] * sinNEDtoXYZ + actualHorizontalVelocity[Y] * cosNEDtoXYZ;

        // Start with zero adjustments
        for (axis = 0; axis < 4; axis++) {
            rcAdjustment[axis] = 0;
        }

        // FIXME: Use normal PID for PH (remove d term zeroing on slow motion)

        // Now calculate pitch/roll adjustments to achieve desired velocities
        if (navShouldApplyPosHold() || navShouldApplyWaypoint()) {
            float axisAdjustment[2], error;
            axisAdjustment[X] = 0;
            axisAdjustment[Y] = 0;

            if (navShouldApplyPosHold()) {
                // Calculate pitch/roll
                for (axis = 0; axis < 2; axis++) {
                    error = constrainf(desiredAircraftVel[axis] - actualAircraftVel[axis], -500.0f, 500.0f); // limit error to 5 m/s

                    axisAdjustment[axis] = pidGetP(error, &posholdRatePID[axis]) +
                                           pidGetI(error, dTnav, &posholdRatePID[axis]);

                    float d = pidGetD(error, dTnav, &posholdRatePID[axis]);
                    d = constrainf(d, -2000.0f, 2000.0f);

                    // get rid of noise
                    if (ABS(actualHorizontalVelocity[axis]) < 10)
                        d = 0;

                    axisAdjustment[axis] += d;

                    navigationRatePID[axis].integrator = posholdRatePID[axis].integrator;
                }
            }
            else if (navShouldApplyWaypoint()) {
                // Calculate pitch/roll
                for (axis = 0; axis < 2; axis++) {
                    error = constrainf(desiredAircraftVel[axis] - actualAircraftVel[axis], -500.0f, 500.0f); // limit error to 5 m/s
                    axisAdjustment[axis] = pidGetPID(error, dTnav, &navigationRatePID[axis]);
                }
            }

            /* Apply NAV expo curve */
            // Ported over from CrashPilot1000's TestCode3
            if (navProfile->nav_expo) {
                float navExpo = constrain(navProfile->nav_expo, 0, 100) / 100.0f;

                for (axis = 0; axis < 2; axis++) {
                    // Apply some smoothing to attitude adjustments we make
                    axisAdjustment[axis] = (lastAxisAdjustment[axis] + axisAdjustment[axis]) * 0.5f;
                    lastAxisAdjustment[axis] = axisAdjustment[axis];

                    float tmp = axisAdjustment[axis] / 10.0f;
                    tmp = tmp / (float)NAV_ROLL_PITCH_MAX;
                    tmp = constrainf(tmp, -1.0f, 1.0f); // Put in range of -1 +1

                    axisAdjustment[axis] = (tmp * (1.0f - navExpo) + tmp * tmp * tmp * navExpo) * (float)NAV_ROLL_PITCH_MAX;
                }
            }

            /* X axis adjustment is actually pitch, Y adjustment is roll */
            rcAdjustment[PITCH] = axisAdjustment[X];
            rcAdjustment[ROLL] = axisAdjustment[Y];
        }

#if defined(NAV_HEADING_CONTROL_PID)
        // Calculate yaw correction
        if (navShouldApplyHeadingControl()) {
            int32_t headingError = wrap_18000((actualPosition.heading - desiredHeading) * 100);
            headingError *= masterConfig.yaw_control_direction;

            headingError = constrain(headingError, -3000, +3000); // limit error to +- 30 degrees to avoid fast rotation

            // FIXME: SMALL_ANGLE might prevent NAV from adjusting yaw when banking is too high (i.e. nav in high wind)
            if (STATE(SMALL_ANGLE)) {
                // Heading PID controller takes degrees, not centidegrees (this pid duplicates MAGHOLD)
                rcAdjustment[YAW] = pidGetP(headingError / 100.0f, &headingRatePID);
            }
        }
#endif
    }
}
#endif

#define NAV_THROTTLE_ANGLE_CORRECTION_CF    0.998f
#define NAV_THROTTLE_ANGLE_CORRECTION_MAX   150
static void calculateAndUpdateThrottleAngleCorrection(uint16_t throttle)
{
    // If we don't have a valid source of vertical speed for reference - don't calculate this
    if (!(sensors(SENSOR_BARO) || sensors(SENSOR_SONAR)))
        return;

    // We keep track of two values:
    //  1. level hover throttle, calculated when quad is hovering at a very small angle
    //  2. angle hover throttle, calculated when quad is flying as a constant altitude
    // Then we use these two values to calculate desired throttle_correction_value

    // If we are moving and/or accelerating by Z axis - don't calculate anything, current throttle value is useless
    if ((fabs(imuAverageAcceleration[Z]) >= 50) || fabs(imuAverageVelocity[Z]) >= 5)
        return;

    uint16_t tiltAngle = calculateTiltAngle();

    if (tiltAngle < 100) {
        if (hoverThrottleAtZeroTiltInitialized) {
            // Apply IIR LPF to throttle - smooth spikes and variances
            hoverThrottleAtZeroTilt = hoverThrottleAtZeroTilt * NAV_THROTTLE_ANGLE_CORRECTION_CF + 
                                      throttle * (1.0f - NAV_THROTTLE_ANGLE_CORRECTION_CF);
        }
        else {
            hoverThrottleAtZeroTilt = throttle;
            hoverThrottleAtZeroTiltInitialized = true;
        }
    }
    else if (tiltAngle < NAV_THROTTLE_CORRECTION_ANGLE) {
        if (hoverThrottleAtZeroTiltInitialized && (throttle > hoverThrottleAtZeroTilt)) {
            float newThrottleCoffection = calculateThrottleCorrectionValue(throttle - hoverThrottleAtZeroTilt, NAV_THROTTLE_CORRECTION_ANGLE);
            newThrottleCoffection = constrainf(newThrottleCoffection, 0, NAV_THROTTLE_ANGLE_CORRECTION_MAX);

            // Adjust this slowly
            throttleAngleCorrectionValue = throttleAngleCorrectionValue * NAV_THROTTLE_ANGLE_CORRECTION_CF + 
                                           newThrottleCoffection * (1.0f - NAV_THROTTLE_ANGLE_CORRECTION_CF);
#if defined(NAV_BLACKBOX)
            navThrottleAngleCorrection = constrain(lrintf(throttleAngleCorrectionValue), -32678, 32767);
#endif
        }
    }
}

static void calculateThrottleAdjustment(float dTnav)
{
    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
        if (navShouldApplyAltHold()) {
            if (navIsThrustFacingDownwards(&inclination)) {
                float error = desiredVerticalVel - actualVerticalVelocity;

                rcAdjustment[THROTTLE] = pidGetPID(error, dTnav, &altitudeRatePID);
                rcAdjustment[THROTTLE] = constrain(rcAdjustment[THROTTLE], -500, 500);

                if (navProfile->flags.throttle_tilt_comp) {
                    rcAdjustment[THROTTLE] += calculateThrottleAngleCorrection(throttleAngleCorrectionValue, NAV_THROTTLE_CORRECTION_ANGLE);
                }
            }
        }
    }
}

/*-----------------------------------------------------------
 * NAV throttle PID controllers
 *-----------------------------------------------------------*/
// FIXME: Make this configurable, default to about 5% highet than minthrottle
#define minFlyableThrottle  (masterConfig.escAndServoConfig.minthrottle + (masterConfig.escAndServoConfig.maxthrottle - masterConfig.escAndServoConfig.minthrottle) * 5 / 100)   
static void setAltHoldInitialThrottle(int16_t throttle)
{
    if (navProfile->flags.use_midrc_for_althold) {
        altholdInitialThrottle = masterConfig.rxConfig.midrc;
    }
    else {
        altholdInitialThrottle = throttle;
    }
}

static void adjustVerticalVelocityFromRCInput(void)
{
    // In some cases pilot has no control over flight direction
    if (!navCanAdjustVerticalVelocityFromRCInput())
        return;

    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
        int16_t rcThrottleAdjustment = applyDeadband(rcCommand[THROTTLE] - altholdInitialThrottle, rcControlsConfig->alt_hold_deadband);

        if (rcThrottleAdjustment) {
            // set velocity proportional to stick movement
            desiredVerticalVel = rcThrottleAdjustment * navProfile->nav_manual_speed_vertical / 500.0f;

            // We are changing altitude, apply new altitude hold setpoint
            activeWpOrHoldPosition.altitude = actualPosition.altitude;
        }
    }
}

/*-----------------------------------------------------------
 * NAV pilot's adjustments to attitude/throttle
 *-----------------------------------------------------------*/
#if defined(NAV_3D)
static void adjustHorizontalVelocityFromRCInput()
{
    // In some cases pilot has no control over flight direction
    if (!navCanAdjustHorizontalVelocityFromRCInput())
        return;

    if (STATE(FIXED_WING)) { // FIXED_WING
        // TODO
    }
    else { // MULTIROTOR
        int16_t rcPitchAdjustment = applyDeadband(rcCommand[PITCH], navProfile->nav_rc_deadband);
        int16_t rcRollAdjustment = applyDeadband(rcCommand[ROLL], navProfile->nav_rc_deadband);

        if (rcPitchAdjustment || rcRollAdjustment) {
            // Calculate desired velocities according to stick movement (copter frame of reference)
            float rcVelX = rcPitchAdjustment * navProfile->nav_manual_speed_horizontal / 500.0f;
            float rcVelY = rcRollAdjustment * navProfile->nav_manual_speed_horizontal / 500.0f;

            // Rotate these velocities from body frame to to earth frame
            desiredHorizontalVel[X] += rcVelX * cosNEDtoXYZ - rcVelY * sinNEDtoXYZ;
            desiredHorizontalVel[Y] += rcVelX * sinNEDtoXYZ + rcVelY * cosNEDtoXYZ;

            // If we are in position hold mode, so adjust poshold position
            // It will allow "sharded" control in WP/RTH mode while not messing up with target position
            if (navShouldApplyPosHold()) {
                activeWpOrHoldPosition.coordinates[LAT] = actualPosition.coordinates[LAT];
                activeWpOrHoldPosition.coordinates[LON] = actualPosition.coordinates[LON];
            }
        }
    }
}

#if defined(NAV_HEADING_CONTROL_PID)
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
                    activeWpOrHoldPosition.heading = actualPosition.heading;
                }
            }
        }
    }
}
#endif
#endif

/*-----------------------------------------------------------
 * NAV updates
 *-----------------------------------------------------------*/
static navigationMode_e selectNavModeFromBoxModeInput(void);
void applyWaypointNavigationAndAltitudeHold(void)
{
    static uint32_t previousTime;
    uint32_t currentTime = micros();
    float dTnav = (currentTime - previousTime) / 1e6;

    previousTime = currentTime;
    
    if (!ARMING_FLAG(ARMED)) {
        navEnabled = false;
        return;
    }

    if (!navEnabled) {
        if (navProfile->flags.lock_nav_until_takeoff) {
            if (navProfile->flags.use_midrc_for_althold) {
                if (rcCommand[THROTTLE] > (masterConfig.rxConfig.midrc + navProfile->nav_rc_deadband))
                    navEnabled = true;
            }
            else {
                if (rcCommand[THROTTLE] > minFlyableThrottle)
                    navEnabled = true;
            }
        }
        else {
            navEnabled = true;
        }
    }

    // If throttle low don't apply navigation either 
    if (!navEnabled) {
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
        // We do adjustments NAZA-style and think for pilot. In NAV mode pilot does not control the THROTTLE, PITCH and ROLL angles/rates directly,
        // except for a few navigation modes. Instead of that pilot controls velocities in 3D space.
        if (navShouldApplyAltHold()) {
            // Calculate desired vertical velocity & throttle adjustment
            calculateDesiredVerticalVelocity(&actualPosition, &activeWpOrHoldPosition, dTnav);
            adjustVerticalVelocityFromRCInput();
            calculateThrottleAdjustment(dTnav);

            // Apply rcAdjustment to throttle
            // FIXME: Add hover_throttle parameter and use it here instead of altholdInitialThrottle
            rcCommand[THROTTLE] = constrain(altholdInitialThrottle + rcAdjustment[THROTTLE], masterConfig.escAndServoConfig.minthrottle, masterConfig.escAndServoConfig.maxthrottle);
        }

#if defined(NAV_3D)
        if (navShouldApplyPosHold() || navShouldApplyWaypoint() || navShouldApplyHeadingControl()) {
            // Calculate PH/RTH/WP and attitude adjustment
            if (navShouldApplyPosHold() || navShouldApplyWaypoint()) {
                calculateDesiredHorizontalVelocity(&actualPosition, &activeWpOrHoldPosition, dTnav);
                adjustHorizontalVelocityFromRCInput();
            }

            // Apply rcAdjustment to yaw
            if (navShouldApplyHeadingControl()) {
                calculateDesiredHeading(&actualPosition, &activeWpOrHoldPosition, dTnav);
            }

            // Now correct desired velocities and heading to attitude corrections
            calculateAttitudeAdjustment(dTnav);

#if defined(NAV_HEADING_CONTROL_PID)
            // Check if YAW adjustment can be overridden by pilot
            adjustHeadingFromRCInput();
#endif

            // Apply rcAdjustment to pitch/roll
            if (navShouldApplyPosHold() || navShouldApplyWaypoint()) {
                rcCommand[PITCH] = constrain(rcAdjustment[PITCH], -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX);
                rcCommand[ROLL] = constrain(rcAdjustment[ROLL], -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX);
            }

            if (navShouldApplyHeadingControl()) {
#if defined(NAV_HEADING_CONTROL_PID)
                // Control yaw by NAV PID
                rcCommand[YAW] = constrain(rcAdjustment[YAW], -500, 500);
#else
                // Simply set heading for mag heading hold
                magHold = desiredHeading / 100;
#endif
            }
        }
#endif

        /* NAV has enough data to automatically calculate throttle angle correction - do it so if needed it would be ready */
        calculateAndUpdateThrottleAngleCorrection(rcCommand[THROTTLE]);
    }
    
    // Save processed throttle for future use
    lastAdjustedThrottle = rcCommand[THROTTLE];
}

/*-----------------------------------------------------------
 * NAV land detector
 *-----------------------------------------------------------*/
 #if defined(NAV_3D)
static bool isLandingDetected(void)
{
    static uint32_t landingConditionsNotSatisfiedTime;
    bool landingConditionsSatisfied = true;
    uint32_t currentTime = micros();

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
    if (fabs(actualAverageVerticalVelocity) > 20) {
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
#endif

/*-----------------------------------------------------------
 * NAV mode updates
 *-----------------------------------------------------------*/
void swithNavigationFlightModes(navigationMode_e navMode)
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
        case NAV_MODE_NONE:
        default:
            DISABLE_FLIGHT_MODE(NAV_ALTHOLD_MODE | NAV_POSHOLD_MODE | NAV_RTH_MODE | NAV_WP_MODE);
            break;
    }
}

static navigationMode_e selectNavModeFromBoxModeInput(void)
{
    // Flags if we can activate certain nav modes (check if we have required sensors and they provide valid data)
    bool canActivateAltHold = sensors(SENSOR_BARO) || sensors(SENSOR_SONAR);
#if defined(NAV_3D)
    bool canActivatePosHold = sensors(SENSOR_ACC) && (sensors(SENSOR_GPS) && STATE(GPS_FIX) && GPS_numSat >= 5) && (sensors(SENSOR_MAG) && persistentFlag(FLAG_MAG_CALIBRATION_DONE));
    bool canActivateRTHOrWP = canActivatePosHold && canActivateAltHold;
#endif

#if defined(NAV_3D)
    // Figure out, what mode pilot want to activate, also check if it is possible
    if (IS_RC_MODE_ACTIVE(BOXNAVRTH) && canActivateRTHOrWP && STATE(GPS_FIX_HOME)) {
        return NAV_MODE_RTH;
    }
    else if (IS_RC_MODE_ACTIVE(BOXNAVWP) && canActivateRTHOrWP) {
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
#else
    if (IS_RC_MODE_ACTIVE(BOXNAVALTHOLD) && canActivateAltHold) {
        return NAV_MODE_ALTHOLD;
    }
#endif

    return NAV_MODE_NONE;
}

bool navigationControlsThrottleAngleCorrection(void)
{
    return navProfile->flags.throttle_tilt_comp && navShouldApplyAltHold();
}

bool naivationRequiresAngleMode(void)
{
    return navShouldApplyPosHold() || navShouldApplyWaypoint();
}

#if defined(NAV_3D)
bool naivationControlsHeadingNow(void)
{
#if defined(NAV_HEADING_CONTROL_PID)
    return navShouldApplyHeadingControl();
#else
    return false;
#endif
}
#endif

void updateWaypointsAndNavigationMode(void)
{
#if defined(NAV_3D)
    static navRthState_t rthState;
#endif

    navigationMode_e newNavMode = NAV_MODE_NONE;

    if (navEnabled)
        newNavMode = selectNavModeFromBoxModeInput();

    // Process mode transition
    if (newNavMode != navMode) {
        if (navMode == NAV_MODE_NONE) {
            resetNavigation();
        }

        switch (newNavMode) {
            case NAV_MODE_ALTHOLD:
                if (STATE(FIXED_WING)) { // FIXED_WING
                    // TODO
                }
                else {
                    // Check if previous mode was using ALTHOLD, re-use target altitude if necessary
                    if (navShouldApplyAltHold()) {
                        setNextWaypointAndHeadingLock(actualPosition.coordinates[LAT], actualPosition.coordinates[LON], activeWpOrHoldPosition.altitude, actualPosition.heading);
                    }
                    else {
                        setAltHoldInitialThrottle(rcCommand[THROTTLE]);
                        setNextWaypointAndHeadingLock(actualPosition.coordinates[LAT], actualPosition.coordinates[LON], actualPosition.altitude, actualPosition.heading);
                    }
                }
                navMode = NAV_MODE_ALTHOLD;
                break;
#if defined(NAV_3D)
            case NAV_MODE_POSHOLD_2D:
                if (navShouldApplyPosHold())
                    setNextWaypointAndHeadingLock(activeWpOrHoldPosition.coordinates[LAT], activeWpOrHoldPosition.coordinates[LON], actualPosition.altitude, actualPosition.heading);
                else
                    setNextWaypointAndHeadingLock(activeWpOrHoldPosition.coordinates[LAT], activeWpOrHoldPosition.coordinates[LON], actualPosition.altitude, actualPosition.heading);
                navMode = NAV_MODE_POSHOLD_2D;
                break;
            case NAV_MODE_POSHOLD_3D:
                if (STATE(FIXED_WING)) { // FIXED_WING
                    // TODO
                }
                else {
                    // Depending on current navMode we can re-use target position and/or altitude
                    if (navShouldApplyAltHold()) {
                        if (navShouldApplyPosHold()) {
                            setNextWaypointAndHeadingLock(activeWpOrHoldPosition.coordinates[LAT], activeWpOrHoldPosition.coordinates[LON], activeWpOrHoldPosition.altitude, actualPosition.heading);
                        }
                        else {
                            setNextWaypointAndHeadingLock(actualPosition.coordinates[LAT], actualPosition.coordinates[LON], activeWpOrHoldPosition.altitude, actualPosition.heading);
                        }
                    }
                    else {
                        setAltHoldInitialThrottle(rcCommand[THROTTLE]);
                        if (navShouldApplyPosHold()) {
                            setNextWaypointAndHeadingLock(activeWpOrHoldPosition.coordinates[LAT], activeWpOrHoldPosition.coordinates[LON], actualPosition.altitude, actualPosition.heading);
                        }
                        else {
                            setNextWaypointAndHeadingLock(actualPosition.coordinates[LAT], actualPosition.coordinates[LON], actualPosition.altitude, actualPosition.heading);
                        }
                    }
                }

                navMode = NAV_MODE_POSHOLD_3D;
                break;
            case NAV_MODE_WP:
                // TODO
                navMode = NAV_MODE_NONE;
                break;
            case NAV_MODE_RTH:
                // We fix @ current position and climb to safe altitude
                setNextWaypointAndCalculateBearing(actualPosition.coordinates[LAT], actualPosition.coordinates[LON], actualPosition.altitude);
                navMode = NAV_MODE_RTH;
                rthState = NAV_RTH_STATE_INIT;
                break;
#endif
            default: // NAV_MODE_NONE
                resetNavigation();
                navMode = newNavMode;
                break;
        }
    }

    swithNavigationFlightModes(navMode);

#if defined(NAV_3D)
    // Process RTH state machine
    if (STATE(FIXED_WING)) { // FIXED_WING
        // FIXME: Multirotor only, fixed-wing logic must be different
    }
    else {
        if (navMode == NAV_MODE_RTH) {
            switch (rthState) {
                case NAV_RTH_STATE_INIT:
                    if (distanceToHome < navProfile->nav_min_rth_distance) {
                        // Prevent RTH jump in your face, when arming copter accidentally activating RTH (or RTH on failsafe)
                        // Inspired by CrashPilot1000's TestCode3
                        resetHomePosition();
                        rthState = NAV_RTH_STATE_HOME_AUTOLAND;
                    }
                    else {
                        // Climb to safe altitude if needed
                        if (actualPosition.altitude <= 1000) {
                            setNextWaypointAndHeadingLock(actualPosition.coordinates[LAT], actualPosition.coordinates[LON], 1000 + 50.0f, actualPosition.heading);
                        }
                        rthState = NAV_RTH_STATE_CLIMB_TO_SAVE_ALTITUDE;
                    }
                    break;
                case NAV_RTH_STATE_CLIMB_TO_SAVE_ALTITUDE:
                    if (actualPosition.altitude > 1000) {
                        setNextWaypointAndCalculateBearing(homePosition.coordinates[LAT], homePosition.coordinates[LON], actualPosition.altitude);
                        rthState = NAV_RTH_STATE_HEAD_HOME;
                    }
                    break;
                case NAV_RTH_STATE_HEAD_HOME:
                    // Stay at this state until home reached
                    if (navIsWaypointReached(&actualPosition, &homePosition)) {
                        rthState = NAV_RTH_STATE_HOME_AUTOLAND;
                    }
                    break;
                case NAV_RTH_STATE_HOME_AUTOLAND:
                    if (isLandingDetected() || ARMING_FLAG(ARMED)) {
                        rthState = NAV_RTH_STATE_LANDED;
                    }
                    else {
                        // Gradually reduce descent speed depending on actual altitude. Descent from 20m should take about 50 seconds with default PIDs
                        if (actualPosition.altitude > 1000) {
                            // Fast descent (altitude target below actual altitude, about 1m/s descent)
                            setNextWaypointAndHeadingLock(homePosition.coordinates[LAT], homePosition.coordinates[LON], actualPosition.altitude - 100.0f / altitudePID.param.kP, homePosition.heading);
                        }
                        else if (actualPosition.altitude > 250) {
                            // Medium descent (altitude target below actual altitude, about 50 cm/s descent)
                            setNextWaypointAndHeadingLock(homePosition.coordinates[LAT], homePosition.coordinates[LON], actualPosition.altitude - 50.0f / altitudePID.param.kP, homePosition.heading);
                        }
                        else {
                            // Slow descent (altitude target below actual altitude, about 10 cm/s descent)
                            setNextWaypointAndHeadingLock(homePosition.coordinates[LAT], homePosition.coordinates[LON], actualPosition.altitude - 10.0f / altitudePID.param.kP, homePosition.heading);
                        }
                    }
                    break;
                case NAV_RTH_STATE_LANDED:
                    // RTH is a non-normal flight mode. Engaging RTH likely means that pilot cannot or don't want to control aircraft.
                    // Craft in RTH mode should return home, land, disarm and lock out rearming to prevent accidental takeoff
                    ENABLE_ARMING_FLAG(PREVENT_ARMING);
                    mwDisarm();

                    rthState = NAV_RTH_STATE_FINISHED;
                    break;
                case NAV_RTH_STATE_FINISHED:
                    // Stay in this state forever
                    break;
            }
        }
    }
#endif
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

#define POSHOLD_IMAX           20       // degrees
#define POSHOLD_VEL_IMAX       20       // degrees
#define NAV_VEL_IMAX           20       // degrees
void navigationUsePIDs(pidProfile_t *pidProfile)
{
    int axis;

    // Initialize position hold PI-controller
    for (axis = 0; axis < 2; axis++) {
        pidInit(&positionPID[axis], (float)pidProfile->P8[PIDPOS] / 100.0f,
                                    (float)pidProfile->I8[PIDPOS] / 100.0f,
                                    0,
                                    POSHOLD_IMAX * 100.0);
    }

    // Initialize altitude hold P-controller
    pidInit(&altitudePID, (float)pidProfile->P8[PIDALT] / 10.0f,
                          (float)pidProfile->P8[PIDALT] / 100.0f,
                          (float)pidProfile->P8[PIDALT] / 1000.0f,
                          200.0);

    // Initialize vertical velocity PID-controller
    pidInit(&altitudeRatePID, (float)pidProfile->P8[PIDVEL] / 10.0f,
                              (float)pidProfile->I8[PIDVEL] / 1000.0f,
                              (float)pidProfile->D8[PIDVEL] / 1000.0f,
                              200.0);

    // Initialize horizontal velocity PID-controllers
    for (axis = 0; axis < 2; axis++) {
        pidInit(&navigationRatePID[axis], (float)pidProfile->P8[PIDNAVR] / 10.0f,
                                          (float)pidProfile->I8[PIDNAVR] / 100.0f,
                                          (float)pidProfile->D8[PIDNAVR] / 1000.0f,
                                          NAV_VEL_IMAX * 100.0);

        pidInit(&posholdRatePID[axis], (float)pidProfile->P8[PIDPOSR] / 10.0f,
                                           (float)pidProfile->I8[PIDPOSR] / 100.0f,
                                           (float)pidProfile->D8[PIDPOSR] / 1000.0f,
                                           NAV_VEL_IMAX * 100.0);
    }

#if defined(NAV_HEADING_CONTROL_PID)
    // Heading PID (duplicates maghold)
    pidInit(&headingRatePID, (float)pidProfile->P8[PIDMAG] / 30.0f, 0, 0, 0);
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
 * NAV data collection and pre-processing code
 * This is the largest sensor-dependent part of nav-rewrite.
 * Adding new sensors, implementing EKF, etc. should modify
 * this part of code and do not touch the above code (if possible)
 *-----------------------------------------------------------*/
#if defined(NAV_3D)
// Why is this here: Because GPS will be sending at quiet a nailed rate (if not overloaded by junk tasks at the brink of its specs)
// but we might read out with timejitter because Irq might be off by a few us so we do a +-10% margin around the time between GPS
// datasets representing the most common Hz-rates today. You might want to extend the list or find a smarter way.
// Don't overload your GPS in its config with trash, choose a Hz rate that it can deliver at a sustained rate.
// (c) CrashPilot1000
static float getGPSDeltaTimeFilter(uint32_t dTus)
{
    if (dTus >= 225000 && dTus <= 275000) return 1.0f / 4.0f;       //  4Hz Data 250ms
    if (dTus >= 180000 && dTus <= 220000) return 1.0f / 5.0f;       //  5Hz Data 200ms
    if (dTus >=  90000 && dTus <= 110000) return 1.0f / 10.0f;      // 10Hz Data 100ms
    if (dTus >=  45000 && dTus <=  55000) return 1.0f / 20.0f;      // 20Hz Data  50ms
    if (dTus >=  30000 && dTus <=  36000) return 1.0f / 30.0f;      // 30Hz Data  33ms
    if (dTus >=  23000 && dTus <=  27000) return 1.0f / 40.0f;      // 40Hz Data  25ms
    if (dTus >=  18000 && dTus <=  22000) return 1.0f / 50.0f;      // 50Hz Data  20ms
    return dTus * 1e-6;                                             // Filter failed. Set GPS Hz by measurement
}

void onNewGPSData(int32_t newLat, int32_t newLon)
{
    static uint32_t previousTime;
    static bool isFirstUpdate = true;
    static int32_t previousLat;
    static int32_t previousLon;

    float gpsVelocityX, gpsVelocityY;

    // Don't have a valid GPS 3D fix, do nothing
    if (!(STATE(GPS_FIX) && GPS_numSat >= 5)) {
        return;
    }

    uint32_t currentTime = micros();

    // If not first update - calculate velocities
    if (!isFirstUpdate) {
        float dT = getGPSDeltaTimeFilter(currentTime - previousTime);

        // this is used to offset the shrinking longitude as we go towards the poles
        gpsScaleLonDown = cosf((ABS(newLat) / 10000000.0f) * 0.0174532925f);

        // Calculate velocities based on GPS coordinates change
        gpsVelocityX = DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * (newLat - previousLat) / dT;
        gpsVelocityY = gpsScaleLonDown * DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR * (newLon - previousLon) / dT;

#if defined(NAV_BLACKBOX)
        navGPSVelocity[X] = constrain(lrintf(gpsVelocityX), -32678, 32767);
        navGPSVelocity[Y] = constrain(lrintf(gpsVelocityY), -32678, 32767);
#endif

        // Update IMU velocities with complementary filter to keep them close to real velocities (as given by GPS)
        // Rotate velocities from XYZ to NED coordinates
        imuApplyFilterToActualVelocity(X, navProfile->gps_cf_vel, gpsVelocityX);
        imuApplyFilterToActualVelocity(Y, navProfile->gps_cf_vel, gpsVelocityY);

        //updateActualHorizontalVelocity(gpsVelocityX, gpsVelocityY);
    }

    // Set previous coordinates
    previousLat = newLat;
    previousLon = newLon;
    isFirstUpdate = false;

    previousTime = currentTime;

    // Update NAV's position (from GPS) and velocity (from IMU)
    updateActualHorizontalPosition(newLat, newLon);

    //Update velocities 
    updateActualHorizontalVelocity(imuAverageVelocity[X], imuAverageVelocity[Y]);

    // Handle update of home position
    updateHomePosition();
}

void updateEstimatedPositionFromIMU(void)
{
    // TODO: calculate estimated position based on IMU

    // IMU velocity updates at more rate than GPS, update velocities only
    updateActualHorizontalVelocity(imuAverageVelocity[X], imuAverageVelocity[Y]);
}

void updateEstimatedHeading(void)
{
    // NAV uses heading in centidegrees
    updateActualHeading((int32_t)heading * 100);
}
#endif

// max 40hz update rate (20hz LPF on acc), seconds
#define BARO_UPDATE_FREQUENCY_40HZ (1.0f / 25)

// TODO: this is mostly ported from CF's original althold code, need cleaning up
void updateEstimatedAltitude(void)
{
    static uint32_t previousTime = 0;
    int32_t sonarAlt = -1;
    float sonarTransition;
    static int32_t baroAlt_offset = 0;
    int32_t baroVel;
    static float accAlt = 0.0f;
    static int32_t lastBaroAlt;

    // If we have baro and it is not ready - skip update
    if (sensors(SENSOR_BARO) && !isBaroReady())
        return;

    // We currently can use only BARO and SONAR as sources of altitude
    if (!(sensors(SENSOR_BARO) || sensors(SENSOR_SONAR)))
        return;

    uint32_t currentTime = micros();
    float dT = (currentTime - previousTime) * 1e-6;

    // too fast, likely no new data available
    if (dT < BARO_UPDATE_FREQUENCY_40HZ)
        return;

    previousTime = currentTime;

#ifdef BARO
    if (!isBaroCalibrationComplete()) {
        performBaroCalibrationCycle();
        accAlt = 0;
    }
    BaroAlt = baroCalculateAltitude();
#else
    BaroAlt = 0;
#endif

#ifdef SONAR
    sonarAlt = sonarCalculateAltitude(sonarAlt, calculateTiltAngle());
#endif

    // Use sonar up to 2/3 of maximum range, smoothly transit to baro if upper 1/3 sonar range
    if (sonarAlt > 0 && sonarAlt < (SONAR_MAX_RANGE * 2 / 3)) {
        baroAlt_offset = BaroAlt - sonarAlt;
        BaroAlt = sonarAlt;
    } else {
        BaroAlt -= baroAlt_offset;
        if (sonarAlt > (SONAR_MAX_RANGE * 2 / 3) && sonarAlt < SONAR_MAX_RANGE) {
            sonarTransition = (SONAR_MAX_RANGE - sonarAlt) / (SONAR_MAX_RANGE / 3);
            BaroAlt = sonarAlt * sonarTransition + BaroAlt * (1.0f - sonarTransition);
        }
    }

    // Integrator - Altitude in cm
    accAlt += (0.5f * imuAverageAcceleration[Z] * dT + imuAverageVelocity[Z]) * dT;  //  a * t^2 / 2 + v * t
    accAlt = accAlt * barometerConfig->baro_cf_alt + (float)BaroAlt * (1.0f - barometerConfig->baro_cf_alt);    // complementary filter for altitude estimation (baro & acc)

#ifdef BARO
    if (!isBaroCalibrationComplete()) {
        return;
    }
#endif

    if (sonarAlt > 0 && sonarAlt < (SONAR_MAX_RANGE * 2 / 3)) {
        // the sonar has the best range
        updateActualAltitudeAndVelocity(BaroAlt, imuAverageVelocity[Z]);
    } else {
        updateActualAltitudeAndVelocity(accAlt, imuAverageVelocity[Z]);
    }

    baroVel = (BaroAlt - lastBaroAlt) / dT;
    lastBaroAlt = BaroAlt;

    baroVel = constrain(baroVel, -1500, 1500);  // constrain baro velocity +/- 1500cm/s
    baroVel = applyDeadband(baroVel, 10);       // to reduce noise near zero

    // By using CF it's possible to correct the drift of integrated accZ (velocity) without loosing the phase, i.e without delay
    imuApplyFilterToActualVelocity(Z, barometerConfig->baro_cf_vel, baroVel);
}

#endif  // NAV
