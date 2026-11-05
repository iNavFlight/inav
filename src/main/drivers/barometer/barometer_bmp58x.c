/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 * Bosch BMP580 / BMP581 / BMP585 barometer driver.
 *
 * Unlike BMP280/BMP388 the BMP58x family outputs fully compensated data,
 * so no trimming coefficients have to be read or applied. The sensor runs
 * in NORMAL mode at a fixed ODR and the driver simply fetches the latest
 * sample, which avoids depending on forced-mode conversion timings.
 */

#include <stdbool.h>
#include <stdint.h>

#include <platform.h>

#include "build/build_config.h"
#include "build/debug.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/io.h"
#include "drivers/bus.h"
#include "drivers/barometer/barometer.h"
#include "drivers/barometer/barometer_bmp58x.h"

#if defined(USE_BARO) && defined(USE_BARO_BMP58X)

// Register map, see BMP581 datasheet section 7
#define BMP58X_CHIP_ID_REG              (0x01)
#define BMP58X_REV_ID_REG               (0x02)
#define BMP58X_CHIP_STATUS_REG          (0x11)
#define BMP58X_DRIVE_CONFIG_REG         (0x13)
#define BMP58X_INT_CONFIG_REG           (0x14)
#define BMP58X_INT_SOURCE_REG           (0x15)
#define BMP58X_TEMP_DATA_XLSB_REG       (0x1D)
#define BMP58X_TEMP_DATA_LSB_REG        (0x1E)
#define BMP58X_TEMP_DATA_MSB_REG        (0x1F)
#define BMP58X_PRESS_DATA_XLSB_REG      (0x20)
#define BMP58X_PRESS_DATA_LSB_REG       (0x21)
#define BMP58X_PRESS_DATA_MSB_REG       (0x22)
#define BMP58X_INT_STATUS_REG           (0x27)
#define BMP58X_STATUS_REG               (0x28)
#define BMP58X_DSP_CONFIG_REG           (0x30)
#define BMP58X_DSP_IIR_REG              (0x31)
#define BMP58X_OSR_CONFIG_REG           (0x36)
#define BMP58X_ODR_CONFIG_REG           (0x37)
#define BMP58X_OSR_EFF_REG              (0x38)
#define BMP58X_CMD_REG                  (0x7E)

// Temperature and pressure are contiguous, read them in a single burst
#define BMP58X_DATA_FRAME_SIZE          ((BMP58X_PRESS_DATA_MSB_REG - BMP58X_TEMP_DATA_XLSB_REG) + 1)

#define BMP580_581_CHIP_ID              (0x50)
#define BMP585_CHIP_ID                  (0x51)

#define BMP58X_CMD_SOFT_RESET           (0xB6)

// STATUS register
#define BMP58X_STATUS_CORE_RDY          (1 << 0)
#define BMP58X_STATUS_NVM_RDY           (1 << 1)
#define BMP58X_STATUS_NVM_ERR           (1 << 2)

// INT_STATUS register
#define BMP58X_INT_STATUS_POR           (1 << 4)

// OSR_CONFIG register
#define BMP58X_OSR_T_SHIFT              0
#define BMP58X_OSR_T_MASK               (0x07)
#define BMP58X_OSR_P_SHIFT              3
#define BMP58X_OSR_P_MASK               (0x38)
#define BMP58X_PRESS_EN                 (1 << 6)

#define BMP58X_OVERSAMP_1X              (0x00)
#define BMP58X_OVERSAMP_2X              (0x01)
#define BMP58X_OVERSAMP_4X              (0x02)
#define BMP58X_OVERSAMP_8X              (0x03)
#define BMP58X_OVERSAMP_16X             (0x04)
#define BMP58X_OVERSAMP_32X             (0x05)
#define BMP58X_OVERSAMP_64X             (0x06)
#define BMP58X_OVERSAMP_128X            (0x07)

// ODR_CONFIG register
#define BMP58X_PWR_MODE_SHIFT           0
#define BMP58X_PWR_MODE_MASK            (0x03)
#define BMP58X_ODR_SHIFT                2
#define BMP58X_ODR_MASK                 (0x7C)
#define BMP58X_DEEP_DISABLE             (1 << 7)

#define BMP58X_PWR_MODE_STANDBY         (0x00)
#define BMP58X_PWR_MODE_NORMAL          (0x01)
#define BMP58X_PWR_MODE_FORCED          (0x02)
#define BMP58X_PWR_MODE_CONTINUOUS      (0x03)

#define BMP58X_ODR_50_HZ                (0x0F)

// DSP_IIR register
#define BMP58X_IIR_T_SHIFT              0
#define BMP58X_IIR_P_SHIFT              3
#define BMP58X_IIR_BYPASS               (0x00)

// High resolution setting. OSR_P 16x + OSR_T 2x converts well within the 20 ms ODR period.
#define BMP58X_PRESSURE_OSR             (BMP58X_OVERSAMP_16X)
#define BMP58X_TEMPERATURE_OSR          (BMP58X_OVERSAMP_2X)
#define BMP58X_ODR                      (BMP58X_ODR_50_HZ)
#define BMP58X_SAMPLE_PERIOD_US         (20000)

// INAV runs its own baro filtering; keeping the on-chip IIR off avoids adding group delay
#define BMP58X_PRESSURE_IIR             (BMP58X_IIR_BYPASS)
#define BMP58X_TEMPERATURE_IIR          (BMP58X_IIR_BYPASS)

#define BMP58X_SOFT_RESET_DELAY_MS      2
#define BMP58X_STANDBY_DELAY_MS         3
#define DETECTION_MAX_RETRY_COUNT       5
#define DETECTION_RETRY_DELAY_MS        10

#if defined(BMP58X_I2C_ADDR)
#define BMP58X_ADDRESS_COUNT            1
#else
#define BMP58X_ADDRESS_COUNT            2
#endif

// Latest raw sample: temperature LSB = 1/65536 degC, pressure LSB = 1/64 Pa
static int32_t bmp58xRawTemperature = 0;
static uint32_t bmp58xRawPressure = 0;

static bool bmp58xReadMeasurement(baroDev_t *baro)
{
    uint8_t data[BMP58X_DATA_FRAME_SIZE];

    // Keep the previous sample on bus error rather than feeding garbage to the estimator
    if (!busReadBuf(baro->busDev, BMP58X_TEMP_DATA_XLSB_REG, data, BMP58X_DATA_FRAME_SIZE)) {
        return false;
    }

    const uint32_t rawTemperature = (uint32_t)data[0] | ((uint32_t)data[1] << 8) | ((uint32_t)data[2] << 16);
    const uint32_t rawPressure = (uint32_t)data[3] | ((uint32_t)data[4] << 8) | ((uint32_t)data[5] << 16);

    // A zero pressure reading means the first conversion has not completed yet
    if (rawPressure == 0) {
        return false;
    }

    // Temperature is a 24-bit two's complement value, sign extend it to 32 bits
    bmp58xRawTemperature = (int32_t)(rawTemperature << 8) >> 8;
    bmp58xRawPressure = rawPressure;

    return true;
}

STATIC_UNIT_TESTED bool bmp58xCalculate(baroDev_t *baro, int32_t *pressure, int32_t *temperature)
{
    UNUSED(baro);

    if (pressure) {
        // Pa, rounded
        *pressure = (int32_t)((bmp58xRawPressure + 32) >> 6);
    }

    if (temperature) {
        // 0.01 degC
        *temperature = (int32_t)(((int64_t)bmp58xRawTemperature * 100) / 65536);
    }

    return true;
}

static bool deviceDetect(busDevice_t *busDev)
{
    uint8_t chipId = 0;

    // On SPI the first transaction after power-up only switches the interface from I2C to SPI
    if (busDev->busType == BUSTYPE_SPI) {
        busRead(busDev, BMP58X_CHIP_ID_REG, &chipId);
    }

    if (!busRead(busDev, BMP58X_CHIP_ID_REG, &chipId)) {
        return false;
    }

    return chipId == BMP580_581_CHIP_ID || chipId == BMP585_CHIP_ID;
}

static bool deviceConfigure(busDevice_t *busDev)
{
    uint8_t value = 0;

    if (!busWrite(busDev, BMP58X_CMD_REG, BMP58X_CMD_SOFT_RESET)) {
        return false;
    }
    delay(BMP58X_SOFT_RESET_DELAY_MS);

    // Soft reset puts the interface back into auto-detect, switch SPI again
    if (busDev->busType == BUSTYPE_SPI) {
        busRead(busDev, BMP58X_CHIP_ID_REG, &value);
    }

    // Verify the chip finished loading its NVM trimming data
    if (!busRead(busDev, BMP58X_STATUS_REG, &value)) {
        return false;
    }
    if (!(value & BMP58X_STATUS_NVM_RDY) || (value & BMP58X_STATUS_NVM_ERR)) {
        return false;
    }

    // INT_STATUS is clear-on-read; the POR flag confirms the reset completed
    if (!busRead(busDev, BMP58X_INT_STATUS_REG, &value) || !(value & BMP58X_INT_STATUS_POR)) {
        return false;
    }

    // OSR/ODR may only be changed in standby. Disable deep standby so registers are retained.
    if (!busWrite(busDev, BMP58X_ODR_CONFIG_REG, BMP58X_DEEP_DISABLE | (BMP58X_PWR_MODE_STANDBY << BMP58X_PWR_MODE_SHIFT))) {
        return false;
    }
    delay(BMP58X_STANDBY_DELAY_MS);

    const uint8_t iirConfig = (BMP58X_TEMPERATURE_IIR << BMP58X_IIR_T_SHIFT) | (BMP58X_PRESSURE_IIR << BMP58X_IIR_P_SHIFT);
    if (!busWrite(busDev, BMP58X_DSP_IIR_REG, iirConfig)) {
        return false;
    }

    const uint8_t osrConfig = BMP58X_PRESS_EN |
        ((BMP58X_TEMPERATURE_OSR << BMP58X_OSR_T_SHIFT) & BMP58X_OSR_T_MASK) |
        ((BMP58X_PRESSURE_OSR << BMP58X_OSR_P_SHIFT) & BMP58X_OSR_P_MASK);
    if (!busWrite(busDev, BMP58X_OSR_CONFIG_REG, osrConfig)) {
        return false;
    }

    const uint8_t odrConfig = BMP58X_DEEP_DISABLE |
        ((BMP58X_ODR << BMP58X_ODR_SHIFT) & BMP58X_ODR_MASK) |
        ((BMP58X_PWR_MODE_NORMAL << BMP58X_PWR_MODE_SHIFT) & BMP58X_PWR_MODE_MASK);
    if (!busWrite(busDev, BMP58X_ODR_CONFIG_REG, odrConfig)) {
        return false;
    }

    // Read back to make sure the sensor accepted the configuration
    if (!busRead(busDev, BMP58X_OSR_CONFIG_REG, &value) || value != osrConfig) {
        return false;
    }

    return true;
}

bool bmp58xDetect(baroDev_t *baro)
{
    busDevice_t *candidates[BMP58X_ADDRESS_COUNT];
    int candidateCount = 0;

    for (int index = 0; index < BMP58X_ADDRESS_COUNT; index++) {
        candidates[index] = busDeviceInit(BUSTYPE_ANY, DEVHW_BMP58X_0 + index, 0, OWNER_BARO);
        if (candidates[index]) {
            busSetSpeed(candidates[index], BUS_SPEED_STANDARD);
            candidateCount++;
        }
    }

    baro->busDev = NULL;

    // All candidate addresses share one retry window so a second address adds no boot delay
    for (int retry = 0; retry < DETECTION_MAX_RETRY_COUNT && candidateCount > 0 && !baro->busDev; retry++) {
        delay(DETECTION_RETRY_DELAY_MS);

        for (int index = 0; index < BMP58X_ADDRESS_COUNT && !baro->busDev; index++) {
            if (!candidates[index] || !deviceDetect(candidates[index])) {
                continue;
            }

            if (deviceConfigure(candidates[index])) {
                baro->busDev = candidates[index];
            }
        }
    }

    for (int index = 0; index < BMP58X_ADDRESS_COUNT; index++) {
        if (candidates[index] && candidates[index] != baro->busDev) {
            busDeviceDeInit(candidates[index]);
        }
    }

    if (!baro->busDev) {
        return false;
    }

    baro->ut_delay = 0;
    baro->start_ut = NULL;
    baro->get_ut = NULL;

    // Poll at twice the ODR; the full read/calculate cycle then matches the sensor sample rate
    baro->up_delay = BMP58X_SAMPLE_PERIOD_US / 2;
    baro->start_up = NULL;
    baro->get_up = bmp58xReadMeasurement;

    baro->calculate = bmp58xCalculate;

    return true;
}

#endif
