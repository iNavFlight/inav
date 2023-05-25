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

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/system.h"
#include "drivers/time.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_mpu.h"
#include "drivers/accgyro/accgyro_icm42605.h"

#if defined(USE_IMU_ICM42605)

#define ICM42605_RA_PWR_MGMT0                       0x4E

#define ICM42605_PWR_MGMT0_ACCEL_MODE_LN            (3 << 0)
#define ICM42605_PWR_MGMT0_GYRO_MODE_LN             (3 << 2)
#define ICM42605_PWR_MGMT0_TEMP_DISABLE_OFF         (0 << 5)
#define ICM42605_PWR_MGMT0_TEMP_DISABLE_ON          (1 << 5)

#define ICM42605_RA_GYRO_CONFIG0                    0x4F
#define ICM42605_RA_ACCEL_CONFIG0                   0x50

#define ICM42605_RA_GYRO_ACCEL_CONFIG0              0x52

#define ICM42605_ACCEL_UI_FILT_BW_LOW_LATENCY       (14 << 4)
#define ICM42605_GYRO_UI_FILT_BW_LOW_LATENCY        (14 << 0)

#define ICM42605_RA_GYRO_DATA_X1                    0x25
#define ICM42605_RA_ACCEL_DATA_X1                   0x1F

#define ICM42605_RA_INT_CONFIG                      0x14
#define ICM42605_INT1_MODE_PULSED                   (0 << 2)
#define ICM42605_INT1_MODE_LATCHED                  (1 << 2)
#define ICM42605_INT1_DRIVE_CIRCUIT_OD              (0 << 1)
#define ICM42605_INT1_DRIVE_CIRCUIT_PP              (1 << 1)
#define ICM42605_INT1_POLARITY_ACTIVE_LOW           (0 << 0)
#define ICM42605_INT1_POLARITY_ACTIVE_HIGH          (1 << 0)

#define ICM42605_RA_INT_CONFIG0                     0x63
#define ICM42605_UI_DRDY_INT_CLEAR_ON_SBR           ((0 << 5) || (0 << 4))
#define ICM42605_UI_DRDY_INT_CLEAR_ON_SBR_DUPLICATE ((0 << 5) || (0 << 4)) // duplicate settings in datasheet, Rev 1.2.
#define ICM42605_UI_DRDY_INT_CLEAR_ON_F1BR          ((1 << 5) || (0 << 4))
#define ICM42605_UI_DRDY_INT_CLEAR_ON_SBR_AND_F1BR  ((1 << 5) || (1 << 4))

#define ICM42605_RA_INT_CONFIG1                     0x64
#define ICM42605_INT_ASYNC_RESET_BIT                4
#define ICM42605_INT_TDEASSERT_DISABLE_BIT          5
#define ICM42605_INT_TDEASSERT_ENABLED              (0 << ICM42605_INT_TDEASSERT_DISABLE_BIT)
#define ICM42605_INT_TDEASSERT_DISABLED             (1 << ICM42605_INT_TDEASSERT_DISABLE_BIT)
#define ICM42605_INT_TPULSE_DURATION_BIT            6
#define ICM42605_INT_TPULSE_DURATION_100            (0 << ICM42605_INT_TPULSE_DURATION_BIT)
#define ICM42605_INT_TPULSE_DURATION_8              (1 << ICM42605_INT_TPULSE_DURATION_BIT)


#define ICM42605_RA_INT_SOURCE0                     0x65
#define ICM42605_UI_DRDY_INT1_EN_DISABLED           (0 << 3)
#define ICM42605_UI_DRDY_INT1_EN_ENABLED            (1 << 3)


static void icm42605AccInit(accDev_t *acc)
{
    acc->acc_1G = 512 * 4;
}

static bool icm42605AccRead(accDev_t *acc)
{
    uint8_t data[6];

    const bool ack = busReadBuf(acc->busDev, ICM42605_RA_ACCEL_DATA_X1, data, 6);
    if (!ack) {
        return false;
    }

    acc->ADCRaw[X] = (float) int16_val_big_endian(ctx->gyroRaw, 0);
    acc->ADCRaw[Y] = (float) int16_val_big_endian(ctx->gyroRaw, 1);
    acc->ADCRaw[Z] = (float) int16_val_big_endian(ctx->gyroRaw, 2);

    return true;
}

bool icm42605AccDetect(accDev_t *acc)
{
    acc->busDev = busDeviceOpen(BUSTYPE_ANY, DEVHW_ICM42605, acc->imuSensorToUse);
    if (acc->busDev == NULL) {
        return false;
    }

    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(acc->busDev);
    if (ctx->chipMagicNumber != 0x4265) {
        return false;
    }

    acc->initFn = icm42605AccInit;
    acc->readFn = icm42605AccRead;
    acc->accAlign = acc->busDev->param;

    return true;
}

static const gyroFilterAndRateConfig_t icm42605GyroConfigs[] = {
    /*                            DLPF  ODR */
    { GYRO_LPF_256HZ,   8000,   { 6,    3  } }, /* 400 Hz LPF */
    { GYRO_LPF_256HZ,   4000,   { 5,    4  } }, /* 250 Hz LPF */
    { GYRO_LPF_256HZ,   2000,   { 3,    5  } }, /* 250 Hz LPF */
    { GYRO_LPF_256HZ,   1000,   { 1,    6  } }, /* 250 Hz LPF */
    { GYRO_LPF_256HZ,    500,   { 0,    15 } }, /* 250 Hz LPF */

    { GYRO_LPF_188HZ,   1000,   { 3,   6  } },  /* 125 HZ */
    { GYRO_LPF_188HZ,    500,   { 1,   15 } },  /* 125 HZ */

    { GYRO_LPF_98HZ,    1000,   { 4,    6  } }, /* 100 HZ*/
    { GYRO_LPF_98HZ,     500,   { 2,    15 } }, /* 100 HZ*/

    { GYRO_LPF_42HZ,    1000,   { 6,    6  } }, /* 50 HZ */
    { GYRO_LPF_42HZ,     500,   { 4,    15 } },

    { GYRO_LPF_20HZ,    1000,   { 7,    6  } }, /* 25 HZ */
    { GYRO_LPF_20HZ,     500,   { 6,    15 } },

    { GYRO_LPF_10HZ,    1000,   { 7,    6  } }, /* 25 HZ */
    { GYRO_LPF_10HZ,     500,   { 7,    15 } }  /* 12.5 HZ */
};

static void icm42605AccAndGyroInit(gyroDev_t *gyro)
{
    busDevice_t * dev = gyro->busDev;
    const gyroFilterAndRateConfig_t * config = chooseGyroConfig(gyro->lpf, 1000000 / gyro->requestedSampleIntervalUs,
                                                                &icm42605GyroConfigs[0], ARRAYLEN(icm42605GyroConfigs));
    gyro->sampleRateIntervalUs = 1000000 / config->gyroRateHz;

    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);

    busWrite(dev, ICM42605_RA_PWR_MGMT0, ICM42605_PWR_MGMT0_TEMP_DISABLE_OFF | ICM42605_PWR_MGMT0_ACCEL_MODE_LN | ICM42605_PWR_MGMT0_GYRO_MODE_LN);
    delay(15);

    /* ODR and dynamic range */
    busWrite(dev, ICM42605_RA_GYRO_CONFIG0, (0x00) << 5 | (config->gyroConfigValues[1] & 0x0F));    /* 2000 deg/s */
    delay(15);

    busWrite(dev, ICM42605_RA_ACCEL_CONFIG0, (0x00) << 5 | (config->gyroConfigValues[1] & 0x0F));    /* 16 G deg/s */
    delay(15);

    /* LPF bandwidth */
    busWrite(dev, ICM42605_RA_GYRO_ACCEL_CONFIG0, (config->gyroConfigValues[1]) | (config->gyroConfigValues[1] << 4));
    delay(15);

    busWrite(dev, ICM42605_RA_INT_CONFIG, ICM42605_INT1_MODE_PULSED | ICM42605_INT1_DRIVE_CIRCUIT_PP | ICM42605_INT1_POLARITY_ACTIVE_HIGH);
    delay(15);

    busWrite(dev, ICM42605_RA_INT_CONFIG0, ICM42605_UI_DRDY_INT_CLEAR_ON_SBR);
    delay(100);

    uint8_t intConfig1Value;

    busWrite(dev, ICM42605_RA_INT_SOURCE0, ICM42605_UI_DRDY_INT1_EN_ENABLED);

    // Datasheet says: "User should change setting to 0 from default setting of 1, for proper INT1 and INT2 pin operation"
    busRead(dev, ICM42605_RA_INT_CONFIG1, &intConfig1Value);

    intConfig1Value &= ~(1 << ICM42605_INT_ASYNC_RESET_BIT);
    intConfig1Value |= (ICM42605_INT_TPULSE_DURATION_8 | ICM42605_INT_TDEASSERT_DISABLED);

    busWrite(dev, ICM42605_RA_INT_CONFIG1, intConfig1Value);
    delay(15);

    busSetSpeed(dev, BUS_SPEED_FAST);
}

static bool icm42605DeviceDetect(busDevice_t * dev)
{
    uint8_t tmp;
    uint8_t attemptsRemaining = 5;

    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);

    busWrite(dev, ICM42605_RA_PWR_MGMT0, 0x00);

    do {
        delay(150);

        busRead(dev, MPU_RA_WHO_AM_I, &tmp);

        switch (tmp) {
            /* ICM42605 and ICM42688P share the register structure*/
            case ICM42605_WHO_AM_I_CONST:
            case ICM42688P_WHO_AM_I_CONST:
                return true;

            default:
                // Retry detection
                break;
        }
    } while (attemptsRemaining--);

    return false;
}

static bool icm42605GyroRead(gyroDev_t *gyro)
{
    uint8_t data[6];

    const bool ack = busReadBuf(gyro->busDev, ICM42605_RA_GYRO_DATA_X1, data, 6);
    if (!ack) {
        return false;
    }

    gyro->gyroADCRaw[X] = (float) int16_val_big_endian(ctx->gyroRaw, 0);
    gyro->gyroADCRaw[Y] = (float) int16_val_big_endian(ctx->gyroRaw, 0);
    gyro->gyroADCRaw[Z] = (float) int16_val_big_endian(ctx->gyroRaw, 0);

    return true;
}

bool icm42605GyroDetect(gyroDev_t *gyro)
{
    gyro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_ICM42605, gyro->imuSensorToUse, OWNER_MPU);
    if (gyro->busDev == NULL) {
        return false;
    }

    if (!icm42605DeviceDetect(gyro->busDev)) {
        busDeviceDeInit(gyro->busDev);
        return false;
    }

    // Magic number for ACC detection to indicate that we have detected icm42605 gyro
    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(gyro->busDev);
    ctx->chipMagicNumber = 0x4265;

    gyro->initFn = icm42605AccAndGyroInit;
    gyro->readFn = icm42605GyroRead;
    gyro->intStatusFn = gyroCheckDataReady;
    gyro->temperatureFn = NULL;
    gyro->scale = 1.0f / 16.4f;     // 16.4 dps/lsb scalefactor
    gyro->gyroAlign = gyro->busDev->param;

    return true;
}

#endif
