// Consolidated enums — generated on 2025-10-29 20:51:02.071648

// ../../../src/main/common/calibration.h
typedef enum {
    ZERO_CALIBRATION_NONE = 0,
    ZERO_CALIBRATION_IN_PROGRESS,
    ZERO_CALIBRATION_DONE,
    ZERO_CALIBRATION_FAIL,
} zeroCalibrationState_e;

// ../../../src/main/common/color.h
typedef enum {
    RGB_RED = 0,
    RGB_GREEN,
    RGB_BLUE
} colorComponent_e;

// ../../../src/main/common/color.h
typedef enum {
    HSV_HUE = 0,
    HSV_SATURATION,
    HSV_VALUE
} hsvColorComponent_e;

// ../../../src/main/common/axis.h
typedef enum {
    X = 0,
    Y,
    Z
} axis_e;

// ../../../src/main/common/axis.h
typedef enum {
    FD_ROLL = 0,
    FD_PITCH,
    FD_YAW
} flight_dynamics_index_t;

// ../../../src/main/common/axis.h
typedef enum {
    AI_ROLL = 0,
    AI_PITCH,
} angle_index_t;

// ../../../src/main/common/tristate.h
typedef enum {
    TRISTATE_AUTO = 0,
    TRISTATE_ON = 1,
    TRISTATE_OFF = 2,
} tristate_e;

// ../../../src/main/common/log.h
typedef enum {
    LOG_TOPIC_SYSTEM,           
    LOG_TOPIC_GYRO,             
    LOG_TOPIC_BARO,             
    LOG_TOPIC_PITOT,            
    LOG_TOPIC_PWM,              
    LOG_TOPIC_TIMER,            
    LOG_TOPIC_IMU,              
    LOG_TOPIC_TEMPERATURE,      
    LOG_TOPIC_POS_ESTIMATOR,    
    LOG_TOPIC_VTX,              
    LOG_TOPIC_OSD,              

    LOG_TOPIC_COUNT,
} logTopic_e;

// ../../../src/main/common/fp_pid.h
typedef enum {
    PID_DTERM_FROM_ERROR            = 1 << 0,
    PID_ZERO_INTEGRATOR             = 1 << 1,
    PID_SHRINK_INTEGRATOR           = 1 << 2,
    PID_LIMIT_INTEGRATOR            = 1 << 3,
    PID_FREEZE_INTEGRATOR           = 1 << 4,
} pidControllerFlags_e;

// ../../../src/main/common/time.h
typedef enum {
    TZ_AUTO_DST_OFF,
    TZ_AUTO_DST_EU,
    TZ_AUTO_DST_USA,
} tz_automatic_dst_e;

// ../../../src/main/common/filter.h
typedef enum {
    FILTER_PT1 = 0,
    FILTER_BIQUAD,
    FILTER_PT2,
    FILTER_PT3,
    FILTER_LULU
} filterType_e;

// ../../../src/main/common/filter.h
typedef enum {
    FILTER_LPF,
    FILTER_NOTCH
} biquadFilterType_e;

// ../../../src/main/navigation/navigation_private.h
typedef enum {
    NAV_POS_UPDATE_NONE                 = 0,
    NAV_POS_UPDATE_Z                    = 1 << 1,
    NAV_POS_UPDATE_XY                   = 1 << 0,
    NAV_POS_UPDATE_HEADING              = 1 << 2,
    NAV_POS_UPDATE_BEARING              = 1 << 3,
    NAV_POS_UPDATE_BEARING_TAIL_FIRST   = 1 << 4,
} navSetWaypointFlags_t;

// ../../../src/main/navigation/navigation_private.h
typedef enum {
    ROC_TO_ALT_CURRENT,
    ROC_TO_ALT_CONSTANT,
    ROC_TO_ALT_TARGET
} climbRateToAltitudeControllerMode_e;

// ../../../src/main/navigation/navigation_private.h
typedef enum {
    EST_NONE = 0,       
    EST_USABLE = 1,     
    EST_TRUSTED = 2     
} navigationEstimateStatus_e;

// ../../../src/main/navigation/navigation_private.h
typedef enum {
    NAV_HOME_INVALID = 0,
    NAV_HOME_VALID_XY = 1 << 0,
    NAV_HOME_VALID_Z = 1 << 1,
    NAV_HOME_VALID_HEADING = 1 << 2,
    NAV_HOME_VALID_ALL = NAV_HOME_VALID_XY | NAV_HOME_VALID_Z | NAV_HOME_VALID_HEADING,
} navigationHomeFlags_t;

// ../../../src/main/navigation/navigation_private.h
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

    NAV_FSM_EVENT_STATE_SPECIFIC_1,             
    NAV_FSM_EVENT_STATE_SPECIFIC_2,             
    NAV_FSM_EVENT_STATE_SPECIFIC_3,             
    NAV_FSM_EVENT_STATE_SPECIFIC_4,             
    NAV_FSM_EVENT_STATE_SPECIFIC_5,             

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

// ../../../src/main/navigation/navigation_private.h
typedef enum {
    NAV_PERSISTENT_ID_UNDEFINED                                 = 0,

    NAV_PERSISTENT_ID_IDLE                                      = 1,

    NAV_PERSISTENT_ID_ALTHOLD_INITIALIZE                        = 2,
    NAV_PERSISTENT_ID_ALTHOLD_IN_PROGRESS                       = 3,

    NAV_PERSISTENT_ID_UNUSED_1                                  = 4,  
    NAV_PERSISTENT_ID_UNUSED_2                                  = 5,  

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
    NAV_PERSISTENT_ID_UNUSED_3                                  = 27, 
    NAV_PERSISTENT_ID_LAUNCH_IN_PROGRESS                        = 28,

    NAV_PERSISTENT_ID_COURSE_HOLD_INITIALIZE                    = 29,
    NAV_PERSISTENT_ID_COURSE_HOLD_IN_PROGRESS                   = 30,
    NAV_PERSISTENT_ID_COURSE_HOLD_ADJUSTING                     = 31,

    NAV_PERSISTENT_ID_CRUISE_INITIALIZE                         = 32,
    NAV_PERSISTENT_ID_CRUISE_IN_PROGRESS                        = 33,
    NAV_PERSISTENT_ID_CRUISE_ADJUSTING                          = 34,

    NAV_PERSISTENT_ID_WAYPOINT_HOLD_TIME                        = 35,
    NAV_PERSISTENT_ID_RTH_LOITER_ABOVE_HOME                     = 36,
    NAV_PERSISTENT_ID_UNUSED_4                                  = 37, 
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

// ../../../src/main/navigation/navigation_private.h
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

// ../../../src/main/navigation/navigation_private.h
typedef enum {
    
    NAV_CTL_ALT             = (1 << 0),     
    NAV_CTL_POS             = (1 << 1),     
    NAV_CTL_YAW             = (1 << 2),
    NAV_CTL_EMERG           = (1 << 3),
    NAV_CTL_LAUNCH          = (1 << 4),

    
    NAV_REQUIRE_ANGLE       = (1 << 5),
    NAV_REQUIRE_ANGLE_FW    = (1 << 6),
    NAV_REQUIRE_MAGHOLD     = (1 << 7),
    NAV_REQUIRE_THRTILT     = (1 << 8),

    
    NAV_AUTO_RTH            = (1 << 9),
    NAV_AUTO_WP             = (1 << 10),

    
    NAV_RC_ALT              = (1 << 11),
    NAV_RC_POS              = (1 << 12),
    NAV_RC_YAW              = (1 << 13),

    
    NAV_CTL_LAND            = (1 << 14),
    NAV_AUTO_WP_DONE        = (1 << 15),    

    NAV_MIXERAT             = (1 << 16),    
    NAV_CTL_HOLD            = (1 << 17),    
} navigationFSMStateFlags_t;

// ../../../src/main/navigation/navigation_private.h
typedef enum {
    FW_AUTOLAND_WP_TURN,
    FW_AUTOLAND_WP_FINAL_APPROACH,
    FW_AUTOLAND_WP_LAND,
    FW_AUTOLAND_WP_COUNT,
} fwAutolandWaypoint_t;

// ../../../src/main/navigation/navigation_private.h
typedef enum {
    RTH_HOME_ENROUTE_INITIAL,       
    RTH_HOME_ENROUTE_PROPORTIONAL,  
    RTH_HOME_ENROUTE_FINAL,         
    RTH_HOME_FINAL_LOITER,          
    RTH_HOME_FINAL_LAND,            
} rthTargetMode_e;

// ../../../src/main/navigation/navigation_pos_estimator_private.h
typedef enum {
    SURFACE_QUAL_LOW,   
    SURFACE_QUAL_MID,   
    SURFACE_QUAL_HIGH   
} navAGLEstimateQuality_e;

// ../../../src/main/navigation/navigation_pos_estimator_private.h
typedef enum {
    EST_GPS_XY_VALID            = (1 << 0),
    EST_GPS_Z_VALID             = (1 << 1),
    EST_BARO_VALID              = (1 << 2),
    EST_SURFACE_VALID           = (1 << 3),
    EST_FLOW_VALID              = (1 << 4),
    EST_XY_VALID                = (1 << 5),
    EST_Z_VALID                 = (1 << 6),
} navPositionEstimationFlags_e;

// ../../../src/main/navigation/navigation_pos_estimator_private.h
typedef enum {
    ALTITUDE_SOURCE_GPS,
    ALTITUDE_SOURCE_BARO,
    ALTITUDE_SOURCE_GPS_ONLY,
    ALTITUDE_SOURCE_BARO_ONLY,
} navDefaultAltitudeSensor_e;

// ../../../src/main/navigation/navigation_fw_launch.c
typedef enum {
    FW_LAUNCH_MESSAGE_TYPE_NONE = 0,
    FW_LAUNCH_MESSAGE_TYPE_WAIT_THROTTLE,
    FW_LAUNCH_MESSAGE_TYPE_WAIT_IDLE,
    FW_LAUNCH_MESSAGE_TYPE_WAIT_DETECTION,
    FW_LAUNCH_MESSAGE_TYPE_IN_PROGRESS,
    FW_LAUNCH_MESSAGE_TYPE_FINISHING
} fixedWingLaunchMessage_t;

// ../../../src/main/navigation/navigation_fw_launch.c
typedef enum {
    FW_LAUNCH_EVENT_NONE = 0,
    FW_LAUNCH_EVENT_SUCCESS,
    FW_LAUNCH_EVENT_GOTO_DETECTION,
    FW_LAUNCH_EVENT_ABORT,
    FW_LAUNCH_EVENT_THROTTLE_LOW,
    FW_LAUNCH_EVENT_COUNT
} fixedWingLaunchEvent_t;

// ../../../src/main/navigation/navigation_fw_launch.c
typedef enum {  
    FW_LAUNCH_STATE_WAIT_THROTTLE = 0,
    FW_LAUNCH_STATE_IDLE_WIGGLE_WAIT,
    FW_LAUNCH_STATE_IDLE_MOTOR_DELAY,
    FW_LAUNCH_STATE_MOTOR_IDLE,
    FW_LAUNCH_STATE_WAIT_DETECTION,
    FW_LAUNCH_STATE_DETECTED,           
    FW_LAUNCH_STATE_MOTOR_DELAY,
    FW_LAUNCH_STATE_MOTOR_SPINUP,
    FW_LAUNCH_STATE_IN_PROGRESS,
    FW_LAUNCH_STATE_FINISH,
    FW_LAUNCH_STATE_ABORTED,            
    FW_LAUNCH_STATE_FLYING,             
    FW_LAUNCH_STATE_COUNT
} fixedWingLaunchState_t;

// ../../../src/main/navigation/navigation.h
typedef enum {
    SAFEHOME_USAGE_OFF = 0,    
    SAFEHOME_USAGE_RTH = 1,    
    SAFEHOME_USAGE_RTH_FS = 2, 
} safehomeUsageMode_e;

// ../../../src/main/navigation/navigation.h
typedef enum  {
    FW_AUTOLAND_APPROACH_DIRECTION_LEFT,
    FW_AUTOLAND_APPROACH_DIRECTION_RIGHT
} fwAutolandApproachDirection_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    FW_AUTOLAND_STATE_IDLE,
    FW_AUTOLAND_STATE_LOITER,
    FW_AUTOLAND_STATE_DOWNWIND,
    FW_AUTOLAND_STATE_BASE_LEG,
    FW_AUTOLAND_STATE_FINAL_APPROACH,
    FW_AUTOLAND_STATE_GLIDE,
    FW_AUTOLAND_STATE_FLARE
} fwAutolandState_t;

// ../../../src/main/navigation/navigation.h
typedef enum {
    GEOZONE_MESSAGE_STATE_NONE,
    GEOZONE_MESSAGE_STATE_NFZ,
    GEOZONE_MESSAGE_STATE_LEAVING_FZ,
    GEOZONE_MESSAGE_STATE_OUTSIDE_FZ,
    GEOZONE_MESSAGE_STATE_ENTERING_NFZ,
    GEOZONE_MESSAGE_STATE_AVOIDING_FB,
    GEOZONE_MESSAGE_STATE_RETURN_TO_ZONE,
    GEOZONE_MESSAGE_STATE_FLYOUT_NFZ,
    GEOZONE_MESSAGE_STATE_AVOIDING_ALTITUDE_BREACH,
    GEOZONE_MESSAGE_STATE_POS_HOLD
} geozoneMessageState_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    NAV_RESET_NEVER = 0,
    NAV_RESET_ON_FIRST_ARM,
    NAV_RESET_ON_EACH_ARM,
} nav_reset_type_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    NAV_RTH_ALLOW_LANDING_NEVER = 0,
    NAV_RTH_ALLOW_LANDING_ALWAYS = 1,
    NAV_RTH_ALLOW_LANDING_FS_ONLY = 2, 
} navRTHAllowLanding_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    NAV_EXTRA_ARMING_SAFETY_ON = 0,
    NAV_EXTRA_ARMING_SAFETY_ALLOW_BYPASS = 1, 
} navExtraArmingSafety_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    NAV_ARMING_BLOCKER_NONE = 0,
    NAV_ARMING_BLOCKER_MISSING_GPS_FIX = 1,
    NAV_ARMING_BLOCKER_NAV_IS_ALREADY_ACTIVE = 2,
    NAV_ARMING_BLOCKER_FIRST_WAYPOINT_TOO_FAR = 3,
    NAV_ARMING_BLOCKER_JUMP_WAYPOINT_ERROR = 4,
} navArmingBlocker_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    NOMS_OFF_ALWAYS,
    NOMS_OFF,
    NOMS_AUTO_ONLY,
    NOMS_ALL_NAV
} navOverridesMotorStop_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    RTH_CLIMB_OFF,
    RTH_CLIMB_ON,
    RTH_CLIMB_ON_FW_SPIRAL,
} navRTHClimbFirst_e;

// ../../../src/main/navigation/navigation.h
typedef enum {  
    FW_LAUNCH_DETECTED = 5,
    FW_LAUNCH_ABORTED = 10,
    FW_LAUNCH_FLYING = 11,
} navFwLaunchStatus_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    WP_PLAN_WAIT,
    WP_PLAN_SAVE,
    WP_PLAN_OK,
    WP_PLAN_FULL,
} wpMissionPlannerStatus_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    WP_MISSION_START,
    WP_MISSION_RESUME,
    WP_MISSION_SWITCH,
} navMissionRestart_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    RTH_TRACKBACK_OFF,
    RTH_TRACKBACK_ON,
    RTH_TRACKBACK_FS,
} rthTrackbackMode_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    WP_TURN_SMOOTHING_OFF,
    WP_TURN_SMOOTHING_ON,
    WP_TURN_SMOOTHING_CUT,
} wpFwTurnSmoothing_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    MC_ALT_HOLD_STICK,
    MC_ALT_HOLD_MID,
    MC_ALT_HOLD_HOVER,
} navMcAltHoldThrottle_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    NAV_WP_ACTION_WAYPOINT  = 0x01,
    NAV_WP_ACTION_HOLD_TIME = 0x03,
    NAV_WP_ACTION_RTH       = 0x04,
    NAV_WP_ACTION_SET_POI   = 0x05,
    NAV_WP_ACTION_JUMP      = 0x06,
    NAV_WP_ACTION_SET_HEAD  = 0x07,
    NAV_WP_ACTION_LAND      = 0x08
} navWaypointActions_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    NAV_WP_HEAD_MODE_NONE  = 0,
    NAV_WP_HEAD_MODE_POI   = 1,
    NAV_WP_HEAD_MODE_FIXED = 2
} navWaypointHeadings_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    NAV_WP_FLAG_HOME = 0x48,
    NAV_WP_FLAG_LAST = 0xA5
} navWaypointFlags_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    NAV_WP_ALTMODE = (1<<0),
    NAV_WP_USER1 = (1<<1),
    NAV_WP_USER2 = (1<<2),
    NAV_WP_USER3 = (1<<3),
    NAV_WP_USER4 = (1<<4)
} navWaypointP3Flags_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    MW_GPS_MODE_NONE = 0,
    MW_GPS_MODE_HOLD,
    MW_GPS_MODE_RTH,
    MW_GPS_MODE_NAV,
    MW_GPS_MODE_EMERG = 15
} navSystemStatus_Mode_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    MW_NAV_STATE_NONE = 0,                
    MW_NAV_STATE_RTH_START,               
    MW_NAV_STATE_RTH_ENROUTE,             
    MW_NAV_STATE_HOLD_INFINIT,            
    MW_NAV_STATE_HOLD_TIMED,              
    MW_NAV_STATE_WP_ENROUTE,              
    MW_NAV_STATE_PROCESS_NEXT,            
    MW_NAV_STATE_DO_JUMP,                 
    MW_NAV_STATE_LAND_START,              
    MW_NAV_STATE_LAND_IN_PROGRESS,        
    MW_NAV_STATE_LANDED,                  
    MW_NAV_STATE_LAND_SETTLE,             
    MW_NAV_STATE_LAND_START_DESCENT,      
    MW_NAV_STATE_HOVER_ABOVE_HOME,        
    MW_NAV_STATE_EMERGENCY_LANDING,       
    MW_NAV_STATE_RTH_CLIMB,               
} navSystemStatus_State_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    MW_NAV_ERROR_NONE = 0,            
    MW_NAV_ERROR_TOOFAR,              
    MW_NAV_ERROR_SPOILED_GPS,         
    MW_NAV_ERROR_WP_CRC,              
    MW_NAV_ERROR_FINISH,              
    MW_NAV_ERROR_TIMEWAIT,            
    MW_NAV_ERROR_INVALID_JUMP,        
    MW_NAV_ERROR_INVALID_DATA,        
    MW_NAV_ERROR_WAIT_FOR_RTH_ALT,    
    MW_NAV_ERROR_GPS_FIX_LOST,        
    MW_NAV_ERROR_DISARMED,            
    MW_NAV_ERROR_LANDING              
} navSystemStatus_Error_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    MW_NAV_FLAG_ADJUSTING_POSITION  = 1 << 0,
    MW_NAV_FLAG_ADJUSTING_ALTITUDE  = 1 << 1,
} navSystemStatus_Flags_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    GEO_ALT_ABSOLUTE,
    GEO_ALT_RELATIVE
} geoAltitudeConversionMode_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    GEO_ORIGIN_SET,
    GEO_ORIGIN_RESET_ALTITUDE
} geoOriginResetMode_e;

// ../../../src/main/navigation/navigation.h
typedef enum {
    NAV_WP_TAKEOFF_DATUM,
    NAV_WP_MSL_DATUM
} geoAltitudeDatumFlag_e;

// ../../../src/main/navigation/navigation_geozone.c
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

// ../../../src/main/sensors/battery_config_structs.h
typedef enum {
    CURRENT_SENSOR_NONE = 0,
    CURRENT_SENSOR_ADC,
    CURRENT_SENSOR_VIRTUAL,
    CURRENT_SENSOR_FAKE,
    CURRENT_SENSOR_ESC,
    CURRENT_SENSOR_MAX = CURRENT_SENSOR_FAKE
} currentSensor_e;

// ../../../src/main/sensors/battery_config_structs.h
typedef enum {
    VOLTAGE_SENSOR_NONE = 0,
    VOLTAGE_SENSOR_ADC,
    VOLTAGE_SENSOR_ESC,
    VOLTAGE_SENSOR_FAKE
} voltageSensor_e;

// ../../../src/main/sensors/battery_config_structs.h
typedef enum {
    BAT_CAPACITY_UNIT_MAH,
    BAT_CAPACITY_UNIT_MWH,
} batCapacityUnit_e;

// ../../../src/main/sensors/battery_config_structs.h
typedef enum {
    BAT_VOLTAGE_RAW,
    BAT_VOLTAGE_SAG_COMP
} batVoltageSource_e;

// ../../../src/main/sensors/rangefinder.h
typedef enum {
    RANGEFINDER_NONE                = 0,
    RANGEFINDER_SRF10               = 1,
    RANGEFINDER_VL53L0X             = 2,
    RANGEFINDER_MSP                 = 3,
    RANGEFINDER_BENEWAKE            = 4,
    RANGEFINDER_VL53L1X             = 5,
    RANGEFINDER_US42                = 6,
    RANGEFINDER_TOF10102I2C         = 7,
    RANGEFINDER_FAKE                = 8,
    RANGEFINDER_TERARANGER_EVO      = 9,
    RANGEFINDER_USD1_V0             = 10,
    RANGEFINDER_NANORADAR           = 11,
} rangefinderType_e;

// ../../../src/main/sensors/gyro.h
typedef enum {
    GYRO_NONE = 0,
    GYRO_AUTODETECT,
    GYRO_MPU6000,
    GYRO_MPU6500,
    GYRO_MPU9250,
    GYRO_BMI160,
    GYRO_ICM20689,
    GYRO_BMI088,
    GYRO_ICM42605,
    GYRO_BMI270,
    GYRO_LSM6DXX,
    GYRO_FAKE
   
} gyroSensor_e;

// ../../../src/main/sensors/gyro.h
typedef enum {
    DYNAMIC_NOTCH_MODE_2D = 0,
    DYNAMIC_NOTCH_MODE_3D
} dynamicGyroNotchMode_e;

// ../../../src/main/sensors/gyro.h
typedef enum {
    GYRO_FILTER_MODE_OFF = 0,
    GYRO_FILTER_MODE_STATIC = 1,
    GYRO_FILTER_MODE_DYNAMIC = 2,
    GYRO_FILTER_MODE_ADAPTIVE = 3
} gyroFilterMode_e;

// ../../../src/main/sensors/opflow.h
typedef enum {
    OPFLOW_NONE         = 0,
    OPFLOW_CXOF         = 1,
    OPFLOW_MSP          = 2,
    OPFLOW_FAKE         = 3,
} opticalFlowSensor_e;

// ../../../src/main/sensors/opflow.h
typedef enum {
    OPFLOW_QUALITY_INVALID,
    OPFLOW_QUALITY_VALID
} opflowQuality_e;

// ../../../src/main/sensors/battery.h
typedef enum {
    BATTERY_OK = 0,
    BATTERY_WARNING,
    BATTERY_CRITICAL,
    BATTERY_NOT_PRESENT
} batteryState_e;

// ../../../src/main/sensors/temperature.h
typedef enum {
    TEMP_SENSOR_NONE = 0,
    TEMP_SENSOR_LM75,
    TEMP_SENSOR_DS18B20
} tempSensorType_e;

// ../../../src/main/sensors/pitotmeter.h
typedef enum {
    PITOT_NONE = 0,
    PITOT_AUTODETECT = 1,
    PITOT_MS4525 = 2,
    PITOT_ADC = 3,
    PITOT_VIRTUAL = 4,
    PITOT_FAKE = 5,
    PITOT_MSP = 6,
    PITOT_DLVR = 7,
} pitotSensor_e;

// ../../../src/main/sensors/esc_sensor.c
typedef enum {
    ESC_SENSOR_WAIT_STARTUP = 0,
    ESC_SENSOR_READY = 1,
    ESC_SENSOR_WAITING = 2
} escSensorState_t;

// ../../../src/main/sensors/esc_sensor.c
typedef enum {
    ESC_SENSOR_FRAME_PENDING,
    ESC_SENSOR_FRAME_COMPLETE,
    ESC_SENSOR_FRAME_FAILED
} escSensorFrameStatus_t;

// ../../../src/main/sensors/sensors.h
typedef enum {
    SENSOR_INDEX_GYRO = 0,
    SENSOR_INDEX_ACC,
    SENSOR_INDEX_BARO,
    SENSOR_INDEX_MAG,
    SENSOR_INDEX_RANGEFINDER,
    SENSOR_INDEX_PITOT,
    SENSOR_INDEX_OPFLOW,
    SENSOR_INDEX_COUNT
} sensorIndex_e;

// ../../../src/main/sensors/sensors.h
typedef enum {
    SENSOR_GYRO = 1 << 0, 
    SENSOR_ACC = 1 << 1,
    SENSOR_BARO = 1 << 2,
    SENSOR_MAG = 1 << 3,
    SENSOR_RANGEFINDER = 1 << 4,
    SENSOR_PITOT = 1 << 5,
    SENSOR_OPFLOW = 1 << 6,
    SENSOR_GPS = 1 << 7,
    SENSOR_GPSMAG = 1 << 8,
    SENSOR_TEMP = 1 << 9
} sensors_e;

// ../../../src/main/sensors/sensors.h
typedef enum {
    SENSOR_TEMP_CAL_INITIALISE,
    SENSOR_TEMP_CAL_IN_PROGRESS,
    SENSOR_TEMP_CAL_COMPLETE,
} sensorTempCalState_e;

// ../../../src/main/sensors/barometer.c
typedef enum {
    BAROMETER_NEEDS_SAMPLES = 0,
    BAROMETER_NEEDS_CALCULATION
} barometerState_e;

// ../../../src/main/sensors/acceleration.h
typedef enum {
    ACC_NONE = 0,
    ACC_AUTODETECT,
    ACC_MPU6000,
    ACC_MPU6500,
    ACC_MPU9250,
    ACC_BMI160,
    ACC_ICM20689,
    ACC_BMI088,
    ACC_ICM42605,
    ACC_BMI270,
    ACC_LSM6DXX,
    ACC_FAKE,
    ACC_MAX = ACC_FAKE
} accelerationSensor_e;

// ../../../src/main/sensors/barometer.h
typedef enum {
    BARO_NONE = 0,
    BARO_AUTODETECT = 1,
    BARO_BMP085 = 2,
    BARO_MS5611 = 3,
    BARO_BMP280 = 4,
    BARO_MS5607 = 5,
    BARO_LPS25H = 6,
    BARO_SPL06  = 7,
    BARO_BMP388 = 8,
    BARO_DPS310 = 9,
    BARO_B2SMPB = 10,
    BARO_MSP    = 11,
    BARO_FAKE   = 12,
    BARO_MAX    = BARO_FAKE
} baroSensor_e;

// ../../../src/main/sensors/diagnostics.h
typedef enum {
    HW_SENSOR_NONE          = 0,    
    HW_SENSOR_OK            = 1,    
    HW_SENSOR_UNAVAILABLE   = 2,    
    HW_SENSOR_UNHEALTHY     = 3,    
} hardwareSensorStatus_e;

// ../../../src/main/sensors/compass.h
typedef enum {
    MAG_NONE = 0,
    MAG_AUTODETECT,
    MAG_HMC5883,
    MAG_AK8975,
    MAG_MAG3110,
    MAG_AK8963,
    MAG_IST8310,
    MAG_QMC5883,
    MAG_MPU9250,
    MAG_IST8308,
    MAG_LIS3MDL,
    MAG_MSP,
    MAG_RM3100,
    MAG_VCM5883,
    MAG_MLX90393,
    MAG_FAKE,
    MAG_MAX = MAG_FAKE
} magSensor_e;

// ../../../src/main/programming/logic_condition.h
typedef enum {
    LOGIC_CONDITION_TRUE                        = 0,
    LOGIC_CONDITION_EQUAL                       = 1,
    LOGIC_CONDITION_GREATER_THAN                = 2,
    LOGIC_CONDITION_LOWER_THAN                  = 3,
    LOGIC_CONDITION_LOW                         = 4,
    LOGIC_CONDITION_MID                         = 5,
    LOGIC_CONDITION_HIGH                        = 6,
    LOGIC_CONDITION_AND                         = 7,
    LOGIC_CONDITION_OR                          = 8,
    LOGIC_CONDITION_XOR                         = 9,
    LOGIC_CONDITION_NAND                        = 10,
    LOGIC_CONDITION_NOR                         = 11,
    LOGIC_CONDITION_NOT                         = 12,
    LOGIC_CONDITION_STICKY                      = 13,
    LOGIC_CONDITION_ADD                         = 14,
    LOGIC_CONDITION_SUB                         = 15,
    LOGIC_CONDITION_MUL                         = 16,
    LOGIC_CONDITION_DIV                         = 17,
    LOGIC_CONDITION_GVAR_SET                    = 18,
    LOGIC_CONDITION_GVAR_INC                    = 19,
    LOGIC_CONDITION_GVAR_DEC                    = 20,
    LOGIC_CONDITION_PORT_SET                    = 21,
    LOGIC_CONDITION_OVERRIDE_ARMING_SAFETY      = 22,
    LOGIC_CONDITION_OVERRIDE_THROTTLE_SCALE     = 23,
    LOGIC_CONDITION_SWAP_ROLL_YAW               = 24,
    LOGIC_CONDITION_SET_VTX_POWER_LEVEL         = 25,
    LOGIC_CONDITION_INVERT_ROLL                 = 26,
    LOGIC_CONDITION_INVERT_PITCH                = 27,
    LOGIC_CONDITION_INVERT_YAW                  = 28,
    LOGIC_CONDITION_OVERRIDE_THROTTLE           = 29,
    LOGIC_CONDITION_SET_VTX_BAND                = 30,
    LOGIC_CONDITION_SET_VTX_CHANNEL             = 31,
    LOGIC_CONDITION_SET_OSD_LAYOUT              = 32,
    LOGIC_CONDITION_SIN                         = 33,
    LOGIC_CONDITION_COS                         = 34,
    LOGIC_CONDITION_TAN                         = 35,
    LOGIC_CONDITION_MAP_INPUT                   = 36,
    LOGIC_CONDITION_MAP_OUTPUT                  = 37,
    LOGIC_CONDITION_RC_CHANNEL_OVERRIDE         = 38,
    LOGIC_CONDITION_SET_HEADING_TARGET          = 39,
    LOGIC_CONDITION_MODULUS                     = 40,
    LOGIC_CONDITION_LOITER_OVERRIDE             = 41,
    LOGIC_CONDITION_SET_PROFILE                 = 42,
    LOGIC_CONDITION_MIN                         = 43,
    LOGIC_CONDITION_MAX                         = 44,
    LOGIC_CONDITION_FLIGHT_AXIS_ANGLE_OVERRIDE  = 45,
    LOGIC_CONDITION_FLIGHT_AXIS_RATE_OVERRIDE   = 46,
    LOGIC_CONDITION_EDGE                        = 47,
    LOGIC_CONDITION_DELAY                       = 48,
    LOGIC_CONDITION_TIMER                       = 49,
    LOGIC_CONDITION_DELTA                       = 50,
    LOGIC_CONDITION_APPROX_EQUAL                = 51,
    LOGIC_CONDITION_LED_PIN_PWM                 = 52,
    LOGIC_CONDITION_DISABLE_GPS_FIX             = 53,
    LOGIC_CONDITION_RESET_MAG_CALIBRATION       = 54,
    LOGIC_CONDITION_SET_GIMBAL_SENSITIVITY      = 55,
    LOGIC_CONDITION_LAST                        = 56,
} logicOperation_e;

// ../../../src/main/programming/logic_condition.h
typedef enum logicOperandType_s {
    LOGIC_CONDITION_OPERAND_TYPE_VALUE = 0,
    LOGIC_CONDITION_OPERAND_TYPE_RC_CHANNEL,
    LOGIC_CONDITION_OPERAND_TYPE_FLIGHT,
    LOGIC_CONDITION_OPERAND_TYPE_FLIGHT_MODE,
    LOGIC_CONDITION_OPERAND_TYPE_LC,    
    LOGIC_CONDITION_OPERAND_TYPE_GVAR,  
    LOGIC_CONDITION_OPERAND_TYPE_PID,  
    LOGIC_CONDITION_OPERAND_TYPE_WAYPOINTS,
    LOGIC_CONDITION_OPERAND_TYPE_LAST
} logicOperandType_e;

// ../../../src/main/programming/logic_condition.h
typedef enum {
    LOGIC_CONDITION_OPERAND_FLIGHT_ARM_TIMER = 0, 
    LOGIC_CONDITION_OPERAND_FLIGHT_HOME_DISTANCE, 
    LOGIC_CONDITION_OPERAND_FLIGHT_TRIP_DISTANCE, 
    LOGIC_CONDITION_OPERAND_FLIGHT_RSSI,                                    
    LOGIC_CONDITION_OPERAND_FLIGHT_VBAT, 
    LOGIC_CONDITION_OPERAND_FLIGHT_CELL_VOLTAGE, 
    LOGIC_CONDITION_OPERAND_FLIGHT_CURRENT, 
    LOGIC_CONDITION_OPERAND_FLIGHT_MAH_DRAWN, 
    LOGIC_CONDITION_OPERAND_FLIGHT_GPS_SATS,                                
    LOGIC_CONDITION_OPERAND_FLIGHT_GROUD_SPEED, 
    LOGIC_CONDITION_OPERAND_FLIGHT_3D_SPEED, 
    LOGIC_CONDITION_OPERAND_FLIGHT_AIR_SPEED, 
    LOGIC_CONDITION_OPERAND_FLIGHT_ALTITUDE, 
    LOGIC_CONDITION_OPERAND_FLIGHT_VERTICAL_SPEED, 
    LOGIC_CONDITION_OPERAND_FLIGHT_TROTTLE_POS, 
    LOGIC_CONDITION_OPERAND_FLIGHT_ATTITUDE_ROLL, 
    LOGIC_CONDITION_OPERAND_FLIGHT_ATTITUDE_PITCH, 
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_ARMED, 
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_AUTOLAUNCH, 
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_ALTITUDE_CONTROL, 
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_POSITION_CONTROL, 
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_EMERGENCY_LANDING, 
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_RTH, 
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_LANDING, 
    LOGIC_CONDITION_OPERAND_FLIGHT_IS_FAILSAFE, 
    LOGIC_CONDITION_OPERAND_FLIGHT_STABILIZED_ROLL,                         
    LOGIC_CONDITION_OPERAND_FLIGHT_STABILIZED_PITCH,                        
    LOGIC_CONDITION_OPERAND_FLIGHT_STABILIZED_YAW,                          
    LOGIC_CONDITION_OPERAND_FLIGHT_3D_HOME_DISTANCE,                        
    LOGIC_CONDITION_OPERAND_FLIGHT_LQ_UPLINK,                               
    LOGIC_CONDITION_OPERAND_FLIGHT_SNR,                                     
    LOGIC_CONDITION_OPERAND_FLIGHT_GPS_VALID, 
    LOGIC_CONDITION_OPERAND_FLIGHT_LOITER_RADIUS,                           
    LOGIC_CONDITION_OPERAND_FLIGHT_ACTIVE_PROFILE, 
    LOGIC_CONDITION_OPERAND_FLIGHT_BATT_CELLS,                              
    LOGIC_CONDITION_OPERAND_FLIGHT_AGL_STATUS, 
    LOGIC_CONDITION_OPERAND_FLIGHT_AGL, 
    LOGIC_CONDITION_OPERAND_FLIGHT_RANGEFINDER_RAW, 
    LOGIC_CONDITION_OPERAND_FLIGHT_ACTIVE_MIXER_PROFILE, 
    LOGIC_CONDITION_OPERAND_FLIGHT_MIXER_TRANSITION_ACTIVE, 
    LOGIC_CONDITION_OPERAND_FLIGHT_ATTITUDE_YAW, 
    LOGIC_CONDITION_OPERAND_FLIGHT_FW_LAND_STATE,                           
    LOGIC_CONDITION_OPERAND_FLIGHT_BATT_PROFILE, 
    LOGIC_CONDITION_OPERAND_FLIGHT_FLOWN_LOITER_RADIUS,                     
    LOGIC_CONDITION_OPERAND_FLIGHT_LQ_DOWNLINK,                             
    LOGIC_CONDITION_OPERAND_FLIGHT_UPLINK_RSSI_DBM,                         
} logicFlightOperands_e;

// ../../../src/main/programming/logic_condition.h
typedef enum {
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_FAILSAFE,                           
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_MANUAL,                             
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_RTH,                                
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_POSHOLD,                            
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_CRUISE,                             
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_ALTHOLD,                            
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_ANGLE,                              
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_HORIZON,                            
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_AIR,                                
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_USER1,                              
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_USER2,                              
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_COURSE_HOLD,                        
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_USER3,                              
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_USER4,                              
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_ACRO,                               
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_WAYPOINT_MISSION,                   
    LOGIC_CONDITION_OPERAND_FLIGHT_MODE_ANGLEHOLD,                          
} logicFlightModeOperands_e;

// ../../../src/main/programming/logic_condition.h
typedef enum {
    LOGIC_CONDITION_OPERAND_WAYPOINTS_IS_WP, 
    LOGIC_CONDITION_OPERAND_WAYPOINTS_WAYPOINT_INDEX,                       
    LOGIC_CONDITION_OPERAND_WAYPOINTS_WAYPOINT_ACTION,                      
    LOGIC_CONDITION_OPERAND_WAYPOINTS_NEXT_WAYPOINT_ACTION,                 
    LOGIC_CONDITION_OPERAND_WAYPOINTS_WAYPOINT_DISTANCE,                    
    LOGIC_CONDTIION_OPERAND_WAYPOINTS_DISTANCE_FROM_WAYPOINT,               
    LOGIC_CONDITION_OPERAND_WAYPOINTS_USER1_ACTION,                         
    LOGIC_CONDITION_OPERAND_WAYPOINTS_USER2_ACTION,                         
    LOGIC_CONDITION_OPERAND_WAYPOINTS_USER3_ACTION,                         
    LOGIC_CONDITION_OPERAND_WAYPOINTS_USER4_ACTION,                         
    LOGIC_CONDITION_OPERAND_WAYPOINTS_USER1_ACTION_NEXT_WP,                 
    LOGIC_CONDITION_OPERAND_WAYPOINTS_USER2_ACTION_NEXT_WP,                 
    LOGIC_CONDITION_OPERAND_WAYPOINTS_USER3_ACTION_NEXT_WP,                 
    LOGIC_CONDITION_OPERAND_WAYPOINTS_USER4_ACTION_NEXT_WP,                 
} logicWaypointOperands_e;

// ../../../src/main/programming/logic_condition.h
typedef enum {
    LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_ARMING_SAFETY = (1 << 0),
    LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_THROTTLE_SCALE = (1 << 1),
    LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_SWAP_ROLL_YAW = (1 << 2),
    LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_ROLL = (1 << 3),
    LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_PITCH = (1 << 4),
    LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_INVERT_YAW = (1 << 5),
    LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_THROTTLE = (1 << 6),
    LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_OSD_LAYOUT = (1 << 7),
    LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_RC_CHANNEL = (1 << 8),
    LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_LOITER_RADIUS = (1 << 9),
    LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_FLIGHT_AXIS = (1 << 10),
#ifdef USE_GPS_FIX_ESTIMATION
    LOGIC_CONDITION_GLOBAL_FLAG_DISABLE_GPS_FIX = (1 << 11),
#endif
} logicConditionsGlobalFlags_t;

// ../../../src/main/programming/logic_condition.h
typedef enum {
    LOGIC_CONDITION_FLAG_LATCH              = 1 << 0,
    LOGIC_CONDITION_FLAG_TIMEOUT_SATISFIED  = 1 << 1,
} logicConditionFlags_e;

// ../../../src/main/rx/rx.h
typedef enum {
    RX_FRAME_PENDING             = 0,         
    RX_FRAME_COMPLETE            = (1 << 0),  
    RX_FRAME_FAILSAFE            = (1 << 1),  
    RX_FRAME_PROCESSING_REQUIRED = (1 << 2),
    RX_FRAME_DROPPED             = (1 << 3),  
} rxFrameState_e;

// ../../../src/main/rx/rx.h
typedef enum {
    RX_TYPE_NONE = 0,
    RX_TYPE_SERIAL,
    RX_TYPE_MSP,
    RX_TYPE_SIM
} rxReceiverType_e;

// ../../../src/main/rx/rx.h
typedef enum {
    SERIALRX_SPEKTRUM1024 = 0,
    SERIALRX_SPEKTRUM2048,
    SERIALRX_SBUS,
    SERIALRX_SUMD,
    SERIALRX_IBUS,
    SERIALRX_JETIEXBUS,
    SERIALRX_CRSF,
    SERIALRX_FPORT,
    SERIALRX_SBUS_FAST,
    SERIALRX_FPORT2,
    SERIALRX_SRXL2,
    SERIALRX_GHST,
    SERIALRX_MAVLINK,
    SERIALRX_FBUS,
    SERIALRX_SBUS2,
} rxSerialReceiverType_e;

// ../../../src/main/rx/rx.h
typedef enum {
    RSSI_SOURCE_NONE = 0,
    RSSI_SOURCE_AUTO,
    RSSI_SOURCE_ADC,
    RSSI_SOURCE_RX_CHANNEL,
    RSSI_SOURCE_RX_PROTOCOL,
    RSSI_SOURCE_MSP,
} rssiSource_e;

// ../../../src/main/rx/crsf.h
typedef enum {
    CRSF_ADDRESS_BROADCAST = 0x00,
    CRSF_ADDRESS_USB = 0x10,
    CRSF_ADDRESS_TBS_CORE_PNP_PRO = 0x80,
    CRSF_ADDRESS_RESERVED1 = 0x8A,
    CRSF_ADDRESS_CURRENT_SENSOR = 0xC0,
    CRSF_ADDRESS_GPS = 0xC2,
    CRSF_ADDRESS_TBS_BLACKBOX = 0xC4,
    CRSF_ADDRESS_FLIGHT_CONTROLLER = 0xC8,
    CRSF_ADDRESS_RESERVED2 = 0xCA,
    CRSF_ADDRESS_RACE_TAG = 0xCC,
    CRSF_ADDRESS_RADIO_TRANSMITTER = 0xEA,
    CRSF_ADDRESS_CRSF_RECEIVER = 0xEC,
    CRSF_ADDRESS_CRSF_TRANSMITTER = 0xEE
} crsfAddress_e;

// ../../../src/main/rx/crsf.h
typedef enum {
    CRSF_FRAMETYPE_GPS = 0x02,
    CRSF_FRAMETYPE_VARIO_SENSOR = 0x07,
    CRSF_FRAMETYPE_BATTERY_SENSOR = 0x08,
    CRSF_FRAMETYPE_BAROMETER_ALTITUDE = 0x09,
    CRSF_FRAMETYPE_LINK_STATISTICS = 0x14,
    CRSF_FRAMETYPE_RC_CHANNELS_PACKED = 0x16,
    CRSF_FRAMETYPE_ATTITUDE = 0x1E,
    CRSF_FRAMETYPE_FLIGHT_MODE = 0x21,
    
    CRSF_FRAMETYPE_DEVICE_PING = 0x28,
    CRSF_FRAMETYPE_DEVICE_INFO = 0x29,
    CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY = 0x2B,
    CRSF_FRAMETYPE_PARAMETER_READ = 0x2C,
    CRSF_FRAMETYPE_PARAMETER_WRITE = 0x2D,
    CRSF_FRAMETYPE_COMMAND = 0x32,
    
    CRSF_FRAMETYPE_MSP_REQ = 0x7A,   
    CRSF_FRAMETYPE_MSP_RESP = 0x7B,  
    CRSF_FRAMETYPE_MSP_WRITE = 0x7C,  
    CRSF_FRAMETYPE_DISPLAYPORT_CMD = 0x7D, 
} crsfFrameType_e;

// ../../../src/main/rx/sbus.c
typedef enum {
    STATE_SBUS_SYNC = 0,
    STATE_SBUS_PAYLOAD,
    STATE_SBUS26_PAYLOAD,
    STATE_SBUS_WAIT_SYNC
} sbusDecoderState_e;

// ../../../src/main/rx/ghst_protocol.h
typedef enum {
    GHST_ADDR_RADIO             = 0x80,
    GHST_ADDR_TX_MODULE_SYM     = 0x81,     
    GHST_ADDR_TX_MODULE_ASYM    = 0x88,     
    GHST_ADDR_FC                = 0x82,
    GHST_ADDR_GOGGLES           = 0x83,
    GHST_ADDR_QUANTUM_TEE1      = 0x84,     
    GHST_ADDR_QUANTUM_TEE2      = 0x85,
    GHST_ADDR_QUANTUM_GW1       = 0x86,
    GHST_ADDR_5G_CLK            = 0x87,     
    GHST_ADDR_RX                = 0x89
} ghstAddr_e;

// ../../../src/main/rx/ghst_protocol.h
typedef enum {
    
    
    
    GHST_UL_RC_CHANS_HS4_FIRST  = 0x10,     
    GHST_UL_RC_CHANS_HS4_5TO8   = 0x10,     
    GHST_UL_RC_CHANS_HS4_9TO12  = 0x11,     
    GHST_UL_RC_CHANS_HS4_13TO16 = 0x12,     
    GHST_UL_RC_CHANS_HS4_RSSI   = 0x13,     
    GHST_UL_RC_CHANS_HS4_LAST   = 0x1f      
} ghstUl_e;

// ../../../src/main/rx/ghst_protocol.h
typedef enum {
    GHST_DL_OPENTX_SYNC         = 0x20,
    GHST_DL_LINK_STAT           = 0x21,
    GHST_DL_VTX_STAT            = 0x22,
    GHST_DL_PACK_STAT           = 0x23,     
    GHST_DL_GPS_PRIMARY         = 0x25,     
    GHST_DL_GPS_SECONDARY       = 0x26      
} ghstDl_e;

// ../../../src/main/rx/fport2.c
typedef enum {
    CFT_RC = 0xFF,
    CFT_OTA_START = 0xF0,
    CFT_OTA_DATA = 0xF1,
    CFT_OTA_STOP = 0xF2
} fport2_control_frame_type_e;

// ../../../src/main/rx/fport2.c
typedef enum {
    FT_CONTROL,
    FT_DOWNLINK
} frame_type_e;

// ../../../src/main/rx/fport2.c
typedef enum {
    FS_CONTROL_FRAME_START,
    FS_CONTROL_FRAME_TYPE,
    FS_CONTROL_FRAME_DATA,
    FS_DOWNLINK_FRAME_START,
    FS_DOWNLINK_FRAME_DATA
} frame_state_e;

// ../../../src/main/rx/srxl2_types.h
typedef enum {
    Disabled,
    ListenForActivity,
    SendHandshake,
    ListenForHandshake,
    Running
} Srxl2State;

// ../../../src/main/rx/srxl2_types.h
typedef enum {
    Handshake = 0x21,
    BindInfo = 0x41,
    ParameterConfiguration = 0x50,
    SignalQuality = 0x55,
    TelemetrySensorData = 0x80,
    ControlData = 0xCD,
} Srxl2PacketType;

// ../../../src/main/rx/srxl2_types.h
typedef enum {
    ChannelData = 0x00,
    FailsafeChannelData = 0x01,
    VTXData = 0x02,
} Srxl2ControlDataCommand;

// ../../../src/main/rx/srxl2_types.h
typedef enum {
    NoDevice = 0,
    RemoteReceiver = 1,
    Receiver = 2,
    FlightController = 3,
    ESC = 4,
    Reserved = 5,
    SRXLServo = 6,
    SRXLServo_2 = 7,
    VTX = 8,
} Srxl2DeviceType;

// ../../../src/main/rx/srxl2_types.h
typedef enum {
    FlightControllerDefault = 0x30,
    FlightControllerMax = 0x3F,
    Broadcast = 0xFF,
} Srxl2DeviceId;

// ../../../src/main/rx/srxl2_types.h
typedef enum {
    EnterBindMode = 0xEB,
    RequestBindStatus = 0xB5,
    BoundDataReport = 0xDB,
    SetBindInfo = 0x5B,
} Srxl2BindRequest;

// ../../../src/main/rx/srxl2_types.h
typedef enum {
    NotBound = 0x0,
    DSM2_1024_22ms = 0x01,
    DSM2_1024_MC24 = 0x02,
    DMS2_2048_11ms = 0x12,
    DMSX_22ms = 0xA2,
    DMSX_11ms = 0xB2,
    Surface_DSM2_16_5ms = 0x63,
    DSMR_11ms_22ms = 0xE2,
    DSMR_5_5ms = 0xE4,
} Srxl2BindType;

// ../../../src/main/telemetry/crsf.c
typedef enum {
    CRSF_ACTIVE_ANTENNA1 = 0,
    CRSF_ACTIVE_ANTENNA2 = 1
} crsfActiveAntenna_e;

// ../../../src/main/telemetry/crsf.c
typedef enum {
    CRSF_RF_MODE_4_HZ = 0,
    CRSF_RF_MODE_50_HZ = 1,
    CRSF_RF_MODE_150_HZ = 2
} crsrRfMode_e;

// ../../../src/main/telemetry/crsf.c
typedef enum {
    CRSF_RF_POWER_0_mW = 0,
    CRSF_RF_POWER_10_mW = 1,
    CRSF_RF_POWER_25_mW = 2,
    CRSF_RF_POWER_100_mW = 3,
    CRSF_RF_POWER_500_mW = 4,
    CRSF_RF_POWER_1000_mW = 5,
    CRSF_RF_POWER_2000_mW = 6,
    CRSF_RF_POWER_250_mW = 7
} crsrRfPower_e;

// ../../../src/main/telemetry/crsf.c
typedef enum {
    CRSF_FRAME_START_INDEX = 0,
    CRSF_FRAME_ATTITUDE_INDEX = CRSF_FRAME_START_INDEX,
    CRSF_FRAME_BATTERY_SENSOR_INDEX,
    CRSF_FRAME_FLIGHT_MODE_INDEX,
    CRSF_FRAME_GPS_INDEX,
    CRSF_FRAME_VARIO_SENSOR_INDEX,
    CRSF_FRAME_BAROMETER_ALTITUDE_INDEX,
    CRSF_SCHEDULE_COUNT_MAX
} crsfFrameTypeIndex_e;

// ../../../src/main/telemetry/telemetry.h
typedef enum {
    LTM_RATE_NORMAL,
    LTM_RATE_MEDIUM,
    LTM_RATE_SLOW
} ltmUpdateRate_e;

// ../../../src/main/telemetry/telemetry.h
typedef enum {
    MAVLINK_RADIO_GENERIC,
    MAVLINK_RADIO_ELRS,
    MAVLINK_RADIO_SIK,
} mavlinkRadio_e;

// ../../../src/main/telemetry/telemetry.h
typedef enum {
    SMARTPORT_FUEL_UNIT_PERCENT,
    SMARTPORT_FUEL_UNIT_MAH,
    SMARTPORT_FUEL_UNIT_MWH
} smartportFuelUnit_e;

// ../../../src/main/telemetry/hott.c
typedef enum {
    HOTT_WAITING_FOR_REQUEST,
    HOTT_RECEIVING_REQUEST,
    HOTT_WAITING_FOR_TX_WINDOW,
    HOTT_TRANSMITTING,
    HOTT_ENDING_TRANSMISSION
} hottState_e;

// ../../../src/main/telemetry/hott.c
typedef enum {
    GPS_FIX_CHAR_NONE = '-',
    GPS_FIX_CHAR_2D = '2',
    GPS_FIX_CHAR_3D = '3',
    GPS_FIX_CHAR_DGPS = 'D',
} gpsFixChar_e;

// ../../../src/main/telemetry/hott.h
typedef enum {
    HOTT_EAM_ALARM1_FLAG_NONE = 0,
    HOTT_EAM_ALARM1_FLAG_MAH = (1 << 0),
    HOTT_EAM_ALARM1_FLAG_BATTERY_1 = (1 << 1),
    HOTT_EAM_ALARM1_FLAG_BATTERY_2 = (1 << 2),
    HOTT_EAM_ALARM1_FLAG_TEMPERATURE_1 = (1 << 3),
    HOTT_EAM_ALARM1_FLAG_TEMPERATURE_2 = (1 << 4),
    HOTT_EAM_ALARM1_FLAG_ALTITUDE = (1 << 5),
    HOTT_EAM_ALARM1_FLAG_CURRENT = (1 << 6),
    HOTT_EAM_ALARM1_FLAG_MAIN_VOLTAGE = (1 << 7),
} hottEamAlarm1Flag_e;

// ../../../src/main/telemetry/hott.h
typedef enum {
    HOTT_EAM_ALARM2_FLAG_NONE = 0,
    HOTT_EAM_ALARM2_FLAG_MS = (1 << 0),
    HOTT_EAM_ALARM2_FLAG_M3S = (1 << 1),
    HOTT_EAM_ALARM2_FLAG_ALTITUDE_DUPLICATE = (1 << 2),
    HOTT_EAM_ALARM2_FLAG_MS_DUPLICATE = (1 << 3),
    HOTT_EAM_ALARM2_FLAG_M3S_DUPLICATE = (1 << 4),
    HOTT_EAM_ALARM2_FLAG_UNKNOWN_1 = (1 << 5),
    HOTT_EAM_ALARM2_FLAG_UNKNOWN_2 = (1 << 6),
    HOTT_EAM_ALARM2_FLAG_ON_SIGN_OR_TEXT_ACTIVE = (1 << 7),
} hottEamAlarm2Flag_e;

// ../../../src/main/telemetry/ltm.h
typedef enum {
    LTM_FRAME_START = 0,
    LTM_AFRAME = LTM_FRAME_START, 
    LTM_SFRAME, 
#if defined(USE_GPS)
    LTM_GFRAME, 
    LTM_OFRAME, 
    LTM_XFRAME, 
#endif
    LTM_NFRAME, 
    LTM_FRAME_COUNT
} ltm_frame_e;

// ../../../src/main/telemetry/ltm.h
typedef enum {
    LTM_MODE_MANUAL = 0,
    LTM_MODE_RATE,
    LTM_MODE_ANGLE,
    LTM_MODE_HORIZON,
    LTM_MODE_ACRO,
    LTM_MODE_STABALIZED1,
    LTM_MODE_STABALIZED2,
    LTM_MODE_STABILIZED3,
    LTM_MODE_ALTHOLD,
    LTM_MODE_GPSHOLD,
    LTM_MODE_WAYPOINTS,
    LTM_MODE_HEADHOLD,
    LTM_MODE_CIRCLE,
    LTM_MODE_RTH,
    LTM_MODE_FOLLOWWME,
    LTM_MODE_LAND,
    LTM_MODE_FLYBYWIRE1,
    LTM_MODE_FLYBYWIRE2,
    LTM_MODE_CRUISE,
    LTM_MODE_UNKNOWN,
        
    LTM_MODE_LAUNCH,
    LTM_MODE_AUTOTUNE
} ltm_modes_e;

// ../../../src/main/telemetry/ibus_shared.h
typedef enum {
    IBUS_MEAS_TYPE_INTERNAL_VOLTAGE = 0x00, 
    IBUS_MEAS_TYPE_TEMPERATURE      = 0x01, 
    IBUS_MEAS_TYPE_RPM              = 0x02, 
    IBUS_MEAS_TYPE_EXTERNAL_VOLTAGE = 0x03, 
    IBUS_MEAS_TYPE_HEADING          = 0x04, 
    IBUS_MEAS_TYPE_CURRENT          = 0x05, 
    IBUS_MEAS_TYPE_CLIMB            = 0x06, 
    IBUS_MEAS_TYPE_ACC_Z            = 0x07, 
    IBUS_MEAS_TYPE_ACC_Y            = 0x08, 
    IBUS_MEAS_TYPE_ACC_X            = 0x09, 
    IBUS_MEAS_TYPE_VSPEED           = 0x0a, 
    IBUS_MEAS_TYPE_SPEED            = 0x0b, 
    IBUS_MEAS_TYPE_DIST             = 0x0c, 
    IBUS_MEAS_TYPE_ARMED            = 0x0d,	
    IBUS_MEAS_TYPE_MODE             = 0x0e, 
    
    IBUS_MEAS_TYPE_PRES             = 0x41, 
    
    
    IBUS_MEAS_TYPE_SPE              = 0x7e, 
    IBUS_MEAS_TYPE_COG              = 0x80, 
    IBUS_MEAS_TYPE_GPS_STATUS       = 0x81, 
    IBUS_MEAS_TYPE_GPS_LON          = 0x82, 
    IBUS_MEAS_TYPE_GPS_LAT          = 0x83, 
    IBUS_MEAS_TYPE_ALT              = 0x84, 
    IBUS_MEAS_TYPE_S85              = 0x85, 
    IBUS_MEAS_TYPE_S86              = 0x86, 
    IBUS_MEAS_TYPE_S87              = 0x87, 
    IBUS_MEAS_TYPE_S88              = 0x88, 
    IBUS_MEAS_TYPE_S89              = 0x89, 
    IBUS_MEAS_TYPE_S8A              = 0x8A, 
    IBUS_MEAS_TYPE_GALT             = 0xf9, 
    
    
    
    IBUS_MEAS_TYPE_GPS              = 0xfd 
    
} ibusSensorType_e;

// ../../../src/main/telemetry/ibus_shared.h
typedef enum {
    IBUS_MEAS_TYPE1_INTV             = 0x00,
    IBUS_MEAS_TYPE1_TEM              = 0x01,
    IBUS_MEAS_TYPE1_MOT              = 0x02,
    IBUS_MEAS_TYPE1_EXTV             = 0x03,
    IBUS_MEAS_TYPE1_CELL             = 0x04,
    IBUS_MEAS_TYPE1_BAT_CURR         = 0x05,
    IBUS_MEAS_TYPE1_FUEL             = 0x06,
    IBUS_MEAS_TYPE1_RPM              = 0x07,
    IBUS_MEAS_TYPE1_CMP_HEAD         = 0x08,
    IBUS_MEAS_TYPE1_CLIMB_RATE       = 0x09,
    IBUS_MEAS_TYPE1_COG              = 0x0a,
    IBUS_MEAS_TYPE1_GPS_STATUS       = 0x0b,
    IBUS_MEAS_TYPE1_ACC_X            = 0x0c,
    IBUS_MEAS_TYPE1_ACC_Y            = 0x0d,
    IBUS_MEAS_TYPE1_ACC_Z            = 0x0e,
    IBUS_MEAS_TYPE1_ROLL             = 0x0f,
    IBUS_MEAS_TYPE1_PITCH            = 0x10,
    IBUS_MEAS_TYPE1_YAW              = 0x11,
    IBUS_MEAS_TYPE1_VERTICAL_SPEED   = 0x12,
    IBUS_MEAS_TYPE1_GROUND_SPEED     = 0x13,
    IBUS_MEAS_TYPE1_GPS_DIST         = 0x14,
    IBUS_MEAS_TYPE1_ARMED            = 0x15,
    IBUS_MEAS_TYPE1_FLIGHT_MODE      = 0x16,
    IBUS_MEAS_TYPE1_PRES             = 0x41,
    
    
    IBUS_MEAS_TYPE1_SPE              = 0x7e,
    
    IBUS_MEAS_TYPE1_GPS_LAT          = 0x80,
    IBUS_MEAS_TYPE1_GPS_LON          = 0x81,
    IBUS_MEAS_TYPE1_GPS_ALT          = 0x82,
    IBUS_MEAS_TYPE1_ALT              = 0x83,
    IBUS_MEAS_TYPE1_S84              = 0x84,
    IBUS_MEAS_TYPE1_S85              = 0x85,
    IBUS_MEAS_TYPE1_S86              = 0x86,
    IBUS_MEAS_TYPE1_S87              = 0x87,
    IBUS_MEAS_TYPE1_S88              = 0x88,
    IBUS_MEAS_TYPE1_S89              = 0x89,
    IBUS_MEAS_TYPE1_S8a              = 0x8a
    
    
    
    
} ibusSensorType1_e;

// ../../../src/main/telemetry/ibus_shared.h
typedef enum {
    IBUS_MEAS_VALUE_NONE             = 0x00, 
    IBUS_MEAS_VALUE_TEMPERATURE      = 0x01, 
    IBUS_MEAS_VALUE_MOT              = 0x02, 
    IBUS_MEAS_VALUE_EXTERNAL_VOLTAGE = 0x03, 
    IBUS_MEAS_VALUE_CELL             = 0x04, 
    IBUS_MEAS_VALUE_CURRENT          = 0x05, 
    IBUS_MEAS_VALUE_FUEL             = 0x06, 
    IBUS_MEAS_VALUE_RPM              = 0x07, 
    IBUS_MEAS_VALUE_HEADING          = 0x08, 
    IBUS_MEAS_VALUE_CLIMB            = 0x09, 
    IBUS_MEAS_VALUE_COG              = 0x0a, 
    IBUS_MEAS_VALUE_GPS_STATUS       = 0x0b, 
    IBUS_MEAS_VALUE_ACC_X            = 0x0c, 
    IBUS_MEAS_VALUE_ACC_Y            = 0x0d, 
    IBUS_MEAS_VALUE_ACC_Z            = 0x0e, 
    IBUS_MEAS_VALUE_ROLL             = 0x0f, 
    IBUS_MEAS_VALUE_PITCH            = 0x10, 
    IBUS_MEAS_VALUE_YAW              = 0x11, 
    IBUS_MEAS_VALUE_VSPEED           = 0x12, 
    IBUS_MEAS_VALUE_SPEED            = 0x13, 
    IBUS_MEAS_VALUE_DIST             = 0x14, 
    IBUS_MEAS_VALUE_ARMED            = 0x15, 
    IBUS_MEAS_VALUE_MODE             = 0x16, 
    IBUS_MEAS_VALUE_PRES             = 0x41, 
    IBUS_MEAS_VALUE_SPE              = 0x7e, 
    IBUS_MEAS_VALUE_GPS_LAT          = 0x80, 
    IBUS_MEAS_VALUE_GPS_LON          = 0x81, 
    IBUS_MEAS_VALUE_GALT4            = 0x82, 
    IBUS_MEAS_VALUE_ALT4             = 0x83, 
    IBUS_MEAS_VALUE_GALT             = 0x84, 
    IBUS_MEAS_VALUE_ALT              = 0x85, 
    IBUS_MEAS_VALUE_STATUS           = 0x87, 
    IBUS_MEAS_VALUE_GPS_LAT1         = 0x88, 
    IBUS_MEAS_VALUE_GPS_LON1         = 0x89, 
    IBUS_MEAS_VALUE_GPS_LAT2         = 0x90, 
    IBUS_MEAS_VALUE_GPS_LON2         = 0x91, 
    IBUS_MEAS_VALUE_GPS              = 0xfd 
} ibusSensorValue_e;

// ../../../src/main/telemetry/ibus_shared.c
typedef enum {
    IBUS_COMMAND_DISCOVER_SENSOR      = 0x80,
    IBUS_COMMAND_SENSOR_TYPE          = 0x90,
    IBUS_COMMAND_MEASUREMENT          = 0xA0
} ibusCommand_e;

// ../../../src/main/telemetry/sim.c
typedef enum  {
    SIM_MODULE_NOT_DETECTED = 0,
    SIM_MODULE_NOT_REGISTERED,
    SIM_MODULE_REGISTERED,
} simModuleState_e;

// ../../../src/main/telemetry/sim.c
typedef enum  {
    SIM_STATE_INIT = 0,
    SIM_STATE_INIT2,
    SIM_STATE_INIT_ENTER_PIN,
    SIM_STATE_SET_MODES,
    SIM_STATE_SEND_SMS,
    SIM_STATE_SEND_SMS_ENTER_MESSAGE
} simTelemetryState_e;

// ../../../src/main/telemetry/sim.c
typedef enum  {
    SIM_AT_OK = 0,
    SIM_AT_ERROR,
    SIM_AT_WAITING_FOR_RESPONSE
} simATCommandState_e;

// ../../../src/main/telemetry/sim.c
typedef enum  {
    SIM_READSTATE_RESPONSE = 0,
    SIM_READSTATE_SMS,
    SIM_READSTATE_SKIP
} simReadState_e;

// ../../../src/main/telemetry/sim.c
typedef enum  {
    SIM_TX_NO = 0,
    SIM_TX_FS,
    SIM_TX
} simTransmissionState_e;

// ../../../src/main/telemetry/sim.c
typedef enum {
    ACC_EVENT_NONE = 0,
    ACC_EVENT_HIGH,
    ACC_EVENT_LOW,
    ACC_EVENT_NEG_X
} accEvent_t;

// ../../../src/main/telemetry/mavlink.c
typedef enum APM_PLANE_MODE
{
   PLANE_MODE_MANUAL=0,
   PLANE_MODE_CIRCLE=1,
   PLANE_MODE_STABILIZE=2,
   PLANE_MODE_TRAINING=3,
   PLANE_MODE_ACRO=4,
   PLANE_MODE_FLY_BY_WIRE_A=5,
   PLANE_MODE_FLY_BY_WIRE_B=6,
   PLANE_MODE_CRUISE=7,
   PLANE_MODE_AUTOTUNE=8,
   PLANE_MODE_AUTO=10,
   PLANE_MODE_RTL=11,
   PLANE_MODE_LOITER=12,
   PLANE_MODE_TAKEOFF=13,
   PLANE_MODE_AVOID_ADSB=14,
   PLANE_MODE_GUIDED=15,
   PLANE_MODE_INITIALIZING=16,
   PLANE_MODE_QSTABILIZE=17,
   PLANE_MODE_QHOVER=18,
   PLANE_MODE_QLOITER=19,
   PLANE_MODE_QLAND=20,
   PLANE_MODE_QRTL=21,
   PLANE_MODE_QAUTOTUNE=22,
   PLANE_MODE_ENUM_END=23,
} APM_PLANE_MODE;

// ../../../src/main/telemetry/mavlink.c
typedef enum APM_COPTER_MODE
{
   COPTER_MODE_STABILIZE=0,
   COPTER_MODE_ACRO=1,
   COPTER_MODE_ALT_HOLD=2,
   COPTER_MODE_AUTO=3,
   COPTER_MODE_GUIDED=4,
   COPTER_MODE_LOITER=5,
   COPTER_MODE_RTL=6,
   COPTER_MODE_CIRCLE=7,
   COPTER_MODE_LAND=9,
   COPTER_MODE_DRIFT=11,
   COPTER_MODE_SPORT=13,
   COPTER_MODE_FLIP=14,
   COPTER_MODE_AUTOTUNE=15,
   COPTER_MODE_POSHOLD=16,
   COPTER_MODE_BRAKE=17,
   COPTER_MODE_THROW=18,
   COPTER_MODE_AVOID_ADSB=19,
   COPTER_MODE_GUIDED_NOGPS=20,
   COPTER_MODE_SMART_RTL=21,
   COPTER_MODE_ENUM_END=22,
} APM_COPTER_MODE;

// ../../../src/main/telemetry/sim.h
typedef enum  {
    SIM_TX_FLAG                 = (1 << 0),
    SIM_TX_FLAG_FAILSAFE        = (1 << 1),
    SIM_TX_FLAG_GPS             = (1 << 2),
    SIM_TX_FLAG_ACC             = (1 << 3),
    SIM_TX_FLAG_LOW_ALT         = (1 << 4),
    SIM_TX_FLAG_RESPONSE        = (1 << 5)
} simTxFlags_e;

// ../../../src/main/telemetry/ghst.c
typedef enum {
    GHST_FRAME_START_INDEX = 0,
    GHST_FRAME_PACK_INDEX = GHST_FRAME_START_INDEX, 
    GHST_FRAME_GPS_PRIMARY_INDEX,                   
    GHST_FRAME_GPS_SECONDARY_INDEX,                 
   GHST_SCHEDULE_COUNT_MAX
} ghstFrameTypeIndex_e;

// ../../../src/main/io/gps.h
typedef enum {
    GPS_UBLOX = 0,
    GPS_MSP,
    GPS_FAKE,
    GPS_PROVIDER_COUNT
} gpsProvider_e;

// ../../../src/main/io/gps.h
typedef enum {
    SBAS_AUTO = 0,
    SBAS_EGNOS,
    SBAS_WAAS,
    SBAS_MSAS,
    SBAS_GAGAN,
    SBAS_SPAN,
    SBAS_NONE
} sbasMode_e;

// ../../../src/main/io/gps.h
typedef enum {
    GPS_BAUDRATE_115200 = 0,
    GPS_BAUDRATE_57600,
    GPS_BAUDRATE_38400,
    GPS_BAUDRATE_19200,
    GPS_BAUDRATE_9600,
    GPS_BAUDRATE_230400,
    GPS_BAUDRATE_460800,
    GPS_BAUDRATE_921600,
    GPS_BAUDRATE_COUNT
} gpsBaudRate_e;

// ../../../src/main/io/gps.h
typedef enum {
    GPS_AUTOCONFIG_OFF = 0,
    GPS_AUTOCONFIG_ON,
} gpsAutoConfig_e;

// ../../../src/main/io/gps.h
typedef enum {
    GPS_AUTOBAUD_OFF = 0,
    GPS_AUTOBAUD_ON
} gpsAutoBaud_e;

// ../../../src/main/io/gps.h
typedef enum {
    GPS_DYNMODEL_PEDESTRIAN = 0,
    GPS_DYNMODEL_AUTOMOTIVE,
    GPS_DYNMODEL_AIR_1G,
    GPS_DYNMODEL_AIR_2G,
    GPS_DYNMODEL_AIR_4G,
    GPS_DYNMODEL_SEA,
    GPS_DYNMODEL_MOWER,
} gpsDynModel_e;

// ../../../src/main/io/gps.h
typedef enum {
    GPS_NO_FIX = 0,
    GPS_FIX_2D,
    GPS_FIX_3D
} gpsFixType_e;

// ../../../src/main/io/displayport_msp.h
typedef enum {
    MSP_DP_HEARTBEAT = 0,       
    MSP_DP_RELEASE = 1,         
    MSP_DP_CLEAR_SCREEN = 2,    
    MSP_DP_WRITE_STRING = 3,    
    MSP_DP_DRAW_SCREEN = 4,     
    MSP_DP_OPTIONS = 5,         
    MSP_DP_SYS = 6,             
    MSP_DP_COUNT,
} displayportMspCommand_e;

// ../../../src/main/io/gimbal_serial.h
typedef enum {
    WAITING_HDR1,
    WAITING_HDR2,
    WAITING_PAYLOAD,
    WAITING_CRCH,
    WAITING_CRCL,
} gimbalHeadtrackerState_e;

// ../../../src/main/io/osd_dji_hd.c
typedef enum {
    DJI_OSD_CN_MESSAGES,
    DJI_OSD_CN_THROTTLE,
    DJI_OSD_CN_THROTTLE_AUTO_THR,
    DJI_OSD_CN_AIR_SPEED,
    DJI_OSD_CN_EFFICIENCY,
    DJI_OSD_CN_DISTANCE,
    DJI_OSD_CN_ADJUSTEMNTS,
    DJI_OSD_CN_MAX_ELEMENTS
} DjiCraftNameElements_t;

// ../../../src/main/io/vtx.h
typedef enum {
    VTX_LOW_POWER_DISARM_OFF = 0,
    VTX_LOW_POWER_DISARM_ALWAYS = 1,
    VTX_LOW_POWER_DISARM_UNTIL_FIRST_ARM = 2, 
} vtxLowerPowerDisarm_e;

// ../../../src/main/io/vtx_tramp.c
typedef enum {
    VTX_STATE_RESET         = 0,
    VTX_STATE_OFFILE        = 1,    
    VTX_STATE_DETECTING     = 2,    
    VTX_STATE_IDLE          = 3,    
    VTX_STATE_QUERY_DELAY   = 4,
    VTX_STATE_QUERY_STATUS  = 5,
    VTX_STATE_WAIT_STATUS   = 6,    
} vtxProtoState_e;

// ../../../src/main/io/vtx_tramp.c
typedef enum {
    VTX_RESPONSE_TYPE_NONE,
    VTX_RESPONSE_TYPE_CAPABILITIES,
    VTX_RESPONSE_TYPE_STATUS,
} vtxProtoResponseType_e;

// ../../../src/main/io/serial.h
typedef enum {
    PORTSHARING_UNUSED = 0,
    PORTSHARING_NOT_SHARED,
    PORTSHARING_SHARED
} portSharing_e;

// ../../../src/main/io/serial.h
typedef enum {
    FUNCTION_NONE                       = 0,
    FUNCTION_MSP                        = (1 << 0), 
    FUNCTION_GPS                        = (1 << 1), 
    FUNCTION_UNUSED_3                   = (1 << 2), 
    FUNCTION_TELEMETRY_HOTT             = (1 << 3), 
    FUNCTION_TELEMETRY_LTM              = (1 << 4), 
    FUNCTION_TELEMETRY_SMARTPORT        = (1 << 5), 
    FUNCTION_RX_SERIAL                  = (1 << 6), 
    FUNCTION_BLACKBOX                   = (1 << 7), 
    FUNCTION_TELEMETRY_MAVLINK          = (1 << 8), 
    FUNCTION_TELEMETRY_IBUS             = (1 << 9), 
    FUNCTION_RCDEVICE                   = (1 << 10), 
    FUNCTION_VTX_SMARTAUDIO             = (1 << 11), 
    FUNCTION_VTX_TRAMP                  = (1 << 12), 
    FUNCTION_UNUSED_1                   = (1 << 13), 
    FUNCTION_OPTICAL_FLOW               = (1 << 14), 
    FUNCTION_LOG                        = (1 << 15), 
    FUNCTION_RANGEFINDER                = (1 << 16), 
    FUNCTION_VTX_FFPV                   = (1 << 17), 
    FUNCTION_ESCSERIAL                  = (1 << 18), 
    FUNCTION_TELEMETRY_SIM              = (1 << 19), 
    FUNCTION_FRSKY_OSD                  = (1 << 20), 
    FUNCTION_DJI_HD_OSD                 = (1 << 21), 
    FUNCTION_SERVO_SERIAL               = (1 << 22), 
    FUNCTION_TELEMETRY_SMARTPORT_MASTER = (1 << 23), 
    FUNCTION_UNUSED_2                   = (1 << 24), 
    FUNCTION_MSP_OSD                    = (1 << 25), 
    FUNCTION_GIMBAL                     = (1 << 26), 
    FUNCTION_GIMBAL_HEADTRACKER         = (1 << 27), 
} serialPortFunction_e;

// ../../../src/main/io/serial.h
typedef enum {
    BAUD_AUTO = 0,
    BAUD_1200,
    BAUD_2400,
    BAUD_4800,
    BAUD_9600,
    BAUD_19200,
    BAUD_38400,
    BAUD_57600,
    BAUD_115200,
    BAUD_230400,
    BAUD_250000,
    BAUD_460800,
    BAUD_921600,
    BAUD_1000000,
    BAUD_1500000,
    BAUD_2000000,
    BAUD_2470000,

    BAUD_MIN = BAUD_AUTO,
    BAUD_MAX = BAUD_2470000,
} baudRate_e;

// ../../../src/main/io/serial.h
typedef enum {
    SERIAL_PORT_NONE = -1,
    SERIAL_PORT_USART1 = 0,
    SERIAL_PORT_USART2,
    SERIAL_PORT_USART3,
    SERIAL_PORT_USART4,
    SERIAL_PORT_USART5,
    SERIAL_PORT_USART6,
    SERIAL_PORT_USART7,
    SERIAL_PORT_USART8,
    SERIAL_PORT_USB_VCP = 20,
    SERIAL_PORT_SOFTSERIAL1 = 30,
    SERIAL_PORT_SOFTSERIAL2,
    SERIAL_PORT_IDENTIFIER_MAX = SERIAL_PORT_SOFTSERIAL2
} serialPortIdentifier_e;

// ../../../src/main/io/rcdevice.h
typedef enum {
    RCDEVICE_PROTOCOL_FEATURE_SIMULATE_POWER_BUTTON    = (1 << 0),
    RCDEVICE_PROTOCOL_FEATURE_SIMULATE_WIFI_BUTTON     = (1 << 1),
    RCDEVICE_PROTOCOL_FEATURE_CHANGE_MODE              = (1 << 2),
    RCDEVICE_PROTOCOL_FEATURE_SIMULATE_5_KEY_OSD_CABLE = (1 << 3),
    RCDEVICE_PROTOCOL_FEATURE_START_RECORDING          = (1 << 6),
    RCDEVICE_PROTOCOL_FEATURE_STOP_RECORDING           = (1 << 7),
    RCDEVICE_PROTOCOL_FEATURE_CMS_MENU                 = (1 << 8),
} rcdevice_features_e;

// ../../../src/main/io/rcdevice.h
typedef enum {
    RCDEVICE_PROTOCOL_CAM_CTRL_SIMULATE_WIFI_BTN        = 0x00,
    RCDEVICE_PROTOCOL_CAM_CTRL_SIMULATE_POWER_BTN       = 0x01,
    RCDEVICE_PROTOCOL_CAM_CTRL_CHANGE_MODE              = 0x02,
    RCDEVICE_PROTOCOL_CAM_CTRL_START_RECORDING          = 0x03,
    RCDEVICE_PROTOCOL_CAM_CTRL_STOP_RECORDING           = 0x04,
    RCDEVICE_PROTOCOL_CAM_CTRL_UNKNOWN_CAMERA_OPERATION = 0xFF
} rcdevice_camera_control_opeation_e;

// ../../../src/main/io/rcdevice.h
typedef enum {
    RCDEVICE_PROTOCOL_5KEY_SIMULATION_NONE  = 0x00,
    RCDEVICE_PROTOCOL_5KEY_SIMULATION_SET   = 0x01,
    RCDEVICE_PROTOCOL_5KEY_SIMULATION_LEFT  = 0x02,
    RCDEVICE_PROTOCOL_5KEY_SIMULATION_RIGHT = 0x03,
    RCDEVICE_PROTOCOL_5KEY_SIMULATION_UP    = 0x04,
    RCDEVICE_PROTOCOL_5KEY_SIMULATION_DOWN  = 0x05
} rcdevice_5key_simulation_operation_e;

// ../../../src/main/io/rcdevice.h
typedef enum {
    RCDEVICE_PROTOCOL_5KEY_CONNECTION_OPEN = 0x01,
    RCDEVICE_PROTOCOL_5KEY_CONNECTION_CLOSE = 0x02
} RCDEVICE_5key_connection_event_e;

// ../../../src/main/io/rcdevice.h
typedef enum {
    RCDEVICE_CAM_KEY_NONE,
    RCDEVICE_CAM_KEY_ENTER,
    RCDEVICE_CAM_KEY_LEFT,
    RCDEVICE_CAM_KEY_UP,
    RCDEVICE_CAM_KEY_RIGHT,
    RCDEVICE_CAM_KEY_DOWN,
    RCDEVICE_CAM_KEY_CONNECTION_CLOSE,
    RCDEVICE_CAM_KEY_CONNECTION_OPEN,
    RCDEVICE_CAM_KEY_RELEASE,
} rcdeviceCamSimulationKeyEvent_e;

// ../../../src/main/io/rcdevice.h
typedef enum {
    RCDEVICE_PROTOCOL_RCSPLIT_VERSION = 0x00, 
                                              
                                              
    RCDEVICE_PROTOCOL_VERSION_1_0 = 0x01,
    RCDEVICE_PROTOCOL_UNKNOWN
} rcdevice_protocol_version_e;

// ../../../src/main/io/rcdevice.h
typedef enum {
    RCDEVICE_RESP_SUCCESS = 0,
    RCDEVICE_RESP_INCORRECT_CRC = 1,
    RCDEVICE_RESP_TIMEOUT = 2
} rcdeviceResponseStatus_e;

// ../../../src/main/io/vtx.c
typedef enum {
    VTX_PARAM_POWER = 0,
    VTX_PARAM_BANDCHAN,
    VTX_PARAM_PITMODE,
    VTX_PARAM_COUNT
} vtxScheduleParams_e;

// ../../../src/main/io/beeper.h
typedef enum {
    
    BEEPER_SILENCE = 0,                 

    BEEPER_RUNTIME_CALIBRATION_DONE,
    BEEPER_HARDWARE_FAILURE,            
    BEEPER_RX_LOST,                     
    BEEPER_RX_LOST_LANDING,             
    BEEPER_DISARMING,                   
    BEEPER_ARMING,                      
    BEEPER_ARMING_GPS_FIX,              
    BEEPER_BAT_CRIT_LOW,                
    BEEPER_BAT_LOW,                     
    BEEPER_GPS_STATUS,                  
    BEEPER_RX_SET,                      
    BEEPER_ACTION_SUCCESS,              
    BEEPER_ACTION_FAIL,                 
    BEEPER_READY_BEEP,                  
    BEEPER_MULTI_BEEPS,                 
    BEEPER_DISARM_REPEAT,               
    BEEPER_ARMED,                       
    BEEPER_SYSTEM_INIT,                 
    BEEPER_USB,                         
    BEEPER_LAUNCH_MODE_ENABLED,         
    BEEPER_LAUNCH_MODE_LOW_THROTTLE,    
    BEEPER_LAUNCH_MODE_IDLE_START,      
    BEEPER_CAM_CONNECTION_OPEN,         
    BEEPER_CAM_CONNECTION_CLOSE,        

    BEEPER_ALL,                         
    BEEPER_PREFERENCE,                  
    
} beeperMode_e;

// ../../../src/main/io/frsky_osd.c
typedef enum
{
    OSD_CMD_RESPONSE_ERROR = 0,

    OSD_CMD_INFO = 1,
    OSD_CMD_READ_FONT = 2,
    OSD_CMD_WRITE_FONT = 3,
    OSD_CMD_GET_CAMERA = 4,
    OSD_CMD_SET_CAMERA = 5,
    OSD_CMD_GET_ACTIVE_CAMERA = 6,
    OSD_CMD_GET_OSD_ENABLED = 7,
    OSD_CMD_SET_OSD_ENABLED = 8,

    OSD_CMD_TRANSACTION_BEGIN = 16,
    OSD_CMD_TRANSACTION_COMMIT = 17,
    OSD_CMD_TRANSACTION_BEGIN_PROFILED = 18,
    OSD_CMD_TRANSACTION_BEGIN_RESET_DRAWING = 19,

    OSD_CMD_DRAWING_SET_STROKE_COLOR = 22,
    OSD_CMD_DRAWING_SET_FILL_COLOR = 23,
    OSD_CMD_DRAWING_SET_STROKE_AND_FILL_COLOR = 24,
    OSD_CMD_DRAWING_SET_COLOR_INVERSION = 25,
    OSD_CMD_DRAWING_SET_PIXEL = 26,
    OSD_CMD_DRAWING_SET_PIXEL_TO_STROKE_COLOR = 27,
    OSD_CMD_DRAWING_SET_PIXEL_TO_FILL_COLOR = 28,
    OSD_CMD_DRAWING_SET_STROKE_WIDTH = 29,
    OSD_CMD_DRAWING_SET_LINE_OUTLINE_TYPE = 30,
    OSD_CMD_DRAWING_SET_LINE_OUTLINE_COLOR = 31,

    OSD_CMD_DRAWING_CLIP_TO_RECT = 40,
    OSD_CMD_DRAWING_CLEAR_SCREEN = 41,
    OSD_CMD_DRAWING_CLEAR_RECT = 42,
    OSD_CMD_DRAWING_RESET = 43,
    OSD_CMD_DRAWING_DRAW_BITMAP = 44,
    OSD_CMD_DRAWING_DRAW_BITMAP_MASK = 45,
    OSD_CMD_DRAWING_DRAW_CHAR = 46,
    OSD_CMD_DRAWING_DRAW_CHAR_MASK = 47,
    OSD_CMD_DRAWING_DRAW_STRING = 48,
    OSD_CMD_DRAWING_DRAW_STRING_MASK = 49,
    OSD_CMD_DRAWING_MOVE_TO_POINT = 50,
    OSD_CMD_DRAWING_STROKE_LINE_TO_POINT = 51,
    OSD_CMD_DRAWING_STROKE_TRIANGLE = 52,
    OSD_CMD_DRAWING_FILL_TRIANGLE = 53,
    OSD_CMD_DRAWING_FILL_STROKE_TRIANGLE = 54,
    OSD_CMD_DRAWING_STROKE_RECT = 55,
    OSD_CMD_DRAWING_FILL_RECT = 56,
    OSD_CMD_DRAWING_FILL_STROKE_RECT = 57,
    OSD_CMD_DRAWING_STROKE_ELLIPSE_IN_RECT = 58,
    OSD_CMD_DRAWING_FILL_ELLIPSE_IN_RECT = 59,
    OSD_CMD_DRAWING_FILL_STROKE_ELLIPSE_IN_RECT = 60,

    OSD_CMD_CTM_RESET = 80,
    OSD_CMD_CTM_SET = 81,
    OSD_CMD_CTM_TRANSLATE = 82,
    OSD_CMD_CTM_SCALE = 83,
    OSD_CMD_CTM_ROTATE = 84,
    OSD_CMD_CTM_ROTATE_ABOUT = 85,
    OSD_CMD_CTM_SHEAR = 86,
    OSD_CMD_CTM_SHEAR_ABOUT = 87,
    OSD_CMD_CTM_MULTIPLY = 88,

    OSD_CMD_CONTEXT_PUSH = 100,
    OSD_CMD_CONTEXT_POP = 101,

    
    OSD_CMD_DRAW_GRID_CHR = 110,
    OSD_CMD_DRAW_GRID_STR = 111,
    OSD_CMD_DRAW_GRID_CHR_2 = 112,                                  
    OSD_CMD_DRAW_GRID_STR_2 = 113,                                  

    OSD_CMD_WIDGET_SET_CONFIG = 115,                                
    OSD_CMD_WIDGET_DRAW = 116,                                      
    OSD_CMD_WIDGET_ERASE = 117,                                     

    OSD_CMD_SET_DATA_RATE = 122,
} osdCommand_e;

// ../../../src/main/io/frsky_osd.c
typedef enum {
    RECV_STATE_NONE,
    RECV_STATE_SYNC,
    RECV_STATE_LENGTH,
    RECV_STATE_DATA,
    RECV_STATE_CHECKSUM,
    RECV_STATE_DONE,
} frskyOSDRecvState_e;

// ../../../src/main/io/gps_ublox.h
typedef enum {
    UBLOX_SIG_HEALTH_UNKNOWN = 0,
    UBLOX_SIG_HEALTH_HEALTHY = 1,
    UBLOX_SIG_HEALTH_UNHEALTHY = 2
} ublox_nav_sig_health_e;

// ../../../src/main/io/gps_ublox.h
typedef enum {
    UBLOX_SIG_QUALITY_NOSIGNAL = 0,
    UBLOX_SIG_QUALITY_SEARCHING = 1,
    UBLOX_SIG_QUALITY_ACQUIRED = 2,
    UBLOX_SIG_QUALITY_UNUSABLE = 3,
    UBLOX_SIG_QUALITY_CODE_LOCK_TIME_SYNC = 4,
    UBLOX_SIG_QUALITY_CODE_CARRIER_LOCK_TIME_SYNC = 5,
    UBLOX_SIG_QUALITY_CODE_CARRIER_LOCK_TIME_SYNC2 = 6,
    UBLOX_SIG_QUALITY_CODE_CARRIER_LOCK_TIME_SYNC3 = 7,
} ublox_nav_sig_quality;

// ../../../src/main/io/gps_ublox.h
typedef enum {
    UBX_ACK_WAITING = 0,
    UBX_ACK_GOT_ACK = 1,
    UBX_ACK_GOT_NAK = 2
} ubx_ack_state_t;

// ../../../src/main/io/gps_ublox.h
typedef enum {
    PREAMBLE1 = 0xB5,
    PREAMBLE2 = 0x62,
    CLASS_NAV = 0x01,
    CLASS_ACK = 0x05,
    CLASS_CFG = 0x06,
    CLASS_MON = 0x0A,
    MSG_CLASS_UBX = 0x01,
    MSG_CLASS_NMEA = 0xF0,
    MSG_VER = 0x04,
    MSG_ACK_NACK = 0x00,
    MSG_ACK_ACK = 0x01,
    MSG_NMEA_GGA = 0x0,
    MSG_NMEA_GLL = 0x1,
    MSG_NMEA_GSA = 0x2,
    MSG_NMEA_GSV = 0x3,
    MSG_NMEA_RMC = 0x4,
    MSG_NMEA_VGS = 0x5,
    MSG_POSLLH = 0x2,
    MSG_STATUS = 0x3,
    MSG_SOL = 0x6,
    MSG_PVT = 0x7,
    MSG_VELNED = 0x12,
    MSG_TIMEUTC = 0x21,
    MSG_SVINFO = 0x30,
    MSG_NAV_SAT = 0x35,
    MSG_CFG_PRT = 0x00,
    MSG_CFG_RATE = 0x08,
    MSG_CFG_SET_RATE = 0x01,
    MSG_CFG_NAV_SETTINGS = 0x24,
    MSG_CFG_SBAS = 0x16,
    MSG_CFG_GNSS = 0x3e,
    MSG_MON_GNSS = 0x28,
    MSG_NAV_SIG = 0x43
} ubx_protocol_bytes_t;

// ../../../src/main/io/gps_ublox.h
typedef enum {
    FIX_NONE = 0,
    FIX_DEAD_RECKONING = 1,
    FIX_2D = 2,
    FIX_3D = 3,
    FIX_GPS_DEAD_RECKONING = 4,
    FIX_TIME = 5
} ubs_nav_fix_type_t;

// ../../../src/main/io/gps_ublox.h
typedef enum {
    NAV_STATUS_FIX_VALID = 1
} ubx_nav_status_bits_t;

// ../../../src/main/io/dashboard.h
typedef enum {
    PAGE_WELCOME,
    PAGE_ARMED,
    PAGE_STATUS
} pageId_e;

// ../../../src/main/io/osd_common.h
typedef enum {
    OSD_SPEED_SOURCE_GROUND    = 0,
    OSD_SPEED_SOURCE_3D        = 1,
    OSD_SPEED_SOURCE_AIR       = 2
} osdSpeedSource_e;

// ../../../src/main/io/osd_common.h
typedef enum {
    OSD_DRAW_POINT_TYPE_GRID,
    OSD_DRAW_POINT_TYPE_PIXEL,
} osdDrawPointType_e;

// ../../../src/main/io/ledstrip.h
typedef enum {
    COLOR_BLACK = 0,
    COLOR_WHITE,
    COLOR_RED,
    COLOR_ORANGE,
    COLOR_YELLOW,
    COLOR_LIME_GREEN,
    COLOR_GREEN,
    COLOR_MINT_GREEN,
    COLOR_CYAN,
    COLOR_LIGHT_BLUE,
    COLOR_BLUE,
    COLOR_DARK_VIOLET,
    COLOR_MAGENTA,
    COLOR_DEEP_PINK,
} colorId_e;

// ../../../src/main/io/ledstrip.h
typedef enum {
    LED_MODE_ORIENTATION = 0,
    LED_MODE_HEADFREE,
    LED_MODE_HORIZON,
    LED_MODE_ANGLE,
    LED_MODE_MAG,
    LED_MODE_BARO,
    LED_SPECIAL
} ledModeIndex_e;

// ../../../src/main/io/ledstrip.h
typedef enum {
    LED_SCOLOR_DISARMED = 0,
    LED_SCOLOR_ARMED,
    LED_SCOLOR_ANIMATION,
    LED_SCOLOR_BACKGROUND,
    LED_SCOLOR_BLINKBACKGROUND,
    LED_SCOLOR_GPSNOSATS,
    LED_SCOLOR_GPSNOLOCK,
    LED_SCOLOR_GPSLOCKED,
    LED_SCOLOR_STROBE
} ledSpecialColorIds_e;

// ../../../src/main/io/ledstrip.h
typedef enum {
    LED_DIRECTION_NORTH = 0,
    LED_DIRECTION_EAST,
    LED_DIRECTION_SOUTH,
    LED_DIRECTION_WEST,
    LED_DIRECTION_UP,
    LED_DIRECTION_DOWN
} ledDirectionId_e;

// ../../../src/main/io/ledstrip.h
typedef enum {
    LED_FUNCTION_COLOR,
    LED_FUNCTION_FLIGHT_MODE,
    LED_FUNCTION_ARM_STATE,
    LED_FUNCTION_BATTERY,
    LED_FUNCTION_RSSI,
    LED_FUNCTION_GPS,
    LED_FUNCTION_THRUST_RING,
    LED_FUNCTION_CHANNEL,
} ledBaseFunctionId_e;

// ../../../src/main/io/ledstrip.h
typedef enum {
    LED_OVERLAY_THROTTLE,
    LED_OVERLAY_LARSON_SCANNER,
    LED_OVERLAY_BLINK,
    LED_OVERLAY_LANDING_FLASH,
    LED_OVERLAY_INDICATOR,
    LED_OVERLAY_WARNING,
    LED_OVERLAY_STROBE
} ledOverlayId_e;

// ../../../src/main/io/smartport_master.c
typedef enum {
    PT_ACTIVE_ID,
    PT_INACTIVE_ID
} pollType_e;

// ../../../src/main/io/statusindicator.c
typedef enum {
    WARNING_LED_OFF = 0,
    WARNING_LED_ON,
    WARNING_LED_FLASH
} warningLedState_e;

// ../../../src/main/io/frsky_osd.h
typedef enum {
    FRSKY_OSD_TRANSACTION_OPT_PROFILED = 1 << 0,
    FRSKY_OSD_TRANSACTION_OPT_RESET_DRAWING = 1 << 1,
} frskyOSDTransactionOptions_e;

// ../../../src/main/io/frsky_osd.h
typedef enum {
    FRSKY_OSD_COLOR_BLACK = 0,
    FRSKY_OSD_COLOR_TRANSPARENT = 1,
    FRSKY_OSD_COLOR_WHITE = 2,
    FRSKY_OSD_COLOR_GRAY = 3,
} frskyOSDColor_e;

// ../../../src/main/io/frsky_osd.h
typedef enum {
    FRSKY_OSD_OUTLINE_TYPE_NONE = 0,
    FRSKY_OSD_OUTLINE_TYPE_TOP = 1 << 0,
    FRSKY_OSD_OUTLINE_TYPE_RIGHT = 1 << 1,
    FRSKY_OSD_OUTLINE_TYPE_BOTTOM = 1 << 2,
    FRSKY_OSD_OUTLINE_TYPE_LEFT = 1 << 3,
} frskyOSDLineOutlineType_e;

// ../../../src/main/io/frsky_osd.h
typedef enum
{
    FRSKY_OSD_WIDGET_ID_AHI = 0,

    FRSKY_OSD_WIDGET_ID_SIDEBAR_0 = 1,
    FRSKY_OSD_WIDGET_ID_SIDEBAR_1 = 2,

    FRSKY_OSD_WIDGET_ID_GRAPH_0 = 3,
    FRSKY_OSD_WIDGET_ID_GRAPH_1 = 4,
    FRSKY_OSD_WIDGET_ID_GRAPH_2 = 5,
    FRSKY_OSD_WIDGET_ID_GRAPH_3 = 6,

    FRSKY_OSD_WIDGET_ID_CHARGAUGE_0 = 7,
    FRSKY_OSD_WIDGET_ID_CHARGAUGE_1 = 8,
    FRSKY_OSD_WIDGET_ID_CHARGAUGE_2 = 9,
    FRSKY_OSD_WIDGET_ID_CHARGAUGE_3 = 10,

    FRSKY_OSD_WIDGET_ID_SIDEBAR_FIRST = FRSKY_OSD_WIDGET_ID_SIDEBAR_0,
    FRSKY_OSD_WIDGET_ID_SIDEBAR_LAST = FRSKY_OSD_WIDGET_ID_SIDEBAR_1,

    FRSKY_OSD_WIDGET_ID_GRAPH_FIRST = FRSKY_OSD_WIDGET_ID_GRAPH_0,
    FRSKY_OSD_WIDGET_ID_GRAPH_LAST = FRSKY_OSD_WIDGET_ID_GRAPH_3,

    FRSKY_OSD_WIDGET_ID_CHARGAUGE_FIRST = FRSKY_OSD_WIDGET_ID_CHARGAUGE_0,
    FRSKY_OSD_WIDGET_ID_CHARGAUGE_LAST = FRSKY_OSD_WIDGET_ID_CHARGAUGE_3,
} frskyOSDWidgetID_e;

// ../../../src/main/io/osd.h
typedef enum {
    OSD_RSSI_VALUE,
    OSD_MAIN_BATT_VOLTAGE,
    OSD_CROSSHAIRS,
    OSD_ARTIFICIAL_HORIZON,
    OSD_HORIZON_SIDEBARS,
    OSD_ONTIME,
    OSD_FLYTIME,
    OSD_FLYMODE,
    OSD_CRAFT_NAME,
    OSD_THROTTLE_POS,
    OSD_VTX_CHANNEL,
    OSD_CURRENT_DRAW,
    OSD_MAH_DRAWN,
    OSD_GPS_SPEED,
    OSD_GPS_SATS,
    OSD_ALTITUDE,
    OSD_ROLL_PIDS,
    OSD_PITCH_PIDS,
    OSD_YAW_PIDS,
    OSD_POWER,
    OSD_GPS_LON,
    OSD_GPS_LAT,
    OSD_HOME_DIR,
    OSD_HOME_DIST,
    OSD_HEADING,
    OSD_VARIO,
    OSD_VARIO_NUM,
    OSD_AIR_SPEED,
    OSD_ONTIME_FLYTIME,
    OSD_RTC_TIME,
    OSD_MESSAGES,
    OSD_GPS_HDOP,
    OSD_MAIN_BATT_CELL_VOLTAGE,
    OSD_SCALED_THROTTLE_POS,
    OSD_HEADING_GRAPH,
    OSD_EFFICIENCY_MAH_PER_KM,
    OSD_WH_DRAWN,
    OSD_BATTERY_REMAINING_CAPACITY,
    OSD_BATTERY_REMAINING_PERCENT,
    OSD_EFFICIENCY_WH_PER_KM,
    OSD_TRIP_DIST,
    OSD_ATTITUDE_PITCH,
    OSD_ATTITUDE_ROLL,
    OSD_MAP_NORTH,
    OSD_MAP_TAKEOFF,
    OSD_RADAR,
    OSD_WIND_SPEED_HORIZONTAL,
    OSD_WIND_SPEED_VERTICAL,
    OSD_REMAINING_FLIGHT_TIME_BEFORE_RTH,
    OSD_REMAINING_DISTANCE_BEFORE_RTH,
    OSD_HOME_HEADING_ERROR,
    OSD_COURSE_HOLD_ERROR,
    OSD_COURSE_HOLD_ADJUSTMENT,
    OSD_SAG_COMPENSATED_MAIN_BATT_VOLTAGE,
    OSD_MAIN_BATT_SAG_COMPENSATED_CELL_VOLTAGE,
    OSD_POWER_SUPPLY_IMPEDANCE,
    OSD_LEVEL_PIDS,
    OSD_POS_XY_PIDS,
    OSD_POS_Z_PIDS,
    OSD_VEL_XY_PIDS,
    OSD_VEL_Z_PIDS,
    OSD_HEADING_P,
    OSD_BOARD_ALIGN_ROLL,
    OSD_BOARD_ALIGN_PITCH,
    OSD_RC_EXPO,
    OSD_RC_YAW_EXPO,
    OSD_THROTTLE_EXPO,
    OSD_PITCH_RATE,
    OSD_ROLL_RATE,
    OSD_YAW_RATE,
    OSD_MANUAL_RC_EXPO,
    OSD_MANUAL_RC_YAW_EXPO,
    OSD_MANUAL_PITCH_RATE,
    OSD_MANUAL_ROLL_RATE,
    OSD_MANUAL_YAW_RATE,
    OSD_NAV_FW_CRUISE_THR,
    OSD_NAV_FW_PITCH2THR,
    OSD_FW_MIN_THROTTLE_DOWN_PITCH_ANGLE,
    OSD_DEBUG, 
    OSD_FW_ALT_PID_OUTPUTS,
    OSD_FW_POS_PID_OUTPUTS,
    OSD_MC_VEL_X_PID_OUTPUTS,
    OSD_MC_VEL_Y_PID_OUTPUTS,
    OSD_MC_VEL_Z_PID_OUTPUTS,
    OSD_MC_POS_XYZ_P_OUTPUTS,
    OSD_3D_SPEED,
    OSD_IMU_TEMPERATURE,
    OSD_BARO_TEMPERATURE,
    OSD_TEMP_SENSOR_0_TEMPERATURE,
    OSD_TEMP_SENSOR_1_TEMPERATURE,
    OSD_TEMP_SENSOR_2_TEMPERATURE,
    OSD_TEMP_SENSOR_3_TEMPERATURE,
    OSD_TEMP_SENSOR_4_TEMPERATURE,
    OSD_TEMP_SENSOR_5_TEMPERATURE,
    OSD_TEMP_SENSOR_6_TEMPERATURE,
    OSD_TEMP_SENSOR_7_TEMPERATURE,
    OSD_ALTITUDE_MSL,
    OSD_PLUS_CODE,
    OSD_MAP_SCALE,
    OSD_MAP_REFERENCE,
    OSD_GFORCE,
    OSD_GFORCE_X,
    OSD_GFORCE_Y,
    OSD_GFORCE_Z,
    OSD_RC_SOURCE,
    OSD_VTX_POWER,
    OSD_ESC_RPM,
    OSD_ESC_TEMPERATURE,
    OSD_AZIMUTH,
    OSD_RSSI_DBM,
    OSD_LQ_UPLINK,
    OSD_SNR_DB,
    OSD_TX_POWER_UPLINK,
    OSD_GVAR_0,
    OSD_GVAR_1,
    OSD_GVAR_2,
    OSD_GVAR_3,
    OSD_TPA,
    OSD_NAV_FW_CONTROL_SMOOTHNESS,
    OSD_VERSION,
    OSD_RANGEFINDER,
    OSD_PLIMIT_REMAINING_BURST_TIME,
    OSD_PLIMIT_ACTIVE_CURRENT_LIMIT,
    OSD_PLIMIT_ACTIVE_POWER_LIMIT,
    OSD_GLIDESLOPE,
    OSD_GPS_MAX_SPEED,
    OSD_3D_MAX_SPEED,
    OSD_AIR_MAX_SPEED,
    OSD_ACTIVE_PROFILE,
    OSD_MISSION,
    OSD_SWITCH_INDICATOR_0,
    OSD_SWITCH_INDICATOR_1,
    OSD_SWITCH_INDICATOR_2,
    OSD_SWITCH_INDICATOR_3,
    OSD_TPA_TIME_CONSTANT,
    OSD_FW_LEVEL_TRIM,
    OSD_GLIDE_TIME_REMAINING,
    OSD_GLIDE_RANGE,
    OSD_CLIMB_EFFICIENCY,
    OSD_NAV_WP_MULTI_MISSION_INDEX,
    OSD_GROUND_COURSE,      
    OSD_CROSS_TRACK_ERROR,
    OSD_PILOT_NAME,
    OSD_PAN_SERVO_CENTRED,
    OSD_MULTI_FUNCTION,
    OSD_ODOMETER,
    OSD_PILOT_LOGO,
    OSD_CUSTOM_ELEMENT_1,
    OSD_CUSTOM_ELEMENT_2,
    OSD_CUSTOM_ELEMENT_3,
    OSD_ADSB_WARNING, 
    OSD_ADSB_INFO,
    OSD_BLACKBOX,
    OSD_FORMATION_FLIGHT,
    OSD_CUSTOM_ELEMENT_4,
    OSD_CUSTOM_ELEMENT_5,
    OSD_CUSTOM_ELEMENT_6,
    OSD_CUSTOM_ELEMENT_7,
    OSD_CUSTOM_ELEMENT_8,
    OSD_LQ_DOWNLINK,
    OSD_RX_POWER_DOWNLINK, 
    OSD_RX_BAND,
    OSD_RX_MODE,
    OSD_COURSE_TO_FENCE,
    OSD_H_DIST_TO_FENCE,
    OSD_V_DIST_TO_FENCE,
    OSD_NAV_FW_ALT_CONTROL_RESPONSE,
    OSD_ITEM_COUNT 
} osd_items_e;

// ../../../src/main/io/osd.h
typedef enum {
    OSD_UNIT_IMPERIAL,
    OSD_UNIT_METRIC,
    OSD_UNIT_METRIC_MPH,    
    OSD_UNIT_UK,            
    OSD_UNIT_GA,            

    OSD_UNIT_MAX = OSD_UNIT_GA,
} osd_unit_e;

// ../../../src/main/io/osd.h
typedef enum {
    OSD_STATS_ENERGY_UNIT_MAH,
    OSD_STATS_ENERGY_UNIT_WH,
} osd_stats_energy_unit_e;

// ../../../src/main/io/osd.h
typedef enum {
    OSD_CROSSHAIRS_STYLE_DEFAULT,
    OSD_CROSSHAIRS_STYLE_AIRCRAFT,
    OSD_CROSSHAIRS_STYLE_TYPE3,
    OSD_CROSSHAIRS_STYLE_TYPE4,
    OSD_CROSSHAIRS_STYLE_TYPE5,
    OSD_CROSSHAIRS_STYLE_TYPE6,
    OSD_CROSSHAIRS_STYLE_TYPE7,
} osd_crosshairs_style_e;

// ../../../src/main/io/osd.h
typedef enum {
    OSD_SIDEBAR_SCROLL_NONE,
    OSD_SIDEBAR_SCROLL_ALTITUDE,
    OSD_SIDEBAR_SCROLL_SPEED,
    OSD_SIDEBAR_SCROLL_HOME_DISTANCE,

    OSD_SIDEBAR_SCROLL_MAX = OSD_SIDEBAR_SCROLL_HOME_DISTANCE,
} osd_sidebar_scroll_e;

// ../../../src/main/io/osd.h
typedef enum {
    OSD_ALIGN_LEFT,
    OSD_ALIGN_RIGHT
} osd_alignment_e;

// ../../../src/main/io/osd.h
typedef enum {
    OSD_AHI_STYLE_DEFAULT,
    OSD_AHI_STYLE_LINE,
} osd_ahi_style_e;

// ../../../src/main/io/osd.h
typedef enum {
    OSD_CRSF_LQ_TYPE1,
    OSD_CRSF_LQ_TYPE2,
    OSD_CRSF_LQ_TYPE3
} osd_crsf_lq_format_e;

// ../../../src/main/io/ledstrip.c
typedef enum {
    
    QUADRANT_NORTH      = 1 << 0,
    QUADRANT_SOUTH      = 1 << 1,
    QUADRANT_EAST       = 1 << 2,
    QUADRANT_WEST       = 1 << 3,
    QUADRANT_NORTH_EAST = 1 << 4,
    QUADRANT_SOUTH_EAST = 1 << 5,
    QUADRANT_NORTH_WEST = 1 << 6,
    QUADRANT_SOUTH_WEST = 1 << 7,
    QUADRANT_NONE       = 1 << 8,
    QUADRANT_NOTDIAG    = 1 << 9,  
    
    QUADRANT_ANY        = QUADRANT_NORTH | QUADRANT_SOUTH | QUADRANT_EAST | QUADRANT_WEST | QUADRANT_NONE,
} quadrant_e;

// ../../../src/main/io/ledstrip.c
typedef enum {
    WARNING_ARMING_DISABLED,
    WARNING_LOW_BATTERY,
    WARNING_FAILSAFE,
    WARNING_HW_ERROR,
} warningFlags_e;

// ../../../src/main/io/ledstrip.c
typedef enum {
    timBlink = 0,
    timLarson,
    timBattery,
    timRssi,
#ifdef USE_GPS
    timGps,
#endif
    timWarning,
    timIndicator,
#ifdef USE_LED_ANIMATION
    timAnimation,
#endif
    timRing,
    timTimerCount
} timId_e;

// ../../../src/main/io/vtx_smartaudio.h
typedef enum {
    SA_UNKNOWN, 
    SA_1_0,
    SA_2_0,
    SA_2_1
} smartAudioVersion_e;

// ../../../src/main/io/gps_private.h
typedef enum {
    GPS_UNKNOWN,                
    GPS_INITIALIZING,           
    GPS_RUNNING,                
    GPS_LOST_COMMUNICATION,     
} gpsState_e;

// ../../../src/main/io/smartport_master.h
typedef enum {
    VS600_BAND_A,
    VS600_BAND_B,
    VS600_BAND_C,
    VS600_BAND_D,
    VS600_BAND_E,
    VS600_BAND_F,
} vs600Band_e;

// ../../../src/main/io/smartport_master.h
typedef enum {
    VS600_POWER_PIT,
    VS600_POWER_25MW,
    VS600_POWER_200MW,
    VS600_POWER_600MW,
} vs600Power_e;

// ../../../src/main/io/displayport_msp_osd.c
typedef enum {          
    SD_3016,
    HD_5018,
    HD_3016,           
    HD_6022,           
    HD_5320            
} resolutionType_e;

// ../../../src/main/io/osd_grid.c
typedef enum {
    OSD_SIDEBAR_ARROW_NONE,
    OSD_SIDEBAR_ARROW_UP,
    OSD_SIDEBAR_ARROW_DOWN,
} osd_sidebar_arrow_e;

// ../../../src/main/io/osd/custom_elements.h
typedef enum {
    CUSTOM_ELEMENT_TYPE_NONE            = 0,
    CUSTOM_ELEMENT_TYPE_TEXT            = 1,
    CUSTOM_ELEMENT_TYPE_ICON_STATIC     = 2,
    CUSTOM_ELEMENT_TYPE_ICON_GV         = 3,
    CUSTOM_ELEMENT_TYPE_ICON_LC         = 4,
    CUSTOM_ELEMENT_TYPE_GV_1            = 5,
    CUSTOM_ELEMENT_TYPE_GV_2            = 6,
    CUSTOM_ELEMENT_TYPE_GV_3            = 7,
    CUSTOM_ELEMENT_TYPE_GV_4            = 8,
    CUSTOM_ELEMENT_TYPE_GV_5            = 9,
    CUSTOM_ELEMENT_TYPE_GV_FLOAT_1_1    = 10,
    CUSTOM_ELEMENT_TYPE_GV_FLOAT_1_2    = 11,
    CUSTOM_ELEMENT_TYPE_GV_FLOAT_2_1    = 12,
    CUSTOM_ELEMENT_TYPE_GV_FLOAT_2_2    = 13,
    CUSTOM_ELEMENT_TYPE_GV_FLOAT_3_1    = 14,
    CUSTOM_ELEMENT_TYPE_GV_FLOAT_3_2    = 15,
    CUSTOM_ELEMENT_TYPE_GV_FLOAT_4_1    = 16,
    CUSTOM_ELEMENT_TYPE_LC_1            = 17,
    CUSTOM_ELEMENT_TYPE_LC_2            = 18,
    CUSTOM_ELEMENT_TYPE_LC_3            = 19,
    CUSTOM_ELEMENT_TYPE_LC_4            = 20,
    CUSTOM_ELEMENT_TYPE_LC_5            = 21,
    CUSTOM_ELEMENT_TYPE_LC_FLOAT_1_1    = 22,
    CUSTOM_ELEMENT_TYPE_LC_FLOAT_1_2    = 23,
    CUSTOM_ELEMENT_TYPE_LC_FLOAT_2_1    = 24,
    CUSTOM_ELEMENT_TYPE_LC_FLOAT_2_2    = 25,
    CUSTOM_ELEMENT_TYPE_LC_FLOAT_3_1    = 26,
    CUSTOM_ELEMENT_TYPE_LC_FLOAT_3_2    = 27,
    CUSTOM_ELEMENT_TYPE_LC_FLOAT_4_1    = 28,
    CUSTOM_ELEMENT_TYPE_END             
} osdCustomElementType_e;

// ../../../src/main/io/osd/custom_elements.h
typedef enum {
    CUSTOM_ELEMENT_VISIBILITY_ALWAYS    = 0,
    CUSTOM_ELEMENT_VISIBILITY_GV        = 1,
    CUSTOM_ELEMENT_VISIBILITY_LOGIC_CON = 2,
} osdCustomElementTypeVisibility_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.h
typedef enum {
    AFATFS_FILESYSTEM_STATE_UNKNOWN,
    AFATFS_FILESYSTEM_STATE_FATAL,
    AFATFS_FILESYSTEM_STATE_INITIALIZATION,
    AFATFS_FILESYSTEM_STATE_READY,
} afatfsFilesystemState_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.h
typedef enum {
    AFATFS_OPERATION_IN_PROGRESS,
    AFATFS_OPERATION_SUCCESS,
    AFATFS_OPERATION_FAILURE,
} afatfsOperationStatus_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.h
typedef enum {
    AFATFS_ERROR_NONE = 0,
    AFATFS_ERROR_GENERIC = 1,
    AFATFS_ERROR_BAD_MBR = 2,
    AFATFS_ERROR_BAD_FILESYSTEM_HEADER = 3
} afatfsError_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.h
typedef enum {
    AFATFS_SEEK_SET,
    AFATFS_SEEK_CUR,
    AFATFS_SEEK_END,
} afatfsSeek_e;

// ../../../src/main/io/asyncfatfs/fat_standard.h
typedef enum {
    FAT_FILESYSTEM_TYPE_INVALID,
    FAT_FILESYSTEM_TYPE_FAT12,
    FAT_FILESYSTEM_TYPE_FAT16,
    FAT_FILESYSTEM_TYPE_FAT32,
} fatFilesystemType_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.c
typedef enum {
    AFATFS_SAVE_DIRECTORY_NORMAL,
    AFATFS_SAVE_DIRECTORY_FOR_CLOSE,
    AFATFS_SAVE_DIRECTORY_DELETED,
} afatfsSaveDirectoryEntryMode_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.c
typedef enum {
    AFATFS_CACHE_STATE_EMPTY,
    AFATFS_CACHE_STATE_IN_SYNC,
    AFATFS_CACHE_STATE_READING,
    AFATFS_CACHE_STATE_WRITING,
    AFATFS_CACHE_STATE_DIRTY
} afatfsCacheBlockState_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.c
typedef enum {
    AFATFS_FILE_TYPE_NONE,
    AFATFS_FILE_TYPE_NORMAL,
    AFATFS_FILE_TYPE_FAT16_ROOT_DIRECTORY,
    AFATFS_FILE_TYPE_DIRECTORY
} afatfsFileType_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.c
typedef enum {
    CLUSTER_SEARCH_FREE_AT_BEGINNING_OF_FAT_SECTOR,
    CLUSTER_SEARCH_FREE,
    CLUSTER_SEARCH_OCCUPIED,
} afatfsClusterSearchCondition_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.c
typedef enum {
    AFATFS_FIND_CLUSTER_IN_PROGRESS,
    AFATFS_FIND_CLUSTER_FOUND,
    AFATFS_FIND_CLUSTER_FATAL,
    AFATFS_FIND_CLUSTER_NOT_FOUND,
} afatfsFindClusterStatus_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.c
typedef enum {
    AFATFS_FAT_PATTERN_UNTERMINATED_CHAIN,
    AFATFS_FAT_PATTERN_TERMINATED_CHAIN,
    AFATFS_FAT_PATTERN_FREE
} afatfsFATPattern_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.c
typedef enum {
    AFATFS_FREE_SPACE_SEARCH_PHASE_FIND_HOLE,
    AFATFS_FREE_SPACE_SEARCH_PHASE_GROW_HOLE
} afatfsFreeSpaceSearchPhase_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.c
typedef enum {
    AFATFS_APPEND_SUPERCLUSTER_PHASE_INIT = 0,
    AFATFS_APPEND_SUPERCLUSTER_PHASE_UPDATE_FREEFILE_DIRECTORY,
    AFATFS_APPEND_SUPERCLUSTER_PHASE_UPDATE_FAT,
    AFATFS_APPEND_SUPERCLUSTER_PHASE_UPDATE_FILE_DIRECTORY,
} afatfsAppendSuperclusterPhase_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.c
typedef enum {
    AFATFS_APPEND_FREE_CLUSTER_PHASE_INITIAL = 0,
    AFATFS_APPEND_FREE_CLUSTER_PHASE_FIND_FREESPACE = 0,
    AFATFS_APPEND_FREE_CLUSTER_PHASE_UPDATE_FAT1,
    AFATFS_APPEND_FREE_CLUSTER_PHASE_UPDATE_FAT2,
    AFATFS_APPEND_FREE_CLUSTER_PHASE_UPDATE_FILE_DIRECTORY,
    AFATFS_APPEND_FREE_CLUSTER_PHASE_COMPLETE,
    AFATFS_APPEND_FREE_CLUSTER_PHASE_FAILURE,
} afatfsAppendFreeClusterPhase_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.c
typedef enum {
    AFATFS_EXTEND_SUBDIRECTORY_PHASE_INITIAL = 0,
    AFATFS_EXTEND_SUBDIRECTORY_PHASE_ADD_FREE_CLUSTER = 0,
    AFATFS_EXTEND_SUBDIRECTORY_PHASE_WRITE_SECTORS,
    AFATFS_EXTEND_SUBDIRECTORY_PHASE_SUCCESS,
    AFATFS_EXTEND_SUBDIRECTORY_PHASE_FAILURE
} afatfsExtendSubdirectoryPhase_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.c
typedef enum {
    AFATFS_TRUNCATE_FILE_INITIAL = 0,
    AFATFS_TRUNCATE_FILE_UPDATE_DIRECTORY = 0,
    AFATFS_TRUNCATE_FILE_ERASE_FAT_CHAIN_NORMAL,
#ifdef AFATFS_USE_FREEFILE
    AFATFS_TRUNCATE_FILE_ERASE_FAT_CHAIN_CONTIGUOUS,
    AFATFS_TRUNCATE_FILE_PREPEND_TO_FREEFILE,
#endif
    AFATFS_TRUNCATE_FILE_SUCCESS,
} afatfsTruncateFilePhase_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.c
typedef enum {
    AFATFS_DELETE_FILE_DELETE_DIRECTORY_ENTRY,
    AFATFS_DELETE_FILE_DEALLOCATE_CLUSTERS,
} afatfsDeleteFilePhase_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.c
typedef enum {
    AFATFS_FILE_OPERATION_NONE,
    AFATFS_FILE_OPERATION_CREATE_FILE,
    AFATFS_FILE_OPERATION_SEEK, 
    AFATFS_FILE_OPERATION_CLOSE,
    AFATFS_FILE_OPERATION_TRUNCATE,
    AFATFS_FILE_OPERATION_UNLINK,
#ifdef AFATFS_USE_FREEFILE
    AFATFS_FILE_OPERATION_APPEND_SUPERCLUSTER,
    AFATFS_FILE_OPERATION_LOCKED,
#endif
    AFATFS_FILE_OPERATION_APPEND_FREE_CLUSTER,
    AFATFS_FILE_OPERATION_EXTEND_SUBDIRECTORY,
} afatfsFileOperation_e;

// ../../../src/main/io/asyncfatfs/asyncfatfs.c
typedef enum {
    AFATFS_INITIALIZATION_READ_MBR,
    AFATFS_INITIALIZATION_READ_VOLUME_ID,

#ifdef AFATFS_USE_FREEFILE
    AFATFS_INITIALIZATION_FREEFILE_CREATE,
    AFATFS_INITIALIZATION_FREEFILE_CREATING,
    AFATFS_INITIALIZATION_FREEFILE_FAT_SEARCH,
    AFATFS_INITIALIZATION_FREEFILE_UPDATE_FAT,
    AFATFS_INITIALIZATION_FREEFILE_SAVE_DIR_ENTRY,
    AFATFS_INITIALIZATION_FREEFILE_LAST = AFATFS_INITIALIZATION_FREEFILE_SAVE_DIR_ENTRY,
#endif

    AFATFS_INITIALIZATION_DONE
} afatfsInitializationPhase_e;

// ../../../src/main/flight/servos.h
typedef enum {
    INPUT_STABILIZED_ROLL           = 0,
    INPUT_STABILIZED_PITCH          = 1,
    INPUT_STABILIZED_YAW            = 2,
    INPUT_STABILIZED_THROTTLE       = 3,
    INPUT_RC_ROLL                   = 4,
    INPUT_RC_PITCH                  = 5,
    INPUT_RC_YAW                    = 6,
    INPUT_RC_THROTTLE               = 7,
    INPUT_RC_CH5                    = 8,
    INPUT_RC_CH6                    = 9,
    INPUT_RC_CH7                    = 10,
    INPUT_RC_CH8                    = 11,
    INPUT_GIMBAL_PITCH              = 12,
    INPUT_GIMBAL_ROLL               = 13,
    INPUT_FEATURE_FLAPS             = 14,
    INPUT_RC_CH9                    = 15,
    INPUT_RC_CH10                   = 16,
    INPUT_RC_CH11                   = 17,
    INPUT_RC_CH12                   = 18,
    INPUT_RC_CH13                   = 19,
    INPUT_RC_CH14                   = 20,
    INPUT_RC_CH15                   = 21,
    INPUT_RC_CH16                   = 22,
    INPUT_STABILIZED_ROLL_PLUS      = 23,
    INPUT_STABILIZED_ROLL_MINUS     = 24,
    INPUT_STABILIZED_PITCH_PLUS     = 25,
    INPUT_STABILIZED_PITCH_MINUS    = 26,
    INPUT_STABILIZED_YAW_PLUS       = 27,
    INPUT_STABILIZED_YAW_MINUS      = 28,
    INPUT_MAX                       = 29,
    INPUT_GVAR_0                    = 30,
    INPUT_GVAR_1                    = 31,
    INPUT_GVAR_2                    = 32,
    INPUT_GVAR_3                    = 33,
    INPUT_GVAR_4                    = 34,
    INPUT_GVAR_5                    = 35,
    INPUT_GVAR_6                    = 36,
    INPUT_GVAR_7                    = 37,
    INPUT_MIXER_TRANSITION          = 38,
    INPUT_HEADTRACKER_PAN           = 39,
    INPUT_HEADTRACKER_TILT          = 40,
    INPUT_HEADTRACKER_ROLL          = 41,
    INPUT_RC_CH17                   = 42,
    INPUT_RC_CH18                   = 43,
    INPUT_RC_CH19                   = 44,
    INPUT_RC_CH20                   = 45,
    INPUT_RC_CH21                   = 46,
    INPUT_RC_CH22                   = 47,
    INPUT_RC_CH23                   = 48,
    INPUT_RC_CH24                   = 49,
    INPUT_RC_CH25                   = 50,
    INPUT_RC_CH26                   = 51,
    INPUT_RC_CH27                   = 52,
    INPUT_RC_CH28                   = 53,
    INPUT_RC_CH29                   = 54,
    INPUT_RC_CH30                   = 55,
    INPUT_RC_CH31                   = 56,
    INPUT_RC_CH32                   = 57,
    INPUT_RC_CH33                   = 58,
    INPUT_RC_CH34                   = 59,
    INPUT_SOURCE_COUNT
} inputSource_e;

// ../../../src/main/flight/servos.h
typedef enum {
    SERVO_GIMBAL_PITCH = 0,
    SERVO_GIMBAL_ROLL = 1,
    SERVO_ELEVATOR = 2,
    SERVO_FLAPPERON_1 = 3,
    SERVO_FLAPPERON_2 = 4,
    SERVO_RUDDER = 5,

    SERVO_BICOPTER_LEFT = 4,
    SERVO_BICOPTER_RIGHT = 5,

    SERVO_DUALCOPTER_LEFT = 4,
    SERVO_DUALCOPTER_RIGHT = 5,

    SERVO_SINGLECOPTER_1 = 3,
    SERVO_SINGLECOPTER_2 = 4,
    SERVO_SINGLECOPTER_3 = 5,
    SERVO_SINGLECOPTER_4 = 6,

} servoIndex_e;

// ../../../src/main/flight/mixer_profile.h
typedef enum {
    MIXERAT_REQUEST_NONE, 
    MIXERAT_REQUEST_RTH,
    MIXERAT_REQUEST_LAND,
    MIXERAT_REQUEST_ABORT,
} mixerProfileATRequest_e;

// ../../../src/main/flight/mixer_profile.h
typedef enum {
    MIXERAT_PHASE_IDLE,
    MIXERAT_PHASE_TRANSITION_INITIALIZE,
    MIXERAT_PHASE_TRANSITIONING,
    MIXERAT_PHASE_DONE,
} mixerProfileATState_e;

// ../../../src/main/flight/pid_autotune.c
typedef enum {
    DEMAND_TOO_LOW,
    DEMAND_UNDERSHOOT,
    DEMAND_OVERSHOOT,
    TUNE_UPDATED,
} pidAutotuneState_e;

// ../../../src/main/flight/servos.c
typedef enum {
    AUTOTRIM_IDLE,
    AUTOTRIM_COLLECTING,
    AUTOTRIM_SAVE_PENDING,
    AUTOTRIM_DONE,
} servoAutotrimState_e;

// ../../../src/main/flight/pid.h
typedef enum {
    
    PID_ROLL,       
    PID_PITCH,      
    PID_YAW,        
    PID_POS_Z,      
    PID_POS_XY,     
    PID_VEL_XY,     
    PID_SURFACE,    
    PID_LEVEL,      
    PID_HEADING,    
    PID_VEL_Z,      
    PID_POS_HEADING,
    PID_ITEM_COUNT
} pidIndex_e;

// ../../../src/main/flight/pid.h
typedef enum {
    PID_TYPE_NONE = 0,  
    PID_TYPE_PID,   
    PID_TYPE_PIFF,  
    PID_TYPE_AUTO,  
} pidType_e;

// ../../../src/main/flight/pid.h
typedef enum {
    ITERM_RELAX_OFF = 0,
    ITERM_RELAX_RP,
    ITERM_RELAX_RPY
} itermRelax_e;

// ../../../src/main/flight/pid.h
typedef enum {
    FIXED,
    LIMIT,
    AUTO,
} fw_autotune_rate_adjustment_e;

// ../../../src/main/flight/mixer.h
typedef enum {
    PLATFORM_MULTIROTOR     = 0,
    PLATFORM_AIRPLANE       = 1,
    PLATFORM_HELICOPTER     = 2,
    PLATFORM_TRICOPTER      = 3,
    PLATFORM_ROVER          = 4,
    PLATFORM_BOAT           = 5
} flyingPlatformType_e;

// ../../../src/main/flight/mixer.h
typedef enum {
    OUTPUT_MODE_AUTO     = 0,
    OUTPUT_MODE_MOTORS,
    OUTPUT_MODE_SERVOS,
    OUTPUT_MODE_LED
} outputMode_e;

// ../../../src/main/flight/mixer.h
typedef enum {
    MOTOR_STOPPED_USER,
    MOTOR_STOPPED_AUTO,
    MOTOR_RUNNING
} motorStatus_e;

// ../../../src/main/flight/mixer.h
typedef enum {
    MOTOR_DIRECTION_FORWARD,
    MOTOR_DIRECTION_BACKWARD,
    MOTOR_DIRECTION_DEADBAND
} reversibleMotorsThrottleState_e;

// ../../../src/main/flight/imu.h
typedef enum
{
    COMPMETHOD_VELNED = 0,
    COMPMETHOD_TURNRATE,
    COMPMETHOD_ADAPTIVE
} imu_inertia_comp_method_e;

// ../../../src/main/flight/failsafe.h
typedef enum {
    FAILSAFE_IDLE = 0,
    
    FAILSAFE_RX_LOSS_DETECTED,
    
    FAILSAFE_RX_LOSS_IDLE,
    
    FAILSAFE_RETURN_TO_HOME,
    
    FAILSAFE_LANDING,
    
    FAILSAFE_LANDED,
    
    FAILSAFE_RX_LOSS_MONITORING,
    
    FAILSAFE_RX_LOSS_RECOVERED
    
} failsafePhase_e;

// ../../../src/main/flight/failsafe.h
typedef enum {
    FAILSAFE_RXLINK_DOWN = 0,
    FAILSAFE_RXLINK_UP
} failsafeRxLinkState_e;

// ../../../src/main/flight/failsafe.h
typedef enum {
    FAILSAFE_PROCEDURE_AUTO_LANDING = 0,
    FAILSAFE_PROCEDURE_DROP_IT,
    FAILSAFE_PROCEDURE_RTH,
    FAILSAFE_PROCEDURE_NONE
} failsafeProcedure_e;

// ../../../src/main/flight/failsafe.h
typedef enum {
    RTH_IDLE = 0,               
    RTH_IN_PROGRESS,            
    RTH_HAS_LANDED              
} rthState_e;

// ../../../src/main/flight/failsafe.h
typedef enum {
    EMERG_LAND_IDLE = 0,        
    EMERG_LAND_IN_PROGRESS,     
    EMERG_LAND_HAS_LANDED       
} emergLandState_e;

// ../../../src/main/flight/failsafe.c
typedef enum {
    FAILSAFE_CHANNEL_HOLD,      
    FAILSAFE_CHANNEL_NEUTRAL,   
} failsafeChannelBehavior_e;

// ../../../src/main/fc/fc_core.h
typedef enum disarmReason_e {
    DISARM_NONE         = 0,
    DISARM_TIMEOUT      = 1,
    DISARM_STICKS       = 2,
    DISARM_SWITCH_3D    = 3,
    DISARM_SWITCH       = 4,
    DISARM_FAILSAFE     = 6,
    DISARM_NAVIGATION   = 7,
    DISARM_LANDING      = 8,
    DISARM_REASON_COUNT
} disarmReason_t;

// ../../../src/main/fc/fc_init.c
typedef enum {
    SYSTEM_STATE_INITIALISING   = 0,
    SYSTEM_STATE_CONFIG_LOADED  = (1 << 0),
    SYSTEM_STATE_SENSORS_READY  = (1 << 1),
    SYSTEM_STATE_MOTORS_READY   = (1 << 2),
    SYSTEM_STATE_TRANSPONDER_ENABLED = (1 << 3),
    SYSTEM_STATE_READY          = (1 << 7)
} systemState_e;

// ../../../src/main/fc/rc_modes.h
typedef enum {
    BOXARM           = 0,
    BOXANGLE         = 1,
    BOXHORIZON       = 2,
    BOXNAVALTHOLD    = 3,    
    BOXHEADINGHOLD   = 4,    
    BOXHEADFREE      = 5,
    BOXHEADADJ       = 6,
    BOXCAMSTAB       = 7,
    BOXNAVRTH        = 8,    
    BOXNAVPOSHOLD    = 9,    
    BOXMANUAL        = 10,
    BOXBEEPERON      = 11,
    BOXLEDLOW        = 12,
    BOXLIGHTS        = 13,
    BOXNAVLAUNCH     = 14,
    BOXOSD           = 15,
    BOXTELEMETRY     = 16,
    BOXBLACKBOX      = 17,
    BOXFAILSAFE      = 18,
    BOXNAVWP         = 19,
    BOXAIRMODE       = 20,
    BOXHOMERESET     = 21,
    BOXGCSNAV        = 22,
    BOXSURFACE       = 24,
    BOXFLAPERON      = 25,
    BOXTURNASSIST    = 26,
    BOXAUTOTRIM      = 27,
    BOXAUTOTUNE      = 28,
    BOXCAMERA1       = 29,
    BOXCAMERA2       = 30,
    BOXCAMERA3       = 31,
    BOXOSDALT1       = 32,
    BOXOSDALT2       = 33,
    BOXOSDALT3       = 34,
    BOXNAVCOURSEHOLD = 35,
    BOXBRAKING       = 36,
    BOXUSER1         = 37,
    BOXUSER2         = 38,
    BOXFPVANGLEMIX   = 39,
    BOXLOITERDIRCHN  = 40,
    BOXMSPRCOVERRIDE = 41,
    BOXPREARM        = 42,
    BOXTURTLE        = 43,
    BOXNAVCRUISE     = 44,
    BOXAUTOLEVEL     = 45,
    BOXPLANWPMISSION = 46,
    BOXSOARING       = 47,
    BOXUSER3         = 48,
    BOXUSER4         = 49,
    BOXCHANGEMISSION = 50,
    BOXBEEPERMUTE    = 51,
    BOXMULTIFUNCTION = 52,
    BOXMIXERPROFILE  = 53,
    BOXMIXERTRANSITION = 54,
    BOXANGLEHOLD     = 55,
    BOXGIMBALTLOCK   = 56,
    BOXGIMBALRLOCK   = 57,
    BOXGIMBALCENTER  = 58,
    BOXGIMBALHTRK    = 59,
    CHECKBOX_ITEM_COUNT
} boxId_e;

// ../../../src/main/fc/rc_modes.h
typedef enum {
    MODE_OPERATOR_OR, 
    MODE_OPERATOR_AND
} modeActivationOperator_e;

// ../../../src/main/fc/cli.c
typedef enum {
    DUMP_MASTER = (1 << 0),
    DUMP_CONTROL_PROFILE = (1 << 1),
    DUMP_BATTERY_PROFILE = (1 << 2),
    DUMP_MIXER_PROFILE = (1 << 3),
    DUMP_ALL = (1 << 4),
    DO_DIFF = (1 << 5),
    SHOW_DEFAULTS = (1 << 6),
    HIDE_UNUSED = (1 << 7)
} dumpFlags_e;

// ../../../src/main/fc/settings.h
typedef enum {
    
    VAR_UINT8 = (0 << SETTING_TYPE_OFFSET),
    VAR_INT8 = (1 << SETTING_TYPE_OFFSET),
    VAR_UINT16 = (2 << SETTING_TYPE_OFFSET),
    VAR_INT16 = (3 << SETTING_TYPE_OFFSET),
    VAR_UINT32 = (4 << SETTING_TYPE_OFFSET),
    VAR_FLOAT = (5 << SETTING_TYPE_OFFSET), 
    VAR_STRING = (6 << SETTING_TYPE_OFFSET) 
} setting_type_e;

// ../../../src/main/fc/settings.h
typedef enum {
    
    MASTER_VALUE = (0 << SETTING_SECTION_OFFSET),
    PROFILE_VALUE = (1 << SETTING_SECTION_OFFSET),
    CONTROL_RATE_VALUE = (2 << SETTING_SECTION_OFFSET),
    BATTERY_CONFIG_VALUE = (3 << SETTING_SECTION_OFFSET),
    MIXER_CONFIG_VALUE = (4 << SETTING_SECTION_OFFSET),
    EZ_TUNE_VALUE = (5 << SETTING_SECTION_OFFSET)
} setting_section_e;

// ../../../src/main/fc/settings.h
typedef enum {
    
    MODE_DIRECT = (0 << SETTING_MODE_OFFSET),
    MODE_LOOKUP = (1 << SETTING_MODE_OFFSET), 
} setting_mode_e;

// ../../../src/main/fc/fc_init.h
typedef enum {
    SYSTEM_STATE_INITIALISING   = 0,
    SYSTEM_STATE_CONFIG_LOADED  = (1 << 0),
    SYSTEM_STATE_SENSORS_READY  = (1 << 1),
    SYSTEM_STATE_MOTORS_READY   = (1 << 2),
    SYSTEM_STATE_TRANSPONDER_ENABLED = (1 << 3),
    SYSTEM_STATE_READY          = (1 << 7)
} systemState_e;

// ../../../src/main/fc/fc_msp.c
typedef enum {
    MSP_SDCARD_STATE_NOT_PRESENT = 0,
    MSP_SDCARD_STATE_FATAL       = 1,
    MSP_SDCARD_STATE_CARD_INIT   = 2,
    MSP_SDCARD_STATE_FS_INIT     = 3,
    MSP_SDCARD_STATE_READY       = 4
} mspSDCardState_e;

// ../../../src/main/fc/fc_msp.c
typedef enum {
    MSP_SDCARD_FLAG_SUPPORTTED   = 1
} mspSDCardFlags_e;

// ../../../src/main/fc/fc_msp.c
typedef enum {
    MSP_FLASHFS_BIT_READY        = 1,
    MSP_FLASHFS_BIT_SUPPORTED    = 2
} mspFlashfsFlags_e;

// ../../../src/main/fc/fc_msp.c
typedef enum {
    MSP_PASSTHROUGH_SERIAL_ID          = 0xFD,
    MSP_PASSTHROUGH_SERIAL_FUNCTION_ID = 0xFE,
    MSP_PASSTHROUGH_ESC_4WAY           = 0xFF,
 } mspPassthroughType_e;

// ../../../src/main/fc/rc_controls.h
typedef enum rc_alias {
    ROLL = 0,
    PITCH,
    YAW,
    THROTTLE,
    AUX1, 
    AUX2, 
    AUX3, 
    AUX4, 
    AUX5, 
    AUX6, 
    AUX7, 
    AUX8, 
    AUX9, 
    AUX10, 
    AUX11, 
    AUX12, 
    AUX13, 
    AUX14, 
#ifdef USE_34CHANNELS
    AUX15, 
    AUX16, 
    AUX17, 
    AUX18, 
    AUX19, 
    AUX20, 
    AUX21, 
    AUX22, 
    AUX23, 
    AUX24, 
    AUX25, 
    AUX26, 
    AUX27, 
    AUX28, 
    AUX29, 
    AUX30, 
#endif
} rc_alias_e;

// ../../../src/main/fc/rc_controls.h
typedef enum {
    THROTTLE_LOW = 0,
    THROTTLE_HIGH
} throttleStatus_e;

// ../../../src/main/fc/rc_controls.h
typedef enum {
    THROTTLE_STATUS_TYPE_RC = 0,
    THROTTLE_STATUS_TYPE_COMMAND
} throttleStatusType_e;

// ../../../src/main/fc/rc_controls.h
typedef enum {
    NOT_CENTERED = 0,
    CENTERED
} rollPitchStatus_e;

// ../../../src/main/fc/rc_controls.h
typedef enum {
    STICK_CENTER = 0,
    THROTTLE_THRESHOLD,
    STICK_CENTER_ONCE
} airmodeHandlingType_e;

// ../../../src/main/fc/rc_controls.h
typedef enum {
    ROL_LO = (1 << (2 * ROLL)),
    ROL_CE = (3 << (2 * ROLL)),
    ROL_HI = (2 << (2 * ROLL)),

    PIT_LO = (1 << (2 * PITCH)),
    PIT_CE = (3 << (2 * PITCH)),
    PIT_HI = (2 << (2 * PITCH)),

    YAW_LO = (1 << (2 * YAW)),
    YAW_CE = (3 << (2 * YAW)),
    YAW_HI = (2 << (2 * YAW)),

    THR_LO = (1 << (2 * THROTTLE)),
    THR_CE = (3 << (2 * THROTTLE)),
    THR_HI = (2 << (2 * THROTTLE))
} stickPositions_e;

// ../../../src/main/fc/multifunction.h
typedef enum {
    MF_SUSPEND_SAFEHOMES    = (1 << 0),
    MF_SUSPEND_TRACKBACK    = (1 << 1),
    MF_TURTLE_MODE          = (1 << 2),
} multiFunctionFlags_e;

// ../../../src/main/fc/multifunction.h
typedef enum {
    MULTI_FUNC_NONE,
    MULTI_FUNC_1,
    MULTI_FUNC_2,
    MULTI_FUNC_3,
    MULTI_FUNC_4,
    MULTI_FUNC_5,
    MULTI_FUNC_6,
    MULTI_FUNC_END,
} multi_function_e;

// ../../../src/main/fc/rc_adjustments.h
typedef enum {
    ADJUSTMENT_NONE                             = 0,
    ADJUSTMENT_RC_RATE                          = 1,
    ADJUSTMENT_RC_EXPO                          = 2,
    ADJUSTMENT_THROTTLE_EXPO                    = 3,
    ADJUSTMENT_PITCH_ROLL_RATE                  = 4,
    ADJUSTMENT_YAW_RATE                         = 5,
    ADJUSTMENT_PITCH_ROLL_P                     = 6,
    ADJUSTMENT_PITCH_ROLL_I                     = 7,
    ADJUSTMENT_PITCH_ROLL_D                     = 8,
    ADJUSTMENT_PITCH_ROLL_FF                    = 9,
    ADJUSTMENT_PITCH_P                          = 10,
    ADJUSTMENT_PITCH_I                          = 11,
    ADJUSTMENT_PITCH_D                          = 12,
    ADJUSTMENT_PITCH_FF                         = 13,
    ADJUSTMENT_ROLL_P                           = 14,
    ADJUSTMENT_ROLL_I                           = 15,
    ADJUSTMENT_ROLL_D                           = 16,
    ADJUSTMENT_ROLL_FF                          = 17,
    ADJUSTMENT_YAW_P                            = 18,
    ADJUSTMENT_YAW_I                            = 19,
    ADJUSTMENT_YAW_D                            = 20,
    ADJUSTMENT_YAW_FF                           = 21,
    ADJUSTMENT_RATE_PROFILE                     = 22, 
    ADJUSTMENT_PITCH_RATE                       = 23,
    ADJUSTMENT_ROLL_RATE                        = 24,
    ADJUSTMENT_RC_YAW_EXPO                      = 25,
    ADJUSTMENT_MANUAL_RC_EXPO                   = 26,
    ADJUSTMENT_MANUAL_RC_YAW_EXPO               = 27,
    ADJUSTMENT_MANUAL_PITCH_ROLL_RATE           = 28,
    ADJUSTMENT_MANUAL_ROLL_RATE                 = 29,
    ADJUSTMENT_MANUAL_PITCH_RATE                = 30,
    ADJUSTMENT_MANUAL_YAW_RATE                  = 31,
    ADJUSTMENT_NAV_FW_CRUISE_THR                = 32,
    ADJUSTMENT_NAV_FW_PITCH2THR                 = 33,
    ADJUSTMENT_ROLL_BOARD_ALIGNMENT             = 34,
    ADJUSTMENT_PITCH_BOARD_ALIGNMENT            = 35,
    ADJUSTMENT_LEVEL_P                          = 36,
    ADJUSTMENT_LEVEL_I                          = 37,
    ADJUSTMENT_LEVEL_D                          = 38,
    ADJUSTMENT_POS_XY_P                         = 39,
    ADJUSTMENT_POS_XY_I                         = 40,
    ADJUSTMENT_POS_XY_D                         = 41,
    ADJUSTMENT_POS_Z_P                          = 42,
    ADJUSTMENT_POS_Z_I                          = 43,
    ADJUSTMENT_POS_Z_D                          = 44,
    ADJUSTMENT_HEADING_P                        = 45,
    ADJUSTMENT_VEL_XY_P                         = 46,
    ADJUSTMENT_VEL_XY_I                         = 47,
    ADJUSTMENT_VEL_XY_D                         = 48,
    ADJUSTMENT_VEL_Z_P                          = 49,
    ADJUSTMENT_VEL_Z_I                          = 50,
    ADJUSTMENT_VEL_Z_D                          = 51,
    ADJUSTMENT_FW_MIN_THROTTLE_DOWN_PITCH_ANGLE = 52,
    ADJUSTMENT_VTX_POWER_LEVEL                  = 53,
    ADJUSTMENT_TPA                              = 54,
    ADJUSTMENT_TPA_BREAKPOINT                   = 55,
    ADJUSTMENT_NAV_FW_CONTROL_SMOOTHNESS        = 56,
    ADJUSTMENT_FW_TPA_TIME_CONSTANT             = 57,
    ADJUSTMENT_FW_LEVEL_TRIM                    = 58,
    ADJUSTMENT_NAV_WP_MULTI_MISSION_INDEX       = 59,
    ADJUSTMENT_NAV_FW_ALT_CONTROL_RESPONSE      = 60,
    ADJUSTMENT_FUNCTION_COUNT 
} adjustmentFunction_e;

// ../../../src/main/fc/rc_adjustments.h
typedef enum {
    ADJUSTMENT_MODE_STEP,
    ADJUSTMENT_MODE_SELECT
} adjustmentMode_e;

// ../../../src/main/fc/config.h
typedef enum {
    FEATURE_THR_VBAT_COMP = 1 << 0,
    FEATURE_VBAT = 1 << 1,
    FEATURE_TX_PROF_SEL = 1 << 2,       
    FEATURE_BAT_PROFILE_AUTOSWITCH = 1 << 3,
    FEATURE_GEOZONE = 1 << 4,  
    FEATURE_UNUSED_1 = 1 << 5,   
    FEATURE_SOFTSERIAL = 1 << 6,
    FEATURE_GPS = 1 << 7,
    FEATURE_UNUSED_3 = 1 << 8,        
    FEATURE_UNUSED_4 = 1 << 9,          
    FEATURE_TELEMETRY = 1 << 10,
    FEATURE_CURRENT_METER = 1 << 11,
    FEATURE_REVERSIBLE_MOTORS = 1 << 12,
    FEATURE_UNUSED_5 = 1 << 13,         
    FEATURE_UNUSED_6 = 1 << 14,         
    FEATURE_RSSI_ADC = 1 << 15,
    FEATURE_LED_STRIP = 1 << 16,
    FEATURE_DASHBOARD = 1 << 17,
    FEATURE_UNUSED_7 = 1 << 18,         
    FEATURE_BLACKBOX = 1 << 19,
    FEATURE_UNUSED_10 = 1 << 20,        
    FEATURE_TRANSPONDER = 1 << 21,
    FEATURE_AIRMODE = 1 << 22,
    FEATURE_SUPEREXPO_RATES = 1 << 23,
    FEATURE_VTX = 1 << 24,
    FEATURE_UNUSED_8 = 1 << 25,         
    FEATURE_UNUSED_9 = 1 << 26,         
    FEATURE_UNUSED_11 = 1 << 27,        
    FEATURE_PWM_OUTPUT_ENABLE = 1 << 28,
    FEATURE_OSD = 1 << 29,
    FEATURE_FW_LAUNCH = 1 << 30,
    FEATURE_FW_AUTOTRIM = 1 << 31,
} features_e;

// ../../../src/main/fc/runtime_config.h
typedef enum {
    ARMED                                           = (1 << 2),
    WAS_EVER_ARMED                                  = (1 << 3),
    SIMULATOR_MODE_HITL                             = (1 << 4),
    SIMULATOR_MODE_SITL                             = (1 << 5),
    ARMING_DISABLED_GEOZONE                         = (1 << 6),
    ARMING_DISABLED_FAILSAFE_SYSTEM                 = (1 << 7),
    ARMING_DISABLED_NOT_LEVEL                       = (1 << 8),
    ARMING_DISABLED_SENSORS_CALIBRATING             = (1 << 9),
    ARMING_DISABLED_SYSTEM_OVERLOADED               = (1 << 10),
    ARMING_DISABLED_NAVIGATION_UNSAFE               = (1 << 11),
    ARMING_DISABLED_COMPASS_NOT_CALIBRATED          = (1 << 12),
    ARMING_DISABLED_ACCELEROMETER_NOT_CALIBRATED    = (1 << 13),
    ARMING_DISABLED_ARM_SWITCH                      = (1 << 14),
    ARMING_DISABLED_HARDWARE_FAILURE                = (1 << 15),
    ARMING_DISABLED_BOXFAILSAFE                     = (1 << 16),

    ARMING_DISABLED_RC_LINK                         = (1 << 18),
    ARMING_DISABLED_THROTTLE                        = (1 << 19),
    ARMING_DISABLED_CLI                             = (1 << 20),
    ARMING_DISABLED_CMS_MENU                        = (1 << 21),
    ARMING_DISABLED_OSD_MENU                        = (1 << 22),
    ARMING_DISABLED_ROLLPITCH_NOT_CENTERED          = (1 << 23),
    ARMING_DISABLED_SERVO_AUTOTRIM                  = (1 << 24),
    ARMING_DISABLED_OOM                             = (1 << 25),
    ARMING_DISABLED_INVALID_SETTING                 = (1 << 26),
    ARMING_DISABLED_PWM_OUTPUT_ERROR                = (1 << 27),
    ARMING_DISABLED_NO_PREARM                       = (1 << 28),
    ARMING_DISABLED_DSHOT_BEEPER                    = (1 << 29),
    ARMING_DISABLED_LANDING_DETECTED                = (1 << 30),

    ARMING_DISABLED_ALL_FLAGS                       = (ARMING_DISABLED_GEOZONE | ARMING_DISABLED_FAILSAFE_SYSTEM | ARMING_DISABLED_NOT_LEVEL | 
                                                       ARMING_DISABLED_SENSORS_CALIBRATING | ARMING_DISABLED_SYSTEM_OVERLOADED | ARMING_DISABLED_NAVIGATION_UNSAFE |
                                                       ARMING_DISABLED_COMPASS_NOT_CALIBRATED | ARMING_DISABLED_ACCELEROMETER_NOT_CALIBRATED |
                                                       ARMING_DISABLED_ARM_SWITCH | ARMING_DISABLED_HARDWARE_FAILURE | ARMING_DISABLED_BOXFAILSAFE |
                                                       ARMING_DISABLED_RC_LINK | ARMING_DISABLED_THROTTLE | ARMING_DISABLED_CLI |
                                                       ARMING_DISABLED_CMS_MENU | ARMING_DISABLED_OSD_MENU | ARMING_DISABLED_ROLLPITCH_NOT_CENTERED |
                                                       ARMING_DISABLED_SERVO_AUTOTRIM | ARMING_DISABLED_OOM | ARMING_DISABLED_INVALID_SETTING |
                                                       ARMING_DISABLED_PWM_OUTPUT_ERROR | ARMING_DISABLED_NO_PREARM | ARMING_DISABLED_DSHOT_BEEPER |
                                                       ARMING_DISABLED_LANDING_DETECTED),
} armingFlag_e;

// ../../../src/main/fc/runtime_config.h
typedef enum {
    ANGLE_MODE            = (1 << 0),
    HORIZON_MODE          = (1 << 1),
    HEADING_MODE          = (1 << 2),
    NAV_ALTHOLD_MODE      = (1 << 3),
    NAV_RTH_MODE          = (1 << 4),
    NAV_POSHOLD_MODE      = (1 << 5),
    HEADFREE_MODE         = (1 << 6),
    NAV_LAUNCH_MODE       = (1 << 7),
    MANUAL_MODE           = (1 << 8),
    FAILSAFE_MODE         = (1 << 9),
    AUTO_TUNE             = (1 << 10),
    NAV_WP_MODE           = (1 << 11),
    NAV_COURSE_HOLD_MODE  = (1 << 12),
    FLAPERON              = (1 << 13),
    TURN_ASSISTANT        = (1 << 14),
    TURTLE_MODE           = (1 << 15),
    SOARING_MODE          = (1 << 16),
    ANGLEHOLD_MODE        = (1 << 17),
    NAV_FW_AUTOLAND       = (1 << 18),
    NAV_SEND_TO           = (1 << 19),
} flightModeFlags_e;

// ../../../src/main/fc/runtime_config.h
typedef enum {
    GPS_FIX_HOME                        = (1 << 0),
    GPS_FIX                             = (1 << 1),
    CALIBRATE_MAG                       = (1 << 2),
    SMALL_ANGLE                         = (1 << 3),
    FIXED_WING_LEGACY                   = (1 << 4),     
    ANTI_WINDUP                         = (1 << 5),
    FLAPERON_AVAILABLE                  = (1 << 6),
    NAV_MOTOR_STOP_OR_IDLE              = (1 << 7),     
    COMPASS_CALIBRATED                  = (1 << 8),
    ACCELEROMETER_CALIBRATED            = (1 << 9),
#ifdef USE_GPS_FIX_ESTIMATION
    GPS_ESTIMATED_FIX                   = (1 << 10),
#endif
    NAV_CRUISE_BRAKING                  = (1 << 11),
    NAV_CRUISE_BRAKING_BOOST            = (1 << 12),
    NAV_CRUISE_BRAKING_LOCKED           = (1 << 13),
    NAV_EXTRA_ARMING_SAFETY_BYPASSED    = (1 << 14),    
    AIRMODE_ACTIVE                      = (1 << 15),
    ESC_SENSOR_ENABLED                  = (1 << 16),
    AIRPLANE                            = (1 << 17),
    MULTIROTOR                          = (1 << 18),
    ROVER                               = (1 << 19),
    BOAT                                = (1 << 20),
    ALTITUDE_CONTROL                    = (1 << 21),    
    MOVE_FORWARD_ONLY                   = (1 << 22),
    SET_REVERSIBLE_MOTORS_FORWARD       = (1 << 23),
    FW_HEADING_USE_YAW                  = (1 << 24),
    ANTI_WINDUP_DEACTIVATED             = (1 << 25),
    LANDING_DETECTED                    = (1 << 26),
    IN_FLIGHT_EMERG_REARM               = (1 << 27),
    TAILSITTER                          = (1 << 28), 
} stateFlags_t;

// ../../../src/main/fc/runtime_config.h
typedef enum {
    FLM_MANUAL,
    FLM_ACRO,
    FLM_ACRO_AIR,
    FLM_ANGLE,
    FLM_HORIZON,
    FLM_ALTITUDE_HOLD,
    FLM_POSITION_HOLD,
    FLM_RTH,
    FLM_MISSION,
    FLM_COURSE_HOLD,
    FLM_CRUISE,
    FLM_LAUNCH,
    FLM_FAILSAFE,
    FLM_ANGLEHOLD,
    FLM_COUNT
} flightModeForTelemetry_e;

// ../../../src/main/fc/runtime_config.h
typedef enum {
    HITL_RESET_FLAGS            = (0 << 0),
    HITL_ENABLE					= (1 << 0),
    HITL_SIMULATE_BATTERY		= (1 << 1),
    HITL_MUTE_BEEPER			= (1 << 2),
    HITL_USE_IMU			    = (1 << 3), 
    HITL_HAS_NEW_GPS_DATA		= (1 << 4),
    HITL_EXT_BATTERY_VOLTAGE	= (1 << 5), 
    HITL_AIRSPEED               = (1 << 6),
    HITL_EXTENDED_FLAGS         = (1 << 7), 
    HITL_GPS_TIMEOUT            = (1 << 8),
    HITL_PITOT_FAILURE          = (1 << 9)
} simulatorFlags_t;

