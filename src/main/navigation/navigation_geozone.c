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
 * along with INAV. If not, see <http://www.gnu.org/licenses/>.
 */


#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <float.h>
#include <string.h>
#include <math.h>

#include "platform.h"

#include "common/utils.h"
#include "common/vector.h"
#include "common/printf.h"

#include "config/config_reset.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/rc_modes.h"
#include "fc/rc_controls.h"
#include "fc/settings.h"

#include "flight/imu.h"

#include "rx/rx.h"

#include "scheduler/scheduler.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#ifdef USE_GEOZONE

#include "navigation_geozone_calculations.h"

#define MAX_VERTICES (MAX_VERTICES_IN_CONFIG + 1)
#define MAX_GEOZONES (MAX_GEOZONES_IN_CONFIG + 1) // +1 for safe home

#define MAX_DISTANCE_FLY_OVER_POINTS 50000
#define MAX_PATH_PONITS (2 + 2 * MAX_VERTICES)
#define POS_DETECTION_DISTANCE 7500 
#define STICK_LOCK_MIN_TIME 2500
#define AVOID_TIMEOUT 30000
#define MAX_LOCAL_VERTICES 128
#define GEOZONE_INCLUSE_IGNORE_DISTANCE 2000 * 100 // m
#define STICK_MOVE_THRESHOULD 40
#define MAX_RTH_WAYPOINTS (MAX_VERTICES / 2)
#define GEOZONE_INACTIVE INT8_MAX
#define RTH_OVERRIDE_TIMEOUT 1000

#define IS_IN_TOLERANCE_RANGE(a, b, t) (((a) > (b) - (t)) && ((a) < (b) + (t)))

typedef enum {
    GEOZONE_ACTION_STATE_NONE,
    GEOZONE_ACTION_STATE_AVOIDING,
    GEOZONE_ACTION_STATE_AVOIDING_UPWARD,
    GEOZONE_ACTION_STATE_AVOIDING_ALTITUDE,
    GEOZONE_ACTION_STATE_RETURN_TO_FZ,
    GEOZONE_ACTION_STATE_FLYOUT_NFZ,
    GEOZONE_ACTION_STATE_POSHOLD,
    GEOZONE_ACTION_STATE_RTH
} geozoneActionState_e;

typedef struct geoZoneRuntimeConfig_s
{
    geoZoneConfig_t config;
    bool enable;
    bool isInfZone;
    uint32_t radius;
    fpVector2_t *verticesLocal;
} geoZoneRuntimeConfig_t;

typedef struct pathPoint_s pathPoint_t;
struct pathPoint_s {
    fpVector3_t point;
    pathPoint_t* prev;
    float distance;
    bool visited;
};

static bool isInitalised = false;
static geoZoneRuntimeConfig_t *currentZones[MAX_GEOZONES];
static fpVector2_t verticesLocal[MAX_VERTICES];
static uint8_t currentZoneCount = 0;

static bool isAtLeastOneInclusiveZoneActive = false;
static geoZoneRuntimeConfig_t activeGeoZones[MAX_GEOZONES]; 
static uint8_t activeGeoZonesCount = 0;
static geoZoneConfig_t safeHomeGeozoneConfig;
static geozoneActionState_e actionState = GEOZONE_ACTION_STATE_NONE;
static timeMs_t actionStartTime = 0;
static int32_t avoidCourse;
static geoZoneRuntimeConfig_t *nearestHorZone = NULL;
static geoZoneRuntimeConfig_t *nearestInclusiveZone = NULL;
static fpVector3_t avoidingPoint;
static bool geozoneIsEnabled = false;
static fpVector3_t rthWaypoints[MAX_RTH_WAYPOINTS];
static uint8_t rthWaypointIndex = 0;
static int8_t rthWaypointCount = 0;
static bool aboveOrUnderZone = false;
static timeMs_t rthOverrideStartTime;
static bool noZoneRTH = false;
static bool rthHomeSwitchLastState = false;
static bool lockRTZ = false;

geozone_t geozone;

PG_REGISTER_WITH_RESET_TEMPLATE(geozone_config_t, geoZoneConfig, PG_GEOZONE_CONFIG, 0);

PG_RESET_TEMPLATE(geozone_config_t, geoZoneConfig,
    .fenceDetectionDistance = SETTING_GEOZONE_DETECTION_DISTANCE_DEFAULT,
    .avoidAltitudeRange = SETTING_GEOZONE_AVOID_ALTITUDE_RANGE_DEFAULT,
    .safeAltitudeDistance = SETTING_GEOZONE_SAFE_ALTITUDE_DISTANCE_DEFAULT,
    .nearestSafeHomeAsInclusivZone = SETTING_GEOZONE_SAFEHOME_AS_INCLUSIVE_DEFAULT,
    .copterFenceStopDistance = SETTING_GEOZONE_MR_STOP_DISTANCE_DEFAULT,
    .noWayHomeAction = SETTING_GEOZONE_NO_WAY_HOME_ACTION_DEFAULT
);

PG_REGISTER_ARRAY_WITH_RESET_FN(geoZoneConfig_t, MAX_GEOZONES_IN_CONFIG, geoZonesConfig, PG_GEOZONES, 1);

void pgResetFn_geoZonesConfig(geoZoneConfig_t *instance)
{           
    for (int i = 0; i < MAX_GEOZONES_IN_CONFIG; i++) {
        RESET_CONFIG(geoZoneConfig_t, &instance[i], 
            .shape = GEOZONE_TYPE_EXCLUSIVE,
            .type = GEOZONE_SHAPE_CIRCULAR,
            .minAltitude = 0,
            .maxAltitude = 0,
            .isSealevelRef = false,
            .fenceAction = GEOFENCE_ACTION_NONE,
            .vertexCount = 0
        );
    }
}

PG_REGISTER_ARRAY_WITH_RESET_FN(vertexConfig_t, MAX_VERTICES_IN_CONFIG, geoZoneVertices, PG_GEOZONE_VERTICES, 0); 

void pgResetFn_geoZoneVertices(vertexConfig_t *instance)
{
     for (int i = 0; i < MAX_VERTICES_IN_CONFIG; i++) {
        RESET_CONFIG(vertexConfig_t, &instance[i], 
            .zoneId = -1,
            .idx = 0,
            .lat = 0,
            .lon = 0
        );
     }
}

uint8_t geozoneGetUsedVerticesCount(void)
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < MAX_GEOZONES_IN_CONFIG; i++) {
        count += geoZonesConfig(i)->vertexCount;
    }
    return count;
}

void geozoneReset(const int8_t idx) 
{   
    if (idx < 0) {
        for (uint8_t i = 0; i < MAX_GEOZONES_IN_CONFIG; i++) {
            geoZonesConfigMutable(i)->shape = GEOZONE_SHAPE_CIRCULAR;
            geoZonesConfigMutable(i)->type = GEOZONE_TYPE_EXCLUSIVE;
            geoZonesConfigMutable(i)->maxAltitude = 0;
            geoZonesConfigMutable(i)->minAltitude = 0;
            geoZonesConfigMutable(i)->isSealevelRef = false;
            geoZonesConfigMutable(i)->fenceAction = GEOFENCE_ACTION_NONE;
            geoZonesConfigMutable(i)->vertexCount = 0;
        }
    } else if (idx < MAX_GEOZONES_IN_CONFIG) {
        geoZonesConfigMutable(idx)->shape = GEOZONE_SHAPE_CIRCULAR;
        geoZonesConfigMutable(idx)->type = GEOZONE_TYPE_EXCLUSIVE;
        geoZonesConfigMutable(idx)->maxAltitude = 0;
        geoZonesConfigMutable(idx)->minAltitude = 0;
        geoZonesConfigMutable(idx)->isSealevelRef = false;
        geoZonesConfigMutable(idx)->fenceAction = GEOFENCE_ACTION_NONE;
        geoZonesConfigMutable(idx)->vertexCount = 0;
    }
}

void geozoneResetVertices(const int8_t zoneId, const int16_t idx)
{
    if (zoneId < 0 && idx < 0) {
         for (uint8_t i = 0; i < MAX_VERTICES_IN_CONFIG; i++) {        
                geoZoneVerticesMutable(i)->lat = 0;
                geoZoneVerticesMutable(i)->lon = 0;
                geoZoneVerticesMutable(i)->idx = 0;
                geoZoneVerticesMutable(i)->zoneId = -1;        
        }
        for (uint8_t i = 0; i < MAX_GEOZONES_IN_CONFIG; i++) {
            geoZonesConfigMutable(i)->vertexCount = 0;
        } 
    } else if (zoneId >= 0 && zoneId < MAX_GEOZONES_IN_CONFIG && idx < 0) {     
        bool found = false;
        for (uint8_t i = 0; i < MAX_VERTICES_IN_CONFIG; i++) {        
            if (geoZoneVertices(i)->zoneId == zoneId) {
                geoZoneVerticesMutable(i)->lat = 0;
                geoZoneVerticesMutable(i)->lon = 0;
                geoZoneVerticesMutable(i)->idx = 0;
                geoZoneVerticesMutable(i)->zoneId = -1;
                found = true;
            }
        }
        if (found) {
            geoZonesConfigMutable(zoneId)->vertexCount = 0;
        }
    } else if (zoneId >= 0 && zoneId < MAX_GEOZONES_IN_CONFIG && idx >= 0 && idx < MAX_VERTICES_IN_CONFIG) {
        bool found = false;
        for (uint8_t i = 0; i < MAX_VERTICES_IN_CONFIG; i++) {
            if (geoZoneVertices(i)->zoneId == zoneId && geoZoneVertices(i)->idx == idx) {
                geoZoneVerticesMutable(i)->lat = 0;
                geoZoneVerticesMutable(i)->lon = 0;
                geoZoneVerticesMutable(i)->idx = 0;
                geoZoneVerticesMutable(i)->zoneId = -1;
                found = true;               
                break;
            }
        }
        if (found) {
            geoZonesConfigMutable(zoneId)->vertexCount--; 
        }
    }
}

bool geozoneSetVertex(uint8_t zoneId, uint8_t vertexId, int32_t lat, int32_t lon) 
{
    if (vertexId < MAX_VERTICES_IN_CONFIG)
    {
        int16_t vertexConfigIdx = -1;
        for (uint8_t i = 0; i < MAX_VERTICES_IN_CONFIG; i++) {
            if (geoZoneVertices(i)->zoneId == -1) {
                vertexConfigIdx = i;
                break;
            }
        }
        if (vertexConfigIdx >= 0) {                  
            geoZoneVerticesMutable(vertexConfigIdx)->zoneId = zoneId;
            geoZoneVerticesMutable(vertexConfigIdx)->idx = vertexId;
            geoZoneVerticesMutable(vertexConfigIdx)->lat = lat;
            geoZoneVerticesMutable(vertexConfigIdx)->lon = lon;
            return true;
        }
    } 
    return false;
}

int8_t geozoneGetVertexIdx(uint8_t zoneId, uint8_t vertexId) 
{
    for (uint8_t i = 0; i < MAX_VERTICES_IN_CONFIG; i++) {
        if (geoZoneVertices(i)->zoneId == zoneId && geoZoneVertices(i)->idx == vertexId) {
            return i;
        }
    }
    return -1;
}

static bool areSticksDeflectdFromChannelValue(void) 
{
    return ABS(rxGetChannelValue(ROLL) - PWM_RANGE_MIDDLE) + ABS(rxGetChannelValue(PITCH) - PWM_RANGE_MIDDLE) + ABS(rxGetChannelValue(YAW) - PWM_RANGE_MIDDLE) >= STICK_MOVE_THRESHOULD;
}

static bool isNonGeozoneModeFromBoxInput(void)
{
    return  !(IS_RC_MODE_ACTIVE(BOXNAVCRUISE) || IS_RC_MODE_ACTIVE(BOXNAVCOURSEHOLD) || IS_RC_MODE_ACTIVE(BOXHORIZON) || IS_RC_MODE_ACTIVE(BOXANGLE));
}

static bool isPointOnBorder(geoZoneRuntimeConfig_t *zone, const fpVector3_t *pos)
{
    fpVector2_t *prev = &zone->verticesLocal[zone->config.vertexCount -1];
    fpVector2_t *current;
    bool isOnBorder = false;
    for (uint8_t i = 0; i < zone->config.vertexCount; i++) {
        current = &zone->verticesLocal[i];
        if (isPointOnLine2(prev, current, (fpVector2_t*)pos)) {
            isOnBorder = true;
            break;
        }
        prev = current;
    }

    if (isOnBorder) {
        return (pos->z >= zone->config.minAltitude || zone->config.minAltitude == 0) && pos->z <= zone->config.maxAltitude;
    }

    return isOnBorder;
}

static bool isInZoneAltitudeRange(geoZoneRuntimeConfig_t *zone, const float pos)
{
    return (pos >= zone->config.minAltitude || zone->config.minAltitude == 0) && pos <= zone->config.maxAltitude;
}

static bool isInGeozone(geoZoneRuntimeConfig_t *zone, const fpVector3_t *pos, bool ignoreAltitude)
{
    if (activeGeoZonesCount == 0)  {
        return false;
    }

    bool isIn2D = false;
    if (zone->config.shape == GEOZONE_SHAPE_POLYGON) {
        isIn2D = isPointInPloygon((fpVector2_t*)pos, zone->verticesLocal, zone->config.vertexCount) || isPointOnBorder(zone, pos);
    } else { // cylindric
        isIn2D = isPointInCircle((fpVector2_t*)pos, &zone->verticesLocal[0], zone->radius);
    }

    if (isIn2D && !ignoreAltitude) {
        return isInZoneAltitudeRange(zone, pos->z);
    }

    return isIn2D;
}

static bool isPointInAnyOtherZone(const geoZoneRuntimeConfig_t *zone, uint8_t type, const fpVector3_t *pos)
{
    bool isInZone = false;        
    for (uint8_t i = 0; i < activeGeoZonesCount; i++) {
        if (zone != &activeGeoZones[i] && activeGeoZones[i].config.type == type && isInGeozone(&activeGeoZones[i], pos, false)) {
            isInZone = true;
            break;
        }
    }
    return isInZone;
}

static uint8_t getZonesForPos(geoZoneRuntimeConfig_t *zones[], const fpVector3_t *pos, const bool ignoreAltitude) 
{
    uint8_t count = 0;
    for (int i = 0; i < activeGeoZonesCount; i++) {
        if (isInGeozone(&activeGeoZones[i], pos, ignoreAltitude)) {
            zones[count++] = &activeGeoZones[i];
        }
    }
    return count;
}

static uint8_t getCurrentZones(geoZoneRuntimeConfig_t *zones[], const bool ignoreAltitude) 
{
    return getZonesForPos(zones, &navGetCurrentActualPositionAndVelocity()->pos, ignoreAltitude);
}

static int geoZoneRTComp(const void *a, const void *b)
{
    geoZoneRuntimeConfig_t *zoneA = (geoZoneRuntimeConfig_t*)a;
    geoZoneRuntimeConfig_t *zoneB = (geoZoneRuntimeConfig_t*)b;

    if (zoneA->enable == zoneB->enable) {
        return 0;
    } else if (zoneA->enable) {
        return -1;
    } else {
        return 1;
    }
}

// in cm and cms/s
static uint32_t calcTime(const int32_t distance, const int32_t speed)
{
    if (speed <= 0) {
        return 0;
    }

    return distance / speed;
}

static void calcPreviewPoint(fpVector3_t *target, const int32_t distance)
{
    calculateFarAwayTarget(target, DECIDEGREES_TO_CENTIDEGREES(gpsSol.groundCourse), distance);
    target->z = getEstimatedActualPosition(Z) + calcTime(geoZoneConfig()->fenceDetectionDistance, gpsSol.groundSpeed) * getEstimatedActualVelocity(Z);
}

static bool calcIntersectionForZone(fpVector3_t *intersection, float *distance, geoZoneRuntimeConfig_t *zone, const fpVector3_t *start, const fpVector3_t *end)
{
    bool hasIntersection = false;
    if (zone->config.shape == GEOZONE_SHAPE_POLYGON) {
        if (calcLine3dPolygonIntersection(
            intersection,
            distance,
            start,
            end,
            zone->verticesLocal,
            zone->config.vertexCount,
            zone->config.minAltitude,
            zone->config.maxAltitude,
            zone->config.type == GEOZONE_TYPE_INCLUSIVE)) {
                hasIntersection = true;
        }
    } else if (zone->config.shape == GEOZONE_SHAPE_CIRCULAR) {
        fpVector3_t circleCenter = { .x = zone->verticesLocal[0].x, .y = zone->verticesLocal[0].y, .z = zone->config.minAltitude };
        if (calcLineCylinderIntersection(
            intersection,
            distance,
            start,
            end,
            &circleCenter, 
            zone->radius, 
            zone->config.maxAltitude - zone->config.minAltitude,
            zone->config.type == GEOZONE_TYPE_INCLUSIVE)) {
                hasIntersection = true;
        }
    }

    if (hasIntersection && isPointOnLine3(start, end, intersection)){
        return true;
    }
    *distance = -1;
    return false;
}

static int32_t calcBouceCoursePolygon(const int32_t course, const fpVector2_t* pos, const fpVector2_t *intersect, const fpVector2_t* p1, const fpVector2_t* p2)
{
	int32_t newCourse = 0;
	int32_t angelp1 = DEGREES_TO_CENTIDEGREES(calcAngelFrom3Points(p1, pos, intersect));
	int32_t angelp2 = DEGREES_TO_CENTIDEGREES(calcAngelFrom3Points(p2, pos, intersect));
	if (angelp1 < angelp2) {
		if (isPointRightFromLine(pos, intersect, p1)) {
			newCourse = course - 2 * angelp1;
		}
		else {
			newCourse = course + 2 * angelp1;
		}
	}
	else {
		if (isPointRightFromLine(pos, intersect, p2)) {
			newCourse = course - 2 * angelp2;
		}
		else {
			newCourse = course + 2 * angelp2;
		}
	}
	return wrap_36000(newCourse);
}

static int32_t calcBouceCourseCircle(const int32_t course, const fpVector2_t* pos, const fpVector2_t* intersect, const fpVector2_t* mid)
{
	int32_t newCourse = 0;
	int32_t angel = DEGREES_TO_CENTIDEGREES(calcAngelFrom3Points(mid, pos, intersect));
	if (isPointRightFromLine(pos, mid, intersect)) {
		newCourse = course + 2 * (angel - 9000);
	}
	else {
		newCourse = course - 2 * (angel - 9000);
	}
	return wrap_36000(newCourse);
}

static bool findNearestIntersectionZone(geoZoneRuntimeConfig_t **intersectZone, fpVector3_t *intersection, float *distance, const float detectionDistance, const fpVector3_t *start, const fpVector3_t *end)
{
    geoZoneRuntimeConfig_t *zone = NULL;
    fpVector3_t intersect;
    float distanceToZone = FLT_MAX;  

    for (uint8_t i = 0; i < activeGeoZonesCount; i++) {
        fpVector3_t currentIntersect;
        float currentDistance = FLT_MAX;
            if (!calcIntersectionForZone(
                &currentIntersect,
                &currentDistance,
                &activeGeoZones[i],
                start, 
                end)) {
                    continue;
            }
        
        if (currentDistance < distanceToZone) {
            distanceToZone = currentDistance;
            zone = &activeGeoZones[i];
            intersect = currentIntersect;
        }
    }

    if (zone && distanceToZone < detectionDistance) {
        *intersectZone = zone;
        *distance = distanceToZone;
        memcpy(intersection, &intersect, sizeof(fpVector3_t));
        return true;
    }

    return false;
}

static bool isPointDirectReachable(const fpVector3_t* start, const fpVector3_t *point)
{
    float currentDistance = 0;
    bool pointIsInOverlappingZone = false, pointIsInExclusiveZone = false, hasIntersection = false;

    /*
    if (start->x == point->x && start->y == point->y) {
        return false;
    }
    */

    for (uint8_t i = 0; i < activeGeoZonesCount; i++) {
        fpVector3_t currentIntersect;
        
        if (!calcIntersectionForZone(&currentIntersect, &currentDistance, &activeGeoZones[i], start, point)) {
            continue;
        }
        hasIntersection = true;

        // Intersct a exclusive Zone?
        geoZoneRuntimeConfig_t *intersectZones[MAX_GEOZONES];
        uint8_t intersectZonesCount = getZonesForPos(intersectZones, &currentIntersect, false);
        for (uint8_t j = 0; j < intersectZonesCount; j++) {
            if (intersectZones[j]->config.type == GEOZONE_TYPE_EXCLUSIVE ) {
                pointIsInExclusiveZone = true;
                break;
            }    
        }

        if (pointIsInExclusiveZone) {
            break;
        }
        
        // We targeting a exit point (in min two zones)
        if (intersectZonesCount < 2) {
            break;
        }

        geoZoneRuntimeConfig_t *startZones[MAX_GEOZONES];
        uint8_t startZonesCount = getZonesForPos(startZones, start, false);
        geoZoneRuntimeConfig_t *endZones[MAX_GEOZONES];
        uint8_t endZonesCount = getZonesForPos(endZones, point, false);

        for (uint8_t j = 0; j < intersectZonesCount; j++) {
            for (uint8_t k = 0; k < startZonesCount; k++) {
                for (uint8_t l = 0; l < endZonesCount; l++) {
                    if (intersectZones[j] == startZones[k] && intersectZones[j] == endZones[l]) {
                        pointIsInOverlappingZone = true;
                        break;
                    }
                }
                if (pointIsInOverlappingZone) {
                    break;
                }
            }
            if (pointIsInOverlappingZone) {
                break;
            }
        }
    }
    
   return !pointIsInExclusiveZone && (!hasIntersection || pointIsInOverlappingZone);
} 

uint32_t geozoneGetDetectionDistance(void)
{
    uint32_t detctionDistance = 0;
    if (STATE(AIRPLANE)) {
        detctionDistance = navConfig()->fw.loiter_radius * 1.5;
    } else {
        detctionDistance = geoZoneConfig()->copterFenceStopDistance;
    }
    return detctionDistance;
}

static int32_t calcBounceCourseForZone(geoZoneRuntimeConfig_t *zone, fpVector3_t *prevPoint, fpVector3_t *intersection)
{
    int32_t course = 0;
    if (zone->config.shape == GEOZONE_SHAPE_POLYGON)     {
        fpVector2_t intersect;
        bool found = false;
        fpVector2_t *p1 = &zone->verticesLocal[zone->config.vertexCount - 1];
        fpVector2_t *p2;
        for (uint8_t i = 0; i < zone->config.vertexCount; i++) {
            p2 = &zone->verticesLocal[i];
            if (calcLineLineIntersection(&intersect, p1, p2, (fpVector2_t*)&navGetCurrentActualPositionAndVelocity()->pos, (fpVector2_t*)prevPoint, true)) {
                found = true;
                break;
            }
            p1 = p2;
        }

        if (!found) {
            return -1;
        } 
        course = calcBouceCoursePolygon(DECIDEGREES_TO_CENTIDEGREES(gpsSol.groundCourse), (fpVector2_t*)&navGetCurrentActualPositionAndVelocity()->pos, &intersect, p1, p2);
    } else if (zone->config.shape == GEOZONE_SHAPE_CIRCULAR) {
        course = calcBouceCourseCircle(DECIDEGREES_TO_CENTIDEGREES(gpsSol.groundCourse), (fpVector2_t*)&navGetCurrentActualPositionAndVelocity()->pos, (fpVector2_t*)intersection, &zone->verticesLocal[0]);
    }
    return course;
}

static bool initPathPoint(pathPoint_t *pathPoints, const fpVector3_t pos, uint8_t *idx)
{
    if (*idx + 1 >= MAX_PATH_PONITS) {
        return false;
    }

    pathPoints[*idx].distance = FLT_MAX;
    pathPoints[*idx].visited = false;
    pathPoints[(*idx)++].point = pos;

    return true;
}

static bool isPosInGreenAlt(geoZoneRuntimeConfig_t *zones[], const uint8_t zoneCount, const float alt) 
{
    bool isInNfz = false, isInFz = false;
    for (uint8_t j = 0; j < zoneCount; j++) { 
        if(isInZoneAltitudeRange(zones[j], alt)){
            isInFz = zones[j]->config.type == GEOZONE_TYPE_INCLUSIVE;
            isInNfz = zones[j]->config.type == GEOZONE_TYPE_EXCLUSIVE;     
        } 
    }
    return !isInNfz && (!isAtLeastOneInclusiveZoneActive || isInFz);
}

static bool checkPathPointOrSetAlt(fpVector3_t *pos) 
{
    geoZoneRuntimeConfig_t *zones[MAX_GEOZONES];
    uint8_t zoneCount = getZonesForPos(zones, pos, true);

    if (zoneCount == 0) { 
        return !isAtLeastOneInclusiveZoneActive;
    }

    if (zoneCount == 1 && zones[0]->config.type == GEOZONE_TYPE_INCLUSIVE) {
        return true;
    }

    bool isInExclusice = false;
    for (uint8_t i = 0; i < zoneCount; i++) {
        if (zones[i]->config.type == GEOZONE_TYPE_EXCLUSIVE && isInGeozone(zones[i], pos, false)) {   
            isInExclusice = true;
            if (zones[i]->config.minAltitude != 0) {
                float min = zones[i]->config.minAltitude - 2 * geoZoneConfig()->safeAltitudeDistance;     
                if (isPosInGreenAlt(zones, zoneCount, min)) {
                    pos->z = min;
                    return true;
                }
            }

            if (!zones[i]->isInfZone || zones[i]->config.maxAltitude < INT32_MAX) {
                float max = zones[i]->config.maxAltitude + 2 * geoZoneConfig()->safeAltitudeDistance;
                if(isPosInGreenAlt(zones, zoneCount, max)) {
                    pos->z = max;
                    return true;
                }
            }
        } 
    }

    return !isInExclusice;
}

// Return value: 0 - Target direct reachable; -1 No way; >= 1 Waypoints to target 
#define CIRCLE_POLY_SIDES 6
static int8_t calcRthCourse(fpVector3_t* waypoints, const fpVector3_t* point, fpVector3_t* target)
{
    pathPoint_t pathPoints[MAX_PATH_PONITS];
    uint8_t pathPointCount = 0, waypointCount = 0;
    fpVector3_t start = *point;
    
    if (isPointDirectReachable(&start, target)) {
        return 0;
    } 
    
    // Set starting point slightly away from our current position 
    float offset = geozoneGetDetectionDistance();
    if (geozone.distanceVertToNearestZone <= offset) {
        int bearing = wrap_36000(geozone.directionToNearestZone + 18000);
        start.x += offset * cos_approx(CENTIDEGREES_TO_RADIANS(bearing));
        start.y += offset * sin_approx(CENTIDEGREES_TO_RADIANS(bearing));
    }

    pathPoints[pathPointCount].visited = true;
    pathPoints[pathPointCount].distance = 0;
    pathPoints[pathPointCount++].point = start;

    // Calculate possible waypoints
    // Vertices of the zones are possible waypoints, 
    // inclusive zones are “reduced”, exclusive zones are “enlarged” to keep distance,
    // round zones are converted into hexagons and long sides get additional points to be able to fly over zones.
    for (uint8_t i = 0 ; i < activeGeoZonesCount; i++) {        
        fpVector2_t *verticesZone;
        fpVector2_t verticesCirclePoly[CIRCLE_POLY_SIDES];
        uint8_t verticesZoneCount;
        if (activeGeoZones[i].config.shape == GEOZONE_SHAPE_POLYGON) {
            verticesZone = activeGeoZones[i].verticesLocal;
            verticesZoneCount = activeGeoZones[i].config.vertexCount;
        } else {
            generatePolygonFromCircle(verticesCirclePoly, &activeGeoZones[i].verticesLocal[0], activeGeoZones[i].radius, CIRCLE_POLY_SIDES);
            verticesZone = verticesCirclePoly;
            verticesZoneCount = CIRCLE_POLY_SIDES;
        }        
        
        fpVector2_t safeZone[MAX_VERTICES];
        offset = geozoneGetDetectionDistance() * 2 / 3;
        if (activeGeoZones[i].config.type == GEOZONE_TYPE_INCLUSIVE) {
            offset *= -1;
        }
        
        float zMin = start.z, zMax = 0;
        if (!isInZoneAltitudeRange(&activeGeoZones[i], start.z) && activeGeoZones[i].config.minAltitude > 0) {
            zMin = activeGeoZones[i].config.minAltitude + 2 * geoZoneConfig()->safeAltitudeDistance;
        }

        if (activeGeoZones[i].config.type == GEOZONE_TYPE_INCLUSIVE && (!activeGeoZones[i].isInfZone || activeGeoZones[i].config.maxAltitude < INT32_MAX)) {
            zMax = activeGeoZones[i].config.maxAltitude - 2 * geoZoneConfig()->safeAltitudeDistance;
        } else if (activeGeoZones[i].config.type == GEOZONE_TYPE_EXCLUSIVE && (!activeGeoZones[i].isInfZone || activeGeoZones[i].config.maxAltitude < INT32_MAX)) {
            zMax = activeGeoZones[i].config.maxAltitude + 2 * geoZoneConfig()->safeAltitudeDistance;
        }
                
        generateOffsetPolygon(safeZone, verticesZone, verticesZoneCount, offset);
        fpVector2_t *prev = &safeZone[verticesZoneCount - 1];
        fpVector2_t *current;
        for (uint8_t j = 0; j < verticesZoneCount; j++) {
            current = &safeZone[j];
            
            if (zMax > 0 ) {
                fpVector3_t max = { .x = current->x, .y = current->y, .z = zMax };
                if (checkPathPointOrSetAlt(&max)) {
                    if (!initPathPoint(pathPoints, max, &pathPointCount)) {
                        return -1;
                    }
                }

                if (activeGeoZones[i].config.type == GEOZONE_TYPE_EXCLUSIVE) {
                    // Set some "fly over points"
                    float dist = calculateDistance2(prev, current);
                    if (dist > MAX_DISTANCE_FLY_OVER_POINTS) {
                        uint8_t sectionCount = (uint8_t)(dist / MAX_DISTANCE_FLY_OVER_POINTS);
                        float dist = MAX_DISTANCE_FLY_OVER_POINTS;
                        for (uint8_t k = 0; k < sectionCount; k++) {
                            fpVector3_t flyOverPoint;
                            calcPointOnLine((fpVector2_t*)&flyOverPoint, prev, current, dist);
                            fpVector3_t maxFo = { .x = flyOverPoint.x, .y = flyOverPoint.y, .z = zMax };
                            if (checkPathPointOrSetAlt(&maxFo)) {
                                if (!initPathPoint(pathPoints, maxFo, &pathPointCount)) {
                                    return -1;
                                }
                            }
                            dist += MAX_DISTANCE_FLY_OVER_POINTS;
                        }
                    }
                }            
            }

            if (zMin > 0) {
                fpVector3_t min = { .x = current->x, .y = current->y, .z = zMin };
                if (checkPathPointOrSetAlt(&min)) {
                    if (!initPathPoint(pathPoints, min, &pathPointCount)) {
                        return -1;
                    }
                } 
                
            }
            prev = current;
        }
    }

    if (!initPathPoint(pathPoints, *target, &pathPointCount)) {
        return -1;
    }

    // Dijkstra
    pathPoint_t *current = pathPoints;
    while (!pathPoints[pathPointCount - 1].visited) {
        pathPoint_t *next = current;
        float min = FLT_MAX;
        for (uint8_t i = 1; i < pathPointCount; i++) {
            
            float currentDist = FLT_MAX;
            if (isPointDirectReachable(&current->point, &pathPoints[i].point)) {
                float dist2D = calculateDistance2((fpVector2_t*)&current->point, (fpVector2_t*)&pathPoints[i].point);
                float distAlt = ABS(current->point.z - pathPoints[i].point.z);
                currentDist = current->distance + dist2D + 2 * distAlt;
            }

            if (currentDist < pathPoints[i].distance && !pathPoints[i].visited) {
                pathPoints[i].distance = currentDist;
                pathPoints[i].prev = current;
            }
            if (min > pathPoints[i].distance && !pathPoints[i].visited) {
                min = pathPoints[i].distance;
                next = &pathPoints[i];
            }
        }

        if (min == FLT_MAX) {
            return -1;
        }
        
        current = next;
        current->visited = true;
    }
    
    waypointCount = 0;
    current = &pathPoints[pathPointCount - 1];
    while (current != pathPoints) {
        waypointCount++;
        current = current->prev;
    }
    // Don't set home to the WP list
    current = pathPoints[pathPointCount - 1].prev;
    uint8_t i = waypointCount - 2;
    while (current != pathPoints) {
        waypoints[i] = current->point;
        current = current->prev;
        i--;
    }
    return waypointCount - 1;    
}

static void updateCurrentZones(void)
{
    currentZoneCount = getCurrentZones(currentZones, false);
    geozone.insideNfz = false;
    geozone.insideFz = false;
    for (uint8_t i = 0; i < currentZoneCount; i++) {
        if (currentZones[i]->config.type == GEOZONE_TYPE_EXCLUSIVE) {   
            geozone.insideNfz = true;
        }
        if (currentZones[i]->config.type == GEOZONE_TYPE_INCLUSIVE) {
            geozone.insideFz = true;
        }
    }
}

static void updateZoneInfos(void)
{
    float nearestDistanceToBorder = FLT_MAX;
    fpVector3_t nearestBorderPoint;
    aboveOrUnderZone = false;
   
    nearestHorZone = NULL;
    geoZoneRuntimeConfig_t *currentZones[MAX_GEOZONES];
    uint8_t currentZoneCount = getCurrentZones(currentZones, true);
    int32_t currentMaxAltitude = INT32_MIN, currentMinAltitude = INT32_MAX;

    if (currentZoneCount == 1) {
        currentMaxAltitude = currentZones[0]->config.maxAltitude;
        currentMinAltitude = currentZones[0]->config.minAltitude;
        nearestHorZone = currentZones[0];
    
    } else if (currentZoneCount >= 2) {
        
        geoZoneRuntimeConfig_t *aboveZone = NULL, *belowZone = NULL;
        float distAbove = FLT_MAX, distBelow = FLT_MAX;
        geoZoneRuntimeConfig_t *current = NULL;
        for (uint8_t i = 0; i < currentZoneCount; i++) {
            current = currentZones[i];
            
            if (isInZoneAltitudeRange(current, getEstimatedActualPosition(Z))) {
                currentMaxAltitude = MAX(current->config.maxAltitude, currentMaxAltitude);
                currentMinAltitude = MIN(current->config.minAltitude, currentMinAltitude);
                nearestHorZone = current;
            }

            if (current->config.minAltitude > getEstimatedActualPosition(Z)) {
                float dist = current->config.maxAltitude - getEstimatedActualPosition(Z);
                if (dist < distAbove) {
                    aboveZone = current;
                    distAbove = dist;
                }
            }
            
            if (current->config.maxAltitude < getEstimatedActualPosition(Z)) {
                float dist = getEstimatedActualPosition(Z) - current->config.maxAltitude;
                if (dist < distBelow) {
                    belowZone = current;
                    distBelow = dist;
                }
            }
        }

        if (aboveZone) {
            if (aboveZone->config.type == GEOZONE_TYPE_INCLUSIVE) {
                currentMaxAltitude = MAX(aboveZone->config.maxAltitude, currentMaxAltitude);
                nearestHorZone = aboveZone;
            } else {
                currentMaxAltitude = MIN(aboveZone->config.minAltitude, currentMaxAltitude);
            }
        } 

        if (belowZone) {
            if (belowZone->config.type == GEOZONE_TYPE_INCLUSIVE) {
                currentMinAltitude = MIN(belowZone->config.minAltitude, currentMinAltitude);
                nearestHorZone = belowZone;
            } else {
                currentMinAltitude = MAX(belowZone->config.maxAltitude, currentMinAltitude);
            }
        }  
    }

    if (currentMinAltitude == INT32_MAX) {
        currentMinAltitude = 0;
    }

    if (currentMaxAltitude == INT32_MIN) {
        currentMaxAltitude = 0;
    }

    if (currentMaxAltitude == INT32_MAX && currentMinAltitude != 0) {
        geozone.distanceVertToNearestZone = ABS(currentMinAltitude - getEstimatedActualPosition(Z));
    } else if (currentMinAltitude == 0 && currentMaxAltitude != 0) {
        geozone.distanceVertToNearestZone  = currentMaxAltitude - getEstimatedActualPosition(Z);
    } else if (currentMinAltitude != 0 && currentMaxAltitude > 0) {  
        int32_t distToMin = currentMinAltitude - getEstimatedActualPosition(Z);
        int32_t distToMax = currentMaxAltitude - getEstimatedActualPosition(Z);
        if (getEstimatedActualPosition(Z) > currentMinAltitude && getEstimatedActualPosition(Z) < currentMaxAltitude) {
            if (ABS(distToMin) < ABS(currentMaxAltitude - currentMinAltitude) / 2 ) {
                geozone.distanceVertToNearestZone  = distToMin;
            } else {
                geozone.distanceVertToNearestZone  = distToMax;
            }
        } else {
            geozone.distanceVertToNearestZone = MIN(ABS(distToMin), distToMax);
        }
    } else {
        geozone.distanceVertToNearestZone  = 0;
    }

    if (currentZoneCount == 0) {
        geozone.currentzoneMaxAltitude = 0;
        geozone.currentzoneMinAltitude = 0;
    } else {
        
        if (getEstimatedActualPosition(Z) < currentMinAltitude || getEstimatedActualPosition(Z) > currentMaxAltitude) {
            aboveOrUnderZone = true;
        }

        if (currentMaxAltitude > 0) {
            geozone.currentzoneMaxAltitude = currentMaxAltitude - geoZoneConfig()->safeAltitudeDistance;
        } else {
            geozone.currentzoneMaxAltitude = 0;
        }

        if (currentMinAltitude > 0) {
            geozone.currentzoneMinAltitude = currentMinAltitude + geoZoneConfig()->safeAltitudeDistance;
        } else {
            geozone.currentzoneMinAltitude = 0;
        }
    }

    for (uint8_t i = 0; i < activeGeoZonesCount; i++)
    {
        // Ignore NFZ for now, we want back fo the FZ, we will check for NFZ later at RTH
        if (currentZoneCount == 0 && isAtLeastOneInclusiveZoneActive && activeGeoZones[i].config.type == GEOZONE_TYPE_EXCLUSIVE) {
            continue;
        }
        
        if (activeGeoZones[i].config.shape == GEOZONE_SHAPE_POLYGON) {
            fpVector2_t* prev = &activeGeoZones[i].verticesLocal[activeGeoZones[i].config.vertexCount - 1];
            fpVector2_t* current = NULL;
            for (uint8_t j = 0; j < activeGeoZones[i].config.vertexCount; j++) {         
                current = &activeGeoZones[i].verticesLocal[j];
                fpVector2_t pos = calcNearestPointOnLine(prev, current, (fpVector2_t*)&navGetCurrentActualPositionAndVelocity()->pos);
                float dist = calculateDistance2((fpVector2_t*)&navGetCurrentActualPositionAndVelocity()->pos, &pos);
                if (dist < nearestDistanceToBorder) {
                    nearestDistanceToBorder = dist;
                    nearestBorderPoint.x = pos.x;
                    nearestBorderPoint.y = pos.y;
                    nearestBorderPoint.z = getEstimatedActualPosition(Z);
                    geozone.directionToNearestZone = calculateBearingToDestination(&nearestBorderPoint);
                    geozone.distanceHorToNearestZone = roundf(nearestDistanceToBorder);
                    nearestInclusiveZone = &activeGeoZones[i];
                }
                prev = current;
            }
        } else if (activeGeoZones[i].config.shape == GEOZONE_SHAPE_CIRCULAR) {
            float dist = fabsf(calculateDistance2((fpVector2_t*)&navGetCurrentActualPositionAndVelocity()->pos, &activeGeoZones[i].verticesLocal[0]) - activeGeoZones[i].radius);
            if (dist < nearestDistanceToBorder) {
                nearestDistanceToBorder = dist;
                int32_t directionToBorder = calculateBearingToDestination((fpVector3_t*)&activeGeoZones[i].verticesLocal[0]);

                if (isPointInCircle((fpVector2_t*)&navGetCurrentActualPositionAndVelocity()->pos, &activeGeoZones[i].verticesLocal[0], activeGeoZones[i].radius)) {
                    directionToBorder = wrap_36000(directionToBorder + 18000);
                }
                geozone.directionToNearestZone = directionToBorder;
                geozone.distanceHorToNearestZone = roundf(dist);
                nearestInclusiveZone = &activeGeoZones[i];
            }
        }

        if (aboveOrUnderZone && nearestHorZone != NULL && ABS(geozone.distanceVertToNearestZone) < geozone.distanceHorToNearestZone) {
            nearestInclusiveZone = nearestHorZone;
            geozone.distanceHorToNearestZone = 0;
        }
    } 

    geozone.nearestHorZoneHasAction = nearestHorZone && nearestHorZone->config.fenceAction != GEOFENCE_ACTION_NONE;
}

void performeFenceAction(geoZoneRuntimeConfig_t *zone, fpVector3_t *intersection)
{
    // Actions only for assisted modes now
    if (isNonGeozoneModeFromBoxInput() || geozone.avoidInRTHInProgress || (calculateDistanceToDestination(intersection) > geozoneGetDetectionDistance())) {
        return;
    }
    
    int32_t alt = roundf(intersection->z);
    if (alt == zone->config.maxAltitude || alt == zone->config.minAltitude) {
        return;
    }

    fpVector3_t prevPoint;
    calcPreviewPoint(&prevPoint, geoZoneConfig()->fenceDetectionDistance);
    
    if (zone->config.fenceAction == GEOFENCE_ACTION_AVOID) {     
        bool avoidFzStep = false;
        float fzStepAlt = 0;
        if (zone->config.type == GEOZONE_TYPE_INCLUSIVE) {
            geoZoneRuntimeConfig_t *zones[MAX_GEOZONES_IN_CONFIG];
            uint8_t zoneCount = getZonesForPos(zones, intersection, true);
            
            float maxAlt = FLT_MAX;
            for (uint8_t i = 0; i < zoneCount; i++) {
                if (zones[i]->config.type == GEOZONE_TYPE_INCLUSIVE && zones[i]->config.minAltitude > intersection->z) {
                    float alt = zones[i]->config.minAltitude;
                    if (alt < maxAlt) {
                        maxAlt = alt;
                    }
                }
            }

            if (maxAlt < FLT_MAX) {
                fzStepAlt = maxAlt + geoZoneConfig()->safeAltitudeDistance * 2;
                avoidFzStep = true;
            }
        }
        // We can move upwards
        if ((zone->config.type == GEOZONE_TYPE_EXCLUSIVE && geozone.zoneInfo > 0 && (geozone.zoneInfo < geoZoneConfig()->avoidAltitudeRange)) || avoidFzStep)  {

            calculateFarAwayTarget(&posControl.sendTo.targetPos, posControl.actualState.cog, geoZoneConfig()->fenceDetectionDistance * 2);
            if (avoidFzStep) {
                posControl.sendTo.targetPos.z = fzStepAlt;
            } else {
                posControl.sendTo.targetPos.z = zone->config.maxAltitude + geoZoneConfig()->safeAltitudeDistance * 2;
            }

            posControl.sendTo.lockSticks = true;
            posControl.sendTo.lockStickTime = STICK_LOCK_MIN_TIME;
            posControl.sendTo.targetRange = POS_DETECTION_DISTANCE;
            posControl.sendTo.altitudeTargetRange = geoZoneConfig()->safeAltitudeDistance / 2;
            avoidingPoint = *intersection;
            actionState = GEOZONE_ACTION_STATE_AVOIDING_UPWARD;
            actionStartTime = millis();
            avoidingPoint = *intersection;
            activateSendTo();
        } else  {
            // Calc new course
            avoidCourse = calcBounceCourseForZone(zone, &prevPoint, intersection);
            
            if (avoidCourse == -1) {
                return;
            }

            calculateFarAwayTarget(&posControl.sendTo.targetPos, avoidCourse, geoZoneConfig()->fenceDetectionDistance * 2); // Its too far, mode will be abort if we are on the right course

            // Check for min/max altitude
            if (geozone.currentzoneMaxAltitude > 0 && getEstimatedActualPosition(Z) > geozone.currentzoneMaxAltitude) {
                posControl.sendTo.targetPos.z = geozone.currentzoneMaxAltitude - geoZoneConfig()->safeAltitudeDistance * 0.25;
            } else if (geozone.currentzoneMinAltitude != 0 && getEstimatedActualPosition(Z) < geozone.currentzoneMinAltitude) {
                posControl.sendTo.targetPos.z = geozone.currentzoneMinAltitude + geoZoneConfig()->safeAltitudeDistance * 0.25;
            }
        
            posControl.sendTo.lockSticks = true;
            posControl.sendTo.lockStickTime = AVOID_TIMEOUT;
            posControl.sendTo.targetRange = POS_DETECTION_DISTANCE;
            posControl.sendTo.altitudeTargetRange = geoZoneConfig()->safeAltitudeDistance / 2;
            avoidingPoint = *intersection;
            actionState = GEOZONE_ACTION_STATE_AVOIDING;
            actionStartTime = millis();
            avoidingPoint = *intersection;
            activateSendTo();
        }
    } else if (zone->config.fenceAction == GEOFENCE_ACTION_POS_HOLD) {
        actionState = GEOZONE_ACTION_STATE_POSHOLD;
        
        if (STATE(AIRPLANE)) {
            if (zone->config.shape == GEOZONE_SHAPE_POLYGON) {
                fpVector3_t refPoint;
                int32_t course = calcBounceCourseForZone(zone, &prevPoint, intersection);
                calculateFarAwayTarget(&refPoint, course, geoZoneConfig()->fenceDetectionDistance * 2);
                if (isPointRightFromLine((fpVector2_t*)&navGetCurrentActualPositionAndVelocity()->pos, (fpVector2_t*)&prevPoint, (fpVector2_t*)&refPoint)) {
                    geozone.loiterDir = 1; // Right
                } else {
                    geozone.loiterDir = -1; // Left
                }
            } else if (zone->config.shape == GEOZONE_SHAPE_CIRCULAR) {
                if (isPointRightFromLine((fpVector2_t*)&navGetCurrentActualPositionAndVelocity()->pos, (fpVector2_t*)&prevPoint, &zone->verticesLocal[0])) {
                    geozone.loiterDir = -1; // Left
                } else {
                    geozone.loiterDir = 1; // Right
                }
            }
        }

        geozone.sticksLocked = true;
        activateForcedPosHold();
        actionStartTime = millis();
    } else if (zone->config.fenceAction == GEOFENCE_ACTION_RTH) {
        actionState = GEOZONE_ACTION_STATE_RTH;
        geozone.sticksLocked = true;
        activateForcedRTH();
        actionStartTime = millis();
    }
}

static void endFenceAction(void)
{
    posControl.sendTo.lockSticks = false;
    geozone.sticksLocked = false;
    geozone.sticksLocked = 0;

    switch (actionState) {
        case GEOZONE_ACTION_STATE_AVOIDING:
        case GEOZONE_ACTION_STATE_AVOIDING_ALTITUDE:
        case GEOZONE_ACTION_STATE_FLYOUT_NFZ:
        case GEOZONE_ACTION_STATE_RETURN_TO_FZ:
            abortSendTo();
            break;
        case GEOZONE_ACTION_STATE_POSHOLD:
            abortForcedPosHold();
            break;
        case GEOZONE_ACTION_STATE_RTH:
            abortForcedRTH();
            break;
        default:
            break;
    }

    actionState = GEOZONE_ACTION_STATE_NONE;

    if (IS_RC_MODE_ACTIVE(BOXNAVALTHOLD) || IS_RC_MODE_ACTIVE(BOXNAVCRUISE)){
        setDesiredPosition(&posControl.sendTo.targetPos, 0, NAV_POS_UPDATE_Z);
    }

    abortSendTo();
}

static void geoZoneInit(void)
{
    activeGeoZonesCount = 0;
    uint8_t expectedVertices = 0, configuredVertices = 0;
    for (uint8_t i = 0; i < MAX_GEOZONES_IN_CONFIG; i++)
    {
        if (geoZonesConfig(i)->vertexCount > 0) {
            memcpy(&activeGeoZones[activeGeoZonesCount].config, geoZonesConfig(i), sizeof(geoZoneRuntimeConfig_t));
            if (activeGeoZones[i].config.maxAltitude == 0) {
                activeGeoZones[i].config.maxAltitude = INT32_MAX;
            }

            if (activeGeoZones[i].config.isSealevelRef) {
                
                if (activeGeoZones[i].config.maxAltitude != 0) {
                    activeGeoZones[i].config.maxAltitude -= GPS_home.alt;
                }
                
                if (activeGeoZones[i].config.minAltitude != 0) {
                    activeGeoZones[i].config.minAltitude -= GPS_home.alt;
                }
            }
            
            activeGeoZones[i].isInfZone = activeGeoZones[i].config.maxAltitude == INT32_MAX && activeGeoZones[i].config.minAltitude == 0;
            
            if (!STATE(AIRPLANE) && activeGeoZones[i].config.fenceAction == GEOFENCE_ACTION_AVOID) {
                activeGeoZones[i].config.fenceAction = GEOFENCE_ACTION_POS_HOLD;
            }

            activeGeoZones[activeGeoZonesCount].enable = true;
            activeGeoZonesCount++;
        }
        expectedVertices += geoZonesConfig(i)->vertexCount;
    }
    
    if (activeGeoZonesCount > 0) {
        // Covert geozone vertices to local
        for (uint8_t i = 0; i < MAX_VERTICES_IN_CONFIG; i++)  {
            gpsLocation_t vertexLoc;
            fpVector3_t posLocal3;

            if (geoZoneVertices(i)->zoneId >= 0 && geoZoneVertices(i)->zoneId < MAX_GEOZONES_IN_CONFIG && geoZoneVertices(i)->idx <= MAX_VERTICES_IN_CONFIG) {         
                configuredVertices++;
                if (geoZonesConfig(geoZoneVertices(i)->zoneId)->shape == GEOZONE_SHAPE_CIRCULAR && geoZoneVertices(i)->idx == 1) {
                    activeGeoZones[geoZoneVertices(i)->zoneId].radius = geoZoneVertices(i)->lat;
                    activeGeoZones[geoZoneVertices(i)->zoneId].config.vertexCount = 1;
                    continue;
                }
                
                vertexLoc.lat = geoZoneVertices(i)->lat;
                vertexLoc.lon = geoZoneVertices(i)->lon;
                geoConvertGeodeticToLocal(&posLocal3, &posControl.gpsOrigin, &vertexLoc, GEO_ALT_ABSOLUTE);

                uint8_t vertexIdx = 0;
                for (uint8_t j = 0; j < geoZoneVertices(i)->zoneId; j++) {
                    vertexIdx += activeGeoZones[j].config.vertexCount;
                }
                vertexIdx += geoZoneVertices(i)->idx;

                verticesLocal[vertexIdx].x = posLocal3.x;
                verticesLocal[vertexIdx].y = posLocal3.y;

                if (geoZoneVertices(i)->idx == 0) {
                    activeGeoZones[geoZoneVertices(i)->zoneId].verticesLocal = &verticesLocal[vertexIdx];
                }
            }
        }
    }

    if (geoZoneConfig()->nearestSafeHomeAsInclusivZone && posControl.safehomeState.index >= 0)
    {       
        safeHomeGeozoneConfig.shape = GEOZONE_SHAPE_CIRCULAR;
        safeHomeGeozoneConfig.type = GEOZONE_TYPE_INCLUSIVE;
        safeHomeGeozoneConfig.fenceAction = geoZoneConfig()->safeHomeFenceAction;
        safeHomeGeozoneConfig.maxAltitude = 0;
        safeHomeGeozoneConfig.minAltitude = 0;
        safeHomeGeozoneConfig.vertexCount = 1;

        activeGeoZones[activeGeoZonesCount].config = safeHomeGeozoneConfig;
        activeGeoZones[activeGeoZonesCount].verticesLocal = (fpVector2_t*)&posControl.safehomeState.nearestSafeHome;
        activeGeoZones[activeGeoZonesCount].radius = navConfig()->general.safehome_max_distance;
        activeGeoZonesCount++;
        expectedVertices++;
        configuredVertices++;
    }

    updateCurrentZones();
    uint8_t newActiveZoneCount = activeGeoZonesCount;
    for (uint8_t i = 0; i < activeGeoZonesCount; i++) {
        if (!geozone.insideFz) {
            // Deactivate all inclusive geozones with distance > GEOZONE_INCLUSE_IGNORE_DISTANCE
            if (activeGeoZones[i].config.type == GEOZONE_TYPE_INCLUSIVE && !isInGeozone(&activeGeoZones[i], &navGetCurrentActualPositionAndVelocity()->pos, false)) {
                fpVector2_t pos2 = { .x = getEstimatedActualPosition(X), .y = getEstimatedActualPosition(Y) };

                uint32_t minDistanceToZone = INT32_MAX;
                for (uint8_t j = 0; j < activeGeoZones[i].config.vertexCount; j++) {            
                    float dist = calculateDistance2(&pos2, &activeGeoZones[i].verticesLocal[j]);
                    if (activeGeoZones[i].config.shape == GEOZONE_SHAPE_CIRCULAR) {
                        minDistanceToZone = dist - activeGeoZones[i].radius;
                        break;
                    }

                    if (dist < minDistanceToZone) {
                        minDistanceToZone = dist;
                    }
                }
                if (minDistanceToZone > GEOZONE_INCLUSE_IGNORE_DISTANCE) {
                    activeGeoZones[i].enable = false;
                    newActiveZoneCount--;
                }
            }
        }    
    }

    activeGeoZonesCount = newActiveZoneCount;
    if (activeGeoZonesCount == 0 || expectedVertices != configuredVertices) {
        setTaskEnabled(TASK_GEOZONE, false);
        geozoneIsEnabled = false;
        return;
    }
    geozoneIsEnabled = true;

    qsort(activeGeoZones, MAX_GEOZONES, sizeof(geoZoneRuntimeConfig_t), geoZoneRTComp);
    
    for (int i = 0; i < activeGeoZonesCount; i++) {
        if (activeGeoZones[i].config.type == GEOZONE_TYPE_INCLUSIVE) {
            isAtLeastOneInclusiveZoneActive = true;
            break;
        }
    }
}

// Called by Scheduler
void geozoneUpdate(timeUs_t curentTimeUs)
{   
    UNUSED(curentTimeUs);
    
    geozone.messageState = GEOZONE_MESSAGE_STATE_NONE;
    if (!isInitalised && navigationPositionEstimateIsHealthy()) {
        geoZoneInit();
        isInitalised = true;
    }
          
    if (!ARMING_FLAG(ARMED) || !isInitalised || activeGeoZonesCount == 0) {
        noZoneRTH = false;
        return;
    } 
    
    // Zone RTH Override: Toggle RTH switch short
    if (geozone.avoidInRTHInProgress) {
        if (rthHomeSwitchLastState != IS_RC_MODE_ACTIVE(BOXNAVRTH)) {
            if (millis() - rthOverrideStartTime < RTH_OVERRIDE_TIMEOUT) {
                geozoneResetRTH();
                noZoneRTH = true;
            }
            rthOverrideStartTime = millis();  
        } 
        rthHomeSwitchLastState = IS_RC_MODE_ACTIVE(BOXNAVRTH);
    }

    updateCurrentZones();
    updateZoneInfos();

    if (STATE(AIRPLANE) && (navGetCurrentStateFlags() & NAV_CTL_LAUNCH)) {
        return;
    }
    
    // User has switched to non geofence mode, end all actions and switch to mode from box input
    if (isNonGeozoneModeFromBoxInput()) {
        if (actionState != GEOZONE_ACTION_STATE_NONE) {
            endFenceAction();
        }
        lockRTZ = false;
        return;
    }

    switch (actionState)
    {
        case GEOZONE_ACTION_STATE_AVOIDING: 
            if (calculateDistanceToDestination(&avoidingPoint) > geozoneGetDetectionDistance()) {
                posControl.sendTo.lockSticks = false;
            }            
            geozone.messageState = GEOZONE_MESSAGE_STATE_AVOIDING_FB;
            if (IS_IN_TOLERANCE_RANGE(avoidCourse, DECIDEGREES_TO_CENTIDEGREES(gpsSol.groundCourse), 500) || millis() - actionStartTime > AVOID_TIMEOUT || !posControl.flags.sendToActive) {
                endFenceAction();
            }
            return;
        case GEOZONE_ACTION_STATE_AVOIDING_ALTITUDE:
            geozone.messageState = GEOZONE_MESSAGE_STATE_AVOIDING_ALTITUDE_BREACH;
            if (IS_IN_TOLERANCE_RANGE(getEstimatedActualPosition(Z), posControl.sendTo.targetPos.z, posControl.sendTo.altitudeTargetRange) || !posControl.flags.sendToActive || millis() - actionStartTime > AVOID_TIMEOUT) {
                endFenceAction();
            }
            return;
        case GEOZONE_ACTION_STATE_RETURN_TO_FZ:
            geozone.messageState = GEOZONE_MESSAGE_STATE_RETURN_TO_ZONE;
            if ((geozone.insideFz && ABS(geozone.distanceVertToNearestZone ) > geoZoneConfig()->safeAltitudeDistance) || !posControl.flags.sendToActive) {
                lockRTZ = true;
                endFenceAction();
            }
            return;
        case GEOZONE_ACTION_STATE_FLYOUT_NFZ:
            geozone.messageState = GEOZONE_MESSAGE_STATE_FLYOUT_NFZ;
            if (!geozone.insideNfz || !posControl.flags.sendToActive) {       
                endFenceAction();
            }
            return;
        case GEOZONE_ACTION_STATE_AVOIDING_UPWARD:
            geozone.messageState = GEOZONE_MESSAGE_STATE_AVOIDING_FB;
            if (IS_IN_TOLERANCE_RANGE(getEstimatedActualPosition(Z), posControl.sendTo.targetPos.z, posControl.sendTo.altitudeTargetRange) || !posControl.flags.sendToActive || millis() - actionStartTime > AVOID_TIMEOUT) {
                endFenceAction();
            }
            return;
        case GEOZONE_ACTION_STATE_RTH:
        case GEOZONE_ACTION_STATE_POSHOLD:
            geozone.messageState = GEOZONE_MESSAGE_STATE_POS_HOLD;
            if (geozone.sticksLocked && millis() - actionStartTime > STICK_LOCK_MIN_TIME) {
                geozone.sticksLocked = false;
            }
            if (!geozone.sticksLocked && areSticksDeflectdFromChannelValue()) {
                endFenceAction();
            }
            return;
        default:
            break;
    }
    
    if ((IS_RC_MODE_ACTIVE(BOXHORIZON) || IS_RC_MODE_ACTIVE(BOXANGLE)) && 
        actionState == GEOZONE_ACTION_STATE_NONE &&
        geozone.nearestHorZoneHasAction && 
        ABS(geozone.distanceVertToNearestZone) > 0 && 
        ABS(geozone.distanceVertToNearestZone) < geoZoneConfig()->safeAltitudeDistance) {
        
        float targetAltitide = 0;
        uint32_t extraSafteyAlt = geoZoneConfig()->safeAltitudeDistance * 0.25;
        if (nearestHorZone->config.type == GEOZONE_TYPE_INCLUSIVE && geozone.insideFz) {
            if (geozone.distanceVertToNearestZone > 0) {
                targetAltitide = geozone.currentzoneMaxAltitude - extraSafteyAlt;
            } else {
                targetAltitide = geozone.currentzoneMinAltitude + extraSafteyAlt;
            }
        } else if (nearestHorZone->config.type == GEOZONE_TYPE_EXCLUSIVE && !geozone.insideNfz) {
           if (geozone.distanceVertToNearestZone > 0) {
                targetAltitide = geozone.currentzoneMinAltitude - extraSafteyAlt;
            } else {
                targetAltitide = geozone.currentzoneMaxAltitude + extraSafteyAlt;
            }
        }
        
        fpVector3_t targetPos;
        calculateFarAwayTarget(&targetPos, posControl.actualState.cog, 100000);
        posControl.sendTo.targetPos.x = targetPos.x;
        posControl.sendTo.targetPos.y = targetPos.y;
        posControl.sendTo.targetPos.z = targetAltitide;
        posControl.sendTo.altitudeTargetRange = 200;
        posControl.sendTo.targetRange = POS_DETECTION_DISTANCE;
        posControl.sendTo.lockSticks = true;
        posControl.sendTo.lockStickTime = STICK_LOCK_MIN_TIME;
        actionState = GEOZONE_ACTION_STATE_AVOIDING_ALTITUDE;
        actionStartTime = millis();
        activateSendTo();
        return;
    }    

    // RTH active: Further checks are done in RTH logic
    if ((navGetCurrentStateFlags() & NAV_AUTO_RTH) || IS_RC_MODE_ACTIVE(BOXNAVRTH) || posControl.flags.forcedRTHActivated || FLIGHT_MODE(NAV_FW_AUTOLAND)) {
        return;
    } else if (geozone.avoidInRTHInProgress) {
        geozoneResetRTH();
    } 

    if (lockRTZ && (geozone.insideFz || geozone.insideNfz)) {
        lockRTZ = false;
    }

    // RTZ: Return to zone: 
    if (geozone.insideNfz || (!geozone.insideFz && isAtLeastOneInclusiveZoneActive)) {

        if (isAtLeastOneInclusiveZoneActive && !geozone.insideFz) {
            geozone.messageState = GEOZONE_MESSAGE_STATE_OUTSIDE_FZ;
        }
        
        if (geozone.insideNfz) {
            geozone.messageState = GEOZONE_MESSAGE_STATE_NFZ;
        }
        
        if (!isNonGeozoneModeFromBoxInput()) {
            bool flyOutNfz = false;
            geoZoneRuntimeConfig_t *targetZone = nearestInclusiveZone;
            
            for (uint8_t i = 0; i < currentZoneCount; i++) {
                if (currentZones[i]->config.type == GEOZONE_TYPE_EXCLUSIVE && currentZones[i]->config.fenceAction != GEOFENCE_ACTION_NONE) {
                    flyOutNfz = true;
                    targetZone = currentZones[i];
                    break;
                }
            }

            if (targetZone != NULL && !lockRTZ && (flyOutNfz || (!geozone.insideFz && targetZone->config.fenceAction != GEOFENCE_ACTION_NONE))) {
                int32_t targetAltitude = 0;                
                if (getEstimatedActualPosition(Z) >= targetZone->config.maxAltitude - geoZoneConfig()->safeAltitudeDistance) {
                    targetAltitude = targetZone->config.maxAltitude - geoZoneConfig()->safeAltitudeDistance * 1.5;
                } else if (getEstimatedActualPosition(Z) <= targetZone->config.minAltitude + geoZoneConfig()->safeAltitudeDistance) {
                    targetAltitude = targetZone->config.minAltitude + geoZoneConfig()->safeAltitudeDistance * 1.5;
                } else {
                    targetAltitude = getEstimatedActualPosition(Z);
                }
                
                fpVector3_t targetPos;
                if (aboveOrUnderZone) {
                    if (ABS(geozone.distanceVertToNearestZone) < 2000) {
                        calculateFarAwayTarget(&targetPos, posControl.actualState.cog, 100000);
                        if(geozone.distanceVertToNearestZone > 0) {
                            targetAltitude = getEstimatedActualPosition(Z) + ABS(geozone.distanceVertToNearestZone) + geoZoneConfig()->safeAltitudeDistance * 1.5;
                        } else {
                            targetAltitude = getEstimatedActualPosition(Z) - ABS(geozone.distanceVertToNearestZone) - geoZoneConfig()->safeAltitudeDistance * 1.5;
                        }

                    } else {
                        targetPos = navGetCurrentActualPositionAndVelocity()->pos;
                    }
                } else {
                    calculateFarAwayTarget(&targetPos, geozone.directionToNearestZone, geozone.distanceHorToNearestZone + geoZoneConfig()->fenceDetectionDistance / 2);
                }
             
                posControl.sendTo.targetPos.x = targetPos.x;
                posControl.sendTo.targetPos.y = targetPos.y;
                posControl.sendTo.targetPos.z = targetAltitude;
                posControl.sendTo.altitudeTargetRange = 200;
                posControl.sendTo.targetRange = POS_DETECTION_DISTANCE;
                posControl.sendTo.lockSticks = true;
                posControl.sendTo.lockStickTime = STICK_LOCK_MIN_TIME;
                
                if (flyOutNfz) {
                    actionState = GEOZONE_ACTION_STATE_FLYOUT_NFZ;
                } else {
                    actionState = GEOZONE_ACTION_STATE_RETURN_TO_FZ;
                }
                
                activateSendTo();
            }
        }
    }


    geoZoneRuntimeConfig_t *intersectZone = NULL;
    fpVector3_t intersection, prevPoint;
    float distanceToZone = 0;
    calcPreviewPoint(&prevPoint, geoZoneConfig()->fenceDetectionDistance);
    if (findNearestIntersectionZone(&intersectZone, &intersection, &distanceToZone, geoZoneConfig()->fenceDetectionDistance, &navGetCurrentActualPositionAndVelocity()->pos, &prevPoint)) {    
        if (geozone.insideFz) {       
            if (!isPointInAnyOtherZone(intersectZone, GEOZONE_TYPE_INCLUSIVE, &intersection)) {
                geozone.distanceToZoneBorder3d = (uint32_t)roundf(distanceToZone);
                geozone.messageState = GEOZONE_MESSAGE_STATE_LEAVING_FZ;
                performeFenceAction(intersectZone, &intersection);
            }
        } 
        
        if (!geozone.insideNfz && intersectZone->config.type == GEOZONE_TYPE_EXCLUSIVE && (intersectZone->config.minAltitude != 0 || intersection.z > 0)) {
            geozone.distanceToZoneBorder3d = (uint32_t)roundf(distanceToZone);            
            int32_t minAltitude = intersectZone->config.minAltitude;
            int32_t maxAltitude = intersectZone->config.maxAltitude;
            if (intersectZone->isInfZone || (minAltitude == 0 && maxAltitude == INT32_MAX)) {
                geozone.zoneInfo = INT32_MAX;
            } else if (maxAltitude == INT32_MAX) {
                geozone.zoneInfo = minAltitude - getEstimatedActualPosition(Z);
            } else if (minAltitude == 0) {
                geozone.zoneInfo = maxAltitude - getEstimatedActualPosition(Z);
            } else {
                int32_t distToMax = maxAltitude - getEstimatedActualPosition(Z);
                int32_t distToMin = minAltitude - getEstimatedActualPosition(Z);
                if (ABS(distToMin) < ABS(distToMax)) {
                    geozone.zoneInfo = distToMin;
                } else {
                    geozone.zoneInfo = distToMax;
                }
            }
        
            geozone.messageState = GEOZONE_MESSAGE_STATE_ENTERING_NFZ;
            performeFenceAction(intersectZone, &intersection);
        }
    } 
}

fpVector3_t *geozoneGetCurrentRthAvoidWaypoint(void)
{
    return &rthWaypoints[rthWaypointIndex];
}

void geozoneAdvanceRthAvoidWaypoint(void)
{
    if (rthWaypointIndex < rthWaypointCount) {
        rthWaypointIndex++;
    }
}

bool geoZoneIsLastRthWaypoint(void)
{
    return rthWaypointIndex == rthWaypointCount - 1;
}

void geozoneResetRTH(void)
{
    geozone.avoidInRTHInProgress = false;
    rthWaypointIndex = 0;
    rthWaypointCount = 0;
}

void geozoneSetupRTH(void) {
    if (!geozone.insideFz && isAtLeastOneInclusiveZoneActive) {
        noZoneRTH = true;
    } else {
        noZoneRTH = false;
    }
}

// Return value
// -1: Unable to calculate a course home
//  0: No NFZ in the way
// >0: Number of waypoints 
int8_t geozoneCheckForNFZAtCourse(bool isRTH)
{    
    UNUSED(isRTH);
    
    if (geozone.avoidInRTHInProgress || noZoneRTH || !geozoneIsEnabled || !isInitalised) {
        return 0;
    }

    updateCurrentZones();
    
    // Never mind, lets fly out of the zone on current course 
    if (geozone.insideNfz || (isAtLeastOneInclusiveZoneActive && !geozone.insideFz)) {
        return 0;
    }

    int8_t waypointCount = calcRthCourse(rthWaypoints, &navGetCurrentActualPositionAndVelocity()->pos, &posControl.rthState.homePosition.pos);
    if (waypointCount > 0) {
        rthWaypointCount = waypointCount;
        rthWaypointIndex = 0;
        geozone.avoidInRTHInProgress = true;
        return 1;
    } 

    return waypointCount;
}

bool isGeozoneActive(void) 
{
    return activeGeoZonesCount > 0;
}

void geozoneUpdateMaxHomeAltitude(void) {
    int32_t altitude = INT32_MIN;
    geoZoneRuntimeConfig_t *zones[MAX_GEOZONES];
    uint8_t zoneCount = getZonesForPos(zones, &posControl.rthState.homePosition.pos, false);
    for (uint8_t i = 0; i < zoneCount; i++) {
        if (zones[i]->config.type == GEOZONE_TYPE_INCLUSIVE) {
            altitude = MAX(zones[i]->config.maxAltitude, altitude);
        }
    }

    if (altitude > INT32_MIN) {
        geozone.maxHomeAltitude = altitude - geoZoneConfig()->safeAltitudeDistance;
        geozone.homeHasMaxAltitue = true;
    } else {
        geozone.homeHasMaxAltitue = false;
    }
}

// Avoid arming in NFZ 
bool geozoneIsBlockingArming(void)
{
    // Do not generate arming flags unless we are sure about them
    if (!isInitalised || !geozoneIsEnabled || activeGeoZonesCount == 0)  {
        return false;
    }
    
    for (uint8_t i = 0; i < activeGeoZonesCount; i++) {
        if (activeGeoZones[i].config.type == GEOZONE_TYPE_EXCLUSIVE && isInGeozone(&activeGeoZones[i], &navGetCurrentActualPositionAndVelocity()->pos, false)) {
            return true;
        }
    }

    for (uint8_t i = 0; i < activeGeoZonesCount; i++) {
        if (activeGeoZones[i].config.type == GEOZONE_TYPE_INCLUSIVE && isInGeozone(&activeGeoZones[i], &navGetCurrentActualPositionAndVelocity()->pos, false)) {
            return false;
        }
    }

    // We aren't in any zone, is an inclusive one still active?
    for (uint8_t i = 0; i < activeGeoZonesCount; i++) {
        if (activeGeoZones[i].config.type == GEOZONE_TYPE_INCLUSIVE) {
            return true;
        }
    }
    
    return false;
}

#endif
