/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

/* --------------------------------------------------------------------------------
 * == RTH Trackback ==
 * Saves track during flight which is used during RTH to back track
 * along arrival route rather than immediately heading directly toward home.
 * Max desired trackback distance set by user or limited by number of available points.
 * Reverts to normal RTH heading direct to home when end of track reached.
 * Trackpoints logged with precedence for course/altitude changes. Distance based changes
 * only logged if no course/altitude changes logged over an extended distance.
 * Tracking suspended during fixed wing loiter (PosHold and WP Mode timed hold).
 * --------------------------------------------------------------------------------- */

#include "platform.h"

#include "fc/multifunction.h"
#include "fc/rc_controls.h"

#include "navigation/rth_trackback.h"
#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

rth_trackback_t rth_trackback;

bool rthTrackBackCanBeActivated(void)
{
    return posControl.flags.estPosStatus >= EST_USABLE &&
           (navConfig()->general.flags.rth_trackback_mode == RTH_TRACKBACK_ON || (navConfig()->general.flags.rth_trackback_mode == RTH_TRACKBACK_FS && posControl.flags.forcedRTHActivated));
}

void rthTrackBackUpdate(bool forceSaveTrackPoint)
{
    static bool suspendTracking = false;
    bool fwLoiterIsActive = STATE(AIRPLANE) && (NAV_Status.state == MW_NAV_STATE_HOLD_TIMED || FLIGHT_MODE(NAV_POSHOLD_MODE));

    if (!fwLoiterIsActive && suspendTracking) {
        suspendTracking = false;
    }

    if (navConfig()->general.flags.rth_trackback_mode == RTH_TRACKBACK_OFF || FLIGHT_MODE(NAV_RTH_MODE) || !ARMING_FLAG(ARMED) || suspendTracking) {
        return;
    }

    // Record trackback points based on significant change in course/altitude until points limit reached. Overwrite older points from then on.
    if (posControl.flags.estPosStatus >= EST_USABLE && posControl.flags.estAltStatus >= EST_USABLE) {
        static int32_t previousTBTripDist;      // cm
        static int16_t previousTBCourse;        // degrees
        static int16_t previousTBAltitude;      // meters
        static uint8_t distanceCounter = 0;
        bool saveTrackpoint = forceSaveTrackPoint;
        bool GPSCourseIsValid = isGPSHeadingValid();

        // Start recording when some distance from home
        if (rth_trackback.activePointIndex < 0) {
            saveTrackpoint = posControl.homeDistance > METERS_TO_CENTIMETERS(NAV_RTH_TRACKBACK_MIN_DIST_TO_START);
            previousTBCourse = CENTIDEGREES_TO_DEGREES(posControl.actualState.cog);
            previousTBTripDist = posControl.totalTripDistance;
        } else {
            // Minimum distance increment between course change track points when GPS course valid
            const bool distanceIncrement = posControl.totalTripDistance - previousTBTripDist > METERS_TO_CENTIMETERS(NAV_RTH_TRACKBACK_MIN_TRIP_DIST_TO_SAVE);

            // Altitude change
            if (ABS(previousTBAltitude - CENTIMETERS_TO_METERS(posControl.actualState.abs.pos.z)) > NAV_RTH_TRACKBACK_MIN_Z_DIST_TO_SAVE) {
                saveTrackpoint = true;
            } else if (distanceIncrement && GPSCourseIsValid) {
                // Course change - set to 45 degrees
                if (ABS(wrap_18000(DEGREES_TO_CENTIDEGREES(DECIDEGREES_TO_DEGREES(gpsSol.groundCourse) - previousTBCourse))) > DEGREES_TO_CENTIDEGREES(45)) {
                    saveTrackpoint = true;
                } else if (distanceCounter >= 9) {
                    // Distance based trackpoint logged if at least 10 distance increments occur without altitude or course change and deviation from projected course path > 20m
                    float distToPrevPoint = calculateDistanceToDestination(&rth_trackback.pointsList[rth_trackback.activePointIndex]);

                    fpVector3_t virtualCoursePoint;
                    virtualCoursePoint.x = rth_trackback.pointsList[rth_trackback.activePointIndex].x + distToPrevPoint * cos_approx(DEGREES_TO_RADIANS(previousTBCourse));
                    virtualCoursePoint.y = rth_trackback.pointsList[rth_trackback.activePointIndex].y + distToPrevPoint * sin_approx(DEGREES_TO_RADIANS(previousTBCourse));

                    saveTrackpoint = calculateDistanceToDestination(&virtualCoursePoint) > METERS_TO_CENTIMETERS(NAV_RTH_TRACKBACK_MIN_XY_DIST_TO_SAVE);
                }
                distanceCounter++;
                previousTBTripDist = posControl.totalTripDistance;
            } else if (!GPSCourseIsValid) {
                // If no reliable course revert to basic distance logging based on direct distance from last point
                saveTrackpoint = calculateDistanceToDestination(&rth_trackback.pointsList[rth_trackback.activePointIndex]) > METERS_TO_CENTIMETERS(NAV_RTH_TRACKBACK_MIN_XY_DIST_TO_SAVE);
                previousTBTripDist = posControl.totalTripDistance;
            }

            // Suspend tracking during loiter on fixed wing. Save trackpoint at start of loiter.
            if (distanceCounter && fwLoiterIsActive) {
                saveTrackpoint = suspendTracking = true;
            }
        }

        // When trackpoint store full, overwrite from start of store using 'WrapAroundCounter' to track overwrite position
        if (saveTrackpoint) {
            if (rth_trackback.activePointIndex == (NAV_RTH_TRACKBACK_POINTS - 1)) {   // Wraparound to start
                rth_trackback.WrapAroundCounter = rth_trackback.activePointIndex = 0;
            } else {
                rth_trackback.activePointIndex++;
                if (rth_trackback.WrapAroundCounter > -1) {   // Track wraparound overwrite position after store first filled
                    rth_trackback.WrapAroundCounter = rth_trackback.activePointIndex;
                }
            }

            rth_trackback.pointsList[rth_trackback.activePointIndex] = posControl.actualState.abs.pos;
            rth_trackback.lastSavedIndex = rth_trackback.activePointIndex;
            previousTBAltitude = CENTIMETERS_TO_METERS(posControl.actualState.abs.pos.z);
            previousTBCourse = GPSCourseIsValid ? DECIDEGREES_TO_DEGREES(gpsSol.groundCourse) : previousTBCourse;
            distanceCounter = 0;
        }
    }
}

bool rthTrackBackSetNewPosition(void)
{
    if (posControl.flags.estPosStatus == EST_NONE) {
        return false;   // will fall back to RTH initialize allowing full RTH to handle position loss correctly
    }

    const int32_t distFromStartTrackback = CENTIMETERS_TO_METERS(calculateDistanceToDestination(&rth_trackback.pointsList[rth_trackback.lastSavedIndex]));

#ifdef USE_MULTI_FUNCTIONS
    const bool overrideTrackback = rthAltControlStickOverrideCheck(ROLL) || MULTI_FUNC_FLAG(MF_SUSPEND_TRACKBACK);
#else
    const bool overrideTrackback = rthAltControlStickOverrideCheck(ROLL);
#endif
    const bool cancelTrackback = distFromStartTrackback > navConfig()->general.rth_trackback_distance || (overrideTrackback && !posControl.flags.forcedRTHActivated);

    if (rth_trackback.activePointIndex < 0 || cancelTrackback) {
        rth_trackback.WrapAroundCounter = rth_trackback.activePointIndex = -1;
        posControl.flags.rthTrackbackActive = false;
        return false;    // No more trackback points to set, procede to home
    }

    if (isWaypointReached(&posControl.activeWaypoint.pos, &posControl.activeWaypoint.bearing)) {
        rth_trackback.activePointIndex--;

        if (rth_trackback.WrapAroundCounter > -1 && rth_trackback.activePointIndex < 0) {
            rth_trackback.activePointIndex = NAV_RTH_TRACKBACK_POINTS - 1;
        }

        calculateAndSetActiveWaypointToLocalPosition(getRthTrackBackPosition());

        if (rth_trackback.activePointIndex - rth_trackback.WrapAroundCounter == 0) {
            rth_trackback.WrapAroundCounter = rth_trackback.activePointIndex = -1;
        }
    } else {
        setDesiredPosition(getRthTrackBackPosition(), 0, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_BEARING);
    }

    return true;
}

fpVector3_t *getRthTrackBackPosition(void)
{
    // Ensure trackback altitude never lower than altitude of start point
    if (rth_trackback.pointsList[rth_trackback.activePointIndex].z < rth_trackback.pointsList[rth_trackback.lastSavedIndex].z) {
        rth_trackback.pointsList[rth_trackback.activePointIndex].z = rth_trackback.pointsList[rth_trackback.lastSavedIndex].z;
    }

    return &rth_trackback.pointsList[rth_trackback.activePointIndex];
}

void resetRthTrackBack(void)
{
    rth_trackback.activePointIndex = -1;
    posControl.flags.rthTrackbackActive = false;
    rth_trackback.WrapAroundCounter = -1;
}