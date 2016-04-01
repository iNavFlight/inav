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
#include <string.h>
#include <stddef.h>

#include "platform.h"

#include "build_config.h"

#include "common/color.h"
#include "common/axis.h"
#include "common/maths.h"
#include "common/filter.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/sensor.h"
#include "drivers/accgyro.h"
#include "drivers/compass.h"
#include "drivers/system.h"
#include "drivers/serial.h"

#include "io/rate_profile.h"
#include "io/rc_controls.h"
#include "io/rc_adjustments.h"

#include "sensors/sensors.h"
#include "sensors/gyro.h"
#include "sensors/compass.h"
#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/boardalignment.h"
#include "sensors/battery.h"

#include "io/beeper.h"
#include "io/serial.h"
#include "io/gimbal.h"
#include "io/escservo.h"
#include "io/rc_controls.h"
#include "io/rc_curves.h"
#include "io/ledstrip.h"
#include "io/gps.h"

#include "rx/rx.h"

#include "blackbox/blackbox_io.h"
#include "blackbox/blackbox.h"

#include "telemetry/telemetry.h"
#include "telemetry/frsky.h"
#include "telemetry/hott.h"

#include "flight/mixer.h"
#include "flight/pid.h"
#include "flight/imu.h"
#include "flight/failsafe.h"
#include "flight/navigation_rewrite.h"

#include "config/config.h"
#include "config/config_eeprom.h"
#include "config/feature.h"
#include "config/profile.h"

#include "config/config_system.h"

#ifndef DEFAULT_RX_FEATURE
#define DEFAULT_RX_FEATURE FEATURE_RX_PARALLEL_PWM
#endif

#define BRUSHED_MOTORS_PWM_RATE 16000
#define BRUSHLESS_MOTORS_PWM_RATE 400


#define RESET_CONFIG(_type, _name, ...)                 \
    static const _type _name ## _reset = {               \
        __VA_ARGS__                                     \
    };                                                  \
    memcpy(_name, &_name ## _reset, sizeof(*_name));    \
    /**/

#define RESET_CONFIG_2(_type, _name, ...)                  \
    *_name = (_type) {                                    \
        __VA_ARGS__                                       \
    };                                                    \
    /**/


void resetPidProfile(pidProfile_t *pidProfile)
{
    RESET_CONFIG(pidProfile_t, pidProfile,
        .P8[PIDROLL] = 30,
        .I8[PIDROLL] = 20,
        .D8[PIDROLL] = 70,
        .P8[PIDPITCH] = 30,
        .I8[PIDPITCH] = 20,
        .D8[PIDPITCH] = 70,
        .P8[PIDYAW] = 100,
        .I8[PIDYAW] = 40,
        .D8[PIDYAW] = 0,
        .P8[PIDALT] = 50,       // NAV_POS_Z_P * 100
        .I8[PIDALT] = 0,        // not used
        .D8[PIDALT] = 0,        // not used
        .P8[PIDPOS] = 65,       // NAV_POS_XY_P * 100
        .I8[PIDPOS] = 120,      // posDecelerationTime * 100
        .D8[PIDPOS] = 10,       // posResponseExpo * 100
        .P8[PIDPOSR] = 180,     // NAV_VEL_XY_P * 100
        .I8[PIDPOSR] = 15,      // NAV_VEL_XY_I * 100
        .D8[PIDPOSR] = 100,     // NAV_VEL_XY_D * 100
        .P8[PIDNAVR] = 10,      // FW_NAV_P * 100
        .I8[PIDNAVR] = 5,       // FW_NAV_I * 100
        .D8[PIDNAVR] = 8,       // FW_NAV_D * 100
        .P8[PIDLEVEL] = 120,    // Self-level strength * 40 (4 * 40)
        .I8[PIDLEVEL] = 15,     // Self-leveing low-pass frequency (0 - disabled)
        .D8[PIDLEVEL] = 75,     // 75% horizon strength
        .P8[PIDMAG] = 60,
        .P8[PIDVEL] = 100,      // NAV_VEL_Z_P * 100
        .I8[PIDVEL] = 50,       // NAV_VEL_Z_I * 100
        .D8[PIDVEL] = 10,       // NAV_VEL_Z_D * 100

        .dterm_lpf_hz = 30,
        .yaw_lpf_hz = 30,
        .yaw_p_limit = YAW_P_LIMIT_MAX,
        .mag_hold_rate_limit = MAG_HOLD_RATE_LIMIT_DEFAULT,
        .max_angle_inclination[FD_ROLL] = 300,
        .max_angle_inclination[FD_PITCH] = 300
    );
}

#ifdef NAV
void resetNavConfig(navConfig_t * navConfig)
{
    RESET_CONFIG(navConfig_t, navConfig,
        // Navigation flags
        .flags = {
            .use_thr_mid_for_althold = 1,
            .extra_arming_safety = 1,
            .user_control_mode = NAV_GPS_ATTI,
            .rth_alt_control_style = NAV_RTH_AT_LEAST_ALT,
            .rth_tail_first = 0,
            .disarm_on_landing = 0,
        },

        // Inertial position estimator parameters
        .inav = {
            .gps_min_sats = 6,
            .gps_delay_ms = 200,
            .accz_unarmed_cal = 1,
            .use_gps_velned = 0,
            .w_z_baro_p = 0.35f,
            .w_z_gps_p = 0.2f,
            .w_z_gps_v = 0.2f,
            .w_xy_gps_p = 1.0f,
            .w_xy_gps_v = 2.0f,
            .w_z_res_v = 0.5f,
            .w_xy_res_v = 0.5f,
            .w_acc_bias = 0.01f,
            .max_eph_epv = 1000.0f,
            .baro_epv = 100.0f,
        },

        // General navigation parameters
        .pos_failure_timeout = 5,           // 5 sec
        .waypoint_radius = 100,             // 2m diameter
        .max_speed = 300,                   // 3 m/s = 10.8 km/h
        .max_manual_speed = 500,
        .max_manual_climb_rate = 200,
        .land_descent_rate = 200,           // 2 m/s
        .land_slowdown_minalt = 500,        // 5 meters of altitude
        .land_slowdown_maxalt = 2000,       // 20 meters of altitude
        .emerg_descent_rate = 500,          // 5 m/s
        .min_rth_distance = 500,            // If closer than 5m - land immediately
        .rth_altitude = 1000,               // 10m

        // MC-specific
        .mc_max_bank_angle = 30,            // 30 deg
        .mc_hover_throttle = 1500,
        .mc_min_fly_throttle = 1200,

        // Fixed wing
        .fw_max_bank_angle = 20,            // 30 deg
        .fw_max_climb_angle = 20,
        .fw_max_dive_angle = 15,
        .fw_cruise_throttle = 1400,
        .fw_max_throttle = 1700,
        .fw_min_throttle = 1200,
        .fw_pitch_to_throttle = 10,
        .fw_roll_to_pitch = 75,
        .fw_loiter_radius = 5000            // 50m
    );

#if defined(NAV_AUTO_MAG_DECLINATION)
    navConfig->inav.automatic_mag_declination = 1;
#endif
}

void validateNavConfig(navConfig_t * navConfig)
{
    // Make sure minAlt is not more than maxAlt, maxAlt cannot be set lower than 500.
    navConfig->land_slowdown_minalt = MIN(navConfig->land_slowdown_minalt, navConfig->land_slowdown_maxalt - 100);
}
#endif

void resetBarometerConfig(barometerConfig_t *barometerConfig)
{
    RESET_CONFIG(barometerConfig_t, barometerConfig,
        .use_median_filtering = 1
    );
}

void resetSensorAlignment(sensorAlignmentConfig_t *sensorAlignmentConfig)
{
    RESET_CONFIG(sensorAlignmentConfig_t, sensorAlignmentConfig,
        .gyro_align = ALIGN_DEFAULT,
        .acc_align = ALIGN_DEFAULT,
        .mag_align = ALIGN_DEFAULT,
    );
}

void resetEscAndServoConfig(escAndServoConfig_t *escAndServoConfig)
{
    RESET_CONFIG(escAndServoConfig_t, escAndServoConfig,
        .minthrottle = 1150,
        .maxthrottle = 1850,
        .mincommand = 1000,
        .servoCenterPulse = 1500,
    );
}

void resetMotor3DConfig(motor3DConfig_t *motor3DConfig)
{
    RESET_CONFIG(motor3DConfig_t, motor3DConfig,
        .deadband3d_low = 1406,
        .deadband3d_high = 1514,
        .neutral3d = 1460,
    );
}

void resetTelemetryConfig(void)
{
    RESET_CONFIG_2(telemetryConfig_t, &telemetryConfig,
        .telemetry_inversion = 0,
        .telemetry_switch = 0,
    )

#if defined(TELEMETRY_FRSKY)
    RESET_CONFIG_2(frskyTelemetryConfig_t, &frskyTelemetryConfig,
        .gpsNoFixLatitude = 0,
        .gpsNoFixLongitude = 0,
        .frsky_coordinate_format = FRSKY_FORMAT_DMS,
        .frsky_unit = FRSKY_UNIT_METRICS,
        .frsky_vfas_precision = 0,
    );
#endif

#if defined(TELEMETRY_HOTT)
    RESET_CONFIG_2(hottTelemetryConfig_t, &hottTelemetryConfig,
        .hottAlarmSoundInterval = 5,
    );
#endif
}

void resetBatteryConfig(batteryConfig_t *batteryConfig)
{
    RESET_CONFIG(batteryConfig_t, batteryConfig,
        .vbatscale = VBAT_SCALE_DEFAULT,
        .vbatresdivval = VBAT_RESDIVVAL_DEFAULT,
        .vbatresdivmultiplier = VBAT_RESDIVMULTIPLIER_DEFAULT,
        .vbatmaxcellvoltage = 43,
        .vbatmincellvoltage = 33,
        .vbatwarningcellvoltage = 35,
        .currentMeterOffset = 0,
        .currentMeterScale = 400, // for Allegro ACS758LCB-100U (40mV/A)
        .batteryCapacity = 0,
        .currentMeterType = CURRENT_SENSOR_ADC,
    );
}

#ifdef SWAP_SERIAL_PORT_0_AND_1_DEFAULTS
#define FIRST_PORT_INDEX 1
#define SECOND_PORT_INDEX 0
#else
#define FIRST_PORT_INDEX 0
#define SECOND_PORT_INDEX 1
#endif

void resetSerialConfig(serialConfig_t *serialConfig)
{
    memset(serialConfig, 0, sizeof(serialConfig_t));

    serialPortConfig_t portConfig_Reset = {
        .msp_baudrateIndex = BAUD_115200,
        .gps_baudrateIndex = BAUD_38400,
        .telemetry_baudrateIndex = BAUD_AUTO,
        .blackbox_baudrateIndex = BAUD_115200,
    };

    for (int i = 0; i < SERIAL_PORT_COUNT; i++) {
        memcpy(&serialConfig->portConfigs[i], &portConfig_Reset, sizeof(serialConfig->portConfigs[i]));
        serialConfig->portConfigs[i].identifier = serialPortIdentifiers[i];
    }

    serialConfig->portConfigs[0].functionMask = FUNCTION_MSP;

#ifdef CC3D
    // This allows MSP connection via USART & VCP so the board can be reconfigured.
    serialConfig->portConfigs[1].functionMask = FUNCTION_MSP;
#endif

    serialConfig->reboot_character = 'R';
}

void resetRcControlsConfig(rcControlsConfig_t *rcControlsConfig)
{
    RESET_CONFIG(rcControlsConfig_t, rcControlsConfig,
        .deadband = 0,
        .yaw_deadband = 0,
        .pos_hold_deadband = 20,
        .alt_hold_deadband = 40,
        .deadband3d_throttle = 50
    );
}

static void resetMixerConfig(mixerConfig_t *mixerConfig)
{
#ifdef USE_SERVOS
    RESET_CONFIG(mixerConfig_t, mixerConfig,
        .mixerMode = MIXER_QUADX,
        .yaw_motor_direction = 1,
        .yaw_jump_prevention_limit = 200,
        .tri_unarmed_servo = 1,
        .servo_lowpass_freq = 400,
        .servo_lowpass_enable = 0
    );
#else
    RESET_CONFIG(mixerConfig_t, mixerConfig,
        .mixerMode = MIXER_QUADX,
        .yaw_motor_direction = 1,
        .yaw_jump_prevention_limit = 200,
    );
#endif
}

static void resetIMUConfig(imuConfig_t * imuConfig)
{
    RESET_CONFIG(imuConfig_t, imuConfig,
        .small_angle = 25,

        .dcm_kp_acc = 2500,
        .dcm_ki_acc = 50,
        .dcm_kp_mag = 10000,
        .dcm_ki_mag = 0,

        .looptime = 2000,

        .gyroSync = 1,
        .gyroSyncDenominator = 2
    );
}

static void resetAccelerometerConfig(accConfig_t * accConfig)
{
    RESET_CONFIG(accConfig_t, accConfig,
        .accZero = {
            .values = {
                .pitch = 0,
                .roll = 0,
                .yaw = 0
            }
        },

        .accGain = {
            .values = {
                .pitch = 4096,
                .roll = 4096,
                .yaw = 4096
            }
        },

        .acc_soft_lpf_hz = 15,
    );
}

static void resetGyroConfig(gyroConfig_t * gyroConfig)
{
    RESET_CONFIG(gyroConfig_t, gyroConfig,
        .gyro_lpf = 3,                  // INV_FILTER_42HZ, In case of ST gyro, will default to 32Hz instead
        .gyro_soft_lpf_hz = 60,
        .gyroMovementCalibrationThreshold = 32
    );
}

uint16_t getCurrentMinthrottle(void)
{
    return escAndServoConfig.minthrottle;
}

// Default settings
STATIC_UNIT_TESTED void resetConf(void)
{
    int i;

    pgResetAll(MAX_PROFILE_COUNT);

    setProfile(0);
    pgActivateProfile(0);

    setControlRateProfile(0);

    featureClearAll();
#if defined(CJMCU) || defined(SPARKY) || defined(COLIBRI_RACE) || defined(MOTOLAB) || defined(LUX_RACE)
    featureSet(FEATURE_RX_PPM);
#endif

#ifdef BOARD_HAS_VOLTAGE_DIVIDER
    // only enable the VBAT feature by default if the board has a voltage divider otherwise
    // the user may see incorrect readings and unexpected issues with pin mappings may occur.
    featureSet(FEATURE_VBAT);
#endif

    featureSet(FEATURE_FAILSAFE);
    
    // beeper config
    beeperConfig.beeper_off_flags = 0;
    beeperConfig.prefered_beeper_off_flags = 0;

    resetIMUConfig(&imuConfig);

    resetGyroConfig(&gyroConfig);

    resetAccelerometerConfig(&accConfig);

    resetSensorAlignment(&sensorAlignmentConfig);

    resetBatteryConfig(&batteryConfig);

#ifdef TELEMETRY
    resetTelemetryConfig();
#endif

    rxConfig.serialrx_provider = 0;
    rxConfig.sbus_inversion = 1;
    rxConfig.spektrum_sat_bind = 0;
    rxConfig.midrc = 1500;
    rxConfig.mincheck = 1100;
    rxConfig.maxcheck = 1900;
    rxConfig.rx_min_usec = 885;          // any of first 4 channels below this value will trigger rx loss detection
    rxConfig.rx_max_usec = 2115;         // any of first 4 channels above this value will trigger rx loss detection

    rxConfig.rssi_channel = 0;
    rxConfig.rssi_scale = RSSI_SCALE_DEFAULT;
    rxConfig.rssi_ppm_invert = 0;
    rxConfig.rcSmoothing = 1;

    for (i = 0; i < MAX_SUPPORTED_RC_CHANNEL_COUNT; i++) {
        rxFailsafeChannelConfiguration_t *channelFailsafeConfiguration = &rxConfig.failsafe_channel_configurations[i];
        channelFailsafeConfiguration->mode = (i < NON_AUX_CHANNEL_COUNT) ? RX_FAILSAFE_MODE_AUTO : RX_FAILSAFE_MODE_HOLD;
        channelFailsafeConfiguration->step = (i == THROTTLE) ? CHANNEL_VALUE_TO_RXFAIL_STEP(rxConfig.rx_min_usec) : CHANNEL_VALUE_TO_RXFAIL_STEP(rxConfig.midrc);
    }

    resetAllRxChannelRangeConfigurations(rxConfig.channelRanges);

    parseRcChannels("AETR1234", &rxConfig);

    armingConfig.disarm_kill_switch = 1;
    armingConfig.auto_disarm_delay = 5;
    armingConfig.max_arm_angle = 25;

    // Motor/ESC/Servo
    resetEscAndServoConfig(&escAndServoConfig);
    resetMotor3DConfig(&motor3DConfig);

#ifdef BRUSHED_MOTORS
    escAndServoConfig.motor_pwm_rate = BRUSHED_MOTORS_PWM_RATE;
#else
    escAndServoConfig.motor_pwm_rate = BRUSHLESS_MOTORS_PWM_RATE;
#endif
    escAndServoConfig.servo_pwm_rate = 50;

#ifdef GPS
    // gps/nav stuff
    gpsConfig.provider = GPS_UBLOX;
    gpsConfig.sbasMode = SBAS_AUTO;
    gpsConfig.autoConfig = GPS_AUTOCONFIG_ON;
    gpsConfig.autoBaud = GPS_AUTOBAUD_ON;
    gpsConfig.dynModel = GPS_DYNMODEL_AIR_1G;
#endif

#ifdef NAV
    resetNavConfig(&navConfig);
#endif

    resetSerialConfig(&serialConfig);

    systemConfig.i2c_highspeed = 1;

    resetPidProfile(pidProfile);
#ifdef GTUNE
    resetGTuneConfig(gtuneConfig);
#endif

    resetControlRateConfig(&controlRateProfiles[0]);

    compassConfig->mag_declination = 0;

    resetBarometerConfig(&barometerConfig);

    // Radio

    resetRcControlsConfig(rcControlsConfig);

    throttleCorrectionConfig->throttle_tilt_compensation_strength = 0;      // 0-100, 0 - disabled

    // Failsafe Variables
    failsafeConfig.failsafe_delay = 10;              // 1sec
    failsafeConfig.failsafe_off_delay = 200;         // 20sec
    failsafeConfig.failsafe_throttle = 1000;         // default throttle off.
    failsafeConfig.failsafe_kill_switch = 0;         // default failsafe switch action is identical to rc link loss
    failsafeConfig.failsafe_throttle_low_delay = 100; // default throttle low delay for "just disarm" on failsafe condition
    failsafeConfig.failsafe_procedure = 0;           // default full failsafe procedure is 0: auto-landing

    resetMixerConfig(&mixerConfig);

#ifdef USE_SERVOS
    // servos
    for (i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
        servoProfile->servoConf[i].min = DEFAULT_SERVO_MIN;
        servoProfile->servoConf[i].max = DEFAULT_SERVO_MAX;
        servoProfile->servoConf[i].middle = DEFAULT_SERVO_MIDDLE;
        servoProfile->servoConf[i].rate = 100;
        servoProfile->servoConf[i].angleAtMin = DEFAULT_SERVO_MIN_ANGLE;
        servoProfile->servoConf[i].angleAtMax = DEFAULT_SERVO_MAX_ANGLE;
        servoProfile->servoConf[i].forwardFromChannel = CHANNEL_FORWARDING_DISABLED;
    }

    // gimbal
    gimbalConfig->mode = GIMBAL_MODE_NORMAL;
#endif

    // custom mixer. clear by defaults.
    for (i = 0; i < MAX_SUPPORTED_MOTORS; i++)
        customMotorMixer[i].throttle = 0.0f;

#ifdef LED_STRIP
    applyDefaultColors();
    applyDefaultLedStripConfig();
#endif

#ifdef TRANSPONDER
    static const uint8_t defaultTransponderData[6] = { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC }; // Note, this is NOT a valid transponder code, it's just for testing production hardware

    memcpy(&transponderConfig.data, &defaultTransponderData, sizeof(defaultTransponderData));
#endif

#ifdef BLACKBOX
#ifdef ENABLE_BLACKBOX_LOGGING_ON_SPIFLASH_BY_DEFAULT
    featureSet(FEATURE_BLACKBOX);
    blackboxConfig.device = BLACKBOX_DEVICE_FLASH;
#elif defined(ENABLE_BLACKBOX_LOGGING_ON_SDCARD_BY_DEFAULT)
    featureSet(FEATURE_BLACKBOX);
    blackboxConfig.device = BLACKBOX_DEVICE_SDCARD;
#else
    blackboxConfig.device = BLACKBOX_DEVICE_SERIAL;
#endif

    blackboxConfig.rate_num = 1;
    blackboxConfig.rate_denom = 1;
#endif

    // alternative defaults settings for COLIBRI RACE targets
#if defined(COLIBRI_RACE)
    imuConfig.looptime = 1000;

    parseRcChannels("TAER1234", &rxConfig);

    featureSet(FEATURE_ONESHOT125);
    featureSet(FEATURE_VBAT);
    featureSet(FEATURE_LED_STRIP);
    featureSet(FEATURE_FAILSAFE);
#endif

    // alternative defaults settings for ALIENWIIF1 and ALIENWIIF3 targets
#ifdef ALIENWII32
    featureSet(FEATURE_RX_SERIAL);
    featureSet(FEATURE_MOTOR_STOP);
#ifdef ALIENWIIF3
    serialConfig.portConfigs[2].functionMask = FUNCTION_RX_SERIAL;
    batteryConfig.vbatscale = 20;
#else
    serialConfig.portConfigs[1].functionMask = FUNCTION_RX_SERIAL;
#endif
    rxConfig.serialrx_provider = 1;
    rxConfig.spektrum_sat_bind = 5;
    escAndServoConfig.minthrottle = 1000;
    escAndServoConfig.maxthrottle = 2000;
    escAndServoConfig.motor_pwm_rate = 32000;
    imuConfig.looptime = 2000;
    pidProfile->P8[PIDROLL] = 36;
    pidProfile->P8[PIDPITCH] = 36;
    failsafeConfig.failsafe_delay = 2;
    failsafeConfig.failsafe_off_delay = 0;
    currentControlRateProfile->rcRate8 = 130;
    currentControlRateProfile->rates[ROLL] = 20;
    currentControlRateProfile->rates[PITCH] = 20;
    currentControlRateProfile->rates[YAW] = 100;
    parseRcChannels("TAER1234", &rxConfig);

    //  { 1.0f, -0.414178f,  1.0f, -1.0f },          // REAR_R
    customMotorMixer[0].throttle = 1.0f;
    customMotorMixer[0].roll = -0.414178f;
    customMotorMixer[0].pitch = 1.0f;
    customMotorMixer[0].yaw = -1.0f;

    //  { 1.0f, -0.414178f, -1.0f,  1.0f },          // FRONT_R
    customMotorMixer[1].throttle = 1.0f;
    customMotorMixer[1].roll = -0.414178f;
    customMotorMixer[1].pitch = -1.0f;
    customMotorMixer[1].yaw = 1.0f;

    //  { 1.0f,  0.414178f,  1.0f,  1.0f },          // REAR_L
    customMotorMixer[2].throttle = 1.0f;
    customMotorMixer[2].roll = 0.414178f;
    customMotorMixer[2].pitch = 1.0f;
    customMotorMixer[2].yaw = 1.0f;

    //  { 1.0f,  0.414178f, -1.0f, -1.0f },          // FRONT_L
    customMotorMixer[3].throttle = 1.0f;
    customMotorMixer[3].roll = 0.414178f;
    customMotorMixer[3].pitch = -1.0f;
    customMotorMixer[3].yaw = -1.0f;

    //  { 1.0f, -1.0f, -0.414178f, -1.0f },          // MIDFRONT_R
    customMotorMixer[4].throttle = 1.0f;
    customMotorMixer[4].roll = -1.0f;
    customMotorMixer[4].pitch = -0.414178f;
    customMotorMixer[4].yaw = -1.0f;

    //  { 1.0f,  1.0f, -0.414178f,  1.0f },          // MIDFRONT_L
    customMotorMixer[5].throttle = 1.0f;
    customMotorMixer[5].roll = 1.0f;
    customMotorMixer[5].pitch = -0.414178f;
    customMotorMixer[5].yaw = 1.0f;

    //  { 1.0f, -1.0f,  0.414178f,  1.0f },          // MIDREAR_R
    customMotorMixer[6].throttle = 1.0f;
    customMotorMixer[6].roll = -1.0f;
    customMotorMixer[6].pitch = 0.414178f;
    customMotorMixer[6].yaw = 1.0f;

    //  { 1.0f,  1.0f,  0.414178f, -1.0f },          // MIDREAR_L
    customMotorMixer[7].throttle = 1.0f;
    customMotorMixer[7].roll = 1.0f;
    customMotorMixer[7].pitch = 0.414178f;
    customMotorMixer[7].yaw = -1.0f;
#endif

    // copy first profile into remaining profile
    PG_FOREACH_PROFILE(reg) {
        for (int i = 1; i < MAX_PROFILE_COUNT; i++) {
            memcpy(reg->address + i * pgSize(reg), reg->address, pgSize(reg));
        }
    }

    // FIXME implement differently

    // copy first control rate config into remaining profile
    for (i = 1; i < MAX_CONTROL_RATE_PROFILE_COUNT; i++) {
        memcpy(&controlRateProfiles[i], &controlRateProfiles[0], sizeof(controlRateConfig_t));
    }

    // TODO
    for (i = 1; i < MAX_PROFILE_COUNT; i++) {
        configureRateProfileSelection(i, i % MAX_CONTROL_RATE_PROFILE_COUNT);
    }
}

void activateConfig(void)
{
    static imuRuntimeConfig_t imuRuntimeConfig;

    activateControlRateConfig();

    resetAdjustmentStates();

    useRcControlsConfig(
            modeActivationProfile->modeActivationConditions
    );

    useFailsafeConfig();

    mixerUseConfigs(
#ifdef USE_SERVOS
            servoProfile->servoConf
#endif
    );

    imuRuntimeConfig.dcm_kp_acc = imuConfig.dcm_kp_acc / 10000.0f;
    imuRuntimeConfig.dcm_ki_acc = imuConfig.dcm_ki_acc / 10000.0f;
    imuRuntimeConfig.dcm_kp_mag = imuConfig.dcm_kp_mag / 10000.0f;
    imuRuntimeConfig.dcm_ki_mag = imuConfig.dcm_ki_mag / 10000.0f;

    recalculateMagneticDeclination();

    imuConfigure(&imuRuntimeConfig);

#ifdef NAV
    navigationUsePIDs();
#endif
}

void validateAndFixConfig(void)
{
    if (!(featureConfigured(FEATURE_RX_PARALLEL_PWM) || featureConfigured(FEATURE_RX_PPM) || featureConfigured(FEATURE_RX_SERIAL) || featureConfigured(FEATURE_RX_MSP))) {
        featureSet(FEATURE_RX_PARALLEL_PWM); // Consider changing the default to PPM
    }

    if (featureConfigured(FEATURE_RX_PPM)) {
        featureClear(FEATURE_RX_PARALLEL_PWM);
    }

    if (featureConfigured(FEATURE_RX_MSP)) {
        featureClear(FEATURE_RX_SERIAL);
        featureClear(FEATURE_RX_PARALLEL_PWM);
        featureClear(FEATURE_RX_PPM);
    }

    if (featureConfigured(FEATURE_RX_SERIAL)) {
        featureClear(FEATURE_RX_PARALLEL_PWM);
        featureClear(FEATURE_RX_PPM);
    }

#if defined(NAV)
    // Ensure sane values of navConfig settings
    validateNavConfig(&navConfig);
#endif

    if (featureConfigured(FEATURE_RX_PARALLEL_PWM)) {
#if defined(STM32F10X)
        // rssi adc needs the same ports
        featureClear(FEATURE_RSSI_ADC);
        // current meter needs the same ports
        if (batteryConfig.currentMeterType == CURRENT_SENSOR_ADC) {
            featureClear(FEATURE_CURRENT_METER);
        }
#endif

#if defined(STM32F10X) || defined(CHEBUZZ) || defined(STM32F3DISCOVERY)
        // led strip needs the same ports
        featureClear(FEATURE_LED_STRIP);
#endif

        // software serial needs free PWM ports
        featureClear(FEATURE_SOFTSERIAL);
    }

#ifdef STM32F10X
    // avoid overloading the CPU on F1 targets when using gyro sync and GPS.
    if (imuConfig.gyroSync && imuConfig.gyroSyncDenominator < 2 && featureConfigured(FEATURE_GPS)) {
        imuConfig.gyroSyncDenominator = 2;
    }

    // avoid overloading the CPU when looptime < 2000 and GPS
    if (imuConfig.looptime && featureConfigured(FEATURE_GPS)) {
        imuConfig.looptime = 2000;
    }
#endif

#if defined(LED_STRIP) && (defined(USE_SOFTSERIAL1) || defined(USE_SOFTSERIAL2))
    if (featureConfigured(FEATURE_SOFTSERIAL) && (
            0
#ifdef USE_SOFTSERIAL1
            || (LED_STRIP_TIMER == SOFTSERIAL_1_TIMER)
#endif
#ifdef USE_SOFTSERIAL2
            || (LED_STRIP_TIMER == SOFTSERIAL_2_TIMER)
#endif
    )) {
        // led strip needs the same timer as softserial
        featureClear(FEATURE_LED_STRIP);
    }
#endif

#if defined(NAZE) && defined(SONAR)
    if (featureConfigured(FEATURE_RX_PARALLEL_PWM) && featureConfigured(FEATURE_SONAR) && featureConfigured(FEATURE_CURRENT_METER) && batteryConfig.currentMeterType == CURRENT_SENSOR_ADC) {
        featureClear(FEATURE_CURRENT_METER);
    }
#endif

#ifdef STM32F303xC
    // hardware supports serial port inversion, make users life easier for those that want to connect SBus RX's
#ifdef TELEMETRY
    telemetryConfig.telemetry_inversion = 1;
#endif
#endif

#if defined(CC3D) && defined(DISPLAY) && defined(USE_USART3)
    if (doesConfigurationUsePort(SERIAL_PORT_USART3) && feature(FEATURE_DISPLAY)) {
        featureClear(FEATURE_DISPLAY);
    }
#endif

#ifdef STM32F303xC
    // hardware supports serial port inversion, make users life easier for those that want to connect SBus RX's
    masterConfig.telemetryConfig.telemetry_inversion = 1;
#endif

#if defined(CC3D) && defined(SONAR) && defined(USE_SOFTSERIAL1)
    if (feature(FEATURE_SONAR) && feature(FEATURE_SOFTSERIAL)) {
        featureClear(FEATURE_SONAR);
    }
#endif

#if defined(COLIBRI_RACE)
    serialConfig.portConfigs[0].functionMask = FUNCTION_MSP;
    if(featureConfigured(FEATURE_RX_SERIAL)) {
        serialConfig.portConfigs[2].functionMask = FUNCTION_RX_SERIAL;
    }
#endif

    if (!isSerialConfigValid(&serialConfig)) {
        resetSerialConfig(&serialConfig);
    }

    /*
     * If provided predefined mixer setup is disabled, fallback to default one
     */
     if (!isMixerEnabled(mixerConfig.mixerMode)) {
         mixerConfig.mixerMode = DEFAULT_MIXER;
     }
}

void applyAndSaveBoardAlignmentDelta(int16_t roll, int16_t pitch)
{
    updateBoardAlignment(roll, pitch);

    saveConfigAndNotify();
}

void readEEPROM(void)
{
    suspendRxSignal();

    // Sanity check
    // Read flash
    if (!scanEEPROM(true)) {
        failureMode(FAILURE_INVALID_EEPROM_CONTENTS);
    }

    pgActivateProfile(getCurrentProfile());

    if (rateProfileSelection->defaultRateProfileIndex > MAX_CONTROL_RATE_PROFILE_COUNT - 1) // sanity check
        rateProfileSelection->defaultRateProfileIndex = 0;

    setControlRateProfile(rateProfileSelection->defaultRateProfileIndex);

    validateAndFixConfig();
    activateConfig();

    resumeRxSignal();
}

void writeEEPROM(void)
{
    suspendRxSignal();

    writeConfigToEEPROM();

    resumeRxSignal();
}

void ensureEEPROMContainsValidData(void)
{
    if (isEEPROMContentValid()) {
        return;
    }

    resetEEPROM();
}

void resetEEPROM(void)
{
    resetConf();
    writeEEPROM();
}

void saveConfigAndNotify(void)
{
    writeEEPROM();
    readEEPROM();
    beeperConfirmationBeeps(1);
}

void changeProfile(uint8_t profileIndex)
{
    setProfile(profileIndex);
    writeEEPROM();
    readEEPROM();
}

void handleOneshotFeatureChangeOnRestart(void)
{
    // Shutdown PWM on all motors prior to soft restart
    StopPwmAllMotors();
    delay(50);
    // Apply additional delay when OneShot125 feature changed from on to off state
    if (feature(FEATURE_ONESHOT125) && !featureConfigured(FEATURE_ONESHOT125)) {
        delay(ONESHOT_FEATURE_CHANGED_DELAY_ON_BOOT_MS);
    }
}
