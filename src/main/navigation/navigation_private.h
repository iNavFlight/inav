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

#define DISTANCE_BETWEEN_TWO_LONGITUDE_POINTS_AT_EQUATOR    1.113195f  // MagicEarthNumber from APM

#include "common/axis.h"
#include "common/maths.h"
#include "common/filter.h"
#include "common/time.h"
#include "common/vector.h"
#include "fc/runtime_config.h"
#include "navigation/navigation.h"

#define MIN_POSITION_UPDATE_RATE_HZ         5       // Minimum position update rate at which XYZ controllers would be applied
#define NAV_THROTTLE_CUTOFF_FREQENCY_HZ     4       // low-pass filter on throttle output
#define NAV_FW_CONTROL_MONITORING_RATE      2
#define NAV_DTERM_CUT_HZ                    10.0f
#define NAV_VEL_Z_DERIVATIVE_CUT_HZ         5.0f
#define NAV_VEL_Z_ERROR_CUT_HZ              5.0f
#define NAV_ACCELERATION_XY_MAX             980.0f  // cm/s/s       // approx 45 deg lean angle

#define INAV_SURFACE_MAX_DISTANCE           40

#define MC_POS_CONTROL_JERK_LIMIT_CMSSS     1700.0f // jerk limit on horizontal acceleration (cm/s^3)

#define MC_LAND_CHECK_VEL_XY_MOVING         100.0f  // cm/s
#define MC_LAND_CHECK_VEL_Z_MOVING          100.0f  // cm/s
#define MC_LAND_THR_STABILISE_DELAY         1       // seconds
#define MC_LAND_DESCEND_THROTTLE            40      // RC pwm units (us)
#define MC_LAND_SAFE_SURFACE                5.0f    // cm

#define MAX_POSITION_UPDATE_INTERVAL_US     HZ2US(MIN_POSITION_UPDATE_RATE_HZ)        // convenience macro
_Static_assert(MAX_POSITION_UPDATE_INTERVAL_US <= TIMEDELTA_MAX, "deltaMicros can overflow!");

typedef enum {
    NAV_POS_UPDATE_NONE                 = 0,
    NAV_POS_UPDATE_Z                    = 1 << 1,
    NAV_POS_UPDATE_XY                   = 1 << 0,
    NAV_POS_UPDATE_HEADING              = 1 << 2,
    NAV_POS_UPDATE_BEARING              = 1 << 3,
    NAV_POS_UPDATE_BEARING_TAIL_FIRST   = 1 << 4,
} navSetWaypointFlags_t;

typedef enum {
    ROC_TO_ALT_CURRENT,
    ROC_TO_ALT_CONSTANT,
    ROC_TO_ALT_TARGET
} climbRateToAltitudeControllerMode_e;

typedef enum {
    EST_NONE = 0,       // No valid sensor present
    EST_USABLE = 1,     // Estimate is usable but may be inaccurate
    EST_TRUSTED = 2     // Estimate is usable and based on actual sensor data
} navigationEstimateStatus_e;

typedef enum {
    NAV_HOME_INVALID = 0,
    NAV_HOME_VALID_XY = 1 << 0,
    NAV_HOME_VALID_Z = 1 << 1,
    NAV_HOME_VALID_HEADING = 1 << 2,
    NAV_HOME_VALID_ALL = NAV_HOME_VALID_XY | NAV_HOME_VALID_Z | NAV_HOME_VALID_HEADING,
} navigationHomeFlags_t;

typedef struct navigationFlags_s {
    bool horizontalPositionDataNew;
    bool verticalPositionDataNew;

    bool horizontalPositionDataConsumed;
    bool verticalPositionDataConsumed;

    navigationEstimateStatus_e estAltStatus;        // Indicates that we have a working altitude sensor (got at least one valid reading from it)
    navigationEstimateStatus_e estPosStatus;        // Indicates that GPS is working (or not)
    navigationEstimateStatus_e estVelStatus;        // Indicates that GPS is working (or not)
    navigationEstimateStatus_e estAglStatus;
    navigationEstimateStatus_e estHeadingStatus;    // Indicate valid heading - wither mag or GPS at certain speed on airplane
    bool gpsCfEstimatedAltitudeMismatch;            // Indicates a mismatch between GPS altitude and estimated altitude

    climbRateToAltitudeControllerMode_e rocToAltMode;

    bool isAdjustingPosition;
    bool isAdjustingAltitude;
    bool isAdjustingHeading;

    // Behaviour modifiers
    bool isGCSAssistedNavigationEnabled;    // Does INAV accept WP#255 - follow-me etc.
    bool isGCSAssistedNavigationReset;      // GCS control was disabled - indicate that so code could take action accordingly
    bool isTerrainFollowEnabled;            // Does INAV use rangefinder for terrain following (adjusting baro altitude target according to rangefinders readings)

    // Failsafe actions
    bool forcedRTHActivated;
    bool forcedEmergLandingActivated;

    /* Landing detector */
    bool resetLandingDetector;

    bool wpMissionPlannerActive;            // Activation status of WP mission planner
    bool rthTrackbackActive;                // Activation status of RTH trackback
    bool wpTurnSmoothingActive;             // Activation status WP turn smoothing
    bool manualEmergLandActive;             // Activation status of manual emergency landing

#ifdef USE_GEOZONE
    bool sendToActive;
    bool forcedPosholdActive;
#endif
} navigationFlags_t;

typedef struct {
    fpVector3_t pos;
    fpVector3_t vel;
} navEstimatedPosVel_t;

typedef struct {
    // Local estimated states
    navEstimatedPosVel_t    abs;
    navEstimatedPosVel_t    agl;
    int32_t                 yaw;
    int32_t                 cog;

    // Service values
    float                   sinYaw;
    float                   cosYaw;
    float                   surfaceMin;
    float                   velXY;
} navigationEstimatedState_t;

typedef struct {
    fpVector3_t pos;
    fpVector3_t vel;
    int32_t     yaw;
    int16_t     climbRateDemand;
} navigationDesiredState_t;

typedef enum {
    NAV_FSM_EVENT_NONE = 0,
    NAV_FSM_EVENT_TIMEOUT,

    NAV_FSM_EVENT_SUCCESS,
    NAV_FSM_EVENT_ERROR,

    NAV_FSM_EVENT_SWITCH_TO_IDLE,
    NAV_FSM_EVENT_SWITCH_TO_ALTHOLD,
    NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D,
    NAV_FSM_EVENT_SWITCH_TO_RTH,
    NAV_FSM_EVENT_SWITCH_TO_WAYPOINT,
    NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING,
    NAV_FSM_EVENT_SWITCH_TO_LAUNCH,
    NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD,
    NAV_FSM_EVENT_SWITCH_TO_CRUISE,
    NAV_FSM_EVENT_SWITCH_TO_COURSE_ADJ,
    NAV_FSM_EVENT_SWITCH_TO_MIXERAT,
    NAV_FSM_EVENT_SWITCH_TO_NAV_STATE_FW_LANDING,
    NAV_FSM_EVENT_SWITCH_TO_SEND_TO,

    NAV_FSM_EVENT_STATE_SPECIFIC_1,             // State-specific event
    NAV_FSM_EVENT_STATE_SPECIFIC_2,             // State-specific event
    NAV_FSM_EVENT_STATE_SPECIFIC_3,             // State-specific event
    NAV_FSM_EVENT_STATE_SPECIFIC_4,             // State-specific event
    NAV_FSM_EVENT_STATE_SPECIFIC_5,             // State-specific event

    NAV_FSM_EVENT_SWITCH_TO_NAV_STATE_FW_LANDING_ABORT = NAV_FSM_EVENT_STATE_SPECIFIC_1,
    NAV_FSM_EVENT_SWITCH_TO_NAV_STATE_FW_LANDING_FINISHED = NAV_FSM_EVENT_STATE_SPECIFIC_2,
    NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_HOLD_TIME = NAV_FSM_EVENT_STATE_SPECIFIC_1,
    NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_RTH_LAND = NAV_FSM_EVENT_STATE_SPECIFIC_2,
    NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_FINISHED = NAV_FSM_EVENT_STATE_SPECIFIC_3,
    NAV_FSM_EVENT_SWITCH_TO_NAV_STATE_RTH_INITIALIZE = NAV_FSM_EVENT_STATE_SPECIFIC_1,
    NAV_FSM_EVENT_SWITCH_TO_NAV_STATE_RTH_TRACKBACK = NAV_FSM_EVENT_STATE_SPECIFIC_2,
    NAV_FSM_EVENT_SWITCH_TO_RTH_HEAD_HOME = NAV_FSM_EVENT_STATE_SPECIFIC_3,
    NAV_FSM_EVENT_SWITCH_TO_RTH_LOITER_ABOVE_HOME = NAV_FSM_EVENT_STATE_SPECIFIC_4,
    NAV_FSM_EVENT_SWITCH_TO_RTH_LANDING = NAV_FSM_EVENT_STATE_SPECIFIC_5,

    NAV_FSM_EVENT_COUNT,
} navigationFSMEvent_t;

// This enum is used to keep values in blackbox logs stable, so we can
// freely change navigationFSMState_t.
typedef enum {
    NAV_PERSISTENT_ID_UNDEFINED                                 = 0,

    NAV_PERSISTENT_ID_IDLE                                      = 1,

    NAV_PERSISTENT_ID_ALTHOLD_INITIALIZE                        = 2,
    NAV_PERSISTENT_ID_ALTHOLD_IN_PROGRESS                       = 3,

    NAV_PERSISTENT_ID_UNUSED_1                                  = 4,  // was NAV_STATE_POSHOLD_2D_INITIALIZE
    NAV_PERSISTENT_ID_UNUSED_2                                  = 5,  // was NAV_STATE_POSHOLD_2D_IN_PROGRESS

    NAV_PERSISTENT_ID_POSHOLD_3D_INITIALIZE                     = 6,
    NAV_PERSISTENT_ID_POSHOLD_3D_IN_PROGRESS                    = 7,

    NAV_PERSISTENT_ID_RTH_INITIALIZE                            = 8,
    NAV_PERSISTENT_ID_RTH_CLIMB_TO_SAFE_ALT                     = 9,
    NAV_PERSISTENT_ID_RTH_HEAD_HOME                             = 10,
    NAV_PERSISTENT_ID_RTH_LOITER_PRIOR_TO_LANDING               = 11,
    NAV_PERSISTENT_ID_RTH_LANDING                               = 12,
    NAV_PERSISTENT_ID_RTH_FINISHING                             = 13,
    NAV_PERSISTENT_ID_RTH_FINISHED                              = 14,

    NAV_PERSISTENT_ID_WAYPOINT_INITIALIZE                       = 15,
    NAV_PERSISTENT_ID_WAYPOINT_PRE_ACTION                       = 16,
    NAV_PERSISTENT_ID_WAYPOINT_IN_PROGRESS                      = 17,
    NAV_PERSISTENT_ID_WAYPOINT_REACHED                          = 18,
    NAV_PERSISTENT_ID_WAYPOINT_NEXT                             = 19,
    NAV_PERSISTENT_ID_WAYPOINT_FINISHED                         = 20,
    NAV_PERSISTENT_ID_WAYPOINT_RTH_LAND                         = 21,

    NAV_PERSISTENT_ID_EMERGENCY_LANDING_INITIALIZE              = 22,
    NAV_PERSISTENT_ID_EMERGENCY_LANDING_IN_PROGRESS             = 23,
    NAV_PERSISTENT_ID_EMERGENCY_LANDING_FINISHED                = 24,

    NAV_PERSISTENT_ID_LAUNCH_INITIALIZE                         = 25,
    NAV_PERSISTENT_ID_LAUNCH_WAIT                               = 26,
    NAV_PERSISTENT_ID_UNUSED_3                                  = 27, // was NAV_STATE_LAUNCH_MOTOR_DELAY
    NAV_PERSISTENT_ID_LAUNCH_IN_PROGRESS                        = 28,

    NAV_PERSISTENT_ID_COURSE_HOLD_INITIALIZE                    = 29,
    NAV_PERSISTENT_ID_COURSE_HOLD_IN_PROGRESS                   = 30,
    NAV_PERSISTENT_ID_COURSE_HOLD_ADJUSTING                     = 31,

    NAV_PERSISTENT_ID_CRUISE_INITIALIZE                         = 32,
    NAV_PERSISTENT_ID_CRUISE_IN_PROGRESS                        = 33,
    NAV_PERSISTENT_ID_CRUISE_ADJUSTING                          = 34,

    NAV_PERSISTENT_ID_WAYPOINT_HOLD_TIME                        = 35,
    NAV_PERSISTENT_ID_RTH_LOITER_ABOVE_HOME                     = 36,
    NAV_PERSISTENT_ID_UNUSED_4                                  = 37, // was NAV_STATE_WAYPOINT_HOVER_ABOVE_HOME
    NAV_PERSISTENT_ID_RTH_TRACKBACK                             = 38,

    NAV_PERSISTENT_ID_MIXERAT_INITIALIZE                        = 39,
    NAV_PERSISTENT_ID_MIXERAT_IN_PROGRESS                       = 40,
    NAV_PERSISTENT_ID_MIXERAT_ABORT                             = 41,
    NAV_PERSISTENT_ID_FW_LANDING_CLIMB_TO_LOITER                = 42,
    NAV_PERSISTENT_ID_FW_LANDING_LOITER                         = 43,
    NAV_PERSISTENT_ID_FW_LANDING_APPROACH                       = 44,
    NAV_PERSISTENT_ID_FW_LANDING_GLIDE                          = 45,
    NAV_PERSISTENT_ID_FW_LANDING_FLARE                          = 46,
    NAV_PERSISTENT_ID_FW_LANDING_ABORT                          = 47,
    NAV_PERSISTENT_ID_FW_LANDING_FINISHED                       = 48,

    NAV_PERSISTENT_ID_SEND_TO_INITALIZE                         = 49,
    NAV_PERSISTENT_ID_SEND_TO_IN_PROGRES                        = 50,
    NAV_PERSISTENT_ID_SEND_TO_FINISHED                          = 51
} navigationPersistentId_e;

typedef enum {
    NAV_STATE_UNDEFINED = 0,

    NAV_STATE_IDLE,

    NAV_STATE_ALTHOLD_INITIALIZE,
    NAV_STATE_ALTHOLD_IN_PROGRESS,

    NAV_STATE_POSHOLD_3D_INITIALIZE,
    NAV_STATE_POSHOLD_3D_IN_PROGRESS,

    NAV_STATE_RTH_INITIALIZE,
    NAV_STATE_RTH_CLIMB_TO_SAFE_ALT,
    NAV_STATE_RTH_TRACKBACK,
    NAV_STATE_RTH_HEAD_HOME,
    NAV_STATE_RTH_LOITER_PRIOR_TO_LANDING,
    NAV_STATE_RTH_LOITER_ABOVE_HOME,
    NAV_STATE_RTH_LANDING,
    NAV_STATE_RTH_FINISHING,
    NAV_STATE_RTH_FINISHED,

    NAV_STATE_WAYPOINT_INITIALIZE,
    NAV_STATE_WAYPOINT_PRE_ACTION,
    NAV_STATE_WAYPOINT_IN_PROGRESS,
    NAV_STATE_WAYPOINT_REACHED,
    NAV_STATE_WAYPOINT_HOLD_TIME,
    NAV_STATE_WAYPOINT_NEXT,
    NAV_STATE_WAYPOINT_FINISHED,
    NAV_STATE_WAYPOINT_RTH_LAND,

    NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
    NAV_STATE_EMERGENCY_LANDING_IN_PROGRESS,
    NAV_STATE_EMERGENCY_LANDING_FINISHED,

    NAV_STATE_LAUNCH_INITIALIZE,
    NAV_STATE_LAUNCH_WAIT,
    NAV_STATE_LAUNCH_IN_PROGRESS,

    NAV_STATE_COURSE_HOLD_INITIALIZE,
    NAV_STATE_COURSE_HOLD_IN_PROGRESS,
    NAV_STATE_COURSE_HOLD_ADJUSTING,
    NAV_STATE_CRUISE_INITIALIZE,
    NAV_STATE_CRUISE_IN_PROGRESS,
    NAV_STATE_CRUISE_ADJUSTING,

    NAV_STATE_FW_LANDING_CLIMB_TO_LOITER,
    NAV_STATE_FW_LANDING_LOITER,
    NAV_STATE_FW_LANDING_APPROACH,
    NAV_STATE_FW_LANDING_GLIDE,
    NAV_STATE_FW_LANDING_FLARE,
    NAV_STATE_FW_LANDING_FINISHED,
    NAV_STATE_FW_LANDING_ABORT,

    NAV_STATE_MIXERAT_INITIALIZE,
    NAV_STATE_MIXERAT_IN_PROGRESS,
    NAV_STATE_MIXERAT_ABORT,

    NAV_STATE_SEND_TO_INITALIZE,
    NAV_STATE_SEND_TO_IN_PROGESS,
    NAV_STATE_SEND_TO_FINISHED,

    NAV_STATE_COUNT,
} navigationFSMState_t;

typedef enum {
    /* Navigation controllers */
    NAV_CTL_ALT             = (1 << 0),     // Altitude controller
    NAV_CTL_POS             = (1 << 1),     // Position controller
    NAV_CTL_YAW             = (1 << 2),
    NAV_CTL_EMERG           = (1 << 3),
    NAV_CTL_LAUNCH          = (1 << 4),

    /* Navigation requirements for flight modes and controllers */
    NAV_REQUIRE_ANGLE       = (1 << 5),
    NAV_REQUIRE_ANGLE_FW    = (1 << 6),
    NAV_REQUIRE_MAGHOLD     = (1 << 7),
    NAV_REQUIRE_THRTILT     = (1 << 8),

    /* Navigation autonomous modes */
    NAV_AUTO_RTH            = (1 << 9),
    NAV_AUTO_WP             = (1 << 10),

    /* Adjustments for navigation modes from RC input */
    NAV_RC_ALT              = (1 << 11),
    NAV_RC_POS              = (1 << 12),
    NAV_RC_YAW              = (1 << 13),

    /* Additional flags */
    NAV_CTL_LAND            = (1 << 14),
    NAV_AUTO_WP_DONE        = (1 << 15),    // Waypoint mission reached the last waypoint and is idling

    NAV_MIXERAT             = (1 << 16),    // MIXERAT in progress
    NAV_CTL_HOLD            = (1 << 17),    // Nav loiter active at position
} navigationFSMStateFlags_t;

typedef struct {
    navigationPersistentId_e            persistentId;
    navigationFSMEvent_t                (*onEntry)(navigationFSMState_t previousState);
    uint32_t                            timeoutMs;
    navSystemStatus_State_e             mwState;
    navSystemStatus_Error_e             mwError;
    navigationFSMStateFlags_t           stateFlags;
    flightModeFlags_e                   mapToFlightModes;
    navigationFSMState_t                onEvent[NAV_FSM_EVENT_COUNT];
} navigationFSMStateDescriptor_t;

typedef struct {
    timeMs_t        lastCheckTime;
    bool            rthSanityOK;
    float           minimalDistanceToHome;
} rthSanityChecker_t;

typedef struct {
    int32_t                     course;
    int32_t                     previousCourse;
    timeMs_t                    lastCourseAdjustmentTime;
    float                       multicopterSpeed;
} navCruise_t;

typedef struct {
    navigationHomeFlags_t   homeFlags;
    navWaypointPosition_t   homePosition;           // Original home position and base altitude
    float                   rthInitialAltitude;     // Altitude at start of RTH, can include added margins and extra height
    float                   rthClimbStageAltitude;  // Altitude at end of the climb phase
    float                   rthFinalAltitude;       // Altitude at end of RTH approach
    float                   rthInitialDistance;     // Distance when starting flight home
    fpVector3_t             homeTmpWaypoint;        // Temporary storage for home target
    fpVector3_t             originalHomePosition;   // the original rth home - save it, since it could be replaced by safehome or HOME_RESET
    bool                    rthLinearDescentActive; // Activation status of Linear Descent
} rthState_t;

#ifdef USE_FW_AUTOLAND
typedef enum {
    FW_AUTOLAND_WP_TURN,
    FW_AUTOLAND_WP_FINAL_APPROACH,
    FW_AUTOLAND_WP_LAND,
    FW_AUTOLAND_WP_COUNT,
} fwAutolandWaypoint_t;

typedef struct {
    timeUs_t loiterStartTime;
    fpVector3_t landWaypoints[FW_AUTOLAND_WP_COUNT];
    fpVector3_t landPos;
    int32_t landPosHeading;
    int32_t landingDirection;
    int32_t landAproachAltAgl;
    int32_t landAltAgl;
    uint8_t approachSettingIdx;
    fwAutolandWaypoint_t landCurrentWp;
    bool landAborted;
    bool landWp;
    fwAutolandState_t landState;
} fwLandState_t;
#endif

typedef enum {
    RTH_HOME_ENROUTE_INITIAL,       // Initial position for RTH approach
    RTH_HOME_ENROUTE_PROPORTIONAL,  // Prorpotional position for RTH approach
    RTH_HOME_ENROUTE_FINAL,         // Final position for RTH approach
    RTH_HOME_FINAL_LOITER,          // Final loiter altitude (if rth_home_altitude is set)
    RTH_HOME_FINAL_LAND,            // Home position and altitude
} rthTargetMode_e;

#ifdef USE_GEOZONE
typedef struct navSendTo_s {
    fpVector3_t targetPos;
    uint16_t altitudeTargetRange;   // 0 for only "2D" 
    uint32_t targetRange;
    bool lockSticks;
    uint32_t lockStickTime;
    timeMs_t startTime;
} navSendTo_t;
#endif

typedef struct {
    fpVector3_t nearestSafeHome;    // The nearestSafeHome found during arming
    uint32_t    distance;           // distance to the nearest safehome
    int8_t      index;              // -1 if no safehome, 0 to MAX_SAFEHOMES -1 otherwise
    bool        isApplied;          // whether the safehome has been applied to home
} safehomeState_t;

typedef struct {
    /* Flags and navigation system state */
    navigationFSMState_t        navState;
    navigationPersistentId_e    navPersistentId;

    navigationFlags_t           flags;

    /* Navigation PID controllers + pre-computed flight parameters */
    navigationPIDControllers_t  pids;
    float                       posDecelerationTime;
    float                       posResponseExpo;

    /* Local system state, both actual (estimated) and desired (target setpoint)*/
    navigationEstimatedState_t  actualState;
    navigationDesiredState_t    desiredState;   // waypoint coordinates + velocity

    uint32_t                    lastValidPositionTimeMs;
    uint32_t                    lastValidAltitudeTimeMs;

    /* INAV GPS origin (position where GPS fix was first acquired) */
    gpsOrigin_t                 gpsOrigin;

    /* Home/RTH parameters - NEU coordinates (geodetic position of home (LLH) is stored in GPS_home variable) */
    rthSanityChecker_t          rthSanityChecker;
    rthState_t                  rthState;
    uint32_t                    homeDistance;   // cm
    int32_t                     homeDirection;  // deg*100
    timeMs_t                    landingDelay;

    /* Safehome parameters */
    safehomeState_t             safehomeState;

    /* Cruise */
    navCruise_t                 cruise;

    /* Waypoint list */
    navWaypoint_t               waypointList[NAV_MAX_WAYPOINTS];
    bool                        waypointListValid;
    int8_t                      waypointCount;              // number of WPs in loaded mission
    int8_t                      startWpIndex;               // index of first waypoint in mission
    int8_t                      geoWaypointCount;           // total geospatial WPs in mission
    bool                        wpMissionRestart;           // mission restart from first waypoint

    /* WP Mission planner */
    int8_t                      wpMissionPlannerStatus;     // WP save status for setting in flight WP mission planner
    int8_t                      wpPlannerActiveWPIndex;
#ifdef USE_MULTI_MISSION
    /* Multi Missions */
    int8_t                      multiMissionCount;          // number of missions in multi mission entry
    int8_t                      loadedMultiMissionIndex;    // index of selected multi mission
    int8_t                      totalMultiMissionWpCount;   // total number of waypoints in all multi missions
#endif
    navWaypointPosition_t       activeWaypoint;             // Local position, current bearing and turn angle to next WP, filled on waypoint activation
    int8_t                      activeWaypointIndex;
    float                       wpInitialAltitude;          // Altitude at start of WP
    float                       wpInitialDistance;          // Distance when starting flight to WP
    float                       wpDistance;                 // Distance to active WP
    timeMs_t                    wpReachedTime;              // Time the waypoint was reached
    bool                        wpAltitudeReached;          // WP altitude achieved

#ifdef USE_FW_AUTOLAND
    /* Fixedwing autoland */
    fwLandState_t fwLandState;
#endif

#ifdef USE_GEOZONE
    navSendTo_t                  sendTo; // Used for Geozones
#endif

    /* Internals & statistics */
    int16_t                     rcAdjustment[4];
    float                       totalTripDistance;
} navigationPosControl_t;

typedef struct {
    float dTermAttenuation;
    float dTermAttenuationStart;
    float dTermAttenuationEnd;
    float breakingBoostFactor;
} multicopterPosXyCoefficients_t;

#if defined(NAV_NON_VOLATILE_WAYPOINT_STORAGE)
PG_DECLARE_ARRAY(navWaypoint_t, NAV_MAX_WAYPOINTS, nonVolatileWaypointList);
#endif

extern navigationPosControl_t posControl;
extern multicopterPosXyCoefficients_t multicopterPosXyCoefficients;

/* Internally used functions */
const navEstimatedPosVel_t * navGetCurrentActualPositionAndVelocity(void);

bool isThrustFacingDownwards(void);
uint32_t calculateDistanceToDestination(const fpVector3_t * destinationPos);
void calculateFarAwayTarget(fpVector3_t * farAwayPos, int32_t bearing, int32_t distance);
int32_t calculateBearingToDestination(const fpVector3_t * destinationPos);
float calculateDistance2(const fpVector2_t* startPos, const fpVector2_t* destinationPos);

bool isLandingDetected(void);
void resetLandingDetector(void);
bool isFlightDetected(void);
bool isFixedWingFlying(void);
bool isMulticopterFlying(void);

navigationFSMStateFlags_t navGetCurrentStateFlags(void);
flightModeFlags_e navGetMappedFlightModes(navigationFSMState_t state);

void setHomePosition(const fpVector3_t * pos, int32_t heading, navSetWaypointFlags_t useMask, navigationHomeFlags_t homeFlags);
void setDesiredPosition(const fpVector3_t * pos, int32_t yaw, navSetWaypointFlags_t useMask);
void setDesiredSurfaceOffset(float surfaceOffset);
void setDesiredPositionToFarAwayTarget(int32_t yaw, int32_t distance, navSetWaypointFlags_t useMask);   // NOT USED
void updateClimbRateToAltitudeController(float desiredClimbRate, float targetAltitude, climbRateToAltitudeControllerMode_e mode);

bool isNavHoldPositionActive(void);
bool isLastMissionWaypoint(void);
float getActiveSpeed(void);
bool isWaypointNavTrackingActive(void);

void updateActualHeading(bool headingValid, int32_t newHeading, int32_t newGroundCourse);
void updateActualHorizontalPositionAndVelocity(bool estPosValid, bool estVelValid, float newX, float newY, float newVelX, float newVelY);
void updateActualAltitudeAndClimbRate(bool estimateValid, float newAltitude, float newVelocity, float surfaceDistance, float surfaceVelocity, navigationEstimateStatus_e surfaceStatus, float gpsCfEstimatedAltitudeError);
float getDesiredClimbRate(float targetAltitude, timeDelta_t deltaMicros);

bool checkForPositionSensorTimeout(void);

bool isGPSGlitchDetected(void);

/* Multicopter-specific functions */
void setupMulticopterAltitudeController(void);

void resetMulticopterAltitudeController(void);
void resetMulticopterPositionController(void);
void resetMulticopterHeadingController(void);
void resetMulticopterBrakingMode(void);

bool adjustMulticopterAltitudeFromRCInput(void);
bool adjustMulticopterHeadingFromRCInput(void);
bool adjustMulticopterPositionFromRCInput(int16_t rcPitchAdjustment, int16_t rcRollAdjustment);

void applyMulticopterNavigationController(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs);
bool isMulticopterLandingDetected(void);
void calculateMulticopterInitialHoldPosition(fpVector3_t * pos);
float getSqrtControllerVelocity(float targetAltitude, timeDelta_t deltaMicros);

/* Fixed-wing specific functions */
void setupFixedWingAltitudeController(void);

void resetFixedWingAltitudeController(void);
void resetFixedWingPositionController(void);
void resetFixedWingHeadingController(void);

bool adjustFixedWingAltitudeFromRCInput(void);
bool adjustFixedWingHeadingFromRCInput(void);
bool adjustFixedWingPositionFromRCInput(void);

void applyFixedWingPositionController(timeUs_t currentTimeUs);
float processHeadingYawController(timeDelta_t deltaMicros, int32_t navHeadingError, bool errorIsDecreasing);
void applyFixedWingNavigationController(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs);

bool isFixedWingLandingDetected(void);

void calculateFixedWingInitialHoldPosition(fpVector3_t * pos);

/* Fixed-wing launch controller */
void resetFixedWingLaunchController(timeUs_t currentTimeUs);
void enableFixedWingLaunchController(timeUs_t currentTimeUs);
void abortFixedWingLaunch(void);
void applyFixedWingLaunchController(timeUs_t currentTimeUs);

/*
 * Rover specific functions
 */
void applyRoverBoatNavigationController(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs);
