/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "platform.h"

#if defined(USE_SAFE_HOME) && defined(USE_FW_AUTOLAND)

#include <navigation/navigation_fw_autoland.h>

#include "common/utils.h"
#include "flight/wind_estimator.h"
#include "fc/rc_controls.h"
#include "fc/settings.h"
#include "rx/rx.h"
#include "flight/mixer.h"
#include "navigation/navigation_private.h"
#include "config/parameter_group_ids.h"

#define LOITER_MIN_TIME 30000000 // usec (30 sec)
#define TARGET_ALT_TOLERANCE 150

#define LAND_WP_TURN 0
#define LAND_WP_FINAL_APPROACH 1
#define LAND_WP_LAND 2

typedef enum {
    FW_AUTOLAND_EVENT_NONE,
    FW_AUTOLAND_EVENT_SUCCSESS,
    FW_AUTOLAND_EVENT_ABORT,
    FW_AUTOLAND_EVENT_COUNT
} fwAutolandEvent_t;

typedef struct fixedWingAutolandStateDescriptor_s {
    fwAutolandEvent_t (*onEntry)(timeUs_t currentTimeUs);
    fwAutolandState_t onEvent[FW_AUTOLAND_EVENT_COUNT];
    fwAutolandMessageState_t message;
} fixedWingAutolandStateDescriptor_t;

typedef struct fixedWingAutolandData_s {
    timeUs_t currentStateTimeUs;
    fwAutolandState_t currentState;
    timeUs_t loiterStartTime;
    fpVector3_t waypoint[3];
    fpVector3_t flareWp;
    uint32_t finalApproachLength;
} fixedWingAutolandData_t;

static EXTENDED_FASTRAM fixedWingAutolandData_t fwAutoland;

static fwAutolandEvent_t fwAutolandState_ABOVE_LOITER_ALT(timeUs_t currentTimeUs);
static fwAutolandEvent_t fwAutolandState_LOITER(timeUs_t currentTimeUs);
static fwAutolandEvent_t fwAutolandState_DOWNWIND(timeUs_t currentTimeUs);
static fwAutolandEvent_t fwAutolandState_BASE(timeUs_t currentTimeUs);
static fwAutolandEvent_t fwAutolandState_FINAL_APPROACH(timeUs_t currentTimeUs);
static fwAutolandEvent_t fwAutolandState_GLIDE(timeUs_t currentTimeUs);
static fwAutolandEvent_t fwAutolandState_FLARE(timeUs_t currentTimeUs);
static fwAutolandEvent_t fwAutolandState_ABORT(timeUs_t currentTimeUs);

static const fixedWingAutolandStateDescriptor_t autolandStateMachine[FW_AUTOLAND_STATE_COUNT] = {
    [FW_AUTOLAND_STATE_ABOVE_LOITER_ALT] = {
        .onEntry                            = fwAutolandState_ABOVE_LOITER_ALT,
        .onEvent = {
            [FW_AUTOLAND_EVENT_SUCCSESS]    = FW_AUTOLAND_STATE_LOITER,
            [FW_AUTOLAND_EVENT_ABORT]       = FW_AUTOLAND_STATE_ABORT,
        },
        .message                            = FW_AUTOLAND_MESSAGE_ABOVE_LOITER_ALT
    },
    [FW_AUTOLAND_STATE_LOITER] = {
        .onEntry                            = fwAutolandState_LOITER,
        .onEvent = {
            [FW_AUTOLAND_EVENT_SUCCSESS]    = FW_AUTOLAND_STATE_DOWNWIND,
            [FW_AUTOLAND_EVENT_ABORT]       = FW_AUTOLAND_STATE_ABORT,
        },
        .message                            = FW_AUTOLAND_MESSAGE_LOITER
    },
    [FW_AUTOLAND_STATE_DOWNWIND] = {
        .onEntry                            = fwAutolandState_DOWNWIND,
        .onEvent = {
            [FW_AUTOLAND_EVENT_SUCCSESS]    = FW_AUTOLAND_STATE_BASE,
            [FW_AUTOLAND_EVENT_ABORT]       = FW_AUTOLAND_STATE_ABORT,
        },
        .message                            = FW_AUTOLAND_MESSAGE_LOITER
    }, 
    [FW_AUTOLAND_STATE_BASE] = {
        .onEntry                            = fwAutolandState_BASE,
        .onEvent = {
            [FW_AUTOLAND_EVENT_SUCCSESS]    = FW_AUTOLAND_STATE_FINAL_APPROACH,
            [FW_AUTOLAND_EVENT_ABORT]       = FW_AUTOLAND_STATE_ABORT,
        },
        .message                            = FW_AUTOLAND_MESSAGE_LOITER
    },
    [FW_AUTOLAND_STATE_FINAL_APPROACH] = {
        .onEntry                            = fwAutolandState_FINAL_APPROACH,
        .onEvent = {
            [FW_AUTOLAND_EVENT_SUCCSESS]    = FW_AUTOLAND_STATE_GLIDE,
            [FW_AUTOLAND_EVENT_ABORT]       = FW_AUTOLAND_STATE_ABORT,
        },
        .message                            = FW_AUTOLAND_MESSAGE_LOITER
    },
    [FW_AUTOLAND_STATE_GLIDE] = {
        .onEntry                            = fwAutolandState_GLIDE,
        .onEvent = {
            [FW_AUTOLAND_EVENT_SUCCSESS]    = FW_AUTOLAND_STATE_FLARE,
            [FW_AUTOLAND_EVENT_ABORT]       = FW_AUTOLAND_STATE_ABORT,
        },
        .message                            = FW_AUTOLAND_MESSAGE_LOITER
    },
    [FW_AUTOLAND_STATE_FLARE] = {
        .onEntry                            = fwAutolandState_FLARE,
        .onEvent = {
            [FW_AUTOLAND_EVENT_SUCCSESS]    = FW_AUTOLAND_STATE_LANDED,
            [FW_AUTOLAND_EVENT_ABORT]       = FW_AUTOLAND_STATE_ABORT,
        },
        .message                            = FW_AUTOLAND_MESSAGE_LOITER
    }

};

PG_REGISTER_WITH_RESET_TEMPLATE(navFwAutolandConfig_t, navFwAutolandConfig, PG_FW_AUTOLAND_CONFIG, 0);
PG_RESET_TEMPLATE(navFwAutolandConfig_t, navFwAutolandConfig,
    .approachAngle = SETTING_NAV_FW_LAND_APPROACH_ANGLE_DEFAULT,
    .glideAltitude = SETTING_NAV_FW_LAND_GLIDE_ALT_DEFAULT,
    .flareAltitude = SETTING_NAV_FW_LAND_FLARE_ALT_DEFAULT,
    .flarePitch = SETTING_NAV_FW_LAND_FLARE_PITCH_DEFAULT,
    .maxTailwind = SETTING_NAV_FW_LAND_MAX_TAILWIND_DEFAULT,
);

static uint32_t getFinalApproachHeading(int32_t approachHeading, int32_t windAngle, int32_t *windAngleRelToRunway)
{
    if (approachHeading == 0) {
        return 0;
    }
    
    *windAngleRelToRunway = wrap_36000(windAngle - ABS(approachHeading));
    if (*windAngleRelToRunway > 27000 || *windAngleRelToRunway < 9000) {
        return approachHeading;
    }
   
    if (approachHeading > 0) {
        return wrap_36000(approachHeading + 18000);
    }
   
    return 0;
}

static int32_t calcApproachLength(int32_t finalApproachAlt, int16_t glidePitch) 
{
    return finalApproachAlt * 1.0f / sin_approx(DEGREES_TO_RADIANS(glidePitch)) * sin_approx(DEGREES_TO_RADIANS(90 - glidePitch));
}

static void setLandWaypoint(const fpVector3_t *pos, const fpVector3_t *nextWpPos) 
{
    if (posControl.activeWaypoint.pos.x == 0 && posControl.activeWaypoint.pos.y == 0) {
       posControl.activeWaypoint.bearing = calculateBearingToDestination(pos);
    } else {
       posControl.activeWaypoint.bearing = calculateBearingBetweenLocalPositions(&posControl.activeWaypoint.pos, pos);
    } 
    
    posControl.activeWaypoint.pos = *pos;
    setDesiredPosition(&posControl.activeWaypoint.pos, posControl.activeWaypoint.bearing, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING);

    if (navConfig()->fw.wp_turn_smoothing && nextWpPos != NULL) {
        int32_t bearingToNextWp = calculateBearingBetweenLocalPositions(pos, nextWpPos);
        posControl.activeWaypoint.nextTurnAngle = wrap_18000(bearingToNextWp - posControl.activeWaypoint.bearing);
    } else {
        posControl.activeWaypoint.nextTurnAngle = -1;
    }
    
    posControl.wpInitialDistance = calculateDistanceToDestination(&posControl.activeWaypoint.pos);
    posControl.wpInitialAltitude = posControl.actualState.abs.pos.z;
    posControl.wpAltitudeReached = false;
}

static void checkLandWpAndUpdateZ(void)
{
    fpVector3_t tmpWaypoint;
    tmpWaypoint.x = posControl.activeWaypoint.pos.x;
    tmpWaypoint.y = posControl.activeWaypoint.pos.y;
    tmpWaypoint.z = scaleRangef(constrainf(posControl.wpDistance, posControl.wpInitialDistance / 10.0f, posControl.wpInitialDistance),
    posControl.wpInitialDistance, posControl.wpInitialDistance / 10.0f,
    posControl.wpInitialAltitude, posControl.activeWaypoint.pos.z);
    setDesiredPosition(&tmpWaypoint, 0, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_BEARING);
}

static bool isLandWpReached(const fpVector3_t * waypointPos, const int32_t * waypointBearing)
{
    posControl.wpDistance = calculateDistanceToDestination(waypointPos);

    /*
    if (getFwAutolandState() == FW_AUTOLAND_STATE_BASE) {
         return calculateDistanceToDestination(&fwAutoland.waypoint[LAND_WP_LAND]) <= fwAutoland.finalApproachLength - navConfig()->fw.loiter_radius; 
    }
    */

    if (posControl.flags.wpTurnSmoothingActive) {
        posControl.flags.wpTurnSmoothingActive = false;
        return true;
    }

    uint16_t relativeBearing = posControl.flags.wpTurnSmoothingActive ? 6000 : 10000;
    if (ABS(wrap_18000(calculateBearingToDestination(waypointPos) - *waypointBearing)) > relativeBearing) {
        return true;
    }

   return posControl.wpDistance <= (navConfig()->general.waypoint_radius);
}

static fwAutolandEvent_t fwAutolandState_ABOVE_LOITER_ALT(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);
    int32_t approachAltAbs = safeHomeConfig(safehome_index)->approachAltMSL - GPS_home.alt;

    if (ABS(getEstimatedActualPosition(Z) - approachAltAbs) < TARGET_ALT_TOLERANCE) {
        return FW_AUTOLAND_EVENT_SUCCSESS;
    } else {
        fpVector3_t tmpPoint = posControl.rthState.homePosition.pos;
        tmpPoint.z = approachAltAbs;
        setDesiredPosition(&tmpPoint, 0, NAV_POS_UPDATE_Z);
    }

    return FW_AUTOLAND_EVENT_NONE;
}

static fwAutolandEvent_t fwAutolandState_LOITER(timeUs_t currentTimeUs) 
{
#ifndef USE_WIND_ESTIMATOR
    return FW_AUTOLAND_EVENT_ABORT;
#endif
    
    int32_t landAltAbs = safeHomeConfig(safehome_index)->landAltMSL - GPS_home.alt;
    int32_t approachAltAbs = safeHomeConfig(safehome_index)->approachAltMSL + landAltAbs - GPS_home.alt;
    if (fwAutoland.loiterStartTime == 0) {
        fwAutoland.loiterStartTime = currentTimeUs;
    } else if (micros() - fwAutoland.loiterStartTime > LOITER_MIN_TIME) {
        if (isEstimatedWindSpeedValid()) {
            
            uint16_t windAngle = 0;
            uint32_t landingDirection = 0;
            float windSpeed = getEstimatedHorizontalWindSpeed(&windAngle);
            windAngle = wrap_36000(windAngle + 18000);

            int32_t windAngel1 = 0, windAngel2 = 0;
            int32_t heading1 = getFinalApproachHeading(DEGREES_TO_CENTIDEGREES(safeHomeConfig(safehome_index)->landApproachHeading1), windAngle, &windAngel1);
            int32_t heading2 = getFinalApproachHeading(DEGREES_TO_CENTIDEGREES(safeHomeConfig(safehome_index)->landApproachHeading2), windAngle, &windAngel2);

            if (heading1 == 0 && heading2 == 0 && windSpeed < navFwAutolandConfig()->maxTailwind) {
                heading1 = safeHomeConfig(safehome_index)->landApproachHeading1;
                heading2 = safeHomeConfig(safehome_index)->landApproachHeading2;
            }

            if (heading1 == 0 && heading2 > 0) {
                landingDirection = heading2;
            } else if (heading1 > 0 && heading2 == 0) {
                landingDirection = heading1;
            } else {
                if (windAngel1 < windAngel2) {
                    landingDirection = heading1;
                }
                else {
                    landingDirection = heading2;
                }
            }

            if (landingDirection != 0) {
                fpVector3_t tmpPos;
                
                int32_t finalApproachAlt = approachAltAbs / 3 * 2;
                fwAutoland.finalApproachLength = calcApproachLength(finalApproachAlt, navFwAutolandConfig()->approachAngle);
                
                int32_t dir = 0;
                if (safeHomeConfig(safehome_index)->approachDirection == FW_AUTOLAND_APPROACH_DIRECTION_LEFT) {
                    dir = wrap_36000(landingDirection - 9000);
                } else {
                     dir = wrap_36000(landingDirection + 9000);
                }
                
                tmpPos = posControl.rthState.homePosition.pos;
                tmpPos.z = landAltAbs;
                fwAutoland.waypoint[LAND_WP_LAND] = tmpPos;
                
                fpVector3_t tmp2;
                calculateFarAwayPos(&tmp2, &posControl.rthState.homePosition.pos, wrap_36000(landingDirection + 18000), fwAutoland.finalApproachLength);
                calculateFarAwayPos(&tmpPos, &tmp2, dir, navConfig()->fw.loiter_radius);
                tmp2.z = finalApproachAlt;
                fwAutoland.waypoint[LAND_WP_FINAL_APPROACH] = tmp2;
                
                calculateFarAwayPos(&tmpPos, &fwAutoland.waypoint[LAND_WP_FINAL_APPROACH], dir, (fwAutoland.finalApproachLength / 2));
                tmpPos.z = finalApproachAlt;
                fwAutoland.waypoint[LAND_WP_TURN] = tmpPos;
                
                setLandWaypoint(&fwAutoland.waypoint[LAND_WP_TURN], &fwAutoland.waypoint[LAND_WP_FINAL_APPROACH]);

                return FW_AUTOLAND_EVENT_SUCCSESS;
            } else {
                return FW_AUTOLAND_EVENT_ABORT;
            }
        } else {
            fwAutoland.loiterStartTime = currentTimeUs;
        }
    }
    
    fpVector3_t tmpPoint = posControl.rthState.homePosition.pos;
    tmpPoint.z = approachAltAbs;
    setDesiredPosition(&tmpPoint, posControl.rthState.homePosition.heading, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING);
    
    return FW_AUTOLAND_EVENT_NONE;
}


static fwAutolandEvent_t fwAutolandState_DOWNWIND(timeUs_t currentTimeUs) 
{
    UNUSED(currentTimeUs);
    
    if (isLandWpReached(&fwAutoland.waypoint[LAND_WP_TURN], &posControl.activeWaypoint.bearing)) {
        setLandWaypoint(&fwAutoland.waypoint[LAND_WP_FINAL_APPROACH], &fwAutoland.waypoint[LAND_WP_LAND]);
        return FW_AUTOLAND_EVENT_SUCCSESS;
    }
    
    checkLandWpAndUpdateZ();
    
    return FW_AUTOLAND_EVENT_NONE;
}

static fwAutolandEvent_t fwAutolandState_BASE(timeUs_t currentTimeUs) 
{
    if (isLandWpReached(&fwAutoland.waypoint[LAND_WP_FINAL_APPROACH], &posControl.activeWaypoint.bearing)) {
        setLandWaypoint(&fwAutoland.waypoint[LAND_WP_LAND], NULL);
        /*
        resetGCSFlags();
        resetPositionController();
        resetAltitudeController(false);
        setupAltitudeController();
        posControl.cruise.course = calculateBearingToDestination(&fwAutoland.waypoint[LAND_WP_LAND]);
        */
        return FW_AUTOLAND_EVENT_SUCCSESS;
    }
    
    checkLandWpAndUpdateZ();

    return FW_AUTOLAND_EVENT_NONE;
}

static fwAutolandEvent_t fwAutolandState_FINAL_APPROACH(timeUs_t currentTimeUs) 
{    
    if (getEstimatedActualPosition(Z) <= safeHomeConfig(safehome_index)->landAltMSL + navFwAutolandConfig()->glideAltitude - GPS_home.alt) {
        setLandWaypoint(&fwAutoland.waypoint[LAND_WP_LAND], NULL);
        return FW_AUTOLAND_EVENT_SUCCSESS;
    }
    posControl.wpDistance = calculateDistanceToDestination(&fwAutoland.waypoint[LAND_WP_LAND]);
    checkLandWpAndUpdateZ();    
    return FW_AUTOLAND_EVENT_NONE;
}

static fwAutolandEvent_t fwAutolandState_GLIDE(timeUs_t currentTimeUs)
{
    if (getEstimatedActualPosition(Z) <= safeHomeConfig(safehome_index)->landAltMSL + navFwAutolandConfig()->flareAltitude - GPS_home.alt) {
        return FW_AUTOLAND_EVENT_SUCCSESS;
    }
    posControl.wpDistance = calculateDistanceToDestination(&fwAutoland.waypoint[LAND_WP_LAND]);
    checkLandWpAndUpdateZ();
    return FW_AUTOLAND_EVENT_NONE;
}

static fwAutolandEvent_t fwAutolandState_FLARE(timeUs_t currentTimeUs)
{
    rcCommand[THROTTLE] = getThrottleIdleValue();
    ENABLE_STATE(NAV_MOTOR_STOP_OR_IDLE);
    rxOverrideRcChannel(PITCH, -navFwAutolandConfig()->flarePitch);
    
    if (isLandingDetected()) {
        rxOverrideRcChannel(PITCH, -1);
        return FW_AUTOLAND_EVENT_SUCCSESS;
    }
    return FW_AUTOLAND_EVENT_NONE;
}

static fwAutolandEvent_t fwAutolandState_ABORT(timeUs_t currentTimeUs) 
{
    UNUSED(currentTimeUs);
    return FW_AUTOLAND_EVENT_NONE;
}

static void setCurrentState(fwAutolandState_t nextState, timeUs_t currentTimeUs)
{
    fwAutoland.currentState = nextState;
    fwAutoland.currentStateTimeUs = currentTimeUs;
}

// Public methods
fwAutolandState_t getFwAutolandState(void)
{
    return fwAutoland.currentState;
}

bool isFwAutolandActive(void) 
{
    return fwAutoland.currentState >= FW_AUTOLAND_STATE_LOITER;
}

bool allowFwAutoland(void) 
{
    return safehome_index >= 0 && (safeHomeConfig(safehome_index)->landApproachHeading1 != 0 || safeHomeConfig(safehome_index)->landApproachHeading2 != 0);
}

void resetFwAutolandController(timeUs_t currentTimeUs)
{
    setCurrentState(FW_AUTOLAND_STATE_ABOVE_LOITER_ALT, currentTimeUs);
    rxOverrideRcChannel(PITCH, -1);
    fwAutoland.loiterStartTime = 0;
}

void applyFixedWingAutolandController(timeUs_t currentTimeUs)
{
    while (autolandStateMachine[fwAutoland.currentState].onEntry) {
        fwAutolandEvent_t newEvent = autolandStateMachine[fwAutoland.currentState].onEntry(currentTimeUs);
        if (newEvent == FW_AUTOLAND_EVENT_NONE) {
            break;
        }
        setCurrentState(autolandStateMachine[fwAutoland.currentState].onEvent[newEvent], currentTimeUs);
    }
}

#endif