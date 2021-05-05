/*
 * This file is part of iNav.
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

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include <platform.h>
#include "build/build_config.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/barometer/barometer.h"
#include "drivers/barometer/barometer_spl06.h"

#if defined(USE_BARO_SPL06)

// SPL06, address 0x76

typedef struct {
    int16_t c0;
    int16_t c1;
    int32_t c00;
    int32_t c10;
    int16_t c01;
    int16_t c11;
    int16_t c20;
    int16_t c21;
    int16_t c30;
} spl06_coeffs_t;


spl06_coeffs_t spl06_cal;
// uncompensated pressure and temperature
static int32_t spl06_pressure_raw = 0;
static int32_t spl06_temperature_raw = 0;

static int8_t spl06_samples_to_cfg_reg_value(uint8_t sample_rate)
{
    switch(sample_rate)
    {
        case 1: return 0;
        case 2: return 1;
        case 4: return 2;
        case 8: return 3;
        case 16: return 4;
        case 32: return 5;
        case 64: return 6;
        case 128: return 7;
        default: return -1; // invalid
    }
}

static int32_t spl06_raw_value_scale_factor(uint8_t oversampling_rate)
{
    switch(oversampling_rate)
    {
        case 1: return 524288;
        case 2: return 1572864;
        case 4: return 3670016;
        case 8: return 7864320;
        case 16: return 253952;
        case 32: return 516096;
        case 64: return 1040384;
        case 128: return 2088960;
        default: return -1; // invalid
    }
}

static bool spl06_start_temperature_measurement(baroDev_t * baro)
{
    return busWrite(baro->busDev, SPL06_MODE_AND_STATUS_REG, SPL06_MEAS_TEMPERATURE);
}

static bool spl06_read_temperature(baroDev_t * baro)
{
    uint8_t data[SPL06_TEMPERATURE_LEN];
    int32_t spl06_temperature;

    bool ack = busReadBuf(baro->busDev, SPL06_TEMPERATURE_START_REG, data, SPL06_TEMPERATURE_LEN);

    if (ack) {
        spl06_temperature = (int32_t)((data[0] & 0x80 ? 0xFF000000 : 0) | (((uint32_t)(data[0])) << 16) | (((uint32_t)(data[1])) << 8) | ((uint32_t)data[2]));
        spl06_temperature_raw = spl06_temperature;
    }

    return ack;
}

static bool spl06_start_pressure_measurement(baroDev_t * baro)
{
    return busWrite(baro->busDev, SPL06_MODE_AND_STATUS_REG, SPL06_MEAS_PRESSURE);
}

static bool spl06_read_pressure(baroDev_t * baro)
{
    uint8_t data[SPL06_PRESSURE_LEN];
    int32_t spl06_pressure;

    bool ack = busReadBuf(baro->busDev, SPL06_PRESSURE_START_REG, data, SPL06_PRESSURE_LEN);

    if (ack) {
        spl06_pressure = (int32_t)((data[0] & 0x80 ? 0xFF000000 : 0) | (((uint32_t)(data[0])) << 16) | (((uint32_t)(data[1])) << 8) | ((uint32_t)data[2]));
        spl06_pressure_raw = spl06_pressure;
    }

    return ack;
}

// Returns temperature in degrees centigrade
static float spl06_compensate_temperature(int32_t temperature_raw)
{
    const float t_raw_sc = (float)temperature_raw / spl06_raw_value_scale_factor(SPL06_TEMPERATURE_OVERSAMPLING);
    const float temp_comp = (float)spl06_cal.c0 / 2 + t_raw_sc * spl06_cal.c1;
    return temp_comp;
}

// Returns pressure in Pascal
static float spl06_compensate_pressure(int32_t pressure_raw, int32_t temperature_raw)
{
    const float p_raw_sc = (float)pressure_raw / spl06_raw_value_scale_factor(SPL06_PRESSURE_OVERSAMPLING);
    const float t_raw_sc = (float)temperature_raw / spl06_raw_value_scale_factor(SPL06_TEMPERATURE_OVERSAMPLING);

    const float pressure_cal = (float)spl06_cal.c00 + p_raw_sc * ((float)spl06_cal.c10 + p_raw_sc * ((float)spl06_cal.c20 + p_raw_sc * spl06_cal.c30));
    const float p_temp_comp = t_raw_sc * ((float)spl06_cal.c01 + p_raw_sc * ((float)spl06_cal.c11 + p_raw_sc * spl06_cal.c21));

    return pressure_cal + p_temp_comp;
}

bool spl06_calculate(baroDev_t * baro, int32_t * pressure, int32_t * temperature)
{
    UNUSED(baro);

    if (pressure) {
        *pressure = lrintf(spl06_compensate_pressure(spl06_pressure_raw, spl06_temperature_raw));
    }

    if (temperature) {
        *temperature = lrintf(spl06_compensate_temperature(spl06_temperature_raw) * 100);
    }

    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(busDevice_t * busDev)
{
    uint8_t chipId;
    for (int retry = 0; retry < DETECTION_MAX_RETRY_COUNT; retry++) {
        delay(100);
        bool ack = busRead(busDev, SPL06_CHIP_ID_REG, &chipId);
        if (ack && chipId == SPL06_DEFAULT_CHIP_ID) {
            return true;
        }
    };

    return false;
}

static bool read_calibration_coefficients(baroDev_t *baro) {
    uint8_t sstatus;
    if (!(busRead(baro->busDev, SPL06_MODE_AND_STATUS_REG, &sstatus) && (sstatus & SPL06_MEAS_CFG_COEFFS_RDY)))
        return false;   // error reading status or coefficients not ready

    uint8_t caldata[SPL06_CALIB_COEFFS_LEN];

    if (!busReadBuf(baro->busDev, SPL06_CALIB_COEFFS_START, (uint8_t *)&caldata, SPL06_CALIB_COEFFS_LEN)) {
        return false;
    }

    spl06_cal.c0 = (caldata[0] & 0x80 ? 0xF000 : 0) | ((uint16_t)caldata[0] << 4) | (((uint16_t)caldata[1] & 0xF0) >> 4);
    spl06_cal.c1 = ((caldata[1] & 0x8 ? 0xF000 : 0) | ((uint16_t)caldata[1] & 0x0F) << 8) | (uint16_t)caldata[2];
    spl06_cal.c00 = (caldata[3] & 0x80 ? 0xFFF00000 : 0) | ((uint32_t)caldata[3] << 12) | ((uint32_t)caldata[4] << 4) | (((uint32_t)caldata[5] & 0xF0) >> 4);
    spl06_cal.c10 = (caldata[5] & 0x8 ? 0xFFF00000 : 0) | (((uint32_t)caldata[5] & 0x0F) << 16) | ((uint32_t)caldata[6] << 8) | (uint32_t)caldata[7];
    spl06_cal.c01 = ((uint16_t)caldata[8] << 8) | ((uint16_t)caldata[9]);
    spl06_cal.c11 = ((uint16_t)caldata[10] << 8) | (uint16_t)caldata[11];
    spl06_cal.c20 = ((uint16_t)caldata[12] << 8) | (uint16_t)caldata[13];
    spl06_cal.c21 = ((uint16_t)caldata[14] << 8) | (uint16_t)caldata[15];
    spl06_cal.c30 = ((uint16_t)caldata[16] << 8) | (uint16_t)caldata[17];

    return true;
}

static bool spl06_configure_measurements(baroDev_t *baro)
{
    uint8_t reg_value;

    reg_value = SPL06_TEMP_USE_EXT_SENSOR | spl06_samples_to_cfg_reg_value(SPL06_TEMPERATURE_OVERSAMPLING);
    if (!busWrite(baro->busDev, SPL06_TEMPERATURE_CFG_REG, reg_value)) {
        return false;
    }

    reg_value = spl06_samples_to_cfg_reg_value(SPL06_PRESSURE_OVERSAMPLING);
    if (!busWrite(baro->busDev, SPL06_PRESSURE_CFG_REG, reg_value)) {
        return false;
    }

    reg_value = 0;
    if (SPL06_TEMPERATURE_OVERSAMPLING > 8) {
        reg_value |= SPL06_TEMPERATURE_RESULT_BIT_SHIFT;
    }
    if (SPL06_PRESSURE_OVERSAMPLING > 8) {
        reg_value |= SPL06_PRESSURE_RESULT_BIT_SHIFT;
    }
    if (!busWrite(baro->busDev, SPL06_INT_AND_FIFO_CFG_REG, reg_value)) {
        return false;
    }

    return true;
}

bool spl06Detect(baroDev_t *baro)
{
    baro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_SPL06, 0, OWNER_BARO);
    if (baro->busDev == NULL) {
        return false;
    }

    busSetSpeed(baro->busDev, BUS_SPEED_STANDARD);

    if (!(deviceDetect(baro->busDev) && read_calibration_coefficients(baro) && spl06_configure_measurements(baro))) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    baro->ut_delay = SPL06_MEASUREMENT_TIME(SPL06_TEMPERATURE_OVERSAMPLING) * 1000;
    baro->get_ut = spl06_read_temperature;
    baro->start_ut = spl06_start_temperature_measurement;

    baro->up_delay = SPL06_MEASUREMENT_TIME(SPL06_PRESSURE_OVERSAMPLING) * 1000;
    baro->start_up = spl06_start_pressure_measurement;
    baro->get_up = spl06_read_pressure;

    baro->calculate = spl06_calculate;

    return true;
}

#endif
