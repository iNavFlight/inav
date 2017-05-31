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

#include "platform.h"

#ifdef USE_CLI

#include "blackbox/blackbox.h"

#include "build/debug.h"

#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "fc/config.h"
#include "fc/controlrate_profile.h"
#include "fc/rc_adjustments.h"
#include "fc/rc_controls.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/failsafe.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/pid.h"
#include "flight/servos.h"

#include "io/gimbal.h"
#include "io/gps.h"
#include "io/ledstrip.h"
#include "io/osd.h"
#include "io/serial.h"

#include "navigation/navigation.h"

#include "rx/rx.h"
#include "rx/spektrum.h"

#include "scheduler/scheduler.h"

#include "sensors/acceleration.h"
#include "sensors/barometer.h"
#include "sensors/battery.h"
#include "sensors/boardalignment.h"
#include "sensors/compass.h"
#include "sensors/diagnostics.h"
#include "sensors/gyro.h"
#include "sensors/pitotmeter.h"
#include "sensors/rangefinder.h"
#include "sensors/sensors.h"

#include "telemetry/frsky.h"
#include "telemetry/telemetry.h"


/* Sensor names (used in lookup tables for *_hardware settings and in status command output) */
// sync with accelerationSensor_e
static const char * const lookupTableAccHardware[] = { "NONE", "AUTO", "ADXL345", "MPU6050", "MMA845x", "BMA280", "LSM303DLHC", "MPU6000", "MPU6500", "MPU9250", "FAKE"};
#if (FLASH_SIZE > 64)
// sync with gyroSensor_e
static const char * const gyroNames[] = { "NONE", "AUTO", "MPU6050", "L3G4200D", "MPU3050", "L3GD20", "MPU6000", "MPU6500", "MPU9250", "FAKE"};
// sync with baroSensor_e
static const char * const lookupTableBaroHardware[] = { "NONE", "AUTO", "BMP085", "MS5611", "BMP280", "MS5607", "FAKE"};
// sync with magSensor_e
static const char * const lookupTableMagHardware[] = { "NONE", "AUTO", "HMC5883", "AK8975", "GPSMAG", "MAG3110", "AK8963", "IST8310", "FAKE"};
// sycn with rangefinderType_e
static const char * const lookupTableRangefinderHardware[] = { "NONE", "HCSR04", "SRF10"};
// sync with pitotSensor_e
static const char * const lookupTablePitotHardware[] = { "NONE", "AUTO", "MS4525", "ADC", "VIRTUAL", "FAKE"};

const char * const *sensorHardwareNames[] = {
        gyroNames,
        lookupTableAccHardware,
        lookupTableBaroHardware,
        lookupTableMagHardware,
        lookupTableRangefinderHardware,
        lookupTablePitotHardware
};
#endif

static const char * const lookupTableOffOn[] = {
    "OFF", "ON"
};

static const char * const lookupTableUnit[] = {
    "IMPERIAL", "METRIC"
};

static const char * const lookupTableAlignment[] = {
    "DEFAULT",
    "CW0",
    "CW90",
    "CW180",
    "CW270",
    "CW0FLIP",
    "CW90FLIP",
    "CW180FLIP",
    "CW270FLIP"
};

#ifdef GPS
static const char * const lookupTableGPSProvider[] = {
    "NMEA", "UBLOX", "I2C-NAV", "NAZA", "UBLOX7"
};

static const char * const lookupTableGPSSBASMode[] = {
    "AUTO", "EGNOS", "WAAS", "MSAS", "GAGAN", "NONE"
};

static const char * const lookupTableGpsModel[] = {
    "PEDESTRIAN", "AIR_1G", "AIR_4G"
};
#endif

static const char * const lookupTableCurrentSensor[] = {
    "NONE", "ADC", "VIRTUAL"
};

#ifdef USE_SERVOS
static const char * const lookupTableGimbalMode[] = {
    "NORMAL", "MIXTILT"
};
#endif

#ifdef BLACKBOX
static const char * const lookupTableBlackboxDevice[] = {
    "SERIAL", "SPIFLASH", "SDCARD"
};
#endif

#ifdef SERIAL_RX
static const char * const lookupTableSerialRX[] = {
    "SPEK1024",
    "SPEK2048",
    "SBUS",
    "SUMD",
    "SUMH",
    "XB-B",
    "XB-B-RJ01",
    "IBUS",
    "JETIEXBUS",
    "CRSF"
};
#endif

#ifdef USE_RX_SPI
// sync with rx_spi_protocol_e
static const char * const lookupTableRxSpi[] = {
    "V202_250K",
    "V202_1M",
    "SYMA_X",
    "SYMA_X5C",
    "CX10",
    "CX10A",
    "H8_3D",
    "INAV"
};
#endif

static const char * const lookupTableGyroLpf[] = {
    "256HZ",
    "188HZ",
    "98HZ",
    "42HZ",
    "20HZ",
    "10HZ"
};

static const char * const lookupTableFailsafeProcedure[] = {
    "SET-THR", "DROP", "RTH", "NONE"
};

#ifdef NAV
static const char * const lookupTableNavControlMode[] = {
    "ATTI", "CRUISE"
};

static const char * const lookupTableNavRthAltMode[] = {
    "CURRENT", "EXTRA", "FIXED", "MAX", "AT_LEAST"
};

static const char * const lookupTableNavResetAltitude[] = {
    "NEVER", "FIRST_ARM", "EACH_ARM"
};

#endif

static const char * const lookupTableAuxOperator[] = {
    "OR", "AND"
};

#ifdef OSD
static const char * const lookupTableOsdType[] = {
    "AUTO",
    "PAL",
    "NTSC"
};
#endif

static const char * const lookupTablePwmProtocol[] = {
    "STANDARD", "ONESHOT125", "ONESHOT42", "MULTISHOT", "BRUSHED"
};

#ifdef ASYNC_GYRO_PROCESSING
static const char * const lookupTableAsyncMode[] = {
    "NONE", "GYRO", "ALL"
};
#endif

static const char * const lookupTableDebug[DEBUG_COUNT] = {
    "NONE",
    "GYRO",
    "NOTCH",
    "NAV_LANDING",
    "FW_ALTITUDE"
};

#ifdef TELEMETRY_LTM
static const char * const lookupTableLTMRates[] = {
    "NORMAL", "MEDIUM", "SLOW"
};
#endif

const lookupTableEntry_t lookupTables[] = {
    { lookupTableOffOn, sizeof(lookupTableOffOn) / sizeof(char *) },
    { lookupTableUnit, sizeof(lookupTableUnit) / sizeof(char *) },
    { lookupTableAlignment, sizeof(lookupTableAlignment) / sizeof(char *) },
#ifdef GPS
    { lookupTableGPSProvider, sizeof(lookupTableGPSProvider) / sizeof(char *) },
    { lookupTableGPSSBASMode, sizeof(lookupTableGPSSBASMode) / sizeof(char *) },
    { lookupTableGpsModel, sizeof(lookupTableGpsModel) / sizeof(char *) },
#endif
#ifdef BLACKBOX
    { lookupTableBlackboxDevice, sizeof(lookupTableBlackboxDevice) / sizeof(char *) },
#endif
    { lookupTableCurrentSensor, sizeof(lookupTableCurrentSensor) / sizeof(char *) },
#ifdef USE_SERVOS
    { lookupTableGimbalMode, sizeof(lookupTableGimbalMode) / sizeof(char *) },
#endif
#ifdef SERIAL_RX
    { lookupTableSerialRX, sizeof(lookupTableSerialRX) / sizeof(char *) },
#endif
#ifdef USE_RX_SPI
    { lookupTableRxSpi, sizeof(lookupTableRxSpi) / sizeof(char *) },
#endif
    { lookupTableGyroLpf, sizeof(lookupTableGyroLpf) / sizeof(char *) },
    { lookupTableAccHardware, sizeof(lookupTableAccHardware) / sizeof(char *) },
#ifdef BARO
    { lookupTableBaroHardware, sizeof(lookupTableBaroHardware) / sizeof(char *) },
#endif
#ifdef MAG
    { lookupTableMagHardware, sizeof(lookupTableMagHardware) / sizeof(char *) },
#endif
#ifdef SONAR
    { lookupTableRangefinderHardware, sizeof(lookupTableRangefinderHardware) / sizeof(char *) },
#endif
#ifdef PITOT
    { lookupTablePitotHardware, sizeof(lookupTablePitotHardware) / sizeof(char *) },
#endif
#ifdef NAV
    { lookupTableNavControlMode, sizeof(lookupTableNavControlMode) / sizeof(char *) },
    { lookupTableNavRthAltMode, sizeof(lookupTableNavRthAltMode) / sizeof(char *) },
    { lookupTableNavResetAltitude, sizeof(lookupTableNavResetAltitude) / sizeof(char *) },
#endif
    { lookupTableAuxOperator, sizeof(lookupTableAuxOperator) / sizeof(char *) },
    { lookupTablePwmProtocol, sizeof(lookupTablePwmProtocol) / sizeof(char *) },
    { lookupTableFailsafeProcedure, sizeof(lookupTableFailsafeProcedure) / sizeof(char *) },
#ifdef ASYNC_GYRO_PROCESSING
    { lookupTableAsyncMode, sizeof(lookupTableAsyncMode) / sizeof(char *) },
#endif
#ifdef OSD
    { lookupTableOsdType, sizeof(lookupTableOsdType) / sizeof(char *) },
#endif
    { lookupTableDebug, sizeof(lookupTableDebug) / sizeof(char *) },
#ifdef TELEMETRY_LTM
    {lookupTableLTMRates, sizeof(lookupTableLTMRates) / sizeof(char *) },
#endif
};

const clivalue_t valueTable[] = {
// PG_GYRO_CONFIG
    { "looptime",                   VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 9000}, PG_GYRO_CONFIG, offsetof(gyroConfig_t, looptime) },
    { "gyro_sync",                  VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyroSync) },
    { "gyro_sync_denom",            VAR_UINT8  | MASTER_VALUE, .config.minmax = { 1,  32 }, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyroSyncDenominator) },
    { "align_gyro",                 VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ALIGNMENT }, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_align) },
    { "gyro_hardware_lpf",          VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GYRO_LPF }, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_lpf) },
    { "gyro_lpf_hz",                VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0, 200 }, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_soft_lpf_hz)  },
    { "moron_threshold",            VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  128 }, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyroMovementCalibrationThreshold) },
#ifdef USE_GYRO_NOTCH_1
    { "gyro_notch1_hz",             VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 500 }, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_soft_notch_hz_1)  },
    { "gyro_notch1_cutoff",         VAR_UINT16 | MASTER_VALUE, .config.minmax = {1, 500 }, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_soft_notch_cutoff_1)  },
#endif
#ifdef USE_GYRO_NOTCH_2
    { "gyro_notch2_hz",             VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 500 }, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_soft_notch_hz_2)  },
    { "gyro_notch2_cutoff",         VAR_UINT16 | MASTER_VALUE, .config.minmax = {1, 500 }, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_soft_notch_cutoff_2)  },
#endif
#ifdef USE_DUAL_GYRO
    { "gyro_to_use",                VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0, 1 }, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_to_use) },
#endif

// PG_ACCELEROMETER_CONFIG
    { "align_acc",                  VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ALIGNMENT }, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, acc_align) },
    { "acc_hardware",               VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ACC_HARDWARE }, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, acc_hardware) },
    { "acc_lpf_hz",                 VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 200 }, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, acc_lpf_hz) },
    { "acczero_x",                  VAR_INT16  | MASTER_VALUE, .config.minmax = { INT16_MIN,  INT16_MAX }, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, accZero.raw[X]) },
    { "acczero_y",                  VAR_INT16  | MASTER_VALUE, .config.minmax = { INT16_MIN,  INT16_MAX }, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, accZero.raw[Y]) },
    { "acczero_z",                  VAR_INT16  | MASTER_VALUE, .config.minmax = { INT16_MIN,  INT16_MAX }, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, accZero.raw[Z]) },
    { "accgain_x",                  VAR_INT16  | MASTER_VALUE, .config.minmax = { 1,  8192 }, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, accGain.raw[X]) },
    { "accgain_y",                  VAR_INT16  | MASTER_VALUE, .config.minmax = { 1,  8192 }, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, accGain.raw[Y]) },
    { "accgain_z",                  VAR_INT16  | MASTER_VALUE, .config.minmax = { 1,  8192 }, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, accGain.raw[Z]) },

// PG_RANGEFINDER_CONFIG
#ifdef SONAR
    { "rangefinder_hardware",       VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_RANGEFINDER_HARDWARE }, PG_RANGEFINDER_CONFIG, offsetof(rangefinderConfig_t, rangefinder_hardware) },
#endif

// PG_COMPASS_CONFIG
#ifdef MAG
    { "align_mag",                  VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ALIGNMENT }, PG_COMPASS_CONFIG, offsetof(compassConfig_t, mag_align) },
    { "mag_hardware",               VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_MAG_HARDWARE }, PG_COMPASS_CONFIG, offsetof(compassConfig_t, mag_hardware) },
    { "mag_declination",            VAR_INT16  | MASTER_VALUE, .config.minmax = { -18000,  18000 }, PG_COMPASS_CONFIG, offsetof(compassConfig_t, mag_declination) },
    { "magzero_x",                  VAR_INT16  | MASTER_VALUE, .config.minmax = { INT16_MIN,  INT16_MAX }, PG_COMPASS_CONFIG, offsetof(compassConfig_t, magZero.raw[X]) },
    { "magzero_y",                  VAR_INT16  | MASTER_VALUE, .config.minmax = { INT16_MIN,  INT16_MAX }, PG_COMPASS_CONFIG, offsetof(compassConfig_t, magZero.raw[Y]) },
    { "magzero_z",                  VAR_INT16  | MASTER_VALUE, .config.minmax = { INT16_MIN,  INT16_MAX }, PG_COMPASS_CONFIG, offsetof(compassConfig_t, magZero.raw[Z]) },
    { "mag_calibration_time",       VAR_UINT8  | MASTER_VALUE, .config.minmax = { 30,  120 }, PG_COMPASS_CONFIG, offsetof(compassConfig_t, magCalibrationTimeLimit) },
#endif

// PG_BAROMETER_CONFIG
#ifdef BARO
    { "baro_hardware",              VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_BARO_HARDWARE }, PG_BAROMETER_CONFIG, offsetof(barometerConfig_t, baro_hardware) },
    { "baro_use_median_filter",     VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_BAROMETER_CONFIG, offsetof(barometerConfig_t, use_median_filtering) },
#endif

// PG_PITOTMETER_CONFIG
#ifdef PITOT
    { "pitot_hardware",             VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_PITOT_HARDWARE }, PG_PITOTMETER_CONFIG, offsetof(pitotmeterConfig_t, pitot_hardware) },
    { "pitot_use_median_filter",    VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_PITOTMETER_CONFIG, offsetof(pitotmeterConfig_t, use_median_filtering) },
    { "pitot_noise_lpf",            VAR_FLOAT  | MASTER_VALUE, .config.minmax = { 0, 1 }, PG_PITOTMETER_CONFIG, offsetof(pitotmeterConfig_t, pitot_noise_lpf) },
    { "pitot_scale",                VAR_FLOAT  | MASTER_VALUE, .config.minmax = { 0, 100 }, PG_PITOTMETER_CONFIG, offsetof(pitotmeterConfig_t, pitot_scale) },
#endif

// PG_RX_CONFIG
    { "mid_rc",                     VAR_UINT16 | MASTER_VALUE, .config.minmax = { 1200,  1700 }, PG_RX_CONFIG, offsetof(rxConfig_t, midrc) },
    { "min_check",                  VAR_UINT16 | MASTER_VALUE, .config.minmax = { PWM_RANGE_ZERO,  PWM_RANGE_MAX }, PG_RX_CONFIG, offsetof(rxConfig_t, mincheck) },
    { "max_check",                  VAR_UINT16 | MASTER_VALUE, .config.minmax = { PWM_RANGE_ZERO,  PWM_RANGE_MAX }, PG_RX_CONFIG, offsetof(rxConfig_t, maxcheck) },
    { "rssi_channel",               VAR_INT8   | MASTER_VALUE, .config.minmax = { 0,  MAX_SUPPORTED_RC_CHANNEL_COUNT }, PG_RX_CONFIG, offsetof(rxConfig_t, rssi_channel) },
    { "rssi_scale",                 VAR_UINT8  | MASTER_VALUE, .config.minmax = { RSSI_SCALE_MIN,  RSSI_SCALE_MAX }, PG_RX_CONFIG, offsetof(rxConfig_t, rssi_scale) },
    { "rssi_invert",                VAR_INT8   | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_RX_CONFIG, offsetof(rxConfig_t, rssiInvert) },
    { "rc_smoothing",               VAR_INT8   | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_RX_CONFIG, offsetof(rxConfig_t, rcSmoothing) },
#ifdef SERIAL_RX
    { "serialrx_provider",          VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_SERIAL_RX }, PG_RX_CONFIG, offsetof(rxConfig_t, serialrx_provider) },
    { "sbus_inversion",             VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_RX_CONFIG, offsetof(rxConfig_t, sbus_inversion) },
#endif
#ifdef USE_RX_SPI
    { "rx_spi_protocol",            VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_RX_SPI }, PG_RX_CONFIG, offsetof(rxConfig_t, rx_spi_protocol) },
    { "rx_spi_id",                  VAR_UINT32 | MASTER_VALUE, .config.minmax = { 0, 0 }, PG_RX_CONFIG, offsetof(rxConfig_t, rx_spi_id) },
    { "rx_spi_rf_channel_count",    VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0, 8 }, PG_RX_CONFIG, offsetof(rxConfig_t, rx_spi_rf_channel_count) },
#endif
#ifdef SPEKTRUM_BIND
    { "spektrum_sat_bind",          VAR_UINT8  | MASTER_VALUE, .config.minmax = { SPEKTRUM_SAT_BIND_DISABLED,  SPEKTRUM_SAT_BIND_MAX}, PG_RX_CONFIG, offsetof(rxConfig_t, spektrum_sat_bind) },
#endif
    { "rx_min_usec",                VAR_UINT16 | MASTER_VALUE, .config.minmax = { PWM_PULSE_MIN,  PWM_PULSE_MAX }, PG_RX_CONFIG, offsetof(rxConfig_t, rx_min_usec) },
    { "rx_max_usec",                VAR_UINT16 | MASTER_VALUE, .config.minmax = { PWM_PULSE_MIN,  PWM_PULSE_MAX }, PG_RX_CONFIG, offsetof(rxConfig_t, rx_max_usec) },
#ifdef STM32F4
    { "serialrx_halfduplex",        VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_RX_CONFIG, offsetof(rxConfig_t, halfDuplex) },
#endif

// PG_BLACKBOX_CONFIG
#ifdef BLACKBOX
    { "blackbox_rate_num",          VAR_UINT8  | MASTER_VALUE, .config.minmax = { 1,  255 }, PG_BLACKBOX_CONFIG, offsetof(blackboxConfig_t, rate_num) },
    { "blackbox_rate_denom",        VAR_UINT8  | MASTER_VALUE, .config.minmax = { 1,  255 }, PG_BLACKBOX_CONFIG, offsetof(blackboxConfig_t, rate_denom) },
    { "blackbox_device",            VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_BLACKBOX_DEVICE }, PG_BLACKBOX_CONFIG, offsetof(blackboxConfig_t, device) },
#endif

// PG_MOTOR_CONFIG
    { "min_throttle",               VAR_UINT16 | MASTER_VALUE, .config.minmax = { PWM_RANGE_ZERO,  PWM_RANGE_MAX }, PG_MOTOR_CONFIG, offsetof(motorConfig_t, minthrottle) },
    { "max_throttle",               VAR_UINT16 | MASTER_VALUE, .config.minmax = { PWM_RANGE_ZERO,  PWM_RANGE_MAX }, PG_MOTOR_CONFIG, offsetof(motorConfig_t, maxthrottle) },
    { "min_command",                VAR_UINT16 | MASTER_VALUE, .config.minmax = { PWM_RANGE_ZERO,  PWM_RANGE_MAX }, PG_MOTOR_CONFIG, offsetof(motorConfig_t, mincommand) },
    { "motor_pwm_rate",             VAR_UINT16 | MASTER_VALUE, .config.minmax = { 50,  32000 }, PG_MOTOR_CONFIG, offsetof(motorConfig_t, motorPwmRate) },
    { "motor_pwm_protocol",         VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_MOTOR_PWM_PROTOCOL }, PG_MOTOR_CONFIG, offsetof(motorConfig_t, motorPwmProtocol) },

// PG_FAILSAFE_CONFIG
    { "failsafe_delay",             VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  200 }, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_delay) },
    { "failsafe_recovery_delay",    VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  200 }, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_recovery_delay) },
    { "failsafe_off_delay",         VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  200 }, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_off_delay) },
    { "failsafe_throttle",          VAR_UINT16 | MASTER_VALUE, .config.minmax = { 1000, 2000 }, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_throttle) },
    { "failsafe_throttle_low_delay",VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0,  300 }, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_throttle_low_delay) },
    { "failsafe_procedure",         VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_FAILSAFE_PROCEDURE }, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_procedure) },
    { "failsafe_stick_threshold",   VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0,  500 }, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_stick_motion_threshold) },
    { "failsafe_fw_roll_angle",     VAR_INT16 | MASTER_VALUE, .config.minmax = { -800,  800 }, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_fw_roll_angle) },
    { "failsafe_fw_pitch_angle",    VAR_INT16 | MASTER_VALUE, .config.minmax = { -800,  800 }, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_fw_pitch_angle) },
    { "failsafe_fw_yaw_rate",       VAR_INT16 | MASTER_VALUE, .config.minmax = { -1000,  1000 }, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_fw_yaw_rate) },

// PG_BOARDALIGNMENT_CONFIG
    { "align_board_roll",           VAR_INT16  | MASTER_VALUE, .config.minmax = { -1800,  3600 }, PG_BOARD_ALIGNMENT, offsetof(boardAlignment_t, rollDeciDegrees) },
    { "align_board_pitch",          VAR_INT16  | MASTER_VALUE, .config.minmax = { -1800,  3600 }, PG_BOARD_ALIGNMENT, offsetof(boardAlignment_t, pitchDeciDegrees) },
    { "align_board_yaw",            VAR_INT16  | MASTER_VALUE, .config.minmax = { -1800,  3600 }, PG_BOARD_ALIGNMENT, offsetof(boardAlignment_t, yawDeciDegrees) },

// PG_GIMBAL_CONFIG
#ifdef USE_SERVOS
    { "gimbal_mode",                VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GIMBAL_MODE }, PG_GIMBAL_CONFIG, offsetof(gimbalConfig_t, mode) },
#endif

// PG_BATTERY_CONFIG
    { "battery_capacity",           VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0,  20000 }, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, batteryCapacity) },
    { "vbat_scale",                 VAR_UINT8  | MASTER_VALUE, .config.minmax = { VBAT_SCALE_MIN,  VBAT_SCALE_MAX }, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, vbatscale) },
    { "vbat_max_cell_voltage",      VAR_UINT8  | MASTER_VALUE, .config.minmax = { 10,  50 }, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, vbatmaxcellvoltage) },
    { "vbat_min_cell_voltage",      VAR_UINT8  | MASTER_VALUE, .config.minmax = { 10,  50 }, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, vbatmincellvoltage) },
    { "vbat_warning_cell_voltage",  VAR_UINT8  | MASTER_VALUE, .config.minmax = { 10,  50 }, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, vbatwarningcellvoltage) },
    { "current_meter_scale",        VAR_INT16  | MASTER_VALUE, .config.minmax = { -10000,  10000 }, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, currentMeterScale) },
    { "current_meter_offset",       VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0,  3300 }, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, currentMeterOffset) },
    { "multiwii_current_meter_output", VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, multiwiiCurrentMeterOutput) },
    { "current_meter_type",         VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_CURRENT_SENSOR }, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, currentMeterType) },

// PG_MIXER_CONFIG
    { "yaw_motor_direction",        VAR_INT8   | MASTER_VALUE, .config.minmax = { -1,  1 }, PG_MIXER_CONFIG, offsetof(mixerConfig_t, yaw_motor_direction) },
    { "yaw_jump_prevention_limit",  VAR_UINT16 | MASTER_VALUE, .config.minmax = { YAW_JUMP_PREVENTION_LIMIT_LOW,  YAW_JUMP_PREVENTION_LIMIT_HIGH }, PG_MIXER_CONFIG, offsetof(mixerConfig_t, yaw_jump_prevention_limit) },

// PG_MOTOR_3D_CONFIG
    { "3d_deadband_low",            VAR_UINT16 | MASTER_VALUE, .config.minmax = { PWM_RANGE_ZERO,  PWM_RANGE_MAX }, PG_MOTOR_3D_CONFIG, offsetof(flight3DConfig_t, deadband3d_low) }, // FIXME upper limit should match code in the mixer, 1500 currently
    { "3d_deadband_high",           VAR_UINT16 | MASTER_VALUE, .config.minmax = { PWM_RANGE_ZERO,  PWM_RANGE_MAX }, PG_MOTOR_3D_CONFIG, offsetof(flight3DConfig_t, deadband3d_high) }, // FIXME lower limit should match code in the mixer, 1500 currently,
    { "3d_neutral",                 VAR_UINT16 | MASTER_VALUE, .config.minmax = { PWM_RANGE_ZERO,  PWM_RANGE_MAX }, PG_MOTOR_3D_CONFIG, offsetof(flight3DConfig_t, neutral3d) },

#ifdef USE_SERVOS
// PG_SERVO_CONFIG
    { "servo_center_pulse",         VAR_UINT16 | MASTER_VALUE, .config.minmax = { PWM_RANGE_ZERO,  PWM_RANGE_MAX }, PG_SERVO_CONFIG, offsetof(servoConfig_t, servoCenterPulse) },
    { "servo_pwm_rate",             VAR_UINT16 | MASTER_VALUE, .config.minmax = { 50,  498 }, PG_SERVO_CONFIG, offsetof(servoConfig_t, servoPwmRate) },
    { "servo_lpf_hz",               VAR_INT16  | MASTER_VALUE, .config.minmax = { 0,  400}, PG_SERVO_CONFIG, offsetof(servoConfig_t, servo_lowpass_freq) },
    { "flaperon_throw_offset",      VAR_INT16  | MASTER_VALUE, .config.minmax = { FLAPERON_THROW_MIN,  FLAPERON_THROW_MAX}, PG_SERVO_CONFIG, offsetof(servoConfig_t, flaperon_throw_offset) },
    { "tri_unarmed_servo",          VAR_INT8   | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_SERVO_CONFIG, offsetof(servoConfig_t, tri_unarmed_servo) },
#endif

// PG_CONTROLRATE_PROFILE
    { "rc_expo",                    VAR_UINT8  | CONTROL_RATE_VALUE, .config.minmax = { 0,  100 }, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, rcExpo8) },
    { "rc_yaw_expo",                VAR_UINT8  | CONTROL_RATE_VALUE, .config.minmax = { 0,  100 }, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, rcYawExpo8) },
    { "thr_mid",                    VAR_UINT8  | CONTROL_RATE_VALUE, .config.minmax = { 0,  100 }, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, thrMid8) },
    { "thr_expo",                   VAR_UINT8  | CONTROL_RATE_VALUE, .config.minmax = { 0,  100 }, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, thrExpo8) },

    // New rates are in dps/10. That means, Rate of 20 means 200dps of rotation speed on given axis.
    // Rate 180 (1800dps) is max. value gyro can measure reliably
    { "roll_rate",                  VAR_UINT8  | CONTROL_RATE_VALUE, .config.minmax = { CONTROL_RATE_CONFIG_ROLL_PITCH_RATE_MIN,  CONTROL_RATE_CONFIG_ROLL_PITCH_RATE_MAX }, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, rates[FD_ROLL]) },
    { "pitch_rate",                 VAR_UINT8  | CONTROL_RATE_VALUE, .config.minmax = { CONTROL_RATE_CONFIG_ROLL_PITCH_RATE_MIN,  CONTROL_RATE_CONFIG_ROLL_PITCH_RATE_MAX }, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, rates[FD_PITCH]) },
    { "yaw_rate",                   VAR_UINT8  | CONTROL_RATE_VALUE, .config.minmax = { CONTROL_RATE_CONFIG_YAW_RATE_MIN,  CONTROL_RATE_CONFIG_YAW_RATE_MAX }, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, rates[FD_YAW]) },

    { "tpa_rate",                   VAR_UINT8  | CONTROL_RATE_VALUE, .config.minmax = { 0,  CONTROL_RATE_CONFIG_TPA_MAX}, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, dynThrPID) },
    { "tpa_breakpoint",             VAR_UINT16 | CONTROL_RATE_VALUE, .config.minmax = { PWM_RANGE_MIN,  PWM_RANGE_MAX}, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, tpa_breakpoint) },

// PG_SERIAL_CONFIG
    { "reboot_character",           VAR_UINT8  | MASTER_VALUE, .config.minmax = { 48,  126 }, PG_SERIAL_CONFIG, offsetof(serialConfig_t, reboot_character) },

// PG_IMU_CONFIG
    { "imu_dcm_kp",                 VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = { UINT16_MAX }, PG_IMU_CONFIG, offsetof(imuConfig_t, dcm_kp_acc) },
    { "imu_dcm_ki",                 VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = { UINT16_MAX }, PG_IMU_CONFIG, offsetof(imuConfig_t, dcm_ki_acc) },
    { "imu_dcm_kp_mag",             VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = { UINT16_MAX }, PG_IMU_CONFIG, offsetof(imuConfig_t, dcm_kp_mag) },
    { "imu_dcm_ki_mag",             VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = { UINT16_MAX }, PG_IMU_CONFIG, offsetof(imuConfig_t, dcm_ki_mag) },
    { "small_angle",                VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  180 }, PG_IMU_CONFIG, offsetof(imuConfig_t, small_angle) },

// PG_ARMING_CONFIG
    { "fixed_wing_auto_arm",        VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_ARMING_CONFIG, offsetof(armingConfig_t, fixed_wing_auto_arm) },
    { "disarm_kill_switch",         VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_ARMING_CONFIG, offsetof(armingConfig_t, disarm_kill_switch) },
    { "auto_disarm_delay",          VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  60 }, PG_ARMING_CONFIG, offsetof(armingConfig_t, auto_disarm_delay) },

// PG_GPS_CONFIG
#ifdef GPS
    { "gps_provider",               VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GPS_PROVIDER }, PG_GPS_CONFIG, offsetof(gpsConfig_t, provider) },
    { "gps_sbas_mode",              VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GPS_SBAS_MODE }, PG_GPS_CONFIG, offsetof(gpsConfig_t, sbasMode) },
    { "gps_dyn_model",              VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GPS_DYN_MODEL }, PG_GPS_CONFIG, offsetof(gpsConfig_t, dynModel) },
    { "gps_auto_config",            VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_GPS_CONFIG, offsetof(gpsConfig_t, autoConfig) },
    { "gps_auto_baud",              VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_GPS_CONFIG, offsetof(gpsConfig_t, autoBaud) },
    { "gps_min_sats",               VAR_UINT8  | MASTER_VALUE, .config.minmax = { 5,  10}, PG_GPS_CONFIG, offsetof(gpsConfig_t, gpsMinSats) },
#endif

// PG_RC_CONTROLS_CONFIG
    { "deadband",                   VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  32 }, PG_RC_CONTROLS_CONFIG, offsetof(rcControlsConfig_t, deadband) },
    { "yaw_deadband",               VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  100 }, PG_RC_CONTROLS_CONFIG, offsetof(rcControlsConfig_t, yaw_deadband) },
    { "pos_hold_deadband",          VAR_UINT8  | MASTER_VALUE, .config.minmax = { 10,  250 }, PG_RC_CONTROLS_CONFIG, offsetof(rcControlsConfig_t, pos_hold_deadband) },
    { "alt_hold_deadband",          VAR_UINT8  | MASTER_VALUE, .config.minmax = { 10,  250 }, PG_RC_CONTROLS_CONFIG, offsetof(rcControlsConfig_t, alt_hold_deadband) },
    { "3d_deadband_throttle",       VAR_UINT16 | MASTER_VALUE, .config.minmax = { PWM_RANGE_ZERO,  PWM_RANGE_MAX }, PG_RC_CONTROLS_CONFIG, offsetof(rcControlsConfig_t, deadband3d_throttle) },

// PG_PID_PROFILE
//    { "default_rate_profile",       VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  MAX_CONTROL_RATE_PROFILE_COUNT - 1 }, PG_PID_CONFIG, offsetof(pidProfile_t, defaultRateProfileIndex) },
    { "mc_p_pitch",                 VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_PITCH].P) },
    { "mc_i_pitch",                 VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_PITCH].I) },
    { "mc_d_pitch",                 VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_PITCH].D) },
    { "mc_p_roll",                  VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_ROLL].P) },
    { "mc_i_roll",                  VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_ROLL].I) },
    { "mc_d_roll",                  VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_ROLL].D) },
    { "mc_p_yaw",                   VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_YAW].P) },
    { "mc_i_yaw",                   VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_YAW].I) },
    { "mc_d_yaw",                   VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_YAW].D) },

    { "mc_p_level",                 VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_LEVEL].P) },
    { "mc_i_level",                 VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  100 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_LEVEL].I) },
    { "mc_d_level",                 VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  100 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_LEVEL].D) },

    { "fw_p_pitch",                 VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_PITCH].P) },
    { "fw_i_pitch",                 VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_PITCH].I) },
    { "fw_ff_pitch",                VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_PITCH].D) },
    { "fw_p_roll",                  VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_ROLL].P) },
    { "fw_i_roll",                  VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_ROLL].I) },
    { "fw_ff_roll",                 VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_ROLL].D) },
    { "fw_p_yaw",                   VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_YAW].P) },
    { "fw_i_yaw",                   VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_YAW].I) },
    { "fw_ff_yaw",                  VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  200 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_YAW].D) },

    { "fw_p_level",                 VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_LEVEL].P) },
    { "fw_i_level",                 VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  100 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_LEVEL].I) },
    { "fw_d_level",                 VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  100 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_LEVEL].D) },

    { "max_angle_inclination_rll",  VAR_INT16  | PROFILE_VALUE, .config.minmax = { 100,  900 }, PG_PID_PROFILE, offsetof(pidProfile_t, max_angle_inclination[FD_ROLL]) },
    { "max_angle_inclination_pit",  VAR_INT16  | PROFILE_VALUE, .config.minmax = { 100,  900 }, PG_PID_PROFILE, offsetof(pidProfile_t, max_angle_inclination[FD_PITCH]) },

    { "dterm_lpf_hz",               VAR_UINT8  | PROFILE_VALUE, .config.minmax = {0, 200 }, PG_PID_PROFILE, offsetof(pidProfile_t, dterm_lpf_hz) },
    { "yaw_lpf_hz",                 VAR_UINT8  | PROFILE_VALUE, .config.minmax = {0, 200 }, PG_PID_PROFILE, offsetof(pidProfile_t, yaw_lpf_hz) },
    { "dterm_setpoint_weight",      VAR_FLOAT  | PROFILE_VALUE, .config.minmax = {0, 2 }, PG_PID_PROFILE, offsetof(pidProfile_t, dterm_setpoint_weight) },
#ifdef USE_SERVOS
    { "fw_iterm_throw_limit",       VAR_UINT16 | PROFILE_VALUE, .config.minmax = { FW_ITERM_THROW_LIMIT_MIN,  FW_ITERM_THROW_LIMIT_MAX}, PG_PID_PROFILE, offsetof(pidProfile_t, fixedWingItermThrowLimit) },
    { "fw_reference_airspeed",      VAR_FLOAT  | PROFILE_VALUE, .config.minmax = { 1,  5000}, PG_PID_PROFILE, offsetof(pidProfile_t, fixedWingReferenceAirspeed) },
    { "fw_turn_assist_yaw_gain",    VAR_FLOAT  | PROFILE_VALUE, .config.minmax = { 0,  2}, PG_PID_PROFILE, offsetof(pidProfile_t, fixedWingCoordinatedYawGain) },
#endif
#ifdef USE_DTERM_NOTCH
    { "dterm_notch_hz",             VAR_UINT16 | PROFILE_VALUE, .config.minmax = {0, 500 }, PG_PID_PROFILE, offsetof(pidProfile_t, dterm_soft_notch_hz) },
    { "dterm_notch_cutoff",         VAR_UINT16 | PROFILE_VALUE, .config.minmax = {1, 500 }, PG_PID_PROFILE, offsetof(pidProfile_t, dterm_soft_notch_cutoff) },
#endif

    { "pidsum_limit",               VAR_UINT16 | PROFILE_VALUE, .config.minmax = { PID_SUM_LIMIT_MIN,  PID_SUM_LIMIT_MAX }, PG_PID_PROFILE, offsetof(pidProfile_t, pidSumLimit) },

    { "yaw_p_limit",                VAR_UINT16 | PROFILE_VALUE, .config.minmax = { YAW_P_LIMIT_MIN,  YAW_P_LIMIT_MAX }, PG_PID_PROFILE, offsetof(pidProfile_t, yaw_p_limit) },

    { "iterm_ignore_threshold",     VAR_UINT16 | PROFILE_VALUE, .config.minmax = {15, 1000 }, PG_PID_PROFILE, offsetof(pidProfile_t, rollPitchItermIgnoreRate) },
    { "yaw_iterm_ignore_threshold", VAR_UINT16 | PROFILE_VALUE, .config.minmax = {15, 1000 }, PG_PID_PROFILE, offsetof(pidProfile_t, yawItermIgnoreRate) },

    { "rate_accel_limit_roll_pitch",VAR_UINT32 | PROFILE_VALUE | MODE_MAX, .config.max = { 500000 }, PG_PID_PROFILE, offsetof(pidProfile_t, axisAccelerationLimitRollPitch) },
    { "rate_accel_limit_yaw",       VAR_UINT32 | PROFILE_VALUE | MODE_MAX, .config.max = { 500000 }, PG_PID_PROFILE, offsetof(pidProfile_t, axisAccelerationLimitYaw) },

    { "heading_hold_rate_limit",    VAR_UINT8  | PROFILE_VALUE, .config.minmax = { HEADING_HOLD_RATE_LIMIT_MIN,  HEADING_HOLD_RATE_LIMIT_MAX }, PG_PID_PROFILE, offsetof(pidProfile_t, heading_hold_rate_limit) },

#ifdef NAV
    { "nav_mc_pos_z_p",             VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_POS_Z].P) },
    { "nav_mc_pos_z_i",             VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_POS_Z].I) },
    { "nav_mc_pos_z_d",             VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_POS_Z].D) },

    { "nav_mc_vel_z_p",             VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_Z].P) },
    { "nav_mc_vel_z_i",             VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_Z].I) },
    { "nav_mc_vel_z_d",             VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_Z].D) },

    { "nav_mc_pos_xy_p",            VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_POS_XY].P) },
    { "nav_mc_pos_xy_i",            VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_POS_XY].I) },
    { "nav_mc_pos_xy_d",            VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_POS_XY].D) },
    { "nav_mc_vel_xy_p",            VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_XY].P) },
    { "nav_mc_vel_xy_i",            VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_XY].I) },
    { "nav_mc_vel_xy_d",            VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_XY].D) },

    { "nav_fw_pos_z_p",             VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_POS_Z].P) },
    { "nav_fw_pos_z_i",             VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_POS_Z].I) },
    { "nav_fw_pos_z_d",             VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_POS_Z].D) },

    { "nav_fw_pos_xy_p",            VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_POS_XY].P) },
    { "nav_fw_pos_xy_i",            VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_POS_XY].I) },
    { "nav_fw_pos_xy_d",            VAR_UINT8  | PROFILE_VALUE, .config.minmax = { 0,  255 }, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_POS_XY].D) },

// PG_PID_AUTOTUNE_CONFIG
    { "fw_autotune_overshoot_time", VAR_UINT16 | MASTER_VALUE, .config.minmax = { 50,  500 }, PG_PID_AUTOTUNE_CONFIG, offsetof(pidAutotuneConfig_t, fw_overshoot_time) },
    { "fw_autotune_undershoot_time",VAR_UINT16 | MASTER_VALUE, .config.minmax = { 50,  500 }, PG_PID_AUTOTUNE_CONFIG, offsetof(pidAutotuneConfig_t, fw_undershoot_time) },
    { "fw_autotune_threshold",      VAR_UINT8 | MASTER_VALUE, .config.minmax = { 0,  100 }, PG_PID_AUTOTUNE_CONFIG, offsetof(pidAutotuneConfig_t, fw_max_rate_threshold) },
    { "fw_autotune_ff_to_p_gain",   VAR_UINT8 | MASTER_VALUE, .config.minmax = { 0,  100 }, PG_PID_AUTOTUNE_CONFIG, offsetof(pidAutotuneConfig_t, fw_ff_to_p_gain) },
    { "fw_autotune_ff_to_i_tc",     VAR_UINT16 | MASTER_VALUE, .config.minmax = { 100,  5000 }, PG_PID_AUTOTUNE_CONFIG, offsetof(pidAutotuneConfig_t, fw_ff_to_i_time_constant) },

// PG_POSITION_ESTIMATION_CONFIG
#if defined(NAV_AUTO_MAG_DECLINATION)
    { "inav_auto_mag_decl",         VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, automatic_mag_declination) },
#endif
    { "inav_gravity_cal_tolerance", VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  255 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, gravity_calibration_tolerance) },
    { "inav_use_gps_velned",        VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, use_gps_velned) },
    { "inav_gps_delay",             VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0,  500 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, gps_delay_ms) },
    { "inav_reset_altitude",        VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_NAV_RESET_ALTITUDE }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, reset_altitude_type) },

    { "inav_max_sonar_altitude",    VAR_UINT16  | MASTER_VALUE, .config.minmax = { 0,  1000 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, max_sonar_altitude) },

    { "inav_w_z_sonar_p",           VAR_FLOAT  | MASTER_VALUE, .config.minmax = { 0,  10 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_z_sonar_p) },
    { "inav_w_z_sonar_v",           VAR_FLOAT  | MASTER_VALUE, .config.minmax = { 0,  10 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_z_sonar_v) },
    { "inav_w_z_baro_p",            VAR_FLOAT  | MASTER_VALUE, .config.minmax = { 0,  10 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_z_baro_p) },
    { "inav_w_z_gps_p",             VAR_FLOAT  | MASTER_VALUE, .config.minmax = { 0,  10 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_z_gps_p) },
    { "inav_w_z_gps_v",             VAR_FLOAT  | MASTER_VALUE, .config.minmax = { 0,  10 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_z_gps_v) },
    { "inav_w_xy_gps_p",            VAR_FLOAT  | MASTER_VALUE, .config.minmax = { 0,  10 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_xy_gps_p) },
    { "inav_w_xy_gps_v",            VAR_FLOAT  | MASTER_VALUE, .config.minmax = { 0,  10 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_xy_gps_v) },
    { "inav_w_z_res_v",             VAR_FLOAT  | MASTER_VALUE, .config.minmax = { 0,  10 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_z_res_v) },
    { "inav_w_xy_res_v",            VAR_FLOAT  | MASTER_VALUE, .config.minmax = { 0,  10 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_xy_res_v) },
    { "inav_w_acc_bias",            VAR_FLOAT  | MASTER_VALUE, .config.minmax = { 0,  1 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_acc_bias) },

    { "inav_max_eph_epv",           VAR_FLOAT  | MASTER_VALUE, .config.minmax = { 0,  9999 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, max_eph_epv) },
    { "inav_baro_epv",              VAR_FLOAT  | MASTER_VALUE, .config.minmax = { 0,  9999 }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, baro_epv) },

// PG_NAV_CONFIG
    { "nav_disarm_on_landing",      VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.disarm_on_landing) },
    { "nav_use_midthr_for_althold", VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.use_thr_mid_for_althold) },
    { "nav_extra_arming_safety",    VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.extra_arming_safety) },
    { "nav_user_control_mode",      VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_NAV_USER_CTL_MODE }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.user_control_mode) },
    { "nav_position_timeout",       VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  10 }, PG_NAV_CONFIG, offsetof(navConfig_t, general.pos_failure_timeout) },
    { "nav_wp_radius",              VAR_UINT16 | MASTER_VALUE, .config.minmax = { 10,  10000 }, PG_NAV_CONFIG, offsetof(navConfig_t, general.waypoint_radius) },
    { "nav_wp_safe_distance",       VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = { 65000 }, PG_NAV_CONFIG, offsetof(navConfig_t, general.waypoint_safe_distance) },
    { "nav_auto_speed",             VAR_UINT16 | MASTER_VALUE, .config.minmax = { 10,  2000 }, PG_NAV_CONFIG, offsetof(navConfig_t, general.max_auto_speed) },
    { "nav_auto_climb_rate",        VAR_UINT16 | MASTER_VALUE, .config.minmax = { 10,  2000 }, PG_NAV_CONFIG, offsetof(navConfig_t, general.max_auto_climb_rate) },
    { "nav_manual_speed",           VAR_UINT16 | MASTER_VALUE, .config.minmax = { 10,  2000 }, PG_NAV_CONFIG, offsetof(navConfig_t, general.max_manual_speed) },
    { "nav_manual_climb_rate",      VAR_UINT16 | MASTER_VALUE, .config.minmax = { 10,  2000 }, PG_NAV_CONFIG, offsetof(navConfig_t, general.max_manual_climb_rate ) },
    { "nav_landing_speed",          VAR_UINT16 | MASTER_VALUE, .config.minmax = { 100,  2000 }, PG_NAV_CONFIG, offsetof(navConfig_t, general.land_descent_rate) },
    { "nav_land_slowdown_minalt",   VAR_UINT16 | MASTER_VALUE, .config.minmax = { 50,  1000 }, PG_NAV_CONFIG, offsetof(navConfig_t, general.land_slowdown_minalt) },
    { "nav_land_slowdown_maxalt",   VAR_UINT16 | MASTER_VALUE, .config.minmax = { 500,  4000 }, PG_NAV_CONFIG, offsetof(navConfig_t, general.land_slowdown_maxalt) },
    { "nav_emerg_landing_speed",    VAR_UINT16 | MASTER_VALUE, .config.minmax = { 100,  2000 }, PG_NAV_CONFIG, offsetof(navConfig_t, general.emerg_descent_rate) },
    { "nav_min_rth_distance",       VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0,  5000 }, PG_NAV_CONFIG, offsetof(navConfig_t, general.min_rth_distance) },
    { "nav_rth_climb_first",        VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.rth_climb_first) },
    { "nav_rth_climb_ignore_emerg", VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.rth_climb_ignore_emerg) },
    { "nav_rth_tail_first",         VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.rth_tail_first) },
    { "nav_rth_allow_landing",      VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.rth_allow_landing) },
    { "nav_rth_alt_mode",           VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_NAV_RTH_ALT_MODE }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.rth_alt_control_mode) },
    { "nav_rth_abort_threshold",    VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = { 65000 }, PG_NAV_CONFIG, offsetof(navConfig_t, general.rth_abort_threshold) },
    { "nav_rth_altitude",           VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = { 65000 }, PG_NAV_CONFIG, offsetof(navConfig_t, general.rth_altitude) },

    { "nav_mc_bank_angle",          VAR_UINT8  | MASTER_VALUE, .config.minmax = { 15,  45 }, PG_NAV_CONFIG, offsetof(navConfig_t, mc.max_bank_angle) },
    { "nav_mc_hover_thr",           VAR_UINT16 | MASTER_VALUE, .config.minmax = { 1000,  2000 }, PG_NAV_CONFIG, offsetof(navConfig_t, mc.hover_throttle) },
    { "nav_mc_auto_disarm_delay",   VAR_UINT16 | MASTER_VALUE, .config.minmax = { 100,  10000 }, PG_NAV_CONFIG, offsetof(navConfig_t, mc.auto_disarm_delay) },

    { "nav_fw_cruise_thr",          VAR_UINT16 | MASTER_VALUE, .config.minmax = { 1000,  2000 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.cruise_throttle) },
    { "nav_fw_min_thr",             VAR_UINT16 | MASTER_VALUE, .config.minmax = { 1000,  2000 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.min_throttle) },
    { "nav_fw_max_thr",             VAR_UINT16 | MASTER_VALUE, .config.minmax = { 1000,  2000 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.max_throttle) },
    { "nav_fw_bank_angle",          VAR_UINT8  | MASTER_VALUE, .config.minmax = { 5,  80 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.max_bank_angle) },
    { "nav_fw_climb_angle",         VAR_UINT8  | MASTER_VALUE, .config.minmax = { 5,  80 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.max_climb_angle) },
    { "nav_fw_dive_angle",          VAR_UINT8  | MASTER_VALUE, .config.minmax = { 5,  80 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.max_dive_angle) },
    { "nav_fw_pitch2thr",           VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  100 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.pitch_to_throttle) },
    { "nav_fw_loiter_radius",       VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0,  10000 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.loiter_radius) },

    { "nav_fw_launch_velocity",     VAR_UINT16 | MASTER_VALUE, .config.minmax = { 100,  10000 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_velocity_thresh) },
    { "nav_fw_launch_accel",        VAR_UINT16 | MASTER_VALUE, .config.minmax = { 1000,  20000 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_accel_thresh) },
    { "nav_fw_launch_max_angle",    VAR_UINT8  | MASTER_VALUE, .config.minmax = { 5,  180 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_max_angle) },
    { "nav_fw_launch_detect_time",  VAR_UINT16 | MASTER_VALUE, .config.minmax = { 10,  1000 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_time_thresh) },
    { "nav_fw_launch_thr",          VAR_UINT16 | MASTER_VALUE, .config.minmax = { 1000,  2000 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_throttle) },
    { "nav_fw_launch_idle_thr",     VAR_UINT16 | MASTER_VALUE, .config.minmax = { 1000,  2000 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_idle_throttle) },
    { "nav_fw_launch_motor_delay",  VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0,  5000 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_motor_timer) },
    { "nav_fw_launch_spinup_time",  VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0,  1000 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_motor_spinup_time) },
    { "nav_fw_launch_timeout",      VAR_UINT16 | MASTER_VALUE | MODE_MAX , .config.max = { 60000 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_timeout) },
    { "nav_fw_launch_climb_angle",  VAR_UINT8  | MASTER_VALUE, .config.minmax = { 5,  45 }, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_climb_angle) },
#endif

#ifdef TELEMETRY
// PG_TELEMETRY_CONFIG
    { "telemetry_switch",           VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, telemetry_switch) },
    { "telemetry_inversion",        VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, telemetry_inversion) },
    { "frsky_default_latitude",    VAR_FLOAT  | MASTER_VALUE, .config.minmax = { -90,  90 }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, gpsNoFixLatitude) },
    { "frsky_default_longitude",    VAR_FLOAT  | MASTER_VALUE, .config.minmax = { -180, 180 }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, gpsNoFixLongitude) },
    { "frsky_coordinates_format",   VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  FRSKY_FORMAT_NMEA }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, frsky_coordinate_format) },
    { "frsky_unit",                 VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_UNIT }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, frsky_unit) },
    { "frsky_vfas_precision",       VAR_UINT8  | MASTER_VALUE, .config.minmax = { FRSKY_VFAS_PRECISION_LOW,  FRSKY_VFAS_PRECISION_HIGH }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, frsky_vfas_precision) },
    { "frsky_vfas_cell_voltage",    VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, frsky_vfas_cell_voltage) },
    { "hott_alarm_sound_interval",  VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  120 }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, hottAlarmSoundInterval) },
#ifdef TELEMETRY_SMARTPORT
    { "smartport_uart_unidir",      VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, smartportUartUnidirectional) },
#endif
    { "ibus_telemetry_type",       VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  255 }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, ibusTelemetryType ) },
#ifdef TELEMETRY_LTM
    {"ltm_update_rate",            VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_LTM_UPDATE_RATE }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, ltmUpdateRate) },
#endif
#endif
#ifdef LED_STRIP
    { "ledstrip_visual_beeper",     VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_LED_STRIP_CONFIG, offsetof(ledStripConfig_t, ledstrip_visual_beeper) },
#endif
#ifdef OSD
    { "osd_video_system",           VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0, 2 }, PG_OSD_CONFIG, offsetof(osdConfig_t, video_system) },
    { "osd_row_shiftdown",          VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0, 1 }, PG_OSD_CONFIG, offsetof(osdConfig_t, row_shiftdown) },
    { "osd_units",                  VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_UNIT }, PG_OSD_CONFIG, offsetof(osdConfig_t, units) },

    { "osd_rssi_alarm",             VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0, 100 }, PG_OSD_CONFIG, offsetof(osdConfig_t, rssi_alarm) },
    { "osd_cap_alarm",              VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, 20000 }, PG_OSD_CONFIG, offsetof(osdConfig_t, cap_alarm) },
    { "osd_time_alarm",             VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, 60 }, PG_OSD_CONFIG, offsetof(osdConfig_t, time_alarm) },
    { "osd_alt_alarm",              VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, 10000 }, PG_OSD_CONFIG, offsetof(osdConfig_t, alt_alarm) },

    { "osd_main_voltage_pos",       VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_MAIN_BATT_VOLTAGE]) },
    { "osd_rssi_pos",               VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_RSSI_VALUE]) },
    { "osd_flytimer_pos",           VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_FLYTIME]) },
    { "osd_ontime_pos",             VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_ONTIME]) },
    { "osd_flymode_pos",            VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_FLYMODE]) },
    { "osd_throttle_pos",           VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_THROTTLE_POS]) },
    { "osd_vtx_channel_pos",        VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_VTX_CHANNEL]) },
    { "osd_crosshairs",             VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_CROSSHAIRS]) },
    { "osd_artificial_horizon",     VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_ARTIFICIAL_HORIZON]) },
    { "osd_current_draw_pos",       VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_CURRENT_DRAW]) },
    { "osd_mah_drawn_pos",          VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_MAH_DRAWN]) },
    { "osd_craft_name_pos",         VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_CRAFT_NAME]) },
    { "osd_gps_speed_pos",          VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_GPS_SPEED]) },
    { "osd_gps_sats_pos",           VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_GPS_SATS]) },
    { "osd_gps_lon",                VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_GPS_LON]) },
    { "osd_gps_lat",                VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_GPS_LAT]) },
    { "osd_home_dir",               VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_HOME_DIR]) },
    { "osd_home_dist",              VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_HOME_DIST]) },
    { "osd_altitude_pos",           VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_ALTITUDE]) },
    { "osd_vario",                  VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_VARIO]) },
    { "osd_vario_num",              VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_VARIO_NUM]) },
    { "osd_pid_roll_pos",           VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_ROLL_PIDS]) },
    { "osd_pid_pitch_pos",          VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_PITCH_PIDS]) },
    { "osd_pid_yaw_pos",            VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_YAW_PIDS]) },
    { "osd_power_pos",              VAR_UINT16 | MASTER_VALUE, .config.minmax = { 0, OSD_POS_MAX_CLI }, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_POWER]) },
#endif
// PG_SYSTEM_CONFIG
    { "i2c_overclock",              VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_SYSTEM_CONFIG, offsetof(systemConfig_t, i2c_overclock) },
    { "debug_mode",                 VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_DEBUG }, PG_SYSTEM_CONFIG, offsetof(systemConfig_t, debug_mode) },
#ifdef ASYNC_GYRO_PROCESSING
    { "acc_task_frequency",         VAR_UINT16 | MASTER_VALUE, .config.minmax = { ACC_TASK_FREQUENCY_MIN,  ACC_TASK_FREQUENCY_MAX }, PG_SYSTEM_CONFIG, offsetof(systemConfig_t, accTaskFrequency) },
    { "attitude_task_frequency",    VAR_UINT16 | MASTER_VALUE, .config.minmax = { ATTITUDE_TASK_FREQUENCY_MIN,  ATTITUDE_TASK_FREQUENCY_MAX }, PG_SYSTEM_CONFIG, offsetof(systemConfig_t, attitudeTaskFrequency) },
    { "async_mode",                 VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ASYNC_MODE }, PG_SYSTEM_CONFIG, offsetof(systemConfig_t, asyncMode) },
#endif
    { "throttle_tilt_comp_str",     VAR_UINT8  | MASTER_VALUE, .config.minmax = { 0,  100 }, PG_SYSTEM_CONFIG, offsetof(systemConfig_t, throttle_tilt_compensation_strength) },
    { "input_filtering_mode",       VAR_INT8   | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_SYSTEM_CONFIG, offsetof(systemConfig_t, pwmRxInputFilteringMode) },
    { "mode_range_logic_operator",  VAR_UINT8  | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_AUX_OPERATOR }, PG_MODE_ACTIVATION_OPERATOR_CONFIG, offsetof(modeActivationOperatorConfig_t, modeActivationOperator) },
};

const uint16_t valueTableEntryCount = ARRAYLEN(valueTable);

void settingsBuildCheck() {
    BUILD_BUG_ON(LOOKUP_TABLE_COUNT != ARRAYLEN(lookupTables));
}

#endif
