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

/*-----------------------------------------------------------
 * Altitude controller
 *-----------------------------------------------------------*/
void setupFixedWingAltitudeController(void)
{
    // TODO
}

void resetFixedWingAltitudeController()
{
    navPidReset(&posControl.pids.accz);
    posControl.rcAdjustment[PITCH] = 0;
}

static void updateAltitudeTargetFromRCInput_FW(uint32_t deltaMicros)
{
    // In some cases pilot has no control over flight direction
    if (!navCanAdjustAltitudeFromRCInput()) {
        posControl.flags.isAdjustingAltitude = false;
        return;
    }

    int16_t rcAdjustment = applyDeadband(rcCommand[PITCH], posControl.navConfig->alt_hold_deadband);

    if (rcAdjustment) {
        // set velocity proportional to stick movement
        float rcClimbRate = rcAdjustment * posControl.navConfig->max_manual_climb_rate / (500.0f - posControl.navConfig->alt_hold_deadband);
        updateAltitudeTargetFromClimbRate(deltaMicros, rcClimbRate);
        posControl.flags.isAdjustingAltitude = true;
    }
    else {
        posControl.flags.isAdjustingAltitude = false;
    }
}

// Position to velocity controller for Z axis
static void updateAltitudeVelocityController_FW(uint32_t deltaMicros)
{
    static float velzFilterState;

    float altitudeError = posControl.desiredState.pos.V.Z - posControl.actualState.pos.V.Z;

    // FIXME: Rework PIDs for fixed-wing
    posControl.desiredState.vel.V.Z = navPidGetPID(altitudeError, US2S(deltaMicros), &posControl.pids.accz, false);
    posControl.desiredState.vel.V.Z = navApplyFilter(posControl.desiredState.vel.V.Z, NAV_FW_VEL_CUTOFF_FREQENCY_HZ, US2S(deltaMicros), &velzFilterState);
    posControl.desiredState.vel.V.Z = constrainf(posControl.desiredState.vel.V.Z, -300, 300); // hard limit velocity to +/- 3 m/s

#if defined(NAV_BLACKBOX)
    navDesiredVelocity[Z] = constrain(lrintf(posControl.desiredState.vel.V.Z), -32678, 32767);
    navLatestPositionError[Z] = constrain(lrintf(altitudeError), -32678, 32767);
    navTargetPosition[Z] = constrain(lrintf(posControl.desiredState.pos.V.Z), -32678, 32767);
#endif
}

static void updateAltitudePitchController_FW(void)
{
    // On a fixed wing we might not have a reliable climb rate source (if no BARO available)

    float velocityXY = sqrtf(sq(posControl.actualState.vel.V.X) + sq(posControl.actualState.vel.V.Y));
    velocityXY = MAX(velocityXY, 600.0f);   // Limit min velocity for PID controller at about 20 km/h

    // Calculate pitch angle from target climb rate and actual horizontal velocity
    posControl.rcAdjustment[PITCH] = atan2_approx(posControl.desiredState.vel.V.Z, velocityXY) / RADX100;
    posControl.rcAdjustment[PITCH] = constrain(posControl.rcAdjustment[PITCH], -NAV_ROLL_PITCH_MAX_FW, NAV_ROLL_PITCH_MAX_FW) * 0.1f;
}

void applyFixedWingAltitudeController(uint32_t currentTime)
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
        resetFixedWingAltitudeController();
        return;
    }

    // Update altitude target from RC input or RTL controller
    if (updateTimer(&targetPositionUpdateTimer, HZ2US(POSITION_TARGET_UPDATE_RATE_HZ), currentTime)) {
        if (navShouldApplyAutonomousLandingLogic()) {
            // TODO
        }
        
        updateAltitudeTargetFromRCInput_FW(getTimerDeltaMicros(&targetPositionUpdateTimer));
    }

    // If we have an update on vertical position data - update velocity and accel targets
    if (posControl.flags.verticalPositionNewData) {
        uint32_t deltaMicrosPositionUpdate = currentTime - previousTimePositionUpdate;
        previousTimePositionUpdate = currentTime;

        // Check if last correction was too log ago - ignore this update
        if (deltaMicrosPositionUpdate < HZ2US(MIN_POSITION_UPDATE_RATE_HZ)) {
            updateAltitudeVelocityController_FW(deltaMicrosPositionUpdate);
        }
        else {
            // due to some glitch position update has not occurred in time, reset altitude controller
            resetFixedWingAltitudeController();
        }

        // Indicate that information is no longer usable
        posControl.flags.verticalPositionNewData = 0;
    }

    updateAltitudePitchController_FW();
}

/*-----------------------------------------------------------
 * Calculate rcAdjustment for YAW
 *-----------------------------------------------------------*/
void applyFixedWingHeadingController(void)
{
    if (posControl.flags.headingNewData) {
#if defined(NAV_BLACKBOX)
        navDesiredHeading = constrain(lrintf(posControl.desiredState.yaw), -32678, 32767);
#endif

        // TODO

        // Indicate that information is no longer usable
        posControl.flags.headingNewData = 0;
    }

    // Control yaw by NAV PID
    //rcCommand[YAW] = constrain(posControl.rcAdjustment[YAW], -500, 500);
}

void resetFixedWingHeadingController(void)
{
    // TODO
}

/*-----------------------------------------------------------
 * XY-position controller for multicopter aircraft
 *-----------------------------------------------------------*/
void resetFixedWingPositionController(void)
{
    // TODO
}

void applyFixedWingPositionController(uint32_t currentTime)
{
    static navigationTimer_t targetPositionUpdateTimer; // Occurs @ POSITION_TARGET_UPDATE_RATE_HZ
    static uint32_t previousTimePositionUpdate;         // Occurs @ GPS update rate
    static uint32_t previousTimeUpdate;                 // Occurs @ looptime rate

    uint32_t deltaMicros = currentTime - previousTimeUpdate;
    previousTimeUpdate = currentTime;

    // If last position update was too long in the past - ignore it (likely restarting altitude controller)
    if (deltaMicros > HZ2US(MIN_POSITION_UPDATE_RATE_HZ)) {
        resetTimer(&targetPositionUpdateTimer, currentTime);
        previousTimeUpdate = currentTime;
        previousTimePositionUpdate = currentTime;
        resetFixedWingPositionController();
        return;
    }

    // Update position target from RC input
    if (updateTimer(&targetPositionUpdateTimer, HZ2US(POSITION_TARGET_UPDATE_RATE_HZ), currentTime)) {
        // TODO
    }

    // If we have new position - update velocity and acceleration controllers
    if (posControl.flags.horizontalPositionNewData) {
        uint32_t deltaMicrosPositionUpdate = currentTime - previousTimePositionUpdate;
        previousTimePositionUpdate = currentTime;

        if (deltaMicrosPositionUpdate < HZ2US(MIN_POSITION_UPDATE_RATE_HZ)) {
            // TODO
        }
        else {
            resetFixedWingPositionController();
        }

        // Indicate that information is no longer usable
        posControl.flags.horizontalPositionNewData = 0;
    }

    // TODO
}

/*-----------------------------------------------------------
 * FixedWing land detector
 *-----------------------------------------------------------*/
bool isFixedWingLandingDetected(uint32_t * landingTimer)
{
    uint32_t currentTime = micros();

    // TODO

    *landingTimer = currentTime;
    return false;
}

/*-----------------------------------------------------------
 * FixedWing emergency landing
 *-----------------------------------------------------------*/
void applyFixedWingEmergencyLandingController(void)
{
    // TODO
}

/*-----------------------------------------------------------
 * Fixed-wing-specific automatic parameter update 
 *-----------------------------------------------------------*/
void updateFixedWingSpecificData(uint32_t currentTime)
{
    UNUSED(currentTime);
}

#endif  // NAV
