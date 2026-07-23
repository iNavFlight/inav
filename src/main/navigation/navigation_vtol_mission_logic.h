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

#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    NAV_MISSION_VTOL_PRECONDITION_READY = 0,
    NAV_MISSION_VTOL_PRECONDITION_WAIT,
    NAV_MISSION_VTOL_PRECONDITION_REJECT,
} navMissionVtolTransitionPrecondition_e;

typedef enum {
    NAV_MISSION_VTOL_START_VALIDATION_READY = 0,
    NAV_MISSION_VTOL_START_VALIDATION_FAIL_ACTION,
    NAV_MISSION_VTOL_START_VALIDATION_REJECT,
} navMissionVtolTransitionStartValidation_e;

static inline navMissionVtolTransitionPrecondition_e navMissionVtolTransitionPreconditionDisposition(
    const bool armed,
    const bool failsafeActive,
    const bool sensorsCalibrating,
    const bool positionUsable,
    const bool headingUsable,
    const bool mixerProfileModeConfigured,
    const bool hotSwitchAvailable,
    const bool mixerAtActive)
{
    if (!armed || failsafeActive) {
        return NAV_MISSION_VTOL_PRECONDITION_REJECT;
    }

    if (sensorsCalibrating || !positionUsable || !headingUsable || mixerAtActive) {
        return NAV_MISSION_VTOL_PRECONDITION_WAIT;
    }

    if (!mixerProfileModeConfigured || !hotSwitchAvailable) {
        return NAV_MISSION_VTOL_PRECONDITION_REJECT;
    }

    return NAV_MISSION_VTOL_PRECONDITION_READY;
}

static inline navMissionVtolTransitionStartValidation_e navMissionVtolTransitionStartValidation(
    const bool targetPlatformMatchesRequest,
    const bool transitionRequestAllowed)
{
    if (!targetPlatformMatchesRequest) {
        return NAV_MISSION_VTOL_START_VALIDATION_REJECT;
    }

    if (!transitionRequestAllowed) {
        return NAV_MISSION_VTOL_START_VALIDATION_FAIL_ACTION;
    }

    return NAV_MISSION_VTOL_START_VALIDATION_READY;
}

static inline bool navMissionTransitionWaypointAcceptedForAdvance(
    const bool transitionStartedAtWaypoint,
    const bool ordinaryWaypoint,
    const uint16_t waypointEnforceAltitudeCm,
    const float transitionStartAltitudeCm,
    const float waypointAltitudeCm)
{
    return transitionStartedAtWaypoint &&
           ordinaryWaypoint &&
           (waypointEnforceAltitudeCm == 0 ||
            transitionStartAltitudeCm >= waypointAltitudeCm - waypointEnforceAltitudeCm);
}

static inline bool navMissionTransitionReachedTargetProfile(
    const bool mixerAtAborted,
    const bool forcedProfileSwitchCompleted)
{
    // A configured FORCE_SWITCH completes the requested profile change after
    // an airspeed timeout, so mission handling must distinguish it from a
    // real abort that leaves the old profile active.
    return !mixerAtAborted || forcedProfileSwitchCompleted;
}

static inline bool navMissionShouldAdvanceWaypointAfterTransition(
    const bool waypointAcceptedForAdvance,
    const bool transitionReachedTargetProfile,
    const uint16_t waypointEnforceAltitudeCm,
    const float currentAltitudeCm,
    const float waypointAltitudeCm)
{
    // A transition may add altitude after the waypoint was accepted. Do not
    // make the aircraft loiter back down to the old waypoint, but never skip
    // a waypoint while below its enforced minimum altitude.
    return waypointAcceptedForAdvance &&
           transitionReachedTargetProfile &&
           (waypointEnforceAltitudeCm == 0 ||
            currentAltitudeCm >= waypointAltitudeCm - waypointEnforceAltitudeCm);
}

static inline bool navMissionTransitionShouldCaptureBeforeWaypointResume(
    const bool missionTransitionActive,
    const bool transitionSucceeded,
    const bool transitionedToMultirotor,
    const bool currentStateIsMultirotor,
    const bool waypointShouldAdvance)
{
    // A fixed-wing model may travel well beyond the transition waypoint while
    // it decelerates. Capture its real MC motion before trying to revisit a
    // HOLD_TIME, LAND, or altitude-incomplete waypoint.
    return missionTransitionActive &&
           transitionSucceeded &&
           transitionedToMultirotor &&
           currentStateIsMultirotor &&
           !waypointShouldAdvance;
}

static inline bool navMissionTransitionShouldRebaseWaypointBeforeResume(
    const bool missionTransitionActive,
    const bool transitionSucceeded,
    const bool waypointShouldAdvance,
    const bool missionCapturePending)
{
    // Do not let the bearing that authorized the transition immediately mark
    // the same waypoint as reached after the new platform takes control.
    return missionTransitionActive &&
           transitionSucceeded &&
           !waypointShouldAdvance &&
           !missionCapturePending;
}

static inline bool navMissionVtolTransitionSuppressesWaypointRestartGuard(
    const bool missionTransitionInProgress)
{
    return missionTransitionInProgress;
}

static inline bool navMissionVtolTransitionSuppressesWaypointFallbackToRth(
    const bool missionTransitionInProgress)
{
    return missionTransitionInProgress;
}

static inline bool navMissionVtolTransitionHoldsWaypointSelector(
    const bool missionTransitionInProgress,
    const bool waypointModeSwitchActive)
{
    return missionTransitionInProgress && waypointModeSwitchActive;
}

static inline bool navMissionShouldHoldMultirotorWaypointForAltitude(
    const bool isMultirotor,
    const bool waypointAltitudeEnforceActive,
    const uint16_t waypointEnforceAltitudeCm,
    const float waypointDistanceCm,
    const float waypointInitialDistanceCm,
    const uint16_t waypointRadiusCm,
    const float altitudeErrorCm)
{
    return isMultirotor &&
           waypointEnforceAltitudeCm > 0 &&
           (waypointAltitudeEnforceActive || waypointDistanceCm <= waypointRadiusCm || waypointInitialDistanceCm <= waypointRadiusCm) &&
           altitudeErrorCm >= waypointEnforceAltitudeCm;
}

static inline bool navMissionUpdateMultirotorWaypointAltitudeEnforceActive(
    const bool isMultirotor,
    const bool waypointAltitudeEnforceActive,
    const bool forceFromWaypointStart,
    const uint16_t waypointEnforceAltitudeCm,
    const float waypointDistanceCm,
    const float waypointInitialDistanceCm,
    const uint16_t waypointRadiusCm)
{
    return waypointAltitudeEnforceActive ||
           (isMultirotor &&
            waypointEnforceAltitudeCm > 0 &&
            (forceFromWaypointStart || waypointDistanceCm <= waypointRadiusCm || waypointInitialDistanceCm <= waypointRadiusCm));
}

static inline bool navMissionShouldForceFirstGeoWaypointAltitudeFromStart(
    const bool isMultirotor,
    const bool firstGeoWaypoint)
{
    return isMultirotor && firstGeoWaypoint;
}

static inline bool navMissionShouldHoldFirstMultirotorWaypointXyForAltitude(
    const bool isMultirotor,
    const bool waypointAltitudeEnforceFromStart,
    const uint16_t waypointEnforceAltitudeCm,
    const float altitudeErrorCm)
{
    return isMultirotor &&
           waypointAltitudeEnforceFromStart &&
           waypointEnforceAltitudeCm > 0 &&
           altitudeErrorCm >= waypointEnforceAltitudeCm;
}

static inline float navMissionMultirotorWaypointAltitudeTarget(
    const bool isMultirotor,
    const bool waypointAltitudeEnforceActive,
    const float waypointAltitudeCm,
    const float interpolatedAltitudeCm)
{
    return (isMultirotor && waypointAltitudeEnforceActive) ?
           waypointAltitudeCm :
           interpolatedAltitudeCm;
}
