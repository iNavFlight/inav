
#include "build/debug.h"

#include "common/maths.h"
#include "common/utils.h"

#include "fc/config.h"
#include "fc/fc_core.h"
#include "fc/runtime_config.h"

#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/wind_estimator.h"

#include "navigation/navigation.h"

#include "sensors/battery.h"

#include <stdint.h>

#if defined(USE_ADC) && defined(USE_GPS)

/* INPUTS:
 *   - forwardSpeed (same unit as horizontalWindSpeed)
 *   - heading degrees
 *   - horizontalWindSpeed (same unit as forwardSpeed)
 *   - windHeading degrees
 * OUTPUT:
 *   returns degrees
 */
static float windDriftCompensationAngle(float forwardSpeed, float heading, float horizontalWindSpeed, float windHeading) {
    return RADIANS_TO_DEGREES(asin_approx(-horizontalWindSpeed * sin_approx(DEGREES_TO_RADIANS(windHeading - heading)) / forwardSpeed));
}

/* INPUTS:
 *   - forwardSpeed (same unit as horizontalWindSpeed)
 *   - heading degrees
 *   - horizontalWindSpeed (same unit as forwardSpeed)
 *   - windHeading degrees
 * OUTPUT:
 *   returns (same unit as forwardSpeed and horizontalWindSpeed)
 */
static float windDriftCorrectedForwardSpeed(float forwardSpeed, float heading, float horizontalWindSpeed, float windHeading) {
    return forwardSpeed * cos_approx(DEGREES_TO_RADIANS(windDriftCompensationAngle(forwardSpeed, heading, horizontalWindSpeed, windHeading)));
}

/* INPUTS:
 *   - heading degrees
 *   - horizontalWindSpeed
 *   - windHeading degrees
 * OUTPUT:
 *   returns same unit as horizontalWindSpeed
 */
static float forwardWindSpeed(float heading, float horizontalWindSpeed, float windHeading) {
    return horizontalWindSpeed * cos_approx(DEGREES_TO_RADIANS(windHeading - heading));
}

/* INPUTS:
 *   - forwardSpeed (same unit as horizontalWindSpeed)
 *   - heading degrees
 *   - horizontalWindSpeed (same unit as forwardSpeed)
 *   - windHeading degrees
 * OUTPUT:
 *   returns (same unit as forwardSpeed and horizontalWindSpeed)
 */
static float windCompensatedForwardSpeed(float forwardSpeed, float heading, float horizontalWindSpeed, float windHeading) {
    return windDriftCorrectedForwardSpeed(forwardSpeed, heading, horizontalWindSpeed, windHeading) + forwardWindSpeed(heading, horizontalWindSpeed, windHeading);
}

// returns degrees
static int8_t RTHAltitudeChangePitchAngle(float altitudeChange) {
    return altitudeChange < 0 ? navConfig()->fw.max_dive_angle : -navConfig()->fw.max_climb_angle;
}

// altitudeChange is in meters
// idle_power and cruise_power are in deciWatt
// output is in Watt
static float estimateRTHAltitudeChangePower(float altitudeChange) {
    uint16_t altitudeChangeThrottle = navConfig()->fw.cruise_throttle - RTHAltitudeChangePitchAngle(altitudeChange) * navConfig()->fw.pitch_to_throttle;
    altitudeChangeThrottle = constrain(altitudeChangeThrottle, navConfig()->fw.min_throttle, navConfig()->fw.max_throttle);
    const float altitudeChangeThrToCruiseThrRatio = (float)(altitudeChangeThrottle - motorConfig()->minthrottle) / (navConfig()->fw.cruise_throttle - motorConfig()->minthrottle);
    return (float)heatLossesCompensatedPower(batteryMetersConfig()->idle_power + batteryMetersConfig()->cruise_power * altitudeChangeThrToCruiseThrRatio) / 100;
}

// altitudeChange is in m
// verticalWindSpeed is in m/s
// cruise_speed is in cm/s
// output is in seconds
static float estimateRTHAltitudeChangeTime(float altitudeChange, float verticalWindSpeed) {
    // Assuming increase in throttle keeps air speed at cruise speed
    const float estimatedVerticalSpeed = (float)navConfig()->fw.cruise_speed / 100 * sin_approx(DEGREES_TO_RADIANS(RTHAltitudeChangePitchAngle(altitudeChange))) + verticalWindSpeed;
    return altitudeChange / estimatedVerticalSpeed;
}

// altitudeChange is in m
// horizontalWindSpeed is in m/s
// windHeading is in degrees
// verticalWindSpeed is in m/s
// cruise_speed is in cm/s
// output is in meters
static float estimateRTHAltitudeChangeGroundDistance(float altitudeChange, float horizontalWindSpeed, float windHeading, float verticalWindSpeed) {
    // Assuming increase in throttle keeps air speed at cruise speed
    float estimatedHorizontalSpeed = (float)navConfig()->fw.cruise_speed / 100 * cos_approx(DEGREES_TO_RADIANS(RTHAltitudeChangePitchAngle(altitudeChange))) + forwardWindSpeed(DECIDEGREES_TO_DEGREES((float)attitude.values.yaw), horizontalWindSpeed, windHeading);
    return estimateRTHAltitudeChangeTime(altitudeChange, verticalWindSpeed) * estimatedHorizontalSpeed;
}

// altitudeChange is in m
// verticalWindSpeed is in m/s
// output is in Wh
static uint16_t estimateRTHAltitudeChangeEnergy(float altitudeChange, float verticalWindSpeed) {
    return estimateRTHAltitudeChangePower(altitudeChange) * estimateRTHAltitudeChangeTime(altitudeChange, verticalWindSpeed) / 3600;
}

// returns distance in m
// *heading is in degrees
static float estimateRTHDistanceAndHeadingAfterAltitudeChange(float altitudeChange, float horizontalWindSpeed, float windHeading, float verticalWindSpeed, float *heading) {
    float estimatedAltitudeChangeGroundDistance = estimateRTHAltitudeChangeGroundDistance(altitudeChange, horizontalWindSpeed, windHeading, verticalWindSpeed);
    if (navConfig()->general.flags.rth_climb_first && (altitudeChange > 0)) {
        float headingDiff = DEGREES_TO_RADIANS(DECIDEGREES_TO_DEGREES((float)attitude.values.yaw) - GPS_directionToHome);
        float triangleAltitude = GPS_distanceToHome * sin_approx(headingDiff);
        float triangleAltitudeToReturnStart = estimatedAltitudeChangeGroundDistance - GPS_distanceToHome * cos_approx(headingDiff);
        const float reverseHeadingDiff = RADIANS_TO_DEGREES(atan2_approx(triangleAltitude, triangleAltitudeToReturnStart));
        *heading = CENTIDEGREES_TO_DEGREES(wrap_36000(DEGREES_TO_CENTIDEGREES(180 + reverseHeadingDiff + DECIDEGREES_TO_DEGREES((float)attitude.values.yaw))));
        return sqrt(sq(triangleAltitude) + sq(triangleAltitudeToReturnStart));
    } else {
        *heading = GPS_directionToHome;
        return GPS_distanceToHome - estimatedAltitudeChangeGroundDistance;
    }
}

// returns mWh
static int32_t calculateRemainingEnergyBeforeRTH(bool takeWindIntoAccount) {
    // Fixed wing only for now
    if (!STATE(FIXED_WING))
        return -1;

    if (!(feature(FEATURE_VBAT) && feature(FEATURE_CURRENT_METER) && navigationPositionEstimateIsHealthy() && (batteryMetersConfig()->cruise_power > 0) && (ARMING_FLAG(ARMED)) && ((!STATE(FIXED_WING)) || (isNavLaunchEnabled() && isFixedWingLaunchDetected())) && (navConfig()->fw.cruise_speed > 0) && (currentBatteryProfile->capacity.unit == BAT_CAPACITY_UNIT_MWH) && (currentBatteryProfile->capacity.value > 0) && batteryWasFullWhenPluggedIn() && isImuHeadingValid()
#ifdef USE_WIND_ESTIMATOR
            && isEstimatedWindSpeedValid()
#endif
       ))
        return -1;

#ifdef USE_WIND_ESTIMATOR
    uint16_t windHeading; // centidegrees
    const float horizontalWindSpeed = takeWindIntoAccount ? getEstimatedHorizontalWindSpeed(&windHeading) / 100 : 0; // m/s
    const float windHeadingDegrees = CENTIDEGREES_TO_DEGREES((float)windHeading);
    const float verticalWindSpeed = getEstimatedWindSpeed(Z) / 100;
#else
    UNUSED(takeWindIntoAccount);
    const float horizontalWindSpeed = 0; // m/s
    const float windHeadingDegrees = 0;
    const float verticalWindSpeed = 0;
#endif

    const float RTH_altitude_change = (RTHAltitude() - getEstimatedActualPosition(Z)) / 100;
    float RTH_heading; // degrees
    const float RTH_distance = estimateRTHDistanceAndHeadingAfterAltitudeChange(RTH_altitude_change, horizontalWindSpeed, windHeadingDegrees, verticalWindSpeed, &RTH_heading);
    const float RTH_speed = windCompensatedForwardSpeed((float)navConfig()->fw.cruise_speed / 100, DECIDEGREES_TO_DEGREES(attitude.values.yaw), horizontalWindSpeed, windHeadingDegrees);

    DEBUG_SET(DEBUG_REM_FLIGHT_TIME, 0, lrintf(RTH_altitude_change * 100));
    DEBUG_SET(DEBUG_REM_FLIGHT_TIME, 1, lrintf(RTH_distance * 100));
    DEBUG_SET(DEBUG_REM_FLIGHT_TIME, 2, lrintf(RTH_speed * 100));
    DEBUG_SET(DEBUG_REM_FLIGHT_TIME, 3, lrintf(horizontalWindSpeed * 100));

    if (RTH_speed <= 0)
        return -2; // wind is too strong

    const uint32_t time_to_home = RTH_distance / RTH_speed; // seconds
    const uint32_t energy_to_home = estimateRTHAltitudeChangeEnergy(RTH_altitude_change, verticalWindSpeed) * 1000 + heatLossesCompensatedPower(batteryMetersConfig()->idle_power + batteryMetersConfig()->cruise_power) * time_to_home / 360; // mWh
    const uint32_t energy_margin_abs = (currentBatteryProfile->capacity.value - currentBatteryProfile->capacity.critical) * batteryMetersConfig()->rth_energy_margin / 100; // mWh
    const int32_t remaining_energy_before_rth = getBatteryRemainingCapacity() - energy_margin_abs - energy_to_home; // mWh

    if (remaining_energy_before_rth < 0) // No energy left = No time left
        return 0;

    return remaining_energy_before_rth;
}

// returns seconds
int32_t calculateRemainingFlightTimeBeforeRTH(bool takeWindIntoAccount) {

    const int32_t remainingEnergyBeforeRTH = calculateRemainingEnergyBeforeRTH(takeWindIntoAccount);

    // error: return error code directly
    if (remainingEnergyBeforeRTH < 0)
        return remainingEnergyBeforeRTH;

    const int32_t averagePower = calculateAveragePower();

    if (averagePower == 0)
        return -1;

    const uint32_t time_before_rth = remainingEnergyBeforeRTH * 360 / averagePower;

    if (time_before_rth > 0x7FFFFFFF) // int32 overflow
        return -1;

    return time_before_rth;
}

// returns meters
int32_t calculateRemainingDistanceBeforeRTH(bool takeWindIntoAccount) {

    const int32_t remainingFlightTimeBeforeRTH = calculateRemainingFlightTimeBeforeRTH(takeWindIntoAccount);

    // error: return error code directly
    if (remainingFlightTimeBeforeRTH < 0)
        return remainingFlightTimeBeforeRTH;

    return remainingFlightTimeBeforeRTH * calculateAverageSpeed();
}

#endif
