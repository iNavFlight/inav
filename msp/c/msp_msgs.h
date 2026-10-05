#pragma once
// Generated from the MSP YAML schema by msp/generator/gen_c.py. Do not edit by hand.

#include <stdint.h>
#include "msp_consts.h"
#include "msp_protocol.h"   // message ids

#if defined(_MSC_VER)
#  pragma pack(push, 1)
#  define MSP_PACKED
#else
#  define MSP_PACKED __attribute__((__packed__))
#endif

#if !defined(MSP_STATIC_ASSERT)
#  if defined(__cplusplus)
#    define MSP_STATIC_ASSERT(cond, name) static_assert(cond, #name)
#  elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#    define MSP_STATIC_ASSERT(cond, name) _Static_assert(cond, #name)
#  else
#    define MSP_STATIC_ASSERT(cond, name) typedef char name[(cond) ? 1 : -1]
#  endif
#endif

// MSP_API_VERSION (MSPv1) id=1
// Provides the MSP protocol version and the INAV API version.
// Notes: Used by configurators to check compatibility.
typedef struct MSP_PACKED {
    uint8_t mspProtocolVersion;  // MSP Protocol version (`MSP_PROTOCOL_VERSION`, typically 0)
    uint8_t apiVersionMajor;  // INAV API Major version (`API_VERSION_MAJOR`)
    uint8_t apiVersionMinor;  // INAV API Minor version (`API_VERSION_MINOR`)
} mspApiVersionReply_t;
MSP_STATIC_ASSERT(sizeof(mspApiVersionReply_t) == 3, mspApiVersionReply_t_size);

// MSP_FC_VARIANT (MSPv1) id=2
// Identifies the flight controller firmware variant (e.g., INAV, Betaflight).
// Notes: See `FLIGHT_CONTROLLER_IDENTIFIER_LENGTH`.
typedef struct MSP_PACKED {
    char fcVariantIdentifier[4];  // 4-character identifier string (e.g., "INAV"). Defined by `flightControllerIdentifier`.
} mspFcVariantReply_t;
MSP_STATIC_ASSERT(sizeof(mspFcVariantReply_t) == 4, mspFcVariantReply_t_size);

// MSP_FC_VERSION (MSPv1) id=3
// Provides the specific version number of the flight controller firmware.
typedef struct MSP_PACKED {
    uint8_t fcVersionMajor;  // Firmware Major version (`FC_VERSION_MAJOR`)
    uint8_t fcVersionMinor;  // Firmware Minor version (`FC_VERSION_MINOR`)
    uint8_t fcVersionPatch;  // Firmware Patch level (`FC_VERSION_PATCH_LEVEL`)
} mspFcVersionReply_t;
MSP_STATIC_ASSERT(sizeof(mspFcVersionReply_t) == 3, mspFcVersionReply_t_size);

// MSP_BOARD_INFO (MSPv1) id=4
// Provides information about the specific hardware board and its capabilities.
// Notes: `BOARD_IDENTIFIER_LENGTH` is 4.
typedef struct MSP_PACKED {
    char boardIdentifier[4];  // 4-character UPPER CASE board identifier (`TARGET_BOARD_IDENTIFIER`)
    uint16_t hardwareRevision;  // Hardware revision number. 0 if not detected (`USE_HARDWARE_REVISION_DETECTION`)
    uint8_t osdSupport;  // OSD chip type: 0=None, 2=Onboard (`USE_OSD`). INAV does not support slave OSD (1)
    uint8_t commCapabilities;  // Bitmask: Communication capabilities: Bit 0=VCP support (`USE_VCP`), Bit 1=SoftSerial support (`USE_SOFTSERIAL1`/`2`), Bit 2=compass orientation auto-detection during mag calibration (`USE_MAG_CALIBRATION_ORIENTATION`) | bitmask
    uint8_t targetNameLength;  // Length of the target name string that follows
    char targetName[];  // Target name string (e.g., "MATEKF405"). Length given by previous field
} mspBoardInfoReply_t;
// variable length: sizeof(mspBoardInfoReply_t) is the fixed header only

// MSP_BUILD_INFO (MSPv1) id=5
// Provides build date, time, and Git revision of the firmware.
typedef struct MSP_PACKED {
    char buildDate[BUILD_DATE_LENGTH];  // Build date string (e.g., "Dec 31 2023"). `BUILD_DATE_LENGTH`.
    char buildTime[BUILD_TIME_LENGTH];  // Build time string (e.g., "23:59:59"). `BUILD_TIME_LENGTH`.
    char gitRevision[GIT_SHORT_REVISION_LENGTH];  // Short Git revision string. `GIT_SHORT_REVISION_LENGTH`.
} mspBuildInfoReply_t;
MSP_STATIC_ASSERT(sizeof(mspBuildInfoReply_t) == 27, mspBuildInfoReply_t_size);

// MSP_INAV_PID (MSPv1) id=6
// Retrieves legacy INAV-specific PID controller related settings. Many fields are now obsolete or placeholders.
// Notes: Superseded by `MSP2_PID` for core PIDs and other specific messages for filter settings.
typedef struct MSP_PACKED {
    uint8_t legacyAsyncProcessing;  // Legacy, unused. Always 0 | always 0
    uint16_t legacyAsyncValue1;  // Legacy, unused. Always 0 | always 0
    int16_t legacyAsyncValue2;  // Legacy, unused. Always 0 | always 0
    uint8_t headingHoldRateLimit;  // Max rate for heading hold P term (`pidProfile()->heading_hold_rate_limit`) | deg/s
    uint8_t headingHoldLpfFreq;  // Fixed LPF frequency for heading hold error (`HEADING_HOLD_ERROR_LPF_FREQ`) | Hz
    int16_t legacyYawJumpLimit;  // Legacy, unused. Always 0 | always 0
    uint8_t legacyGyroLpf;  // Fixed value `GYRO_LPF_256HZ` | Hz
    uint8_t accLpfHz;  // Accelerometer LPF frequency (`accelerometerConfig()->acc_lpf_hz`) cutoff frequency for the low pass filter used on the acc z-axis for althold in Hz | Hz
    uint8_t reserved1;  // Reserved. Always 0 | always 0
    uint8_t reserved2;  // Reserved. Always 0 | always 0
    uint8_t reserved3;  // Reserved. Always 0 | always 0
    uint8_t reserved4;  // Reserved. Always 0 | always 0
} mspInavPidReply_t;
MSP_STATIC_ASSERT(sizeof(mspInavPidReply_t) == 15, mspInavPidReply_t_size);

// MSP_SET_INAV_PID (MSPv1) id=7
// Sets legacy INAV-specific PID controller related settings.
// Notes: Expects 15 bytes.
typedef struct MSP_PACKED {
    uint8_t legacyAsyncProcessing;  // Legacy, ignored
    int16_t legacyAsyncValue1;  // Legacy, ignored
    int16_t legacyAsyncValue2;  // Legacy, ignored
    uint8_t headingHoldRateLimit;  // Sets `pidProfileMutable()->heading_hold_rate_limit`. | deg/s
    uint8_t headingHoldLpfFreq;  // Ignored (fixed value `HEADING_HOLD_ERROR_LPF_FREQ` used) | Hz
    int16_t legacyYawJumpLimit;  // Legacy, ignored
    uint8_t legacyGyroLpf;  // Ignored (historically mapped to `gyro_lpf_e` values).
    uint8_t accLpfHz;  // Sets `accelerometerConfigMutable()->acc_lpf_hz`. | Hz
    uint8_t reserved1;  // Ignored
    uint8_t reserved2;  // Ignored
    uint8_t reserved3;  // Ignored
    uint8_t reserved4;  // Ignored
} mspSetInavPidRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetInavPidRequest_t) == 15, mspSetInavPidRequest_t_size);

// MSP_NAME (MSPv1) id=10
// Returns the user-defined craft name.
// payload is a bare array of craftName; no fixed header, so no struct.
// element count = payload_size / sizeof(mspNameReplyElem_t)
typedef char mspNameReplyElem_t;

// MSP_SET_NAME (MSPv1) id=11
// Sets the user-defined craft name.
// Notes: Maximum length is `MAX_NAME_LENGTH`.
// payload is a bare array of craftName; no fixed header, so no struct.
// element count = payload_size / sizeof(mspSetNameRequestElem_t)
typedef char mspSetNameRequestElem_t;

// MSP_NAV_POSHOLD (MSPv1) id=12
// Retrieves navigation position hold and general manual/auto flight parameters. Some parameters depend on the platform type (Multirotor vs Fixed Wing).
typedef struct MSP_PACKED {
    uint8_t userControlMode;  // Navigation user control mode NAV_GPS_ATTI (0) or NAV_GPS_CRUISE (1)
    uint16_t maxAutoSpeed;  // Max speed in autonomous modes (`navConfig()->general.max_auto_speed`) | cm/s
    uint16_t maxAutoClimbRate;  // Max climb rate in autonomous modes (uses `fw.max_auto_climb_rate` or `mc.max_auto_climb_rate` based on platform) | cm/s
    uint16_t maxManualSpeed;  // Max speed in manual modes with GPS aiding (`navConfig()->general.max_manual_speed`) | cm/s
    uint16_t maxManualClimbRate;  // Max climb rate in manual modes with GPS aiding (uses `fw.max_manual_climb_rate` or `mc.max_manual_climb_rate`) | cm/s
    uint8_t mcMaxBankAngle;  // Max bank angle for multirotor position hold (`navConfig()->mc.max_bank_angle`) | degrees
    uint8_t mcAltHoldThrottleType;  // Enum `navMcAltHoldThrottle_e` mirrored from `navConfig()->mc.althold_throttle_type`.
    uint16_t mcHoverThrottle;  // Multirotor hover throttle PWM value (`currentBatteryProfile->nav.mc.hover_throttle`). | PWM
} mspNavPosholdReply_t;
MSP_STATIC_ASSERT(sizeof(mspNavPosholdReply_t) == 13, mspNavPosholdReply_t_size);

// MSP_SET_NAV_POSHOLD (MSPv1) id=13
// Sets navigation position hold and general manual/auto flight parameters.
// Notes: Expects 13 bytes.
typedef struct MSP_PACKED {
    uint8_t userControlMode;  // Sets `navConfigMutable()->general.flags.user_control_mode`. WARNING: uses unnamed enum in navigation.h 'NAV_GPS_ATTI/NAV_GPS_CRUISE' | enum nav_control_type_e
    uint16_t maxAutoSpeed;  // Sets `navConfigMutable()->general.max_auto_speed`. | cm/s
    uint16_t maxAutoClimbRate;  // Sets `navConfigMutable()->fw.max_auto_climb_rate` or `navConfigMutable()->mc.max_auto_climb_rate` based on `mixerConfig()->platformType`. | cm/s
    uint16_t maxManualSpeed;  // Sets `navConfigMutable()->general.max_manual_speed`. | cm/s
    uint16_t maxManualClimbRate;  // Sets `navConfigMutable()->fw.max_manual_climb_rate` or `navConfigMutable()->mc.max_manual_climb_rate`. | cm/s
    uint8_t mcMaxBankAngle;  // Sets `navConfigMutable()->mc.max_bank_angle`. | degrees
    uint8_t mcAltHoldThrottleType;  // Enum `navMcAltHoldThrottle_e`; updates `navConfigMutable()->mc.althold_throttle_type`.
    uint16_t mcHoverThrottle;  // Sets `currentBatteryProfileMutable->nav.mc.hover_throttle`. | PWM
} mspSetNavPosholdRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetNavPosholdRequest_t) == 13, mspSetNavPosholdRequest_t_size);

// MSP_CALIBRATION_DATA (MSPv1) id=14
// Retrieves sensor calibration data (Accelerometer zero/gain, Magnetometer zero/gain, Optical Flow scale).
// Notes: Total size 27 bytes. Fields related to optional sensors are zero if the sensor is not used.
typedef struct MSP_PACKED {
    uint8_t accCalibAxisFlags;  // Bitmask: Flags indicating which axes of the accelerometer have been calibrated (`accGetCalibrationAxisFlags()`) | bitmask
    int16_t accZeroX;  // Accelerometer zero offset for X-axis (`accelerometerConfig()->accZero.raw[X]`) | Raw ADC
    int16_t accZeroY;  // Accelerometer zero offset for Y-axis (`accelerometerConfig()->accZero.raw[Y]`) | Raw ADC
    int16_t accZeroZ;  // Accelerometer zero offset for Z-axis (`accelerometerConfig()->accZero.raw[Z]`) | Raw ADC
    int16_t accGainX;  // Accelerometer gain/scale for X-axis (`accelerometerConfig()->accGain.raw[X]`) | Raw ADC
    int16_t accGainY;  // Accelerometer gain/scale for Y-axis (`accelerometerConfig()->accGain.raw[Y]`) | Raw ADC
    int16_t accGainZ;  // Accelerometer gain/scale for Z-axis (`accelerometerConfig()->accGain.raw[Z]`) | Raw ADC
    int16_t magZeroX;  // Magnetometer zero offset for X-axis (`compassConfig()->magZero.raw[X]`). 0 if `USE_MAG` disabled | Raw ADC
    int16_t magZeroY;  // Magnetometer zero offset for Y-axis (`compassConfig()->magZero.raw[Y]`). 0 if `USE_MAG` disabled | Raw ADC
    int16_t magZeroZ;  // Magnetometer zero offset for Z-axis (`compassConfig()->magZero.raw[Z]`). 0 if `USE_MAG` disabled | Raw ADC
    uint16_t opflowScale;  // Optical flow scale factor (`opticalFlowConfig()->opflow_scale * 256`). 0 if `USE_OPFLOW` disabled | Scale * 256
    int16_t magGainX;  // Magnetometer gain/scale for X-axis (`compassConfig()->magGain[X]`). 0 if `USE_MAG` disabled | Raw ADC
    int16_t magGainY;  // Magnetometer gain/scale for Y-axis (`compassConfig()->magGain[Y]`). 0 if `USE_MAG` disabled | Raw ADC
    int16_t magGainZ;  // Magnetometer gain/scale for Z-axis (`compassConfig()->magGain[Z]`). 0 if `USE_MAG` disabled | Raw ADC
} mspCalibrationDataReply_t;
MSP_STATIC_ASSERT(sizeof(mspCalibrationDataReply_t) == 27, mspCalibrationDataReply_t_size);

// MSP_SET_CALIBRATION_DATA (MSPv1) id=15
// Sets sensor calibration data.
// Notes: Minimum payload 18 bytes. Adds +6 bytes for magnetometer zeros, +2 for optical flow scale, and +6 for magnetometer gains when those features (`USE_MAG`, `USE_OPFLOW`) are compiled in.
typedef struct MSP_PACKED {
    int16_t accZeroX;  // Sets `accelerometerConfigMutable()->accZero.raw[X]`. | Raw ADC
    int16_t accZeroY;  // Sets `accelerometerConfigMutable()->accZero.raw[Y]`. | Raw ADC
    int16_t accZeroZ;  // Sets `accelerometerConfigMutable()->accZero.raw[Z]`. | Raw ADC
    int16_t accGainX;  // Sets `accelerometerConfigMutable()->accGain.raw[X]`. | Raw ADC
    int16_t accGainY;  // Sets `accelerometerConfigMutable()->accGain.raw[Y]`. | Raw ADC
    int16_t accGainZ;  // Sets `accelerometerConfigMutable()->accGain.raw[Z]`. | Raw ADC
    int16_t magZeroX;  // Sets `compassConfigMutable()->magZero.raw[X]` (if `USE_MAG`) | Raw ADC
    int16_t magZeroY;  // Sets `compassConfigMutable()->magZero.raw[Y]` (if `USE_MAG`) | Raw ADC
    int16_t magZeroZ;  // Sets `compassConfigMutable()->magZero.raw[Z]` (if `USE_MAG`) | Raw ADC
    uint16_t opflowScale;  // Sets `opticalFlowConfigMutable()->opflow_scale = value / 256.0f` (if `USE_OPFLOW`) | Scale * 256 | OPTIONAL: may be absent from a shorter payload
    int16_t magGainX;  // Sets `compassConfigMutable()->magGain[X]` (if `USE_MAG`) | Raw ADC | OPTIONAL: may be absent from a shorter payload
    int16_t magGainY;  // Sets `compassConfigMutable()->magGain[Y]` (if `USE_MAG`) | Raw ADC | OPTIONAL: may be absent from a shorter payload
    int16_t magGainZ;  // Sets `compassConfigMutable()->magGain[Z]` (if `USE_MAG`) | Raw ADC | OPTIONAL: may be absent from a shorter payload
} mspSetCalibrationDataRequest_t;

// MSP_POSITION_ESTIMATION_CONFIG (MSPv1) id=16
// Retrieves parameters related to the INAV position estimation fusion weights and GPS minimum satellite count.
typedef struct MSP_PACKED {
    uint16_t weightZBaroP;  // Barometer Z position fusion weight (`positionEstimationConfig()->w_z_baro_p * 100`) | Weight * 100
    uint16_t weightZGPSP;  // GPS Z position fusion weight (`positionEstimationConfig()->w_z_gps_p * 100`) | Weight * 100
    uint16_t weightZGPSV;  // GPS Z velocity fusion weight (`positionEstimationConfig()->w_z_gps_v * 100`) | Weight * 100
    uint16_t weightXYGPSP;  // GPS XY position fusion weight (`positionEstimationConfig()->w_xy_gps_p * 100`) | Weight * 100
    uint16_t weightXYGPSV;  // GPS XY velocity fusion weight (`positionEstimationConfig()->w_xy_gps_v * 100`) | Weight * 100
    uint8_t minSats;  // Minimum satellites required for GPS use (`gpsConfigMutable()->gpsMinSats`) | Count
    uint8_t useGPSVelNED;  // Legacy flag, always 1 (GPS velocity is always used if available) | Boolean | always 1
} mspPositionEstimationConfigReply_t;
MSP_STATIC_ASSERT(sizeof(mspPositionEstimationConfigReply_t) == 12, mspPositionEstimationConfigReply_t_size);

// MSP_SET_POSITION_ESTIMATION_CONFIG (MSPv1) id=17
// Sets parameters related to the INAV position estimation fusion weights and GPS minimum satellite count.
// Notes: Expects 12 bytes.
typedef struct MSP_PACKED {
    uint16_t weightZBaroP;  // Sets `positionEstimationConfigMutable()->w_z_baro_p = value / 100.0f` (constrained 0.0-10.0) | Weight * 100
    uint16_t weightZGPSP;  // Sets `positionEstimationConfigMutable()->w_z_gps_p = value / 100.0f` (constrained 0.0-10.0) | Weight * 100
    uint16_t weightZGPSV;  // Sets `positionEstimationConfigMutable()->w_z_gps_v = value / 100.0f` (constrained 0.0-10.0) | Weight * 100
    uint16_t weightXYGPSP;  // Sets `positionEstimationConfigMutable()->w_xy_gps_p = value / 100.0f` (constrained 0.0-10.0) | Weight * 100
    uint16_t weightXYGPSV;  // Sets `positionEstimationConfigMutable()->w_xy_gps_v = value / 100.0f` (constrained 0.0-10.0) | Weight * 100
    uint8_t minSats;  // Sets `gpsConfigMutable()->gpsMinSats` (constrained 5-10) | Count
    uint8_t useGPSVelNED;  // Legacy flag, ignored | Boolean
} mspSetPositionEstimationConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetPositionEstimationConfigRequest_t) == 12, mspSetPositionEstimationConfigRequest_t_size);

// MSP_WP_MISSION_LOAD (MSPv1) id=18
// Commands the FC to load the waypoint mission stored in non-volatile memory (e.g., EEPROM or FlashFS) into the active mission buffer.
// Notes: Only functional if `NAV_NON_VOLATILE_WAYPOINT_STORAGE` is defined. Requires 1 byte payload. Returns error if loading fails.
typedef struct MSP_PACKED {
    uint8_t missionID;  // Reserved for future use, currently ignored
} mspWpMissionLoadRequest_t;
MSP_STATIC_ASSERT(sizeof(mspWpMissionLoadRequest_t) == 1, mspWpMissionLoadRequest_t_size);

// MSP_WP_MISSION_SAVE (MSPv1) id=19
// Commands the FC to save the currently active waypoint mission from RAM to non-volatile memory (e.g., EEPROM or FlashFS).
// Notes: Only functional if `NAV_NON_VOLATILE_WAYPOINT_STORAGE` is defined. Requires 1 byte payload. Returns error if saving fails.
typedef struct MSP_PACKED {
    uint8_t missionID;  // Reserved for future use, currently ignored
} mspWpMissionSaveRequest_t;
MSP_STATIC_ASSERT(sizeof(mspWpMissionSaveRequest_t) == 1, mspWpMissionSaveRequest_t_size);

// MSP_WP_GETINFO (MSPv1) id=20
// Retrieves information about the waypoint mission capabilities and the status of the currently loaded mission.
typedef struct MSP_PACKED {
    uint8_t wpCapabilities;  // Reserved for future waypoint capabilities flags. Currently always 0 | always 0
    uint8_t maxWaypoints;  // Maximum number of waypoints supported (`NAV_MAX_WAYPOINTS`)
    uint8_t missionValid;  // Boolean flag indicating if the current mission in RAM is valid (`isWaypointListValid()`)
    uint8_t waypointCount;  // Number of waypoints currently defined in the mission (`getWaypointCount()`)
} mspWpGetinfoReply_t;
MSP_STATIC_ASSERT(sizeof(mspWpGetinfoReply_t) == 4, mspWpGetinfoReply_t_size);

// MSP_RTH_AND_LAND_CONFIG (MSPv1) id=21
// Retrieves configuration parameters related to Return-to-Home (RTH) and automatic landing behaviors.
typedef struct MSP_PACKED {
    uint16_t minRthDistance;  // Minimum distance from home required for RTH to engage (`navConfig()->general.min_rth_distance`) | cm
    uint8_t rthClimbFirst;  // Flag: Climb to RTH altitude before returning (`navConfig()->general.flags.rth_climb_first`) | Boolean
    uint8_t rthClimbIgnoreEmerg;  // Flag: Climb even in emergency RTH (`navConfig()->general.flags.rth_climb_ignore_emerg`) | Boolean
    uint8_t rthTailFirst;  // Flag: Return tail-first during RTH (`navConfig()->general.flags.rth_tail_first`) | Boolean
    uint8_t rthAllowLanding;  // Flag: Allow automatic landing after RTH (`navConfig()->general.flags.rth_allow_landing`) | Boolean
    uint8_t rthAltControlMode;  // RTH altitude control mode (`navConfig()->general.flags.rth_alt_control_mode`). WARNING: uses unnamed enum in navigation.h:253 'NAV_RTH_NO_ALT...' | enum nav_rth_alt_profile_e
    uint16_t rthAbortThreshold;  // Distance increase threshold to abort RTH (`navConfig()->general.rth_abort_threshold`) | cm
    uint16_t rthAltitude;  // Target RTH altitude (`navConfig()->general.rth_altitude`) | cm
    uint16_t landMinAltVspd;  // Landing vertical speed at minimum slowdown altitude (`navConfig()->general.land_minalt_vspd`) | cm/s
    uint16_t landMaxAltVspd;  // Landing vertical speed at maximum slowdown altitude (`navConfig()->general.land_maxalt_vspd`) | cm/s
    uint16_t landSlowdownMinAlt;  // Altitude below which `landMinAltVspd` applies (`navConfig()->general.land_slowdown_minalt`) | cm
    uint16_t landSlowdownMaxAlt;  // Altitude above which `landMaxAltVspd` applies (`navConfig()->general.land_slowdown_maxalt`) | cm
    uint16_t emergDescentRate;  // Vertical speed during emergency landing descent (`navConfig()->general.emerg_descent_rate`) | cm/s
} mspRthAndLandConfigReply_t;
MSP_STATIC_ASSERT(sizeof(mspRthAndLandConfigReply_t) == 21, mspRthAndLandConfigReply_t_size);

// MSP_SET_RTH_AND_LAND_CONFIG (MSPv1) id=22
// Sets configuration parameters related to Return-to-Home (RTH) and automatic landing behaviors.
// Notes: Expects 21 bytes.
typedef struct MSP_PACKED {
    uint16_t minRthDistance;  // Sets `navConfigMutable()->general.min_rth_distance`. | cm
    uint8_t rthClimbFirst;  // Sets `navConfigMutable()->general.flags.rth_climb_first`. | Boolean
    uint8_t rthClimbIgnoreEmerg;  // Sets `navConfigMutable()->general.flags.rth_climb_ignore_emerg`. | Boolean
    uint8_t rthTailFirst;  // Sets `navConfigMutable()->general.flags.rth_tail_first`. | Boolean
    uint8_t rthAllowLanding;  // Sets `navConfigMutable()->general.flags.rth_allow_landing`. | Boolean
    uint8_t rthAltControlMode;  // Sets `navConfigMutable()->general.flags.rth_alt_control_mode`. WARNING: uses unnamed enum in navigation.h:253 | enum nav_rth_alt_profile_e
    uint16_t rthAbortThreshold;  // Sets `navConfigMutable()->general.rth_abort_threshold`. | cm
    uint16_t rthAltitude;  // Sets `navConfigMutable()->general.rth_altitude`. | cm
    uint16_t landMinAltVspd;  // Sets `navConfigMutable()->general.land_minalt_vspd`. | cm/s
    uint16_t landMaxAltVspd;  // Sets `navConfigMutable()->general.land_maxalt_vspd`. | cm/s
    uint16_t landSlowdownMinAlt;  // Sets `navConfigMutable()->general.land_slowdown_minalt`. | cm
    uint16_t landSlowdownMaxAlt;  // Sets `navConfigMutable()->general.land_slowdown_maxalt`. | cm
    uint16_t emergDescentRate;  // Sets `navConfigMutable()->general.emerg_descent_rate`. | cm/s
} mspSetRthAndLandConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetRthAndLandConfigRequest_t) == 21, mspSetRthAndLandConfigRequest_t_size);

// MSP_FW_CONFIG (MSPv1) id=23
// Retrieves configuration parameters specific to Fixed Wing navigation.
typedef struct MSP_PACKED {
    uint16_t cruiseThrottle;  // Cruise throttle command (`currentBatteryProfile->nav.fw.cruise_throttle`). | PWM
    uint16_t minThrottle;  // Minimum throttle during autonomous flight (`currentBatteryProfile->nav.fw.min_throttle`). | PWM
    uint16_t maxThrottle;  // Maximum throttle during autonomous flight (`currentBatteryProfile->nav.fw.max_throttle`). | PWM
    uint8_t maxBankAngle;  // Maximum bank angle allowed (`navConfig()->fw.max_bank_angle`) | degrees
    uint8_t maxClimbAngle;  // Maximum pitch angle during climb (`navConfig()->fw.max_climb_angle`) | degrees
    uint8_t maxDiveAngle;  // Maximum negative pitch angle during descent (`navConfig()->fw.max_dive_angle`) | degrees
    uint8_t pitchToThrottle;  // Pitch-to-throttle gain (`currentBatteryProfile->nav.fw.pitch_to_throttle`); PWM microseconds per degree (10 units ≈ 1% throttle). | us/deg
    uint16_t loiterRadius;  // Default loiter radius (`navConfig()->fw.loiter_radius`). | cm
} mspFwConfigReply_t;
MSP_STATIC_ASSERT(sizeof(mspFwConfigReply_t) == 12, mspFwConfigReply_t_size);

// MSP_SET_FW_CONFIG (MSPv1) id=24
// Sets configuration parameters specific to Fixed Wing navigation.
// Notes: Expects 12 bytes.
typedef struct MSP_PACKED {
    uint16_t cruiseThrottle;  // Sets `currentBatteryProfileMutable->nav.fw.cruise_throttle`. | PWM
    uint16_t minThrottle;  // Sets `currentBatteryProfileMutable->nav.fw.min_throttle`. | PWM
    uint16_t maxThrottle;  // Sets `currentBatteryProfileMutable->nav.fw.max_throttle`. | PWM
    uint8_t maxBankAngle;  // Sets `navConfigMutable()->fw.max_bank_angle`. | degrees
    uint8_t maxClimbAngle;  // Sets `navConfigMutable()->fw.max_climb_angle`. | degrees
    uint8_t maxDiveAngle;  // Sets `navConfigMutable()->fw.max_dive_angle`. | degrees
    uint8_t pitchToThrottle;  // Sets `currentBatteryProfileMutable->nav.fw.pitch_to_throttle` (PWM microseconds per degree; 10 units ≈ 1% throttle). | us/deg
    uint16_t loiterRadius;  // Sets `navConfigMutable()->fw.loiter_radius`. | cm
} mspSetFwConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetFwConfigRequest_t) == 12, mspSetFwConfigRequest_t_size);

// MSP_MODE_RANGES (MSPv1) id=34
// Returns all defined mode activation ranges (aux channel assignments for flight modes).
// Notes: The number of steps and mapping to PWM values depends on internal range calculations.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint8_t modePermanentId;  // Permanent ID of the flight mode (maps to `boxId` via `findBoxByActiveBoxId`). 0 if entry unused | ID
        uint8_t auxChannelIndex;  // 0-based index of the AUX channel used for activation | Index
        uint8_t rangeStartStep;  // Start step (0-48). Each step is 25 PWM units; 0 is <=900 and 48 is >=2100. | step
        uint8_t rangeEndStep;  // End step (0-48). Uses the same 25-PWM step mapping as rangeStartStep. | step
    } items[MAX_MODE_ACTIVATION_CONDITION_COUNT];  // repeat: MAX_MODE_ACTIVATION_CONDITION_COUNT
} mspModeRangesReply_t;
MSP_STATIC_ASSERT(sizeof(mspModeRangesReply_t) == 160, mspModeRangesReply_t_size);

// MSP_SET_MODE_RANGE (MSPv1) id=35
// Sets a single mode activation range by its index.
// Notes: Expects 5 bytes. Updates the mode configuration and recalculates used mode flags. Returns error if `rangeIndex` or `modePermanentId` is invalid.
typedef struct MSP_PACKED {
    uint8_t rangeIndex;  // Index of the mode range to set (0 to `MAX_MODE_ACTIVATION_CONDITION_COUNT - 1`) | Index
    uint8_t modePermanentId;  // Permanent ID of the flight mode to assign | ID
    uint8_t auxChannelIndex;  // 0-based index of the AUX channel | Index
    uint8_t rangeStartStep;  // Start step (0-48). Each step is 25 PWM units; 0 is <=900 and 48 is >=2100. | step
    uint8_t rangeEndStep;  // End step (0-48). Uses the same 25-PWM step mapping as rangeStartStep. | step
} mspSetModeRangeRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetModeRangeRequest_t) == 5, mspSetModeRangeRequest_t_size);

// MSP_FEATURE (MSPv1) id=36
// Returns a bitmask of enabled features.
// Notes: Feature bits are defined in `feature.h`.
typedef struct MSP_PACKED {
    uint32_t featureMask;  // Bitmask: active features (see `featureMask()`) | bitmask | enum features_e
} mspFeatureReply_t;
MSP_STATIC_ASSERT(sizeof(mspFeatureReply_t) == 4, mspFeatureReply_t_size);

// MSP_SET_FEATURE (MSPv1) id=37
// Sets the enabled features using a bitmask. Clears all previous features first.
// Notes: Expects 4 bytes. Updates feature configuration and related settings (e.g., RSSI source).
typedef struct MSP_PACKED {
    uint32_t featureMask;  // Bitmask: features to enable | bitmask | enum features_e
} mspSetFeatureRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetFeatureRequest_t) == 4, mspSetFeatureRequest_t_size);

// MSP_BOARD_ALIGNMENT (MSPv1) id=38
// Returns the sensor board alignment angles relative to the craft frame.
// Notes: Ranges are typically -1800 to +1800 (i.e. -180.0° to +180.0°).
typedef struct MSP_PACKED {
    int16_t rollAlign;  // Board alignment roll angle (`boardAlignment()->rollDeciDegrees`). Negative values tilt left. | deci-degrees
    int16_t pitchAlign;  // Board alignment pitch angle (`boardAlignment()->pitchDeciDegrees`). Negative values nose down. | deci-degrees
    int16_t yawAlign;  // Board alignment yaw angle (`boardAlignment()->yawDeciDegrees`). Negative values rotate counter-clockwise. | deci-degrees
} mspBoardAlignmentReply_t;
MSP_STATIC_ASSERT(sizeof(mspBoardAlignmentReply_t) == 6, mspBoardAlignmentReply_t_size);

// MSP_SET_BOARD_ALIGNMENT (MSPv1) id=39
// Sets the sensor board alignment angles.
// Notes: Expects 6 bytes encoded as little-endian signed deci-degrees (-1800 to +1800 typical).
typedef struct MSP_PACKED {
    int16_t rollAlign;  // Sets `boardAlignmentMutable()->rollDeciDegrees`. | deci-degrees
    int16_t pitchAlign;  // Sets `boardAlignmentMutable()->pitchDeciDegrees`. | deci-degrees
    int16_t yawAlign;  // Sets `boardAlignmentMutable()->yawDeciDegrees`. | deci-degrees
} mspSetBoardAlignmentRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetBoardAlignmentRequest_t) == 6, mspSetBoardAlignmentRequest_t_size);

// MSP_CURRENT_METER_CONFIG (MSPv1) id=40
// Retrieves the configuration for the current sensor.
// Notes: Scale and offset are signed values matching `batteryMetersConfig()->current` fields.
typedef struct MSP_PACKED {
    int16_t scale;  // Current sensor scale factor (`batteryMetersConfig()->current.scale`). Stored in 0.1 mV/A; signed for calibration. | 0.1 mV/A
    int16_t offset;  // Current sensor offset (`batteryMetersConfig()->current.offset`). Signed millivolt adjustment. | mV
    uint8_t type;  // Enum `currentSensor_e` Type of current sensor hardware
    uint16_t capacity;  // Battery capacity (constrained 0-65535) (`currentBatteryProfile->capacity.value`). Note: This is legacy, use `MSP2_INAV_BATTERY_CONFIG` for full 32-bit capacity | mAh (legacy)
} mspCurrentMeterConfigReply_t;
MSP_STATIC_ASSERT(sizeof(mspCurrentMeterConfigReply_t) == 7, mspCurrentMeterConfigReply_t_size);

// MSP_SET_CURRENT_METER_CONFIG (MSPv1) id=41
// Sets the configuration for the current sensor.
// Notes: Expects 7 bytes. Signed values use little-endian two's complement.
typedef struct MSP_PACKED {
    int16_t scale;  // Sets `batteryMetersConfigMutable()->current.scale` (0.1 mV/A, signed). | 0.1 mV/A
    int16_t offset;  // Sets `batteryMetersConfigMutable()->current.offset` (signed millivolts). | mV
    uint8_t type;  // Enum `currentSensor_e` Sets `batteryMetersConfigMutable()->current.type`.
    uint16_t capacity;  // Sets `currentBatteryProfileMutable->capacity.value` (truncated to 16 bits) | mAh (legacy)
} mspSetCurrentMeterConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetCurrentMeterConfigRequest_t) == 7, mspSetCurrentMeterConfigRequest_t_size);

// MSP_MIXER (MSPv1) id=42
// Retrieves the mixer type (Legacy, INAV always returns QuadX).
// Notes: This command is largely obsolete. Mixer configuration is handled differently in INAV (presets, custom mixes). See `MSP2_INAV_MIXER`.
typedef struct MSP_PACKED {
    uint8_t mixerMode;  // Always 3 (QuadX) in INAV for compatibility | always 3
} mspMixerReply_t;
MSP_STATIC_ASSERT(sizeof(mspMixerReply_t) == 1, mspMixerReply_t_size);

// MSP_SET_MIXER (MSPv1) id=43
// Sets the mixer type (Legacy, ignored by INAV).
// Notes: Expects 1 byte. Calls `mixerUpdateStateFlags()` for potential side effects related to presets.
typedef struct MSP_PACKED {
    uint8_t mixerMode;  // Mixer mode to set (ignored by INAV)
} mspSetMixerRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetMixerRequest_t) == 1, mspSetMixerRequest_t_size);

// MSP_RX_CONFIG (MSPv1) id=44
// Retrieves receiver configuration settings. Some fields are Betaflight compatibility placeholders.
typedef struct MSP_PACKED {
    uint8_t serialRxProvider;  // Enum `rxSerialReceiverType_e`. Serial RX provider (`rxConfig()->serialrx_provider`).
    uint16_t maxCheck;  // Upper channel value threshold for stick commands (`rxConfig()->maxcheck`) | PWM
    uint16_t midRc;  // Center channel value (`PWM_RANGE_MIDDLE`, typically 1500) | PWM
    uint16_t minCheck;  // Lower channel value threshold for stick commands (`rxConfig()->mincheck`) | PWM
    uint8_t spektrumSatBind;  // Spektrum bind pulses (`rxConfig()->spektrum_sat_bind`). 0 if `USE_SPEKTRUM_BIND` disabled. | Count/Flag
    uint16_t rxMinUsec;  // Minimum expected pulse width (`rxConfig()->rx_min_usec`) | PWM
    uint16_t rxMaxUsec;  // Maximum expected pulse width (`rxConfig()->rx_max_usec`) | PWM
    uint8_t bfCompatRcInterpolation;  // BF compatibility. Always 0 | always 0
    uint8_t bfCompatRcInterpolationInt;  // BF compatibility. Always 0 | always 0
    uint16_t bfCompatAirModeThreshold;  // BF compatibility. Always 0 | always 0
    uint8_t reserved1;  // Reserved/Padding. Always 0 | always 0
    uint32_t reserved2;  // Reserved/Padding. Always 0 | always 0
    uint8_t reserved3;  // Reserved/Padding. Always 0 | always 0
    uint8_t bfCompatFpvCamAngle;  // BF compatibility. Always 0 | always 0
    uint8_t receiverType;  // Enum `rxReceiverType_e` Receiver type (Parallel PWM, PPM, Serial) ('rxConfig()->receiverType')
} mspRxConfigReply_t;
MSP_STATIC_ASSERT(sizeof(mspRxConfigReply_t) == 24, mspRxConfigReply_t_size);

// MSP_SET_RX_CONFIG (MSPv1) id=45
// Sets receiver configuration settings.
// Notes: Expects 24 bytes.
typedef struct MSP_PACKED {
    uint8_t serialRxProvider;  // Enum `rxSerialReceiverType_e`. Sets `rxConfigMutable()->serialrx_provider`.
    uint16_t maxCheck;  // Sets `rxConfigMutable()->maxcheck`. | PWM
    uint16_t midRc;  // Ignored (`PWM_RANGE_MIDDLE` is used) | PWM
    uint16_t minCheck;  // Sets `rxConfigMutable()->mincheck`. | PWM
    uint8_t spektrumSatBind;  // Sets `rxConfigMutable()->spektrum_sat_bind` (if `USE_SPEKTRUM_BIND`). | Count/Flag
    uint16_t rxMinUsec;  // Sets `rxConfigMutable()->rx_min_usec`. | PWM
    uint16_t rxMaxUsec;  // Sets `rxConfigMutable()->rx_max_usec`. | PWM
    uint8_t bfCompatRcInterpolation;  // Ignored
    uint8_t bfCompatRcInterpolationInt;  // Ignored
    uint16_t bfCompatAirModeThreshold;  // Ignored
    uint8_t reserved1;  // Ignored
    uint32_t reserved2;  // Ignored
    uint8_t reserved3;  // Ignored
    uint8_t bfCompatFpvCamAngle;  // Ignored
    uint8_t receiverType;  // Enum `rxReceiverType_e` Sets `rxConfigMutable()->receiverType`.
} mspSetRxConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetRxConfigRequest_t) == 24, mspSetRxConfigRequest_t_size);

// MSP_LED_COLORS (MSPv1) id=46
// Retrieves the HSV color definitions for configurable LED colors.
// Notes: Only available if `USE_LED_STRIP` is defined.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint16_t hue;  // Hue value (0-359)
        uint8_t saturation;  // Saturation value (0-255)
        uint8_t value;  // Value/Brightness (0-255)
    } items[LED_CONFIGURABLE_COLOR_COUNT];  // repeat: LED_CONFIGURABLE_COLOR_COUNT
} mspLedColorsReply_t;
MSP_STATIC_ASSERT(sizeof(mspLedColorsReply_t) == 64, mspLedColorsReply_t_size);

// MSP_SET_LED_COLORS (MSPv1) id=47
// Sets the HSV color definitions for configurable LED colors.
// Notes: Only available if `USE_LED_STRIP` is defined. Expects `LED_CONFIGURABLE_COLOR_COUNT * 4` bytes.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint16_t hue;  // Hue value (0-359)
        uint8_t saturation;  // Saturation value (0-255)
        uint8_t value;  // Value/Brightness (0-255)
    } items[LED_CONFIGURABLE_COLOR_COUNT];  // repeat: LED_CONFIGURABLE_COLOR_COUNT
} mspSetLedColorsRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetLedColorsRequest_t) == 64, mspSetLedColorsRequest_t_size);

// MSP_LED_STRIP_CONFIG (MSPv1) id=48
// Retrieves the configuration for each LED on the strip (legacy packed format).
// Notes: Only available if `USE_LED_STRIP` is defined. Superseded by `MSP2_INAV_LED_STRIP_CONFIG_EX` which uses a clearer struct.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint32_t legacyLedConfig;  // Packed LED configuration (position, function, overlay, color, direction, params). See C code for bit packing details
    } items[LED_MAX_STRIP_LENGTH];  // repeat: LED_MAX_STRIP_LENGTH
} mspLedStripConfigReply_t;
MSP_STATIC_ASSERT(sizeof(mspLedStripConfigReply_t) == 512, mspLedStripConfigReply_t_size);

// MSP_SET_LED_STRIP_CONFIG (MSPv1) id=49
// Sets the configuration for a single LED on the strip using the legacy packed format.
// Notes: Only available if `USE_LED_STRIP` is defined. Expects 5 bytes. Calls `reevaluateLedConfig()`. Superseded by `MSP2_INAV_SET_LED_STRIP_CONFIG_EX`.
typedef struct MSP_PACKED {
    uint8_t ledIndex;  // Index of the LED to configure (0 to `LED_MAX_STRIP_LENGTH - 1`)
    uint32_t legacyLedConfig;  // Packed LED configuration to set
} mspSetLedStripConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetLedStripConfigRequest_t) == 5, mspSetLedStripConfigRequest_t_size);

// MSP_RSSI_CONFIG (MSPv1) id=50
// Retrieves the channel used for analog RSSI input.
typedef struct MSP_PACKED {
    uint8_t rssiChannel;  // AUX channel index (1-based) used for RSSI, or 0 if disabled (`rxConfig()->rssi_channel`)
} mspRssiConfigReply_t;
MSP_STATIC_ASSERT(sizeof(mspRssiConfigReply_t) == 1, mspRssiConfigReply_t_size);

// MSP_SET_RSSI_CONFIG (MSPv1) id=51
// Sets the channel used for analog RSSI input.
// Notes: Expects 1 byte. Input value is constrained 0 to `MAX_SUPPORTED_RC_CHANNEL_COUNT`. Updates the effective RSSI source.
typedef struct MSP_PACKED {
    uint8_t rssiChannel;  // AUX channel index (1-based) to use for RSSI, or 0 to disable
} mspSetRssiConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetRssiConfigRequest_t) == 1, mspSetRssiConfigRequest_t_size);

// MSP_ADJUSTMENT_RANGES (MSPv1) id=52
// Returns all defined RC adjustment ranges (tuning via aux channels).
// Notes: See `adjustmentRange_t`.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint8_t adjustmentIndex;  // Index of the adjustment slot (0 to `MAX_SIMULTANEOUS_ADJUSTMENT_COUNT - 1`)
        uint8_t auxChannelIndex;  // 0-based index of the AUX channel controlling the adjustment value
        uint8_t rangeStartStep;  // Start step (0-48). Each step is 25 PWM units; 0 is <=900 and 48 is >=2100. | step
        uint8_t rangeEndStep;  // End step (0-48). Uses the same 25-PWM step mapping as rangeStartStep. | step
        uint8_t adjustmentFunction;  // Function/parameter being adjusted (see `adjustmentFunction_e`). | enum adjustmentFunction_e
        uint8_t auxSwitchChannelIndex;  // 0-based index of the AUX channel acting as an enable switch (or 0 if always enabled)
    } items[MAX_ADJUSTMENT_RANGE_COUNT];  // repeat: MAX_ADJUSTMENT_RANGE_COUNT
} mspAdjustmentRangesReply_t;
MSP_STATIC_ASSERT(sizeof(mspAdjustmentRangesReply_t) == 120, mspAdjustmentRangesReply_t_size);

// MSP_SET_ADJUSTMENT_RANGE (MSPv1) id=53
// Sets a single RC adjustment range configuration by its index.
// Notes: Expects 7 bytes. Returns error if `rangeIndex` or `adjustmentIndex` is invalid.
typedef struct MSP_PACKED {
    uint8_t rangeIndex;  // Index of the adjustment range to set (0 to `MAX_ADJUSTMENT_RANGE_COUNT - 1`)
    uint8_t adjustmentIndex;  // Adjustment slot index (0 to `MAX_SIMULTANEOUS_ADJUSTMENT_COUNT - 1`)
    uint8_t auxChannelIndex;  // 0-based index of the control AUX channel
    uint8_t rangeStartStep;  // Start step (0-48). Each step is 25 PWM units; 0 is <=900 and 48 is >=2100. | step
    uint8_t rangeEndStep;  // End step (0-48). Uses the same 25-PWM step mapping as rangeStartStep. | step
    uint8_t adjustmentFunction;  // Function/parameter being adjusted. | enum adjustmentFunction_e
    uint8_t auxSwitchChannelIndex;  // 0-based index of the enable switch AUX channel (or 0)
} mspSetAdjustmentRangeRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetAdjustmentRangeRequest_t) == 7, mspSetAdjustmentRangeRequest_t_size);

// MSP_VOLTAGE_METER_CONFIG (MSPv1) id=56
// Retrieves legacy voltage meter configuration (scaled values).
// Notes: Superseded by `MSP2_INAV_BATTERY_CONFIG`.
typedef struct MSP_PACKED {
    uint8_t vbatScale;  // Voltage sensor scale factor / 10 (`batteryMetersConfig()->voltage.scale / 10`). 0 if `USE_ADC` disabled | Scale / 10
    uint8_t vbatMinCell;  // Minimum cell voltage / 10 (`currentBatteryProfile->voltage.cellMin / 10`). 0 if `USE_ADC` disabled | 0.1V
    uint8_t vbatMaxCell;  // Maximum cell voltage / 10 (`currentBatteryProfile->voltage.cellMax / 10`). 0 if `USE_ADC` disabled | 0.1V
    uint8_t vbatWarningCell;  // Warning cell voltage / 10 (`currentBatteryProfile->voltage.cellWarning / 10`). 0 if `USE_ADC` disabled | 0.1V
} mspVoltageMeterConfigReply_t;
MSP_STATIC_ASSERT(sizeof(mspVoltageMeterConfigReply_t) == 4, mspVoltageMeterConfigReply_t_size);

// MSP_SET_VOLTAGE_METER_CONFIG (MSPv1) id=57
// Sets legacy voltage meter configuration (scaled values).
// Notes: Expects 4 bytes. Superseded by `MSP2_INAV_SET_BATTERY_CONFIG`.
typedef struct MSP_PACKED {
    uint8_t vbatScale;  // Sets `batteryMetersConfigMutable()->voltage.scale = value * 10` (if `USE_ADC`) | Scale / 10
    uint8_t vbatMinCell;  // Sets `currentBatteryProfileMutable->voltage.cellMin = value * 10` (if `USE_ADC`) | 0.1V
    uint8_t vbatMaxCell;  // Sets `currentBatteryProfileMutable->voltage.cellMax = value * 10` (if `USE_ADC`) | 0.1V
    uint8_t vbatWarningCell;  // Sets `currentBatteryProfileMutable->voltage.cellWarning = value * 10` (if `USE_ADC`) | 0.1V
} mspSetVoltageMeterConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetVoltageMeterConfigRequest_t) == 4, mspSetVoltageMeterConfigRequest_t_size);

// MSP_SONAR_ALTITUDE (MSPv1) id=58
// Retrieves the altitude measured by the primary rangefinder (sonar or lidar).
typedef struct MSP_PACKED {
    int32_t rangefinderAltitude;  // Latest altitude reading from the rangefinder (`rangefinderGetLatestAltitude()`). 0 if `USE_RANGEFINDER` disabled or no reading. | cm
} mspSonarAltitudeReply_t;
MSP_STATIC_ASSERT(sizeof(mspSonarAltitudeReply_t) == 4, mspSonarAltitudeReply_t_size);

// MSP_RX_MAP (MSPv1) id=64
// Retrieves the RC channel mapping array (AETR, etc.).
// Notes: `MAX_MAPPABLE_RX_INPUTS` is currently 4 (Roll, Pitch, Yaw, Throttle).
typedef struct MSP_PACKED {
    uint8_t rcMap[MAX_MAPPABLE_RX_INPUTS];  // Array defining the mapping from input channel index to logical function (Roll, Pitch, Yaw, Throttle, Aux1...)
} mspRxMapReply_t;
MSP_STATIC_ASSERT(sizeof(mspRxMapReply_t) == 4, mspRxMapReply_t_size);

// MSP_SET_RX_MAP (MSPv1) id=65
// Sets the RC channel mapping array.
// Notes: Expects `MAX_MAPPABLE_RX_INPUTS` bytes (currently 4).
typedef struct MSP_PACKED {
    uint8_t rcMap[MAX_MAPPABLE_RX_INPUTS];  // Array defining the new channel mapping
} mspSetRxMapRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetRxMapRequest_t) == 4, mspSetRxMapRequest_t_size);

// MSP_DATAFLASH_SUMMARY (MSPv1) id=70
// Retrieves summary information about the onboard dataflash chip (if present and used for Blackbox via FlashFS).
// Notes: Requires `USE_FLASHFS`.
typedef struct MSP_PACKED {
    uint8_t flashReady;  // Boolean: 1 if flash chip is ready, 0 otherwise. (`flashIsReady()`). 0 if `USE_FLASHFS` disabled
    uint32_t sectorCount;  // Total number of sectors on the flash chip (`geometry->sectors`). 0 if `USE_FLASHFS` disabled
    uint32_t totalSize;  // Total size of the flash chip in bytes (`geometry->totalSize`). 0 if `USE_FLASHFS` disabled
    uint32_t usedSize;  // Currently used size in bytes (FlashFS offset) (`flashfsGetOffset()`). 0 if `USE_FLASHFS` disabled
} mspDataflashSummaryReply_t;
MSP_STATIC_ASSERT(sizeof(mspDataflashSummaryReply_t) == 13, mspDataflashSummaryReply_t_size);

// MSP_DATAFLASH_READ (MSPv1) id=71
// Reads a block of data from the onboard dataflash (FlashFS).
// Notes: Requires `USE_FLASHFS`. Read length may be truncated by buffer size or end of flashfs volume.
typedef struct MSP_PACKED {
    uint32_t address;  // Starting address to read from within the FlashFS volume
    uint16_t size;  // (Optional) Number of bytes to read. Defaults to 128 if not provided | OPTIONAL: may be absent from a shorter payload
} mspDataflashReadRequest_t;

// MSP_DATAFLASH_READ (MSPv1) id=71
// Reads a block of data from the onboard dataflash (FlashFS).
// Notes: Requires `USE_FLASHFS`. Read length may be truncated by buffer size or end of flashfs volume.
typedef struct MSP_PACKED {
    uint32_t address;  // The starting address from which data was actually read
    uint8_t data[];  // The data read from flash. Length is MIN(requested size, remaining buffer space, remaining flashfs data)
} mspDataflashReadReply_t;
// variable length: sizeof(mspDataflashReadReply_t) is the fixed header only

// MSP_LOOP_TIME (MSPv1) id=73
// Retrieves the configured loop time (PID loop frequency denominator).
// Notes: This is the *configured* target loop time, not necessarily the *actual* measured cycle time (see `MSP_STATUS`).
typedef struct MSP_PACKED {
    uint16_t looptime;  // Configured loop time (`gyroConfig()->looptime`) | PWM
} mspLoopTimeReply_t;
MSP_STATIC_ASSERT(sizeof(mspLoopTimeReply_t) == 2, mspLoopTimeReply_t_size);

// MSP_SET_LOOP_TIME (MSPv1) id=74
// Sets the configured loop time.
// Notes: Expects 2 bytes.
typedef struct MSP_PACKED {
    uint16_t looptime;  // New loop time to set (`gyroConfigMutable()->looptime`) | PWM
} mspSetLoopTimeRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetLoopTimeRequest_t) == 2, mspSetLoopTimeRequest_t_size);

// MSP_FAILSAFE_CONFIG (MSPv1) id=75
// Retrieves the failsafe configuration settings.
typedef struct MSP_PACKED {
    uint8_t failsafeDelay;  // Delay before failsafe stage 1 activates (`failsafeConfig()->failsafe_delay`) | 0.1s
    uint8_t failsafeOffDelay;  // Delay after signal recovery before returning control (`failsafeConfig()->failsafe_off_delay`) | 0.1s
    uint16_t failsafeThrottle;  // Throttle level during failsafe stage 2 (`currentBatteryProfile->failsafe_throttle`) | PWM
    uint8_t legacyKillSwitch;  // Legacy flag, always 0 | always 0
    uint16_t failsafeThrottleLowDelay;  // Delay for throttle-based failsafe detection (`failsafeConfig()->failsafe_throttle_low_delay`). Units of 0.1 seconds. | 0.1s
    uint8_t failsafeProcedure;  // Enum `failsafeProcedure_e` Failsafe procedure (Drop, RTH, Land, etc.) ('failsafeConfig()->failsafe_procedure')
    uint8_t failsafeRecoveryDelay;  // Delay after RTH finishes before attempting recovery (`failsafeConfig()->failsafe_recovery_delay`) | 0.1s
    int16_t failsafeFWRollAngle;  // Fixed-wing failsafe roll angle (`failsafeConfig()->failsafe_fw_roll_angle`). Signed deci-degrees. | deci-degrees
    int16_t failsafeFWPitchAngle;  // Fixed-wing failsafe pitch angle (`failsafeConfig()->failsafe_fw_pitch_angle`). Signed deci-degrees. | deci-degrees
    int16_t failsafeFWYawRate;  // Fixed-wing failsafe yaw rate (`failsafeConfig()->failsafe_fw_yaw_rate`). Signed degrees per second. | deg/s
    uint16_t failsafeStickThreshold;  // Stick movement threshold to exit failsafe (`failsafeConfig()->failsafe_stick_motion_threshold`) | PWM units
    uint16_t failsafeMinDistance;  // Minimum distance from home for RTH failsafe (`failsafeConfig()->failsafe_min_distance`). Units of centimeters. | cm
    uint8_t failsafeMinDistanceProc;  // Enum `failsafeProcedure_e` Failsafe procedure if below min distance ('failsafeConfig()->failsafe_min_distance_procedure')
} mspFailsafeConfigReply_t;
MSP_STATIC_ASSERT(sizeof(mspFailsafeConfigReply_t) == 20, mspFailsafeConfigReply_t_size);

// MSP_SET_FAILSAFE_CONFIG (MSPv1) id=76
// Sets the failsafe configuration settings.
// Notes: Expects 20 bytes.
typedef struct MSP_PACKED {
    uint8_t failsafeDelay;  // Sets `failsafeConfigMutable()->failsafe_delay`. | 0.1s
    uint8_t failsafeOffDelay;  // Sets `failsafeConfigMutable()->failsafe_off_delay`. | 0.1s
    uint16_t failsafeThrottle;  // Sets `currentBatteryProfileMutable->failsafe_throttle`. | PWM
    uint8_t legacyKillSwitch;  // Ignored
    uint16_t failsafeThrottleLowDelay;  // Sets `failsafeConfigMutable()->failsafe_throttle_low_delay`. Units of 0.1 seconds. | 0.1s
    uint8_t failsafeProcedure;  // Enum `failsafeProcedure_e`. Sets `failsafeConfigMutable()->failsafe_procedure`.
    uint8_t failsafeRecoveryDelay;  // Sets `failsafeConfigMutable()->failsafe_recovery_delay`. | 0.1s
    int16_t failsafeFWRollAngle;  // Sets `failsafeConfigMutable()->failsafe_fw_roll_angle`. Signed deci-degrees. | deci-degrees
    int16_t failsafeFWPitchAngle;  // Sets `failsafeConfigMutable()->failsafe_fw_pitch_angle`. Signed deci-degrees. | deci-degrees
    int16_t failsafeFWYawRate;  // Sets `failsafeConfigMutable()->failsafe_fw_yaw_rate`. Signed degrees per second. | deg/s
    uint16_t failsafeStickThreshold;  // Sets `failsafeConfigMutable()->failsafe_stick_motion_threshold`. | PWM units
    uint16_t failsafeMinDistance;  // Sets `failsafeConfigMutable()->failsafe_min_distance`. Units of centimeters. | cm
    uint8_t failsafeMinDistanceProc;  // Enum `failsafeProcedure_e`. Sets `failsafeConfigMutable()->failsafe_min_distance_procedure`.
} mspSetFailsafeConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetFailsafeConfigRequest_t) == 20, mspSetFailsafeConfigRequest_t_size);

// MSP_SDCARD_SUMMARY (MSPv1) id=79
// Retrieves summary information about the SD card status and filesystem.
// Notes: Requires `USE_SDCARD` and `USE_ASYNCFATFS`.
typedef struct MSP_PACKED {
    uint8_t sdCardSupported;  // Bitmask: Bit 0 = 1 if SD card support compiled in (`USE_SDCARD`) | bitmask
    uint8_t sdCardState;  // Enum (`mspSDCardState_e`): Current state (Not Present, Fatal, Card Init, FS Init, Ready). 0 if `USE_SDCARD` disabled | enum mspSDCardState_e
    uint8_t fsError;  // Last filesystem error code (`afatfs_getLastError()`). 0 if `USE_SDCARD` disabled
    uint32_t freeSpaceKB;  // Free space in KiB (`afatfs_getContiguousFreeSpace() / 1024`). 0 if `USE_SDCARD` disabled
    uint32_t totalSpaceKB;  // Total space in KiB (`sdcard_getMetadata()->numBlocks / 2`). 0 if `USE_SDCARD` disabled
} mspSdcardSummaryReply_t;
MSP_STATIC_ASSERT(sizeof(mspSdcardSummaryReply_t) == 11, mspSdcardSummaryReply_t_size);

// MSP_BLACKBOX_CONFIG (MSPv1) id=80
// Legacy command to retrieve Blackbox configuration. Superseded by `MSP2_BLACKBOX_CONFIG`.
// Notes: Returns fixed zero values. Use `MSP2_BLACKBOX_CONFIG`.
typedef struct MSP_PACKED {
    uint8_t blackboxDevice;  // Always 0 (API no longer supported) | always 0
    uint8_t blackboxRateNum;  // Always 0 | always 0
    uint8_t blackboxRateDenom;  // Always 0 | always 0
    uint8_t blackboxPDenom;  // Always 0 | always 0
} mspBlackboxConfigReply_t;
MSP_STATIC_ASSERT(sizeof(mspBlackboxConfigReply_t) == 4, mspBlackboxConfigReply_t_size);

// MSP_OSD_CONFIG (MSPv1) id=84
// Retrieves OSD configuration settings and layout for screen 0. Coordinates are packed as `(Y << 8) | X`. When `USE_OSD` is not compiled in, only `osdDriverType` = `OSD_DRIVER_NONE` is returned.
// Notes: 1 byte if `USE_OSD` disabled; full payload (1 + fields + 2*OSD_ITEM_COUNT bytes) otherwise.
typedef struct MSP_PACKED {
    uint8_t osdDriverType;  // Enum `osdDriver_e`: `OSD_DRIVER_MAX7456` if `USE_OSD`, else `OSD_DRIVER_NONE`.
    uint8_t videoSystem;  // Enum `videoSystem_e`: Video system (Auto/PAL/NTSC) (`osdConfig()->video_system`). Absent when `USE_OSD` is not compiled in | OPTIONAL: may be absent from a shorter payload
    uint8_t units;  // Enum `osd_unit_e` Measurement units (Metric/Imperial) (`osdConfig()->units`). Absent when `USE_OSD` is not compiled in | OPTIONAL: may be absent from a shorter payload
    uint8_t rssiAlarm;  // RSSI alarm threshold (`osdConfig()->rssi_alarm`). Absent when `USE_OSD` is not compiled in | % | OPTIONAL: may be absent from a shorter payload
    uint16_t capAlarm;  // Capacity alarm threshold (`currentBatteryProfile->capacity.warning`). Truncated to 16 bits. Absent when `USE_OSD` is not compiled in. | mAh/mWh | OPTIONAL: may be absent from a shorter payload
    uint16_t timerAlarm;  // Timer alarm threshold in minutes (`osdConfig()->time_alarm`). Absent when `USE_OSD` is not compiled in. | minutes | OPTIONAL: may be absent from a shorter payload
    uint16_t altAlarm;  // Altitude alarm threshold (`osdConfig()->alt_alarm`). Absent when `USE_OSD` is not compiled in | meters | OPTIONAL: may be absent from a shorter payload
    uint16_t distAlarm;  // Distance alarm threshold (`osdConfig()->dist_alarm`). Absent when `USE_OSD` is not compiled in | meters | OPTIONAL: may be absent from a shorter payload
    uint16_t negAltAlarm;  // Negative altitude alarm threshold (`osdConfig()->neg_alt_alarm`). Absent when `USE_OSD` is not compiled in | meters | OPTIONAL: may be absent from a shorter payload
    uint16_t itemPositions[OSD_ITEM_COUNT];  // Packed X/Y position for each OSD item on screen 0 (`osdLayoutsConfig()->item_pos[0][i]`). Absent when `USE_OSD` is not compiled in | packed | OPTIONAL: may be absent from a shorter payload
} mspOsdConfigReply_t;

// MSP_SET_OSD_CONFIG (MSPv1) id=85
// Sets OSD configuration or a single item's position on screen 0.
// Notes: Requires `USE_OSD`. Distinguishes formats based on the first byte. Format 1 requires at least 10 bytes. Format 2 requires 3 bytes. Triggers an OSD redraw. See `MSP2_INAV_OSD_SET_*` for more advanced control.
typedef struct MSP_PACKED {
    uint8_t selector;  // Must be 0xFF (-1) to indicate a configuration update. | always 255
    uint8_t videoSystem;  // Enum `videoSystem_e`: Video system (Auto/PAL/NTSC) (`osdConfig()->video_system`).
    uint8_t units;  // Enum `osd_unit_e` Measurement units (Metric/Imperial) (`osdConfig()->units`).
    uint8_t rssiAlarm;  // RSSI alarm threshold (`osdConfig()->rssi_alarm`). | %
    uint16_t capAlarm;  // Capacity alarm threshold (`currentBatteryProfile->capacity.warning`). Truncated to 16 bits. | mAh/mWh
    uint16_t timerAlarm;  // Timer alarm threshold in minutes (`osdConfig()->time_alarm`). | minutes
    uint16_t altAlarm;  // Altitude alarm threshold (`osdConfig()->alt_alarm`). | meters
    uint16_t distAlarm;  // Distance alarm threshold (`osdConfig()->dist_alarm`). Optional trailing field. | meters | OPTIONAL: may be absent from a shorter payload
    uint16_t negAltAlarm;  // Negative altitude alarm threshold (`osdConfig()->neg_alt_alarm`). Optional trailing field. | meters | OPTIONAL: may be absent from a shorter payload
} mspSetOsdConfig_dataSize_ge_10Request_t;

// MSP_SET_OSD_CONFIG (MSPv1) id=85
// Sets OSD configuration or a single item's position on screen 0.
// Notes: Requires `USE_OSD`. Distinguishes formats based on the first byte. Format 1 requires at least 10 bytes. Format 2 requires 3 bytes. Triggers an OSD redraw. See `MSP2_INAV_OSD_SET_*` for more advanced control.
typedef struct MSP_PACKED {
    uint8_t itemIndex;  // Index of the OSD item to update (0 to `OSD_ITEM_COUNT - 1`). | Index
    uint16_t itemPosition;  // Packed X/Y position (`(Y << 8) | X`) for the specified item. | packed
} mspSetOsdConfig_dataSize_eq_3Request_t;
MSP_STATIC_ASSERT(sizeof(mspSetOsdConfig_dataSize_eq_3Request_t) == 3, mspSetOsdConfig_dataSize_eq_3Request_t_size);

// MSP_OSD_CHAR_WRITE (MSPv1) id=87
// Writes character data to the OSD font memory.
// Notes: Requires `USE_OSD`. Minimum payload is `OSD_CHAR_VISIBLE_BYTES + 1` (8-bit address + 54 bytes). Payload size determines the address width and whether the extra metadata bytes are present. Writes characters via `displayWriteFontCharacter()`.
typedef struct MSP_PACKED {
    uint16_t address;  // Character slot index (0-1023).
    uint8_t charData[OSD_CHAR_BYTES];  // All 64 bytes, including driver metadata.
} mspOsdCharWrite_payloadSize_ge_OSD_CHAR_BYTES_2_ge66_bytesRequest_t;
MSP_STATIC_ASSERT(sizeof(mspOsdCharWrite_payloadSize_ge_OSD_CHAR_BYTES_2_ge66_bytesRequest_t) == 66, mspOsdCharWrite_payloadSize_ge_OSD_CHAR_BYTES_2_ge66_bytesRequest_t_size);

// MSP_OSD_CHAR_WRITE (MSPv1) id=87
// Writes character data to the OSD font memory.
// Notes: Requires `USE_OSD`. Minimum payload is `OSD_CHAR_VISIBLE_BYTES + 1` (8-bit address + 54 bytes). Payload size determines the address width and whether the extra metadata bytes are present. Writes characters via `displayWriteFontCharacter()`.
typedef struct MSP_PACKED {
    uint8_t address;  // Character slot index (0-255).
    uint8_t charData[OSD_CHAR_BYTES];  // All 64 bytes, including driver metadata.
} mspOsdCharWrite_payloadSize_eq_OSD_CHAR_BYTES_1_65_bytesRequest_t;
MSP_STATIC_ASSERT(sizeof(mspOsdCharWrite_payloadSize_eq_OSD_CHAR_BYTES_1_65_bytesRequest_t) == 65, mspOsdCharWrite_payloadSize_eq_OSD_CHAR_BYTES_1_65_bytesRequest_t_size);

// MSP_OSD_CHAR_WRITE (MSPv1) id=87
// Writes character data to the OSD font memory.
// Notes: Requires `USE_OSD`. Minimum payload is `OSD_CHAR_VISIBLE_BYTES + 1` (8-bit address + 54 bytes). Payload size determines the address width and whether the extra metadata bytes are present. Writes characters via `displayWriteFontCharacter()`.
typedef struct MSP_PACKED {
    uint16_t address;  // Character slot index (0-1023).
    uint8_t charData[OSD_CHAR_VISIBLE_BYTES];  // Visible pixel data only (no metadata).
} mspOsdCharWrite_payloadSize_eq_OSD_CHAR_VISIBLE_BYTES_2_56_bytesRequest_t;
MSP_STATIC_ASSERT(sizeof(mspOsdCharWrite_payloadSize_eq_OSD_CHAR_VISIBLE_BYTES_2_56_bytesRequest_t) == 56, mspOsdCharWrite_payloadSize_eq_OSD_CHAR_VISIBLE_BYTES_2_56_bytesRequest_t_size);

// MSP_OSD_CHAR_WRITE (MSPv1) id=87
// Writes character data to the OSD font memory.
// Notes: Requires `USE_OSD`. Minimum payload is `OSD_CHAR_VISIBLE_BYTES + 1` (8-bit address + 54 bytes). Payload size determines the address width and whether the extra metadata bytes are present. Writes characters via `displayWriteFontCharacter()`.
typedef struct MSP_PACKED {
    uint8_t address;  // Character slot index (0-255).
    uint8_t charData[OSD_CHAR_VISIBLE_BYTES];  // Visible pixel data only (no metadata).
} mspOsdCharWrite_payloadSize_eq_OSD_CHAR_VISIBLE_BYTES_1_55_bytesRequest_t;
MSP_STATIC_ASSERT(sizeof(mspOsdCharWrite_payloadSize_eq_OSD_CHAR_VISIBLE_BYTES_1_55_bytesRequest_t) == 55, mspOsdCharWrite_payloadSize_eq_OSD_CHAR_VISIBLE_BYTES_1_55_bytesRequest_t_size);

// MSP_VTX_CONFIG (MSPv1) id=88
// Retrieves the current VTX (Video Transmitter) configuration and capabilities.
// Notes: Returns 1 byte (`VTXDEV_UNKNOWN`) when no VTX is detected or `USE_VTX_CONTROL` is disabled; otherwise sends full payload. BF compatibility field `frequency` (uint16) is missing compared to some BF versions. Use `MSP_VTXTABLE_BAND` and `MSP_VTXTABLE_POWERLEVEL` for details.
typedef struct MSP_PACKED {
    uint8_t vtxDeviceType;  // Enum (`vtxDevType_e`): Type of VTX device detected/configured. `VTXDEV_UNKNOWN` if none | enum vtxDevType_e
    uint8_t band;  // VTX band number (from `vtxSettingsConfig`) | OPTIONAL: may be absent from a shorter payload
    uint8_t channel;  // VTX channel number (from `vtxSettingsConfig`) | OPTIONAL: may be absent from a shorter payload
    uint8_t power;  // VTX power level index (from `vtxSettingsConfig()`). | OPTIONAL: may be absent from a shorter payload
    uint8_t pitMode;  // Boolean: 1 if VTX is currently in pit mode, 0 otherwise. | OPTIONAL: may be absent from a shorter payload
    uint8_t vtxReady;  // Boolean: 1 if VTX device reported ready, 0 otherwise | OPTIONAL: may be absent from a shorter payload
    uint8_t lowPowerDisarm;  // Enum `vtxLowerPowerDisarm_e`: Low-power behaviour while disarmed (`vtxSettingsConfig()->lowPowerDisarm`). | OPTIONAL: may be absent from a shorter payload
    uint8_t vtxTableAvailable;  // Boolean: 1 if VTX tables (band/power) are available for query | OPTIONAL: may be absent from a shorter payload
    uint8_t bandCount;  // Number of bands supported by the VTX device | OPTIONAL: may be absent from a shorter payload
    uint8_t channelCount;  // Number of channels per band supported by the VTX device | OPTIONAL: may be absent from a shorter payload
    uint8_t powerCount;  // Number of power levels supported by the VTX device | OPTIONAL: may be absent from a shorter payload
    uint8_t minPowerIndex;  // Lowest selectable power index; 0 for `VTXDEV_MSP`, otherwise 1. | OPTIONAL: may be absent from a shorter payload
} mspVtxConfigReply_t;

// MSP_SET_VTX_CONFIG (MSPv1) id=89
// Sets VTX band/channel and related options. Fields are a progressive superset based on payload length.
// Notes: Requires dataSize >= 2. If no VTX device or device type is VTXDEV_UNKNOWN, fields are read and discarded. The first uint16 is interpreted as band/channel when value <= VTXCOMMON_MSP_BANDCHAN_CHKVAL, otherwise treated as a frequency value that is not applied by this path. Subsequent fields are applied only if present. If dataSize < 2 the command returns MSP_RESULT_ERROR.
typedef struct MSP_PACKED {
    uint16_t bandChanOrFreq;  // Encoded band/channel if <= `VTXCOMMON_MSP_BANDCHAN_CHKVAL`; otherwise frequency placeholder.
    uint8_t power;
    uint8_t pitMode;
    uint8_t lowPowerDisarm;  // enum vtxLowerPowerDisarm_e
    uint16_t pitModeFreq;
    uint8_t band;
    uint8_t channel;
    uint16_t frequency;
    uint8_t bandCount;  // Read and ignored.
    uint8_t channelCount;  // Read and ignored.
    uint8_t powerCount;  // If 0 < value < current capability, caps `vtxDevice->capability.powerCount`.
} mspSetVtxConfig_payloadSize_ge_14Request_t;
MSP_STATIC_ASSERT(sizeof(mspSetVtxConfig_payloadSize_ge_14Request_t) == 14, mspSetVtxConfig_payloadSize_ge_14Request_t_size);

// MSP_SET_VTX_CONFIG (MSPv1) id=89
// Sets VTX band/channel and related options. Fields are a progressive superset based on payload length.
// Notes: Requires dataSize >= 2. If no VTX device or device type is VTXDEV_UNKNOWN, fields are read and discarded. The first uint16 is interpreted as band/channel when value <= VTXCOMMON_MSP_BANDCHAN_CHKVAL, otherwise treated as a frequency value that is not applied by this path. Subsequent fields are applied only if present. If dataSize < 2 the command returns MSP_RESULT_ERROR.
typedef struct MSP_PACKED {
    uint16_t bandChanOrFreq;
    uint8_t power;
    uint8_t pitMode;
    uint8_t lowPowerDisarm;  // enum vtxLowerPowerDisarm_e
    uint16_t pitModeFreq;
    uint8_t band;
    uint8_t channel;
    uint16_t frequency;  // Read and ignored by INAV.
} mspSetVtxConfig_payloadSize_ge_11Request_t;
MSP_STATIC_ASSERT(sizeof(mspSetVtxConfig_payloadSize_ge_11Request_t) == 11, mspSetVtxConfig_payloadSize_ge_11Request_t_size);

// MSP_SET_VTX_CONFIG (MSPv1) id=89
// Sets VTX band/channel and related options. Fields are a progressive superset based on payload length.
// Notes: Requires dataSize >= 2. If no VTX device or device type is VTXDEV_UNKNOWN, fields are read and discarded. The first uint16 is interpreted as band/channel when value <= VTXCOMMON_MSP_BANDCHAN_CHKVAL, otherwise treated as a frequency value that is not applied by this path. Subsequent fields are applied only if present. If dataSize < 2 the command returns MSP_RESULT_ERROR.
typedef struct MSP_PACKED {
    uint16_t bandChanOrFreq;
    uint8_t power;
    uint8_t pitMode;
    uint8_t lowPowerDisarm;  // enum vtxLowerPowerDisarm_e
    uint16_t pitModeFreq;
    uint8_t band;  // 1..N; overrides band when present.
    uint8_t channel;  // 1..8; overrides channel when present.
} mspSetVtxConfig_payloadSize_ge_9Request_t;
MSP_STATIC_ASSERT(sizeof(mspSetVtxConfig_payloadSize_ge_9Request_t) == 9, mspSetVtxConfig_payloadSize_ge_9Request_t_size);

// MSP_SET_VTX_CONFIG (MSPv1) id=89
// Sets VTX band/channel and related options. Fields are a progressive superset based on payload length.
// Notes: Requires dataSize >= 2. If no VTX device or device type is VTXDEV_UNKNOWN, fields are read and discarded. The first uint16 is interpreted as band/channel when value <= VTXCOMMON_MSP_BANDCHAN_CHKVAL, otherwise treated as a frequency value that is not applied by this path. Subsequent fields are applied only if present. If dataSize < 2 the command returns MSP_RESULT_ERROR.
typedef struct MSP_PACKED {
    uint16_t bandChanOrFreq;
    uint8_t power;
    uint8_t pitMode;
    uint8_t lowPowerDisarm;  // enum vtxLowerPowerDisarm_e
    uint16_t pitModeFreq;  // Read and skipped.
} mspSetVtxConfig_payloadSize_ge_7Request_t;
MSP_STATIC_ASSERT(sizeof(mspSetVtxConfig_payloadSize_ge_7Request_t) == 7, mspSetVtxConfig_payloadSize_ge_7Request_t_size);

// MSP_SET_VTX_CONFIG (MSPv1) id=89
// Sets VTX band/channel and related options. Fields are a progressive superset based on payload length.
// Notes: Requires dataSize >= 2. If no VTX device or device type is VTXDEV_UNKNOWN, fields are read and discarded. The first uint16 is interpreted as band/channel when value <= VTXCOMMON_MSP_BANDCHAN_CHKVAL, otherwise treated as a frequency value that is not applied by this path. Subsequent fields are applied only if present. If dataSize < 2 the command returns MSP_RESULT_ERROR.
typedef struct MSP_PACKED {
    uint16_t bandChanOrFreq;
    uint8_t power;
    uint8_t pitMode;
    uint8_t lowPowerDisarm;  // 0=Off, 1=Always, 2=Until first arm. | enum vtxLowerPowerDisarm_e
} mspSetVtxConfig_payloadSize_ge_5Request_t;
MSP_STATIC_ASSERT(sizeof(mspSetVtxConfig_payloadSize_ge_5Request_t) == 5, mspSetVtxConfig_payloadSize_ge_5Request_t_size);

// MSP_SET_VTX_CONFIG (MSPv1) id=89
// Sets VTX band/channel and related options. Fields are a progressive superset based on payload length.
// Notes: Requires dataSize >= 2. If no VTX device or device type is VTXDEV_UNKNOWN, fields are read and discarded. The first uint16 is interpreted as band/channel when value <= VTXCOMMON_MSP_BANDCHAN_CHKVAL, otherwise treated as a frequency value that is not applied by this path. Subsequent fields are applied only if present. If dataSize < 2 the command returns MSP_RESULT_ERROR.
typedef struct MSP_PACKED {
    uint16_t bandChanOrFreq;
    uint8_t power;
    uint8_t pitMode;
} mspSetVtxConfig_payloadSize_ge_4Request_t;
MSP_STATIC_ASSERT(sizeof(mspSetVtxConfig_payloadSize_ge_4Request_t) == 4, mspSetVtxConfig_payloadSize_ge_4Request_t_size);

// MSP_SET_VTX_CONFIG (MSPv1) id=89
// Sets VTX band/channel and related options. Fields are a progressive superset based on payload length.
// Notes: Requires dataSize >= 2. If no VTX device or device type is VTXDEV_UNKNOWN, fields are read and discarded. The first uint16 is interpreted as band/channel when value <= VTXCOMMON_MSP_BANDCHAN_CHKVAL, otherwise treated as a frequency value that is not applied by this path. Subsequent fields are applied only if present. If dataSize < 2 the command returns MSP_RESULT_ERROR.
typedef struct MSP_PACKED {
    uint16_t bandChanOrFreq;  // If <= `VTXCOMMON_MSP_BANDCHAN_CHKVAL`, decoded as band/channel; otherwise treated as a frequency placeholder.
} mspSetVtxConfig_payloadSize_eq_2Request_t;
MSP_STATIC_ASSERT(sizeof(mspSetVtxConfig_payloadSize_eq_2Request_t) == 2, mspSetVtxConfig_payloadSize_eq_2Request_t_size);

// MSP_ADVANCED_CONFIG (MSPv1) id=90
// Retrieves advanced hardware-related configuration (PWM protocols, rates). Some fields are BF compatibility placeholders.
typedef struct MSP_PACKED {
    uint8_t gyroSyncDenom;  // Always 1 (BF compatibility) | always 1
    uint8_t pidProcessDenom;  // Always 1 (BF compatibility) | always 1
    uint8_t useUnsyncedPwm;  // Always 1 (BF compatibility, INAV uses async PWM based on protocol) | always 1
    uint8_t motorPwmProtocol;  // Motor PWM protocol type (`motorConfig()->motorPwmProtocol`). | enum motorPwmProtocolTypes_e
    uint16_t motorPwmRate;  // Motor PWM rate (if applicable) (`motorConfig()->motorPwmRate`). | Hz
    uint16_t servoPwmRate;  // Servo PWM rate (`servoConfig()->servoPwmRate`). | Hz
    uint8_t legacyGyroSync;  // Always 0 (BF compatibility) | always 0
} mspAdvancedConfigReply_t;
MSP_STATIC_ASSERT(sizeof(mspAdvancedConfigReply_t) == 9, mspAdvancedConfigReply_t_size);

// MSP_SET_ADVANCED_CONFIG (MSPv1) id=91
// Sets advanced hardware-related configuration (PWM protocols, rates).
// Notes: Expects 9 bytes.
typedef struct MSP_PACKED {
    uint8_t gyroSyncDenom;  // Ignored (legacy Betaflight field).
    uint8_t pidProcessDenom;  // Ignored (legacy Betaflight field).
    uint8_t useUnsyncedPwm;  // Ignored (legacy Betaflight field).
    uint8_t motorPwmProtocol;  // Sets `motorConfigMutable()->motorPwmProtocol`. | enum motorPwmProtocolTypes_e
    uint16_t motorPwmRate;  // Sets `motorConfigMutable()->motorPwmRate`. | Hz
    uint16_t servoPwmRate;  // Sets `servoConfigMutable()->servoPwmRate`. | Hz
    uint8_t legacyGyroSync;  // Ignored (legacy Betaflight field).
} mspSetAdvancedConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetAdvancedConfigRequest_t) == 9, mspSetAdvancedConfigRequest_t_size);

// MSP_FILTER_CONFIG (MSPv1) id=92
// Retrieves filter configuration settings (Gyro, D-term, Yaw, Accel). Some fields are BF compatibility placeholders or legacy.
typedef struct MSP_PACKED {
    uint8_t gyroMainLpfHz;  // Gyro main low-pass filter cutoff frequency (`gyroConfig()->gyro_main_lpf_hz`) | Hz
    uint16_t dtermLpfHz;  // D-term low-pass filter cutoff frequency (`pidProfile()->dterm_lpf_hz`) | Hz
    uint16_t yawLpfHz;  // Yaw low-pass filter cutoff frequency (`pidProfile()->yaw_lpf_hz`) | Hz
    uint16_t legacyGyroNotchHz;  // Always 0 (Legacy) | always 0
    uint16_t legacyGyroNotchCutoff;  // Always 1 (Legacy) | always 1
    uint16_t bfCompatDtermNotchHz;  // Always 0 (BF compatibility) | always 0
    uint16_t bfCompatDtermNotchCutoff;  // Always 1 (BF compatibility) | always 1
    uint16_t bfCompatGyroNotch2Hz;  // Always 0 (BF compatibility) | always 0
    uint16_t bfCompatGyroNotch2Cutoff;  // Always 1 (BF compatibility) | always 1
    uint16_t accNotchHz;  // Accelerometer notch filter center frequency (`accelerometerConfig()->acc_notch_hz`) | Hz
    uint16_t accNotchCutoff;  // Accelerometer notch filter cutoff frequency (`accelerometerConfig()->acc_notch_cutoff`) | Hz
    uint16_t legacyGyroStage2LpfHz;  // Always 0 (Legacy) | always 0
} mspFilterConfigReply_t;
MSP_STATIC_ASSERT(sizeof(mspFilterConfigReply_t) == 23, mspFilterConfigReply_t_size);

// MSP_SET_FILTER_CONFIG (MSPv1) id=93
// Sets filter configuration settings. Handles different payload lengths for backward compatibility.
// Notes: Requires at least 22 bytes; intermediate length checks enforce legacy Betaflight frame layout and call `pidInitFilters()` once the D-term notch placeholders are consumed.
typedef struct MSP_PACKED {
    uint8_t gyroMainLpfHz;  // Sets `gyroConfigMutable()->gyro_main_lpf_hz`. (Size >= 5) | Hz
    uint16_t dtermLpfHz;  // Sets `pidProfileMutable()->dterm_lpf_hz` (constrained 0-500). (Size >= 5) | Hz
    uint16_t yawLpfHz;  // Sets `pidProfileMutable()->yaw_lpf_hz` (constrained 0-255). (Size >= 5) | Hz
    uint16_t legacyGyroNotchHz;  // Ignored. (Size >= 9)
    uint16_t legacyGyroNotchCutoff;  // Ignored. (Size >= 9)
    uint16_t bfCompatDtermNotchHz;  // Ignored. (Size >= 13)
    uint16_t bfCompatDtermNotchCutoff;  // Ignored. (Size >= 13)
    uint16_t bfCompatGyroNotch2Hz;  // Ignored. (Size >= 17)
    uint16_t bfCompatGyroNotch2Cutoff;  // Ignored. (Size >= 17)
    uint16_t accNotchHz;  // Sets `accelerometerConfigMutable()->acc_notch_hz` (constrained 0-255). (Size >= 21) | Hz
    uint16_t accNotchCutoff;  // Sets `accelerometerConfigMutable()->acc_notch_cutoff` (constrained 1-255). (Size >= 21) | Hz
    uint16_t legacyGyroStage2LpfHz;  // Ignored. (Size >= 22)
} mspSetFilterConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetFilterConfigRequest_t) == 23, mspSetFilterConfigRequest_t_size);

// MSP_PID_ADVANCED (MSPv1) id=94
// Retrieves advanced PID tuning parameters. Many fields are BF compatibility placeholders.
// Notes: Acceleration limits are scaled by 10 for compatibility.
typedef struct MSP_PACKED {
    uint16_t legacyRollPitchItermIgnore;  // Always 0 (Legacy) | always 0
    uint16_t legacyYawItermIgnore;  // Always 0 (Legacy) | always 0
    uint16_t legacyYawPLimit;  // Always 0 (Legacy) | always 0
    uint8_t bfCompatDeltaMethod;  // Always 0 (BF compatibility) | always 0
    uint8_t bfCompatVbatPidComp;  // Always 0 (BF compatibility) | always 0
    uint8_t bfCompatSetpointRelaxRatio;  // Always 0 (BF compatibility) | always 0
    uint8_t reserved1;  // Always 0 | always 0
    uint16_t legacyPidSumLimit;  // Always 0 (Legacy) | always 0
    uint8_t bfCompatItermThrottleGain;  // Always 0 (BF compatibility) | always 0
    uint16_t accelLimitRollPitch;  // Axis acceleration limit for Roll/Pitch / 10 (`pidProfile()->axisAccelerationLimitRollPitch / 10`) | dps / 10
    uint16_t accelLimitYaw;  // Axis acceleration limit for Yaw / 10 (`pidProfile()->axisAccelerationLimitYaw / 10`) | dps / 10
} mspPidAdvancedReply_t;
MSP_STATIC_ASSERT(sizeof(mspPidAdvancedReply_t) == 17, mspPidAdvancedReply_t_size);

// MSP_SET_PID_ADVANCED (MSPv1) id=95
// Sets advanced PID tuning parameters.
// Notes: Expects 17 bytes.
typedef struct MSP_PACKED {
    uint16_t legacyRollPitchItermIgnore;  // Ignored (legacy compatibility).
    uint16_t legacyYawItermIgnore;  // Ignored (legacy compatibility).
    uint16_t legacyYawPLimit;  // Ignored (legacy compatibility).
    uint8_t bfCompatDeltaMethod;  // Ignored (BF compatibility).
    uint8_t bfCompatVbatPidComp;  // Ignored (BF compatibility).
    uint8_t bfCompatSetpointRelaxRatio;  // Ignored (BF compatibility).
    uint8_t reserved1;  // Ignored (reserved).
    uint16_t legacyPidSumLimit;  // Ignored (legacy compatibility).
    uint8_t bfCompatItermThrottleGain;  // Ignored (BF compatibility).
    uint16_t accelLimitRollPitch;  // Sets `pidProfileMutable()->axisAccelerationLimitRollPitch = value * 10`. | dps / 10
    uint16_t accelLimitYaw;  // Sets `pidProfileMutable()->axisAccelerationLimitYaw = value * 10`. | dps / 10
} mspSetPidAdvancedRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetPidAdvancedRequest_t) == 17, mspSetPidAdvancedRequest_t_size);

// MSP_SENSOR_CONFIG (MSPv1) id=96
// Retrieves the configured hardware type for various sensors.
typedef struct MSP_PACKED {
    uint8_t accHardware;  // Enum (`accelerationSensor_e`): Accelerometer hardware type (`accelerometerConfig()->acc_hardware`) | enum accelerationSensor_e
    uint8_t baroHardware;  // Enum (`baroSensor_e`): Barometer hardware type (`barometerConfig()->baro_hardware`). 0 if `USE_BARO` disabled | enum baroSensor_e
    uint8_t magHardware;  // Enum (`magSensor_e`): Magnetometer hardware type (`compassConfig()->mag_hardware`). 0 if `USE_MAG` disabled | enum magSensor_e
    uint8_t pitotHardware;  // Enum (`pitotSensor_e`): Pitot tube hardware type (`pitotmeterConfig()->pitot_hardware`). 0 if `USE_PITOT` disabled | enum pitotSensor_e
    uint8_t rangefinderHardware;  // Enum (`rangefinderType_e`): Rangefinder hardware type (`rangefinderConfig()->rangefinder_hardware`). 0 if `USE_RANGEFINDER` disabled | enum rangefinderType_e
    uint8_t opflowHardware;  // Enum (`opticalFlowSensor_e`): Optical flow hardware type (`opticalFlowConfig()->opflow_hardware`). 0 if `USE_OPFLOW` disabled | enum opticalFlowSensor_e
} mspSensorConfigReply_t;
MSP_STATIC_ASSERT(sizeof(mspSensorConfigReply_t) == 6, mspSensorConfigReply_t_size);

// MSP_SET_SENSOR_CONFIG (MSPv1) id=97
// Sets the configured hardware type for various sensors.
// Notes: Expects 6 bytes.
typedef struct MSP_PACKED {
    uint8_t accHardware;  // Sets `accelerometerConfigMutable()->acc_hardware` | enum accelerationSensor_e
    uint8_t baroHardware;  // Sets `barometerConfigMutable()->baro_hardware` (if `USE_BARO`) | enum baroSensor_e
    uint8_t magHardware;  // Sets `compassConfigMutable()->mag_hardware` (if `USE_MAG`) | enum magSensor_e
    uint8_t pitotHardware;  // Sets `pitotmeterConfigMutable()->pitot_hardware` (if `USE_PITOT`) | enum pitotSensor_e
    uint8_t rangefinderHardware;  // Sets `rangefinderConfigMutable()->rangefinder_hardware` (if `USE_RANGEFINDER`) | enum rangefinderType_e
    uint8_t opflowHardware;  // Sets `opticalFlowConfigMutable()->opflow_hardware` (if `USE_OPFLOW`) | enum opticalFlowSensor_e
} mspSetSensorConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetSensorConfigRequest_t) == 6, mspSetSensorConfigRequest_t_size);

// MSP_STATUS (MSPv1) id=101
// Provides basic flight controller status including cycle time, errors, sensor status, active modes (first 32), and the current configuration profile.
// Notes: Superseded by `MSP_STATUS_EX` and `MSP2_INAV_STATUS`. `sensorStatus` bitmask: (Bit 0: ACC, 1: BARO, 2: MAG, 3: GPS, 4: RANGEFINDER, 5: OPFLOW, 6: PITOT, 7: TEMP; Bit 15: hardware failure). `activeModesLow` only contains the first 32 modes; use `MSP_ACTIVEBOXES` for the full set.
typedef struct MSP_PACKED {
    uint16_t cycleTime;  // Main loop cycle time (`cycleTime`) | µs
    uint16_t i2cErrors;  // Number of I2C errors encountered (`i2cGetErrorCounter()`). 0 if `USE_I2C` not defined | Count
    uint16_t sensorStatus;  // Bitmask: available/active sensors (`packSensorStatus()`). See notes | bitmask | enum sensors_e
    uint32_t activeModesLow;  // Bitmask: First 32 bits of the active flight modes bitmask (`packBoxModeFlags()`) | bitmask
    uint8_t profile;  // Current configuration profile index (0-based) (`getConfigProfile()`) | Index
} mspStatusReply_t;
MSP_STATIC_ASSERT(sizeof(mspStatusReply_t) == 11, mspStatusReply_t_size);

// MSP_RAW_IMU (MSPv1) id=102
// Provides raw sensor readings from the IMU (Accelerometer, Gyroscope, Magnetometer).
// Notes: Acc scaling is approximate (512 LSB/G). Mag units depend on the sensor.
typedef struct MSP_PACKED {
    int16_t accX;  // Raw accelerometer X reading, scaled (`acc.accADCf[X] * 512`) | ~1/512 G
    int16_t accY;  // Raw accelerometer Y reading, scaled (`acc.accADCf[Y] * 512`) | ~1/512 G
    int16_t accZ;  // Raw accelerometer Z reading, scaled (`acc.accADCf[Z] * 512`) | ~1/512 G
    int16_t gyroX;  // Gyroscope X-axis rate (`gyroRateDps(X)`) | deg/s
    int16_t gyroY;  // Gyroscope Y-axis rate (`gyroRateDps(Y)`) | deg/s
    int16_t gyroZ;  // Gyroscope Z-axis rate (`gyroRateDps(Z)`) | deg/s
    int16_t magX;  // Raw magnetometer X reading (`mag.magADC[X]`). 0 if `USE_MAG` disabled | Raw units
    int16_t magY;  // Raw magnetometer Y reading (`mag.magADC[Y]`). 0 if `USE_MAG` disabled | Raw units
    int16_t magZ;  // Raw magnetometer Z reading (`mag.magADC[Z]`). 0 if `USE_MAG` disabled | Raw units
} mspRawImuReply_t;
MSP_STATIC_ASSERT(sizeof(mspRawImuReply_t) == 18, mspRawImuReply_t_size);

// MSP_SERVO (MSPv1) id=103
// Provides the current output values for all supported servos.
typedef struct MSP_PACKED {
    int16_t servoOutputs[MAX_SUPPORTED_SERVOS];  // Array of current servo output values (typically 1000-2000) | PWM
} mspServoReply_t;
MSP_STATIC_ASSERT(sizeof(mspServoReply_t) == 36, mspServoReply_t_size);

// MSP_MOTOR (MSPv1) id=104
// Provides the current output values for the first 8 motors.
typedef struct MSP_PACKED {
    int16_t motorOutputs[8];  // Array of current motor output values (typically 1000-2000). Values beyond `MAX_SUPPORTED_MOTORS` are 0 | PWM
} mspMotorReply_t;
MSP_STATIC_ASSERT(sizeof(mspMotorReply_t) == 16, mspMotorReply_t_size);

// MSP_RC (MSPv1) id=105
// Provides the current values of the received RC channels.
// Notes: Array length equals `rxRuntimeConfig.channelCount`.
// payload is a bare array of rcChannels; no fixed header, so no struct.
// element count = payload_size / sizeof(mspRcReplyElem_t)
typedef int16_t mspRcReplyElem_t;

// MSP_RAW_GPS (MSPv1) id=106
// Provides raw GPS data (fix status, coordinates, altitude, speed, course).
// Notes: Only available if `USE_GPS` is defined. Altitude is truncated to meters.
typedef struct MSP_PACKED {
    uint8_t fixType;  // Enum `gpsFixType_e` GPS fix type (`gpsSol.fixType`)
    uint8_t numSat;  // Number of satellites used in solution (`gpsSol.numSat`) | Count
    int32_t latitude;  // Latitude (`gpsSol.llh.lat`) | deg * 1e7
    int32_t longitude;  // Longitude (`gpsSol.llh.lon`) | deg * 1e7
    int16_t altitude;  // Altitude above MSL, sent as whole metres (`gpsSol.llh.alt / 100`) | m
    int16_t speed;  // Ground speed (`gpsSol.groundSpeed`) | cm/s
    int16_t groundCourse;  // Ground course (`gpsSol.groundCourse`) | deci-degrees
    uint16_t hdop;  // Horizontal Dilution of Precision (`gpsSol.hdop`) | HDOP * 100
} mspRawGpsReply_t;
MSP_STATIC_ASSERT(sizeof(mspRawGpsReply_t) == 18, mspRawGpsReply_t_size);

// MSP_COMP_GPS (MSPv1) id=107
// Provides computed GPS values: distance and direction to home.
// Notes: Only available if `USE_GPS` is defined.
typedef struct MSP_PACKED {
    uint16_t distanceToHome;  // Distance to the home point (`GPS_distanceToHome`) | meters
    int16_t directionToHome;  // Direction to the home point (0-360) (`GPS_directionToHome`) | degrees
    uint8_t gpsHeartbeat;  // Indicates if GPS data is being received (`gpsSol.flags.gpsHeartbeat`) | Boolean
} mspCompGpsReply_t;
MSP_STATIC_ASSERT(sizeof(mspCompGpsReply_t) == 5, mspCompGpsReply_t_size);

// MSP_ATTITUDE (MSPv1) id=108
// Provides the current attitude estimate (roll, pitch, yaw).
// Notes: Yaw is in degrees.
typedef struct MSP_PACKED {
    int16_t roll;  // Roll angle (`attitude.values.roll`) | deci-degrees
    int16_t pitch;  // Pitch angle (`attitude.values.pitch`) | deci-degrees
    int16_t yaw;  // Yaw/Heading angle (`DECIDEGREES_TO_DEGREES(attitude.values.yaw)`) | degrees
} mspAttitudeReply_t;
MSP_STATIC_ASSERT(sizeof(mspAttitudeReply_t) == 6, mspAttitudeReply_t_size);

// MSP_ALTITUDE (MSPv1) id=109
// Provides estimated altitude, vertical speed (variometer), and raw barometric altitude.
typedef struct MSP_PACKED {
    int32_t estimatedAltitude;  // Estimated altitude above home/sea level (`getEstimatedActualPosition(Z)`) | cm
    int16_t variometer;  // Estimated vertical speed (`getEstimatedActualVelocity(Z)`) | cm/s
    int32_t baroAltitude;  // Latest raw altitude from barometer (`baroGetLatestAltitude()`). 0 if `USE_BARO` disabled | cm
} mspAltitudeReply_t;
MSP_STATIC_ASSERT(sizeof(mspAltitudeReply_t) == 10, mspAltitudeReply_t_size);

// MSP_ANALOG (MSPv1) id=110
// Provides analog sensor readings: battery voltage, current consumption (mAh), RSSI, and current draw (Amps).
// Notes: Superseded by `MSP2_INAV_ANALOG` which provides higher precision and more fields.
typedef struct MSP_PACKED {
    uint8_t vbat;  // Battery voltage, scaled (`getBatteryVoltage() / 10`), constrained 0-255 | 0.1V
    uint16_t mAhDrawn;  // Consumed battery capacity (`getMAhDrawn()`), constrained 0-65535 | mAh
    uint16_t rssi;  // Received Signal Strength Indicator (`getRSSI()`). Units depend on source | 0-1023 or %
    int16_t amperage;  // Current draw (`getAmperage()`), constrained -32768 to 32767 | 0.01A
} mspAnalogReply_t;
MSP_STATIC_ASSERT(sizeof(mspAnalogReply_t) == 7, mspAnalogReply_t_size);

// MSP_RC_TUNING (MSPv1) id=111
// Retrieves RC tuning parameters (rates, expos, TPA) for the current control rate profile.
// Notes: Superseded by `MSP2_INAV_RATE_PROFILE` which includes manual rates/expos.
typedef struct MSP_PACKED {
    uint8_t legacyRcRate;  // Always 100 (Legacy, unused) | always 100
    uint8_t rcExpo;  // Roll/Pitch RC Expo (`currentControlRateProfile->stabilized.rcExpo8`)
    uint8_t rollRate;  // Roll Rate (`currentControlRateProfile->stabilized.rates[FD_ROLL]`)
    uint8_t pitchRate;  // Pitch Rate (`currentControlRateProfile->stabilized.rates[FD_PITCH]`)
    uint8_t yawRate;  // Yaw Rate (`currentControlRateProfile->stabilized.rates[FD_YAW]`)
    uint8_t dynamicThrottlePID;  // Dynamic Throttle PID (TPA) value (`currentControlRateProfile->throttle.dynPID`)
    uint8_t throttleMid;  // Throttle Midpoint (`currentControlRateProfile->throttle.rcMid8`)
    uint8_t throttleExpo;  // Throttle Expo (`currentControlRateProfile->throttle.rcExpo8`)
    uint16_t tpaBreakpoint;  // Throttle PID Attenuation (TPA) breakpoint (`currentControlRateProfile->throttle.tpa_breakpoint`)
    uint8_t rcYawExpo;  // Yaw RC Expo (`currentControlRateProfile->stabilized.rcYawExpo8`)
} mspRcTuningReply_t;
MSP_STATIC_ASSERT(sizeof(mspRcTuningReply_t) == 11, mspRcTuningReply_t_size);

// MSP_ACTIVEBOXES (MSPv1) id=113
// Provides the full bitmask of currently active flight modes (boxes).
// Notes: Use this instead of `MSP_STATUS` or `MSP_STATUS_EX` if more than 32 modes are possible.
typedef struct MSP_PACKED {
    boxBitmask_t activeModes;  // Bitmask: all active modes (`packBoxModeFlags()`). Size depends on `boxBitmask_t` definition | bitmask
} mspActiveboxesReply_t;

// MSP_MISC (MSPv1) id=114
// Retrieves miscellaneous configuration settings, mostly related to RC, GPS, Mag, and Battery voltage (legacy formats).
// Notes: Superseded by `MSP2_INAV_MISC` and other specific commands which offer better precision and more fields.
typedef struct MSP_PACKED {
    uint16_t midRc;  // Mid RC value (`PWM_RANGE_MIDDLE`, typically 1500) | PWM
    uint16_t legacyMinThrottle;  // Always 0 (Legacy) | always 0
    uint16_t maxThrottle;  // Maximum throttle command (`getMaxThrottle()`) | PWM
    uint16_t minCommand;  // Minimum motor command when disarmed (`motorConfig()->mincommand`) | PWM
    uint16_t failsafeThrottle;  // Failsafe throttle level (`currentBatteryProfile->failsafe_throttle`) | PWM
    uint8_t gpsType;  // Enum `gpsProvider_e` GPS provider type (`gpsConfig()->provider`). 0 if `USE_GPS` disabled
    uint8_t legacyGpsBaud;  // Always 0 (Legacy) | always 0
    uint8_t gpsSbasMode;  // Enum `sbasMode_e` GPS SBAS mode (`gpsConfig()->sbasMode`). 0 if `USE_GPS` disabled
    uint8_t legacyMwCurrentOut;  // Always 0 (Legacy) | always 0
    uint8_t rssiChannel;  // RSSI channel index (1-based) (`rxConfig()->rssi_channel`) | Index
    uint8_t reserved1;  // Always 0 | always 0
    uint16_t magDeclination;  // Magnetic declination / 10 (`compassConfig()->mag_declination / 10`). 0 if `USE_MAG` disabled | 0.1 degrees
    uint8_t vbatScale;  // Voltage scale / 10 (`batteryMetersConfig()->voltage.scale / 10`). 0 if `USE_ADC` disabled | Scale / 10
    uint8_t vbatMinCell;  // Min cell voltage / 10 (`currentBatteryProfile->voltage.cellMin / 10`). 0 if `USE_ADC` disabled | 0.1V
    uint8_t vbatMaxCell;  // Max cell voltage / 10 (`currentBatteryProfile->voltage.cellMax / 10`). 0 if `USE_ADC` disabled | 0.1V
    uint8_t vbatWarningCell;  // Warning cell voltage / 10 (`currentBatteryProfile->voltage.cellWarning / 10`). 0 if `USE_ADC` disabled | 0.1V
} mspMiscReply_t;
MSP_STATIC_ASSERT(sizeof(mspMiscReply_t) == 22, mspMiscReply_t_size);

// MSP_BOXNAMES (MSPv1) id=116
// Provides a semicolon-separated string containing the names of all available flight modes (boxes).
// Notes: The exact set of names depends on compiled features and configuration. Due to the size of the payload, it is recommended that [`MSP_BOXIDS`](#msp_boxids-119--0x77) is used instead.
// payload is a bare array of boxNamesString; no fixed header, so no struct.
// element count = payload_size / sizeof(mspBoxnamesReplyElem_t)
typedef char mspBoxnamesReplyElem_t;

// MSP_PIDNAMES (MSPv1) id=117
// Provides a semicolon-separated string containing the names of the PID controllers.
// payload is a bare array of pidNamesString; no fixed header, so no struct.
// element count = payload_size / sizeof(mspPidnamesReplyElem_t)
typedef char mspPidnamesReplyElem_t;

// MSP_WP (MSPv1) id=118
// Get/Set a single waypoint from the mission plan.
// Notes: See `navWaypoint_t` and `navWaypointActions_e`.
typedef struct MSP_PACKED {
    uint8_t waypointIndex;  // Index of the waypoint to retrieve (0 to `NAV_MAX_WAYPOINTS - 1`)
} mspWpRequest_t;
MSP_STATIC_ASSERT(sizeof(mspWpRequest_t) == 1, mspWpRequest_t_size);

// MSP_WP (MSPv1) id=118
// Get/Set a single waypoint from the mission plan.
// Notes: See `navWaypoint_t` and `navWaypointActions_e`.
typedef struct MSP_PACKED {
    uint8_t waypointIndex;  // Index of the returned waypoint | Index
    uint8_t action;  // Enum `navWaypointActions_e` Waypoint action type
    int32_t latitude;  // Latitude coordinate | deg * 1e7
    int32_t longitude;  // Longitude coordinate | deg * 1e7
    int32_t altitude;  // Altitude coordinate (relative to home or sea level, see flag) | cm
    int16_t param1;  // Parameter 1 (meaning depends on action) | Varies
    int16_t param2;  // Parameter 2 (meaning depends on action) | Varies
    int16_t param3;  // Parameter 3 (meaning depends on action) | Varies
    uint8_t flag;  // Bitmask: Waypoint flags (`NAV_WP_FLAG_*`) | bitmask
} mspWpReply_t;
MSP_STATIC_ASSERT(sizeof(mspWpReply_t) == 21, mspWpReply_t_size);

// MSP_BOXIDS (MSPv1) id=119
// Provides a list of permanent IDs associated with the available flight modes (boxes).
// Notes: Useful for mapping mode range configurations (`MSP_MODE_RANGES`) back to user-understandable modes via `MSP_BOXNAMES`.
// payload is a bare array of boxIds; no fixed header, so no struct.
// element count = payload_size / sizeof(mspBoxidsReplyElem_t)
typedef uint8_t mspBoxidsReplyElem_t;

// MSP_SERVO_CONFIGURATIONS (MSPv1) id=120
// Retrieves the configuration parameters for all supported servos (min, max, middle, rate). Legacy format with unused fields.
// Notes: Superseded by `MSP2_INAV_SERVO_CONFIG` which has a cleaner structure.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        int16_t min;  // Minimum servo endpoint (`servoParams(i)->min`) | PWM
        int16_t max;  // Maximum servo endpoint (`servoParams(i)->max`) | PWM
        int16_t middle;  // Middle/Neutral servo position (`servoParams(i)->middle`) | PWM
        int8_t rate;  // Servo rate/scaling (`servoParams(i)->rate`, -125..125). Encoded as two's complement | % (-100 to 100)
        uint8_t reserved1;  // Always 0 | always 0
        uint8_t reserved2;  // Always 0 | always 0
        uint8_t legacyForwardChan;  // Always 255 (Legacy) | always 255
        uint32_t legacyReversedSources;  // Always 0 (Legacy) | always 0
    } items[MAX_SUPPORTED_SERVOS];  // repeat: MAX_SUPPORTED_SERVOS
} mspServoConfigurationsReply_t;
MSP_STATIC_ASSERT(sizeof(mspServoConfigurationsReply_t) == 252, mspServoConfigurationsReply_t_size);

// MSP_NAV_STATUS (MSPv1) id=121
// Retrieves the current status of the navigation system.
// Notes: Requires `USE_GPS`.
typedef struct MSP_PACKED {
    uint8_t navMode;  // Enum (`navSystemStatus_Mode_e`): Current navigation mode (None, RTH, NAV, Hold, etc.) (`NAV_Status.mode`) | enum navSystemStatus_Mode_e
    uint8_t navState;  // Enum (`navSystemStatus_State_e`): Current navigation state (`NAV_Status.state`) | enum navSystemStatus_State_e
    uint8_t activeWpAction;  // Enum (`navWaypointActions_e`): Action of the currently executing waypoint (`NAV_Status.activeWpAction`) | enum navWaypointActions_e
    uint8_t activeWpNumber;  // Index: Index of the currently executing waypoint (`NAV_Status.activeWpNumber`)
    uint8_t navError;  // Enum (`navSystemStatus_Error_e`): Current navigation error code (`NAV_Status.error`) | enum navSystemStatus_Error_e
    int16_t targetHeading;  // Target heading for heading controller (`getHeadingHoldTarget()`) | degrees
    uint16_t desiredHeading;  // Guidance course/track the navigation controller is steering to (`navDesiredHeading`, `wrap_36000()` of the desired yaw) | centi-degrees
} mspNavStatusReply_t;
MSP_STATIC_ASSERT(sizeof(mspNavStatusReply_t) == 9, mspNavStatusReply_t_size);

// MSP_3D (MSPv1) id=124
// Retrieves settings related to 3D/reversible motor operation.
// Notes: Requires reversible motor support.
typedef struct MSP_PACKED {
    uint16_t deadbandLow;  // Lower deadband limit for 3D mode (`reversibleMotorsConfig()->deadband_low`) | PWM
    uint16_t deadbandHigh;  // Upper deadband limit for 3D mode (`reversibleMotorsConfig()->deadband_high`) | PWM
    uint16_t neutral;  // Neutral throttle point for 3D mode (`reversibleMotorsConfig()->neutral`) | PWM
} msp3dReply_t;
MSP_STATIC_ASSERT(sizeof(msp3dReply_t) == 6, msp3dReply_t_size);

// MSP_RC_DEADBAND (MSPv1) id=125
// Retrieves RC input deadband settings.
typedef struct MSP_PACKED {
    uint8_t deadband;  // General RC deadband for Roll/Pitch (`rcControlsConfig()->deadband`) | PWM
    uint8_t yawDeadband;  // Specific deadband for Yaw (`rcControlsConfig()->yaw_deadband`) | PWM
    uint8_t altHoldDeadband;  // Deadband for altitude hold adjustments (`rcControlsConfig()->alt_hold_deadband`) | PWM
    uint16_t throttleDeadband;  // Deadband around throttle mid-stick (`rcControlsConfig()->mid_throttle_deadband`) | PWM
} mspRcDeadbandReply_t;
MSP_STATIC_ASSERT(sizeof(mspRcDeadbandReply_t) == 5, mspRcDeadbandReply_t_size);

// MSP_SENSOR_ALIGNMENT (MSPv1) id=126
// Retrieves sensor alignment settings (legacy format).
// Notes: Board alignment is now typically handled by `MSP_BOARD_ALIGNMENT`. This returns legacy enum values where applicable.
typedef struct MSP_PACKED {
    uint8_t gyroAlign;  // Always 0 (Legacy alignment enum) | always 0
    uint8_t accAlign;  // Always 0 (Legacy alignment enum) | always 0
    uint8_t magAlign;  // Magnetometer alignment (`compassConfig()->mag_align`). 0 if `USE_MAG` disabled
    uint8_t opflowAlign;  // Optical flow alignment (`opticalFlowConfig()->opflow_align`). 0 if `USE_OPFLOW` disabled
} mspSensorAlignmentReply_t;
MSP_STATIC_ASSERT(sizeof(mspSensorAlignmentReply_t) == 4, mspSensorAlignmentReply_t_size);

// MSP_LED_STRIP_MODECOLOR (MSPv1) id=127
// Retrieves the color index assigned to each LED mode and function/direction combination, including special colors.
// Notes: Only available if `USE_LED_STRIP` is defined. Entries where `modeIndex == LED_MODE_COUNT` describe special colors.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint8_t modeIndex;  // Index of the LED mode Enum (`ledModeIndex_e`). `LED_MODE_COUNT` for special colors | enum ledModeIndex_e
        uint8_t directionOrSpecialIndex;  // Index of the direction (`ledDirectionId_e`) or special color (`ledSpecialColorIds_e`)
        uint8_t colorIndex;  // Index of the color assigned from `ledStripConfig()->colors`
    } items[51];  // repeat: 51
} mspLedStripModecolorReply_t;
MSP_STATIC_ASSERT(sizeof(mspLedStripModecolorReply_t) == 153, mspLedStripModecolorReply_t_size);

// MSP_BATTERY_STATE (MSPv1) id=130
// Provides battery state information, formatted primarily for DJI FPV Goggles compatibility.
// Notes: Only available if `USE_DJI_HD_OSD` or `USE_MSP_DISPLAYPORT` is defined. Some values are duplicated from `MSP_ANALOG` / `MSP2_INAV_ANALOG` but potentially with different scaling/types.
typedef struct MSP_PACKED {
    uint8_t cellCount;  // Number of battery cells (`getBatteryCellCount()`) | Count
    uint16_t capacity;  // Battery capacity (`currentBatteryProfile->capacity.value`) | mAh
    uint8_t vbatScaled;  // Battery voltage / 10 (`getBatteryVoltage() / 10`) | 0.1V
    uint16_t mAhDrawn;  // Consumed capacity (`getMAhDrawn()`) | mAh
    int16_t amperage;  // Current draw (`getAmperage()`) | 0.01A
    uint8_t batteryState;  // Enum `batteryState_e` Current battery state (`getBatteryState()`, see `BATTERY_STATE_*`)
    uint16_t vbatActual;  // Actual battery voltage (`getBatteryVoltage()`) | 0.01V
} mspBatteryStateReply_t;
MSP_STATIC_ASSERT(sizeof(mspBatteryStateReply_t) == 11, mspBatteryStateReply_t_size);

// MSP_VTXTABLE_POWERLEVEL (MSPv1) id=138
// Retrieves information about a specific VTX power level from the VTX table.
// Notes: Requires `USE_VTX_CONTROL`. Returns error if index is out of bounds. The `powerValue` field is unused.
typedef struct MSP_PACKED {
    uint8_t powerLevelIndex;  // 1-based index of the power level to query
} mspVtxtablePowerlevelRequest_t;
MSP_STATIC_ASSERT(sizeof(mspVtxtablePowerlevelRequest_t) == 1, mspVtxtablePowerlevelRequest_t_size);

// MSP_VTXTABLE_POWERLEVEL (MSPv1) id=138
// Retrieves information about a specific VTX power level from the VTX table.
// Notes: Requires `USE_VTX_CONTROL`. Returns error if index is out of bounds. The `powerValue` field is unused.
typedef struct MSP_PACKED {
    uint8_t powerLevelIndex;  // 1-based index of the returned power level
    uint16_t powerValue;  // Always 0 (Actual power value in mW is not stored/returned via MSP) | always 0
    uint8_t labelLength;  // Length of the power level label string that follows
    char label[];  // Power level label string (e.g., "25", "200"). Length given by previous field
} mspVtxtablePowerlevelReply_t;
// variable length: sizeof(mspVtxtablePowerlevelReply_t) is the fixed header only

// MSP_STATUS_EX (MSPv1) id=150
// Provides extended flight controller status, including CPU load, arming flags, and calibration status, in addition to `MSP_STATUS` fields.
// Notes: Superseded by `MSP2_INAV_STATUS` which provides the full 32-bit `armingFlags` and other enhancements. The `accCalibAxisFlags` field is not present in `MSP2_INAV_STATUS` but is available via `MSP_CALIBRATION_DATA`.
typedef struct MSP_PACKED {
    uint16_t cycleTime;  // Main loop cycle time | µs
    uint16_t i2cErrors;  // I2C errors | Count
    uint16_t sensorStatus;  // Bitmask: Sensor status | bitmask | enum sensors_e
    uint32_t activeModesLow;  // Bitmask: First 32 active modes | bitmask
    uint8_t profile;  // Current config profile index | Index
    uint16_t cpuLoad;  // Average system load percentage (`averageSystemLoadPercent`) | %
    uint16_t armingFlags;  // Bitmask: Flight controller arming flags (`armingFlags`). Note: Truncated to 16 bits | bitmask | enum armingFlag_e
    uint8_t accCalibAxisFlags;  // Bitmask: Accelerometer calibrated axes flags (`accGetCalibrationAxisFlags()`) | bitmask
} mspStatusExReply_t;
MSP_STATIC_ASSERT(sizeof(mspStatusExReply_t) == 16, mspStatusExReply_t_size);

// MSP_SENSOR_STATUS (MSPv1) id=151
// Provides the hardware status for each individual sensor system.
// Notes: Status values map to the `hardwareSensorStatus_e` enum: `HW_SENSOR_NONE`, `HW_SENSOR_OK`, `HW_SENSOR_UNAVAILABLE`, `HW_SENSOR_UNHEALTHY`.
typedef struct MSP_PACKED {
    uint8_t overallHealth;  // 1 if all essential hardware is healthy, 0 otherwise (`isHardwareHealthy()`) | Boolean
    uint8_t gyroStatus;  // Enum `hardwareSensorStatus_e` Gyro hardware status (`getHwGyroStatus()`)
    uint8_t accStatus;  // Enum `hardwareSensorStatus_e` Accelerometer hardware status (`getHwAccelerometerStatus()`)
    uint8_t magStatus;  // Enum `hardwareSensorStatus_e` Compass hardware status (`getHwCompassStatus()`)
    uint8_t baroStatus;  // Enum `hardwareSensorStatus_e` Barometer hardware status (`getHwBarometerStatus()`)
    uint8_t gpsStatus;  // Enum `hardwareSensorStatus_e` GPS hardware status (`getHwGPSStatus()`)
    uint8_t rangefinderStatus;  // Enum `hardwareSensorStatus_e` Rangefinder hardware status (`getHwRangefinderStatus()`)
    uint8_t pitotStatus;  // Enum `hardwareSensorStatus_e` Pitot hardware status (`getHwPitotmeterStatus()`)
    uint8_t opflowStatus;  // Enum `hardwareSensorStatus_e` Optical Flow hardware status (`getHwOpticalFlowStatus()`)
} mspSensorStatusReply_t;
MSP_STATIC_ASSERT(sizeof(mspSensorStatusReply_t) == 9, mspSensorStatusReply_t_size);

// MSP_UID (MSPv1) id=160
// Provides the unique identifier of the microcontroller.
// Notes: Total 12 bytes, representing a 96-bit unique ID.
typedef struct MSP_PACKED {
    uint32_t uid0;  // First 32 bits of the unique ID (`U_ID_0`)
    uint32_t uid1;  // Middle 32 bits of the unique ID (`U_ID_1`)
    uint32_t uid2;  // Last 32 bits of the unique ID (`U_ID_2`)
} mspUidReply_t;
MSP_STATIC_ASSERT(sizeof(mspUidReply_t) == 12, mspUidReply_t_size);

// MSP_GPSSVINFO (MSPv1) id=164
// Provides satellite signal strength information (legacy U-Blox compatibility stub).
// Notes: Requires `USE_GPS`. This is just a stub in INAV and does not provide actual per-satellite signal info. HDOP digits are not formatted correctly: tens and units both contain `gpsSol.hdop / 100`.
typedef struct MSP_PACKED {
    uint8_t protocolVersion;  // Always 1 (Stub version) | always 1
    uint8_t numChannels;  // Always 0 (Number of SV info channels reported) | always 0
    uint8_t hdopHundredsDigit;  // Hundreds digit of HDOP (stub always writes 0)
    uint8_t hdopTensDigit;  // Tens digit of HDOP (`gpsSol.hdop / 100`, truncated)
    uint8_t hdopUnitsDigit;  // Units digit of HDOP (`gpsSol.hdop / 100`, duplicated by stub)
} mspGpssvinfoReply_t;
MSP_STATIC_ASSERT(sizeof(mspGpssvinfoReply_t) == 5, mspGpssvinfoReply_t_size);

// MSP_GPSSTATISTICS (MSPv1) id=166
// Provides debugging statistics for the GPS communication link.
// Notes: Requires `USE_GPS`.
typedef struct MSP_PACKED {
    uint16_t lastMessageDt;  // Time since last valid GPS message (`gpsStats.lastMessageDt`) | ms
    uint32_t errors;  // Number of GPS communication errors (`gpsStats.errors`) | Count
    uint32_t timeouts;  // Number of GPS communication timeouts (`gpsStats.timeouts`) | Count
    uint32_t packetCount;  // Number of valid GPS packets received (`gpsStats.packetCount`) | Count
    uint16_t hdop;  // Horizontal Dilution of Precision (`gpsSol.hdop`) | HDOP * 100
    uint16_t eph;  // Estimated Horizontal Position Accuracy (`gpsSol.eph`) | cm
    uint16_t epv;  // Estimated Vertical Position Accuracy (`gpsSol.epv`) | cm
    uint8_t hwVersion;  // GPS hardware version bit-field: bits[7:6]=series (0b01=u-blox Neo/M), bits[5:0]=generation. E.g. 0x48=M8, 0x49=M9, 0x4A=M10, 0=unknown.
} mspGpsstatisticsReply_t;
MSP_STATIC_ASSERT(sizeof(mspGpsstatisticsReply_t) == 21, mspGpsstatisticsReply_t_size);

// MSP_DISPLAYPORT (MSPv1) id=182
// Drives an external MSP DisplayPort OSD (DJI, HDZero, Walksnail). Sent by the flight controller to the display device rather than requested from it, so it carries a reply payload with no request and expects no response.
// Notes: Requires an MSP DisplayPort OSD device. Sub-commands are emitted by `io/displayport_msp_osd.c`; `MSP_DP_OPTIONS` is reserved and unused by INAV.
typedef struct MSP_PACKED {
    uint8_t subCommand;  // DisplayPort sub-command (`displayportMspCommand_e` in `io/displayport_msp.h`) | enum displayportMspCommand_e
    uint8_t subCommandData[];  // Sub-command payload. Empty for `MSP_DP_HEARTBEAT`, `MSP_DP_RELEASE`, `MSP_DP_CLEAR_SCREEN` and `MSP_DP_DRAW_SCREEN`. For `MSP_DP_WRITE_STRING`: row, column, attributes (font page in bits 0-1, blink in bit 3), then the character bytes.
} mspDisplayportReply_t;
// variable length: sizeof(mspDisplayportReply_t) is the fixed header only

// MSP_SET_TX_INFO (MSPv1) id=186
// Allows a transmitter LUA script (or similar) to send runtime information (currently only RSSI) to the firmware.
// Notes: Calls `setRSSIFromMSP()`. Expects 1 byte.
typedef struct MSP_PACKED {
    uint8_t rssi;  // RSSI value (0-255) provided by the external source; firmware scales it to 10-bit (`value << 2`) | Raw
} mspSetTxInfoRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetTxInfoRequest_t) == 1, mspSetTxInfoRequest_t_size);

// MSP_TX_INFO (MSPv1) id=187
// Provides information potentially useful for transmitter LUA scripts.
// Notes: See `rssiSource_e`.
typedef struct MSP_PACKED {
    uint8_t rssiSource;  // Enum: Source of the RSSI value (`getRSSISource()`, see `rssiSource_e`) | enum rssiSource_e
    uint8_t rtcDateTimeIsSet;  // Boolean: 1 if the RTC has been set, 0 otherwise
} mspTxInfoReply_t;
MSP_STATIC_ASSERT(sizeof(mspTxInfoReply_t) == 2, mspTxInfoReply_t_size);

// MSP_SET_RAW_RC (MSPv1) id=200
// Provides raw RC channel data to the flight controller, typically used when the receiver is connected via MSP (e.g., MSP RX feature).
// Notes: Requires `USE_RX_MSP`. Maximum channels `MAX_SUPPORTED_RC_CHANNEL_COUNT`. Calls `rxMspFrameReceive()`.
// payload is a bare array of rcChannels; no fixed header, so no struct.
// element count = payload_size / sizeof(mspSetRawRcRequestElem_t)
typedef uint16_t mspSetRawRcRequestElem_t;

// MSP_SET_RAW_GPS (MSPv1) id=201
// Provides raw GPS data to the flight controller, typically for simulation or external GPS injection.
// Notes: Requires `USE_GPS`. Expects 14 bytes. Updates `gpsSol` structure and calls `onNewGPSData()`. Note the altitude unit mismatch (meters in MSP, cm internal). Does not provide velocity components.
typedef struct MSP_PACKED {
    uint8_t fixType;  // Enum `gpsFixType_e` GPS fix type
    uint8_t numSat;  // Number of satellites | Count
    int32_t latitude;  // Latitude | deg * 1e7
    int32_t longitude;  // Longitude | deg * 1e7
    uint16_t altitude;  // Altitude in meters (converted to centimeters internally; limited to 0-65535 m) | m
    uint16_t speed;  // Ground speed (`gpsSol.groundSpeed`) | cm/s
} mspSetRawGpsRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetRawGpsRequest_t) == 14, mspSetRawGpsRequest_t_size);

// MSP_SET_RC_TUNING (MSPv1) id=204
// Sets RC tuning parameters (rates, expos, TPA) for the current control rate profile.
// Notes: Expects 10 or 11 bytes. Calls `schedulePidGainsUpdate()`. Superseded by `MSP2_INAV_SET_RATE_PROFILE`.
typedef struct MSP_PACKED {
    uint8_t legacyRcRate;  // Ignored
    uint8_t rcExpo;  // Sets `currentControlRateProfile->stabilized.rcExpo8`
    uint8_t rollRate;  // Sets `currentControlRateProfile->stabilized.rates[FD_ROLL]` (constrained)
    uint8_t pitchRate;  // Sets `currentControlRateProfile->stabilized.rates[FD_PITCH]` (constrained)
    uint8_t yawRate;  // Sets `currentControlRateProfile->stabilized.rates[FD_YAW]` (constrained)
    uint8_t dynamicThrottlePID;  // Sets `currentControlRateProfile->throttle.dynPID` (constrained)
    uint8_t throttleMid;  // Sets `currentControlRateProfile->throttle.rcMid8`
    uint8_t throttleExpo;  // Sets `currentControlRateProfile->throttle.rcExpo8`
    uint16_t tpaBreakpoint;  // Sets `currentControlRateProfile->throttle.tpa_breakpoint`
    uint8_t rcYawExpo;  // (Optional) Sets `currentControlRateProfile->stabilized.rcYawExpo8` | OPTIONAL: may be absent from a shorter payload
} mspSetRcTuningRequest_t;

// MSP_SET_MISC (MSPv1) id=207
// Sets miscellaneous configuration settings (legacy formats/scaling).
// Notes: Expects 22 bytes. Superseded by `MSP2_INAV_SET_MISC`.
typedef struct MSP_PACKED {
    uint16_t midRc;  // Ignored | PWM
    uint16_t legacyMinThrottle;  // Ignored
    uint16_t legacyMaxThrottle;  // Ignored
    uint16_t minCommand;  // Sets `motorConfigMutable()->mincommand` (constrained 0-PWM_RANGE_MAX) | PWM
    uint16_t failsafeThrottle;  // Sets `currentBatteryProfileMutable->failsafe_throttle` (constrained PWM_RANGE_MIN/MAX) | PWM
    uint8_t gpsType;  // Enum `gpsProvider_e` (Sets `gpsConfigMutable()->provider`)
    uint8_t legacyGpsBaud;  // Ignored
    uint8_t gpsSbasMode;  // Enum `sbasMode_e` (Sets `gpsConfigMutable()->sbasMode`)
    uint8_t legacyMwCurrentOut;  // Ignored
    uint8_t rssiChannel;  // Sets `rxConfigMutable()->rssi_channel` (constrained 0-MAX_SUPPORTED_RC_CHANNEL_COUNT). Updates source | Index
    uint8_t reserved1;  // Ignored
    uint16_t magDeclination;  // Sets `compassConfigMutable()->mag_declination = value * 10` (if `USE_MAG`) | 0.1 degrees
    uint8_t vbatScale;  // Sets `batteryMetersConfigMutable()->voltage.scale = value * 10` (if `USE_ADC`) | Scale / 10
    uint8_t vbatMinCell;  // Sets `currentBatteryProfileMutable->voltage.cellMin = value * 10` (if `USE_ADC`) | 0.1V
    uint8_t vbatMaxCell;  // Sets `currentBatteryProfileMutable->voltage.cellMax = value * 10` (if `USE_ADC`) | 0.1V
    uint8_t vbatWarningCell;  // Sets `currentBatteryProfileMutable->voltage.cellWarning = value * 10` (if `USE_ADC`) | 0.1V
} mspSetMiscRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetMiscRequest_t) == 22, mspSetMiscRequest_t_size);

// MSP_SET_WP (MSPv1) id=209
// Sets a single waypoint in the mission plan.
// Notes: Expects 21 bytes. Calls `setWaypoint()`. If `USE_FW_AUTOLAND` is enabled, this also interacts with autoland approach settings based on waypoint index and flags.
typedef struct MSP_PACKED {
    uint8_t waypointIndex;  // Index of the waypoint to set (0 to `NAV_MAX_WAYPOINTS - 1`) | Index
    uint8_t action;  // Enum `navWaypointActions_e` Waypoint action type
    int32_t latitude;  // Latitude coordinate | deg * 1e7
    int32_t longitude;  // Longitude coordinate | deg * 1e7
    int32_t altitude;  // Altitude coordinate | cm
    uint16_t param1;  // Parameter 1 | Varies
    uint16_t param2;  // Parameter 2 | Varies
    uint16_t param3;  // Parameter 3 | Varies
    uint8_t flag;  // Bitmask: Waypoint flags (`navWaypointFlags_e`) | bitmask | enum navWaypointFlags_e
} mspSetWpRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetWpRequest_t) == 21, mspSetWpRequest_t_size);

// MSP_SELECT_SETTING (MSPv1) id=210
// Selects the active configuration profile and saves it.
// Notes: Will fail if armed. Calls `setConfigProfileAndWriteEEPROM()`.
typedef struct MSP_PACKED {
    uint8_t profileIndex;  // Index of the profile to activate (0-based)
} mspSelectSettingRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSelectSettingRequest_t) == 1, mspSelectSettingRequest_t_size);

// MSP_SET_HEAD (MSPv1) id=211
// Sets the target heading for the heading hold controller (e.g., during MAG mode).
// Notes: Expects 2 bytes. Calls `updateHeadingHoldTarget()`. Also synchronizes navigation yaw targets (including cruise/course) when NAV is controlling yaw.
typedef struct MSP_PACKED {
    uint16_t heading;  // Target heading (0-359) | degrees
} mspSetHeadRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetHeadRequest_t) == 2, mspSetHeadRequest_t_size);

// MSP_SET_SERVO_CONFIGURATION (MSPv1) id=212
// Sets the configuration for a single servo (legacy format).
// Notes: Expects 15 bytes. Returns error if index is invalid. Calls `servoComputeScalingFactors()`. Superseded by `MSP2_INAV_SET_SERVO_CONFIG`.
typedef struct MSP_PACKED {
    uint8_t servoIndex;  // Index of the servo to configure (0 to `MAX_SUPPORTED_SERVOS - 1`) | Index
    uint16_t min;  // Minimum servo endpoint | PWM
    uint16_t max;  // Maximum servo endpoint | PWM
    uint16_t middle;  // Middle/Neutral servo position | PWM
    uint8_t rate;  // Servo rate/scaling | %
    uint8_t reserved1;  // Ignored
    uint8_t reserved2;  // Ignored
    uint8_t legacyForwardChan;  // Ignored
    uint32_t legacyReversedSources;  // Ignored
} mspSetServoConfigurationRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetServoConfigurationRequest_t) == 15, mspSetServoConfigurationRequest_t_size);

// MSP_SET_MOTOR (MSPv1) id=214
// Sets the disarmed motor values, typically used for motor testing or propeller balancing functions in a configurator.
// Notes: Expects 16 bytes. Modifies the `motor_disarmed` array. These values are *not* saved persistently.
typedef struct MSP_PACKED {
    uint16_t motorValues[8];  // Array of motor values to set when disarmed. Only affects first `MAX_SUPPORTED_MOTORS` entries | PWM
} mspSetMotorRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetMotorRequest_t) == 16, mspSetMotorRequest_t_size);

// MSP_SET_3D (MSPv1) id=217
// Sets parameters related to 3D/reversible motor operation.
// Notes: Expects 6 bytes. Requires reversible motor support.
typedef struct MSP_PACKED {
    uint16_t deadbandLow;  // Sets `reversibleMotorsConfigMutable()->deadband_low` | PWM
    uint16_t deadbandHigh;  // Sets `reversibleMotorsConfigMutable()->deadband_high` | PWM
    uint16_t neutral;  // Sets `reversibleMotorsConfigMutable()->neutral` | PWM
} mspSet3dRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSet3dRequest_t) == 6, mspSet3dRequest_t_size);

// MSP_SET_RC_DEADBAND (MSPv1) id=218
// Sets RC input deadband values.
// Notes: Expects 5 bytes.
typedef struct MSP_PACKED {
    uint8_t deadband;  // Sets `rcControlsConfigMutable()->deadband` | PWM
    uint8_t yawDeadband;  // Sets `rcControlsConfigMutable()->yaw_deadband` | PWM
    uint8_t altHoldDeadband;  // Sets `rcControlsConfigMutable()->alt_hold_deadband` | PWM
    uint16_t throttleDeadband;  // Sets `rcControlsConfigMutable()->mid_throttle_deadband` | PWM
} mspSetRcDeadbandRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetRcDeadbandRequest_t) == 5, mspSetRcDeadbandRequest_t_size);

// MSP_SET_SENSOR_ALIGNMENT (MSPv1) id=220
// Sets sensor alignment (legacy format).
// Notes: Expects 4 bytes. Use `MSP_SET_BOARD_ALIGNMENT` for primary board orientation.
typedef struct MSP_PACKED {
    uint8_t gyroAlign;  // Ignored
    uint8_t accAlign;  // Ignored
    uint8_t magAlign;  // Sets `compassConfigMutable()->mag_align` (if `USE_MAG`)
    uint8_t opflowAlign;  // Sets `opticalFlowConfigMutable()->opflow_align` (if `USE_OPFLOW`)
} mspSetSensorAlignmentRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetSensorAlignmentRequest_t) == 4, mspSetSensorAlignmentRequest_t_size);

// MSP_SET_LED_STRIP_MODECOLOR (MSPv1) id=221
// Sets the color index for a specific LED mode/function combination.
// Notes: Only available if `USE_LED_STRIP` is defined. Expects 3 bytes. Returns error if setting fails (invalid index).
typedef struct MSP_PACKED {
    uint8_t modeIndex;  // Index of the LED mode (`ledModeIndex_e` or `LED_MODE_COUNT` for special) | enum ledModeIndex_e
    uint8_t directionOrSpecialIndex;  // Index of the direction (`ledDirectionId_e`) or special color (`ledSpecialColorIds_e`)
    uint8_t colorIndex;  // Index of the color to assign from `ledStripConfig()->colors`
} mspSetLedStripModecolorRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetLedStripModecolorRequest_t) == 3, mspSetLedStripModecolorRequest_t_size);

// MSP_SERVO_MIX_RULES (MSPv1) id=241
// Retrieves the custom servo mixer rules (legacy format).
// Notes: Superseded by `MSP2_INAV_SERVO_MIXER`.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint8_t targetChannel;  // Servo output channel index (0-based) | Index
        uint8_t inputSource;  // Enum `inputSource_e` Input source for the mix (RC chan, Roll, Pitch...)
        int16_t rate;  // Mixing rate/weight (`-1000` to `+1000`, percent with sign) | %
        uint8_t speed;  // Speed/Slew rate limit (`0`=instant, higher slows response) | 0-255
        uint8_t reserved1;  // Always 0 | always 0
        uint8_t legacyMax;  // Always 100 (Legacy) | always 100
        uint8_t legacyBox;  // Always 0 (Legacy) | always 0
    } items[MAX_SERVO_RULES];  // repeat: MAX_SERVO_RULES
} mspServoMixRulesReply_t;
MSP_STATIC_ASSERT(sizeof(mspServoMixRulesReply_t) == 288, mspServoMixRulesReply_t_size);

// MSP_SET_SERVO_MIX_RULE (MSPv1) id=242
// Sets a single custom servo mixer rule (legacy format).
// Notes: Expects 9 bytes. Returns error if index invalid. Calls `loadCustomServoMixer()`. Superseded by `MSP2_INAV_SET_SERVO_MIXER`.
typedef struct MSP_PACKED {
    uint8_t ruleIndex;  // Index of the rule to set (0 to `MAX_SERVO_RULES - 1`) | Index
    uint8_t targetChannel;  // Servo output channel index | Index
    uint8_t inputSource;  // Enum `inputSource_e` Input source for the mix
    int16_t rate;  // Mixing rate/weight (`-1000` to `+1000`, percent with sign) | %
    uint8_t speed;  // Speed/Slew rate limit (`0`=instant, higher slows response) | 0-255
    uint16_t legacyMinMax;  // Ignored
    uint8_t legacyBox;  // Ignored
} mspSetServoMixRuleRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetServoMixRuleRequest_t) == 9, mspSetServoMixRuleRequest_t_size);

// MSP_SET_PASSTHROUGH (MSPv1) id=245
// Enables serial passthrough mode to peripherals like ESCs (BLHeli 4-way) or other serial devices.
// Notes: Accepts 0 bytes (defaults to ESC 4-way) or up to 2 bytes for mode/argument. If successful, sets `mspPostProcessFn` to the appropriate handler (`mspSerialPassthroughFn` or `esc4wayProcess`). This handler takes over the serial port after the reply is sent. Requires `USE_SERIAL_4WAY_BLHELI_INTERFACE` for ESC passthrough. ESC 4-way passthrough will fail if the craft is armed: the reply carries `MSP_RESULT_ERROR` and the serial port is not handed over. Serial passthrough to another device is unaffected.
typedef struct MSP_PACKED {
    uint8_t status;  // 1 if passthrough started successfully, 0 on error (e.g., port not found). For 4way, returns number of ESCs found
} mspSetPassthroughReply_t;
MSP_STATIC_ASSERT(sizeof(mspSetPassthroughReply_t) == 1, mspSetPassthroughReply_t_size);

// MSP_RTC (MSPv1) id=246
// Retrieves the current Real-Time Clock time.
// Notes: Requires RTC hardware/support. Returns (0, 0) if time is not available/set.
typedef struct MSP_PACKED {
    int32_t seconds;  // Seconds since epoch (or relative time if not set). 0 if RTC time unknown | Seconds
    uint16_t millis;  // Millisecond part of the time. 0 if RTC time unknown | Milliseconds
} mspRtcReply_t;
MSP_STATIC_ASSERT(sizeof(mspRtcReply_t) == 6, mspRtcReply_t_size);

// MSP_SET_RTC (MSPv1) id=247
// Sets the Real-Time Clock time.
// Notes: Requires RTC hardware/support. Expects 6 bytes. Uses `rtcSet()`.
typedef struct MSP_PACKED {
    int32_t seconds;  // Seconds component of time to set | Seconds
    uint16_t millis;  // Millisecond component of time to set | Milliseconds
} mspSetRtcRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSetRtcRequest_t) == 6, mspSetRtcRequest_t_size);

// MSP_DEBUGMSG (MSPv1) id=253
// Retrieves debug ("serial printf") messages from the firmware.
// Notes: Published via the LOG UART or shared MSP/LOG port using `mspSerialPushPort()`.
// payload is a bare array of messageText; no fixed header, so no struct.
// element count = payload_size / sizeof(mspDebugmsgReplyElem_t)
typedef char mspDebugmsgReplyElem_t;

// MSP_DEBUG (MSPv1) id=254
// Retrieves values from the firmware's `debug[]` array (legacy 16-bit version).
// Notes: Useful for developers. Values are truncated to the lower 16 bits of each `debug[]` entry. See `MSP2_INAV_DEBUG` for full 32-bit values.
typedef struct MSP_PACKED {
    uint16_t debugValues[4];  // First 4 values from the `debug` array
} mspDebugReply_t;
MSP_STATIC_ASSERT(sizeof(mspDebugReply_t) == 8, mspDebugReply_t_size);

// MSP2_COMMON_TZ (MSPv2) id=4097
// Gets the time zone offset configuration.
typedef struct MSP_PACKED {
    int16_t tzOffsetMinutes;  // Time zone offset from UTC (`timeConfig()->tz_offset`) | Minutes
    uint8_t tzAutoDst;  // Automatic daylight saving time enabled (`timeConfig()->tz_automatic_dst`) | Boolean
} msp2CommonTzReply_t;
MSP_STATIC_ASSERT(sizeof(msp2CommonTzReply_t) == 3, msp2CommonTzReply_t_size);

// MSP2_COMMON_SET_TZ (MSPv2) id=4098
// Sets the time zone offset configuration.
// Notes: Accepts 2 or 3 bytes.
typedef struct MSP_PACKED {
    int16_t tz_offset;  // Timezone offset from UTC. | minutes
} msp2CommonSetTz_dataSize_eq_2Request_t;
MSP_STATIC_ASSERT(sizeof(msp2CommonSetTz_dataSize_eq_2Request_t) == 2, msp2CommonSetTz_dataSize_eq_2Request_t_size);

// MSP2_COMMON_SET_TZ (MSPv2) id=4098
// Sets the time zone offset configuration.
// Notes: Accepts 2 or 3 bytes.
typedef struct MSP_PACKED {
    int16_t tz_offset;  // Timezone offset from UTC. | minutes
    uint8_t tz_automatic_dst;  // Automatic DST enable (0/1). | bool
} msp2CommonSetTz_dataSize_eq_3Request_t;
MSP_STATIC_ASSERT(sizeof(msp2CommonSetTz_dataSize_eq_3Request_t) == 3, msp2CommonSetTz_dataSize_eq_3Request_t_size);

// msp2CommonSettingRequest_t: NOT GENERATED
//   msp2CommonSettingRequest_t.settingIdentifier: field has no type
//   This payload needs a hand-written codec.

// MSP2_COMMON_SETTING (MSPv2) id=4099
// Gets the value of a specific configuration setting, identified by name or index.
// Notes: Returns error if setting not found. Use `MSP2_COMMON_SETTING_INFO` to discover settings, types, and sizes.
// payload is a bare array of settingValue; no fixed header, so no struct.
// element count = payload_size / sizeof(msp2CommonSettingReplyElem_t)
typedef uint8_t msp2CommonSettingReplyElem_t;

// msp2CommonSetSettingRequest_t: NOT GENERATED
//   msp2CommonSetSettingRequest_t.settingIdentifier: field has no type
//   This payload needs a hand-written codec.

// MSP2_COMMON_MOTOR_MIXER (MSPv2) id=4101
// Retrieves the current motor mixer configuration (throttle, roll, pitch, yaw weights) for each motor.
// Notes: Scaling is `(float_weight + 2.0) * 1000`. `primaryMotorMixer()` provides the data. If multiple mixer profiles are enabled (`MAX_MIXER_PROFILE_COUNT > 1`), an additional block of mixes for the next profile follows immediately.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint16_t motorMix[4];  // Weights for a single motor `[throttle, roll, pitch, yaw]`, each encoded as `(mix + 2.0) * 1000` (range 0-4000) | Scaled (0-4000)
    } items[MAX_SUPPORTED_MOTORS];  // repeat: MAX_SUPPORTED_MOTORS
} msp2CommonMotorMixerReply_t;

// MSP2_COMMON_SET_MOTOR_MIXER (MSPv2) id=4102
// Sets the motor mixer weights for a single motor in the primary mixer profile.
// Notes: Expects 9 bytes. Modifies `primaryMotorMixerMutable()`. Returns error if index is invalid.
typedef struct MSP_PACKED {
    uint8_t motorIndex;  // Index of the motor to configure (0 to `MAX_SUPPORTED_MOTORS - 1`) | Index
    uint16_t throttleWeight;  // Sets throttle weight from `(value / 1000.0) - 2.0 | Scaled (0-4000)
    uint16_t rollWeight;  // Sets roll weight from `(value / 1000.0) - 2.0 | Scaled (0-4000)
    uint16_t pitchWeight;  // Sets pitch weight from `(value / 1000.0) - 2.0 | Scaled (0-4000)
    uint16_t yawWeight;  // Sets yaw weight from `(value / 1000.0) - 2.0 | Scaled (0-4000)
} msp2CommonSetMotorMixerRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2CommonSetMotorMixerRequest_t) == 9, msp2CommonSetMotorMixerRequest_t_size);

// msp2CommonSettingInfoReply_t: NOT GENERATED
//   msp2CommonSettingInfoReply_t.settingName: 'cstring' is variable-length but is not the final field, so this payload cannot be one C struct
//   This payload needs a hand-written codec.

// MSP2_COMMON_PG_LIST (MSPv2) id=4104
// Gets a list of Parameter Group Numbers (PGNs) used by settings, along with the start and end setting indexes for each group. Can request info for a single PGN.
// Notes: Allows efficient fetching of related settings by group. Record count is not a constant: mspParameterGroupsCommand(): caller may request one PGN or the full PG_ID_FIRST..PG_ID_LAST range, skipping absent groups; read until the payload is exhausted.
typedef struct MSP_PACKED {
    uint16_t pgn;  // (Optional) PGN ID to query. If omitted, returns all used PGNs | OPTIONAL: may be absent from a shorter payload
} msp2CommonPgListRequest_t;

// MSP2_COMMON_PG_LIST (MSPv2) id=4104
// Gets a list of Parameter Group Numbers (PGNs) used by settings, along with the start and end setting indexes for each group. Can request info for a single PGN.
// Notes: Allows efficient fetching of related settings by group. Record count is not a constant: mspParameterGroupsCommand(): caller may request one PGN or the full PG_ID_FIRST..PG_ID_LAST range, skipping absent groups; read until the payload is exhausted.
// payload is a bare sequence of records (repeat: until_end); no fixed
// header, so no wrapper struct. count = payload_size / sizeof(msp2CommonPgListReplyElem_t)
typedef struct MSP_PACKED {
    uint16_t pgn;  // Parameter Group Number (PGN) ID
    uint16_t startIndex;  // Absolute index of the first setting in this group
    uint16_t endIndex;  // Absolute index of the last setting in this group
} msp2CommonPgListReplyElem_t;
MSP_STATIC_ASSERT(sizeof(msp2CommonPgListReplyElem_t) == 6, msp2CommonPgListReplyElem_t_size);

// MSP2_COMMON_SERIAL_CONFIG (MSPv2) id=4105
// Retrieves the configuration for all available serial ports.
// Notes: Baud rate indexes map to actual baud rates (e.g., 9600, 115200). See `baudRates` array. Record count is not a constant: loops SERIAL_PORT_COUNT, emitting only ports where serialIsPortAvailable(); read until the payload is exhausted.
// payload is a bare sequence of records (repeat: until_end); no fixed
// header, so no wrapper struct. count = payload_size / sizeof(msp2CommonSerialConfigReplyElem_t)
typedef struct MSP_PACKED {
    int8_t identifier;  // Port identifier Enum (`serialPortIdentifier_e`) | enum serialPortIdentifier_e
    uint32_t functionMask;  // Bitmask: enabled functions (`FUNCTION_*`) | bitmask | enum serialPortFunction_e
    uint8_t mspBaudIndex;  // Baud rate index for MSP function
    uint8_t gpsBaudIndex;  // Baud rate index for GPS function
    uint8_t telemetryBaudIndex;  // Baud rate index for Telemetry function
    uint8_t peripheralBaudIndex;  // Baud rate index for other peripheral functions
} msp2CommonSerialConfigReplyElem_t;
MSP_STATIC_ASSERT(sizeof(msp2CommonSerialConfigReplyElem_t) == 9, msp2CommonSerialConfigReplyElem_t_size);

// MSP2_COMMON_SET_SERIAL_CONFIG (MSPv2) id=4106
// Sets the configuration for one or more serial ports.
// Notes: Payload size must be a multiple of the size of one port config entry (1 + 4 + 4 = 9 bytes). Returns error if identifier is invalid or size is incorrect. Baud rate indexes are constrained `BAUD_MIN` to `BAUD_MAX`. Record count is not a constant: mirrors MSP2_COMMON_SERIAL_CONFIG; the sender chooses how many ports to configure; read until the payload is exhausted.
// payload is a bare sequence of records (repeat: until_end); no fixed
// header, so no wrapper struct. count = payload_size / sizeof(msp2CommonSetSerialConfigRequestElem_t)
typedef struct MSP_PACKED {
    int8_t identifier;  // Port identifier Enum (`serialPortIdentifier_e`) | enum serialPortIdentifier_e
    uint32_t functionMask;  // Bitmask: functions to enable | bitmask | enum serialPortFunction_e
    uint8_t mspBaudIndex;  // Baud rate index for MSP
    uint8_t gpsBaudIndex;  // Baud rate index for GPS
    uint8_t telemetryBaudIndex;  // Baud rate index for Telemetry
    uint8_t peripheralBaudIndex;  // Baud rate index for peripherals
} msp2CommonSetSerialConfigRequestElem_t;
MSP_STATIC_ASSERT(sizeof(msp2CommonSetSerialConfigRequestElem_t) == 9, msp2CommonSetSerialConfigRequestElem_t_size);

// MSP2_COMMON_SET_RADAR_POS (MSPv2) id=4107
// Sets the position and status information for a "radar" Point of Interest (POI). Used for displaying other craft/objects on the OSD map.
// Notes: Expects 19 bytes. POI index is clamped to `RADAR_MAX_POIS - 1`. Updates the `radar_pois` array.
typedef struct MSP_PACKED {
    uint8_t poiIndex;  // Index of the POI slot (0 to `RADAR_MAX_POIS - 1`) | Index
    uint8_t state;  // Status of the POI (0=undefined, 1=armed, 2=lost)
    int32_t latitude;  // Latitude of the POI | deg * 1e7
    int32_t longitude;  // Longitude of the POI | deg * 1e7
    int32_t altitude;  // Altitude of the POI | cm
    uint16_t heading;  // Heading of the POI | degrees
    uint16_t speed;  // Speed of the POI | cm/s
    uint8_t linkQuality;  // Link quality indicator | 0-4
} msp2CommonSetRadarPosRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2CommonSetRadarPosRequest_t) == 19, msp2CommonSetRadarPosRequest_t_size);

// MSP2_COMMON_SET_MSP_RC_LINK_STATS (MSPv2) id=4109
// Provides RC link statistics (RSSI, LQ) to the FC, typically from an MSP-based RC link (like ExpressLRS). Sent periodically by the RC link.
// Notes: Requires `USE_RX_MSP`. Expects at least 7 bytes. Updates `rxLinkStatistics` and sets RSSI via `setRSSIFromMSP_RC()` only if `sublinkID` is 0. This message expects **no reply** (`MSP_RESULT_NO_REPLY`).
typedef struct MSP_PACKED {
    uint8_t sublinkID;  // Sublink identifier (usually 0)
    uint8_t validLink;  // Indicates if the link is currently valid (not in failsafe) | Boolean
    uint8_t rssiPercent;  // Uplink RSSI percentage (0-100) | %
    uint8_t uplinkRSSI_dBm;  // Uplink RSSI in dBm (sent as positive, e.g., 70 means -70dBm) | -dBm
    uint8_t downlinkLQ;  // Downlink Link Quality (0-100) | %
    uint8_t uplinkLQ;  // Uplink Link Quality (0-100) | %
    int8_t uplinkSNR;  // Uplink Signal-to-Noise Ratio | dB
} msp2CommonSetMspRcLinkStatsRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2CommonSetMspRcLinkStatsRequest_t) == 7, msp2CommonSetMspRcLinkStatsRequest_t_size);

// MSP2_COMMON_SET_MSP_RC_INFO (MSPv2) id=4110
// Provides additional RC link information (power levels, band, mode) to the FC from an MSP-based RC link. Sent less frequently than link stats.
// Notes: Requires `USE_RX_MSP`. Expects at least 15 bytes. Updates `rxLinkStatistics` only if `sublinkID` is 0. Converts band/mode strings to uppercase. This message expects **no reply** (`MSP_RESULT_NO_REPLY`).
typedef struct MSP_PACKED {
    uint8_t sublinkID;  // Sublink identifier (usually 0)
    uint16_t uplinkTxPower;  // Uplink transmitter power level | mW
    uint16_t downlinkTxPower;  // Downlink transmitter power level | mW
    char band[4];  // Operating band string (e.g., "2G4", "900"), null-terminated/padded
    char mode[6];  // Operating mode/rate string (e.g., "100HZ", "F1000"), null-terminated/padded
} msp2CommonSetMspRcInfoRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2CommonSetMspRcInfoRequest_t) == 15, msp2CommonSetMspRcInfoRequest_t_size);

// MSP2_COMMON_GET_RADAR_GPS (MSPv2) id=4111
// Provides the GPS positions (latitude, longitude, altitude) for each radar point of interest.
// Notes: Returns the stored GPS coordinates for all radar POIs (`radar_pois[i].gps`).
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        int32_t poiLatitude;  // Latitude of a radar POI | deg * 1e7
        int32_t poiLongitude;  // Longitude of a radar POI | deg * 1e7
        int32_t poiAltitude;  // Altitude of a radar POI | cm
    } items[RADAR_MAX_POIS];  // repeat: RADAR_MAX_POIS
} msp2CommonGetRadarGpsReply_t;
MSP_STATIC_ASSERT(sizeof(msp2CommonGetRadarGpsReply_t) == 60, msp2CommonGetRadarGpsReply_t_size);

// MSP2_SENSOR_RANGEFINDER (MSPv2) id=7937
// Provides rangefinder data (distance, quality) from an external MSP-based sensor.
// Notes: Requires `USE_RANGEFINDER_MSP`. Calls `mspRangefinderReceiveNewData()`.
typedef struct MSP_PACKED {
    uint8_t quality;  // Quality of the measurement | 0-255
    int32_t distanceMm;  // Measured distance. Negative value indicates out of range | mm
} msp2SensorRangefinderRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2SensorRangefinderRequest_t) == 5, msp2SensorRangefinderRequest_t_size);

// MSP2_SENSOR_OPTIC_FLOW (MSPv2) id=7938
// Provides optical flow data (motion, quality) from an external MSP-based sensor.
// Notes: Requires `USE_OPFLOW_MSP`. Calls `mspOpflowReceiveNewData()`.
typedef struct MSP_PACKED {
    uint8_t quality;  // Quality of the measurement (0-255)
    int32_t motionX;  // Raw integrated flow value X
    int32_t motionY;  // Raw integrated flow value Y
} msp2SensorOpticFlowRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2SensorOpticFlowRequest_t) == 9, msp2SensorOpticFlowRequest_t_size);

// MSP2_SENSOR_GPS (MSPv2) id=7939
// Provides detailed GPS data from an external MSP-based GPS module.
// Notes: Requires `USE_GPS_PROTO_MSP`. Calls `mspGPSReceiveNewData()`.
typedef struct MSP_PACKED {
    uint8_t instance;  // Sensor instance number (for multi-GPS)
    uint16_t gpsWeek;  // GPS week number (0xFFFF if unavailable)
    uint32_t msTOW;  // Milliseconds Time of Week | ms
    uint8_t fixType;  // Enum `gpsFixType_e` Type of GPS fix
    uint8_t satellitesInView;  // Number of satellites used in solution | Count
    uint16_t hPosAccuracy;  // Horizontal position accuracy estimate in milimeters | mm
    uint16_t vPosAccuracy;  // Vertical position accuracy estimate in milimeters | mm
    uint16_t hVelAccuracy;  // Horizontal velocity accuracy estimate | cm/s
    uint16_t hdop;  // Horizontal Dilution of Precision | HDOP * 100
    int32_t longitude;  // Longitude | deg * 1e7
    int32_t latitude;  // Latitude | deg * 1e7
    int32_t mslAltitude;  // Altitude above Mean Sea Level | cm
    int32_t nedVelNorth;  // North velocity (NED frame) | cm/s
    int32_t nedVelEast;  // East velocity (NED frame) | cm/s
    int32_t nedVelDown;  // Down velocity (NED frame) | cm/s
    uint16_t groundCourse;  // Ground course (0-36000) | deg * 100
    uint16_t trueYaw;  // True heading/yaw (0-36000, 65535 if unavailable) | deg * 100
    uint16_t year;  // Year (e.g., 2023)
    uint8_t month;  // Month (1-12)
    uint8_t day;  // Day of month (1-31)
    uint8_t hour;  // Hour (0-23)
    uint8_t min;  // Minute (0-59)
    uint8_t sec;  // Second (0-59)
} msp2SensorGpsRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2SensorGpsRequest_t) == 52, msp2SensorGpsRequest_t_size);

// MSP2_SENSOR_COMPASS (MSPv2) id=7940
// Provides magnetometer data from an external MSP-based compass module.
// Notes: Requires `USE_MAG_MSP`. Calls `mspMagReceiveNewData()`.
typedef struct MSP_PACKED {
    uint8_t instance;  // Sensor instance number
    uint32_t timeMs;  // Timestamp from the sensor | ms
    int16_t magX;  // Front component reading | mGauss
    int16_t magY;  // Right component reading | mGauss
    int16_t magZ;  // Down component reading | mGauss
} msp2SensorCompassRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2SensorCompassRequest_t) == 11, msp2SensorCompassRequest_t_size);

// MSP2_SENSOR_BAROMETER (MSPv2) id=7941
// Provides barometer data from an external MSP-based barometer module.
// Notes: Requires `USE_BARO_MSP`. Calls `mspBaroReceiveNewData()`.
typedef struct MSP_PACKED {
    uint8_t instance;  // Sensor instance number
    uint32_t timeMs;  // Timestamp from the sensor | ms
    float pressurePa;  // Absolute pressure | Pa
    int16_t temp;  // Temperature | 0.01 deg C
} msp2SensorBarometerRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2SensorBarometerRequest_t) == 11, msp2SensorBarometerRequest_t_size);

// MSP2_SENSOR_AIRSPEED (MSPv2) id=7942
// Provides airspeed data from an external MSP-based pitot sensor module.
// Notes: Requires `USE_PITOT_MSP`. Calls `mspPitotmeterReceiveNewData()`.
typedef struct MSP_PACKED {
    uint8_t instance;  // Sensor instance number
    uint32_t timeMs;  // Timestamp from the sensor | ms
    float diffPressurePa;  // Differential pressure | Pa
    int16_t temp;  // Temperature | 0.01 deg C
} msp2SensorAirspeedRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2SensorAirspeedRequest_t) == 11, msp2SensorAirspeedRequest_t_size);

// MSP2_SENSOR_HEADTRACKER (MSPv2) id=7943
// Provides head tracker orientation data.
// Notes: Requires `USE_HEADTRACKER` and `USE_HEADTRACKER_MSP`. Calls `mspHeadTrackerReceiverNewData()`, which rejects any payload whose size is not exactly `sizeof(headtrackerMspMessage_t)` (9 bytes). Layout matches `headtrackerMspMessage_t` in `io/headtracker_msp.h`. `pan`, `tilt` and `roll` are constrained to `HEADTRACKER_RANGE_MIN`..`HEADTRACKER_RANGE_MAX` on receipt.
typedef struct MSP_PACKED {
    uint8_t version;  // Message version. Currently 0.
    int16_t pan;  // -2048~2047. Scale is min/max angle for gimbal
    int16_t tilt;  // -2048~2047. Scale is min/max angle for gimbal
    int16_t roll;  // -2048~2047. Scale is min/max angle for gimbal
    int16_t sensitivity;  // -16~15. Scale is min/max angle for gimbal
} msp2SensorHeadtrackerRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2SensorHeadtrackerRequest_t) == 9, msp2SensorHeadtrackerRequest_t_size);

// MSP2_INAV_STATUS (MSPv2) id=8192
// Provides comprehensive flight controller status, extending `MSP_STATUS_EX` with full arming flags, battery profile, and mixer profile.
// Notes: `sensorStatus` bits follow `packSensorStatus()` (bit 15 indicates hardware failure). `profileAndBattProfile` packs the current config profile in the low nibble and the battery profile in the high nibble. `activeModes` is emitted as a little-endian array of 32-bit words sized to `CHECKBOX_ITEM_COUNT`.
typedef struct MSP_PACKED {
    uint16_t cycleTime;  // Main loop cycle time | µs
    uint16_t i2cErrors;  // I2C errors | Count
    uint16_t sensorStatus;  // Bitmask: Sensor status | bitmask | enum sensors_e
    uint16_t cpuLoad;  // Average system load percentage | %
    uint8_t profileAndBattProfile;  // Bits 0-3: Config profile index (`getConfigProfile()`), Bits 4-7: Battery profile index (`getConfigBatteryProfile()`) | Packed
    uint32_t armingFlags;  // Bitmask: Full 32-bit flight controller arming flags (`armingFlags`) | bitmask | enum armingFlag_e
    boxBitmask_t activeModes;  // Bitmask words for active flight modes (`packBoxModeFlags()`) | bitmask
    uint8_t mixerProfile;  // Current mixer profile index (`getConfigMixerProfile()`) | Index
} msp2InavStatusReply_t;

// MSP2_INAV_OPTICAL_FLOW (MSPv2) id=8193
// Provides data from the optical flow sensor.
// Notes: Requires `USE_OPFLOW`.
typedef struct MSP_PACKED {
    uint8_t quality;  // Raw quality indicator from the sensor (`opflow.rawQuality`). 0 if `USE_OPFLOW` disabled | 0-255
    int16_t flowRateX;  // Optical flow rate X (roll axis) (`RADIANS_TO_DEGREES(opflow.flowRate[X])`). 0 if `USE_OPFLOW` disabled | degrees/s
    int16_t flowRateY;  // Optical flow rate Y (pitch axis) (`RADIANS_TO_DEGREES(opflow.flowRate[Y])`). 0 if `USE_OPFLOW` disabled | degrees/s
    int16_t bodyRateX;  // Compensated body rate X (roll axis) (`RADIANS_TO_DEGREES(opflow.bodyRate[X])`). 0 if `USE_OPFLOW` disabled | degrees/s
    int16_t bodyRateY;  // Compensated body rate Y (pitch axis) (`RADIANS_TO_DEGREES(opflow.bodyRate[Y])`). 0 if `USE_OPFLOW` disabled | degrees/s
} msp2InavOpticalFlowReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOpticalFlowReply_t) == 9, msp2InavOpticalFlowReply_t_size);

// MSP2_INAV_ANALOG (MSPv2) id=8194
// Provides detailed analog sensor readings, superseding `MSP_ANALOG` with higher precision and additional fields.
// Notes: Requires `USE_CURRENT_METER`/`USE_ADC` for current-related fields; values fall back to zero when unavailable. Capacity fields are reported in the units configured by `batteryMetersConfig()->capacity_unit` (mAh or mWh).
typedef struct MSP_PACKED {
    uint8_t batteryFlags;  // Bitmask: Bit0=Full on plug-in, Bit1=Use capacity thresholds, Bits2-3=`batteryState_e` (`getBatteryState()`), Bits4-7=Cell count (`getBatteryCellCount()`) | bitmask
    uint16_t vbat;  // Battery voltage (`getBatteryVoltage()`) | 0.01V
    int16_t amperage;  // Current draw (`getAmperage()`) | 0.01A
    uint32_t powerDraw;  // Power draw (`getPower()`) | 0.01W
    uint32_t mAhDrawn;  // Consumed capacity (`getMAhDrawn()`) | mAh
    uint32_t mWhDrawn;  // Consumed energy (`getMWhDrawn()`) | mWh
    uint32_t remainingCapacity;  // Estimated remaining capacity (`getBatteryRemainingCapacity()`) | Capacity unit (`batteryMetersConfig()->capacity_unit`)
    uint8_t percentageRemaining;  // Estimated remaining capacity percentage (`calculateBatteryPercentage()`) | %
    uint16_t rssi;  // RSSI value (`getRSSI()`) | Raw (0-1023)
} msp2InavAnalogReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavAnalogReply_t) == 24, msp2InavAnalogReply_t_size);

// MSP2_INAV_MISC (MSPv2) id=8195
// Retrieves miscellaneous configuration settings, superseding `MSP_MISC` with higher precision and capacity fields.
typedef struct MSP_PACKED {
    uint16_t midRc;  // Mid RC value (`PWM_RANGE_MIDDLE`) | PWM
    uint16_t legacyMinThrottle;  // Always 0 (Legacy) | always 0
    uint16_t maxThrottle;  // Maximum throttle command (`getMaxThrottle()`) | PWM
    uint16_t minCommand;  // Minimum motor command (`motorConfig()->mincommand`) | PWM
    uint16_t failsafeThrottle;  // Failsafe throttle level (`currentBatteryProfile->failsafe_throttle`) | PWM
    uint8_t gpsType;  // Enum `gpsProvider_e` GPS provider type (`gpsConfig()->provider`). 0 if `USE_GPS` disabled
    uint8_t legacyGpsBaud;  // Always 0 (Legacy) | always 0
    uint8_t gpsSbasMode;  // Enum `sbasMode_e` GPS SBAS mode (`gpsConfig()->sbasMode`). 0 if `USE_GPS` disabled
    uint8_t rssiChannel;  // RSSI channel index (1-based, 0 disables) (`rxConfig()->rssi_channel`) | Index
    int16_t magDeclination;  // Magnetic declination / 10 (`compassConfig()->mag_declination / 10`). 0 if `USE_MAG` disabled | 0.1 degrees
    uint16_t vbatScale;  // Voltage scale (`batteryMetersConfig()->voltage.scale`). 0 if `USE_ADC` disabled | Scale
    uint8_t vbatSource;  // Enum `batVoltageSource_e` Voltage source (`batteryMetersConfig()->voltageSource`). 0 if `USE_ADC` disabled
    uint8_t cellCount;  // Configured cell count (`currentBatteryProfile->cells`). 0 if `USE_ADC` disabled | Count
    uint16_t vbatCellDetect;  // Cell detection voltage (`currentBatteryProfile->voltage.cellDetect`). 0 if `USE_ADC` disabled | 0.01V
    uint16_t vbatMinCell;  // Min cell voltage (`currentBatteryProfile->voltage.cellMin`). 0 if `USE_ADC` disabled | 0.01V
    uint16_t vbatMaxCell;  // Max cell voltage (`currentBatteryProfile->voltage.cellMax`). 0 if `USE_ADC` disabled | 0.01V
    uint16_t vbatWarningCell;  // Warning cell voltage (`currentBatteryProfile->voltage.cellWarning`). 0 if `USE_ADC` disabled | 0.01V
    uint32_t capacityValue;  // Battery capacity (`currentBatteryProfile->capacity.value`) | mAh/mWh
    uint32_t capacityWarning;  // Capacity warning threshold (`currentBatteryProfile->capacity.warning`) | mAh/mWh
    uint32_t capacityCritical;  // Capacity critical threshold (`currentBatteryProfile->capacity.critical`) | mAh/mWh
    uint8_t capacityUnit;  // Enum `batCapacityUnit_e` Capacity unit (`batteryMetersConfig()->capacity_unit`)
} msp2InavMiscReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavMiscReply_t) == 41, msp2InavMiscReply_t_size);

// MSP2_INAV_SET_MISC (MSPv2) id=8196
// Sets miscellaneous configuration settings, superseding `MSP_SET_MISC`.
// Notes: Expects 41 bytes. Performs validation on `vbatSource` and `capacityUnit`.
typedef struct MSP_PACKED {
    uint16_t midRc;  // Ignored | PWM
    uint16_t legacyMinThrottle;  // Ignored
    uint16_t legacyMaxThrottle;  // Ignored
    uint16_t minCommand;  // Sets `motorConfigMutable()->mincommand` (constrained) | PWM
    uint16_t failsafeThrottle;  // Sets `currentBatteryProfileMutable->failsafe_throttle` (constrained) | PWM
    uint8_t gpsType;  // Enum `gpsProvider_e` Sets `gpsConfigMutable()->provider` (if `USE_GPS`)
    uint8_t legacyGpsBaud;  // Ignored
    uint8_t gpsSbasMode;  // Enum `sbasMode_e` Sets `gpsConfigMutable()->sbasMode` (if `USE_GPS`)
    uint8_t rssiChannel;  // Sets `rxConfigMutable()->rssi_channel` (1-based, 0 disables) when <= `MAX_SUPPORTED_RC_CHANNEL_COUNT` | Index
    int16_t magDeclination;  // Sets `compassConfigMutable()->mag_declination = value * 10` (if `USE_MAG`) | 0.1 degrees
    uint16_t vbatScale;  // Sets `batteryMetersConfigMutable()->voltage.scale` (if `USE_ADC`) | Scale
    uint8_t vbatSource;  // Enum `batVoltageSource_e` Sets `batteryMetersConfigMutable()->voltageSource` (if `USE_ADC`, validated)
    uint8_t cellCount;  // Sets `currentBatteryProfileMutable->cells` (if `USE_ADC`) | Count
    uint16_t vbatCellDetect;  // Sets `currentBatteryProfileMutable->voltage.cellDetect` (if `USE_ADC`) | 0.01V
    uint16_t vbatMinCell;  // Sets `currentBatteryProfileMutable->voltage.cellMin` (if `USE_ADC`) | 0.01V
    uint16_t vbatMaxCell;  // Sets `currentBatteryProfileMutable->voltage.cellMax` (if `USE_ADC`) | 0.01V
    uint16_t vbatWarningCell;  // Sets `currentBatteryProfileMutable->voltage.cellWarning` (if `USE_ADC`) | 0.01V
    uint32_t capacityValue;  // Sets `currentBatteryProfileMutable->capacity.value` | mAh/mWh
    uint32_t capacityWarning;  // Sets `currentBatteryProfileMutable->capacity.warning` | mAh/mWh
    uint32_t capacityCritical;  // Sets `currentBatteryProfileMutable->capacity.critical` | mAh/mWh
    uint8_t capacityUnit;  // Enum `batCapacityUnit_e` Sets `batteryMetersConfigMutable()->capacity_unit` (validated, updates OSD energy unit if changed)
} msp2InavSetMiscRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetMiscRequest_t) == 41, msp2InavSetMiscRequest_t_size);

// MSP2_INAV_BATTERY_CONFIG (MSPv2) id=8197
// Retrieves the configuration specific to the battery voltage and current sensors and capacity settings for the current battery profile.
// Notes: Fields are 0 if `USE_ADC` is not defined.
typedef struct MSP_PACKED {
    uint16_t vbatScale;  // Voltage scale (`batteryMetersConfig()->voltage.scale`) | Scale
    uint8_t vbatSource;  // Enum `batVoltageSource_e` Voltage source (`batteryMetersConfig()->voltageSource`)
    uint8_t cellCount;  // Configured cell count (`currentBatteryProfile->cells`) | Count
    uint16_t vbatCellDetect;  // Cell detection voltage (`currentBatteryProfile->voltage.cellDetect`) | 0.01V
    uint16_t vbatMinCell;  // Min cell voltage (`currentBatteryProfile->voltage.cellMin`) | 0.01V
    uint16_t vbatMaxCell;  // Max cell voltage (`currentBatteryProfile->voltage.cellMax`) | 0.01V
    uint16_t vbatWarningCell;  // Warning cell voltage (`currentBatteryProfile->voltage.cellWarning`) | 0.01V
    int16_t currentOffset;  // Current sensor offset (`batteryMetersConfig()->current.offset`) | mV
    int16_t currentScale;  // Current sensor scale (`batteryMetersConfig()->current.scale`) | 0.1 mV/A
    uint32_t capacityValue;  // Battery capacity (`currentBatteryProfile->capacity.value`) | mAh/mWh
    uint32_t capacityWarning;  // Capacity warning threshold (`currentBatteryProfile->capacity.warning`) | mAh/mWh
    uint32_t capacityCritical;  // Capacity critical threshold (`currentBatteryProfile->capacity.critical`) | mAh/mWh
    uint8_t capacityUnit;  // Enum `batCapacityUnit_e` Capacity unit (`batteryMetersConfig()->capacity_unit`)
} msp2InavBatteryConfigReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavBatteryConfigReply_t) == 29, msp2InavBatteryConfigReply_t_size);

// MSP2_INAV_SET_BATTERY_CONFIG (MSPv2) id=8198
// Sets the battery voltage/current sensor configuration and capacity settings for the current battery profile.
// Notes: Expects 29 bytes. Performs validation on `vbatSource` and `capacityUnit`.
typedef struct MSP_PACKED {
    uint16_t vbatScale;  // Sets `batteryMetersConfigMutable()->voltage.scale` (if `USE_ADC`) | Scale
    uint8_t vbatSource;  // Enum `batVoltageSource_e` Sets `batteryMetersConfigMutable()->voltageSource` (if `USE_ADC`, validated)
    uint8_t cellCount;  // Sets `currentBatteryProfileMutable->cells` (if `USE_ADC`) | Count
    uint16_t vbatCellDetect;  // Sets `currentBatteryProfileMutable->voltage.cellDetect` (if `USE_ADC`) | 0.01V
    uint16_t vbatMinCell;  // Sets `currentBatteryProfileMutable->voltage.cellMin` (if `USE_ADC`) | 0.01V
    uint16_t vbatMaxCell;  // Sets `currentBatteryProfileMutable->voltage.cellMax` (if `USE_ADC`) | 0.01V
    uint16_t vbatWarningCell;  // Sets `currentBatteryProfileMutable->voltage.cellWarning` (if `USE_ADC`) | 0.01V
    int16_t currentOffset;  // Sets `batteryMetersConfigMutable()->current.offset` | mV
    int16_t currentScale;  // Sets `batteryMetersConfigMutable()->current.scale` | 0.1 mV/A
    uint32_t capacityValue;  // Sets `currentBatteryProfileMutable->capacity.value` | mAh/mWh
    uint32_t capacityWarning;  // Sets `currentBatteryProfileMutable->capacity.warning` | mAh/mWh
    uint32_t capacityCritical;  // Sets `currentBatteryProfileMutable->capacity.critical` | mAh/mWh
    uint8_t capacityUnit;  // Enum `batCapacityUnit_e` Sets `batteryMetersConfigMutable()->capacity_unit` (validated, updates OSD energy unit if changed)
} msp2InavSetBatteryConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetBatteryConfigRequest_t) == 29, msp2InavSetBatteryConfigRequest_t_size);

// MSP2_INAV_RATE_PROFILE (MSPv2) id=8199
// Retrieves the rates and expos for the current control rate profile, including both stabilized and manual flight modes. Supersedes `MSP_RC_TUNING`.
typedef struct MSP_PACKED {
    uint8_t throttleMid;  // Throttle Midpoint (`currentControlRateProfile->throttle.rcMid8`)
    uint8_t throttleExpo;  // Throttle Expo (`currentControlRateProfile->throttle.rcExpo8`)
    uint8_t dynamicThrottlePID;  // TPA value (`currentControlRateProfile->throttle.dynPID`)
    uint16_t tpaBreakpoint;  // TPA breakpoint (`currentControlRateProfile->throttle.tpa_breakpoint`)
    uint8_t stabRcExpo;  // Stabilized Roll/Pitch Expo (`currentControlRateProfile->stabilized.rcExpo8`)
    uint8_t stabRcYawExpo;  // Stabilized Yaw Expo (`currentControlRateProfile->stabilized.rcYawExpo8`)
    uint8_t stabRollRate;  // Stabilized Roll Rate (`currentControlRateProfile->stabilized.rates[FD_ROLL]`)
    uint8_t stabPitchRate;  // Stabilized Pitch Rate (`currentControlRateProfile->stabilized.rates[FD_PITCH]`)
    uint8_t stabYawRate;  // Stabilized Yaw Rate (`currentControlRateProfile->stabilized.rates[FD_YAW]`)
    uint8_t manualRcExpo;  // Manual Roll/Pitch Expo (`currentControlRateProfile->manual.rcExpo8`)
    uint8_t manualRcYawExpo;  // Manual Yaw Expo (`currentControlRateProfile->manual.rcYawExpo8`)
    uint8_t manualRollRate;  // Manual Roll Rate (`currentControlRateProfile->manual.rates[FD_ROLL]`)
    uint8_t manualPitchRate;  // Manual Pitch Rate (`currentControlRateProfile->manual.rates[FD_PITCH]`)
    uint8_t manualYawRate;  // Manual Yaw Rate (`currentControlRateProfile->manual.rates[FD_YAW]`)
} msp2InavRateProfileReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavRateProfileReply_t) == 15, msp2InavRateProfileReply_t_size);

// MSP2_INAV_SET_RATE_PROFILE (MSPv2) id=8200
// Sets the rates and expos for the current control rate profile (stabilized and manual). Supersedes `MSP_SET_RC_TUNING`.
// Notes: Expects 15 bytes. Constraints applied to rates based on axis.
typedef struct MSP_PACKED {
    uint8_t throttleMid;  // Sets `currentControlRateProfile->throttle.rcMid8`
    uint8_t throttleExpo;  // Sets `currentControlRateProfile->throttle.rcExpo8`
    uint8_t dynamicThrottlePID;  // Sets `currentControlRateProfile->throttle.dynPID`
    uint16_t tpaBreakpoint;  // Sets `currentControlRateProfile->throttle.tpa_breakpoint`
    uint8_t stabRcExpo;  // Sets `currentControlRateProfile->stabilized.rcExpo8`
    uint8_t stabRcYawExpo;  // Sets `currentControlRateProfile->stabilized.rcYawExpo8`
    uint8_t stabRollRate;  // Sets `currentControlRateProfile->stabilized.rates[FD_ROLL]` (constrained)
    uint8_t stabPitchRate;  // Sets `currentControlRateProfile->stabilized.rates[FD_PITCH]` (constrained)
    uint8_t stabYawRate;  // Sets `currentControlRateProfile->stabilized.rates[FD_YAW]` (constrained)
    uint8_t manualRcExpo;  // Sets `currentControlRateProfile->manual.rcExpo8`
    uint8_t manualRcYawExpo;  // Sets `currentControlRateProfile->manual.rcYawExpo8`
    uint8_t manualRollRate;  // Sets `currentControlRateProfile->manual.rates[FD_ROLL]` (constrained)
    uint8_t manualPitchRate;  // Sets `currentControlRateProfile->manual.rates[FD_PITCH]` (constrained)
    uint8_t manualYawRate;  // Sets `currentControlRateProfile->manual.rates[FD_YAW]` (constrained)
} msp2InavSetRateProfileRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetRateProfileRequest_t) == 15, msp2InavSetRateProfileRequest_t_size);

// MSP2_INAV_AIR_SPEED (MSPv2) id=8201
// Retrieves the estimated or measured airspeed.
// Notes: Requires `USE_PITOT`; returns 0 when pitot functionality is not enabled or calibrated.
typedef struct MSP_PACKED {
    uint32_t airspeed;  // Estimated/measured airspeed (`getAirspeedEstimate()`, cm/s). 0 if unavailable | cm/s
} msp2InavAirSpeedReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavAirSpeedReply_t) == 4, msp2InavAirSpeedReply_t_size);

// MSP2_INAV_OUTPUT_MAPPING (MSPv2) id=8202
// Retrieves the output mapping configuration (identifies which timer outputs are used for Motors/Servos). Legacy version sending only 8-bit usage flags.
// Notes: Superseded by `MSP2_INAV_OUTPUT_MAPPING_EXT2`. Only includes timers *not* used for PPM/PWM input. Record count is not a constant: loops timerHardwareCount, skipping timers flagged TIM_USE_PPM or TIM_USE_PWM; read until the payload is exhausted.
// payload is a bare sequence of records (repeat: until_end); no fixed
// header, so no wrapper struct. count = payload_size / sizeof(msp2InavOutputMappingReplyElem_t)
typedef struct MSP_PACKED {
    uint8_t usageFlags;  // Timer usage flags (lower 8 bits of `timerHardware[i].usageFlags`, e.g. `TIM_USE_MOTOR`, `TIM_USE_SERVO`)
} msp2InavOutputMappingReplyElem_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOutputMappingReplyElem_t) == 1, msp2InavOutputMappingReplyElem_t_size);

// MSP2_INAV_MC_BRAKING (MSPv2) id=8203
// Retrieves configuration parameters for the multirotor braking mode feature.
// Notes: Payload is empty if `USE_MR_BRAKING_MODE` is not defined.
typedef struct MSP_PACKED {
    uint16_t brakingSpeedThreshold;  // Speed above which braking engages (`navConfig()->mc.braking_speed_threshold`) | cm/s
    uint16_t brakingDisengageSpeed;  // Speed below which braking disengages (`navConfig()->mc.braking_disengage_speed`) | cm/s
    uint16_t brakingTimeout;  // Timeout before braking force reduces (`navConfig()->mc.braking_timeout`) | ms
    uint8_t brakingBoostFactor;  // Boost factor applied during braking (`navConfig()->mc.braking_boost_factor`) | %
    uint16_t brakingBoostTimeout;  // Timeout for the boost factor (`navConfig()->mc.braking_boost_timeout`) | ms
    uint16_t brakingBoostSpeedThreshold;  // Speed threshold for boost engagement (`navConfig()->mc.braking_boost_speed_threshold`) | cm/s
    uint16_t brakingBoostDisengageSpeed;  // Speed threshold for boost disengagement (`navConfig()->mc.braking_boost_disengage_speed`) | cm/s
    uint8_t brakingBankAngle;  // Maximum bank angle allowed during braking (`navConfig()->mc.braking_bank_angle`) | degrees
} msp2InavMcBrakingReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavMcBrakingReply_t) == 14, msp2InavMcBrakingReply_t_size);

// MSP2_INAV_SET_MC_BRAKING (MSPv2) id=8204
// Sets configuration parameters for the multirotor braking mode feature.
// Notes: Expects 14 bytes. Returns error if `USE_MR_BRAKING_MODE` is not defined.
typedef struct MSP_PACKED {
    uint16_t brakingSpeedThreshold;  // Sets `navConfigMutable()->mc.braking_speed_threshold` | cm/s
    uint16_t brakingDisengageSpeed;  // Sets `navConfigMutable()->mc.braking_disengage_speed` | cm/s
    uint16_t brakingTimeout;  // Sets `navConfigMutable()->mc.braking_timeout` | ms
    uint8_t brakingBoostFactor;  // Sets `navConfigMutable()->mc.braking_boost_factor` | %
    uint16_t brakingBoostTimeout;  // Sets `navConfigMutable()->mc.braking_boost_timeout` | ms
    uint16_t brakingBoostSpeedThreshold;  // Sets `navConfigMutable()->mc.braking_boost_speed_threshold` | cm/s
    uint16_t brakingBoostDisengageSpeed;  // Sets `navConfigMutable()->mc.braking_boost_disengage_speed` | cm/s
    uint8_t brakingBankAngle;  // Sets `navConfigMutable()->mc.braking_bank_angle` | degrees
} msp2InavSetMcBrakingRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetMcBrakingRequest_t) == 14, msp2InavSetMcBrakingRequest_t_size);

// MSP2_INAV_OUTPUT_MAPPING_EXT (MSPv2) id=8205
// Retrieves extended output mapping configuration (timer ID and usage flags). Obsolete, use `MSP2_INAV_OUTPUT_MAPPING_EXT2`.
// Notes: Usage flags are truncated to 8 bits. `timerId` mapping is target-specific. Record count is not a constant: loops timerHardwareCount, skipping timers flagged TIM_USE_PPM or TIM_USE_PWM; read until the payload is exhausted.
// payload is a bare sequence of records (repeat: until_end); no fixed
// header, so no wrapper struct. count = payload_size / sizeof(msp2InavOutputMappingExtReplyElem_t)
typedef struct MSP_PACKED {
    uint8_t timerId;  // Hardware timer identifier (e.g., `TIM1`, `TIM2`). Value depends on target
    uint8_t usageFlags;  // Timer usage flags (lower 8 bits of `timerHardware[i].usageFlags`, e.g. `TIM_USE_MOTOR`, `TIM_USE_SERVO`)
} msp2InavOutputMappingExtReplyElem_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOutputMappingExtReplyElem_t) == 2, msp2InavOutputMappingExtReplyElem_t_size);

// MSP2_INAV_TIMER_OUTPUT_MODE (MSPv2) id=8206
// Reads timer output mode overrides.
// Notes: Non-SITL only. HARDWARE_TIMER_DEFINITION_COUNT is target specific. Returns MSP_RESULT_ACK on success, MSP_RESULT_ERROR on invalid timer index.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint8_t timerIndex;  // Timer index
        uint8_t outputMode;  // OUTPUT_MODE_* | enum outputMode_e
    } items[HARDWARE_TIMER_DEFINITION_COUNT];  // repeat: HARDWARE_TIMER_DEFINITION_COUNT
} msp2InavTimerOutputMode_dataSize_eq_0Reply_t;

// MSP2_INAV_TIMER_OUTPUT_MODE (MSPv2) id=8206
// Reads timer output mode overrides.
// Notes: Non-SITL only. HARDWARE_TIMER_DEFINITION_COUNT is target specific. Returns MSP_RESULT_ACK on success, MSP_RESULT_ERROR on invalid timer index.
typedef struct MSP_PACKED {
    uint8_t timerIndex;  // 0..HARDWARE_TIMER_DEFINITION_COUNT-1
} msp2InavTimerOutputMode_dataSize_eq_1Request_t;
MSP_STATIC_ASSERT(sizeof(msp2InavTimerOutputMode_dataSize_eq_1Request_t) == 1, msp2InavTimerOutputMode_dataSize_eq_1Request_t_size);

// MSP2_INAV_TIMER_OUTPUT_MODE (MSPv2) id=8206
// Reads timer output mode overrides.
// Notes: Non-SITL only. HARDWARE_TIMER_DEFINITION_COUNT is target specific. Returns MSP_RESULT_ACK on success, MSP_RESULT_ERROR on invalid timer index.
typedef struct MSP_PACKED {
    uint8_t timerIndex;  // Echoed timer index
    uint8_t outputMode;  // OUTPUT_MODE_* | enum outputMode_e
} msp2InavTimerOutputMode_dataSize_eq_1Reply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavTimerOutputMode_dataSize_eq_1Reply_t) == 2, msp2InavTimerOutputMode_dataSize_eq_1Reply_t_size);

// MSP2_INAV_SET_TIMER_OUTPUT_MODE (MSPv2) id=8207
// Set the output mode override for a specific hardware timer.
// Notes: Only available on non-SITL builds. Expects 2 bytes. Returns error if `timerIndex` is invalid.
typedef struct MSP_PACKED {
    uint8_t timerIndex;  // Index of the hardware timer definition
    uint8_t outputMode;  // Output mode override (`outputMode_e` enum) to set | enum outputMode_e
} msp2InavSetTimerOutputModeRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetTimerOutputModeRequest_t) == 2, msp2InavSetTimerOutputModeRequest_t_size);

// MSP2_INAV_MIXER (MSPv2) id=8208
// Retrieves INAV-specific mixer configuration details.
typedef struct MSP_PACKED {
    uint8_t motorDirectionInverted;  // Boolean: 1 if motor direction is reversed globally (`mixerConfig()->motorDirectionInverted`)
    uint8_t reserved1;  // Always 0 (Was yaw jump prevention limit) | always 0
    uint8_t motorStopOnLow;  // Boolean: 1 if motors stop at minimum throttle (`mixerConfig()->motorstopOnLow`)
    uint8_t platformType;  // Enum (`mixerConfig()->platformType`) | enum flyingPlatformType_e
    uint8_t hasFlaps;  // Boolean: 1 if the current mixer configuration includes flaps (`mixerConfig()->hasFlaps`)
    int16_t appliedMixerPreset;  // Mixer preset currently applied (`mixerConfig()->appliedMixerPreset`). Plain integer, not an enum: the firmware never interprets it and only stores whatever the configurator wrote, defaulting to `SETTING_MODEL_PREVIEW_TYPE_DEFAULT`.
    uint8_t maxMotors;  // Constant: Maximum motors supported (`MAX_SUPPORTED_MOTORS`)
    uint8_t maxServos;  // Constant: Maximum servos supported (`MAX_SUPPORTED_SERVOS`)
} msp2InavMixerReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavMixerReply_t) == 9, msp2InavMixerReply_t_size);

// MSP2_INAV_SET_MIXER (MSPv2) id=8209
// Sets INAV-specific mixer configuration details.
// Notes: Expects 9 bytes. Calls `mixerUpdateStateFlags()`.
typedef struct MSP_PACKED {
    uint8_t motorDirectionInverted;  // Sets `mixerConfigMutable()->motorDirectionInverted`
    uint8_t reserved1;  // Ignored
    uint8_t motorStopOnLow;  // Sets `mixerConfigMutable()->motorstopOnLow`
    uint8_t platformType;  // Sets `mixerConfigMutable()->platformType` | enum flyingPlatformType_e
    uint8_t hasFlaps;  // Sets `mixerConfigMutable()->hasFlaps`
    int16_t appliedMixerPreset;  // Sets `mixerConfigMutable()->appliedMixerPreset`
    uint8_t maxMotors;  // Ignored
    uint8_t maxServos;  // Ignored
} msp2InavSetMixerRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetMixerRequest_t) == 9, msp2InavSetMixerRequest_t_size);

// MSP2_INAV_OSD_LAYOUTS (MSPv2) id=8210
// Retrieves OSD layout metadata or item positions for specific layouts/items.
// Notes: Requires `USE_OSD`. Returns `MSP_RESULT_ACK` on success, `MSP_RESULT_ERROR` if indexes are out of range.
typedef struct MSP_PACKED {
    uint8_t layoutCount;  // Number of OSD layouts (`OSD_LAYOUT_COUNT`)
    uint8_t itemCount;  // Number of OSD items per layout (`OSD_ITEM_COUNT`)
} msp2InavOsdLayouts_dataSize_eq_0Reply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOsdLayouts_dataSize_eq_0Reply_t) == 2, msp2InavOsdLayouts_dataSize_eq_0Reply_t_size);

// MSP2_INAV_OSD_LAYOUTS (MSPv2) id=8210
// Retrieves OSD layout metadata or item positions for specific layouts/items.
// Notes: Requires `USE_OSD`. Returns `MSP_RESULT_ACK` on success, `MSP_RESULT_ERROR` if indexes are out of range.
typedef struct MSP_PACKED {
    uint8_t layoutIndex;  // Layout index (0 to `OSD_LAYOUT_COUNT - 1`)
} msp2InavOsdLayouts_dataSize_eq_1Request_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOsdLayouts_dataSize_eq_1Request_t) == 1, msp2InavOsdLayouts_dataSize_eq_1Request_t_size);

// MSP2_INAV_OSD_LAYOUTS (MSPv2) id=8210
// Retrieves OSD layout metadata or item positions for specific layouts/items.
// Notes: Requires `USE_OSD`. Returns `MSP_RESULT_ACK` on success, `MSP_RESULT_ERROR` if indexes are out of range.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint16_t itemPosition;  // Packed X/Y position (`osdLayoutsConfig()->item_pos[layoutIndex][item]`) | packed coords
    } items[OSD_ITEM_COUNT];  // repeat: OSD_ITEM_COUNT
} msp2InavOsdLayouts_dataSize_eq_1Reply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOsdLayouts_dataSize_eq_1Reply_t) == 344, msp2InavOsdLayouts_dataSize_eq_1Reply_t_size);

// MSP2_INAV_OSD_LAYOUTS (MSPv2) id=8210
// Retrieves OSD layout metadata or item positions for specific layouts/items.
// Notes: Requires `USE_OSD`. Returns `MSP_RESULT_ACK` on success, `MSP_RESULT_ERROR` if indexes are out of range.
typedef struct MSP_PACKED {
    uint8_t layoutIndex;  // Layout index (0 to `OSD_LAYOUT_COUNT - 1`)
    uint16_t itemIndex;  // OSD item index (0 to `OSD_ITEM_COUNT - 1`)
} msp2InavOsdLayouts_dataSize_eq_3Request_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOsdLayouts_dataSize_eq_3Request_t) == 3, msp2InavOsdLayouts_dataSize_eq_3Request_t_size);

// MSP2_INAV_OSD_LAYOUTS (MSPv2) id=8210
// Retrieves OSD layout metadata or item positions for specific layouts/items.
// Notes: Requires `USE_OSD`. Returns `MSP_RESULT_ACK` on success, `MSP_RESULT_ERROR` if indexes are out of range.
typedef struct MSP_PACKED {
    uint16_t itemPosition;  // Packed X/Y position (`osdLayoutsConfig()->item_pos[layoutIndex][itemIndex]`) | packed coords
} msp2InavOsdLayouts_dataSize_eq_3Reply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOsdLayouts_dataSize_eq_3Reply_t) == 2, msp2InavOsdLayouts_dataSize_eq_3Reply_t_size);

// MSP2_INAV_OSD_SET_LAYOUT_ITEM (MSPv2) id=8211
// Sets the position of a single OSD item within a specific layout.
// Notes: Requires `USE_OSD`. Expects 4 bytes. Returns error if indexes are invalid. If the modified layout is not the currently active one, it temporarily overrides the active layout for 10 seconds to show the change. Otherwise, triggers a full OSD redraw.
typedef struct MSP_PACKED {
    uint8_t layoutIndex;  // Index of the OSD layout (0 to `OSD_LAYOUT_COUNT - 1`) | Index
    uint8_t itemIndex;  // Index of the OSD item | Index
    uint16_t itemPosition;  // Packed X/Y position using `OSD_POS(x, y)` with `OSD_VISIBLE_FLAG` bit | Coordinates
} msp2InavOsdSetLayoutItemRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOsdSetLayoutItemRequest_t) == 4, msp2InavOsdSetLayoutItemRequest_t_size);

// MSP2_INAV_OSD_ALARMS (MSPv2) id=8212
// Retrieves OSD alarm threshold settings.
// Notes: Requires `USE_OSD`.
typedef struct MSP_PACKED {
    uint8_t rssiAlarm;  // RSSI alarm threshold (`osdConfig()->rssi_alarm`) | %
    uint16_t timerAlarm;  // Timer alarm threshold (`osdConfig()->time_alarm`) | seconds
    uint16_t altAlarm;  // Altitude alarm threshold (`osdConfig()->alt_alarm`) | meters
    uint16_t distAlarm;  // Distance alarm threshold (`osdConfig()->dist_alarm`) | meters
    uint16_t negAltAlarm;  // Negative altitude alarm threshold (`osdConfig()->neg_alt_alarm`) | meters
    uint16_t gForceAlarm;  // G-force alarm threshold (`osdConfig()->gforce_alarm * 1000`) | G * 1000
    int16_t gForceAxisMinAlarm;  // Min G-force per-axis alarm (`osdConfig()->gforce_axis_alarm_min * 1000`) | G * 1000
    int16_t gForceAxisMaxAlarm;  // Max G-force per-axis alarm (`osdConfig()->gforce_axis_alarm_max * 1000`) | G * 1000
    uint8_t currentAlarm;  // Current draw alarm threshold (`osdConfig()->current_alarm`) | A
    int16_t imuTempMinAlarm;  // Min IMU temperature alarm (`osdConfig()->imu_temp_alarm_min`) | degrees C
    int16_t imuTempMaxAlarm;  // Max IMU temperature alarm (`osdConfig()->imu_temp_alarm_max`) | degrees C
    int16_t baroTempMinAlarm;  // Min Baro temperature alarm (`osdConfig()->baro_temp_alarm_min`). 0 if `USE_BARO` disabled | degrees C
    int16_t baroTempMaxAlarm;  // Max Baro temperature alarm (`osdConfig()->baro_temp_alarm_max`). 0 if `USE_BARO` disabled | degrees C
    uint16_t adsbWarnDistance;  // ADSB warning distance (`osdConfig()->adsb_distance_warning`). 0 if `USE_ADSB` disabled | meters
    uint16_t adsbAlertDistance;  // ADSB alert distance (`osdConfig()->adsb_distance_alert`). 0 if `USE_ADSB` disabled | meters
} msp2InavOsdAlarmsReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOsdAlarmsReply_t) == 28, msp2InavOsdAlarmsReply_t_size);

// MSP2_INAV_OSD_SET_ALARMS (MSPv2) id=8213
// Sets OSD alarm threshold settings.
// Notes: Requires `USE_OSD`. Expects 24 bytes. ADSB alarms are not settable via this message.
typedef struct MSP_PACKED {
    uint8_t rssiAlarm;  // Sets `osdConfigMutable()->rssi_alarm | %
    uint16_t timerAlarm;  // Sets `osdConfigMutable()->time_alarm | seconds
    uint16_t altAlarm;  // Sets `osdConfigMutable()->alt_alarm | meters
    uint16_t distAlarm;  // Sets `osdConfigMutable()->dist_alarm | meters
    uint16_t negAltAlarm;  // Sets `osdConfigMutable()->neg_alt_alarm` | meters
    uint16_t gForceAlarm;  // Sets `osdConfigMutable()->gforce_alarm = value / 1000.0f` | G * 1000
    int16_t gForceAxisMinAlarm;  // Sets `osdConfigMutable()->gforce_axis_alarm_min = value / 1000.0f` | G * 1000
    int16_t gForceAxisMaxAlarm;  // Sets `osdConfigMutable()->gforce_axis_alarm_max = value / 1000.0f` | G * 1000
    uint8_t currentAlarm;  // Sets `osdConfigMutable()->current_alarm` | A
    int16_t imuTempMinAlarm;  // Sets `osdConfigMutable()->imu_temp_alarm_min` | degrees C
    int16_t imuTempMaxAlarm;  // Sets `osdConfigMutable()->imu_temp_alarm_max` | degrees C
    int16_t baroTempMinAlarm;  // Sets `osdConfigMutable()->baro_temp_alarm_min` (if `USE_BARO`) | degrees C
    int16_t baroTempMaxAlarm;  // Sets `osdConfigMutable()->baro_temp_alarm_max` (if `USE_BARO`) | degrees C
} msp2InavOsdSetAlarmsRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOsdSetAlarmsRequest_t) == 24, msp2InavOsdSetAlarmsRequest_t_size);

// MSP2_INAV_OSD_PREFERENCES (MSPv2) id=8214
// Retrieves OSD display preferences (video system, units, styles, etc.).
// Notes: Requires `USE_OSD`.
typedef struct MSP_PACKED {
    uint8_t videoSystem;  // Enum `videoSystem_e`: Video system (Auto/PAL/NTSC) (`osdConfig()->video_system`)
    uint8_t mainVoltageDecimals;  // Count: Decimal places for main voltage display (`osdConfig()->main_voltage_decimals`)
    uint8_t ahiReverseRoll;  // Boolean: Reverse roll direction on Artificial Horizon (`osdConfig()->ahi_reverse_roll`)
    uint8_t crosshairsStyle;  // Enum `osd_crosshairs_style_e`: Style of the center crosshairs (`osdConfig()->crosshairs_style`)
    uint8_t leftSidebarScroll;  // Enum `osd_sidebar_scroll_e`: Left sidebar scroll behavior (`osdConfig()->left_sidebar_scroll`)
    uint8_t rightSidebarScroll;  // Enum `osd_sidebar_scroll_e`: Right sidebar scroll behavior (`osdConfig()->right_sidebar_scroll`)
    uint8_t sidebarScrollArrows;  // Boolean: Show arrows for scrollable sidebars (`osdConfig()->sidebar_scroll_arrows`)
    uint8_t units;  // Enum: `osd_unit_e` Measurement units (Metric/Imperial) (`osdConfig()->units`) | enum osd_unit_e
    uint8_t statsEnergyUnit;  // Enum `osd_stats_energy_unit_e`: Unit for energy display in post-flight stats (`osdConfig()->stats_energy_unit`)
    uint8_t adsbWarningStyle;  // Enum `osd_adsb_warning_style_e`: How ADSB proximity warnings are drawn (`osdConfig()->adsb_warning_style`). 0 if `USE_ADSB` disabled
} msp2InavOsdPreferencesReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOsdPreferencesReply_t) == 10, msp2InavOsdPreferencesReply_t_size);

// MSP2_INAV_OSD_SET_PREFERENCES (MSPv2) id=8215
// Sets OSD display preferences.
// Notes: Requires `USE_OSD`. Expects 9 bytes. Triggers a full OSD redraw.
typedef struct MSP_PACKED {
    uint8_t videoSystem;  // Sets `osdConfigMutable()->video_system` | enum videoSystem_e
    uint8_t mainVoltageDecimals;  // Sets `osdConfigMutable()->main_voltage_decimals`
    uint8_t ahiReverseRoll;  // Sets `osdConfigMutable()->ahi_reverse_roll`
    uint8_t crosshairsStyle;  // Sets `osdConfigMutable()->crosshairs_style` | enum osd_crosshairs_style_e
    uint8_t leftSidebarScroll;  // Sets `osdConfigMutable()->left_sidebar_scroll` | enum osd_sidebar_scroll_e
    uint8_t rightSidebarScroll;  // Sets `osdConfigMutable()->right_sidebar_scroll` | enum osd_sidebar_scroll_e
    uint8_t sidebarScrollArrows;  // Sets `osdConfigMutable()->sidebar_scroll_arrows`
    uint8_t units;  // Sets `osdConfigMutable()->units` (enum `osd_unit_e`)
    uint8_t statsEnergyUnit;  // Sets `osdConfigMutable()->stats_energy_unit` | enum osd_stats_energy_unit_e
    uint8_t adsbWarningStyle;  // Sets `osdConfigMutable()->adsb_warning_style`. Only read when the payload is at least 10 bytes and the firmware has `USE_ADSB` | OPTIONAL: may be absent from a shorter payload
} msp2InavOsdSetPreferencesRequest_t;

// MSP2_INAV_SELECT_BATTERY_PROFILE (MSPv2) id=8216
// Selects the active battery profile and saves configuration.
// Notes: Expects 1 byte. Will fail if armed. Calls `setConfigBatteryProfileAndWriteEEPROM()`.
typedef struct MSP_PACKED {
    uint8_t batteryProfileIndex;  // Index of the battery profile to activate (0-based)
} msp2InavSelectBatteryProfileRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSelectBatteryProfileRequest_t) == 1, msp2InavSelectBatteryProfileRequest_t_size);

// MSP2_INAV_DEBUG (MSPv2) id=8217
// Retrieves values from the firmware's 32-bit `debug[]` array. Supersedes `MSP_DEBUG`.
// Notes: `DEBUG32_VALUE_COUNT` is usually 8.
typedef struct MSP_PACKED {
    int32_t debugValues[DEBUG32_VALUE_COUNT];  // Values from the `debug` array (signed, typically 8 entries)
} msp2InavDebugReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavDebugReply_t) == 32, msp2InavDebugReply_t_size);

// MSP2_BLACKBOX_CONFIG (MSPv2) id=8218
// Retrieves the Blackbox configuration. Supersedes `MSP_BLACKBOX_CONFIG`.
// Notes: If `USE_BLACKBOX` is disabled, only the first four fields are returned (all zero).
typedef struct MSP_PACKED {
    uint8_t blackboxSupported;  // Boolean: 1 if Blackbox is supported (`USE_BLACKBOX`), 0 otherwise
    uint8_t blackboxDevice;  // Enum `BlackboxDevice`: Target device for logging (`blackboxConfig()->device`). 0 if not supported
    uint16_t blackboxRateNum;  // Numerator for logging rate divider (`blackboxConfig()->rate_num`). 0 if not supported
    uint16_t blackboxRateDenom;  // Denominator for logging rate divider (`blackboxConfig()->rate_denom`). 0 if not supported
    uint32_t blackboxIncludeFlags;  // Bitmask: Flags for fields included/excluded from logging (`blackboxConfig()->includeFlags`). Absent when the firmware was built without `USE_BLACKBOX`, which replies with the first 6 bytes only | bitmask | OPTIONAL: may be absent from a shorter payload
} msp2BlackboxConfigReply_t;

// MSP2_SET_BLACKBOX_CONFIG (MSPv2) id=8219
// Sets the Blackbox configuration. Supersedes `MSP_SET_BLACKBOX_CONFIG`.
// Notes: Requires `USE_BLACKBOX`. Expects 9 bytes. Returns error if Blackbox is currently logging (`!blackboxMayEditConfig()`).
typedef struct MSP_PACKED {
    uint8_t blackboxDevice;  // Sets `blackboxConfigMutable()->device` | enum BlackboxDevice
    uint16_t blackboxRateNum;  // Sets `blackboxConfigMutable()->rate_num`
    uint16_t blackboxRateDenom;  // Sets `blackboxConfigMutable()->rate_denom`
    uint32_t blackboxIncludeFlags;  // Sets `blackboxConfigMutable()->includeFlags`
} msp2SetBlackboxConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2SetBlackboxConfigRequest_t) == 9, msp2SetBlackboxConfigRequest_t_size);

// MSP2_INAV_TEMP_SENSOR_CONFIG (MSPv2) id=8220
// Retrieves the configuration for all onboard temperature sensors.
// Notes: Requires `USE_TEMPERATURE_SENSOR`.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint8_t type;  // Enum (`tempSensorType_e`): Type of the temperature sensor | enum tempSensorType_e
        uint64_t address;  // Sensor address/ID (e.g., for 1-Wire sensors)
        int16_t alarmMin;  // Min temperature alarm threshold (`sensorConfig->alarm_min`) | 0.1°C
        int16_t alarmMax;  // Max temperature alarm threshold (`sensorConfig->alarm_max`) | 0.1°C
        uint8_t osdSymbol;  // Index: OSD symbol to use for this sensor (0 to `TEMP_SENSOR_SYM_COUNT`)
        char label[TEMPERATURE_LABEL_LEN];  // User-defined label for the sensor
    } items[MAX_TEMP_SENSORS];  // repeat: MAX_TEMP_SENSORS
} msp2InavTempSensorConfigReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavTempSensorConfigReply_t) == 144, msp2InavTempSensorConfigReply_t_size);

// MSP2_INAV_SET_TEMP_SENSOR_CONFIG (MSPv2) id=8221
// Sets the configuration for all onboard temperature sensors.
// Notes: Requires `USE_TEMPERATURE_SENSOR`. Payload must include `MAX_TEMP_SENSORS` consecutive `tempSensorConfig_t` structures (labels are uppercased).
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint8_t type;  // Sets sensor type (`tempSensorType_e`) | enum tempSensorType_e
        uint64_t address;  // Sets sensor address/ID
        int16_t alarmMin;  // Sets min alarm threshold (`tempSensorConfigMutable(index)->alarm_min`) | 0.1°C
        int16_t alarmMax;  // Sets max alarm threshold (`tempSensorConfigMutable(index)->alarm_max`) | 0.1°C
        uint8_t osdSymbol;  // Sets OSD symbol index (validated)
        char label[TEMPERATURE_LABEL_LEN];  // Sets sensor label (converted to uppercase)
    } items[MAX_TEMP_SENSORS];  // repeat: MAX_TEMP_SENSORS
} msp2InavSetTempSensorConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetTempSensorConfigRequest_t) == 144, msp2InavSetTempSensorConfigRequest_t_size);

// MSP2_INAV_TEMPERATURES (MSPv2) id=8222
// Retrieves the current readings from all configured temperature sensors.
// Notes: Requires `USE_TEMPERATURE_SENSOR`.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        int16_t temperature;  // Current temperature reading. -1000 if sensor is invalid or reading failed | 0.1°C
    } items[MAX_TEMP_SENSORS];  // repeat: MAX_TEMP_SENSORS
} msp2InavTemperaturesReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavTemperaturesReply_t) == 16, msp2InavTemperaturesReply_t_size);

// MSP_SIMULATOR (MSPv2) id=8223
// Handles Hardware-in-the-Loop (HITL) simulation data exchange. Receives simulated sensor data and options, sends back control outputs and debug info.
// Notes: Requires `USE_SIMULATOR`. Complex message handling state changes for enabling/disabling HITL. Sensor data is injected directly. OSD data is sent using a custom RLE scheme. See `simulatorData` struct and associated code for details.
typedef struct MSP_PACKED {
    uint8_t simulatorVersion;  // Version of the simulator protocol (`SIMULATOR_MSP_VERSION`)
    uint8_t simulatorFlags_t;  // Bitmask: Options for HITL (`HITL_*` flags) | bitmask | enum simulatorFlags_t
    uint8_t gpsFixType;  // Enum `gpsFixType_e` Type of GPS fix (If `HITL_HAS_NEW_GPS_DATA`)
    uint8_t gpsNumSat;  // (If `HITL_HAS_NEW_GPS_DATA`) Simulated satellite count
    int32_t gpsLat;  // (If `HITL_HAS_NEW_GPS_DATA`) Simulated latitude (1e7 deg)
    int32_t gpsLon;  // (If `HITL_HAS_NEW_GPS_DATA`) Simulated longitude (1e7 deg)
    int32_t gpsAlt;  // (If `HITL_HAS_NEW_GPS_DATA`) Simulated altitude (cm)
    uint16_t gpsSpeed;  // (If `HITL_HAS_NEW_GPS_DATA`) Simulated ground speed (cm/s)
    uint16_t gpsCourse;  // (If `HITL_HAS_NEW_GPS_DATA`) Simulated ground course (deci-deg)
    int16_t gpsVelN;  // (If `HITL_HAS_NEW_GPS_DATA`) Simulated North velocity (cm/s)
    int16_t gpsVelE;  // (If `HITL_HAS_NEW_GPS_DATA`) Simulated East velocity (cm/s)
    int16_t gpsVelD;  // (If `HITL_HAS_NEW_GPS_DATA`) Simulated Down velocity (cm/s)
    int16_t imuRoll;  // (If NOT `HITL_USE_IMU`) Simulated Roll (deci-deg)
    int16_t imuPitch;  // (If NOT `HITL_USE_IMU`) Simulated Pitch (deci-deg)
    int16_t imuYaw;  // (If NOT `HITL_USE_IMU`) Simulated Yaw (deci-deg)
    int16_t accX;  // mG (G * 1000)
    int16_t accY;  // mG (G * 1000)
    int16_t accZ;  // mG (G * 1000)
    int16_t gyroX;  // dps * 16
    int16_t gyroY;  // dps * 16
    int16_t gyroZ;  // dps * 16
    uint32_t baroPressure;  // Pa
    int16_t magX;  // Scaled
    int16_t magY;  // Scaled
    int16_t magZ;  // Scaled
    uint8_t vbat;  // (If `HITL_EXT_BATTERY_VOLTAGE`) Simulated battery voltage (0.1V units)
    uint16_t airspeed;  // (If `HITL_AIRSPEED`) Simulated airspeed (cm/s)
    uint8_t extFlags;  // (If `HITL_EXTENDED_FLAGS`) Additional flags (upper 8 bits)
} mspSimulatorRequest_t;
MSP_STATIC_ASSERT(sizeof(mspSimulatorRequest_t) == 58, mspSimulatorRequest_t_size);

// MSP_SIMULATOR (MSPv2) id=8223
// Handles Hardware-in-the-Loop (HITL) simulation data exchange. Receives simulated sensor data and options, sends back control outputs and debug info.
// Notes: Requires `USE_SIMULATOR`. Complex message handling state changes for enabling/disabling HITL. Sensor data is injected directly. OSD data is sent using a custom RLE scheme. See `simulatorData` struct and associated code for details.
typedef struct MSP_PACKED {
    uint16_t stabilizedRoll;  // Stabilized Roll command output (-500 to 500)
    uint16_t stabilizedPitch;  // Stabilized Pitch command output (-500 to 500)
    uint16_t stabilizedYaw;  // Stabilized Yaw command output (-500 to 500)
    uint16_t stabilizedThrottle;  // Stabilized Throttle command output (-500 to 500 if armed, else -500)
    uint8_t debugFlags;  // Packed flags: Debug index (0-7), Platform type, Armed state, OSD feature status
    uint32_t debugValue;  // Current debug value (`debug[simulatorData.debugIndex]`)
    int16_t attitudeRoll;  // Current estimated Roll (deci-deg)
    int16_t attitudePitch;  // Current estimated Pitch (deci-deg)
    int16_t attitudeYaw;  // Current estimated Yaw (deci-deg)
    uint8_t osdHeader;  // OSD RLE Header (255) | OPTIONAL: may be absent from a shorter payload
    uint8_t osdRows;  // (If OSD supported) Number of OSD rows | OPTIONAL: may be absent from a shorter payload
    uint8_t osdCols;  // (If OSD supported) Number of OSD columns | OPTIONAL: may be absent from a shorter payload
    uint8_t osdStartY;  // (If OSD supported) Starting row for RLE data | OPTIONAL: may be absent from a shorter payload
    uint8_t osdStartX;  // (If OSD supported) Starting column for RLE data | OPTIONAL: may be absent from a shorter payload
    uint8_t osdRleData[];  // (If OSD supported) Run-length encoded OSD character data. Terminated by `[0, 0]` | OPTIONAL: may be absent from a shorter payload
} mspSimulatorReply_t;
// variable length: sizeof(mspSimulatorReply_t) is the fixed header only

// MSP2_INAV_SERVO_MIXER (MSPv2) id=8224
// Retrieves the custom servo mixer rules, including programming framework condition IDs, for primary and secondary mixer profiles. Supersedes `MSP_SERVO_MIX_RULES`.
// Notes: `conditionId` requires `USE_PROGRAMMING_FRAMEWORK`. If multiple mixer profiles are enabled (`MAX_MIXER_PROFILE_COUNT > 1`), a second block of `MAX_SERVO_RULES` rules for the next profile follows immediately.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint8_t targetChannel;  // Servo output channel index (0-based)
        uint8_t inputSource;  // Enum `inputSource_e` Input source
        int16_t rate;  // Mixing rate/weight
        uint8_t speed;  // Speed/Slew rate limit (0-100)
        int8_t conditionId;  // Logic Condition ID (0 to `MAX_LOGIC_CONDITIONS - 1`, or 255/-1 if none/disabled)
    } items[MAX_SERVO_RULES];  // repeat: MAX_SERVO_RULES
} msp2InavServoMixerReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavServoMixerReply_t) == 216, msp2InavServoMixerReply_t_size);

// MSP2_INAV_SET_SERVO_MIXER (MSPv2) id=8225
// Sets a single custom servo mixer rule, including programming framework condition ID. Supersedes `MSP_SET_SERVO_MIX_RULE`.
// Notes: Expects 7 bytes. Returns error if index invalid. Calls `loadCustomServoMixer()`.
typedef struct MSP_PACKED {
    uint8_t ruleIndex;  // Index of the rule to set (0 to `MAX_SERVO_RULES - 1`)
    uint8_t targetChannel;  // Servo output channel index
    uint8_t inputSource;  // Enum `inputSource_e` Input source
    int16_t rate;  // Mixing rate/weight
    uint8_t speed;  // Speed/Slew rate limit (0-100)
    int8_t conditionId;  // Logic Condition ID (255/-1 if none). Ignored if `USE_PROGRAMMING_FRAMEWORK` is disabled
} msp2InavSetServoMixerRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetServoMixerRequest_t) == 7, msp2InavSetServoMixerRequest_t_size);

// MSP2_INAV_SET_LOGIC_CONDITIONS (MSPv2) id=8227
// Sets the configuration for a single Logic Condition by its index.
// Notes: Requires `USE_PROGRAMMING_FRAMEWORK`. Expects 15 bytes. Returns error if index is invalid.
typedef struct MSP_PACKED {
    uint8_t conditionIndex;  // Index of the condition to set (0 to `MAX_LOGIC_CONDITIONS - 1`)
    uint8_t enabled;  // Boolean: 1 to enable the condition
    int8_t activatorId;  // Activator condition ID (-1/255 if none)
    uint8_t operation;  // Enum `logicOperation_e` Logical operation
    uint8_t operandAType;  // Enum `logicOperandType_e` Type of operand A
    int32_t operandAValue;  // Value/ID of operand A
    uint8_t operandBType;  // Enum `logicOperandType_e` Type of operand B
    int32_t operandBValue;  // Value/ID of operand B
    uint8_t flags;  // Bitmask: Condition flags (`logicConditionFlags_e`) | bitmask | enum logicConditionFlags_e
} msp2InavSetLogicConditionsRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetLogicConditionsRequest_t) == 15, msp2InavSetLogicConditionsRequest_t_size);

// MSP2_INAV_LOGIC_CONDITIONS_STATUS (MSPv2) id=8230
// Retrieves the current evaluated status (true/false or numerical value) of all logic conditions.
// Notes: Requires `USE_PROGRAMMING_FRAMEWORK`.
typedef struct MSP_PACKED {
    int32_t conditionValues[MAX_LOGIC_CONDITIONS];  // Array of current values for each logic condition (`logicConditionGetValue(i)`). 1 for true, 0 for false, or numerical value depending on operation
} msp2InavLogicConditionsStatusReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavLogicConditionsStatusReply_t) == 256, msp2InavLogicConditionsStatusReply_t_size);

// MSP2_INAV_GVAR_STATUS (MSPv2) id=8231
// Retrieves the current values of all Global Variables (GVARS).
// Notes: Requires `USE_PROGRAMMING_FRAMEWORK`.
typedef struct MSP_PACKED {
    int32_t gvarValues[MAX_GLOBAL_VARIABLES];  // Array of current values for each global variable (`gvGet(i)`)
} msp2InavGvarStatusReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavGvarStatusReply_t) == 32, msp2InavGvarStatusReply_t_size);

// MSP2_INAV_PROGRAMMING_PID (MSPv2) id=8232
// Retrieves the configuration of all Programming PIDs.
// Notes: Requires `USE_PROGRAMMING_FRAMEWORK`. See `programmingPid_t` structure.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint8_t enabled;  // Boolean: 1 if the PID is enabled
        uint8_t setpointType;  // Enum (`logicOperandType_e`) Type of the setpoint source | enum logicOperandType_e
        int32_t setpointValue;  // Value/ID of the setpoint source
        uint8_t measurementType;  // Enum (`logicOperandType_e`) Type of the measurement source | enum logicOperandType_e
        int32_t measurementValue;  // Value/ID of the measurement source
        uint16_t gainP;  // Proportional gain
        uint16_t gainI;  // Integral gain
        uint16_t gainD;  // Derivative gain
        uint16_t gainFF;  // Feed-forward gain
    } items[MAX_PROGRAMMING_PID_COUNT];  // repeat: MAX_PROGRAMMING_PID_COUNT
} msp2InavProgrammingPidReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavProgrammingPidReply_t) == 76, msp2InavProgrammingPidReply_t_size);

// MSP2_INAV_SET_PROGRAMMING_PID (MSPv2) id=8233
// Sets the configuration for a single Programming PID by its index.
// Notes: Requires `USE_PROGRAMMING_FRAMEWORK`. Expects 20 bytes. Returns error if index is invalid.
typedef struct MSP_PACKED {
    uint8_t pidIndex;  // Index of the Programming PID to set (0 to `MAX_PROGRAMMING_PID_COUNT - 1`)
    uint8_t enabled;  // Boolean: 1 to enable the PID
    uint8_t setpointType;  // Enum (`logicOperandType_e`) Type of the setpoint source | enum logicOperandType_e
    int32_t setpointValue;  // Value/ID of the setpoint source
    uint8_t measurementType;  // Enum (`logicOperandType_e`) Type of the measurement source | enum logicOperandType_e
    int32_t measurementValue;  // Value/ID of the measurement source
    uint16_t gainP;  // Proportional gain
    uint16_t gainI;  // Integral gain
    uint16_t gainD;  // Derivative gain
    uint16_t gainFF;  // Feed-forward gain
} msp2InavSetProgrammingPidRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetProgrammingPidRequest_t) == 20, msp2InavSetProgrammingPidRequest_t_size);

// MSP2_INAV_PROGRAMMING_PID_STATUS (MSPv2) id=8234
// Retrieves the current output value of all Programming PIDs.
// Notes: Requires `USE_PROGRAMMING_FRAMEWORK`.
typedef struct MSP_PACKED {
    int32_t pidOutputs[MAX_PROGRAMMING_PID_COUNT];  // Array of current output values for each Programming PID (`programmingPidGetOutput(i)`, signed)
} msp2InavProgrammingPidStatusReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavProgrammingPidStatusReply_t) == 16, msp2InavProgrammingPidStatusReply_t_size);

// MSP2_PID (MSPv2) id=8240
// Retrieves the standard PID controller gains (P, I, D, FF) for the current PID profile.
// Notes: `PID_ITEM_COUNT` defines the number of standard PID controllers (Roll, Pitch, Yaw, Alt, Vel, etc.). Updates from EZ-Tune if enabled.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint8_t P;  // Proportional gain (`pidBank()->pid[i].P`), constrained 0-255
        uint8_t I;  // Integral gain (`pidBank()->pid[i].I`), constrained 0-255
        uint8_t D;  // Derivative gain (`pidBank()->pid[i].D`), constrained 0-255
        uint8_t FF;  // Feed-forward gain (`pidBank()->pid[i].FF`), constrained 0-255
    } items[PID_ITEM_COUNT];  // repeat: PID_ITEM_COUNT
} msp2PidReply_t;
MSP_STATIC_ASSERT(sizeof(msp2PidReply_t) == 48, msp2PidReply_t_size);

// MSP2_SET_PID (MSPv2) id=8241
// Sets the standard PID controller gains (P, I, D, FF) for the current PID profile.
// Notes: Expects `PID_ITEM_COUNT * 4` bytes. Calls `schedulePidGainsUpdate()` and `navigationUsePIDs()`.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint8_t P;  // Sets Proportional gain (`pidBankMutable()->pid[i].P`)
        uint8_t I;  // Sets Integral gain (`pidBankMutable()->pid[i].I`)
        uint8_t D;  // Sets Derivative gain (`pidBankMutable()->pid[i].D`)
        uint8_t FF;  // Sets Feed-forward gain (`pidBankMutable()->pid[i].FF`)
    } items[PID_ITEM_COUNT];  // repeat: PID_ITEM_COUNT
} msp2SetPidRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2SetPidRequest_t) == 48, msp2SetPidRequest_t_size);

// MSP2_INAV_FWUPDT_PREPARE (MSPv2) id=8243
// Prepares the flight controller to receive a firmware update via MSP.
// Notes: Requires `MSP_FIRMWARE_UPDATE`. Expects 4 bytes. Returns error if preparation fails (e.g., no storage, invalid size). Calls `firmwareUpdatePrepare()`.
typedef struct MSP_PACKED {
    uint32_t firmwareSize;  // Total size of the incoming firmware file in bytes
} msp2InavFwupdtPrepareRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavFwupdtPrepareRequest_t) == 4, msp2InavFwupdtPrepareRequest_t_size);

// MSP2_INAV_FWUPDT_STORE (MSPv2) id=8244
// Stores a chunk of firmware data received via MSP.
// Notes: Requires `MSP_FIRMWARE_UPDATE`. Returns error if storage fails (e.g., out of space, checksum error). Called repeatedly until the entire firmware is transferred. Calls `firmwareUpdateStore()`.
// payload is a bare array of firmwareChunk; no fixed header, so no struct.
// element count = payload_size / sizeof(msp2InavFwupdtStoreRequestElem_t)
typedef uint8_t msp2InavFwupdtStoreRequestElem_t;

// MSP2_INAV_FWUPDT_EXEC (MSPv2) id=8245
// Executes the firmware update process (flashes the stored firmware and reboots).
// Notes: Requires `MSP_FIRMWARE_UPDATE`. Expects 1 byte. Returns error if update cannot start (e.g., not fully received). Calls `firmwareUpdateExec()`. If successful, the device will reboot into the new firmware.
typedef struct MSP_PACKED {
    uint8_t updateType;  // Type of update (e.g., full flash, specific section - currently ignored/unused)
} msp2InavFwupdtExecRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavFwupdtExecRequest_t) == 1, msp2InavFwupdtExecRequest_t_size);

// MSP2_INAV_SAFEHOME (MSPv2) id=8248
// Get or Set configuration for a specific Safe Home location.
// Notes: Requires `USE_SAFE_HOME`. Used by `mspFcSafeHomeOutCommand`. See `MSP2_INAV_SET_SAFEHOME` for setting.
typedef struct MSP_PACKED {
    uint8_t safehomeIndex;  // Index of the safe home location (0 to `MAX_SAFE_HOMES - 1`)
} msp2InavSafehomeRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSafehomeRequest_t) == 1, msp2InavSafehomeRequest_t_size);

// MSP2_INAV_SAFEHOME (MSPv2) id=8248
// Get or Set configuration for a specific Safe Home location.
// Notes: Requires `USE_SAFE_HOME`. Used by `mspFcSafeHomeOutCommand`. See `MSP2_INAV_SET_SAFEHOME` for setting.
typedef struct MSP_PACKED {
    uint8_t safehomeIndex;  // Index requested
    uint8_t enabled;  // Boolean: 1 if this safe home is enabled
    int32_t latitude;  // Latitude (1e7 deg)
    int32_t longitude;  // Longitude (1e7 deg)
} msp2InavSafehomeReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSafehomeReply_t) == 10, msp2InavSafehomeReply_t_size);

// MSP2_INAV_SET_SAFEHOME (MSPv2) id=8249
// Sets the configuration for a specific Safe Home location.
// Notes: Requires `USE_SAFE_HOME`. Expects 10 bytes. Returns error if index invalid. Resets corresponding FW autoland approach if `USE_FW_AUTOLAND` is enabled.
typedef struct MSP_PACKED {
    uint8_t safehomeIndex;  // Index of the safe home location (0 to `MAX_SAFE_HOMES - 1`)
    uint8_t enabled;  // Boolean: 1 to enable this safe home
    int32_t latitude;  // Latitude (1e7 deg)
    int32_t longitude;  // Longitude (1e7 deg)
} msp2InavSetSafehomeRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetSafehomeRequest_t) == 10, msp2InavSetSafehomeRequest_t_size);

// MSP2_INAV_MISC2 (MSPv2) id=8250
// Retrieves miscellaneous runtime information including timers and throttle status.
typedef struct MSP_PACKED {
    uint32_t uptimeSeconds;  // Time since boot (`micros() / 1000000`) | Seconds
    uint32_t flightTimeSeconds;  // Accumulated flight time (`getFlightTime()`) | Seconds
    uint8_t throttlePercent;  // Current throttle output percentage (`getThrottlePercent(true)`) | %
    uint8_t autoThrottleFlag;  // 1 if navigation is controlling throttle, 0 otherwise (`navigationIsControllingThrottle()`) | Boolean
} msp2InavMisc2Reply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavMisc2Reply_t) == 10, msp2InavMisc2Reply_t_size);

// MSP2_INAV_LOGIC_CONDITIONS_SINGLE (MSPv2) id=8251
// Gets the configuration for a single Logic Condition by its index.
// Notes: Requires `USE_PROGRAMMING_FRAMEWORK`. Used by `mspFcLogicConditionCommand`.
typedef struct MSP_PACKED {
    uint8_t conditionIndex;  // Index of the condition to retrieve (0 to `MAX_LOGIC_CONDITIONS - 1`)
} msp2InavLogicConditionsSingleRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavLogicConditionsSingleRequest_t) == 1, msp2InavLogicConditionsSingleRequest_t_size);

// MSP2_INAV_LOGIC_CONDITIONS_SINGLE (MSPv2) id=8251
// Gets the configuration for a single Logic Condition by its index.
// Notes: Requires `USE_PROGRAMMING_FRAMEWORK`. Used by `mspFcLogicConditionCommand`.
typedef struct MSP_PACKED {
    uint8_t enabled;  // Boolean: 1 if enabled
    int8_t activatorId;  // Activator ID (-1/255 if none)
    uint8_t operation;  // Enum `logicOperation_e` Logical operation
    uint8_t operandAType;  // Enum `logicOperandType_e` Type of operand A
    int32_t operandAValue;  // Value/ID of operand A
    uint8_t operandBType;  // Enum `logicOperandType_e` Type of operand B
    int32_t operandBValue;  // Value/ID of operand B
    uint8_t flags;  // Bitmask: Condition flags (`logicConditionFlags_e`) | bitmask | enum logicConditionFlags_e
} msp2InavLogicConditionsSingleReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavLogicConditionsSingleReply_t) == 14, msp2InavLogicConditionsSingleReply_t_size);

// MSP2_INAV_LOGIC_CONDITIONS_CONFIGURED (MSPv2) id=8252
// Returns a bitmask of which logic conditions are configured, so a client can fetch only the used slots instead of all of them.
// Notes: Requires `USE_PROGRAMMING_FRAMEWORK`. Fixed 8-byte reply carrying one 64-bit mask as two `uint32_t` halves, low half first. Only the first `MIN(MAX_LOGIC_CONDITIONS, 64)` bits are evaluated. A condition counts as configured when any of `enabled`, `activatorId` (default -1), `operation`, `operandA.type`, `operandA.value`, `operandB.type`, `operandB.value` or `flags` differs from its default.
typedef struct MSP_PACKED {
    uint32_t configuredMaskLow;  // Bits 0-31 of the bitmask. Bit N is set when logic condition N differs from its default values.
    uint32_t configuredMaskHigh;  // Bits 32-63 of the bitmask. Always 0 on targets where `MAX_LOGIC_CONDITIONS` is 32 or fewer.
} msp2InavLogicConditionsConfiguredReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavLogicConditionsConfiguredReply_t) == 8, msp2InavLogicConditionsConfiguredReply_t_size);

// MSP2_INAV_ESC_RPM (MSPv2) id=8256
// Retrieves the RPM reported by each ESC via telemetry.
// Notes: Requires `USE_ESC_SENSOR`. Payload size depends on the number of detected motors with telemetry. Record count is not a constant: loops getMotorCount(); read until the payload is exhausted.
// payload is a bare sequence of records (repeat: until_end); no fixed
// header, so no wrapper struct. count = payload_size / sizeof(msp2InavEscRpmReplyElem_t)
typedef struct MSP_PACKED {
    uint32_t escRpm;  // RPM reported by the ESC | RPM
} msp2InavEscRpmReplyElem_t;
MSP_STATIC_ASSERT(sizeof(msp2InavEscRpmReplyElem_t) == 4, msp2InavEscRpmReplyElem_t_size);

// MSP2_INAV_ESC_TELEM (MSPv2) id=8257
// Retrieves the full telemetry data structure reported by each ESC.
// Notes: Requires `USE_ESC_SENSOR`. See `escSensorData_t` in `sensors/esc_sensor.h` for the exact structure fields.
typedef struct MSP_PACKED {
    uint8_t motorCount;  // Number of motors reporting telemetry (`getMotorCount()`)
    struct MSP_PACKED {
        escSensorData_t esc;  // One ESC's telemetry (voltage, current, temperature, RPM, error count); see `escSensorData_t`
    } escData[];  // repeat: motorCount
} msp2InavEscTelemReply_t;
// variable length: sizeof(msp2InavEscTelemReply_t) is the fixed header only

// MSP2_INAV_DRONECAN_NODES (MSPv2) id=8258
// Returns the list of all detected DroneCAN nodes with their current status.
// Notes: Requires `USE_DRONECAN`. Response is `nodeCount` followed by `nodeCount` records of 13 bytes each: nodeID(1)+health(1)+mode(1)+last_seen_ms(4)+uptime_sec(4)+vendor_status_code(2). Maximum payload 1 + (DRONECAN_MAX_NODES * 13) = 417 bytes. For full node detail (name, SW/HW version, unique ID) use MSP2_INAV_DRONECAN_ASYNC_REQUEST with service_id=DRONECAN_SERVICE_GETNODEINFO(1).
typedef struct MSP_PACKED {
    uint8_t nodeCount;  // Number of detected DroneCAN nodes
    struct MSP_PACKED {
        uint8_t nodeID;  // DroneCAN node ID (1-127)
        uint8_t health;  // Node health: 0=OK, 1=WARNING, 2=ERROR, 3=CRITICAL
        uint8_t mode;  // Node mode: 0=OPERATIONAL, 1=INITIALIZATION, 2=MAINTENANCE, 3=SOFTWARE_UPDATE, 7=OFFLINE
        uint32_t last_seen_ms;  // Milliseconds since this node was last seen (FC-local timestamp delta) | ms
        uint32_t uptime_sec;  // Node uptime in seconds (from NodeStatus broadcast) | s
        uint16_t vendor_status_code;  // Vendor-specific status code
    } items[];  // repeat: nodeCount
} msp2InavDronecanNodesReply_t;
// variable length: sizeof(msp2InavDronecanNodesReply_t) is the fixed header only

// MSP2_INAV_DRONECAN_ASYNC_REQUEST (MSPv2) id=8259
// Initiates an asynchronous DroneCAN service request (GetNodeInfo, ParamGetSet, ExecuteOpcode, RestartNode) to a specific node. Result retrieved via MSP2_INAV_DRONECAN_ASYNC_RESULT.
// Notes: Requires `USE_DRONECAN`. Initiates an async DroneCAN service request; poll MSP2_INAV_DRONECAN_ASYNC_RESULT at ~100ms intervals until state=READY(2) or ERROR(3). Only one request in-flight at a time. Service-specific request fields follow the common header in the request payload: EXECUTE_OPCODE appends opcode(u8); PARAM_GETSET appends index(u16)+is_write(u8) and optionally value_type(u8)+value(variable) for writes, then req_name_len(u8)+req_name(bytes) for named lookup. Param value encoding: INT=lo(u32)+hi(u32), FLOAT=raw(u32), BOOL=u8, STRING=len(u8)+data. Requests time out after DRONECAN_ASYNC_TIMEOUT_MS (2000ms). If bus is not in STATE_DRONECAN_NORMAL, returns accepted=0xFF without dispatching.
typedef struct MSP_PACKED {
    uint16_t service_id;  // Service to invoke: 1=GETNODEINFO, 5=RESTART_NODE, 10=EXECUTE_OPCODE, 11=PARAM_GETSET. Transmitted as u16 for MSP alignment; only low 8 bits used.
    uint8_t nodeID;  // Target DroneCAN node ID (1-127)
} msp2InavDronecanAsyncRequestRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavDronecanAsyncRequestRequest_t) == 3, msp2InavDronecanAsyncRequestRequest_t_size);

// MSP2_INAV_DRONECAN_ASYNC_REQUEST (MSPv2) id=8259
// Initiates an asynchronous DroneCAN service request (GetNodeInfo, ParamGetSet, ExecuteOpcode, RestartNode) to a specific node. Result retrieved via MSP2_INAV_DRONECAN_ASYNC_RESULT.
// Notes: Requires `USE_DRONECAN`. Initiates an async DroneCAN service request; poll MSP2_INAV_DRONECAN_ASYNC_RESULT at ~100ms intervals until state=READY(2) or ERROR(3). Only one request in-flight at a time. Service-specific request fields follow the common header in the request payload: EXECUTE_OPCODE appends opcode(u8); PARAM_GETSET appends index(u16)+is_write(u8) and optionally value_type(u8)+value(variable) for writes, then req_name_len(u8)+req_name(bytes) for named lookup. Param value encoding: INT=lo(u32)+hi(u32), FLOAT=raw(u32), BOOL=u8, STRING=len(u8)+data. Requests time out after DRONECAN_ASYNC_TIMEOUT_MS (2000ms). If bus is not in STATE_DRONECAN_NORMAL, returns accepted=0xFF without dispatching.
typedef struct MSP_PACKED {
    uint8_t accepted;  // 0=request accepted; 1=busy (slot in use) or unrecognised service_id; 0xFF=bus not in STATE_DRONECAN_NORMAL (not ready)
    uint8_t seq;  // Sequence number; correlate with MSP2_INAV_DRONECAN_ASYNC_RESULT to verify the result belongs to this request
} msp2InavDronecanAsyncRequestReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavDronecanAsyncRequestReply_t) == 2, msp2InavDronecanAsyncRequestReply_t_size);

// MSP2_INAV_DRONECAN_ASYNC_RESULT (MSPv2) id=8260
// Polls the result of the most recent MSP2_INAV_DRONECAN_ASYNC_REQUEST. Poll at ~100ms intervals until state is READY(2) or ERROR(3).
// Notes: Requires `USE_DRONECAN`. When state=READY(2), service-specific result fields follow the 5-byte common header. GETNODEINFO: name_len(u8)+name(bytes)+sw_major(u8)+sw_minor(u8)+sw_optional_field_flags(u8)+sw_vcs_commit(u32)+hw_major(u8)+hw_minor(u8)+hw_unique_id(u8[16]). PARAM_GETSET: name_len(u8)+name(bytes)+type(u8)+value(variable)+min_type(u8)+min(variable)+max_type(u8)+max(variable); value/min/max encoding: INT=lo(u32)+hi(u32), FLOAT=raw(u32), BOOL=u8, STRING=len(u8)+data; EMPTY(0) min/max type means no bound is present. EXECUTE_OPCODE and RESTART_NODE: ok(u8) where 1=success. Reading result when state=READY transitions slot back to IDLE.
typedef struct MSP_PACKED {
    uint8_t state;  // Async slot state: 0=IDLE, 1=PENDING, 2=READY, 3=ERROR
    uint8_t seq;  // Sequence number matching the originating MSP2_INAV_DRONECAN_ASYNC_REQUEST reply
    uint16_t service_id;  // Service ID of the in-flight or just-completed request
    uint8_t node_id;  // Node ID of the target
} msp2InavDronecanAsyncResultReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavDronecanAsyncResultReply_t) == 5, msp2InavDronecanAsyncResultReply_t_size);

// MSP2_INAV_LED_STRIP_CONFIG_EX (MSPv2) id=8264
// Retrieves the full configuration for each LED on the strip using the `ledConfig_t` structure. Supersedes `MSP_LED_STRIP_CONFIG`.
// Notes: Requires `USE_LED_STRIP`. See `ledConfig_t` in `io/ledstrip.h` for structure fields (position, function, overlay, color, direction, params). `ledConfig_t` is a packed bitfield struct of 40 bits = 5 bytes (led_position:8, led_function:8, led_overlay:8, led_color:4, led_direction:6, led_params:6); the reply is `LED_MAX_STRIP_LENGTH` consecutive 5-byte records.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        ledConfig_t ledConfig;  // Raw `ledConfig_t` structure (5 bytes) holding position, function, overlay, color, direction, and params bitfields (`io/ledstrip.h`).
    } items[LED_MAX_STRIP_LENGTH];  // repeat: LED_MAX_STRIP_LENGTH
} msp2InavLedStripConfigExReply_t;

// MSP2_INAV_SET_LED_STRIP_CONFIG_EX (MSPv2) id=8265
// Sets the configuration for a single LED on the strip using the `ledConfig_t` structure. Supersedes `MSP_SET_LED_STRIP_CONFIG`.
// Notes: Requires `USE_LED_STRIP`. Expects `1 + sizeof(ledConfig_t)` bytes. Returns error if index invalid. Calls `reevaluateLedConfig()`.
typedef struct MSP_PACKED {
    uint8_t ledIndex;  // Index of the LED to configure (0 to `LED_MAX_STRIP_LENGTH - 1`)
    ledConfig_t ledConfig;  // Raw `ledConfig_t` structure (6 bytes) mirroring the firmware layout.
} msp2InavSetLedStripConfigExRequest_t;

// MSP2_INAV_FW_APPROACH (MSPv2) id=8266
// Get or Set configuration for a specific Fixed Wing Autoland approach.
// Notes: Requires `USE_FW_AUTOLAND`. Used by `mspFwApproachOutCommand`. See `MSP2_INAV_SET_FW_APPROACH` for setting.
typedef struct MSP_PACKED {
    uint8_t approachIndex;  // Index of the approach setting (0 to `MAX_FW_LAND_APPOACH_SETTINGS - 1`)
} msp2InavFwApproachRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavFwApproachRequest_t) == 1, msp2InavFwApproachRequest_t_size);

// MSP2_INAV_FW_APPROACH (MSPv2) id=8266
// Get or Set configuration for a specific Fixed Wing Autoland approach.
// Notes: Requires `USE_FW_AUTOLAND`. Used by `mspFwApproachOutCommand`. See `MSP2_INAV_SET_FW_APPROACH` for setting.
typedef struct MSP_PACKED {
    uint8_t approachIndex;  // Index requested | Index
    int32_t approachAlt;  // Signed altitude for the approach phase (`navFwAutolandApproach_t.approachAlt`) | cm
    int32_t landAlt;  // Signed altitude for the final landing phase (`navFwAutolandApproach_t.landAlt`) | cm
    uint8_t approachDirection;  // Enum `fwAutolandApproachDirection_e`: Direction of approach (From WP, Specific Heading)
    int16_t landHeading1;  // Primary landing heading (if approachDirection requires it) | degrees
    int16_t landHeading2;  // Secondary landing heading (if approachDirection requires it) | degrees
    uint8_t isSeaLevelRef;  // 1 if altitudes are relative to sea level, 0 if relative to home | Boolean
} msp2InavFwApproachReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavFwApproachReply_t) == 15, msp2InavFwApproachReply_t_size);

// MSP2_INAV_SET_FW_APPROACH (MSPv2) id=8267
// Sets the configuration for a specific Fixed Wing Autoland approach.
// Notes: Requires `USE_FW_AUTOLAND`. Expects 15 bytes. Returns error if index invalid.
typedef struct MSP_PACKED {
    uint8_t approachIndex;  // Index of the approach setting (0 to `MAX_FW_LAND_APPOACH_SETTINGS - 1`) | Index
    int32_t approachAlt;  // Signed approach altitude (`navFwAutolandApproach_t.approachAlt`) | cm
    int32_t landAlt;  // Signed landing altitude (`navFwAutolandApproach_t.landAlt`) | cm
    uint8_t approachDirection;  // Enum `fwAutolandApproachDirection_e` Sets approach direction
    int16_t landHeading1;  // Sets primary landing heading | degrees
    int16_t landHeading2;  // Sets secondary landing heading | degrees
    uint8_t isSeaLevelRef;  // Sets altitude reference | Boolean
} msp2InavSetFwApproachRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetFwApproachRequest_t) == 15, msp2InavSetFwApproachRequest_t_size);

// MSP2_INAV_GPS_UBLOX_COMMAND (MSPv2) id=8272
// Sends a raw command directly to a U-Blox GPS module connected to the FC.
// Notes: Requires GPS feature enabled (`FEATURE_GPS`) and the GPS driver to be U-Blox (`isGpsUblox()`). Payload must be at least 8 bytes (minimum UBX frame size). Use with extreme caution, incorrect commands can misconfigure the GPS module. Calls `gpsUbloxSendCommand()`.
// payload is a bare array of ubxCommand; no fixed header, so no struct.
// element count = payload_size / sizeof(msp2InavGpsUbloxCommandRequestElem_t)
typedef uint8_t msp2InavGpsUbloxCommandRequestElem_t;

// MSP2_INAV_RATE_DYNAMICS (MSPv2) id=8288
// Retrieves Rate Dynamics configuration parameters for the current control rate profile.
// Notes: Requires `USE_RATE_DYNAMICS`.
typedef struct MSP_PACKED {
    uint8_t sensitivityCenter;  // Sensitivity at stick center (`currentControlRateProfile->rateDynamics.sensitivityCenter`) | %
    uint8_t sensitivityEnd;  // Sensitivity at stick ends (`currentControlRateProfile->rateDynamics.sensitivityEnd`) | %
    uint8_t correctionCenter;  // Correction strength at stick center (`currentControlRateProfile->rateDynamics.correctionCenter`) | %
    uint8_t correctionEnd;  // Correction strength at stick ends (`currentControlRateProfile->rateDynamics.correctionEnd`) | %
    uint8_t weightCenter;  // Transition weight at stick center (`currentControlRateProfile->rateDynamics.weightCenter`) | %
    uint8_t weightEnd;  // Transition weight at stick ends (`currentControlRateProfile->rateDynamics.weightEnd`) | %
} msp2InavRateDynamicsReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavRateDynamicsReply_t) == 6, msp2InavRateDynamicsReply_t_size);

// MSP2_INAV_SET_RATE_DYNAMICS (MSPv2) id=8289
// Sets Rate Dynamics configuration parameters for the current control rate profile.
// Notes: Requires `USE_RATE_DYNAMICS`. Expects 6 bytes.
typedef struct MSP_PACKED {
    uint8_t sensitivityCenter;  // Sets sensitivity at center | %
    uint8_t sensitivityEnd;  // Sets sensitivity at ends | %
    uint8_t correctionCenter;  // Sets correction at center | %
    uint8_t correctionEnd;  // Sets correction at ends | %
    uint8_t weightCenter;  // Sets weight at center | %
    uint8_t weightEnd;  // Sets weight at ends | %
} msp2InavSetRateDynamicsRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetRateDynamicsRequest_t) == 6, msp2InavSetRateDynamicsRequest_t_size);

// MSP2_INAV_EZ_TUNE (MSPv2) id=8304
// Retrieves the current EZ-Tune parameters.
// Notes: Requires `USE_EZ_TUNE`. Calls `ezTuneUpdate()` before sending.
typedef struct MSP_PACKED {
    uint8_t enabled;  // Boolean: 1 if EZ-Tune is enabled (`ezTune()->enabled`)
    uint16_t filterHz;  // Filter frequency used during tuning (`ezTune()->filterHz`)
    uint8_t axisRatio;  // Roll vs Pitch axis tuning ratio (`ezTune()->axisRatio`)
    uint8_t response;  // Desired response characteristic (`ezTune()->response`)
    uint8_t damping;  // Desired damping characteristic (`ezTune()->damping`)
    uint8_t stability;  // Stability preference (`ezTune()->stability`)
    uint8_t aggressiveness;  // Aggressiveness preference (`ezTune()->aggressiveness`)
    uint8_t rate;  // Resulting rate setting (`ezTune()->rate`)
    uint8_t expo;  // Resulting expo setting (`ezTune()->expo`)
    uint8_t snappiness;  // Snappiness preference (`ezTune()->snappiness`)
} msp2InavEzTuneReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavEzTuneReply_t) == 11, msp2InavEzTuneReply_t_size);

// MSP2_INAV_EZ_TUNE_SET (MSPv2) id=8305
// Sets the EZ-Tune parameters and triggers an update.
// Notes: Requires `USE_EZ_TUNE`. Expects 10 or 11 bytes. Calls `ezTuneUpdate()` after setting parameters.
typedef struct MSP_PACKED {
    uint8_t enabled;  // Sets enabled state
    uint16_t filterHz;  // Sets filter frequency
    uint8_t axisRatio;  // Sets axis ratio
    uint8_t response;  // Sets response characteristic
    uint8_t damping;  // Sets damping characteristic
    uint8_t stability;  // Sets stability preference
    uint8_t aggressiveness;  // Sets aggressiveness preference
    uint8_t rate;  // Sets rate setting
    uint8_t expo;  // Sets expo setting
    uint8_t snappiness;  // (Optional) Sets snappiness preference | OPTIONAL: may be absent from a shorter payload
} msp2InavEzTuneSetRequest_t;

// MSP2_INAV_SELECT_MIXER_PROFILE (MSPv2) id=8320
// Selects the active mixer profile and saves configuration.
// Notes: Expects 1 byte. Will fail if armed. Calls `setConfigMixerProfileAndWriteEEPROM()`. Only applicable if `MAX_MIXER_PROFILE_COUNT` > 1.
typedef struct MSP_PACKED {
    uint8_t mixerProfileIndex;  // Index of the mixer profile to activate (0-based)
} msp2InavSelectMixerProfileRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSelectMixerProfileRequest_t) == 1, msp2InavSelectMixerProfileRequest_t_size);

// MSP2_ADSB_VEHICLE_LIST (MSPv2) id=8336
// Retrieves the list of currently tracked ADSB (Automatic Dependent Surveillance–Broadcast) vehicles. See `adsbVehicle_t` and `adsbVehicleValues_t` in `io/adsb.h` for the exact structure fields.
// Notes: Requires `USE_ADSB`. Only a subset of `adsbVehicle_t` is transmitted (callsign, core values, heading in whole degrees, TSLC, emitter type, TTL).
typedef struct MSP_PACKED {
    uint8_t maxVehicles;  // Maximum number of vehicles tracked (`MAX_ADSB_VEHICLES`). 0 if `USE_ADSB` disabled
    uint8_t callsignLength;  // Maximum length of callsign string (`ADSB_CALL_SIGN_MAX_LENGTH`). 0 if `USE_ADSB` disabled
    uint32_t totalVehicleMsgs;  // Total vehicle messages received (`getAdsbStatus()->vehiclesMessagesTotal`). 0 if `USE_ADSB` disabled
    uint32_t totalHeartbeatMsgs;  // Total heartbeat messages received (`getAdsbStatus()->heartbeatMessagesTotal`). 0 if `USE_ADSB` disabled
    struct MSP_PACKED {
        char callsign[ADSB_CALL_SIGN_MAX_LENGTH];  // Fixed-length callsign from `adsbVehicle->vehicleValues.callsign` (padded with NULs if shorter).
        uint32_t icao;  // ICAO address (`adsbVehicle->vehicleValues.icao`).
        int32_t lat;  // Latitude in degrees * 1e7 (`adsbVehicle->vehicleValues.gps.lat`). | 1e-7 deg
        int32_t lon;  // Longitude in degrees * 1e7 (`adsbVehicle->vehicleValues.gps.lon`). | 1e-7 deg
        int32_t alt;  // Altitude above sea level (`adsbVehicle->vehicleValues.alt`). | cm
        uint16_t headingDeg;  // Course over ground in whole degrees (`CENTIDEGREES_TO_DEGREES(vehicleValues.heading)`). | deg
        uint8_t tslc;  // Time since last communication (`adsbVehicle->vehicleValues.tslc`). | s
        uint8_t emitterType;  // Emitter category (`adsbVehicle->vehicleValues.emitterType`) (refers to enum 'ADSB_EMITTER_TYPE', but none found)
        uint8_t ttl;  // TTL counter used for list maintenance (`adsbVehicle->ttl`).
    } items[];  // repeat: maxVehicles
} msp2AdsbVehicleListReply_t;
// variable length: sizeof(msp2AdsbVehicleListReply_t) is the fixed header only

// MSP2_ADSB_LIMITS (MSPv2) id=8337
// Retrieves the configured ADSB proximity distance limits used for OSD warnings and alerts.
// Notes: Requires `USE_ADSB`; all three fields are 0 when it is not compiled in.
typedef struct MSP_PACKED {
    uint16_t distanceWarning;  // `osdConfig()->adsb_distance_warning` (setting `osd_adsb_distance_warning`). Distance within which an ADSB vehicle is displayed. | m
    uint16_t distanceAlert;  // `osdConfig()->adsb_distance_alert` (setting `osd_adsb_distance_alert`). Distance inside which ADSB data flashes as a proximity warning. | m
    uint16_t ignorePlaneAboveMeLimit;  // `osdConfig()->adsb_ignore_plane_above_me_limit` (setting `osd_adsb_ignore_plane_above_me_limit`). Vehicles higher than this above the craft are ignored; 0 disables the limit. | m
} msp2AdsbLimitsReply_t;
MSP_STATIC_ASSERT(sizeof(msp2AdsbLimitsReply_t) == 6, msp2AdsbLimitsReply_t_size);

// MSP2_ADSB_WARNING_VEHICLE_ICAO (MSPv2) id=8338
// Returns the ICAO address of the ADSB vehicle currently triggering a proximity warning or alert.
// Notes: Requires `USE_ADSB`. Alert takes priority: `findVehicleForAlert()` is tried first using `osd_adsb_distance_alert`, then `findVehicleForWarning()` using `osd_adsb_distance_warning`, both bounded by `osd_adsb_ignore_plane_above_me_limit`. Replies 0/0 when `USE_ADSB` is not compiled in, when `isEnvironmentOkForCalculatingADSBDistanceBearing()` is false, or when no vehicle matches.
typedef struct MSP_PACKED {
    uint32_t icao;  // ICAO address of the vehicle currently triggering an alert or warning; 0 when none applies.
    uint8_t isAlert;  // 1 when the vehicle matched the alert distance, 0 when it matched only the warning distance or when `icao` is 0. | Boolean
} msp2AdsbWarningVehicleIcaoReply_t;
MSP_STATIC_ASSERT(sizeof(msp2AdsbWarningVehicleIcaoReply_t) == 5, msp2AdsbWarningVehicleIcaoReply_t_size);

// MSP2_ADSB_VEHICLE (MSPv2) id=8339
// Retrieves a single tracked ADSB (Automatic Dependent Surveillance-Broadcast) vehicle by slot index. Intended for polling one slot at a time: query `MSP2_ADSB_VEHICLE_COUNT` for the iteration bound, then request indices `0 .. count-1`, skipping slots with `ttl == 0`, and identify each aircraft by its `icao`. See `adsbVehicle_t` / `adsbVehicleValues_t` in `io/adsb.h`.
// Notes: Requires `USE_ADSB`. Reads a single ADSB vehicle slot by index. THE INDEX IS NOT A STABLE HANDLE: slots are reused, so a given index may hold a different aircraft (or be empty, `ttl == 0`) between polls. Correlate aircraft by the `icao` field in the reply, never by index. Compared with the bulk `MSP2_ADSB_VEHICLE_LIST`, this message adds horizontal velocity and reports heading at full (centidegree) resolution, and orders the callsign last. Returns an error result for an out-of-range index.
typedef struct MSP_PACKED {
    uint8_t index;  // Slot index to read, `0 .. (MSP2_ADSB_VEHICLE_COUNT - 1)`. WARNING: this is an iteration cursor over fixed slots, NOT a stable identifier. The same index may return a different aircraft (or an empty slot) on a later poll. Always identify the aircraft by the `icao` field in the reply; never cache or correlate data by index. Returns an error result if the index is out of range.
} msp2AdsbVehicleRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2AdsbVehicleRequest_t) == 1, msp2AdsbVehicleRequest_t_size);

// MSP2_ADSB_VEHICLE (MSPv2) id=8339
// Retrieves a single tracked ADSB (Automatic Dependent Surveillance-Broadcast) vehicle by slot index. Intended for polling one slot at a time: query `MSP2_ADSB_VEHICLE_COUNT` for the iteration bound, then request indices `0 .. count-1`, skipping slots with `ttl == 0`, and identify each aircraft by its `icao`. See `adsbVehicle_t` / `adsbVehicleValues_t` in `io/adsb.h`.
// Notes: Requires `USE_ADSB`. Reads a single ADSB vehicle slot by index. THE INDEX IS NOT A STABLE HANDLE: slots are reused, so a given index may hold a different aircraft (or be empty, `ttl == 0`) between polls. Correlate aircraft by the `icao` field in the reply, never by index. Compared with the bulk `MSP2_ADSB_VEHICLE_LIST`, this message adds horizontal velocity and reports heading at full (centidegree) resolution, and orders the callsign last. Returns an error result for an out-of-range index.
typedef struct MSP_PACKED {
    uint32_t icao;  // ICAO 24-bit address (`vehicleValues.icao`). This is the stable per-aircraft identifier; use it to correlate replies, not the request index. An empty slot reports `icao == 0` and `ttl == 0`.
    int32_t lat;  // Latitude (`vehicleValues.gps.lat`). | 1e-7 deg
    int32_t lon;  // Longitude (`vehicleValues.gps.lon`). | 1e-7 deg
    int32_t alt;  // Altitude above sea level (`vehicleValues.alt`). | cm
    uint16_t heading;  // Course over ground at full resolution (`vehicleValues.heading`). Unlike `MSP2_ADSB_VEHICLE_LIST`, this is in centidegrees, not whole degrees. | 1e-2 deg
    uint16_t horVelocity;  // Horizontal (ground) speed (`vehicleValues.horVelocity`). Not present in `MSP2_ADSB_VEHICLE_LIST`. | cm/s
    uint8_t tslc;  // Time since last communication (`vehicleValues.tslc`). | s
    uint8_t emitterType;  // Emitter category (`vehicleValues.emitterType`).
    uint8_t ttl;  // Remaining time-to-live for this slot (`adsbVehicle->ttl`). `ttl == 0` means the slot is empty/expired and its contents are stale; skip such entries. | s
    char callsign[ADSB_CALL_SIGN_MAX_LENGTH];  // Fixed-length callsign (`vehicleValues.callsign`), padded with NULs if shorter.
} msp2AdsbVehicleReply_t;
MSP_STATIC_ASSERT(sizeof(msp2AdsbVehicleReply_t) == 32, msp2AdsbVehicleReply_t_size);

// MSP2_ADSB_VEHICLE_COUNT (MSPv2) id=8340
// Returns the number of ADSB vehicle slots available to iterate with `MSP2_ADSB_VEHICLE`.
// Notes: Requires `USE_ADSB`. Returns the iteration bound for `MSP2_ADSB_VEHICLE`: request indices `0 .. count-1` and skip any slot whose `ttl == 0`.
typedef struct MSP_PACKED {
    uint8_t count;  // Number of vehicle slots to iterate (`MAX_ADSB_VEHICLES`). This is the slot capacity / iteration bound, not the number of currently active aircraft - some slots may be empty (`ttl == 0`). 0 if `USE_ADSB` is disabled.
} msp2AdsbVehicleCountReply_t;
MSP_STATIC_ASSERT(sizeof(msp2AdsbVehicleCountReply_t) == 1, msp2AdsbVehicleCountReply_t_size);

// MSP2_INAV_CUSTOM_OSD_ELEMENTS (MSPv2) id=8448
// Retrieves counts related to custom OSD elements defined by the programming framework.
// Notes: Requires `USE_PROGRAMMING_FRAMEWORK`.
typedef struct MSP_PACKED {
    uint8_t maxElements;  // Maximum number of custom elements (`MAX_CUSTOM_ELEMENTS`)
    uint8_t maxTextLength;  // Maximum length of the text part (`OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1`)
    uint8_t maxParts;  // Maximum number of parts per element (`CUSTOM_ELEMENTS_PARTS`)
} msp2InavCustomOsdElementsReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavCustomOsdElementsReply_t) == 3, msp2InavCustomOsdElementsReply_t_size);

// MSP2_INAV_CUSTOM_OSD_ELEMENT (MSPv2) id=8449
// Gets the configuration of a single custom OSD element defined by the programming framework.
// Notes: Reply emitted only if idx < MAX_CUSTOM_ELEMENTS; otherwise no body is written.
typedef struct MSP_PACKED {
    uint8_t elementIndex;  // Index of the custom element (0 to `MAX_CUSTOM_ELEMENTS - 1`)
} msp2InavCustomOsdElementRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavCustomOsdElementRequest_t) == 1, msp2InavCustomOsdElementRequest_t_size);

// MSP2_INAV_CUSTOM_OSD_ELEMENT (MSPv2) id=8449
// Gets the configuration of a single custom OSD element defined by the programming framework.
// Notes: Reply emitted only if idx < MAX_CUSTOM_ELEMENTS; otherwise no body is written.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        uint8_t partType;  // Type of this part | enum osdCustomElementType_e
        uint16_t partValue;  // Value/ID associated with this part
    } items[CUSTOM_ELEMENTS_PARTS];  // repeat: CUSTOM_ELEMENTS_PARTS
    uint8_t visibilityType;  // Visibility condition source | enum osdCustomElementTypeVisibility_e
    uint16_t visibilityValue;  // Value/ID of the visibility condition source
    char elementText[15];  // Static text bytes
} msp2InavCustomOsdElementReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavCustomOsdElementReply_t) == 27, msp2InavCustomOsdElementReply_t_size);

// MSP2_INAV_SET_CUSTOM_OSD_ELEMENTS (MSPv2) id=8450
// Sets the configuration of one custom OSD element.
// Notes: Payload length must be (OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1) + (CUSTOM_ELEMENTS_PARTS * 3) + 4 bytes including elementIndex. elementIndex must be < MAX_CUSTOM_ELEMENTS. Each partType must be < CUSTOM_ELEMENT_TYPE_END. Firmware NUL-terminates elementText internally.
typedef struct MSP_PACKED {
    uint8_t elementIndex;  // Index of the custom element (0 to `MAX_CUSTOM_ELEMENTS - 1`)
    struct MSP_PACKED {
        uint8_t partType;  // Type of this part | enum osdCustomElementType_e
        uint16_t partValue;  // Value/ID associated with this part
    } items[CUSTOM_ELEMENTS_PARTS];  // repeat: CUSTOM_ELEMENTS_PARTS
    uint8_t visibilityType;  // Visibility condition source | enum osdCustomElementTypeVisibility_e
    uint16_t visibilityValue;  // Value/ID of the visibility condition source
    char elementText[15];  // Raw bytes
} msp2InavSetCustomOsdElementsRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetCustomOsdElementsRequest_t) == 28, msp2InavSetCustomOsdElementsRequest_t_size);

// MSP2_INAV_GET_LINK_STATS (MSPv2) id=8451
// Provides uplink RC link statistics for monitoring on a GCS.
// Notes: Useful for GCS monitoring of the active RC link quality and signal margin.
typedef struct MSP_PACKED {
    uint8_t uplinkRSSI_dBm;  // Uplink RSSI in dBm, sent as a positive magnitude (`getRSSI()`). For example, 70 means -70dBm. | -dBm
    uint8_t uplinkLQ;  // Uplink Link Quality (`rxLinkStatistics.uplinkLQ`) | %
    int8_t uplinkSNR;  // Uplink Signal-to-Noise Ratio (`rxLinkStatistics.uplinkSNR`) | dB
} msp2InavGetLinkStatsReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavGetLinkStatsReply_t) == 3, msp2InavGetLinkStatsReply_t_size);

// MSP2_INAV_OUTPUT_MAPPING_EXT2 (MSPv2) id=8461
// Retrieves the full extended output mapping configuration (timer ID, full 32-bit usage flags, and pin label). Supersedes `MSP2_INAV_OUTPUT_MAPPING_EXT`.
// Notes: Provides complete usage flags and helps identify pins repurposed for functions like LED strip. Record count is not a constant: loops timerHardwareCount, skipping timers flagged TIM_USE_PPM or TIM_USE_PWM; read until the payload is exhausted.
// payload is a bare sequence of records (repeat: until_end); no fixed
// header, so no wrapper struct. count = payload_size / sizeof(msp2InavOutputMappingExt2ReplyElem_t)
typedef struct MSP_PACKED {
    uint8_t timerId;  // Hardware timer identifier (e.g., `TIM1`, `TIM2`). SITL uses index
    uint32_t usageFlags;  // Full 32-bit timer usage flags (`TIM_USE_*`)
    uint8_t pinLabel;  // Label for special pin usage (`PIN_LABEL_*` enum, e.g., `PIN_LABEL_LED`). 0 (`PIN_LABEL_NONE`) otherwise | enum pinLabel_e
} msp2InavOutputMappingExt2ReplyElem_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOutputMappingExt2ReplyElem_t) == 6, msp2InavOutputMappingExt2ReplyElem_t_size);

// MSP2_INAV_OUTPUT_ASSIGNMENT (MSPv2) id=8462
// Returns the finalized post-boot mapping of timer outputs to motors, servos and the beeper.
// Notes: Not available on SITL builds (`#ifndef SITL_BUILD`). The reply is 3 bytes per assigned output with no leading count field: motors first (`maxTimMotorCount`), then servos (`maxTimServoCount`), then at most one beeper record, emitted only when some timer override is set to `OUTPUT_MODE_BEEPER`. Reads the assignment finalized at boot via `pwmGetOutputAssignment()`. Record count is not a constant: maxTimMotorCount motors, then maxTimServoCount servos, then at most one beeper entry (the loop breaks on the first match); read until the payload is exhausted.
// payload is a bare sequence of records (repeat: until_end); no fixed
// header, so no wrapper struct. count = payload_size / sizeof(msp2InavOutputAssignmentReplyElem_t)
typedef struct MSP_PACKED {
    uint8_t outputIndex;  // Index into the target's `timerHardware[]` array for this output. | Index
    uint8_t usageType;  // Bit index of the `TIM_USE_*` flag, not the flag value itself: 2 = `TIM_USE_MOTOR`, 3 = `TIM_USE_SERVO`, 25 = `TIM_USE_BEEPER`. Derived in the firmware with `__builtin_ctz(TIM_USE_x)` and matching the `TIM_USE_*` constants in the configurator's `outputMapping.js`.
    uint8_t functionIndex;  // 1-based ordinal within the usage type: motor 1..n, servo 1..n. Always 1 for the beeper entry. | Index
} msp2InavOutputAssignmentReplyElem_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOutputAssignmentReplyElem_t) == 3, msp2InavOutputAssignmentReplyElem_t_size);

// MSP2_INAV_QUERY_OUTPUT_ASSIGNMENT (MSPv2) id=8463
// Previews the output assignment that would result from a proposed set of timer output-mode overrides, without applying them.
// Notes: Not available on SITL builds (`#ifndef SITL_BUILD`). Nothing is written to the configuration: `pwmCalculateAssignment()` is run against a proposed override array so a client can preview the effect of timer overrides before committing them with `MSP2_INAV_SET_TIMER_OUTPUT_MODE`. The reply has the same 3-byte record layout as `MSP2_INAV_OUTPUT_ASSIGNMENT`. Returns `MSP_RESULT_ERROR` if `timerCount` exceeds `HARDWARE_TIMER_DEFINITION_COUNT` or if the remaining request bytes are not exactly `timerCount * 2`. Pairs whose `timerId` is out of range are ignored rather than rejected. Record count is not a constant: maxTimMotorCount motors, then maxTimServoCount servos, then at most one beeper entry; read until the payload is exhausted.
// payload is a bare sequence of records (repeat: until_end); no fixed
// header, so no wrapper struct. count = payload_size / sizeof(msp2InavQueryOutputAssignment_dataSize_eq_0ReplyElem_t)
typedef struct MSP_PACKED {
    uint8_t outputIndex;  // Index into the target's `timerHardware[]` array for this output. | Index
    uint8_t usageType;  // Bit index of the `TIM_USE_*` flag, not the flag value itself: 2 = `TIM_USE_MOTOR`, 3 = `TIM_USE_SERVO`, 25 = `TIM_USE_BEEPER`. Derived in the firmware with `__builtin_ctz(TIM_USE_x)` and matching the `TIM_USE_*` constants in the configurator's `outputMapping.js`.
    uint8_t functionIndex;  // 1-based ordinal within the usage type: motor 1..n, servo 1..n. Always 1 for the beeper entry. | Index
} msp2InavQueryOutputAssignment_dataSize_eq_0ReplyElem_t;
MSP_STATIC_ASSERT(sizeof(msp2InavQueryOutputAssignment_dataSize_eq_0ReplyElem_t) == 3, msp2InavQueryOutputAssignment_dataSize_eq_0ReplyElem_t_size);

// MSP2_INAV_QUERY_OUTPUT_ASSIGNMENT (MSPv2) id=8463
// Previews the output assignment that would result from a proposed set of timer output-mode overrides, without applying them.
// Notes: Not available on SITL builds (`#ifndef SITL_BUILD`). Nothing is written to the configuration: `pwmCalculateAssignment()` is run against a proposed override array so a client can preview the effect of timer overrides before committing them with `MSP2_INAV_SET_TIMER_OUTPUT_MODE`. The reply has the same 3-byte record layout as `MSP2_INAV_OUTPUT_ASSIGNMENT`. Returns `MSP_RESULT_ERROR` if `timerCount` exceeds `HARDWARE_TIMER_DEFINITION_COUNT` or if the remaining request bytes are not exactly `timerCount * 2`. Pairs whose `timerId` is out of range are ignored rather than rejected. Record count is not a constant: maxTimMotorCount motors, then maxTimServoCount servos, then at most one beeper entry; read until the payload is exhausted.
typedef struct MSP_PACKED {
    uint8_t timerCount;  // Number of override pairs that follow. Must be <= `HARDWARE_TIMER_DEFINITION_COUNT`.
    struct MSP_PACKED {
        uint8_t timerId;  // Hardware timer index (0 to `HARDWARE_TIMER_DEFINITION_COUNT - 1`). Out-of-range values are silently skipped. | Index
        uint8_t outputMode;  // Proposed output mode override (`outputMode_e`) for that timer | enum outputMode_e
    } items[];  // repeat: timerCount
} msp2InavQueryOutputAssignment_dataSize_ge_1Request_t;
// variable length: sizeof(msp2InavQueryOutputAssignment_dataSize_ge_1Request_t) is the fixed header only

// MSP2_INAV_QUERY_OUTPUT_ASSIGNMENT (MSPv2) id=8463
// Previews the output assignment that would result from a proposed set of timer output-mode overrides, without applying them.
// Notes: Not available on SITL builds (`#ifndef SITL_BUILD`). Nothing is written to the configuration: `pwmCalculateAssignment()` is run against a proposed override array so a client can preview the effect of timer overrides before committing them with `MSP2_INAV_SET_TIMER_OUTPUT_MODE`. The reply has the same 3-byte record layout as `MSP2_INAV_OUTPUT_ASSIGNMENT`. Returns `MSP_RESULT_ERROR` if `timerCount` exceeds `HARDWARE_TIMER_DEFINITION_COUNT` or if the remaining request bytes are not exactly `timerCount * 2`. Pairs whose `timerId` is out of range are ignored rather than rejected. Record count is not a constant: maxTimMotorCount motors, then maxTimServoCount servos, then at most one beeper entry; read until the payload is exhausted.
// payload is a bare sequence of records (repeat: until_end); no fixed
// header, so no wrapper struct. count = payload_size / sizeof(msp2InavQueryOutputAssignment_dataSize_ge_1ReplyElem_t)
typedef struct MSP_PACKED {
    uint8_t outputIndex;  // Index into the target's `timerHardware[]` array for this output. | Index
    uint8_t usageType;  // Bit index of the `TIM_USE_*` flag, not the flag value itself: 2 = `TIM_USE_MOTOR`, 3 = `TIM_USE_SERVO`, 25 = `TIM_USE_BEEPER`. Derived in the firmware with `__builtin_ctz(TIM_USE_x)` and matching the `TIM_USE_*` constants in the configurator's `outputMapping.js`.
    uint8_t functionIndex;  // 1-based ordinal within the usage type: motor 1..n, servo 1..n. Always 1 for the beeper entry. | Index
} msp2InavQueryOutputAssignment_dataSize_ge_1ReplyElem_t;
MSP_STATIC_ASSERT(sizeof(msp2InavQueryOutputAssignment_dataSize_ge_1ReplyElem_t) == 3, msp2InavQueryOutputAssignment_dataSize_ge_1ReplyElem_t_size);

// MSP2_INAV_OSD_UPDATE_POSITION (MSPv2) id=8472
// Moves a single OSD item within the active layout and redraws it immediately.
// Notes: Requires `USE_OSD`. Expects 3 bytes; returns `MSP_RESULT_ERROR` if fewer are supplied or if `itemIndex >= OSD_ITEM_COUNT`, otherwise `MSP_RESULT_ACK`. Writes to the currently active layout (`getCurrentLayout()`) and takes no layout argument; use `MSP2_INAV_OSD_SET_LAYOUT_ITEM` to address a specific layout. Erases the item at its old position and redraws it immediately rather than triggering a full OSD redraw; the erase step only clears custom elements (items 147-149 and 154-158), so moving other item types can leave the old glyphs on screen until the next full redraw.
typedef struct MSP_PACKED {
    uint8_t itemIndex;  // OSD item index (0 to `OSD_ITEM_COUNT - 1`) | Index
    uint16_t itemPosition;  // Packed X/Y position built with `OSD_POS(x, y)`. The firmware ORs in `OSD_VISIBLE_FLAG` (0x2000), so the item is always made visible regardless of the bit supplied. | Coordinates
} msp2InavOsdUpdatePositionRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavOsdUpdatePositionRequest_t) == 3, msp2InavOsdUpdatePositionRequest_t_size);

// MSP2_INAV_SERVO_CONFIG (MSPv2) id=8704
// Retrieves the configuration parameters for all supported servos (min, max, middle, rate). Supersedes `MSP_SERVO_CONFIGURATIONS`.
typedef struct MSP_PACKED {
    struct MSP_PACKED {
        int16_t min;  // Minimum servo endpoint (`servoParams(i)->min`) | PWM
        int16_t max;  // Maximum servo endpoint (`servoParams(i)->max`) | PWM
        int16_t middle;  // Middle/Neutral servo position (`servoParams(i)->middle`) | PWM
        int8_t rate;  // Servo rate/scaling (`servoParams(i)->rate`) | % (-125 to 125)
    } items[MAX_SUPPORTED_SERVOS];  // repeat: MAX_SUPPORTED_SERVOS
} msp2InavServoConfigReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavServoConfigReply_t) == 126, msp2InavServoConfigReply_t_size);

// MSP2_INAV_SET_SERVO_CONFIG (MSPv2) id=8705
// Sets the configuration parameters for a single servo. Supersedes `MSP_SET_SERVO_CONFIGURATION`.
// Notes: Expects 8 bytes. Returns error if index invalid. Calls `servoComputeScalingFactors()`.
typedef struct MSP_PACKED {
    uint8_t servoIndex;  // Index of the servo to configure (0 to `MAX_SUPPORTED_SERVOS - 1`) | Index
    int16_t min;  // Sets minimum servo endpoint | PWM
    int16_t max;  // Sets maximum servo endpoint | PWM
    int16_t middle;  // Sets middle/neutral servo position | PWM
    int8_t rate;  // Sets servo rate/scaling | % (-125 to 125)
} msp2InavSetServoConfigRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetServoConfigRequest_t) == 8, msp2InavSetServoConfigRequest_t_size);

// MSP2_INAV_GEOZONE (MSPv2) id=8720
// Get configuration for a specific Geozone.
// Notes: Requires `USE_GEOZONE`. Used by `mspFcGeozoneOutCommand`.
typedef struct MSP_PACKED {
    uint8_t geozoneIndex;  // Index of the geozone (0 to `MAX_GEOZONES_IN_CONFIG - 1`)
} msp2InavGeozoneRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavGeozoneRequest_t) == 1, msp2InavGeozoneRequest_t_size);

// MSP2_INAV_GEOZONE (MSPv2) id=8720
// Get configuration for a specific Geozone.
// Notes: Requires `USE_GEOZONE`. Used by `mspFcGeozoneOutCommand`.
typedef struct MSP_PACKED {
    uint8_t geozoneIndex;  // Index requested
    uint8_t type;  // Define (`GEOZONE_TYPE_EXCLUSIVE/INCLUSIVE`): Zone type (Inclusion/Exclusion)
    uint8_t shape;  // Define (`GEOZONE_SHAPE_CIRCULAR/POLYGON`): Zone shape (Polygon/Circular)
    int32_t minAltitude;  // Minimum allowed altitude within the zone (`geoZonesConfig(idx)->minAltitude`) | cm
    int32_t maxAltitude;  // Maximum allowed altitude within the zone (`geoZonesConfig(idx)->maxAltitude`) | cm
    uint8_t isSeaLevelRef;  // Boolean: 1 if altitudes are relative to sea level, 0 if relative to home
    uint8_t fenceAction;  // Enum (`fenceAction_e`): Action to take upon boundary violation | enum fenceAction_e
    uint8_t vertexCount;  // Number of vertices defined for this zone
} msp2InavGeozoneReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavGeozoneReply_t) == 14, msp2InavGeozoneReply_t_size);

// MSP2_INAV_SET_GEOZONE (MSPv2) id=8721
// Sets the main configuration for a specific Geozone (type, shape, altitude, action). **This command resets (clears) all vertices associated with the zone.**
// Notes: Requires `USE_GEOZONE`. Expects 14 bytes. Returns error if index invalid. Calls `geozoneResetVertices()`. Vertices must be set subsequently using `MSP2_INAV_SET_GEOZONE_VERTEX`.
typedef struct MSP_PACKED {
    uint8_t geozoneIndex;  // Index of the geozone (0 to `MAX_GEOZONES_IN_CONFIG - 1`)
    uint8_t type;  // Define (`GEOZONE_TYPE_EXCLUSIVE/INCLUSIVE`): Zone type (Inclusion/Exclusion)
    uint8_t shape;  // Define (`GEOZONE_SHAPE_CIRCULAR/POLYGON`): Zone shape (Polygon/Circular)
    int32_t minAltitude;  // Minimum allowed altitude (`geoZonesConfigMutable()->minAltitude`) | cm
    int32_t maxAltitude;  // Maximum allowed altitude (`geoZonesConfigMutable()->maxAltitude`) | cm
    uint8_t isSeaLevelRef;  // Boolean: Altitude reference
    uint8_t fenceAction;  // Enum (`fenceAction_e`): Action to take upon boundary violation | enum fenceAction_e
    uint8_t vertexCount;  // Number of vertices to be defined (used for validation later)
} msp2InavSetGeozoneRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetGeozoneRequest_t) == 14, msp2InavSetGeozoneRequest_t_size);

// MSP2_INAV_GEOZONE_VERTEX (MSPv2) id=8722
// Get a specific vertex (or center+radius for circular zones) of a Geozone.
// Notes: Requires `USE_GEOZONE`. Returns error if indexes are invalid or vertex doesn't exist. For circular zones, the radius is stored internally as the 'latitude' of the vertex with index 1.
typedef struct MSP_PACKED {
    uint8_t geozoneIndex;  // Index of the geozone
    uint8_t vertexId;  // Index of the vertex within the zone (0-based). For circles, 0 = center
} msp2InavGeozoneVertexRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavGeozoneVertexRequest_t) == 2, msp2InavGeozoneVertexRequest_t_size);

// MSP2_INAV_GEOZONE_VERTEX (MSPv2) id=8722
// Get a specific vertex (or center+radius for circular zones) of a Geozone.
// Notes: Requires `USE_GEOZONE`. Returns error if indexes are invalid or vertex doesn't exist. For circular zones, the radius is stored internally as the 'latitude' of the vertex with index 1.
typedef struct MSP_PACKED {
    uint8_t geozoneIndex;  // Geozone index requested | Index
    uint8_t vertexId;  // Vertex index requested | Index
    int32_t latitude;  // Vertex latitude | deg * 1e7
    int32_t longitude;  // Vertex longitude | deg * 1e7
    int32_t radius;  // If vertex is circle, Radius of the circular zone | cm | OPTIONAL: may be absent from a shorter payload
} msp2InavGeozoneVertexReply_t;

// MSP2_INAV_SET_GEOZONE_VERTEX (MSPv2) id=8723
// Sets a specific vertex (or center+radius for circular zones) for a Geozone.
// Notes: Requires `USE_GEOZONE`. Expects 10 bytes (Polygon) or 14 bytes (Circular). Returns error if indexes invalid or if trying to set vertex beyond `vertexCount` defined in `MSP2_INAV_SET_GEOZONE`. Calls `geozoneSetVertex()`. For circular zones, sets center (vertex 0) and radius (vertex 1's latitude).
typedef struct MSP_PACKED {
    uint8_t geozoneIndex;  // Geozone index requested | Index
    uint8_t vertexId;  // Vertex index requested | Index
    int32_t latitude;  // Vertex latitude | deg * 1e7
    int32_t longitude;  // Vertex longitude | deg * 1e7
} msp2InavSetGeozoneVertex_polygonRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetGeozoneVertex_polygonRequest_t) == 10, msp2InavSetGeozoneVertex_polygonRequest_t_size);

// MSP2_INAV_SET_GEOZONE_VERTEX (MSPv2) id=8723
// Sets a specific vertex (or center+radius for circular zones) for a Geozone.
// Notes: Requires `USE_GEOZONE`. Expects 10 bytes (Polygon) or 14 bytes (Circular). Returns error if indexes invalid or if trying to set vertex beyond `vertexCount` defined in `MSP2_INAV_SET_GEOZONE`. Calls `geozoneSetVertex()`. For circular zones, sets center (vertex 0) and radius (vertex 1's latitude).
typedef struct MSP_PACKED {
    uint8_t geozoneIndex;  // Geozone index requested | Index
    uint8_t vertexId;  // Vertex index requested | Index
    int32_t latitude;  // Vertex/Center latitude | deg * 1e7
    int32_t longitude;  // Vertex/Center longitude | deg * 1e7
    int32_t radius;  // Radius of the circular zone | cm
} msp2InavSetGeozoneVertex_circleRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetGeozoneVertex_circleRequest_t) == 14, msp2InavSetGeozoneVertex_circleRequest_t_size);

// MSP2_INAV_SET_GVAR (MSPv2) id=8724
// Sets the specified Global Variable (GVAR) to the provided value.
// Notes: Requires `USE_PROGRAMMING_FRAMEWORK`. Expects 5 bytes. Returns error if index is outside `MAX_GLOBAL_VARIABLES`.
typedef struct MSP_PACKED {
    uint8_t gvarIndex;  // Index of the Global Variable to set | Index
    int32_t value;  // New value to store (clamped to configured min/max by `gvSet()`)
} msp2InavSetGvarRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetGvarRequest_t) == 5, msp2InavSetGvarRequest_t_size);

// MSP2_INAV_SET_ALT_TARGET (MSPv2) id=8725
// Set the active altitude hold target using updateClimbRateToAltitudeController.
// Notes: Set new altitude target. Requires 5-byte payload (datum + target) and is set-only. Valid only in NAV or ALTHOLD modes. Command is rejected unless altitude control is active, not landing/emergency landing, altitude estimation is valid, and datum is supported (MSL requires valid GPS origin; TERRAIN is reserved and rejected).
typedef struct MSP_PACKED {
    uint8_t altitudeDatum;  // Altitude reference datum flag (`geoAltitudeDatumFlag_e`): `NAV_WP_TAKEOFF_DATUM` (default), `NAV_WP_MSL_DATUM`, `NAV_WP_TERRAIN_DATUM` and `NAV_WP_RELATIVE_DATUM` (not implemented yet) | enum geoAltitudeDatumFlag_e
    int32_t altitudeTarget;  // Desired altitude target according to reference datum | cm
} msp2InavSetAltTargetRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetAltTargetRequest_t) == 5, msp2InavSetAltTargetRequest_t_size);

// MSP2_INAV_FLIGHT_AXIS_ANGLE_OVERRIDE (MSPv2) id=8726
// Enables or disables a flight-axis angle override for the selected axis.
// Notes: Uses the same override path as logic conditions and bypasses stick-derived angle targets.
typedef struct MSP_PACKED {
    uint8_t overrideMask;  // Bitmask of desired-state fields that follow (Roll, Pitch, Yaw). Non-zero enables the override; zero disables it for that axis. | bitmask
    int16_t angleTargetRoll;  // Angle target in deci-degrees. Roll/Pitch clamped to configured angle limits | deci-degrees
    int16_t angleTargetPitch;  // Angle target in deci-degrees. Roll/Pitch clamped to configured angle limits | deci-degrees
    int16_t angleTargetYaw;  // Angle target in deci-degrees. Yaw clamped to 0–3600. | deci-degrees
} msp2InavFlightAxisAngleOverrideRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavFlightAxisAngleOverrideRequest_t) == 7, msp2InavFlightAxisAngleOverrideRequest_t_size);

// MSP2_INAV_FLIGHT_AXIS_RATE_OVERRIDE (MSPv2) id=8727
// Enables or disables a flight-axis rate override for the selected axis.
// Notes: Expects 7 bytes. Overrides rate targets just before control is applied, bypassing stick-derived setpoints.
typedef struct MSP_PACKED {
    uint8_t overrideMask;  // Bitmask of desired-state fields that follow (Roll, Pitch, Yaw). Non-zero enables the override; zero disables it for that axis. | bitmask
    int16_t rateTargetRoll;  // Rate target, clamped to ±2000 | deg/s
    int16_t rateTargetPitch;  // Rate target, clamped to ±2000 | deg/s
    int16_t rateTargetYaw;  // Rate target, clamped to ±2000 | deg/s
} msp2InavFlightAxisRateOverrideRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavFlightAxisRateOverrideRequest_t) == 7, msp2InavFlightAxisRateOverrideRequest_t_size);

// MSP2_INAV_SET_LOCAL_TARGET (MSPv2) id=8728
// Sets a body-frame offset target relative to the current vehicle position.
// Notes: Offsets are in the vehicle body frame (forward/right/up, cm) and are rotated into the NEU frame using the current yaw, applied relative to current position. Z offset is always provided; Z=0 keeps current altitude, non-zero offsets are relative to current altitude. Requires GCSNAV/offboard to be active and a valid guided poshold; updates the navigation desired position via `setDesiredPosition()`.
typedef struct MSP_PACKED {
    int32_t offsetForward;  // Body-frame forward offset from the current position, rotated into NEU by the current yaw | cm
    int32_t offsetRight;  // Body-frame right offset from the current position, rotated into NEU by the current yaw | cm
    int32_t offsetUp;  // Offset above the current altitude (up-positive). 0 keeps the current altitude | cm
} msp2InavSetLocalTargetRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetLocalTargetRequest_t) == 12, msp2InavSetLocalTargetRequest_t_size);

// MSP2_INAV_LOCAL_TARGET (MSPv2) id=8729
// Returns the current navigation desired state (position, velocity, yaw, and climb rate).
// Notes: Local frame is NEU. Mirrors `posControl.desiredState` (position, velocity, yaw, climb rate) used by the position controller.
typedef struct MSP_PACKED {
    int32_t posX;  // Desired X in local NEU frame (`posControl.desiredState.pos.x`) | cm
    int32_t posY;  // Desired Y in local NEU frame (`posControl.desiredState.pos.y`) | cm
    int32_t posZ;  // Desired Z in local NEU frame (`posControl.desiredState.pos.z`, up-positive) | cm
    int16_t velX;  // Desired X velocity (`posControl.desiredState.vel.x`) | cm/s
    int16_t velY;  // Desired Y velocity (`posControl.desiredState.vel.y`) | cm/s
    int16_t velZ;  // Desired Z velocity (`posControl.desiredState.vel.z`) | cm/s
    int32_t yaw;  // Desired heading (`posControl.desiredState.yaw`) | centi-degrees
    int16_t climbRate;  // Desired climb rate demand (`posControl.desiredState.climbRateDemand`) | cm/s
} msp2InavLocalTargetReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavLocalTargetReply_t) == 24, msp2InavLocalTargetReply_t_size);

// MSP2_INAV_SET_GLOBAL_TARGET (MSPv2) id=8730
// Sets desired GCS Nav position with global coordinates (WP 254/GOTO).
// Notes: Uses the GCSNAV/offboard path; rejected when GCSNAV is not active. Rejects `NAV_WP_TERRAIN_DATUM`; other datums are converted to local NEU and applied through `setDesiredPosition()`. Altitude of 0 leaves current Z unchanged. Existing 13-byte payloads are still accepted; 17-byte payloads append `loiterRadius`, where `0` clears the temporary override and non-zero values are centimeters.
typedef struct MSP_PACKED {
    int32_t latitude;  // Latitude coordinate | deg * 1e7
    int32_t longitude;  // Longitude coordinate | deg * 1e7
    int32_t altitudeTarget;  // Desired altitude target according to reference datum (0 keeps current altitude) | cm
    uint8_t altitudeDatum;  // Altitude reference datum flag (`geoAltitudeDatumFlag_e`): `NAV_WP_TAKEOFF_DATUM`, `NAV_WP_MSL_DATUM`, `NAV_WP_TERRAIN_DATUM` (not implemented yet) | enum geoAltitudeDatumFlag_e
    int32_t loiterRadius;  // Optional temporary fixed-wing PosHold loiter radius override. Appended field; omit to leave unchanged. `0` clears the override and uses `navConfig()->fw.loiter_radius`. | cm | OPTIONAL: may be absent from a shorter payload
} msp2InavSetGlobalTargetRequest_t;

// MSP2_INAV_NAV_TARGET (MSPv2) id=8731
// Returns the current navigation desired global target (lat/lon/alt, heading, climb rate).
// Notes: Altitude target is reported in the takeoff datum frame (local Z). Heading is sourced from the heading-hold target. Intended for monitoring the active navigation desired target (Goto/Followme/RTH/Safehome). The appended `loiterRadius` reports the temporary override only; `0` means the configured default is active.
typedef struct MSP_PACKED {
    int32_t latTarget;  // Latitude in degrees * 1e7 | 1e-7 deg
    int32_t lonTarget;  // Longitude in degrees * 1e7 | 1e-7 deg
    int32_t altitudeTarget;  // Desired altitude target (takeoff datum, cm) as used by altitude/position hold | cm
    uint16_t headingTarget;  // Current heading-hold target (`getHeadingHoldTarget()`), wrapped to 0–359.99 | degrees
    int16_t climbRate;  // Desired climb rate demand (`posControl.desiredState.climbRateDemand`) | cm/s
    uint32_t loiterRadius;  // Temporary fixed-wing PosHold loiter radius override. `0` means no override; the configured `navConfig()->fw.loiter_radius` is used. | cm
} msp2InavNavTargetReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavNavTargetReply_t) == 20, msp2InavNavTargetReply_t_size);

// MSP2_INAV_FULL_LOCAL_POSE (MSPv2) id=8736
// Provides estimates of current attitude, local NEU position, and velocity.
// Notes: All attitude angles are in deci-degrees.
typedef struct MSP_PACKED {
    int16_t roll;  // Roll angle (`attitude.values.roll`) | deci-degrees
    int16_t pitch;  // Pitch angle (`attitude.values.pitch`) | deci-degrees
    int16_t yaw;  // Yaw/Heading angle (`attitude.values.yaw`) | deci-degrees
    int32_t localPositionNorth;  // Estimated North coordinate in local NEU frame (`posControl.actualState.abs.pos.x`) | cm
    int16_t localVelocityNorth;  // Estimated North component of velocity in local NEU frame (`posControl.actualState.abs.vel.x`) | cm/s
    int32_t localPositionEast;  // Estimated East coordinate in local NEU frame (`posControl.actualState.abs.pos.y`) | cm
    int16_t localVelocityEast;  // Estimated East component of velocity in local NEU frame (`posControl.actualState.abs.vel.y`) | cm/s
    int32_t localPositionUp;  // Estimated Up coordinate in local NEU frame (`posControl.actualState.abs.pos.z`) | cm
    int16_t localVelocityUp;  // Estimated Up component of velocity in local NEU frame (`posControl.actualState.abs.vel.z`) | cm/s
} msp2InavFullLocalPoseReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavFullLocalPoseReply_t) == 24, msp2InavFullLocalPoseReply_t_size);

// MSP2_INAV_SET_WP_INDEX (MSPv2) id=8737
// Jumps to a specific waypoint during an active waypoint mission, causing the aircraft to immediately begin navigating toward the new target waypoint.
// Notes: Returns error if the aircraft is not armed, `NAV_WP_MODE` is not active, or the index is outside the valid mission range (`startWpIndex` to `startWpIndex + waypointCount - 1`). On success, sets `posControl.activeWaypointIndex` to the requested index and fires `NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_JUMP`, transitioning the navigation FSM back to `NAV_STATE_WAYPOINT_PRE_ACTION` so the flight controller re-initialises navigation for the new target.
typedef struct MSP_PACKED {
    uint8_t wp_index;  // 0-based waypoint index to jump to, relative to the mission start waypoint (`posControl.startWpIndex`) | -
} msp2InavSetWpIndexRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetWpIndexRequest_t) == 1, msp2InavSetWpIndexRequest_t_size);

// MSP2_INAV_SET_CRUISE_HEADING (MSPv2) id=8739
// Sets the course heading target while Cruise or Course Hold mode is active, causing the aircraft to turn to and maintain the new heading.
// Notes: Returns error if the aircraft is not armed or `NAV_COURSE_HOLD_MODE` is not active. On success, sets both `posControl.cruise.course` and `posControl.cruise.previousCourse` to the normalised value, preventing spurious heading adjustments from `getCruiseHeadingAdjustment()` on the next control cycle.
typedef struct MSP_PACKED {
    int32_t heading_centidegrees;  // Target heading in centidegrees (0-35999). Values are wrapped modulo 36000 before being applied. | centidegrees
} msp2InavSetCruiseHeadingRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavSetCruiseHeadingRequest_t) == 4, msp2InavSetCruiseHeadingRequest_t_size);

// MSP2_INAV_ARM_DISARM (MSPv2) id=8743
// Arms or disarms the flight controller using the normal FC arming path.
// Notes: Returns an error for values other than 0 or 1, or when the requested armed state is not reached.
typedef struct MSP_PACKED {
    uint8_t arm;  // Requested armed state: 0 disarms, 1 arms through the normal arming checks. | Boolean
} msp2InavArmDisarmRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavArmDisarmRequest_t) == 1, msp2InavArmDisarmRequest_t_size);

// MSP2_INAV_TIMESYNC (MSPv2) id=8744
// Returns the local monotonic boot time in nanoseconds.
// Notes: The value is little-endian like other MSP integer fields and uses the same boot-time clock returned by MAVLink `TIMESYNC`.
typedef struct MSP_PACKED {
    uint64_t timeNs;  // Monotonic flight-controller boot time, calculated as `(uint64_t)micros() * 1000`. | ns
} msp2InavTimesyncReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavTimesyncReply_t) == 8, msp2InavTimesyncReply_t_size);

// MSP2_INAV_SET_AUX_RC (MSPv2) id=8752
// Bandwidth-efficient auxiliary RC channel update. Sets CH13-CH32 with configurable resolution (2/4/8/16-bit) without affecting primary flight controls. Designed for extending channel count beyond native RC link capacity via MSP passthrough.
// Notes: CH1-CH12 (index 0-11) are protected and will return `MSP_RESULT_ERROR`. Payload size must be 2-49 bytes. Constraint: `startChannel + channelCount <= 32`. Values persist until overwritten; no timeout. Applied as a post-RX overlay in `calculateRxChannelsAndUpdateFailsafe()` after MSP RC Override but before failsafe. Does not require `USE_RX_MSP` or MSP-RC-OVERRIDE flight mode. Does not affect failsafe detection. When MSP is the primary RX provider, channels covered by `MSP_SET_RAW_RC` are automatically skipped. Channels in the `mspOverrideChannels` bitmask are skipped when MSP RC Override mode is active. Recommended to send with `MSP_FLAG_DONT_REPLY` (flags=0x01) to save bandwidth on telemetry passthrough links. 16-bit mode requires even number of data bytes and values are clamped to 750-2250us.
typedef struct MSP_PACKED {
    uint8_t definitionByte;  // Packed start channel and resolution. Bits 7-3: start channel index (valid range 12-31 for CH13-CH32; 0-11 rejected as error). Bits 2-0: resolution mode (0=2-bit, 1=4-bit, 2=8-bit, 3=16-bit; 4-7 reserved/error).
    uint8_t channelData[];  // Packed channel values, sequential from start channel. Number of channels is derived from data size and resolution. Value 0 means skip (no update). Sub-byte modes (2-bit, 4-bit) are packed MSB-first. 2-bit values 1-3 map to 1000/1500/2000us. 4-bit values 1-15 map to 1000 + (val-1)*1000/14 us. 8-bit values 1-255 map to 1000 + (val-1)*1000/254 us. 16-bit values are direct PWM, clamped to 750-2250us. | PWM (encoded)
} msp2InavSetAuxRcRequest_t;
// variable length: sizeof(msp2InavSetAuxRcRequest_t) is the fixed header only

// MSP2_INAV_WIND (MSPv2) id=8753
// Retrieves the estimated horizontal wind speed and direction from the internal wind estimator.
// Notes: Requires `USE_WIND_ESTIMATOR`; returns zeroes when wind estimation is not compiled in or not yet valid. Check bit 0 of `flags` before using speed/angle values.
typedef struct MSP_PACKED {
    uint16_t windSpeed;  // Estimated horizontal wind speed (`getEstimatedHorizontalWindSpeed()`). 0 if unavailable. | cm/s
    uint16_t windAngle;  // Estimated wind direction in degrees (0–359, 0 = North). Derived from centidegree value divided by 100. 0 if unavailable. | degrees
    uint8_t flags;  // Validity flags. Bit 0: wind estimate valid (`isEstimatedWindSpeedValid()`). Remaining bits reserved.
} msp2InavWindReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavWindReply_t) == 5, msp2InavWindReply_t_size);

// MSP2_INAV_MAG_UNALIGNED (MSPv2) id=8754
// Reads the unaligned magnetometer vector.
// Notes: Returns rounded mag.magADCUnaligned values before board alignment. Returns three zeroes without USE_MAG.
typedef struct MSP_PACKED {
    int16_t magADCUnaligned[3];  // X, Y and Z components, rounded to signed 16-bit values.
} msp2InavMagUnalignedReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavMagUnalignedReply_t) == 6, msp2InavMagUnalignedReply_t_size);

// MSP2_INAV_ESC_SRXL2_STATUS (MSPv2) id=8755
// Reads SRXL2 ESC calibration and connection status.
// Notes: Requires USE_MOTOR_SRXL2. Counts report opened motor ports and the current mixer motor count, not hardware capacity.
typedef struct MSP_PACKED {
    uint8_t phase;  // Calibration phase (srxl2CalPhase_e).
    uint8_t connected;  // 1 when every opened ESC is connected; otherwise 0.
    uint8_t lastResult;  // Last calibration start result (srxl2CalResult_e).
    uint8_t portCount;  // Number of opened SRXL2 motor ports.
    uint8_t motorCount;  // Number of motors in the current mixer.
} msp2InavEscSrxl2StatusReply_t;
MSP_STATIC_ASSERT(sizeof(msp2InavEscSrxl2StatusReply_t) == 5, msp2InavEscSrxl2StatusReply_t_size);

// MSP2_INAV_ESC_SRXL2_CALIBRATE (MSPv2) id=8756
// Controls SRXL2 ESC throttle-range calibration.
// Notes: Requires USE_MOTOR_SRXL2 and at least one request byte. Accepted commands: 0 abort, 1 automatic start, 4 manual high, 5 manual low. Driver safety checks can reject start requests; read MSP2_INAV_ESC_SRXL2_STATUS for the reason. Other command values return an MSP error. Remove propellers before calibration.
typedef struct MSP_PACKED {
    uint8_t phase;  // Requested calibration action (srxl2CalPhase_e): 0, 1, 4 or 5.
} msp2InavEscSrxl2CalibrateRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2InavEscSrxl2CalibrateRequest_t) == 1, msp2InavEscSrxl2CalibrateRequest_t_size);

// MSP2_RX_BIND (MSPv2) id=12289
// Initiates binding for MSP receivers (mLRS).
// Notes: Requires a receiver using MSP as the protocol, sends MSP2_RX_BIND to the receiver.
typedef struct MSP_PACKED {
    uint8_t port_id;  // Port ID
    uint8_t reserved_for_custom_use[3];  // Reserved for custom use
} msp2RxBindRequest_t;
MSP_STATIC_ASSERT(sizeof(msp2RxBindRequest_t) == 4, msp2RxBindRequest_t_size);

// MSP2_RX_BIND (MSPv2) id=12289
// Initiates binding for MSP receivers (mLRS).
// Notes: Requires a receiver using MSP as the protocol, sends MSP2_RX_BIND to the receiver.
typedef struct MSP_PACKED {
    uint8_t port_id;  // Port ID
    uint8_t reserved_for_custom_use[3];  // Reserved for custom use
} msp2RxBindReply_t;
MSP_STATIC_ASSERT(sizeof(msp2RxBindReply_t) == 4, msp2RxBindReply_t_size);


#if defined(_MSC_VER)
#  pragma pack(pop)
#endif
#undef MSP_PACKED
