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
#include <string.h>

#include "platform.h"

#include "build/debug.h"

#include "common/axis.h"
#include "common/filter.h"
#include "common/maths.h"
#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/fc_core.h"
#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#ifdef USE_MULTI_MISSION
#include "fc/rc_adjustments.h"
#include "fc/cli.h"
#endif
#include "fc/settings.h"

#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/pid.h"

#include "io/beeper.h"
#include "io/gps.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

#include "rx/rx.h"

#include "sensors/sensors.h"
#include "sensors/acceleration.h"
#include "sensors/boardalignment.h"
#include "sensors/battery.h"

// Multirotors:
#define MR_RTH_CLIMB_OVERSHOOT_CM   100  // target this amount of cm *above* the target altitude to ensure it is actually reached (Vz > 0 at target alt)
#define MR_RTH_CLIMB_MARGIN_MIN_CM  100  // start cruising home this amount of cm *before* reaching the cruise altitude (while continuing the ascend)
#define MR_RTH_CLIMB_MARGIN_PERCENT 15   // on high RTH altitudes use even bigger margin - percent of the altitude set

// Planes:
#define FW_RTH_CLIMB_OVERSHOOT_CM   100
#define FW_RTH_CLIMB_MARGIN_MIN_CM  100
#define FW_RTH_CLIMB_MARGIN_PERCENT 15

/*-----------------------------------------------------------
 * Compatibility for home position
 *-----------------------------------------------------------*/
gpsLocation_t GPS_home;
uint32_t      GPS_distanceToHome;        // distance to home point in meters
int16_t       GPS_directionToHome;       // direction to home point in degrees

fpVector3_t   original_rth_home;         // the original rth home - save it, since it could be replaced by safehome or HOME_RESET

radar_pois_t radar_pois[RADAR_MAX_POIS];
#if defined(USE_SAFE_HOME)
int8_t safehome_index = -1;               // -1 if no safehome, 0 to MAX_SAFEHOMES -1 otherwise
uint32_t safehome_distance = 0;           // distance to the nearest safehome
fpVector3_t nearestSafeHome;              // The nearestSafeHome found during arming
bool safehome_applied = false;            // whether the safehome has been applied to home.

PG_REGISTER_ARRAY(navSafeHome_t, MAX_SAFE_HOMES, safeHomeConfig, PG_SAFE_HOME_CONFIG , 0);

#endif

// waypoint 254, 255 are special waypoints
STATIC_ASSERT(NAV_MAX_WAYPOINTS < 254, NAV_MAX_WAYPOINTS_exceeded_allowable_range);

#if defined(NAV_NON_VOLATILE_WAYPOINT_STORAGE)
PG_REGISTER_ARRAY(navWaypoint_t, NAV_MAX_WAYPOINTS, nonVolatileWaypointList, PG_WAYPOINT_MISSION_STORAGE, 2);
#endif

PG_REGISTER_WITH_RESET_TEMPLATE(navConfig_t, navConfig, PG_NAV_CONFIG, 2);

PG_RESET_TEMPLATE(navConfig_t, navConfig,
    .general = {

        .flags = {
            .use_thr_mid_for_althold = SETTING_NAV_USE_MIDTHR_FOR_ALTHOLD_DEFAULT,
            .extra_arming_safety = SETTING_NAV_EXTRA_ARMING_SAFETY_DEFAULT,
            .user_control_mode = SETTING_NAV_USER_CONTROL_MODE_DEFAULT,
            .rth_alt_control_mode = SETTING_NAV_RTH_ALT_MODE_DEFAULT,
            .rth_climb_first = SETTING_NAV_RTH_CLIMB_FIRST_DEFAULT,                         // Climb first, turn after reaching safe altitude
            .rth_climb_first_stage_mode = SETTING_NAV_RTH_CLIMB_FIRST_STAGE_MODE_DEFAULT,   // To determine how rth_climb_first_stage_altitude is used
            .rth_climb_ignore_emerg = SETTING_NAV_RTH_CLIMB_IGNORE_EMERG_DEFAULT,           // Ignore GPS loss on initial climb
            .rth_tail_first = SETTING_NAV_RTH_TAIL_FIRST_DEFAULT,
            .disarm_on_landing = SETTING_NAV_DISARM_ON_LANDING_DEFAULT,
            .rth_allow_landing = SETTING_NAV_RTH_ALLOW_LANDING_DEFAULT,
            .rth_alt_control_override = SETTING_NAV_RTH_ALT_CONTROL_OVERRIDE_DEFAULT,       // Override RTH Altitude and Climb First using Pitch and Roll stick
            .nav_overrides_motor_stop = SETTING_NAV_OVERRIDES_MOTOR_STOP_DEFAULT,
            .safehome_usage_mode = SETTING_SAFEHOME_USAGE_MODE_DEFAULT,
            .mission_planner_reset = SETTING_NAV_MISSION_PLANNER_RESET_DEFAULT,       // Allow mode switch toggle to reset Mission Planner WPs
            .waypoint_mission_restart = SETTING_NAV_WP_MISSION_RESTART_DEFAULT,       // WP mission restart action
            .soaring_motor_stop = SETTING_NAV_FW_SOARING_MOTOR_STOP_DEFAULT,          // stops motor when Saoring mode enabled
            .rth_trackback_mode = SETTING_NAV_RTH_TRACKBACK_MODE_DEFAULT              // RTH trackback useage mode
        },

        // General navigation parameters
        .pos_failure_timeout = SETTING_NAV_POSITION_TIMEOUT_DEFAULT,                            // 5 sec
        .waypoint_radius = SETTING_NAV_WP_RADIUS_DEFAULT,                                       // 2m diameter
        .waypoint_safe_distance = SETTING_NAV_WP_SAFE_DISTANCE_DEFAULT,                         // centimeters - first waypoint should be closer than this
#ifdef USE_MULTI_MISSION
        .waypoint_multi_mission_index = SETTING_NAV_WP_MULTI_MISSION_INDEX_DEFAULT,             // mission index selected from multi mission WP entry
#endif
        .waypoint_load_on_boot = SETTING_NAV_WP_LOAD_ON_BOOT_DEFAULT,                           // load waypoints automatically during boot
        .auto_speed = SETTING_NAV_AUTO_SPEED_DEFAULT,                                           // speed in autonomous modes (3 m/s = 10.8 km/h)
        .max_auto_speed = SETTING_NAV_MAX_AUTO_SPEED_DEFAULT,                                   // max allowed speed autonomous modes
        .max_auto_climb_rate = SETTING_NAV_AUTO_CLIMB_RATE_DEFAULT,                             // 5 m/s
        .max_manual_speed = SETTING_NAV_MANUAL_SPEED_DEFAULT,
        .max_manual_climb_rate = SETTING_NAV_MANUAL_CLIMB_RATE_DEFAULT,
        .land_slowdown_minalt = SETTING_NAV_LAND_SLOWDOWN_MINALT_DEFAULT,                       // altitude in centimeters
        .land_slowdown_maxalt = SETTING_NAV_LAND_SLOWDOWN_MAXALT_DEFAULT,                       // altitude in meters
        .land_minalt_vspd = SETTING_NAV_LAND_MINALT_VSPD_DEFAULT,                               // centimeters/s
        .land_maxalt_vspd = SETTING_NAV_LAND_MAXALT_VSPD_DEFAULT,                               // centimeters/s
        .emerg_descent_rate = SETTING_NAV_EMERG_LANDING_SPEED_DEFAULT,                          // centimeters/s
        .min_rth_distance = SETTING_NAV_MIN_RTH_DISTANCE_DEFAULT,                               // centimeters, if closer than this land immediately
        .rth_altitude = SETTING_NAV_RTH_ALTITUDE_DEFAULT,                                       // altitude in centimeters
        .rth_home_altitude = SETTING_NAV_RTH_HOME_ALTITUDE_DEFAULT,                             // altitude in centimeters
        .rth_climb_first_stage_altitude = SETTING_NAV_RTH_CLIMB_FIRST_STAGE_ALTITUDE_DEFAULT,   // altitude in centimetres, 0= off
        .rth_abort_threshold = SETTING_NAV_RTH_ABORT_THRESHOLD_DEFAULT,                         // centimeters - 500m should be safe for all aircraft
        .max_terrain_follow_altitude = SETTING_NAV_MAX_TERRAIN_FOLLOW_ALT_DEFAULT,              // max altitude in centimeters in terrain following mode
        .safehome_max_distance = SETTING_SAFEHOME_MAX_DISTANCE_DEFAULT,                         // Max distance that a safehome is from the arming point
        .max_altitude = SETTING_NAV_MAX_ALTITUDE_DEFAULT,
        .rth_trackback_distance = SETTING_NAV_RTH_TRACKBACK_DISTANCE_DEFAULT,                   // Max distance allowed for RTH trackback
        .waypoint_enforce_altitude = SETTING_NAV_WP_ENFORCE_ALTITUDE_DEFAULT,                   // Forces set wp altitude to be achieved
        .land_detect_sensitivity = SETTING_NAV_LAND_DETECT_SENSITIVITY_DEFAULT,                 // Changes sensitivity of landing detection
        .auto_disarm_delay = SETTING_NAV_AUTO_DISARM_DELAY_DEFAULT,                             // 2000 ms - time delay to disarm when auto disarm after landing enabled
    },

    // MC-specific
    .mc = {
        .max_bank_angle = SETTING_NAV_MC_BANK_ANGLE_DEFAULT,                          // degrees

#ifdef USE_MR_BRAKING_MODE
        .braking_speed_threshold = SETTING_NAV_MC_BRAKING_SPEED_THRESHOLD_DEFAULT,               // Braking can become active above 1m/s
        .braking_disengage_speed = SETTING_NAV_MC_BRAKING_DISENGAGE_SPEED_DEFAULT,               // Stop when speed goes below 0.75m/s
        .braking_timeout = SETTING_NAV_MC_BRAKING_TIMEOUT_DEFAULT,                               // Timeout barking after 2s
        .braking_boost_factor = SETTING_NAV_MC_BRAKING_BOOST_FACTOR_DEFAULT,                     // A 100% boost by default
        .braking_boost_timeout = SETTING_NAV_MC_BRAKING_BOOST_TIMEOUT_DEFAULT,                   // Timout boost after 750ms
        .braking_boost_speed_threshold = SETTING_NAV_MC_BRAKING_BOOST_SPEED_THRESHOLD_DEFAULT,   // Boost can happen only above 1.5m/s
        .braking_boost_disengage_speed = SETTING_NAV_MC_BRAKING_BOOST_DISENGAGE_SPEED_DEFAULT,   // Disable boost at 1m/s
        .braking_bank_angle = SETTING_NAV_MC_BRAKING_BANK_ANGLE_DEFAULT,                        // Max braking angle
#endif

        .posDecelerationTime = SETTING_NAV_MC_POS_DECELERATION_TIME_DEFAULT,          // posDecelerationTime * 100
        .posResponseExpo = SETTING_NAV_MC_POS_EXPO_DEFAULT,                           // posResponseExpo * 100
        .slowDownForTurning = SETTING_NAV_MC_WP_SLOWDOWN_DEFAULT,
    },

    // Fixed wing
    .fw = {
        .max_bank_angle = SETTING_NAV_FW_BANK_ANGLE_DEFAULT,                    // degrees
        .max_climb_angle = SETTING_NAV_FW_CLIMB_ANGLE_DEFAULT,                  // degrees
        .max_dive_angle = SETTING_NAV_FW_DIVE_ANGLE_DEFAULT,                    // degrees
        .cruise_speed = SETTING_NAV_FW_CRUISE_SPEED_DEFAULT,                    // cm/s
        .control_smoothness = SETTING_NAV_FW_CONTROL_SMOOTHNESS_DEFAULT,
        .pitch_to_throttle_smooth = SETTING_NAV_FW_PITCH2THR_SMOOTHING_DEFAULT,
        .pitch_to_throttle_thresh = SETTING_NAV_FW_PITCH2THR_THRESHOLD_DEFAULT,
        .loiter_radius = SETTING_NAV_FW_LOITER_RADIUS_DEFAULT,                  // 75m

        //Fixed wing landing
        .land_dive_angle = SETTING_NAV_FW_LAND_DIVE_ANGLE_DEFAULT,              // 2 degrees dive by default

        // Fixed wing launch
        .launch_velocity_thresh = SETTING_NAV_FW_LAUNCH_VELOCITY_DEFAULT,       // 3 m/s
        .launch_accel_thresh = SETTING_NAV_FW_LAUNCH_ACCEL_DEFAULT,             // cm/s/s (1.9*G)
        .launch_time_thresh = SETTING_NAV_FW_LAUNCH_DETECT_TIME_DEFAULT,        // 40ms
        .launch_motor_timer = SETTING_NAV_FW_LAUNCH_MOTOR_DELAY_DEFAULT,        // ms
        .launch_idle_motor_timer = SETTING_NAV_FW_LAUNCH_IDLE_MOTOR_DELAY_DEFAULT,   // ms
        .launch_motor_spinup_time = SETTING_NAV_FW_LAUNCH_SPINUP_TIME_DEFAULT,  // ms, time to gredually increase throttle from idle to launch
        .launch_end_time = SETTING_NAV_FW_LAUNCH_END_TIME_DEFAULT,              // ms, time to gradually decrease/increase throttle and decrease pitch angle from launch to the current flight mode
        .launch_min_time = SETTING_NAV_FW_LAUNCH_MIN_TIME_DEFAULT,              // ms, min time in launch mode
        .launch_timeout = SETTING_NAV_FW_LAUNCH_TIMEOUT_DEFAULT,                // ms, timeout for launch procedure
        .launch_max_altitude = SETTING_NAV_FW_LAUNCH_MAX_ALTITUDE_DEFAULT,      // cm, altitude where to consider launch ended
        .launch_climb_angle = SETTING_NAV_FW_LAUNCH_CLIMB_ANGLE_DEFAULT,        // 18 degrees
        .launch_max_angle = SETTING_NAV_FW_LAUNCH_MAX_ANGLE_DEFAULT,            // 45 deg
        .launch_manual_throttle = SETTING_NAV_FW_LAUNCH_MANUAL_THROTTLE_DEFAULT,// OFF
        .launch_abort_deadband = SETTING_NAV_FW_LAUNCH_ABORT_DEADBAND_DEFAULT,  // 100 us

        .cruise_yaw_rate  = SETTING_NAV_FW_CRUISE_YAW_RATE_DEFAULT,             // 20dps
        .allow_manual_thr_increase = SETTING_NAV_FW_ALLOW_MANUAL_THR_INCREASE_DEFAULT,
        .useFwNavYawControl = SETTING_NAV_USE_FW_YAW_CONTROL_DEFAULT,
        .yawControlDeadband = SETTING_NAV_FW_YAW_DEADBAND_DEFAULT,
        .soaring_pitch_deadband = SETTING_NAV_FW_SOARING_PITCH_DEADBAND_DEFAULT,// pitch angle mode deadband when Saoring mode enabled
        .wp_tracking_accuracy = SETTING_NAV_FW_WP_TRACKING_ACCURACY_DEFAULT,    // 0, improves course tracking accuracy during FW WP missions
        .wp_tracking_max_angle = SETTING_NAV_FW_WP_TRACKING_MAX_ANGLE_DEFAULT,  // 60 degs
        .wp_turn_smoothing = SETTING_NAV_FW_WP_TURN_SMOOTHING_DEFAULT,          // 0, smooths turns during FW WP mode missions
    }
);

/* NAV variables */
static navWapointHeading_t wpHeadingControl;
navigationPosControl_t posControl;
navSystemStatus_t NAV_Status;

EXTENDED_FASTRAM multicopterPosXyCoefficients_t multicopterPosXyCoefficients;

// Blackbox states
int16_t navCurrentState;
int16_t navActualVelocity[3];
int16_t navDesiredVelocity[3];
int16_t navActualHeading;
int16_t navDesiredHeading;
int32_t navTargetPosition[3];
int32_t navLatestActualPosition[3];
int16_t navActualSurface;
uint16_t navFlags;
uint16_t navEPH;
uint16_t navEPV;
int16_t navAccNEU[3];
//End of blackbox states

static fpVector3_t * rthGetHomeTargetPosition(rthTargetMode_e mode);
static void updateDesiredRTHAltitude(void);
static void resetAltitudeController(bool useTerrainFollowing);
static void resetPositionController(void);
static void setupAltitudeController(void);
static void resetHeadingController(void);
void resetGCSFlags(void);

static void setupJumpCounters(void);
static void resetJumpCounter(void);
static void clearJumpCounters(void);

static void calculateAndSetActiveWaypoint(const navWaypoint_t * waypoint);
static void calculateAndSetActiveWaypointToLocalPosition(const fpVector3_t * pos);
void calculateInitialHoldPosition(fpVector3_t * pos);
void calculateFarAwayTarget(fpVector3_t * farAwayPos, int32_t yaw, int32_t distance);
void calculateNewCruiseTarget(fpVector3_t * origin, int32_t yaw, int32_t distance);
static bool isWaypointReached(const fpVector3_t * waypointPos, const int32_t * waypointYaw);
bool isWaypointAltitudeReached(void);
static void mapWaypointToLocalPosition(fpVector3_t * localPos, const navWaypoint_t * waypoint, geoAltitudeConversionMode_e altConv);
static navigationFSMEvent_t nextForNonGeoStates(void);
static bool isWaypointMissionValid(void);
void missionPlannerSetWaypoint(void);

void initializeRTHSanityChecker(void);
bool validateRTHSanityChecker(void);
void updateHomePosition(void);
bool abortLaunchAllowed(void);

static bool rthAltControlStickOverrideCheck(unsigned axis);

static void updateRthTrackback(bool forceSaveTrackPoint);
static fpVector3_t * rthGetTrackbackPos(void);

/*************************************************************************************************/
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_IDLE(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_ALTHOLD_INITIALIZE(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_ALTHOLD_IN_PROGRESS(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_POSHOLD_3D_INITIALIZE(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_POSHOLD_3D_IN_PROGRESS(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_COURSE_HOLD_INITIALIZE(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_COURSE_HOLD_IN_PROGRESS(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_COURSE_HOLD_ADJUSTING(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_CRUISE_INITIALIZE(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_CRUISE_IN_PROGRESS(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_CRUISE_ADJUSTING(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_INITIALIZE(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_CLIMB_TO_SAFE_ALT(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_TRACKBACK(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_HEAD_HOME(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_HOVER_PRIOR_TO_LANDING(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_HOVER_ABOVE_HOME(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_LANDING(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_FINISHING(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_FINISHED(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_INITIALIZE(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_PRE_ACTION(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_IN_PROGRESS(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_REACHED(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_HOLD_TIME(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_NEXT(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_FINISHED(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_RTH_LAND(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_EMERGENCY_LANDING_INITIALIZE(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_EMERGENCY_LANDING_IN_PROGRESS(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_EMERGENCY_LANDING_FINISHED(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_LAUNCH_INITIALIZE(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_LAUNCH_WAIT(navigationFSMState_t previousState);
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_LAUNCH_IN_PROGRESS(navigationFSMState_t previousState);

static const navigationFSMStateDescriptor_t navFSM[NAV_STATE_COUNT] = {
    /** Idle state ******************************************************/
    [NAV_STATE_IDLE] = {
        .persistentId = NAV_PERSISTENT_ID_IDLE,
        .onEntry = navOnEnteringState_NAV_STATE_IDLE,
        .timeoutMs = 0,
        .stateFlags = 0,
        .mapToFlightModes = 0,
        .mwState = MW_NAV_STATE_NONE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent    = {
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]                  = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT]             = NAV_STATE_WAYPOINT_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_LAUNCH]               = NAV_STATE_LAUNCH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]          = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]               = NAV_STATE_CRUISE_INITIALIZE,
        }
    },

    /** ALTHOLD mode ***************************************************/
    [NAV_STATE_ALTHOLD_INITIALIZE] = {
        .persistentId = NAV_PERSISTENT_ID_ALTHOLD_INITIALIZE,
        .onEntry = navOnEnteringState_NAV_STATE_ALTHOLD_INITIALIZE,
        .timeoutMs = 0,
        .stateFlags = NAV_CTL_ALT | NAV_REQUIRE_ANGLE_FW | NAV_REQUIRE_THRTILT,
        .mapToFlightModes = NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_NONE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_ALTHOLD_IN_PROGRESS,
            [NAV_FSM_EVENT_ERROR]                          = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
        }
    },

    [NAV_STATE_ALTHOLD_IN_PROGRESS] = {
        .persistentId = NAV_PERSISTENT_ID_ALTHOLD_IN_PROGRESS,
        .onEntry = navOnEnteringState_NAV_STATE_ALTHOLD_IN_PROGRESS,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_REQUIRE_ANGLE_FW | NAV_REQUIRE_THRTILT | NAV_RC_ALT,
        .mapToFlightModes = NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_NONE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_ALTHOLD_IN_PROGRESS,    // re-process the state
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]                  = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT]             = NAV_STATE_WAYPOINT_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]          = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]               = NAV_STATE_CRUISE_INITIALIZE,
        }
    },

    /** POSHOLD_3D mode ************************************************/
    [NAV_STATE_POSHOLD_3D_INITIALIZE] = {
        .persistentId = NAV_PERSISTENT_ID_POSHOLD_3D_INITIALIZE,
        .onEntry = navOnEnteringState_NAV_STATE_POSHOLD_3D_INITIALIZE,
        .timeoutMs = 0,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_REQUIRE_ANGLE | NAV_REQUIRE_THRTILT,
        .mapToFlightModes = NAV_ALTHOLD_MODE | NAV_POSHOLD_MODE,
        .mwState = MW_NAV_STATE_HOLD_INFINIT,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_POSHOLD_3D_IN_PROGRESS,
            [NAV_FSM_EVENT_ERROR]                          = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
        }
    },

    [NAV_STATE_POSHOLD_3D_IN_PROGRESS] = {
        .persistentId = NAV_PERSISTENT_ID_POSHOLD_3D_IN_PROGRESS,
        .onEntry = navOnEnteringState_NAV_STATE_POSHOLD_3D_IN_PROGRESS,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_REQUIRE_THRTILT | NAV_RC_ALT | NAV_RC_POS | NAV_RC_YAW,
        .mapToFlightModes = NAV_ALTHOLD_MODE | NAV_POSHOLD_MODE,
        .mwState = MW_NAV_STATE_HOLD_INFINIT,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_POSHOLD_3D_IN_PROGRESS,    // re-process the state
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]                  = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT]             = NAV_STATE_WAYPOINT_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]          = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]               = NAV_STATE_CRUISE_INITIALIZE,
        }
    },
    /** CRUISE_HOLD mode ************************************************/
    [NAV_STATE_COURSE_HOLD_INITIALIZE] = {
        .persistentId = NAV_PERSISTENT_ID_COURSE_HOLD_INITIALIZE,
        .onEntry = navOnEnteringState_NAV_STATE_COURSE_HOLD_INITIALIZE,
        .timeoutMs = 0,
        .stateFlags = NAV_REQUIRE_ANGLE,
        .mapToFlightModes = NAV_COURSE_HOLD_MODE,
        .mwState = MW_NAV_STATE_NONE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_COURSE_HOLD_IN_PROGRESS,
            [NAV_FSM_EVENT_ERROR]                          = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
        }
    },

    [NAV_STATE_COURSE_HOLD_IN_PROGRESS] = {
        .persistentId = NAV_PERSISTENT_ID_COURSE_HOLD_IN_PROGRESS,
        .onEntry = navOnEnteringState_NAV_STATE_COURSE_HOLD_IN_PROGRESS,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_RC_POS | NAV_RC_YAW,
        .mapToFlightModes = NAV_COURSE_HOLD_MODE,
        .mwState = MW_NAV_STATE_NONE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_COURSE_HOLD_IN_PROGRESS,    // re-process the state
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]               = NAV_STATE_CRUISE_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_ADJ]           = NAV_STATE_COURSE_HOLD_ADJUSTING,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]                  = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT]             = NAV_STATE_WAYPOINT_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
        }
    },

        [NAV_STATE_COURSE_HOLD_ADJUSTING] = {
        .persistentId = NAV_PERSISTENT_ID_COURSE_HOLD_ADJUSTING,
        .onEntry = navOnEnteringState_NAV_STATE_COURSE_HOLD_ADJUSTING,
        .timeoutMs = 10,
        .stateFlags =  NAV_REQUIRE_ANGLE | NAV_RC_POS,
        .mapToFlightModes = NAV_COURSE_HOLD_MODE,
        .mwState = MW_NAV_STATE_NONE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_COURSE_HOLD_IN_PROGRESS,
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_COURSE_HOLD_ADJUSTING,
            [NAV_FSM_EVENT_ERROR]                          = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]               = NAV_STATE_CRUISE_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]                  = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT]             = NAV_STATE_WAYPOINT_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
        }
    },

    /** CRUISE_3D mode ************************************************/
    [NAV_STATE_CRUISE_INITIALIZE] = {
        .persistentId = NAV_PERSISTENT_ID_CRUISE_INITIALIZE,
        .onEntry = navOnEnteringState_NAV_STATE_CRUISE_INITIALIZE,
        .timeoutMs = 0,
        .stateFlags = NAV_REQUIRE_ANGLE,
        .mapToFlightModes = NAV_ALTHOLD_MODE | NAV_COURSE_HOLD_MODE,
        .mwState = MW_NAV_STATE_NONE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_CRUISE_IN_PROGRESS,
            [NAV_FSM_EVENT_ERROR]                          = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
        }
    },

    [NAV_STATE_CRUISE_IN_PROGRESS] = {
        .persistentId = NAV_PERSISTENT_ID_CRUISE_IN_PROGRESS,
        .onEntry = navOnEnteringState_NAV_STATE_CRUISE_IN_PROGRESS,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_POS | NAV_CTL_YAW | NAV_CTL_ALT | NAV_REQUIRE_ANGLE | NAV_RC_POS | NAV_RC_YAW | NAV_RC_ALT,
        .mapToFlightModes = NAV_ALTHOLD_MODE | NAV_COURSE_HOLD_MODE,
        .mwState = MW_NAV_STATE_NONE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_CRUISE_IN_PROGRESS,    // re-process the state
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]          = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_ADJ]           = NAV_STATE_CRUISE_ADJUSTING,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]                  = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT]             = NAV_STATE_WAYPOINT_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
        }
    },

    [NAV_STATE_CRUISE_ADJUSTING] = {
        .persistentId = NAV_PERSISTENT_ID_CRUISE_ADJUSTING,
        .onEntry = navOnEnteringState_NAV_STATE_CRUISE_ADJUSTING,
        .timeoutMs = 10,
        .stateFlags =  NAV_CTL_ALT | NAV_REQUIRE_ANGLE | NAV_RC_POS | NAV_RC_ALT,
        .mapToFlightModes = NAV_ALTHOLD_MODE | NAV_COURSE_HOLD_MODE,
        .mwState = MW_NAV_STATE_NONE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_CRUISE_IN_PROGRESS,
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_CRUISE_ADJUSTING,
            [NAV_FSM_EVENT_ERROR]                          = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]          = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]                  = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT]             = NAV_STATE_WAYPOINT_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
        }
    },

    /** RTH_3D mode ************************************************/
    [NAV_STATE_RTH_INITIALIZE] = {
        .persistentId = NAV_PERSISTENT_ID_RTH_INITIALIZE,
        .onEntry = navOnEnteringState_NAV_STATE_RTH_INITIALIZE,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_RTH,
        .mapToFlightModes = NAV_RTH_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_RTH_START,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                             = NAV_STATE_RTH_INITIALIZE,      // re-process the state
            [NAV_FSM_EVENT_SUCCESS]                             = NAV_STATE_RTH_CLIMB_TO_SAFE_ALT,
            [NAV_FSM_EVENT_SWITCH_TO_NAV_STATE_RTH_TRACKBACK]   = NAV_STATE_RTH_TRACKBACK,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]         = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_RTH_LANDING]               = NAV_STATE_RTH_HOVER_PRIOR_TO_LANDING,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                      = NAV_STATE_IDLE,
        }
    },

    [NAV_STATE_RTH_CLIMB_TO_SAFE_ALT] = {
        .persistentId = NAV_PERSISTENT_ID_RTH_CLIMB_TO_SAFE_ALT,
        .onEntry = navOnEnteringState_NAV_STATE_RTH_CLIMB_TO_SAFE_ALT,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_RTH | NAV_RC_POS | NAV_RC_YAW,     // allow pos adjustment while climbind to safe alt
        .mapToFlightModes = NAV_RTH_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_RTH_CLIMB,
        .mwError = MW_NAV_ERROR_WAIT_FOR_RTH_ALT,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_RTH_CLIMB_TO_SAFE_ALT,   // re-process the state
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_RTH_HEAD_HOME,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]          = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]               = NAV_STATE_CRUISE_INITIALIZE,
        }
    },

    [NAV_STATE_RTH_TRACKBACK] = {
        .persistentId = NAV_PERSISTENT_ID_RTH_TRACKBACK,
        .onEntry = navOnEnteringState_NAV_STATE_RTH_TRACKBACK,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_RTH,
        .mapToFlightModes = NAV_RTH_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_RTH_ENROUTE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                             = NAV_STATE_RTH_TRACKBACK,           // re-process the state
            [NAV_FSM_EVENT_SWITCH_TO_NAV_STATE_RTH_INITIALIZE]  = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]         = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                      = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]                   = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]                = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]               = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]                    = NAV_STATE_CRUISE_INITIALIZE,
        }
    },

    [NAV_STATE_RTH_HEAD_HOME] = {
        .persistentId = NAV_PERSISTENT_ID_RTH_HEAD_HOME,
        .onEntry = navOnEnteringState_NAV_STATE_RTH_HEAD_HOME,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_RTH | NAV_RC_POS | NAV_RC_YAW,
        .mapToFlightModes = NAV_RTH_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_RTH_ENROUTE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_RTH_HEAD_HOME,           // re-process the state
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_RTH_HOVER_PRIOR_TO_LANDING,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]          = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]               = NAV_STATE_CRUISE_INITIALIZE,
        }
    },

    [NAV_STATE_RTH_HOVER_PRIOR_TO_LANDING] = {
        .persistentId = NAV_PERSISTENT_ID_RTH_HOVER_PRIOR_TO_LANDING,
        .onEntry = navOnEnteringState_NAV_STATE_RTH_HOVER_PRIOR_TO_LANDING,
        .timeoutMs = 500,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_RTH | NAV_RC_POS | NAV_RC_YAW,
        .mapToFlightModes = NAV_RTH_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_LAND_SETTLE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_RTH_HOVER_PRIOR_TO_LANDING,
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_RTH_LANDING,
            [NAV_FSM_EVENT_SWITCH_TO_RTH_HOVER_ABOVE_HOME] = NAV_STATE_RTH_HOVER_ABOVE_HOME,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]          = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]               = NAV_STATE_CRUISE_INITIALIZE,
        }
    },

    [NAV_STATE_RTH_HOVER_ABOVE_HOME] = {
        .persistentId = NAV_PERSISTENT_ID_RTH_HOVER_ABOVE_HOME,
        .onEntry = navOnEnteringState_NAV_STATE_RTH_HOVER_ABOVE_HOME,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_RTH | NAV_RC_POS | NAV_RC_YAW | NAV_RC_ALT,
        .mapToFlightModes = NAV_RTH_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_HOVER_ABOVE_HOME,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_RTH_HOVER_ABOVE_HOME,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]          = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]               = NAV_STATE_CRUISE_INITIALIZE,
        }
    },

    [NAV_STATE_RTH_LANDING] = {
        .persistentId = NAV_PERSISTENT_ID_RTH_LANDING,
        .onEntry = navOnEnteringState_NAV_STATE_RTH_LANDING,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_CTL_LAND | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_RTH | NAV_RC_POS | NAV_RC_YAW,
        .mapToFlightModes = NAV_RTH_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_LAND_IN_PROGRESS,
        .mwError = MW_NAV_ERROR_LANDING,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_RTH_LANDING,         // re-process state
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_RTH_FINISHING,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
        }
    },

    [NAV_STATE_RTH_FINISHING] = {
        .persistentId = NAV_PERSISTENT_ID_RTH_FINISHING,
        .onEntry = navOnEnteringState_NAV_STATE_RTH_FINISHING,
        .timeoutMs = 0,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_CTL_LAND | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_RTH,
        .mapToFlightModes = NAV_RTH_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_LAND_IN_PROGRESS,
        .mwError = MW_NAV_ERROR_LANDING,
        .onEvent = {
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_RTH_FINISHED,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
        }
    },

    [NAV_STATE_RTH_FINISHED] = {
        .persistentId = NAV_PERSISTENT_ID_RTH_FINISHED,
        .onEntry = navOnEnteringState_NAV_STATE_RTH_FINISHED,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_LAND | NAV_REQUIRE_ANGLE | NAV_REQUIRE_THRTILT | NAV_AUTO_RTH,
        .mapToFlightModes = NAV_RTH_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_LANDED,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_RTH_FINISHED,         // re-process state
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
        }
    },

    /** WAYPOINT mode ************************************************/
    [NAV_STATE_WAYPOINT_INITIALIZE] = {
        .persistentId = NAV_PERSISTENT_ID_WAYPOINT_INITIALIZE,
        .onEntry = navOnEnteringState_NAV_STATE_WAYPOINT_INITIALIZE,
        .timeoutMs = 0,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_WP,
        .mapToFlightModes = NAV_WP_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_PROCESS_NEXT,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_WAYPOINT_PRE_ACTION,
            [NAV_FSM_EVENT_ERROR]                          = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_FINISHED]    = NAV_STATE_WAYPOINT_FINISHED,
        }
    },

    [NAV_STATE_WAYPOINT_PRE_ACTION] = {
        .persistentId = NAV_PERSISTENT_ID_WAYPOINT_PRE_ACTION,
        .onEntry = navOnEnteringState_NAV_STATE_WAYPOINT_PRE_ACTION,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_WP,
        .mapToFlightModes = NAV_WP_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_PROCESS_NEXT,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                     = NAV_STATE_WAYPOINT_PRE_ACTION,   // re-process the state (for JUMP)
            [NAV_FSM_EVENT_SUCCESS]                     = NAV_STATE_WAYPOINT_IN_PROGRESS,
            [NAV_FSM_EVENT_ERROR]                       = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]              = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]               = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_FINISHED] = NAV_STATE_WAYPOINT_FINISHED,
        }
    },

    [NAV_STATE_WAYPOINT_IN_PROGRESS] = {
        .persistentId = NAV_PERSISTENT_ID_WAYPOINT_IN_PROGRESS,
        .onEntry = navOnEnteringState_NAV_STATE_WAYPOINT_IN_PROGRESS,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_WP,
        .mapToFlightModes = NAV_WP_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_WP_ENROUTE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_WAYPOINT_IN_PROGRESS,   // re-process the state
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_WAYPOINT_REACHED,       // successfully reached waypoint
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]                  = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]          = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]               = NAV_STATE_CRUISE_INITIALIZE,
        }
    },

    [NAV_STATE_WAYPOINT_REACHED] = {
        .persistentId = NAV_PERSISTENT_ID_WAYPOINT_REACHED,
        .onEntry = navOnEnteringState_NAV_STATE_WAYPOINT_REACHED,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_WP,
        .mapToFlightModes = NAV_WP_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_PROCESS_NEXT,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                     = NAV_STATE_WAYPOINT_REACHED,   // re-process state
            [NAV_FSM_EVENT_SUCCESS]                     = NAV_STATE_WAYPOINT_NEXT,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_HOLD_TIME] = NAV_STATE_WAYPOINT_HOLD_TIME,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_FINISHED] = NAV_STATE_WAYPOINT_FINISHED,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_RTH_LAND] = NAV_STATE_WAYPOINT_RTH_LAND,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]              = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]           = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]        = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]               = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING] = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]       = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]            = NAV_STATE_CRUISE_INITIALIZE,
        }
    },

    [NAV_STATE_WAYPOINT_HOLD_TIME] = {
        .persistentId = NAV_PERSISTENT_ID_WAYPOINT_HOLD_TIME,                             // There is no state for timed hold?
        .onEntry = navOnEnteringState_NAV_STATE_WAYPOINT_HOLD_TIME,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_WP,
        .mapToFlightModes = NAV_WP_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_HOLD_TIMED,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_WAYPOINT_HOLD_TIME,     // re-process the state
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_WAYPOINT_NEXT,          // successfully reached waypoint
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]                  = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]          = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]               = NAV_STATE_CRUISE_INITIALIZE,
        }
    },

    [NAV_STATE_WAYPOINT_RTH_LAND] = {
        .persistentId = NAV_PERSISTENT_ID_WAYPOINT_RTH_LAND,
        .onEntry = navOnEnteringState_NAV_STATE_WAYPOINT_RTH_LAND,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_CTL_LAND | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_WP,
        .mapToFlightModes = NAV_WP_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_LAND_IN_PROGRESS,
        .mwError = MW_NAV_ERROR_LANDING,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_WAYPOINT_RTH_LAND,   // re-process state
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_WAYPOINT_FINISHED,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]                  = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]          = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]               = NAV_STATE_CRUISE_INITIALIZE,
        }
    },

    [NAV_STATE_WAYPOINT_NEXT] = {
        .persistentId = NAV_PERSISTENT_ID_WAYPOINT_NEXT,
        .onEntry = navOnEnteringState_NAV_STATE_WAYPOINT_NEXT,
        .timeoutMs = 0,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_WP,
        .mapToFlightModes = NAV_WP_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_PROCESS_NEXT,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_WAYPOINT_PRE_ACTION,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_FINISHED]    = NAV_STATE_WAYPOINT_FINISHED,
        }
    },

    [NAV_STATE_WAYPOINT_FINISHED] = {
        .persistentId = NAV_PERSISTENT_ID_WAYPOINT_FINISHED,
        .onEntry = navOnEnteringState_NAV_STATE_WAYPOINT_FINISHED,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_ALT | NAV_CTL_POS | NAV_CTL_YAW | NAV_REQUIRE_ANGLE | NAV_REQUIRE_MAGHOLD | NAV_REQUIRE_THRTILT | NAV_AUTO_WP | NAV_AUTO_WP_DONE,
        .mapToFlightModes = NAV_WP_MODE | NAV_ALTHOLD_MODE,
        .mwState = MW_NAV_STATE_WP_ENROUTE,
        .mwError = MW_NAV_ERROR_FINISH,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_WAYPOINT_FINISHED,   // re-process state
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D]           = NAV_STATE_POSHOLD_3D_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]                  = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING]    = NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD]          = NAV_STATE_COURSE_HOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_CRUISE]               = NAV_STATE_CRUISE_INITIALIZE,
        }
    },

    /** EMERGENCY LANDING ************************************************/
    [NAV_STATE_EMERGENCY_LANDING_INITIALIZE] = {
        .persistentId = NAV_PERSISTENT_ID_EMERGENCY_LANDING_INITIALIZE,
        .onEntry = navOnEnteringState_NAV_STATE_EMERGENCY_LANDING_INITIALIZE,
        .timeoutMs = 0,
        .stateFlags = NAV_CTL_EMERG | NAV_REQUIRE_ANGLE,
        .mapToFlightModes = 0,
        .mwState = MW_NAV_STATE_EMERGENCY_LANDING,
        .mwError = MW_NAV_ERROR_LANDING,
        .onEvent = {
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_EMERGENCY_LANDING_IN_PROGRESS,
            [NAV_FSM_EVENT_ERROR]                          = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]                  = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT]             = NAV_STATE_WAYPOINT_INITIALIZE,
        }
    },

    [NAV_STATE_EMERGENCY_LANDING_IN_PROGRESS] = {
        .persistentId = NAV_PERSISTENT_ID_EMERGENCY_LANDING_IN_PROGRESS,
        .onEntry = navOnEnteringState_NAV_STATE_EMERGENCY_LANDING_IN_PROGRESS,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_EMERG | NAV_REQUIRE_ANGLE,
        .mapToFlightModes = 0,
        .mwState = MW_NAV_STATE_EMERGENCY_LANDING,
        .mwError = MW_NAV_ERROR_LANDING,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_EMERGENCY_LANDING_IN_PROGRESS,    // re-process the state
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_EMERGENCY_LANDING_FINISHED,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_ALTHOLD]              = NAV_STATE_ALTHOLD_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_RTH]                  = NAV_STATE_RTH_INITIALIZE,
            [NAV_FSM_EVENT_SWITCH_TO_WAYPOINT]             = NAV_STATE_WAYPOINT_INITIALIZE,
        }
    },

    [NAV_STATE_EMERGENCY_LANDING_FINISHED] = {
        .persistentId = NAV_PERSISTENT_ID_EMERGENCY_LANDING_FINISHED,
        .onEntry = navOnEnteringState_NAV_STATE_EMERGENCY_LANDING_FINISHED,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_EMERG | NAV_REQUIRE_ANGLE,
        .mapToFlightModes = 0,
        .mwState = MW_NAV_STATE_LANDED,
        .mwError = MW_NAV_ERROR_LANDING,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_EMERGENCY_LANDING_FINISHED,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
        }
    },

    [NAV_STATE_LAUNCH_INITIALIZE] = {
        .persistentId = NAV_PERSISTENT_ID_LAUNCH_INITIALIZE,
        .onEntry = navOnEnteringState_NAV_STATE_LAUNCH_INITIALIZE,
        .timeoutMs = 0,
        .stateFlags = NAV_REQUIRE_ANGLE,
        .mapToFlightModes = NAV_LAUNCH_MODE,
        .mwState = MW_NAV_STATE_NONE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_LAUNCH_WAIT,
            [NAV_FSM_EVENT_ERROR]                          = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
        }
    },

    [NAV_STATE_LAUNCH_WAIT] = {
        .persistentId = NAV_PERSISTENT_ID_LAUNCH_WAIT,
        .onEntry = navOnEnteringState_NAV_STATE_LAUNCH_WAIT,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_LAUNCH | NAV_REQUIRE_ANGLE,
        .mapToFlightModes = NAV_LAUNCH_MODE,
        .mwState = MW_NAV_STATE_NONE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_LAUNCH_WAIT,    // re-process the state
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_LAUNCH_IN_PROGRESS,
            [NAV_FSM_EVENT_ERROR]                          = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
        }
    },

    [NAV_STATE_LAUNCH_IN_PROGRESS] = {
        .persistentId = NAV_PERSISTENT_ID_LAUNCH_IN_PROGRESS,
        .onEntry = navOnEnteringState_NAV_STATE_LAUNCH_IN_PROGRESS,
        .timeoutMs = 10,
        .stateFlags = NAV_CTL_LAUNCH | NAV_REQUIRE_ANGLE,
        .mapToFlightModes = NAV_LAUNCH_MODE,
        .mwState = MW_NAV_STATE_NONE,
        .mwError = MW_NAV_ERROR_NONE,
        .onEvent = {
            [NAV_FSM_EVENT_TIMEOUT]                        = NAV_STATE_LAUNCH_IN_PROGRESS,    // re-process the state
            [NAV_FSM_EVENT_SUCCESS]                        = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_ERROR]                          = NAV_STATE_IDLE,
            [NAV_FSM_EVENT_SWITCH_TO_IDLE]                 = NAV_STATE_IDLE,
        }
    },
};

static navigationFSMStateFlags_t navGetStateFlags(navigationFSMState_t state)
{
    return navFSM[state].stateFlags;
}

static flightModeFlags_e navGetMappedFlightModes(navigationFSMState_t state)
{
    return navFSM[state].mapToFlightModes;
}

navigationFSMStateFlags_t navGetCurrentStateFlags(void)
{
    return navGetStateFlags(posControl.navState);
}

static bool navTerrainFollowingRequested(void)
{
    // Terrain following not supported on FIXED WING aircraft yet
    return !STATE(FIXED_WING_LEGACY) && IS_RC_MODE_ACTIVE(BOXSURFACE);
}

/*************************************************************************************************/
static navigationFSMEvent_t navOnEnteringState_NAV_STATE_IDLE(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    resetAltitudeController(false);
    resetHeadingController();
    resetPositionController();

    return NAV_FSM_EVENT_NONE;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_ALTHOLD_INITIALIZE(navigationFSMState_t previousState)
{
    const navigationFSMStateFlags_t prevFlags = navGetStateFlags(previousState);
    const bool terrainFollowingToggled = (posControl.flags.isTerrainFollowEnabled != navTerrainFollowingRequested());

    resetGCSFlags();

    // If surface tracking mode changed value - reset altitude controller
    if ((prevFlags & NAV_CTL_ALT) == 0 || terrainFollowingToggled) {
        resetAltitudeController(navTerrainFollowingRequested());
    }

    if (((prevFlags & NAV_CTL_ALT) == 0) || ((prevFlags & NAV_AUTO_RTH) != 0) || ((prevFlags & NAV_AUTO_WP) != 0) || terrainFollowingToggled) {
        setupAltitudeController();
        setDesiredPosition(&navGetCurrentActualPositionAndVelocity()->pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);  // This will reset surface offset
    }

    return NAV_FSM_EVENT_SUCCESS;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_ALTHOLD_IN_PROGRESS(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    // If GCS was disabled - reset altitude setpoint
    if (posControl.flags.isGCSAssistedNavigationReset) {
        setDesiredPosition(&navGetCurrentActualPositionAndVelocity()->pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);
        resetGCSFlags();
    }

    return NAV_FSM_EVENT_NONE;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_POSHOLD_3D_INITIALIZE(navigationFSMState_t previousState)
{
    const navigationFSMStateFlags_t prevFlags = navGetStateFlags(previousState);
    const bool terrainFollowingToggled = (posControl.flags.isTerrainFollowEnabled != navTerrainFollowingRequested());

    resetGCSFlags();

    if ((prevFlags & NAV_CTL_POS) == 0) {
        resetPositionController();
    }

    if ((prevFlags & NAV_CTL_ALT) == 0 || terrainFollowingToggled) {
        resetAltitudeController(navTerrainFollowingRequested());
        setupAltitudeController();
    }

    if (((prevFlags & NAV_CTL_ALT) == 0) || ((prevFlags & NAV_AUTO_RTH) != 0) || ((prevFlags & NAV_AUTO_WP) != 0) || terrainFollowingToggled) {
        setDesiredPosition(&navGetCurrentActualPositionAndVelocity()->pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);  // This will reset surface offset
    }

    if ((previousState != NAV_STATE_RTH_HOVER_PRIOR_TO_LANDING) && (previousState != NAV_STATE_RTH_HOVER_ABOVE_HOME) && (previousState != NAV_STATE_RTH_LANDING)) {
        fpVector3_t targetHoldPos;
        calculateInitialHoldPosition(&targetHoldPos);
        setDesiredPosition(&targetHoldPos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
    }

    return NAV_FSM_EVENT_SUCCESS;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_POSHOLD_3D_IN_PROGRESS(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    // If GCS was disabled - reset 2D pos setpoint
    if (posControl.flags.isGCSAssistedNavigationReset) {
        fpVector3_t targetHoldPos;
        calculateInitialHoldPosition(&targetHoldPos);
        setDesiredPosition(&navGetCurrentActualPositionAndVelocity()->pos, posControl.actualState.yaw, NAV_POS_UPDATE_Z);
        setDesiredPosition(&targetHoldPos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);
        resetGCSFlags();
    }

    return NAV_FSM_EVENT_NONE;
}
/////////////////

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_COURSE_HOLD_INITIALIZE(navigationFSMState_t previousState)
{
    const navigationFSMStateFlags_t prevFlags = navGetStateFlags(previousState);

    if (!STATE(FIXED_WING_LEGACY)) { return NAV_FSM_EVENT_ERROR; } // Only on FW for now

    DEBUG_SET(DEBUG_CRUISE, 0, 1);
    if (checkForPositionSensorTimeout()) {
        return NAV_FSM_EVENT_SWITCH_TO_IDLE;
    }  // Switch to IDLE if we do not have an healty position. Try the next iteration.

    if (!(prevFlags & NAV_CTL_POS)) {
        resetPositionController();
    }

    posControl.cruise.yaw = posControl.actualState.yaw; // Store the yaw to follow
    posControl.cruise.previousYaw = posControl.cruise.yaw;
    posControl.cruise.lastYawAdjustmentTime = 0;

    return NAV_FSM_EVENT_SUCCESS; // Go to CRUISE_XD_IN_PROGRESS state
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_COURSE_HOLD_IN_PROGRESS(navigationFSMState_t previousState)
{
    const timeMs_t currentTimeMs = millis();

    if (checkForPositionSensorTimeout()) {
        return NAV_FSM_EVENT_SWITCH_TO_IDLE;
    } // Switch to IDLE if we do not have an healty position. Do the CRUISE init the next iteration.

    DEBUG_SET(DEBUG_CRUISE, 0, 2);
    DEBUG_SET(DEBUG_CRUISE, 2, 0);

    if (posControl.flags.isAdjustingPosition) {
        return NAV_FSM_EVENT_SWITCH_TO_COURSE_ADJ;
    }

    // User is yawing. We record the desidered yaw and we change the desidered target in the meanwhile
    if (posControl.flags.isAdjustingHeading) {
        timeMs_t timeDifference = currentTimeMs - posControl.cruise.lastYawAdjustmentTime;
        if (timeDifference > 100) timeDifference = 0; // if adjustment was called long time ago, reset the time difference.
        float rateTarget = scaleRangef((float)rcCommand[YAW], -500.0f, 500.0f, -DEGREES_TO_CENTIDEGREES(navConfig()->fw.cruise_yaw_rate), DEGREES_TO_CENTIDEGREES(navConfig()->fw.cruise_yaw_rate));
        float centidegsPerIteration = rateTarget * timeDifference * 0.001f;
        posControl.cruise.yaw = wrap_36000(posControl.cruise.yaw - centidegsPerIteration);
        DEBUG_SET(DEBUG_CRUISE, 1, CENTIDEGREES_TO_DEGREES(posControl.cruise.yaw));
        posControl.cruise.lastYawAdjustmentTime = currentTimeMs;
    }

    if (currentTimeMs - posControl.cruise.lastYawAdjustmentTime > 4000)
        posControl.cruise.previousYaw = posControl.cruise.yaw;

    uint32_t distance = gpsSol.groundSpeed * 60; // next WP to be reached in 60s [cm]

    if ((previousState == NAV_STATE_COURSE_HOLD_INITIALIZE) || (previousState == NAV_STATE_COURSE_HOLD_ADJUSTING)
            || (previousState == NAV_STATE_CRUISE_INITIALIZE) || (previousState == NAV_STATE_CRUISE_ADJUSTING)
            || posControl.flags.isAdjustingHeading) {
        calculateFarAwayTarget(&posControl.cruise.targetPos, posControl.cruise.yaw, distance);
        DEBUG_SET(DEBUG_CRUISE, 2, 1);
    } else if (calculateDistanceToDestination(&posControl.cruise.targetPos) <= (navConfig()->fw.loiter_radius * 1.10f)) { //10% margin
        calculateNewCruiseTarget(&posControl.cruise.targetPos, posControl.cruise.yaw, distance);
        DEBUG_SET(DEBUG_CRUISE, 2, 2);
    }

    setDesiredPosition(&posControl.cruise.targetPos, posControl.cruise.yaw, NAV_POS_UPDATE_XY);

    return NAV_FSM_EVENT_NONE;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_COURSE_HOLD_ADJUSTING(navigationFSMState_t previousState)
{
    UNUSED(previousState);
    DEBUG_SET(DEBUG_CRUISE, 0, 3);

    // User is rolling, changing manually direction. Wait until it is done and then restore CRUISE
    if (posControl.flags.isAdjustingPosition) {
        posControl.cruise.yaw = posControl.actualState.yaw; //store current heading
        posControl.cruise.lastYawAdjustmentTime = millis();
        return NAV_FSM_EVENT_NONE;  // reprocess the state
    }

    resetPositionController();

    return NAV_FSM_EVENT_SUCCESS; // go back to the CRUISE_XD_IN_PROGRESS state
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_CRUISE_INITIALIZE(navigationFSMState_t previousState)
{
    if (!STATE(FIXED_WING_LEGACY)) { return NAV_FSM_EVENT_ERROR; } // only on FW for now

    navOnEnteringState_NAV_STATE_ALTHOLD_INITIALIZE(previousState);

    return navOnEnteringState_NAV_STATE_COURSE_HOLD_INITIALIZE(previousState);
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_CRUISE_IN_PROGRESS(navigationFSMState_t previousState)
{
    navOnEnteringState_NAV_STATE_ALTHOLD_IN_PROGRESS(previousState);

    return navOnEnteringState_NAV_STATE_COURSE_HOLD_IN_PROGRESS(previousState);
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_CRUISE_ADJUSTING(navigationFSMState_t previousState)
{
    navOnEnteringState_NAV_STATE_ALTHOLD_IN_PROGRESS(previousState);

    return navOnEnteringState_NAV_STATE_COURSE_HOLD_ADJUSTING(previousState);
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_INITIALIZE(navigationFSMState_t previousState)
{
    navigationFSMStateFlags_t prevFlags = navGetStateFlags(previousState);

    if ((posControl.flags.estHeadingStatus == EST_NONE) || (posControl.flags.estAltStatus == EST_NONE) || !STATE(GPS_FIX_HOME)) {
        // Heading sensor, altitude sensor and HOME fix are mandatory for RTH. If not satisfied - switch to emergency landing
        // Relevant to failsafe forced RTH only. Switched RTH blocked in selectNavEventFromBoxModeInput if sensors unavailable.
        // If we are in dead-reckoning mode - also fail, since coordinates may be unreliable
        return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
    }

    if (STATE(FIXED_WING_LEGACY) && (posControl.homeDistance < navConfig()->general.min_rth_distance) && !posControl.flags.forcedRTHActivated) {
        // Prevent RTH from activating on airplanes if too close to home unless it's a failsafe RTH
        return NAV_FSM_EVENT_SWITCH_TO_IDLE;
    }

    // If we have valid position sensor or configured to ignore it's loss at initial stage - continue
    if ((posControl.flags.estPosStatus >= EST_USABLE) || navConfig()->general.flags.rth_climb_ignore_emerg) {
        // Reset altitude and position controllers if necessary
        if ((prevFlags & NAV_CTL_POS) == 0) {
            resetPositionController();
        }

        // Reset altitude controller if it was not enabled or if we are in terrain follow mode
        if ((prevFlags & NAV_CTL_ALT) == 0 || posControl.flags.isTerrainFollowEnabled) {
            // Make sure surface tracking is not enabled - RTH uses global altitude, not AGL
            resetAltitudeController(false);
            setupAltitudeController();
        }

        // If close to home - reset home position and land
        if (posControl.homeDistance < navConfig()->general.min_rth_distance) {
            setHomePosition(&navGetCurrentActualPositionAndVelocity()->pos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING, NAV_HOME_VALID_ALL);
            setDesiredPosition(&navGetCurrentActualPositionAndVelocity()->pos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING);

            return NAV_FSM_EVENT_SWITCH_TO_RTH_LANDING;   // NAV_STATE_RTH_HOVER_PRIOR_TO_LANDING
        }
        else {
            // Switch to RTH trackback
            bool trackbackActive = navConfig()->general.flags.rth_trackback_mode == RTH_TRACKBACK_ON ||
                                   (navConfig()->general.flags.rth_trackback_mode == RTH_TRACKBACK_FS && posControl.flags.forcedRTHActivated);

            if (trackbackActive && posControl.activeRthTBPointIndex >= 0 && !isWaypointMissionRTHActive()) {
                updateRthTrackback(true);       // save final trackpoint for altitude and max trackback distance reference
                posControl.flags.rthTrackbackActive = true;
                calculateAndSetActiveWaypointToLocalPosition(rthGetTrackbackPos());
                return NAV_FSM_EVENT_SWITCH_TO_NAV_STATE_RTH_TRACKBACK;
            }

            fpVector3_t targetHoldPos;

            if (STATE(FIXED_WING_LEGACY)) {
                // Airplane - climbout before heading home
                if (navConfig()->general.flags.rth_climb_first == RTH_CLIMB_ON_FW_SPIRAL) {
                    // Spiral climb centered at xy of RTH activation
                    calculateInitialHoldPosition(&targetHoldPos);
                } else {
                    calculateFarAwayTarget(&targetHoldPos, posControl.actualState.yaw, 100000.0f);  // 1km away Linear climb
                }
            } else {
                // Multicopter, hover and climb
                calculateInitialHoldPosition(&targetHoldPos);

                // Initialize RTH sanity check to prevent fly-aways on RTH
                // For airplanes this is delayed until climb-out is finished
                initializeRTHSanityChecker();
            }

            setDesiredPosition(&targetHoldPos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING);

            return NAV_FSM_EVENT_SUCCESS;   // NAV_STATE_RTH_CLIMB_TO_SAFE_ALT
        }
    }
    /* Position sensor failure timeout - land. Land immediately if failsafe RTH and timeout disabled (set to 0) */
    else if (checkForPositionSensorTimeout() || (!navConfig()->general.pos_failure_timeout && posControl.flags.forcedRTHActivated)) {
        return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
    }
    /* No valid POS sensor but still within valid timeout - wait */
    return NAV_FSM_EVENT_NONE;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_CLIMB_TO_SAFE_ALT(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    if (!STATE(ALTITUDE_CONTROL)) {
        //If altitude control is not a thing, switch to RTH in progress instead
        return NAV_FSM_EVENT_SUCCESS; //Will cause NAV_STATE_RTH_HEAD_HOME
    }

    rthAltControlStickOverrideCheck(PITCH);

    /* Position sensor failure timeout and not configured to ignore GPS loss - land */
    if ((posControl.flags.estHeadingStatus == EST_NONE) ||
        (checkForPositionSensorTimeout() && !navConfig()->general.flags.rth_climb_ignore_emerg)) {
        return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
    }

    const uint8_t rthClimbMarginPercent = STATE(FIXED_WING_LEGACY) ? FW_RTH_CLIMB_MARGIN_PERCENT : MR_RTH_CLIMB_MARGIN_PERCENT;
    const float rthAltitudeMargin = MAX(FW_RTH_CLIMB_MARGIN_MIN_CM, (rthClimbMarginPercent/100.0) * fabsf(posControl.rthState.rthInitialAltitude - posControl.rthState.homePosition.pos.z));

    // If we reached desired initial RTH altitude or we don't want to climb first
    if (((navGetCurrentActualPositionAndVelocity()->pos.z - posControl.rthState.rthInitialAltitude) > -rthAltitudeMargin) || (navConfig()->general.flags.rth_climb_first == RTH_CLIMB_OFF) || rthAltControlStickOverrideCheck(ROLL) || rthClimbStageActiveAndComplete()) {

        // Delayed initialization for RTH sanity check on airplanes - allow to finish climb first as it can take some distance
        if (STATE(FIXED_WING_LEGACY)) {
            initializeRTHSanityChecker();
        }

        // Save initial home distance for future use
        posControl.rthState.rthInitialDistance = posControl.homeDistance;
        fpVector3_t * tmpHomePos = rthGetHomeTargetPosition(RTH_HOME_ENROUTE_INITIAL);

        if (navConfig()->general.flags.rth_tail_first && !STATE(FIXED_WING_LEGACY)) {
            setDesiredPosition(tmpHomePos, 0, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_BEARING_TAIL_FIRST);
        }
        else {
            setDesiredPosition(tmpHomePos, 0, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_BEARING);
        }

        return NAV_FSM_EVENT_SUCCESS;   // NAV_STATE_RTH_HEAD_HOME

    } else {

        fpVector3_t * tmpHomePos = rthGetHomeTargetPosition(RTH_HOME_ENROUTE_INITIAL);

        /* For multi-rotors execute sanity check during initial ascent as well */
        if (!STATE(FIXED_WING_LEGACY) && !validateRTHSanityChecker()) {
            return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
        }

        // Climb to safe altitude and turn to correct direction
        if (STATE(FIXED_WING_LEGACY)) {
            if (navConfig()->general.flags.rth_climb_first == RTH_CLIMB_ON_FW_SPIRAL) {
                float altitudeChangeDirection = (tmpHomePos->z += FW_RTH_CLIMB_OVERSHOOT_CM) > navGetCurrentActualPositionAndVelocity()->pos.z ? 1 : -1;
                updateClimbRateToAltitudeController(altitudeChangeDirection * navConfig()->general.max_auto_climb_rate, ROC_TO_ALT_NORMAL);
            } else {
                tmpHomePos->z += FW_RTH_CLIMB_OVERSHOOT_CM;
                setDesiredPosition(tmpHomePos, 0, NAV_POS_UPDATE_Z);
            }
        } else {
            // Until the initial climb phase is complete target slightly *above* the cruise altitude to ensure we actually reach
            // it in a reasonable time. Immediately after we finish this phase - target the original altitude.
            tmpHomePos->z += MR_RTH_CLIMB_OVERSHOOT_CM;

            if (navConfig()->general.flags.rth_tail_first) {
                setDesiredPosition(tmpHomePos, 0, NAV_POS_UPDATE_Z | NAV_POS_UPDATE_BEARING_TAIL_FIRST);
            } else {
                setDesiredPosition(tmpHomePos, 0, NAV_POS_UPDATE_Z | NAV_POS_UPDATE_BEARING);
            }
        }

        return NAV_FSM_EVENT_NONE;
    }
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_TRACKBACK(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    /* If position sensors unavailable - land immediately */
    if ((posControl.flags.estHeadingStatus == EST_NONE) || checkForPositionSensorTimeout()) {
        return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
    }

    if (posControl.flags.estPosStatus >= EST_USABLE) {
        const int32_t distFromStartTrackback = calculateDistanceToDestination(&posControl.rthTBPointsList[posControl.rthTBLastSavedIndex]) / 100;
        const bool cancelTrackback = distFromStartTrackback > navConfig()->general.rth_trackback_distance ||
                                     (rthAltControlStickOverrideCheck(ROLL) && !posControl.flags.forcedRTHActivated);

        if (posControl.activeRthTBPointIndex < 0 || cancelTrackback) {
            posControl.rthTBWrapAroundCounter = posControl.activeRthTBPointIndex = -1;
            posControl.flags.rthTrackbackActive = false;
            return NAV_FSM_EVENT_SWITCH_TO_NAV_STATE_RTH_INITIALIZE;    // procede to home after final trackback point
        }

        if (isWaypointReached(&posControl.activeWaypoint.pos, &posControl.activeWaypoint.yaw)) {
            posControl.activeRthTBPointIndex--;

            if (posControl.rthTBWrapAroundCounter > -1 && posControl.activeRthTBPointIndex < 0) {
                posControl.activeRthTBPointIndex = NAV_RTH_TRACKBACK_POINTS - 1;
            }
            calculateAndSetActiveWaypointToLocalPosition(rthGetTrackbackPos());

            if (posControl.activeRthTBPointIndex - posControl.rthTBWrapAroundCounter == 0) {
                posControl.rthTBWrapAroundCounter = posControl.activeRthTBPointIndex = -1;
            }
        } else {
            setDesiredPosition(rthGetTrackbackPos(), 0, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_BEARING);
        }
    }

    return NAV_FSM_EVENT_NONE;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_HEAD_HOME(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    rthAltControlStickOverrideCheck(PITCH);

    /* If position sensors unavailable - land immediately */
    if ((posControl.flags.estHeadingStatus == EST_NONE) || !validateRTHSanityChecker()) {
        return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
    }

    // If we have position sensor - continue home
    if ((posControl.flags.estPosStatus >= EST_USABLE)) {
        fpVector3_t * tmpHomePos = rthGetHomeTargetPosition(RTH_HOME_ENROUTE_PROPORTIONAL);

        if (isWaypointReached(tmpHomePos, 0)) {
            // Successfully reached position target - update XYZ-position
            setDesiredPosition(tmpHomePos, posControl.rthState.homePosition.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING);
            return NAV_FSM_EVENT_SUCCESS;       // NAV_STATE_RTH_HOVER_PRIOR_TO_LANDING
        } else {
            setDesiredPosition(tmpHomePos, 0, NAV_POS_UPDATE_Z | NAV_POS_UPDATE_XY);
            return NAV_FSM_EVENT_NONE;
        }
    }
    /* Position sensor failure timeout - land */
    else if (checkForPositionSensorTimeout()) {
        return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
    }
    /* No valid POS sensor but still within valid timeout - wait */
    return NAV_FSM_EVENT_NONE;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_HOVER_PRIOR_TO_LANDING(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    //On ROVER and BOAT we immediately switch to the next event
    if (!STATE(ALTITUDE_CONTROL)) {
        return NAV_FSM_EVENT_SUCCESS;
    }

    /* If position sensors unavailable - land immediately (wait for timeout on GPS) */
    if ((posControl.flags.estHeadingStatus == EST_NONE) || checkForPositionSensorTimeout() || !validateRTHSanityChecker()) {
        return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
    }

    // If position ok OR within valid timeout - continue
    // Wait until target heading is reached for MR (with 15 deg margin for error), or continue for Fixed Wing
    if ((ABS(wrap_18000(posControl.rthState.homePosition.yaw - posControl.actualState.yaw)) < DEGREES_TO_CENTIDEGREES(15)) || STATE(FIXED_WING_LEGACY)) {
        resetLandingDetector();     // force reset landing detector just in case
        updateClimbRateToAltitudeController(0, ROC_TO_ALT_RESET);
        return navigationRTHAllowsLanding() ? NAV_FSM_EVENT_SUCCESS : NAV_FSM_EVENT_SWITCH_TO_RTH_HOVER_ABOVE_HOME; // success = land
    } else {
        fpVector3_t * tmpHomePos = rthGetHomeTargetPosition(RTH_HOME_ENROUTE_FINAL);
        setDesiredPosition(tmpHomePos, posControl.rthState.homePosition.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING);
        return NAV_FSM_EVENT_NONE;
    }
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_HOVER_ABOVE_HOME(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    /* If position sensors unavailable - land immediately (wait for timeout on GPS) */
    if (posControl.flags.estHeadingStatus == EST_NONE || checkForPositionSensorTimeout() || !validateRTHSanityChecker()) {
        return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
    }

    fpVector3_t * tmpHomePos = rthGetHomeTargetPosition(RTH_HOME_FINAL_HOVER);

    if (navConfig()->general.rth_home_altitude) {
        float timeToReachHomeAltitude = fabsf(tmpHomePos->z - navGetCurrentActualPositionAndVelocity()->pos.z) / navConfig()->general.max_auto_climb_rate;

        if (timeToReachHomeAltitude < 1) {
            // we almost reached the target home altitude so set the desired altitude to the desired home altitude
            setDesiredPosition(tmpHomePos, 0, NAV_POS_UPDATE_Z);
        } else {
            float altitudeChangeDirection = tmpHomePos->z > navGetCurrentActualPositionAndVelocity()->pos.z ? 1 : -1;
            updateClimbRateToAltitudeController(altitudeChangeDirection * navConfig()->general.max_auto_climb_rate, ROC_TO_ALT_NORMAL);
        }
    }
    else {
        setDesiredPosition(tmpHomePos, 0, NAV_POS_UPDATE_Z);
    }

    return NAV_FSM_EVENT_NONE;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_LANDING(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    //On ROVER and BOAT we immediately switch to the next event
    if (!STATE(ALTITUDE_CONTROL)) {
        return NAV_FSM_EVENT_SUCCESS;
    }

    if (!ARMING_FLAG(ARMED) || STATE(LANDING_DETECTED)) {
        return NAV_FSM_EVENT_SUCCESS;
    }

    /* If position sensors unavailable - land immediately (wait for timeout on GPS)
     * Continue to check for RTH sanity during landing */
    if (posControl.flags.estHeadingStatus == EST_NONE || checkForPositionSensorTimeout() || !validateRTHSanityChecker()) {
        return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
    }

    float descentVelLimited = 0;
    int32_t landingElevation = posControl.rthState.homeTmpWaypoint.z;

    // A safeguard - if surface altitude sensors is available and it is reading < 50cm altitude - drop to low descend speed
    if ((posControl.flags.estAglStatus == EST_TRUSTED) && posControl.actualState.agl.pos.z < 50.0f) {
        // land_descent_rate == 200 : descend speed = 30 cm/s, gentle touchdown
        // Do not allow descent velocity slower than -30cm/s so the landing detector works.
        descentVelLimited = navConfig()->general.land_minalt_vspd;
    } else {
        // Ramp down descent velocity from 100% at maxAlt altitude to 25% from minAlt to 0cm.
        float descentVelScaled = scaleRangef(navGetCurrentActualPositionAndVelocity()->pos.z,
                                 navConfig()->general.land_slowdown_minalt + landingElevation,
                                 navConfig()->general.land_slowdown_maxalt + landingElevation,
                                 navConfig()->general.land_minalt_vspd, navConfig()->general.land_maxalt_vspd);

        descentVelLimited = constrainf(descentVelScaled, navConfig()->general.land_minalt_vspd, navConfig()->general.land_maxalt_vspd);
    }

    updateClimbRateToAltitudeController(-descentVelLimited, ROC_TO_ALT_NORMAL);

    return NAV_FSM_EVENT_NONE;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_FINISHING(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    //On ROVER and BOAT disarm immediately
    if (!STATE(ALTITUDE_CONTROL)) {
        disarm(DISARM_NAVIGATION);
    }

    return NAV_FSM_EVENT_SUCCESS;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_RTH_FINISHED(navigationFSMState_t previousState)
{
    // Stay in this state
    UNUSED(previousState);

    if (STATE(ALTITUDE_CONTROL)) {
        updateClimbRateToAltitudeController(-1.1f * navConfig()->general.land_minalt_vspd, ROC_TO_ALT_NORMAL);  // FIXME
    }

    // Prevent I-terms growing when already landed
    pidResetErrorAccumulators();
    return NAV_FSM_EVENT_NONE;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_INITIALIZE(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    if (posControl.waypointCount == 0 || !posControl.waypointListValid) {
        return NAV_FSM_EVENT_ERROR;
    }
    else {
        // Prepare controllers
        resetPositionController();

        // Make sure surface tracking is not enabled - RTH uses global altitude, not AGL
        resetAltitudeController(false);
        setupAltitudeController();
/*
  Use p3 as the volatile jump counter, allowing embedded, rearmed jumps
  Using p3 minimises the risk of saving an invalid counter if a mission is aborted.
*/
        if (posControl.activeWaypointIndex == posControl.startWpIndex || posControl.wpMissionRestart) {
            setupJumpCounters();
            posControl.activeWaypointIndex = posControl.startWpIndex;
            wpHeadingControl.mode = NAV_WP_HEAD_MODE_NONE;
        }

        if (navConfig()->general.flags.waypoint_mission_restart == WP_MISSION_SWITCH) {
            posControl.wpMissionRestart = posControl.activeWaypointIndex > posControl.startWpIndex ? !posControl.wpMissionRestart : false;
        } else {
            posControl.wpMissionRestart = navConfig()->general.flags.waypoint_mission_restart == WP_MISSION_START;
        }

        return NAV_FSM_EVENT_SUCCESS;   // will switch to NAV_STATE_WAYPOINT_PRE_ACTION
    }
}

static navigationFSMEvent_t nextForNonGeoStates(void)
{
    /* simple helper for non-geographical states that just set other data */
    if (isLastMissionWaypoint()) { // non-geo state is the last waypoint, switch to finish.
        return NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_FINISHED;
    } else {    // Finished non-geo,  move to next WP
        posControl.activeWaypointIndex++;
        return NAV_FSM_EVENT_NONE; // re-process the state passing to the next WP
    }
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_PRE_ACTION(navigationFSMState_t previousState)
{
    /* A helper function to do waypoint-specific action */
    UNUSED(previousState);

    switch ((navWaypointActions_e)posControl.waypointList[posControl.activeWaypointIndex].action) {
        case NAV_WP_ACTION_HOLD_TIME:
        case NAV_WP_ACTION_WAYPOINT:
        case NAV_WP_ACTION_LAND:
            calculateAndSetActiveWaypoint(&posControl.waypointList[posControl.activeWaypointIndex]);
            posControl.wpInitialDistance = calculateDistanceToDestination(&posControl.activeWaypoint.pos);
            posControl.wpInitialAltitude = posControl.actualState.abs.pos.z;
            posControl.wpAltitudeReached = false;
            return NAV_FSM_EVENT_SUCCESS;       // will switch to NAV_STATE_WAYPOINT_IN_PROGRESS

                // We use p3 as the volatile jump counter (p2 is the static value)
        case NAV_WP_ACTION_JUMP:
            if (posControl.waypointList[posControl.activeWaypointIndex].p3 != -1) {
                if (posControl.waypointList[posControl.activeWaypointIndex].p3 == 0) {
                    resetJumpCounter();
                    return nextForNonGeoStates();
                }
                else
                {
                    posControl.waypointList[posControl.activeWaypointIndex].p3--;
                }
            }
            posControl.activeWaypointIndex = posControl.waypointList[posControl.activeWaypointIndex].p1 + posControl.startWpIndex;
            return NAV_FSM_EVENT_NONE; // re-process the state passing to the next WP

        case NAV_WP_ACTION_SET_POI:
            if (STATE(MULTIROTOR)) {
                wpHeadingControl.mode = NAV_WP_HEAD_MODE_POI;
                mapWaypointToLocalPosition(&wpHeadingControl.poi_pos,
                                           &posControl.waypointList[posControl.activeWaypointIndex], GEO_ALT_RELATIVE);
            }
            return nextForNonGeoStates();

        case NAV_WP_ACTION_SET_HEAD:
            if (STATE(MULTIROTOR)) {
                if (posControl.waypointList[posControl.activeWaypointIndex].p1 < 0 ||
                    posControl.waypointList[posControl.activeWaypointIndex].p1 > 359) {
                    wpHeadingControl.mode = NAV_WP_HEAD_MODE_NONE;
                } else {
                    wpHeadingControl.mode = NAV_WP_HEAD_MODE_FIXED;
                    wpHeadingControl.heading = DEGREES_TO_CENTIDEGREES(posControl.waypointList[posControl.activeWaypointIndex].p1);
                }
            }
            return nextForNonGeoStates();

        case NAV_WP_ACTION_RTH:
            posControl.wpMissionRestart = true;
            return NAV_FSM_EVENT_SWITCH_TO_RTH;
    };

    UNREACHABLE();
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_IN_PROGRESS(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    // If no position sensor available - land immediately
    if ((posControl.flags.estPosStatus >= EST_USABLE) && (posControl.flags.estHeadingStatus >= EST_USABLE)) {
        switch ((navWaypointActions_e)posControl.waypointList[posControl.activeWaypointIndex].action) {
            case NAV_WP_ACTION_HOLD_TIME:
            case NAV_WP_ACTION_WAYPOINT:
            case NAV_WP_ACTION_LAND:
                if (isWaypointReached(&posControl.activeWaypoint.pos, &posControl.activeWaypoint.yaw)) {
                    return NAV_FSM_EVENT_SUCCESS;   // will switch to NAV_STATE_WAYPOINT_REACHED
                }
                else {
                    fpVector3_t tmpWaypoint;
                    tmpWaypoint.x = posControl.activeWaypoint.pos.x;
                    tmpWaypoint.y = posControl.activeWaypoint.pos.y;
                    tmpWaypoint.z = scaleRangef(constrainf(posControl.wpDistance, posControl.wpInitialDistance / 10.0f, posControl.wpInitialDistance),
                        posControl.wpInitialDistance, posControl.wpInitialDistance / 10.0f,
                        posControl.wpInitialAltitude, posControl.activeWaypoint.pos.z);
                    setDesiredPosition(&tmpWaypoint, 0, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_BEARING);
                    if(STATE(MULTIROTOR)) {
                        switch (wpHeadingControl.mode) {
                            case NAV_WP_HEAD_MODE_NONE:
                                break;
                            case NAV_WP_HEAD_MODE_FIXED:
                                setDesiredPosition(NULL, wpHeadingControl.heading, NAV_POS_UPDATE_HEADING);
                                break;
                            case NAV_WP_HEAD_MODE_POI:
                                setDesiredPosition(&wpHeadingControl.poi_pos, 0, NAV_POS_UPDATE_BEARING);
                                break;
                        }
                    }
                    return NAV_FSM_EVENT_NONE;      // will re-process state in >10ms
                }
                break;

            case NAV_WP_ACTION_JUMP:
            case NAV_WP_ACTION_SET_HEAD:
            case NAV_WP_ACTION_SET_POI:
            case NAV_WP_ACTION_RTH:
                UNREACHABLE();
        }
    }
    /* If position sensors unavailable - land immediately (wait for timeout on GPS) */
    else if (checkForPositionSensorTimeout() || (posControl.flags.estHeadingStatus == EST_NONE)) {
        return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
    }

    return NAV_FSM_EVENT_NONE;      // will re-process state in >10ms
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_REACHED(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    if (navConfig()->general.waypoint_enforce_altitude) {
        posControl.wpAltitudeReached = isWaypointAltitudeReached();
    }

    switch ((navWaypointActions_e)posControl.waypointList[posControl.activeWaypointIndex].action) {
        case NAV_WP_ACTION_WAYPOINT:
            if (navConfig()->general.waypoint_enforce_altitude && !posControl.wpAltitudeReached) {
                return NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_HOLD_TIME;
            } else {
                return NAV_FSM_EVENT_SUCCESS;   // NAV_STATE_WAYPOINT_NEXT
            }

        case NAV_WP_ACTION_JUMP:
        case NAV_WP_ACTION_SET_HEAD:
        case NAV_WP_ACTION_SET_POI:
        case NAV_WP_ACTION_RTH:
            UNREACHABLE();

        case NAV_WP_ACTION_LAND:
            return NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_RTH_LAND;

        case NAV_WP_ACTION_HOLD_TIME:
            // Save the current time for the time the waypoint was reached
            posControl.wpReachedTime = millis();
            return NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_HOLD_TIME;
    }

    UNREACHABLE();
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_HOLD_TIME(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    /* If position sensors unavailable - land immediately (wait for timeout on GPS) */
    if (posControl.flags.estHeadingStatus == EST_NONE || checkForPositionSensorTimeout()) {
        return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
    }

    if (navConfig()->general.waypoint_enforce_altitude && !posControl.wpAltitudeReached) {
        // Adjust altitude to waypoint setting
        if (STATE(AIRPLANE)) {
            int8_t altitudeChangeDirection = posControl.activeWaypoint.pos.z > navGetCurrentActualPositionAndVelocity()->pos.z ? 1 : -1;
            updateClimbRateToAltitudeController(altitudeChangeDirection * navConfig()->general.max_auto_climb_rate, ROC_TO_ALT_NORMAL);
        } else {
            setDesiredPosition(&posControl.activeWaypoint.pos, 0, NAV_POS_UPDATE_Z);
        }

        posControl.wpAltitudeReached = isWaypointAltitudeReached();

        if (posControl.wpAltitudeReached) {
            posControl.wpReachedTime = millis();
        } else {
            return NAV_FSM_EVENT_NONE;
        }
    }

    timeMs_t currentTime = millis();

    if (posControl.waypointList[posControl.activeWaypointIndex].p1 <= 0 ||
        (posControl.wpReachedTime != 0 && currentTime - posControl.wpReachedTime >= (timeMs_t)posControl.waypointList[posControl.activeWaypointIndex].p1*1000L)) {
        return NAV_FSM_EVENT_SUCCESS;
    }

    return NAV_FSM_EVENT_NONE;      // will re-process state in >10ms
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_RTH_LAND(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    const navigationFSMEvent_t landEvent = navOnEnteringState_NAV_STATE_RTH_LANDING(previousState);
    if (landEvent == NAV_FSM_EVENT_SUCCESS) {
        // Landing controller returned success - invoke RTH finishing state and finish the waypoint
        navOnEnteringState_NAV_STATE_RTH_FINISHING(previousState);
        return NAV_FSM_EVENT_SUCCESS;
    }
    else {
        return NAV_FSM_EVENT_NONE;
    }
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_NEXT(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    if (isLastMissionWaypoint()) {      // Last waypoint reached
        return NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_FINISHED;
    }
    else {
        // Waypoint reached, do something and move on to next waypoint
        posControl.activeWaypointIndex++;
        return NAV_FSM_EVENT_SUCCESS;   // will switch to NAV_STATE_WAYPOINT_PRE_ACTION
    }
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_WAYPOINT_FINISHED(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    clearJumpCounters();
    posControl.wpMissionRestart = true;

    /* If position sensors unavailable - land immediately (wait for timeout on GPS) */
    if (posControl.flags.estHeadingStatus == EST_NONE || checkForPositionSensorTimeout()) {
        return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
    }

    return NAV_FSM_EVENT_NONE;      // will re-process state in >10ms
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_EMERGENCY_LANDING_INITIALIZE(navigationFSMState_t previousState)
{
    // TODO:
    UNUSED(previousState);

    // Emergency landing MAY use common altitude controller if vertical position is valid - initialize it
    // Make sure terrain following is not enabled
    resetAltitudeController(false);

    return NAV_FSM_EVENT_SUCCESS;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_EMERGENCY_LANDING_IN_PROGRESS(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    if (STATE(LANDING_DETECTED)) {
        return NAV_FSM_EVENT_SUCCESS;
    }

    return NAV_FSM_EVENT_NONE;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_EMERGENCY_LANDING_FINISHED(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    disarm(DISARM_NAVIGATION);

    return NAV_FSM_EVENT_NONE;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_LAUNCH_INITIALIZE(navigationFSMState_t previousState)
{
    const timeUs_t currentTimeUs = micros();
    UNUSED(previousState);

    resetFixedWingLaunchController(currentTimeUs);

    return NAV_FSM_EVENT_SUCCESS;   // NAV_STATE_LAUNCH_WAIT
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_LAUNCH_WAIT(navigationFSMState_t previousState)
{
    const timeUs_t currentTimeUs = micros();
    UNUSED(previousState);

    // Continue immediately to launch in progress if manual launch throttle used
    if (navConfig()->fw.launch_manual_throttle) {
        return NAV_FSM_EVENT_SUCCESS;
    }

    if (fixedWingLaunchStatus() == FW_LAUNCH_DETECTED) {
        enableFixedWingLaunchController(currentTimeUs);
        return NAV_FSM_EVENT_SUCCESS;   // NAV_STATE_LAUNCH_IN_PROGRESS
    }

    // abort NAV_LAUNCH_MODE by moving sticks with low throttle or throttle stick < launch idle throttle
    if (abortLaunchAllowed() && isRollPitchStickDeflected(navConfig()->fw.launch_abort_deadband)) {
        abortFixedWingLaunch();
        return NAV_FSM_EVENT_SWITCH_TO_IDLE;
    }

    return NAV_FSM_EVENT_NONE;
}

static navigationFSMEvent_t navOnEnteringState_NAV_STATE_LAUNCH_IN_PROGRESS(navigationFSMState_t previousState)
{
    UNUSED(previousState);

    if (fixedWingLaunchStatus() >= FW_LAUNCH_ABORTED) {
        return NAV_FSM_EVENT_SUCCESS;
    }

    return NAV_FSM_EVENT_NONE;
}

static navigationFSMState_t navSetNewFSMState(navigationFSMState_t newState)
{
    navigationFSMState_t previousState;

    previousState = posControl.navState;
    if (posControl.navState != newState) {
        posControl.navState = newState;
        posControl.navPersistentId = navFSM[newState].persistentId;
    }
    return previousState;
}

static void navProcessFSMEvents(navigationFSMEvent_t injectedEvent)
{
    const timeMs_t currentMillis = millis();
    navigationFSMState_t previousState;
    static timeMs_t lastStateProcessTime = 0;

    /* Process new injected event if event defined,
     * otherwise process timeout event if defined */
    if (injectedEvent != NAV_FSM_EVENT_NONE && navFSM[posControl.navState].onEvent[injectedEvent] != NAV_STATE_UNDEFINED) {
        /* Update state */
        previousState = navSetNewFSMState(navFSM[posControl.navState].onEvent[injectedEvent]);
    } else if ((navFSM[posControl.navState].timeoutMs > 0) && (navFSM[posControl.navState].onEvent[NAV_FSM_EVENT_TIMEOUT] != NAV_STATE_UNDEFINED) &&
            ((currentMillis - lastStateProcessTime) >= navFSM[posControl.navState].timeoutMs)) {
        /* Update state */
        previousState = navSetNewFSMState(navFSM[posControl.navState].onEvent[NAV_FSM_EVENT_TIMEOUT]);
    }

    if (previousState) {    /* If state updated call new state's entry function */
        while (navFSM[posControl.navState].onEntry) {
            navigationFSMEvent_t newEvent = navFSM[posControl.navState].onEntry(previousState);

            if ((newEvent != NAV_FSM_EVENT_NONE) && (navFSM[posControl.navState].onEvent[newEvent] != NAV_STATE_UNDEFINED)) {
                previousState = navSetNewFSMState(navFSM[posControl.navState].onEvent[newEvent]);
            }
            else {
                break;
            }
        }

        lastStateProcessTime = currentMillis;
    }

    /* Update public system state information */
    NAV_Status.mode = MW_GPS_MODE_NONE;

    if (ARMING_FLAG(ARMED)) {
        navigationFSMStateFlags_t navStateFlags = navGetStateFlags(posControl.navState);

        if (navStateFlags & NAV_AUTO_RTH) {
            NAV_Status.mode = MW_GPS_MODE_RTH;
        }
        else if (navStateFlags & NAV_AUTO_WP) {
            NAV_Status.mode = MW_GPS_MODE_NAV;
        }
        else if (navStateFlags & NAV_CTL_EMERG) {
            NAV_Status.mode = MW_GPS_MODE_EMERG;
        }
        else if (navStateFlags & NAV_CTL_POS) {
            NAV_Status.mode = MW_GPS_MODE_HOLD;
        }
    }

    NAV_Status.state = navFSM[posControl.navState].mwState;
    NAV_Status.error = navFSM[posControl.navState].mwError;

    NAV_Status.flags = 0;
    if (posControl.flags.isAdjustingPosition)   NAV_Status.flags |= MW_NAV_FLAG_ADJUSTING_POSITION;
    if (posControl.flags.isAdjustingAltitude)   NAV_Status.flags |= MW_NAV_FLAG_ADJUSTING_ALTITUDE;

    NAV_Status.activeWpIndex = posControl.activeWaypointIndex - posControl.startWpIndex;
    NAV_Status.activeWpNumber = NAV_Status.activeWpIndex + 1;

    NAV_Status.activeWpAction = 0;
    if ((posControl.activeWaypointIndex >= 0) && (posControl.activeWaypointIndex < NAV_MAX_WAYPOINTS)) {
        NAV_Status.activeWpAction = posControl.waypointList[posControl.activeWaypointIndex].action;
    }
}

static fpVector3_t * rthGetHomeTargetPosition(rthTargetMode_e mode)
{
    posControl.rthState.homeTmpWaypoint = posControl.rthState.homePosition.pos;

    switch (mode) {
        case RTH_HOME_ENROUTE_INITIAL:
            posControl.rthState.homeTmpWaypoint.z = posControl.rthState.rthInitialAltitude;
            break;

        case RTH_HOME_ENROUTE_PROPORTIONAL:
            {
                float rthTotalDistanceToTravel = posControl.rthState.rthInitialDistance - (STATE(FIXED_WING_LEGACY) ? navConfig()->fw.loiter_radius : 0);
                if (rthTotalDistanceToTravel >= 100) {
                    float ratioNotTravelled = constrainf(posControl.homeDistance / rthTotalDistanceToTravel, 0.0f, 1.0f);
                    posControl.rthState.homeTmpWaypoint.z = (posControl.rthState.rthInitialAltitude * ratioNotTravelled) + (posControl.rthState.rthFinalAltitude * (1.0f - ratioNotTravelled));
                }
                else {
                    posControl.rthState.homeTmpWaypoint.z = posControl.rthState.rthFinalAltitude;
                }
            }
            break;

        case RTH_HOME_ENROUTE_FINAL:
            posControl.rthState.homeTmpWaypoint.z = posControl.rthState.rthFinalAltitude;
            break;

        case RTH_HOME_FINAL_HOVER:
            if (navConfig()->general.rth_home_altitude) {
                posControl.rthState.homeTmpWaypoint.z = posControl.rthState.homePosition.pos.z + navConfig()->general.rth_home_altitude;
            }
            else {
                // If home altitude not defined - fall back to final ENROUTE altitude
                posControl.rthState.homeTmpWaypoint.z = posControl.rthState.rthFinalAltitude;
            }
            break;

        case RTH_HOME_FINAL_LAND:
            // if WP mission p2 > 0 use p2 value as landing elevation (in meters !) (otherwise default to takeoff home elevation)
            if (FLIGHT_MODE(NAV_WP_MODE) && posControl.waypointList[posControl.activeWaypointIndex].action == NAV_WP_ACTION_LAND && posControl.waypointList[posControl.activeWaypointIndex].p2 != 0) {
                posControl.rthState.homeTmpWaypoint.z = posControl.waypointList[posControl.activeWaypointIndex].p2 * 100;   // 100 -> m to cm
                if (waypointMissionAltConvMode(posControl.waypointList[posControl.activeWaypointIndex].p3) == GEO_ALT_ABSOLUTE) {
                    posControl.rthState.homeTmpWaypoint.z -= posControl.gpsOrigin.alt;  // correct to relative if absolute SL altitude datum used
                }
            }
            break;
    }

    return &posControl.rthState.homeTmpWaypoint;
}

/*-----------------------------------------------------------
 * Detects if thrust vector is facing downwards
 *-----------------------------------------------------------*/
bool isThrustFacingDownwards(void)
{
    // Tilt angle <= 80 deg; cos(80) = 0.17364817766693034885171662676931
    return (calculateCosTiltAngle() >= 0.173648178f);
}

/*-----------------------------------------------------------
 * Checks if position sensor (GPS) is failing for a specified timeout (if enabled)
 *-----------------------------------------------------------*/
bool checkForPositionSensorTimeout(void)
{
    if (navConfig()->general.pos_failure_timeout) {
        if ((posControl.flags.estPosStatus == EST_NONE) && ((millis() - posControl.lastValidPositionTimeMs) > (1000 * navConfig()->general.pos_failure_timeout))) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        // Timeout not defined, never fail
        return false;
    }
}

/*-----------------------------------------------------------
 * Processes an update to XY-position and velocity
 *-----------------------------------------------------------*/
void updateActualHorizontalPositionAndVelocity(bool estPosValid, bool estVelValid, float newX, float newY, float newVelX, float newVelY)
{
    posControl.actualState.abs.pos.x = newX;
    posControl.actualState.abs.pos.y = newY;
    posControl.actualState.abs.vel.x = newVelX;
    posControl.actualState.abs.vel.y = newVelY;

    posControl.actualState.agl.pos.x = newX;
    posControl.actualState.agl.pos.y = newY;
    posControl.actualState.agl.vel.x = newVelX;
    posControl.actualState.agl.vel.y = newVelY;

    posControl.actualState.velXY = calc_length_pythagorean_2D(newVelX, newVelY);

    // CASE 1: POS & VEL valid
    if (estPosValid && estVelValid) {
        posControl.flags.estPosStatus = EST_TRUSTED;
        posControl.flags.estVelStatus = EST_TRUSTED;
        posControl.flags.horizontalPositionDataNew = true;
        posControl.lastValidPositionTimeMs = millis();
    }
    // CASE 1: POS invalid, VEL valid
    else if (!estPosValid && estVelValid) {
        posControl.flags.estPosStatus = EST_USABLE;     // Pos usable, but not trusted
        posControl.flags.estVelStatus = EST_TRUSTED;
        posControl.flags.horizontalPositionDataNew = true;
        posControl.lastValidPositionTimeMs = millis();
    }
    // CASE 3: can't use pos/vel data
    else {
        posControl.flags.estPosStatus = EST_NONE;
        posControl.flags.estVelStatus = EST_NONE;
        posControl.flags.horizontalPositionDataNew = false;
    }

    //Update blackbox data
    navLatestActualPosition[X] = newX;
    navLatestActualPosition[Y] = newY;
    navActualVelocity[X] = constrain(newVelX, -32678, 32767);
    navActualVelocity[Y] = constrain(newVelY, -32678, 32767);
}

/*-----------------------------------------------------------
 * Processes an update to Z-position and velocity
 *-----------------------------------------------------------*/
void updateActualAltitudeAndClimbRate(bool estimateValid, float newAltitude, float newVelocity, float surfaceDistance, float surfaceVelocity, navigationEstimateStatus_e surfaceStatus)
{
    posControl.actualState.abs.pos.z = newAltitude;
    posControl.actualState.abs.vel.z = newVelocity;

    posControl.actualState.agl.pos.z = surfaceDistance;
    posControl.actualState.agl.vel.z = surfaceVelocity;

    // Update altitude that would be used when executing RTH
    if (estimateValid) {
        updateDesiredRTHAltitude();

        // If we acquired new surface reference - changing from NONE/USABLE -> TRUSTED
        if ((surfaceStatus == EST_TRUSTED) && (posControl.flags.estAglStatus != EST_TRUSTED)) {
            // If we are in terrain-following modes - signal that we should update the surface tracking setpoint
            //      NONE/USABLE means that we were flying blind, now we should lock to surface
            //updateSurfaceTrackingSetpoint();
        }

        posControl.flags.estAglStatus = surfaceStatus;  // Could be TRUSTED or USABLE
        posControl.flags.estAltStatus = EST_TRUSTED;
        posControl.flags.verticalPositionDataNew = true;
        posControl.lastValidAltitudeTimeMs = millis();
    }
    else {
        posControl.flags.estAltStatus = EST_NONE;
        posControl.flags.estAglStatus = EST_NONE;
        posControl.flags.verticalPositionDataNew = false;
    }

    if (ARMING_FLAG(ARMED)) {
        if ((posControl.flags.estAglStatus == EST_TRUSTED) && surfaceDistance > 0) {
            if (posControl.actualState.surfaceMin > 0) {
                posControl.actualState.surfaceMin = MIN(posControl.actualState.surfaceMin, surfaceDistance);
            }
            else {
                posControl.actualState.surfaceMin = surfaceDistance;
            }
        }
    }
    else {
        posControl.actualState.surfaceMin = -1;
    }

    //Update blackbox data
    navLatestActualPosition[Z] = navGetCurrentActualPositionAndVelocity()->pos.z;
    navActualVelocity[Z] = constrain(navGetCurrentActualPositionAndVelocity()->vel.z, -32678, 32767);
}

/*-----------------------------------------------------------
 * Processes an update to estimated heading
 *-----------------------------------------------------------*/
void updateActualHeading(bool headingValid, int32_t newHeading)
{
    /* Update heading. Check if we're acquiring a valid heading for the
     * first time and update home heading accordingly.
     */

    navigationEstimateStatus_e newEstHeading = headingValid ? EST_TRUSTED : EST_NONE;

#ifdef USE_DEV_TOOLS
    if (systemConfig()->groundTestMode && STATE(AIRPLANE)) {
        newEstHeading = EST_TRUSTED;
    }
#endif
    if (newEstHeading >= EST_USABLE && posControl.flags.estHeadingStatus < EST_USABLE &&
        (posControl.rthState.homeFlags & (NAV_HOME_VALID_XY | NAV_HOME_VALID_Z)) &&
        (posControl.rthState.homeFlags & NAV_HOME_VALID_HEADING) == 0) {

        // Home was stored using the fake heading (assuming boot as 0deg). Calculate
        // the offset from the fake to the actual yaw and apply the same rotation
        // to the home point.
        int32_t fakeToRealYawOffset = newHeading - posControl.actualState.yaw;

        posControl.rthState.homePosition.yaw += fakeToRealYawOffset;
        if (posControl.rthState.homePosition.yaw < 0) {
            posControl.rthState.homePosition.yaw += DEGREES_TO_CENTIDEGREES(360);
        }
        if (posControl.rthState.homePosition.yaw >= DEGREES_TO_CENTIDEGREES(360)) {
            posControl.rthState.homePosition.yaw -= DEGREES_TO_CENTIDEGREES(360);
        }
        posControl.rthState.homeFlags |= NAV_HOME_VALID_HEADING;
    }
    posControl.actualState.yaw = newHeading;
    posControl.flags.estHeadingStatus = newEstHeading;

    /* Precompute sin/cos of yaw angle */
    posControl.actualState.sinYaw = sin_approx(CENTIDEGREES_TO_RADIANS(newHeading));
    posControl.actualState.cosYaw = cos_approx(CENTIDEGREES_TO_RADIANS(newHeading));
}

/*-----------------------------------------------------------
 * Returns pointer to currently used position (ABS or AGL) depending on surface tracking status
 *-----------------------------------------------------------*/
const navEstimatedPosVel_t * navGetCurrentActualPositionAndVelocity(void)
{
    return posControl.flags.isTerrainFollowEnabled ? &posControl.actualState.agl : &posControl.actualState.abs;
}

/*-----------------------------------------------------------
 * Calculates distance and bearing to destination point
 *-----------------------------------------------------------*/
static uint32_t calculateDistanceFromDelta(float deltaX, float deltaY)
{
    return calc_length_pythagorean_2D(deltaX, deltaY);
}

static int32_t calculateBearingFromDelta(float deltaX, float deltaY)
{
    return wrap_36000(RADIANS_TO_CENTIDEGREES(atan2_approx(deltaY, deltaX)));
}

uint32_t calculateDistanceToDestination(const fpVector3_t * destinationPos)
{
    const navEstimatedPosVel_t *posvel = navGetCurrentActualPositionAndVelocity();
    const float deltaX = destinationPos->x - posvel->pos.x;
    const float deltaY = destinationPos->y - posvel->pos.y;

    return calculateDistanceFromDelta(deltaX, deltaY);
}

int32_t calculateBearingToDestination(const fpVector3_t * destinationPos)
{
    const navEstimatedPosVel_t *posvel = navGetCurrentActualPositionAndVelocity();
    const float deltaX = destinationPos->x - posvel->pos.x;
    const float deltaY = destinationPos->y - posvel->pos.y;

    return calculateBearingFromDelta(deltaX, deltaY);
}

int32_t calculateBearingBetweenLocalPositions(const fpVector3_t * startPos, const fpVector3_t * endPos)
{
    const float deltaX = endPos->x - startPos->x;
    const float deltaY = endPos->y - startPos->y;

    return calculateBearingFromDelta(deltaX, deltaY);
}

bool navCalculatePathToDestination(navDestinationPath_t *result, const fpVector3_t * destinationPos)
{
    if (posControl.flags.estPosStatus == EST_NONE ||
        posControl.flags.estHeadingStatus == EST_NONE) {

        return false;
    }

    const navEstimatedPosVel_t *posvel = navGetCurrentActualPositionAndVelocity();
    const float deltaX = destinationPos->x - posvel->pos.x;
    const float deltaY = destinationPos->y - posvel->pos.y;

    result->distance = calculateDistanceFromDelta(deltaX, deltaY);
    result->bearing = calculateBearingFromDelta(deltaX, deltaY);
    return true;
}

static bool getLocalPosNextWaypoint(fpVector3_t * nextWpPos)
{
    // Only for WP Mode not Trackback. Ignore non geo waypoints except RTH and JUMP.
    if (FLIGHT_MODE(NAV_WP_MODE) && !isLastMissionWaypoint()) {
        navWaypointActions_e nextWpAction = posControl.waypointList[posControl.activeWaypointIndex + 1].action;

        if (!(nextWpAction == NAV_WP_ACTION_SET_POI || nextWpAction == NAV_WP_ACTION_SET_HEAD)) {
            uint8_t nextWpIndex = posControl.activeWaypointIndex + 1;
            if (nextWpAction == NAV_WP_ACTION_JUMP) {
                if (posControl.waypointList[posControl.activeWaypointIndex + 1].p3 != 0 ||
                    posControl.waypointList[posControl.activeWaypointIndex + 1].p2 == -1) {
                    nextWpIndex = posControl.waypointList[posControl.activeWaypointIndex + 1].p1 + posControl.startWpIndex;
                } else if (posControl.activeWaypointIndex + 2 <= posControl.startWpIndex + posControl.waypointCount - 1) {
                    if (posControl.waypointList[posControl.activeWaypointIndex + 2].action != NAV_WP_ACTION_JUMP) {
                        nextWpIndex++;
                    } else {
                        return false;   // give up - too complicated
                    }
                }
            }
            mapWaypointToLocalPosition(nextWpPos, &posControl.waypointList[nextWpIndex], 0);
            return true;
        }
    }

    return false;   // no position available
}

/*-----------------------------------------------------------
 * Check if waypoint is/was reached.
 * waypointYaw stores initial bearing to waypoint
 *-----------------------------------------------------------*/
static bool isWaypointReached(const fpVector3_t * waypointPos, const int32_t * waypointYaw)
{
    posControl.wpDistance = calculateDistanceToDestination(waypointPos);

    // Airplane will do a circular loiter at hold waypoints and might never approach them closer than waypoint_radius
    // Check within 10% margin of circular loiter radius
    if (STATE(AIRPLANE) && isNavHoldPositionActive() && posControl.wpDistance <= (navConfig()->fw.loiter_radius * 1.10f)) {
        return true;
    }

    if (navGetStateFlags(posControl.navState) & NAV_AUTO_WP || posControl.flags.rthTrackbackActive) {
        // If WP turn smoothing CUT option used WP is reached when start of turn is initiated
        if (navConfig()->fw.wp_turn_smoothing == WP_TURN_SMOOTHING_CUT && posControl.flags.wpTurnSmoothingActive) {
            posControl.flags.wpTurnSmoothingActive = false;
            return true;
        }
        // Check if waypoint was missed based on bearing to WP exceeding 100 degrees relative to waypoint Yaw
        // Same method for turn smoothing option but relative bearing set at 60 degrees
        uint16_t relativeBearing = posControl.flags.wpTurnSmoothingActive ? 6000 : 10000;
        if (ABS(wrap_18000(calculateBearingToDestination(waypointPos) - *waypointYaw)) > relativeBearing) {
            return true;
        }
    }

    return posControl.wpDistance <= (navConfig()->general.waypoint_radius);
}

bool isWaypointAltitudeReached(void)
{
    return ABS(navGetCurrentActualPositionAndVelocity()->pos.z - posControl.activeWaypoint.pos.z) < navConfig()->general.waypoint_enforce_altitude;
}

static void updateHomePositionCompatibility(void)
{
    geoConvertLocalToGeodetic(&GPS_home, &posControl.gpsOrigin, &posControl.rthState.homePosition.pos);
    GPS_distanceToHome = posControl.homeDistance * 0.01f;
    GPS_directionToHome = posControl.homeDirection * 0.01f;
}

// Backdoor for RTH estimator
float getFinalRTHAltitude(void)
{
    return posControl.rthState.rthFinalAltitude;
}

/*-----------------------------------------------------------
 * Update the RTH Altitudes
 *-----------------------------------------------------------*/
static void updateDesiredRTHAltitude(void)
{
    if (ARMING_FLAG(ARMED)) {
        if (!((navGetStateFlags(posControl.navState) & NAV_AUTO_RTH)
          || ((navGetStateFlags(posControl.navState) & NAV_AUTO_WP) && posControl.waypointList[posControl.activeWaypointIndex].action == NAV_WP_ACTION_RTH))) {
            switch (navConfig()->general.flags.rth_climb_first_stage_mode) {
                case NAV_RTH_CLIMB_STAGE_AT_LEAST:
                    posControl.rthState.rthClimbStageAltitude = posControl.rthState.homePosition.pos.z + navConfig()->general.rth_climb_first_stage_altitude;
                    break;
                case NAV_RTH_CLIMB_STAGE_EXTRA:
                    posControl.rthState.rthClimbStageAltitude = posControl.actualState.abs.pos.z + navConfig()->general.rth_climb_first_stage_altitude;
                    break;
            }

            switch (navConfig()->general.flags.rth_alt_control_mode) {
                case NAV_RTH_NO_ALT:
                    posControl.rthState.rthInitialAltitude = posControl.actualState.abs.pos.z;
                    posControl.rthState.rthFinalAltitude = posControl.rthState.rthInitialAltitude;
                    break;

                case NAV_RTH_EXTRA_ALT: // Maintain current altitude + predefined safety margin
                    posControl.rthState.rthInitialAltitude = posControl.actualState.abs.pos.z + navConfig()->general.rth_altitude;
                    posControl.rthState.rthFinalAltitude = posControl.rthState.rthInitialAltitude;
                    break;

                case NAV_RTH_MAX_ALT:
                    posControl.rthState.rthInitialAltitude = MAX(posControl.rthState.rthInitialAltitude, posControl.actualState.abs.pos.z);
                    if (navConfig()->general.rth_altitude > 0) {
                        posControl.rthState.rthInitialAltitude = MAX(posControl.rthState.rthInitialAltitude, posControl.rthState.homePosition.pos.z + navConfig()->general.rth_altitude);
                    }
                    posControl.rthState.rthFinalAltitude = posControl.rthState.rthInitialAltitude;
                    break;

                case NAV_RTH_AT_LEAST_ALT:  // Climb to at least some predefined altitude above home
                    posControl.rthState.rthInitialAltitude = MAX(posControl.rthState.homePosition.pos.z + navConfig()->general.rth_altitude, posControl.actualState.abs.pos.z);
                    posControl.rthState.rthFinalAltitude = posControl.rthState.rthInitialAltitude;
                    break;

                case NAV_RTH_AT_LEAST_ALT_LINEAR_DESCENT:
                    posControl.rthState.rthInitialAltitude = MAX(posControl.rthState.homePosition.pos.z + navConfig()->general.rth_altitude, posControl.actualState.abs.pos.z);
                    posControl.rthState.rthFinalAltitude = posControl.rthState.homePosition.pos.z + navConfig()->general.rth_altitude;
                    break;

                case NAV_RTH_CONST_ALT:     // Climb/descend to predefined altitude above home
                default:
                    posControl.rthState.rthInitialAltitude = posControl.rthState.homePosition.pos.z + navConfig()->general.rth_altitude;
                    posControl.rthState.rthFinalAltitude = posControl.rthState.rthInitialAltitude;
            }
        }
    } else {
        posControl.rthState.rthClimbStageAltitude = posControl.actualState.abs.pos.z;
        posControl.rthState.rthInitialAltitude = posControl.actualState.abs.pos.z;
        posControl.rthState.rthFinalAltitude = posControl.actualState.abs.pos.z;
    }
}

/*-----------------------------------------------------------
 * RTH sanity test logic
 *-----------------------------------------------------------*/
void initializeRTHSanityChecker(void)
{
    const timeMs_t currentTimeMs = millis();

    posControl.rthSanityChecker.lastCheckTime = currentTimeMs;
    posControl.rthSanityChecker.rthSanityOK = true;
    posControl.rthSanityChecker.minimalDistanceToHome = calculateDistanceToDestination(&posControl.rthState.homePosition.pos);
}

bool validateRTHSanityChecker(void)
{
    const timeMs_t currentTimeMs = millis();

    // Ability to disable sanity checker
    if (navConfig()->general.rth_abort_threshold == 0) {
        return true;
    }

    // Check at 10Hz rate
    if ((currentTimeMs - posControl.rthSanityChecker.lastCheckTime) > 100) {
        const float currentDistanceToHome = calculateDistanceToDestination(&posControl.rthState.homePosition.pos);
        posControl.rthSanityChecker.lastCheckTime = currentTimeMs;

        if (currentDistanceToHome < posControl.rthSanityChecker.minimalDistanceToHome) {
            posControl.rthSanityChecker.minimalDistanceToHome = currentDistanceToHome;
        } else {
            // If while doing RTH we got even farther away from home - RTH is doing something crazy
            posControl.rthSanityChecker.rthSanityOK = (currentDistanceToHome - posControl.rthSanityChecker.minimalDistanceToHome) < navConfig()->general.rth_abort_threshold;
        }
    }

    return posControl.rthSanityChecker.rthSanityOK;
}

/*-----------------------------------------------------------
 * Reset home position to current position
 *-----------------------------------------------------------*/
void setHomePosition(const fpVector3_t * pos, int32_t yaw, navSetWaypointFlags_t useMask, navigationHomeFlags_t homeFlags)
{
    // XY-position
    if ((useMask & NAV_POS_UPDATE_XY) != 0) {
        posControl.rthState.homePosition.pos.x = pos->x;
        posControl.rthState.homePosition.pos.y = pos->y;
        if (homeFlags & NAV_HOME_VALID_XY) {
            posControl.rthState.homeFlags |= NAV_HOME_VALID_XY;
        } else {
            posControl.rthState.homeFlags &= ~NAV_HOME_VALID_XY;
        }
    }

    // Z-position
    if ((useMask & NAV_POS_UPDATE_Z) != 0) {
        posControl.rthState.homePosition.pos.z = pos->z;
        if (homeFlags & NAV_HOME_VALID_Z) {
            posControl.rthState.homeFlags |= NAV_HOME_VALID_Z;
        } else {
            posControl.rthState.homeFlags &= ~NAV_HOME_VALID_Z;
        }
    }

    // Heading
    if ((useMask & NAV_POS_UPDATE_HEADING) != 0) {
        // Heading
        posControl.rthState.homePosition.yaw = yaw;
        if (homeFlags & NAV_HOME_VALID_HEADING) {
            posControl.rthState.homeFlags |= NAV_HOME_VALID_HEADING;
        } else {
            posControl.rthState.homeFlags &= ~NAV_HOME_VALID_HEADING;
        }
    }

    posControl.homeDistance = 0;
    posControl.homeDirection = 0;

    // Update target RTH altitude as a waypoint above home
    updateDesiredRTHAltitude();

    //  Reset RTH sanity checker for new home position if RTH active
    if (FLIGHT_MODE(NAV_RTH_MODE)) {
        initializeRTHSanityChecker();
    }

    updateHomePositionCompatibility();
    ENABLE_STATE(GPS_FIX_HOME);
}

static navigationHomeFlags_t navigationActualStateHomeValidity(void)
{
    navigationHomeFlags_t flags = 0;

    if (posControl.flags.estPosStatus >= EST_USABLE) {
        flags |= NAV_HOME_VALID_XY | NAV_HOME_VALID_Z;
    }

    if (posControl.flags.estHeadingStatus >= EST_USABLE) {
        flags |= NAV_HOME_VALID_HEADING;
    }

    return flags;
}

#if defined(USE_SAFE_HOME)

void checkSafeHomeState(bool shouldBeEnabled)
{
    const bool safehomeNotApplicable = navConfig()->general.flags.safehome_usage_mode == SAFEHOME_USAGE_OFF ||
                                       posControl.flags.rthTrackbackActive ||
                                       (!safehome_applied && posControl.homeDistance < navConfig()->general.min_rth_distance);

	if (safehomeNotApplicable) {
		shouldBeEnabled = false;
	} else if (navConfig()->general.flags.safehome_usage_mode == SAFEHOME_USAGE_RTH_FS && shouldBeEnabled) {
		// if safehomes are only used with failsafe and we're trying to enable safehome
		// then enable the safehome only with failsafe
		shouldBeEnabled = posControl.flags.forcedRTHActivated;
	}
    // no safe homes found when arming or safehome feature in the correct state, then we don't need to do anything
	if (safehome_distance == 0 || (safehome_applied == shouldBeEnabled)) {
		return;
	}
    if (shouldBeEnabled) {
		// set home to safehome
        setHomePosition(&nearestSafeHome, 0, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING, navigationActualStateHomeValidity());
		safehome_applied = true;
	} else {
		// set home to original arming point
        setHomePosition(&original_rth_home, 0, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING, navigationActualStateHomeValidity());
		safehome_applied = false;
	}
	// if we've changed the home position, update the distance and direction
    updateHomePosition();
}

/***********************************************************
 *  See if there are any safehomes near where we are arming.
 *  If so, save the nearest one in case we need it later for RTH.
 **********************************************************/
bool findNearestSafeHome(void)
{
    safehome_index = -1;
    uint32_t nearest_safehome_distance = navConfig()->general.safehome_max_distance + 1;
    uint32_t distance_to_current;
    fpVector3_t currentSafeHome;
    gpsLocation_t shLLH;
    shLLH.alt = 0;
    for (uint8_t i = 0; i < MAX_SAFE_HOMES; i++) {
		if (!safeHomeConfig(i)->enabled)
		    continue;

        shLLH.lat = safeHomeConfig(i)->lat;
        shLLH.lon = safeHomeConfig(i)->lon;
        geoConvertGeodeticToLocal(&currentSafeHome, &posControl.gpsOrigin, &shLLH, GEO_ALT_RELATIVE);
        distance_to_current = calculateDistanceToDestination(&currentSafeHome);
        if (distance_to_current < nearest_safehome_distance) {
             // this safehome is the nearest so far - keep track of it.
             safehome_index = i;
             nearest_safehome_distance = distance_to_current;
             nearestSafeHome.x = currentSafeHome.x;
             nearestSafeHome.y = currentSafeHome.y;
             nearestSafeHome.z = currentSafeHome.z;
        }
    }
    if (safehome_index >= 0) {
		safehome_distance = nearest_safehome_distance;
    } else {
        safehome_distance = 0;
    }
    return safehome_distance > 0;
}
#endif

/*-----------------------------------------------------------
 * Update home position, calculate distance and bearing to home
 *-----------------------------------------------------------*/
void updateHomePosition(void)
{
    // Disarmed and have a valid position, constantly update home
    if (!ARMING_FLAG(ARMED)) {
        if (posControl.flags.estPosStatus >= EST_USABLE) {
            const navigationHomeFlags_t validHomeFlags = NAV_HOME_VALID_XY | NAV_HOME_VALID_Z;
            bool setHome = (posControl.rthState.homeFlags & validHomeFlags) != validHomeFlags;
            switch ((nav_reset_type_e)positionEstimationConfig()->reset_home_type) {
                case NAV_RESET_NEVER:
                    break;
                case NAV_RESET_ON_FIRST_ARM:
                    setHome |= !ARMING_FLAG(WAS_EVER_ARMED);
                    break;
                case NAV_RESET_ON_EACH_ARM:
                    setHome = true;
                    break;
            }
            if (setHome) {
#if defined(USE_SAFE_HOME)
                findNearestSafeHome();
#endif
                setHomePosition(&posControl.actualState.abs.pos, posControl.actualState.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING, navigationActualStateHomeValidity());
                // save the current location in case it is replaced by a safehome or HOME_RESET
                original_rth_home.x = posControl.rthState.homePosition.pos.x;
                original_rth_home.y = posControl.rthState.homePosition.pos.y;
                original_rth_home.z = posControl.rthState.homePosition.pos.z;
            }
        }
    }
    else {
        static bool isHomeResetAllowed = false;

        // If pilot so desires he may reset home position to current position
        if (IS_RC_MODE_ACTIVE(BOXHOMERESET)) {
            if (isHomeResetAllowed && !FLIGHT_MODE(FAILSAFE_MODE) && !FLIGHT_MODE(NAV_RTH_MODE) && !FLIGHT_MODE(NAV_WP_MODE) && (posControl.flags.estPosStatus >= EST_USABLE)) {
                const navSetWaypointFlags_t homeUpdateFlags = STATE(GPS_FIX_HOME) ? (NAV_POS_UPDATE_XY | NAV_POS_UPDATE_HEADING) : (NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING);
                setHomePosition(&posControl.actualState.abs.pos, posControl.actualState.yaw, homeUpdateFlags, navigationActualStateHomeValidity());
                isHomeResetAllowed = false;
            }
        }
        else {
            isHomeResetAllowed = true;
        }

        // Update distance and direction to home if armed (home is not updated when armed)
        if (STATE(GPS_FIX_HOME)) {
            fpVector3_t * tmpHomePos = rthGetHomeTargetPosition(RTH_HOME_FINAL_LAND);
            posControl.homeDistance = calculateDistanceToDestination(tmpHomePos);
            posControl.homeDirection = calculateBearingToDestination(tmpHomePos);
            updateHomePositionCompatibility();
        }
    }
}

/* -----------------------------------------------------------
 * Override RTH preset altitude and Climb First option
 * using Pitch/Roll stick held for > 1 seconds
 * Climb First override limited to Fixed Wing only
 * Roll also cancels RTH trackback on Fixed Wing and Multirotor
 *-----------------------------------------------------------*/
static bool rthAltControlStickOverrideCheck(unsigned axis)
{
    if (!navConfig()->general.flags.rth_alt_control_override || posControl.flags.forcedRTHActivated ||
        (axis == ROLL && STATE(MULTIROTOR) && !posControl.flags.rthTrackbackActive)) {
        return false;
    }
    static timeMs_t rthOverrideStickHoldStartTime[2];

    if (rxGetChannelValue(axis) > rxConfig()->maxcheck) {
        timeDelta_t holdTime = millis() - rthOverrideStickHoldStartTime[axis];

        if (!rthOverrideStickHoldStartTime[axis]) {
            rthOverrideStickHoldStartTime[axis] = millis();
        } else if (ABS(1500 - holdTime) < 500) {    // 1s delay to activate, activation duration limited to 1 sec
            if (axis == PITCH) {           // PITCH down to override preset altitude reset to current altitude
                posControl.rthState.rthInitialAltitude = posControl.actualState.abs.pos.z;
                posControl.rthState.rthFinalAltitude = posControl.rthState.rthInitialAltitude;
                return true;
            } else if (axis == ROLL) {     // ROLL right to override climb first
                return true;
            }
        }
    } else {
        rthOverrideStickHoldStartTime[axis] = 0;
    }

    return false;
}

/* ---------------------------------------------------
 * If climb stage is being used, see if it is time to
 * transiton in to turn.
 * Limited to fixed wing only.
 * --------------------------------------------------- */
 bool rthClimbStageActiveAndComplete() {
    if ((STATE(FIXED_WING_LEGACY) || STATE(AIRPLANE)) && (navConfig()->general.rth_climb_first_stage_altitude > 0)) {
        if (posControl.actualState.abs.pos.z >= posControl.rthState.rthClimbStageAltitude) {
            return true;
        }
    }

    return false;
 }

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
 static void updateRthTrackback(bool forceSaveTrackPoint)
{
    static bool suspendTracking = false;
    bool fwLoiterIsActive = STATE(AIRPLANE) && (NAV_Status.state == MW_NAV_STATE_HOLD_TIMED || FLIGHT_MODE(NAV_POSHOLD_MODE));
    if (!fwLoiterIsActive && suspendTracking) {
        suspendTracking = false;
    }

    if (navConfig()->general.flags.rth_trackback_mode == RTH_TRACKBACK_OFF || FLIGHT_MODE(NAV_RTH_MODE) || !ARMING_FLAG(ARMED || suspendTracking)) {
        return;
    }

    // Record trackback points based on significant change in course/altitude until
    // points limit reached. Overwrite older points from then on.
    if (posControl.flags.estPosStatus >= EST_USABLE && posControl.flags.estAltStatus >= EST_USABLE) {
        static int32_t previousTBTripDist;      // cm
        static int16_t previousTBCourse;        // degrees
        static int16_t previousTBAltitude;      // meters
        static uint8_t distanceCounter = 0;
        bool saveTrackpoint = forceSaveTrackPoint;
        bool GPSCourseIsValid = isGPSHeadingValid();

        // start recording when some distance from home, 50m seems reasonable.
        if (posControl.activeRthTBPointIndex < 0) {
            saveTrackpoint = posControl.homeDistance > METERS_TO_CENTIMETERS(50);

            previousTBCourse = CENTIDEGREES_TO_DEGREES(posControl.actualState.yaw);
            previousTBTripDist = posControl.totalTripDistance;
        } else {
            // Minimum distance increment between course change track points when GPS course valid - set to 10m
            const bool distanceIncrement = posControl.totalTripDistance - previousTBTripDist > METERS_TO_CENTIMETERS(10);

            // Altitude change
            if (ABS(previousTBAltitude - CENTIMETERS_TO_METERS(posControl.actualState.abs.pos.z)) > 10) {   // meters
                saveTrackpoint = true;
            } else if (distanceIncrement && GPSCourseIsValid) {
                // Course change - set to 45 degrees
                if (ABS(wrap_18000(DEGREES_TO_CENTIDEGREES(DECIDEGREES_TO_DEGREES(gpsSol.groundCourse) - previousTBCourse))) > DEGREES_TO_CENTIDEGREES(45)) {
                    saveTrackpoint = true;
                } else if (distanceCounter >= 9) {
                    // Distance based trackpoint logged if at least 10 distance increments occur without altitude or course change
                    // and deviation from projected course path > 20m
                    float distToPrevPoint = calculateDistanceToDestination(&posControl.rthTBPointsList[posControl.activeRthTBPointIndex]);

                    fpVector3_t virtualCoursePoint;
                    virtualCoursePoint.x = posControl.rthTBPointsList[posControl.activeRthTBPointIndex].x + distToPrevPoint * cos_approx(DEGREES_TO_RADIANS(previousTBCourse));
                    virtualCoursePoint.y = posControl.rthTBPointsList[posControl.activeRthTBPointIndex].y + distToPrevPoint * sin_approx(DEGREES_TO_RADIANS(previousTBCourse));

                    saveTrackpoint = calculateDistanceToDestination(&virtualCoursePoint) > METERS_TO_CENTIMETERS(20);
                }
                distanceCounter++;
                previousTBTripDist = posControl.totalTripDistance;
            } else if (!GPSCourseIsValid) {
                // if no reliable course revert to basic distance logging based on direct distance from last point - set to 20m
                saveTrackpoint = calculateDistanceToDestination(&posControl.rthTBPointsList[posControl.activeRthTBPointIndex]) > METERS_TO_CENTIMETERS(20);
                previousTBTripDist = posControl.totalTripDistance;
            }

            // Suspend tracking during loiter on fixed wing. Save trackpoint at start of loiter.
            if (distanceCounter && fwLoiterIsActive) {
                saveTrackpoint = suspendTracking = true;
            }
        }

        // when trackpoint store full, overwrite from start of store using 'rthTBWrapAroundCounter' to track overwrite position
        if (saveTrackpoint) {
            if (posControl.activeRthTBPointIndex == (NAV_RTH_TRACKBACK_POINTS - 1)) {   // wraparound to start
                posControl.rthTBWrapAroundCounter = posControl.activeRthTBPointIndex = 0;
            } else {
                posControl.activeRthTBPointIndex++;
                if (posControl.rthTBWrapAroundCounter > -1) {   // track wraparound overwrite position after store first filled
                    posControl.rthTBWrapAroundCounter = posControl.activeRthTBPointIndex;
                }
            }
            posControl.rthTBPointsList[posControl.activeRthTBPointIndex] = posControl.actualState.abs.pos;

            posControl.rthTBLastSavedIndex = posControl.activeRthTBPointIndex;
            previousTBAltitude = CENTIMETERS_TO_METERS(posControl.actualState.abs.pos.z);
            previousTBCourse = GPSCourseIsValid ? DECIDEGREES_TO_DEGREES(gpsSol.groundCourse) : previousTBCourse;
            distanceCounter = 0;
        }
    }
}

static fpVector3_t * rthGetTrackbackPos(void)
{
    // ensure trackback altitude never lower than altitude of start point
    if (posControl.rthTBPointsList[posControl.activeRthTBPointIndex].z < posControl.rthTBPointsList[posControl.rthTBLastSavedIndex].z) {
        posControl.rthTBPointsList[posControl.activeRthTBPointIndex].z = posControl.rthTBPointsList[posControl.rthTBLastSavedIndex].z;
    }

    return &posControl.rthTBPointsList[posControl.activeRthTBPointIndex];
}

/*-----------------------------------------------------------
 * Update flight statistics
 *-----------------------------------------------------------*/
static void updateNavigationFlightStatistics(void)
{
    static timeMs_t previousTimeMs = 0;
    const timeMs_t currentTimeMs = millis();
    const timeDelta_t timeDeltaMs = currentTimeMs - previousTimeMs;
    previousTimeMs = currentTimeMs;

    if (ARMING_FLAG(ARMED)) {
        posControl.totalTripDistance += posControl.actualState.velXY * MS2S(timeDeltaMs);
    }
}

uint32_t getTotalTravelDistance(void)
{
    return lrintf(posControl.totalTripDistance);
}

/*-----------------------------------------------------------
 * Calculate platform-specific hold position (account for deceleration)
 *-----------------------------------------------------------*/
void calculateInitialHoldPosition(fpVector3_t * pos)
{
    if (STATE(FIXED_WING_LEGACY)) { // FIXED_WING_LEGACY
        calculateFixedWingInitialHoldPosition(pos);
    }
    else {
        calculateMulticopterInitialHoldPosition(pos);
    }
}

/*-----------------------------------------------------------
 * Set active XYZ-target and desired heading
 *-----------------------------------------------------------*/
void setDesiredPosition(const fpVector3_t * pos, int32_t yaw, navSetWaypointFlags_t useMask)
{
    // XY-position update is allowed only when not braking in NAV_CRUISE_BRAKING
    if ((useMask & NAV_POS_UPDATE_XY) != 0 && !STATE(NAV_CRUISE_BRAKING)) {
        posControl.desiredState.pos.x = pos->x;
        posControl.desiredState.pos.y = pos->y;
    }

    // Z-position
    if ((useMask & NAV_POS_UPDATE_Z) != 0) {
        updateClimbRateToAltitudeController(0, ROC_TO_ALT_RESET);   // Reset RoC/RoD -> altitude controller
        posControl.desiredState.pos.z = pos->z;
    }

    // Heading
    if ((useMask & NAV_POS_UPDATE_HEADING) != 0) {
        // Heading
        posControl.desiredState.yaw = yaw;
    }
    else if ((useMask & NAV_POS_UPDATE_BEARING) != 0) {
        posControl.desiredState.yaw = calculateBearingToDestination(pos);
    }
    else if ((useMask & NAV_POS_UPDATE_BEARING_TAIL_FIRST) != 0) {
        posControl.desiredState.yaw = wrap_36000(calculateBearingToDestination(pos) - 18000);
    }
}

void calculateFarAwayTarget(fpVector3_t * farAwayPos, int32_t yaw, int32_t distance)
{
    farAwayPos->x = navGetCurrentActualPositionAndVelocity()->pos.x + distance * cos_approx(CENTIDEGREES_TO_RADIANS(yaw));
    farAwayPos->y = navGetCurrentActualPositionAndVelocity()->pos.y + distance * sin_approx(CENTIDEGREES_TO_RADIANS(yaw));
    farAwayPos->z = navGetCurrentActualPositionAndVelocity()->pos.z;
}

void calculateNewCruiseTarget(fpVector3_t * origin, int32_t yaw, int32_t distance)
{
    origin->x = origin->x + distance * cos_approx(CENTIDEGREES_TO_RADIANS(yaw));
    origin->y = origin->y + distance * sin_approx(CENTIDEGREES_TO_RADIANS(yaw));
    origin->z = origin->z;
}

/*-----------------------------------------------------------
 * NAV land detector
 *-----------------------------------------------------------*/
void updateLandingStatus(void)
{
    if (STATE(AIRPLANE) && !navConfig()->general.flags.disarm_on_landing) {
        return;     // no point using this with a fixed wing if not set to disarm
    }

    static bool landingDetectorIsActive;

    DEBUG_SET(DEBUG_LANDING, 0, landingDetectorIsActive);
    DEBUG_SET(DEBUG_LANDING, 1, STATE(LANDING_DETECTED));

    if (!ARMING_FLAG(ARMED)) {
        resetLandingDetector();
        landingDetectorIsActive = false;
        if (!IS_RC_MODE_ACTIVE(BOXARM)) {
            DISABLE_ARMING_FLAG(ARMING_DISABLED_LANDING_DETECTED);
        }
        return;
    }

    if (!landingDetectorIsActive) {
        if (isFlightDetected()) {
            landingDetectorIsActive = true;
            resetLandingDetector();
        }
    } else if (STATE(LANDING_DETECTED)) {
        pidResetErrorAccumulators();
        if (navConfig()->general.flags.disarm_on_landing) {
            ENABLE_ARMING_FLAG(ARMING_DISABLED_LANDING_DETECTED);
            disarm(DISARM_LANDING);
        } else if (!navigationIsFlyingAutonomousMode()) {
            // for multirotor only - reactivate landing detector without disarm when throttle raised toward hover throttle
            landingDetectorIsActive = rxGetChannelValue(THROTTLE) < (0.5 * (currentBatteryProfile->nav.mc.hover_throttle + getThrottleIdleValue()));
        }
    } else if (isLandingDetected()) {
        ENABLE_STATE(LANDING_DETECTED);
    }
}

bool isLandingDetected(void)
{
    return STATE(AIRPLANE) ? isFixedWingLandingDetected() : isMulticopterLandingDetected();
}

void resetLandingDetector(void)
{
    DISABLE_STATE(LANDING_DETECTED);
    posControl.flags.resetLandingDetector = true;
}

bool isFlightDetected(void)
{
    return STATE(AIRPLANE) ? isFixedWingFlying() : isMulticopterFlying();
}

/*-----------------------------------------------------------
 * Z-position controller
 *-----------------------------------------------------------*/
void updateClimbRateToAltitudeController(float desiredClimbRate, climbRateToAltitudeControllerMode_e mode)
{
    static timeUs_t lastUpdateTimeUs;
    timeUs_t currentTimeUs = micros();

    // Terrain following uses different altitude measurement
    const float altitudeToUse = navGetCurrentActualPositionAndVelocity()->pos.z;

    if (mode == ROC_TO_ALT_RESET) {
        lastUpdateTimeUs = currentTimeUs;
        posControl.desiredState.pos.z = altitudeToUse;
    }
    else {      // ROC_TO_ALT_NORMAL

        /*
         * If max altitude is set, reset climb rate if altitude is reached and climb rate is > 0
         * In other words, when altitude is reached, allow it only to shrink
         */
        if (navConfig()->general.max_altitude > 0 &&
            altitudeToUse >= navConfig()->general.max_altitude &&
            desiredClimbRate > 0
        ) {
            desiredClimbRate = 0;
        }

        if (STATE(FIXED_WING_LEGACY)) {
            // Fixed wing climb rate controller is open-loop. We simply move the known altitude target
            float timeDelta = US2S(currentTimeUs - lastUpdateTimeUs);

            if (timeDelta <= HZ2S(MIN_POSITION_UPDATE_RATE_HZ)) {
                posControl.desiredState.pos.z += desiredClimbRate * timeDelta;
                posControl.desiredState.pos.z = constrainf(posControl.desiredState.pos.z, altitudeToUse - 500, altitudeToUse + 500);    // FIXME: calculate sanity limits in a smarter way
            }
        }
        else {
            // Multicopter climb-rate control is closed-loop, it's possible to directly calculate desired altitude setpoint to yield the required RoC/RoD
            posControl.desiredState.pos.z = altitudeToUse + (desiredClimbRate / posControl.pids.pos[Z].param.kP);
        }

        lastUpdateTimeUs = currentTimeUs;
    }
}

static void resetAltitudeController(bool useTerrainFollowing)
{
    // Set terrain following flag
    posControl.flags.isTerrainFollowEnabled = useTerrainFollowing;

    if (STATE(FIXED_WING_LEGACY)) {
        resetFixedWingAltitudeController();
    }
    else {
        resetMulticopterAltitudeController();
    }
}

static void setupAltitudeController(void)
{
    if (STATE(FIXED_WING_LEGACY)) {
        setupFixedWingAltitudeController();
    }
    else {
        setupMulticopterAltitudeController();
    }
}

static bool adjustAltitudeFromRCInput(void)
{
    if (STATE(FIXED_WING_LEGACY)) {
        return adjustFixedWingAltitudeFromRCInput();
    }
    else {
        return adjustMulticopterAltitudeFromRCInput();
    }
}

/*-----------------------------------------------------------
 * Jump Counter support functions
 *-----------------------------------------------------------*/
static void setupJumpCounters(void)
{
    for (uint8_t wp = posControl.startWpIndex; wp < posControl.waypointCount + posControl.startWpIndex; wp++) {
        if (posControl.waypointList[wp].action == NAV_WP_ACTION_JUMP){
            posControl.waypointList[wp].p3 = posControl.waypointList[wp].p2;
        }
    }
}

static void resetJumpCounter(void)
{
        // reset the volatile counter from the set / static value
    posControl.waypointList[posControl.activeWaypointIndex].p3 = posControl.waypointList[posControl.activeWaypointIndex].p2;
}

static void clearJumpCounters(void)
{
    for (uint8_t wp = posControl.startWpIndex; wp < posControl.waypointCount + posControl.startWpIndex; wp++) {
        if (posControl.waypointList[wp].action == NAV_WP_ACTION_JUMP) {
            posControl.waypointList[wp].p3 = 0;
        }
    }
}



/*-----------------------------------------------------------
 * Heading controller (pass-through to MAG mode)
 *-----------------------------------------------------------*/
static void resetHeadingController(void)
{
    if (STATE(FIXED_WING_LEGACY)) {
        resetFixedWingHeadingController();
    }
    else {
        resetMulticopterHeadingController();
    }
}

static bool adjustHeadingFromRCInput(void)
{
    if (STATE(FIXED_WING_LEGACY)) {
        return adjustFixedWingHeadingFromRCInput();
    }
    else {
        return adjustMulticopterHeadingFromRCInput();
    }
}

/*-----------------------------------------------------------
 * XY Position controller
 *-----------------------------------------------------------*/
static void resetPositionController(void)
{
    if (STATE(FIXED_WING_LEGACY)) {
        resetFixedWingPositionController();
    }
    else {
        resetMulticopterPositionController();
        resetMulticopterBrakingMode();
    }
}

static bool adjustPositionFromRCInput(void)
{
    bool retValue;

    if (STATE(FIXED_WING_LEGACY)) {
        retValue = adjustFixedWingPositionFromRCInput();
    }
    else {

        const int16_t rcPitchAdjustment = applyDeadbandRescaled(rcCommand[PITCH], rcControlsConfig()->pos_hold_deadband, -500, 500);
        const int16_t rcRollAdjustment = applyDeadbandRescaled(rcCommand[ROLL], rcControlsConfig()->pos_hold_deadband, -500, 500);

        retValue = adjustMulticopterPositionFromRCInput(rcPitchAdjustment, rcRollAdjustment);
    }

    return retValue;
}

/*-----------------------------------------------------------
 * WP controller
 *-----------------------------------------------------------*/
void resetGCSFlags(void)
{
    posControl.flags.isGCSAssistedNavigationReset = false;
    posControl.flags.isGCSAssistedNavigationEnabled = false;
}

void getWaypoint(uint8_t wpNumber, navWaypoint_t * wpData)
{
    /* Default waypoint to send */
    wpData->action = NAV_WP_ACTION_RTH;
    wpData->lat = 0;
    wpData->lon = 0;
    wpData->alt = 0;
    wpData->p1 = 0;
    wpData->p2 = 0;
    wpData->p3 = 0;
    wpData->flag = NAV_WP_FLAG_LAST;

    // WP #0 - special waypoint - HOME
    if (wpNumber == 0) {
        if (STATE(GPS_FIX_HOME)) {
            wpData->lat = GPS_home.lat;
            wpData->lon = GPS_home.lon;
            wpData->alt = GPS_home.alt;
        }
    }
    // WP #255 - special waypoint - directly get actualPosition
    else if (wpNumber == 255) {
        gpsLocation_t wpLLH;

        geoConvertLocalToGeodetic(&wpLLH, &posControl.gpsOrigin, &navGetCurrentActualPositionAndVelocity()->pos);

        wpData->lat = wpLLH.lat;
        wpData->lon = wpLLH.lon;
        wpData->alt = wpLLH.alt;
    }
    // WP #254 - special waypoint - get desiredPosition that was set by ground control station if in 3D-guided mode
    else if (wpNumber == 254) {
        navigationFSMStateFlags_t navStateFlags = navGetStateFlags(posControl.navState);

        if ((posControl.gpsOrigin.valid) && (navStateFlags & NAV_CTL_ALT) && (navStateFlags & NAV_CTL_POS)) {
            gpsLocation_t wpLLH;

            geoConvertLocalToGeodetic(&wpLLH, &posControl.gpsOrigin, &posControl.desiredState.pos);

            wpData->lat = wpLLH.lat;
            wpData->lon = wpLLH.lon;
            wpData->alt = wpLLH.alt;
        }
    }
    // WP #1 - #60 - common waypoints - pre-programmed mission
    else if ((wpNumber >= 1) && (wpNumber <= NAV_MAX_WAYPOINTS)) {
        if (wpNumber <= getWaypointCount()) {
            *wpData = posControl.waypointList[wpNumber - 1 + (ARMING_FLAG(ARMED) ? posControl.startWpIndex : 0)];
            if(wpData->action == NAV_WP_ACTION_JUMP) {
                wpData->p1 += 1; // make WP # (vice index)
            }
        }
    }
}

void setWaypoint(uint8_t wpNumber, const navWaypoint_t * wpData)
{
    gpsLocation_t wpLLH;
    navWaypointPosition_t wpPos;

    // Pre-fill structure to convert to local coordinates
    wpLLH.lat = wpData->lat;
    wpLLH.lon = wpData->lon;
    wpLLH.alt = wpData->alt;

    // WP #0 - special waypoint - HOME
    if ((wpNumber == 0) && ARMING_FLAG(ARMED) && (posControl.flags.estPosStatus >= EST_USABLE) && posControl.gpsOrigin.valid && posControl.flags.isGCSAssistedNavigationEnabled) {
        // Forcibly set home position. Note that this is only valid if already armed, otherwise home will be reset instantly
        geoConvertGeodeticToLocal(&wpPos.pos, &posControl.gpsOrigin, &wpLLH, GEO_ALT_RELATIVE);
        setHomePosition(&wpPos.pos, 0, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING, NAV_HOME_VALID_ALL);
    }
    // WP #255 - special waypoint - directly set desiredPosition
    // Only valid when armed and in poshold mode
    else if ((wpNumber == 255) && (wpData->action == NAV_WP_ACTION_WAYPOINT) &&
             ARMING_FLAG(ARMED) && (posControl.flags.estPosStatus == EST_TRUSTED) && posControl.gpsOrigin.valid && posControl.flags.isGCSAssistedNavigationEnabled &&
             (posControl.navState == NAV_STATE_POSHOLD_3D_IN_PROGRESS)) {
        // Convert to local coordinates
        geoConvertGeodeticToLocal(&wpPos.pos, &posControl.gpsOrigin, &wpLLH, GEO_ALT_RELATIVE);

        navSetWaypointFlags_t waypointUpdateFlags = NAV_POS_UPDATE_XY;

        // If we received global altitude == 0, use current altitude
        if (wpData->alt != 0) {
            waypointUpdateFlags |= NAV_POS_UPDATE_Z;
        }

        if (wpData->p1 > 0 && wpData->p1 < 360) {
            waypointUpdateFlags |= NAV_POS_UPDATE_HEADING;
        }

        setDesiredPosition(&wpPos.pos, DEGREES_TO_CENTIDEGREES(wpData->p1), waypointUpdateFlags);
    }
    // WP #1 - #NAV_MAX_WAYPOINTS - common waypoints - pre-programmed mission
    else if ((wpNumber >= 1) && (wpNumber <= NAV_MAX_WAYPOINTS) && !ARMING_FLAG(ARMED)) {
        if (wpData->action == NAV_WP_ACTION_WAYPOINT || wpData->action == NAV_WP_ACTION_JUMP || wpData->action == NAV_WP_ACTION_RTH || wpData->action == NAV_WP_ACTION_HOLD_TIME || wpData->action == NAV_WP_ACTION_LAND || wpData->action == NAV_WP_ACTION_SET_POI || wpData->action == NAV_WP_ACTION_SET_HEAD ) {
            // Only allow upload next waypoint (continue upload mission) or first waypoint (new mission)
            static int8_t nonGeoWaypointCount = 0;

            if (wpNumber == (posControl.waypointCount + 1) || wpNumber == 1) {
                if (wpNumber == 1) {
                    resetWaypointList();
                }
                posControl.waypointList[wpNumber - 1] = *wpData;
                if(wpData->action == NAV_WP_ACTION_SET_POI || wpData->action == NAV_WP_ACTION_SET_HEAD || wpData->action == NAV_WP_ACTION_JUMP) {
                    nonGeoWaypointCount += 1;
                    if(wpData->action == NAV_WP_ACTION_JUMP) {
                        posControl.waypointList[wpNumber - 1].p1 -= 1; // make index (vice WP #)
                    }
                }

                posControl.waypointCount = wpNumber;
                posControl.waypointListValid = (wpData->flag == NAV_WP_FLAG_LAST);
                posControl.geoWaypointCount = posControl.waypointCount - nonGeoWaypointCount;
                if (posControl.waypointListValid) {
                    nonGeoWaypointCount = 0;
                }
            }
        }
    }
}

void resetWaypointList(void)
{
    posControl.waypointCount = 0;
    posControl.waypointListValid = false;
    posControl.geoWaypointCount = 0;
    posControl.startWpIndex = 0;
#ifdef USE_MULTI_MISSION
    posControl.totalMultiMissionWpCount = 0;
    posControl.loadedMultiMissionIndex = 0;
    posControl.multiMissionCount = 0;
#endif
}

bool isWaypointListValid(void)
{
    return posControl.waypointListValid;
}

int getWaypointCount(void)
{
    uint8_t waypointCount = posControl.waypointCount;
#ifdef USE_MULTI_MISSION
    if (!ARMING_FLAG(ARMED) && posControl.totalMultiMissionWpCount) {
        waypointCount = posControl.totalMultiMissionWpCount;
    }
#endif
    return waypointCount;
}

#ifdef USE_MULTI_MISSION
void selectMultiMissionIndex(int8_t increment)
{
    if (posControl.multiMissionCount > 1) {     // stick selection only active when multi mission loaded
        navConfigMutable()->general.waypoint_multi_mission_index = constrain(navConfigMutable()->general.waypoint_multi_mission_index + increment, 1, posControl.multiMissionCount);
    }
}

void loadSelectedMultiMission(uint8_t missionIndex)
{
    uint8_t missionCount = 1;
    posControl.waypointCount = 0;
    posControl.geoWaypointCount = 0;

    for (int i = 0; i < NAV_MAX_WAYPOINTS; i++) {
        if ((missionCount == missionIndex)) {
            /* store details of selected mission: start wp index, mission wp count, geo wp count */
            if (!(posControl.waypointList[i].action == NAV_WP_ACTION_SET_POI ||
                    posControl.waypointList[i].action == NAV_WP_ACTION_SET_HEAD ||
                        posControl.waypointList[i].action == NAV_WP_ACTION_JUMP)) {
                posControl.geoWaypointCount++;
            }
            // mission start WP
            if (posControl.waypointCount == 0) {
                posControl.waypointCount = 1;   // start marker only, value unimportant (but not 0)
                posControl.startWpIndex = i;
            }
            // mission end WP
            if (posControl.waypointList[i].flag == NAV_WP_FLAG_LAST) {
                posControl.waypointCount = i - posControl.startWpIndex + 1;
                break;
            }
        } else if (posControl.waypointList[i].flag == NAV_WP_FLAG_LAST) {
            missionCount++;
        }
    }

    posControl.loadedMultiMissionIndex = posControl.multiMissionCount ? missionIndex : 0;
    posControl.activeWaypointIndex = posControl.startWpIndex;
}

bool updateWpMissionChange(void)
{
    /* Function only called when ARMED */

    if (posControl.multiMissionCount < 2 || posControl.wpPlannerActiveWPIndex || FLIGHT_MODE(NAV_WP_MODE)) {
        return true;
    }

    uint8_t setMissionIndex = navConfig()->general.waypoint_multi_mission_index;
    if (!(IS_RC_MODE_ACTIVE(BOXCHANGEMISSION) || isAdjustmentFunctionSelected(ADJUSTMENT_NAV_WP_MULTI_MISSION_INDEX))) {
        /* reload mission if mission index changed */
        if (posControl.loadedMultiMissionIndex != setMissionIndex) {
            loadSelectedMultiMission(setMissionIndex);
        }
        return true;
    }

    static bool toggleFlag = false;
    if (IS_RC_MODE_ACTIVE(BOXNAVWP) && toggleFlag) {
        if (setMissionIndex == posControl.multiMissionCount) {
            navConfigMutable()->general.waypoint_multi_mission_index = 1;
        } else {
            selectMultiMissionIndex(1);
        }
        toggleFlag = false;
    } else if (!IS_RC_MODE_ACTIVE(BOXNAVWP)) {
        toggleFlag = true;
    }
    return false;   // block WP mode while changing mission when armed
}

bool checkMissionCount(int8_t waypoint)
{
    if (nonVolatileWaypointList(waypoint)->flag == NAV_WP_FLAG_LAST) {
        posControl.multiMissionCount += 1;  // count up no missions in multi mission WP file
        if (waypoint != NAV_MAX_WAYPOINTS - 1) {
            return (nonVolatileWaypointList(waypoint + 1)->flag == NAV_WP_FLAG_LAST &&
                    nonVolatileWaypointList(waypoint + 1)->action ==NAV_WP_ACTION_RTH);
            // end of multi mission file if successive NAV_WP_FLAG_LAST and default action (RTH)
        }
    }
    return false;
}
#endif  // multi mission
#ifdef NAV_NON_VOLATILE_WAYPOINT_STORAGE
bool loadNonVolatileWaypointList(bool clearIfLoaded)
{
    /* Don't load if armed or mission planner active */
    if (ARMING_FLAG(ARMED) || posControl.wpPlannerActiveWPIndex) {
        return false;
    }

    // if forced and waypoints are already loaded, just unload them.
    if (clearIfLoaded && posControl.waypointCount > 0) {
        resetWaypointList();
        return false;
    }
#ifdef USE_MULTI_MISSION
    /* Reset multi mission index to 1 if exceeds number of available missions */
    if (navConfig()->general.waypoint_multi_mission_index > posControl.multiMissionCount) {
        navConfigMutable()->general.waypoint_multi_mission_index = 1;
    }
#endif
    for (int i = 0; i < NAV_MAX_WAYPOINTS; i++) {
        setWaypoint(i + 1, nonVolatileWaypointList(i));
#ifdef USE_MULTI_MISSION
        /* count up number of missions and exit after last multi mission */
        if (checkMissionCount(i)) {
            break;
        }
    }
    posControl.totalMultiMissionWpCount = posControl.waypointCount;
    loadSelectedMultiMission(navConfig()->general.waypoint_multi_mission_index);

    /* Mission sanity check failed - reset the list
     * Also reset if no selected mission loaded (shouldn't happen) */
    if (!posControl.waypointListValid || !posControl.waypointCount) {
#else
        // check this is the last waypoint
        if (nonVolatileWaypointList(i)->flag == NAV_WP_FLAG_LAST) {
            break;
        }
    }

    // Mission sanity check failed - reset the list
    if (!posControl.waypointListValid) {
#endif
        resetWaypointList();
    }

    return posControl.waypointListValid;
}

bool saveNonVolatileWaypointList(void)
{
    if (ARMING_FLAG(ARMED) || !posControl.waypointListValid)
        return false;

    for (int i = 0; i < NAV_MAX_WAYPOINTS; i++) {
        getWaypoint(i + 1, nonVolatileWaypointListMutable(i));
    }
#ifdef USE_MULTI_MISSION
    navConfigMutable()->general.waypoint_multi_mission_index = 1;    // reset selected mission to 1 when new entries saved
#endif
    saveConfigAndNotify();

    return true;
}
#endif

#if defined(USE_SAFE_HOME)

void resetSafeHomes(void)
{
    memset(safeHomeConfigMutable(0), 0, sizeof(navSafeHome_t) * MAX_SAFE_HOMES);
}
#endif

static void mapWaypointToLocalPosition(fpVector3_t * localPos, const navWaypoint_t * waypoint, geoAltitudeConversionMode_e altConv)
{
    gpsLocation_t wpLLH;

    /* Default to home position if lat & lon = 0 or HOME flag set
     * Applicable to WAYPOINT, HOLD_TIME & LANDING WP types */
    if ((waypoint->lat == 0 && waypoint->lon == 0) || waypoint->flag == NAV_WP_FLAG_HOME) {
        wpLLH.lat = GPS_home.lat;
        wpLLH.lon = GPS_home.lon;
    } else {
        wpLLH.lat = waypoint->lat;
        wpLLH.lon = waypoint->lon;
    }
    wpLLH.alt = waypoint->alt;

    geoConvertGeodeticToLocal(localPos, &posControl.gpsOrigin, &wpLLH, altConv);
}

static void calculateAndSetActiveWaypointToLocalPosition(const fpVector3_t * pos)
{
    // Calculate bearing towards waypoint and store it in waypoint yaw parameter (this will further be used to detect missed waypoints)
    if (isWaypointNavTrackingActive() && !(posControl.activeWaypoint.pos.x == pos->x && posControl.activeWaypoint.pos.y == pos->y)) {
        posControl.activeWaypoint.yaw = calculateBearingBetweenLocalPositions(&posControl.activeWaypoint.pos, pos);
    } else {
        posControl.activeWaypoint.yaw = calculateBearingToDestination(pos);
    }
    posControl.activeWaypoint.nextTurnAngle = -1;     // no turn angle set (-1), will be set by WP mode as required

    posControl.activeWaypoint.pos = *pos;

    // Set desired position to next waypoint (XYZ-controller)
    setDesiredPosition(&posControl.activeWaypoint.pos, posControl.activeWaypoint.yaw, NAV_POS_UPDATE_XY | NAV_POS_UPDATE_Z | NAV_POS_UPDATE_HEADING);
}

geoAltitudeConversionMode_e waypointMissionAltConvMode(geoAltitudeDatumFlag_e datumFlag)
{
    return ((datumFlag & NAV_WP_MSL_DATUM) == NAV_WP_MSL_DATUM) ? GEO_ALT_ABSOLUTE : GEO_ALT_RELATIVE;
}

static void calculateAndSetActiveWaypoint(const navWaypoint_t * waypoint)
{
    fpVector3_t localPos;
    mapWaypointToLocalPosition(&localPos, waypoint, waypointMissionAltConvMode(waypoint->p3));
    calculateAndSetActiveWaypointToLocalPosition(&localPos);

    if (navConfig()->fw.wp_turn_smoothing) {
        fpVector3_t posNextWp;
        if (getLocalPosNextWaypoint(&posNextWp)) {
            int32_t bearingToNextWp = calculateBearingBetweenLocalPositions(&posControl.activeWaypoint.pos, &posNextWp);
            posControl.activeWaypoint.nextTurnAngle = wrap_18000(bearingToNextWp - posControl.activeWaypoint.yaw);
        }
    }
}

/* Checks if active waypoint is last in mission */
bool isLastMissionWaypoint(void)
{
    return FLIGHT_MODE(NAV_WP_MODE) && (posControl.activeWaypointIndex >= (posControl.startWpIndex + posControl.waypointCount - 1) ||
            (posControl.waypointList[posControl.activeWaypointIndex].flag == NAV_WP_FLAG_LAST));
}

/* Checks if Nav hold position is active */
bool isNavHoldPositionActive(void)
{
    // WP mode last WP hold and Timed hold positions
    if (FLIGHT_MODE(NAV_WP_MODE)) {
        return isLastMissionWaypoint() || NAV_Status.state == MW_NAV_STATE_HOLD_TIMED;
    }
    // RTH mode (spiral climb and Home positions but excluding RTH Trackback point positions) and POSHOLD mode
    return (FLIGHT_MODE(NAV_RTH_MODE) && !posControl.flags.rthTrackbackActive) || FLIGHT_MODE(NAV_POSHOLD_MODE);
}

float getActiveWaypointSpeed(void)
{
    if (posControl.flags.isAdjustingPosition) {
        // In manual control mode use different cap for speed
        return navConfig()->general.max_manual_speed;
    }
    else {
        uint16_t waypointSpeed = navConfig()->general.auto_speed;

        if (navGetStateFlags(posControl.navState) & NAV_AUTO_WP) {
            if (posControl.waypointCount > 0 && (posControl.waypointList[posControl.activeWaypointIndex].action == NAV_WP_ACTION_WAYPOINT || posControl.waypointList[posControl.activeWaypointIndex].action == NAV_WP_ACTION_HOLD_TIME || posControl.waypointList[posControl.activeWaypointIndex].action == NAV_WP_ACTION_LAND)) {
                float wpSpecificSpeed = 0.0f;
                if(posControl.waypointList[posControl.activeWaypointIndex].action == NAV_WP_ACTION_HOLD_TIME)
                    wpSpecificSpeed = posControl.waypointList[posControl.activeWaypointIndex].p2; // P1 is hold time
                else
                    wpSpecificSpeed = posControl.waypointList[posControl.activeWaypointIndex].p1; // default case

                if (wpSpecificSpeed >= 50.0f && wpSpecificSpeed <= navConfig()->general.max_auto_speed) {
                    waypointSpeed = wpSpecificSpeed;
                } else if (wpSpecificSpeed > navConfig()->general.max_auto_speed) {
                    waypointSpeed = navConfig()->general.max_auto_speed;
                }
            }
        }

        return waypointSpeed;
    }
}

bool isWaypointNavTrackingActive(void)
{
    // NAV_WP_MODE flag used rather than state flag NAV_AUTO_WP to ensure heading to initial waypoint
    // is set from current position not previous WP. Works for WP Restart intermediate WP as well as first mission WP.
    // (NAV_WP_MODE flag isn't set until WP initialisation is finished, i.e. after calculateAndSetActiveWaypoint called)

    return FLIGHT_MODE(NAV_WP_MODE) || (posControl.flags.rthTrackbackActive && posControl.activeRthTBPointIndex != posControl.rthTBLastSavedIndex);
}

/*-----------------------------------------------------------
 * Process adjustments to alt, pos and yaw controllers
 *-----------------------------------------------------------*/
static void processNavigationRCAdjustments(void)
{
    /* Process pilot's RC input. Disable all pilot's input when in FAILSAFE_MODE */
    navigationFSMStateFlags_t navStateFlags = navGetStateFlags(posControl.navState);
    if ((navStateFlags & NAV_RC_ALT) && (!FLIGHT_MODE(FAILSAFE_MODE))) {
        posControl.flags.isAdjustingAltitude = adjustAltitudeFromRCInput();
    }
    else {
        posControl.flags.isAdjustingAltitude = false;
    }

    if (navStateFlags & NAV_RC_POS) {
        posControl.flags.isAdjustingPosition = adjustPositionFromRCInput() && !FLIGHT_MODE(FAILSAFE_MODE);
        if (STATE(MULTIROTOR) && FLIGHT_MODE(FAILSAFE_MODE)) {
            resetMulticopterBrakingMode();
        }
    }
    else {
        posControl.flags.isAdjustingPosition = false;
    }

    if ((navStateFlags & NAV_RC_YAW) && (!FLIGHT_MODE(FAILSAFE_MODE))) {
        posControl.flags.isAdjustingHeading = adjustHeadingFromRCInput();
    }
    else {
        posControl.flags.isAdjustingHeading = false;
    }
}

/*-----------------------------------------------------------
 * A main function to call position controllers at loop rate
 *-----------------------------------------------------------*/
void applyWaypointNavigationAndAltitudeHold(void)
{
    const timeUs_t currentTimeUs = micros();

    //Updata blackbox data
    navFlags = 0;
    if (posControl.flags.estAltStatus == EST_TRUSTED)       navFlags |= (1 << 0);
    if (posControl.flags.estAglStatus == EST_TRUSTED)       navFlags |= (1 << 1);
    if (posControl.flags.estPosStatus == EST_TRUSTED)       navFlags |= (1 << 2);
    if (posControl.flags.isTerrainFollowEnabled)            navFlags |= (1 << 3);
#if defined(NAV_GPS_GLITCH_DETECTION)
    if (isGPSGlitchDetected())                              navFlags |= (1 << 4);
#endif
    if (posControl.flags.estHeadingStatus == EST_TRUSTED)   navFlags |= (1 << 5);

    // Reset all navigation requests - NAV controllers will set them if necessary
    DISABLE_STATE(NAV_MOTOR_STOP_OR_IDLE);

    // No navigation when disarmed
    if (!ARMING_FLAG(ARMED)) {
        // If we are disarmed, abort forced RTH or Emergency Landing
        posControl.flags.forcedRTHActivated = false;
        posControl.flags.forcedEmergLandingActivated = false;
        //  ensure WP missions always restart from first waypoint after disarm
        posControl.activeWaypointIndex = posControl.startWpIndex;
        // Reset RTH trackback
        posControl.activeRthTBPointIndex = -1;
        posControl.flags.rthTrackbackActive = false;
        posControl.rthTBWrapAroundCounter = -1;

        return;
    }

    /* Reset flags */
    posControl.flags.horizontalPositionDataConsumed = false;
    posControl.flags.verticalPositionDataConsumed = false;

    /* Process controllers */
    navigationFSMStateFlags_t navStateFlags = navGetStateFlags(posControl.navState);
    if (STATE(ROVER) || STATE(BOAT)) {
        applyRoverBoatNavigationController(navStateFlags, currentTimeUs);
    } else if (STATE(FIXED_WING_LEGACY)) {
        applyFixedWingNavigationController(navStateFlags, currentTimeUs);
    }
    else {
        applyMulticopterNavigationController(navStateFlags, currentTimeUs);
    }

    /* Consume position data */
    if (posControl.flags.horizontalPositionDataConsumed)
        posControl.flags.horizontalPositionDataNew = false;

    if (posControl.flags.verticalPositionDataConsumed)
        posControl.flags.verticalPositionDataNew = false;

    //Update blackbox data
    if (posControl.flags.isAdjustingPosition)       navFlags |= (1 << 6);
    if (posControl.flags.isAdjustingAltitude)       navFlags |= (1 << 7);
    if (posControl.flags.isAdjustingHeading)        navFlags |= (1 << 8);

    navTargetPosition[X] = lrintf(posControl.desiredState.pos.x);
    navTargetPosition[Y] = lrintf(posControl.desiredState.pos.y);
    navTargetPosition[Z] = lrintf(posControl.desiredState.pos.z);
}

/*-----------------------------------------------------------
 * Set CF's FLIGHT_MODE from current NAV_MODE
 *-----------------------------------------------------------*/
void switchNavigationFlightModes(void)
{
    const flightModeFlags_e enabledNavFlightModes = navGetMappedFlightModes(posControl.navState);
    const flightModeFlags_e disabledFlightModes = (NAV_ALTHOLD_MODE | NAV_RTH_MODE | NAV_POSHOLD_MODE | NAV_WP_MODE | NAV_LAUNCH_MODE | NAV_COURSE_HOLD_MODE) & (~enabledNavFlightModes);
    DISABLE_FLIGHT_MODE(disabledFlightModes);
    ENABLE_FLIGHT_MODE(enabledNavFlightModes);
}

/*-----------------------------------------------------------
 * desired NAV_MODE from combination of FLIGHT_MODE flags
 *-----------------------------------------------------------*/
static bool canActivateAltHoldMode(void)
{
    return (posControl.flags.estAltStatus >= EST_USABLE);
}

static bool canActivatePosHoldMode(void)
{
    return (posControl.flags.estPosStatus >= EST_USABLE) && (posControl.flags.estVelStatus == EST_TRUSTED) && (posControl.flags.estHeadingStatus >= EST_USABLE);
}

static bool canActivateNavigationModes(void)
{
    return (posControl.flags.estPosStatus == EST_TRUSTED) && (posControl.flags.estVelStatus == EST_TRUSTED) && (posControl.flags.estHeadingStatus >= EST_USABLE);
}

static bool isWaypointMissionValid(void)
{
    return posControl.waypointListValid && (posControl.waypointCount > 0);
}

static navigationFSMEvent_t selectNavEventFromBoxModeInput(void)
{
    static bool canActivateWaypoint = false;
    static bool canActivateLaunchMode = false;

    //We can switch modes only when ARMED
    if (ARMING_FLAG(ARMED)) {
        // Ask failsafe system if we can use navigation system
        if (failsafeBypassNavigation()) {
            return NAV_FSM_EVENT_SWITCH_TO_IDLE;
        }

        // Flags if we can activate certain nav modes (check if we have required sensors and they provide valid data)
        const bool canActivateAltHold    = canActivateAltHoldMode();
        const bool canActivatePosHold    = canActivatePosHoldMode();
        const bool canActivateNavigation = canActivateNavigationModes();
        const bool isExecutingRTH        = navGetStateFlags(posControl.navState) & NAV_AUTO_RTH;
        checkSafeHomeState(isExecutingRTH || posControl.flags.forcedRTHActivated);

        // deactivate rth trackback if RTH not active
        if (posControl.flags.rthTrackbackActive) {
            posControl.flags.rthTrackbackActive = isExecutingRTH;
        }

        /* Emergency landing triggered by failsafe when Failsafe procedure set to Landing */
        if (posControl.flags.forcedEmergLandingActivated) {
            return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
        }

        /* Keep Emergency landing mode active once triggered.
         * If caused by sensor failure - landing auto cancelled if sensors working again or when WP and RTH deselected or if Althold selected.
         * If caused by RTH Sanity Checking - landing cancelled if RTH deselected.
         * Remains active if failsafe active regardless of mode selections */
        if (navigationIsExecutingAnEmergencyLanding()) {
            bool autonomousNavIsPossible = canActivateNavigation && canActivateAltHold && STATE(GPS_FIX_HOME);
            bool emergLandingCancel = (!autonomousNavIsPossible &&
                                      ((IS_RC_MODE_ACTIVE(BOXNAVALTHOLD) && canActivateAltHold) || !(IS_RC_MODE_ACTIVE(BOXNAVWP) || IS_RC_MODE_ACTIVE(BOXNAVRTH)))) ||
                                      (autonomousNavIsPossible && !IS_RC_MODE_ACTIVE(BOXNAVRTH));

            if ((!posControl.rthSanityChecker.rthSanityOK || !autonomousNavIsPossible) && (!emergLandingCancel || FLIGHT_MODE(FAILSAFE_MODE))) {
                return NAV_FSM_EVENT_SWITCH_TO_EMERGENCY_LANDING;
            }
        }
        posControl.rthSanityChecker.rthSanityOK = true;

        // Keep canActivateWaypoint flag at FALSE if there is no mission loaded
        // Also block WP mission if we are executing RTH
        if (!isWaypointMissionValid() || isExecutingRTH) {
            canActivateWaypoint = false;
        }

        /* Airplane specific modes */
        if (STATE(AIRPLANE)) {
            // LAUNCH mode has priority over any other NAV mode
            if (isNavLaunchEnabled()) {     // FIXME: Only available for fixed wing aircrafts now
                if (canActivateLaunchMode) {
                    canActivateLaunchMode = false;
                    return NAV_FSM_EVENT_SWITCH_TO_LAUNCH;
                }
                else if FLIGHT_MODE(NAV_LAUNCH_MODE) {
                    // Make sure we don't bail out to IDLE
                    return NAV_FSM_EVENT_NONE;
                }
            }
            else {
                // If we were in LAUNCH mode - force switch to IDLE only if the throttle is low or throttle stick < launch idle throttle
                if (FLIGHT_MODE(NAV_LAUNCH_MODE)) {
                    if (abortLaunchAllowed()) {
                        return NAV_FSM_EVENT_SWITCH_TO_IDLE;
                    } else {
                        return NAV_FSM_EVENT_NONE;
                    }
                }
            }

            /* Soaring mode, disables altitude control in Position hold and Course hold modes.
             * Pitch allowed to freefloat within defined Angle mode deadband */
            if (IS_RC_MODE_ACTIVE(BOXSOARING) && (FLIGHT_MODE(NAV_POSHOLD_MODE) || FLIGHT_MODE(NAV_COURSE_HOLD_MODE))) {
                if (!FLIGHT_MODE(SOARING_MODE)) {
                    ENABLE_FLIGHT_MODE(SOARING_MODE);
                }
            } else {
                DISABLE_FLIGHT_MODE(SOARING_MODE);
            }
        }

        // Failsafe_RTH (can override MANUAL)
        if (posControl.flags.forcedRTHActivated) {
            // If we request forced RTH - attempt to activate it no matter what
            // This might switch to emergency landing controller if GPS is unavailable
            return NAV_FSM_EVENT_SWITCH_TO_RTH;
        }

        /* Pilot-triggered RTH (can override MANUAL), also fall-back for WP if there is no mission loaded
         * Prevent MANUAL falling back to RTH if selected during active mission (canActivateWaypoint is set false on MANUAL selection)
         * Also prevent WP falling back to RTH if WP mission planner is active */
        const bool blockWPFallback = IS_RC_MODE_ACTIVE(BOXMANUAL) || posControl.flags.wpMissionPlannerActive;
        if (IS_RC_MODE_ACTIVE(BOXNAVRTH) || (IS_RC_MODE_ACTIVE(BOXNAVWP) && !canActivateWaypoint && !blockWPFallback)) {
            // Check for isExecutingRTH to prevent switching our from RTH in case of a brief GPS loss
            // If don't keep this, loss of any of the canActivateNavigation && canActivateAltHold
            // will kick us out of RTH state machine via NAV_FSM_EVENT_SWITCH_TO_IDLE and will prevent any of the fall-back
            // logic to kick in (waiting for GPS on airplanes, switch to emergency landing etc)
            if (isExecutingRTH || (canActivateNavigation && canActivateAltHold && STATE(GPS_FIX_HOME))) {
                return NAV_FSM_EVENT_SWITCH_TO_RTH;
            }
        }

        // MANUAL mode has priority over WP/PH/AH
        if (IS_RC_MODE_ACTIVE(BOXMANUAL)) {
            canActivateWaypoint = false;    // Block WP mode if we are in PASSTHROUGH mode
            return NAV_FSM_EVENT_SWITCH_TO_IDLE;
        }

        // Pilot-activated waypoint mission. Fall-back to RTH in case of no mission loaded
        // Block activation if using WP Mission Planner
        // Also check multimission mission change status before activating WP mode
#ifdef USE_MULTI_MISSION
        if (updateWpMissionChange() && IS_RC_MODE_ACTIVE(BOXNAVWP) && !posControl.flags.wpMissionPlannerActive) {
#else
        if (IS_RC_MODE_ACTIVE(BOXNAVWP) && !posControl.flags.wpMissionPlannerActive) {
#endif
            if (FLIGHT_MODE(NAV_WP_MODE) || (canActivateWaypoint && canActivateNavigation && canActivateAltHold && STATE(GPS_FIX_HOME)))
                return NAV_FSM_EVENT_SWITCH_TO_WAYPOINT;
        }
        else {
            // Arm the state variable if the WP BOX mode is not enabled
            canActivateWaypoint = true;
        }

        if (IS_RC_MODE_ACTIVE(BOXNAVPOSHOLD)) {
            if (FLIGHT_MODE(NAV_POSHOLD_MODE) || (canActivatePosHold && canActivateAltHold))
                return NAV_FSM_EVENT_SWITCH_TO_POSHOLD_3D;
        }

        // CRUISE has priority over COURSE_HOLD and AH
        if (IS_RC_MODE_ACTIVE(BOXNAVCRUISE)) {
            if ((FLIGHT_MODE(NAV_COURSE_HOLD_MODE) && FLIGHT_MODE(NAV_ALTHOLD_MODE)) || (canActivatePosHold && canActivateAltHold))
                return NAV_FSM_EVENT_SWITCH_TO_CRUISE;
        }

        // PH has priority over COURSE_HOLD
        // CRUISE has priority on AH
        if (IS_RC_MODE_ACTIVE(BOXNAVCOURSEHOLD)) {

            if (IS_RC_MODE_ACTIVE(BOXNAVALTHOLD) && ((FLIGHT_MODE(NAV_COURSE_HOLD_MODE) && FLIGHT_MODE(NAV_ALTHOLD_MODE)) || (canActivatePosHold && canActivateAltHold)))
                return NAV_FSM_EVENT_SWITCH_TO_CRUISE;

            if (FLIGHT_MODE(NAV_COURSE_HOLD_MODE) || (canActivatePosHold))
                return NAV_FSM_EVENT_SWITCH_TO_COURSE_HOLD;

        }

        if (IS_RC_MODE_ACTIVE(BOXNAVALTHOLD)) {
            if ((FLIGHT_MODE(NAV_ALTHOLD_MODE)) || (canActivateAltHold))
                return NAV_FSM_EVENT_SWITCH_TO_ALTHOLD;
        }
    }
    else {
        canActivateWaypoint = false;

        // Launch mode can be activated if feature FW_LAUNCH is enabled or BOX is turned on prior to arming (avoid switching to LAUNCH in flight)
        canActivateLaunchMode = isNavLaunchEnabled();
    }

    return NAV_FSM_EVENT_SWITCH_TO_IDLE;
}

/*-----------------------------------------------------------
 * An indicator that throttle tilt compensation is forced
 *-----------------------------------------------------------*/
bool navigationRequiresThrottleTiltCompensation(void)
{
    return !STATE(FIXED_WING_LEGACY) && (navGetStateFlags(posControl.navState) & NAV_REQUIRE_THRTILT);
}

/*-----------------------------------------------------------
 * An indicator that ANGLE mode must be forced per NAV requirement
 *-----------------------------------------------------------*/
bool navigationRequiresAngleMode(void)
{
    const navigationFSMStateFlags_t currentState = navGetStateFlags(posControl.navState);
    return (currentState & NAV_REQUIRE_ANGLE) || ((currentState & NAV_REQUIRE_ANGLE_FW) && STATE(FIXED_WING_LEGACY));
}

/*-----------------------------------------------------------
 * An indicator that TURN ASSISTANCE is required for navigation
 *-----------------------------------------------------------*/
bool navigationRequiresTurnAssistance(void)
{
    const navigationFSMStateFlags_t currentState = navGetStateFlags(posControl.navState);
    if (STATE(FIXED_WING_LEGACY)) {
        // For airplanes turn assistant is always required when controlling position
        return (currentState & (NAV_CTL_POS | NAV_CTL_ALT));
    }
    else {
        return false;
    }
}

/**
 * An indicator that NAV is in charge of heading control (a signal to disable other heading controllers)
 */
int8_t navigationGetHeadingControlState(void)
{
    // For airplanes report as manual heading control
    if (STATE(FIXED_WING_LEGACY)) {
        return NAV_HEADING_CONTROL_MANUAL;
    }

    // For multirotors it depends on navigation system mode
    if (navGetStateFlags(posControl.navState) & NAV_REQUIRE_MAGHOLD) {
        if (posControl.flags.isAdjustingHeading) {
            return NAV_HEADING_CONTROL_MANUAL;
        }
        else {
            return NAV_HEADING_CONTROL_AUTO;
        }
    }
    else {
        return NAV_HEADING_CONTROL_NONE;
    }
}

bool navigationTerrainFollowingEnabled(void)
{
    return posControl.flags.isTerrainFollowEnabled;
}

uint32_t distanceToFirstWP(void)
{
    fpVector3_t startingWaypointPos;
    mapWaypointToLocalPosition(&startingWaypointPos, &posControl.waypointList[posControl.startWpIndex], GEO_ALT_RELATIVE);
    return calculateDistanceToDestination(&startingWaypointPos);
}

bool navigationPositionEstimateIsHealthy(void)
{
    return (posControl.flags.estPosStatus >= EST_USABLE) && STATE(GPS_FIX_HOME);
}

navArmingBlocker_e navigationIsBlockingArming(bool *usedBypass)
{
    const bool navBoxModesEnabled = IS_RC_MODE_ACTIVE(BOXNAVRTH) || IS_RC_MODE_ACTIVE(BOXNAVWP) || IS_RC_MODE_ACTIVE(BOXNAVPOSHOLD) || (STATE(FIXED_WING_LEGACY) && IS_RC_MODE_ACTIVE(BOXNAVALTHOLD)) || (STATE(FIXED_WING_LEGACY) && (IS_RC_MODE_ACTIVE(BOXNAVCOURSEHOLD) || IS_RC_MODE_ACTIVE(BOXNAVCRUISE)));
    const bool navLaunchComboModesEnabled = isNavLaunchEnabled() && (IS_RC_MODE_ACTIVE(BOXNAVRTH) || IS_RC_MODE_ACTIVE(BOXNAVWP) || IS_RC_MODE_ACTIVE(BOXNAVALTHOLD) || IS_RC_MODE_ACTIVE(BOXNAVCOURSEHOLD) || IS_RC_MODE_ACTIVE(BOXNAVCRUISE));

    if (usedBypass) {
        *usedBypass = false;
    }

    // Apply extra arming safety only if pilot has any of GPS modes configured
    if ((isUsingNavigationModes() || failsafeMayRequireNavigationMode()) && !navigationPositionEstimateIsHealthy()) {
        if (navConfig()->general.flags.extra_arming_safety == NAV_EXTRA_ARMING_SAFETY_ALLOW_BYPASS &&
            (STATE(NAV_EXTRA_ARMING_SAFETY_BYPASSED) || checkStickPosition(YAW_HI))) {
            if (usedBypass) {
                *usedBypass = true;
            }
            return NAV_ARMING_BLOCKER_NONE;
        }
        return NAV_ARMING_BLOCKER_MISSING_GPS_FIX;
    }

    // Don't allow arming if any of NAV modes is active
    if (!ARMING_FLAG(ARMED) && navBoxModesEnabled && !navLaunchComboModesEnabled) {
        return NAV_ARMING_BLOCKER_NAV_IS_ALREADY_ACTIVE;
    }

    // Don't allow arming if first waypoint is farther than configured safe distance
    if ((posControl.waypointCount > 0) && (navConfig()->general.waypoint_safe_distance != 0)) {
        if (distanceToFirstWP() > navConfig()->general.waypoint_safe_distance && !checkStickPosition(YAW_HI)) {
            return NAV_ARMING_BLOCKER_FIRST_WAYPOINT_TOO_FAR;
        }
    }

        /*
         * Don't allow arming if any of JUMP waypoint has invalid settings
         * First WP can't be JUMP
         * Can't jump to immediately adjacent WPs (pointless)
         * Can't jump beyond WP list
         * Only jump to geo-referenced WP types
         */
    if (posControl.waypointCount) {
        for (uint8_t wp = posControl.startWpIndex; wp < posControl.waypointCount + posControl.startWpIndex; wp++){
            if (posControl.waypointList[wp].action == NAV_WP_ACTION_JUMP){
                if (wp == posControl.startWpIndex || posControl.waypointList[wp].p1 >= posControl.waypointCount ||
                (posControl.waypointList[wp].p1 > (wp - posControl.startWpIndex - 2) && posControl.waypointList[wp].p1 < (wp - posControl.startWpIndex + 2)) || posControl.waypointList[wp].p2 < -1) {
                    return NAV_ARMING_BLOCKER_JUMP_WAYPOINT_ERROR;
                }

                    /* check for target geo-ref sanity */
                uint16_t target = posControl.waypointList[wp].p1 + posControl.startWpIndex;
                if (!(posControl.waypointList[target].action == NAV_WP_ACTION_WAYPOINT || posControl.waypointList[target].action == NAV_WP_ACTION_HOLD_TIME || posControl.waypointList[target].action == NAV_WP_ACTION_LAND)) {
                    return NAV_ARMING_BLOCKER_JUMP_WAYPOINT_ERROR;
                }
            }
        }
    }

    return NAV_ARMING_BLOCKER_NONE;
}

/**
 * Indicate ready/not ready status
 */
static void updateReadyStatus(void)
{
    static bool posReadyBeepDone = false;

    /* Beep out READY_BEEP once when position lock is firstly acquired and HOME set */
    if (navigationPositionEstimateIsHealthy() && !posReadyBeepDone) {
        beeper(BEEPER_READY_BEEP);
        posReadyBeepDone = true;
    }
}

void updateFlightBehaviorModifiers(void)
{
    if (posControl.flags.isGCSAssistedNavigationEnabled && !IS_RC_MODE_ACTIVE(BOXGCSNAV)) {
        posControl.flags.isGCSAssistedNavigationReset = true;
    }

    posControl.flags.isGCSAssistedNavigationEnabled = IS_RC_MODE_ACTIVE(BOXGCSNAV);
}

/* On the fly WP mission planner mode allows WP missions to be setup during navigation.
 * Uses the WP mode switch to save WP at current location (WP mode disabled when active)
 * Mission can be flown after mission planner mode switched off and saved after disarm. */

void updateWpMissionPlanner(void)
{
    static timeMs_t resetTimerStart = 0;
    if (IS_RC_MODE_ACTIVE(BOXPLANWPMISSION) && !(FLIGHT_MODE(NAV_WP_MODE) || isWaypointMissionRTHActive())) {
        const bool positionTrusted = posControl.flags.estAltStatus == EST_TRUSTED && posControl.flags.estPosStatus == EST_TRUSTED && STATE(GPS_FIX);

        posControl.flags.wpMissionPlannerActive = true;
        if (millis() - resetTimerStart < 1000 && navConfig()->general.flags.mission_planner_reset) {
            posControl.waypointCount = posControl.wpPlannerActiveWPIndex = 0;
            posControl.waypointListValid = false;
            posControl.wpMissionPlannerStatus = WP_PLAN_WAIT;
        }
        if (positionTrusted && posControl.wpMissionPlannerStatus != WP_PLAN_FULL) {
            missionPlannerSetWaypoint();
        } else {
            posControl.wpMissionPlannerStatus = posControl.wpMissionPlannerStatus == WP_PLAN_FULL ? WP_PLAN_FULL : WP_PLAN_WAIT;
        }
    } else if (posControl.flags.wpMissionPlannerActive) {
        posControl.flags.wpMissionPlannerActive = false;
        posControl.activeWaypointIndex = 0;
        resetTimerStart = millis();
    }
}

void missionPlannerSetWaypoint(void)
{
    static bool boxWPModeIsReset = true;

    boxWPModeIsReset = !boxWPModeIsReset ? !IS_RC_MODE_ACTIVE(BOXNAVWP) : boxWPModeIsReset; // only able to save new WP when WP mode reset
    posControl.wpMissionPlannerStatus = boxWPModeIsReset ? boxWPModeIsReset : posControl.wpMissionPlannerStatus;  // hold save status until WP mode reset

    if (!boxWPModeIsReset || !IS_RC_MODE_ACTIVE(BOXNAVWP)) {
        return;
    }

    if (!posControl.wpPlannerActiveWPIndex) {   // reset existing mission data before adding first WP
        resetWaypointList();
    }

    gpsLocation_t wpLLH;
    geoConvertLocalToGeodetic(&wpLLH, &posControl.gpsOrigin, &navGetCurrentActualPositionAndVelocity()->pos);

    posControl.waypointList[posControl.wpPlannerActiveWPIndex].action = 1;
    posControl.waypointList[posControl.wpPlannerActiveWPIndex].lat = wpLLH.lat;
    posControl.waypointList[posControl.wpPlannerActiveWPIndex].lon = wpLLH.lon;
    posControl.waypointList[posControl.wpPlannerActiveWPIndex].alt = wpLLH.alt;
    posControl.waypointList[posControl.wpPlannerActiveWPIndex].p1 = posControl.waypointList[posControl.wpPlannerActiveWPIndex].p2 = 0;
    posControl.waypointList[posControl.wpPlannerActiveWPIndex].p3 |= NAV_WP_ALTMODE;      // use absolute altitude datum
    posControl.waypointList[posControl.wpPlannerActiveWPIndex].flag = NAV_WP_FLAG_LAST;
    posControl.waypointListValid = true;

    if (posControl.wpPlannerActiveWPIndex) {
        posControl.waypointList[posControl.wpPlannerActiveWPIndex - 1].flag = 0; // rollling reset of previous end of mission flag when new WP added
    }

    posControl.wpPlannerActiveWPIndex += 1;
    posControl.waypointCount = posControl.geoWaypointCount = posControl.wpPlannerActiveWPIndex;
    posControl.wpMissionPlannerStatus = posControl.waypointCount == NAV_MAX_WAYPOINTS ? WP_PLAN_FULL : WP_PLAN_OK;
    boxWPModeIsReset = false;
}

/**
 * Process NAV mode transition and WP/RTH state machine
 *  Update rate: RX (data driven or 50Hz)
 */
void updateWaypointsAndNavigationMode(void)
{
    /* Initiate home position update */
    updateHomePosition();

    /* Update flight statistics */
    updateNavigationFlightStatistics();

    /* Update NAV ready status */
    updateReadyStatus();

    // Update flight behaviour modifiers
    updateFlightBehaviorModifiers();

    // Process switch to a different navigation mode (if needed)
    navProcessFSMEvents(selectNavEventFromBoxModeInput());

    // Process pilot's RC input to adjust behaviour
    processNavigationRCAdjustments();

    // Map navMode back to enabled flight modes
    switchNavigationFlightModes();

    // Update WP mission planner
    updateWpMissionPlanner();

    // Update RTH trackback
    updateRthTrackback(false);

    //Update Blackbox data
    navCurrentState = (int16_t)posControl.navPersistentId;
}

/*-----------------------------------------------------------
 * NAV main control functions
 *-----------------------------------------------------------*/
void navigationUsePIDs(void)
{
    /** Multicopter PIDs */
    // Brake time parameter
    posControl.posDecelerationTime = (float)navConfig()->mc.posDecelerationTime / 100.0f;

    // Position controller expo (taret vel expo for MC)
    posControl.posResponseExpo = constrainf((float)navConfig()->mc.posResponseExpo / 100.0f, 0.0f, 1.0f);

    // Initialize position hold P-controller
    for (int axis = 0; axis < 2; axis++) {
        navPidInit(
            &posControl.pids.pos[axis],
            (float)pidProfile()->bank_mc.pid[PID_POS_XY].P / 100.0f,
            0.0f,
            0.0f,
            0.0f,
            NAV_DTERM_CUT_HZ,
            0.0f
        );

        navPidInit(&posControl.pids.vel[axis], (float)pidProfile()->bank_mc.pid[PID_VEL_XY].P / 20.0f,
                                               (float)pidProfile()->bank_mc.pid[PID_VEL_XY].I / 100.0f,
                                               (float)pidProfile()->bank_mc.pid[PID_VEL_XY].D / 100.0f,
                                               (float)pidProfile()->bank_mc.pid[PID_VEL_XY].FF / 100.0f,
                                               pidProfile()->navVelXyDTermLpfHz,
                                               0.0f
        );
    }

    /*
     * Set coefficients used in MC VEL_XY
     */
    multicopterPosXyCoefficients.dTermAttenuation = pidProfile()->navVelXyDtermAttenuation / 100.0f;
    multicopterPosXyCoefficients.dTermAttenuationStart = pidProfile()->navVelXyDtermAttenuationStart / 100.0f;
    multicopterPosXyCoefficients.dTermAttenuationEnd = pidProfile()->navVelXyDtermAttenuationEnd / 100.0f;

#ifdef USE_MR_BRAKING_MODE
    multicopterPosXyCoefficients.breakingBoostFactor = (float) navConfig()->mc.braking_boost_factor / 100.0f;
#endif

    // Initialize altitude hold PID-controllers (pos_z, vel_z, acc_z
    navPidInit(
        &posControl.pids.pos[Z],
        (float)pidProfile()->bank_mc.pid[PID_POS_Z].P / 100.0f,
        0.0f,
        0.0f,
        0.0f,
        NAV_DTERM_CUT_HZ,
        0.0f
    );

    navPidInit(&posControl.pids.vel[Z], (float)pidProfile()->bank_mc.pid[PID_VEL_Z].P / 66.7f,
                                        (float)pidProfile()->bank_mc.pid[PID_VEL_Z].I / 20.0f,
                                        (float)pidProfile()->bank_mc.pid[PID_VEL_Z].D / 100.0f,
                                        0.0f,
                                        NAV_VEL_Z_DERIVATIVE_CUT_HZ,
                                        NAV_VEL_Z_ERROR_CUT_HZ
    );

    // Initialize surface tracking PID
    navPidInit(&posControl.pids.surface, 2.0f,
                                         0.0f,
                                         0.0f,
                                         0.0f,
                                         NAV_DTERM_CUT_HZ,
                                         0.0f
    );

    /** Airplane PIDs */
    // Initialize fixed wing PID controllers
    navPidInit(&posControl.pids.fw_nav, (float)pidProfile()->bank_fw.pid[PID_POS_XY].P / 100.0f,
                                        (float)pidProfile()->bank_fw.pid[PID_POS_XY].I / 100.0f,
                                        (float)pidProfile()->bank_fw.pid[PID_POS_XY].D / 100.0f,
                                        0.0f,
                                        NAV_DTERM_CUT_HZ,
                                        0.0f
    );

    navPidInit(&posControl.pids.fw_alt, (float)pidProfile()->bank_fw.pid[PID_POS_Z].P / 10.0f,
                                        (float)pidProfile()->bank_fw.pid[PID_POS_Z].I / 10.0f,
                                        (float)pidProfile()->bank_fw.pid[PID_POS_Z].D / 10.0f,
                                        0.0f,
                                        NAV_DTERM_CUT_HZ,
                                        0.0f
    );

    navPidInit(&posControl.pids.fw_heading, (float)pidProfile()->bank_fw.pid[PID_POS_HEADING].P / 10.0f,
                                        (float)pidProfile()->bank_fw.pid[PID_POS_HEADING].I / 10.0f,
                                        (float)pidProfile()->bank_fw.pid[PID_POS_HEADING].D / 100.0f,
                                        0.0f,
                                        2.0f,
                                        0.0f
    );
}

void navigationInit(void)
{
    /* Initial state */
    posControl.navState = NAV_STATE_IDLE;

    posControl.flags.horizontalPositionDataNew = false;
    posControl.flags.verticalPositionDataNew = false;

    posControl.flags.estAltStatus = EST_NONE;
    posControl.flags.estPosStatus = EST_NONE;
    posControl.flags.estVelStatus = EST_NONE;
    posControl.flags.estHeadingStatus = EST_NONE;
    posControl.flags.estAglStatus = EST_NONE;

    posControl.flags.forcedRTHActivated = false;
    posControl.flags.forcedEmergLandingActivated = false;
    posControl.waypointCount = 0;
    posControl.activeWaypointIndex = 0;
    posControl.waypointListValid = false;
    posControl.wpPlannerActiveWPIndex = 0;
    posControl.flags.wpMissionPlannerActive = false;
    posControl.startWpIndex = 0;
#ifdef USE_MULTI_MISSION
    posControl.multiMissionCount = 0;
#endif
    /* Set initial surface invalid */
    posControl.actualState.surfaceMin = -1.0f;

    /* Reset statistics */
    posControl.totalTripDistance = 0.0f;

    /* Use system config */
    navigationUsePIDs();

    if (
        mixerConfig()->platformType == PLATFORM_BOAT ||
        mixerConfig()->platformType == PLATFORM_ROVER ||
        navConfig()->fw.useFwNavYawControl
    ) {
        ENABLE_STATE(FW_HEADING_USE_YAW);
    } else {
        DISABLE_STATE(FW_HEADING_USE_YAW);
    }
#if defined(NAV_NON_VOLATILE_WAYPOINT_STORAGE)
    /* configure WP missions at boot */
#ifdef USE_MULTI_MISSION
    for (int8_t i = 0; i < NAV_MAX_WAYPOINTS; i++) {    // check number missions in NVM
        if (checkMissionCount(i)) {
            break;
        }
    }
    /* set index to 1 if saved mission index > available missions */
    if (navConfig()->general.waypoint_multi_mission_index > posControl.multiMissionCount) {
        navConfigMutable()->general.waypoint_multi_mission_index = 1;
    }
#endif
    /* load mission on boot */
    if (navConfig()->general.waypoint_load_on_boot) {
        loadNonVolatileWaypointList(false);
    }
#endif
}

/*-----------------------------------------------------------
 * Access to estimated position/velocity data
 *-----------------------------------------------------------*/
float getEstimatedActualVelocity(int axis)
{
    return navGetCurrentActualPositionAndVelocity()->vel.v[axis];
}

float getEstimatedActualPosition(int axis)
{
    return navGetCurrentActualPositionAndVelocity()->pos.v[axis];
}

/*-----------------------------------------------------------
 * Ability to execute RTH on external event
 *-----------------------------------------------------------*/
void activateForcedRTH(void)
{
    abortFixedWingLaunch();
    posControl.flags.forcedRTHActivated = true;
    checkSafeHomeState(true);
    navProcessFSMEvents(selectNavEventFromBoxModeInput());
}

void abortForcedRTH(void)
{
    // Disable failsafe RTH and make sure we back out of navigation mode to IDLE
    // If any navigation mode was active prior to RTH it will be re-enabled with next RX update
    posControl.flags.forcedRTHActivated = false;
    checkSafeHomeState(false);
    navProcessFSMEvents(NAV_FSM_EVENT_SWITCH_TO_IDLE);
}

rthState_e getStateOfForcedRTH(void)
{
    /* If forced RTH activated and in AUTO_RTH or EMERG state */
    if (posControl.flags.forcedRTHActivated && (navGetStateFlags(posControl.navState) & (NAV_AUTO_RTH | NAV_CTL_EMERG))) {
        if (posControl.navState == NAV_STATE_RTH_FINISHED || posControl.navState == NAV_STATE_EMERGENCY_LANDING_FINISHED) {
            return RTH_HAS_LANDED;
        }
        else {
            return RTH_IN_PROGRESS;
        }
    }
    else {
        return RTH_IDLE;
    }
}

/*-----------------------------------------------------------
 * Ability to execute Emergency Landing on external event
 *-----------------------------------------------------------*/
void activateForcedEmergLanding(void)
{
    abortFixedWingLaunch();
    posControl.flags.forcedEmergLandingActivated = true;
    navProcessFSMEvents(selectNavEventFromBoxModeInput());
}

void abortForcedEmergLanding(void)
{
    // Disable emergency landing and make sure we back out of navigation mode to IDLE
    // If any navigation mode was active prior to emergency landing it will be re-enabled with next RX update
    posControl.flags.forcedEmergLandingActivated = false;
    navProcessFSMEvents(NAV_FSM_EVENT_SWITCH_TO_IDLE);
}

emergLandState_e getStateOfForcedEmergLanding(void)
{
    /* If forced emergency landing activated and in EMERG state */
    if (posControl.flags.forcedEmergLandingActivated && (navGetStateFlags(posControl.navState) & NAV_CTL_EMERG)) {
        if (posControl.navState == NAV_STATE_EMERGENCY_LANDING_FINISHED) {
            return EMERG_LAND_HAS_LANDED;
        } else {
            return EMERG_LAND_IN_PROGRESS;
        }
    } else {
        return EMERG_LAND_IDLE;
    }
}

bool isWaypointMissionRTHActive(void)
{
    return (navGetStateFlags(posControl.navState) & NAV_AUTO_RTH) && IS_RC_MODE_ACTIVE(BOXNAVWP) &&
           !(IS_RC_MODE_ACTIVE(BOXNAVRTH) || posControl.flags.forcedRTHActivated);
}

bool navigationIsExecutingAnEmergencyLanding(void)
{
    return navGetCurrentStateFlags() & NAV_CTL_EMERG;
}

bool navigationInAutomaticThrottleMode(void)
{
    navigationFSMStateFlags_t stateFlags = navGetCurrentStateFlags();
    return (stateFlags & (NAV_CTL_ALT | NAV_CTL_EMERG | NAV_CTL_LAND)) ||
           ((stateFlags & NAV_CTL_LAUNCH) && !navConfig()->fw.launch_manual_throttle);
}

bool navigationIsControllingThrottle(void)
{
    // Note that this makes a detour into mixer code to evaluate actual motor status
    return navigationInAutomaticThrottleMode() && getMotorStatus() != MOTOR_STOPPED_USER && !FLIGHT_MODE(SOARING_MODE);
}

bool navigationIsControllingAltitude(void) {
    navigationFSMStateFlags_t stateFlags = navGetCurrentStateFlags();
    return (stateFlags & NAV_CTL_ALT);
}

bool navigationIsFlyingAutonomousMode(void)
{
    navigationFSMStateFlags_t stateFlags = navGetCurrentStateFlags();
    return (stateFlags & (NAV_AUTO_RTH | NAV_AUTO_WP));
}

bool navigationRTHAllowsLanding(void)
{
    // WP mission RTH landing setting
    if (isWaypointMissionRTHActive() && isWaypointMissionValid()) {
        return posControl.waypointList[posControl.startWpIndex + posControl.waypointCount - 1].p1 > 0;
    }

    // normal RTH landing setting
    navRTHAllowLanding_e allow = navConfig()->general.flags.rth_allow_landing;
    return allow == NAV_RTH_ALLOW_LANDING_ALWAYS ||
        (allow == NAV_RTH_ALLOW_LANDING_FS_ONLY && FLIGHT_MODE(FAILSAFE_MODE));
}

bool isNavLaunchEnabled(void)
{
    return IS_RC_MODE_ACTIVE(BOXNAVLAUNCH) || feature(FEATURE_FW_LAUNCH);
}

bool abortLaunchAllowed(void)
{
    // allow NAV_LAUNCH_MODE to be aborted if throttle is low or throttle stick position is < launch idle throttle setting
    throttleStatus_e throttleStatus = calculateThrottleStatus(THROTTLE_STATUS_TYPE_RC);
    return throttleStatus == THROTTLE_LOW || throttleStickMixedValue() < currentBatteryProfile->nav.fw.launch_idle_throttle;
}

int32_t navigationGetHomeHeading(void)
{
    return posControl.rthState.homePosition.yaw;
}

// returns m/s
float calculateAverageSpeed() {
    float flightTime = getFlightTime();
    if (flightTime == 0.0f) return 0;
    return (float)getTotalTravelDistance() / (flightTime * 100);
}

const navigationPIDControllers_t* getNavigationPIDControllers(void) {
    return &posControl.pids;
}

bool isAdjustingPosition(void) {
    return posControl.flags.isAdjustingPosition;
}

bool isAdjustingHeading(void) {
    return posControl.flags.isAdjustingHeading;
}

int32_t getCruiseHeadingAdjustment(void) {
    return wrap_18000(posControl.cruise.yaw - posControl.cruise.previousYaw);
}
