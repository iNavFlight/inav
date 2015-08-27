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

/*-----------------------------------------------------------
 * Altitude controller for multicopter aircraft
 *-----------------------------------------------------------*/
static int16_t altholdInitialThrottle;  // Throttle input when althold was activated

static void updateAltitudeTargetFromRCInput_MC(uint32_t deltaMicros)
{
    // In some cases pilot has no control over flight direction
    if (!navCanAdjustAltitudeFromRCInput())
        return;

    int16_t rcThrottleAdjustment = applyDeadband(rcCommand[THROTTLE] - altholdInitialThrottle, posControl.rcControlsConfig->alt_hold_deadband);

    if (rcThrottleAdjustment) {
        // set velocity proportional to stick movement
        float rcClimbRate = rcThrottleAdjustment * posControl.navProfile->nav_manual_speed_vertical / (500.0f - posControl.rcControlsConfig->alt_hold_deadband);
        updateAltitudeTargetFromClimbRate(deltaMicros, rcClimbRate);
    }
}

// Position to velocity controller for Z axis
static void updateAltitudeVelocityController_MC(uint32_t deltaMicros)
{
    UNUSED(deltaMicros);

    float altitudeError = posControl.desiredState.pos.V.Z - posControl.actualState.pos.V.Z;

    // Use only P term for PH velocity calculation
    altitudeError = constrainf(altitudeError, -500, 500);
    //altitudeError = applyDeadband(altitudeError, 10); // remove small P parameter to reduce noise near zero position
    posControl.desiredState.vel.V.Z = altitudeError * posControl.pids.pos[Z].param.kP;
    posControl.desiredState.vel.V.Z = constrainf(posControl.desiredState.vel.V.Z, -300, 300); // hard limit velocity to +/- 3 m/s

#if defined(NAV_BLACKBOX)
    navDesiredVelocity[Z] = constrain(lrintf(posControl.desiredState.vel.V.Z), -32678, 32767);
    navLatestPositionError[Z] = constrain(lrintf(altitudeError), -32678, 32767);
    navTargetPosition[Z] = constrain(lrintf(posControl.desiredState.pos.V.Z), -32678, 32767);
#endif
}

static void updateAltitudeAccelController_MC(uint32_t deltaMicros)
{
    static float velFilterState;

    // calculate rate error and filter with cut off frequency of 2 Hz
    float velError = posControl.desiredState.vel.V.Z - posControl.actualState.vel.V.Z;
    velError = navApplyFilter(velError, NAV_VEL_ERROR_CUTOFF_FREQENCY_HZ, US2S(deltaMicros), &velFilterState);
    posControl.desiredState.acc.V.Z = velError * posControl.pids.vel[Z].param.kP;
}

static void updateAltitudeThrottleController_MC(uint32_t deltaMicros)
{
    static float throttleFilterState;
    float accError = posControl.desiredState.acc.V.Z - imuAverageAcceleration.V.Z;

    posControl.rcAdjustment[THROTTLE] = navPidGetPID(accError, US2S(deltaMicros), &posControl.pids.accz);
    posControl.rcAdjustment[THROTTLE] = navApplyFilter(posControl.rcAdjustment[THROTTLE], NAV_THROTTLE_CUTOFF_FREQENCY_HZ, US2S(deltaMicros), &throttleFilterState);
    posControl.rcAdjustment[THROTTLE] = constrain(posControl.rcAdjustment[THROTTLE], -500, 500);
}

void setupMulticopterAltitudeController(void)
{
    if (posControl.navProfile->flags.use_midrc_for_althold) {
        altholdInitialThrottle = masterConfig.rxConfig.midrc;
    }
    else {
        altholdInitialThrottle = rcCommand[THROTTLE];
    }
}

void resetMulticopterAltitudeController()
{
    navPidReset(&posControl.pids.vel[Z]);
    navPidReset(&posControl.pids.accz);
    posControl.rcAdjustment[THROTTLE] = 0;
}

void applyMulticopterAltitudeController(uint32_t currentTime)
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
        resetMulticopterAltitudeController();
        return;
    }

    // Update altitude target from RC input or RTL controller
    if (currentTime - previousTimeTargetPositionUpdate >= HZ2US(POSITION_TARGET_UPDATE_RATE_HZ)) {
        uint32_t deltaMicrosPositionTargetUpdate = currentTime - previousTimeTargetPositionUpdate;
        previousTimeTargetPositionUpdate = currentTime;

        if (navShouldApplyRTHLandingLogic()) {
            // Gradually reduce descent speed depending on actual altitude.
            if (posControl.actualState.pos.V.Z > (posControl.homeWaypoint.pos.V.Z + 1000)) {
                updateAltitudeTargetFromClimbRate(deltaMicrosPositionTargetUpdate, -200.0f);
            }
            else if (posControl.actualState.pos.V.Z > (posControl.homeWaypoint.pos.V.Z + 250)) {
                updateAltitudeTargetFromClimbRate(deltaMicrosPositionTargetUpdate, -100.0f);
            }
            else {
                updateAltitudeTargetFromClimbRate(deltaMicrosPositionTargetUpdate, -50.0f);
            }
        }

        updateAltitudeTargetFromRCInput_MC(deltaMicrosPositionTargetUpdate);
    }

    // If we have an update on vertical position data - update velocity and accel targets
    if (posControl.flags.verticalPositionNewData) {
        uint32_t deltaMicrosPositionUpdate = currentTime - previousTimePositionUpdate;
        previousTimePositionUpdate = currentTime;

        // Check if last correction was too log ago - ignore this update
        if (deltaMicrosPositionUpdate < HZ2US(MIN_ALTITUDE_UPDATE_FREQUENCY_HZ)) {
            updateAltitudeVelocityController_MC(deltaMicrosPositionUpdate);
            updateAltitudeAccelController_MC(deltaMicrosPositionUpdate);
        }
        else {
            // due to some glitch position update has not occurred in time, reset altitude controller
            resetMulticopterAltitudeController();
        }

        // Indicate that information is no longer usable
        posControl.flags.verticalPositionNewData = 0;
    }

    // Update throttle controller
    // We are controlling acceleration here, IMU updates accel every loop so this step is executed at full loop rate,
    // regardless of available altitude and velocity data
    updateAltitudeThrottleController_MC(deltaMicros);

    uint16_t newThrottle = constrain(altholdInitialThrottle + posControl.rcAdjustment[THROTTLE], masterConfig.escAndServoConfig.minthrottle, masterConfig.escAndServoConfig.maxthrottle);
    if (posControl.navProfile->flags.throttle_tilt_comp && isThrustFacingDownwards(&inclination)) {
        float tiltCompFactor = 1.0f / constrainf(calculateCosTiltAngle(), 0.6f, 1.0f);  // max tilt about 50 deg
        newThrottle *= tiltCompFactor;
    }

    rcCommand[THROTTLE] = constrain(newThrottle, masterConfig.escAndServoConfig.minthrottle, masterConfig.escAndServoConfig.maxthrottle);
}

/*-----------------------------------------------------------
 * Heading controller for multicopter aircraft
 *-----------------------------------------------------------*/

/*-----------------------------------------------------------
 * Calculate rcAdjustment for YAW
 *-----------------------------------------------------------*/
static void calculateHeadingAdjustment_MC(float dTnav)
{
    UNUSED(dTnav);

    // FIXME: Account for fixed-wing config without rudder (flying wing)

    // Calculate yaw correction
    int32_t headingError = wrap_18000(posControl.actualState.yaw - posControl.desiredState.yaw) * masterConfig.yaw_control_direction;
    headingError = constrain(headingError, -3000, +3000); // limit error to +- 30 degrees to avoid fast rotation

    // FIXME: SMALL_ANGLE might prevent NAV from adjusting yaw when banking is too high (i.e. nav in high wind)
    if (STATE(SMALL_ANGLE)) {
        // Heading PID controller takes degrees, not centidegrees (this pid duplicates MAGHOLD)
        posControl.rcAdjustment[YAW] = (headingError / 100.0f) * posControl.pids.heading.param.kP;
    }
}

/*-----------------------------------------------------------
 * Adjusts desired heading from pilot's input
 *-----------------------------------------------------------*/
static void adjustHeadingFromRCInput_MC()
{
    // In some cases pilot has no control over flight direction
    if (!navCanAdjustHeadingFromRCInput())
        return;

    // Passthrough yaw input if stick is moved
    int16_t rcYawAdjustment = applyDeadband(rcCommand[YAW], posControl.navProfile->nav_rc_deadband);

    if (rcYawAdjustment) {
        posControl.rcAdjustment[YAW] = rcYawAdjustment;

        // Can only allow pilot to set the new heading if doing PH, during RTH copter will target itself to home
        if (navShouldApplyPosHold()) {
            posControl.desiredState.yaw = posControl.actualState.yaw;
        }
    }
}

void applyMulticopterHeadingController(uint32_t currentTime)
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

        calculateHeadingAdjustment_MC(dTnav);
        adjustHeadingFromRCInput_MC();

        // Indicate that information is no longer usable
        posControl.flags.headingNewData = 0;
    }

    // Control yaw by NAV PID
    rcCommand[YAW] = constrain(posControl.rcAdjustment[YAW], -500, 500);
}

void resetMulticopterHeadingController(void)
{
    posControl.rcAdjustment[YAW] = 0;
}

/*-----------------------------------------------------------
 * XY-position controller for multicopter aircraft
 *-----------------------------------------------------------*/
void resetMulticopterPositionController(void)
{
    int axis;
    for (axis = 0; axis < 2; axis++) {
        navPidReset(&posControl.pids.vel[axis]);
        posControl.rcAdjustment[axis] = 0;
    }
}

static void updatePositionTargetFromRCInput_MC(uint32_t deltaMicros)
{
    UNUSED(deltaMicros);

    // In some cases pilot has no control over flight direction
    if (!navCanAdjustHorizontalVelocityAndAttitudeFromRCInput())
        return;

    if (posControl.navProfile->flags.user_control_mode != NAV_GPS_CRUISE)
        return;

    int16_t rcPitchAdjustment = applyDeadband(rcCommand[PITCH], posControl.navProfile->nav_rc_deadband);
    int16_t rcRollAdjustment = applyDeadband(rcCommand[ROLL], posControl.navProfile->nav_rc_deadband);

    if (rcPitchAdjustment || rcRollAdjustment) {
        float rcVelX = rcPitchAdjustment * posControl.navProfile->nav_manual_speed_horizontal / (500.0f - posControl.navProfile->nav_rc_deadband);
        float rcVelY = rcRollAdjustment * posControl.navProfile->nav_manual_speed_horizontal / (500.0f - posControl.navProfile->nav_rc_deadband);

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

static void updatePositionLeanAngleFromRCInput_MC(uint32_t deltaMicros)
{
    UNUSED(deltaMicros);

    // In some cases pilot has no control over flight direction
    if (!navCanAdjustHorizontalVelocityAndAttitudeFromRCInput())
        return;

    if (posControl.navProfile->flags.user_control_mode != NAV_GPS_ATTI)
        return;

    int16_t rcPitchAdjustment = applyDeadband(rcCommand[PITCH], posControl.navProfile->nav_rc_deadband);
    int16_t rcRollAdjustment = applyDeadband(rcCommand[ROLL], posControl.navProfile->nav_rc_deadband);

    if (rcPitchAdjustment || rcRollAdjustment) {
        // Direct attitude control
        posControl.rcAdjustment[PITCH] = rcPitchAdjustment;
        posControl.rcAdjustment[ROLL] = rcRollAdjustment;

        // If we are in position hold mode, so adjust poshold position
        if (navShouldApplyPosHold()) {
            posControl.desiredState.pos.V.X = posControl.actualState.pos.V.X;
            posControl.desiredState.pos.V.Y = posControl.actualState.pos.V.Y;
        }

        // When sticks are released we should restart velocity PIDs
        navPidReset(&posControl.pids.vel[X]);
        navPidReset(&posControl.pids.vel[Y]);
    }
}

static void updatePositionVelocityController_MC(uint32_t deltaMicros)
{
    UNUSED(deltaMicros);

    float posErrorX = posControl.desiredState.pos.V.X - posControl.actualState.pos.V.X;
    float posErrorY = posControl.desiredState.pos.V.Y - posControl.actualState.pos.V.Y;

   //TODO: Apply some non-linear approach here. If we are close - go linear, if we are far, increase slower
    float newVelX = posErrorX * posControl.pids.pos[X].param.kP;
    float newVelY = posErrorY * posControl.pids.pos[Y].param.kP;
    float newVelTotal = sqrtf(sq(newVelX) + sq(newVelY));

    if (newVelTotal > posControl.navProfile->nav_speed_max) {
        newVelX = posControl.navProfile->nav_speed_max * (newVelX / newVelTotal);
        newVelY = posControl.navProfile->nav_speed_max * (newVelY / newVelTotal);
    }

    posControl.desiredState.vel.V.X = newVelX;
    posControl.desiredState.vel.V.Y = newVelY;

#if defined(NAV_BLACKBOX)
    navDesiredVelocity[X] = constrain(lrintf(posControl.desiredState.vel.V.X), -32678, 32767);
    navDesiredVelocity[Y] = constrain(lrintf(posControl.desiredState.vel.V.Y), -32678, 32767);
    navLatestPositionError[X] = constrain(lrintf(posErrorX), -32678, 32767);
    navLatestPositionError[Y] = constrain(lrintf(posErrorY), -32678, 32767);
    navTargetPosition[X] = constrain(lrintf(posControl.desiredState.pos.V.X), -32678, 32767);
    navTargetPosition[Y] = constrain(lrintf(posControl.desiredState.pos.V.Y), -32678, 32767);
#endif
}

static void updatePositionAccelController_MC(uint32_t deltaMicros, float maxAccelLimit)
{
    static float accFilterStateX = 0.0f, accFilterStateY = 0.0f;
    float velError, newAccelX, newAccelY;

    // Calculate acceleration target on X-axis
    velError = constrainf(posControl.desiredState.vel.V.X - posControl.actualState.vel.V.X, -500.0f, 500.0f); // limit error to 5 m/s
    newAccelX = navPidGetPID(velError, US2S(deltaMicros), &posControl.pids.vel[X]);

#if defined(NAV_BLACKBOX)
    NAV_BLACKBOX_DEBUG(0, velError);
    NAV_BLACKBOX_DEBUG(1, posControl.pids.vel[X].lastP);
    NAV_BLACKBOX_DEBUG(2, posControl.pids.vel[X].lastI);
    NAV_BLACKBOX_DEBUG(3, posControl.pids.vel[X].lastD);
#endif

    // Calculate acceleration target on Y-axis
    velError = constrainf(posControl.desiredState.vel.V.Y - posControl.actualState.vel.V.Y, -500.0f, 500.0f); // limit error to 5 m/s
    newAccelY = navPidGetPID(velError, US2S(deltaMicros), &posControl.pids.vel[Y]);

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

static void updatePositionLeanAngleController_MC(uint32_t deltaMicros)
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

    posControl.rcAdjustment[ROLL] = constrainf(desiredRoll, -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX);
    posControl.rcAdjustment[PITCH] = constrainf(desiredPitch, -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX);
}

void applyMulticopterPositionController(uint32_t currentTime)
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
        resetMulticopterPositionController();
        return;
    }

    // Update altitude target from RC input
    if (currentTime - previousTimeTargetPositionUpdate >= HZ2US(POSITION_TARGET_UPDATE_RATE_HZ)) {
        uint32_t deltaMicrosPositionTargetUpdate = currentTime - previousTimeTargetPositionUpdate;
        previousTimeTargetPositionUpdate = currentTime;
        updatePositionTargetFromRCInput_MC(deltaMicrosPositionTargetUpdate);
    }

    // If we have new position - update velocity and acceleration controllers
    if (posControl.flags.horizontalPositionNewData) {
        uint32_t deltaMicrosPositionUpdate = currentTime - previousTimePositionUpdate;
        previousTimePositionUpdate = currentTime;

        if (deltaMicrosPositionUpdate < HZ2US(MIN_POSITION_UPDATE_FREQUENCY_HZ)) {
            updatePositionVelocityController_MC(deltaMicrosPositionUpdate);

            if (navShouldApplyWaypoint() || navShouldApplyRTH()) {
                // In case of waypoint navigation and RTH limit maximum acceleration to lower value
                updatePositionAccelController_MC(deltaMicrosPositionUpdate, NAV_ACCEL_SLOW_XY_MAX);
            }
            else {
                // In case of PH limit acceleration to some high value
                updatePositionAccelController_MC(deltaMicrosPositionUpdate, NAV_ACCELERATION_XY_MAX);
            }
        }
        else {
            resetMulticopterPositionController();
        }

        // Indicate that information is no longer usable
        posControl.flags.horizontalPositionNewData = 0;
    }

    // Update lean angle controller. This update occurs at loop rate
    // TODO: Investigate if PositionLeanAngle controller need to be run @ looprate
    // All updates occur at GPS rate and lean angle should probably be recalculated at that rate as well
    updatePositionLeanAngleController_MC(deltaMicros);
    updatePositionLeanAngleFromRCInput_MC(deltaMicros);

    // Convert target angle (rcAdjustment) to rcCommand, account for the way PID controllers treat the value
    if (posControl.pidProfile->pidController == PID_CONTROLLER_LUX_FLOAT) {
        // LuxFloat is the only PID controller that uses raw rcCommand as target angle
        rcCommand[PITCH] = constrain(posControl.rcAdjustment[PITCH], -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX);
        rcCommand[ROLL] = constrain(posControl.rcAdjustment[ROLL], -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX);
    }
    else {
        // Most PID controllers use 2 * rcCommand as target angle for ANGLE mode
        rcCommand[PITCH] = constrain(posControl.rcAdjustment[PITCH], -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX) / 2;
        rcCommand[ROLL] = constrain(posControl.rcAdjustment[ROLL], -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX) / 2;
    }
}

#endif  // NAV
