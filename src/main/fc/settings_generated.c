static const char *words[] = {
	NULL,
	"nav",
	"fw",
	"osd",
	"pos",
	"mc",
	"p",
	"z",
	"yaw",
	"inav",
	"gps",
	"rate",
	"i",
	"angle",
	"xy",
	"d",
	"roll",
	"pitch",
	"w",
	"max",
	"gyro",
	"failsafe",
	"launch",
	"delay",
	"hz",
	"min",
	"rth",
	"thr",
	"mode",
	"limit",
	"time",
	"acc",
	"mag",
	"speed",
	"lpf",
	"auto",
	"eleres",
	"deadband",
	"threshold",
	"hardware",
	"rssi",
	"climb",
	"frsky",
	"current",
	"level",
	"channel",
	"vel",
	"throttle",
	"align",
	"telemetry",
	"vbat",
	"v",
	"voltage",
	"servo",
	"imu",
	"alarm",
	"rx",
	"cell",
	"rc",
	"pitot",
	"3d",
	"disarm",
	"notch",
	"landing",
	"use",
	"motor",
	"cutoff",
	"dterm",
	"adc",
	"altitude",
	"meter",
	"dcm",
	"baro",
	"scale",
	"pid",
	"expo",
	"accel",
	"ff",
	"autotune",
	"x",
	"board",
	"pwm",
	"hold",
	"stats",
	"power",
	"iterm",
	"y",
	"magzero",
	"acczero",
	"accgain",
	"loc",
	"blackbox",
	"alt",
	"surface",
	"land",
	"ignore",
	"slowdown",
	"low",
	"spi",
	"notch1",
	"timeout",
	"epv",
	"dist",
	"mid",
	"usec",
	"denom",
	"vfas",
	"protocol",
	"dive",
	"switch",
	"wp",
	"kp",
	"ki",
	"check",
	"default",
	"num",
	"vario",
	"tpa",
	"manual",
	"task",
	"throw",
	"home",
	"distance",
	"serialrx",
	"radius",
	"filter",
	"airspeed",
	"sync",
	"emerg",
	"bank",
	"en",
	"frequency",
	"sats",
	"inversion",
	"type",
	"offset",
	"notch2",
	"res",
	"inclination",
	"median",
	"detect",
	"provider",
	"first",
	"total",
	"neutral",
	"dir",
	"fw_autotune_ff_to_p_gain",
	"nav_use_midthr_for_althold",
	"pidsum",
	"dyn",
	"prevention",
	"latitude",
	"warning",
	"looptime",
	"loiter",
	"freq",
	"i2c",
	"recovery",
	"async",
	"pitch2thr",
	"ltm",
	"jump",
	"vtx",
	"fw_turn_assist_yaw_gain",
	"reboot",
	"row",
	"rangefinder",
	"inav_gravity_cal_tolerance",
	"direction",
	"sbas",
	"fw_autotune_ff_to_i_tc",
	"control",
	"spinup",
	"update",
	"longitude",
	"pulse",
	"tri",
	"bind",
	"horizon",
	"input",
	"reference",
	"minalt",
	"model",
	"kill",
	"breakpoint",
	"smoothing",
	"lat",
	"unit",
	"safe",
	"attitude",
	"config",
	"multiwii",
	"ledstrip",
	"craft",
	"air",
	"bias",
	"rll",
	"pit",
	"system",
	"debug",
	"battery",
	"visual",
	"on",
	"lon",
	"video",
	"stick",
	"flymode",
	"draw",
	"main",
	"moron",
	"calibration",
	"command",
	"spektrum",
	"heading",
	"id",
	"uart",
	"crosshairs",
	"signature",
	"invert",
	"reset",
	"flaperon",
	"hover",
	"artificial",
	"maxalt",
	"abort",
	"character",
	"overshoot",
	"name",
	"velocity",
	"setpoint",
	"weight",
	"noise",
	"sbus",
	"idle",
	"ontime",
	"high",
	"flytimer",
	"hott_alarm_sound_interval",
	"coordinates",
	"smartport",
	"beeper",
	"tail",
	"to",
	"procedure",
	"eph",
	"user",
	"ibus",
	"units",
	"decl",
	"unidir",
	"format",
	"output",
	"sdcard",
	"rx_spi_rf_channel_count",
	"precision",
	"undershoot",
	"position",
	"velned",
	"halfduplex",
	"mode_range_logic_operator",
	"throttle_tilt_comp_str",
	"gimbal",
	"capacity",
	"unarmed",
	"inverted",
	"sat",
	"mah",
	"small",
	"baud",
	"allow",
	"fixed_wing_auto_arm",
	"drawn",
	"cap",
	"center",
	"cruise",
	"filtering",
	"shiftdown",
	"off",
	"device",
	"nav_extra_arming_safety",
	"declination",
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
	{ {153, 1, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {9000}, PG_GYRO_CONFIG, offsetof(gyroConfig_t, looptime) },
	{ {20, 127, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyroSync) },
	{ {20, 127, 105, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {1, 32}, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyroSyncDenominator) },
	{ {48, 20, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ALIGNMENT }, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_align) },
	{ {20, 39, 34, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GYRO_LPF }, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_lpf) },
	{ {20, 34, 24, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_MAX, .config.max = {200}, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_soft_lpf_hz) },
	{ {209, 1, 38, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_MAX, .config.max = {128}, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyroMovementCalibrationThreshold) },
#ifdef USE_GYRO_NOTCH_1
	{ {20, 99, 24, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {500}, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_soft_notch_hz_1) },
	{ {20, 99, 66, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1, 500}, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_soft_notch_cutoff_1) },
#endif
#ifdef USE_GYRO_NOTCH_2
	{ {20, 136, 1, 24, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {500}, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_soft_notch_hz_2) },
	{ {20, 136, 1, 66, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1, 500}, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_soft_notch_cutoff_2) },
#endif
#ifdef USE_DUAL_GYRO
	{ {20, 242, 1, 64, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 1}, PG_GYRO_CONFIG, offsetof(gyroConfig_t, gyro_to_use) },
#endif
// PG_ADC_CHANNEL_CONFIG
#ifdef USE_ADC
	{ {50, 68, 45, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {ADC_CHN_NONE, ADC_CHN_MAX}, PG_ADC_CHANNEL_CONFIG, offsetof(adcChannelConfig_t, adcFunctionChannel[ADC_BATTERY]) },
	{ {40, 68, 45, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {ADC_CHN_NONE, ADC_CHN_MAX}, PG_ADC_CHANNEL_CONFIG, offsetof(adcChannelConfig_t, adcFunctionChannel[ADC_RSSI]) },
	{ {43, 68, 45, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {ADC_CHN_NONE, ADC_CHN_MAX}, PG_ADC_CHANNEL_CONFIG, offsetof(adcChannelConfig_t, adcFunctionChannel[ADC_CURRENT]) },
	{ {126, 68, 45, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {ADC_CHN_NONE, ADC_CHN_MAX}, PG_ADC_CHANNEL_CONFIG, offsetof(adcChannelConfig_t, adcFunctionChannel[ADC_AIRSPEED]) },
#endif
// PG_ACCELEROMETER_CONFIG
#ifdef USE_ACC_NOTCH
	{ {31, 62, 24, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 255}, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, acc_notch_hz) },
	{ {31, 62, 66, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {1, 255}, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, acc_notch_cutoff) },
#endif
	{ {48, 31, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ALIGNMENT }, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, acc_align) },
	{ {31, 39, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ACC_HARDWARE }, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, acc_hardware) },
	{ {31, 34, 24, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {200}, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, acc_lpf_hz) },
	{ {88, 79, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, accZero.raw[X]) },
	{ {88, 86, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, accZero.raw[Y]) },
	{ {88, 7, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, accZero.raw[Z]) },
	{ {89, 79, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {1, 8192}, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, accGain.raw[X]) },
	{ {89, 86, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {1, 8192}, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, accGain.raw[Y]) },
	{ {89, 7, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {1, 8192}, PG_ACCELEROMETER_CONFIG, offsetof(accelerometerConfig_t, accGain.raw[Z]) },
// PG_RANGEFINDER_CONFIG
#ifdef USE_RANGEFINDER
	{ {166, 1, 39, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_RANGEFINDER_HARDWARE }, PG_RANGEFINDER_CONFIG, offsetof(rangefinderConfig_t, rangefinder_hardware) },
#endif
// PG_COMPASS_CONFIG
#ifdef MAG
	{ {48, 32, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ALIGNMENT }, PG_COMPASS_CONFIG, offsetof(compassConfig_t, mag_align) },
	{ {32, 39, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_MAG_HARDWARE }, PG_COMPASS_CONFIG, offsetof(compassConfig_t, mag_hardware) },
	{ {32, 152, 2, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-18000, 18000}, PG_COMPASS_CONFIG, offsetof(compassConfig_t, mag_declination) },
	{ {87, 79, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, PG_COMPASS_CONFIG, offsetof(compassConfig_t, magZero.raw[X]) },
	{ {87, 86, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, PG_COMPASS_CONFIG, offsetof(compassConfig_t, magZero.raw[Y]) },
	{ {87, 7, 0, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, PG_COMPASS_CONFIG, offsetof(compassConfig_t, magZero.raw[Z]) },
	{ {32, 210, 1, 30, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {30, 120}, PG_COMPASS_CONFIG, offsetof(compassConfig_t, magCalibrationTimeLimit) },
#endif
// PG_BAROMETER_CONFIG
#ifdef BARO
	{ {72, 39, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_BARO_HARDWARE }, PG_BAROMETER_CONFIG, offsetof(barometerConfig_t, baro_hardware) },
	{ {72, 64, 139, 1, 125, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_BAROMETER_CONFIG, offsetof(barometerConfig_t, use_median_filtering) },
#endif
// PG_PITOTMETER_CONFIG
#ifdef PITOT
	{ {59, 39, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_PITOT_HARDWARE }, PG_PITOTMETER_CONFIG, offsetof(pitotmeterConfig_t, pitot_hardware) },
	{ {59, 64, 139, 1, 125, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_PITOTMETER_CONFIG, offsetof(pitotmeterConfig_t, use_median_filtering) },
	{ {59, 231, 1, 34, 0, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 1}, PG_PITOTMETER_CONFIG, offsetof(pitotmeterConfig_t, pitot_noise_lpf) },
	{ {59, 73, 0, 0, 0, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 100}, PG_PITOTMETER_CONFIG, offsetof(pitotmeterConfig_t, pitot_scale) },
#endif
// PG_RX_CONFIG
	{ {103, 58, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1200, 1700}, PG_RX_CONFIG, offsetof(rxConfig_t, midrc) },
	{ {25, 113, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, PG_RX_CONFIG, offsetof(rxConfig_t, mincheck) },
	{ {19, 113, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, PG_RX_CONFIG, offsetof(rxConfig_t, maxcheck) },
	{ {40, 45, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, MAX_SUPPORTED_RC_CHANNEL_COUNT}, PG_RX_CONFIG, offsetof(rxConfig_t, rssi_channel) },
	{ {40, 73, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {RSSI_SCALE_MIN, RSSI_SCALE_MAX}, PG_RX_CONFIG, offsetof(rxConfig_t, rssi_scale) },
	{ {40, 218, 1, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_RX_CONFIG, offsetof(rxConfig_t, rssiInvert) },
	{ {58, 185, 1, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_RX_CONFIG, offsetof(rxConfig_t, rcSmoothing) },
#ifdef SERIAL_RX
	{ {123, 141, 1, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_SERIAL_RX }, PG_RX_CONFIG, offsetof(rxConfig_t, serialrx_provider) },
	{ {232, 1, 133, 1, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_RX_CONFIG, offsetof(rxConfig_t, sbus_inversion) },
#endif
#ifdef USE_RX_SPI
	{ {56, 98, 107, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_RX_SPI_PROTOCOL }, PG_RX_CONFIG, offsetof(rxConfig_t, rx_spi_protocol) },
	{ {56, 98, 214, 1, 0, 0, }, VAR_UINT32 | MASTER_VALUE, .config.minmax = {0, 0}, PG_RX_CONFIG, offsetof(rxConfig_t, rx_spi_id) },
#endif
	{ {253, 1, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 8}, PG_RX_CONFIG, offsetof(rxConfig_t, rx_spi_rf_channel_count) },
#ifdef SPEKTRUM_BIND
	{ {212, 1, 137, 2, 177, 1, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {SPEKTRUM_SAT_BIND_DISABLED, SPEKTRUM_SAT_BIND_MAX}, PG_RX_CONFIG, offsetof(rxConfig_t, spektrum_sat_bind) },
#endif
	{ {56, 25, 104, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_PULSE_MIN, PWM_PULSE_MAX}, PG_RX_CONFIG, offsetof(rxConfig_t, rx_min_usec) },
	{ {56, 19, 104, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_PULSE_MIN, PWM_PULSE_MAX}, PG_RX_CONFIG, offsetof(rxConfig_t, rx_max_usec) },
#ifdef STM32F4
	{ {123, 130, 2, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_RX_CONFIG, offsetof(rxConfig_t, halfDuplex) },
#endif
// PG_BLACKBOX_CONFIG
#ifdef BLACKBOX
	{ {91, 11, 115, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {1, 255}, PG_BLACKBOX_CONFIG, offsetof(blackboxConfig_t, rate_num) },
	{ {91, 11, 105, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {1, 255}, PG_BLACKBOX_CONFIG, offsetof(blackboxConfig_t, rate_denom) },
	{ {91, 150, 2, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_BLACKBOX_DEVICE }, PG_BLACKBOX_CONFIG, offsetof(blackboxConfig_t, device) },
#ifdef USE_SDCARD
	{ {252, 1, 140, 1, 136, 2, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_BLACKBOX_CONFIG, offsetof(blackboxConfig_t, invertedCardDetection) },
#endif
#endif
// PG_MOTOR_CONFIG
	{ {25, 47, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, PG_MOTOR_CONFIG, offsetof(motorConfig_t, minthrottle) },
	{ {19, 47, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, PG_MOTOR_CONFIG, offsetof(motorConfig_t, maxthrottle) },
	{ {25, 211, 1, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, PG_MOTOR_CONFIG, offsetof(motorConfig_t, mincommand) },
	{ {65, 81, 11, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {50, 32000}, PG_MOTOR_CONFIG, offsetof(motorConfig_t, motorPwmRate) },
	{ {65, 81, 107, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_MOTOR_PWM_PROTOCOL }, PG_MOTOR_CONFIG, offsetof(motorConfig_t, motorPwmProtocol) },
// PG_FAILSAFE_CONFIG
	{ {21, 23, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 200}, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_delay) },
	{ {21, 157, 1, 23, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 200}, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_recovery_delay) },
	{ {21, 149, 2, 23, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 200}, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_off_delay) },
	{ {21, 47, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 2000}, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_throttle) },
	{ {21, 47, 97, 23, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 300}, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_throttle_low_delay) },
	{ {21, 243, 1, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_FAILSAFE_PROCEDURE }, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_procedure) },
	{ {21, 205, 1, 38, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 500}, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_stick_motion_threshold) },
	{ {21, 2, 16, 13, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-800, 800}, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_fw_roll_angle) },
	{ {21, 2, 17, 13, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-800, 800}, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_fw_pitch_angle) },
	{ {21, 2, 8, 11, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-1000, 1000}, PG_FAILSAFE_CONFIG, offsetof(failsafeConfig_t, failsafe_fw_yaw_rate) },
// PG_BOARD_ALIGNMENT
	{ {48, 80, 16, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-1800, 3600}, PG_BOARD_ALIGNMENT, offsetof(boardAlignment_t, rollDeciDegrees) },
	{ {48, 80, 17, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-1800, 3600}, PG_BOARD_ALIGNMENT, offsetof(boardAlignment_t, pitchDeciDegrees) },
	{ {48, 80, 8, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-1800, 3600}, PG_BOARD_ALIGNMENT, offsetof(boardAlignment_t, yawDeciDegrees) },
// PG_GIMBAL_CONFIG
#ifdef USE_SERVOS
	{ {133, 2, 28, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GIMBAL_MODE }, PG_GIMBAL_CONFIG, offsetof(gimbalConfig_t, mode) },
#endif
// PG_BATTERY_CONFIG
	{ {200, 1, 134, 2, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 20000}, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, batteryCapacity) },
#ifdef USE_ADC
	{ {50, 73, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {VBAT_SCALE_MIN, VBAT_SCALE_MAX}, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, vbatscale) },
	{ {50, 19, 57, 52, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {10, 50}, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, vbatmaxcellvoltage) },
	{ {50, 25, 57, 52, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {10, 50}, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, vbatmincellvoltage) },
	{ {50, 152, 1, 57, 52, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {10, 50}, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, vbatwarningcellvoltage) },
#endif
	{ {43, 70, 73, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {-10000, 10000}, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, currentMeterScale) },
	{ {43, 70, 135, 1, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 3300}, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, currentMeterOffset) },
	{ {191, 1, 43, 70, 251, 1, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, multiwiiCurrentMeterOutput) },
	{ {43, 70, 134, 1, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_CURRENT_SENSOR }, PG_BATTERY_CONFIG, offsetof(batteryConfig_t, currentMeterType) },
// PG_MIXER_CONFIG
	{ {8, 65, 168, 1, 0, 0, }, VAR_INT8 | MASTER_VALUE, .config.minmax = {-1, 1}, PG_MIXER_CONFIG, offsetof(mixerConfig_t, yaw_motor_direction) },
	{ {8, 161, 1, 150, 1, 29, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {YAW_JUMP_PREVENTION_LIMIT_LOW, YAW_JUMP_PREVENTION_LIMIT_HIGH}, PG_MIXER_CONFIG, offsetof(mixerConfig_t, yaw_jump_prevention_limit) },
// PG_MOTOR_3D_CONFIG
	{ {60, 37, 97, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, PG_MOTOR_3D_CONFIG, offsetof(flight3DConfig_t, deadband3d_low) },
	{ {60, 37, 235, 1, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, PG_MOTOR_3D_CONFIG, offsetof(flight3DConfig_t, deadband3d_high) },
	{ {60, 144, 1, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, PG_MOTOR_3D_CONFIG, offsetof(flight3DConfig_t, neutral3d) },
// PG_SERVO_CONFIG
#ifdef USE_SERVOS
	{ {53, 145, 2, 175, 1, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, PG_SERVO_CONFIG, offsetof(servoConfig_t, servoCenterPulse) },
	{ {53, 81, 11, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {50, 498}, PG_SERVO_CONFIG, offsetof(servoConfig_t, servoPwmRate) },
	{ {53, 34, 24, 0, 0, 0, }, VAR_INT16 | MASTER_VALUE, .config.minmax = {0, 400}, PG_SERVO_CONFIG, offsetof(servoConfig_t, servo_lowpass_freq) },
	{ {220, 1, 120, 135, 1, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {FLAPERON_THROW_MIN, FLAPERON_THROW_MAX}, PG_SERVO_CONFIG, offsetof(servoConfig_t, flaperon_throw_offset) },
	{ {176, 1, 135, 2, 53, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_SERVO_CONFIG, offsetof(servoConfig_t, tri_unarmed_servo) },
#endif
// PG_CONTROL_RATE_PROFILES
	{ {58, 75, 0, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {0, 100}, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, rcExpo8) },
	{ {58, 8, 75, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {0, 100}, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, rcYawExpo8) },
	{ {27, 103, 0, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {0, 100}, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, thrMid8) },
	{ {27, 75, 0, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {0, 100}, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, thrExpo8) },
	{ {16, 11, 0, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {CONTROL_RATE_CONFIG_ROLL_PITCH_RATE_MIN, CONTROL_RATE_CONFIG_ROLL_PITCH_RATE_MAX}, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, rates[FD_ROLL]) },
	{ {17, 11, 0, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {CONTROL_RATE_CONFIG_ROLL_PITCH_RATE_MIN, CONTROL_RATE_CONFIG_ROLL_PITCH_RATE_MAX}, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, rates[FD_PITCH]) },
	{ {8, 11, 0, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {CONTROL_RATE_CONFIG_YAW_RATE_MIN, CONTROL_RATE_CONFIG_YAW_RATE_MAX}, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, rates[FD_YAW]) },
	{ {117, 11, 0, 0, 0, 0, }, VAR_UINT8 | CONTROL_RATE_VALUE, .config.minmax = {0, CONTROL_RATE_CONFIG_TPA_MAX}, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, dynThrPID) },
	{ {117, 184, 1, 0, 0, 0, }, VAR_UINT16 | CONTROL_RATE_VALUE, .config.minmax = {PWM_RANGE_MIN, PWM_RANGE_MAX}, PG_CONTROL_RATE_PROFILES, offsetof(controlRateConfig_t, tpa_breakpoint) },
// PG_SERIAL_CONFIG
	{ {164, 1, 225, 1, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {48, 126}, PG_SERIAL_CONFIG, offsetof(serialConfig_t, reboot_character) },
// PG_IMU_CONFIG
	{ {54, 71, 111, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {UINT16_MAX}, PG_IMU_CONFIG, offsetof(imuConfig_t, dcm_kp_acc) },
	{ {54, 71, 112, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {UINT16_MAX}, PG_IMU_CONFIG, offsetof(imuConfig_t, dcm_ki_acc) },
	{ {54, 71, 111, 32, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {UINT16_MAX}, PG_IMU_CONFIG, offsetof(imuConfig_t, dcm_kp_mag) },
	{ {54, 71, 112, 32, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {UINT16_MAX}, PG_IMU_CONFIG, offsetof(imuConfig_t, dcm_ki_mag) },
	{ {139, 2, 13, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 180}, PG_IMU_CONFIG, offsetof(imuConfig_t, small_angle) },
// PG_ARMING_CONFIG
	{ {142, 2, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_ARMING_CONFIG, offsetof(armingConfig_t, fixed_wing_auto_arm) },
	{ {61, 183, 1, 109, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_ARMING_CONFIG, offsetof(armingConfig_t, disarm_kill_switch) },
	{ {35, 61, 23, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 60}, PG_ARMING_CONFIG, offsetof(armingConfig_t, auto_disarm_delay) },
// PG_GPS_CONFIG
#ifdef GPS
	{ {10, 141, 1, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GPS_PROVIDER }, PG_GPS_CONFIG, offsetof(gpsConfig_t, provider) },
	{ {10, 169, 1, 28, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GPS_SBAS_MODE }, PG_GPS_CONFIG, offsetof(gpsConfig_t, sbasMode) },
	{ {10, 149, 1, 182, 1, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GPS_DYN_MODEL }, PG_GPS_CONFIG, offsetof(gpsConfig_t, dynModel) },
	{ {10, 35, 190, 1, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_GPS_CONFIG, offsetof(gpsConfig_t, autoConfig) },
	{ {10, 35, 140, 2, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_GPS_CONFIG, offsetof(gpsConfig_t, autoBaud) },
	{ {10, 25, 132, 1, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {5, 10}, PG_GPS_CONFIG, offsetof(gpsConfig_t, gpsMinSats) },
#endif
// PG_RC_CONTROLS_CONFIG
	{ {37, 0, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 32}, PG_RC_CONTROLS_CONFIG, offsetof(rcControlsConfig_t, deadband) },
	{ {8, 37, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 100}, PG_RC_CONTROLS_CONFIG, offsetof(rcControlsConfig_t, yaw_deadband) },
	{ {4, 82, 37, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {10, 250}, PG_RC_CONTROLS_CONFIG, offsetof(rcControlsConfig_t, pos_hold_deadband) },
	{ {92, 82, 37, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {10, 250}, PG_RC_CONTROLS_CONFIG, offsetof(rcControlsConfig_t, alt_hold_deadband) },
	{ {60, 37, 47, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, PG_RC_CONTROLS_CONFIG, offsetof(rcControlsConfig_t, deadband3d_throttle) },
// PG_PID_PROFILE
	{ {5, 6, 17, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_PITCH].P) },
	{ {5, 12, 17, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_PITCH].I) },
	{ {5, 15, 17, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_PITCH].D) },
	{ {5, 6, 16, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_ROLL].P) },
	{ {5, 12, 16, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_ROLL].I) },
	{ {5, 15, 16, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_ROLL].D) },
	{ {5, 6, 8, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_YAW].P) },
	{ {5, 12, 8, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_YAW].I) },
	{ {5, 15, 8, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_YAW].D) },
	{ {5, 6, 44, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_LEVEL].P) },
	{ {5, 12, 44, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_LEVEL].I) },
	{ {5, 15, 44, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_LEVEL].D) },
	{ {2, 6, 17, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_PITCH].P) },
	{ {2, 12, 17, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_PITCH].I) },
	{ {2, 77, 17, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_PITCH].D) },
	{ {2, 6, 16, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_ROLL].P) },
	{ {2, 12, 16, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_ROLL].I) },
	{ {2, 77, 16, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_ROLL].D) },
	{ {2, 6, 8, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_YAW].P) },
	{ {2, 12, 8, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_YAW].I) },
	{ {2, 77, 8, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_YAW].D) },
	{ {2, 6, 44, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_LEVEL].P) },
	{ {2, 12, 44, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_LEVEL].I) },
	{ {2, 15, 44, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_LEVEL].D) },
	{ {19, 13, 138, 1, 196, 1, }, VAR_INT16 | PROFILE_VALUE, .config.minmax = {100, 900}, PG_PID_PROFILE, offsetof(pidProfile_t, max_angle_inclination[FD_ROLL]) },
	{ {19, 13, 138, 1, 197, 1, }, VAR_INT16 | PROFILE_VALUE, .config.minmax = {100, 900}, PG_PID_PROFILE, offsetof(pidProfile_t, max_angle_inclination[FD_PITCH]) },
	{ {67, 34, 24, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, dterm_lpf_hz) },
	{ {8, 34, 24, 0, 0, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 200}, PG_PID_PROFILE, offsetof(pidProfile_t, yaw_lpf_hz) },
	{ {67, 229, 1, 230, 1, 0, }, VAR_FLOAT | PROFILE_VALUE, .config.minmax = {0, 2}, PG_PID_PROFILE, offsetof(pidProfile_t, dterm_setpoint_weight) },
#ifdef USE_SERVOS
	{ {2, 85, 120, 29, 0, 0, }, VAR_UINT16 | PROFILE_VALUE, .config.minmax = {FW_ITERM_THROW_LIMIT_MIN, FW_ITERM_THROW_LIMIT_MAX}, PG_PID_PROFILE, offsetof(pidProfile_t, fixedWingItermThrowLimit) },
	{ {2, 180, 1, 126, 0, 0, }, VAR_FLOAT | PROFILE_VALUE, .config.minmax = {1, 5000}, PG_PID_PROFILE, offsetof(pidProfile_t, fixedWingReferenceAirspeed) },
	{ {163, 1, 0, 0, 0, 0, }, VAR_FLOAT | PROFILE_VALUE, .config.minmax = {0, 2}, PG_PID_PROFILE, offsetof(pidProfile_t, fixedWingCoordinatedYawGain) },
#endif
#ifdef USE_DTERM_NOTCH
	{ {67, 62, 24, 0, 0, 0, }, VAR_UINT16 | PROFILE_VALUE, .config.minmax = {0, 500}, PG_PID_PROFILE, offsetof(pidProfile_t, dterm_soft_notch_hz) },
	{ {67, 62, 66, 0, 0, 0, }, VAR_UINT16 | PROFILE_VALUE, .config.minmax = {1, 500}, PG_PID_PROFILE, offsetof(pidProfile_t, dterm_soft_notch_cutoff) },
#endif
	{ {148, 1, 29, 0, 0, 0, }, VAR_UINT16 | PROFILE_VALUE, .config.minmax = {PID_SUM_LIMIT_MIN, PID_SUM_LIMIT_MAX}, PG_PID_PROFILE, offsetof(pidProfile_t, pidSumLimit) },
	{ {8, 6, 29, 0, 0, 0, }, VAR_UINT16 | PROFILE_VALUE, .config.minmax = {YAW_P_LIMIT_MIN, YAW_P_LIMIT_MAX}, PG_PID_PROFILE, offsetof(pidProfile_t, yaw_p_limit) },
	{ {85, 95, 38, 0, 0, 0, }, VAR_UINT16 | PROFILE_VALUE, .config.minmax = {15, 1000}, PG_PID_PROFILE, offsetof(pidProfile_t, rollPitchItermIgnoreRate) },
	{ {8, 85, 95, 38, 0, 0, }, VAR_UINT16 | PROFILE_VALUE, .config.minmax = {15, 10000}, PG_PID_PROFILE, offsetof(pidProfile_t, yawItermIgnoreRate) },
	{ {11, 76, 29, 16, 17, 0, }, VAR_UINT32 | PROFILE_VALUE | MODE_MAX, .config.max = {500000}, PG_PID_PROFILE, offsetof(pidProfile_t, axisAccelerationLimitRollPitch) },
	{ {11, 76, 29, 8, 0, 0, }, VAR_UINT32 | PROFILE_VALUE | MODE_MAX, .config.max = {500000}, PG_PID_PROFILE, offsetof(pidProfile_t, axisAccelerationLimitYaw) },
	{ {213, 1, 82, 11, 29, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {HEADING_HOLD_RATE_LIMIT_MIN, HEADING_HOLD_RATE_LIMIT_MAX}, PG_PID_PROFILE, offsetof(pidProfile_t, heading_hold_rate_limit) },
#ifdef NAV
	{ {1, 5, 4, 7, 6, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_POS_Z].P) },
	{ {1, 5, 4, 7, 12, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_POS_Z].I) },
	{ {1, 5, 4, 7, 15, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_POS_Z].D) },
	{ {1, 5, 46, 7, 6, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_Z].P) },
	{ {1, 5, 46, 7, 12, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_Z].I) },
	{ {1, 5, 46, 7, 15, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_Z].D) },
	{ {1, 5, 4, 14, 6, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_POS_XY].P) },
	{ {1, 5, 4, 14, 12, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_POS_XY].I) },
	{ {1, 5, 4, 14, 15, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_POS_XY].D) },
	{ {1, 5, 46, 14, 6, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_XY].P) },
	{ {1, 5, 46, 14, 12, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_XY].I) },
	{ {1, 5, 46, 14, 15, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_mc.pid[PID_VEL_XY].D) },
	{ {1, 2, 4, 7, 6, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_POS_Z].P) },
	{ {1, 2, 4, 7, 12, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_POS_Z].I) },
	{ {1, 2, 4, 7, 15, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_POS_Z].D) },
	{ {1, 2, 4, 14, 6, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_POS_XY].P) },
	{ {1, 2, 4, 14, 12, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_POS_XY].I) },
	{ {1, 2, 4, 14, 15, 0, }, VAR_UINT8 | PROFILE_VALUE, .config.minmax = {0, 255}, PG_PID_PROFILE, offsetof(pidProfile_t, bank_fw.pid[PID_POS_XY].D) },
#endif
// PG_PID_AUTOTUNE_CONFIG
#ifdef NAV
	{ {2, 78, 226, 1, 30, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {50, 500}, PG_PID_AUTOTUNE_CONFIG, offsetof(pidAutotuneConfig_t, fw_overshoot_time) },
	{ {2, 78, 255, 1, 30, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {50, 500}, PG_PID_AUTOTUNE_CONFIG, offsetof(pidAutotuneConfig_t, fw_undershoot_time) },
	{ {2, 78, 38, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 100}, PG_PID_AUTOTUNE_CONFIG, offsetof(pidAutotuneConfig_t, fw_max_rate_threshold) },
	{ {146, 1, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 100}, PG_PID_AUTOTUNE_CONFIG, offsetof(pidAutotuneConfig_t, fw_ff_to_p_gain) },
	{ {170, 1, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 5000}, PG_PID_AUTOTUNE_CONFIG, offsetof(pidAutotuneConfig_t, fw_ff_to_i_time_constant) },
#endif
// PG_POSITION_ESTIMATION_CONFIG
#ifdef NAV
#ifdef NAV_AUTO_MAG_DECLINATION
	{ {9, 35, 32, 248, 1, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, automatic_mag_declination) },
#endif
	{ {167, 1, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 255}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, gravity_calibration_tolerance) },
	{ {9, 64, 10, 129, 2, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, use_gps_velned) },
	{ {9, 10, 23, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 500}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, gps_delay_ms) },
	{ {9, 219, 1, 69, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_RESET_ALTITUDE }, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, reset_altitude_type) },
	{ {9, 19, 93, 69, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 1000}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, max_surface_altitude) },
	{ {9, 18, 7, 93, 6, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_z_surface_p) },
	{ {9, 18, 7, 93, 51, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_z_surface_v) },
	{ {9, 18, 7, 72, 6, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_z_baro_p) },
	{ {9, 18, 7, 10, 6, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_z_gps_p) },
	{ {9, 18, 7, 10, 51, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_z_gps_v) },
	{ {9, 18, 14, 10, 6, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_xy_gps_p) },
	{ {9, 18, 14, 10, 51, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_xy_gps_v) },
	{ {9, 18, 7, 137, 1, 51, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_z_res_v) },
	{ {9, 18, 14, 137, 1, 51, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 10}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_xy_res_v) },
	{ {9, 18, 31, 195, 1, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 1}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, w_acc_bias) },
	{ {9, 19, 244, 1, 101, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 9999}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, max_eph_epv) },
	{ {9, 72, 101, 0, 0, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 9999}, PG_POSITION_ESTIMATION_CONFIG, offsetof(positionEstimationConfig_t, baro_epv) },
#endif
// PG_NAV_CONFIG
#ifdef NAV
	{ {1, 61, 202, 1, 63, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.disarm_on_landing) },
	{ {147, 1, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.use_thr_mid_for_althold) },
	{ {151, 2, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.extra_arming_safety) },
	{ {1, 245, 1, 171, 1, 28, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_NAV_USER_CONTROL_MODE }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.user_control_mode) },
	{ {1, 128, 2, 100, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 10}, PG_NAV_CONFIG, offsetof(navConfig_t, general.pos_failure_timeout) },
	{ {1, 110, 124, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {10, 10000}, PG_NAV_CONFIG, offsetof(navConfig_t, general.waypoint_radius) },
	{ {1, 110, 188, 1, 122, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {65000}, PG_NAV_CONFIG, offsetof(navConfig_t, general.waypoint_safe_distance) },
	{ {1, 35, 33, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {10, 2000}, PG_NAV_CONFIG, offsetof(navConfig_t, general.max_auto_speed) },
	{ {1, 35, 41, 11, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {10, 2000}, PG_NAV_CONFIG, offsetof(navConfig_t, general.max_auto_climb_rate) },
	{ {1, 118, 33, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {10, 2000}, PG_NAV_CONFIG, offsetof(navConfig_t, general.max_manual_speed) },
	{ {1, 118, 41, 11, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {10, 2000}, PG_NAV_CONFIG, offsetof(navConfig_t, general.max_manual_climb_rate) },
	{ {1, 63, 33, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {100, 2000}, PG_NAV_CONFIG, offsetof(navConfig_t, general.land_descent_rate) },
	{ {1, 94, 96, 181, 1, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {50, 1000}, PG_NAV_CONFIG, offsetof(navConfig_t, general.land_slowdown_minalt) },
	{ {1, 94, 96, 223, 1, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {500, 4000}, PG_NAV_CONFIG, offsetof(navConfig_t, general.land_slowdown_maxalt) },
	{ {1, 128, 1, 63, 33, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {100, 2000}, PG_NAV_CONFIG, offsetof(navConfig_t, general.emerg_descent_rate) },
	{ {1, 25, 26, 122, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 5000}, PG_NAV_CONFIG, offsetof(navConfig_t, general.min_rth_distance) },
	{ {1, 26, 41, 142, 1, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.rth_climb_first) },
	{ {1, 26, 41, 95, 128, 1, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.rth_climb_ignore_emerg) },
	{ {1, 26, 241, 1, 142, 1, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.rth_tail_first) },
	{ {1, 26, 141, 2, 63, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.rth_allow_landing) },
	{ {1, 26, 92, 28, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_NAV_RTH_ALT_MODE }, PG_NAV_CONFIG, offsetof(navConfig_t, general.flags.rth_alt_control_mode) },
	{ {1, 26, 224, 1, 38, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {65000}, PG_NAV_CONFIG, offsetof(navConfig_t, general.rth_abort_threshold) },
	{ {1, 26, 69, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {65000}, PG_NAV_CONFIG, offsetof(navConfig_t, general.rth_altitude) },
	{ {1, 5, 129, 1, 13, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {15, 45}, PG_NAV_CONFIG, offsetof(navConfig_t, mc.max_bank_angle) },
	{ {1, 5, 221, 1, 27, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 2000}, PG_NAV_CONFIG, offsetof(navConfig_t, mc.hover_throttle) },
	{ {1, 5, 35, 61, 23, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {100, 10000}, PG_NAV_CONFIG, offsetof(navConfig_t, mc.auto_disarm_delay) },
	{ {1, 2, 146, 2, 27, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 2000}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.cruise_throttle) },
	{ {1, 2, 25, 27, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 2000}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.min_throttle) },
	{ {1, 2, 19, 27, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 2000}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.max_throttle) },
	{ {1, 2, 129, 1, 13, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {5, 80}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.max_bank_angle) },
	{ {1, 2, 41, 13, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {5, 80}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.max_climb_angle) },
	{ {1, 2, 108, 13, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {5, 80}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.max_dive_angle) },
	{ {1, 2, 159, 1, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 100}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.pitch_to_throttle) },
	{ {1, 2, 154, 1, 124, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 10000}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.loiter_radius) },
#ifdef FIXED_WING_LANDING
	{ {1, 2, 94, 108, 13, 0, }, VAR_INT8 | MASTER_VALUE, .config.minmax = {-20, 20}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.land_dive_angle) },
#endif
	{ {1, 2, 22, 228, 1, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {100, 10000}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_velocity_thresh) },
	{ {1, 2, 22, 76, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 20000}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_accel_thresh) },
	{ {1, 2, 22, 19, 13, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {5, 180}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_max_angle) },
	{ {1, 2, 22, 140, 1, 30, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {10, 1000}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_time_thresh) },
	{ {1, 2, 22, 27, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 2000}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_throttle) },
	{ {1, 2, 22, 233, 1, 27, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {1000, 2000}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_idle_throttle) },
	{ {1, 2, 22, 65, 23, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 5000}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_motor_timer) },
	{ {1, 2, 22, 172, 1, 30, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 1000}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_motor_spinup_time) },
	{ {1, 2, 22, 100, 0, 0, }, VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {60000}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_timeout) },
	{ {1, 2, 22, 41, 13, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {5, 45}, PG_NAV_CONFIG, offsetof(navConfig_t, fw.launch_climb_angle) },
#endif
// PG_TELEMETRY_CONFIG
#ifdef TELEMETRY
	{ {49, 109, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, telemetry_switch) },
	{ {49, 133, 1, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, telemetry_inversion) },
	{ {42, 114, 151, 1, 0, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {-90, 90}, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, gpsNoFixLatitude) },
	{ {42, 114, 174, 1, 0, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {-180, 180}, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, gpsNoFixLongitude) },
	{ {42, 238, 1, 250, 1, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, FRSKY_FORMAT_NMEA}, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, frsky_coordinate_format) },
	{ {42, 187, 1, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_UNIT }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, frsky_unit) },
	{ {42, 106, 254, 1, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {FRSKY_VFAS_PRECISION_LOW, FRSKY_VFAS_PRECISION_HIGH}, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, frsky_vfas_precision) },
	{ {42, 106, 57, 52, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, frsky_vfas_cell_voltage) },
	{ {237, 1, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 120}, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, hottAlarmSoundInterval) },
#ifdef TELEMETRY_SMARTPORT
	{ {239, 1, 215, 1, 249, 1, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, smartportUartUnidirectional) },
#endif
	{ {246, 1, 49, 134, 1, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 255}, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, ibusTelemetryType) },
#ifdef TELEMETRY_LTM
	{ {160, 1, 173, 1, 11, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_LTM_RATES }, PG_TELEMETRY_CONFIG, offsetof(telemetryConfig_t, ltmUpdateRate) },
#endif
#endif
// PG_ELERES_CONFIG
#ifdef USE_RX_ELERES
	{ {36, 155, 1, 0, 0, 0, }, VAR_FLOAT | MASTER_VALUE, .config.minmax = {415, 450}, PG_ELERES_CONFIG, offsetof(eleresConfig_t, eleresFreq) },
	{ {36, 49, 130, 1, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_ELERES_CONFIG, offsetof(eleresConfig_t, eleresTelemetryEn) },
	{ {36, 49, 84, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 7}, PG_ELERES_CONFIG, offsetof(eleresConfig_t, eleresTelemetryPower) },
	{ {36, 90, 130, 1, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_ELERES_CONFIG, offsetof(eleresConfig_t, eleresLocEn) },
	{ {36, 90, 84, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 7}, PG_ELERES_CONFIG, offsetof(eleresConfig_t, eleresLocPower) },
	{ {36, 90, 23, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {30, 1800}, PG_ELERES_CONFIG, offsetof(eleresConfig_t, eleresLocDelay) },
	{ {36, 217, 1, 0, 0, 0, }, VAR_UINT32 | MASTER_VALUE | MODE_MAX, .config.max = {UINT32_MAX}, PG_ELERES_CONFIG, offsetof(eleresConfig_t, eleresSignature) },
#endif
// PG_LED_STRIP_CONFIG
#ifdef LED_STRIP
	{ {192, 1, 201, 1, 240, 1, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_LED_STRIP_CONFIG, offsetof(ledStripConfig_t, ledstrip_visual_beeper) },
#endif
// PG_OSD_CONFIG
#ifdef OSD
	{ {3, 204, 1, 198, 1, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 2}, PG_OSD_CONFIG, offsetof(osdConfig_t, video_system) },
	{ {3, 165, 1, 148, 2, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 1}, PG_OSD_CONFIG, offsetof(osdConfig_t, row_shiftdown) },
	{ {3, 247, 1, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_UNIT }, PG_OSD_CONFIG, offsetof(osdConfig_t, units) },
	{ {3, 40, 55, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 100}, PG_OSD_CONFIG, offsetof(osdConfig_t, rssi_alarm) },
	{ {3, 144, 2, 55, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 20000}, PG_OSD_CONFIG, offsetof(osdConfig_t, cap_alarm) },
	{ {3, 30, 55, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 60}, PG_OSD_CONFIG, offsetof(osdConfig_t, time_alarm) },
	{ {3, 92, 55, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, 10000}, PG_OSD_CONFIG, offsetof(osdConfig_t, alt_alarm) },
	{ {3, 208, 1, 52, 4, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_MAIN_BATT_VOLTAGE) },
	{ {3, 40, 4, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_RSSI_VALUE]) },
	{ {3, 236, 1, 4, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_FLYTIME]) },
	{ {3, 234, 1, 4, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_ONTIME]) },
	{ {3, 206, 1, 4, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_FLYMODE]) },
	{ {3, 47, 4, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_THROTTLE_POS]) },
	{ {3, 162, 1, 45, 4, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_VTX_CHANNEL]) },
	{ {3, 216, 1, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_CROSSHAIRS]) },
	{ {3, 222, 1, 178, 1, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_ARTIFICIAL_HORIZON]) },
	{ {3, 43, 207, 1, 4, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_CURRENT_DRAW]) },
	{ {3, 138, 2, 143, 2, 4, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_MAH_DRAWN]) },
	{ {3, 193, 1, 227, 1, 4, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_CRAFT_NAME]) },
	{ {3, 10, 33, 4, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_GPS_SPEED]) },
	{ {3, 10, 132, 1, 4, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_GPS_SATS]) },
	{ {3, 10, 203, 1, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_GPS_LON]) },
	{ {3, 10, 186, 1, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_GPS_LAT]) },
	{ {3, 121, 145, 1, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_HOME_DIR]) },
	{ {3, 121, 102, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_HOME_DIST]) },
	{ {3, 69, 4, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_ALTITUDE]) },
	{ {3, 116, 0, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_VARIO]) },
	{ {3, 116, 115, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_VARIO_NUM]) },
	{ {3, 74, 16, 4, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_ROLL_PIDS]) },
	{ {3, 74, 17, 4, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_PITCH_PIDS]) },
	{ {3, 74, 8, 4, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_YAW_PIDS]) },
	{ {3, 84, 4, 0, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_POWER]) },
	{ {3, 194, 1, 33, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {0, OSD_POS_MAX_CLI}, PG_OSD_CONFIG, offsetof(osdConfig_t, item_pos[OSD_AIR_SPEED]) },
#endif
// PG_SYSTEM_CONFIG
#ifdef USE_I2C
	{ {156, 1, 33, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_I2C_SPEED }, PG_SYSTEM_CONFIG, offsetof(systemConfig_t, i2c_speed) },
#endif
	{ {199, 1, 28, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_DEBUG_MODES }, PG_SYSTEM_CONFIG, offsetof(systemConfig_t, debug_mode) },
#ifdef ASYNC_GYRO_PROCESSING
	{ {31, 119, 131, 1, 0, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {ACC_TASK_FREQUENCY_MIN, ACC_TASK_FREQUENCY_MAX}, PG_SYSTEM_CONFIG, offsetof(systemConfig_t, accTaskFrequency) },
	{ {189, 1, 119, 131, 1, 0, }, VAR_UINT16 | MASTER_VALUE, .config.minmax = {ATTITUDE_TASK_FREQUENCY_MIN, ATTITUDE_TASK_FREQUENCY_MAX}, PG_SYSTEM_CONFIG, offsetof(systemConfig_t, attitudeTaskFrequency) },
	{ {158, 1, 28, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ASYNC_MODE }, PG_SYSTEM_CONFIG, offsetof(systemConfig_t, asyncMode) },
#endif
	{ {132, 2, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 100}, PG_SYSTEM_CONFIG, offsetof(systemConfig_t, throttle_tilt_compensation_strength) },
	{ {179, 1, 147, 2, 28, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_SYSTEM_CONFIG, offsetof(systemConfig_t, pwmRxInputFilteringMode) },
// PG_MODE_ACTIVATION_OPERATOR_CONFIG
	{ {131, 2, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_AUX_OPERATOR }, PG_MODE_ACTIVATION_OPERATOR_CONFIG, offsetof(modeActivationOperatorConfig_t, modeActivationOperator) },
// PG_STATS_CONFIG
#ifdef STATS
	{ {83, 0, 0, 0, 0, 0, }, VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, PG_STATS_CONFIG, offsetof(statsConfig_t, stats_enabled) },
	{ {83, 143, 1, 30, 0, 0, }, VAR_UINT32 | MASTER_VALUE | MODE_MAX, .config.max = {INT32_MAX}, PG_STATS_CONFIG, offsetof(statsConfig_t, stats_total_time) },
	{ {83, 143, 1, 102, 0, 0, }, VAR_UINT32 | MASTER_VALUE | MODE_MAX, .config.max = {INT32_MAX}, PG_STATS_CONFIG, offsetof(statsConfig_t, stats_total_dist) },
#endif
};
