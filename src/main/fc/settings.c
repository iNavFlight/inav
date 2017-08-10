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
static const char *table_gyro_lpf[] = {
	"256HZ",
	"188HZ",
	"98HZ",
	"42HZ",
	"20HZ",
	"10HZ",
};
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
enum {
	TABLE_ACC_HARDWARE,
	TABLE_ALIGNMENT,
#if defined(BARO)
	TABLE_BARO_HARDWARE,
#endif
	TABLE_GYRO_LPF,
#if defined(MAG)
	TABLE_MAG_HARDWARE,
#endif
	TABLE_OFF_ON,
#if defined(PITOT)
	TABLE_PITOT_HARDWARE,
#endif
#if defined(USE_RANGEFINDER)
	TABLE_RANGEFINDER_HARDWARE,
#endif
#if defined(USE_RX_SPI)
	TABLE_RX_SPI_PROTOCOL,
#endif
#if defined(SERIAL_RX)
	TABLE_SERIAL_RX,
#endif
	LOOKUP_TABLE_COUNT,
};
static const lookupTableEntry_t lookupTables[] = {
	{ table_acc_hardware, sizeof(table_acc_hardware) / sizeof(char*) },
	{ table_alignment, sizeof(table_alignment) / sizeof(char*) },
#if defined(BARO)
	{ table_baro_hardware, sizeof(table_baro_hardware) / sizeof(char*) },
#endif
	{ table_gyro_lpf, sizeof(table_gyro_lpf) / sizeof(char*) },
#if defined(MAG)
	{ table_mag_hardware, sizeof(table_mag_hardware) / sizeof(char*) },
#endif
	{ table_off_on, sizeof(table_off_on) / sizeof(char*) },
#if defined(PITOT)
	{ table_pitot_hardware, sizeof(table_pitot_hardware) / sizeof(char*) },
#endif
#if defined(USE_RANGEFINDER)
	{ table_rangefinder_hardware, sizeof(table_rangefinder_hardware) / sizeof(char*) },
#endif
#if defined(USE_RX_SPI)
	{ table_rx_spi_protocol, sizeof(table_rx_spi_protocol) / sizeof(char*) },
#endif
#if defined(SERIAL_RX)
	{ table_serial_rx, sizeof(table_serial_rx) / sizeof(char*) },
#endif
};
const clivalue_t valueTable[] = {
// PG_GYRO_CONFIG
	{ "looptime", VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {9000}, offsetof(gyroConfig_t, looptime) },
	{ "gyro_sync", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(gyroConfig_t, gyroSync) },
	{ "gyro_sync_denom", VAR_UINT8 | MASTER_VALUE, .config.minmax = {1, 32}, offsetof(gyroConfig_t, gyroSyncDenominator) },
	{ "align_gyro", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ALIGNMENT }, offsetof(gyroConfig_t, gyro_align) },
	{ "gyro_hardware_lpf", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_GYRO_LPF }, offsetof(gyroConfig_t, gyro_lpf) },
	{ "gyro_lpf_hz", VAR_UINT8 | MASTER_VALUE | MODE_MAX, .config.max = {200}, offsetof(gyroConfig_t, gyro_soft_lpf_hz) },
	{ "moron_threshold", VAR_UINT8 | MASTER_VALUE | MODE_MAX, .config.max = {128}, offsetof(gyroConfig_t, gyroMovementCalibrationThreshold) },
#ifdef USE_GYRO_NOTCH_1
	{ "gyro_notch1_hz", VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {500}, offsetof(gyroConfig_t, gyro_soft_notch_hz_1) },
	{ "gyro_notch1_cutoff", VAR_UINT16 | MASTER_VALUE, .config.minmax = {1, 500}, offsetof(gyroConfig_t, gyro_soft_notch_cutoff_1) },
#endif
#ifdef USE_GYRO_NOTCH_2
	{ "gyro_notch2_hz", VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {500}, offsetof(gyroConfig_t, gyro_soft_notch_hz_2) },
	{ "gyro_notch2_cutoff", VAR_UINT16 | MASTER_VALUE, .config.minmax = {1, 500}, offsetof(gyroConfig_t, gyro_soft_notch_cutoff_2) },
#endif
#ifdef USE_DUAL_GYRO
	{ "gyro_to_use", VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 1}, offsetof(gyroConfig_t, gyro_to_use) },
#endif
// PG_ADC_CHANNEL_CONFIG
#ifdef USE_ADC
	{ "vbat_adc_channel", VAR_UINT8 | MASTER_VALUE, .config.minmax = {ADC_CHN_NONE, ADC_CHN_MAX}, offsetof(adcChannelConfig_t, adcFunctionChannel[ADC_BATTERY]) },
	{ "rssi_adc_channel", VAR_UINT8 | MASTER_VALUE, .config.minmax = {ADC_CHN_NONE, ADC_CHN_MAX}, offsetof(adcChannelConfig_t, adcFunctionChannel[ADC_RSSI]) },
	{ "current_adc_channel", VAR_UINT8 | MASTER_VALUE, .config.minmax = {ADC_CHN_NONE, ADC_CHN_MAX}, offsetof(adcChannelConfig_t, adcFunctionChannel[ADC_CURRENT]) },
	{ "airspeed_adc_channel", VAR_UINT8 | MASTER_VALUE, .config.minmax = {ADC_CHN_NONE, ADC_CHN_MAX}, offsetof(adcChannelConfig_t, adcFunctionChannel[ADC_AIRSPEED]) },
#endif
// PG_ACCELEROMETER_CONFIG
#ifdef USE_ACC_NOTCH
	{ "acc_notch_hz", VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 255}, offsetof(accelerometerConfig_t, acc_notch_hz) },
	{ "acc_notch_cutoff", VAR_UINT8 | MASTER_VALUE, .config.minmax = {1, 255}, offsetof(accelerometerConfig_t, acc_notch_cutoff) },
#endif
	{ "align_acc", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ALIGNMENT }, offsetof(accelerometerConfig_t, acc_align) },
	{ "acc_hardware", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ACC_HARDWARE }, offsetof(accelerometerConfig_t, acc_hardware) },
	{ "acc_lpf_hz", VAR_UINT16 | MASTER_VALUE | MODE_MAX, .config.max = {200}, offsetof(accelerometerConfig_t, acc_lpf_hz) },
	{ "acczero_x", VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, offsetof(accelerometerConfig_t, accZero.raw[X]) },
	{ "acczero_y", VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, offsetof(accelerometerConfig_t, accZero.raw[Y]) },
	{ "acczero_z", VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, offsetof(accelerometerConfig_t, accZero.raw[Z]) },
	{ "accgain_x", VAR_INT16 | MASTER_VALUE, .config.minmax = {1, 8192}, offsetof(accelerometerConfig_t, accGain.raw[X]) },
	{ "accgain_y", VAR_INT16 | MASTER_VALUE, .config.minmax = {1, 8192}, offsetof(accelerometerConfig_t, accGain.raw[Y]) },
	{ "accgain_z", VAR_INT16 | MASTER_VALUE, .config.minmax = {1, 8192}, offsetof(accelerometerConfig_t, accGain.raw[Z]) },
// PG_RANGEFINDER_CONFIG
#ifdef USE_RANGEFINDER
	{ "rangefinder_hardware", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_RANGEFINDER_HARDWARE }, offsetof(rangefinderConfig_t, rangefinder_hardware) },
#endif
// PG_COMPASS_CONFIG
#ifdef MAG
	{ "align_mag", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_ALIGNMENT }, offsetof(compassConfig_t, mag_align) },
	{ "mag_hardware", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_MAG_HARDWARE }, offsetof(compassConfig_t, mag_hardware) },
	{ "mag_declination", VAR_INT16 | MASTER_VALUE, .config.minmax = {-18000, 18000}, offsetof(compassConfig_t, mag_declination) },
	{ "magzero_x", VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, offsetof(compassConfig_t, magZero.raw[X]) },
	{ "magzero_y", VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, offsetof(compassConfig_t, magZero.raw[Y]) },
	{ "magzero_z", VAR_INT16 | MASTER_VALUE, .config.minmax = {INT16_MIN, INT16_MAX}, offsetof(compassConfig_t, magZero.raw[Z]) },
	{ "mag_calibration_time", VAR_UINT8 | MASTER_VALUE, .config.minmax = {30, 120}, offsetof(compassConfig_t, magCalibrationTimeLimit) },
#endif
// PG_BAROMETER_CONFIG
#ifdef BARO
	{ "baro_hardware", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_BARO_HARDWARE }, offsetof(barometerConfig_t, baro_hardware) },
	{ "baro_use_median_filter", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(barometerConfig_t, use_median_filtering) },
#endif
// PG_PITOTMETER_CONFIG
#ifdef PITOT
	{ "pitot_hardware", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_PITOT_HARDWARE }, offsetof(pitotmeterConfig_t, pitot_hardware) },
	{ "pitot_use_median_filter", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(pitotmeterConfig_t, use_median_filtering) },
	{ "pitot_noise_lpf", VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 1}, offsetof(pitotmeterConfig_t, pitot_noise_lpf) },
	{ "pitot_scale", VAR_FLOAT | MASTER_VALUE, .config.minmax = {0, 100}, offsetof(pitotmeterConfig_t, pitot_scale) },
#endif
// PG_RX_CONFIG
	{ "mid_rc", VAR_UINT16 | MASTER_VALUE, .config.minmax = {1200, 1700}, offsetof(rxConfig_t, midrc) },
	{ "min_check", VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, offsetof(rxConfig_t, mincheck) },
	{ "max_check", VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_RANGE_ZERO, PWM_RANGE_MAX}, offsetof(rxConfig_t, maxcheck) },
	{ "rssi_channel", VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, MAX_SUPPORTED_RC_CHANNEL_COUNT}, offsetof(rxConfig_t, rssi_channel) },
	{ "rssi_scale", VAR_UINT8 | MASTER_VALUE, .config.minmax = {RSSI_SCALE_MIN, RSSI_SCALE_MAX}, offsetof(rxConfig_t, rssi_scale) },
	{ "rssi_invert", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(rxConfig_t, rssiInvert) },
	{ "rc_smoothing", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(rxConfig_t, rcSmoothing) },
#ifdef SERIAL_RX
	{ "serialrx_provider", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_SERIAL_RX }, offsetof(rxConfig_t, serialrx_provider) },
	{ "sbus_inversion", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(rxConfig_t, sbus_inversion) },
#endif
#ifdef USE_RX_SPI
	{ "rx_spi_protocol", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_RX_SPI_PROTOCOL }, offsetof(rxConfig_t, rx_spi_protocol) },
	{ "rx_spi_id", VAR_UINT32 | MASTER_VALUE, .config.minmax = {0, 0}, offsetof(rxConfig_t, rx_spi_id) },
#endif
	{ "rx_spi_rf_channel_count", VAR_UINT8 | MASTER_VALUE, .config.minmax = {0, 8}, offsetof(rxConfig_t, rx_spi_rf_channel_count) },
#ifdef SPEKTRUM_BIND
	{ "spektrum_sat_bind", VAR_UINT8 | MASTER_VALUE, .config.minmax = {SPEKTRUM_SAT_BIND_DISABLED, SPEKTRUM_SAT_BIND_MAX}, offsetof(rxConfig_t, spektrum_sat_bind) },
#endif
	{ "rx_min_usec", VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_PULSE_MIN, PWM_PULSE_MAX}, offsetof(rxConfig_t, rx_min_usec) },
	{ "rx_max_usec", VAR_UINT16 | MASTER_VALUE, .config.minmax = {PWM_PULSE_MIN, PWM_PULSE_MAX}, offsetof(rxConfig_t, rx_max_usec) },
#ifdef STM32F4
	{ "serialrx_halfduplex", VAR_UINT8 | MASTER_VALUE | MODE_LOOKUP, .config.lookup = { TABLE_OFF_ON }, offsetof(rxConfig_t, halfDuplex) },
#endif
};
