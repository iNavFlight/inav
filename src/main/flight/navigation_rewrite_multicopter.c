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
#include "common/maths.h"

#include "drivers/system.h"
#include "drivers/sensor.h"
#include "drivers/accgyro.h"

#include "sensors/sensors.h"
#include "sensors/acceleration.h"
#include "sensors/boardalignment.h"

#include "flight/pid.h"
#include "flight/imu.h"
#include "flight/navigation_rewrite.h"
#include "flight/navigation_rewrite_private.h"

#include "config/runtime_config.h"
#include "config/config.h"

#if defined(NAV)

#define HOVER_ACCZ_THRESHOLD    5.0f    // cm/s/s
#define HOVER_THR_FILTER        0.025f

/*-----------------------------------------------------------
 * Altitude controller for multicopter aircraft
 *-----------------------------------------------------------*/
static int16_t altholdInitialThrottle;      // Throttle input when althold was activated
static float hoverThrottle = 0;
static int16_t rcCommandAdjustedThrottle;
static bool accelLimitingXY = false;        // true if acceleration limiting active

static void updateHoverThrottle(void)
{
    if (hoverThrottle <= posControl.escAndServoConfig->minthrottle) {
        // FIXME: make this configurable
        hoverThrottle = posControl.rxConfig->midrc;
    }
    else {
        if (posControl.flags.hasValidAltitudeSensor && posControl.actualState.acc.V.Z <= HOVER_ACCZ_THRESHOLD) {
            hoverThrottle += (rcCommand[THROTTLE] - hoverThrottle) * HOVER_THR_FILTER;
        }
    }

    // FIXME: Make us of this after verification that it works
    NAV_BLACKBOX_DEBUG(0, hoverThrottle);
}

static void updateAltitudeTargetFromRCInput_MC(uint32_t deltaMicros)
{
    // In some cases pilot has no control over flight direction
    if (!navCanAdjustAltitudeFromRCInput()) {
        posControl.flags.isAdjustingAltitude = false;
        return;
    }

    int16_t rcThrottleAdjustment = applyDeadband(rcCommand[THROTTLE] - altholdInitialThrottle, posControl.navConfig->alt_hold_deadband);
    if (rcThrottleAdjustment) {
        // set velocity proportional to stick movement
        float rcClimbRate = rcThrottleAdjustment * posControl.navConfig->max_manual_climb_rate / (500.0f - posControl.navConfig->alt_hold_deadband);
        updateAltitudeTargetFromClimbRate(deltaMicros, rcClimbRate);
        posControl.flags.isAdjustingAltitude = true;
    }
    else {
        // Adjusting finished - reset desired position to stay exactly where pilot released the stick
        if (posControl.flags.isAdjustingAltitude) {
            updateAltitudeTargetFromClimbRate(deltaMicros, 0);
        }
        posControl.flags.isAdjustingAltitude = false;
    }
}

// Position to velocity controller for Z axis
static void updateAltitudeVelocityController_MC(void)
{
    float altitudeError = posControl.desiredState.pos.V.Z - posControl.actualState.pos.V.Z;
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
    float velError = posControl.desiredState.vel.V.Z - posControl.actualState.vel.V.Z;
    velError = navApplyFilter(velError, NAV_VEL_ERROR_CUTOFF_FREQENCY_HZ, US2S(deltaMicros), &velFilterState);
    posControl.desiredState.acc.V.Z = velError * posControl.pids.vel[Z].param.kP;
}

static void updateAltitudeThrottleController_MC(uint32_t deltaMicros)
{
    static float throttleFilterState;
    float accError = posControl.desiredState.acc.V.Z - posControl.actualState.acc.V.Z;
    posControl.rcAdjustment[THROTTLE] = navPidGetPID(accError, US2S(deltaMicros), &posControl.pids.accz, false);
    posControl.rcAdjustment[THROTTLE] = navApplyFilter(posControl.rcAdjustment[THROTTLE], NAV_THROTTLE_CUTOFF_FREQENCY_HZ, US2S(deltaMicros), &throttleFilterState);
    posControl.rcAdjustment[THROTTLE] = constrain(posControl.rcAdjustment[THROTTLE], -500, 500);
}

void setupMulticopterAltitudeController(void)
{
    if (posControl.navConfig->flags.use_midrc_for_althold) {
        altholdInitialThrottle = posControl.rxConfig->midrc;
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
    static navigationTimer_t targetPositionUpdateTimer; // Occurs @ POSITION_TARGET_UPDATE_RATE_HZ
    static uint32_t previousTimePositionUpdate;         // Occurs @ altitude sensor update rate (max MAX_ALTITUDE_UPDATE_RATE_HZ)
    static uint32_t previousTimeUpdate;                 // Occurs @ looptime rate

    uint32_t deltaMicros = currentTime - previousTimeUpdate;
    previousTimeUpdate = currentTime;

    // If last position update was too long in the past - ignore it (likely restarting altitude controller)
    if (deltaMicros > HZ2US(MIN_POSITION_UPDATE_RATE_HZ)) {
        resetTimer(&targetPositionUpdateTimer, currentTime);
        previousTimeUpdate = currentTime;
        previousTimePositionUpdate = currentTime;
        resetMulticopterAltitudeController();
        return;
    }

    // Update altitude target from RC input or RTL controller
    if (updateTimer(&targetPositionUpdateTimer, HZ2US(POSITION_TARGET_UPDATE_RATE_HZ), currentTime)) {
        uint32_t deltaMicrosPositionTargetUpdate = getTimerDeltaMicros(&targetPositionUpdateTimer);

        if (navShouldApplyAutonomousLandingLogic()) {
            // Gradually reduce descent speed depending on actual altitude.
            if (posControl.actualState.pos.V.Z > (posControl.homePosition.pos.V.Z + 1000)) {
                updateAltitudeTargetFromClimbRate(deltaMicrosPositionTargetUpdate, -200.0f);
            }
            else if (posControl.actualState.pos.V.Z > (posControl.homePosition.pos.V.Z + 250)) {
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
        if (deltaMicrosPositionUpdate < HZ2US(MIN_POSITION_UPDATE_RATE_HZ)) {
            updateAltitudeVelocityController_MC();
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

    rcCommand[THROTTLE] = constrain(altholdInitialThrottle + posControl.rcAdjustment[THROTTLE], posControl.escAndServoConfig->minthrottle, posControl.escAndServoConfig->maxthrottle);

    // Save processed throttle for future use
    rcCommandAdjustedThrottle = rcCommand[THROTTLE];
}

/*-----------------------------------------------------------
 * Heading controller for multicopter aircraft
 *-----------------------------------------------------------*/

/*-----------------------------------------------------------
 * Calculate rcAdjustment for YAW
 *-----------------------------------------------------------*/
static void calculateHeadingAdjustment_MC(void)
{
    // Calculate yaw correction
    int32_t headingError = wrap_18000(posControl.actualState.yaw - posControl.desiredState.yaw) * posControl.yawControlDirection;
    headingError = constrain(headingError, -3000, 3000); // limit error to +- 30 degrees to avoid fast rotation

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
    if (!navCanAdjustHeadingFromRCInput()) {
        posControl.flags.isAdjustingHeading = false;
        return;
    }

    // Passthrough yaw input if stick is moved
    int16_t rcYawAdjustment = applyDeadband(rcCommand[YAW], posControl.navConfig->pos_hold_deadband);
    if (rcYawAdjustment) {
        posControl.rcAdjustment[YAW] = rcYawAdjustment;

        // Can only allow pilot to set the new heading if doing PH, during RTH copter will target itself to home
        if (navShouldApplyPosHold()) {
            posControl.desiredState.yaw = posControl.actualState.yaw;
        }

        posControl.flags.isAdjustingHeading = true;
    }
    else {
        posControl.flags.isAdjustingHeading = false;
    }
}

void applyMulticopterHeadingController(void)
{
    if (posControl.flags.headingNewData) {
#if defined(NAV_BLACKBOX)
        navDesiredHeading = constrain(lrintf(posControl.desiredState.yaw), -32678, 32767);
#endif

        calculateHeadingAdjustment_MC();
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
static float mcPosControllerAccFilterStateX = 0.0f, mcPosControllerAccFilterStateY = 0.0f;
static float lastAccelTargetX = 0.0f, lastAccelTargetY = 0.0f;

void resetMulticopterPositionController(void)
{
    int axis;
    for (axis = 0; axis < 2; axis++) {
        navPidReset(&posControl.pids.vel[axis]);
        posControl.rcAdjustment[axis] = 0;
        mcPosControllerAccFilterStateX = 0.0f;
        mcPosControllerAccFilterStateY = 0.0f;
        lastAccelTargetX = 0.0f;
        lastAccelTargetY = 0.0f;
    }
}

static void updatePositionTargetFromRCInput_MC(void)
{
    // In some cases pilot has no control over flight direction
    if (!navCanAdjustHorizontalVelocityAndAttitudeFromRCInput()) {
        posControl.flags.isAdjustingPosition = false;
        return;
    }

    int16_t rcPitchAdjustment = applyDeadband(rcCommand[PITCH], posControl.navConfig->pos_hold_deadband);
    int16_t rcRollAdjustment = applyDeadband(rcCommand[ROLL], posControl.navConfig->pos_hold_deadband);

    if (rcPitchAdjustment || rcRollAdjustment) {
        float rcVelX = rcPitchAdjustment * posControl.navConfig->max_manual_speed / (500.0f - posControl.navConfig->pos_hold_deadband);
        float rcVelY = rcRollAdjustment * posControl.navConfig->max_manual_speed / (500.0f - posControl.navConfig->pos_hold_deadband);

        // Calculate rotation coefficients
        float sinYaw = sin_approx(posControl.actualState.yaw * RADX100);
        float cosYaw = cos_approx(posControl.actualState.yaw * RADX100);

        // Rotate these velocities from body frame to to earth frame
        float neuVelX = rcVelX * cosYaw - rcVelY * sinYaw;
        float neuVelY = rcVelX * sinYaw + rcVelY * cosYaw;

        // Calculate new position target, so Pos-to-Vel P-controller would yield desired velocity
        posControl.desiredState.pos.V.X = posControl.actualState.pos.V.X + (neuVelX / posControl.pids.pos[X].param.kP);
        posControl.desiredState.pos.V.Y = posControl.actualState.pos.V.Y + (neuVelY / posControl.pids.pos[Y].param.kP);

        posControl.flags.isAdjustingPosition = true;
    }
    else {
        // Adjusting finished - reset desired position to stay exactly where pilot released the stick
        if (posControl.flags.isAdjustingPosition) {
            posControl.desiredState.pos.V.X = posControl.actualState.pos.V.X;
            posControl.desiredState.pos.V.Y = posControl.actualState.pos.V.Y;
        }

        posControl.flags.isAdjustingPosition = false;
    }
}

static void updatePositionLeanAngleFromRCInput_MC()
{
    // In some cases pilot has no control over flight direction
    if (!navCanAdjustHorizontalVelocityAndAttitudeFromRCInput()) {
        posControl.flags.isAdjustingPosition = false;
        return;
    }

    int16_t rcRollAdjustment = applyDeadband(rcCommand[ROLL], posControl.navConfig->pos_hold_deadband);
    int16_t rcPitchAdjustment = applyDeadband(rcCommand[PITCH], posControl.navConfig->pos_hold_deadband);

    if (rcPitchAdjustment || rcRollAdjustment) {
        // Direct attitude control
        posControl.rcAdjustment[ROLL] = rcCommandToLeanAngle(rcCommand[ROLL]);
        posControl.rcAdjustment[PITCH] = rcCommandToLeanAngle(rcCommand[PITCH]);

        // If we are in position hold mode, so adjust poshold position
        if (navShouldApplyPosHold()) {
            posControl.desiredState.pos.V.X = posControl.actualState.pos.V.X;
            posControl.desiredState.pos.V.Y = posControl.actualState.pos.V.Y;
        }

        // When sticks are released we should restart velocity PIDs
        navPidReset(&posControl.pids.vel[X]);
        navPidReset(&posControl.pids.vel[Y]);

        posControl.flags.isAdjustingPosition = true;
    }
    else {
        posControl.flags.isAdjustingPosition = false;
    }
}

static void updatePositionVelocityController_MC(void)
{
    float posErrorX = posControl.desiredState.pos.V.X - posControl.actualState.pos.V.X;
    float posErrorY = posControl.desiredState.pos.V.Y - posControl.actualState.pos.V.Y;

    float newVelX = posErrorX * posControl.pids.pos[X].param.kP;
    float newVelY = posErrorY * posControl.pids.pos[Y].param.kP;
    float newVelTotal = sqrtf(sq(newVelX) + sq(newVelY));

    if (newVelTotal > posControl.navConfig->max_speed) {
        newVelX = posControl.navConfig->max_speed * (newVelX / newVelTotal);
        newVelY = posControl.navConfig->max_speed * (newVelY / newVelTotal);
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
    float velError, newAccelX, newAccelY;

    // Calculate acceleration target on X-axis
    velError = posControl.desiredState.vel.V.X - posControl.actualState.vel.V.X;
    newAccelX = navPidGetPID(velError, US2S(deltaMicros), &posControl.pids.vel[X], accelLimitingXY);

    // Calculate acceleration target on Y-axis
    velError = posControl.desiredState.vel.V.Y - posControl.actualState.vel.V.Y;
    newAccelY = navPidGetPID(velError, US2S(deltaMicros), &posControl.pids.vel[Y], accelLimitingXY);

#if defined(NAV_BLACKBOX)
    // NAV_BLACKBOX_DEBUG(0, lrintf(posControl.pids.vel[X].lastP));
    // NAV_BLACKBOX_DEBUG(1, lrintf(posControl.pids.vel[X].lastI));
    // NAV_BLACKBOX_DEBUG(2, lrintf(posControl.pids.vel[Y].lastP));
    // NAV_BLACKBOX_DEBUG(3, lrintf(posControl.pids.vel[Y].lastI));
#endif

    // Check if required acceleration exceeds maximum allowed accel
    float newAccelTotal = sqrtf(sq(newAccelX) + sq(newAccelY));

    // Recalculate acceleration
    if (newAccelTotal > maxAccelLimit) {
        accelLimitingXY = true;
        newAccelX *= maxAccelLimit / newAccelTotal;
        newAccelY *= maxAccelLimit / newAccelTotal;
    }
    else {
        accelLimitingXY = false;
    }

    // apply jerk limit of 10 m/s^3
    float maxAccelChange = US2S(deltaMicros) * 1000.0f;
    float accelChangeMagnitude = sqrtf(sq(newAccelX - lastAccelTargetX) + sq(newAccelY - lastAccelTargetY));

    if(accelChangeMagnitude > maxAccelChange) {
        newAccelX = lastAccelTargetX + (newAccelX - lastAccelTargetX) * (maxAccelChange / accelChangeMagnitude);
        newAccelY = lastAccelTargetY + (newAccelY - lastAccelTargetY) * (maxAccelChange / accelChangeMagnitude);
    }

    // Save last acceleration target
    lastAccelTargetX = newAccelX;
    lastAccelTargetY = newAccelY;

    // Apply LPF to jerk limited acceleration target
    posControl.desiredState.acc.V.X = navApplyFilter(newAccelX, NAV_ACCEL_CUTOFF_FREQUENCY_HZ, US2S(deltaMicros), &mcPosControllerAccFilterStateX);
    posControl.desiredState.acc.V.Y = navApplyFilter(newAccelY, NAV_ACCEL_CUTOFF_FREQUENCY_HZ, US2S(deltaMicros), &mcPosControllerAccFilterStateY);
}

static void updatePositionLeanAngleController_MC(void)
{
    // Rotate acceleration target into forward-right frame (aircraft)
    float sinYaw = sin_approx(posControl.actualState.yaw * RADX100);
    float cosYaw = cos_approx(posControl.actualState.yaw * RADX100);

    float accelForward = posControl.desiredState.acc.V.X * cosYaw + posControl.desiredState.acc.V.Y * sinYaw;
    float accelRight = -posControl.desiredState.acc.V.X * sinYaw + posControl.desiredState.acc.V.Y * cosYaw;

    // Calculate banking angles
    float desiredPitch = atan2_approx(accelForward, GRAVITY_CMSS) / RADX100;
    float desiredRoll = atan2_approx(accelRight * cos_approx(desiredPitch * RADX100), GRAVITY_CMSS) / RADX100;

    posControl.rcAdjustment[ROLL] = constrainf(desiredRoll, -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX) * 0.1f;
    posControl.rcAdjustment[PITCH] = constrainf(desiredPitch, -NAV_ROLL_PITCH_MAX, NAV_ROLL_PITCH_MAX) * 0.1f;
}

void applyMulticopterPositionController(uint32_t currentTime)
{
    static navigationTimer_t targetPositionUpdateTimer; // Occurs @ POSITION_TARGET_UPDATE_RATE_HZ
    static uint32_t previousTimePositionUpdate;         // Occurs @ GPS update rate
    static uint32_t previousTimeUpdate;                 // Occurs @ looptime rate
    bool forceGPSAttiMode = false;

    uint32_t deltaMicros = currentTime - previousTimeUpdate;
    previousTimeUpdate = currentTime;

    // If last position update was too long in the past - ignore it (likely restarting position controller)
    if (deltaMicros > HZ2US(MIN_POSITION_UPDATE_RATE_HZ)) {
        resetTimer(&targetPositionUpdateTimer, currentTime);
        previousTimeUpdate = currentTime;
        previousTimePositionUpdate = currentTime;
        resetMulticopterPositionController();
        return;
    }

    // Apply controller only if position source is valid. In absence of valid pos sensor (GPS loss), we'd stick in forced ANGLE mode
    // and pilots input would be passed thru to PID controller
    if (posControl.flags.hasValidPositionSensor) {
        // Update position target from RC input
        if (updateTimer(&targetPositionUpdateTimer, HZ2US(POSITION_TARGET_UPDATE_RATE_HZ), currentTime)) {
            if (posControl.navConfig->flags.user_control_mode == NAV_GPS_CRUISE && !forceGPSAttiMode) {
                updatePositionTargetFromRCInput_MC();
            }
        }

        // If we have new position - update velocity and acceleration controllers
        if (posControl.flags.horizontalPositionNewData) {
            uint32_t deltaMicrosPositionUpdate = currentTime - previousTimePositionUpdate;
            previousTimePositionUpdate = currentTime;

            if (deltaMicrosPositionUpdate < HZ2US(MIN_POSITION_UPDATE_RATE_HZ)) {
                updatePositionVelocityController_MC();

                if (navShouldApplyWaypoint() || navShouldApplyRTH()) {
                    // In case of waypoint navigation and RTH limit maximum acceleration to lower value
                    updatePositionAccelController_MC(deltaMicrosPositionUpdate, NAV_ACCEL_SLOW_XY_MAX);
                }
                else {
                    // In case of PH limit acceleration to some high value
                    updatePositionAccelController_MC(deltaMicrosPositionUpdate, NAV_ACCELERATION_XY_MAX);
                }

                // Update lean angle controller.
                updatePositionLeanAngleController_MC();
            }
            else {
                resetMulticopterPositionController();
            }

            // Indicate that information is no longer usable
            posControl.flags.horizontalPositionNewData = 0;
        }
    }
    else {
        /* No position data, disable automatic adjustment */
        posControl.rcAdjustment[PITCH] = 0;
        posControl.rcAdjustment[ROLL] = 0;

        /* Force GPS_ATTI mode */
        forceGPSAttiMode = true;
    }

    /* Process pilot input (only GPS_ATTI mode) */
    if (posControl.navConfig->flags.user_control_mode == NAV_GPS_ATTI || forceGPSAttiMode) {
        updatePositionLeanAngleFromRCInput_MC();
    }

    rcCommand[PITCH] = leanAngleToRcCommand(posControl.rcAdjustment[PITCH]);
    rcCommand[ROLL] = leanAngleToRcCommand(posControl.rcAdjustment[ROLL]);
}

/*-----------------------------------------------------------
 * Multicopter land detector
 *-----------------------------------------------------------*/
bool isMulticopterLandingDetected(uint32_t * landingTimer)
{
    uint32_t currentTime = micros();

    // Average climb rate should be low enough
    bool verticalMovement = fabsf(posControl.actualState.vel.V.Z) > 25.0f;

    // check if we are moving horizontally
    bool horizontalMovement = sqrtf(sq(posControl.actualState.vel.V.X) + sq(posControl.actualState.vel.V.Y)) > 100.0f;

    // Throttle should be low enough
    // We use rcCommandAdjustedThrottle to keep track of NAV corrected throttle (isLandingDetected is executed
    // from processRx() and rcCommand at that moment holds rc input, not adjusted values from NAV core)
    bool minimalThrust = rcCommandAdjustedThrottle <= (posControl.escAndServoConfig->minthrottle + (posControl.escAndServoConfig->maxthrottle - posControl.escAndServoConfig->minthrottle) * 0.25f);

    if (!minimalThrust || !navShouldApplyAutonomousLandingLogic() || !navShouldApplyAltHold() || verticalMovement || horizontalMovement) {
        *landingTimer = currentTime;
        return false;
    }
    else {
        return ((currentTime - *landingTimer) > (LAND_DETECTOR_TRIGGER_TIME_MS * 1000)) ? true : false;
    }
}

/*-----------------------------------------------------------
 * Multicopter emergency landing
 *-----------------------------------------------------------*/
void applyMulticopterEmergencyLandingController(void)
{
    rcCommand[ROLL] = 0;
    rcCommand[PITCH] = 0;
    rcCommand[YAW] = 0;
    rcCommand[THROTTLE] = 1300; // FIXME
}

/*-----------------------------------------------------------
 * Multicopter-specific automatic parameter update 
 *-----------------------------------------------------------*/
void updateMulticopterSpecificData(uint32_t currentTime)
{
    UNUSED(currentTime);
    updateHoverThrottle();
}

#endif  // NAV
