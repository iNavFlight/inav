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

#include <platform.h>

#include "build/build_config.h"
#include "common/utils.h"

#include "drivers/barometer/barometer.h"

#include "drivers/time.h"
#include "drivers/bus.h"
#include "drivers/nvic.h"
#include "drivers/exti.h"
#include "drivers/io.h"

#include "drivers/barometer/barometer_bmp085.h"

#if defined(USE_BARO_BMP085)

#define BMP085_I2C_ADDR         0x77
#define BMP085_CHIP_ID          0x55

typedef struct {
    int16_t ac1;
    int16_t ac2;
    int16_t ac3;
    uint16_t ac4;
    uint16_t ac5;
    uint16_t ac6;
    int16_t b1;
    int16_t b2;
    int16_t mb;
    int16_t mc;
    int16_t md;
} bmp085_smd500_calibration_param_t;

typedef struct {
    bmp085_smd500_calibration_param_t cal_param;
    int32_t param_b5;
    int16_t oversampling_setting;
} bmp085_t;

#define BOSCH_PRESSURE_BMP085   85
#define BMP085_CHIP_ID_REG      0xD0
#define BMP085_VERSION_REG      0xD1
#define E_SENSOR_NOT_DETECTED   (char) 0
#define BMP085_PROM_START__ADDR 0xaa
#define BMP085_PROM_DATA__LEN   22
#define BMP085_T_MEASURE        0x2E                // temperature measurent
#define BMP085_P_MEASURE        0x34                // pressure measurement
#define BMP085_CTRL_MEAS_REG    0xF4
#define BMP085_ADC_OUT_MSB_REG  0xF6
#define BMP085_ADC_OUT_LSB_REG  0xF7
#define BMP085_CHIP_ID__POS     0
#define BMP085_CHIP_ID__MSK     0xFF
#define BMP085_CHIP_ID__LEN     8
#define BMP085_CHIP_ID__REG     BMP085_CHIP_ID_REG

#define BMP085_ML_VERSION__POS      0
#define BMP085_ML_VERSION__LEN      4
#define BMP085_ML_VERSION__MSK      0x0F
#define BMP085_ML_VERSION__REG      BMP085_VERSION_REG

#define BMP085_AL_VERSION__POS      4
#define BMP085_AL_VERSION__LEN      4
#define BMP085_AL_VERSION__MSK      0xF0
#define BMP085_AL_VERSION__REG      BMP085_VERSION_REG

#define BMP085_GET_BITSLICE(regvar, bitname) ((regvar & bitname##__MSK) >> bitname##__POS)
#define BMP085_SET_BITSLICE(regvar, bitname, val) ((regvar & ~bitname##__MSK) | ((val<<bitname##__POS)&bitname##__MSK))

#define SMD500_PARAM_MG      3038        //calibration parameter
#define SMD500_PARAM_MH     -7357        //calibration parameter
#define SMD500_PARAM_MI      3791        //calibration parameter

STATIC_UNIT_TESTED bmp085_t bmp085;

#define UT_DELAY    6000        // 1.5ms margin according to the spec (4.5ms T conversion time)
#define UP_DELAY    27000       // 6000+21000=27000 1.5ms margin according to the spec (25.5ms P conversion time with OSS=3)

STATIC_UNIT_TESTED uint16_t bmp085_ut;  // static result of temperature measurement
STATIC_UNIT_TESTED uint32_t bmp085_up;  // static result of pressure measurement

static void bmp085_get_cal_param(baroDev_t *baro)
{
    uint8_t data[BMP085_PROM_DATA__LEN];
    busReadBuf(baro->busDev, BMP085_PROM_START__ADDR, data, BMP085_PROM_DATA__LEN);

    /* parameters AC1-AC6 */
    bmp085.cal_param.ac1 = (data[0] << 8) | data[1];
    bmp085.cal_param.ac2 = (data[2] << 8) | data[3];
    bmp085.cal_param.ac3 = (data[4] << 8) | data[5];
    bmp085.cal_param.ac4 = (data[6] << 8) | data[7];
    bmp085.cal_param.ac5 = (data[8] << 8) | data[9];
    bmp085.cal_param.ac6 = (data[10] << 8) | data[11];

    /* parameters B1,B2 */
    bmp085.cal_param.b1 = (data[12] << 8) | data[13];
    bmp085.cal_param.b2 = (data[14] << 8) | data[15];

    /* parameters MB,MC,MD */
    bmp085.cal_param.mb = (data[16] << 8) | data[17];
    bmp085.cal_param.mc = (data[18] << 8) | data[19];
    bmp085.cal_param.md = (data[20] << 8) | data[21];
}

static int32_t bmp085_get_temperature(uint32_t ut)
{
    int32_t temperature;
    int32_t x1, x2;

    x1 = (((int32_t) ut - (int32_t) bmp085.cal_param.ac6) * (int32_t) bmp085.cal_param.ac5) >> 15;
    x2 = ((int32_t) bmp085.cal_param.mc << 11) / (x1 + bmp085.cal_param.md);
    bmp085.param_b5 = x1 + x2;
    temperature = ((bmp085.param_b5 * 10 + 8) >> 4);  // temperature in 0.01 C (make same as MS5611)

    return temperature;
}

static int32_t bmp085_get_pressure(uint32_t up)
{
    int32_t pressure, x1, x2, x3, b3, b6;
    uint32_t b4, b7;

    b6 = bmp085.param_b5 - 4000;
    // *****calculate B3************
    x1 = (b6 * b6) >> 12;
    x1 *= bmp085.cal_param.b2;
    x1 >>= 11;

    x2 = (bmp085.cal_param.ac2 * b6);
    x2 >>= 11;

    x3 = x1 + x2;

    b3 = (((((int32_t) bmp085.cal_param.ac1) * 4 + x3) << bmp085.oversampling_setting) + 2) >> 2;

    // *****calculate B4************
    x1 = (bmp085.cal_param.ac3 * b6) >> 13;
    x2 = (bmp085.cal_param.b1 * ((b6 * b6) >> 12)) >> 16;
    x3 = ((x1 + x2) + 2) >> 2;
    b4 = (bmp085.cal_param.ac4 * (uint32_t)(x3 + 32768)) >> 15;

    b7 = ((uint32_t)(up - b3) * (50000 >> bmp085.oversampling_setting));
    if (b7 < 0x80000000) {
        pressure = (b7 << 1) / b4;
    } else {
        pressure = (b7 / b4) << 1;
    }

    x1 = pressure >> 8;
    x1 *= x1;
    x1 = (x1 * SMD500_PARAM_MG) >> 16;
    x2 = (pressure * SMD500_PARAM_MH) >> 16;
    pressure += (x1 + x2 + SMD500_PARAM_MI) >> 4;   // pressure in Pa

    return pressure;
}

static bool bmp085_start_ut(baroDev_t *baro)
{
    bool ack = busWrite(baro->busDev, BMP085_CTRL_MEAS_REG, BMP085_T_MEASURE);
    return ack;
}

static bool bmp085_get_ut(baroDev_t *baro)
{
    uint8_t data[2];
    bool ack = busReadBuf(baro->busDev, BMP085_ADC_OUT_MSB_REG, data, 2);
    if (ack) {
        bmp085_ut = (data[0] << 8) | data[1];
    }

    return ack;
}

static bool bmp085_start_up(baroDev_t *baro)
{
    uint8_t ctrl_reg_data = BMP085_P_MEASURE + (bmp085.oversampling_setting << 6);
    bool ack = busWrite(baro->busDev, BMP085_CTRL_MEAS_REG, ctrl_reg_data);
    return ack;
}

/** read out up for pressure conversion
 depending on the oversampling ratio setting up can be 16 to 19 bit
 \return up parameter that represents the uncompensated pressure value
 */
static bool bmp085_get_up(baroDev_t *baro)
{
    uint8_t data[3];
    bool ack = busReadBuf(baro->busDev, BMP085_ADC_OUT_MSB_REG, data, 3);

    if (ack) {
        bmp085_up = (((uint32_t) data[0] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[2]) >> (8 - bmp085.oversampling_setting);
    }

    return ack;
}

STATIC_UNIT_TESTED bool bmp085_calculate(baroDev_t *baro, int32_t *pressure, int32_t *temperature)
{
    UNUSED(baro);
    int32_t temp, press;

    temp = bmp085_get_temperature(bmp085_ut);
    press = bmp085_get_pressure(bmp085_up);
    if (pressure)
        *pressure = press;
    if (temperature)
        *temperature = temp;
    return true;
}


#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(busDevice_t * busDev)
{
    for (int retry = 0; retry < DETECTION_MAX_RETRY_COUNT; retry++) {
        uint8_t chipId = 0;

        delay(10);

        bool ack = busRead(busDev, BMP085_CHIP_ID__REG, &chipId);
        if (ack && BMP085_GET_BITSLICE(chipId, BMP085_CHIP_ID) == BMP085_CHIP_ID) {
            return true;
        }
    };

    return false;
}

bool bmp085Detect(baroDev_t *baro)
{
    baro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_BMP085, 0, OWNER_BARO);
    if (baro->busDev == NULL) {
        return false;
    }

    if (!deviceDetect(baro->busDev)) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    bmp085.oversampling_setting = 3;
    bmp085_get_cal_param(baro); /* readout bmp085 calibparam structure */

    baro->ut_delay = UT_DELAY;
    baro->start_ut = bmp085_start_ut;
    baro->get_ut = bmp085_get_ut;

    baro->up_delay = UP_DELAY;
    baro->start_up = bmp085_start_up;
    baro->get_up = bmp085_get_up;

    baro->calculate = bmp085_calculate;

    return true;
}

#endif /* BARO */
