static const char *words[] = {
	NULL,
	"3d",
	"abort",
	"acc",
	"accel",
	"accgain",
	"acczero",
	"adc",
	"air",
	"airspeed",
	"alarm",
	"align",
	"allow",
	"alt",
	"althold",
	"altitude",
	"angle",
	"arm",
	"arming",
	"artificial",
	"assist",
	"async",
	"attitude",
	"auto",
	"autotune",
	"bank",
	"baro",
	"battery",
	"baud",
	"beeper",
	"bias",
	"bind",
	"blackbox",
	"board",
	"breakpoint",
	"cal",
	"calibration",
	"cap",
	"capacity",
	"cell",
	"center",
	"channel",
	"character",
	"check",
	"climb",
	"command",
	"comp",
	"config",
	"control",
	"coordinates",
	"count",
	"craft",
	"crosshairs",
	"cruise",
	"current",
	"cutoff",
	"d",
	"dcm",
	"deadband",
	"debug",
	"decl",
	"declination",
	"default",
	"delay",
	"denom",
	"detect",
	"device",
	"dir",
	"direction",
	"disarm",
	"dist",
	"distance",
	"dive",
	"draw",
	"drawn",
	"dterm",
	"dyn",
	"eleres",
	"emerg",
	"en",
	"eph",
	"epv",
	"expo",
	"extra",
	"failsafe",
	"ff",
	"filter",
	"filtering",
	"first",
	"fixed",
	"flaperon",
	"flymode",
	"flytimer",
	"for",
	"format",
	"freq",
	"frequency",
	"frsky",
	"fw",
	"gain",
	"gimbal",
	"gps",
	"gravity",
	"gyro",
	"halfduplex",
	"hardware",
	"heading",
	"high",
	"hold",
	"home",
	"horizon",
	"hott",
	"hover",
	"hz",
	"i",
	"i2c",
	"ibus",
	"id",
	"idle",
	"ignore",
	"imu",
	"inav",
	"inclination",
	"input",
	"interval",
	"inversion",
	"invert",
	"inverted",
	"iterm",
	"jump",
	"ki",
	"kill",
	"kp",
	"land",
	"landing",
	"lat",
	"latitude",
	"launch",
	"ledstrip",
	"level",
	"limit",
	"loc",
	"logic",
	"loiter",
	"lon",
	"longitude",
	"looptime",
	"low",
	"lpf",
	"ltm",
	"mag",
	"magzero",
	"mah",
	"main",
	"manual",
	"max",
	"maxalt",
	"mc",
	"median",
	"meter",
	"mid",
	"midthr",
	"min",
	"minalt",
	"mode",
	"model",
	"moron",
	"motor",
	"multiwii",
	"name",
	"nav",
	"neutral",
	"noise",
	"notch",
	"notch1",
	"notch2",
	"num",
	"off",
	"offset",
	"on",
	"ontime",
	"operator",
	"osd",
	"output",
	"overshoot",
	"p",
	"pid",
	"pidsum",
	"pit",
	"pitch",
	"pitch2thr",
	"pitot",
	"pos",
	"position",
	"power",
	"precision",
	"prevention",
	"procedure",
	"protocol",
	"provider",
	"pulse",
	"pwm",
	"radius",
	"range",
	"rangefinder",
	"rate",
	"rc",
	"reboot",
	"recovery",
	"reference",
	"res",
	"reset",
	"rf",
	"rll",
	"roll",
	"row",
	"rssi",
	"rth",
	"rx",
	"safe",
	"safety",
	"sat",
	"sats",
	"sbas",
	"sbus",
	"scale",
	"sdcard",
	"serialrx",
	"servo",
	"setpoint",
	"shiftdown",
	"signature",
	"slowdown",
	"small",
	"smartport",
	"smoothing",
	"sound",
	"speed",
	"spektrum",
	"spi",
	"spinup",
	"stats",
	"stick",
	"str",
	"surface",
	"switch",
	"sync",
	"system",
	"tail",
	"task",
	"tc",
	"telemetry",
	"thr",
	"threshold",
	"throttle",
	"throw",
	"tilt",
	"time",
	"timeout",
	"to",
	"tolerance",
	"total",
	"tpa",
	"tri",
	"turn",
	"type",
	"uart",
	"unarmed",
	"undershoot",
	"unidir",
	"unit",
	"units",
	"update",
	"use",
	"usec",
	"user",
	"v",
	"vario",
	"vbat",
	"vel",
	"velned",
	"velocity",
	"vfas",
	"video",
	"visual",
	"voltage",
	"vtx",
	"w",
	"warning",
	"weight",
	"wing",
	"wp",
	"x",
	"xy",
	"y",
	"yaw",
	"z",
};
static const char *table_acc_hardware[] = {
	"NONE",
	"AUTO",
	"ADXL345",
	"MPU6050",
	"MMA845x",
	"BMA280",
	"LSM303DLHC",
	"MPU6000",
	"MPU6500",
	"MPU9250",
	"FAKE",
};
static const char *table_alignment[] = {
	"DEFAULT",
	"CW0",
	"CW90",
	"CW180",
	"CW270",
	"CW0FLIP",
	"CW90FLIP",
	"CW180FLIP",
	"CW270FLIP",
};
#if defined(ASYNC_GYRO_PROCESSING)
static const char *table_async_mode[] = {
	"NONE",
	"GYRO",
	"ALL",
};
#endif
static const char *table_aux_operator[] = {
	"OR",
	"AND",
};
#if defined(BARO)
static const char *table_baro_hardware[] = {
	"NONE",
	"AUTO",
	"BMP085",
	"MS5611",
	"BMP280",
	"MS5607",
	"FAKE",
};
#endif
#if defined(BLACKBOX)
static const char *table_blackbox_device[] = {
	"SERIAL",
	"SPIFLASH",
	"SDCARD",
};
#endif
static const char *table_current_sensor[] = {
	"NONE",
	"ADC",
	"VIRTUAL",
};
static const char *table_debug_modes[] = {
	"NONE",
	"GYRO",
	"NOTCH",
	"NAV_LANDING",
	"FW_ALTITUDE",
	"RFIND",
	"RFIND_Q",
	"PITOT",
};
static const char *table_failsafe_procedure[] = {
	"SET-THR",
	"DROP",
	"RTH",
	"NONE",
};
#if defined(USE_SERVOS)
static const char *table_gimbal_mode[] = {
	"NORMAL",
	"MIXTILT",
};
#endif
#if defined(GPS)
static const char *table_gps_dyn_model[] = {
	"PEDESTRIAN",
	"AIR_1G",
	"AIR_4G",
};
#endif
#if defined(GPS)
static const char *table_gps_provider[] = {
	"NMEA",
	"UBLOX",
	"I2C-NAV",
	"NAZA",
	"UBLOX7",
	"MTK",
};
#endif
#if defined(GPS)
static const char *table_gps_sbas_mode[] = {
	"AUTO",
	"EGNOS",
	"WAAS",
	"MSAS",
	"GAGAN",
	"NONE",
};
#endif
static const char *table_gyro_lpf[] = {
	"256HZ",
	"188HZ",
	"98HZ",
	"42HZ",
	"20HZ",
	"10HZ",
};
#if defined(USE_I2C)
static const char *table_i2c_speed[] = {
	"400KHZ",
	"800KHZ",
	"100KHZ",
	"200KHZ",
};
#endif
#if defined(TELEMETRY) && defined(TELEMETRY_LTM)
static const char *table_ltm_rates[] = {
	"NORMAL",
	"MEDIUM",
	"SLOW",
};
#endif
#if defined(MAG)
static const char *table_mag_hardware[] = {
	"NONE",
	"AUTO",
	"HMC5883",
	"AK8975",
	"GPSMAG",
	"MAG3110",
	"AK8963",
	"IST8310",
	"FAKE",
};
#endif
static const char *table_motor_pwm_protocol[] = {
	"STANDARD",
	"ONESHOT125",
	"ONESHOT42",
	"MULTISHOT",
	"BRUSHED",
};
#if defined(NAV)
static const char *table_nav_rth_alt_mode[] = {
	"CURRENT",
	"EXTRA",
	"FIXED",
	"MAX",
	"AT_LEAST",
};
#endif
#if defined(NAV)
static const char *table_nav_user_control_mode[] = {
	"ATTI",
	"CRUISE",
};
#endif
static const char *table_off_on[] = {
	"OFF",
	"ON",
};
#if defined(PITOT)
static const char *table_pitot_hardware[] = {
	"NONE",
	"AUTO",
	"MS4525",
	"ADC",
	"VIRTUAL",
	"FAKE",
};
#endif
#if defined(USE_RANGEFINDER)
static const char *table_rangefinder_hardware[] = {
	"NONE",
	"HCSR04",
	"SRF10",
	"HCSR04I2C",
	"VL53L0X",
};
#endif
#if defined(NAV)
static const char *table_reset_altitude[] = {
	"NEVER",
	"FIRST_ARM",
	"EACH_ARM",
};
#endif
#if defined(USE_RX_SPI)
static const char *table_rx_spi_protocol[] = {
	"V202_250K",
	"V202_1M",
	"SYMA_X",
	"SYMA_X5C",
	"CX10",
	"CX10A",
	"H8_3D",
	"INAV",
	"ELERES",
};
#endif
#if defined(SERIAL_RX)
static const char *table_serial_rx[] = {
	"SPEK1024",
	"SPEK2048",
	"SBUS",
	"SUMD",
	"SUMH",
	"XB-B",
	"XB-B-RJ01",
	"IBUS",
	"JETIEXBUS",
	"CRSF",
};
#endif
#if defined(TELEMETRY) || defined(OSD)
static const char *table_unit[] = {
	"IMPERIAL",
	"METRIC",
};
#endif
enum {
	TABLE_ACC_HARDWARE,
	TABLE_ALIGNMENT,
#if defined(ASYNC_GYRO_PROCESSING)
	TABLE_ASYNC_MODE,
#endif
	TABLE_AUX_OPERATOR,
#if defined(BARO)
	TABLE_BARO_HARDWARE,
#endif
#if defined(BLACKBOX)
	TABLE_BLACKBOX_DEVICE,
#endif
	TABLE_CURRENT_SENSOR,
	TABLE_DEBUG_MODES,
	TABLE_FAILSAFE_PROCEDURE,
#if defined(USE_SERVOS)
	TABLE_GIMBAL_MODE,
#endif
#if defined(GPS)
	TABLE_GPS_DYN_MODEL,
#endif
#if defined(GPS)
	TABLE_GPS_PROVIDER,
#endif
#if defined(GPS)
	TABLE_GPS_SBAS_MODE,
#endif
	TABLE_GYRO_LPF,
#if defined(USE_I2C)
	TABLE_I2C_SPEED,
#endif
#if defined(TELEMETRY) && defined(TELEMETRY_LTM)
	TABLE_LTM_RATES,
#endif
#if defined(MAG)
	TABLE_MAG_HARDWARE,
#endif
	TABLE_MOTOR_PWM_PROTOCOL,
#if defined(NAV)
	TABLE_NAV_RTH_ALT_MODE,
#endif
#if defined(NAV)
	TABLE_NAV_USER_CONTROL_MODE,
#endif
	TABLE_OFF_ON,
#if defined(PITOT)
	TABLE_PITOT_HARDWARE,
#endif
#if defined(USE_RANGEFINDER)
	TABLE_RANGEFINDER_HARDWARE,
#endif
#if defined(NAV)
	TABLE_RESET_ALTITUDE,
#endif
#if defined(USE_RX_SPI)
	TABLE_RX_SPI_PROTOCOL,
#endif
#if defined(SERIAL_RX)
	TABLE_SERIAL_RX,
#endif
#if defined(TELEMETRY) || defined(OSD)
	TABLE_UNIT,
#endif
	LOOKUP_TABLE_COUNT,
};
static const lookupTableEntry_t lookupTables[] = {
	{ table_acc_hardware, sizeof(table_acc_hardware) / sizeof(char*) },
	{ table_alignment, sizeof(table_alignment) / sizeof(char*) },
#if defined(ASYNC_GYRO_PROCESSING)
	{ table_async_mode, sizeof(table_async_mode) / sizeof(char*) },
#endif
	{ table_aux_operator, sizeof(table_aux_operator) / sizeof(char*) },
#if defined(BARO)
	{ table_baro_hardware, sizeof(table_baro_hardware) / sizeof(char*) },
#endif
#if defined(BLACKBOX)
	{ table_blackbox_device, sizeof(table_blackbox_device) / sizeof(char*) },
#endif
	{ table_current_sensor, sizeof(table_current_sensor) / sizeof(char*) },
	{ table_debug_modes, sizeof(table_debug_modes) / sizeof(char*) },
	{ table_failsafe_procedure, sizeof(table_failsafe_procedure) / sizeof(char*) },
#if defined(USE_SERVOS)
	{ table_gimbal_mode, sizeof(table_gimbal_mode) / sizeof(char*) },
#endif
#if defined(GPS)
	{ table_gps_dyn_model, sizeof(table_gps_dyn_model) / sizeof(char*) },
#endif
#if defined(GPS)
	{ table_gps_provider, sizeof(table_gps_provider) / sizeof(char*) },
#endif
#if defined(GPS)
	{ table_gps_sbas_mode, sizeof(table_gps_sbas_mode) / sizeof(char*) },
#endif
	{ table_gyro_lpf, sizeof(table_gyro_lpf) / sizeof(char*) },
#if defined(USE_I2C)
	{ table_i2c_speed, sizeof(table_i2c_speed) / sizeof(char*) },
#endif
#if defined(TELEMETRY) && defined(TELEMETRY_LTM)
	{ table_ltm_rates, sizeof(table_ltm_rates) / sizeof(char*) },
#endif
#if defined(MAG)
	{ table_mag_hardware, sizeof(table_mag_hardware) / sizeof(char*) },
#endif
	{ table_motor_pwm_protocol, sizeof(table_motor_pwm_protocol) / sizeof(char*) },
#if defined(NAV)
	{ table_nav_rth_alt_mode, sizeof(table_nav_rth_alt_mode) / sizeof(char*) },
#endif
#if defined(NAV)
	{ table_nav_user_control_mode, sizeof(table_nav_user_control_mode) / sizeof(char*) },
#endif
	{ table_off_on, sizeof(table_off_on) / sizeof(char*) },
#if defined(PITOT)
	{ table_pitot_hardware, sizeof(table_pitot_hardware) / sizeof(char*) },
#endif
#if defined(USE_RANGEFINDER)
	{ table_rangefinder_hardware, sizeof(table_rangefinder_hardware) / sizeof(char*) },
#endif
#if defined(NAV)
	{ table_reset_altitude, sizeof(table_reset_altitude) / sizeof(char*) },
#endif
#if defined(USE_RX_SPI)
	{ table_rx_spi_protocol, sizeof(table_rx_spi_protocol) / sizeof(char*) },
#endif
#if defined(SERIAL_RX)
	{ table_serial_rx, sizeof(table_serial_rx) / sizeof(char*) },
#endif
#if defined(TELEMETRY) || defined(OSD)
	{ table_unit, sizeof(table_unit) / sizeof(char*) },
#endif
};
const clivalue_t valueTable[] = {
// PG_GYRO_CONFIG
	{ {145, 0, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {9000}, offsetof(gyroConfig_t, looptime) },
	{ {102, 245, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(gyroConfig_t, gyroSync) },
	{ {102, 245, 63, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {1, 32}, offsetof(gyroConfig_t, gyroSyncDenominator) },
	{ {10, 102, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ALIGNMENT }, offsetof(gyroConfig_t, gyro_align) },
	{ {102, 104, 147, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GYRO_LPF }, offsetof(gyroConfig_t, gyro_lpf) },
	{ {102, 147, 112, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_MAX, .config.max = {200}, offsetof(gyroConfig_t, gyro_soft_lpf_hz) },
	{ {165, 252, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_MAX, .config.max = {128}, offsetof(gyroConfig_t, gyroMovementCalibrationThreshold) },
#ifdef USE_GYRO_NOTCH_1
	{ {102, 173, 112, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {500}, offsetof(gyroConfig_t, gyro_soft_notch_hz_1) },
	{ {102, 173, 54, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1, 500}, offsetof(gyroConfig_t, gyro_soft_notch_cutoff_1) },
#endif
#ifdef USE_GYRO_NOTCH_2
	{ {102, 174, 112, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {500}, offsetof(gyroConfig_t, gyro_soft_notch_hz_2) },
	{ {102, 174, 54, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1, 500}, offsetof(gyroConfig_t, gyro_soft_notch_cutoff_2) },
#endif
#ifdef USE_DUAL_GYRO
	{ {102, 2, 16, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 1}, offsetof(gyroConfig_t, gyro_to_use) },
#endif
// PG_ADC_CHANNEL_CONFIG
#ifdef USE_ADC
	{ {21, 6, 40, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {ADC_CHN_NONE, ADC_CHN_MAX}, offsetof(adcChannelConfig_t, adcFunctionChannel[ADC_BATTERY]) },
	{ {215, 6, 40, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {ADC_CHN_NONE, ADC_CHN_MAX}, offsetof(adcChannelConfig_t, adcFunctionChannel[ADC_RSSI]) },
	{ {53, 6, 40, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {ADC_CHN_NONE, ADC_CHN_MAX}, offsetof(adcChannelConfig_t, adcFunctionChannel[ADC_CURRENT]) },
	{ {8, 6, 40, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {ADC_CHN_NONE, ADC_CHN_MAX}, offsetof(adcChannelConfig_t, adcFunctionChannel[ADC_AIRSPEED]) },
#endif
// PG_ACCELEROMETER_CONFIG
#ifdef USE_ACC_NOTCH
	{ {2, 172, 112, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 255}, offsetof(accelerometerConfig_t, acc_notch_hz) },
	{ {2, 172, 54, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {1, 255}, offsetof(accelerometerConfig_t, acc_notch_cutoff) },
#endif
	{ {10, 2, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ALIGNMENT }, offsetof(accelerometerConfig_t, acc_align) },
	{ {2, 104, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ACC_HARDWARE }, offsetof(accelerometerConfig_t, acc_hardware) },
	{ {2, 147, 112, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {200}, offsetof(accelerometerConfig_t, acc_lpf_hz) },
	{ {5, 35, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, offsetof(accelerometerConfig_t, accZero.raw[X]) },
	{ {5, 37, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, offsetof(accelerometerConfig_t, accZero.raw[Y]) },
	{ {5, 39, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, offsetof(accelerometerConfig_t, accZero.raw[Z]) },
	{ {4, 35, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {1, 8192}, offsetof(accelerometerConfig_t, accGain.raw[X]) },
	{ {4, 37, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {1, 8192}, offsetof(accelerometerConfig_t, accGain.raw[Y]) },
	{ {4, 39, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {1, 8192}, offsetof(accelerometerConfig_t, accGain.raw[Z]) },
// PG_RANGEFINDER_CONFIG
#ifdef USE_RANGEFINDER
	{ {203, 104, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_RANGEFINDER_HARDWARE }, offsetof(rangefinderConfig_t, rangefinder_hardware) },
#endif
// PG_COMPASS_CONFIG
#ifdef MAG
	{ {10, 149, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ALIGNMENT }, offsetof(compassConfig_t, mag_align) },
	{ {149, 104, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_MAG_HARDWARE }, offsetof(compassConfig_t, mag_hardware) },
	{ {149, 60, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-18000, 18000}, offsetof(compassConfig_t, mag_declination) },
	{ {150, 35, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, offsetof(compassConfig_t, magZero.raw[X]) },
	{ {150, 37, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, offsetof(compassConfig_t, magZero.raw[Y]) },
	{ {150, 39, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, offsetof(compassConfig_t, magZero.raw[Z]) },
	{ {149, 35, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {30, 120}, offsetof(compassConfig_t, magCalibrationTimeLimit) },
#endif
// PG_BAROMETER_CONFIG
#ifdef BARO
	{ {25, 104, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_BARO_HARDWARE }, offsetof(barometerConfig_t, baro_hardware) },
	{ {25, 16, 157, 85, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(barometerConfig_t, use_median_filtering) },
#endif
// PG_PITOTMETER_CONFIG
#ifdef PITOT
	{ {190, 104, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_PITOT_HARDWARE }, offsetof(pitotmeterConfig_t, pitot_hardware) },
	{ {190, 16, 157, 85, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(pitotmeterConfig_t, use_median_filtering) },
	{ {190, 171, 147, 0, 0, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 1}, offsetof(pitotmeterConfig_t, pitot_noise_lpf) },
	{ {190, 224, 0, 0, 0, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 100}, offsetof(pitotmeterConfig_t, pitot_scale) },
#endif
// PG_RX_CONFIG
	{ {159, 205, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1200, 1700}, offsetof(rxConfig_t, midrc) },
	{ {161, 42, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, offsetof(rxConfig_t, mincheck) },
	{ {154, 42, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, offsetof(rxConfig_t, maxcheck) },
	{ {215, 40, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, MAX_SUPPORTED_RC_CHANNEL_COUNT}, offsetof(rxConfig_t, rssi_channel) },
	{ {215, 224, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {RSSI_SCALE_MIN, RSSI_SCALE_MAX}, offsetof(rxConfig_t, rssi_scale) },
	{ {215, 125, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(rxConfig_t, rssiInvert) },
	{ {205, 234, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(rxConfig_t, rcSmoothing) },
#ifdef SERIAL_RX
	{ {226, 198, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_SERIAL_RX }, offsetof(rxConfig_t, serialrx_provider) },
	{ {223, 124, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(rxConfig_t, sbus_inversion) },
#endif
#ifdef USE_RX_SPI
	{ {217, 238, 197, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_RX_SPI_PROTOCOL }, offsetof(rxConfig_t, rx_spi_protocol) },
	{ {217, 238, 116, 0, 0, 0, }, VAR_UINT32 | MASTER_VALUE, .config.minmax = {0, 0}, offsetof(rxConfig_t, rx_spi_id) },
#endif
	{ {217, 238, 211, 40, 49, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 8}, offsetof(rxConfig_t, rx_spi_rf_channel_count) },
#ifdef SPEKTRUM_BIND
	{ {237, 220, 30, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {SPEKTRUM_SAT_BIND_DISABLED, SPEKTRUM_SAT_BIND_MAX}, offsetof(rxConfig_t, spektrum_sat_bind) },
#endif
	{ {217, 161, 17, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_PULSE_MIN, PWM_PULSE_MAX}, offsetof(rxConfig_t, rx_min_usec) },
	{ {217, 154, 17, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_PULSE_MIN, PWM_PULSE_MAX}, offsetof(rxConfig_t, rx_max_usec) },
#ifdef STM32F4
	{ {226, 103, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(rxConfig_t, halfDuplex) },
#endif
// PG_BLACKBOX_CONFIG
#ifdef BLACKBOX
	{ {31, 204, 175, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {1, 255}, offsetof(blackboxConfig_t, rate_num) },
	{ {31, 204, 63, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {1, 255}, offsetof(blackboxConfig_t, rate_denom) },
	{ {31, 65, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_BLACKBOX_DEVICE }, offsetof(blackboxConfig_t, device) },
#ifdef USE_SDCARD
	{ {225, 64, 126, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(blackboxConfig_t, invertedCardDetection) },
#endif
#endif
// PG_MOTOR_CONFIG
	{ {161, 253, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, offsetof(motorConfig_t, minthrottle) },
	{ {154, 253, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, offsetof(motorConfig_t, maxthrottle) },
	{ {161, 44, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, offsetof(motorConfig_t, mincommand) },
	{ {166, 200, 204, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {50, 32000}, offsetof(motorConfig_t, motorPwmRate) },
	{ {166, 200, 197, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_MOTOR_PWM_PROTOCOL }, offsetof(motorConfig_t, motorPwmProtocol) },
// PG_FAILSAFE_CONFIG
	{ {83, 62, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 200}, offsetof(failsafeConfig_t, failsafe_delay) },
	{ {83, 207, 62, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 200}, offsetof(failsafeConfig_t, failsafe_recovery_delay) },
	{ {83, 176, 62, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 200}, offsetof(failsafeConfig_t, failsafe_off_delay) },
	{ {83, 253, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 2000}, offsetof(failsafeConfig_t, failsafe_throttle) },
	{ {83, 253, 146, 62, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 300}, offsetof(failsafeConfig_t, failsafe_throttle_low_delay) },
	{ {83, 196, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_FAILSAFE_PROCEDURE }, offsetof(failsafeConfig_t, failsafe_procedure) },
	{ {83, 241, 252, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 500}, offsetof(failsafeConfig_t, failsafe_stick_motion_threshold) },
	{ {83, 97, 213, 15, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-800, 800}, offsetof(failsafeConfig_t, failsafe_fw_roll_angle) },
	{ {83, 97, 188, 15, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-800, 800}, offsetof(failsafeConfig_t, failsafe_fw_pitch_angle) },
	{ {83, 97, 38, 204, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-1000, 1000}, offsetof(failsafeConfig_t, failsafe_fw_yaw_rate) },
// PG_BOARDALIGNMENT_CONFIG
	{ {10, 32, 213, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-1800, 3600}, offsetof(boardAlignment_t, rollDeciDegrees) },
	{ {10, 32, 188, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-1800, 3600}, offsetof(boardAlignment_t, pitchDeciDegrees) },
	{ {10, 32, 38, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-1800, 3600}, offsetof(boardAlignment_t, yawDeciDegrees) },
// PG_GIMBAL_CONFIG
#ifdef USE_SERVOS
	{ {99, 163, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GIMBAL_MODE }, offsetof(gimbalConfig_t, mode) },
#endif
// PG_BATTERY_CONFIG
	{ {26, 37, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 20000}, offsetof(batteryConfig_t, batteryCapacity) },
#ifdef USE_ADC
	{ {21, 224, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {VBAT_SCALE_MIN, VBAT_SCALE_MAX}, offsetof(batteryConfig_t, vbatscale) },
	{ {21, 154, 38, 28, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {10, 50}, offsetof(batteryConfig_t, vbatmaxcellvoltage) },
	{ {21, 161, 38, 28, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {10, 50}, offsetof(batteryConfig_t, vbatmincellvoltage) },
	{ {21, 31, 38, 28, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {10, 50}, offsetof(batteryConfig_t, vbatwarningcellvoltage) },
#endif
	{ {53, 158, 224, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-10000, 10000}, offsetof(batteryConfig_t, currentMeterScale) },
	{ {53, 158, 177, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 3300}, offsetof(batteryConfig_t, currentMeterOffset) },
	{ {167, 53, 158, 182, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(batteryConfig_t, multiwiiCurrentMeterOutput) },
	{ {53, 158, 8, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_CURRENT_SENSOR }, offsetof(batteryConfig_t, currentMeterType) },
// PG_MIXER_CONFIG
	{ {38, 166, 67, 0, 0, 0, }, VAR_INT8 | MASTER_VALUE, .config.minmax = {-1, 1}, offsetof(mixerConfig_t, yaw_motor_direction) },
	{ {38, 128, 195, 139, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {YAW_JUMP_PREVENTION_LIMIT_LOW, YAW_JUMP_PREVENTION_LIMIT_HIGH}, offsetof(mixerConfig_t, yaw_jump_prevention_limit) },
// PG_MOTOR_3D_CONFIG
	{ {0, 57, 146, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, offsetof(flight3DConfig_t, deadband3d_low) },
	{ {0, 57, 106, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, offsetof(flight3DConfig_t, deadband3d_high) },
	{ {0, 170, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, offsetof(flight3DConfig_t, neutral3d) },
// PG_SERVO_CONFIG
#ifdef USE_SERVOS
	{ {227, 39, 199, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, offsetof(servoConfig_t, servoCenterPulse) },
	{ {227, 200, 204, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {50, 498}, offsetof(servoConfig_t, servoPwmRate) },
	{ {227, 147, 112, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {0, 400}, offsetof(servoConfig_t, servo_lowpass_freq) },
	{ {89, 254, 177, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {FLAPERON_THROW_MIN, FLAPERON_THROW_MAX}, offsetof(servoConfig_t, flaperon_throw_offset) },
	{ {6, 10, 227, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(servoConfig_t, tri_unarmed_servo) },
#endif
// PG_CONTROLRATE_PROFILE
	{ {205, 81, 0, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {0, 100}, offsetof(controlRateConfig_t, rcExpo8) },
	{ {205, 38, 81, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {0, 100}, offsetof(controlRateConfig_t, rcYawExpo8) },
	{ {251, 159, 0, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {0, 100}, offsetof(controlRateConfig_t, thrMid8) },
	{ {251, 81, 0, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {0, 100}, offsetof(controlRateConfig_t, thrExpo8) },
	{ {213, 204, 0, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {CONTROL_RATE_CONFIG_ROLL_PITCH_RATE_MIN, CONTROL_RATE_CONFIG_ROLL_PITCH_RATE_MAX}, offsetof(controlRateConfig_t, rates[FD_ROLL]) },
	{ {188, 204, 0, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {CONTROL_RATE_CONFIG_ROLL_PITCH_RATE_MIN, CONTROL_RATE_CONFIG_ROLL_PITCH_RATE_MAX}, offsetof(controlRateConfig_t, rates[FD_PITCH]) },
	{ {38, 204, 0, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {CONTROL_RATE_CONFIG_YAW_RATE_MIN, CONTROL_RATE_CONFIG_YAW_RATE_MAX}, offsetof(controlRateConfig_t, rates[FD_YAW]) },
	{ {5, 204, 0, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {0, CONTROL_RATE_CONFIG_TPA_MAX}, offsetof(controlRateConfig_t, dynThrPID) },
	{ {5, 33, 0, 0, 0, 0, }, VAR_UINT16 | CONTROL_RATE_VALUE, .config.minmax = {PWM_RANGE_MIN, PWM_RANGE_MAX}, offsetof(controlRateConfig_t, tpa_breakpoint) },
// PG_SERIAL_CONFIG
	{ {206, 41, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {48, 126}, offsetof(serialConfig_t, reboot_character) },
// PG_IMU_CONFIG
	{ {119, 56, 131, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {UINT16_MAX}, offsetof(imuConfig_t, dcm_kp_acc) },
	{ {119, 56, 129, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {UINT16_MAX}, offsetof(imuConfig_t, dcm_ki_acc) },
	{ {119, 56, 131, 149, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {UINT16_MAX}, offsetof(imuConfig_t, dcm_kp_mag) },
	{ {119, 56, 129, 149, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {UINT16_MAX}, offsetof(imuConfig_t, dcm_ki_mag) },
	{ {232, 15, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 180}, offsetof(imuConfig_t, small_angle) },
// PG_ARMING_CONFIG
	{ {88, 33, 22, 16, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(armingConfig_t, fixed_wing_auto_arm) },
	{ {68, 130, 244, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(armingConfig_t, disarm_kill_switch) },
	{ {22, 68, 62, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 60}, offsetof(armingConfig_t, auto_disarm_delay) },
// PG_GPS_CONFIG
#ifdef GPS
	{ {100, 198, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GPS_PROVIDER }, offsetof(gpsConfig_t, provider) },
	{ {100, 222, 163, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GPS_SBAS_MODE }, offsetof(gpsConfig_t, sbasMode) },
	{ {100, 75, 164, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GPS_DYN_MODEL }, offsetof(gpsConfig_t, dynModel) },
	{ {100, 22, 46, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(gpsConfig_t, autoConfig) },
	{ {100, 22, 27, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(gpsConfig_t, autoBaud) },
	{ {100, 161, 221, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {5, 10}, offsetof(gpsConfig_t, gpsMinSats) },
#endif
// PG_RC_CONTROLS_CONFIG
	{ {57, 0, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 32}, offsetof(rcControlsConfig_t, deadband) },
	{ {38, 57, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 100}, offsetof(rcControlsConfig_t, yaw_deadband) },
	{ {191, 107, 57, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {10, 250}, offsetof(rcControlsConfig_t, pos_hold_deadband) },
	{ {12, 107, 57, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {10, 250}, offsetof(rcControlsConfig_t, alt_hold_deadband) },
	{ {0, 57, 253, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, offsetof(rcControlsConfig_t, deadband3d_throttle) },
// PG_PID_PROFILE
	{ {156, 184, 188, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_mc.pid[PID_PITCH].P) },
	{ {156, 113, 188, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_mc.pid[PID_PITCH].I) },
	{ {156, 55, 188, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_mc.pid[PID_PITCH].D) },
	{ {156, 184, 213, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_mc.pid[PID_ROLL].P) },
	{ {156, 113, 213, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_mc.pid[PID_ROLL].I) },
	{ {156, 55, 213, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_mc.pid[PID_ROLL].D) },
	{ {156, 184, 38, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_mc.pid[PID_YAW].P) },
	{ {156, 113, 38, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_mc.pid[PID_YAW].I) },
	{ {156, 55, 38, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_mc.pid[PID_YAW].D) },
	{ {156, 184, 138, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_mc.pid[PID_LEVEL].P) },
	{ {156, 113, 138, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_mc.pid[PID_LEVEL].I) },
	{ {156, 55, 138, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_mc.pid[PID_LEVEL].D) },
	{ {97, 184, 188, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_fw.pid[PID_PITCH].P) },
	{ {97, 113, 188, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_fw.pid[PID_PITCH].I) },
	{ {97, 84, 188, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_fw.pid[PID_PITCH].D) },
	{ {97, 184, 213, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_fw.pid[PID_ROLL].P) },
	{ {97, 113, 213, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_fw.pid[PID_ROLL].I) },
	{ {97, 84, 213, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_fw.pid[PID_ROLL].D) },
	{ {97, 184, 38, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_fw.pid[PID_YAW].P) },
	{ {97, 113, 38, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_fw.pid[PID_YAW].I) },
	{ {97, 84, 38, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_fw.pid[PID_YAW].D) },
	{ {97, 184, 138, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_fw.pid[PID_LEVEL].P) },
	{ {97, 113, 138, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_fw.pid[PID_LEVEL].I) },
	{ {97, 55, 138, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, bank_fw.pid[PID_LEVEL].D) },
	{ {154, 15, 121, 212, 0, 0, }, VAR_INT16 | PROFILE_VALUE, .config.minmax = {100, 900}, offsetof(pidProfile_t, max_angle_inclination[FD_ROLL]) },
	{ {154, 15, 121, 187, 0, 0, }, VAR_INT16 | PROFILE_VALUE, .config.minmax = {100, 900}, offsetof(pidProfile_t, max_angle_inclination[FD_PITCH]) },
	{ {74, 147, 112, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, dterm_lpf_hz) },
	{ {38, 147, 112, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, offsetof(pidProfile_t, yaw_lpf_hz) },
	{ {74, 228, 32, 0, 0, 0, }, VAR_FLOAT | PROFILE_VALUE, .config.minmax = {0, 2}, offsetof(pidProfile_t, dterm_setpoint_weight) },
#ifdef USE_SERVOS
	{ {97, 127, 254, 139, 0, 0, }, VAR_UINT16 | PROFILE_VALUE, .config.minmax = {FW_ITERM_THROW_LIMIT_MIN, FW_ITERM_THROW_LIMIT_MAX}, offsetof(pidProfile_t, fixedWingItermThrowLimit) },
	{ {97, 208, 8, 0, 0, 0, }, VAR_FLOAT | PROFILE_VALUE, .config.minmax = {1, 5000}, offsetof(pidProfile_t, fixedWingReferenceAirspeed) },
	{ {97, 7, 19, 38, 98, 0, }, VAR_FLOAT | PROFILE_VALUE, .config.minmax = {0, 2}, offsetof(pidProfile_t, fixedWingCoordinatedYawGain) },
#endif
#ifdef USE_DTERM_NOTCH
	{ {74, 172, 112, 0, 0, 0, }, VAR_UINT16 | PROFILE_VALUE, .config.minmax = {0, 500}, offsetof(pidProfile_t, dterm_soft_notch_hz) },
	{ {74, 172, 54, 0, 0, 0, }, VAR_UINT16 | PROFILE_VALUE, .config.minmax = {1, 500}, offsetof(pidProfile_t, dterm_soft_notch_cutoff) },
#endif
	{ {186, 139, 0, 0, 0, 0, }, VAR_UINT16 | PROFILE_VALUE, .config.minmax = {PID_SUM_LIMIT_MIN, PID_SUM_LIMIT_MAX}, offsetof(pidProfile_t, pidSumLimit) },
	{ {38, 184, 139, 0, 0, 0, }, VAR_UINT16 | PROFILE_VALUE, .config.minmax = {YAW_P_LIMIT_MIN, YAW_P_LIMIT_MAX}, offsetof(pidProfile_t, yaw_p_limit) },
	{ {127, 118, 252, 0, 0, 0, }, VAR_UINT16 | PROFILE_VALUE, .config.minmax = {15, 1000}, offsetof(pidProfile_t, rollPitchItermIgnoreRate) },
	{ {38, 127, 118, 252, 0, 0, }, VAR_UINT16 | PROFILE_VALUE, .config.minmax = {15, 10000}, offsetof(pidProfile_t, yawItermIgnoreRate) },
	{ {204, 3, 139, 213, 188, 0, }, VAR_UINT32 | PROFILE_VALUE | MODE_MAX, .config.max = {500000}, offsetof(pidProfile_t, axisAccelerationLimitRollPitch) },
	{ {204, 3, 139, 38, 0, 0, }, VAR_UINT32 | PROFILE_VALUE | MODE_MAX, .config.max = {500000}, offsetof(pidProfile_t, axisAccelerationLimitYaw) },
	{ {105, 107, 204, 139, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {HEADING_HOLD_RATE_LIMIT_MIN, HEADING_HOLD_RATE_LIMIT_MAX}, offsetof(pidProfile_t, heading_hold_rate_limit) },
#ifdef NAV
	{ {169, 156, 191, 39, 184, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_mc.pid[PID_POS_Z].P) },
	{ {169, 156, 191, 39, 113, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_mc.pid[PID_POS_Z].I) },
	{ {169, 156, 191, 39, 55, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_mc.pid[PID_POS_Z].D) },
	{ {169, 156, 22, 39, 184, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_Z].P) },
	{ {169, 156, 22, 39, 113, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_Z].I) },
	{ {169, 156, 22, 39, 55, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_Z].D) },
	{ {169, 156, 191, 36, 184, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_mc.pid[PID_POS_XY].P) },
	{ {169, 156, 191, 36, 113, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_mc.pid[PID_POS_XY].I) },
	{ {169, 156, 191, 36, 55, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_mc.pid[PID_POS_XY].D) },
	{ {169, 156, 22, 36, 184, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_XY].P) },
	{ {169, 156, 22, 36, 113, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_XY].I) },
	{ {169, 156, 22, 36, 55, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_XY].D) },
	{ {169, 97, 191, 39, 184, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_fw.pid[PID_POS_Z].P) },
	{ {169, 97, 191, 39, 113, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_fw.pid[PID_POS_Z].I) },
	{ {169, 97, 191, 39, 55, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_fw.pid[PID_POS_Z].D) },
	{ {169, 97, 191, 36, 184, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_fw.pid[PID_POS_XY].P) },
	{ {169, 97, 191, 36, 113, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_fw.pid[PID_POS_XY].I) },
	{ {169, 97, 191, 36, 55, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, offsetof(pidProfile_t, bank_fw.pid[PID_POS_XY].D) },
#endif
// PG_PID_AUTOTUNE_CONFIG
#ifdef NAV
	{ {97, 23, 183, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {50, 500}, offsetof(pidAutotuneConfig_t, fw_overshoot_time) },
	{ {97, 23, 11, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {50, 500}, offsetof(pidAutotuneConfig_t, fw_undershoot_time) },
	{ {97, 23, 252, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 100}, offsetof(pidAutotuneConfig_t, fw_max_rate_threshold) },
	{ {97, 23, 84, 2, 184, 98, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 100}, offsetof(pidAutotuneConfig_t, fw_ff_to_p_gain) },
	{ {97, 23, 84, 2, 113, 249, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 5000}, offsetof(pidAutotuneConfig_t, fw_ff_to_i_time_constant) },
#endif
// PG_POSITION_ESTIMATION_CONFIG
#ifdef NAV
#ifdef NAV_AUTO_MAG_DECLINATION
	{ {120, 22, 149, 59, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(positionEstimationConfig_t, automatic_mag_declination) },
#endif
	{ {120, 101, 34, 3, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 255}, offsetof(positionEstimationConfig_t, gravity_calibration_tolerance) },
	{ {120, 16, 100, 23, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(positionEstimationConfig_t, use_gps_velned) },
	{ {120, 100, 62, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 500}, offsetof(positionEstimationConfig_t, gps_delay_ms) },
	{ {120, 210, 14, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_RESET_ALTITUDE }, offsetof(positionEstimationConfig_t, reset_altitude_type) },
	{ {120, 154, 243, 14, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 1000}, offsetof(positionEstimationConfig_t, max_surface_altitude) },
	{ {120, 30, 39, 243, 184, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, offsetof(positionEstimationConfig_t, w_z_surface_p) },
	{ {120, 30, 39, 243, 19, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, offsetof(positionEstimationConfig_t, w_z_surface_v) },
	{ {120, 30, 39, 25, 184, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, offsetof(positionEstimationConfig_t, w_z_baro_p) },
	{ {120, 30, 39, 100, 184, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, offsetof(positionEstimationConfig_t, w_z_gps_p) },
	{ {120, 30, 39, 100, 19, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, offsetof(positionEstimationConfig_t, w_z_gps_v) },
	{ {120, 30, 36, 100, 184, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, offsetof(positionEstimationConfig_t, w_xy_gps_p) },
	{ {120, 30, 36, 100, 19, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, offsetof(positionEstimationConfig_t, w_xy_gps_v) },
	{ {120, 30, 39, 209, 19, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, offsetof(positionEstimationConfig_t, w_z_res_v) },
	{ {120, 30, 36, 209, 19, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, offsetof(positionEstimationConfig_t, w_xy_res_v) },
	{ {120, 30, 2, 29, 0, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 1}, offsetof(positionEstimationConfig_t, w_acc_bias) },
	{ {120, 154, 79, 80, 0, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 9999}, offsetof(positionEstimationConfig_t, max_eph_epv) },
	{ {120, 25, 80, 0, 0, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 9999}, offsetof(positionEstimationConfig_t, baro_epv) },
#endif
// PG_NAV_CONFIG
#ifdef NAV
	{ {169, 68, 178, 133, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(navConfig_t, general.flags.disarm_on_landing) },
	{ {169, 16, 160, 92, 13, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(navConfig_t, general.flags.use_thr_mid_for_althold) },
	{ {169, 82, 17, 219, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(navConfig_t, general.flags.extra_arming_safety) },
	{ {169, 18, 47, 163, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_NAV_USER_CONTROL_MODE }, offsetof(navConfig_t, general.flags.user_control_mode) },
	{ {169, 192, 1, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 10}, offsetof(navConfig_t, general.pos_failure_timeout) },
	{ {169, 34, 201, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {10, 10000}, offsetof(navConfig_t, general.waypoint_radius) },
	{ {169, 34, 218, 70, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {65000}, offsetof(navConfig_t, general.waypoint_safe_distance) },
	{ {169, 22, 236, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {10, 2000}, offsetof(navConfig_t, general.max_auto_speed) },
	{ {169, 22, 43, 204, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {10, 2000}, offsetof(navConfig_t, general.max_auto_climb_rate) },
	{ {169, 153, 236, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {10, 2000}, offsetof(navConfig_t, general.max_manual_speed) },
	{ {169, 153, 43, 204, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {10, 2000}, offsetof(navConfig_t, general.max_manual_climb_rate) },
	{ {169, 133, 236, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {100, 2000}, offsetof(navConfig_t, general.land_descent_rate) },
	{ {169, 132, 231, 162, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {50, 1000}, offsetof(navConfig_t, general.land_slowdown_minalt) },
	{ {169, 132, 231, 155, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {500, 4000}, offsetof(navConfig_t, general.land_slowdown_maxalt) },
	{ {169, 77, 133, 236, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {100, 2000}, offsetof(navConfig_t, general.emerg_descent_rate) },
	{ {169, 161, 216, 70, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 5000}, offsetof(navConfig_t, general.min_rth_distance) },
	{ {169, 216, 43, 87, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(navConfig_t, general.flags.rth_climb_first) },
	{ {169, 216, 43, 118, 77, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(navConfig_t, general.flags.rth_climb_ignore_emerg) },
	{ {169, 216, 247, 87, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(navConfig_t, general.flags.rth_tail_first) },
	{ {169, 216, 11, 133, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(navConfig_t, general.flags.rth_allow_landing) },
	{ {169, 216, 12, 163, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_NAV_RTH_ALT_MODE }, offsetof(navConfig_t, general.flags.rth_alt_control_mode) },
	{ {169, 216, 1, 252, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {65000}, offsetof(navConfig_t, general.rth_abort_threshold) },
	{ {169, 216, 14, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {65000}, offsetof(navConfig_t, general.rth_altitude) },
	{ {169, 156, 24, 15, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {15, 45}, offsetof(navConfig_t, mc.max_bank_angle) },
	{ {169, 156, 111, 251, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 2000}, offsetof(navConfig_t, mc.hover_throttle) },
	{ {169, 156, 22, 68, 62, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {100, 10000}, offsetof(navConfig_t, mc.auto_disarm_delay) },
	{ {169, 97, 52, 251, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 2000}, offsetof(navConfig_t, fw.cruise_throttle) },
	{ {169, 97, 161, 251, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 2000}, offsetof(navConfig_t, fw.min_throttle) },
	{ {169, 97, 154, 251, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 2000}, offsetof(navConfig_t, fw.max_throttle) },
	{ {169, 97, 24, 15, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {5, 80}, offsetof(navConfig_t, fw.max_bank_angle) },
	{ {169, 97, 43, 15, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {5, 80}, offsetof(navConfig_t, fw.max_climb_angle) },
	{ {169, 97, 71, 15, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {5, 80}, offsetof(navConfig_t, fw.max_dive_angle) },
	{ {169, 97, 189, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 100}, offsetof(navConfig_t, fw.pitch_to_throttle) },
	{ {169, 97, 142, 201, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 10000}, offsetof(navConfig_t, fw.loiter_radius) },
#ifdef FIXED_WING_LANDING
	{ {169, 97, 132, 71, 15, 0, }, VAR_INT8 | MASTER_VALUE, .config.minmax = {-20, 20}, offsetof(navConfig_t, fw.land_dive_angle) },
#endif
	{ {169, 97, 136, 24, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {100, 10000}, offsetof(navConfig_t, fw.launch_velocity_thresh) },
	{ {169, 97, 136, 3, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 20000}, offsetof(navConfig_t, fw.launch_accel_thresh) },
	{ {169, 97, 136, 154, 15, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {5, 180}, offsetof(navConfig_t, fw.launch_max_angle) },
	{ {169, 97, 136, 64, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {10, 1000}, offsetof(navConfig_t, fw.launch_time_thresh) },
	{ {169, 97, 136, 251, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 2000}, offsetof(navConfig_t, fw.launch_throttle) },
	{ {169, 97, 136, 117, 251, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 2000}, offsetof(navConfig_t, fw.launch_idle_throttle) },
	{ {169, 97, 136, 166, 62, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 5000}, offsetof(navConfig_t, fw.launch_motor_timer) },
	{ {169, 97, 136, 239, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 1000}, offsetof(navConfig_t, fw.launch_motor_spinup_time) },
	{ {169, 97, 136, 1, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {60000}, offsetof(navConfig_t, fw.launch_timeout) },
	{ {169, 97, 136, 43, 15, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {5, 45}, offsetof(navConfig_t, fw.launch_climb_angle) },
#endif
// PG_TELEMETRY_CONFIG
#ifdef TELEMETRY
	{ {250, 244, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(telemetryConfig_t, telemetry_switch) },
	{ {250, 124, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(telemetryConfig_t, telemetry_inversion) },
	{ {96, 61, 135, 0, 0, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {-90, 90}, offsetof(telemetryConfig_t, gpsNoFixLatitude) },
	{ {96, 61, 144, 0, 0, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {-180, 180}, offsetof(telemetryConfig_t, gpsNoFixLongitude) },
	{ {96, 48, 93, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, FRSKY_FORMAT_NMEA}, offsetof(telemetryConfig_t, frsky_coordinate_format) },
	{ {96, 13, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_UNIT }, offsetof(telemetryConfig_t, frsky_unit) },
	{ {96, 25, 194, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {FRSKY_VFAS_PRECISION_LOW, FRSKY_VFAS_PRECISION_HIGH}, offsetof(telemetryConfig_t, frsky_vfas_precision) },
	{ {96, 25, 38, 28, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(telemetryConfig_t, frsky_vfas_cell_voltage) },
	{ {110, 9, 235, 123, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 120}, offsetof(telemetryConfig_t, hottAlarmSoundInterval) },
#ifdef TELEMETRY_SMARTPORT
	{ {233, 9, 12, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(telemetryConfig_t, smartportUartUnidirectional) },
#endif
	{ {115, 250, 8, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 255}, offsetof(telemetryConfig_t, ibusTelemetryType) },
#ifdef TELEMETRY_LTM
	{ {148, 15, 204, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_LTM_RATES }, offsetof(telemetryConfig_t, ltmUpdateRate) },
#endif
#endif
// PG_ELERES_CONFIG
#ifdef USE_RX_ELERES
	{ {76, 94, 0, 0, 0, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {415, 450}, offsetof(eleresConfig_t, eleresFreq) },
	{ {76, 250, 78, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(eleresConfig_t, eleresTelemetryEn) },
	{ {76, 250, 193, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 7}, offsetof(eleresConfig_t, eleresTelemetryPower) },
	{ {76, 140, 78, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(eleresConfig_t, eleresLocEn) },
	{ {76, 140, 193, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 7}, offsetof(eleresConfig_t, eleresLocPower) },
	{ {76, 140, 62, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {30, 1800}, offsetof(eleresConfig_t, eleresLocDelay) },
	{ {76, 230, 0, 0, 0, 0, }, VAR_UINT32 | MASTER_VALUE | MODE_MAX, .config.max = {4294967295}, offsetof(eleresConfig_t, eleresSignature) },
#endif
// PG_LED_STRIP_CONFIG
#ifdef LED_STRIP
	{ {137, 27, 28, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(ledStripConfig_t, ledstrip_visual_beeper) },
#endif
// PG_OSD_CONFIG
#ifdef OSD
	{ {181, 26, 246, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 2}, offsetof(osdConfig_t, video_system) },
	{ {181, 214, 229, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 1}, offsetof(osdConfig_t, row_shiftdown) },
	{ {181, 14, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_UNIT }, offsetof(osdConfig_t, units) },
	{ {181, 215, 9, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 100}, offsetof(osdConfig_t, rssi_alarm) },
	{ {181, 36, 9, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 20000}, offsetof(osdConfig_t, cap_alarm) },
	{ {181, 0, 9, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 60}, offsetof(osdConfig_t, time_alarm) },
	{ {181, 12, 9, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 10000}, offsetof(osdConfig_t, alt_alarm) },
	{ {181, 152, 28, 191, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_MAIN_BATT_VOLTAGE) },
	{ {181, 215, 191, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_RSSI_VALUE]) },
	{ {181, 91, 191, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_FLYTIME]) },
	{ {181, 179, 191, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_ONTIME]) },
	{ {181, 90, 191, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_FLYMODE]) },
	{ {181, 253, 191, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_THROTTLE_POS]) },
	{ {181, 29, 40, 191, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_VTX_CHANNEL]) },
	{ {181, 51, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_CROSSHAIRS]) },
	{ {181, 18, 109, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_ARTIFICIAL_HORIZON]) },
	{ {181, 53, 72, 191, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_CURRENT_DRAW) },
	{ {181, 151, 73, 191, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_MAH_DRAWN]) },
	{ {181, 50, 168, 191, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_CRAFT_NAME]) },
	{ {181, 100, 236, 191, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_GPS_SPEED]) },
	{ {181, 100, 221, 191, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_GPS_SATS]) },
	{ {181, 100, 143, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_GPS_LON]) },
	{ {181, 100, 134, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_GPS_LAT]) },
	{ {181, 108, 66, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_HOME_DIR]) },
	{ {181, 108, 69, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_HOME_DIST]) },
	{ {181, 14, 191, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_ALTITUDE]) },
	{ {181, 20, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_VARIO]) },
	{ {181, 20, 175, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_VARIO_NUM]) },
	{ {181, 185, 213, 191, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_ROLL_PIDS]) },
	{ {181, 185, 188, 191, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_PITCH_PIDS]) },
	{ {181, 185, 38, 191, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_YAW_PIDS]) },
	{ {181, 193, 191, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_POWER]) },
	{ {181, 7, 236, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, offsetof(osdConfig_t, item_pos[OSD_AIR_SPEED]) },
#endif
// PG_SYSTEM_CONFIG
#ifdef USE_I2C
	{ {114, 236, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_I2C_SPEED }, offsetof(systemConfig_t, i2c_speed) },
#endif
	{ {58, 163, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_DEBUG_MODES }, offsetof(systemConfig_t, debug_mode) },
#ifdef ASYNC_GYRO_PROCESSING
	{ {2, 248, 95, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {ACC_TASK_FREQUENCY_MIN, ACC_TASK_FREQUENCY_MAX}, offsetof(systemConfig_t, accTaskFrequency) },
	{ {21, 248, 95, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {ATTITUDE_TASK_FREQUENCY_MIN, ATTITUDE_TASK_FREQUENCY_MAX}, offsetof(systemConfig_t, attitudeTaskFrequency) },
	{ {20, 163, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ASYNC_MODE }, offsetof(systemConfig_t, asyncMode) },
#endif
	{ {253, 255, 45, 242, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 100}, offsetof(systemConfig_t, throttle_tilt_compensation_strength) },
	{ {122, 86, 163, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(systemConfig_t, pwmRxInputFilteringMode) },
// PG_MODE_ACTIVATION_OPERATOR_CONFIG
	{ {163, 202, 141, 180, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_AUX_OPERATOR }, offsetof(modeActivationOperatorConfig_t, modeActivationOperator) },
// PG_STATS_CONFIG
#ifdef STATS
	{ {240, 0, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(statsConfig_t, stats_enabled) },
	{ {240, 4, 0, 0, 0, 0, }, VAR_UINT32 | MASTER_VALUE | MODE_MAX, .config.max = {INT32_MAX}, offsetof(statsConfig_t, stats_total_time) },
	{ {240, 4, 69, 0, 0, 0, }, VAR_UINT32 | MASTER_VALUE | MODE_MAX, .config.max = {INT32_MAX}, offsetof(statsConfig_t, stats_total_dist) },
#endif
};
