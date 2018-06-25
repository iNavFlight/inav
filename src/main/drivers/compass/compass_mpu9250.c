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
#include <string.h>

#include <math.h>

#include "platform.h"

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/bus.h"
#include "drivers/sensor.h"
#include "drivers/system.h"
#include "drivers/time.h"

#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_mpu.h"

#include "drivers/compass/compass.h"
#include "drivers/compass/compass_mpu9250.h"

#if defined(USE_MAG_MPU9250) && defined(USE_GYRO_MPU9250)

// No separate hardware descriptor needed. Hardware descriptor initialization is handled by GYRO driver

// AK8963, mag sensor address
#define AK8963_MAG_I2C_ADDRESS          0x0C
#define AK8963_DEVICE_ID                0x48

// Registers
#define AK8963_MAG_REG_WHO_AM_I         0x00
#define AK8963_MAG_REG_INFO             0x01
#define AK8963_MAG_REG_STATUS1          0x02
#define AK8963_MAG_REG_HXL              0x03
#define AK8963_MAG_REG_HXH              0x04
#define AK8963_MAG_REG_HYL              0x05
#define AK8963_MAG_REG_HYH              0x06
#define AK8963_MAG_REG_HZL              0x07
#define AK8963_MAG_REG_HZH              0x08
#define AK8963_MAG_REG_STATUS2          0x09
#define AK8963_MAG_REG_CNTL             0x0a
#define AK8963_MAG_REG_ASCT             0x0c // self test
#define AK8963_MAG_REG_ASAX             0x10 // Fuse ROM x-axis sensitivity adjustment value
#define AK8963_MAG_REG_ASAY             0x11 // Fuse ROM y-axis sensitivity adjustment value
#define AK8963_MAG_REG_ASAZ             0x12 // Fuse ROM z-axis sensitivity adjustment value

#define READ_FLAG                       0x80

#define STATUS1_DATA_READY              0x01
#define STATUS1_DATA_OVERRUN            0x02

#define STATUS2_MAG_SENSOR_OVERFLOW     0x08

#define CNTL_MODE_POWER_DOWN            0x00
#define CNTL_MODE_ONCE                  0x01
#define CNTL_MODE_CONT1                 0x02
#define CNTL_MODE_CONT2                 0x06
#define CNTL_MODE_SELF_TEST             0x08
#define CNTL_MODE_FUSE_ROM              0x0F
#define CNTL_BIT_14_BIT                 0x00
#define CNTL_BIT_16_BIT                 0x10

#define DETECTION_MAX_RETRY_COUNT   5

typedef enum {
    CHECK_STATUS = 0,
    WAITING_FOR_STATUS,
    WAITING_FOR_DATA
} mpu9250CompassReadState_e;

typedef struct {
    mpu9250CompassReadState_e   state;
    bool                        waiting;
    uint8_t                     len;
    timeUs_t                    readStartedAt; // time read was queued in micros.
} mpu9250CompassReadContext_s;

static int16_t magGain[3];
static mpu9250CompassReadContext_s  ctx;
static bool lastReadResult = false;
static int16_t cachedMagData[3];

static bool mpu9250SlaveI2CRead(magDev_t * mag, uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    // Setting up MPU9250's I2C master to read from AK8963 via internal I2C bus
    busWrite(mag->busDev, MPU_RA_I2C_SLV0_ADDR, addr | READ_FLAG);     // set I2C slave address for read
    busWrite(mag->busDev, MPU_RA_I2C_SLV0_REG, reg);                   // set I2C slave register
    busWrite(mag->busDev, MPU_RA_I2C_SLV0_CTRL, len | 0x80);           // read number of bytes
    delay(10);                                                      // wait for transaction to complete
    busReadBuf(mag->busDev, MPU_RA_EXT_SENS_DATA_00, buf, len);        // read I2C
    return true;
}

static bool mpu9250SlaveI2CWrite(magDev_t * mag, uint8_t addr, uint8_t reg, uint8_t data)
{
    // Setting up MPU9250's I2C master to write to AK8963 via internal I2C bus
    busWrite(mag->busDev, MPU_RA_I2C_SLV0_ADDR, addr);         // set I2C slave address for write
    busWrite(mag->busDev, MPU_RA_I2C_SLV0_REG, reg);           // set I2C slave register
    busWrite(mag->busDev, MPU_RA_I2C_SLV0_DO, data);           // set I2C salve value
    busWrite(mag->busDev, MPU_RA_I2C_SLV0_CTRL, 0x81);         // write 1 byte
    return true;
}

static bool mpu9250SlaveI2CStartRead(magDev_t * mag, uint8_t addr, uint8_t reg, uint8_t len)
{
    if (ctx.waiting) {
        return false;
    }

    ctx.len = len;

    busWrite(mag->busDev, MPU_RA_I2C_SLV0_ADDR, addr | READ_FLAG);     // set I2C slave address for read
    busWrite(mag->busDev, MPU_RA_I2C_SLV0_REG, reg);                   // set I2C slave register
    busWrite(mag->busDev, MPU_RA_I2C_SLV0_CTRL, len | 0x80);           // read number of bytes

    ctx.readStartedAt = micros();
    ctx.waiting = true;
    return true;
}

static timeDelta_t mpu9250SlaveI2CReadTimeRemaining(void)
{
    if (!ctx.waiting) {
        return 0;
    }

    timeDelta_t timeSinceStarted = micros() - ctx.readStartedAt;
    timeDelta_t timeRemaining = 8000 - timeSinceStarted;

    if (timeRemaining < 0) {
        return 0;
    }

    return timeRemaining;
}

static bool mpu9250SlaveI2CCompleteRead(magDev_t * mag, uint8_t * buf)
{
    timeDelta_t timeRemaining = mpu9250SlaveI2CReadTimeRemaining();

    if (timeRemaining > 0) {
        delayMicroseconds(timeRemaining);
    }

    ctx.waiting = false;
    busReadBuf(mag->busDev, MPU_RA_EXT_SENS_DATA_00, buf, ctx.len);
    return true;
}

static bool mpu9250CompassInit(magDev_t * mag)
{
    uint8_t calibration[3];
    uint8_t status;

    // Do init at low speed
    busSetSpeed(mag->busDev, BUS_SPEED_INITIALIZATION);

    mpu9250SlaveI2CWrite(mag, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_CNTL, CNTL_MODE_POWER_DOWN); // power down before entering fuse mode
    delay(20);

    mpu9250SlaveI2CWrite(mag, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_CNTL, CNTL_MODE_FUSE_ROM); // Enter Fuse ROM access mode
    delay(10);

    mpu9250SlaveI2CRead(mag, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_ASAX, calibration, sizeof(calibration)); // Read the x-, y-, and z-axis calibration values
    delay(10);

    magGain[X] = calibration[X] + 128;
    magGain[Y] = calibration[Y] + 128;
    magGain[Z] = calibration[Z] + 128;

    mpu9250SlaveI2CWrite(mag, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_CNTL, CNTL_MODE_POWER_DOWN); // power down after reading.
    delay(10);

    // Clear status registers
    mpu9250SlaveI2CRead(mag, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_STATUS1, &status, 1);
    mpu9250SlaveI2CRead(mag, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_STATUS2, &status, 1);

    // Trigger first measurement
    mpu9250SlaveI2CWrite(mag, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_CNTL, CNTL_BIT_16_BIT | CNTL_MODE_CONT1);

    busSetSpeed(mag->busDev, BUS_SPEED_FAST);
    return true;
}

static int16_t parseMag(uint8_t *raw, int16_t gain) {
  int ret = (int16_t)(raw[1] << 8 | raw[0]) * gain / 256;
  return constrain(ret, INT16_MIN, INT16_MAX);
}

static bool mpu9250CompassRead(magDev_t * mag)
{
    bool ack = false;
    uint8_t buf[7];

    // set magData to latest cached value
    memcpy(&mag->magADCRaw, cachedMagData, sizeof(cachedMagData));

    bool reprocess;

    do {
        reprocess = false;

        switch (ctx.state) {
            case CHECK_STATUS:
                mpu9250SlaveI2CStartRead(mag, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_STATUS1, 1);
                ctx.state = WAITING_FOR_STATUS;
                return lastReadResult;

            case WAITING_FOR_STATUS: {
                uint32_t timeRemaining = mpu9250SlaveI2CReadTimeRemaining();
                if (timeRemaining) {
                    return lastReadResult;
                }

                ack = mpu9250SlaveI2CCompleteRead(mag, &buf[0]);
                uint8_t status = buf[0];

                if (!ack || ((status & STATUS1_DATA_READY) == 0 && (status & STATUS1_DATA_OVERRUN) == 0)) {
                    // too early. queue the status read again
                    ctx.state = CHECK_STATUS;
                    reprocess = true;
                    return lastReadResult;
                }

                // read the 6 bytes of data and the status2 register
                mpu9250SlaveI2CStartRead(mag, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_HXL, 7);
                ctx.state = WAITING_FOR_DATA;
                return lastReadResult;
            }

            case WAITING_FOR_DATA: {
                uint32_t timeRemaining = mpu9250SlaveI2CReadTimeRemaining();
                if (timeRemaining) {
                    return lastReadResult;
                }

                ack = mpu9250SlaveI2CCompleteRead(mag, &buf[0]);
                break;
            }
        }
    } while(reprocess);

    uint8_t status2 = buf[6];
    if (!ack || (status2 & STATUS2_MAG_SENSOR_OVERFLOW)) {
        ctx.state = CHECK_STATUS;
        lastReadResult = false;
        return lastReadResult;
    }

    mag->magADCRaw[X] = -parseMag(buf + 0, magGain[X]);
    mag->magADCRaw[Y] = parseMag(buf + 2, magGain[Y]);
    mag->magADCRaw[Z] = -parseMag(buf + 4, magGain[Z]);

    memcpy(cachedMagData, &mag->magADCRaw, sizeof(cachedMagData));
    ctx.state = CHECK_STATUS;
    lastReadResult = true;

    return lastReadResult;
}

bool mpu9250CompassDetect(magDev_t * mag)
{
    // Compass on MPU9250 is only supported if MPU9250 is connected to SPI bus
    // FIXME: We need to use gyro_to_use here, not mag_to_use
    mag->busDev = busDeviceOpen(BUSTYPE_SPI, DEVHW_MPU9250, mag->magSensorToUse);
    if (mag->busDev == NULL) {
        return false;
    }

    // Check if Gyro driver initialized the chip
    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(mag->busDev);
    if (ctx->chipMagicNumber != 0x9250) {
        return false;
    }

    busSetSpeed(mag->busDev, BUS_SPEED_INITIALIZATION);
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        bool ack = false;
        uint8_t sig = 0;

        // Initialize I2C master via SPI bus (MPU9250)
        busWrite(mag->busDev, MPU_RA_INT_PIN_CFG, 0x10);       // INT_ANYRD_2CLEAR
        delay(15);

        busWrite(mag->busDev, MPU_RA_I2C_MST_CTRL, 0x0D);      // I2C multi-master / 400kHz
        delay(15);

        busWrite(mag->busDev, MPU_RA_USER_CTRL, 0x30);         // I2C master mode, SPI mode only
        delay(15);

        // check for AK8963
        ack = mpu9250SlaveI2CRead(mag, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_WHO_AM_I, &sig, 1);
        if (ack && sig == AK8963_DEVICE_ID) { // 0x48 / 01001000 / 'H'
            mag->init = mpu9250CompassInit;
            mag->read = mpu9250CompassRead;
            return true;
        }
    }

    busSetSpeed(mag->busDev, BUS_SPEED_FAST);
    return false;
}

#endif
