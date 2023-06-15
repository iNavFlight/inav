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
 * from atbetaflight https://github.com/flightng/atbetaflight
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
#include "drivers/accgyro/accgyro_lsm6dxx.h"

#if defined(USE_IMU_LSM6DXX)

typedef struct __attribute__ ((__packed__)) lsm6DContextData_s {
    uint16_t    chipMagicNumber;
    uint8_t     lastReadStatus;
    uint8_t     __padding_dummy;
    uint8_t     accRaw[6];
    uint8_t     gyroRaw[6];
} lsm6DContextData_t;

#define LSM6DSO_CHIP_ID 0x6C
#define LSM6DSL_CHIP_ID 0x6A
#define LSM6DS3_CHIP_ID 0x69

static uint8_t lsm6dID = 0x6C;

static void lsm6dxxWriteRegister(const  busDevice_t *dev, lsm6dxxRegister_e registerID, uint8_t value, unsigned delayMs)
{
    busWrite(dev, registerID, value);
    if (delayMs) {
        delay(delayMs);
    }
}

static void lsm6dxxWriteRegisterBits(const  busDevice_t *dev, lsm6dxxRegister_e registerID, lsm6dxxConfigMasks_e mask, uint8_t value, unsigned delayMs)
{
    uint8_t newValue;
    if (busRead(dev, registerID, &newValue)) {
        delayMicroseconds(2);
        newValue = (newValue & ~mask) | value;
        lsm6dxxWriteRegister(dev, registerID, newValue, delayMs);
    }
}

static uint8_t getLsmDlpfBandwidth(gyroDev_t *gyro)
{
    switch(gyro->lpf) {
        case GYRO_HARDWARE_LPF_NORMAL:
            return LSM6DXX_VAL_CTRL6_C_FTYPE_201HZ;
        case GYRO_HARDWARE_LPF_OPTION_1:
            return LSM6DXX_VAL_CTRL6_C_FTYPE_300HZ;
        case GYRO_HARDWARE_LPF_OPTION_2:
            return LSM6DXX_VAL_CTRL6_C_FTYPE_603HZ;
        case GYRO_HARDWARE_LPF_EXPERIMENTAL:
            return LSM6DXX_VAL_CTRL6_C_FTYPE_603HZ;
    }
    return 0;
}

static void lsm6dxxConfig(gyroDev_t *gyro)
{ 
    busDevice_t * dev = gyro->busDev;
    const gyroFilterAndRateConfig_t * config = mpuChooseGyroConfig(gyro->lpf, 1000000 / gyro->requestedSampleIntervalUs);
    gyro->sampleRateIntervalUs = 1000000 / config->gyroRateHz;

    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);
    // Reset the device (wait 100ms before continuing config)
    lsm6dxxWriteRegisterBits(dev, LSM6DXX_REG_CTRL3_C, LSM6DXX_MASK_CTRL3_C_RESET, BIT(0), 100);

    // Configure data ready pulsed mode
    lsm6dxxWriteRegisterBits(dev, LSM6DXX_REG_COUNTER_BDR1, LSM6DXX_MASK_COUNTER_BDR1, LSM6DXX_VAL_COUNTER_BDR1_DDRY_PM, 0);
 
    // Configure interrupt pin 1 for gyro data ready only
    lsm6dxxWriteRegister(dev, LSM6DXX_REG_INT1_CTRL, LSM6DXX_VAL_INT1_CTRL, 1);

    // Disable interrupt pin 2
    lsm6dxxWriteRegister(dev, LSM6DXX_REG_INT2_CTRL, LSM6DXX_VAL_INT2_CTRL, 1);

    // Configure the accelerometer
    // 833hz ODR, 16G scale, use LPF2 output (default with ODR/4 cutoff)
    lsm6dxxWriteRegister(dev, LSM6DXX_REG_CTRL1_XL, (LSM6DXX_VAL_CTRL1_XL_ODR833 << 4) | (LSM6DXX_VAL_CTRL1_XL_16G << 2) | (LSM6DXX_VAL_CTRL1_XL_LPF2 << 1), 1);

    // Configure the gyro
    // 6664hz ODR, 2000dps scale
    lsm6dxxWriteRegister(dev, LSM6DXX_REG_CTRL2_G, (LSM6DXX_VAL_CTRL2_G_ODR6664 << 4) | (LSM6DXX_VAL_CTRL2_G_2000DPS << 2), 1);

    // Configure control register 3
    // latch LSB/MSB during reads; set interrupt pins active high; set interrupt pins push/pull; set 4-wire SPI; enable auto-increment burst reads
    lsm6dxxWriteRegisterBits(dev, LSM6DXX_REG_CTRL3_C, LSM6DXX_MASK_CTRL3_C, (LSM6DXX_VAL_CTRL3_C_H_LACTIVE | LSM6DXX_VAL_CTRL3_C_PP_OD | LSM6DXX_VAL_CTRL3_C_SIM | LSM6DXX_VAL_CTRL3_C_IF_INC), 1);

    // Configure control register 4
    // enable accelerometer high performane mode; enable gyro LPF1
    lsm6dxxWriteRegisterBits(dev, LSM6DXX_REG_CTRL4_C, LSM6DXX_MASK_CTRL4_C, (LSM6DXX_VAL_CTRL4_C_DRDY_MASK | LSM6DXX_VAL_CTRL4_C_I2C_DISABLE | LSM6DXX_VAL_CTRL4_C_LPF1_SEL_G), 1);

 

    // Configure control register 6
    // disable I2C interface; set gyro LPF1 cutoff according to gyro_hardware_lpf setting
    lsm6dxxWriteRegisterBits(dev, LSM6DXX_REG_CTRL6_C, (lsm6dID == LSM6DSO_CHIP_ID? LSM6DXX_MASK_CTRL6_C:LSM6DSL_MASK_CTRL6_C), (LSM6DXX_VAL_CTRL6_C_XL_HM_MODE | getLsmDlpfBandwidth(gyro)), 1);

    // Configure control register 7
    lsm6dxxWriteRegisterBits(dev, LSM6DXX_REG_CTRL7_G, LSM6DXX_MASK_CTRL7_G, (LSM6DXX_VAL_CTRL7_G_HP_EN_G | LSM6DXX_VAL_CTRL7_G_HPM_G_16), 1);

    // Configure control register 9
    // disable I3C interface
    if(lsm6dID == LSM6DSO_CHIP_ID)
    {
        lsm6dxxWriteRegisterBits(dev, LSM6DXX_REG_CTRL9_XL, LSM6DXX_MASK_CTRL9_XL, LSM6DXX_VAL_CTRL9_XL_I3C_DISABLE, 1);
    }

    busSetSpeed(dev, BUS_SPEED_FAST);
}



static bool lsm6dxxDetect(busDevice_t * dev)
{
    uint8_t tmp;
    uint8_t attemptsRemaining = 5;
    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);
    do {
        delay(150);

        busRead(dev, LSM6DXX_REG_WHO_AM_I, &tmp);

        switch (tmp) {
            case LSM6DSO_CHIP_ID:
            case LSM6DSL_CHIP_ID: 
                 lsm6dID = tmp;
                // Compatible chip detected
                return true;
            default:
                // Retry detection
                break;
        }
    } while (attemptsRemaining--);

    return false;
}

static void lsm6dxxSpiGyroInit(gyroDev_t *gyro)
{   
    lsm6dxxConfig(gyro);
}

static void lsm6dxxSpiAccInit(accDev_t *acc)
{
    // sensor is configured during gyro init
    acc->acc_1G = 512 * 4;   // 16G sensor scale
}

static bool lsm6dxxAccRead(accDev_t *acc)
{
    uint8_t data[6];
    const bool ack = busReadBuf(acc->busDev, LSM6DXX_REG_OUTX_L_A, data, 6);
    if (!ack) {
        return false;
    }
    acc->ADCRaw[X] = (float) int16_val_little_endian(data, 0);
    acc->ADCRaw[Y] = (float) int16_val_little_endian(data, 1);
    acc->ADCRaw[Z] = (float) int16_val_little_endian(data, 2);
    return true; 
}

static bool lsm6dxxGyroRead(gyroDev_t *gyro)
{
    uint8_t data[6];
    const bool ack = busReadBuf(gyro->busDev, LSM6DXX_REG_OUTX_L_G, data, 6);
    if (!ack) {
        return false;
    }
    gyro->gyroADCRaw[X] = (float) int16_val_little_endian(data, 0);
    gyro->gyroADCRaw[Y] = (float) int16_val_little_endian(data, 1);
    gyro->gyroADCRaw[Z] = (float) int16_val_little_endian(data, 2);
    return true;
}

// Init Gyro first,then Acc
bool lsm6dGyroDetect(gyroDev_t *gyro)
{
    gyro->busDev = busDeviceInit(BUSTYPE_SPI, DEVHW_LSM6D, gyro->imuSensorToUse, OWNER_MPU);
    if (gyro->busDev == NULL) {
        return false;
    }

    if (!lsm6dxxDetect(gyro->busDev)) {
        busDeviceDeInit(gyro->busDev);
        return false;
    }

    lsm6DContextData_t * ctx = busDeviceGetScratchpadMemory(gyro->busDev);
    ctx->chipMagicNumber = 0xD6;

    gyro->initFn = lsm6dxxSpiGyroInit;
    gyro->readFn = lsm6dxxGyroRead;
    gyro->intStatusFn = gyroCheckDataReady;
    gyro->scale = 1.0f / 16.4f; // 2000 dps
    return true;

}
bool lsm6dAccDetect(accDev_t *acc)
{
    acc->busDev = busDeviceOpen(BUSTYPE_SPI, DEVHW_LSM6D, acc->imuSensorToUse);
    if (acc->busDev == NULL) {
        return false;
    }

    lsm6DContextData_t * ctx = busDeviceGetScratchpadMemory(acc->busDev);
    if (ctx->chipMagicNumber != 0xD6) {
        return false;
    }
    acc->initFn = lsm6dxxSpiAccInit;
    acc->readFn = lsm6dxxAccRead;
    acc->accAlign = acc->busDev->param;

    return true;
}



#endif
