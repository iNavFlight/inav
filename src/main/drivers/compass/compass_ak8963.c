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

#if defined(USE_MAG_AK8963) || defined(USE_MAG_SPI_AK8963)

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/io.h"
#include "drivers/bus_i2c.h"
#include "drivers/bus_spi.h"

#include "drivers/sensor.h"
#include "drivers/compass/compass.h"

#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_mpu.h"
#include "drivers/accgyro/accgyro_mpu6500.h"
#include "drivers/accgyro/accgyro_spi_mpu6500.h"
#include "drivers/compass/compass_ak8963.h"
#include "drivers/compass/compass_spi_ak8963.h"

static int16_t magGain[XYZ_AXIS_COUNT] = { 256, 256, 256 };

#if defined(USE_SPI) && defined(MPU6500_SPI_INSTANCE)
#define DISABLE_SPI_MPU(spiCsnPin)  IOHi(spiCsnPin)
#define ENABLE_SPI_MPU(spiCsnPin)   IOLo(spiCsnPin)
#define MPU_SPI_INSTANCE            MPU6500_SPI_INSTANCE

typedef struct queuedReadState_s {
    bool waiting;
    uint8_t len;
    timeUs_t readStartedAt; // time read was queued in micros.
} queuedReadState_t;

typedef enum {
    CHECK_STATUS = 0,
    WAITING_FOR_STATUS,
    WAITING_FOR_DATA
} ak8963ReadState_e;

static queuedReadState_t queuedRead = { false, 0, 0};

/* We have AK8963 connected internally to MPU9250 I2C master. Accessing the compass sensor requires
 * setting up the MPU's I2C host. We have separate implementation of SPI read/write functions to access
 * the MPU registers
 */
static bool mpuSpiWriteRegister(const busDevice_t *bus, uint8_t reg, uint8_t data)
{
    ENABLE_SPI_MPU(bus->busdev.spi.csnPin);
    delayMicroseconds(1);
    spiTransferByte(MPU_SPI_INSTANCE, reg & 0x7F);
    spiTransferByte(MPU_SPI_INSTANCE, data);
    DISABLE_SPI_MPU(bus->busdev.spi.csnPin);
    delayMicroseconds(1);

    return true;
}

static bool mpuSpiReadRegister(const busDevice_t *bus, uint8_t reg, uint8_t *data, uint8_t length)
{
    ENABLE_SPI_MPU(bus->busdev.spi.csnPin);
    spiTransferByte(MPU_SPI_INSTANCE, reg | READ_FLAG); // read transaction
    spiTransfer(MPU_SPI_INSTANCE, data, NULL, length);
    DISABLE_SPI_MPU(bus->busdev.spi.csnPin);

    return true;
}

static bool ak8963SensorRead(magDev_t *magDev, uint8_t addr_, uint8_t reg_, uint8_t *buf, uint8_t len_)
{
    // Setting up MPU9250's I2C master to read from AK8963 via internal I2C bus
    mpuSpiWriteRegister(&magDev->bus, MPU_RA_I2C_SLV0_ADDR, addr_ | READ_FLAG);   // set I2C slave address for read
    mpuSpiWriteRegister(&magDev->bus, MPU_RA_I2C_SLV0_REG, reg_);                 // set I2C slave register
    mpuSpiWriteRegister(&magDev->bus, MPU_RA_I2C_SLV0_CTRL, (len_ & 0x0F) | I2C_SLV0_EN); // read number of bytes
    delay(10);                                                                    // wait for transaction to complete
    __disable_irq();
    mpuSpiReadRegister(&magDev->bus, MPU_RA_EXT_SENS_DATA_00, buf, len_);         // read I2C
    __enable_irq();
    return true;
}

static bool ak8963SensorWrite(magDev_t *magDev, uint8_t addr_, uint8_t reg_, uint8_t data)
{
    // Setting up MPU9250's I2C master to write to AK8963 via internal I2C bus
    mpuSpiWriteRegister(&magDev->bus, MPU_RA_I2C_SLV0_ADDR, addr_);               // set I2C slave address for write
    mpuSpiWriteRegister(&magDev->bus, MPU_RA_I2C_SLV0_REG, reg_);                 // set I2C slave register
    mpuSpiWriteRegister(&magDev->bus, MPU_RA_I2C_SLV0_DO, data);                  // set I2C salve value
    mpuSpiWriteRegister(&magDev->bus, MPU_RA_I2C_SLV0_CTRL, (1 & 0x0F) | I2C_SLV0_EN); // write 1 byte
    return true;
}

static bool ak8963SensorStartRead(magDev_t *magDev, uint8_t addr_, uint8_t reg_, uint8_t len_)
{
    // Setting up a read. We can't busy-wait with delay() when in normal operation.
    // Instead we'll set up a read and raise the flag to finalize the read after certain timeout

    if (queuedRead.waiting) {
        return false;
    }

    queuedRead.len = len_;

    mpuSpiWriteRegister(&magDev->bus, MPU_RA_I2C_SLV0_ADDR, addr_ | READ_FLAG);   // set I2C slave address for read
    mpuSpiWriteRegister(&magDev->bus, MPU_RA_I2C_SLV0_REG, reg_);                 // set I2C slave register
    mpuSpiWriteRegister(&magDev->bus, MPU_RA_I2C_SLV0_CTRL, (len_ & 0x0F) | I2C_SLV0_EN);  // read number of bytes

    queuedRead.readStartedAt = micros();
    queuedRead.waiting = true;

    return true;
}

static timeDelta_t ak8963SensorQueuedReadTimeRemaining(void)
{
    if (!queuedRead.waiting) {
        return 0;
    }

    timeDelta_t timeSinceStarted = micros() - queuedRead.readStartedAt;

    timeDelta_t timeRemaining = 8000 - timeSinceStarted;

    if (timeRemaining < 0) {
        return 0;
    }

    return timeRemaining;
}

static bool ak8963SensorCompleteRead(magDev_t *magDev, uint8_t *buf)
{
    // Finalizing the read of AK8963 registers via MPU9250's built-in I2C master
    timeDelta_t timeRemaining = ak8963SensorQueuedReadTimeRemaining();

    if (timeRemaining > 0) {
        delayMicroseconds(timeRemaining);
    }

    queuedRead.waiting = false;

    mpuSpiReadRegister(&magDev->bus, MPU_RA_EXT_SENS_DATA_00, buf, queuedRead.len);               // read I2C buffer
    return true;
}
#else
static bool ak8963SensorRead(magDev_t *magDev, uint8_t addr_, uint8_t reg_, uint8_t* buf, uint8_t len)
{
    UNUSED(magDev);
    return i2cRead(MAG_I2C_INSTANCE, addr_, reg_, len, buf);
}

static bool ak8963SensorWrite(magDev_t *magDev, uint8_t addr_, uint8_t reg_, uint8_t data)
{
    UNUSED(magDev);
    return i2cWrite(MAG_I2C_INSTANCE, addr_, reg_, data);
}
#endif

static bool ak8963Init(magDev_t *magDev)
{
    bool ack;
    UNUSED(ack);
    uint8_t calibration[XYZ_AXIS_COUNT];
    uint8_t status;

    ack = ak8963SensorWrite(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_CNTL1, CNTL1_MODE_POWER_DOWN); // power down before entering fuse mode
    delay(20);

    ack = ak8963SensorWrite(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_CNTL1, CNTL1_MODE_FUSE_ROM); // Enter Fuse ROM access mode
    delay(10);

    ack = ak8963SensorRead(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_ASAX, calibration, sizeof(calibration)); // Read the x-, y-, and z-axis calibration values
    delay(10);

    magGain[X] = calibration[X] + 128;
    magGain[Y] = calibration[Y] + 128;
    magGain[Z] = calibration[Z] + 128;

    ack = ak8963SensorWrite(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_CNTL1, CNTL1_MODE_POWER_DOWN); // power down after reading.
    delay(10);

    // Clear status registers
    ack = ak8963SensorRead(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_ST1, &status, 1);
    ack = ak8963SensorRead(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_ST2, &status, 1);

    // Trigger first measurement
#if defined(USE_SPI) && defined(MPU6500_SPI_INSTANCE)
    ack = ak8963SensorWrite(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_CNTL1, CNTL1_BIT_16_BIT | CNTL1_MODE_CONT1);
#else
    ack = ak8963SensorWrite(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_CNTL1, CNTL1_BIT_16_BIT | CNTL1_MODE_ONCE);
#endif
    return true;
}

int16_t parseMag(uint8_t *raw, int16_t gain) {
  int ret = (int16_t)(raw[1] << 8 | raw[0]) * gain / 256;
  return constrain(ret, INT16_MIN, INT16_MAX);
}

static bool ak8963Read(magDev_t *magDev)
{
    bool ack = false;
    uint8_t buf[7];

    static bool lastReadResult = false;

#if defined(USE_SPI) && defined(MPU6500_SPI_INSTANCE)
    static int16_t cachedMagData[XYZ_AXIS_COUNT];

    // set magData to latest cached value
    memcpy(&magDev->magADCRaw, cachedMagData, sizeof(cachedMagData));

    // we currently need a different approach for the MPU9250 connected via SPI.
    // we cannot use the ak8963SensorRead() method for SPI, it is to slow and blocks for far too long.

    static ak8963ReadState_e state = CHECK_STATUS;

    bool retry = true;

restart:
    switch (state) {
        case CHECK_STATUS:
            ak8963SensorStartRead(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_ST1, 1);
            state++;
            return lastReadResult;

        case WAITING_FOR_STATUS: {
            uint32_t timeRemaining = ak8963SensorQueuedReadTimeRemaining();
            if (timeRemaining) {
                return lastReadResult;
            }

            ack = ak8963SensorCompleteRead(magDev, &buf[0]);

            uint8_t status = buf[0];

            if (!ack || ((status & ST1_DATA_READY) == 0 && (status & ST1_DATA_OVERRUN) == 0)) {
                // too early. queue the status read again
                state = CHECK_STATUS;
                if (retry) {
                    retry = false;
                    goto restart;
                }

                lastReadResult = false;
                return lastReadResult;
            }

            // read the 6 bytes of data and the status2 register
            ak8963SensorStartRead(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_HXL, 7);

            state++;

            return lastReadResult;
        }

        case WAITING_FOR_DATA: {
            uint32_t timeRemaining = ak8963SensorQueuedReadTimeRemaining();
            if (timeRemaining) {
                return lastReadResult;
            }

            ack = ak8963SensorCompleteRead(magDev, &buf[0]);
        }
    }
#else
    ack = ak8963SensorRead(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_ST1, &buf[0], 1);

    uint8_t status = buf[0];

    if (!ack || (status & ST1_DATA_READY) == 0) {
        lastReadResult = false;
        return lastReadResult;
    }

    ack = ak8963SensorRead(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_HXL, &buf[0], 7);
#endif

#if defined(USE_SPI) && defined(MPU6500_SPI_INSTANCE)
    lastReadResult = true;
#else
    lastReadResult = ak8963SensorWrite(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_CNTL1, CNTL1_BIT_16_BIT | CNTL1_MODE_ONCE); // start reading again
#endif

    uint8_t status2 = buf[6];
    if (!ack || (status2 & ST2_MAG_SENSOR_OVERFLOW)) {
        lastReadResult = false;
        return lastReadResult;
    }

    magDev->magADCRaw[X] = -parseMag(buf + 0, magGain[X]);
    magDev->magADCRaw[Y] = -parseMag(buf + 2, magGain[Y]);
    magDev->magADCRaw[Z] = -parseMag(buf + 4, magGain[Z]);

#if defined(USE_SPI) && defined(MPU6500_SPI_INSTANCE)
    // cache mag data for reuse
    memcpy(cachedMagData, &magDev->magADCRaw, sizeof(cachedMagData));
    state = CHECK_STATUS;

#endif

    return lastReadResult;
}

#define DETECTION_MAX_RETRY_COUNT   5
bool ak8963Detect(magDev_t *magDev)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        bool ack = false;
        uint8_t sig = 0;

#if defined(USE_SPI) && defined(AK8963_SPI_INSTANCE)
        // check for SPI AK8963
        if (ak8963SpiDetect(magDev)) {
            return true;
        }
#endif

#if defined(USE_SPI) && defined(MPU6500_SPI_INSTANCE)
        // Initialize I2C master via SPI bus (MPU9250)
        ack = mpuSpiWriteRegister(&magDev->bus, MPU_RA_INT_PIN_CFG, 0x10);               // INT_ANYRD_2CLEAR
        delay(10);

        ack = mpuSpiWriteRegister(&magDev->bus, MPU_RA_I2C_MST_CTRL, 0x0D);              // I2C multi-master / 400kHz
        delay(10);

        ack = mpuSpiWriteRegister(&magDev->bus, MPU_RA_USER_CTRL, 0x30);                 // I2C master mode, SPI mode only
        delay(10);
#endif

        ack = ak8963SensorWrite(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_CNTL2, CNTL2_SOFT_RESET); // reset MAG
        delay(4);

        // check for AK8963
        ack = ak8963SensorRead(magDev, AK8963_MAG_I2C_ADDRESS, AK8963_MAG_REG_WIA, &sig, 1);
        if (ack && sig == AK8963_DEVICE_ID) { // 0x48 / 01001000 / 'H'
            magDev->init = ak8963Init;
            magDev->read = ak8963Read;
            return true;
        }
    }

    return false;
}
#endif
