/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "platform.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/time.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_mpu.h"
#include "drivers/accgyro/accgyro_icm40609d.h"

#if defined(USE_IMU_ICM40609D)

// Register addresses/values verified against TDK ICM-40609-D datasheet
// (DS-000272 rev 0.8), not solely against Betaflight PR #14367/#14415.

#define ICM40609D_RA_DEVICE_CONFIG                  0x11
#define ICM40609D_DEVICE_CONFIG_SOFT_RESET          (1 << 0)

#define ICM40609D_RA_INT_CONFIG                     0x14
#define ICM40609D_INT1_MODE_PULSED                  (0 << 2)
#define ICM40609D_INT1_DRIVE_CIRCUIT_PP              (1 << 1)
#define ICM40609D_INT1_POLARITY_ACTIVE_HIGH          (1 << 0)

#define ICM40609D_RA_PWR_MGMT0                       0x4E
#define ICM40609D_PWR_MGMT0_ACCEL_MODE_LN            (3 << 0)
#define ICM40609D_PWR_MGMT0_GYRO_MODE_LN             (3 << 2)
#define ICM40609D_PWR_MGMT0_TEMP_DISABLE_OFF         (0 << 5)

#define ICM40609D_RA_GYRO_CONFIG0                    0x4F
#define ICM40609D_RA_ACCEL_CONFIG0                   0x50
#define ICM40609D_RA_GYRO_ACCEL_CONFIG0              0x52

// GYRO_FS_SEL = 0 -> +/-2000dps, 16.4 LSB/(deg/s)
#define ICM40609D_GYRO_FS_SEL_2000DPS                (0 << 5)

// ACCEL_FS_SEL = 1 -> +/-16g, 2048 LSB/g (FS_SEL=0 is +/-32g on this chip,
// unlike ICM42605 where 0 is the narrowest common range)
#define ICM40609D_ACCEL_FS_SEL_16G                   (1 << 5)

// Low-latency UI filter bandwidth select, both fields set to the "trivial"
// low-latency option (verified numeric meaning against the same bit layout
// as ICM42605's GYRO_ACCEL_CONFIG0, confirmed by Betaflight PR #14367's
// identically-valued ICM40609_*_UI_FILT_BW_LP_TRIVIAL_200HZ_8XODR macros)
#define ICM40609D_ACCEL_UI_FILT_BW_LOW_LATENCY       (15 << 4)
#define ICM40609D_GYRO_UI_FILT_BW_LOW_LATENCY        (15 << 0)

#define ICM40609D_RA_GYRO_DATA_X1                    0x25
#define ICM40609D_RA_ACCEL_DATA_X1                   0x1F
#define ICM40609D_RA_TEMP_DATA1                      0x1D

#define ICM40609D_RA_INT_CONFIG0                     0x63
#define ICM40609D_UI_DRDY_INT_CLEAR_ON_SBR           ((0 << 5) | (0 << 4))

#define ICM40609D_RA_INT_SOURCE0                     0x65
#define ICM40609D_UI_DRDY_INT1_EN_ENABLED            (1 << 3)

// ODR select values (bits[3:0] of GYRO_CONFIG0/ACCEL_CONFIG0) verified
// against the datasheet's ODR tables -- identical encoding to ICM42605
// for these five rates, so reusing the same gyroFilterAndRateConfig_t
// mechanism and DLPF tag (unused here; this driver doesn't vary the UI
// filter bandwidth by requested LPF, see icm40609dAccAndGyroInit).
static const gyroFilterAndRateConfig_t icm40609dGyroConfigs[] = {
    /*                            DLPF  ODR */
    { GYRO_LPF_256HZ,   8000,   { 0,    3  } },
    { GYRO_LPF_256HZ,   4000,   { 0,    4  } },
    { GYRO_LPF_256HZ,   2000,   { 0,    5  } },
    { GYRO_LPF_256HZ,   1000,   { 0,    6  } },
    { GYRO_LPF_256HZ,    500,   { 0,    15 } },
};

static void icm40609dAccInit(accDev_t *acc)
{
    acc->acc_1G = 512 * 4; // 2048 LSB/g, matches ACCEL_FS_SEL=1 (+/-16g)
}

static bool icm40609dAccRead(accDev_t *acc)
{
    uint8_t data[6];

    const bool ack = busReadBuf(acc->busDev, ICM40609D_RA_ACCEL_DATA_X1, data, 6);
    if (!ack) {
        return false;
    }

    acc->ADCRaw[X] = (float) int16_val_big_endian(data, 0);
    acc->ADCRaw[Y] = (float) int16_val_big_endian(data, 1);
    acc->ADCRaw[Z] = (float) int16_val_big_endian(data, 2);

    return true;
}

bool icm40609dAccDetect(accDev_t *acc)
{
    acc->busDev = busDeviceOpen(BUSTYPE_ANY, DEVHW_ICM40609D, acc->imuSensorToUse);
    if (acc->busDev == NULL) {
        return false;
    }

    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(acc->busDev);
    if (ctx->chipMagicNumber != 0x4609) {
        return false;
    }

    acc->initFn = icm40609dAccInit;
    acc->readFn = icm40609dAccRead;
    acc->accAlign = acc->busDev->param;

    return true;
}

static void icm40609dAccAndGyroInit(gyroDev_t *gyro)
{
    busDevice_t * dev = gyro->busDev;
    const gyroFilterAndRateConfig_t * config = chooseGyroConfig(gyro->lpf, 1000000 / gyro->requestedSampleIntervalUs,
                                                                 &icm40609dGyroConfigs[0], ARRAYLEN(icm40609dGyroConfigs));
    gyro->sampleRateIntervalUs = 1000000 / config->gyroRateHz;

    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);

    busWrite(dev, ICM40609D_RA_PWR_MGMT0, ICM40609D_PWR_MGMT0_TEMP_DISABLE_OFF | ICM40609D_PWR_MGMT0_ACCEL_MODE_LN | ICM40609D_PWR_MGMT0_GYRO_MODE_LN);
    delay(15);

    busWrite(dev, ICM40609D_RA_GYRO_CONFIG0, ICM40609D_GYRO_FS_SEL_2000DPS | (config->gyroConfigValues[1] & 0x0F));
    delay(15);

    busWrite(dev, ICM40609D_RA_ACCEL_CONFIG0, ICM40609D_ACCEL_FS_SEL_16G | (config->gyroConfigValues[1] & 0x0F));
    delay(15);

    // Low latency, same convention as ICM42605
    busWrite(dev, ICM40609D_RA_GYRO_ACCEL_CONFIG0, ICM40609D_ACCEL_UI_FILT_BW_LOW_LATENCY | ICM40609D_GYRO_UI_FILT_BW_LOW_LATENCY);
    delay(15);

    busWrite(dev, ICM40609D_RA_INT_CONFIG, ICM40609D_INT1_MODE_PULSED | ICM40609D_INT1_DRIVE_CIRCUIT_PP | ICM40609D_INT1_POLARITY_ACTIVE_HIGH);
    delay(15);

    // Unlike ICM42605, this chip's Bank 0 register map has no INT_CONFIG1
    // (0x64) / ASYNC_RESET bit -- confirmed absent from DS-000272 rev 0.8
    // (register map jumps from INT_CONFIG0 at 0x63 straight to INT_SOURCE0
    // at 0x65). ICM42605's "clear ASYNC_RESET for proper INT1/INT2
    // operation" erratum step is therefore intentionally omitted here.
    busWrite(dev, ICM40609D_RA_INT_CONFIG0, ICM40609D_UI_DRDY_INT_CLEAR_ON_SBR);
    delay(100);

    busWrite(dev, ICM40609D_RA_INT_SOURCE0, ICM40609D_UI_DRDY_INT1_EN_ENABLED);
    delay(15);

    busSetSpeed(dev, BUS_SPEED_FAST);
}

static bool icm40609dDeviceDetect(busDevice_t * dev)
{
    uint8_t tmp;
    uint8_t attemptsRemaining = 5;

    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);

    // DEVICE_CONFIG bit0 = SOFT_RESET_CONFIG. Betaflight PR #14367 originally
    // wrote this reset bit to register 0x4C (INTF_CONFIG0) instead of the
    // correct 0x11 (DEVICE_CONFIG) -- fixed upstream in PR #14415. Verified
    // against the datasheet directly rather than trusting either revision.
    busWrite(dev, ICM40609D_RA_DEVICE_CONFIG, ICM40609D_DEVICE_CONFIG_SOFT_RESET);

    do {
        delay(150);

        busRead(dev, MPU_RA_WHO_AM_I, &tmp);

        if (tmp == ICM40609D_WHO_AM_I_CONST) {
            return true;
        }
    } while (attemptsRemaining--);

    return false;
}

static bool icm40609dGyroRead(gyroDev_t *gyro)
{
    uint8_t data[6];

    const bool ack = busReadBuf(gyro->busDev, ICM40609D_RA_GYRO_DATA_X1, data, 6);
    if (!ack) {
        return false;
    }

    gyro->gyroADCRaw[X] = (float) int16_val_big_endian(data, 0);
    gyro->gyroADCRaw[Y] = (float) int16_val_big_endian(data, 1);
    gyro->gyroADCRaw[Z] = (float) int16_val_big_endian(data, 2);

    return true;
}

static bool icm40609dReadTemperature(gyroDev_t *gyro, int16_t * temp)
{
    uint8_t data[2];

    const bool ack = busReadBuf(gyro->busDev, ICM40609D_RA_TEMP_DATA1, data, 2);
    if (!ack) {
        return false;
    }
    // From datasheet: Temperature in Degrees Centigrade = (TEMP_DATA / 132.48) + 25
    *temp = ( int16_val_big_endian(data, 0) / 13.248 ) + 250; // Temperature stored as degC*10

    return true;
}

bool icm40609dGyroDetect(gyroDev_t *gyro)
{
    gyro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_ICM40609D, gyro->imuSensorToUse, OWNER_MPU);
    if (gyro->busDev == NULL) {
        return false;
    }

    if (!icm40609dDeviceDetect(gyro->busDev)) {
        busDeviceDeInit(gyro->busDev);
        return false;
    }

    // Magic number for ACC detection to indicate that we have detected icm40609d gyro
    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(gyro->busDev);
    ctx->chipMagicNumber = 0x4609;

    gyro->initFn = icm40609dAccAndGyroInit;
    gyro->readFn = icm40609dGyroRead;
    gyro->intStatusFn = gyroCheckDataReady;
    gyro->temperatureFn = icm40609dReadTemperature;
    gyro->scale = 1.0f / 16.4f;     // 16.4 dps/lsb scalefactor, matches GYRO_FS_SEL=0 (+/-2000dps)
    gyro->gyroAlign = gyro->busDev->param;

    return true;
}

#endif
