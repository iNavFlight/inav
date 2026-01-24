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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>


#include "platform.h"

#if defined(WASM_BUILD)

#include <emscripten.h>
#include "drivers/serial.h"
#include "serial_ex.h"

// Message queue for thread-safe communication
// Increased to 256 bytes to handle larger MSP messages
// MSP responses can include sensor data, attitude info, etc.
#define SERIAL_EX_QUEUE_SIZE 256
#define SERIAL_EX_MAX_MSG_SIZE 256

typedef struct {
    uint8_t portIndex;
    uint8_t data[SERIAL_EX_MAX_MSG_SIZE];
    uint16_t length;
} SerialExMessage;

typedef struct {
    SerialExMessage messages[SERIAL_EX_QUEUE_SIZE];
    volatile uint32_t writeIdx;
    volatile uint32_t readIdx;
} SerialExMessageQueue;

static const struct serialPortVTable serialExVTable[];
static exPort_t serialExPorts[SERIAL_PORT_COUNT];
static SerialExMessageQueue messageQueue = {0};

// Queue-based message sending for thread-safe communication
// Fragments large messages across multiple queue entries if needed
static void serialExEnqueueMessage(uint8_t portIndex, const uint8_t* data, uint16_t length)
{
    uint16_t remaining = length;
    uint16_t offset = 0;
    
    // Fragment large messages into smaller chunks
    while (remaining > 0) {
        uint16_t chunkSize = (remaining > SERIAL_EX_MAX_MSG_SIZE) ? SERIAL_EX_MAX_MSG_SIZE : remaining;
        
        uint32_t nextIdx = (messageQueue.writeIdx + 1) % SERIAL_EX_QUEUE_SIZE;
        
        // Check if queue is full
        if (nextIdx == messageQueue.readIdx) {
            fprintf(stderr, "[SERIAL_EX] Message queue overflow at offset %u/%u, dropping remaining data\n", offset, length);
            return;
        }
        
        // Write message chunk to queue
        SerialExMessage *msg = &messageQueue.messages[messageQueue.writeIdx];
        msg->portIndex = portIndex;
        msg->length = chunkSize;
        memcpy(msg->data, data + offset, chunkSize);
        
        // Update write index (atomic-like operation for single write)
        messageQueue.writeIdx = nextIdx;
        
        // Move to next chunk
        remaining -= chunkSize;
        offset += chunkSize;
    }
}

// Exported function to check if messages are pending
bool serialExHasMessages(void)
{
    return messageQueue.readIdx != messageQueue.writeIdx;
}

// Exported function to retrieve next message
bool serialExGetMessage(uint8_t* outPortIndex, uint8_t* outData, uint16_t* outLength)
{
    if (messageQueue.readIdx == messageQueue.writeIdx) {
        return false;
    }
    
    SerialExMessage *msg = &messageQueue.messages[messageQueue.readIdx];
    *outPortIndex = msg->portIndex;
    *outLength = msg->length;
    memcpy(outData, msg->data, msg->length);
    
    messageQueue.readIdx = (messageQueue.readIdx + 1) % SERIAL_EX_QUEUE_SIZE;
    
    return true;
}

bool inavSerialExConnect(int portIndex) 
{ 
    if (portIndex < 0 || portIndex >= SERIAL_PORT_COUNT || !serialExPorts[portIndex].isInitalized || serialExPorts[portIndex].isConnected) {
        return false;
    }
    fprintf(stderr, "[SOCKET] Connected to UART %d\n", portIndex + 1);
    serialExPorts[portIndex].isConnected = true;
    return true;
}

bool inavSerialExDisconnect(int portIndex) 
{ 
    if (portIndex < 0 || portIndex >= SERIAL_PORT_COUNT || !serialExPorts[portIndex].isInitalized || !serialExPorts[portIndex].isConnected) {
        return false;
    }
    fprintf(stderr, "[SOCKET] Disconnected from UART %d\n", portIndex + 1);
    serialExPorts[portIndex].isConnected = false;
    return true; 
}

bool inavSerialExSend(int portIndex, const uint8_t* buffer, int recvSize )
{
    if (portIndex < 0 || portIndex >= SERIAL_PORT_COUNT || !serialExPorts[portIndex].isInitalized || !serialExPorts[portIndex].isConnected) {
        return false;
    }
    
    exPort_t *port = &serialExPorts[portIndex];
        
    for (ssize_t i = 0; i < recvSize; i++) {
        if (port->serialPort.rxCallback) {
            port->serialPort.rxCallback((uint16_t)buffer[i], port->serialPort.rxCallbackData);
        } else {
            port->serialPort.rxBuffer[port->serialPort.rxBufferHead] = buffer[i];
            port->serialPort.rxBufferHead = (port->serialPort.rxBufferHead + 1) % port->serialPort.rxBufferSize;
        }
    }

    return true;
}

uint8_t serialExRead(serialPort_t *instance)
{
    uint8_t ch;
    exPort_t *port = (exPort_t*)instance;

    ch = port->serialPort.rxBuffer[port->serialPort.rxBufferTail];
    port->serialPort.rxBufferTail = (port->serialPort.rxBufferTail + 1) % port->serialPort.rxBufferSize;

    return ch;
}

void serialExWritBuf(serialPort_t *instance, const void *data, int count)
{
    exPort_t *port = (exPort_t*)instance;

    if (!port->isConnected) {
        return;
    }
    
    // Use message queue instead of callback
    serialExEnqueueMessage(port->id - 1, (uint8_t*)data, count);
}

void serialExWrite(serialPort_t *instance, uint8_t ch)
{
    serialExWritBuf(instance, (void*)&ch, 1);
}

uint32_t serialExTotalRxBytesWaiting(const serialPort_t *instance)
{
    exPort_t *port = (exPort_t*)instance;
    uint32_t count;

    if (port->serialPort.rxBufferHead >= port->serialPort.rxBufferTail) {
        count = port->serialPort.rxBufferHead - port->serialPort.rxBufferTail;
    } else {
        count = port->serialPort.rxBufferSize + port->serialPort.rxBufferHead - port->serialPort.rxBufferTail;
    }
    return count;
}

serialPort_t* serialExInit(USART_TypeDef* USARTx, serialReceiveCallbackPtr callback, void* rxCallbackData, uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    exPort_t *port = NULL;

    const uint32_t id = (uintptr_t)USARTx;
    if (id <= SERIAL_PORT_COUNT) {
       port = &serialExPorts[id - 1];
    } else {
        return NULL;
    }

    port->id = id;
    port->isInitalized = true;

    port->serialPort.vTable = serialExVTable;
    port->serialPort.rxCallback = callback;
    port->serialPort.rxCallbackData = rxCallbackData;
    port->serialPort.rxBufferHead = port->serialPort.rxBufferTail = 0;
    port->serialPort.rxBufferSize = SERIAL_EX_BUFFER_SIZE;
    port->serialPort.rxBuffer = port->rxBuffer;
    port->serialPort.mode = mode;
    port->serialPort.baudRate = baudRate;
    port->serialPort.options = options;

    fprintf(stderr, "[SOCKET] SerialEx interface initialised for UART%d\n", id);

    return (serialPort_t*)port;
}



uint32_t serialExTotalTxBytesFree(const serialPort_t *instance)
{
    UNUSED(instance);
    return SERIAL_EX_MAX_PACKET_SIZE;
}

bool isSerialExTransmitBufferEmpty(const serialPort_t *instance)
{
    UNUSED(instance);
    return true;
}

bool serialExIsConnected(const serialPort_t *instance)
{
    return ((exPort_t*)instance)->isConnected;
}

void serialExSetBaudRate(serialPort_t *instance, uint32_t baudRate)
{
    UNUSED(instance);
    UNUSED(baudRate);
}


void serialExSetMode(serialPort_t *instance, portMode_t mode)
{
    UNUSED(instance);
    UNUSED(mode);
}

void serialExSetOptions(serialPort_t *instance, portOptions_t options)
{
    UNUSED(instance);
    UNUSED(options);
}

static const struct serialPortVTable serialExVTable[] = {
    {
        .serialWrite = serialExWrite,
        .serialTotalRxWaiting = serialExTotalRxBytesWaiting,
        .serialTotalTxFree = serialExTotalTxBytesFree,
        .serialRead = serialExRead,
        .serialSetBaudRate = serialExSetBaudRate,
        .isSerialTransmitBufferEmpty = isSerialExTransmitBufferEmpty,
        .setMode = serialExSetMode,
        .setOptions = serialExSetOptions,
        .isConnected = serialExIsConnected,
        .writeBuf = serialExWritBuf,
        .beginWrite = NULL,
        .endWrite = NULL,
        .isIdle = NULL,
    }
};

#endif
