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
#include "drivers/accgyro/accgyro_bmi270.h"

#define BMI270_CONFIG_SIZE 328

// Declaration for the device config (microcode) that must be uploaded to the sensor
extern const uint8_t bmi270_maximum_fifo_config_file[BMI270_CONFIG_SIZE];

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

#define BMI270_CHIP_ID 0x24

#define BMI270_CMD_SOFTRESET 0xB6

#define BMI270_PWR_CONF_HP 0x00
#define BMI270_PWR_CTRL_GYR_EN 0x02
#define BMI270_PWR_CTRL_ACC_EN 0x04
#define BMI270_PWR_CTRL_TEMP_EN 0x08

#define BMI270_ACC_CONF_HP 0x80
#define BMI270_ACC_RANGE_8G 0x02
#define BMI270_ACC_RANGE_16G 0x03

#define BMI270_GYRO_CONF_NOISE_PERF 0x40
#define BMI270_GYRO_CONF_FILTER_PERF 0x80
#define BMI270_GYRO_RANGE_2000DPS 0x08

#define BMI270_INT_MAP_DATA_DRDY_INT1 0x04
#define BMI270_INT1_IO_CTRL_ACTIVE_HIGH 0x02
#define BMI270_INT1_IO_CTRL_OUTPUT_EN 0x08

#define BMI270_ODR_400 0x0A
#define BMI270_ODR_800 0x0B
#define BMI270_ODR_1600 0x0C
#define BMI270_ODR_3200 0x0D

#define BMI270_BWP_OSR4 0x00
#define BMI270_BWP_OSR2 0x10
#define BMI270_BWP_NORM 0x20

typedef struct __attribute__ ((__packed__)) bmi270ContextData_s {
    uint16_t    chipMagicNumber;
    uint8_t     lastReadStatus;
    uint8_t     __padding_dummy;
    uint8_t     accRaw[6];
    uint8_t     gyroRaw[6];
} bmi270ContextData_t;

STATIC_ASSERT(sizeof(bmi270ContextData_t) < BUS_SCRATCHPAD_MEMORY_SIZE, busDevice_scratchpad_memory_too_small);

static const gyroFilterAndRateConfig_t gyroConfigs[] = {
    { GYRO_LPF_256HZ,   3200,   { BMI270_BWP_NORM | BMI270_ODR_3200} },
    { GYRO_LPF_256HZ,   1600,   { BMI270_BWP_NORM | BMI270_ODR_1600} },
    { GYRO_LPF_256HZ,    800,   { BMI270_BWP_NORM | BMI270_ODR_800 } },

    { GYRO_LPF_188HZ,    800,   { BMI270_BWP_OSR2 | BMI270_ODR_800 } },
    { GYRO_LPF_188HZ,    400,   { BMI270_BWP_NORM | BMI270_ODR_400 } },

    { GYRO_LPF_98HZ,     800,   { BMI270_BWP_OSR4   | BMI270_ODR_800 } },
    { GYRO_LPF_98HZ,     400,   { BMI270_BWP_OSR2   | BMI270_ODR_400 } },

    { GYRO_LPF_42HZ,     800,   { BMI270_BWP_OSR4   | BMI270_ODR_800 } },
    { GYRO_LPF_42HZ,     400,   { BMI270_BWP_OSR4   | BMI270_ODR_400 } },
};

// Toggle the CS to switch the device into SPI mode.
// Device switches initializes as I2C and switches to SPI on a low to high CS transition
static void bmi270EnableSPI(busDevice_t *dev)
{
    IOLo(dev->busdev.spi.csnPin);
    delay(1);
    IOHi(dev->busdev.spi.csnPin);
    delay(10);
}


static bool bmi270DeviceDetect(busDevice_t * busDev)
{
    uint8_t id[2];

    busSetSpeed(busDev, BUS_SPEED_INITIALIZATION);
    bmi270EnableSPI(busDev);

    busReadBuf(busDev, BMI270_REG_CHIP_ID, &id[0], 2);
    if (id[1] == BMI270_CHIP_ID) {
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
       {.rxBuf = NULL, .txBuf=&reg, .length=1},
       {.rxBuf = NULL, .txBuf=(uint8_t *)bmi270_maximum_fifo_config_file, .length=sizeof(bmi270_maximum_fifo_config_file)}
    };

    spiBusTransferMultiple(busDev, tfDesc, 2);

    delay(10);
    busWrite(busDev, BMI270_REG_INIT_CTRL, 1);
    delay(1);
}

static void bmi270AccAndGyroInit(gyroDev_t *gyro)
{
    busDevice_t * busDev = gyro->busDev;

    gyroIntExtiInit(gyro);

    // Perform a soft reset to set all configuration to default
    // Delay 100ms before continuing configuration
    busWrite(busDev, BMI270_REG_CMD, BMI270_CMD_SOFTRESET);
    delay(100);

    // Use standard bus speed
    busSetSpeed(busDev, BUS_SPEED_STANDARD);

    // Toggle the chip into SPI mode
    bmi270EnableSPI(busDev);

    bmi270UploadConfig(busDev);

    // Configure the accelerometer
    busWrite(busDev, BMI270_REG_ACC_CONF, BMI270_ODR_1600 | BMI270_BWP_OSR4 | BMI270_ACC_CONF_HP);
    delay(1);

    // Configure the accelerometer full-scale range
    busWrite(busDev, BMI270_REG_ACC_RANGE, BMI270_ACC_RANGE_8G);
    delay(1);

    // Configure the gyro
    // Figure out suitable filter configuration
    const gyroFilterAndRateConfig_t * config = chooseGyroConfig(gyro->lpf, 1000000 / gyro->requestedSampleIntervalUs, &gyroConfigs[0], ARRAYLEN(gyroConfigs));

    gyro->sampleRateIntervalUs = 1000000 / config->gyroRateHz;

    busWrite(busDev, BMI270_REG_GYRO_CONF, config->gyroConfigValues[0] | BMI270_GYRO_CONF_NOISE_PERF | BMI270_GYRO_CONF_FILTER_PERF);
    delay(1);

    // Configure the gyro full-range scale
    busWrite(busDev, BMI270_REG_GYRO_RANGE, BMI270_GYRO_RANGE_2000DPS);
    delay(1);

    // Configure the gyro data ready interrupt
#ifdef USE_MPU_DATA_READY_SIGNAL
    busWrite(busDev, BMI270_REG_INT_MAP_DATA, BMI270_INT_MAP_DATA_DRDY_INT1);
    delay(1);
#endif

    // Configure the behavior of the INT1 pin
    busWrite(busDev, BMI270_REG_INT1_IO_CTRL, BMI270_INT1_IO_CTRL_ACTIVE_HIGH | BMI270_INT1_IO_CTRL_OUTPUT_EN);
    delay(1);

    // Configure the device for  performance mode
    busWrite(busDev, BMI270_REG_PWR_CONF, BMI270_PWR_CONF_HP);
    delay(1);

    // Enable the gyro and accelerometer
    busWrite(busDev, BMI270_REG_PWR_CTRL, BMI270_PWR_CTRL_GYR_EN | BMI270_PWR_CTRL_ACC_EN);
    delay(1);
}


static bool bmi270yroReadScratchpad(gyroDev_t *gyro)
{
    bmi270ContextData_t * ctx = busDeviceGetScratchpadMemory(gyro->busDev);
    ctx->lastReadStatus = busReadBuf(gyro->busDev, BMI270_REG_ACC_DATA_X_LSB, &ctx->__padding_dummy, 6 + 6 + 1);

    if (ctx->lastReadStatus) {
        gyro->gyroADCRaw[X] = (int16_t)((ctx->gyroRaw[1] << 8) | ctx->gyroRaw[0]);
        gyro->gyroADCRaw[Y] = (int16_t)((ctx->gyroRaw[3] << 8) | ctx->gyroRaw[2]);
        gyro->gyroADCRaw[Z] = (int16_t)((ctx->gyroRaw[5] << 8) | ctx->gyroRaw[4]);

        return true;
    }

    return false;
}

static bool bmi270AccReadScratchpad(accDev_t *acc)
{
    bmi270ContextData_t * ctx = busDeviceGetScratchpadMemory(acc->busDev);

    if (ctx->lastReadStatus) {
        acc->ADCRaw[X] = (int16_t)((ctx->accRaw[1] << 8) | ctx->accRaw[0]);
        acc->ADCRaw[Y] = (int16_t)((ctx->accRaw[3] << 8) | ctx->accRaw[2]);
        acc->ADCRaw[Z] = (int16_t)((ctx->accRaw[5] << 8) | ctx->accRaw[4]);
        return true;
    }

    return false;
}

static bool bmi270TemperatureRead(gyroDev_t *gyro, int16_t * data)
{
    uint8_t buffer[3];

    bool readStatus = busReadBuf(gyro->busDev, BMI270_REG_TEMPERATURE_LSB, &buffer[0], sizeof(buffer));

    if (readStatus) {
        // Convert to degC*10: degC = raw / 512 + 23
        *data = 230 + ((10 * (int32_t)((int16_t)((buffer[2] << 8) | buffer[1]))) >> 9);
        return true;
    }

    return false;
}

static void bmi270GyroInit(gyroDev_t *gyro)
{
    bmi270AccAndGyroInit(gyro);
}

static void bmi270AccInit(accDev_t *acc)
{
    // sensor is configured during gyro init
    acc->acc_1G = 4096;   // 8G sensor scale
}

bool bmi270AccDetect(accDev_t *acc)
{
    acc->busDev = busDeviceOpen(BUSTYPE_SPI, DEVHW_BMI270, acc->imuSensorToUse);
    if (acc->busDev == NULL) {
        return false;
    }

    bmi270ContextData_t * ctx = busDeviceGetScratchpadMemory(acc->busDev);
    if (ctx->chipMagicNumber != 0xB270) {
        return false;
    }

    acc->initFn = bmi270AccInit;
    acc->readFn = bmi270AccReadScratchpad;
    acc->accAlign = acc->busDev->param;

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

    // Magic number for ACC detection to indicate that we have detected BMI270 gyro
    bmi270ContextData_t * ctx = busDeviceGetScratchpadMemory(gyro->busDev);
    ctx->chipMagicNumber = 0xB270;

    gyro->initFn = bmi270GyroInit;
    gyro->readFn = bmi270yroReadScratchpad;
    gyro->temperatureFn = bmi270TemperatureRead;
    gyro->intStatusFn = gyroCheckDataReady;
    gyro->scale = 1.0f / 16.4f; // 2000 dps
    gyro->gyroAlign = gyro->busDev->param;
    return true;
}
#endif // USE_IMU_BMI270
