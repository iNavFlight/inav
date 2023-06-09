/*
 * This file is part of INAV.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 *
 * bmp390 Driver author: Dominic Clifton
 * INAV port: Michel Pastor
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <platform.h>
#include "build/build_config.h"
#include "build/debug.h"
#include "common/utils.h"
#include "common/log.h" // XXX

#include "drivers/time.h"
#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/barometer/barometer.h"
#include "drivers/barometer/barometer_bmp390.h"

#if defined(USE_BARO) && (defined(USE_BARO_bmp390) || defined(USE_BARO_SPI_bmp390))

#define bmp390_I2C_ADDR                                 (0x76) // same as BMP280/BMP180
#define bmp390_DEFAULT_CHIP_ID                          (0x50) // from https://github.com/BoschSensortec/BMP3-Sensor-API/blob/master/bmp3_defs.h#L130

#define bmp390_CMD_REG                                  (0x7E)
#define bmp390_RESERVED_UPPER_REG                       (0x7D)
// everything between bmp390_RESERVED_UPPER_REG and bmp390_RESERVED_LOWER_REG is reserved.
#define bmp390_RESERVED_LOWER_REG                       (0x20)
#define bmp390_CONFIG_REG                               (0x1F)
#define bmp390_RESERVED_0x1E_REG                        (0x1E)
#define bmp390_ODR_REG                                  (0x1D)
#define bmp390_OSR_REG                                  (0x1C)
#define bmp390_PWR_CTRL_REG                             (0x1B)
#define bmp390_IF_CONFIG_REG                            (0x1A)
#define bmp390_INT_CTRL_REG                             (0x19)
#define bmp390_FIFO_CONFIG_2_REG                        (0x18)
#define bmp390_FIFO_CONFIG_1_REG                        (0x17)
#define bmp390_FIFO_WTM_1_REG                           (0x16)
#define bmp390_FIFO_WTM_0_REG                           (0x15)
#define bmp390_FIFO_DATA_REG                            (0x14)
#define bmp390_FIFO_LENGTH_1_REG                        (0x13)
#define bmp390_FIFO_LENGTH_0_REG                        (0x12)
#define bmp390_INT_STATUS_REG                           (0x11)
#define bmp390_EVENT_REG                                (0x10)
#define bmp390_SENSORTIME_3_REG                         (0x0F) // BME780 only
#define bmp390_SENSORTIME_2_REG                         (0x0E)
#define bmp390_SENSORTIME_1_REG                         (0x0D)
#define bmp390_SENSORTIME_0_REG                         (0x0C)
#define bmp390_RESERVED_0x0B_REG                        (0x0B)
#define bmp390_RESERVED_0x0A_REG                        (0x0A)

// see friendly register names below
#define bmp390_DATA_5_REG                               (0x09)
#define bmp390_DATA_4_REG                               (0x08)
#define bmp390_DATA_3_REG                               (0x07)
#define bmp390_DATA_2_REG                               (0x06)
#define bmp390_DATA_1_REG                               (0x05)
#define bmp390_DATA_0_REG                               (0x04)

#define bmp390_STATUS_REG                               (0x03)
#define bmp390_ERR_REG                                  (0x02)
#define bmp390_RESERVED_0x01_REG                        (0x01)
#define bmp390_CHIP_ID_REG                              (0x00)

// friendly register names, from datasheet 4.3.4
#define bmp390_PRESS_MSB_23_16_REG                      bmp390_DATA_2_REG
#define bmp390_PRESS_LSB_15_8_REG                       bmp390_DATA_1_REG
#define bmp390_PRESS_XLSB_7_0_REG                       bmp390_DATA_0_REG

// friendly register names, from datasheet 4.3.5
#define bmp390_TEMP_MSB_23_16_REG                       bmp390_DATA_5_REG
#define bmp390_TEMP_LSB_15_8_REG                        bmp390_DATA_4_REG
#define bmp390_TEMP_XLSB_7_0_REG                        bmp390_DATA_3_REG

#define bmp390_DATA_FRAME_SIZE                          ((bmp390_DATA_5_REG - bmp390_DATA_0_REG) + 1) // +1 for inclusive

// from Datasheet 3.3
#define bmp390_MODE_SLEEP                               (0x00)
#define bmp390_MODE_FORCED                              (0x01)
#define bmp390_MODE_NORMAL                              (0x02)

#define bmp390_CALIRATION_LOWER_REG                     (0x30) // See datasheet 4.3.19, "calibration data"
#define bmp390_TRIMMING_NVM_PAR_T1_LSB_REG              (0x31) // See datasheet 3.11.1 "Memory map trimming coefficients"
#define bmp390_TRIMMING_NVM_PAR_P11_REG                 (0x45) // See datasheet 3.11.1 "Memory map trimming coefficients"
#define bmp390_CALIRATION_UPPER_REG                     (0x57)

#define bmp390_TRIMMING_DATA_LENGTH                     ((bmp390_TRIMMING_NVM_PAR_P11_REG - bmp390_TRIMMING_NVM_PAR_T1_LSB_REG) + 1) // +1 for inclusive

#define bmp390_OVERSAMP_1X               (0x00)
#define bmp390_OVERSAMP_2X               (0x01)
#define bmp390_OVERSAMP_4X               (0x02)
#define bmp390_OVERSAMP_8X               (0x03)
#define bmp390_OVERSAMP_16X              (0x04)
#define bmp390_OVERSAMP_32X              (0x05)

// INT_CTRL register
#define bmp390_INT_OD_BIT                   0
#define bmp390_INT_LEVEL_BIT                1
#define bmp390_INT_LATCH_BIT                2
#define bmp390_INT_FWTM_EN_BIT              3
#define bmp390_INT_FFULL_EN_BIT             4
#define bmp390_INT_RESERVED_5_BIT           5
#define bmp390_INT_DRDY_EN_BIT              6
#define bmp390_INT_RESERVED_7_BIT           7

// OSR register
#define bmp390_OSR_P_BIT                    0 // to 2
#define bmp390_OSR4_T_BIT                   3 // to 5
#define bmp390_OSR_P_MASK                   (0x03)  // -----111
#define bmp390_OSR4_T_MASK                  (0x38)  // --111---

// configure pressure and temperature oversampling, forced sampling mode
#define bmp390_PRESSURE_OSR              (bmp390_OVERSAMP_8X)
#define bmp390_TEMPERATURE_OSR           (bmp390_OVERSAMP_1X)

// see Datasheet 3.11.1 Memory Map Trimming Coefficients
typedef struct bmp390_calib_param_s {
    uint16_t T1;
    uint16_t T2;
    int8_t T3;
    int16_t P1;
    int16_t P2;
    int8_t P3;
    int8_t P4;
    uint16_t P5;
    uint16_t P6;
    int8_t P7;
    int8_t P8;
    int16_t P9;
    int8_t P10;
    int8_t P11;
} __attribute__((packed)) bmp390_calib_param_t;

STATIC_ASSERT(sizeof(bmp390_calib_param_t) == bmp390_TRIMMING_DATA_LENGTH, bmp390_calibration_structure_incorrectly_packed);

static bmp390_calib_param_t bmp390_cal;
// uncompensated pressure and temperature
static uint32_t bmp390_up = 0;
static uint32_t bmp390_ut = 0;
static uint8_t sensor_data[bmp390_DATA_FRAME_SIZE+1];

static int64_t t_lin = 0;

static bool bmp390StartUT(baroDev_t *baro);
static bool bmp390GetUT(baroDev_t *baro);
static bool bmp390StartUP(baroDev_t *baro);
static bool bmp390GetUP(baroDev_t *baro);

static bool bmp390Calculate(baroDev_t *baro, int32_t *pressure, int32_t *temperature);

static bool bmp390BeginForcedMeasurement(busDevice_t *busdev)
{
    // enable pressure measurement, temperature measurement, set power mode and start sampling
    uint8_t mode = bmp390_MODE_FORCED << 4 | 1 << 1 | 1 << 0;
    return busWrite(busdev, bmp390_PWR_CTRL_REG, mode);
}

static bool bmp390StartUT(baroDev_t *baro)
{
    UNUSED(baro);
    // dummy
    return true;
}

static bool bmp390GetUT(baroDev_t *baro)
{
    UNUSED(baro);
    // dummy
    return true;
}

static bool bmp390StartUP(baroDev_t *baro)
{
    // start measurement
    return bmp390BeginForcedMeasurement(baro->busDev);
}

static bool bmp390GetUP(baroDev_t *baro)
{
    if (baro->busDev->busType == BUSTYPE_SPI) {
        // In SPI mode, first byte read is a dummy byte
        busReadBuf(baro->busDev, bmp390_DATA_0_REG, &sensor_data[0], bmp390_DATA_FRAME_SIZE + 1);
    } else {
        // In I2C mode, no dummy byte is read
        busReadBuf(baro->busDev, bmp390_DATA_0_REG, &sensor_data[1], bmp390_DATA_FRAME_SIZE);
    }

    bmp390_up = sensor_data[1] << 0 | sensor_data[2] << 8 | sensor_data[3] << 16;
    bmp390_ut = sensor_data[4] << 0 | sensor_data[5] << 8 | sensor_data[6] << 16;
    return true;
}

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of "5123" equals 51.23 DegC
static int64_t bmp390CompensateTemperature(uint32_t uncomp_temperature)
{
    uint64_t partial_data1;
    uint64_t partial_data2;
    uint64_t partial_data3;
    int64_t partial_data4;
    int64_t partial_data5;
    int64_t partial_data6;
    int64_t comp_temp;

    partial_data1 = uncomp_temperature - (256 * bmp390_cal.T1);
    partial_data2 = bmp390_cal.T2 * partial_data1;
    partial_data3 = partial_data1 * partial_data1;
    partial_data4 = (int64_t)partial_data3 * bmp390_cal.T3;
    partial_data5 = ((int64_t)(partial_data2 * 262144) + partial_data4);
    partial_data6 = partial_data5 / 4294967296;
    /* Update t_lin, needed for pressure calculation */
    t_lin = partial_data6;
    comp_temp = (int64_t)((partial_data6 * 25)  / 16384);

    return comp_temp;
}

static uint64_t bmp390CompensatePressure(uint32_t uncomp_pressure)
{
    int64_t partial_data1;
    int64_t partial_data2;
    int64_t partial_data3;
    int64_t partial_data4;
    int64_t partial_data5;
    int64_t partial_data6;
    int64_t offset;
    int64_t sensitivity;
    uint64_t comp_press;

    partial_data1 = t_lin * t_lin;
    partial_data2 = partial_data1 / 64;
    partial_data3 = (partial_data2 * t_lin) / 256;
    partial_data4 = (bmp390_cal.P8 * partial_data3) / 32;
    partial_data5 = (bmp390_cal.P7 * partial_data1) * 16;
    partial_data6 = (bmp390_cal.P6 * t_lin) * 4194304;
    offset = (bmp390_cal.P5 * 140737488355328) + partial_data4 + partial_data5 + partial_data6;

    partial_data2 = (bmp390_cal.P4 * partial_data3) / 32;
    partial_data4 = (bmp390_cal.P3 * partial_data1) * 4;
    partial_data5 = (bmp390_cal.P2 - 16384) * t_lin * 2097152;
    sensitivity = ((bmp390_cal.P1 - 16384) * 70368744177664) + partial_data2 + partial_data4 + partial_data5;

    partial_data1 = (sensitivity / 16777216) * uncomp_pressure;
    partial_data2 = bmp390_cal.P10 * t_lin;
    partial_data3 = partial_data2 + (65536 * bmp390_cal.P9);
    partial_data4 = (partial_data3 * uncomp_pressure) / 8192;
    partial_data5 = (partial_data4 * uncomp_pressure) / 512;
    partial_data6 = (int64_t)((uint64_t)uncomp_pressure * (uint64_t)uncomp_pressure);
    partial_data2 = (bmp390_cal.P11 * partial_data6) / 65536;
    partial_data3 = (partial_data2 * uncomp_pressure) / 128;
    partial_data4 = (offset / 4) + partial_data1 + partial_data5 + partial_data3;
    comp_press = (((uint64_t)partial_data4 * 25) / (uint64_t)1099511627776);

    return comp_press;
}

STATIC_UNIT_TESTED bool bmp390Calculate(baroDev_t *baro, int32_t *pressure, int32_t *temperature)
{
    UNUSED(baro);

    // calculate
    int64_t t;
    int64_t p;

    t = bmp390CompensateTemperature(bmp390_ut);
    p = bmp390CompensatePressure(bmp390_up);

    if (pressure)
        *pressure = (int32_t)(p / 256);
    if (temperature)
        *temperature = t;

    return true;
}

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(busDevice_t * busDev)
{
    uint8_t chipId[2];
    uint8_t nRead;
    uint8_t * pId;

    if (busDev->busType == BUSTYPE_SPI) {
        // In SPI mode, first byte read is a dummy byte
        nRead = 2;
        pId = &chipId[1];
    } else {
        // In I2C mode, no dummy byte is read
        nRead = 1;
        pId = &chipId[0];
    }

    for (int retry = 0; retry < DETECTION_MAX_RETRY_COUNT; retry++) {
        delay(100);

        bool ack = busReadBuf(busDev, bmp390_CHIP_ID_REG, chipId, nRead);

        if (ack && *pId == bmp390_DEFAULT_CHIP_ID) {
            return true;
        }
    };

    return false;
}

bool bmp390Detect(baroDev_t *baro)
{
    baro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_bmp390, 0, OWNER_BARO);
    if (baro->busDev == NULL) {
        return false;
    }

    busSetSpeed(baro->busDev, BUS_SPEED_STANDARD);

    if (!deviceDetect(baro->busDev)) {
        busDeviceDeInit(baro->busDev);
        return false;
    }

    // read calibration
    if (baro->busDev->busType == BUSTYPE_SPI) {
        // In SPI mode, first byte read is a dummy byte
        uint8_t calibration_buf[sizeof(bmp390_calib_param_t) + 1];
        busReadBuf(baro->busDev, bmp390_TRIMMING_NVM_PAR_T1_LSB_REG, calibration_buf, sizeof(bmp390_calib_param_t) + 1);
        memcpy(&bmp390_cal, calibration_buf + 1, sizeof(bmp390_calib_param_t));
    } else {
        // In I2C mode, no dummy byte is read
        busReadBuf(baro->busDev, bmp390_TRIMMING_NVM_PAR_T1_LSB_REG, (uint8_t*)&bmp390_cal, sizeof(bmp390_calib_param_t));
    }

    // set oversampling + power mode (forced), and start sampling
    busWrite(baro->busDev, bmp390_OSR_REG,
        ((bmp390_PRESSURE_OSR << bmp390_OSR_P_BIT) & bmp390_OSR_P_MASK) |
        ((bmp390_TEMPERATURE_OSR << bmp390_OSR4_T_BIT) & bmp390_OSR4_T_MASK)
    );

    bmp390BeginForcedMeasurement(baro->busDev);

    baro->ut_delay = 0;
    baro->get_ut = bmp390GetUT;
    baro->start_ut = bmp390StartUT;

    baro->up_delay = 234 + (392 + ((1 << (bmp390_PRESSURE_OSR + 1)) * 2000)) + (313 + ((1 << (bmp390_TEMPERATURE_OSR + 1)) * 2000));
    baro->start_up = bmp390StartUP;
    baro->get_up = bmp390GetUP;

    baro->calculate = bmp390Calculate;

    return true;
}

#endif
