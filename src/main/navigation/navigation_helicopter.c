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

#include "drivers/time.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/filter.h"
#include "common/utils.h"

#include "sensors/sensors.h"
#include "sensors/acceleration.h"
#include "sensors/boardalignment.h"
#include "sensors/gyro.h"

#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/rc_curves.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"

#include "flight/pid.h"
#include "flight/imu.h"
#include "flight/failsafe.h"
#include "flight/mixer.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"
#include "navigation/sqrt_controller.h"

#include "sensors/battery.h"

/*-----------------------------------------------------------
 * Altitude controller for helicopter aircraft
 *-----------------------------------------------------------*/
#if defined(USE_VARIABLE_PITCH)

static int16_t rcCommandAdjustedCollective;
static int16_t altHoldThrustRCZero = 1500;
static pt1Filter_t altholdThrustFilterState;
static bool prepareForTakeoffOnReset = false;
static sqrt_controller_t alt_hold_sqrt_controller;

// Position to velocity controller for Z axis
static void updateAltitudeVelocityController_HC(timeDelta_t deltaMicros)
{
    float targetVel = sqrtControllerApply(
        &alt_hold_sqrt_controller,
        posControl.desiredState.pos.z,
        navGetCurrentActualPositionAndVelocity()->pos.z,
        US2S(deltaMicros)
    );

    // hard limit desired target velocity to max_climb_rate
    float vel_max_z = 0.0f;

    if (posControl.flags.isAdjustingAltitude) {
        vel_max_z = navConfig()->general.max_manual_climb_rate;
    } else {
        vel_max_z = navConfig()->general.max_auto_climb_rate;
    }

    targetVel = constrainf(targetVel, -vel_max_z, vel_max_z);

    posControl.pids.pos[Z].output_constrained = targetVel;

    // Limit max up/down acceleration target
    const float smallVelChange = US2S(deltaMicros) * (GRAVITY_CMSS * 0.1f);
    const float velTargetChange = targetVel - posControl.desiredState.vel.z;

    if (velTargetChange <= -smallVelChange) {
        // Large & Negative - acceleration is _down_. We can't reach more than -1G in any possible condition. Hard limit to 0.8G to stay safe
        // This should be safe enough for stability since we only reduce throttle. 
        // woga65: @todo change comment to sth. meaningful since we CAN DEFINITIVELY reach more than -1G on colective-pitch aircraft!
        const float maxVelDifference = US2S(deltaMicros) * (GRAVITY_CMSS * 0.8f);
        posControl.desiredState.vel.z = constrainf(targetVel, posControl.desiredState.vel.z - maxVelDifference, posControl.desiredState.vel.z + maxVelDifference);
    }
    else if (velTargetChange >= smallVelChange) {
        // Large and positive - acceleration is _up_. We are limited by thrust/weight ratio which is usually about 2:1 (hover around 50% throttle).
        // T/W ratio = 2 means we are able to reach 1G acceleration in "UP" direction. Hard limit to 0.5G to be on a safe side and avoid abrupt throttle changes.
        // woga65: @todo change comment to sth. meaningful: 75% collective might be hovering, 50% might be falling, 25% might be accelerating downwards!
        const float maxVelDifference = US2S(deltaMicros) * (GRAVITY_CMSS * 0.5f);
        posControl.desiredState.vel.z = constrainf(targetVel, posControl.desiredState.vel.z - maxVelDifference, posControl.desiredState.vel.z + maxVelDifference);
    }
    else {
        // Small - desired acceleration is less than 0.1G. We should be safe setting velocity target directly - any platform should be able to satisfy this
        posControl.desiredState.vel.z = targetVel;
    }

    navDesiredVelocity[Z] = constrain(lrintf(posControl.desiredState.vel.z), -32678, 32767);
}

static void updateAltitudeCollectiveController_HC(timeDelta_t deltaMicros)
{
    // Calculate min and max collective pitch boundaries (to compensate for integral windup)
    const int16_t collCorrectionMin = 1000 - currentBatteryProfile->nav.mc.hover_throttle;
    const int16_t collCorrectionMax = 2000 - currentBatteryProfile->nav.mc.hover_throttle;

    float velocity_controller = navPidApply2(&posControl.pids.vel[Z], posControl.desiredState.vel.z, navGetCurrentActualPositionAndVelocity()->vel.z, US2S(deltaMicros), collCorrectionMin, collCorrectionMax, 0);

    int16_t rcCollectiveCorrection = pt1FilterApply4(&altholdThrustFilterState, velocity_controller, NAV_THROTTLE_CUTOFF_FREQENCY_HZ, US2S(deltaMicros));
    rcCollectiveCorrection = constrain(rcCollectiveCorrection, collCorrectionMin, collCorrectionMax);

    posControl.rcAdjustment[COLLECTIVE] = constrain(currentBatteryProfile->nav.mc.hover_throttle + rcCollectiveCorrection, 1000, 2000);
}

bool adjustHelicopterAltitudeFromRCInput(void)
{
    if (posControl.flags.isTerrainFollowEnabled) {
        float altTarget = scaleRangef(rcCommand[COLLECTIVE], 1000, 2000, 0, navConfig()->general.max_terrain_follow_altitude);

        // In terrain follow mode we apply different logic for terrain control
        if (posControl.flags.estAglStatus == EST_TRUSTED && altTarget > 10.0f) {
            // We have solid terrain sensor signal - directly map throttle to altitude
            updateClimbRateToAltitudeController(0, 0, ROC_TO_ALT_RESET);
            posControl.desiredState.pos.z = altTarget;
        }
        else {
            updateClimbRateToAltitudeController(-50.0f, 0, ROC_TO_ALT_CONSTANT);
        }

        // In surface tracking we always indicate that we're adjusting altitude
        return true;
    }
    else {
        const int16_t rcThrustAdjustment = applyDeadbandRescaled(rcCommand[COLLECTIVE] - altHoldThrustRCZero, rcControlsConfig()->alt_hold_deadband, -500, 500);    

        if (rcThrustAdjustment) {
            // set velocity proportional to stick movement
            float rcClimbRate;

            // Make sure we can satisfy max_manual_climb_rate in both up and down directions
            if (rcThrustAdjustment > 0) {
                // Scaling from altHoldThrustRCZero to PWM_RANGE_MAX or maxthrottle
                rcClimbRate = rcThrustAdjustment * navConfig()->general.max_manual_climb_rate / (float)(2000 - altHoldThrustRCZero - rcControlsConfig()->alt_hold_deadband);
            }
            else {
                // Scaling from PWM_RANGE_MIN or minthrottle to altHoldThrustRCZero
                rcClimbRate = rcThrustAdjustment * navConfig()->general.max_manual_climb_rate / (float)(altHoldThrustRCZero - 1000 - rcControlsConfig()->alt_hold_deadband);
            }

            updateClimbRateToAltitudeController(rcClimbRate, 0, ROC_TO_ALT_CONSTANT);

            return true;
        }
        else {
            // Adjusting finished - reset desired position to stay exactly where pilot released the stick
            if (posControl.flags.isAdjustingAltitude) {
                updateClimbRateToAltitudeController(0, 0, ROC_TO_ALT_RESET);
            }

            return false;
        }
    }
}

void setupHelicopterAltitudeController(void)
{
    const bool stickIsLow = collectiveStickIsLow();;

    if (navConfig()->general.flags.use_thr_mid_for_althold) {
        altHoldThrustRCZero = 1500;
    }
    else {
        // If throttle / collective is LOW - use Thr Mid anyway
        altHoldThrustRCZero = (stickIsLow) ? 1500 : rcCommand[COLLECTIVE];
    }

    // Make sure we are able to satisfy the deadband
    altHoldThrustRCZero = constrain(altHoldThrustRCZero,
                                      1000 + rcControlsConfig()->alt_hold_deadband + 10,
                                      2000 - rcControlsConfig()->alt_hold_deadband - 10);

    // Force AH controller to initialize althold integral for pending takeoff on reset
    // Signal for that is low throttle _and_ low actual altitude
    if (stickIsLow && fabsf(navGetCurrentActualPositionAndVelocity()->pos.z) <= 50.0f) {
        prepareForTakeoffOnReset = true;
    }
}

void resetHelicopterAltitudeController(void)
{
    const navEstimatedPosVel_t *posToUse = navGetCurrentActualPositionAndVelocity();
    float nav_speed_up = 0.0f;
    float nav_speed_down = 0.0f;
    float nav_accel_z = 0.0f;

    navPidReset(&posControl.pids.vel[Z]);
    navPidReset(&posControl.pids.surface);

    posControl.rcAdjustment[COLLECTIVE] = currentBatteryProfile->nav.mc.hover_throttle;

    posControl.desiredState.vel.z = posToUse->vel.z;   // Gradually transition from current climb

    pt1FilterReset(&altholdThrustFilterState, 0.0f);
    pt1FilterReset(&posControl.pids.vel[Z].error_filter_state, 0.0f);
    pt1FilterReset(&posControl.pids.vel[Z].dterm_filter_state, 0.0f);

    if (FLIGHT_MODE(FAILSAFE_MODE) || FLIGHT_MODE(NAV_RTH_MODE) || FLIGHT_MODE(NAV_WP_MODE) || navigationIsExecutingAnEmergencyLanding()) {
        const float maxSpeed = getActiveWaypointSpeed();
        nav_speed_up = maxSpeed;
        nav_accel_z = maxSpeed;
        nav_speed_down = navConfig()->general.max_auto_climb_rate;
    } else {
        nav_speed_up = navConfig()->general.max_manual_speed;
        nav_accel_z = navConfig()->general.max_manual_speed;
        nav_speed_down = navConfig()->general.max_manual_climb_rate;
    }

    sqrtControllerInit(
        &alt_hold_sqrt_controller,
        posControl.pids.pos[Z].param.kP,
        -fabsf(nav_speed_down),
        nav_speed_up,
        nav_accel_z
    );
}

static void applyHelicopterAltitudeController(timeUs_t currentTimeUs)
{
    static timeUs_t previousTimePositionUpdate = 0;     // Occurs @ altitude sensor update rate (max MAX_ALTITUDE_UPDATE_RATE_HZ)

    // If we have an update on vertical position data - update velocity and accel targets
    if (posControl.flags.verticalPositionDataNew) {
        const timeDeltaLarge_t deltaMicrosPositionUpdate = currentTimeUs - previousTimePositionUpdate;
        previousTimePositionUpdate = currentTimeUs;

        // Check if last correction was not too long ago
        if (deltaMicrosPositionUpdate < MAX_POSITION_UPDATE_INTERVAL_US) {
            // If we are preparing for takeoff - start with lowset possible climb rate, adjust alt target and make sure throttle doesn't jump
            if (prepareForTakeoffOnReset) {
                const navEstimatedPosVel_t *posToUse = navGetCurrentActualPositionAndVelocity();

                posControl.desiredState.vel.z = -navConfig()->general.max_manual_climb_rate;
                posControl.desiredState.pos.z = posToUse->pos.z - (navConfig()->general.max_manual_climb_rate / posControl.pids.pos[Z].param.kP);
                posControl.pids.vel[Z].integrator = -500.0f;
                pt1FilterReset(&altholdThrustFilterState, -500.0f);
                prepareForTakeoffOnReset = false;
            }

            // Execute actual altitude controllers
            updateAltitudeVelocityController_HC(deltaMicrosPositionUpdate);
            updateAltitudeCollectiveController_HC(deltaMicrosPositionUpdate);
        }
        else {
            // Position update has not occurred in time (first start or glitch), reset altitude controller
            resetHelicopterAltitudeController();
        }

        // Indicate that information is no longer usable
        posControl.flags.verticalPositionDataConsumed = true;
    }

    rcCommand[COLLECTIVE] = posControl.rcAdjustment[COLLECTIVE];
    rcCommandAdjustedCollective = rcCommand[COLLECTIVE];
}



bool isHelicopterFlying(void)
{
    bool collectiveCondition = rcCommand[COLLECTIVE] > currentBatteryProfile->nav.mc.hover_throttle; 
    bool gyroCondition = averageAbsGyroRates() > 7.0f;

    return collectiveCondition && gyroCondition;
}

bool isHelicopterFlyingUpright(void)
{
    bool collectiveCondition = rcCommand[COLLECTIVE] > currentBatteryProfile->nav.mc.hover_throttle; 
    bool gyroCondition = averageAbsGyroRates() > 7.0f;
    bool uprightCondition = attitude.values.roll < 900 || attitude.values.roll > -900;

    return (accIsCalibrationComplete()) 
        ? gyroCondition && uprightCondition
        : gyroCondition && collectiveCondition;
}

bool isHelicopterFlyingInverted(void)
{
    bool collectiveCondition = rcCommand[COLLECTIVE] < (1500 - (currentBatteryProfile->nav.mc.hover_throttle - 1500)); 
    bool gyroCondition = averageAbsGyroRates() > 7.0f;
    bool invertedCondition = attitude.values.roll > 900 || attitude.values.roll < -900;

    return (accIsCalibrationComplete()) 
        ? gyroCondition && invertedCondition
        : gyroCondition && collectiveCondition;
}


/*-----------------------------------------------------------
 * Multicopter land detector
 *-----------------------------------------------------------*/
#if defined(USE_BARO)

static bool isLandingGbumpDetected(timeMs_t currentTimeMs)
{
    /* Detection based on G bump at touchdown, falling Baro altitude and throttle below hover.
     * G bump trigger: > 2g then falling back below 1g in < 0.1s.
     * Baro trigger: rate must be -ve at initial trigger g and < -2 m/s when g falls back below 1g
     * Throttle trigger: must be below hover throttle with lower threshold for manual throttle control */

    static timeMs_t gSpikeDetectTimeMs = 0;
    float baroAltRate = updateBaroAltitudeRate(0, false);

    if (!gSpikeDetectTimeMs && acc.accADCf[Z] > 2.0f && baroAltRate < 0.0f) {
        gSpikeDetectTimeMs = currentTimeMs;
    } else if (gSpikeDetectTimeMs) {
        if (currentTimeMs < gSpikeDetectTimeMs + 100) {
            if (acc.accADCf[Z] < 1.0f && baroAltRate < -200.0f) {
                const uint16_t hoverCollectiveRange = currentBatteryProfile->nav.mc.hover_throttle - 1000;
                return rcCommand[COLLECTIVE] < 1000 + ((navigationInAutomaticThrottleMode() ? 0.8 : 0.5) * hoverCollectiveRange);
      }
        } else if (acc.accADCf[Z] <= 1.0f) {
            gSpikeDetectTimeMs = 0;
        }
    }

    return false;
}
#endif

bool isHelicopterLandingDetected(void)
{
    DEBUG_SET(DEBUG_LANDING, 4, 0);
    DEBUG_SET(DEBUG_LANDING, 3, averageAbsGyroRates() * 100);

    const timeMs_t currentTimeMs = millis();

#if defined(USE_BARO)
    if (sensors(SENSOR_BARO) && navConfig()->general.flags.landing_bump_detection && isLandingGbumpDetected(currentTimeMs)) {
        return true;    // Landing flagged immediately if landing bump detected
    }
#endif

    bool throttleIsBelowMidHover = rcCommand[COLLECTIVE] < (0.5 * (currentBatteryProfile->nav.mc.hover_throttle + 1000));

    /* Basic condition to start looking for landing
     * Detection active during Failsafe only if throttle below mid hover throttle
     * and WP mission not active (except landing states).
     * Also active in non autonomous flight modes but only when thottle low */
    bool startCondition = (navGetCurrentStateFlags() & (NAV_CTL_LAND | NAV_CTL_EMERG))
                          || (FLIGHT_MODE(FAILSAFE_MODE) && !FLIGHT_MODE(NAV_WP_MODE) && throttleIsBelowMidHover)
                          || (!navigationIsFlyingAutonomousMode() && collectiveStickIsLow());

    static timeMs_t landingDetectorStartedAt;

    if (!startCondition || posControl.flags.resetLandingDetector) {
        landingDetectorStartedAt = 0;
        return posControl.flags.resetLandingDetector = false;
    }

    const float sensitivity = navConfig()->general.land_detect_sensitivity / 5.0f;

    // check vertical and horizontal velocities are low (cm/s)
    bool velCondition = fabsf(navGetCurrentActualPositionAndVelocity()->vel.z) < (MC_LAND_CHECK_VEL_Z_MOVING * sensitivity) &&
                        posControl.actualState.velXY < (MC_LAND_CHECK_VEL_XY_MOVING * sensitivity);
    // check gyro rates are low (degs/s)
    bool gyroCondition = averageAbsGyroRates() < (4.0f * sensitivity);
    DEBUG_SET(DEBUG_LANDING, 2, velCondition);
    DEBUG_SET(DEBUG_LANDING, 3, gyroCondition);

    bool possibleLandingDetected = false;

    if (navGetCurrentStateFlags() & NAV_CTL_LAND) {
        // We have likely landed if collective is 40 units below average descend collective
        // We use rcCommandAdjustedCollective to keep track of NAV corrected collective (isLandingDetected is executed
        // from processRx() and rcCommand at that moment holds rc input, not adjusted values from NAV core)
        DEBUG_SET(DEBUG_LANDING, 4, 1);

        static int32_t landingThrSum;
        static int32_t landingThrSamples;
        bool isAtMinimalThrust = false;

        if (!landingDetectorStartedAt) {
            landingThrSum = landingThrSamples = 0;
            landingDetectorStartedAt = currentTimeMs;
        }
        if (!landingThrSamples) {
            // Wait for 1 second so throttle has stabilized.
            if (currentTimeMs - landingDetectorStartedAt < S2MS(MC_LAND_THR_STABILISE_DELAY)) {
                return false;
            } else {
                landingDetectorStartedAt = currentTimeMs;
            }
        }
        landingThrSamples += 1;
        landingThrSum += rcCommandAdjustedCollective;
        isAtMinimalThrust = rcCommandAdjustedCollective < (landingThrSum / landingThrSamples - MC_LAND_DESCEND_THROTTLE);

        possibleLandingDetected = isAtMinimalThrust && velCondition;

        DEBUG_SET(DEBUG_LANDING, 6, rcCommandAdjustedCollective);
        DEBUG_SET(DEBUG_LANDING, 7, landingThrSum / landingThrSamples - MC_LAND_DESCEND_THROTTLE);
    } else {    // non autonomous and emergency landing
        DEBUG_SET(DEBUG_LANDING, 4, 2);
        if (landingDetectorStartedAt) {
            possibleLandingDetected = velCondition && gyroCondition;
        } else {
            landingDetectorStartedAt = currentTimeMs;
            return false;
        }
    }

    // If we have surface sensor (for example sonar) - use it to detect touchdown
    if ((posControl.flags.estAglStatus == EST_TRUSTED) && (posControl.actualState.agl.pos.z >= 0)) {
        // TODO: Come up with a clever way to let sonar increase detection performance, not just add extra safety.
        // TODO: Out of range sonar may give reading that looks like we landed, find a way to check if sonar is healthy.
        // surfaceMin is our ground reference. If we are less than 5cm above the ground - we are likely landed
        possibleLandingDetected = possibleLandingDetected && (posControl.actualState.agl.pos.z <= (posControl.actualState.surfaceMin + MC_LAND_SAFE_SURFACE));
    }
    DEBUG_SET(DEBUG_LANDING, 5, possibleLandingDetected);

    if (possibleLandingDetected) {
        /* Conditions need to be held for fixed safety time + optional extra delay.
         * Fixed time increased if Z velocity invalid to provide extra safety margin against false triggers */
        const uint16_t safetyTime = posControl.flags.estAltStatus == EST_NONE ? 5000 : 1000;
        timeMs_t safetyTimeDelay = safetyTime + navConfig()->general.auto_disarm_delay;
        return currentTimeMs - landingDetectorStartedAt > safetyTimeDelay;
    } else {
        landingDetectorStartedAt = currentTimeMs;
        return false;
    }
}

/*-----------------------------------------------------------
 * Helicopter emergency landing
 *-----------------------------------------------------------*/
static void applyHelicopterEmergencyLandingController(timeUs_t currentTimeUs)
{
    static timeUs_t previousTimePositionUpdate = 0;

    /* Attempt to stabilise */
    rcCommand[YAW] = 0;
    rcCommand[ROLL] = 0;
    rcCommand[PITCH] = 0;
    rcCommand[COLLECTIVE] = currentBatteryProfile->failsafe_throttle;

    /* Sensors have gone haywire, attempt to land regardless */
    if ((posControl.flags.estAltStatus < EST_USABLE)) {
        if (failsafeConfig()->failsafe_procedure == FAILSAFE_PROCEDURE_DROP_IT) {
            /* on helicopter center collective pitch */
            rcCommand[COLLECTIVE] = 1500;
            /* on helicopter cut throttle immediately */
            rcCommand[THROTTLE] = 1000;
            return;
        }
    }

    // Normal sensor data available, use controlled landing descent
    if (posControl.flags.verticalPositionDataNew) {
        const timeDeltaLarge_t deltaMicrosPositionUpdate = currentTimeUs - previousTimePositionUpdate;
        previousTimePositionUpdate = currentTimeUs;

        // Check if last correction was not too long ago
        if (deltaMicrosPositionUpdate < MAX_POSITION_UPDATE_INTERVAL_US) {
            // target min descent rate 5m above takeoff altitude
            updateClimbRateToAltitudeController(-navConfig()->general.emerg_descent_rate, 500.0f, ROC_TO_ALT_TARGET);
            updateAltitudeVelocityController_HC(deltaMicrosPositionUpdate);     
            updateAltitudeCollectiveController_HC(deltaMicrosPositionUpdate);
        }
        else {
            // due to some glitch position update has not occurred in time, reset altitude controller
            resetHelicopterAltitudeController();
        }

        // Indicate that information is no longer usable
        posControl.flags.verticalPositionDataConsumed = true;
    }

    // Update throttle controller
    rcCommand[COLLECTIVE] = posControl.rcAdjustment[COLLECTIVE];

    // Hold position if possible
    if ((posControl.flags.estPosStatus >= EST_USABLE)) {
        applyMulticopterPositionController(currentTimeUs);
    }
}


void resetHelicopterHeadingController(void)
{
    updateHeadingHoldTarget(CENTIDEGREES_TO_DEGREES(posControl.actualState.yaw));
}

static void applyHelicopterHeadingController(void)
{
    updateHeadingHoldTarget(CENTIDEGREES_TO_DEGREES(posControl.desiredState.yaw));
}

void applyHelicopterNavigationController(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs)
{
    if (navStateFlags & NAV_CTL_EMERG) {
        applyHelicopterEmergencyLandingController(currentTimeUs);
    }
    else {
        if (navStateFlags & NAV_CTL_ALT)
            applyHelicopterAltitudeController(currentTimeUs);

        if (navStateFlags & NAV_CTL_POS)
            applyMulticopterPositionController(currentTimeUs);

        if (navStateFlags & NAV_CTL_YAW)
            applyHelicopterHeadingController();
    }
}

#endif