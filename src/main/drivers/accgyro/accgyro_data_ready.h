/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "drivers/accgyro/accgyro.h"
#include "drivers/sensor.h"

// A driver that can sets gyro->dataReadyDriver in its initFn, describing its read below

// Decodes the bytes after the address, in the units of the driver's own reads
typedef void (*gyroDataReadyParseFn)(const uint8_t *data, bool withAccAndTemp, int16_t *gyro, int16_t *acc, int16_t *temp);

typedef struct gyroDataReadyRead_s {
    uint8_t reg;                        // the first register, as sent (read bit included)
    uint8_t len;                        // bytes after the address, 0 for no such read
} gyroDataReadyRead_t;

// What one FIFO packet held
typedef enum {
    GYRO_FIFO_PACKET_SAMPLE,            // a sample
    GYRO_FIFO_PACKET_INVALID,           // a packet without a valid sample, such as one before the first ODR
    GYRO_FIFO_PACKET_EMPTY,             // counted a moment before but not there yet: it comes with the next read
    GYRO_FIFO_PACKET_UNEXPECTED,        // not a packet the FIFO was set up for: the reads are out of step
} gyroFifoPacket_e;

// An IMU FIFO signalling once for several samples: one read from the count register brings the
// count and the packets after it
typedef struct gyroDataReadyFifo_s {
    uint8_t countReg;                   // as sent (read bit included), the packets follow it
    uint8_t packetLen;                  // with the accelerometer and temperature
    uint8_t gyroOnlyPacketLen;
    // Puts the samples in the FIFO and the interrupt on `samples` of them
    void (*start)(gyroDev_t *gyro, bool withAccAndTemp, uint8_t samples);
    // Empties the FIFO, from the gyro task
    void (*flush)(gyroDev_t *gyro);
    gyroFifoPacket_e (*parse)(const uint8_t *packet, bool withAccAndTemp, int16_t *gyro, int16_t *acc, int16_t *temp);
} gyroDataReadyFifo_t;

typedef struct gyroDataReadyDriver_s {
    gyroDataReadyRead_t withAccAndTemp; // the gyro with the accelerometer (and temperature)
    gyroDataReadyRead_t gyroOnly;
    bool hasTemp;                       // the first read brings the temperature too
    gyroDataReadyParseFn parse;
    sensorGyroReadFuncPtr registerRead; // the driver's own read, for when samples stop coming
    bool tested;                        // gyro_data_ready AUTO turns it on
    const gyroDataReadyFifo_t *fifo;    // NULL if the IMU has no FIFO this can use
} gyroDataReadyDriver_t;

typedef struct gyroDataReady_s gyroDataReady_t;

#if defined(USE_SPI_DATA_READY)
void gyroDataReadyStart(gyroDev_t *gyro);
void gyroDataReadySuspend(void);
// For the driver's acc and temperature reads: true when the last data-ready read brought them
bool gyroDataReadyAcc(const busDevice_t *dev, int16_t *acc);
bool gyroDataReadyTemperature(const busDevice_t *dev, int16_t *temp);
#endif
