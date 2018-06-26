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
#include <stdlib.h>
#include <string.h>

#include "platform.h"

#include "build/atomic.h"
#include "build/build_config.h"
#include "build/debug.h"

#include "common/maths.h"
#include "common/utils.h"

#include "drivers/bus.h"
#include "drivers/exti.h"
#include "drivers/io.h"
#include "drivers/exti.h"
#include "drivers/nvic.h"
#include "drivers/sensor.h"
#include "drivers/system.h"
#include "drivers/time.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_mpu.h"

// Check busDevice scratchpad memory size
STATIC_ASSERT(sizeof(mpuContextData_t) < BUS_SCRATCHPAD_MEMORY_SIZE, busDevice_scratchpad_memory_too_small);

#ifdef USE_DMA_SPI_DEVICE
#include "drivers/dma_spi.h"
#include "sensors/gyro.h"
#include "sensors/acceleration.h"
#endif //USE_DMA_SPI_DEVICE
#ifdef USE_GYRO_IMUF9001
#include "drivers/accgyro/accgyro_imuf9001.h"
#include "flight/pid.h"
#endif //USE_GYRO_IMUF9001

#ifdef USE_DMA_SPI_DEVICE
static volatile int dmaSpiGyroDataReady = 1;
static volatile uint32_t imufCrcErrorCount = 0;
#endif //USE_DMA_SPI_DEVICE

#ifdef USE_GYRO_IMUF9001
#ifdef USE_DMA_SPI_DEVICE
imufCommand_t *dmaTxBufferImufCmdPtr = (imufCommand_t *)dmaTxBuffer;
imufCommand_t *dmaRxBufferImufCmdPtr = (imufCommand_t *)dmaRxBuffer;
imufData_t imufData;
#endif
#endif
/*
 * Gyro interrupt service routine
 */

static const gyroFilterAndRateConfig_t mpuGyroConfigs[] = {
    { GYRO_LPF_256HZ,   8000,   { MPU_DLPF_256HZ,   0  } },
    { GYRO_LPF_256HZ,   4000,   { MPU_DLPF_256HZ,   1  } },
    { GYRO_LPF_256HZ,   2000,   { MPU_DLPF_256HZ,   3  } },
    { GYRO_LPF_256HZ,   1000,   { MPU_DLPF_256HZ,   7  } },
    { GYRO_LPF_256HZ,    666,   { MPU_DLPF_256HZ,   11 } },
    { GYRO_LPF_256HZ,    500,   { MPU_DLPF_256HZ,   15 } },

    { GYRO_LPF_188HZ,   1000,   { MPU_DLPF_188HZ,   0  } },
    { GYRO_LPF_188HZ,    500,   { MPU_DLPF_188HZ,   1  } },

    { GYRO_LPF_98HZ,    1000,   { MPU_DLPF_98HZ,    0  } },
    { GYRO_LPF_98HZ,     500,   { MPU_DLPF_98HZ,    1  } },

    { GYRO_LPF_42HZ,    1000,   { MPU_DLPF_42HZ,    0  } },
    { GYRO_LPF_42HZ,     500,   { MPU_DLPF_42HZ,    1  } },

    { GYRO_LPF_20HZ,    1000,   { MPU_DLPF_20HZ,    0  } },
    { GYRO_LPF_20HZ,     500,   { MPU_DLPF_20HZ,    1  } },

    { GYRO_LPF_10HZ,    1000,   { MPU_DLPF_10HZ,    0  } },
    { GYRO_LPF_10HZ,     500,   { MPU_DLPF_10HZ,    1  } }
};

const gyroFilterAndRateConfig_t * mpuChooseGyroConfig(uint8_t desiredLpf, uint16_t desiredRateHz)
{
    return chooseGyroConfig(desiredLpf, desiredRateHz, &mpuGyroConfigs[0], ARRAYLEN(mpuGyroConfigs));
}

bool mpuGyroRead(gyroDev_t *gyro)
{
    uint8_t data[6];

    const bool ack = busReadBuf(gyro->busDev, MPU_RA_GYRO_XOUT_H, data, 6);
    if (!ack) {
        return false;
    }

    gyro->gyroADCRaw[X] = (int16_t)((data[0] << 8) | data[1]);
    gyro->gyroADCRaw[Y] = (int16_t)((data[2] << 8) | data[3]);
    gyro->gyroADCRaw[Z] = (int16_t)((data[4] << 8) | data[5]);

    return true;
}

static bool mpuUpdateSensorContext(busDevice_t * busDev, mpuContextData_t * ctx)
{
    ctx->lastReadStatus = busReadBuf(busDev, MPU_RA_ACCEL_XOUT_H, ctx->accRaw, 6 + 2 + 6);
    return ctx->lastReadStatus;
}

bool mpuGyroReadScratchpad(gyroDev_t *gyro)
{
    busDevice_t * busDev = gyro->busDev;
    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(busDev);

    if (mpuUpdateSensorContext(busDev, ctx)) {
        gyro->gyroADCRaw[X] = (int16_t)((ctx->gyroRaw[0] << 8) | ctx->gyroRaw[1]);
        gyro->gyroADCRaw[Y] = (int16_t)((ctx->gyroRaw[2] << 8) | ctx->gyroRaw[3]);
        gyro->gyroADCRaw[Z] = (int16_t)((ctx->gyroRaw[4] << 8) | ctx->gyroRaw[5]);
        return true;
    }

    return false;
}

bool mpuAccReadScratchpad(accDev_t *acc)
{
    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(acc->busDev);

    if (ctx->lastReadStatus) {
        acc->ADCRaw[X] = (int16_t)((ctx->accRaw[0] << 8) | ctx->accRaw[1]);
        acc->ADCRaw[Y] = (int16_t)((ctx->accRaw[2] << 8) | ctx->accRaw[3]);
        acc->ADCRaw[Z] = (int16_t)((ctx->accRaw[4] << 8) | ctx->accRaw[5]);
        return true;
    }

    return false;
}

bool mpuTemperatureReadScratchpad(gyroDev_t *gyro, int16_t * data)
{
    mpuContextData_t * ctx = busDeviceGetScratchpadMemory(gyro->busDev);

    if (ctx->lastReadStatus) {
        // Convert to degC*10: degC = raw / 340 + 36.53
        *data = (int16_t)((ctx->tempRaw[0] << 8) | ctx->tempRaw[1]) / 34 + 365;
        return true;
    }

    return false;
}

#ifdef USE_DMA_SPI_DEVICE
bool mpuGyroDmaSpiReadStart(void)
{
    //(void)(gyro); ///not used at this time
    //no reason not to get acc and gyro data at the same time
    lastImufExtiTime = micros();
    #ifdef USE_GYRO_IMUF9001
    if (isImufCalibrating == IMUF_IS_CALIBRATING) //calibrating
    {
        //two steps
        //step 1 is isImufCalibrating=1, this starts the calibration command and sends it to the IMU-f
        //step 2 is isImufCalibrating=2, this sets the tx buffer back to 0 so we don't keep sending the calibration command over and over
        memset(dmaTxBuffer, 0, sizeof(imufCommand_t)); //clear buffer
        //set calibration command with CRC, typecast the dmaTxBuffer as imufCommand_t
        dmaTxBufferImufCmdPtr->command = IMUF_COMMAND_CALIBRATE;
        dmaTxBufferImufCmdPtr->crc     = getCrcImuf9001((uint32_t *)dmaTxBuffer, 11); //typecast the dmaTxBuffer as a uint32_t array which is what the crc command needs
        //set isImufCalibrating to step 2, which is just used so the memset to 0 runs after the calibration commmand is sent
        isImufCalibrating = IMUF_DONE_CALIBRATING; //go to step two
    }
    else if (isImufCalibrating == IMUF_DONE_CALIBRATING)
    {
        // step 2, memset of the tx buffer has run, set isImufCalibrating to 0.
        dmaTxBufferImufCmdPtr->command = 0;
        dmaTxBufferImufCmdPtr->crc     = 0; //typecast the dmaTxBuffer as a uint32_t array which is what the crc command needs
        imufEndCalibration();
    }
    else
    {
        //send setpoint and arm status
        dmaTxBufferImufCmdPtr->command = IMUF_COMMAND_SETPOINT;
        dmaTxBufferImufCmdPtr->param1  = getSetpointRateInt(0);
        dmaTxBufferImufCmdPtr->param2  = getSetpointRateInt(1);
        dmaTxBufferImufCmdPtr->param3  = getSetpointRateInt(2);
        dmaTxBufferImufCmdPtr->crc     = getCrcImuf9001((uint32_t *)dmaTxBuffer, 11); //typecast the dmaTxBuffer as a uint32_t array which is what the crc command needs
    }
    memset(dmaRxBuffer, 0, gyroConfig()->imuf_mode); //clear buffer
    //send and receive data using SPI and DMA
    dmaSpiTransmitReceive((uint8_t*)dmaTxBuffer, (uint8_t*)dmaRxBuffer, gyroConfig()->imuf_mode, 0);
    #else
    dmaTxBuffer[0] = MPU_RA_ACCEL_XOUT_H | 0x80;
    dmaSpiTransmitReceive((uint8_t*)dmaTxBuffer, (uint8_t*)dmaRxBuffer, 15, 0);
    #endif
    return true;
}

void mpuGyroDmaSpiReadFinish(gyroDev_t * gyro)
{
    //spi rx dma callback
    #ifdef USE_GYRO_IMUF9001
    (void)(gyro); // this is not useful to us since the IMU-f is not a gyro.
    volatile uint32_t crc1 = ( (*(uint32_t *)(dmaRxBuffer+gyroConfig()->imuf_mode-4)) & 0xFF );
    volatile uint32_t crc2 = ( getCrcImuf9001((uint32_t *)(dmaRxBuffer), (gyroConfig()->imuf_mode >> 2)-1) & 0xFF );
    if(crc1 == crc2)
    {
        memcpy(&imufData, dmaRxBuffer, sizeof(imufData_t));
        //acc.accADCf[X]    = imufData.accX * 2.0f;
        //acc.accADCf[Y]    = imufData.accY * 2.0f;
        //acc.accADCf[Z]    = imufData.accZ * 2.0f;
        acc.dev.ADCRaw[X] = (imufData.accX * acc.dev.acc_1G); //plug in raw ACC
        acc.dev.ADCRaw[Y] = (imufData.accY * acc.dev.acc_1G); //plug in raw ACC
        acc.dev.ADCRaw[Z] = (imufData.accZ * acc.dev.acc_1G); //plug in raw ACC
        // gyroDev_t * gyro collides with the gyro struct, so we'll pass the data to a function in gyro.c
        setGyroData(imufData.gyroX, imufData.gyroY, imufData.gyroZ);
        if (gyroConfig()->imuf_mode == GTBCM_GYRO_ACC_QUAT_FILTER_F) {
            imufQuat.w       = imufData.quaternionW;
            imufQuat.x       = imufData.quaternionX;
            imufQuat.y       = imufData.quaternionY;
            imufQuat.z       = imufData.quaternionZ;
        }
    }
    else
    {
        //error handler
        imufCrcErrorCount++; //check every so often and cause a failsafe is this number is above a certain ammount
    }
    #else
    acc.dev.ADCRaw[X]   = (int16_t)((dmaRxBuffer[1] << 8)  | dmaRxBuffer[2]);
    acc.dev.ADCRaw[Y]   = (int16_t)((dmaRxBuffer[3] << 8)  | dmaRxBuffer[4]);
    acc.dev.ADCRaw[Z]   = (int16_t)((dmaRxBuffer[5] << 8)  | dmaRxBuffer[6]);
    gyro.gyroADCf[X]    = (int16_t)((dmaRxBuffer[9] << 8)  | dmaRxBuffer[10]);
    gyro.gyroADCf[Y]    = (int16_t)((dmaRxBuffer[11] << 8) | dmaRxBuffer[12]);
    gyro.gyroADCf[Z]    = (int16_t)((dmaRxBuffer[13] << 8) | dmaRxBuffer[14]);
    #endif
    dmaSpiGyroDataReady = 1; //set flag to tell scheduler data is ready
}
#endif