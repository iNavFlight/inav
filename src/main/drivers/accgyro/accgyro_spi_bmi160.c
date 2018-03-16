/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_BMI160 BMI160 Functions
 * @brief Hardware functions to deal with the 6DOF gyro / accel sensor
 * @{
 *
 * @file       pios_bmi160.c
 * @author     dRonin, http://dRonin.org/, Copyright (C) 2016
 * @brief      BMI160 Gyro / Accel Sensor Routines
 * @see        The GNU Public License (GPL) Version 3
 ******************************************************************************/

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Additional note on redistribution: The copyright and license notices above
 * must be maintained in each individual source file that is a derivative work
 * of this source file; otherwise redistribution is prohibited.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"

#include "build/atomic.h"
#include "build/build_config.h"
#include "build/debug.h"

#include "common/maths.h"
#include "common/utils.h"

#include "drivers/bus_spi.h"
#include "drivers/exti.h"
#include "drivers/io.h"
#include "drivers/exti.h"
#include "drivers/nvic.h"
#include "drivers/sensor.h"
#include "drivers/system.h"
#include "drivers/time.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_spi_bmi160.h"

#ifdef USE_ACCGYRO_BMI160

/* BMI160 Registers */
#define BMI160_REG_CHIPID 0x00
#define BMI160_REG_PMU_STAT 0x03
#define BMI160_REG_GYR_DATA_X_LSB 0x0C
#define BMI160_REG_ACC_DATA_X_LSB 0x12
#define BMI160_REG_STATUS 0x1B
#define BMI160_REG_TEMPERATURE_0 0x20
#define BMI160_REG_ACC_CONF 0x40
#define BMI160_REG_ACC_RANGE 0x41
#define BMI160_REG_GYR_CONF 0x42
#define BMI160_REG_GYR_RANGE 0x43
#define BMI160_REG_INT_EN1 0x51
#define BMI160_REG_INT_OUT_CTRL 0x53
#define BMI160_REG_INT_MAP1 0x56
#define BMI160_REG_FOC_CONF 0x69
#define BMI160_REG_CONF 0x6A
#define BMI160_REG_OFFSET_0 0x77
#define BMI160_REG_CMD 0x7E

/* Register values */
#define BMI160_PMU_CMD_PMU_ACC_NORMAL 0x11
#define BMI160_PMU_CMD_PMU_GYR_NORMAL 0x15
#define BMI160_INT_EN1_DRDY 0x10
#define BMI160_INT_OUT_CTRL_INT1_CONFIG 0x0A
#define BMI160_REG_INT_MAP1_INT1_DRDY 0x80
#define BMI160_CMD_START_FOC 0x03
#define BMI160_CMD_PROG_NVM 0xA0
#define BMI160_REG_STATUS_NVM_RDY 0x10
#define BMI160_REG_STATUS_FOC_RDY 0x08
#define BMI160_REG_CONF_NVM_PROG_EN 0x02

///* Global Variables */
static volatile bool BMI160InitDone = false;
static volatile bool BMI160Detected = false;

//! Private functions
static int32_t BMI160_Config(const busDevice_t *bus);

bool bmi160Detect(busDevice_t * busDev)
{
    uint8_t in;
    if (BMI160Detected) {
        return true;
    }

#if !defined(BUS_SPEED_BMI160)
#define BUS_SPEED_BMI160     BUS_SPEED_STANDARD
#endif

    busSetSpeed(busDev, BUS_SPEED_BMI160);

    /* Read this address to activate SPI (see p. 84) */
    busRead(busDev, 0x7F, &in);

    // Give SPI some time to start up
    delay(100);

    /* Check the chip ID */
    busRead(busDev, BMI160_REG_CHIPID, &in);

    if (in != 0xd1) {
        return false;
    }

    BMI160Detected = true;
    return true;
}


/**
 * @brief Initialize the BMI160 6-axis sensor.
 * @return 0 for success, -1 for failure to allocate, -10 for failure to get irq
 */
static void BMI160_Init(const busDevice_t *busDev)
{
    if (BMI160InitDone || !BMI160Detected) {
        return;
    }

    /* Configure the BMI160 Sensor */
    if (BMI160_Config(busDev) != 0) {
        return;
    }

    BMI160InitDone = true;
}


/**
 * @brief Configure the sensor
 */
static int32_t BMI160_Config(const busDevice_t *busDev)
{
    uint8_t in;
    // Set normal power mode for gyro and accelerometer
    busWrite(busDev, BMI160_REG_CMD, BMI160_PMU_CMD_PMU_GYR_NORMAL);
    delay(100); // can take up to 80ms

    busWrite(busDev, BMI160_REG_CMD, BMI160_PMU_CMD_PMU_ACC_NORMAL);
    delay(5); // can take up to 3.8ms

    // Verify that normal power mode was entered
    busRead(busDev, BMI160_REG_PMU_STAT, &in);
    if ((in & 0x3C) != 0x14){
        return -3;
    }

    // Set odr and ranges
    // Set acc_us = 0 acc_bwp = 0b010 so only the first filter stage is used
    busWrite(busDev, BMI160_REG_ACC_CONF, 0x20 | BMI160_ODR_800_Hz);
    delay(1);

    // Set gyr_bwp = 0b010 so only the first filter stage is used
    busWrite(busDev, BMI160_REG_GYR_CONF, 0x20 | BMI160_ODR_3200_Hz);
    delay(1);

    busWrite(busDev, BMI160_REG_ACC_RANGE, BMI160_RANGE_8G);
    delay(1);

    busWrite(busDev, BMI160_REG_GYR_RANGE, BMI160_RANGE_2000DPS);
    delay(1);

    // Enable offset compensation
    busRead(busDev, BMI160_REG_OFFSET_0, &in);
    busWrite(busDev, BMI160_REG_OFFSET_0, in | 0xC0);

    // Enable data ready interrupt
    busWrite(busDev, BMI160_REG_INT_EN1, BMI160_INT_EN1_DRDY);
    delay(1);

    // Enable INT1 pin
    busWrite(busDev, BMI160_REG_INT_OUT_CTRL, BMI160_INT_OUT_CTRL_INT1_CONFIG);
    delay(1);

    // Map data ready interrupt to INT1 pin
    busWrite(busDev, BMI160_REG_INT_MAP1, BMI160_REG_INT_MAP1_INT1_DRDY);
    delay(1);

    return 0;
}

#if defined(USE_EXTI) && defined(BMI160_INT_EXTI)

void bmi160ExtiHandler(extiCallbackRec_t *cb)
{
    gyroDev_t *gyro = container_of(cb, gyroDev_t, exti);
    gyro->dataReady = true;
}

static void bmi160IntExtiInit(gyroDev_t *gyro)
{
    static bool bmi160ExtiInitDone = false;

    if (bmi160ExtiInitDone) {
        return;
    }

    IO_t mpuIntIO = IOGetByTag(IO_TAG(BMI160_INT_EXTI));

    IOInit(mpuIntIO, OWNER_MPU, RESOURCE_EXTI, 0);
    IOConfigGPIO(mpuIntIO, IOCFG_IN_FLOATING);

    EXTIHandlerInit(&gyro->exti, bmi160ExtiHandler);
    EXTIConfig(mpuIntIO, &gyro->exti, NVIC_PRIO_MPU_INT_EXTI, EXTI_Trigger_Rising);
    EXTIEnable(mpuIntIO, true);

    bmi160ExtiInitDone = true;
}
#endif /* defined(USE_EXTI) && defined(BMI160_INT_EXTI) */


bool bmi160CheckDataReady(gyroDev_t* gyro)
{
    bool ret;
    if (gyro->dataReady) {
        ret = true;
        gyro->dataReady= false;
    } else {
        ret = false;
    }
    return ret;
}

bool bmi160AccRead(accDev_t *acc)
{
    enum {
        IDX_ACCEL_XOUT_L,
        IDX_ACCEL_XOUT_H,
        IDX_ACCEL_YOUT_L,
        IDX_ACCEL_YOUT_H,
        IDX_ACCEL_ZOUT_L,
        IDX_ACCEL_ZOUT_H,
        BUFFER_SIZE,
    };

    uint8_t bmi160_rx_buf[BUFFER_SIZE];

    const bool ack = busReadBuf(acc->busDev, BMI160_REG_ACC_DATA_X_LSB | 0x80, bmi160_rx_buf, BUFFER_SIZE);
    if (!ack) {
        return false;
    }

    acc->ADCRaw[X] = (int16_t)((bmi160_rx_buf[IDX_ACCEL_XOUT_H] << 8) | bmi160_rx_buf[IDX_ACCEL_XOUT_L]);
    acc->ADCRaw[Y] = (int16_t)((bmi160_rx_buf[IDX_ACCEL_YOUT_H] << 8) | bmi160_rx_buf[IDX_ACCEL_YOUT_L]);
    acc->ADCRaw[Z] = (int16_t)((bmi160_rx_buf[IDX_ACCEL_ZOUT_H] << 8) | bmi160_rx_buf[IDX_ACCEL_ZOUT_L]);

    return true;
}


bool bmi160GyroRead(gyroDev_t *gyro)
{
    enum {
        IDX_GYRO_XOUT_L,
        IDX_GYRO_XOUT_H,
        IDX_GYRO_YOUT_L,
        IDX_GYRO_YOUT_H,
        IDX_GYRO_ZOUT_L,
        IDX_GYRO_ZOUT_H,
        BUFFER_SIZE,
    };

    uint8_t bmi160_rx_buf[BUFFER_SIZE];

    const bool ack = busReadBuf(gyro->busDev, BMI160_REG_GYR_DATA_X_LSB | 0x80, bmi160_rx_buf, BUFFER_SIZE);
    if (!ack) {
        return false;
    }

    gyro->gyroADCRaw[X] = (int16_t)((bmi160_rx_buf[IDX_GYRO_XOUT_H] << 8) | bmi160_rx_buf[IDX_GYRO_XOUT_L]);
    gyro->gyroADCRaw[Y] = (int16_t)((bmi160_rx_buf[IDX_GYRO_YOUT_H] << 8) | bmi160_rx_buf[IDX_GYRO_YOUT_L]);
    gyro->gyroADCRaw[Z] = (int16_t)((bmi160_rx_buf[IDX_GYRO_ZOUT_H] << 8) | bmi160_rx_buf[IDX_GYRO_ZOUT_L]);

    return true;
}


void bmi160SpiGyroInit(gyroDev_t *gyro)
{
    BMI160_Init(gyro->busDev);

#if defined(USE_EXTI) && defined(BMI160_INT_EXTI)
    bmi160IntExtiInit(gyro);
#endif /* defined(USE_EXTI) && defined(BMI160_INT_EXTI) */
}

void bmi160SpiAccInit(accDev_t *acc)
{
    BMI160_Init(acc->busDev);

    acc->acc_1G = 512 * 8;
}


bool bmi160SpiAccDetect(accDev_t *acc)
{
    acc->busDev = busDeviceOpen(BUSTYPE_ANY, DEVHW_BMI160, acc->imuSensorToUse);
    if (acc->busDev == NULL) {
        return false;
    }

    if (!bmi160Detect(acc->busDev)) {
        return false;
    }

    acc->initFn = bmi160SpiAccInit;
    acc->readFn = bmi160AccRead;

    return true;
}


bool bmi160SpiGyroDetect(gyroDev_t *gyro)
{
    gyro->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_BMI160, gyro->imuSensorToUse, OWNER_MPU);
    if (gyro->busDev == NULL) {
        return false;
    }

    if (!bmi160Detect(gyro->busDev)) {
        return false;
    }

    gyro->initFn = bmi160SpiGyroInit;
    gyro->readFn = bmi160GyroRead;
    gyro->intStatusFn = bmi160CheckDataReady;
    gyro->scale = 1.0f / 16.4f;

    return true;
}
#endif // USE_ACCGYRO_BMI160
