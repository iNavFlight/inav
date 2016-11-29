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
#include "io_types.h"
#include "rcc_types.h"

#define MAX_BUS_QUEUE_LENGTH        4

typedef enum Bus_e {
#ifdef USE_SPI_DEVICE_1
    BUS_SPI1,
#endif
#ifdef USE_SPI_DEVICE_2
    BUS_SPI2,
#endif
#ifdef USE_SPI_DEVICE_3
    BUS_SPI3,
#endif
    MAX_BUS_COUNT
} Bus_t;

typedef enum BusSpeed_e {
    BUS_SPEED_LOWEST,
    BUS_SPEED_SLOW,
    BUS_SPEED_STANDARD,
    BUS_SPEED_FAST,
    BUS_SPEED_ULTRAFAST
} BusSpeed_e;

/* Bus address:
 *      I2C - device address
 *      SPI - IO_t of a device CS# pin
 */
typedef uint32_t BusDevice_t;

typedef enum BusTransactionType_e {
    BUS_READ,
    BUS_WRITE
} BusTransactionType_e;

typedef enum BusTransactionState_e {
    TXN_IDLE,
    TXN_QUEUED,
    TXN_BUSY_SETUP,
    TXN_BUSY_COMMAND,
    TXN_BUSY_PAYLOAD,
    TXN_BUSY_COMPLETE,
    TXN_DONE,
} BusTransactionState_e;

struct BusTransaction_s;

typedef void busTransactionCallback(const void * param, const uint8_t * payload, const int completed_bytes);

typedef struct BusTransaction_s {
    /* Transaction state */
    volatile BusTransactionState_e  state;
    BusTransactionType_e            type;
    BusDevice_t                     device;
    uint8_t *                       command;
    uint8_t *                       payload;
    int                             command_bytes;
    int                             payload_bytes;
    int                             completed_bytes;
    busTransactionCallback *        callback;
    const void *                    callbackParam;

    /* Bus transfer state - this may change more than once per transaction */
    int                             busSequence;
    uint8_t *                       busRxBufPtr;
    uint8_t *                       busTxBufPtr;
    int                             busBytesRemaining;
    int                             busBytesCompleted;
    uint32_t                        busTimeoutUs;
} BusTransaction_t;

typedef bool busHardwareInit(const void * hwDesc);
typedef void busHardwareProcessTxn(const void * hwDesc, BusTransaction_t * txn, uint32_t currentTime);
typedef void busHardwareSetSpeed(const void * hwDesc, const BusSpeed_e speed);

/* Bus hardware descriptor (read-only, defined per-target) */
typedef struct BusDescriptor_s {
    const void *            hw;
    busHardwareInit *       init;
    busHardwareProcessTxn * processTxn;
    busHardwareSetSpeed *   setSpeed;
} BusDescriptor_t;

/* Bus run-time context */
typedef struct BusContext_s {
    bool                initialised;
    BusDescriptor_t     desc;   // Copy of hardware descriptor
    BusTransaction_t    txnQueue[MAX_BUS_QUEUE_LENGTH];
    int                 queueHead;
    int                 queueTail;
    int                 queueCount;
} BusContext_t;

void busSetupTransfer(BusTransaction_t * txn, uint8_t * rxBuf, uint8_t * txBuf, int byteCount);

bool busQueueRead(const Bus_t busId, const BusDevice_t dev, uint8_t * cmd, const int cmd_size, uint8_t * data, const int data_size, busTransactionCallback callback, const void * callbackParam);
bool busQueueWrite(const Bus_t busId, const BusDevice_t dev, uint8_t * cmd, const int cmd_size, uint8_t * data, const int data_size, busTransactionCallback callback, const void * callbackParam);
bool busRead(const Bus_t busId, const BusDevice_t dev, uint8_t * cmd, const int cmd_size, uint8_t * data, const int data_size);
bool busWrite(const Bus_t busId, const BusDevice_t dev, uint8_t * cmd, const int cmd_size, uint8_t * data, const int data_size);
void busSetSpeed(const Bus_t busId, const BusSpeed_e speed);
bool busIsBusy(const Bus_t busId);

void busInit();
bool busInitDriver(const Bus_t busId);

bool taskBusCheck(uint32_t currentTime, uint32_t currentDeltaTime);
void taskBus(uint32_t currentTime);