
# WARNING: DEPRECATED, OBSOLETE, FULL OF ERRORS, DO NOT USE AS REFERENCE!!!
# (OBSOLETE) INAV MSP Messages reference

**Warning: Work in progress**\
**Generated with Gemini 2.5 Pro Preview O3-25 on source code files**\
**Verification needed, exercise caution until completely verified for accuracy and cleared**\
**Refer to source for absolute certainty**

For details on the structure of MSP, see [The wiki page](https://github.com/iNavFlight/inav/wiki/MSP-V2)


**Basic Concepts:**

*   **MSP Versions:**
    *   **MSPv1:** The original protocol. Uses command IDs from 0 to 254.
    *   **MSPv2:** An extended version. Uses command IDs from 0x1000 onwards. Can be encapsulated within an MSPv1 frame (`MSP_V2_FRAME` ID 255) or used natively.
*   **Direction:**
    *   **Out:** Message sent *from* the Flight Controller (FC) *to* the Ground Control Station (GCS), OSD, or other peripheral. Usually a request for data or status.
    *   **In:** Message sent *from* the GCS/OSD *to* the FC. Usually a command to set a parameter, perform an action, or provide data to the FC.
    *   **In/Out:** Can function in both directions, often used for getting/setting related data where the request might specify a subset (e.g., get specific waypoint, get specific setting info).
*   **Payload:** The data carried by the message, following the command ID. The structure (order, type, size of fields) is critical.
*   **Data Types:** Common C data types are used (`uint8_t`, `int16_t`, `uint32_t`, `float`, etc.). Pay close attention to signed vs. unsigned types and sizes.
*   **Packing:** Data fields are packed sequentially in the order listed. `__attribute__((packed))` is often used in struct definitions to prevent compiler padding.

---

## MSPv1 Core & Versioning Commands (0-5)

### `MSP_API_VERSION` (1 / 0x01)

*   **Direction:** Out
*   **Description:** Provides the MSP protocol version and the INAV API version.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `mspProtocolVersion` | `uint8_t` | 1 | MSP Protocol version (`MSP_PROTOCOL_VERSION`, typically 0) |
    | `apiVersionMajor` | `uint8_t` | 1 | INAV API Major version (`API_VERSION_MAJOR`) |
    | `apiVersionMinor` | `uint8_t` | 1 | INAV API Minor version (`API_VERSION_MINOR`) |
*   **Notes:** Used by configurators to check compatibility.

### `MSP_FC_VARIANT` (2 / 0x02)

*   **Direction:** Out
*   **Description:** Identifies the flight controller firmware variant (e.g., INAV, Betaflight).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `fcVariantIdentifier` | `char[4]` | 4 | 4-character identifier string (e.g., "INAV"). Defined by `flightControllerIdentifier` |
*   **Notes:** See `FLIGHT_CONTROLLER_IDENTIFIER_LENGTH`.

### `MSP_FC_VERSION` (3 / 0x03)

*   **Direction:** Out
*   **Description:** Provides the specific version number of the flight controller firmware.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `fcVersionMajor` | `uint8_t` | 1 | Firmware Major version (`FC_VERSION_MAJOR`) |
    | `fcVersionMinor` | `uint8_t` | 1 | Firmware Minor version (`FC_VERSION_MINOR`) |
    | `fcVersionPatch` | `uint8_t` | 1 | Firmware Patch level (`FC_VERSION_PATCH_LEVEL`) |

### `MSP_BOARD_INFO` (4 / 0x04)

*   **Direction:** Out
*   **Description:** Provides information about the specific hardware board and its capabilities.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `boardIdentifier` | `char[4]` | 4 | 4-character UPPER CASE board identifier (`TARGET_BOARD_IDENTIFIER`) |
    | `hardwareRevision` | `uint16_t` | 2 | Hardware revision number. 0 if not detected (`USE_HARDWARE_REVISION_DETECTION`) |
    | `osdSupport` | `uint8_t` | 1 | OSD chip type: 0=None, 2=Onboard (`USE_OSD`). INAV does not support slave OSD (1) |
    | `commCapabilities` | `uint8_t` | 1 | Communication capabilities bitmask: Bit 0=VCP support (`USE_VCP`), Bit 1=SoftSerial support (`USE_SOFTSERIAL1`/`2`) |
    | `targetNameLength` | `uint8_t` | 1 | Length of the target name string that follows |
    | `targetName` | `char[]` | Variable | Target name string (e.g., "MATEKF405"). Length given by previous field |
*   **Notes:** `BOARD_IDENTIFIER_LENGTH` is 4.

### `MSP_BUILD_INFO` (5 / 0x05)

*   **Direction:** Out
*   **Description:** Provides build date, time, and Git revision of the firmware.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `buildDate` | `char[11]` | 11 | Build date string (e.g., "Dec 31 2023"). `BUILD_DATE_LENGTH` |
    | `buildTime` | `char[8]` | 8 | Build time string (e.g., "23:59:59"). `BUILD_TIME_LENGTH` |
    | `gitRevision` | `char[7]` | 7 | Short Git revision string. `GIT_SHORT_REVISION_LENGTH` |

---

## MSPv1 INAV Configuration Commands (6-24)

### `MSP_INAV_PID` (6 / 0x06)

*   **Direction:** Out
*   **Description:** Retrieves legacy INAV-specific PID controller related settings. Many fields are now obsolete or placeholders.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `legacyAsyncProcessing` | `uint8_t` | 1 | - | Legacy, unused. Always 0 |
    | `legacyAsyncValue1` | `uint16_t` | 2 | - | Legacy, unused. Always 0 |
    | `legacyAsyncValue2` | `uint16_t` | 2 | - | Legacy, unused. Always 0 |
    | `headingHoldRateLimit` | `uint8_t` | 1 | deg/s | Max rate for heading hold P term (`pidProfile()->heading_hold_rate_limit`) |
    | `headingHoldLpfFreq` | `uint8_t` | 1 | Hz | Fixed LPF frequency for heading hold error (`HEADING_HOLD_ERROR_LPF_FREQ`) |
    | `legacyYawJumpLimit` | `uint16_t` | 2 | - | Legacy, unused. Always 0 |
    | `legacyGyroLpf` | `uint8_t` | 1 | Hz | Fixed value `GYRO_LPF_256HZ` |
    | `accLpfHz` | `uint8_t` | 1 | Hz | Accelerometer LPF frequency (`accelerometerConfig()->acc_lpf_hz`) cutoff frequency for the low pass filter used on the acc z-axis for althold in Hz |
    | `reserved1` | `uint8_t` | 1 | - | Reserved. Always 0 |
    | `reserved2` | `uint8_t` | 1 | - | Reserved. Always 0 |
    | `reserved3` | `uint8_t` | 1 | - | Reserved. Always 0 |
    | `reserved4` | `uint8_t` | 1 | - | Reserved. Always 0 |
*   **Notes:** Superseded by `MSP2_PID` for core PIDs and other specific messages for filter settings.

### `MSP_SET_INAV_PID` (7 / 0x07)

*   **Direction:** In
*   **Description:** Sets legacy INAV-specific PID controller related settings.
*   **Payload:** (Matches `MSP_INAV_PID` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `legacyAsyncProcessing` | `uint8_t` | 1 | - | Legacy, ignored |
    | `legacyAsyncValue1` | `uint16_t` | 2 | - | Legacy, ignored |
    | `legacyAsyncValue2` | `uint16_t` | 2 | - | Legacy, ignored |
    | `headingHoldRateLimit` | `uint8_t` | 1 | deg/s | Sets `pidProfileMutable()->heading_hold_rate_limit` |
    | `headingHoldLpfFreq` | `uint8_t` | 1 | Hz | Ignored (fixed value `HEADING_HOLD_ERROR_LPF_FREQ` used) |
    | `legacyYawJumpLimit` | `uint16_t` | 2 | - | Legacy, ignored |
    | `legacyGyroLpf` | `uint8_t` | 1 | Enum | Ignored (was gyro LPF) |
    | `accLpfHz` | `uint8_t` | 1 | Hz | Sets `accelerometerConfigMutable()->acc_lpf_hz` |
    | `reserved1` | `uint8_t` | 1 | - | Ignored |
    | `reserved2` | `uint8_t` | 1 | - | Ignored |
    | `reserved3` | `uint8_t` | 1 | - | Ignored |
    | `reserved4` | `uint8_t` | 1 | - | Ignored |
*   **Notes:** Expects 15 bytes.

### `MSP_NAME` (10 / 0x0A)

*   **Direction:** Out
*   **Description:** Returns the user-defined craft name.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `craftName` | `char[]` | Variable | The craft name string (`systemConfig()->craftName`). Null termination is *not* explicitly sent, the length is determined by the payload size |

### `MSP_SET_NAME` (11 / 0x0B)

*   **Direction:** In
*   **Description:** Sets the user-defined craft name.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `craftName` | `char[]` | 1 to `MAX_NAME_LENGTH` | The new craft name string. Automatically null-terminated by the FC |
*   **Notes:** Maximum length is `MAX_NAME_LENGTH`.

### `MSP_NAV_POSHOLD` (12 / 0x0C)

*   **Direction:** Out
*   **Description:** Retrieves navigation position hold and general manual/auto flight parameters. Some parameters depend on the platform type (Multirotor vs Fixed Wing).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `userControlMode` | `uint8_t` | 1 | - | Navigation user control mode NAV_GPS_ATTI (0) or NAV_GPS_CRUISE (1) |
    | `maxAutoSpeed` | `uint16_t` | 2 | cm/s | Max speed in autonomous modes (`navConfig()->general.max_auto_speed`) |
    | `maxAutoClimbRate` | `uint16_t` | 2 | cm/s | Max climb rate in autonomous modes (uses `fw.max_auto_climb_rate` or `mc.max_auto_climb_rate` based on platform) |
    | `maxManualSpeed` | `uint16_t` | 2 | cm/s | Max speed in manual modes with GPS aiding (`navConfig()->general.max_manual_speed`) |
    | `maxManualClimbRate` | `uint16_t` | 2 | cm/s | Max climb rate in manual modes with GPS aiding (uses `fw.max_manual_climb_rate` or `mc.max_manual_climb_rate`) |
    | `mcMaxBankAngle` | `uint8_t` | 1 | degrees | Max bank angle for multirotor position hold (`navConfig()->mc.max_bank_angle`) |
    | `mcAltHoldThrottleType` | `uint8_t` | 1 | Enum | Enum `navMcAltHoldThrottle_e` Sets 'navConfigMutable()->mc.althold_throttle_type' |
    | `mcHoverThrottle` | `uint16_t` | 2 | PWM | Multirotor hover throttle (`currentBatteryProfile->nav.mc.hover_throttle`) |

### `MSP_SET_NAV_POSHOLD` (13 / 0x0D)

*   **Direction:** In
*   **Description:** Sets navigation position hold and general manual/auto flight parameters.
*   **Payload:** (Matches `MSP_NAV_POSHOLD` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `userControlMode` | `uint8_t` | 1 | Enum | Sets `navConfigMutable()->general.flags.user_control_mode` |
    | `maxAutoSpeed` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->general.max_auto_speed` |
    | `maxAutoClimbRate` | `uint16_t` | 2 | cm/s | Sets `fw.max_auto_climb_rate` or `mc.max_auto_climb_rate` based on current platform type |
    | `maxManualSpeed` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->general.max_manual_speed` |
    | `maxManualClimbRate` | `uint16_t` | 2 | cm/s | Sets `fw.max_manual_climb_rate` or `mc.max_manual_climb_rate` |
    | `mcMaxBankAngle` | `uint8_t` | 1 | degrees | Sets `navConfigMutable()->mc.max_bank_angle` |
    | `mcAltHoldThrottleType` | `uint8_t` | 1 | Enum | Enum `navMcAltHoldThrottle_e` Sets 'navConfigMutable()->mc.althold_throttle_type' |
    | `mcHoverThrottle` | `uint16_t` | 2 | PWM | Sets `currentBatteryProfileMutable->nav.mc.hover_throttle` |
*   **Notes:** Expects 13 bytes.

### `MSP_CALIBRATION_DATA` (14 / 0x0E)

*   **Direction:** Out
*   **Description:** Retrieves sensor calibration data (Accelerometer zero/gain, Magnetometer zero/gain, Optical Flow scale).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `accCalibAxisFlags` | `uint8_t` | 1 | Bitmask | Flags indicating which axes of the accelerometer have been calibrated (`accGetCalibrationAxisFlags()`) |
    | `accZeroX` | `uint16_t` | 2 | Raw ADC | Accelerometer zero offset for X-axis (`accelerometerConfig()->accZero.raw[X]`) |
    | `accZeroY` | `uint16_t` | 2 | Raw ADC | Accelerometer zero offset for Y-axis (`accelerometerConfig()->accZero.raw[Y]`) |
    | `accZeroZ` | `uint16_t` | 2 | Raw ADC | Accelerometer zero offset for Z-axis (`accelerometerConfig()->accZero.raw[Z]`) |
    | `accGainX` | `uint16_t` | 2 | Raw ADC | Accelerometer gain/scale for X-axis (`accelerometerConfig()->accGain.raw[X]`) |
    | `accGainY` | `uint16_t` | 2 | Raw ADC | Accelerometer gain/scale for Y-axis (`accelerometerConfig()->accGain.raw[Y]`) |
    | `accGainZ` | `uint16_t` | 2 | Raw ADC | Accelerometer gain/scale for Z-axis (`accelerometerConfig()->accGain.raw[Z]`) |
    | `magZeroX` | `uint16_t` | 2 | Raw ADC | Magnetometer zero offset for X-axis (`compassConfig()->magZero.raw[X]`). 0 if `USE_MAG` disabled |
    | `magZeroY` | `uint16_t` | 2 | Raw ADC | Magnetometer zero offset for Y-axis (`compassConfig()->magZero.raw[Y]`). 0 if `USE_MAG` disabled |
    | `magZeroZ` | `uint16_t` | 2 | Raw ADC | Magnetometer zero offset for Z-axis (`compassConfig()->magZero.raw[Z]`). 0 if `USE_MAG` disabled |
    | `opflowScale` | `uint16_t` | 2 | Scale * 256 | Optical flow scale factor (`opticalFlowConfig()->opflow_scale * 256`). 0 if `USE_OPFLOW` disabled |
    | `magGainX` | `uint16_t` | 2 | Raw ADC | Magnetometer gain/scale for X-axis (`compassConfig()->magGain[X]`). 0 if `USE_MAG` disabled |
    | `magGainY` | `uint16_t` | 2 | Raw ADC | Magnetometer gain/scale for Y-axis (`compassConfig()->magGain[Y]`). 0 if `USE_MAG` disabled |
    | `magGainZ` | `uint16_t` | 2 | Raw ADC | Magnetometer gain/scale for Z-axis (`compassConfig()->magGain[Z]`). 0 if `USE_MAG` disabled |
*   **Notes:** Total size 27 bytes. Fields related to optional sensors are zero if the sensor is not used.

### `MSP_SET_CALIBRATION_DATA` (15 / 0x0F)

*   **Direction:** In
*   **Description:** Sets sensor calibration data.
*   **Payload:** (Matches `MSP_CALIBRATION_DATA` structure, excluding `accCalibAxisFlags`)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `accZeroX` | `uint16_t` | 2 | Raw ADC | Sets `accelerometerConfigMutable()->accZero.raw[X]` |
    | `accZeroY` | `uint16_t` | 2 | Raw ADC | Sets `accelerometerConfigMutable()->accZero.raw[Y]` |
    | `accZeroZ` | `uint16_t` | 2 | Raw ADC | Sets `accelerometerConfigMutable()->accZero.raw[Z]` |
    | `accGainX` | `uint16_t` | 2 | Raw ADC | Sets `accelerometerConfigMutable()->accGain.raw[X]` |
    | `accGainY` | `uint16_t` | 2 | Raw ADC | Sets `accelerometerConfigMutable()->accGain.raw[Y]` |
    | `accGainZ` | `uint16_t` | 2 | Raw ADC | Sets `accelerometerConfigMutable()->accGain.raw[Z]` |
    | `magZeroX` | `uint16_t` | 2 | Raw ADC | Sets `compassConfigMutable()->magZero.raw[X]` (if `USE_MAG`) |
    | `magZeroY` | `uint16_t` | 2 | Raw ADC | Sets `compassConfigMutable()->magZero.raw[Y]` (if `USE_MAG`) |
    | `magZeroZ` | `uint16_t` | 2 | Raw ADC | Sets `compassConfigMutable()->magZero.raw[Z]` (if `USE_MAG`) |
    | `opflowScale` | `uint16_t` | 2 | Scale * 256 | Sets `opticalFlowConfigMutable()->opflow_scale = value / 256.0f` (if `USE_OPFLOW`) |
    | `magGainX` | `uint16_t` | 2 | Raw ADC | Sets `compassConfigMutable()->magGain[X]` (if `USE_MAG`) |
    | `magGainY` | `uint16_t` | 2 | Raw ADC | Sets `compassConfigMutable()->magGain[Y]` (if `USE_MAG`) |
    | `magGainZ` | `uint16_t` | 2 | Raw ADC | Sets `compassConfigMutable()->magGain[Z]` (if `USE_MAG`) |
*   **Notes:** Expects 26 bytes. Ignores values for sensors not enabled by `USE_*` defines.

### `MSP_POSITION_ESTIMATION_CONFIG` (16 / 0x10)

*   **Direction:** Out
*   **Description:** Retrieves parameters related to the INAV position estimation fusion weights and GPS minimum satellite count.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `weightZBaroP` | `uint16_t` | 2 | Weight * 100 | Barometer Z position fusion weight (`positionEstimationConfig()->w_z_baro_p * 100`) |
    | `weightZGPSP` | `uint16_t` | 2 | Weight * 100 | GPS Z position fusion weight (`positionEstimationConfig()->w_z_gps_p * 100`) |
    | `weightZGPSV` | `uint16_t` | 2 | Weight * 100 | GPS Z velocity fusion weight (`positionEstimationConfig()->w_z_gps_v * 100`) |
    | `weightXYGPSP` | `uint16_t` | 2 | Weight * 100 | GPS XY position fusion weight (`positionEstimationConfig()->w_xy_gps_p * 100`) |
    | `weightXYGPSV` | `uint16_t` | 2 | Weight * 100 | GPS XY velocity fusion weight (`positionEstimationConfig()->w_xy_gps_v * 100`) |
    | `minSats` | `uint8_t` | 1 | Count | Minimum satellites required for GPS use (`gpsConfigMutable()->gpsMinSats`) |
    | `useGPSVelNED` | `uint8_t` | 1 | Boolean | Legacy flag, always 1 (GPS velocity is always used if available) |

### `MSP_SET_POSITION_ESTIMATION_CONFIG` (17 / 0x11)

*   **Direction:** In
*   **Description:** Sets parameters related to the INAV position estimation fusion weights and GPS minimum satellite count.
*   **Payload:** (Matches `MSP_POSITION_ESTIMATION_CONFIG` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `weightZBaroP` | `uint16_t` | 2 | Weight * 100 | Sets `positionEstimationConfigMutable()->w_z_baro_p = value / 100.0f` (constrained 0.0-10.0) |
    | `weightZGPSP` | `uint16_t` | 2 | Weight * 100 | Sets `positionEstimationConfigMutable()->w_z_gps_p = value / 100.0f` (constrained 0.0-10.0) |
    | `weightZGPSV` | `uint16_t` | 2 | Weight * 100 | Sets `positionEstimationConfigMutable()->w_z_gps_v = value / 100.0f` (constrained 0.0-10.0) |
    | `weightXYGPSP` | `uint16_t` | 2 | Weight * 100 | Sets `positionEstimationConfigMutable()->w_xy_gps_p = value / 100.0f` (constrained 0.0-10.0) |
    | `weightXYGPSV` | `uint16_t` | 2 | Weight * 100 | Sets `positionEstimationConfigMutable()->w_xy_gps_v = value / 100.0f` (constrained 0.0-10.0) |
    | `minSats` | `uint8_t` | 1 | Count | Sets `gpsConfigMutable()->gpsMinSats` (constrained 5-10) |
    | `useGPSVelNED` | `uint8_t` | 1 | Boolean | Legacy flag, ignored |
*   **Notes:** Expects 12 bytes.

### `MSP_WP_MISSION_LOAD` (18 / 0x12)

*   **Direction:** In
*   **Description:** Commands the FC to load the waypoint mission stored in non-volatile memory (e.g., EEPROM or FlashFS) into the active mission buffer.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `missionID` | `uint8_t` | 1 | Reserved for future use, currently ignored |
*   **Notes:** Only functional if `NAV_NON_VOLATILE_WAYPOINT_STORAGE` is defined. Requires 1 byte payload. Returns error if loading fails.

### `MSP_WP_MISSION_SAVE` (19 / 0x13)

*   **Direction:** In
*   **Description:** Commands the FC to save the currently active waypoint mission from RAM to non-volatile memory (e.g., EEPROM or FlashFS).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `missionID` | `uint8_t` | 1 | Reserved for future use, currently ignored |
*   **Notes:** Only functional if `NAV_NON_VOLATILE_WAYPOINT_STORAGE` is defined. Requires 1 byte payload. Returns error if saving fails.

### `MSP_WP_GETINFO` (20 / 0x14)

*   **Direction:** Out
*   **Description:** Retrieves information about the waypoint mission capabilities and the status of the currently loaded mission.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `wpCapabilities` | `uint8_t` | 1 | Reserved for future waypoint capabilities flags. Currently always 0 |
    | `maxWaypoints` | `uint8_t` | 1 | Maximum number of waypoints supported (`NAV_MAX_WAYPOINTS`) |
    | `missionValid` | `uint8_t` | 1 | Boolean flag indicating if the current mission in RAM is valid (`isWaypointListValid()`) |
    | `waypointCount` | `uint8_t` | 1 | Number of waypoints currently defined in the mission (`getWaypointCount()`) |

### `MSP_RTH_AND_LAND_CONFIG` (21 / 0x15)

*   **Direction:** Out
*   **Description:** Retrieves configuration parameters related to Return-to-Home (RTH) and automatic landing behaviors.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `minRthDistance` | `uint16_t` | 2 | meters | Minimum distance from home required for RTH to engage (`navConfig()->general.min_rth_distance`) |
    | `rthClimbFirst` | `uint8_t` | 1 | Boolean | Flag: Climb to RTH altitude before returning (`navConfig()->general.flags.rth_climb_first`) |
    | `rthClimbIgnoreEmerg` | `uint8_t` | 1 | Boolean | Flag: Climb even in emergency RTH (`navConfig()->general.flags.rth_climb_ignore_emerg`) |
    | `rthTailFirst` | `uint8_t` | 1 | Boolean | Flag: Multirotor returns tail-first (`navConfig()->general.flags.rth_tail_first`) |
    | `rthAllowLanding` | `uint8_t` | 1 | Boolean | Flag: Allow automatic landing after RTH (`navConfig()->general.flags.rth_allow_landing`) |
    | `rthAltControlMode` | `uint8_t` | 1 | Enum | RTH altitude control mode (`navConfig()->general.flags.rth_alt_control_mode`) |
    | `rthAbortThreshold` | `uint16_t` | 2 | cm/s | Stick input threshold to abort RTH (`navConfig()->general.rth_abort_threshold`) |
    | `rthAltitude` | `uint16_t` | 2 | meters | Target RTH altitude (`navConfig()->general.rth_altitude`) |
    | `landMinAltVspd` | `uint16_t` | 2 | cm/s | Landing vertical speed at minimum slowdown altitude (`navConfig()->general.land_minalt_vspd`) |
    | `landMaxAltVspd` | `uint16_t` | 2 | cm/s | Landing vertical speed at maximum slowdown altitude (`navConfig()->general.land_maxalt_vspd`) |
    | `landSlowdownMinAlt` | `uint16_t` | 2 | meters | Altitude below which `landMinAltVspd` applies (`navConfig()->general.land_slowdown_minalt`) |
    | `landSlowdownMaxAlt` | `uint16_t` | 2 | meters | Altitude above which `landMaxAltVspd` applies (`navConfig()->general.land_slowdown_maxalt`) |
    | `emergDescentRate` | `uint16_t` | 2 | cm/s | Vertical speed during emergency landing descent (`navConfig()->general.emerg_descent_rate`) |

### `MSP_SET_RTH_AND_LAND_CONFIG` (22 / 0x16)

*   **Direction:** In
*   **Description:** Sets configuration parameters related to Return-to-Home (RTH) and automatic landing behaviors.
*   **Payload:** (Matches `MSP_RTH_AND_LAND_CONFIG` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `minRthDistance` | `uint16_t` | 2 | meters | Sets `navConfigMutable()->general.min_rth_distance` |
    | `rthClimbFirst` | `uint8_t` | 1 | Boolean | Sets `navConfigMutable()->general.flags.rth_climb_first` |
    | `rthClimbIgnoreEmerg` | `uint8_t` | 1 | Boolean | Sets `navConfigMutable()->general.flags.rth_climb_ignore_emerg` |
    | `rthTailFirst` | `uint8_t` | 1 | Boolean | Sets `navConfigMutable()->general.flags.rth_tail_first` |
    | `rthAllowLanding` | `uint8_t` | 1 | Boolean | Sets `navConfigMutable()->general.flags.rth_allow_landing` |
    | `rthAltControlMode` | `uint8_t` | 1 | Enum | Sets `navConfigMutable()->general.flags.rth_alt_control_mode` |
    | `rthAbortThreshold` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->general.rth_abort_threshold` |
    | `rthAltitude` | `uint16_t` | 2 | meters | Sets `navConfigMutable()->general.rth_altitude` |
    | `landMinAltVspd` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->general.land_minalt_vspd` |
    | `landMaxAltVspd` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->general.land_maxalt_vspd` |
    | `landSlowdownMinAlt` | `uint16_t` | 2 | meters | Sets `navConfigMutable()->general.land_slowdown_minalt` |
    | `landSlowdownMaxAlt` | `uint16_t` | 2 | meters | Sets `navConfigMutable()->general.land_slowdown_maxalt` |
    | `emergDescentRate` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->general.emerg_descent_rate` |
*   **Notes:** Expects 21 bytes.

### `MSP_FW_CONFIG` (23 / 0x17)

*   **Direction:** Out
*   **Description:** Retrieves configuration parameters specific to Fixed Wing navigation.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `cruiseThrottle` | `uint16_t` | 2 | PWM | Cruise throttle level (`currentBatteryProfile->nav.fw.cruise_throttle`) |
    | `minThrottle` | `uint16_t` | 2 | PWM | Minimum throttle during autonomous flight (`currentBatteryProfile->nav.fw.min_throttle`) |
    | `maxThrottle` | `uint16_t` | 2 | PWM | Maximum throttle during autonomous flight (`currentBatteryProfile->nav.fw.max_throttle`) |
    | `maxBankAngle` | `uint8_t` | 1 | degrees | Maximum bank angle allowed (`navConfig()->fw.max_bank_angle`) |
    | `maxClimbAngle` | `uint8_t` | 1 | degrees | Maximum pitch angle during climb (`navConfig()->fw.max_climb_angle`) |
    | `maxDiveAngle` | `uint8_t` | 1 | degrees | Maximum negative pitch angle during descent (`navConfig()->fw.max_dive_angle`) |
    | `pitchToThrottle` | `uint8_t` | 1 | Ratio (%) | Pitch-to-throttle feed-forward ratio (`currentBatteryProfile->nav.fw.pitch_to_throttle`) |
    | `loiterRadius` | `uint16_t` | 2 | meters | Default loiter radius (`navConfig()->fw.loiter_radius`) |

### `MSP_SET_FW_CONFIG` (24 / 0x18)

*   **Direction:** In
*   **Description:** Sets configuration parameters specific to Fixed Wing navigation.
*   **Payload:** (Matches `MSP_FW_CONFIG` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `cruiseThrottle` | `uint16_t` | 2 | PWM | Sets `currentBatteryProfileMutable->nav.fw.cruise_throttle` |
    | `minThrottle` | `uint16_t` | 2 | PWM | Sets `currentBatteryProfileMutable->nav.fw.min_throttle` |
    | `maxThrottle` | `uint16_t` | 2 | PWM | Sets `currentBatteryProfileMutable->nav.fw.max_throttle` |
    | `maxBankAngle` | `uint8_t` | 1 | degrees | Sets `navConfigMutable()->fw.max_bank_angle` |
    | `maxClimbAngle` | `uint8_t` | 1 | degrees | Sets `navConfigMutable()->fw.max_climb_angle` |
    | `maxDiveAngle` | `uint8_t` | 1 | degrees | Sets `navConfigMutable()->fw.max_dive_angle` |
    | `pitchToThrottle` | `uint8_t` | 1 | Ratio (%) | Sets `currentBatteryProfileMutable->nav.fw.pitch_to_throttle` |
    | `loiterRadius` | `uint16_t` | 2 | meters | Sets `navConfigMutable()->fw.loiter_radius` |
*   **Notes:** Expects 12 bytes.

---

## MSPv1 Cleanflight/Betaflight/INAV Feature Commands (34-58)

These commands were often introduced by Cleanflight or Betaflight and adopted/adapted by INAV.

### `MSP_MODE_RANGES` (34 / 0x22)

*   **Direction:** Out
*   **Description:** Returns all defined mode activation ranges (aux channel assignments for flight modes).
*   **Payload:** Repeated `MAX_MODE_ACTIVATION_CONDITION_COUNT` times:
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `modePermanentId` | `uint8_t` | 1 | ID | Permanent ID of the flight mode (maps to `boxId` via `findBoxByActiveBoxId`). 0 if entry unused |
    | `auxChannelIndex` | `uint8_t` | 1 | Index | 0-based index of the AUX channel used for activation |
    | `rangeStartStep` | `uint8_t` | 1 | 0-20 | Start step (corresponding to channel value range 900-2100 in steps of 50/25, depends on steps calculation) |
    | `rangeEndStep` | `uint8_t` | 1 | 0-20 | End step for the activation range |
*   **Notes:** The number of steps and mapping to PWM values depends on internal range calculations.

### `MSP_SET_MODE_RANGE` (35 / 0x23)

*   **Direction:** In
*   **Description:** Sets a single mode activation range by its index.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `rangeIndex` | `uint8_t` | 1 | Index | Index of the mode range to set (0 to `MAX_MODE_ACTIVATION_CONDITION_COUNT - 1`) |
    | `modePermanentId` | `uint8_t` | 1 | ID | Permanent ID of the flight mode to assign |
    | `auxChannelIndex` | `uint8_t` | 1 | Index | 0-based index of the AUX channel |
    | `rangeStartStep` | `uint8_t` | 1 | 0-20 | Start step for activation |
    | `rangeEndStep` | `uint8_t` | 1 | 0-20 | End step for activation |
*   **Notes:** Expects 5 bytes. Updates the mode configuration and recalculates used mode flags. Returns error if `rangeIndex` or `modePermanentId` is invalid.

### `MSP_FEATURE` (36 / 0x24)

*   **Direction:** Out
*   **Description:** Returns a bitmask of enabled features.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `featureMask` | `uint32_t` | 4 | Bitmask of active features (see `featureMask()`) |
*   **Notes:** Feature bits are defined in `feature.h`.

### `MSP_SET_FEATURE` (37 / 0x25)

*   **Direction:** In
*   **Description:** Sets the enabled features using a bitmask. Clears all previous features first.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `featureMask` | `uint32_t` | 4 | Bitmask of features to enable |
*   **Notes:** Expects 4 bytes. Updates feature configuration and related settings (e.g., RSSI source).

### `MSP_BOARD_ALIGNMENT` (38 / 0x26)

*   **Direction:** Out
*   **Description:** Returns the sensor board alignment angles relative to the craft frame.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `rollAlign` | `uint16_t` | 2 | deci-degrees | Board alignment roll angle (`boardAlignment()->rollDeciDegrees`) |
    | `pitchAlign` | `uint16_t` | 2 | deci-degrees | Board alignment pitch angle (`boardAlignment()->pitchDeciDegrees`) |
    | `yawAlign` | `uint16_t` | 2 | deci-degrees | Board alignment yaw angle (`boardAlignment()->yawDeciDegrees`) |

### `MSP_SET_BOARD_ALIGNMENT` (39 / 0x27)

*   **Direction:** In
*   **Description:** Sets the sensor board alignment angles.
*   **Payload:** (Matches `MSP_BOARD_ALIGNMENT` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `rollAlign` | `uint16_t` | 2 | deci-degrees | Sets `boardAlignmentMutable()->rollDeciDegrees` |
    | `pitchAlign` | `uint16_t` | 2 | deci-degrees | Sets `boardAlignmentMutable()->pitchDeciDegrees` |
    | `yawAlign` | `uint16_t` | 2 | deci-degrees | Sets `boardAlignmentMutable()->yawDeciDegrees` |
*   **Notes:** Expects 6 bytes.

### `MSP_CURRENT_METER_CONFIG` (40 / 0x28)

*   **Direction:** Out
*   **Description:** Retrieves the configuration for the current sensor.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `scale` | `uint16_t` | 2 | mV/10A or similar | Current sensor scale factor (`batteryMetersConfig()->current.scale`). Units depend on sensor type |
    | `offset` | `uint16_t` | 2 | mV | Current sensor offset (`batteryMetersConfig()->current.offset`) |
    | `type` | `uint8_t` | 1 | Enum | Enum `currentSensor_e` Type of current sensor hardware |
    | `capacity` | `uint16_t` | 2 | mAh (legacy) | Battery capacity (constrained 0-65535) (`currentBatteryProfile->capacity.value`). Note: This is legacy, use `MSP2_INAV_BATTERY_CONFIG` for full 32-bit capacity |

### `MSP_SET_CURRENT_METER_CONFIG` (41 / 0x29)

*   **Direction:** In
*   **Description:** Sets the configuration for the current sensor.
*   **Payload:** (Matches `MSP_CURRENT_METER_CONFIG` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `scale` | `uint16_t` | 2 | mV/10A or similar | Sets `batteryMetersConfigMutable()->current.scale` |
    | `offset` | `uint16_t` | 2 | mV | Sets `batteryMetersConfigMutable()->current.offset` |
    | `type` | `uint8_t` | 1 | Enum | Enum `currentSensor_e` Sets 'batteryMetersConfigMutable()->current.type' |
    | `capacity` | `uint16_t` | 2 | mAh (legacy) | Sets `currentBatteryProfileMutable->capacity.value` (truncated to 16 bits) |
*   **Notes:** Expects 7 bytes.

### `MSP_MIXER` (42 / 0x2A)

*   **Direction:** Out
*   **Description:** Retrieves the mixer type (Legacy, INAV always returns QuadX).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `mixerMode` | `uint8_t` | 1 | Always 3 (QuadX) in INAV for compatibility |
*   **Notes:** This command is largely obsolete. Mixer configuration is handled differently in INAV (presets, custom mixes). See `MSP2_INAV_MIXER`.

### `MSP_SET_MIXER` (43 / 0x2B)

*   **Direction:** In
*   **Description:** Sets the mixer type (Legacy, ignored by INAV).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `mixerMode` | `uint8_t` | 1 | Mixer mode to set (ignored by INAV) |
*   **Notes:** Expects 1 byte. Calls `mixerUpdateStateFlags()` for potential side effects related to presets.

### `MSP_RX_CONFIG` (44 / 0x2C)

*   **Direction:** Out
*   **Description:** Retrieves receiver configuration settings. Some fields are Betaflight compatibility placeholders.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `serialRxProvider` | `uint8_t` | 1 | Enum | Enum `rxSerialReceiverType_e` sets Serial RX provider type ('rxConfig()->serialrx_provider') |
    | `maxCheck` | `uint16_t` | 2 | PWM | Upper channel value threshold for stick commands (`rxConfig()->maxcheck`) |
    | `midRc` | `uint16_t` | 2 | PWM | Center channel value (`PWM_RANGE_MIDDLE`, typically 1500) |
    | `minCheck` | `uint16_t` | 2 | PWM | Lower channel value threshold for stick commands (`rxConfig()->mincheck`) |
    | `spektrumSatBind` | `uint8_t` | 1 | Count/Flag | Spektrum bind pulses (`rxConfig()->spektrum_sat_bind`). 0 if `USE_SPEKTRUM_BIND` disabled |
    | `rxMinUsec` | `uint16_t` | 2 | µs | Minimum expected pulse width (`rxConfig()->rx_min_usec`) |
    | `rxMaxUsec` | `uint16_t` | 2 | µs | Maximum expected pulse width (`rxConfig()->rx_max_usec`) |
    | `bfCompatRcInterpolation` | `uint8_t` | 1 | - | BF compatibility. Always 0 |
    | `bfCompatRcInterpolationInt` | `uint8_t` | 1 | - | BF compatibility. Always 0 |
    | `bfCompatAirModeThreshold` | `uint16_t` | 2 | - | BF compatibility. Always 0 |
    | `reserved1` | `uint8_t` | 1 | - | Reserved/Padding. Always 0 |
    | `reserved2` | `uint32_t` | 4 | - | Reserved/Padding. Always 0 |
    | `reserved3` | `uint8_t` | 1 | - | Reserved/Padding. Always 0 |
    | `bfCompatFpvCamAngle` | `uint8_t` | 1 | - | BF compatibility. Always 0 |
    | `receiverType` | `uint8_t` | 1 | Enum | Enum `rxReceiverType_e` Receiver type (Parallel PWM, PPM, Serial) ('rxConfig()->receiverType') |

### `MSP_SET_RX_CONFIG` (45 / 0x2D)

*   **Direction:** In
*   **Description:** Sets receiver configuration settings.
*   **Payload:** (Matches `MSP_RX_CONFIG` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `serialRxProvider` | `uint8_t` | 1 | Enum | Enum `rxSerialReceiverType_e` Serial RX provider type ('rxConfig()->serialrx_provider') |
    | `maxCheck` | `uint16_t` | 2 | PWM | Sets `rxConfigMutable()->maxcheck` |
    | `midRc` | `uint16_t` | 2 | PWM | Ignored (`PWM_RANGE_MIDDLE` is used) |
    | `minCheck` | `uint16_t` | 2 | PWM | Sets `rxConfigMutable()->mincheck` |
    | `spektrumSatBind` | `uint8_t` | 1 | Count/Flag | Sets `rxConfigMutable()->spektrum_sat_bind` (if `USE_SPEKTRUM_BIND`) |
    | `rxMinUsec` | `uint16_t` | 2 | µs | Sets `rxConfigMutable()->rx_min_usec` |
    | `rxMaxUsec` | `uint16_t` | 2 | µs | Sets `rxConfigMutable()->rx_max_usec` |
    | `bfCompatRcInterpolation` | `uint8_t` | 1 | - | Ignored |
    | `bfCompatRcInterpolationInt` | `uint8_t` | 1 | - | Ignored |
    | `bfCompatAirModeThreshold` | `uint16_t` | 2 | - | Ignored |
    | `reserved1` | `uint8_t` | 1 | - | Ignored |
    | `reserved2` | `uint32_t` | 4 | - | Ignored |
    | `reserved3` | `uint8_t` | 1 | - | Ignored |
    | `bfCompatFpvCamAngle` | `uint8_t` | 1 | - | Ignored |
    | `receiverType` | `uint8_t` | 1 | Enum | Enum `rxReceiverType_e` Sets 'rxConfigMutable()->receiverType' |
*   **Notes:** Expects 24 bytes.

### `MSP_LED_COLORS` (46 / 0x2E)

*   **Direction:** Out
*   **Description:** Retrieves the HSV color definitions for configurable LED colors.
*   **Payload:** Repeated `LED_CONFIGURABLE_COLOR_COUNT` times:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `hue` | `uint16_t` | 2 | Hue value (0-359) |
    | `saturation` | `uint8_t` | 1 | Saturation value (0-255) |
    | `value` | `uint8_t` | 1 | Value/Brightness (0-255) |
*   **Notes:** Only available if `USE_LED_STRIP` is defined.

### `MSP_SET_LED_COLORS` (47 / 0x2F)

*   **Direction:** In
*   **Description:** Sets the HSV color definitions for configurable LED colors.
*   **Payload:** Repeated `LED_CONFIGURABLE_COLOR_COUNT` times:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `hue` | `uint16_t` | 2 | Hue value (0-359) |
    | `saturation` | `uint8_t` | 1 | Saturation value (0-255) |
    | `value` | `uint8_t` | 1 | Value/Brightness (0-255) |
*   **Notes:** Only available if `USE_LED_STRIP` is defined. Expects `LED_CONFIGURABLE_COLOR_COUNT * 4` bytes.

### `MSP_LED_STRIP_CONFIG` (48 / 0x30)

*   **Direction:** Out
*   **Description:** Retrieves the configuration for each LED on the strip (legacy packed format).
*   **Payload:** Repeated `LED_MAX_STRIP_LENGTH` times:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `legacyLedConfig` | `uint32_t` | 4 | Packed LED configuration (position, function, overlay, color, direction, params). See C code for bit packing details |
*   **Notes:** Only available if `USE_LED_STRIP` is defined. Superseded by `MSP2_INAV_LED_STRIP_CONFIG_EX` which uses a clearer struct.

### `MSP_SET_LED_STRIP_CONFIG` (49 / 0x31)

*   **Direction:** In
*   **Description:** Sets the configuration for a single LED on the strip using the legacy packed format.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `ledIndex` | `uint8_t` | 1 | Index of the LED to configure (0 to `LED_MAX_STRIP_LENGTH - 1`) |
    | `legacyLedConfig` | `uint32_t` | 4 | Packed LED configuration to set |
*   **Notes:** Only available if `USE_LED_STRIP` is defined. Expects 5 bytes. Calls `reevaluateLedConfig()`. Superseded by `MSP2_INAV_SET_LED_STRIP_CONFIG_EX`.

### `MSP_RSSI_CONFIG` (50 / 0x32)

*   **Direction:** Out
*   **Description:** Retrieves the channel used for analog RSSI input.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `rssiChannel` | `uint8_t` | 1 | AUX channel index (1-based) used for RSSI, or 0 if disabled (`rxConfig()->rssi_channel`) |

### `MSP_SET_RSSI_CONFIG` (51 / 0x33)

*   **Direction:** In
*   **Description:** Sets the channel used for analog RSSI input.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `rssiChannel` | `uint8_t` | 1 | AUX channel index (1-based) to use for RSSI, or 0 to disable |
*   **Notes:** Expects 1 byte. Input value is constrained 0 to `MAX_SUPPORTED_RC_CHANNEL_COUNT`. Updates the effective RSSI source.

### `MSP_ADJUSTMENT_RANGES` (52 / 0x34)

*   **Direction:** Out
*   **Description:** Returns all defined RC adjustment ranges (tuning via aux channels).
*   **Payload:** Repeated `MAX_ADJUSTMENT_RANGE_COUNT` times:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `adjustmentIndex` | `uint8_t` | 1 | Index of the adjustment slot (0 to `MAX_SIMULTANEOUS_ADJUSTMENT_COUNT - 1`) |
    | `auxChannelIndex` | `uint8_t` | 1 | 0-based index of the AUX channel controlling the adjustment value |
    | `rangeStartStep` | `uint8_t` | 1 | Start step (0-20) of the control channel range |
    | `rangeEndStep` | `uint8_t` | 1 | End step (0-20) of the control channel range |
    | `adjustmentFunction` | `uint8_t` | 1 | Function/parameter being adjusted (e.g., PID gain, rate). See `rcAdjustments.h` |
    | `auxSwitchChannelIndex` | `uint8_t` | 1 | 0-based index of the AUX channel acting as an enable switch (or 0 if always enabled) |
*   **Notes:** See `adjustmentRange_t`.

### `MSP_SET_ADJUSTMENT_RANGE` (53 / 0x35)

*   **Direction:** In
*   **Description:** Sets a single RC adjustment range configuration by its index.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `rangeIndex` | `uint8_t` | 1 | Index of the adjustment range to set (0 to `MAX_ADJUSTMENT_RANGE_COUNT - 1`) |
    | `adjustmentIndex` | `uint8_t` | 1 | Adjustment slot index (0 to `MAX_SIMULTANEOUS_ADJUSTMENT_COUNT - 1`) |
    | `auxChannelIndex` | `uint8_t` | 1 | 0-based index of the control AUX channel |
    | `rangeStartStep` | `uint8_t` | 1 | Start step (0-20) |
    | `rangeEndStep` | `uint8_t` | 1 | End step (0-20) |
    | `adjustmentFunction` | `uint8_t` | 1 | Function/parameter being adjusted |
    | `auxSwitchChannelIndex` | `uint8_t` | 1 | 0-based index of the enable switch AUX channel (or 0) |
*   **Notes:** Expects 7 bytes. Returns error if `rangeIndex` or `adjustmentIndex` is invalid.

### `MSP_CF_SERIAL_CONFIG` (54 / 0x36)

*   **Direction:** Out
*   **Description:** Deprecated command to get serial port configuration.
*   **Notes:** Not implemented in INAV `fc_msp.c`. Use `MSP2_COMMON_SERIAL_CONFIG`.

### `MSP_SET_CF_SERIAL_CONFIG` (55 / 0x37)

*   **Direction:** In
*   **Description:** Deprecated command to set serial port configuration.
*   **Notes:** Not implemented in INAV `fc_msp.c`. Use `MSP2_COMMON_SET_SERIAL_CONFIG`.

### `MSP_VOLTAGE_METER_CONFIG` (56 / 0x38)

*   **Direction:** Out
*   **Description:** Retrieves legacy voltage meter configuration (scaled values).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `vbatScale` | `uint8_t` | 1 | Scale / 10 | Voltage sensor scale factor / 10 (`batteryMetersConfig()->voltage.scale / 10`). 0 if `USE_ADC` disabled |
    | `vbatMinCell` | `uint8_t` | 1 | 0.1V | Minimum cell voltage / 10 (`currentBatteryProfile->voltage.cellMin / 10`). 0 if `USE_ADC` disabled |
    | `vbatMaxCell` | `uint8_t` | 1 | 0.1V | Maximum cell voltage / 10 (`currentBatteryProfile->voltage.cellMax / 10`). 0 if `USE_ADC` disabled |
    | `vbatWarningCell` | `uint8_t` | 1 | 0.1V | Warning cell voltage / 10 (`currentBatteryProfile->voltage.cellWarning / 10`). 0 if `USE_ADC` disabled |
*   **Notes:** Superseded by `MSP2_INAV_BATTERY_CONFIG`.

### `MSP_SET_VOLTAGE_METER_CONFIG` (57 / 0x39)

*   **Direction:** In
*   **Description:** Sets legacy voltage meter configuration (scaled values).
*   **Payload:** (Matches `MSP_VOLTAGE_METER_CONFIG` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `vbatScale` | `uint8_t` | 1 | Scale / 10 | Sets `batteryMetersConfigMutable()->voltage.scale = value * 10` (if `USE_ADC`) |
    | `vbatMinCell` | `uint8_t` | 1 | 0.1V | Sets `currentBatteryProfileMutable->voltage.cellMin = value * 10` (if `USE_ADC`) |
    | `vbatMaxCell` | `uint8_t` | 1 | 0.1V | Sets `currentBatteryProfileMutable->voltage.cellMax = value * 10` (if `USE_ADC`) |
    | `vbatWarningCell` | `uint8_t` | 1 | 0.1V | Sets `currentBatteryProfileMutable->voltage.cellWarning = value * 10` (if `USE_ADC`) |
*   **Notes:** Expects 4 bytes. Superseded by `MSP2_INAV_SET_BATTERY_CONFIG`.

### `MSP_SONAR_ALTITUDE` (58 / 0x3A)

*   **Direction:** Out
*   **Description:** Retrieves the altitude measured by the primary rangefinder (sonar or lidar).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `rangefinderAltitude` | `uint32_t` | 4 | cm | Latest altitude reading from the rangefinder (`rangefinderGetLatestAltitude()`). 0 if `USE_RANGEFINDER` disabled or no reading |

---

## MSPv1 Baseflight/INAV Commands (64-99, plus others)

These commands originated in Baseflight or were added later in similar ranges.

### `MSP_RX_MAP` (64 / 0x40)

*   **Direction:** Out
*   **Description:** Retrieves the RC channel mapping array (AETR, etc.).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `rcMap` | `uint8_t[MAX_MAPPABLE_RX_INPUTS]` | `MAX_MAPPABLE_RX_INPUTS` | Array defining the mapping from input channel index to logical function (Roll, Pitch, Yaw, Throttle, Aux1...) |
*   **Notes:** `MAX_MAPPABLE_RX_INPUTS` is typically 8 or more.

### `MSP_SET_RX_MAP` (65 / 0x41)

*   **Direction:** In
*   **Description:** Sets the RC channel mapping array.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `rcMap` | `uint8_t[MAX_MAPPABLE_RX_INPUTS]` | `MAX_MAPPABLE_RX_INPUTS` | Array defining the new channel mapping |
*   **Notes:** Expects `MAX_MAPPABLE_RX_INPUTS` bytes.

### `MSP_REBOOT` (68 / 0x44)

*   **Direction:** Out (but triggers an action)
*   **Description:** Commands the flight controller to reboot.
*   **Payload:** None
*   **Notes:** The FC sends an ACK *before* rebooting. The `mspPostProcessFn` is set to `mspRebootFn` to perform the reboot after the reply is sent. Will fail if the craft is armed.

### `MSP_DATAFLASH_SUMMARY` (70 / 0x46)

*   **Direction:** Out
*   **Description:** Retrieves summary information about the onboard dataflash chip (if present and used for Blackbox via FlashFS).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `flashReady` | `uint8_t` | 1 | Boolean: 1 if flash chip is ready, 0 otherwise. (`flashIsReady()`). 0 if `USE_FLASHFS` disabled |
    | `sectorCount` | `uint32_t` | 4 | Total number of sectors on the flash chip (`geometry->sectors`). 0 if `USE_FLASHFS` disabled |
    | `totalSize` | `uint32_t` | 4 | Total size of the flash chip in bytes (`geometry->totalSize`). 0 if `USE_FLASHFS` disabled |
    | `usedSize` | `uint32_t` | 4 | Currently used size in bytes (FlashFS offset) (`flashfsGetOffset()`). 0 if `USE_FLASHFS` disabled |
*   **Notes:** Requires `USE_FLASHFS`.

### `MSP_DATAFLASH_READ` (71 / 0x47)

*   **Direction:** In/Out
*   **Description:** Reads a block of data from the onboard dataflash (FlashFS).
*   **Request Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `address` | `uint32_t` | 4 | Starting address to read from within the FlashFS volume |
    | `size` | `uint16_t` | 2 | (Optional) Number of bytes to read. Defaults to 128 if not provided |
*   **Reply Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `address` | `uint32_t` | 4 | The starting address from which data was actually read |
    | `data` | `uint8_t[]` | Variable | The data read from flash. Length is MIN(requested size, remaining buffer space, remaining flashfs data) |
*   **Notes:** Requires `USE_FLASHFS`. Read length may be truncated by buffer size or end of flashfs volume.

### `MSP_DATAFLASH_ERASE` (72 / 0x48)

*   **Direction:** In
*   **Description:** Erases the entire onboard dataflash chip (FlashFS volume).
*   **Payload:** None
*   **Notes:** Requires `USE_FLASHFS`. This is a potentially long operation. Use with caution.

### `MSP_LOOP_TIME` (73 / 0x49)

*   **Direction:** Out
*   **Description:** Retrieves the configured loop time (PID loop frequency denominator).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `looptime` | `uint16_t` | 2 | µs | Configured loop time (`gyroConfig()->looptime`) |
*   **Notes:** This is the *configured* target loop time, not necessarily the *actual* measured cycle time (see `MSP_STATUS`).

### `MSP_SET_LOOP_TIME` (74 / 0x4A)

*   **Direction:** In
*   **Description:** Sets the configured loop time.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `looptime` | `uint16_t` | 2 | µs | New loop time to set (`gyroConfigMutable()->looptime`) |
*   **Notes:** Expects 2 bytes.

### `MSP_FAILSAFE_CONFIG` (75 / 0x4B)

*   **Direction:** Out
*   **Description:** Retrieves the failsafe configuration settings.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `failsafeDelay` | `uint8_t` | 1 | 0.1s | Delay before failsafe stage 1 activates (`failsafeConfig()->failsafe_delay`) |
    | `failsafeOffDelay` | `uint8_t` | 1 | 0.1s | Delay after signal recovery before returning control (`failsafeConfig()->failsafe_off_delay`) |
    | `failsafeThrottle` | `uint16_t` | 2 | PWM | Throttle level during failsafe stage 2 (`currentBatteryProfile->failsafe_throttle`) |
    | `legacyKillSwitch` | `uint8_t` | 1 | - | Legacy flag, always 0 |
    | `failsafeThrottleLowDelay` | `uint16_t` | 2 | ms | Delay for throttle-based failsafe detection (`failsafeConfig()->failsafe_throttle_low_delay`) |
    | `failsafeProcedure` | `uint8_t` | 1 | Enum | Enum `failsafeProcedure_e` Failsafe procedure (Drop, RTH, Land, etc.) ('failsafeConfig()->failsafe_procedure') |
    | `failsafeRecoveryDelay` | `uint8_t` | 1 | 0.1s | Delay after RTH finishes before attempting recovery (`failsafeConfig()->failsafe_recovery_delay`) |
    | `failsafeFWRollAngle` | `uint16_t` | 2 | deci-degrees | Fixed wing failsafe roll angle (`failsafeConfig()->failsafe_fw_roll_angle`) |
    | `failsafeFWPitchAngle` | `uint16_t` | 2 | deci-degrees | Fixed wing failsafe pitch angle (`failsafeConfig()->failsafe_fw_pitch_angle`) |
    | `failsafeFWYawRate` | `uint16_t` | 2 | deg/s | Fixed wing failsafe yaw rate (`failsafeConfig()->failsafe_fw_yaw_rate`) |
    | `failsafeStickThreshold` | `uint16_t` | 2 | PWM units | Stick movement threshold to exit failsafe (`failsafeConfig()->failsafe_stick_motion_threshold`) |
    | `failsafeMinDistance` | `uint16_t` | 2 | meters | Minimum distance from home for RTH failsafe (`failsafeConfig()->failsafe_min_distance`) |
    | `failsafeMinDistanceProc` | `uint8_t` | 1 | Enum | Enum `failsafeProcedure_e` Failsafe procedure if below min distance ('failsafeConfig()->failsafe_min_distance_procedure') |

### `MSP_SET_FAILSAFE_CONFIG` (76 / 0x4C)

*   **Direction:** In
*   **Description:** Sets the failsafe configuration settings.
*   **Payload:** (Matches `MSP_FAILSAFE_CONFIG` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `failsafeDelay` | `uint8_t` | 1 | 0.1s | Sets `failsafeConfigMutable()->failsafe_delay` |
    | `failsafeOffDelay` | `uint8_t` | 1 | 0.1s | Sets `failsafeConfigMutable()->failsafe_off_delay` |
    | `failsafeThrottle` | `uint16_t` | 2 | PWM | Sets `currentBatteryProfileMutable->failsafe_throttle` |
    | `legacyKillSwitch` | `uint8_t` | 1 | - | Ignored |
    | `failsafeThrottleLowDelay` | `uint16_t` | 2 | ms | Sets `failsafeConfigMutable()->failsafe_throttle_low_delay` |
    | `failsafeProcedure` | `uint8_t` | 1 | Enum | Enum `failsafeProcedure_e` Sets 'failsafeConfigMutable()->failsafe_procedure' |
    | `failsafeRecoveryDelay` | `uint8_t` | 1 | 0.1s | Sets `failsafeConfigMutable()->failsafe_recovery_delay` |
    | `failsafeFWRollAngle` | `uint16_t` | 2 | deci-degrees | Sets `failsafeConfigMutable()->failsafe_fw_roll_angle` (casted to `int16_t`) |
    | `failsafeFWPitchAngle` | `uint16_t` | 2 | deci-degrees | Sets `failsafeConfigMutable()->failsafe_fw_pitch_angle` (casted to `int16_t`) |
    | `failsafeFWYawRate` | `uint16_t` | 2 | deg/s | Sets `failsafeConfigMutable()->failsafe_fw_yaw_rate` (casted to `int16_t`) |
    | `failsafeStickThreshold` | `uint16_t` | 2 | PWM units | Sets `failsafeConfigMutable()->failsafe_stick_motion_threshold` |
    | `failsafeMinDistance` | `uint16_t` | 2 | meters | Sets `failsafeConfigMutable()->failsafe_min_distance` |
    | `failsafeMinDistanceProc` | `uint8_t` | 1 | Enum | Enum `failsafeProcedure_e` Sets 'failsafeConfigMutable()->failsafe_min_distance_procedure' |
*   **Notes:** Expects 20 bytes.

### `MSP_SDCARD_SUMMARY` (79 / 0x4F)

*   **Direction:** Out
*   **Description:** Retrieves summary information about the SD card status and filesystem.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `sdCardSupported` | `uint8_t` | 1 | Bitmask: Bit 0 = 1 if SD card support compiled in (`USE_SDCARD`) |
    | `sdCardState` | `uint8_t` | 1 | Enum (`mspSDCardState_e`): Current state (Not Present, Fatal, Card Init, FS Init, Ready). 0 if `USE_SDCARD` disabled |
    | `fsError` | `uint8_t` | 1 | Last filesystem error code (`afatfs_getLastError()`). 0 if `USE_SDCARD` disabled |
    | `freeSpaceKB` | `uint32_t` | 4 | Free space in KiB (`afatfs_getContiguousFreeSpace() / 1024`). 0 if `USE_SDCARD` disabled |
    | `totalSpaceKB` | `uint32_t` | 4 | Total space in KiB (`sdcard_getMetadata()->numBlocks / 2`). 0 if `USE_SDCARD` disabled |
*   **Notes:** Requires `USE_SDCARD` and `USE_ASYNCFATFS`.

### `MSP_BLACKBOX_CONFIG` (80 / 0x50)

*   **Direction:** Out
*   **Description:** Legacy command to retrieve Blackbox configuration. Superseded by `MSP2_BLACKBOX_CONFIG`.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `blackboxDevice` | `uint8_t` | 1 | Always 0 (API no longer supported) |
    | `blackboxRateNum` | `uint8_t` | 1 | Always 0 |
    | `blackboxRateDenom` | `uint8_t` | 1 | Always 0 |
    | `blackboxPDenom` | `uint8_t` | 1 | Always 0 |
*   **Notes:** Returns fixed zero values. Use `MSP2_BLACKBOX_CONFIG`.

### `MSP_SET_BLACKBOX_CONFIG` (81 / 0x51)

*   **Direction:** In
*   **Description:** Legacy command to set Blackbox configuration. Superseded by `MSP2_SET_BLACKBOX_CONFIG`.
*   **Payload:** (Ignored)
*   **Notes:** Not implemented in `fc_msp.c`. Use `MSP2_SET_BLACKBOX_CONFIG`.

### `MSP_TRANSPONDER_CONFIG` (82 / 0x52)

*   **Direction:** Out
*   **Description:** Get VTX Transponder settings (likely specific to RaceFlight/Betaflight, not standard INAV VTX).
*   **Notes:** Not implemented in INAV `fc_msp.c`.

### `MSP_SET_TRANSPONDER_CONFIG` (83 / 0x53)

*   **Direction:** In
*   **Description:** Set VTX Transponder settings.
*   **Notes:** Not implemented in INAV `fc_msp.c`.

### `MSP_OSD_CONFIG` (84 / 0x54)

*   **Direction:** Out
*   **Description:** Retrieves OSD configuration settings and layout for screen 0.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `osdDriverType` | `uint8_t` | 1 | Enum | Enum `OSD_DRIVER_MAX7456` if `USE_OSD`, else `OSD_DRIVER_NONE` |
    | `videoSystem` | `uint8_t` | 1 | Enum | Enum `videoSystem_e`: Video system (Auto/PAL/NTSC) (`osdConfig()->video_system`). Sent even if OSD disabled |
    | `units` | `uint8_t` | 1 | Enum | Enum `osd_unit_e` Measurement units (Metric/Imperial) (`osdConfig()->units`). Sent even if OSD disabled |
    | `rssiAlarm` | `uint8_t` | 1 | % | RSSI alarm threshold (`osdConfig()->rssi_alarm`). Sent even if OSD disabled |
    | `capAlarm` | `uint16_t` | 2 | mAh/mWh | Capacity alarm threshold (`currentBatteryProfile->capacity.warning`). Sent even if OSD disabled |
    | `timerAlarm` | `uint16_t` | 2 | seconds | Timer alarm threshold (`osdConfig()->time_alarm`). Sent even if OSD disabled |
    | `altAlarm` | `uint16_t` | 2 | meters | Altitude alarm threshold (`osdConfig()->alt_alarm`). Sent even if OSD disabled |
    | `distAlarm` | `uint16_t` | 2 | meters | Distance alarm threshold (`osdConfig()->dist_alarm`). Sent even if OSD disabled |
    | `negAltAlarm` | `uint16_t` | 2 | meters | Negative altitude alarm threshold (`osdConfig()->neg_alt_alarm`). Sent even if OSD disabled |
    | `itemPositions` | `uint16_t[OSD_ITEM_COUNT]` | `OSD_ITEM_COUNT * 2` | Coordinates | Packed X/Y position for each OSD item on screen 0 (`osdLayoutsConfig()->item_pos[0][i]`). Sent even if OSD disabled |
*   **Notes:** Requires `USE_OSD` for meaningful data, but payload is always sent. Coordinates are packed: `(Y << 8) | X`. See `MSP2_INAV_OSD_*` commands for more detail and multi-layout support.

### `MSP_SET_OSD_CONFIG` (85 / 0x55)

*   **Direction:** In
*   **Description:** Sets OSD configuration or a single item's position on screen 0.
*   **Payload Format 1 (Set General Config):**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `addr` | `uint8_t` | 1 | - | Must be 0xFF (-1) |
    | `videoSystem` | `uint8_t` | 1 | Enum | Enum `videoSystem_e` Sets `osdConfigMutable()->video_system` |
    | `units` | `uint8_t` | 1 | Enum | Enum `osd_unit_e` Sets `osdConfigMutable()->units` |
    | `rssiAlarm` | `uint8_t` | 1 | % | Sets `osdConfigMutable()->rssi_alarm` |
    | `capAlarm` | `uint16_t` | 2 | mAh/mWh | Sets `currentBatteryProfileMutable->capacity.warning` |
    | `timerAlarm` | `uint16_t` | 2 | seconds | Sets `osdConfigMutable()->time_alarm` |
    | `altAlarm` | `uint16_t` | 2 | meters | Sets `osdConfigMutable()->alt_alarm` |
    | `distAlarm` | `uint16_t` | 2 | meters | (Optional) Sets `osdConfigMutable()->dist_alarm` |
    | `negAltAlarm` | `uint16_t` | 2 | meters | (Optional) Sets `osdConfigMutable()->neg_alt_alarm` |
*   **Payload Format 2 (Set Item Position):**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `itemIndex` | `uint8_t` | 1 | Index | Index of the OSD item to position (0 to `OSD_ITEM_COUNT - 1`) |
    | `itemPosition` | `uint16_t` | 2 | Coordinates | Packed X/Y position (`(Y << 8) | X`) for the item on screen 0 |
*   **Notes:** Requires `USE_OSD`. Distinguishes formats based on the first byte. Format 1 requires at least 10 bytes. Format 2 requires 3 bytes. Triggers an OSD redraw. See `MSP2_INAV_OSD_SET_*` for more advanced control.

### `MSP_OSD_CHAR_READ` (86 / 0x56)

*   **Direction:** Out
*   **Description:** Reads character data from the OSD font memory.
*   **Notes:** Not implemented in INAV `fc_msp.c`. Requires direct hardware access, typically done via DisplayPort.

### `MSP_OSD_CHAR_WRITE` (87 / 0x57)

*   **Direction:** In
*   **Description:** Writes character data to the OSD font memory.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `address` | `uint8_t` or `uint16_t` | 1 or 2 | Starting address in font memory. Size depends on total payload size |
    | `charData` | `uint8_t[]` | Variable | Character bitmap data (54 or 64 bytes per char, depending on format) |
*   **Notes:** Requires `USE_OSD`. Payload size determines address size (8/16 bit) and character data size (visible bytes only or full char with metadata). Uses `displayWriteFontCharacter()`. Requires OSD hardware (like MAX7456) to be present and functional.

### `MSP_VTX_CONFIG` (88 / 0x58)

*   **Direction:** Out
*   **Description:** Retrieves the current VTX (Video Transmitter) configuration and capabilities.
*   **Payload:** (Only sent if `USE_VTX_CONTROL` is defined and a VTX device is configured)
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `vtxDeviceType` | `uint8_t` | 1 | Enum (`vtxDevType_e`): Type of VTX device detected/configured. `VTXDEV_UNKNOWN` if none |
    | `band` | `uint8_t` | 1 | VTX band number (from `vtxSettingsConfig`) |
    | `channel` | `uint8_t` | 1 | VTX channel number (from `vtxSettingsConfig`) |
    | `power` | `uint8_t` | 1 | VTX power level index (from `vtxSettingsConfig`) |
    | `pitMode` | `uint8_t` | 1 | Boolean: 1 if VTX is currently in pit mode, 0 otherwise |
    | `vtxReady` | `uint8_t` | 1 | Boolean: 1 if VTX device reported ready, 0 otherwise |
    | `lowPowerDisarm` | `uint8_t` | 1 | Boolean: 1 if low power on disarm is enabled (from `vtxSettingsConfig`) |
    | `vtxTableAvailable` | `uint8_t` | 1 | Boolean: 1 if VTX tables (band/power) are available for query |
    | `bandCount` | `uint8_t` | 1 | Number of bands supported by the VTX device |
    | `channelCount` | `uint8_t` | 1 | Number of channels per band supported by the VTX device |
    | `powerCount` | `uint8_t` | 1 | Number of power levels supported by the VTX device |
*   **Notes:** BF compatibility field `frequency` (uint16) is missing compared to some BF versions. Use `MSP_VTXTABLE_BAND` and `MSP_VTXTABLE_POWERLEVEL` for details.

### `MSP_SET_VTX_CONFIG` (89 / 0x59)

*   **Direction:** In
*   **Description:** Sets the VTX configuration (band, channel, power, pit mode). Supports multiple protocol versions/extensions based on payload size.
*   **Payload (Minimum):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `bandChannelEncoded` | `uint16_t` | 2 | Encoded band/channel value: `(band-1)*8 + (channel-1)`. If <= `VTXCOMMON_MSP_BANDCHAN_CHKVAL` |
*   **Payload (Extended):** (Fields added sequentially based on size)
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `power` | `uint8_t` | 1 | Power level index to set (`vtxSettingsConfigMutable()->power`) |
    | `pitMode` | `uint8_t` | 1 | Pit mode state to set (0=off, 1=on). Directly calls `vtxCommonSetPitMode` |
    | `lowPowerDisarm` | `uint8_t` | 1 | Low power on disarm setting (`vtxSettingsConfigMutable()->lowPowerDisarm`) |
    | `pitModeFreq` | `uint16_t` | 2 | *Ignored*. Betaflight extension |
    | `band` | `uint8_t` | 1 | Explicit band number to set (`vtxSettingsConfigMutable()->band`). Overrides encoded value if present |
    | `channel` | `uint8_t` | 1 | Explicit channel number to set (`vtxSettingsConfigMutable()->channel`). Overrides encoded value if present |
    | `frequency` | `uint16_t` | 2 | *Ignored*. Betaflight extension |
    | `bandCount` | `uint8_t` | 1 | *Ignored*. Betaflight extension |
    | `channelCount` | `uint8_t` | 1 | *Ignored*. Betaflight extension |
    | `powerCount` | `uint8_t` | 1 | *Ignored*. Betaflight extension (can potentially reduce reported power count if valid) |
*   **Notes:** Requires `USE_VTX_CONTROL`. Minimum size 2 bytes. Applies settings to `vtxSettingsConfig` and potentially directly to the device (pit mode).

### `MSP_ADVANCED_CONFIG` (90 / 0x5A)

*   **Direction:** Out
*   **Description:** Retrieves advanced hardware-related configuration (PWM protocols, rates). Some fields are BF compatibility placeholders.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `gyroSyncDenom` | `uint8_t` | 1 | Always 1 (BF compatibility) |
    | `pidProcessDenom` | `uint8_t` | 1 | Always 1 (BF compatibility) |
    | `useUnsyncedPwm` | `uint8_t` | 1 | Always 1 (BF compatibility, INAV uses async PWM based on protocol) |
    | `motorPwmProtocol` | `uint8_t` | 1 | Motor PWM protocol type (`motorConfig()->motorPwmProtocol`) |
    | `motorPwmRate` | `uint16_t` | 2 | Hz: Motor PWM rate (if applicable) (`motorConfig()->motorPwmRate`) |
    | `servoPwmRate` | `uint16_t` | 2 | Hz: Servo PWM rate (`servoConfig()->servoPwmRate`) |
    | `legacyGyroSync` | `uint8_t` | 1 | Always 0 (BF compatibility) |

### `MSP_SET_ADVANCED_CONFIG` (91 / 0x5B)

*   **Direction:** In
*   **Description:** Sets advanced hardware-related configuration (PWM protocols, rates).
*   **Payload:** (Matches `MSP_ADVANCED_CONFIG` structure)
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `gyroSyncDenom` | `uint8_t` | 1 | Ignored |
    | `pidProcessDenom` | `uint8_t` | 1 | Ignored |
    | `useUnsyncedPwm` | `uint8_t` | 1 | Ignored |
    | `motorPwmProtocol` | `uint8_t` | 1 | Sets `motorConfigMutable()->motorPwmProtocol` |
    | `motorPwmRate` | `uint16_t` | 2 | Sets `motorConfigMutable()->motorPwmRate` |
    | `servoPwmRate` | `uint16_t` | 2 | Sets `servoConfigMutable()->servoPwmRate` |
    | `legacyGyroSync` | `uint8_t` | 1 | Ignored |
*   **Notes:** Expects 9 bytes.

### `MSP_FILTER_CONFIG` (92 / 0x5C)

*   **Direction:** Out
*   **Description:** Retrieves filter configuration settings (Gyro, D-term, Yaw, Accel). Some fields are BF compatibility placeholders or legacy.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `gyroMainLpfHz` | `uint8_t` | 1 | Hz | Gyro main low-pass filter cutoff frequency (`gyroConfig()->gyro_main_lpf_hz`) |
    | `dtermLpfHz` | `uint16_t` | 2 | Hz | D-term low-pass filter cutoff frequency (`pidProfile()->dterm_lpf_hz`) |
    | `yawLpfHz` | `uint16_t` | 2 | Hz | Yaw low-pass filter cutoff frequency (`pidProfile()->yaw_lpf_hz`) |
    | `legacyGyroNotchHz` | `uint16_t` | 2 | - | Always 0 (Legacy) |
    | `legacyGyroNotchCutoff` | `uint16_t` | 2 | - | Always 1 (Legacy) |
    | `bfCompatDtermNotchHz` | `uint16_t` | 2 | - | Always 0 (BF compatibility) |
    | `bfCompatDtermNotchCutoff` | `uint16_t` | 2 | - | Always 1 (BF compatibility) |
    | `bfCompatGyroNotch2Hz` | `uint16_t` | 2 | - | Always 0 (BF compatibility) |
    | `bfCompatGyroNotch2Cutoff` | `uint16_t` | 2 | - | Always 1 (BF compatibility) |
    | `accNotchHz` | `uint16_t` | 2 | Hz | Accelerometer notch filter center frequency (`accelerometerConfig()->acc_notch_hz`) |
    | `accNotchCutoff` | `uint16_t` | 2 | Hz | Accelerometer notch filter cutoff frequency (`accelerometerConfig()->acc_notch_cutoff`) |
    | `legacyGyroStage2LpfHz` | `uint16_t` | 2 | - | Always 0 (Legacy) |

### `MSP_SET_FILTER_CONFIG` (93 / 0x5D)

*   **Direction:** In
*   **Description:** Sets filter configuration settings. Handles different payload lengths for backward compatibility.
*   **Payload:** (Fields added sequentially based on size)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `gyroMainLpfHz` | `uint8_t` | 1 | Hz | Sets `gyroConfigMutable()->gyro_main_lpf_hz`. (Size >= 5) |
    | `dtermLpfHz` | `uint16_t` | 2 | Hz | Sets `pidProfileMutable()->dterm_lpf_hz` (constrained 0-500). (Size >= 5) |
    | `yawLpfHz` | `uint16_t` | 2 | Hz | Sets `pidProfileMutable()->yaw_lpf_hz` (constrained 0-255). (Size >= 5) |
    | `legacyGyroNotchHz` | `uint16_t` | 2 | - | Ignored. (Size >= 9) |
    | `legacyGyroNotchCutoff` | `uint16_t` | 2 | - | Ignored. (Size >= 9) |
    | `bfCompatDtermNotchHz` | `uint16_t` | 2 | - | Ignored. (Size >= 13) |
    | `bfCompatDtermNotchCutoff` | `uint16_t` | 2 | - | Ignored. (Size >= 13) |
    | `bfCompatGyroNotch2Hz` | `uint16_t` | 2 | - | Ignored. (Size >= 17) |
    | `bfCompatGyroNotch2Cutoff` | `uint16_t` | 2 | - | Ignored. (Size >= 17) |
    | `accNotchHz` | `uint16_t` | 2 | Hz | Sets `accelerometerConfigMutable()->acc_notch_hz` (constrained 0-255). (Size >= 21) |
    | `accNotchCutoff` | `uint16_t` | 2 | Hz | Sets `accelerometerConfigMutable()->acc_notch_cutoff` (constrained 1-255). (Size >= 21) |
    | `legacyGyroStage2LpfHz` | `uint16_t` | 2 | - | Ignored. (Size >= 22) |
*   **Notes:** Requires specific payload sizes (5, 9, 13, 17, 21, or 22 bytes) to be accepted. Calls `pidInitFilters()` if size >= 13.

### `MSP_PID_ADVANCED` (94 / 0x5E)

*   **Direction:** Out
*   **Description:** Retrieves advanced PID tuning parameters. Many fields are BF compatibility placeholders.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `legacyRollPitchItermIgnore` | `uint16_t` | 2 | - | Always 0 (Legacy) |
    | `legacyYawItermIgnore` | `uint16_t` | 2 | - | Always 0 (Legacy) |
    | `legacyYawPLimit` | `uint16_t` | 2 | - | Always 0 (Legacy) |
    | `bfCompatDeltaMethod` | `uint8_t` | 1 | - | Always 0 (BF compatibility) |
    | `bfCompatVbatPidComp` | `uint8_t` | 1 | - | Always 0 (BF compatibility) |
    | `bfCompatSetpointRelaxRatio` | `uint8_t` | 1 | - | Always 0 (BF compatibility) |
    | `reserved1` | `uint8_t` | 1 | - | Always 0 |
    | `legacyPidSumLimit` | `uint16_t` | 2 | - | Always 0 (Legacy) |
    | `bfCompatItermThrottleGain` | `uint8_t` | 1 | - | Always 0 (BF compatibility) |
    | `accelLimitRollPitch` | `uint16_t` | 2 | dps / 10 | Axis acceleration limit for Roll/Pitch / 10 (`pidProfile()->axisAccelerationLimitRollPitch / 10`) |
    | `accelLimitYaw` | `uint16_t` | 2 | dps / 10 | Axis acceleration limit for Yaw / 10 (`pidProfile()->axisAccelerationLimitYaw / 10`) |
*   **Notes:** Acceleration limits are scaled by 10 for compatibility.

### `MSP_SET_PID_ADVANCED` (95 / 0x5F)

*   **Direction:** In
*   **Description:** Sets advanced PID tuning parameters.
*   **Payload:** (Matches `MSP_PID_ADVANCED` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `legacyRollPitchItermIgnore` | `uint16_t` | 2 | - | Ignored |
    | `legacyYawItermIgnore` | `uint16_t` | 2 | - | Ignored |
    | `legacyYawPLimit` | `uint16_t` | 2 | - | Ignored |
    | `bfCompatDeltaMethod` | `uint8_t` | 1 | - | Ignored |
    | `bfCompatVbatPidComp` | `uint8_t` | 1 | - | Ignored |
    | `bfCompatSetpointRelaxRatio` | `uint8_t` | 1 | - | Ignored |
    | `reserved1` | `uint8_t` | 1 | - | Ignored |
    | `legacyPidSumLimit` | `uint16_t` | 2 | - | Ignored |
    | `bfCompatItermThrottleGain` | `uint8_t` | 1 | - | Ignored |
    | `accelLimitRollPitch` | `uint16_t` | 2 | dps / 10 | Sets `pidProfileMutable()->axisAccelerationLimitRollPitch = value * 10` |
    | `accelLimitYaw` | `uint16_t` | 2 | dps / 10 | Sets `pidProfileMutable()->axisAccelerationLimitYaw = value * 10` |
*   **Notes:** Expects 17 bytes.

### `MSP_SENSOR_CONFIG` (96 / 0x60)

*   **Direction:** Out
*   **Description:** Retrieves the configured hardware type for various sensors.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `accHardware` | `uint8_t` | 1 | Enum (`accHardware_e`): Accelerometer hardware type (`accelerometerConfig()->acc_hardware`) |
    | `baroHardware` | `uint8_t` | 1 | Enum (`baroHardware_e`): Barometer hardware type (`barometerConfig()->baro_hardware`). 0 if `USE_BARO` disabled |
    | `magHardware` | `uint8_t` | 1 | Enum (`magHardware_e`): Magnetometer hardware type (`compassConfig()->mag_hardware`). 0 if `USE_MAG` disabled |
    | `pitotHardware` | `uint8_t` | 1 | Enum (`pitotHardware_e`): Pitot tube hardware type (`pitotmeterConfig()->pitot_hardware`). 0 if `USE_PITOT` disabled |
    | `rangefinderHardware` | `uint8_t` | 1 | Enum (`rangefinderHardware_e`): Rangefinder hardware type (`rangefinderConfig()->rangefinder_hardware`). 0 if `USE_RANGEFINDER` disabled |
    | `opflowHardware` | `uint8_t` | 1 | Enum (`opticalFlowHardware_e`): Optical flow hardware type (`opticalFlowConfig()->opflow_hardware`). 0 if `USE_OPFLOW` disabled |

### `MSP_SET_SENSOR_CONFIG` (97 / 0x61)

*   **Direction:** In
*   **Description:** Sets the configured hardware type for various sensors.
*   **Payload:** (Matches `MSP_SENSOR_CONFIG` structure)
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `accHardware` | `uint8_t` | 1 | Sets `accelerometerConfigMutable()->acc_hardware` |
    | `baroHardware` | `uint8_t` | 1 | Sets `barometerConfigMutable()->baro_hardware` (if `USE_BARO`) |
    | `magHardware` | `uint8_t` | 1 | Sets `compassConfigMutable()->mag_hardware` (if `USE_MAG`) |
    | `pitotHardware` | `uint8_t` | 1 | Sets `pitotmeterConfigMutable()->pitot_hardware` (if `USE_PITOT`) |
    | `rangefinderHardware` | `uint8_t` | 1 | Sets `rangefinderConfigMutable()->rangefinder_hardware` (if `USE_RANGEFINDER`) |
    | `opflowHardware` | `uint8_t` | 1 | Sets `opticalFlowConfigMutable()->opflow_hardware` (if `USE_OPFLOW`) |
*   **Notes:** Expects 6 bytes.

### `MSP_SPECIAL_PARAMETERS` (98 / 0x62)

*   **Direction:** Out
*   **Description:** Betaflight specific, likely unused/unimplemented in INAV.
*   **Notes:** Not implemented in INAV `fc_msp.c`.

### `MSP_SET_SPECIAL_PARAMETERS` (99 / 0x63)

*   **Direction:** In
*   **Description:** Betaflight specific, likely unused/unimplemented in INAV.
*   **Notes:** Not implemented in INAV `fc_msp.c`.

---

## MSPv1 MultiWii Original Commands (100-127, 130)

These are commands originating from the MultiWii project.

### `MSP_IDENT` (100 / 0x64)

*   **Direction:** Out
*   **Description:** Provides basic flight controller identity information. Not implemented in modern INAV, but used by legacy versions and MultiWii.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
	| MultiWii version | uint8_t | 1 | n/a | Scaled version major*100+minor |
	| Mixer Mode |  uint8_t | 1 | Enum | Mixer type |
	| MSP Version | uint8_t | 1 | n/a | Scaled version major*100+minor |
	| Platform Capability | uint32_t | | Bitmask of MW capabilities |
* **Notes:** Obsolete. Listed for legacy compatibility only.

### `MSP_STATUS` (101 / 0x65)

*   **Direction:** Out
*   **Description:** Provides basic flight controller status including cycle time, errors, sensor status, active modes (first 32), and the current configuration profile.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `cycleTime` | `uint16_t` | 2 | µs | Main loop cycle time (`cycleTime`) |
    | `i2cErrors` | `uint16_t` | 2 | Count | Number of I2C errors encountered (`i2cGetErrorCounter()`). 0 if `USE_I2C` not defined |
    | `sensorStatus` | `uint16_t` | 2 | Bitmask | Bitmask indicating available/active sensors (`packSensorStatus()`). See notes |
    | `activeModesLow` | `uint32_t` | 4 | Bitmask | First 32 bits of the active flight modes bitmask (`packBoxModeFlags()`) |
    | `profile` | `uint8_t` | 1 | Index | Current configuration profile index (0-based) (`getConfigProfile()`) |
*   **Notes:** Superseded by `MSP_STATUS_EX` and `MSP2_INAV_STATUS`. `sensorStatus` bitmask: (Bit 0: ACC, 1: BARO, 2: MAG, 3: GPS, 4: RANGEFINDER, 5: GYRO). `activeModesLow` only contains the first 32 modes; use `MSP_ACTIVEBOXES` for the full set.

### `MSP_RAW_IMU` (102 / 0x66)

*   **Direction:** Out
*   **Description:** Provides raw sensor readings from the IMU (Accelerometer, Gyroscope, Magnetometer).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `accX` | `int16_t` | 2 | ~1/512 G | Raw accelerometer X reading, scaled (`acc.accADCf[X] * 512`) |
    | `accY` | `int16_t` | 2 | ~1/512 G | Raw accelerometer Y reading, scaled (`acc.accADCf[Y] * 512`) |
    | `accZ` | `int16_t` | 2 | ~1/512 G | Raw accelerometer Z reading, scaled (`acc.accADCf[Z] * 512`) |
    | `gyroX` | `int16_t` | 2 | deg/s | Gyroscope X-axis rate (`gyroRateDps(X)`) |
    | `gyroY` | `int16_t` | 2 | deg/s | Gyroscope Y-axis rate (`gyroRateDps(Y)`) |
    | `gyroZ` | `int16_t` | 2 | deg/s | Gyroscope Z-axis rate (`gyroRateDps(Z)`) |
    | `magX` | `int16_t` | 2 | Raw units | Raw magnetometer X reading (`mag.magADC[X]`). 0 if `USE_MAG` disabled |
    | `magY` | `int16_t` | 2 | Raw units | Raw magnetometer Y reading (`mag.magADC[Y]`). 0 if `USE_MAG` disabled |
    | `magZ` | `int16_t` | 2 | Raw units | Raw magnetometer Z reading (`mag.magADC[Z]`). 0 if `USE_MAG` disabled |
*   **Notes:** Acc scaling is approximate (512 LSB/G). Mag units depend on the sensor.

### `MSP_SERVO` (103 / 0x67)

*   **Direction:** Out
*   **Description:** Provides the current output values for all supported servos.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `servoOutputs` | `int16_t[MAX_SUPPORTED_SERVOS]` | `MAX_SUPPORTED_SERVOS * 2` | PWM | Array of current servo output values (typically 1000-2000) |

### `MSP_MOTOR` (104 / 0x68)

*   **Direction:** Out
*   **Description:** Provides the current output values for the first 8 motors.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `motorOutputs` | `uint16_t[8]` | 16 | PWM | Array of current motor output values (typically 1000-2000). Values beyond `MAX_SUPPORTED_MOTORS` are 0 |

### `MSP_RC` (105 / 0x69)

*   **Direction:** Out
*   **Description:** Provides the current values of the received RC channels.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `rcChannels` | `uint16_t[]` | `rxRuntimeConfig.channelCount * 2` | PWM | Array of current RC channel values (typically 1000-2000). Length depends on detected channels |

### `MSP_RAW_GPS` (106 / 0x6A)

*   **Direction:** Out
*   **Description:** Provides raw GPS data (fix status, coordinates, altitude, speed, course).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `fixType` | `uint8_t` | 1 | Enum | Enum `gpsFixType_e` GPS fix type (`gpsSol.fixType`) |
    | `numSat` | `uint8_t` | 1 | Count | Number of satellites used in solution (`gpsSol.numSat`) |
    | `latitude` | `int32_t` | 4 | deg * 1e7 | Latitude (`gpsSol.llh.lat`) |
    | `longitude` | `int32_t` | 4 | deg * 1e7 | Longitude (`gpsSol.llh.lon`) |
    | `altitude` | `int16_t` | 2 | meters | Altitude above MSL (`gpsSol.llh.alt / 100`) |
    | `speed` | `uint16_t` | 2 | cm/s | Ground speed (`gpsSol.groundSpeed`) |
    | `groundCourse` | `uint16_t` | 2 | deci-degrees | Ground course (`gpsSol.groundCourse`) |
    | `hdop` | `uint16_t` | 2 | HDOP * 100 | Horizontal Dilution of Precision (`gpsSol.hdop`) |
*   **Notes:** Only available if `USE_GPS` is defined. Altitude is truncated to meters.

### `MSP_COMP_GPS` (107 / 0x6B)

*   **Direction:** Out
*   **Description:** Provides computed GPS values: distance and direction to home.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `distanceToHome` | `uint16_t` | 2 | meters | Distance to the home point (`GPS_distanceToHome`) |
    | `directionToHome` | `uint16_t` | 2 | degrees | Direction to the home point (0-360) (`GPS_directionToHome`) |
    | `gpsHeartbeat` | `uint8_t` | 1 | Boolean | Indicates if GPS data is being received (`gpsSol.flags.gpsHeartbeat`) |
*   **Notes:** Only available if `USE_GPS` is defined.

### `MSP_ATTITUDE` (108 / 0x6C)

*   **Direction:** Out
*   **Description:** Provides the current attitude estimate (roll, pitch, yaw).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `roll` | `int16_t` | 2 | deci-degrees | Roll angle (`attitude.values.roll`) |
    | `pitch` | `int16_t` | 2 | deci-degrees | Pitch angle (`attitude.values.pitch`) |
    | `yaw` | `int16_t` | 2 | degrees | Yaw/Heading angle (`DECIDEGREES_TO_DEGREES(attitude.values.yaw)`) |
*   **Notes:** Yaw is converted from deci-degrees to degrees.

### `MSP_ALTITUDE` (109 / 0x6D)

*   **Direction:** Out
*   **Description:** Provides estimated altitude, vertical speed (variometer), and raw barometric altitude.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `estimatedAltitude` | `int32_t` | 4 | cm | Estimated altitude above home/sea level (`getEstimatedActualPosition(Z)`) |
    | `variometer` | `int16_t` | 2 | cm/s | Estimated vertical speed (`getEstimatedActualVelocity(Z)`) |
    | `baroAltitude` | `int32_t` | 4 | cm | Latest raw altitude from barometer (`baroGetLatestAltitude()`). 0 if `USE_BARO` disabled |

### `MSP_ANALOG` (110 / 0x6E)

*   **Direction:** Out
*   **Description:** Provides analog sensor readings: battery voltage, current consumption (mAh), RSSI, and current draw (Amps).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `vbat` | `uint8_t` | 1 | 0.1V | Battery voltage, scaled (`getBatteryVoltage() / 10`), constrained 0-255 |
    | `mAhDrawn` | `uint16_t` | 2 | mAh | Consumed battery capacity (`getMAhDrawn()`), constrained 0-65535 |
    | `rssi` | `uint16_t` | 2 | 0-1023 or % | Received Signal Strength Indicator (`getRSSI()`). Units depend on source |
    | `amperage` | `int16_t` | 2 | 0.01A | Current draw (`getAmperage()`), constrained -32768 to 32767 |
*   **Notes:** Superseded by `MSP2_INAV_ANALOG` which provides higher precision and more fields.

### `MSP_RC_TUNING` (111 / 0x6F)

*   **Direction:** Out
*   **Description:** Retrieves RC tuning parameters (rates, expos, TPA) for the current control rate profile.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `legacyRcRate` | `uint8_t` | 1 | Always 100 (Legacy, unused) |
    | `rcExpo` | `uint8_t` | 1 | Roll/Pitch RC Expo (`currentControlRateProfile->stabilized.rcExpo8`) |
    | `rollRate` | `uint8_t` | 1 | Roll Rate (`currentControlRateProfile->stabilized.rates[FD_ROLL]`) |
    | `pitchRate` | `uint8_t` | 1 | Pitch Rate (`currentControlRateProfile->stabilized.rates[FD_PITCH]`) |
    | `yawRate` | `uint8_t` | 1 | Yaw Rate (`currentControlRateProfile->stabilized.rates[FD_YAW]`) |
    | `dynamicThrottlePID` | `uint8_t` | 1 | Dynamic Throttle PID (TPA) value (`currentControlRateProfile->throttle.dynPID`) |
    | `throttleMid` | `uint8_t` | 1 | Throttle Midpoint (`currentControlRateProfile->throttle.rcMid8`) |
    | `throttleExpo` | `uint8_t` | 1 | Throttle Expo (`currentControlRateProfile->throttle.rcExpo8`) |
    | `tpaBreakpoint` | `uint16_t` | 2 | Throttle PID Attenuation (TPA) breakpoint (`currentControlRateProfile->throttle.pa_breakpoint`) |
    | `rcYawExpo` | `uint8_t` | 1 | Yaw RC Expo (`currentControlRateProfile->stabilized.rcYawExpo8`) |
*   **Notes:** Superseded by `MSP2_INAV_RATE_PROFILE` which includes manual rates/expos.

### `MSP_ACTIVEBOXES` (113 / 0x71)

*   **Direction:** Out
*   **Description:** Provides the full bitmask of currently active flight modes (boxes).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `activeModes` | `boxBitmask_t` | `sizeof(boxBitmask_t)` | Bitmask of all active modes (`packBoxModeFlags()`). Size depends on `boxBitmask_t` definition |
*   **Notes:** Use this instead of `MSP_STATUS` or `MSP_STATUS_EX` if more than 32 modes are possible.

### `MSP_MISC` (114 / 0x72)

*   **Direction:** Out
*   **Description:** Retrieves miscellaneous configuration settings, mostly related to RC, GPS, Mag, and Battery voltage (legacy formats).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `midRc` | `uint16_t` | 2 | PWM | Mid RC value (`PWM_RANGE_MIDDLE`, typically 1500) |
    | `legacyMinThrottle` | `uint16_t` | 2 | - | Always 0 (Legacy) |
    | `maxThrottle` | `uint16_t` | 2 | PWM | Maximum throttle command (`getMaxThrottle()`) |
    | `minCommand` | `uint16_t` | 2 | PWM | Minimum motor command when disarmed (`motorConfig()->mincommand`) |
    | `failsafeThrottle` | `uint16_t` | 2 | PWM | Failsafe throttle level (`currentBatteryProfile->failsafe_throttle`) |
    | `gpsType` | `uint8_t` | 1 | Enum | Enum `gpsProvider_e` GPS provider type (`gpsConfig()->provider`). 0 if `USE_GPS` disabled |
    | `legacyGpsBaud` | `uint8_t` | 1 | - | Always 0 (Legacy) |
    | `gpsSbasMode` | `uint8_t` | 1 | Enum | Enum `sbasMode_e` GPS SBAS mode (`gpsConfig()->sbasMode`). 0 if `USE_GPS` disabled |
    | `legacyMwCurrentOut` | `uint8_t` | 1 | - | Always 0 (Legacy) |
    | `rssiChannel` | `uint8_t` | 1 | Index | RSSI channel index (1-based) (`rxConfig()->rssi_channel`) |
    | `reserved1` | `uint8_t` | 1 | - | Always 0 |
    | `magDeclination` | `uint16_t` | 2 | 0.1 degrees | Magnetic declination / 10 (`compassConfig()->mag_declination / 10`). 0 if `USE_MAG` disabled |
    | `vbatScale` | `uint8_t` | 1 | Scale / 10 | Voltage scale / 10 (`batteryMetersConfig()->voltage.scale / 10`). 0 if `USE_ADC` disabled |
    | `vbatMinCell` | `uint8_t` | 1 | 0.1V | Min cell voltage / 10 (`currentBatteryProfile->voltage.cellMin / 10`). 0 if `USE_ADC` disabled |
    | `vbatMaxCell` | `uint8_t` | 1 | 0.1V | Max cell voltage / 10 (`currentBatteryProfile->voltage.cellMax / 10`). 0 if `USE_ADC` disabled |
    | `vbatWarningCell` | `uint8_t` | 1 | 0.1V | Warning cell voltage / 10 (`currentBatteryProfile->voltage.cellWarning / 10`). 0 if `USE_ADC` disabled |
*   **Notes:** Superseded by `MSP2_INAV_MISC` and other specific commands which offer better precision and more fields.

### `MSP_BOXNAMES` (116 / 0x74)

*   **Direction:** Out
*   **Description:** Provides a semicolon-separated string containing the names of all available flight modes (boxes).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `boxNamesString` | `char[]` | Variable | String containing mode names separated by ';'. Null termination not guaranteed by MSP, relies on payload size. (`serializeBoxNamesReply()`) |
*   **Notes:** The exact set of names depends on compiled features and configuration. Due to the size of the payload, it is recommended that [`MSP_BOXIDS`](#msp_boxids-119--0x77) is used instead.

### `MSP_PIDNAMES` (117 / 0x75)

*   **Direction:** Out
*   **Description:** Provides a semicolon-separated string containing the names of the PID controllers.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `pidNamesString` | `char[]` | Variable | String "ROLL;PITCH;YAW;ALT;Pos;PosR;NavR;LEVEL;MAG;VEL;". Null termination not guaranteed by MSP |

### `MSP_WP` (118 / 0x76)

*   **Direction:** In/Out
*   **Description:** Get/Set a single waypoint from the mission plan.
*   **Request Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `waypointIndex` | `uint8_t` | 1 | Index of the waypoint to retrieve (0 to `NAV_MAX_WAYPOINTS - 1`) |
*   **Reply Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `waypointIndex` | `uint8_t` | 1 | Index | Index of the returned waypoint |
    | `action` | `uint8_t` | 1 | Enum | Enum `navWaypointActions_e` Waypoint action type |
    | `latitude` | `int32_t` | 4 | deg * 1e7 | Latitude coordinate |
    | `longitude` | `int32_t` | 4 | deg * 1e7 | Longitude coordinate |
    | `altitude` | `int32_t` | 4 | cm | Altitude coordinate (relative to home or sea level, see flag) |
    | `param1` | `uint16_t` | 2 | Varies | Parameter 1 (meaning depends on action) |
    | `param2` | `uint16_t` | 2 | Varies | Parameter 2 (meaning depends on action) |
    | `param3` | `uint16_t` | 2 | Varies | Parameter 3 (meaning depends on action) |
    | `flag` | `uint8_t` | 1 | Bitmask | Waypoint flags (`NAV_WP_FLAG_*`) |
*   **Notes:** See `navWaypoint_t` and `navWaypointActions_e`.

### `MSP_BOXIDS` (119 / 0x77)

*   **Direction:** Out
*   **Description:** Provides a list of permanent IDs associated with the available flight modes (boxes).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `boxIds` | `uint8_t[]` | Variable | Array of permanent IDs for each configured box (`serializeBoxReply()`). Length depends on number of boxes |
*   **Notes:** Useful for mapping mode range configurations (`MSP_MODE_RANGES`) back to user-understandable modes via `MSP_BOXNAMES`.

### `MSP_SERVO_CONFIGURATIONS` (120 / 0x78)

*   **Direction:** Out
*   **Description:** Retrieves the configuration parameters for all supported servos (min, max, middle, rate). Legacy format with unused fields.
*   **Payload:** Repeated `MAX_SUPPORTED_SERVOS` times:
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `min` | `uint16_t` | 2 | PWM | Minimum servo endpoint (`servoParams(i)->min`) |
    | `max` | `uint16_t` | 2 | PWM | Maximum servo endpoint (`servoParams(i)->max`) |
    | `middle` | `uint16_t` | 2 | PWM | Middle/Neutral servo position (`servoParams(i)->middle`) |
    | `rate` | `uint8_t` | 1 | % (-100 to 100) | Servo rate/scaling (`servoParams(i)->rate`) |
    | `reserved1` | `uint8_t` | 1 | - | Always 0 |
    | `reserved2` | `uint8_t` | 1 | - | Always 0 |
    | `legacyForwardChan` | `uint8_t` | 1 | - | Always 255 (Legacy) |
    | `legacyReversedSources` | `uint32_t` | 4 | - | Always 0 (Legacy) |
*   **Notes:** Superseded by `MSP2_INAV_SERVO_CONFIG` which has a cleaner structure.

### `MSP_NAV_STATUS` (121 / 0x79)

*   **Direction:** Out
*   **Description:** Retrieves the current status of the navigation system.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `navMode` | `uint8_t` | 1 | Enum (`NAV_MODE_*`): Current navigation mode (None, RTH, WP, Hold, etc.) (`NAV_Status.mode`) |
    | `navState` | `uint8_t` | 1 | Enum (`NAV_STATE_*`): Current navigation state (`NAV_Status.state`) |
    | `activeWpAction` | `uint8_t` | 1 | Enum (`navWaypointActions_e`): Action of the currently executing waypoint (`NAV_Status.activeWpAction`) |
    | `activeWpNumber` | `uint8_t` | 1 | Index: Index of the currently executing waypoint (`NAV_Status.activeWpNumber`) |
    | `navError` | `uint8_t` | 1 | Enum (`NAV_ERROR_*`): Current navigation error code (`NAV_Status.error`) |
    | `targetHeading` | `int16_t` | 2 | degrees: Target heading for heading controller (`getHeadingHoldTarget()`) |
*   **Notes:** Requires `USE_GPS`.

### `MSP_NAV_CONFIG` (122 / 0x7A)

*   **Not implemented**

### `MSP_3D` (124 / 0x7C)

*   **Direction:** Out
*   **Description:** Retrieves settings related to 3D/reversible motor operation.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `deadbandLow` | `uint16_t` | 2 | PWM | Lower deadband limit for 3D mode (`reversibleMotorsConfig()->deadband_low`) |
    | `deadbandHigh` | `uint16_t` | 2 | PWM | Upper deadband limit for 3D mode (`reversibleMotorsConfig()->deadband_high`) |
    | `neutral` | `uint16_t` | 2 | PWM | Neutral throttle point for 3D mode (`reversibleMotorsConfig()->neutral`) |
*   **Notes:** Requires reversible motor support.

### `MSP_RC_DEADBAND` (125 / 0x7D)

*   **Direction:** Out
*   **Description:** Retrieves RC input deadband settings.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `deadband` | `uint8_t` | 1 | PWM | General RC deadband for Roll/Pitch (`rcControlsConfig()->deadband`) |
    | `yawDeadband` | `uint8_t` | 1 | PWM | Specific deadband for Yaw (`rcControlsConfig()->yaw_deadband`) |
    | `altHoldDeadband` | `uint8_t` | 1 | PWM | Deadband for altitude hold adjustments (`rcControlsConfig()->alt_hold_deadband`) |
    | `throttleDeadband` | `uint16_t` | 2 | PWM | Deadband around throttle mid-stick (`rcControlsConfig()->mid_throttle_deadband`) |

### `MSP_SENSOR_ALIGNMENT` (126 / 0x7E)

*   **Direction:** Out
*   **Description:** Retrieves sensor alignment settings (legacy format).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `gyroAlign` | `uint8_t` | 1 | Always 0 (Legacy alignment enum) |
    | `accAlign` | `uint8_t` | 1 | Always 0 (Legacy alignment enum) |
    | `magAlign` | `uint8_t` | 1 | Magnetometer alignment (`compassConfig()->mag_align`). 0 if `USE_MAG` disabled |
    | `opflowAlign` | `uint8_t` | 1 | Optical flow alignment (`opticalFlowConfig()->opflow_align`). 0 if `USE_OPFLOW` disabled |
*   **Notes:** Board alignment is now typically handled by `MSP_BOARD_ALIGNMENT`. This returns legacy enum values where applicable.

### `MSP_LED_STRIP_MODECOLOR` (127 / 0x7F)

*   **Direction:** Out
*   **Description:** Retrieves the color index assigned to each LED mode and function/direction combination, including special colors.
*   **Payload:** Repeated (`LED_MODE_COUNT * LED_DIRECTION_COUNT` + `LED_SPECIAL_COLOR_COUNT`) times:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `modeIndex` | `uint8_t` | 1 | Index of the LED mode Enum (`ledModeIndex_e`). `LED_MODE_COUNT` for special colors |
    | `directionOrSpecialIndex` | `uint8_t` | 1 | Index of the direction Enum (`ledDirection_e`) or special color (`ledSpecialColor_e`) |
    | `colorIndex` | `uint8_t` | 1 | Index of the color assigned from `ledStripConfig()->colors` |
*   **Notes:** Only available if `USE_LED_STRIP` is defined. Allows mapping modes/directions/specials to configured colors.

### `MSP_BATTERY_STATE` (130 / 0x82)

*   **Direction:** Out
*   **Description:** Provides battery state information, formatted primarily for DJI FPV Goggles compatibility.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `cellCount` | `uint8_t` | 1 | Count | Number of battery cells (`getBatteryCellCount()`) |
    | `capacity` | `uint16_t` | 2 | mAh | Battery capacity (`currentBatteryProfile->capacity.value`) |
    | `vbatScaled` | `uint8_t` | 1 | 0.1V | Battery voltage / 10 (`getBatteryVoltage() / 10`) |
    | `mAhDrawn` | `uint16_t` | 2 | mAh | Consumed capacity (`getMAhDrawn()`) |
    | `amperage` | `int16_t` | 2 | 0.01A | Current draw (`getAmperage()`) |
    | `batteryState` | `uint8_t` | 1 | Enum | Enum `batteryState_e` Current battery state (`getBatteryState()`, see `BATTERY_STATE_*`) |
    | `vbatActual` | `uint16_t` | 2 | 0.01V | Actual battery voltage (`getBatteryVoltage()`) |
*   **Notes:** Only available if `USE_DJI_HD_OSD` or `USE_MSP_DISPLAYPORT` is defined. Some values are duplicated from `MSP_ANALOG` / `MSP2_INAV_ANALOG` but potentially with different scaling/types.

### `MSP_VTXTABLE_BAND` (137 / 0x89)

*   **Direction:** In/Out (?)
*   **Description:** Retrieves information about a specific VTX band from the VTX table. (Implementation missing in provided `fc_msp.c`)
*   **Notes:** The ID is defined, but no handler exists in the provided C code. Likely intended to query band names and frequencies.

### `MSP_VTXTABLE_POWERLEVEL` (138 / 0x8A)

*   **Direction:** In/Out
*   **Description:** Retrieves information about a specific VTX power level from the VTX table.
*   **Request Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `powerLevelIndex` | `uint8_t` | 1 | 1-based index of the power level to query |
*   **Reply Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `powerLevelIndex` | `uint8_t` | 1 | 1-based index of the returned power level |
    | `powerValue` | `uint16_t` | 2 | Always 0 (Actual power value in mW is not stored/returned via MSP) |
    | `labelLength` | `uint8_t` | 1 | Length of the power level label string that follows |
    | `label` | `char[]` | Variable | Power level label string (e.g., "25", "200"). Length given by previous field |
*   **Notes:** Requires `USE_VTX_CONTROL`. Returns error if index is out of bounds. The `powerValue` field is unused.

---

## MSPv1 MultiWii Original Input Commands (200-221)

These commands are sent *to* the FC.

### `MSP_SET_RAW_RC` (200 / 0xC8)

*   **Direction:** In
*   **Description:** Provides raw RC channel data to the flight controller, typically used when the receiver is connected via MSP (e.g., MSP RX feature).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `rcChannels` | `uint16_t[]` | Variable (2 * channelCount) | PWM | Array of RC channel values (typically 1000-2000). Number of channels determined by payload size |
*   **Notes:** Requires `USE_RX_MSP`. Maximum channels `MAX_SUPPORTED_RC_CHANNEL_COUNT`. Calls `rxMspFrameReceive()`.

### `MSP_SET_RAW_GPS` (201 / 0xC9)

*   **Direction:** In
*   **Description:** Provides raw GPS data to the flight controller, typically for simulation or external GPS injection.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `fixType` | `uint8_t` | 1 | Enum | Enum `gpsFixType_e` GPS fix type |
    | `numSat` | `uint8_t` | 1 | Count | Number of satellites |
    | `latitude` | `int32_t` | 4 | deg * 1e7 | Latitude |
    | `longitude` | `int32_t` | 4 | deg * 1e7 | Longitude |
    | `altitude` | `int16_t` | 2 | meters | Altitude (converted to cm internally) |
    | `speed` | `uint16_t` | 2 | cm/s | Ground speed |
    | `groundCourse` | `uint16_t` | 2 | ??? | Ground course (units unclear from code, likely degrees or deci-degrees, ignored in current code) |
*   **Notes:** Requires `USE_GPS`. Expects 14 bytes. Updates `gpsSol` structure and calls `onNewGPSData()`. Note the altitude unit mismatch (meters in MSP, cm internal). Does not provide velocity components.

### `MSP_SET_BOX` (203 / 0xCB)

*   **Direction:** In
*   **Description:** Sets the state of flight modes (boxes). (Likely unused/obsolete in INAV).
*   **Notes:** Not implemented in INAV `fc_msp.c`. Mode changes are typically handled via RC channels (`MSP_MODE_RANGES`).

### `MSP_SET_RC_TUNING` (204 / 0xCC)

*   **Direction:** In
*   **Description:** Sets RC tuning parameters (rates, expos, TPA) for the current control rate profile.
*   **Payload:** (Matches `MSP_RC_TUNING` outgoing structure)
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `legacyRcRate` | `uint8_t` | 1 | Ignored |
    | `rcExpo` | `uint8_t` | 1 | Sets `currentControlRateProfile->stabilized.rcExpo8` |
    | `rollRate` | `uint8_t` | 1 | Sets `currentControlRateProfile->stabilized.rates[FD_ROLL]` (constrained) |
    | `pitchRate` | `uint8_t` | 1 | Sets `currentControlRateProfile->stabilized.rates[FD_PITCH]` (constrained) |
    | `yawRate` | `uint8_t` | 1 | Sets `currentControlRateProfile->stabilized.rates[FD_YAW]` (constrained) |
    | `dynamicThrottlePID` | `uint8_t` | 1 | Sets `currentControlRateProfile->throttle.dynPID` (constrained) |
    | `throttleMid` | `uint8_t` | 1 | Sets `currentControlRateProfile->throttle.rcMid8` |
    | `throttleExpo` | `uint8_t` | 1 | Sets `currentControlRateProfile->throttle.rcExpo8` |
    | `tpaBreakpoint` | `uint16_t` | 2 | Sets `currentControlRateProfile->throttle.pa_breakpoint` |
    | `rcYawExpo` | `uint8_t` | 1 | (Optional) Sets `currentControlRateProfile->stabilized.rcYawExpo8` |
*   **Notes:** Expects 10 or 11 bytes. Calls `schedulePidGainsUpdate()`. Superseded by `MSP2_INAV_SET_RATE_PROFILE`.

### `MSP_ACC_CALIBRATION` (205 / 0xCD)

*   **Direction:** In
*   **Description:** Starts the accelerometer calibration procedure.
*   **Payload:** None
*   **Notes:** Will fail if armed. Calls `accStartCalibration()`.

### `MSP_MAG_CALIBRATION` (206 / 0xCE)

*   **Direction:** In
*   **Description:** Starts the magnetometer calibration procedure.
*   **Payload:** None
*   **Notes:** Will fail if armed. Enables the `CALIBRATE_MAG` state flag.

### `MSP_SET_MISC` (207 / 0xCF)

*   **Direction:** In
*   **Description:** Sets miscellaneous configuration settings (legacy formats/scaling).
*   **Payload:** (Matches `MSP_MISC` outgoing structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `midRc` | `uint16_t` | 2 | PWM | Ignored |
    | `legacyMinThrottle` | `uint16_t` | 2 | - | Ignored |
    | `legacyMaxThrottle` | `uint16_t` | 2 | - | Ignored |
    | `minCommand` | `uint16_t` | 2 | PWM | Sets `motorConfigMutable()->mincommand` (constrained 0-PWM_RANGE_MAX) |
    | `failsafeThrottle` | `uint16_t` | 2 | PWM | Sets `currentBatteryProfileMutable->failsafe_throttle` (constrained PWM_RANGE_MIN/MAX) |
    | `gpsType` | `uint8_t` | 1 | Enum | Enum `gpsProvider_e` (Sets `gpsConfigMutable()->provider`)|
    | `legacyGpsBaud` | `uint8_t` | 1 | - | Ignored |
    | `gpsSbasMode` | `uint8_t` | 1 | Enum | Enum `sbasMode_e` (Sets `gpsConfigMutable()->sbasMode`) |
    | `legacyMwCurrentOut` | `uint8_t` | 1 | - | Ignored |
    | `rssiChannel` | `uint8_t` | 1 | Index | Sets `rxConfigMutable()->rssi_channel` (constrained 0-MAX_SUPPORTED_RC_CHANNEL_COUNT). Updates source |
    | `reserved1` | `uint8_t` | 1 | - | Ignored |
    | `magDeclination` | `uint16_t` | 2 | 0.1 degrees | Sets `compassConfigMutable()->mag_declination = value * 10` (if `USE_MAG`) |
    | `vbatScale` | `uint8_t` | 1 | Scale / 10 | Sets `batteryMetersConfigMutable()->voltage.scale = value * 10` (if `USE_ADC`) |
    | `vbatMinCell` | `uint8_t` | 1 | 0.1V | Sets `currentBatteryProfileMutable->voltage.cellMin = value * 10` (if `USE_ADC`) |
    | `vbatMaxCell` | `uint8_t` | 1 | 0.1V | Sets `currentBatteryProfileMutable->voltage.cellMax = value * 10` (if `USE_ADC`) |
    | `vbatWarningCell` | `uint8_t` | 1 | 0.1V | Sets `currentBatteryProfileMutable->voltage.cellWarning = value * 10` (if `USE_ADC`) |
*   **Notes:** Expects 22 bytes. Superseded by `MSP2_INAV_SET_MISC`.

### `MSP_RESET_CONF` (208 / 0xD0)

*   **Direction:** In
*   **Description:** Resets all configuration settings to their default values and saves to EEPROM.
*   **Payload:** None
*   **Notes:** Will fail if armed. Suspends RX, calls `resetEEPROM()`, `writeEEPROM()`, `readEEPROM()`, resumes RX. Use with caution!

### `MSP_SET_WP` (209 / 0xD1)

*   **Direction:** In
*   **Description:** Sets a single waypoint in the mission plan.
*   **Payload:** (Matches `MSP_WP` reply structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `waypointIndex` | `uint8_t` | 1 | Index | Index of the waypoint to set (0 to `NAV_MAX_WAYPOINTS - 1`) |
    | `action` | `uint8_t` | 1 | Enum | Enum `navWaypointActions_e` Waypoint action type |
    | `latitude` | `int32_t` | 4 | deg * 1e7 | Latitude coordinate |
    | `longitude` | `int32_t` | 4 | deg * 1e7 | Longitude coordinate |
    | `altitude` | `int32_t` | 4 | cm | Altitude coordinate |
    | `param1` | `uint16_t` | 2 | Varies | Parameter 1 |
    | `param2` | `uint16_t` | 2 | Varies | Parameter 2 |
    | `param3` | `uint16_t` | 2 | Varies | Parameter 3 |
    | `flag` | `uint8_t` | 1 | Bitmask | Waypoint flags |
*   **Notes:** Expects 21 bytes. Calls `setWaypoint()`. If `USE_FW_AUTOLAND` is enabled, this also interacts with autoland approach settings based on waypoint index and flags.

### `MSP_SELECT_SETTING` (210 / 0xD2)

*   **Direction:** In
*   **Description:** Selects the active configuration profile and saves it.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `profileIndex` | `uint8_t` | 1 | Index of the profile to activate (0-based) |
*   **Notes:** Will fail if armed. Calls `setConfigProfileAndWriteEEPROM()`.

### `MSP_SET_HEAD` (211 / 0xD3)

*   **Direction:** In
*   **Description:** Sets the target heading for the heading hold controller (e.g., during MAG mode).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `heading` | `int16_t` | 2 | degrees | Target heading (0-359) |
*   **Notes:** Expects 2 bytes. Calls `updateHeadingHoldTarget()`.

### `MSP_SET_SERVO_CONFIGURATION` (212 / 0xD4)

*   **Direction:** In
*   **Description:** Sets the configuration for a single servo (legacy format).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `servoIndex` | `uint8_t` | 1 | Index | Index of the servo to configure (0 to `MAX_SUPPORTED_SERVOS - 1`) |
    | `min` | `uint16_t` | 2 | PWM | Minimum servo endpoint |
    | `max` | `uint16_t` | 2 | PWM | Maximum servo endpoint |
    | `middle` | `uint16_t` | 2 | PWM | Middle/Neutral servo position |
    | `rate` | `uint8_t` | 1 | % | Servo rate/scaling |
    | `reserved1` | `uint8_t` | 1 | - | Ignored |
    | `reserved2` | `uint8_t` | 1 | - | Ignored |
    | `legacyForwardChan` | `uint8_t` | 1 | - | Ignored |
    | `legacyReversedSources` | `uint32_t` | 4 | - | Ignored |
*   **Notes:** Expects 15 bytes. Returns error if index is invalid. Calls `servoComputeScalingFactors()`. Superseded by `MSP2_INAV_SET_SERVO_CONFIG`.

### `MSP_SET_MOTOR` (214 / 0xD6)

*   **Direction:** In
*   **Description:** Sets the disarmed motor values, typically used for motor testing or propeller balancing functions in a configurator.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `motorValues` | `uint16_t[8]` | 16 | PWM | Array of motor values to set when disarmed. Only affects first `MAX_SUPPORTED_MOTORS` |
*   **Notes:** Expects 16 bytes. Modifies the `motor_disarmed` array. These values are *not* saved persistently.

### `MSP_SET_NAV_CONFIG` (215 / 0xD7)

*   **Not implemented**

### `MSP_SET_3D` (217 / 0xD9)

*   **Direction:** In
*   **Description:** Sets parameters related to 3D/reversible motor operation.
*   **Payload:** (Matches `MSP_3D` outgoing structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `deadbandLow` | `uint16_t` | 2 | PWM | Sets `reversibleMotorsConfigMutable()->deadband_low` |
    | `deadbandHigh` | `uint16_t` | 2 | PWM | Sets `reversibleMotorsConfigMutable()->deadband_high` |
    | `neutral` | `uint16_t` | 2 | PWM | Sets `reversibleMotorsConfigMutable()->neutral` |
*   **Notes:** Expects 6 bytes. Requires reversible motor support.

### `MSP_SET_RC_DEADBAND` (218 / 0xDA)

*   **Direction:** In
*   **Description:** Sets RC input deadband values.
*   **Payload:** (Matches `MSP_RC_DEADBAND` outgoing structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `deadband` | `uint8_t` | 1 | PWM | Sets `rcControlsConfigMutable()->deadband` |
    | `yawDeadband` | `uint8_t` | 1 | PWM | Sets `rcControlsConfigMutable()->yaw_deadband` |
    | `altHoldDeadband` | `uint8_t` | 1 | PWM | Sets `rcControlsConfigMutable()->alt_hold_deadband` |
    | `throttleDeadband` | `uint16_t` | 2 | PWM | Sets `rcControlsConfigMutable()->mid_throttle_deadband` |
*   **Notes:** Expects 5 bytes.

### `MSP_SET_RESET_CURR_PID` (219 / 0xDB)

*   **Direction:** In
*   **Description:** Resets the PIDs of the *current* profile to their default values. Does not save.
*   **Payload:** None
*   **Notes:** Calls `PG_RESET_CURRENT(pidProfile)`. To save, follow with `MSP_EEPROM_WRITE`.

### `MSP_SET_SENSOR_ALIGNMENT` (220 / 0xDC)

*   **Direction:** In
*   **Description:** Sets sensor alignment (legacy format).
*   **Payload:** (Matches `MSP_SENSOR_ALIGNMENT` outgoing structure)
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `gyroAlign` | `uint8_t` | 1 | Ignored |
    | `accAlign` | `uint8_t` | 1 | Ignored |
    | `magAlign` | `uint8_t` | 1 | Sets `compassConfigMutable()->mag_align` (if `USE_MAG`) |
    | `opflowAlign` | `uint8_t` | 1 | Sets `opticalFlowConfigMutable()->opflow_align` (if `USE_OPFLOW`) |
*   **Notes:** Expects 4 bytes. Use `MSP_SET_BOARD_ALIGNMENT` for primary board orientation.

### `MSP_SET_LED_STRIP_MODECOLOR` (221 / 0xDD)

*   **Direction:** In
*   **Description:** Sets the color index for a specific LED mode/function combination.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `modeIndex` | `uint8_t` | 1 | Index of the LED mode (`ledModeIndex_e` or `LED_MODE_COUNT` for special) |
    | `directionOrSpecialIndex` | `uint8_t` | 1 | Index of the direction or special color |
    | `colorIndex` | `uint8_t` | 1 | Index of the color to assign from `ledStripConfig()->colors` |
*   **Notes:** Only available if `USE_LED_STRIP` is defined. Expects 3 bytes. Returns error if setting fails (invalid index).

---

## MSPv1 System & Debug Commands (239-254)

### `MSP_SET_ACC_TRIM` (239 / 0xEF)

*   **Direction:** In
*   **Description:** Sets the accelerometer trim values (leveling calibration).
*   **Notes:** Not implemented in INAV `fc_msp.c`. Use `MSP_ACC_CALIBRATION`.

### `MSP_ACC_TRIM` (240 / 0xF0)

*   **Direction:** Out
*   **Description:** Gets the accelerometer trim values.
*   **Notes:** Not implemented in INAV `fc_msp.c`. Calibration data via `MSP_CALIBRATION_DATA`.

### `MSP_SERVO_MIX_RULES` (241 / 0xF1)

*   **Direction:** Out
*   **Description:** Retrieves the custom servo mixer rules (legacy format).
*   **Payload:** Repeated `MAX_SERVO_RULES` times:
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `targetChannel` | `uint8_t` | 1 | Index | Servo output channel index (0-based) |
    | `inputSource` | `uint8_t` | 1 | Enum | Enum `inputSource_e` Input source for the mix (RC chan, Roll, Pitch...) |
    | `rate` | `uint16_t` | 2 | % * 100? | Mixing rate/weight. Needs scaling check |
    | `speed` | `uint8_t` | 1 | 0-100 | Speed/Slew rate limit |
    | `reserved1` | `uint8_t` | 1 | - | Always 0 |
    | `legacyMax` | `uint8_t` | 1 | - | Always 100 (Legacy) |
    | `legacyBox` | `uint8_t` | 1 | - | Always 0 (Legacy) |
*   **Notes:** Superseded by `MSP2_INAV_SERVO_MIXER`.

### `MSP_SET_SERVO_MIX_RULE` (242 / 0xF2)

*   **Direction:** In
*   **Description:** Sets a single custom servo mixer rule (legacy format).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `ruleIndex` | `uint8_t` | 1 | Index | Index of the rule to set (0 to `MAX_SERVO_RULES - 1`) |
    | `targetChannel` | `uint8_t` | 1 | Index | Servo output channel index |
    | `inputSource` | `uint8_t` | 1 | Enum | Enum `inputSource_e` Input source for the mix |
    | `rate` | `uint16_t` | 2 | % * 100? | Mixing rate/weight |
    | `speed` | `uint8_t` | 1 | 0-100 | Speed/Slew rate limit |
    | `legacyMinMax` | `uint16_t` | 2 | - | Ignored |
    | `legacyBox` | `uint8_t` | 1 | - | Ignored |
*   **Notes:** Expects 9 bytes. Returns error if index invalid. Calls `loadCustomServoMixer()`. Superseded by `MSP2_INAV_SET_SERVO_MIXER`.

### `MSP_SET_PASSTHROUGH` (245 / 0xF5)

*   **Direction:** In/Out (Special: In command triggers passthrough mode, Reply confirms start)
*   **Description:** Enables serial passthrough mode to peripherals like ESCs (BLHeli 4-way) or other serial devices.
*   **Request Payload (Legacy - 4way):** None
*   **Request Payload (Extended):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `passthroughMode` | `uint8_t` | 1 | Type of passthrough Enum (`mspPassthroughType_e`: Serial ID, Serial Function, ESC 4way) |
    | `passthroughArgument` | `uint8_t` | 1 | Argument for the mode (e.g., Serial Port Identifier, Serial Function ID). Defaults to 0 if not sent |
*   **Reply Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `status` | `uint8_t` | 1 | 1 if passthrough started successfully, 0 on error (e.g., port not found). For 4way, returns number of ESCs found |
*   **Notes:** If successful, sets `mspPostProcessFn` to the appropriate handler (`mspSerialPassthroughFn` or `esc4wayProcess`). This handler takes over the serial port after the reply is sent. Requires `USE_SERIAL_4WAY_BLHELI_INTERFACE` for ESC passthrough.

### `MSP_RTC` (246 / 0xF6)

*   **Direction:** Out
*   **Description:** Retrieves the current Real-Time Clock time.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `seconds` | `int32_t` | 4 | Seconds | Seconds since epoch (or relative time if not set). 0 if RTC time unknown |
    | `millis` | `uint16_t` | 2 | Milliseconds | Millisecond part of the time. 0 if RTC time unknown |
*   **Notes:** Requires RTC hardware/support. Returns (0, 0) if time is not available/set.

### `MSP_SET_RTC` (247 / 0xF7)

*   **Direction:** In
*   **Description:** Sets the Real-Time Clock time.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `seconds` | `int32_t` | 4 | Seconds | Seconds component of time to set |
    | `millis` | `uint16_t` | 2 | Milliseconds | Millisecond component of time to set |
*   **Notes:** Requires RTC hardware/support. Expects 6 bytes. Uses `rtcSet()`.

### `MSP_EEPROM_WRITE` (250 / 0xFA)

*   **Direction:** In
*   **Description:** Saves the current configuration from RAM to non-volatile memory (EEPROM/Flash).
*   **Payload:** None
*   **Notes:** Will fail if armed. Suspends RX, calls `writeEEPROM()`, `readEEPROM()`, resumes RX.

### `MSP_DEBUGMSG` (253 / 0xFD)

*   **Direction:** Out
*   **Description:** Retrieves debug ("serial printf") messages from the firmware.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | Message Text | `char[]` | Variable | `NUL` terminated [debug message](https://github.com/iNavFlight/inav/blob/master/docs/development/serial_printf_debugging.md) text |

### `MSP_DEBUG` (254 / 0xFE)

*   **Direction:** Out
*   **Description:** Retrieves values from the firmware's `debug[]` array (legacy 16-bit version).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `debugValues` | `uint16_t[4]` | 8 | First 4 values from the `debug` array |
*   **Notes:** Useful for developers. See `MSP2_INAV_DEBUG` for 32-bit values.

### `MSP_V2_FRAME` (255 / 0xFF)

*   **Direction:** N/A (Indicator)
*   **Description:** This ID is used as a *payload indicator* within an MSPv1 message structure (`$M>`) to signify that the following payload conforms to the MSPv2 format. It's not a command itself.
*   **Notes:** See MSPv2 documentation for the actual frame structure that follows this indicator.

---

## MSPv1 Extended/INAV Commands (150-166)

### `MSP_STATUS_EX` (150 / 0x96)

*   **Direction:** Out
*   **Description:** Provides extended flight controller status, including CPU load, arming flags, and calibration status, in addition to `MSP_STATUS` fields.
*   **Payload:** (Starts with `MSP_STATUS` fields)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `cycleTime` | `uint16_t` | 2 | µs | Main loop cycle time |
    | `i2cErrors` | `uint16_t` | 2 | Count | I2C errors |
    | `sensorStatus` | `uint16_t` | 2 | Bitmask | Sensor status bitmask |
    | `activeModesLow` | `uint32_t` | 4 | Bitmask | First 32 active modes |
    | `profile` | `uint8_t` | 1 | Index | Current config profile index |
    | `cpuLoad` | `uint16_t` | 2 | % | Average system load percentage (`averageSystemLoadPercent`) |
    | `armingFlags` | `uint16_t` | 2 | Bitmask | Flight controller arming flags (`armingFlags`). Note: Truncated to 16 bits |
    | `accCalibAxisFlags` | `uint8_t` | 1 | Bitmask | Accelerometer calibrated axes flags (`accGetCalibrationAxisFlags()`) |
*   **Notes:** Superseded by `MSP2_INAV_STATUS` which provides the full 32-bit `armingFlags` and other enhancements.

### `MSP_SENSOR_STATUS` (151 / 0x97)

*   **Direction:** Out
*   **Description:** Provides the hardware status for each individual sensor system.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `overallHealth` | `uint8_t` | 1 | Boolean | 1 if all essential hardware is healthy, 0 otherwise (`isHardwareHealthy()`) |
    | `gyroStatus` | `uint8_t` | 1 | Enum | Enum `hardwareSensorStatus_e` Gyro hardware status (`getHwGyroStatus()`) |
    | `accStatus` | `uint8_t` | 1 | Enum | Enum `hardwareSensorStatus_e` Accelerometer hardware status (`getHwAccelerometerStatus()`) |
    | `magStatus` | `uint8_t` | 1 | Enum | Enum `hardwareSensorStatus_e` Compass hardware status (`getHwCompassStatus()`) |
    | `baroStatus` | `uint8_t` | 1 | Enum | Enum `hardwareSensorStatus_e` Barometer hardware status (`getHwBarometerStatus()`) |
    | `gpsStatus` | `uint8_t` | 1 | Enum | Enum `hardwareSensorStatus_e` GPS hardware status (`getHwGPSStatus()`) |
    | `rangefinderStatus` | `uint8_t` | 1 | Enum | Enum `hardwareSensorStatus_e` Rangefinder hardware status (`getHwRangefinderStatus()`) |
    | `pitotStatus` | `uint8_t` | 1 | Enum | Enum `hardwareSensorStatus_e` Pitot hardware status (`getHwPitotmeterStatus()`) |
    | `opflowStatus` | `uint8_t` | 1 | Enum | Enum `hardwareSensorStatus_e` Optical Flow hardware status (`getHwOpticalFlowStatus()`) |
*   **Notes:** Status values likely correspond to `SENSOR_STATUS_*` enums (e.g., OK, Unhealthy, Not Present).

### `MSP_UID` (160 / 0xA0)

*   **Direction:** Out
*   **Description:** Provides the unique identifier of the microcontroller.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `uid0` | `uint32_t` | 4 | First 32 bits of the unique ID (`U_ID_0`) |
    | `uid1` | `uint32_t` | 4 | Middle 32 bits of the unique ID (`U_ID_1`) |
    | `uid2` | `uint32_t` | 4 | Last 32 bits of the unique ID (`U_ID_2`) |
*   **Notes:** Total 12 bytes, representing a 96-bit unique ID.

### `MSP_GPSSVINFO` (164 / 0xA4)

*   **Direction:** Out
*   **Description:** Provides satellite signal strength information (legacy U-Blox compatibility stub).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `protocolVersion` | `uint8_t` | 1 | Always 1 (Stub version) |
    | `numChannels` | `uint8_t` | 1 | Always 0 (Number of SV info channels reported) |
    | `hdopHundreds` | `uint8_t` | 1 | HDOP / 100 (`gpsSol.hdop / 100`) |
    | `hdopUnits` | `uint8_t` | 1 | HDOP / 100 (`gpsSol.hdop / 100`) |
*   **Notes:** Requires `USE_GPS`. This is just a stub in INAV and does not provide actual per-satellite signal info. `hdopUnits` duplicates `hdopHundreds`.

### `MSP_GPSSTATISTICS` (166 / 0xA6)

*   **Direction:** Out
*   **Description:** Provides debugging statistics for the GPS communication link.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `lastMessageDt` | `uint16_t` | 2 | ms | Time since last valid GPS message (`gpsStats.lastMessageDt`) |
    | `errors` | `uint32_t` | 4 | Count | Number of GPS communication errors (`gpsStats.errors`) |
    | `timeouts` | `uint32_t` | 4 | Count | Number of GPS communication timeouts (`gpsStats.timeouts`) |
    | `packetCount` | `uint32_t` | 4 | Count | Number of valid GPS packets received (`gpsStats.packetCount`) |
    | `hdop` | `uint16_t` | 2 | HDOP * 100 | Horizontal Dilution of Precision (`gpsSol.hdop`) |
    | `eph` | `uint16_t` | 2 | cm | Estimated Horizontal Position Accuracy (`gpsSol.eph`) |
    | `epv` | `uint16_t` | 2 | cm | Estimated Vertical Position Accuracy (`gpsSol.epv`) |
*   **Notes:** Requires `USE_GPS`.

### `MSP_TX_INFO` (187 / 0xBB)

*   **Direction:** Out
*   **Description:** Provides information potentially useful for transmitter LUA scripts.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `rssiSource` | `uint8_t` | 1 | Enum: Source of the RSSI value (`getRSSISource()`) |
    | `rtcDateTimeIsSet` | `uint8_t` | 1 | Boolean: 1 if the RTC has been set, 0 otherwise |
*   **Notes:** See `rssiSource_e`.

### `MSP_SET_TX_INFO` (186 / 0xBA)

*   **Direction:** In
*   **Description:** Allows a transmitter LUA script (or similar) to send runtime information (currently only RSSI) to the firmware.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `rssi` | `uint8_t` | 1 | % | RSSI value (0-100) provided by the external source |
*   **Notes:** Calls `setRSSIFromMSP()`. Expects 1 byte.

---

## MSPv2 Common Commands (0x1000 Range)

These commands are part of the MSPv2 specification and are intended for general configuration and interaction.

### `MSP2_COMMON_TZ` (0x1001 / 4097)

*   **Direction:** Out
*   **Description:** Gets the time zone offset configuration.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `tzOffsetMinutes` | `int16_t` | 2 | Minutes | Time zone offset from UTC (`timeConfig()->tz_offset`) |
    | `tzAutoDst` | `uint8_t` | 1 | Boolean | Automatic daylight saving time enabled (`timeConfig()->tz_automatic_dst`) |

### `MSP2_COMMON_SET_TZ` (0x1002 / 4098)

*   **Direction:** In
*   **Description:** Sets the time zone offset configuration.
*   **Payload (Format 1 - Offset only):**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `tzOffsetMinutes` | `int16_t` | 2 | Minutes | Sets `timeConfigMutable()->tz_offset` |
*   **Payload (Format 2 - Offset + DST):**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `tzOffsetMinutes` | `int16_t` | 2 | Minutes | Sets `timeConfigMutable()->tz_offset` |
    | `tzAutoDst` | `uint8_t` | 1 | Boolean | Sets `timeConfigMutable()->tz_automatic_dst` |
*   **Notes:** Accepts 2 or 3 bytes.

### `MSP2_COMMON_SETTING` (0x1003 / 4099)

*   **Direction:** In/Out
*   **Description:** Gets the value of a specific configuration setting, identified by name or index.
*   **Request Payload (By Name):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `settingName` | `char[]` | Variable | Null-terminated string containing the setting name (e.g., "gyro_main_lpf_hz") |
*   **Request Payload (By Index):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `zeroByte` | `uint8_t` | 1 | Must be 0 |
    | `settingIndex` | `uint16_t` | 2 | Absolute index of the setting |
*   **Reply Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `settingValue` | `uint8_t[]` | Variable | Raw byte value of the setting. Size depends on the setting's type (`settingGetValueSize()`) |
*   **Notes:** Returns error if setting not found. Use `MSP2_COMMON_SETTING_INFO` to discover settings, types, and sizes.

### `MSP2_COMMON_SET_SETTING` (0x1004 / 4100)

*   **Direction:** In
*   **Description:** Sets the value of a specific configuration setting, identified by name or index.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `settingIdentifier` | Varies | Variable | Setting name (null-terminated string) OR Index (0x00 followed by `uint16_t` index) |
    | `settingValue` | `uint8_t[]` | Variable | Raw byte value to set for the setting. Size must match the setting's type |
*   **Notes:** Performs type checking and range validation (min/max). Returns error if setting not found, value size mismatch, or value out of range. Handles different data types (`uint8`, `int16`, `float`, `string`, etc.) internally.

### `MSP2_COMMON_MOTOR_MIXER` (0x1005 / 4101)

*   **Direction:** Out
*   **Description:** Retrieves the current motor mixer configuration (throttle, roll, pitch, yaw weights for each motor) for the primary and secondary mixer profiles.
*   **Payload (Profile 1):** Repeated `MAX_SUPPORTED_MOTORS` times:
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `throttleWeight` | `uint16_t` | 2 | Scaled (0-4000) | Throttle weight * 1000, offset by 2000. (Range -2.0 to +2.0 -> 0 to 4000) |
    | `rollWeight` | `uint16_t` | 2 | Scaled (0-4000) | Roll weight * 1000, offset by 2000 |
    | `pitchWeight` | `uint16_t` | 2 | Scaled (0-4000) | Pitch weight * 1000, offset by 2000 |
    | `yawWeight` | `uint16_t` | 2 | Scaled (0-4000) | Yaw weight * 1000, offset by 2000 |
    | `throttleWeight` | `uint16_t` | 2 | (Optional) Scaled (0-4000) | Profile 2 Throttle weight |
    | `rollWeight` | `uint16_t` | 2 | (Optional) Scaled (0-4000) | Profile 2 Roll weight |
    | `pitchWeight` | `uint16_t` | 2 | (Optional) Scaled (0-4000) | Profile 2 Pitch weight |
    | `yawWeight` | `uint16_t` | 2 | (Optional) Scaled (0-4000) | Profile 2 Yaw weight |
*   **Notes:** Scaling is `(float_weight + 2.0) * 1000`. `primaryMotorMixer()` provides the data.

### `MSP2_COMMON_SET_MOTOR_MIXER` (0x1006 / 4102)

*   **Direction:** In
*   **Description:** Sets the motor mixer weights for a single motor in the primary mixer profile.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `motorIndex` | `uint8_t` | 1 | Index | Index of the motor to configure (0 to `MAX_SUPPORTED_MOTORS - 1`) |
    | `throttleWeight` | `uint16_t` | 2 | Scaled (0-4000) | Sets throttle weight from `(value / 1000.0) - 2.0` |
    | `rollWeight` | `uint16_t` | 2 | Scaled (0-4000) | Sets roll weight from `(value / 1000.0) - 2.0` |
    | `pitchWeight` | `uint16_t` | 2 | Scaled (0-4000) | Sets pitch weight from `(value / 1000.0) - 2.0` |
    | `yawWeight` | `uint16_t` | 2 | Scaled (0-4000) | Sets yaw weight from `(value / 1000.0) - 2.0` |
*   **Notes:** Expects 9 bytes. Modifies `primaryMotorMixerMutable()`. Returns error if index is invalid.

### `MSP2_COMMON_SETTING_INFO` (0x1007 / 4103)

*   **Direction:** In/Out
*   **Description:** Gets detailed information about a specific configuration setting (name, type, range, flags, current value, etc.).
*   **Request Payload:** Same as `MSP2_COMMON_SETTING` request (name string or index).
*   **Reply Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `settingName` | `char[]` | Variable | Null-terminated setting name |
    | `pgn` | `uint16_t` | 2 | Parameter Group Number (PGN) ID |
    | `type` | `uint8_t` | 1 | Variable type (`VAR_UINT8`, `VAR_FLOAT`, etc.) |
    | `section` | `uint8_t` | 1 | Setting section (`MASTER_VALUE`, `PROFILE_VALUE`, etc.) |
    | `mode` | `uint8_t` | 1 | Setting mode (`MODE_NORMAL`, `MODE_LOOKUP`, etc.) |
    | `minValue` | `int32_t` | 4 | Minimum allowed value (as signed 32-bit) |
    | `maxValue` | `uint32_t` | 4 | Maximum allowed value (as unsigned 32-bit) |
    | `settingIndex` | `uint16_t` | 2 | Absolute index of the setting |
    | `profileIndex` | `uint8_t` | 1 | Current profile index (if applicable, else 0) |
    | `profileCount` | `uint8_t` | 1 | Total number of profiles (if applicable, else 0) |
    | `lookupNames` | `char[]` | Variable | (If `mode == MODE_LOOKUP`) Series of null-terminated strings for each possible value from min to max |
    | `settingValue` | `uint8_t[]` | Variable | Current raw byte value of the setting |
*   **Notes:**

    * Very useful for configurators to dynamically build interfaces. Returns error if setting not found.
	* This is a variable length message, depending on the name length, `mode`, and `type`.

### `MSP2_COMMON_PG_LIST` (0x1008 / 4104)

*   **Direction:** In/Out
*   **Description:** Gets a list of Parameter Group Numbers (PGNs) used by settings, along with the start and end setting indexes for each group. Can request info for a single PGN.
*   **Request Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `pgn` | `uint16_t` | 2 | (Optional) PGN ID to query. If omitted, returns all used PGNs |
*   **Reply Payload:** Repeated for each PGN found:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `pgn` | `uint16_t` | 2 | Parameter Group Number (PGN) ID |
    | `startIndex` | `uint16_t` | 2 | Absolute index of the first setting in this group |
    | `endIndex` | `uint16_t` | 2 | Absolute index of the last setting in this group |
*   **Notes:** Allows efficient fetching of related settings by group.

### `MSP2_COMMON_SERIAL_CONFIG` (0x1009 / 4105)

*   **Direction:** Out
*   **Description:** Retrieves the configuration for all available serial ports.
*   **Payload:** Repeated for each available serial port:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `identifier` | `uint8_t` | 1 | Port identifier Enum (`serialPortIdentifier_e`) |
    | `functionMask` | `uint32_t` | 4 | Bitmask of enabled functions (`FUNCTION_*`) |
    | `mspBaudIndex` | `uint8_t` | 1 | Baud rate index for MSP function |
    | `gpsBaudIndex` | `uint8_t` | 1 | Baud rate index for GPS function |
    | `telemetryBaudIndex` | `uint8_t` | 1 | Baud rate index for Telemetry function |
    | `peripheralBaudIndex` | `uint8_t` | 1 | Baud rate index for other peripheral functions |
*   **Notes:** Baud rate indexes map to actual baud rates (e.g., 9600, 115200). See `baudRates` array.

### `MSP2_COMMON_SET_SERIAL_CONFIG` (0x100A / 4106)

*   **Direction:** In
*   **Description:** Sets the configuration for one or more serial ports.
*   **Payload:** Repeated for each port being configured:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `identifier` | `uint8_t` | 1 | Port identifier Enum (`serialPortIdentifier_e`) |
    | `functionMask` | `uint32_t` | 4 | Bitmask of functions to enable |
    | `mspBaudIndex` | `uint8_t` | 1 | Baud rate index for MSP |
    | `gpsBaudIndex` | `uint8_t` | 1 | Baud rate index for GPS |
    | `telemetryBaudIndex` | `uint8_t` | 1 | Baud rate index for Telemetry |
    | `peripheralBaudIndex` | `uint8_t` | 1 | Baud rate index for peripherals |
*   **Notes:** Payload size must be a multiple of the size of one port config entry (1 + 4 + 4 = 9 bytes). Returns error if identifier is invalid or size is incorrect. Baud rate indexes are constrained `BAUD_MIN` to `BAUD_MAX`.

### `MSP2_COMMON_SET_RADAR_POS` (0x100B / 4107)

*   **Direction:** In
*   **Description:** Sets the position and status information for a "radar" Point of Interest (POI). Used for displaying other craft/objects on the OSD map.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `poiIndex` | `uint8_t` | 1 | Index | Index of the POI slot (0 to `RADAR_MAX_POIS - 1`) |
    | `state` | `uint8_t` | 1 | Enum | Status of the POI (0=undefined, 1=armed, 2=lost) |
    | `latitude` | `int32_t` | 4 | deg * 1e7 | Latitude of the POI |
    | `longitude` | `int32_t` | 4 | deg * 1e7 | Longitude of the POI |
    | `altitude` | `int32_t` | 4 | cm | Altitude of the POI |
    | `heading` | `int16_t` | 2 | degrees | Heading of the POI |
    | `speed` | `uint16_t` | 2 | cm/s | Speed of the POI |
    | `linkQuality` | `uint8_t` | 1 | 0-4 | Link quality indicator |
*   **Notes:** Expects 19 bytes. Updates the `radar_pois` array.

### `MSP2_COMMON_SET_RADAR_ITD` (0x100C / 4108)

*   **Direction:** In
*   **Description:** Sets radar information to display (likely internal/unused).
*   **Notes:** Not implemented in INAV `fc_msp.c`.

### `MSP2_COMMON_SET_MSP_RC_LINK_STATS` (0x100D / 4109)

*   **Direction:** In
*   **Description:** Provides RC link statistics (RSSI, LQ) to the FC, typically from an MSP-based RC link (like ExpressLRS). Sent periodically by the RC link.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `sublinkID` | `uint8_t` | 1 | - | Sublink identifier (usually 0) |
    | `validLink` | `uint8_t` | 1 | Boolean | Indicates if the link is currently valid (not in failsafe) |
    | `rssiPercent` | `uint8_t` | 1 | % | Uplink RSSI percentage (0-100) |
    | `uplinkRSSI_dBm` | `uint8_t` | 1 | -dBm | Uplink RSSI in dBm (sent as positive, e.g., 70 means -70dBm) |
    | `downlinkLQ` | `uint8_t` | 1 | % | Downlink Link Quality (0-100) |
    | `uplinkLQ` | `uint8_t` | 1 | % | Uplink Link Quality (0-100) |
    | `uplinkSNR` | `int8_t` | 1 | dB | Uplink Signal-to-Noise Ratio |
*   **Notes:** Requires `USE_RX_MSP`. Expects at least 7 bytes. Updates `rxLinkStatistics` and sets RSSI via `setRSSIFromMSP_RC()` only if `sublinkID` is 0. This message expects **no reply** (`MSP_RESULT_NO_REPLY`).

### `MSP2_COMMON_SET_MSP_RC_INFO` (0x100E / 4110)

*   **Direction:** In
*   **Description:** Provides additional RC link information (power levels, band, mode) to the FC from an MSP-based RC link. Sent less frequently than link stats.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `sublinkID` | `uint8_t` | 1 | - | Sublink identifier (usually 0) |
    | `uplinkTxPower` | `uint16_t` | 2 | mW? | Uplink transmitter power level |
    | `downlinkTxPower` | `uint16_t` | 2 | mW? | Downlink transmitter power level |
    | `band` | `char[4]` | 4 | - | Operating band string (e.g., "2G4", "900") |
    | `mode` | `char[6]` | 6 | - | Operating mode/rate string (e.g., "100HZ", "F1000") |
*   **Notes:** Requires `USE_RX_MSP`. Expects at least 15 bytes. Updates `rxLinkStatistics` only if `sublinkID` is 0. Converts band/mode strings to uppercase. This message expects **no reply** (`MSP_RESULT_NO_REPLY`).

---

## MSPv2 INAV Specific Commands (0x2000 Range)

These commands are specific extensions added by the INAV project.

### `MSP2_INAV_STATUS` (0x2000 / 8192)

*   **Direction:** Out
*   **Description:** Provides comprehensive flight controller status, extending `MSP_STATUS_EX` with full arming flags, battery profile, and mixer profile.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `cycleTime` | `uint16_t` | 2 | µs | Main loop cycle time |
    | `i2cErrors` | `uint16_t` | 2 | Count | I2C errors |
    | `sensorStatus` | `uint16_t` | 2 | Bitmask | Sensor status bitmask |
    | `cpuLoad` | `uint16_t` | 2 | % | Average system load percentage |
    | `profileAndBattProfile` | `uint8_t` | 1 | Packed | Bits 0-3: Config profile index (`getConfigProfile()`), Bits 4-7: Battery profile index (`getConfigBatteryProfile()`) |
    | `armingFlags` | `uint32_t` | 4 | Bitmask | Full 32-bit flight controller arming flags (`armingFlags`) |
    | `activeModes` | `boxBitmask_t` | `sizeof(boxBitmask_t)` | Bitmask | Full bitmask of active flight modes (`packBoxModeFlags()`) |
    | `mixerProfile` | `uint8_t` | 1 | Index | Current mixer profile index (`getConfigMixerProfile()`) |

### `MSP2_INAV_OPTICAL_FLOW` (0x2001 / 8193)

*   **Direction:** Out
*   **Description:** Provides data from the optical flow sensor.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `quality` | `uint8_t` | 1 | 0-255 | Raw quality indicator from the sensor (`opflow.rawQuality`). 0 if `USE_OPFLOW` disabled |
    | `flowRateX` | `int16_t` | 2 | degrees/s | Optical flow rate X (roll axis) (`RADIANS_TO_DEGREES(opflow.flowRate[X])`). 0 if `USE_OPFLOW` disabled |
    | `flowRateY` | `int16_t` | 2 | degrees/s | Optical flow rate Y (pitch axis) (`RADIANS_TO_DEGREES(opflow.flowRate[Y])`). 0 if `USE_OPFLOW` disabled |
    | `bodyRateX` | `int16_t` | 2 | degrees/s | Compensated body rate X (roll axis) (`RADIANS_TO_DEGREES(opflow.bodyRate[X])`). 0 if `USE_OPFLOW` disabled |
    | `bodyRateY` | `int16_t` | 2 | degrees/s | Compensated body rate Y (pitch axis) (`RADIANS_TO_DEGREES(opflow.bodyRate[Y])`). 0 if `USE_OPFLOW` disabled |
*   **Notes:** Requires `USE_OPFLOW`.

### `MSP2_INAV_ANALOG` (0x2002 / 8194)

*   **Direction:** Out
*   **Description:** Provides detailed analog sensor readings, superseding `MSP_ANALOG` with higher precision and additional fields.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `batteryFlags` | `uint8_t` | 1 | Bitmask | Battery status flags: Bit 0=Full on plug-in, Bit 1=Use capacity threshold, Bit 2-3=Battery State enum (`getBatteryState()`), Bit 4-7=Cell Count (`getBatteryCellCount()`) |
    | `vbat` | `uint16_t` | 2 | 0.01V | Battery voltage (`getBatteryVoltage()`) |
    | `amperage` | `uint16_t` | 2 | 0.01A | Current draw (`getAmperage()`) |
    | `powerDraw` | `uint32_t` | 4 | mW | Power draw (`getPower()`) |
    | `mAhDrawn` | `uint32_t` | 4 | mAh | Consumed capacity (`getMAhDrawn()`) |
    | `mWhDrawn` | `uint32_t` | 4 | mWh | Consumed energy (`getMWhDrawn()`) |
    | `remainingCapacity` | `uint32_t` | 4 | mAh/mWh | Estimated remaining capacity (`getBatteryRemainingCapacity()`) |
    | `percentageRemaining` | `uint8_t` | 1 | % | Estimated remaining capacity percentage (`calculateBatteryPercentage()`) |
    | `rssi` | `uint16_t` | 2 | 0-1023 or % | RSSI value (`getRSSI()`) |

### `MSP2_INAV_MISC` (0x2003 / 8195)

*   **Direction:** Out
*   **Description:** Retrieves miscellaneous configuration settings, superseding `MSP_MISC` with higher precision and capacity fields.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `midRc` | `uint16_t` | 2 | PWM | Mid RC value (`PWM_RANGE_MIDDLE`) |
    | `legacyMinThrottle` | `uint16_t` | 2 | - | Always 0 (Legacy) |
    | `maxThrottle` | `uint16_t` | 2 | PWM | Maximum throttle command (`getMaxThrottle()`) |
    | `minCommand` | `uint16_t` | 2 | PWM | Minimum motor command (`motorConfig()->mincommand`) |
    | `failsafeThrottle` | `uint16_t` | 2 | PWM | Failsafe throttle level (`currentBatteryProfile->failsafe_throttle`) |
    | `gpsType` | `uint8_t` | 1 | Enum | Enum `gpsProvider_e` GPS provider type (`gpsConfig()->provider`). 0 if `USE_GPS` disabled |
    | `legacyGpsBaud` | `uint8_t` | 1 | - | Always 0 (Legacy) |
    | `gpsSbasMode` | `uint8_t` | 1 | Enum | Enum `sbasMode_e` GPS SBAS mode (`gpsConfig()->sbasMode`). 0 if `USE_GPS` disabled |
    | `rssiChannel` | `uint8_t` | 1 | Index | RSSI channel index (1-based) (`rxConfig()->rssi_channel`) |
    | `magDeclination` | `uint16_t` | 2 | 0.1 degrees | Magnetic declination / 10 (`compassConfig()->mag_declination / 10`). 0 if `USE_MAG` disabled |
    | `vbatScale` | `uint16_t` | 2 | Scale | Voltage scale (`batteryMetersConfig()->voltage.scale`). 0 if `USE_ADC` disabled |
    | `vbatSource` | `uint8_t` | 1 | Enum | Enum `batVoltageSource_e` Voltage source (`batteryMetersConfig()->voltageSource`). 0 if `USE_ADC` disabled |
    | `cellCount` | `uint8_t` | 1 | Count | Configured cell count (`currentBatteryProfile->cells`). 0 if `USE_ADC` disabled |
    | `vbatCellDetect` | `uint16_t` | 2 | 0.01V | Cell detection voltage (`currentBatteryProfile->voltage.cellDetect`). 0 if `USE_ADC` disabled |
    | `vbatMinCell` | `uint16_t` | 2 | 0.01V | Min cell voltage (`currentBatteryProfile->voltage.cellMin`). 0 if `USE_ADC` disabled |
    | `vbatMaxCell` | `uint16_t` | 2 | 0.01V | Max cell voltage (`currentBatteryProfile->voltage.cellMax`). 0 if `USE_ADC` disabled |
    | `vbatWarningCell` | `uint16_t` | 2 | 0.01V | Warning cell voltage (`currentBatteryProfile->voltage.cellWarning`). 0 if `USE_ADC` disabled |
    | `capacityValue` | `uint32_t` | 4 | mAh/mWh | Battery capacity (`currentBatteryProfile->capacity.value`) |
    | `capacityWarning` | `uint32_t` | 4 | mAh/mWh | Capacity warning threshold (`currentBatteryProfile->capacity.warning`) |
    | `capacityCritical` | `uint32_t` | 4 | mAh/mWh | Capacity critical threshold (`currentBatteryProfile->capacity.critical`) |
    | `capacityUnit` | `uint8_t` | 1 | Enum | Enum `batCapacityUnit_e` Capacity unit ('batteryMetersConfig()->capacity_unit') |

### `MSP2_INAV_SET_MISC` (0x2004 / 8196)

*   **Direction:** In
*   **Description:** Sets miscellaneous configuration settings, superseding `MSP_SET_MISC`.
*   **Payload:** (Matches `MSP2_INAV_MISC` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `midRc` | `uint16_t` | 2 | PWM | Ignored |
    | `legacyMinThrottle` | `uint16_t` | 2 | - | Ignored |
    | `legacyMaxThrottle` | `uint16_t` | 2 | - | Ignored |
    | `minCommand` | `uint16_t` | 2 | PWM | Sets `motorConfigMutable()->mincommand` (constrained) |
    | `failsafeThrottle` | `uint16_t` | 2 | PWM | Sets `currentBatteryProfileMutable->failsafe_throttle` (constrained) |
    | `gpsType` | `uint8_t` | 1 | Enum | Enum `gpsProvider_e` Sets `gpsConfigMutable()->provider` (if `USE_GPS`) |
    | `legacyGpsBaud` | `uint8_t` | 1 | - | Ignored |
    | `gpsSbasMode` | `uint8_t` | 1 | Enum | Enum `sbasMode_e` Sets `gpsConfigMutable()->sbasMode` (if `USE_GPS`) |
    | `rssiChannel` | `uint8_t` | 1 | Index | Sets `rxConfigMutable()->rssi_channel` (constrained). Updates source |
    | `magDeclination` | `uint16_t` | 2 | 0.1 degrees | Sets `compassConfigMutable()->mag_declination = value * 10` (if `USE_MAG`) |
    | `vbatScale` | `uint16_t` | 2 | Scale | Sets `batteryMetersConfigMutable()->voltage.scale` (if `USE_ADC`) |
    | `vbatSource` | `uint8_t` | 1 | Enum | Enum `batVoltageSource_e` Sets `batteryMetersConfigMutable()->voltageSource` (if `USE_ADC`, validated) |
    | `cellCount` | `uint8_t` | 1 | Count | Sets `currentBatteryProfileMutable->cells` (if `USE_ADC`) |
    | `vbatCellDetect` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellDetect` (if `USE_ADC`) |
    | `vbatMinCell` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellMin` (if `USE_ADC`) |
    | `vbatMaxCell` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellMax` (if `USE_ADC`) |
    | `vbatWarningCell` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellWarning` (if `USE_ADC`) |
    | `capacityValue` | `uint32_t` | 4 | mAh/mWh | Sets `currentBatteryProfileMutable->capacity.value` |
    | `capacityWarning` | `uint32_t` | 4 | mAh/mWh | Sets `currentBatteryProfileMutable->capacity.warning` |
    | `capacityCritical` | `uint32_t` | 4 | mAh/mWh | Sets `currentBatteryProfileMutable->capacity.critical` |
    | `capacityUnit` | `uint8_t` | 1 | Enum | Enum `batCapacityUnit_e` sets Capacity unit ('batteryMetersConfig()->capacity_unit'). Updates OSD energy unit if changed |
*   **Notes:** Expects 41 bytes. Performs validation on `vbatSource` and `capacityUnit`.

### `MSP2_INAV_BATTERY_CONFIG` (0x2005 / 8197)

*   **Direction:** Out
*   **Description:** Retrieves the configuration specific to the battery voltage and current sensors and capacity settings for the current battery profile.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `vbatScale` | `uint16_t` | 2 | Scale | Voltage scale (`batteryMetersConfig()->voltage.scale`) |
    | `vbatSource` | `uint8_t` | 1 | Enum | Enum `batVoltageSource_e` Voltage source (`batteryMetersConfig()->voltageSource`) |
    | `cellCount` | `uint8_t` | 1 | Count | Configured cell count (`currentBatteryProfile->cells`) |
    | `vbatCellDetect` | `uint16_t` | 2 | 0.01V | Cell detection voltage (`currentBatteryProfile->voltage.cellDetect`) |
    | `vbatMinCell` | `uint16_t` | 2 | 0.01V | Min cell voltage (`currentBatteryProfile->voltage.cellMin`) |
    | `vbatMaxCell` | `uint16_t` | 2 | 0.01V | Max cell voltage (`currentBatteryProfile->voltage.cellMax`) |
    | `vbatWarningCell` | `uint16_t` | 2 | 0.01V | Warning cell voltage (`currentBatteryProfile->voltage.cellWarning`) |
    | `currentOffset` | `uint16_t` | 2 | mV | Current sensor offset (`batteryMetersConfig()->current.offset`) |
    | `currentScale` | `uint16_t` | 2 | Scale | Current sensor scale (`batteryMetersConfig()->current.scale`) |
    | `capacityValue` | `uint32_t` | 4 | mAh/mWh | Battery capacity (`currentBatteryProfile->capacity.value`) |
    | `capacityWarning` | `uint32_t` | 4 | mAh/mWh | Capacity warning threshold (`currentBatteryProfile->capacity.warning`) |
    | `capacityCritical` | `uint32_t` | 4 | mAh/mWh | Capacity critical threshold (`currentBatteryProfile->capacity.critical`) |
    | `capacityUnit` | `uint8_t` | 1 | Enum | Enum `batCapacityUnit_e` Capacity unit ('batteryMetersConfig()->capacity_unit') |
*   **Notes:** Fields are 0 if `USE_ADC` is not defined.

### `MSP2_INAV_SET_BATTERY_CONFIG` (0x2006 / 8198)

*   **Direction:** In
*   **Description:** Sets the battery voltage/current sensor configuration and capacity settings for the current battery profile.
*   **Payload:** (Matches `MSP2_INAV_BATTERY_CONFIG` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `vbatScale` | `uint16_t` | 2 | Scale | Sets `batteryMetersConfigMutable()->voltage.scale` (if `USE_ADC`) |
    | `vbatSource` | `uint8_t` | 1 | Enum | Enum `batVoltageSource_e` Sets `batteryMetersConfigMutable()->voltageSource` (if `USE_ADC`, validated) |
    | `cellCount` | `uint8_t` | 1 | Count | Sets `currentBatteryProfileMutable->cells` (if `USE_ADC`) |
    | `vbatCellDetect` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellDetect` (if `USE_ADC`) |
    | `vbatMinCell` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellMin` (if `USE_ADC`) |
    | `vbatMaxCell` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellMax` (if `USE_ADC`) |
    | `vbatWarningCell` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellWarning` (if `USE_ADC`) |
    | `currentOffset` | `uint16_t` | 2 | mV | Sets `batteryMetersConfigMutable()->current.offset` |
    | `currentScale` | `uint16_t` | 2 | Scale | Sets `batteryMetersConfigMutable()->current.scale` |
    | `capacityValue` | `uint32_t` | 4 | mAh/mWh | Sets `currentBatteryProfileMutable->capacity.value` |
    | `capacityWarning` | `uint32_t` | 4 | mAh/mWh | Sets `currentBatteryProfileMutable->capacity.warning` |
    | `capacityCritical` | `uint32_t` | 4 | mAh/mWh | Sets `currentBatteryProfileMutable->capacity.critical` |
    | `capacityUnit` | `uint8_t` | 1 | Enum | Enum `batCapacityUnit_e` sets Capacity unit ('batteryMetersConfig()->capacity_unit') Updates OSD energy unit if changed |
*   **Notes:** Expects 29 bytes. Performs validation on `vbatSource` and `capacityUnit`.

### `MSP2_INAV_RATE_PROFILE` (0x2007 / 8199)

*   **Direction:** Out
*   **Description:** Retrieves the rates and expos for the current control rate profile, including both stabilized and manual flight modes. Supersedes `MSP_RC_TUNING`.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `throttleMid` | `uint8_t` | 1 | Throttle Midpoint (`currentControlRateProfile->throttle.rcMid8`) |
    | `throttleExpo` | `uint8_t` | 1 | Throttle Expo (`currentControlRateProfile->throttle.rcExpo8`) |
    | `dynamicThrottlePID` | `uint8_t` | 1 | TPA value (`currentControlRateProfile->throttle.dynPID`) |
    | `tpaBreakpoint` | `uint16_t` | 2 | TPA breakpoint (`currentControlRateProfile->throttle.pa_breakpoint`) |
    | `stabRcExpo` | `uint8_t` | 1 | Stabilized Roll/Pitch Expo (`currentControlRateProfile->stabilized.rcExpo8`) |
    | `stabRcYawExpo` | `uint8_t` | 1 | Stabilized Yaw Expo (`currentControlRateProfile->stabilized.rcYawExpo8`) |
    | `stabRollRate` | `uint8_t` | 1 | Stabilized Roll Rate (`currentControlRateProfile->stabilized.rates[FD_ROLL]`) |
    | `stabPitchRate` | `uint8_t` | 1 | Stabilized Pitch Rate (`currentControlRateProfile->stabilized.rates[FD_PITCH]`) |
    | `stabYawRate` | `uint8_t` | 1 | Stabilized Yaw Rate (`currentControlRateProfile->stabilized.rates[FD_YAW]`) |
    | `manualRcExpo` | `uint8_t` | 1 | Manual Roll/Pitch Expo (`currentControlRateProfile->manual.rcExpo8`) |
    | `manualRcYawExpo` | `uint8_t` | 1 | Manual Yaw Expo (`currentControlRateProfile->manual.rcYawExpo8`) |
    | `manualRollRate` | `uint8_t` | 1 | Manual Roll Rate (`currentControlRateProfile->manual.rates[FD_ROLL]`) |
    | `manualPitchRate` | `uint8_t` | 1 | Manual Pitch Rate (`currentControlRateProfile->manual.rates[FD_PITCH]`) |
    | `manualYawRate` | `uint8_t` | 1 | Manual Yaw Rate (`currentControlRateProfile->manual.rates[FD_YAW]`) |

### `MSP2_INAV_SET_RATE_PROFILE` (0x2008 / 8200)

*   **Direction:** In
*   **Description:** Sets the rates and expos for the current control rate profile (stabilized and manual). Supersedes `MSP_SET_RC_TUNING`.
*   **Payload:** (Matches `MSP2_INAV_RATE_PROFILE` structure)
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `throttleMid` | `uint8_t` | 1 | Sets `currentControlRateProfile_p->throttle.rcMid8` |
    | `throttleExpo` | `uint8_t` | 1 | Sets `currentControlRateProfile_p->throttle.rcExpo8` |
    | `dynamicThrottlePID` | `uint8_t` | 1 | Sets `currentControlRateProfile_p->throttle.dynPID` |
    | `tpaBreakpoint` | `uint16_t` | 2 | Sets `currentControlRateProfile_p->throttle.pa_breakpoint` |
    | `stabRcExpo` | `uint8_t` | 1 | Sets `currentControlRateProfile_p->stabilized.rcExpo8` |
    | `stabRcYawExpo` | `uint8_t` | 1 | Sets `currentControlRateProfile_p->stabilized.rcYawExpo8` |
    | `stabRollRate` | `uint8_t` | 1 | Sets `currentControlRateProfile_p->stabilized.rates[FD_ROLL]` (constrained) |
    | `stabPitchRate` | `uint8_t` | 1 | Sets `currentControlRateProfile_p->stabilized.rates[FD_PITCH]` (constrained) |
    | `stabYawRate` | `uint8_t` | 1 | Sets `currentControlRateProfile_p->stabilized.rates[FD_YAW]` (constrained) |
    | `manualRcExpo` | `uint8_t` | 1 | Sets `currentControlRateProfile_p->manual.rcExpo8` |
    | `manualRcYawExpo` | `uint8_t` | 1 | Sets `currentControlRateProfile_p->manual.rcYawExpo8` |
    | `manualRollRate` | `uint8_t` | 1 | Sets `currentControlRateProfile_p->manual.rates[FD_ROLL]` (constrained) |
    | `manualPitchRate` | `uint8_t` | 1 | Sets `currentControlRateProfile_p->manual.rates[FD_PITCH]` (constrained) |
    | `manualYawRate` | `uint8_t` | 1 | Sets `currentControlRateProfile_p->manual.rates[FD_YAW]` (constrained) |
*   **Notes:** Expects 15 bytes. Constraints applied to rates based on axis.

### `MSP2_INAV_AIR_SPEED` (0x2009 / 8201)

*   **Direction:** Out
*   **Description:** Retrieves the estimated or measured airspeed.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `airspeed` | `uint32_t` | 4 | cm/s | Estimated/measured airspeed (`getAirspeedEstimate()`). 0 if `USE_PITOT` disabled or no valid data |
*   **Notes:** Requires `USE_PITOT` for measured airspeed. May return GPS ground speed if pitot unavailable but GPS is present and configured.

### `MSP2_INAV_OUTPUT_MAPPING` (0x200A / 8202)

*   **Direction:** Out
*   **Description:** Retrieves the output mapping configuration (identifies which timer outputs are used for Motors/Servos). Legacy version sending only 8-bit usage flags.
*   **Payload:** Repeated for each Motor/Servo timer:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `usageFlags` | `uint8_t` | 1 | Timer usage flags (truncated). `TIM_USE_MOTOR` or `TIM_USE_SERVO` |
*   **Notes:** Superseded by `MSP2_INAV_OUTPUT_MAPPING_EXT2`. Only includes timers *not* used for PPM/PWM input.

### `MSP2_INAV_MC_BRAKING` (0x200B / 8203)

*   **Direction:** Out
*   **Description:** Retrieves configuration parameters for the multirotor braking mode feature.
*   **Payload:** (Only if `USE_MR_BRAKING_MODE` defined)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `brakingSpeedThreshold` | `uint16_t` | 2 | cm/s | Speed above which braking engages (`navConfig()->mc.braking_speed_threshold`) |
    | `brakingDisengageSpeed` | `uint16_t` | 2 | cm/s | Speed below which braking disengages (`navConfig()->mc.braking_disengage_speed`) |
    | `brakingTimeout` | `uint16_t` | 2 | ms | Timeout before braking force reduces (`navConfig()->mc.braking_timeout`) |
    | `brakingBoostFactor` | `uint8_t` | 1 | % | Boost factor applied during braking (`navConfig()->mc.braking_boost_factor`) |
    | `brakingBoostTimeout` | `uint16_t` | 2 | ms | Timeout for the boost factor (`navConfig()->mc.braking_boost_timeout`) |
    | `brakingBoostSpeedThreshold`| `uint16_t` | 2 | cm/s | Speed threshold for boost engagement (`navConfig()->mc.braking_boost_speed_threshold`) |
    | `brakingBoostDisengageSpeed`| `uint16_t` | 2 | cm/s | Speed threshold for boost disengagement (`navConfig()->mc.braking_boost_disengage_speed`) |
    | `brakingBankAngle` | `uint8_t` | 1 | degrees | Maximum bank angle allowed during braking (`navConfig()->mc.braking_bank_angle`) |
*   **Notes:** Payload is empty if `USE_MR_BRAKING_MODE` is not defined.

### `MSP2_INAV_SET_MC_BRAKING` (0x200C / 8204)

*   **Direction:** In
*   **Description:** Sets configuration parameters for the multirotor braking mode feature.
*   **Payload:** (Matches `MSP2_INAV_MC_BRAKING` structure, requires `USE_MR_BRAKING_MODE`)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `brakingSpeedThreshold` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->mc.braking_speed_threshold` |
    | `brakingDisengageSpeed` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->mc.braking_disengage_speed` |
    | `brakingTimeout` | `uint16_t` | 2 | ms | Sets `navConfigMutable()->mc.braking_timeout` |
    | `brakingBoostFactor` | `uint8_t` | 1 | % | Sets `navConfigMutable()->mc.braking_boost_factor` |
    | `brakingBoostTimeout` | `uint16_t` | 2 | ms | Sets `navConfigMutable()->mc.braking_boost_timeout` |
    | `brakingBoostSpeedThreshold`| `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->mc.braking_boost_speed_threshold` |
    | `brakingBoostDisengageSpeed`| `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->mc.braking_boost_disengage_speed` |
    | `brakingBankAngle` | `uint8_t` | 1 | degrees | Sets `navConfigMutable()->mc.braking_bank_angle` |
*   **Notes:** Expects 14 bytes. Returns error if `USE_MR_BRAKING_MODE` is not defined.

### `MSP2_INAV_OUTPUT_MAPPING_EXT` (0x200D / 8205)

*   **Direction:** Out
*   **Description:** Retrieves extended output mapping configuration (timer ID and usage flags). Obsolete, use `MSP2_INAV_OUTPUT_MAPPING_EXT2`.
*   **Payload:** Repeated for each Motor/Servo timer:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `timerId` | `uint8_t` | 1 | Hardware timer identifier (e.g., `TIM1`, `TIM2`). Value depends on target |
    | `usageFlags` | `uint8_t` | 1 | Timer usage flags (truncated). `TIM_USE_MOTOR` or `TIM_USE_SERVO` |
*   **Notes:** Usage flags are truncated to 8 bits. `timerId` mapping is target-specific.

### `MSP2_INAV_TIMER_OUTPUT_MODE` (0x200E / 8206)

*   **Direction:** In/Out
*   **Description:** Get or list the output mode override for hardware timers (e.g., force ONESHOT, DSHOT).
*   **Request Payload (Get All):** None
*   **Request Payload (Get One):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `timerIndex` | `uint8_t` | 1 | Index of the hardware timer definition (0 to `HARDWARE_TIMER_DEFINITION_COUNT - 1`) |
*   **Reply Payload (List All):** Repeated `HARDWARE_TIMER_DEFINITION_COUNT` times:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `timerIndex` | `uint8_t` | 1 | Timer index |
    | `outputMode` | `uint8_t` | 1 | Output mode override (`TIMER_OUTPUT_MODE_*` enum) |
*   **Reply Payload (Get One):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `timerIndex` | `uint8_t` | 1 | Timer index requested |
    | `outputMode` | `uint8_t` | 1 | Output mode override for the requested timer |
*   **Notes:** Only available on non-SITL builds. `HARDWARE_TIMER_DEFINITION_COUNT` varies by target.

### `MSP2_INAV_SET_TIMER_OUTPUT_MODE` (0x200F / 8207)

*   **Direction:** In
*   **Description:** Set the output mode override for a specific hardware timer.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `timerIndex` | `uint8_t` | 1 | Index of the hardware timer definition |
    | `outputMode` | `uint8_t` | 1 | Output mode override (`TIMER_OUTPUT_MODE_*` enum) to set |
*   **Notes:** Only available on non-SITL builds. Expects 2 bytes. Returns error if `timerIndex` is invalid.

### `MSP2_INAV_OUTPUT_MAPPING_EXT2` (0x210D / 8461)

*   **Direction:** Out
*   **Description:** Retrieves the full extended output mapping configuration (timer ID, full 32-bit usage flags, and pin label). Supersedes `MSP2_INAV_OUTPUT_MAPPING_EXT`.
*   **Payload:** Repeated for each Motor/Servo timer:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `timerId` | `uint8_t` | 1 | Hardware timer identifier (e.g., `TIM1`, `TIM2`). SITL uses index |
    | `usageFlags` | `uint32_t` | 4 | Full 32-bit timer usage flags (`TIM_USE_*`) |
    | `pinLabel` | `uint8_t` | 1 | Label for special pin usage (`PIN_LABEL_*` enum, e.g., `PIN_LABEL_LED`). 0 (`PIN_LABEL_NONE`) otherwise |
*   **Notes:** Provides complete usage flags and helps identify pins repurposed for functions like LED strip.

### `MSP2_INAV_MIXER` (0x2010 / 8208)

*   **Direction:** Out
*   **Description:** Retrieves INAV-specific mixer configuration details.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `motorDirectionInverted` | `uint8_t` | 1 | Boolean: 1 if motor direction is reversed globally (`mixerConfig()->motorDirectionInverted`) |
    | `reserved1` | `uint8_t` | 1 | Always 0 (Was yaw jump prevention limit) |
    | `motorStopOnLow` | `uint8_t` | 1 | Boolean: 1 if motors stop at minimum throttle (`mixerConfig()->motorstopOnLow`) |
    | `platformType` | `uint8_t` | 1 | Enum (`platformType_e`): Vehicle platform type (Multirotor, Airplane, etc.) (`mixerConfig()->platformType`) |
    | `hasFlaps` | `uint8_t` | 1 | Boolean: 1 if the current mixer configuration includes flaps (`mixerConfig()->hasFlaps`) |
    | `appliedMixerPreset` | `uint16_t` | 2 | Enum (`mixerPreset_e`): Mixer preset currently applied (`mixerConfig()->appliedMixerPreset`) |
    | `maxMotors` | `uint8_t` | 1 | Constant: Maximum motors supported (`MAX_SUPPORTED_MOTORS`) |
    | `maxServos` | `uint8_t` | 1 | Constant: Maximum servos supported (`MAX_SUPPORTED_SERVOS`) |

### `MSP2_INAV_SET_MIXER` (0x2011 / 8209)

*   **Direction:** In
*   **Description:** Sets INAV-specific mixer configuration details.
*   **Payload:** (Matches `MSP2_INAV_MIXER` structure)
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `motorDirectionInverted` | `uint8_t` | 1 | Sets `mixerConfigMutable()->motorDirectionInverted` |
    | `reserved1` | `uint8_t` | 1 | Ignored |
    | `motorStopOnLow` | `uint8_t` | 1 | Sets `mixerConfigMutable()->motorstopOnLow` |
    | `platformType` | `uint8_t` | 1 | Sets `mixerConfigMutable()->platformType` |
    | `hasFlaps` | `uint8_t` | 1 | Sets `mixerConfigMutable()->hasFlaps` |
    | `appliedMixerPreset` | `uint16_t` | 2 | Sets `mixerConfigMutable()->appliedMixerPreset` |
    | `maxMotors` | `uint8_t` | 1 | Ignored |
    | `maxServos` | `uint8_t` | 1 | Ignored |
*   **Notes:** Expects 9 bytes. Calls `mixerUpdateStateFlags()`.

### `MSP2_INAV_OSD_LAYOUTS` (0x2012 / 8210)

*   **Direction:** In/Out
*   **Description:** Gets OSD layout information (counts, positions for a specific layout, or position for a specific item).
*   **Request Payload (Get Counts):** None
*   **Request Payload (Get Layout):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `layoutIndex` | `uint8_t` | 1 | Index of the OSD layout (0 to `OSD_LAYOUT_COUNT - 1`) |
*   **Request Payload (Get Item):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `layoutIndex` | `uint8_t` | 1 | Index of the OSD layout |
    | `itemIndex` | `uint16_t` | 2 | Index of the OSD item (`OSD_ITEM_*` enum, 0 to `OSD_ITEM_COUNT - 1`) |
*   **Reply Payload (Get Counts):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `layoutCount` | `uint8_t` | 1 | Number of OSD layouts (`OSD_LAYOUT_COUNT`) |
    | `itemCount` | `uint8_t` | 1 | Number of OSD items per layout (`OSD_ITEM_COUNT`) |
*   **Reply Payload (Get Layout):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `itemPositions` | `uint16_t[OSD_ITEM_COUNT]` | `OSD_ITEM_COUNT * 2` | Packed X/Y positions for all items in the requested layout |
*   **Reply Payload (Get Item):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `itemPosition` | `uint16_t` | 2 | Packed X/Y position for the requested item in the requested layout |
*   **Notes:** Requires `USE_OSD`. Returns error if indexes are invalid.

### `MSP2_INAV_OSD_SET_LAYOUT_ITEM` (0x2013 / 8211)

*   **Direction:** In
*   **Description:** Sets the position of a single OSD item within a specific layout.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `layoutIndex` | `uint8_t` | 1 | Index | Index of the OSD layout (0 to `OSD_LAYOUT_COUNT - 1`) |
    | `itemIndex` | `uint8_t` | 1 | Index | Index of the OSD item (`OSD_ITEM_*` enum) |
    | `itemPosition` | `uint16_t` | 2 | Coordinates | Packed X/Y position (`(Y << 8) | X`) to set |
*   **Notes:** Requires `USE_OSD`. Expects 4 bytes. Returns error if indexes are invalid. If the modified layout is not the currently active one, it temporarily overrides the active layout for 10 seconds to show the change. Otherwise, triggers a full OSD redraw.

### `MSP2_INAV_OSD_ALARMS` (0x2014 / 8212)

*   **Direction:** Out
*   **Description:** Retrieves OSD alarm threshold settings.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `rssiAlarm` | `uint8_t` | 1 | % | RSSI alarm threshold (`osdConfig()->rssi_alarm`) |
    | `timerAlarm` | `uint16_t` | 2 | seconds | Timer alarm threshold (`osdConfig()->time_alarm`) |
    | `altAlarm` | `uint16_t` | 2 | meters | Altitude alarm threshold (`osdConfig()->alt_alarm`) |
    | `distAlarm` | `uint16_t` | 2 | meters | Distance alarm threshold (`osdConfig()->dist_alarm`) |
    | `negAltAlarm` | `uint16_t` | 2 | meters | Negative altitude alarm threshold (`osdConfig()->neg_alt_alarm`) |
    | `gForceAlarm` | `uint16_t` | 2 | G * 1000 | G-force alarm threshold (`osdConfig()->gforce_alarm * 1000`) |
    | `gForceAxisMinAlarm`| `int16_t` | 2 | G * 1000 | Min G-force per-axis alarm (`osdConfig()->gforce_axis_alarm_min * 1000`) |
    | `gForceAxisMaxAlarm`| `int16_t` | 2 | G * 1000 | Max G-force per-axis alarm (`osdConfig()->gforce_axis_alarm_max * 1000`) |
    | `currentAlarm` | `uint8_t` | 1 | 0.1 A ? | Current draw alarm threshold (`osdConfig()->current_alarm`). Units may need verification |
    | `imuTempMinAlarm` | `uint16_t` | 2 | degrees C | Min IMU temperature alarm (`osdConfig()->imu_temp_alarm_min`) |
    | `imuTempMaxAlarm` | `uint16_t` | 2 | degrees C | Max IMU temperature alarm (`osdConfig()->imu_temp_alarm_max`) |
    | `baroTempMinAlarm` | `uint16_t` | 2 | degrees C | Min Baro temperature alarm (`osdConfig()->baro_temp_alarm_min`). 0 if `USE_BARO` disabled |
    | `baroTempMaxAlarm` | `uint16_t` | 2 | degrees C | Max Baro temperature alarm (`osdConfig()->baro_temp_alarm_max`). 0 if `USE_BARO` disabled |
    | `adsbWarnDistance`| `uint16_t` | 2 | meters | ADSB warning distance (`osdConfig()->adsb_distance_warning`). 0 if `USE_ADSB` disabled |
    | `adsbAlertDistance`| `uint16_t` | 2 | meters | ADSB alert distance (`osdConfig()->adsb_distance_alert`). 0 if `USE_ADSB` disabled |
*   **Notes:** Requires `USE_OSD`.

### `MSP2_INAV_OSD_SET_ALARMS` (0x2015 / 8213)

*   **Direction:** In
*   **Description:** Sets OSD alarm threshold settings.
*   **Payload:** (Matches most of `MSP2_INAV_OSD_ALARMS` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `rssiAlarm` | `uint8_t` | 1 | % | Sets `osdConfigMutable()->rssi_alarm` |
    | `timerAlarm` | `uint16_t` | 2 | seconds | Sets `osdConfigMutable()->time_alarm` |
    | `altAlarm` | `uint16_t` | 2 | meters | Sets `osdConfigMutable()->alt_alarm` |
    | `distAlarm` | `uint16_t` | 2 | meters | Sets `osdConfigMutable()->dist_alarm` |
    | `negAltAlarm` | `uint16_t` | 2 | meters | Sets `osdConfigMutable()->neg_alt_alarm` |
    | `gForceAlarm` | `uint16_t` | 2 | G * 1000 | Sets `osdConfigMutable()->gforce_alarm = value / 1000.0f` |
    | `gForceAxisMinAlarm`| `int16_t` | 2 | G * 1000 | Sets `osdConfigMutable()->gforce_axis_alarm_min = value / 1000.0f` |
    | `gForceAxisMaxAlarm`| `int16_t` | 2 | G * 1000 | Sets `osdConfigMutable()->gforce_axis_alarm_max = value / 1000.0f` |
    | `currentAlarm` | `uint8_t` | 1 | 0.1 A ? | Sets `osdConfigMutable()->current_alarm` |
    | `imuTempMinAlarm` | `uint16_t` | 2 | degrees C | Sets `osdConfigMutable()->imu_temp_alarm_min` |
    | `imuTempMaxAlarm` | `uint16_t` | 2 | degrees C | Sets `osdConfigMutable()->imu_temp_alarm_max` |
    | `baroTempMinAlarm` | `uint16_t` | 2 | degrees C | Sets `osdConfigMutable()->baro_temp_alarm_min` (if `USE_BARO`) |
    | `baroTempMaxAlarm` | `uint16_t` | 2 | degrees C | Sets `osdConfigMutable()->baro_temp_alarm_max` (if `USE_BARO`) |
*   **Notes:** Requires `USE_OSD`. Expects 24 bytes. ADSB alarms are not settable via this message.

### `MSP2_INAV_OSD_PREFERENCES` (0x2016 / 8214)

*   **Direction:** Out
*   **Description:** Retrieves OSD display preferences (video system, units, styles, etc.).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `videoSystem` | `uint8_t` | 1 | Enum: Video system (Auto/PAL/NTSC) (`osdConfig()->video_system`) |
    | `mainVoltageDecimals` | `uint8_t` | 1 | Count: Decimal places for main voltage display (`osdConfig()->main_voltage_decimals`) |
    | `ahiReverseRoll` | `uint8_t` | 1 | Boolean: Reverse roll direction on Artificial Horizon (`osdConfig()->ahi_reverse_roll`) |
    | `crosshairsStyle` | `uint8_t` | 1 | Enum: Style of the center crosshairs (`osdConfig()->crosshairs_style`) |
    | `leftSidebarScroll` | `uint8_t` | 1 | Boolean: Enable scrolling for left sidebar (`osdConfig()->left_sidebar_scroll`) |
    | `rightSidebarScroll` | `uint8_t` | 1 | Boolean: Enable scrolling for right sidebar (`osdConfig()->right_sidebar_scroll`) |
    | `sidebarScrollArrows` | `uint8_t` | 1 | Boolean: Show arrows for scrollable sidebars (`osdConfig()->sidebar_scroll_arrows`) |
    | `units` | `uint8_t` | 1 | Enum: `osd_unit_e` Measurement units (Metric/Imperial) (`osdConfig()->units`) |
    | `statsEnergyUnit` | `uint8_t` | 1 | Enum: Unit for energy display in post-flight stats (`osdConfig()->stats_energy_unit`) |
*   **Notes:** Requires `USE_OSD`.

### `MSP2_INAV_OSD_SET_PREFERENCES` (0x2017 / 8215)

*   **Direction:** In
*   **Description:** Sets OSD display preferences.
*   **Payload:** (Matches `MSP2_INAV_OSD_PREFERENCES` structure)
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `videoSystem` | `uint8_t` | 1 | Sets `osdConfigMutable()->video_system` |
    | `mainVoltageDecimals` | `uint8_t` | 1 | Sets `osdConfigMutable()->main_voltage_decimals` |
    | `ahiReverseRoll` | `uint8_t` | 1 | Sets `osdConfigMutable()->ahi_reverse_roll` |
    | `crosshairsStyle` | `uint8_t` | 1 | Sets `osdConfigMutable()->crosshairs_style` |
    | `leftSidebarScroll` | `uint8_t` | 1 | Sets `osdConfigMutable()->left_sidebar_scroll` |
    | `rightSidebarScroll` | `uint8_t` | 1 | Sets `osdConfigMutable()->right_sidebar_scroll` |
    | `sidebarScrollArrows` | `uint8_t` | 1 | Sets `osdConfigMutable()->sidebar_scroll_arrows` |
    | `units` | `uint8_t` | 1 | Enum `osd_unit_e` Sets `osdConfigMutable()->units` |
    | `statsEnergyUnit` | `uint8_t` | 1 | Sets `osdConfigMutable()->stats_energy_unit` |
*   **Notes:** Requires `USE_OSD`. Expects 9 bytes. Triggers a full OSD redraw.

### `MSP2_INAV_SELECT_BATTERY_PROFILE` (0x2018 / 8216)

*   **Direction:** In
*   **Description:** Selects the active battery profile and saves configuration.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `batteryProfileIndex` | `uint8_t` | 1 | Index of the battery profile to activate (0-based) |
*   **Notes:** Expects 1 byte. Will fail if armed. Calls `setConfigBatteryProfileAndWriteEEPROM()`.

### `MSP2_INAV_DEBUG` (0x2019 / 8217)

*   **Direction:** Out
*   **Description:** Retrieves values from the firmware's 32-bit `debug[]` array. Supersedes `MSP_DEBUG`.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `debugValues` | `uint32_t[DEBUG32_VALUE_COUNT]` | `DEBUG32_VALUE_COUNT * 4` | Values from the `debug` array (typically 8 values) |
*   **Notes:** `DEBUG32_VALUE_COUNT` is usually 8.

### `MSP2_BLACKBOX_CONFIG` (0x201A / 8218)

*   **Direction:** Out
*   **Description:** Retrieves the Blackbox configuration. Supersedes `MSP_BLACKBOX_CONFIG`.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `blackboxSupported` | `uint8_t` | 1 | Boolean: 1 if Blackbox is supported (`USE_BLACKBOX`), 0 otherwise |
    | `blackboxDevice` | `uint8_t` | 1 | Enum (`blackboxDevice_e`): Target device for logging (`blackboxConfig()->device`). 0 if not supported |
    | `blackboxRateNum` | `uint16_t` | 2 | Numerator for logging rate divider (`blackboxConfig()->rate_num`). 0 if not supported |
    | `blackboxRateDenom` | `uint16_t` | 2 | Denominator for logging rate divider (`blackboxConfig()->rate_denom`). 0 if not supported |
    | `blackboxIncludeFlags`| `uint32_t`| 4 | Bitmask: Flags for fields included/excluded from logging (`blackboxConfig()->includeFlags`) |
*   **Notes:** Requires `USE_BLACKBOX`.

### `MSP2_SET_BLACKBOX_CONFIG` (0x201B / 8219)

*   **Direction:** In
*   **Description:** Sets the Blackbox configuration. Supersedes `MSP_SET_BLACKBOX_CONFIG`.
*   **Payload:** (Matches `MSP2_BLACKBOX_CONFIG` structure, excluding `blackboxSupported`)
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `blackboxDevice` | `uint8_t` | 1 | Sets `blackboxConfigMutable()->device` |
    | `blackboxRateNum` | `uint16_t` | 2 | Sets `blackboxConfigMutable()->rate_num` |
    | `blackboxRateDenom` | `uint16_t` | 2 | Sets `blackboxConfigMutable()->rate_denom` |
    | `blackboxIncludeFlags`| `uint32_t`| 4 | Sets `blackboxConfigMutable()->includeFlags` |
*   **Notes:** Requires `USE_BLACKBOX`. Expects 9 bytes. Returns error if Blackbox is currently logging (`!blackboxMayEditConfig()`).

### `MSP2_INAV_TEMP_SENSOR_CONFIG` (0x201C / 8220)

*   **Direction:** Out
*   **Description:** Retrieves the configuration for all onboard temperature sensors.
*   **Payload:** Repeated `MAX_TEMP_SENSORS` times:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `type` | `uint8_t` | 1 | Enum (`tempSensorType_e`): Type of the temperature sensor |
    | `address` | `uint64_t` | 8 | Sensor address/ID (e.g., for 1-Wire sensors) |
    | `alarmMin` | `uint16_t` | 2 | Min temperature alarm threshold (degrees C) |
    | `alarmMax` | `uint16_t` | 2 | Max temperature alarm threshold (degrees C) |
    | `osdSymbol` | `uint8_t` | 1 | Index: OSD symbol to use for this sensor (0 to `TEMP_SENSOR_SYM_COUNT`) |
    | `label` | `char[TEMPERATURE_LABEL_LEN]` | `TEMPERATURE_LABEL_LEN` | User-defined label for the sensor |
*   **Notes:** Requires `USE_TEMPERATURE_SENSOR`.

### `MSP2_INAV_SET_TEMP_SENSOR_CONFIG` (0x201D / 8221)

*   **Direction:** In
*   **Description:** Sets the configuration for all onboard temperature sensors.
*   **Payload:** Repeated `MAX_TEMP_SENSORS` times (matches `MSP2_INAV_TEMP_SENSOR_CONFIG` structure):
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `type` | `uint8_t` | 1 | Sets sensor type |
    | `address` | `uint64_t` | 8 | Sets sensor address/ID |
    | `alarmMin` | `uint16_t` | 2 | Sets min alarm threshold |
    | `alarmMax` | `uint16_t` | 2 | Sets max alarm threshold |
    | `osdSymbol` | `uint8_t` | 1 | Sets OSD symbol index (validated) |
    | `label` | `char[TEMPERATURE_LABEL_LEN]` | `TEMPERATURE_LABEL_LEN` | Sets sensor label (converted to uppercase) |
*   **Notes:** Requires `USE_TEMPERATURE_SENSOR`. Expects `MAX_TEMP_SENSORS * sizeof(tempSensorConfig_t)` bytes.

### `MSP2_INAV_TEMPERATURES` (0x201E / 8222)

*   **Direction:** Out
*   **Description:** Retrieves the current readings from all configured temperature sensors.
*   **Payload:** Repeated `MAX_TEMP_SENSORS` times:
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `temperature` | `int16_t` | 2 | degrees C | Current temperature reading. -1000 if sensor is invalid or reading failed |
*   **Notes:** Requires `USE_TEMPERATURE_SENSOR`.

### `MSP_SIMULATOR` (0x201F / 8223)

*   **Direction:** In/Out
*   **Description:** Handles Hardware-in-the-Loop (HITL) simulation data exchange. Receives simulated sensor data and options, sends back control outputs and debug info.
*   **Request Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `simulatorVersion` | `uint8_t` | 1 | Version of the simulator protocol (`SIMULATOR_MSP_VERSION`) |
    | `hitlFlags` | `uint8_t` | 1 | Bitmask: Options for HITL (`HITL_*` flags) |
    | `gpsFixType` | `uint8_t` | 1 | (If `HITL_HAS_NEW_GPS_DATA`) Simulated GPS fix type |
    | `gpsNumSat` | `uint8_t` | 1 | (If `HITL_HAS_NEW_GPS_DATA`) Simulated satellite count |
    | `gpsLat` | `int32_t` | 4 | (If `HITL_HAS_NEW_GPS_DATA`) Simulated latitude (1e7 deg) |
    | `gpsLon` | `int32_t` | 4 | (If `HITL_HAS_NEW_GPS_DATA`) Simulated longitude (1e7 deg) |
    | `gpsAlt` | `int32_t` | 4 | (If `HITL_HAS_NEW_GPS_DATA`) Simulated altitude (cm) |
    | `gpsSpeed` | `uint16_t` | 2 | (If `HITL_HAS_NEW_GPS_DATA`) Simulated ground speed (cm/s) |
    | `gpsCourse` | `uint16_t` | 2 | (If `HITL_HAS_NEW_GPS_DATA`) Simulated ground course (deci-deg) |
    | `gpsVelN` | `int16_t` | 2 | (If `HITL_HAS_NEW_GPS_DATA`) Simulated North velocity (cm/s) |
    | `gpsVelE` | `int16_t` | 2 | (If `HITL_HAS_NEW_GPS_DATA`) Simulated East velocity (cm/s) |
    | `gpsVelD` | `int16_t` | 2 | (If `HITL_HAS_NEW_GPS_DATA`) Simulated Down velocity (cm/s) |
    | `imuRoll` | `int16_t` | 2 | (If NOT `HITL_USE_IMU`) Simulated Roll (deci-deg) |
    | `imuPitch` | `int16_t` | 2 | (If NOT `HITL_USE_IMU`) Simulated Pitch (deci-deg) |
    | `imuYaw` | `int16_t` | 2 | (If NOT `HITL_USE_IMU`) Simulated Yaw (deci-deg) |
    | `accX` | `int16_t` | 2 | mG (G * 1000) | Simulated Accelerometer X |
    | `accY` | `int16_t` | 2 | mG (G * 1000) | Simulated Accelerometer Y |
    | `accZ` | `int16_t` | 2 | mG (G * 1000) | Simulated Accelerometer Z |
    | `gyroX` | `int16_t` | 2 | dps * 16 | Simulated Gyroscope X rate |
    | `gyroY` | `int16_t` | 2 | dps * 16 | Simulated Gyroscope Y rate |
    | `gyroZ` | `int16_t` | 2 | dps * 16 | Simulated Gyroscope Z rate |
    | `baroPressure` | `uint32_t` | 4 | Pa | Simulated Barometer pressure |
    | `magX` | `int16_t` | 2 | Scaled | Simulated Magnetometer X (scaled by 20) |
    | `magY` | `int16_t` | 2 | Scaled | Simulated Magnetometer Y (scaled by 20) |
    | `magZ` | `int16_t` | 2 | Scaled | Simulated Magnetometer Z (scaled by 20) |
    | `vbat` | `uint8_t` | 1 | (If `HITL_EXT_BATTERY_VOLTAGE`) Simulated battery voltage (0.1V units) |
    | `airspeed` | `uint16_t` | 2 | (If `HITL_AIRSPEED`) Simulated airspeed (cm/s) |
    | `extFlags` | `uint8_t` | 1 | (If `HITL_EXTENDED_FLAGS`) Additional flags (upper 8 bits) |
*   **Reply Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `stabilizedRoll` | `uint16_t` | 2 | Stabilized Roll command output (-500 to 500) |
    | `stabilizedPitch`| `uint16_t` | 2 | Stabilized Pitch command output (-500 to 500) |
    | `stabilizedYaw` | `uint16_t` | 2 | Stabilized Yaw command output (-500 to 500) |
    | `stabilizedThrottle`| `uint16_t`| 2 | Stabilized Throttle command output (-500 to 500 if armed, else -500) |
    | `debugFlags` | `uint8_t` | 1 | Packed flags: Debug index (0-7), Platform type, Armed state, OSD feature status |
    | `debugValue` | `uint32_t` | 4 | Current debug value (`debug[simulatorData.debugIndex]`) |
    | `attitudeRoll` | `int16_t` | 2 | Current estimated Roll (deci-deg) |
    | `attitudePitch`| `int16_t` | 2 | Current estimated Pitch (deci-deg) |
    | `attitudeYaw` | `int16_t` | 2 | Current estimated Yaw (deci-deg) |
    | `osdHeader` | `uint8_t` | 1 | OSD RLE Header (255) |
    | `osdRows` | `uint8_t` | 1 | (If OSD supported) Number of OSD rows |
    | `osdCols` | `uint8_t` | 1 | (If OSD supported) Number of OSD columns |
    | `osdStartY` | `uint8_t` | 1 | (If OSD supported) Starting row for RLE data |
    | `osdStartX` | `uint8_t` | 1 | (If OSD supported) Starting column for RLE data |
    | `osdRleData` | `uint8_t[]` | Variable | (If OSD supported) Run-length encoded OSD character data. Terminated by `[0, 0]` |
*   **Notes:** Requires `USE_SIMULATOR`. Complex message handling state changes for enabling/disabling HITL. Sensor data is injected directly. OSD data is sent using a custom RLE scheme. See `simulatorData` struct and associated code for details.

### `MSP2_INAV_SERVO_MIXER` (0x2020 / 8224)

*   **Direction:** Out
*   **Description:** Retrieves the custom servo mixer rules, including programming framework condition IDs, for primary and secondary mixer profiles. Supersedes `MSP_SERVO_MIX_RULES`.
*   **Payload:** Repeated `MAX_SERVO_RULES` times:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `targetChannel` | `uint8_t` | 1 | Servo output channel index (0-based) |
    | `inputSource` | `uint8_t` | 1 | Enum `inputSource_e` Input source |
    | `rate` | `uint16_t` | 2 | Mixing rate/weight |
    | `speed` | `uint8_t` | 1 | Speed/Slew rate limit (0-100) |
    | `conditionId` | `uint8_t` | 1 | Logic Condition ID (0 to `MAX_LOGIC_CONDITIONS - 1`, or 255/-1 if none/disabled) |
    | `targetChannel` | `uint8_t` | 1 | (Optional) Profile 2 Target channel |
    | `inputSource` | `uint8_t` | 1 | (Optional) Profile 2 Enum `inputSource_e` Input source |
    | `rate` | `uint16_t` | 2 | (Optional) Profile 2 Rate |
    | `speed` | `uint8_t` | 1 | (Optional) Profile 2 Speed |
    | `conditionId` | `uint8_t` | 1 | (Optional) Profile 2 Logic Condition ID |
*   **Notes:** `conditionId` requires `USE_PROGRAMMING_FRAMEWORK`.

### `MSP2_INAV_SET_SERVO_MIXER` (0x2021 / 8225)

*   **Direction:** In
*   **Description:** Sets a single custom servo mixer rule, including programming framework condition ID. Supersedes `MSP_SET_SERVO_MIX_RULE`.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `ruleIndex` | `uint8_t` | 1 | Index of the rule to set (0 to `MAX_SERVO_RULES - 1`) |
    | `targetChannel` | `uint8_t` | 1 | Servo output channel index |
    | `inputSource` | `uint8_t` | 1 | Enum `inputSource_e` Input source |
    | `rate` | `uint16_t` | 2 | Mixing rate/weight |
    | `speed` | `uint8_t` | 1 | Speed/Slew rate limit (0-100) |
    | `conditionId` | `uint8_t` | 1 | Logic Condition ID (255/-1 if none). Ignored if `USE_PROGRAMMING_FRAMEWORK` is disabled |
*   **Notes:** Expects 7 bytes. Returns error if index invalid. Calls `loadCustomServoMixer()`.

### `MSP2_INAV_LOGIC_CONDITIONS` (0x2022 / 8226)

*   **Direction:** Out
*   **Description:** Retrieves the configuration of all defined Logic Conditions.
*   **Payload:** Repeated `MAX_LOGIC_CONDITIONS` times:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `enabled` | `uint8_t` | 1 | Boolean: 1 if the condition is enabled |
    | `activatorId` | `uint8_t` | 1 | ID of the activator condition (if any, 255 if none) |
    | `operation` | `uint8_t` | 1 | Enum `logicConditionOp_e` Logical operation (AND, OR, XOR, etc.) |
    | `operandAType` | `uint8_t` | 1 | Enum `logicOperandType_e` Type of the first operand (Flight Mode, GVAR, etc.) |
    | `operandAValue` | `uint32_t` | 4 | Value/ID of the first operand |
    | `operandBType` | `uint8_t` | 1 | Enum `logicOperandType_e`: Type of the second operand |
    | `operandBValue` | `uint32_t` | 4 | Value/ID of the second operand |
    | `flags` | `uint8_t` | 1 | Bitmask: Condition flags (e.g., `LC_FLAG_FIRST_TIME_TRUE`) |
*   **Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`. See `logicCondition_t` structure.

### `MSP2_INAV_SET_LOGIC_CONDITIONS` (0x2023 / 8227)

*   **Direction:** In
*   **Description:** Sets the configuration for a single Logic Condition by its index.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `conditionIndex` | `uint8_t` | 1 | Index of the condition to set (0 to `MAX_LOGIC_CONDITIONS - 1`) |
    | `enabled` | `uint8_t` | 1 | Boolean: 1 to enable the condition |
    | `activatorId` | `uint8_t` | 1 | Activator condition ID |
    | `operation` | `uint8_t` | 1 | Enum `logicConditionOp_e` Logical operation |
    | `operandAType` | `uint8_t` | 1 | Enum `logicOperandType_e` Type of operand A |
    | `operandAValue` | `uint32_t` | 4 | Value/ID of operand A |
    | `operandBType` | `uint8_t` | 1 | Enum `logicOperandType_e` Type of operand B |
    | `operandBValue` | `uint32_t` | 4 | Value/ID of operand B |
    | `flags` | `uint8_t` | 1 | Bitmask: Condition flags |
*   **Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`. Expects 15 bytes. Returns error if index is invalid.

### `MSP2_INAV_GLOBAL_FUNCTIONS` (0x2024 / 8228)

*   **Not implemented**

### `MSP2_INAV_SET_GLOBAL_FUNCTIONS` (0x2025 / 8229)

*   **Not implemented**

### `MSP2_INAV_LOGIC_CONDITIONS_STATUS` (0x2026 / 8230)

*   **Direction:** Out
*   **Description:** Retrieves the current evaluated status (true/false or numerical value) of all logic conditions.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `conditionValues` | `uint32_t[MAX_LOGIC_CONDITIONS]` | `MAX_LOGIC_CONDITIONS * 4` | Array of current values for each logic condition (`logicConditionGetValue(i)`). 1 for true, 0 for false, or numerical value depending on operation |
*   **Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`.

### `MSP2_INAV_GVAR_STATUS` (0x2027 / 8231)

*   **Direction:** Out
*   **Description:** Retrieves the current values of all Global Variables (GVARS).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `gvarValues` | `uint32_t[MAX_GLOBAL_VARIABLES]` | `MAX_GLOBAL_VARIABLES * 4` | Array of current values for each global variable (`gvGet(i)`) |
*   **Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`.

### `MSP2_INAV_PROGRAMMING_PID` (0x2028 / 8232)

*   **Direction:** Out
*   **Description:** Retrieves the configuration of all Programming PIDs.
*   **Payload:** Repeated `MAX_PROGRAMMING_PID_COUNT` times:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `enabled` | `uint8_t` | 1 | Boolean: 1 if the PID is enabled |
    | `setpointType` | `uint8_t` | 1 | Enum (`logicOperandType_e`) Type of the setpoint source |
    | `setpointValue` | `uint32_t` | 4 | Value/ID of the setpoint source |
    | `measurementType` | `uint8_t` | 1 | Enum (`logicOperandType_e`) Type of the measurement source |
    | `measurementValue` | `uint32_t` | 4 | Value/ID of the measurement source |
    | `gainP` | `uint16_t` | 2 | Proportional gain |
    | `gainI` | `uint16_t` | 2 | Integral gain |
    | `gainD` | `uint16_t` | 2 | Derivative gain |
    | `gainFF` | `uint16_t` | 2 | Feed-forward gain |
*   **Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`. See `programmingPid_t` structure.

### `MSP2_INAV_SET_PROGRAMMING_PID` (0x2029 / 8233)

*   **Direction:** In
*   **Description:** Sets the configuration for a single Programming PID by its index.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `pidIndex` | `uint8_t` | 1 | Index of the Programming PID to set (0 to `MAX_PROGRAMMING_PID_COUNT - 1`) |
    | `enabled` | `uint8_t` | 1 | Boolean: 1 to enable the PID |
    | `setpointType` | `uint8_t` | 1 | Enum (`logicOperandType_e`) Type of the setpoint source |
    | `setpointValue` | `uint32_t` | 4 | Value/ID of the setpoint source |
    | `measurementType` | `uint8_t` | 1 | Enum (`logicOperandType_e`) Type of the measurement source |
    | `measurementValue` | `uint32_t` | 4 | Value/ID of the measurement source |
    | `gainP` | `uint16_t` | 2 | Proportional gain |
    | `gainI` | `uint16_t` | 2 | Integral gain |
    | `gainD` | `uint16_t` | 2 | Derivative gain |
    | `gainFF` | `uint16_t` | 2 | Feed-forward gain |
*   **Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`. Expects 20 bytes. Returns error if index is invalid.

### `MSP2_INAV_PROGRAMMING_PID_STATUS` (0x202A / 8234)

*   **Direction:** Out
*   **Description:** Retrieves the current output value of all Programming PIDs.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `pidOutputs` | `uint32_t[MAX_PROGRAMMING_PID_COUNT]` | `MAX_PROGRAMMING_PID_COUNT * 4` | Array of current output values for each Programming PID (`programmingPidGetOutput(i)`) |
*   **Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`.

### `MSP2_PID` (0x2030 / 8240)

*   **Direction:** Out
*   **Description:** Retrieves the standard PID controller gains (P, I, D, FF) for the current PID profile.
*   **Payload:** Repeated `PID_ITEM_COUNT` times:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `P` | `uint8_t` | 1 | Proportional gain (`pidBank()->pid[i].P`), constrained 0-255 |
    | `I` | `uint8_t` | 1 | Integral gain (`pidBank()->pid[i].I`), constrained 0-255 |
    | `D` | `uint8_t` | 1 | Derivative gain (`pidBank()->pid[i].D`), constrained 0-255 |
    | `FF` | `uint8_t` | 1 | Feed-forward gain (`pidBank()->pid[i].FF`), constrained 0-255 |
*   **Notes:** `PID_ITEM_COUNT` defines the number of standard PID controllers (Roll, Pitch, Yaw, Alt, Vel, etc.). Updates from EZ-Tune if enabled.

### `MSP2_SET_PID` (0x2031 / 8241)

*   **Direction:** In
*   **Description:** Sets the standard PID controller gains (P, I, D, FF) for the current PID profile.
*   **Payload:** Repeated `PID_ITEM_COUNT` times:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `P` | `uint8_t` | 1 | Sets Proportional gain (`pidBankMutable()->pid[i].P`) |
    | `I` | `uint8_t` | 1 | Sets Integral gain (`pidBankMutable()->pid[i].I`) |
    | `D` | `uint8_t` | 1 | Sets Derivative gain (`pidBankMutable()->pid[i].D`) |
    | `FF` | `uint8_t` | 1 | Sets Feed-forward gain (`pidBankMutable()->pid[i].FF`) |
*   **Notes:** Expects `PID_ITEM_COUNT * 4` bytes. Calls `schedulePidGainsUpdate()` and `navigationUsePIDs()`.

### `MSP2_INAV_OPFLOW_CALIBRATION` (0x2032 / 8242)

*   **Direction:** In
*   **Description:** Starts the optical flow sensor calibration procedure.
*   **Payload:** None
*   **Notes:** Requires `USE_OPFLOW`. Will fail if armed. Calls `opflowStartCalibration()`.

### `MSP2_INAV_FWUPDT_PREPARE` (0x2033 / 8243)

*   **Direction:** In
*   **Description:** Prepares the flight controller to receive a firmware update via MSP.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `firmwareSize` | `uint32_t` | 4 | Total size of the incoming firmware file in bytes |
*   **Notes:** Requires `MSP_FIRMWARE_UPDATE`. Expects 4 bytes. Returns error if preparation fails (e.g., no storage, invalid size). Calls `firmwareUpdatePrepare()`.

### `MSP2_INAV_FWUPDT_STORE` (0x2034 / 8244)

*   **Direction:** In
*   **Description:** Stores a chunk of firmware data received via MSP.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `firmwareChunk` | `uint8_t[]` | Variable | Chunk of firmware data |
*   **Notes:** Requires `MSP_FIRMWARE_UPDATE`. Returns error if storage fails (e.g., out of space, checksum error). Called repeatedly until the entire firmware is transferred. Calls `firmwareUpdateStore()`.

### `MSP2_INAV_FWUPDT_EXEC` (0x2035 / 8245)

*   **Direction:** In
*   **Description:** Executes the firmware update process (flashes the stored firmware and reboots).
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `updateType` | `uint8_t` | 1 | Type of update (e.g., full flash, specific section - currently ignored/unused) |
*   **Notes:** Requires `MSP_FIRMWARE_UPDATE`. Expects 1 byte. Returns error if update cannot start (e.g., not fully received). Calls `firmwareUpdateExec()`. If successful, the device will reboot into the new firmware.

### `MSP2_INAV_FWUPDT_ROLLBACK_PREPARE` (0x2036 / 8246)

*   **Direction:** In
*   **Description:** Prepares the flight controller to perform a firmware rollback to the previously stored version.
*   **Payload:** None
*   **Notes:** Requires `MSP_FIRMWARE_UPDATE`. Returns error if rollback preparation fails (e.g., no rollback image available). Calls `firmwareUpdateRollbackPrepare()`.

### `MSP2_INAV_FWUPDT_ROLLBACK_EXEC` (0x2037 / 8247)

*   **Direction:** In
*   **Description:** Executes the firmware rollback process (flashes the stored backup firmware and reboots).
*   **Payload:** None
*   **Notes:** Requires `MSP_FIRMWARE_UPDATE`. Returns error if rollback cannot start. Calls `firmwareUpdateRollbackExec()`. If successful, the device will reboot into the backup firmware.

### `MSP2_INAV_SAFEHOME` (0x2038 / 8248)

*   **Direction:** In/Out
*   **Description:** Get or Set configuration for a specific Safe Home location.
*   **Request Payload (Get):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `safehomeIndex` | `uint8_t` | 1 | Index of the safe home location (0 to `MAX_SAFE_HOMES - 1`) |
*   **Reply Payload (Get):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `safehomeIndex` | `uint8_t` | 1 | Index requested |
    | `enabled` | `uint8_t` | 1 | Boolean: 1 if this safe home is enabled |
    | `latitude` | `int32_t` | 4 | Latitude (1e7 deg) |
    | `longitude` | `int32_t` | 4 | Longitude (1e7 deg) |
*   **Notes:** Requires `USE_SAFE_HOME`. Used by `mspFcSafeHomeOutCommand`. See `MSP2_INAV_SET_SAFEHOME` for setting.

### `MSP2_INAV_SET_SAFEHOME` (0x2039 / 8249)

*   **Direction:** In
*   **Description:** Sets the configuration for a specific Safe Home location.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `safehomeIndex` | `uint8_t` | 1 | Index of the safe home location (0 to `MAX_SAFE_HOMES - 1`) |
    | `enabled` | `uint8_t` | 1 | Boolean: 1 to enable this safe home |
    | `latitude` | `int32_t` | 4 | Latitude (1e7 deg) |
    | `longitude` | `int32_t` | 4 | Longitude (1e7 deg) |
*   **Notes:** Requires `USE_SAFE_HOME`. Expects 10 bytes. Returns error if index invalid. Resets corresponding FW autoland approach if `USE_FW_AUTOLAND` is enabled.

### `MSP2_INAV_MISC2` (0x203A / 8250)

*   **Direction:** Out
*   **Description:** Retrieves miscellaneous runtime information including timers and throttle status.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `uptimeSeconds` | `uint32_t` | 4 | Seconds | Time since boot (`micros() / 1000000`) |
    | `flightTimeSeconds` | `uint32_t` | 4 | Seconds | Accumulated flight time (`getFlightTime()`) |
    | `throttlePercent` | `uint8_t` | 1 | % | Current throttle output percentage (`getThrottlePercent(true)`) |
    | `autoThrottleFlag` | `uint8_t` | 1 | Boolean | 1 if navigation is controlling throttle, 0 otherwise (`navigationIsControllingThrottle()`) |

### `MSP2_INAV_LOGIC_CONDITIONS_SINGLE` (0x203B / 8251)

*   **Direction:** In/Out
*   **Description:** Gets the configuration for a single Logic Condition by its index.
*   **Request Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `conditionIndex` | `uint8_t` | 1 | Index of the condition to retrieve (0 to `MAX_LOGIC_CONDITIONS - 1`) |
*   **Reply Payload:** (Matches structure of one entry in `MSP2_INAV_LOGIC_CONDITIONS`)
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `enabled` | `uint8_t` | 1 | Boolean: 1 if enabled |
    | `activatorId` | `uint8_t` | 1 | Activator ID |
    | `operation` | `uint8_t` | 1 | Enum `logicConditionOp_e` Logical operation |
    | `operandAType` | `uint8_t` | 1 | Enum `logicOperandType_e` Type of operand A |
    | `operandAValue` | `uint32_t` | 4 | Value/ID of operand A |
    | `operandBType` | `uint8_t` | 1 | Enum `logicOperandType_e` Type of operand B |
    | `operandBValue` | `uint32_t` | 4 | Value/ID of operand B |
    | `flags` | `uint8_t` | 1 | Bitmask: Condition flags |
*   **Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`. Used by `mspFcLogicConditionCommand`.

### `MSP2_INAV_ESC_RPM` (0x2040 / 8256)

*   **Direction:** Out
*   **Description:** Retrieves the RPM reported by each ESC via telemetry.
*   **Payload:** Repeated `getMotorCount()` times:
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `escRpm` | `uint32_t` | 4 | RPM | RPM reported by the ESC |
*   **Notes:** Requires `USE_ESC_SENSOR`. Payload size depends on the number of detected motors with telemetry.

### `MSP2_INAV_ESC_TELEM` (0x2041 / 8257)

*   **Direction:** Out
*   **Description:** Retrieves the full telemetry data structure reported by each ESC.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `motorCount` | `uint8_t` | 1 | Number of motors reporting telemetry (`getMotorCount()`) |
    | `escData` | `escSensorData_t[]` | `motorCount * sizeof(escSensorData_t)` | Array of `escSensorData_t` structures containing voltage, current, temp, RPM, errors etc. for each ESC |
*   **Notes:** Requires `USE_ESC_SENSOR`. See `escSensorData_t` in `sensors/esc_sensor.h` for the exact structure fields.

### `MSP2_INAV_LED_STRIP_CONFIG_EX` (0x2048 / 8264)

*   **Direction:** Out
*   **Description:** Retrieves the full configuration for each LED on the strip using the `ledConfig_t` structure. Supersedes `MSP_LED_STRIP_CONFIG`.
*   **Payload:** Repeated `LED_MAX_STRIP_LENGTH` times:
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `ledConfig` | `uint16_t` | 6 | Full configuration structure for the LED, size sizeof(ledConfig_t) |
*   **Notes:** Requires `USE_LED_STRIP`. See `ledConfig_t` in `io/ledstrip.h` for structure fields (position, function, overlay, color, direction, params).

### `MSP2_INAV_SET_LED_STRIP_CONFIG_EX` (0x2049 / 8265)

*   **Direction:** In
*   **Description:** Sets the configuration for a single LED on the strip using the `ledConfig_t` structure. Supersedes `MSP_SET_LED_STRIP_CONFIG`.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `ledIndex` | `uint8_t` | 1 | Index of the LED to configure (0 to `LED_MAX_STRIP_LENGTH - 1`) |
    | `ledConfig` |`uint16_t` | 6  Full configuration structure for the LED , size sizeof(ledConfig_t) |
*   **Notes:** Requires `USE_LED_STRIP`. Expects `1 + sizeof(ledConfig_t)` bytes. Returns error if index invalid. Calls `reevaluateLedConfig()`.

### `MSP2_INAV_FW_APPROACH` (0x204A / 8266)

*   **Direction:** In/Out
*   **Description:** Get or Set configuration for a specific Fixed Wing Autoland approach.
*   **Request Payload (Get):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `approachIndex` | `uint8_t` | 1 | Index of the approach setting (0 to `MAX_FW_LAND_APPOACH_SETTINGS - 1`) |
*   **Reply Payload (Get):**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `approachIndex` | `uint8_t` | 1 | Index | Index requested |
    | `approachAlt` | `uint32_t` | 4 | cm | Altitude for the approach phase |
    | `landAlt` | `uint32_t` | 4 | cm | Altitude for the final landing phase |
    | `approachDirection` | `uint8_t` | 1 | Enum | Enum `fwAutolandApproachDirection_e`: Direction of approach (From WP, Specific Heading) |
    | `landHeading1` | `int16_t` | 2 | degrees | Primary landing heading (if approachDirection requires it) |
    | `landHeading2` | `int16_t` | 2 | degrees | Secondary landing heading (if approachDirection requires it) |
    | `isSeaLevelRef` | `uint8_t` | 1 | Boolean | 1 if altitudes are relative to sea level, 0 if relative to home |
*   **Notes:** Requires `USE_FW_AUTOLAND`. Used by `mspFwApproachOutCommand`. See `MSP2_INAV_SET_FW_APPROACH` for setting.

### `MSP2_INAV_SET_FW_APPROACH` (0x204B / 8267)

*   **Direction:** In
*   **Description:** Sets the configuration for a specific Fixed Wing Autoland approach.
*   **Payload:** (Matches `MSP2_INAV_FW_APPROACH` reply structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `approachIndex` | `uint8_t` | 1 | Index | Index of the approach setting (0 to `MAX_FW_LAND_APPOACH_SETTINGS - 1`) |
    | `approachAlt` | `uint32_t` | 4 | cm | Sets approach altitude |
    | `landAlt` | `uint32_t` | 4 | cm | Sets landing altitude |
    | `approachDirection` | `uint8_t` | 1 | Enum | Enum `fwAutolandApproachDirection_e` Sets approach direction |
    | `landHeading1` | `int16_t` | 2 | degrees | Sets primary landing heading |
    | `landHeading2` | `int16_t` | 2 | degrees | Sets secondary landing heading |
    | `isSeaLevelRef` | `uint8_t` | 1 | Boolean | Sets altitude reference |
*   **Notes:** Requires `USE_FW_AUTOLAND`. Expects 15 bytes. Returns error if index invalid.

### `MSP2_INAV_GPS_UBLOX_COMMAND` (0x2050 / 8272)

*   **Direction:** In
*   **Description:** Sends a raw command directly to a U-Blox GPS module connected to the FC.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `ubxCommand` | `uint8_t[]` | Variable (>= 8) | Raw U-Blox UBX protocol command frame (including header, class, ID, length, payload, checksum) |
*   **Notes:** Requires GPS feature enabled (`FEATURE_GPS`) and the GPS driver to be U-Blox (`isGpsUblox()`). Payload must be at least 8 bytes (minimum UBX frame size). Use with extreme caution, incorrect commands can misconfigure the GPS module. Calls `gpsUbloxSendCommand()`.

### `MSP2_INAV_RATE_DYNAMICS` (0x2060 / 8288)

*   **Direction:** Out
*   **Description:** Retrieves Rate Dynamics configuration parameters for the current control rate profile.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `sensitivityCenter` | `uint8_t` | 1 | % | Sensitivity at stick center (`currentControlRateProfile->rateDynamics.sensitivityCenter`) |
    | `sensitivityEnd` | `uint8_t` | 1 | % | Sensitivity at stick ends (`currentControlRateProfile->rateDynamics.sensitivityEnd`) |
    | `correctionCenter` | `uint8_t` | 1 | % | Correction strength at stick center (`currentControlRateProfile->rateDynamics.correctionCenter`) |
    | `correctionEnd` | `uint8_t` | 1 | % | Correction strength at stick ends (`currentControlRateProfile->rateDynamics.correctionEnd`) |
    | `weightCenter` | `uint8_t` | 1 | % | Transition weight at stick center (`currentControlRateProfile->rateDynamics.weightCenter`) |
    | `weightEnd` | `uint8_t` | 1 | % | Transition weight at stick ends (`currentControlRateProfile->rateDynamics.weightEnd`) |
*   **Notes:** Requires `USE_RATE_DYNAMICS`.

### `MSP2_INAV_SET_RATE_DYNAMICS` (0x2061 / 8289)

*   **Direction:** In
*   **Description:** Sets Rate Dynamics configuration parameters for the current control rate profile.
*   **Payload:** (Matches `MSP2_INAV_RATE_DYNAMICS` structure)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `sensitivityCenter` | `uint8_t` | 1 | % | Sets sensitivity at center |
    | `sensitivityEnd` | `uint8_t` | 1 | % | Sets sensitivity at ends |
    | `correctionCenter` | `uint8_t` | 1 | % | Sets correction at center |
    | `correctionEnd` | `uint8_t` | 1 | % | Sets correction at ends |
    | `weightCenter` | `uint8_t` | 1 | % | Sets weight at center |
    | `weightEnd` | `uint8_t` | 1 | % | Sets weight at ends |
*   **Notes:** Requires `USE_RATE_DYNAMICS`. Expects 6 bytes.

### `MSP2_INAV_EZ_TUNE` (0x2070 / 8304)

*   **Direction:** Out
*   **Description:** Retrieves the current EZ-Tune parameters.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `enabled` | `uint8_t` | 1 | Boolean: 1 if EZ-Tune is enabled (`ezTune()->enabled`) |
    | `filterHz` | `uint16_t` | 2 | Filter frequency used during tuning (`ezTune()->filterHz`) |
    | `axisRatio` | `uint8_t` | 1 | Roll vs Pitch axis tuning ratio (`ezTune()->axisRatio`) |
    | `response` | `uint8_t` | 1 | Desired response characteristic (`ezTune()->response`) |
    | `damping` | `uint8_t` | 1 | Desired damping characteristic (`ezTune()->damping`) |
    | `stability` | `uint8_t` | 1 | Stability preference (`ezTune()->stability`) |
    | `aggressiveness` | `uint8_t` | 1 | Aggressiveness preference (`ezTune()->aggressiveness`) |
    | `rate` | `uint8_t` | 1 | Resulting rate setting (`ezTune()->rate`) |
    | `expo` | `uint8_t` | 1 | Resulting expo setting (`ezTune()->expo`) |
    | `snappiness` | `uint8_t` | 1 | Snappiness preference (`ezTune()->snappiness`) |
*   **Notes:** Requires `USE_EZ_TUNE`. Calls `ezTuneUpdate()` before sending.

### `MSP2_INAV_EZ_TUNE_SET` (0x2071 / 8305)

*   **Direction:** In
*   **Description:** Sets the EZ-Tune parameters and triggers an update.
*   **Payload:** (Matches `MSP2_INAV_EZ_TUNE` structure, potentially omitting `snappiness`)
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `enabled` | `uint8_t` | 1 | Sets enabled state |
    | `filterHz` | `uint16_t` | 2 | Sets filter frequency |
    | `axisRatio` | `uint8_t` | 1 | Sets axis ratio |
    | `response` | `uint8_t` | 1 | Sets response characteristic |
    | `damping` | `uint8_t` | 1 | Sets damping characteristic |
    | `stability` | `uint8_t` | 1 | Sets stability preference |
    | `aggressiveness` | `uint8_t` | 1 | Sets aggressiveness preference |
    | `rate` | `uint8_t` | 1 | Sets rate setting |
    | `expo` | `uint8_t` | 1 | Sets expo setting |
    | `snappiness` | `uint8_t` | 1 | (Optional) Sets snappiness preference |
*   **Notes:** Requires `USE_EZ_TUNE`. Expects 10 or 11 bytes. Calls `ezTuneUpdate()` after setting parameters.

### `MSP2_INAV_SELECT_MIXER_PROFILE` (0x2080 / 8320)

*   **Direction:** In
*   **Description:** Selects the active mixer profile and saves configuration.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `mixerProfileIndex` | `uint8_t` | 1 | Index of the mixer profile to activate (0-based) |
*   **Notes:** Expects 1 byte. Will fail if armed. Calls `setConfigMixerProfileAndWriteEEPROM()`. Only applicable if `MAX_MIXER_PROFILE_COUNT` > 1.

### `MSP2_ADSB_VEHICLE_LIST` (0x2090 / 8336)

*   **Direction:** Out
*   **Description:** Retrieves the list of currently tracked ADSB (Automatic Dependent Surveillance–Broadcast) vehicles.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `maxVehicles` | `uint8_t` | 1 | Maximum number of vehicles tracked (`MAX_ADSB_VEHICLES`). 0 if `USE_ADSB` disabled |
    | `callsignLength` | `uint8_t` | 1 | Maximum length of callsign string (`ADSB_CALL_SIGN_MAX_LENGTH`). 0 if `USE_ADSB` disabled |
    | `totalVehicleMsgs` | `uint32_t` | 4 | Total vehicle messages received (`getAdsbStatus()->vehiclesMessagesTotal`). 0 if `USE_ADSB` disabled |
    | `totalHeartbeatMsgs` | `uint32_t` | 4 | Total heartbeat messages received (`getAdsbStatus()->heartbeatMessagesTotal`). 0 if `USE_ADSB` disabled |
    | **Vehicle Data (Repeated `maxVehicles` times):** | | | |
    | `callsign` | `char[ADSB_CALL_SIGN_MAX_LENGTH]` | `ADSB_CALL_SIGN_MAX_LENGTH` | Vehicle callsign (padded with nulls) |
    | `icao` | `uint32_t` | 4 | ICAO 24-bit address |
    | `latitude` | `int32_t` | 4 | Latitude (1e7 deg) |
    | `longitude` | `int32_t` | 4 | Longitude (1e7 deg) |
    | `altitude` | `int32_t` | 4 | Altitude (cm) |
    | `heading` | `int16_t` | 2 | Heading (degrees) |
    | `tslc` | `uint8_t` | 1 | Time Since Last Communication (seconds) |
    | `emitterType` | `uint8_t` | 1 | Enum: Type of ADSB emitter |
    | `ttl` | `uint8_t` | 1 | Time-to-live counter for this entry |
*   **Notes:** Requires `USE_ADSB`.

### `MSP2_INAV_CUSTOM_OSD_ELEMENTS` (0x2100 / 8448)

*   **Direction:** Out
*   **Description:** Retrieves counts related to custom OSD elements defined by the programming framework.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `maxElements` | `uint8_t` | 1 | Maximum number of custom elements (`MAX_CUSTOM_ELEMENTS`) |
    | `maxTextLength` | `uint8_t` | 1 | Maximum length of the text part (`OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1`) |
    | `maxParts` | `uint8_t` | 1 | Maximum number of parts per element (`CUSTOM_ELEMENTS_PARTS`) |
*   **Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`.

### `MSP2_INAV_CUSTOM_OSD_ELEMENT` (0x2101 / 8449)

*   **Direction:** In/Out
*   **Description:** Gets the configuration of a single custom OSD element defined by the programming framework.
*   **Request Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `elementIndex` | `uint8_t` | 1 | Index of the custom element (0 to `MAX_CUSTOM_ELEMENTS - 1`) |
*   **Reply Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | **Parts Data (Repeated `CUSTOM_ELEMENTS_PARTS` times):** | | | |
    | `partType` | `uint8_t` | 1 | Enum (`customElementType_e`): Type of this part (Text, Variable, Symbol) |
    | `partValue` | `uint16_t` | 2 | Value/ID associated with this part (GVAR index, Symbol ID, etc.) |
    | **Visibility Data:** | | | |
    | `visibilityType` | `uint8_t` | 1 | Enum (`logicOperandType_e`): Type of visibility condition source |
    | `visibilityValue` | `uint16_t` | 2 | Value/ID of the visibility condition source (e.g., Logic Condition ID) |
    | **Text Data:** | | | |
    | `elementText` | `char[OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1]` | `OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1` | Static text part of the element (null padding likely) |
*   **Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`. See `osdCustomElement_t`.

### `MSP2_INAV_SET_CUSTOM_OSD_ELEMENTS` (0x2102 / 8450)

*   **Direction:** In
*   **Description:** Sets the configuration of a single custom OSD element defined by the programming framework.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `elementIndex` | `uint8_t` | 1 | Index of the custom element (0 to `MAX_CUSTOM_ELEMENTS - 1`) |
    | **Parts Data (Repeated `CUSTOM_ELEMENTS_PARTS` times):** | | | |
    | `partType` | `uint8_t` | 1 | Enum (`customElementType_e`): Type of this part |
    | `partValue` | `uint16_t` | 2 | Value/ID associated with this part |
    | **Visibility Data:** | | | |
    | `visibilityType` | `uint8_t` | 1 | Enum (`logicOperandType_e`): Type of visibility condition source |
    | `visibilityValue` | `uint16_t` | 2 | Value/ID of the visibility condition source |
    | **Text Data:** | | | |
    | `elementText` | `char[OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1]` | `OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1` | Static text part of the element |
*   **Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`. Expects `1 + (CUSTOM_ELEMENTS_PARTS * 3) + 3 + (OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1)` bytes. Returns error if index or part type is invalid. Null-terminates the text internally.

### `MSP2_INAV_SERVO_CONFIG` (0x2200 / 8704)

*   **Direction:** Out
*   **Description:** Retrieves the configuration parameters for all supported servos (min, max, middle, rate). Supersedes `MSP_SERVO_CONFIGURATIONS`.
*   **Payload:** Repeated `MAX_SUPPORTED_SERVOS` times:
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `min` | `uint16_t` | 2 | PWM | Minimum servo endpoint (`servoParams(i)->min`) |
    | `max` | `uint16_t` | 2 | PWM | Maximum servo endpoint (`servoParams(i)->max`) |
    | `middle` | `uint16_t` | 2 | PWM | Middle/Neutral servo position (`servoParams(i)->middle`) |
    | `rate` | `uint8_t` | 1 | % (-100 to 100) | Servo rate/scaling (`servoParams(i)->rate`) |

### `MSP2_INAV_SET_SERVO_CONFIG` (0x2201 / 8705)

*   **Direction:** In
*   **Description:** Sets the configuration parameters for a single servo. Supersedes `MSP_SET_SERVO_CONFIGURATION`.
*   **Payload:**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `servoIndex` | `uint8_t` | 1 | Index | Index of the servo to configure (0 to `MAX_SUPPORTED_SERVOS - 1`) |
    | `min` | `uint16_t` | 2 | PWM | Sets minimum servo endpoint |
    | `max` | `uint16_t` | 2 | PWM | Sets maximum servo endpoint |
    | `middle` | `uint16_t` | 2 | PWM | Sets middle/neutral servo position |
    | `rate` | `uint8_t` | 1 | % | Sets servo rate/scaling |
*   **Notes:** Expects 8 bytes. Returns error if index invalid. Calls `servoComputeScalingFactors()`.

### `MSP2_INAV_GEOZONE` (0x2210 / 8720)

*   **Direction:** In/Out
*   **Description:** Get configuration for a specific Geozone.
*   **Request Payload (Get):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `geozoneIndex` | `uint8_t` | 1 | Index of the geozone (0 to `MAX_GEOZONES_IN_CONFIG - 1`) |
*   **Reply Payload (Get):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `geozoneIndex` | `uint8_t` | 1 | Index requested |
    | `type` | `uint8_t` | 1 | Define (`GEOZONE_TYPE_EXCLUSIVE/INCLUSIVE`): Zone type (Inclusion/Exclusion) |
    | `shape` | `uint8_t` | 1 | Define (`GEOZONE_SHAPE_CIRCULAR/POLYGHON`): Zone shape (Polygon/Circular) |
    | `minAltitude` | `uint32_t` | 4 | Minimum allowed altitude within the zone (cm) |
    | `maxAltitude` | `uint32_t` | 4 | Maximum allowed altitude within the zone (cm) |
    | `isSeaLevelRef` | `uint8_t` | 1 | Boolean: 1 if altitudes are relative to sea level, 0 if relative to home |
    | `fenceAction` | `uint8_t` | 1 | Enum (`geozoneActionState_e`): Action to take upon boundary violation |
    | `vertexCount` | `uint8_t` | 1 | Number of vertices defined for this zone |
*   **Notes:** Requires `USE_GEOZONE`. Used by `mspFcGeozoneOutCommand`.

### `MSP2_INAV_SET_GEOZONE` (0x2211 / 8721)

*   **Direction:** In
*   **Description:** Sets the main configuration for a specific Geozone (type, shape, altitude, action). **This command resets (clears) all vertices associated with the zone.**
*   **Payload:**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `geozoneIndex` | `uint8_t` | 1 | Index of the geozone (0 to `MAX_GEOZONES_IN_CONFIG - 1`) |
    | `type` | `uint8_t` | 1 | Define (`GEOZONE_TYPE_EXCLUSIVE/INCLUSIVE`): Zone type (Inclusion/Exclusion) |
    | `shape` | `uint8_t` | 1 | Define (`GEOZONE_SHAPE_CIRCULAR/POLYGHON`): Zone shape (Polygon/Circular) |
    | `minAltitude` | `uint32_t` | 4 | Minimum allowed altitude (cm) |
    | `maxAltitude` | `uint32_t` | 4 | Maximum allowed altitude (cm) |
    | `isSeaLevelRef` | `uint8_t` | 1 | Boolean: Altitude reference |
    | `fenceAction` | `uint8_t` | 1 | Enum (`geozoneActionState_e`): Action to take upon boundary violation |
    | `vertexCount` | `uint8_t` | 1 | Number of vertices to be defined (used for validation later) |
*   **Notes:** Requires `USE_GEOZONE`. Expects 14 bytes. Returns error if index invalid. Calls `geozoneResetVertices()`. Vertices must be set subsequently using `MSP2_INAV_SET_GEOZONE_VERTEX`.

### `MSP2_INAV_GEOZONE_VERTEX` (0x2212 / 8722)

*   **Direction:** In/Out
*   **Description:** Get a specific vertex (or center+radius for circular zones) of a Geozone.
*   **Request Payload (Get):**
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `geozoneIndex` | `uint8_t` | 1 | Index of the geozone |
    | `vertexId` | `uint8_t` | 1 | Index of the vertex within the zone (0-based). For circles, 0 = center |
*   **Reply Payload (Get - Polygon):**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `geozoneIndex` | `uint8_t` | 1 | Index | Geozone index requested |
    | `vertexId` | `uint8_t` | 1 | Index | Vertex index requested |
    | `latitude` | `int32_t` | 4 | deg * 1e7 | Vertex latitude |
    | `longitude` | `int32_t` | 4 | deg * 1e7 | Vertex longitude |
*   **Reply Payload (Get - Circular):**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `geozoneIndex` | `uint8_t` | 1 | Index | Geozone index requested |
    | `vertexId` | `uint8_t` | 1 | Index | Vertex index requested (always 0 for center) |
    | `centerLatitude` | `int32_t` | 4 | deg * 1e7 | Center latitude |
    | `centerLongitude` | `int32_t` | 4 | deg * 1e7 | Center longitude |
    | `radius` | `uint32_t` | 4 | cm | Radius of the circular zone |
*   **Notes:** Requires `USE_GEOZONE`. Returns error if indexes are invalid or vertex doesn't exist. For circular zones, the radius is stored internally as the 'latitude' of the vertex with index 1.

### `MSP2_INAV_SET_GEOZONE_VERTEX` (0x2213 / 8723)

*   **Direction:** In
*   **Description:** Sets a specific vertex (or center+radius for circular zones) for a Geozone.
*   **Payload (Polygon):**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `geozoneIndex` | `uint8_t` | 1 | Index | Geozone index |
    | `vertexId` | `uint8_t` | 1 | Index | Vertex index (0-based) |
    | `latitude` | `int32_t` | 4 | deg * 1e7 | Vertex latitude |
    | `longitude` | `int32_t` | 4 | deg * 1e7 | Vertex longitude |
*   **Payload (Circular):**
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `geozoneIndex` | `uint8_t` | 1 | Index | Geozone index |
    | `vertexId` | `uint8_t` | 1 | Index | Vertex index (must be 0 for center) |
    | `centerLatitude` | `int32_t` | 4 | deg * 1e7 | Center latitude |
    | `centerLongitude` | `int32_t` | 4 | deg * 1e7 | Center longitude |
    | `radius` | `uint32_t` | 4 | cm | Radius of the circular zone |
*   **Notes:** Requires `USE_GEOZONE`. Expects 10 bytes (Polygon) or 14 bytes (Circular). Returns error if indexes invalid or if trying to set vertex beyond `vertexCount` defined in `MSP2_INAV_SET_GEOZONE`. Calls `geozoneSetVertex()`. For circular zones, sets center (vertex 0) and radius (vertex 1's latitude).

---

## MSPv2 Betaflight Commands (0x3000 Range)

### `MSP2_BETAFLIGHT_BIND` (0x3000 / 12288)

*   **Direction:** In
*   **Description:** Initiates the receiver binding procedure for supported serial protocols (CRSF, SRXL2).
*   **Payload:** None
*   **Notes:** Requires `rxConfig()->receiverType == RX_TYPE_SERIAL`. Requires `USE_SERIALRX_CRSF` or `USE_SERIALRX_SRXL2`. Calls `crsfBind()` or `srxl2Bind()` respectively. Returns error if receiver type or provider is not supported for binding.

---

## MSPv2 Sensor Input Commands (0x1F00 Range)

These commands are typically sent *to* the FC from external sensor modules connected via a serial port configured for MSP sensor input. They usually expect **no reply** (`MSP_RESULT_NO_REPLY`).

### `MSP2_SENSOR_RANGEFINDER` (0x1F01 / 7937)

*   **Direction:** In
*   **Description:** Provides rangefinder data (distance, quality) from an external MSP-based sensor.
*   **Payload:** (`mspSensorRangefinderDataMessage_t`)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `quality` | `uint8_t` | 1 | 0-255 | Quality of the measurement |
    | `distanceMm` | `int32_t` | 4 | mm | Measured distance. Negative value indicates out of range |
*   **Notes:** Requires `USE_RANGEFINDER_MSP`. Calls `mspRangefinderReceiveNewData()`.

### `MSP2_SENSOR_OPTIC_FLOW` (0x1F02 / 7938)

*   **Direction:** In
*   **Description:** Provides optical flow data (motion, quality) from an external MSP-based sensor.
*   **Payload:** (`mspSensorOpflowDataMessage_t`)
    | Field | C Type | Size (Bytes) | Description |
    |---|---|---|---|
    | `quality` | `uint8_t` | 1 | Quality of the measurement (0-255) |
    | `motionX` | `int32_t` | 4 | Raw integrated flow value X |
    | `motionY` | `int32_t` | 4 | Raw integrated flow value Y |
*   **Notes:** Requires `USE_OPFLOW_MSP`. Calls `mspOpflowReceiveNewData()`.

### `MSP2_SENSOR_GPS` (0x1F03 / 7939)

*   **Direction:** In
*   **Description:** Provides detailed GPS data from an external MSP-based GPS module.
*   **Payload:** (`mspSensorGpsDataMessage_t`)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `instance` | `uint8_t` | 1 | - | Sensor instance number (for multi-GPS) |
    | `gpsWeek` | `uint16_t` | 2 | - | GPS week number (0xFFFF if unavailable) |
    | `msTOW` | `uint32_t` | 4 | ms | Milliseconds Time of Week |
    | `fixType` | `uint8_t` | 1 | Enum | Enum `gpsFixType_e` Type of GPS fix |
    | `satellitesInView`| `uint8_t` | 1 | Count | Number of satellites used in solution |
    | `hPosAccuracy` | `uint16_t` | 2 | cm | Horizontal position accuracy estimate |
    | `vPosAccuracy` | `uint16_t` | 2 | cm | Vertical position accuracy estimate |
    | `hVelAccuracy` | `uint16_t` | 2 | cm/s | Horizontal velocity accuracy estimate |
    | `hdop` | `uint16_t` | 2 | HDOP * 100 | Horizontal Dilution of Precision |
    | `longitude` | `int32_t` | 4 | deg * 1e7 | Longitude |
    | `latitude` | `int32_t` | 4 | deg * 1e7 | Latitude |
    | `mslAltitude` | `int32_t` | 4 | cm | Altitude above Mean Sea Level |
    | `nedVelNorth` | `int32_t` | 4 | cm/s | North velocity (NED frame) |
    | `nedVelEast` | `int32_t` | 4 | cm/s | East velocity (NED frame) |
    | `nedVelDown` | `int32_t` | 4 | cm/s | Down velocity (NED frame) |
    | `groundCourse` | `uint16_t` | 2 | deg * 100 | Ground course (0-36000) |
    | `trueYaw` | `uint16_t` | 2 | deg * 100 | True heading/yaw (0-36000, 65535 if unavailable) |
    | `year` | `uint16_t` | 2 | - | Year (e.g., 2023) |
    | `month` | `uint8_t` | 1 | - | Month (1-12) |
    | `day` | `uint8_t` | 1 | - | Day of month (1-31) |
    | `hour` | `uint8_t` | 1 | - | Hour (0-23) |
    | `min` | `uint8_t` | 1 | - | Minute (0-59) |
    | `sec` | `uint8_t` | 1 | - | Second (0-59) |
*   **Notes:** Requires `USE_GPS_PROTO_MSP`. Calls `mspGPSReceiveNewData()`.

### `MSP2_SENSOR_COMPASS` (0x1F04 / 7940)

*   **Direction:** In
*   **Description:** Provides magnetometer data from an external MSP-based compass module.
*   **Payload:** (`mspSensorCompassDataMessage_t`)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `instance` | `uint8_t` | 1 | - | Sensor instance number |
    | `timeMs` | `uint32_t` | 4 | ms | Timestamp from the sensor |
    | `magX` | `int16_t` | 2 | mGauss | Front component reading |
    | `magY` | `int16_t` | 2 | mGauss | Right component reading |
    | `magZ` | `int16_t` | 2 | mGauss | Down component reading |
*   **Notes:** Requires `USE_MAG_MSP`. Calls `mspMagReceiveNewData()`.

### `MSP2_SENSOR_BAROMETER` (0x1F05 / 7941)

*   **Direction:** In
*   **Description:** Provides barometer data from an external MSP-based barometer module.
*   **Payload:** (`mspSensorBaroDataMessage_t`)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `instance` | `uint8_t` | 1 | - | Sensor instance number |
    | `timeMs` | `uint32_t` | 4 | ms | Timestamp from the sensor |
    | `pressurePa` | `float` | 4 | Pa | Absolute pressure |
    | `temp` | `int16_t` | 2 | 0.01 deg C | Temperature |
*   **Notes:** Requires `USE_BARO_MSP`. Calls `mspBaroReceiveNewData()`.

### `MSP2_SENSOR_AIRSPEED` (0x1F06 / 7942)

*   **Direction:** In
*   **Description:** Provides airspeed data from an external MSP-based pitot sensor module.
*   **Payload:** (`mspSensorAirspeedDataMessage_t`)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `instance` | `uint8_t` | 1 | - | Sensor instance number |
    | `timeMs` | `uint32_t` | 4 | ms | Timestamp from the sensor |
    | `diffPressurePa`| `float` | 4 | Pa | Differential pressure |
    | `temp` | `int16_t` | 2 | 0.01 deg C | Temperature |
*   **Notes:** Requires `USE_PITOT_MSP`. Calls `mspPitotmeterReceiveNewData()`.

### `MSP2_SENSOR_HEADTRACKER` (0x1F07 / 7943)

*   **Direction:** In
*   **Description:** Provides head tracker orientation data.
*   **Payload:** (Structure not defined in provided headers, but likely Roll, Pitch, Yaw angles)
    | Field | C Type | Size (Bytes) | Units | Description |
    |---|---|---|---|---|
    | `...` | Varies | Variable | Head tracker angles (e.g., int16 Roll, Pitch, Yaw in deci-degrees) |
*   **Notes:** Requires `USE_HEADTRACKER` and `USE_HEADTRACKER_MSP`. Calls `mspHeadTrackerReceiverNewData()`. Payload structure needs verification from `mspHeadTrackerReceiverNewData` implementation.
