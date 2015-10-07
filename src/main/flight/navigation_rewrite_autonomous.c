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
 * Autonomous navigation controller (RTH/WP)
 *-----------------------------------------------------------*/

void setupAutonomousControllerRTH(void)
{
    if (ARMING_FLAG(ARMED)) {
        if (!navShouldApplyRTH()) {
            switch (posControl.navConfig->flags.rth_alt_control_style) {
            case NAV_RTH_NO_ALT:
                posControl.homeWaypointAbove.pos.V.Z = posControl.actualState.pos.V.Z;
                break;
            case NAX_RTH_EXTRA_ALT: // Maintain current altitude + predefined safety margin
                posControl.homeWaypointAbove.pos.V.Z = posControl.actualState.pos.V.Z + posControl.navConfig->rth_altitude;
                break;
            case NAV_RTH_MAX_ALT:
                posControl.homeWaypointAbove.pos.V.Z = MAX(posControl.homeWaypointAbove.pos.V.Z, posControl.actualState.pos.V.Z);
                break;
            case NAV_RTH_AT_LEAST_ALT:  // Climb to at least some predefined altitude above home
                posControl.homeWaypointAbove.pos.V.Z = MAX(posControl.homePosition.pos.V.Z + posControl.navConfig->rth_altitude, posControl.actualState.pos.V.Z);
                break;
            case NAV_RTH_CONST_ALT:     // Climb/descend to predefined altitude above home
            default:
                posControl.homeWaypointAbove.pos.V.Z = posControl.homePosition.pos.V.Z + posControl.navConfig->rth_altitude;
                break;
            }
        }
    }
    else {
        posControl.homeWaypointAbove.pos.V.Z = posControl.actualState.pos.V.Z;
    }
}

void resetAutonomousControllerForWP(void)
{
    posControl.navMissionState = NAV_AUTO_WP_INIT;
}

void resetAutonomousControllerForRTH(void)
{
    posControl.navMissionState = NAV_AUTO_RTH_INIT;
}

void setDesiredPositionToWaypointAndUpdateInitialBearing(navWaypointPosition_t * waypoint)
{
    // Calculate initial bearing towards waypoint and store it in waypoint yaw parameter (this will further be used to detect missed waypoints)
    waypoint->yaw = calculateBearingToDestination(&waypoint->pos);

    // Set desired position to next waypoint (XYZ-controller)
    setDesiredPosition(&waypoint->pos, waypoint->yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING);
}

/**
 * Executed autonomous navigation - WP, RTH or AUTOLAND
 *  Update rate: RX (data driven or 50Hz)
 */
void applyAutonomousController(void)
{
    /* Local variables */
    static uint8_t activeWaypointIndex;

    /* Check if we should apply autonomous functions */
    if (!(navShouldApplyWaypoint() || navShouldApplyRTH()))
        return;

    bool reprocessState = false;

    do {
        switch (posControl.navMissionState) {
            /* Init RTH sequence */
            case NAV_AUTO_RTH_INIT:
                /* 3D RTH mode: climb to safe altitude */
                if (posControl.mode == NAV_MODE_RTH) {
                    if (posControl.homeDistance < posControl.navConfig->min_rth_distance) {
                        // Close to original home - reset home to currect position
                        setHomePosition(&posControl.actualState.pos, posControl.actualState.yaw);
                        // Set position lock on home and heading to original heading, altitude to current altitude
                        setDesiredPosition(&posControl.homeWaypointAbove.pos, posControl.homeWaypointAbove.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                        setDesiredPosition(&posControl.actualState.pos, 0, NAV_POS_UPDATE_Z);
                        posControl.navMissionState = NAV_AUTO_AUTOLAND_INIT;
                        reprocessState = true;
                    }
                    else {
                        // Climb/descend to safe altitude if needed
                        setDesiredPosition(&posControl.actualState.pos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                        setDesiredPosition(&posControl.homeWaypointAbove.pos, 0, NAV_POS_UPDATE_Z);
                        posControl.navMissionState = NAV_AUTO_RTH_CLIMB_TO_SAVE_ALTITUDE;
                        reprocessState = true;
                    }
                }
                /* 2D RTH mode: head straight home */
                else if (posControl.mode == NAV_MODE_RTH_2D) {
                    if (posControl.homeDistance < posControl.navConfig->min_rth_distance) {
                        setHomePosition(&posControl.actualState.pos, posControl.actualState.yaw);
                        setDesiredPosition(&posControl.homeWaypointAbove.pos, posControl.homeWaypointAbove.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                        posControl.navMissionState = NAV_AUTO_FINISHED;
                        reprocessState = true;
                    }
                    else {
                        // In case of 2D RTH - head home immediately
                        setDesiredPosition(&posControl.homeWaypointAbove.pos, posControl.homeWaypointAbove.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_BEARING);
                        posControl.navMissionState = NAV_AUTO_RTH_HEAD_HOME;
                        reprocessState = false;
                    }
                }
                else {
                    reprocessState = false;
                }
                break;

            /* Climb to safe altitude */
            case NAV_AUTO_RTH_CLIMB_TO_SAVE_ALTITUDE:
                if (posControl.mode == NAV_MODE_RTH) {
                    if (fabsf(posControl.actualState.pos.V.Z - posControl.homeWaypointAbove.pos.V.Z) < 50.0f) {
                        // Set target position to home and calculate original bearing
                        setDesiredPosition(&posControl.homeWaypointAbove.pos, posControl.homeWaypointAbove.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_BEARING);
                        setDesiredPosition(&posControl.homeWaypointAbove.pos, 0, NAV_POS_UPDATE_Z);
                        posControl.navMissionState = NAV_AUTO_RTH_HEAD_HOME;
                        reprocessState = false;
                    }
                    else {
                        /* Check for emergency condition: GPS loss */
                        /*
                        if (!posControl.flags.hasValidPositionSensor)
                            posControl.navMissionState = NAV_AUTO_EMERGENCY_LANDING_INIT;
                        */
                        reprocessState = false;
                    }
                }
                else {
                    reprocessState = false;
                }
                break;

            /* Head home */
            case NAV_AUTO_RTH_HEAD_HOME:
                /* 3D RTH mode: head home with respect to altitude */
                if (posControl.mode == NAV_MODE_RTH) {
                    // Stay at this state until home reached
                    if (isWaypointReached(&posControl.homeWaypointAbove)) {
                        // Set position lock on home and heading to original heading when lauched
                        setDesiredPosition(&posControl.homePosition.pos, posControl.homePosition.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                        setDesiredPosition(&posControl.homeWaypointAbove.pos, 0, NAV_POS_UPDATE_Z);
                        posControl.navMissionState = NAV_AUTO_AUTOLAND_INIT;
                        reprocessState = true;
                    }
                    else {
                        // Update XY-position target
                        setDesiredPosition(&posControl.homeWaypointAbove.pos, 0, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_BEARING);

                        /* Check for emergency condition: GPS loss */
                        /*
                        if (!posControl.flags.hasValidPositionSensor)
                            posControl.navMissionState = NAV_AUTO_EMERGENCY_LANDING_INIT;
                        */
                        reprocessState = false;
                    }
                }
                /* 2D RTH mode: head home without altitude control */
                else if (posControl.mode == NAV_MODE_RTH_2D) {
                    // Stay at this state until home reached
                    if (isWaypointReached(&posControl.homeWaypointAbove)) {
                        // Set position lock on home and heading to original heading when launched
                        setDesiredPosition(&posControl.homePosition.pos, posControl.homePosition.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
                        posControl.navMissionState = NAV_AUTO_FINISHED;
                        reprocessState = true;
                    }
                    else {
                        // Update XY-position target
                        setDesiredPosition(&posControl.homeWaypointAbove.pos, 0, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_BEARING);

                        /* Check for emergency condition: GPS loss */
                        /*
                        if (!posControl.flags.hasValidPositionSensor)
                            posControl.navMissionState = NAV_AUTO_EMERGENCY_LANDING_INIT;
                        */
                        reprocessState = false;
                    }
                }
                else {
                    reprocessState = false;
                }
                break;

            /* Init waypoint mission */
            case NAV_AUTO_WP_INIT:
                if (posControl.waypointCount == 0) {
                    /* No waypoints to cycle through */
                    posControl.navMissionState = NAV_AUTO_FINISHED;
                    reprocessState = true;
                }
                else {
                    activeWaypointIndex = 0;
                    setDesiredPositionToWaypointAndUpdateInitialBearing(&posControl.waypointList[activeWaypointIndex]);
                    posControl.navMissionState = NAV_AUTO_WP;
                    reprocessState = false;
                }
                break;

            case NAV_AUTO_WP:
                if (isWaypointReached(&posControl.waypointList[activeWaypointIndex])) {
                    // Waypoint reached - go to next waypoint
                    activeWaypointIndex++;
                    if (activeWaypointIndex >= posControl.waypointCount) {
                        // This is the last waypoint - finalize
                        posControl.navMissionState = NAV_AUTO_FINISHED;
                        reprocessState = true;
                    }
                    else {
                        setDesiredPositionToWaypointAndUpdateInitialBearing(&posControl.waypointList[activeWaypointIndex]);
                        reprocessState = false;
                    }
                }
                else {
                    // Update XY-position target
                    setDesiredPosition(&posControl.waypointList[activeWaypointIndex].pos, posControl.waypointList[activeWaypointIndex].yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_BEARING);

                    /* Check for emergency condition: GPS loss */
                    /*
                    if (!posControl.flags.hasValidPositionSensor)
                        posControl.navMissionState = NAV_AUTO_EMERGENCY_LANDING_INIT;
                    */

                    // TODO: Moving towards a waypoint, maybe do something here
                    reprocessState = false;
                }
                break;

            /* Initialize autolanding sequence */
            case NAV_AUTO_AUTOLAND_INIT:
                resetLandingDetector();
                posControl.navMissionState = NAV_AUTO_AUTOLAND;
                reprocessState = true;
                break;

            /* Process autolanding */
            case NAV_AUTO_AUTOLAND:
                /* Stay in this state. Actual landing is done by Z-controller */
                if (!ARMING_FLAG(ARMED)) {
                    posControl.navMissionState = NAV_AUTO_FINISHED;
                    reprocessState = true;
                }
                else if (isLandingDetected()) {
                    posControl.navMissionState = NAV_AUTO_LANDED;
                    reprocessState = true;
                }
                else {
                    // Still landing. Altitude controller takes care of gradual descent.
                    // TODO: A safeguard in case of emergency
                    reprocessState = false;
                }
                break;

            /* Emergency landing sequence */
            case NAV_AUTO_EMERGENCY_LANDING_INIT:
                posControl.navMissionState = NAV_AUTO_EMERGENCY_LANDING;
                reprocessState = true;
                break;

            case NAV_AUTO_EMERGENCY_LANDING:
                // TODO: Emergency landing controller MUST be functional in case of total position estimation failure and be aware of possibility
                // that pos. estimator may re-start position estimation. This mode must disable poshold and althold controllers and run
                // an emergency landing controller (similar to current failsafe)
                reprocessState = false;
                break;

            case NAV_AUTO_LANDED:
                posControl.navMissionState = NAV_AUTO_FINISHED;
                reprocessState = true;
                break;

            case NAV_AUTO_FINISHED:
                reprocessState = false;
                break;

            default:
                reprocessState = false;
                break;
        }
    } while (reprocessState);
}

#endif  // NAV
