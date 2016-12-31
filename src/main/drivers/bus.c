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

#include "platform.h"

#include "build/atomic.h"
#include "build/build_config.h"
#include "build/assert.h"
#include "build/debug.h"

#include "nvic.h"
#include "io.h"
#include "system.h"
#include "bus.h"

extern const BusDescriptor_t busHwDesc[MAX_BUS_COUNT];
static BusContext_t          bus[MAX_BUS_COUNT];
static volatile int          busTotalQueueCount = 0;

static void busProcessPendingTransactions(uint32_t currentTime);

static void busDequeueTransaction(const Bus_t busId)
{
    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
        busTotalQueueCount--;
        bus[busId].queueCount--;
        bus[busId].queueHead++;

        if (bus[busId].queueHead >= MAX_BUS_QUEUE_LENGTH) {
            bus[busId].queueHead = 0;
        }
    }
}

static BusTransaction_t * busQueueTransaction(const Bus_t busId, const BusDevice_t dev, const BusTransactionType_e type,
        uint8_t * cmd, const int cmd_size, uint8_t * data, const int data_size, busTransactionCallback callback, const void * callbackParam)
{
    BusTransaction_t * txn;

    ATOMIC_BLOCK(NVIC_PRIO_MAX) {
        ATOMIC_BARRIER(txn);
        if (bus[busId].initialised && bus[busId].queueCount < MAX_BUS_QUEUE_LENGTH) {
            // Fill the transaction context
            txn = &bus[busId].txnQueue[bus[busId].queueTail];

            txn->state = TXN_QUEUED;
            txn->type = type;
            txn->device = dev;
            txn->command = cmd;
            txn->payload = data;
            txn->command_bytes = cmd_size;
            txn->payload_bytes = data_size;
            txn->completed_bytes = 0;
            txn->callback = callback;
            txn->callbackParam = callbackParam;

            // Increase the tail index
            bus[busId].queueTail++;
            if (bus[busId].queueTail >= MAX_BUS_QUEUE_LENGTH) {
                bus[busId].queueTail = 0;
            }

            // Increase the queued transaction count
            bus[busId].queueCount++;
            busTotalQueueCount++;
        }
        else {
            txn = NULL;
        }
    }

    return txn;
}

static bool busExecuteBlockingTransaction(const Bus_t busId, const BusDevice_t dev, const BusTransactionType_e type,
        uint8_t * cmd, const int cmd_size, uint8_t * data, const int data_size)
{
    const BusTransaction_t * txn = busQueueTransaction(busId, dev, type, cmd, cmd_size, data, data_size, NULL, NULL);

    if (txn == NULL) {
        return false;
    }
    else {
        // Wait for transaction to complete
        while (txn->state != TXN_IDLE) {
            busProcessPendingTransactions(micros());
        }
    }

    return true;
}

// Enqueue the READ transaction and return, callback will be called when transaction is finished
bool busQueueRead(const Bus_t busId, const BusDevice_t dev, uint8_t * cmd, const int cmd_size, uint8_t * data, const int data_size, busTransactionCallback callback, const void * callbackParam)
{
    const BusTransaction_t * txn = busQueueTransaction(busId, dev, BUS_READ, cmd, cmd_size, data, data_size, callback, callbackParam);
    return (txn != NULL);
}

// Enqueue the WRITE transaction and return, callback will be called when transaction is finished
bool busQueueWrite(const Bus_t busId, const BusDevice_t dev, uint8_t * cmd, const int cmd_size, uint8_t * data, const int data_size, busTransactionCallback callback, const void * callbackParam)
{
    const BusTransaction_t * txn = busQueueTransaction(busId, dev, BUS_WRITE, cmd, cmd_size, data, data_size, callback, callbackParam);
    return (txn != NULL);
}

// Enqueue the READ transaction and wait for completion. If there are more transactions in the queue they will be executed first
bool busRead(const Bus_t busId, const BusDevice_t dev, uint8_t * cmd, const int cmd_size, uint8_t * data, const int data_size)
{
    return busExecuteBlockingTransaction(busId, dev, BUS_READ, cmd, cmd_size, data, data_size);
}

// Enqueue the WRITE transaction and wait for completion. If there are more transactions in the queue they will be executed first
bool busWrite(const Bus_t busId, const BusDevice_t dev, uint8_t * cmd, const int cmd_size, uint8_t * data, const int data_size)
{
    return busExecuteBlockingTransaction(busId, dev, BUS_WRITE, cmd, cmd_size, data, data_size);
}

bool busIsBusy(const Bus_t busId)
{
    if (!bus[busId].initialised || bus[busId].queueCount != 0) {
        // If there are transactions pending or hardware is not initialized - indicate as busy
        return true;
    }
    else {
        return false;
    }
}

void busSetSpeed(const Bus_t busId, const BusSpeed_e speed)
{
    // Can't change speed of uninitialised bus
    if (!bus[busId].initialised) {
        return;
    }

    // Wait until queue is flushed - this call shouldn't affect already queued transactions
    while (bus[busId].queueCount != 0) {
        busProcessPendingTransactions(micros());
    }

    // Call the hardware driver
    bus[busId].desc.setSpeed(bus[busId].desc.hw, speed);
}

static void busProcessPendingTransactions(uint32_t currentTime)
{
    for (Bus_t busId = 0; busId < MAX_BUS_COUNT; busId++) {
        // Count is non-zero, queueHead transaction is valid
        if (bus[busId].initialised && bus[busId].queueCount != 0) {
            BusTransaction_t * txn = &bus[busId].txnQueue[bus[busId].queueHead];

            switch(txn->state) {
                case TXN_QUEUED:        // TXN is not yet executed, mark as busy and fall-through
                case TXN_BUSY_SETUP:    // TXN in progress, pass the TXN pointer to hardware IOCTL
                case TXN_BUSY_COMMAND:
                case TXN_BUSY_PAYLOAD:
                case TXN_BUSY_COMPLETE:
                    bus[busId].desc.processTxn(bus[busId].desc.hw, txn, currentTime);
                    break;

                case TXN_DONE:      // TXN is moved to this state only by HW IOCTL
                    if (txn->callback) {
                        // Call callback if available
                        txn->callback(txn->callbackParam, txn->payload, txn->completed_bytes);
                    }

                    txn->state = TXN_IDLE;
                    busDequeueTransaction(busId);
                    break;

                case TXN_IDLE:      // Shouldn't happen, ignore the transaction
                default:
                    busDequeueTransaction(busId);
                    break;
            }
        }
    }
}

void busInit(void)
{
    for (int busId = 0; busId < MAX_BUS_COUNT; busId++) {
        /* Prepare context */
        memcpy(&bus[busId].desc, &busHwDesc[busId], sizeof(BusDescriptor_t));
        bus[busId].initialised = false;
        bus[busId].queueHead = 0;
        bus[busId].queueTail = 0;
        bus[busId].queueCount = 0;
    }

    busTotalQueueCount = 0;
}

bool busInitDriver(const Bus_t busId)
{
    /* Call hardware init */
    bus[busId].initialised = bus[busId].desc.init(bus[busId].desc.hw);
    return bus[busId].initialised;
}

void busSetupTransfer(BusTransaction_t * txn, uint8_t * rxBuf, uint8_t * txBuf, int byteCount)
{
    txn->busSequence = 0;
    txn->busRxBufPtr = rxBuf;
    txn->busTxBufPtr = txBuf;
    txn->busBytesRemaining = byteCount;
    txn->busBytesCompleted = 0;
}

bool taskBusCheck(uint32_t currentTime, uint32_t currentDeltaTime)
{
    UNUSED(currentTime);
    UNUSED(currentDeltaTime);
    return (busTotalQueueCount > 0);
}

void taskBus(uint32_t currentTime)
{
    busProcessPendingTransactions(currentTime);
}