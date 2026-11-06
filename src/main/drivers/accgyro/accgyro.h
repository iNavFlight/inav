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

#pragma once

#include "platform.h"
#include "common/axis.h"
#include "drivers/sensor.h"

#define GYRO_LPF_256HZ      0
#define GYRO_LPF_188HZ      1
#define GYRO_LPF_98HZ       2
#define GYRO_LPF_42HZ       3
#define GYRO_LPF_20HZ       4
#define GYRO_LPF_10HZ       5
#define GYRO_LPF_5HZ        6
#define GYRO_LPF_NONE       7

typedef struct {
    uint8_t gyroLpf;
    uint16_t gyroRateHz;
    uint8_t gyroConfigValues[2];
} gyroFilterAndRateConfig_t;

// 2 ms at 4 kHz: the interrupt brings samples at the sensor's rate, the task follows on average
#define GYRO_SAMPLE_QUEUE_LENGTH 8

// Whether a driver that can reads its gyro on the data-ready interrupt (gyro_data_ready)
typedef enum {
    GYRO_DEV_DATA_READY_OFF = 0,
    GYRO_DEV_DATA_READY_WHERE_TESTED,   // only where that driver has been tested
    GYRO_DEV_DATA_READY_ON,
} gyroDevDataReady_e;

struct gyroDataReadyDriver_s;
struct gyroDataReady_s;

typedef struct gyroDev_s {
    busDevice_t * busDev;
    sensorGyroInitFuncPtr initFn;                       // initialize function
    sensorGyroReadFuncPtr readFn;                       // read 3 axis data function
    sensorGyroReadDataFuncPtr temperatureFn;            // read temperature if available
    sensorGyroInterruptStatusFuncPtr intStatusFn;
    sensorGyroUpdateFuncPtr updateFn;
    float scale;                                        // scalefactor
    float gyroADCRaw[XYZ_AXIS_COUNT];
    float gyroZero[XYZ_AXIS_COUNT];
    uint8_t imuSensorToUse;
    uint8_t lpf;                                        // Configuration value: Hardware LPF setting
    uint32_t requestedSampleIntervalUs;                 // Requested sample interval
    volatile bool dataReady;
    uint32_t sampleRateIntervalUs;                      // Gyro driver should set this to actual sampling rate as signaled by IRQ
    sensor_align_e gyroAlign;
#if defined(USE_SPI_DATA_READY)
    // Set by gyro.c before initFn
    gyroDevDataReady_e readOnDataReady;
    bool readOnDataReadyWithAcc;
    // Set by a driver that can (accgyro_data_ready.h); started at the end of init
    const struct gyroDataReadyDriver_s *dataReadyDriver;
    // Set once reading on data-ready: readFn then returns false when no sample came, not a failure
    struct gyroDataReady_s *dataReadyState;
    // Filled by the interrupt (head), emptied one per readFn call by the gyro task (tail)
    int16_t sampleQueue[GYRO_SAMPLE_QUEUE_LENGTH][XYZ_AXIS_COUNT];
    volatile uint8_t sampleQueueHead;
    uint8_t sampleQueueTail;
#endif
} gyroDev_t;

typedef struct accDev_s {
    busDevice_t * busDev;
    sensorAccInitFuncPtr initFn;                        // initialize function
    sensorAccReadFuncPtr readFn;                        // read 3 axis data function
    uint16_t acc_1G;
    float ADCRaw[XYZ_AXIS_COUNT];
    uint8_t imuSensorToUse;
    sensor_align_e accAlign;
} accDev_t;

const gyroFilterAndRateConfig_t * chooseGyroConfig(uint8_t desiredLpf, uint16_t desiredRateHz, const gyroFilterAndRateConfig_t * configs, int count);
bool gyroCheckDataReady(struct gyroDev_s *gyro);

#if defined(USE_SPI_DATA_READY)
bool gyroSampleQueuePush(gyroDev_t *gyro, int16_t x, int16_t y, int16_t z);
bool gyroSampleQueuePop(gyroDev_t *gyro);

static inline bool gyroSamplePending(const gyroDev_t *gyro)
{
    return gyro->sampleQueueTail != gyro->sampleQueueHead;
}

// On data-ready no new sample since the last read is not a failed read
static inline bool gyroReadsOnDataReady(const gyroDev_t *gyro)
{
    return gyro->dataReadyState != NULL;
}
#else
static inline bool gyroSamplePending(const gyroDev_t *gyro)
{
    (void)gyro;
    return false;
}

static inline bool gyroReadsOnDataReady(const gyroDev_t *gyro)
{
    (void)gyro;
    return false;
}
#endif
