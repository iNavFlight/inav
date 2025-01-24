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

#include "platform.h"

#include "build/build_config.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/filter.h"

#include "drivers/time.h"

#include "sensors/sensors.h"
#include "sensors/acceleration.h"
#include "sensors/boardalignment.h"
#include "sensors/gyro.h"
#include "sensors/pitotmeter.h"

#include "flight/pid.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/mixer_profile.h"

#include "fc/config.h"
#include "fc/controlrate_profile.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "programming/logic_condition.h"

#include "rx/rx.h"

#include "sensors/battery.h"

// Base frequencies for smoothing pitch and roll
#define NAV_FW_BASE_PITCH_CUTOFF_FREQUENCY_HZ     2.0f
#define NAV_FW_BASE_ROLL_CUTOFF_FREQUENCY_HZ     10.0f

// If we are going slower than the minimum ground speed (navConfig()->general.min_ground_speed) - boost throttle to fight against the wind
#define NAV_FW_THROTTLE_SPEED_BOOST_GAIN        1.5f

// If this is enabled navigation won't be applied if velocity is below 3 m/s
//#define NAV_FW_LIMIT_MIN_FLY_VELOCITY

static bool isPitchAdjustmentValid = false;
static bool isRollAdjustmentValid = false;
static bool isYawAdjustmentValid = false;
static float throttleSpeedAdjustment = 0;
static bool isAutoThrottleManuallyIncreased = false;
static float navCrossTrackError;
static int8_t loiterDirYaw = 1;
bool needToCalculateCircularLoiter;

// Calculates the cutoff frequency for smoothing out roll/pitch commands
// control_smoothness valid range from 0 to 9
// resulting cutoff_freq ranging from baseFreq downwards to ~0.11Hz
static float getSmoothnessCutoffFreq(float baseFreq)
{
    uint16_t smoothness = 10 - navConfig()->fw.control_smoothness;
    return 0.001f * baseFreq * (float)(smoothness*smoothness*smoothness) + 0.1f;
}

// Calculates the cutoff frequency for smoothing out pitchToThrottleCorrection
// pitch_to_throttle_smooth valid range from 0 to 9
// resulting cutoff_freq ranging from baseFreq downwards to ~0.01Hz
static float getPitchToThrottleSmoothnessCutoffFreq(float baseFreq)
{
    uint16_t smoothness = 10 - navConfig()->fw.pitch_to_throttle_smooth;
    return 0.001f * baseFreq * (float)(smoothness*smoothness*smoothness) + 0.01f;
}

/*-----------------------------------------------------------
 * Altitude controller
 *-----------------------------------------------------------*/
void setupFixedWingAltitudeController(void)
{
    // TODO
}

void resetFixedWingAltitudeController(void)
{
    navPidReset(&posControl.pids.fw_alt);
    posControl.rcAdjustment[PITCH] = 0;
    isPitchAdjustmentValid = false;
    throttleSpeedAdjustment = 0;
}

bool adjustFixedWingAltitudeFromRCInput(void)
{
    int16_t rcAdjustment = applyDeadbandRescaled(rcCommand[PITCH], rcControlsConfig()->alt_hold_deadband, -500, 500);

    if (rcAdjustment) {
        // set velocity proportional to stick movement
        float rcClimbRate = -rcAdjustment * navConfig()->fw.max_manual_climb_rate / (500.0f - rcControlsConfig()->alt_hold_deadband);
        updateClimbRateToAltitudeController(rcClimbRate, 0, ROC_TO_ALT_CONSTANT);
        return true;
    }
    else {
        // Adjusting finished - reset desired position to stay exactly where pilot released the stick
        if (posControl.flags.isAdjustingAltitude) {
            updateClimbRateToAltitudeController(0, 0, ROC_TO_ALT_CURRENT);
        }
        return false;
    }
}

// Position to velocity controller for Z axis
static void updateAltitudeVelocityAndPitchController_FW(timeDelta_t deltaMicros)
{
    static pt1Filter_t velzFilterState;

    float desiredClimbRate = getDesiredClimbRate(posControl.desiredState.pos.z, deltaMicros);

    // Reduce max allowed climb rate by 2/3 if performing loiter (stall prevention)
    if (needToCalculateCircularLoiter && desiredClimbRate > 0.67f * navConfig()->fw.max_auto_climb_rate) {
        desiredClimbRate = 0.67f * navConfig()->fw.max_auto_climb_rate;
    }

    // Here we use negative values for dive for better clarity
    const float maxClimbDeciDeg = DEGREES_TO_DECIDEGREES(navConfig()->fw.max_climb_angle);
    const float minDiveDeciDeg = -DEGREES_TO_DECIDEGREES(navConfig()->fw.max_dive_angle);

    // PID controller to translate desired climb rate error into pitch angle [decideg]
    float currentClimbRate = navGetCurrentActualPositionAndVelocity()->vel.z;
    float targetPitchAngle = navPidApply2(&posControl.pids.fw_alt, desiredClimbRate, currentClimbRate, US2S(deltaMicros), minDiveDeciDeg, maxClimbDeciDeg, PID_DTERM_FROM_ERROR);

    // Apply low-pass filter to prevent rapid correction
    targetPitchAngle = pt1FilterApply4(&velzFilterState, targetPitchAngle, getSmoothnessCutoffFreq(NAV_FW_BASE_PITCH_CUTOFF_FREQUENCY_HZ), US2S(deltaMicros));

    // Reconstrain pitch angle (> 0 - climb, < 0 - dive)
    targetPitchAngle = constrainf(targetPitchAngle, minDiveDeciDeg, maxClimbDeciDeg);
    posControl.rcAdjustment[PITCH] = targetPitchAngle;

    posControl.desiredState.vel.z = desiredClimbRate;
    navDesiredVelocity[Z] = constrain(lrintf(posControl.desiredState.vel.z), -32678, 32767);
}

void applyFixedWingAltitudeAndThrottleController(timeUs_t currentTimeUs)
{
    static timeUs_t previousTimePositionUpdate = 0;         // Occurs @ altitude sensor update rate (max MAX_ALTITUDE_UPDATE_RATE_HZ)

    if ((posControl.flags.estAltStatus >= EST_USABLE)) {
        if (posControl.flags.verticalPositionDataNew) {
            const timeDeltaLarge_t deltaMicrosPositionUpdate = currentTimeUs - previousTimePositionUpdate;
            previousTimePositionUpdate = currentTimeUs;

            // Check if last correction was not too long ago
            if (deltaMicrosPositionUpdate < MAX_POSITION_UPDATE_INTERVAL_US) {
                updateAltitudeVelocityAndPitchController_FW(deltaMicrosPositionUpdate);
            }
            else {
                // Position update has not occurred in time (first iteration or glitch), reset altitude controller
                resetFixedWingAltitudeController();
            }

            // Indicate that information is no longer usable
            posControl.flags.verticalPositionDataConsumed = true;
        }

        isPitchAdjustmentValid = true;
    }
    else {
        // No valid altitude sensor data, don't adjust pitch automatically, rcCommand[PITCH] is passed through to PID controller
        isPitchAdjustmentValid = false;
    }
}

/*-----------------------------------------------------------
 * Adjusts desired heading from pilot's input
 *-----------------------------------------------------------*/
bool adjustFixedWingHeadingFromRCInput(void)
{
    if (ABS(rcCommand[YAW]) > rcControlsConfig()->pos_hold_deadband) {
        return true;
    }

    return false;
}

/*-----------------------------------------------------------
 * XY-position controller for multicopter aircraft
 *-----------------------------------------------------------*/
static fpVector3_t virtualDesiredPosition;
static pt1Filter_t fwPosControllerCorrectionFilterState;

/*
 * TODO Currently this function resets both FixedWing and Rover & Boat position controller
 */
void resetFixedWingPositionController(void)
{
    virtualDesiredPosition.x = 0;
    virtualDesiredPosition.y = 0;
    virtualDesiredPosition.z = 0;

    navPidReset(&posControl.pids.fw_nav);
    navPidReset(&posControl.pids.fw_heading);
    posControl.rcAdjustment[ROLL] = 0;
    posControl.rcAdjustment[YAW] = 0;
    isRollAdjustmentValid = false;
    isYawAdjustmentValid = false;

    pt1FilterReset(&fwPosControllerCorrectionFilterState, 0.0f);
}

static int8_t loiterDirection(void) {
    int8_t dir = 1; //NAV_LOITER_RIGHT

    if (navConfig()->fw.loiter_direction == NAV_LOITER_LEFT) {
        dir = -1;
    }

    if (navConfig()->fw.loiter_direction == NAV_LOITER_YAW) {

        if (rcCommand[YAW] < -250) {
            loiterDirYaw = 1; //RIGHT //yaw is contrariwise
        }

        if (rcCommand[YAW] > 250) {
            loiterDirYaw = -1; //LEFT  //see annexCode in fc_core.c
        }

        dir = loiterDirYaw;
    }

    if (IS_RC_MODE_ACTIVE(BOXLOITERDIRCHN)) {
        dir *= -1;
    }

#ifdef USE_GEOZONE
    if (geozone.loiterDir != 0) {
        dir = geozone.loiterDir;
    }
#endif

    return dir;
}

static void calculateVirtualPositionTarget_FW(float trackingPeriod)
{
    if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE) || posControl.navState == NAV_STATE_FW_LANDING_GLIDE || posControl.navState == NAV_STATE_FW_LANDING_FLARE) {
        return;
    }

    float posErrorX = posControl.desiredState.pos.x - navGetCurrentActualPositionAndVelocity()->pos.x;
    float posErrorY = posControl.desiredState.pos.y - navGetCurrentActualPositionAndVelocity()->pos.y;

    float distanceToActualTarget = calc_length_pythagorean_2D(posErrorX, posErrorY);

    // Limit minimum forward velocity to 1 m/s
    float trackingDistance = trackingPeriod * MAX(posControl.actualState.velXY, 100.0f);

    uint32_t navLoiterRadius = getLoiterRadius(navConfig()->fw.loiter_radius);
    fpVector3_t loiterCenterPos = posControl.desiredState.pos;
    int8_t loiterTurnDirection = loiterDirection();

    // Detemine if a circular loiter is required.
    // For waypoints only use circular loiter when angular visibility is > 30 degs, otherwise head straight toward target
    #define TAN_15DEG    0.26795f

    bool loiterApproachActive = isNavHoldPositionActive() &&
                                distanceToActualTarget <= (navLoiterRadius / TAN_15DEG) &&
                                distanceToActualTarget > 50.0f;
    needToCalculateCircularLoiter = loiterApproachActive || (navGetCurrentStateFlags() & NAV_CTL_HOLD);

    //if vtol landing is required, fly straight to homepoint
    if ((posControl.navState == NAV_STATE_RTH_HEAD_HOME) && navigationRTHAllowsLanding() && checkMixerATRequired(MIXERAT_REQUEST_LAND)){
        needToCalculateCircularLoiter = false;
    }

    /* WP turn smoothing with 2 options, 1: pass through WP, 2: cut inside turn missing WP
     * Works for turns > 30 degs and < 160 degs.
     * Option 1 switches to loiter path around waypoint using navLoiterRadius.
     * Loiter centered on point inside turn at required distance from waypoint and
     * on a bearing midway between current and next waypoint course bearings.
     * Option 2 simply uses a normal turn once the turn initiation point is reached */
    int32_t waypointTurnAngle = posControl.activeWaypoint.nextTurnAngle == -1 ? -1 : ABS(posControl.activeWaypoint.nextTurnAngle);
    posControl.flags.wpTurnSmoothingActive = false;
    if (waypointTurnAngle > 3000 && waypointTurnAngle < 16000 && isWaypointNavTrackingActive() && !needToCalculateCircularLoiter) {
        // turnStartFactor adjusts start of loiter based on turn angle
        float turnStartFactor;
        if (navConfig()->fw.wp_turn_smoothing == WP_TURN_SMOOTHING_ON) {     // passes through WP
            turnStartFactor = waypointTurnAngle / 6000.0f;
        } else {    // // cut inside turn missing WP
            turnStartFactor = constrainf(tan_approx(CENTIDEGREES_TO_RADIANS(waypointTurnAngle / 2.0f)), 1.0f, 2.0f);
        }
        // velXY provides additional turn initiation distance based on an assumed 1 second delayed turn response time
        if (posControl.wpDistance < (posControl.actualState.velXY + navLoiterRadius * turnStartFactor)) {
            if (navConfig()->fw.wp_turn_smoothing == WP_TURN_SMOOTHING_ON) {
                int32_t loiterCenterBearing = wrap_36000(((wrap_18000(posControl.activeWaypoint.nextTurnAngle - 18000)) / 2) + posControl.activeWaypoint.bearing + 18000);
                loiterCenterPos.x = posControl.activeWaypoint.pos.x + navLoiterRadius * cos_approx(CENTIDEGREES_TO_RADIANS(loiterCenterBearing));
                loiterCenterPos.y = posControl.activeWaypoint.pos.y + navLoiterRadius * sin_approx(CENTIDEGREES_TO_RADIANS(loiterCenterBearing));

                posErrorX = loiterCenterPos.x - navGetCurrentActualPositionAndVelocity()->pos.x;
                posErrorY = loiterCenterPos.y - navGetCurrentActualPositionAndVelocity()->pos.y;

                // turn direction to next waypoint
                loiterTurnDirection = posControl.activeWaypoint.nextTurnAngle > 0 ? 1 : -1;  // 1 = right

                needToCalculateCircularLoiter = true;
            }
            posControl.flags.wpTurnSmoothingActive = true;
        }
    }

    // We are closing in on a waypoint, calculate circular loiter if required
    if (needToCalculateCircularLoiter) {
        float loiterAngle = atan2_approx(-posErrorY, -posErrorX) + DEGREES_TO_RADIANS(loiterTurnDirection * 45.0f);
        float loiterTargetX = loiterCenterPos.x + navLoiterRadius * cos_approx(loiterAngle);
        float loiterTargetY = loiterCenterPos.y + navLoiterRadius * sin_approx(loiterAngle);

        // We have temporary loiter target. Recalculate distance and position error
        posErrorX = loiterTargetX - navGetCurrentActualPositionAndVelocity()->pos.x;
        posErrorY = loiterTargetY - navGetCurrentActualPositionAndVelocity()->pos.y;
        distanceToActualTarget = calc_length_pythagorean_2D(posErrorX, posErrorY);
    }

    // Calculate virtual waypoint
    virtualDesiredPosition.x = navGetCurrentActualPositionAndVelocity()->pos.x + posErrorX * (trackingDistance / distanceToActualTarget);
    virtualDesiredPosition.y = navGetCurrentActualPositionAndVelocity()->pos.y + posErrorY * (trackingDistance / distanceToActualTarget);

    // Shift position according to pilot's ROLL input (up to max_manual_speed velocity)
    if (posControl.flags.isAdjustingPosition) {
        int16_t rcRollAdjustment = applyDeadbandRescaled(rcCommand[ROLL], rcControlsConfig()->pos_hold_deadband, -500, 500);

        if (rcRollAdjustment) {
            float rcShiftY = rcRollAdjustment * navConfig()->general.max_manual_speed / 500.0f * trackingPeriod;

            // Rotate this target shift from body frame to to earth frame and apply to position target
            virtualDesiredPosition.x += -rcShiftY * posControl.actualState.sinYaw;
            virtualDesiredPosition.y +=  rcShiftY * posControl.actualState.cosYaw;
        }
    }
}

bool adjustFixedWingPositionFromRCInput(void)
{
    int16_t rcRollAdjustment = applyDeadbandRescaled(rcCommand[ROLL], rcControlsConfig()->pos_hold_deadband, -500, 500);
    return (rcRollAdjustment);
}

float processHeadingYawController(timeDelta_t deltaMicros, int32_t navHeadingError, bool errorIsDecreasing) {
    static float limit = 0.0f;

    if (limit == 0.0f) {
        limit = pidProfile()->navFwPosHdgPidsumLimit * 100.0f;
    }

    const pidControllerFlags_e yawPidFlags = errorIsDecreasing ? PID_SHRINK_INTEGRATOR : 0;

    const float yawAdjustment = navPidApply2(
        &posControl.pids.fw_heading,
        0,
        applyDeadband(navHeadingError, navConfig()->fw.yawControlDeadband * 100),
        US2S(deltaMicros),
        -limit,
        limit,
        yawPidFlags
        ) * 0.01f;

    DEBUG_SET(DEBUG_NAV_YAW, 0, posControl.pids.fw_heading.proportional);
    DEBUG_SET(DEBUG_NAV_YAW, 1, posControl.pids.fw_heading.integral);
    DEBUG_SET(DEBUG_NAV_YAW, 2, posControl.pids.fw_heading.derivative);
    DEBUG_SET(DEBUG_NAV_YAW, 3, navHeadingError);
    DEBUG_SET(DEBUG_NAV_YAW, 4, posControl.pids.fw_heading.output_constrained);

    return yawAdjustment;
}

static void updatePositionHeadingController_FW(timeUs_t currentTimeUs, timeDelta_t deltaMicros)
{
    static timeUs_t previousTimeMonitoringUpdate;
    static int32_t previousHeadingError;
    static bool errorIsDecreasing;
    static bool forceTurnDirection = false;
    int32_t virtualTargetBearing;

    if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE) || posControl.navState == NAV_STATE_FW_LANDING_GLIDE || posControl.navState == NAV_STATE_FW_LANDING_FLARE) {
        virtualTargetBearing = posControl.desiredState.yaw;
    } else {
        // We have virtual position target, calculate heading error
        virtualTargetBearing = calculateBearingToDestination(&virtualDesiredPosition);
    }

    if (isWaypointNavTrackingActive()) {
        /* Calculate cross track error */
        posControl.wpDistance = calculateDistanceToDestination(&posControl.activeWaypoint.pos);

        fpVector3_t virtualCoursePoint;
        virtualCoursePoint.x = posControl.activeWaypoint.pos.x -
                               posControl.wpDistance * cos_approx(CENTIDEGREES_TO_RADIANS(posControl.activeWaypoint.bearing));
        virtualCoursePoint.y = posControl.activeWaypoint.pos.y -
                               posControl.wpDistance * sin_approx(CENTIDEGREES_TO_RADIANS(posControl.activeWaypoint.bearing));
        navCrossTrackError = calculateDistanceToDestination(&virtualCoursePoint);

        /* If waypoint tracking enabled force craft toward and closely track along waypoint course line */
        if (navConfig()->fw.wp_tracking_accuracy && !needToCalculateCircularLoiter) {
            static float crossTrackErrorRate;
            static timeUs_t previousCrossTrackErrorUpdateTime;
            static float previousCrossTrackError = 0.0f;
            static pt1Filter_t fwCrossTrackErrorRateFilterState;

            if ((currentTimeUs - previousCrossTrackErrorUpdateTime) >= HZ2US(20) && fabsf(previousCrossTrackError - navCrossTrackError) > 10.0f) {
                const float crossTrackErrorDtSec =  US2S(currentTimeUs - previousCrossTrackErrorUpdateTime);
                if (fabsf(previousCrossTrackError - navCrossTrackError) < 500.0f) {
                    crossTrackErrorRate = (previousCrossTrackError - navCrossTrackError) / crossTrackErrorDtSec;
                }
                crossTrackErrorRate = pt1FilterApply4(&fwCrossTrackErrorRateFilterState, crossTrackErrorRate, 3.0f, crossTrackErrorDtSec);
                previousCrossTrackErrorUpdateTime = currentTimeUs;
                previousCrossTrackError = navCrossTrackError;
            }

            uint16_t trackingDeadband = METERS_TO_CENTIMETERS(navConfig()->fw.wp_tracking_accuracy);

            if ((ABS(wrap_18000(virtualTargetBearing - posControl.actualState.cog)) < 9000 || posControl.wpDistance < 1000.0f) && navCrossTrackError > trackingDeadband) {
                float adjustmentFactor = wrap_18000(posControl.activeWaypoint.bearing - virtualTargetBearing);
                uint16_t angleLimit = DEGREES_TO_CENTIDEGREES(navConfig()->fw.wp_tracking_max_angle);

                /* Apply heading adjustment to match crossTrackErrorRate with fixed convergence speed profile */
                float maxApproachSpeed = posControl.actualState.velXY * sin_approx(CENTIDEGREES_TO_RADIANS(angleLimit));
                float desiredApproachSpeed = constrainf(navCrossTrackError / 3.0f, 50.0f, maxApproachSpeed);
                adjustmentFactor = SIGN(adjustmentFactor) * navCrossTrackError * ((desiredApproachSpeed - crossTrackErrorRate) / desiredApproachSpeed);

                /* Calculate final adjusted virtualTargetBearing */
                uint16_t limit = constrainf(navCrossTrackError, 1000.0f, angleLimit);
                adjustmentFactor = constrainf(adjustmentFactor, -limit, limit);
                virtualTargetBearing = wrap_36000(posControl.activeWaypoint.bearing - adjustmentFactor);
            }
        }
    }
    /*
     * Calculate NAV heading error
     * Units are centidegrees
     */
    int32_t navHeadingError = wrap_18000(virtualTargetBearing - posControl.actualState.cog);

    // Forced turn direction
    // If heading error is close to 180 deg we initiate forced turn and only disable it when heading error goes below 90 deg
    if (ABS(navHeadingError) > 17000) {
        forceTurnDirection = true;
    }
    else if (ABS(navHeadingError) < 9000 && forceTurnDirection) {
        forceTurnDirection = false;
    }

    // If forced turn direction flag is enabled we fix the sign of the direction
    if (forceTurnDirection) {
        navHeadingError = loiterDirection() * ABS(navHeadingError);
    }

    // Slow error monitoring (2Hz rate)
    if ((currentTimeUs - previousTimeMonitoringUpdate) >= HZ2US(NAV_FW_CONTROL_MONITORING_RATE)) {
        // Check if error is decreasing over time
        errorIsDecreasing = (ABS(previousHeadingError) > ABS(navHeadingError));

        // Save values for next iteration
        previousHeadingError = navHeadingError;
        previousTimeMonitoringUpdate = currentTimeUs;
    }

    // Only allow PID integrator to shrink if error is decreasing over time
    const pidControllerFlags_e pidFlags = PID_DTERM_FROM_ERROR | (errorIsDecreasing ? PID_SHRINK_INTEGRATOR : 0);

    // Input error in (deg*100), output roll angle (deg*100)
    float rollAdjustment = navPidApply2(&posControl.pids.fw_nav, posControl.actualState.cog + navHeadingError, posControl.actualState.cog, US2S(deltaMicros),
                                       -DEGREES_TO_CENTIDEGREES(navConfig()->fw.max_bank_angle),
                                        DEGREES_TO_CENTIDEGREES(navConfig()->fw.max_bank_angle),
                                        pidFlags);

    // Apply low-pass filter to prevent rapid correction
    rollAdjustment = pt1FilterApply4(&fwPosControllerCorrectionFilterState, rollAdjustment, getSmoothnessCutoffFreq(NAV_FW_BASE_ROLL_CUTOFF_FREQUENCY_HZ), US2S(deltaMicros));

    // Convert rollAdjustment to decidegrees (rcAdjustment holds decidegrees)
    posControl.rcAdjustment[ROLL] = CENTIDEGREES_TO_DECIDEGREES(rollAdjustment);

    /*
     * Yaw adjustment
     * It is working in relative mode and we aim to keep error at zero
     */
    if (STATE(FW_HEADING_USE_YAW)) {
        posControl.rcAdjustment[YAW] = processHeadingYawController(deltaMicros, navHeadingError, errorIsDecreasing);
    } else {
        posControl.rcAdjustment[YAW] = 0;
    }
}

void applyFixedWingPositionController(timeUs_t currentTimeUs)
{
    static timeUs_t previousTimePositionUpdate = 0;         // Occurs @ GPS update rate

    // Apply controller only if position source is valid. In absence of valid pos sensor (GPS loss), we'd stick in forced ANGLE mode
    if ((posControl.flags.estPosStatus >= EST_USABLE)) {
        // If we have new position - update velocity and acceleration controllers
        if (posControl.flags.horizontalPositionDataNew) {
            const timeDeltaLarge_t deltaMicrosPositionUpdate = currentTimeUs - previousTimePositionUpdate;
            previousTimePositionUpdate = currentTimeUs;

            if (deltaMicrosPositionUpdate < MAX_POSITION_UPDATE_INTERVAL_US) {
                // Calculate virtual position target at a distance of forwardVelocity * HZ2S(POSITION_TARGET_UPDATE_RATE_HZ)
                // Account for pilot's roll input (move position target left/right at max of max_manual_speed)
                // POSITION_TARGET_UPDATE_RATE_HZ should be chosen keeping in mind that position target shouldn't be reached until next pos update occurs
                // FIXME: verify the above
                calculateVirtualPositionTarget_FW(HZ2S(MIN_POSITION_UPDATE_RATE_HZ) * 2);
                updatePositionHeadingController_FW(currentTimeUs, deltaMicrosPositionUpdate);
                needToCalculateCircularLoiter = false;
            }
            else {
                // Position update has not occurred in time (first iteration or glitch), reset altitude controller
                resetFixedWingPositionController();
            }

            // Indicate that information is no longer usable
            posControl.flags.horizontalPositionDataConsumed = true;
        }

        isRollAdjustmentValid = true;
        isYawAdjustmentValid = true;
    }
    else {
        // No valid pos sensor data, don't adjust pitch automatically, rcCommand[ROLL] is passed through to PID controller
        isRollAdjustmentValid = false;
        isYawAdjustmentValid = false;
    }
}

int16_t applyFixedWingMinSpeedController(timeUs_t currentTimeUs)
{
    static timeUs_t previousTimePositionUpdate = 0;         // Occurs @ GPS update rate

    // Apply controller only if position source is valid
    if ((posControl.flags.estPosStatus >= EST_USABLE)) {
        // If we have new position - update velocity and acceleration controllers
        if (posControl.flags.horizontalPositionDataNew) {
            const timeDeltaLarge_t deltaMicrosPositionUpdate = currentTimeUs - previousTimePositionUpdate;
            previousTimePositionUpdate = currentTimeUs;

            if (deltaMicrosPositionUpdate < MAX_POSITION_UPDATE_INTERVAL_US) {
                float velThrottleBoost = ((navConfig()->general.min_ground_speed * 100.0f) - posControl.actualState.velXY) * NAV_FW_THROTTLE_SPEED_BOOST_GAIN * US2S(deltaMicrosPositionUpdate);

                // If we are in the deadband of 50cm/s - don't update speed boost
                if (fabsf(posControl.actualState.velXY - (navConfig()->general.min_ground_speed * 100.0f)) > 50) {
                    throttleSpeedAdjustment += velThrottleBoost;
                }

                throttleSpeedAdjustment = constrainf(throttleSpeedAdjustment, 0.0f, 500.0f);
            }
            else {
                // Position update has not occurred in time (first iteration or glitch), reset altitude controller
                throttleSpeedAdjustment = 0;
            }

            // Indicate that information is no longer usable
            posControl.flags.horizontalPositionDataConsumed = true;
        }
    }
    else {
        // No valid pos sensor data, we can't calculate speed
        throttleSpeedAdjustment = 0;
    }

    return throttleSpeedAdjustment;
}

int16_t fixedWingPitchToThrottleCorrection(int16_t pitch, timeUs_t currentTimeUs)
{
    static timeUs_t previousTimePitchToThrCorr = 0;
    const timeDeltaLarge_t deltaMicrosPitchToThrCorr = currentTimeUs -  previousTimePitchToThrCorr;
    previousTimePitchToThrCorr = currentTimeUs;

    static pt1Filter_t pitchToThrFilterState;

    // Apply low-pass filter to pitch angle to smooth throttle correction
    int16_t filteredPitch = (int16_t)pt1FilterApply4(&pitchToThrFilterState, pitch, getPitchToThrottleSmoothnessCutoffFreq(NAV_FW_BASE_PITCH_CUTOFF_FREQUENCY_HZ), US2S(deltaMicrosPitchToThrCorr));

    int16_t pitchToThrottle = currentBatteryProfile->nav.fw.pitch_to_throttle;

#ifdef USE_FW_AUTOLAND
    if (pitch < 0 && posControl.fwLandState.landState == FW_AUTOLAND_STATE_FINAL_APPROACH) {
        pitchToThrottle *= navFwAutolandConfig()->finalApproachPitchToThrottleMod / 100.0f;
    }
#endif

    if (ABS(pitch - filteredPitch) > navConfig()->fw.pitch_to_throttle_thresh) {
        // Unfiltered throttle correction outside of pitch deadband
        return DECIDEGREES_TO_DEGREES(pitch) * pitchToThrottle;
    }
    else {
        // Filtered throttle correction inside of pitch deadband
        return DECIDEGREES_TO_DEGREES(filteredPitch) * pitchToThrottle;
    }
}

void applyFixedWingPitchRollThrottleController(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs)
{
    int16_t minThrottleCorrection = currentBatteryProfile->nav.fw.min_throttle - currentBatteryProfile->nav.fw.cruise_throttle;
    int16_t maxThrottleCorrection = currentBatteryProfile->nav.fw.max_throttle - currentBatteryProfile->nav.fw.cruise_throttle;

    if (isRollAdjustmentValid && (navStateFlags & NAV_CTL_POS)) {
        // ROLL >0 right, <0 left
        int16_t rollCorrection = constrain(posControl.rcAdjustment[ROLL], -DEGREES_TO_DECIDEGREES(navConfig()->fw.max_bank_angle), DEGREES_TO_DECIDEGREES(navConfig()->fw.max_bank_angle));
        rcCommand[ROLL] = pidAngleToRcCommand(rollCorrection, pidProfile()->max_angle_inclination[FD_ROLL]);
    }

    if (isYawAdjustmentValid && (navStateFlags & NAV_CTL_POS)) {
        rcCommand[YAW] = posControl.rcAdjustment[YAW];
    }

    if (isPitchAdjustmentValid && (navStateFlags & NAV_CTL_ALT)) {
        // PITCH >0 dive, <0 climb
        int16_t pitchCorrection = constrain(posControl.rcAdjustment[PITCH], -DEGREES_TO_DECIDEGREES(navConfig()->fw.max_dive_angle), DEGREES_TO_DECIDEGREES(navConfig()->fw.max_climb_angle));
        rcCommand[PITCH] = -pidAngleToRcCommand(pitchCorrection, pidProfile()->max_angle_inclination[FD_PITCH]);
        int16_t throttleCorrection = fixedWingPitchToThrottleCorrection(pitchCorrection, currentTimeUs);

        if (navStateFlags & NAV_CTL_LAND) {
        // During LAND we do not allow to raise THROTTLE when nose is up to reduce speed
            throttleCorrection = constrain(throttleCorrection, minThrottleCorrection, 0);
        } else {
            throttleCorrection = constrain(throttleCorrection, minThrottleCorrection, maxThrottleCorrection);
        }

        // Speed controller - only apply in POS mode when NOT NAV_CTL_LAND
        if ((navStateFlags & NAV_CTL_POS) && !(navStateFlags & NAV_CTL_LAND)) {
            throttleCorrection += applyFixedWingMinSpeedController(currentTimeUs);
            throttleCorrection = constrain(throttleCorrection, minThrottleCorrection, maxThrottleCorrection);
        }

        uint16_t correctedThrottleValue = constrain(currentBatteryProfile->nav.fw.cruise_throttle + throttleCorrection, currentBatteryProfile->nav.fw.min_throttle, currentBatteryProfile->nav.fw.max_throttle);

        // Manual throttle increase
        if (navConfig()->fw.allow_manual_thr_increase && !FLIGHT_MODE(FAILSAFE_MODE) && !FLIGHT_MODE(NAV_FW_AUTOLAND)) {
            if (rcCommand[THROTTLE] < PWM_RANGE_MIN + (PWM_RANGE_MAX - PWM_RANGE_MIN) * 0.95){
                correctedThrottleValue += MAX(0, rcCommand[THROTTLE] - currentBatteryProfile->nav.fw.cruise_throttle);
            } else {
                correctedThrottleValue = getMaxThrottle();
            }
            isAutoThrottleManuallyIncreased = (rcCommand[THROTTLE] > currentBatteryProfile->nav.fw.cruise_throttle);
        } else {
            isAutoThrottleManuallyIncreased = false;
        }

        rcCommand[THROTTLE] = setDesiredThrottle(correctedThrottleValue, false);
    }

#ifdef USE_FW_AUTOLAND
    // Advanced autoland
    if (posControl.navState == NAV_STATE_FW_LANDING_GLIDE || posControl.navState == NAV_STATE_FW_LANDING_FLARE || STATE(LANDING_DETECTED)) {
        // Set motor to min. throttle and stop it when MOTOR_STOP feature is enabled
        ENABLE_STATE(NAV_MOTOR_STOP_OR_IDLE);

        if (posControl.navState == NAV_STATE_FW_LANDING_GLIDE) {
            rcCommand[PITCH] = pidAngleToRcCommand(-DEGREES_TO_DECIDEGREES(navFwAutolandConfig()->glidePitch), pidProfile()->max_angle_inclination[FD_PITCH]);
        }

        if (posControl.navState == NAV_STATE_FW_LANDING_FLARE) {
            rcCommand[PITCH] = pidAngleToRcCommand(-DEGREES_TO_DECIDEGREES(navFwAutolandConfig()->flarePitch), pidProfile()->max_angle_inclination[FD_PITCH]);
        }
    }
#endif
    // "Traditional" landing as fallback option
    if (navStateFlags & NAV_CTL_LAND) {
        int32_t finalAltitude = navConfig()->general.land_slowdown_minalt + posControl.rthState.homeTmpWaypoint.z;

        if ((posControl.flags.estAltStatus >= EST_USABLE && navGetCurrentActualPositionAndVelocity()->pos.z <= finalAltitude) ||
           (posControl.flags.estAglStatus == EST_TRUSTED && posControl.actualState.agl.pos.z <= navConfig()->general.land_slowdown_minalt)) {

            // Set motor to min. throttle and stop it when MOTOR_STOP feature is enabled
            ENABLE_STATE(NAV_MOTOR_STOP_OR_IDLE);

            // Stabilize ROLL axis on 0 degrees banking regardless of loiter radius and position
            rcCommand[ROLL] = 0;

            // Stabilize PITCH angle into shallow dive as specified by the nav_fw_land_dive_angle setting (default value is 2 - defined in navigation.c).
            rcCommand[PITCH] = pidAngleToRcCommand(DEGREES_TO_DECIDEGREES(navConfig()->fw.land_dive_angle), pidProfile()->max_angle_inclination[FD_PITCH]);
        }
    }
}

bool isFixedWingAutoThrottleManuallyIncreased(void)
{
    return isAutoThrottleManuallyIncreased;
}

bool isFixedWingFlying(void)
{
    float airspeed = 0.0f;
#ifdef USE_PITOT
    if (sensors(SENSOR_PITOT) && pitotIsHealthy()) {
        airspeed = getAirspeedEstimate();
    }
#endif
    bool throttleCondition = getMotorCount() == 0 || rcCommand[THROTTLE] > currentBatteryProfile->nav.fw.cruise_throttle;
    bool velCondition = posControl.actualState.velXY > 250.0f || airspeed > 250.0f;
    bool launchCondition = isNavLaunchEnabled() && fixedWingLaunchStatus() == FW_LAUNCH_FLYING;

    return (isGPSHeadingValid() && throttleCondition && velCondition) || launchCondition;
}

/*-----------------------------------------------------------
 * FixedWing land detector
 *-----------------------------------------------------------*/
bool isFixedWingLandingDetected(void)
{
    DEBUG_SET(DEBUG_LANDING, 4, 0);
    static bool fixAxisCheck = false;

    // Basic condition to start looking for landing
    bool startCondition = (navGetCurrentStateFlags() & (NAV_CTL_LAND | NAV_CTL_EMERG))
                          || FLIGHT_MODE(FAILSAFE_MODE)
                          || FLIGHT_MODE(NAV_FW_AUTOLAND)
                          || (!navigationIsControllingThrottle() && throttleStickIsLow());

    if (!startCondition || posControl.flags.resetLandingDetector) {
        return fixAxisCheck = posControl.flags.resetLandingDetector = false;
    }
    DEBUG_SET(DEBUG_LANDING, 4, 1);

    static timeMs_t fwLandingTimerStartAt;
    static int16_t fwLandSetRollDatum;
    static int16_t fwLandSetPitchDatum;
    const float sensitivity = navConfig()->general.land_detect_sensitivity / 5.0f;

    const timeMs_t currentTimeMs = millis();

    // Check horizontal and vertical velocities are low (cm/s)
    bool velCondition = fabsf(navGetCurrentActualPositionAndVelocity()->vel.z) < (50.0f * sensitivity) &&
                        ( posControl.actualState.velXY < (100.0f * sensitivity));
    // Check angular rates are low (degs/s)
    bool gyroCondition = averageAbsGyroRates() < (2.0f * sensitivity);
    DEBUG_SET(DEBUG_LANDING, 2, velCondition);
    DEBUG_SET(DEBUG_LANDING, 3, gyroCondition);

    if (velCondition && gyroCondition){
        DEBUG_SET(DEBUG_LANDING, 4, 2);
        DEBUG_SET(DEBUG_LANDING, 5, fixAxisCheck);
        if (!fixAxisCheck) {        // capture roll and pitch angles to be used as datums to check for absolute change
            fwLandSetRollDatum = attitude.values.roll;  //0.1 deg increments
            fwLandSetPitchDatum = attitude.values.pitch;
            fixAxisCheck = true;
            fwLandingTimerStartAt = currentTimeMs;
        } else {
            const uint8_t angleLimit = 5 * sensitivity;
            bool isRollAxisStatic = ABS(fwLandSetRollDatum - attitude.values.roll) < angleLimit;
            bool isPitchAxisStatic = ABS(fwLandSetPitchDatum - attitude.values.pitch) < angleLimit;
            DEBUG_SET(DEBUG_LANDING, 6, isRollAxisStatic);
            DEBUG_SET(DEBUG_LANDING, 7, isPitchAxisStatic);
            if (isRollAxisStatic && isPitchAxisStatic) {
                /* Probably landed, low horizontal and vertical velocities and no axis rotation in Roll and Pitch
                 * Conditions need to be held for fixed safety time + optional extra delay.
                 * Fixed time increased if velocities invalid to provide extra safety margin against false triggers */
                const uint16_t safetyTime = posControl.flags.estAltStatus == EST_NONE || posControl.flags.estVelStatus == EST_NONE ? 5000 : 1000;
                timeMs_t safetyTimeDelay = safetyTime + navConfig()->general.auto_disarm_delay;
                return currentTimeMs - fwLandingTimerStartAt > safetyTimeDelay;
            } else {
                fixAxisCheck = false;
            }
        }
    }
    return false;
}

/*-----------------------------------------------------------
 * FixedWing emergency landing
 *-----------------------------------------------------------*/
void applyFixedWingEmergencyLandingController(timeUs_t currentTimeUs)
{
    rcCommand[THROTTLE] = setDesiredThrottle(currentBatteryProfile->failsafe_throttle, true);

    if (posControl.flags.estAltStatus >= EST_USABLE) {
        // target min descent rate at distance 2 x emerg descent rate above takeoff altitude
        updateClimbRateToAltitudeController(0, 2.0f * navConfig()->general.emerg_descent_rate, ROC_TO_ALT_TARGET);
        applyFixedWingAltitudeAndThrottleController(currentTimeUs);

        int16_t pitchCorrection = constrain(posControl.rcAdjustment[PITCH], -DEGREES_TO_DECIDEGREES(navConfig()->fw.max_dive_angle), DEGREES_TO_DECIDEGREES(navConfig()->fw.max_climb_angle));
        rcCommand[PITCH] = -pidAngleToRcCommand(pitchCorrection, pidProfile()->max_angle_inclination[FD_PITCH]);
    } else {
        rcCommand[PITCH] = pidAngleToRcCommand(failsafeConfig()->failsafe_fw_pitch_angle, pidProfile()->max_angle_inclination[FD_PITCH]);
    }

    if (posControl.flags.estPosStatus >= EST_USABLE) {  // Hold position if possible
        applyFixedWingPositionController(currentTimeUs);
        int16_t rollCorrection = constrain(posControl.rcAdjustment[ROLL],
                                            -DEGREES_TO_DECIDEGREES(navConfig()->fw.max_bank_angle),
                                            DEGREES_TO_DECIDEGREES(navConfig()->fw.max_bank_angle));
        rcCommand[ROLL] = pidAngleToRcCommand(rollCorrection, pidProfile()->max_angle_inclination[FD_ROLL]);
        rcCommand[YAW] = 0;
    } else {
        rcCommand[ROLL] = pidAngleToRcCommand(failsafeConfig()->failsafe_fw_roll_angle, pidProfile()->max_angle_inclination[FD_ROLL]);
        rcCommand[YAW] = -pidRateToRcCommand(failsafeConfig()->failsafe_fw_yaw_rate, currentControlRateProfile->stabilized.rates[FD_YAW]);
    }
}

/*-----------------------------------------------------------
 * Calculate loiter target based on current position and velocity
 *-----------------------------------------------------------*/
void calculateFixedWingInitialHoldPosition(fpVector3_t * pos)
{
    // TODO: stub, this should account for velocity and target loiter radius
    *pos = navGetCurrentActualPositionAndVelocity()->pos;
}

void resetFixedWingHeadingController(void)
{
    updateHeadingHoldTarget(CENTIDEGREES_TO_DEGREES(posControl.actualState.cog));
}

void applyFixedWingNavigationController(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs)
{
    if (navStateFlags & NAV_CTL_LAUNCH) {
        applyFixedWingLaunchController(currentTimeUs);
    }
    else if (navStateFlags & NAV_CTL_EMERG) {
        applyFixedWingEmergencyLandingController(currentTimeUs);
    }
    else {
#ifdef NAV_FW_LIMIT_MIN_FLY_VELOCITY
        // Don't apply anything if ground speed is too low (<3m/s)
        if (posControl.actualState.velXY > 300) {
#else
        if (true) {
#endif
            if (navStateFlags & NAV_CTL_ALT) {
                if (getMotorStatus() == MOTOR_STOPPED_USER || FLIGHT_MODE(SOARING_MODE)) {
                    // Motor has been stopped by user or soaring mode enabled to override altitude control
                    resetFixedWingAltitudeController();
                    setDesiredPosition(&navGetCurrentActualPositionAndVelocity()->pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);
                } else {
                    applyFixedWingAltitudeAndThrottleController(currentTimeUs);
                }
            }

            if (navStateFlags & NAV_CTL_POS) {
                applyFixedWingPositionController(currentTimeUs);
            }

        } else {
            posControl.rcAdjustment[PITCH] = 0;
            posControl.rcAdjustment[ROLL] = 0;
        }

        if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE) && posControl.flags.isAdjustingPosition) {
            rcCommand[ROLL] = applyDeadbandRescaled(rcCommand[ROLL], rcControlsConfig()->pos_hold_deadband, -500, 500);
        }

        //if (navStateFlags & NAV_CTL_YAW)
        if ((navStateFlags & NAV_CTL_ALT) || (navStateFlags & NAV_CTL_POS)) {
            applyFixedWingPitchRollThrottleController(navStateFlags, currentTimeUs);
        }

        if (FLIGHT_MODE(SOARING_MODE) && navConfig()->general.flags.soaring_motor_stop) {
            ENABLE_STATE(NAV_MOTOR_STOP_OR_IDLE);
        }
    }
}

float navigationGetCrossTrackError(void)
{
    return navCrossTrackError;
}
