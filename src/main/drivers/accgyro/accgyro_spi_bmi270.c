/*
 * This file is part of Cleanflight and Betaflight.
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
 */

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#if defined(USE_IMU_BMI270)

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/system.h"
#include "drivers/time.h"
#include "drivers/io.h"
#include "drivers/exti.h"
#include "drivers/bus.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_spi_bmi270.h"

// 10 MHz max SPI frequency
#define BMI270_MAX_SPI_CLK_HZ 10000000

#define BMI270_FIFO_FRAME_SIZE 6

#define BMI270_CONFIG_SIZE 328

// Declaration for the device config (microcode) that must be uploaded to the sensor
extern const uint8_t bmi270_maximum_fifo_config_file[BMI270_CONFIG_SIZE];

#define BMI270_CHIP_ID 0x24

// BMI270 registers (not the complete list)
typedef enum {
    BMI270_REG_CHIP_ID = 0x00,
    BMI270_REG_ERR_REG = 0x02,
    BMI270_REG_STATUS = 0x03,
    BMI270_REG_ACC_DATA_X_LSB = 0x0C,
    BMI270_REG_GYR_DATA_X_LSB = 0x12,
    BMI270_REG_SENSORTIME_0 = 0x18,
    BMI270_REG_SENSORTIME_1 = 0x19,
    BMI270_REG_SENSORTIME_2 = 0x1A,
    BMI270_REG_EVENT = 0x1B,
    BMI270_REG_INT_STATUS_0 = 0x1C,
    BMI270_REG_INT_STATUS_1 = 0x1D,
    BMI270_REG_INTERNAL_STATUS = 0x21,
    BMI270_REG_TEMPERATURE_LSB = 0x22,
    BMI270_REG_TEMPERATURE_MSB = 0x23,
    BMI270_REG_FIFO_LENGTH_LSB = 0x24,
    BMI270_REG_FIFO_LENGTH_MSB = 0x25,
    BMI270_REG_FIFO_DATA = 0x26,
    BMI270_REG_ACC_CONF = 0x40,
    BMI270_REG_ACC_RANGE = 0x41,
    BMI270_REG_GYRO_CONF = 0x42,
    BMI270_REG_GYRO_RANGE = 0x43,
    BMI270_REG_AUX_CONF = 0x44,
    BMI270_REG_FIFO_DOWNS = 0x45,
    BMI270_REG_FIFO_WTM_0 = 0x46,
    BMI270_REG_FIFO_WTM_1 = 0x47,
    BMI270_REG_FIFO_CONFIG_0 = 0x48,
    BMI270_REG_FIFO_CONFIG_1 = 0x49,
    BMI270_REG_SATURATION = 0x4A,
    BMI270_REG_INT1_IO_CTRL = 0x53,
    BMI270_REG_INT2_IO_CTRL = 0x54,
    BMI270_REG_INT_LATCH = 0x55,
    BMI270_REG_INT1_MAP_FEAT = 0x56,
    BMI270_REG_INT2_MAP_FEAT = 0x57,
    BMI270_REG_INT_MAP_DATA = 0x58,
    BMI270_REG_INIT_CTRL = 0x59,
    BMI270_REG_INIT_DATA = 0x5E,
    BMI270_REG_ACC_SELF_TEST = 0x6D,
    BMI270_REG_GYR_SELF_TEST_AXES = 0x6E,
    BMI270_REG_PWR_CONF = 0x7C,
    BMI270_REG_PWR_CTRL = 0x7D,
    BMI270_REG_CMD = 0x7E,
} bmi270Register_e;

// BMI270 register configuration values
typedef enum {
    BMI270_VAL_CMD_SOFTRESET = 0xB6,
    BMI270_VAL_CMD_FIFOFLUSH = 0xB0,
    BMI270_VAL_PWR_CTRL = 0x0E,              // enable gyro, acc and temp sensors
    BMI270_VAL_PWR_CONF = 0x02,              // disable advanced power save, enable FIFO self-wake
    BMI270_VAL_ACC_CONF_ODR800 = 0x0B,       // set acc sample rate to 800hz
    BMI270_VAL_ACC_CONF_ODR1600 = 0x0C,      // set acc sample rate to 1600hz
    BMI270_VAL_ACC_CONF_BWP = 0x02,          // set acc filter in normal mode
    BMI270_VAL_ACC_CONF_HP = 0x01,           // set acc in high performance mode
    BMI270_VAL_ACC_RANGE_8G = 0x02,          // set acc to 8G full scale
    BMI270_VAL_ACC_RANGE_16G = 0x03,         // set acc to 16G full scale
    BMI270_VAL_GYRO_CONF_ODR3200 = 0x0D,     // set gyro sample rate to 3200hz
    BMI270_VAL_GYRO_CONF_BWP = 0x02,         // set gyro filter in normal mode
    BMI270_VAL_GYRO_CONF_NOISE_PERF = 0x01,  // set gyro in high performance noise mode
    BMI270_VAL_GYRO_CONF_FILTER_PERF = 0x01, // set gyro in high performance filter mode

    BMI270_VAL_GYRO_RANGE_2000DPS = 0x08,    // set gyro to 2000dps full scale
                                             // for some reason you have to enable the ois_range bit (bit 3) for 2000dps as well
                                             // or else the gyro scale will be 250dps when in prefiltered FIFO mode (not documented in datasheet!)

    BMI270_VAL_INT_MAP_DATA_DRDY_INT1 = 0x04,// enable the data ready interrupt pin 1
    BMI270_VAL_INT_MAP_FIFO_WM_INT1 = 0x02,  // enable the FIFO watermark interrupt pin 1
    BMI270_VAL_INT1_IO_CTRL_PINMODE = 0x0A,  // active high, push-pull, output enabled, input disabled 
    BMI270_VAL_FIFO_CONFIG_0 = 0x00,         // don't stop when full, disable sensortime frame
    BMI270_VAL_FIFO_CONFIG_1 = 0x80,         // only gyro data in FIFO, use headerless mode
    BMI270_VAL_FIFO_DOWNS = 0x00,            // select unfiltered gyro data with no downsampling (6.4KHz samples)
    BMI270_VAL_FIFO_WTM_0 = 0x06,            // set the FIFO watermark level to 1 gyro sample (6 bytes)
    BMI270_VAL_FIFO_WTM_1 = 0x00,            // FIFO watermark MSB
} bmi270ConfigValues_e;

// BMI270 register reads are 16bits with the first byte a "dummy" value 0
// that must be ignored. The result is in the second byte.
static uint8_t bmi270RegisterRead(const extDevice_t *dev, bmi270Register_e registerId)
{
    uint8_t data[2] = { 0, 0 };

    if (spiReadRegMskBufRB(dev, registerId, data, 2)) {
        return data[1];
    } else {
        return 0;
    }
}

static void bmi270RegisterWrite(const extDevice_t *dev, bmi270Register_e registerId, uint8_t value, unsigned delayMs)
{
    spiWriteReg(dev, registerId, value);
    if (delayMs) {
        delay(delayMs);
    }
}

// Toggle the CS to switch the device into SPI mode.
// Device switches initializes as I2C and switches to SPI on a low to high CS transition
static void bmi270EnableSPI(busDevice_t *dev)
{
    IOLo(dev->busType_u.spi.csnPin);
    delay(1);
    IOHi(dev->busType_u.spi.csnPin);
    delay(10);
}


static bool bmi270DeviceDetect(busDevice_t * busDev)
{
    uint8_t id;

    busSetSpeed(busDev, BUS_SPEED_INITIALIZATION);
    bmi270EnableSPI(busDev);

    busRead(busDev, BMI270_REG_CHIP_ID, &id);
    if (id == BMI270_CHIP_ID) {
        return true;
    }

    return false;
}

static void bmi270UploadConfig(busDevice_t * busDev)
{
    busWrite(busDev, BMI270_REG_PWR_CONF, 0);
    delay(1);
    busWrite(busDev, BMI270_REG_INIT_CTRL, 0);
    delay(1);

    // Transfer the config file
    uint8_t reg = BMI270_REG_INIT_DATA;
    busTransferDescriptor_t tfDesc[] = {
       {.rxBuf = NULL, .txBuff=&reg, .length=1},
       {.rxBuf = NULL, .txBuff=(uint8_t *)bmi270_maximum_fifo_config_file, .length=sizeof(bmi270_maximum_fifo_config_file)}
    };

    spiBusTransferMultiple(busDev, &tfDesc, 2);

    delay(10);
    busWrite(busDev, BMI270_REG_INIT_CTRL, 1);
    delay(1);
}

static void bmi270Config(gyroDev_t *gyro)
{
    busDevice_t * busDev = &gyro->dev;

    // Perform a soft reset to set all configuration to default
    // Delay 100ms before continuing configuration
    busWrite(busDev, BMI270_REG_CMD, BMI270_VAL_CMD_SOFTRESET);
    delay(100);

    // Toggle the chip into SPI mode
    bmi270EnableSPI(busDev);

    bmi270UploadConfig(busDev);

    // Configure the accelerometer
    busWrite(busDev, BMI270_REG_ACC_CONF, (BMI270_VAL_ACC_CONF_HP << 7) | (BMI270_VAL_ACC_CONF_BWP << 4) | BMI270_VAL_ACC_CONF_ODR800);
    delay(1);

    // Configure the accelerometer full-scale range
    busWrite(busDev, BMI270_REG_ACC_RANGE, BMI270_VAL_ACC_RANGE_16G);
    delay(1);

    // Configure the gyro
    busWrite(busDev, BMI270_REG_GYRO_CONF, (BMI270_VAL_GYRO_CONF_FILTER_PERF << 7) | (BMI270_VAL_GYRO_CONF_NOISE_PERF << 6) | (BMI270_VAL_GYRO_CONF_BWP << 4) | BMI270_VAL_GYRO_CONF_ODR3200);
    delay(1);


    // Configure the gyro full-range scale
    busWrite(busDev, BMI270_REG_GYRO_RANGE, BMI270_VAL_GYRO_RANGE_2000DPS);
    delay(1);

    // Configure the gyro data ready interrupt
#ifdef USE_MPU_DATA_READY_SIGNAL
    busWrite(busDev, BMI270_REG_INT_MAP_DATA, BMI270_VAL_INT_MAP_DATA_DRDY_INT1);
    delay(1);
#endif

    // Configure the behavior of the INT1 pin
    busWrite(busDev, BMI270_REG_INT1_IO_CTRL, BMI270_VAL_INT1_IO_CTRL_PINMODE);
    delay(1);

    // Configure the device for  performance mode
    busWrite(busDev, BMI270_REG_PWR_CONF, BMI270_VAL_PWR_CONF);
    delay(1)

    // Enable the gyro, accelerometer and temperature sensor - disable aux interface
    busWrite(busDev, BMI270_REG_PWR_CTRL, BMI270_VAL_PWR_CTRL);
    delay(1)
}

static bool bmi270AccRead(accDev_t *acc)
{
    enum {
        IDX_SKIP,
        IDX_ACCEL_XOUT_L,
        IDX_ACCEL_XOUT_H,
        IDX_ACCEL_YOUT_L,
        IDX_ACCEL_YOUT_H,
        IDX_ACCEL_ZOUT_L,
        IDX_ACCEL_ZOUT_H,
        BUFFER_SIZE,
    };

    uint8_t bmi270_rx_buf[BUFFER_SIZE];

    if (busReadBuf(acc->busDev, BMI270_REG_ACC_DATA_X_LSB, bmi270_rx_buf, BUFFER_SIZE)) {
        acc->ADCRaw[X] = (int16_t)((bmi270_rx_buf[IDX_ACCEL_XOUT_H] << 8) | bmi270_rx_buf[IDX_ACCEL_XOUT_L]);
        acc->ADCRaw[Y] = (int16_t)((bmi270_rx_buf[IDX_ACCEL_YOUT_H] << 8) | bmi270_rx_buf[IDX_ACCEL_YOUT_L]);
        acc->ADCRaw[Z] = (int16_t)((bmi270_rx_buf[IDX_ACCEL_ZOUT_H] << 8) | bmi270_rx_buf[IDX_ACCEL_ZOUT_L]);
        return true;
    }

    return false;
}

static bool bmi270GyroRead(gyroDev_t *gyro)
{
    enum {
        IDX_SKIP,
        IDX_GYRO_XOUT_L,
        IDX_GYRO_XOUT_H,
        IDX_GYRO_YOUT_L,
        IDX_GYRO_YOUT_H,
        IDX_GYRO_ZOUT_L,
        IDX_GYRO_ZOUT_H,
        BUFFER_SIZE,
    };

    uint8_t bmi270_rx_buf[BUFFER_SIZE];
    if (busReadBuf(gyro->busDev, BMI270_REG_GYR_DATA_X_LSB, bmi270_rx_buf, BUFFER_SIZE)) {
        gyro->gyroADCRaw[X] = (int16_t)((bmi270_rx_buf[IDX_GYRO_XOUT_H] << 8) | bmi270_rx_buf[IDX_GYRO_XOUT_L]);
        gyro->gyroADCRaw[Y] = (int16_t)((bmi270_rx_buf[IDX_GYRO_YOUT_H] << 8) | bmi270_rx_buf[IDX_GYRO_YOUT_L]);
        gyro->gyroADCRaw[Z] = (int16_t)((bmi270_rx_buf[IDX_GYRO_ZOUT_H] << 8) | bmi270_rx_buf[IDX_GYRO_ZOUT_L]);
        return true;
    }

    return false;
}


static void bmi270SpiGyroInit(gyroDev_t *gyro)
{
    bmi270Config(gyro);
}

static void bmi270SpiAccInit(accDev_t *acc)
{
    // sensor is configured during gyro init
    acc->acc_1G = 512 * 4;   // 16G sensor scale
}

bool bmi270SpiAccDetect(accDev_t *acc)
{
    acc->busDev = busDeviceInit(BUSTYPE_SPI, DEVHW_BMI270, acc->imuSensorToUse, OWNER_MPU);
    if (acc->busDev == NULL) {
        return false;
    }

    if (!bmi270DeviceDetect(acc->busDev)) {
        busDeviceDeInit(acc->busDev);
        return false;
    }

    acc->initFn = bmi270SpiAccInit;
    acc->readFn = bmi270AccRead;

    return true;
}


bool bmi270GyroDetect(gyroDev_t *gyro)
{
    gyro->busDev = busDeviceInit(BUSTYPE_SPI, DEVHW_BMI270, gyro->imuSensorToUse, OWNER_MPU);
    if (gyro->busDev == NULL) {
        return false;
    }

    if (!bmi270DeviceDetect(gyro->busDev)) {
        busDeviceDeInit(gyro->busDev);
        return false;
    }

    gyro->initFn = bmi270SpiGyroInit;
    gyro->readFn = bmi270GyroRead;
    gyro->intStatusFn = bmi160CheckDataReady;
    gyro->scale = 1.0f / 16.4f;
    return true;
}
#endif // USE_IMU_BMI270
