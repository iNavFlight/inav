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

#include "platform.h"

#if defined(WASM_BUILD)

#include <emscripten.h>

#include "common/utils.h"

#include "drivers/serial.h"
#include "drivers/serial_websocket.h"

static const struct serialPortVTable websocketVTable[];
static websocketPort_t websocketPorts[SERIAL_PORT_COUNT];

static websocketPort_t *websocketConfigure(websocketPort_t *port, uint32_t id)
{
    if (port->isInitialized) {
        return port;
    }

    port->id = id;
    port->isInitialized = true;
    port->isConnected = false;

    // Initialize WebSocket via JavaScript
    EM_ASM({
        const portId = $0;
        const uartId = $1;
        
        // Check if we're in a browser environment
        if (typeof window === 'undefined') {
            console.warn('WebSocket initialization skipped: not in browser environment');
            Module._websocketSetConnected(portId, 0);
            return;
        }
        
        if (!window.wasmWebSockets) {
            window.wasmWebSockets = {};
        }

        if (window.wasmWebSockets[uartId]) {
            console.warn('WebSocket for UART' + uartId + ' already exists');
            return;
        }

        // Check if WebSocket is available
        if (typeof WebSocket === 'undefined') {
            console.warn('WebSocket not available in this environment');
            Module._websocketSetConnected(portId, 0);
            return;
        }

        // Default WebSocket URL - can be changed from JavaScript
        const wsUrl = window.wasmWebSocketUrls && window.wasmWebSocketUrls[uartId] 
            ? window.wasmWebSocketUrls[uartId]
            : 'ws://localhost:' + (5760 + uartId - 1);

        console.log('Initializing WebSocket for UART' + uartId + ' at', wsUrl);

        try {
            const ws = new WebSocket(wsUrl);
            ws.binaryType = 'arraybuffer';

            ws.onopen = function() {
                console.log('WebSocket UART' + uartId + ' connected');
                Module._websocketSetConnected(portId, 1);
            };

            ws.onmessage = function(event) {
                const data = new Uint8Array(event.data);
                const buffer = Module._malloc(data.length);
                Module.HEAPU8.set(data, buffer);
                Module._websocketReceiveBytes(portId, buffer, data.length);
                Module._free(buffer);
            };

            ws.onerror = function(error) {
                console.error('WebSocket UART' + uartId + ' error:', error);
                Module._websocketSetConnected(portId, 0);
            };

            ws.onclose = function() {
                console.log('WebSocket UART' + uartId + ' disconnected');
                Module._websocketSetConnected(portId, 0);
            };

            window.wasmWebSockets[uartId] = ws;
        } catch (e) {
            console.error('Failed to create WebSocket for UART' + uartId + ':', e);
            Module._websocketSetConnected(portId, 0);
        }
    }, (int)id, (int)id);

    return port;
}

void websocketReceiveBytes(int portIndex, const uint8_t* buffer, size_t recvSize)
{
    if (portIndex < 0 || portIndex >= SERIAL_PORT_COUNT) {
        return;
    }

    websocketPort_t *port = &websocketPorts[portIndex];

    for (size_t i = 0; i < recvSize; i++) {
        if (port->serialPort.rxCallback) {
            port->serialPort.rxCallback((uint16_t)buffer[i], port->serialPort.rxCallbackData);
        } else {
            port->serialPort.rxBuffer[port->serialPort.rxBufferHead] = buffer[i];
            port->serialPort.rxBufferHead = (port->serialPort.rxBufferHead + 1) % port->serialPort.rxBufferSize;
        }
    }
}

// Called from JavaScript to set connection state
void websocketSetConnected(int portIndex, int connected)
{
    if (portIndex < 0 || portIndex >= SERIAL_PORT_COUNT) {
        return;
    }

    websocketPorts[portIndex].isConnected = (connected != 0);
}

serialPort_t *websocketOpen(USART_TypeDef *USARTx, serialReceiveCallbackPtr callback, void *rxCallbackData, uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    websocketPort_t *port = NULL;

#if defined(USE_UART1) || defined(USE_UART2) || defined(USE_UART3) || defined(USE_UART4) || defined(USE_UART5) || defined(USE_UART6) || defined(USE_UART7) || defined(USE_UART8)
    uint32_t id = (uintptr_t)USARTx;
    if (id > 0 && id <= SERIAL_PORT_COUNT) {
        port = websocketConfigure(&websocketPorts[id - 1], id);
    }
#endif

    if (port == NULL) {
        return NULL;
    }

    port->serialPort.vTable = websocketVTable;
    port->serialPort.rxCallback = callback;
    port->serialPort.rxCallbackData = rxCallbackData;
    port->serialPort.rxBufferHead = port->serialPort.rxBufferTail = 0;
    port->serialPort.rxBufferSize = WEBSOCKET_BUFFER_SIZE;
    port->serialPort.rxBuffer = port->rxBuffer;
    port->serialPort.mode = mode;
    port->serialPort.baudRate = baudRate;
    port->serialPort.options = options;

    return (serialPort_t*)port;
}

uint8_t websocketRead(serialPort_t *instance)
{
    uint8_t ch;
    websocketPort_t *port = (websocketPort_t*)instance;

    ch = port->serialPort.rxBuffer[port->serialPort.rxBufferTail];
    port->serialPort.rxBufferTail = (port->serialPort.rxBufferTail + 1) % port->serialPort.rxBufferSize;

    return ch;
}

void websocketWriteBuf(serialPort_t *instance, const void *data, int count)
{
    websocketPort_t *port = (websocketPort_t*)instance;

    if (!port->isConnected) {
        return;
    }

    EM_ASM({
        const uartId = $0;
        const buffer = $1;
        const count = $2;

        // Check if we're in a browser environment
        if (typeof window === 'undefined') {
            console.warn('WebSocket send skipped: not in browser environment');
            return;
        }

        if (!window.wasmWebSockets || !window.wasmWebSockets[uartId]) {
            console.warn('WebSocket for UART' + uartId + ' not available');
            return;
        }

        const ws = window.wasmWebSockets[uartId];
        if (ws.readyState !== WebSocket.OPEN) {
            console.warn('WebSocket UART' + uartId + ' not open (state: ' + ws.readyState + ')');
            return;
        }

        try {
            const data = Module.HEAPU8.slice(buffer, buffer + count);
            ws.send(data);
        } catch (e) {
            console.error('Failed to send WebSocket data on UART' + uartId + ':', e);
        }
    }, (int)port->id, (intptr_t)data, (int)count);
}

void websocketWrite(serialPort_t *instance, uint8_t ch)
{
    websocketWriteBuf(instance, (void*)&ch, 1);
}

uint32_t websocketTotalRxBytesWaiting(const serialPort_t *instance)
{
    websocketPort_t *port = (websocketPort_t*)instance;
    uint32_t count;

    if (port->serialPort.rxBufferHead >= port->serialPort.rxBufferTail) {
        count = port->serialPort.rxBufferHead - port->serialPort.rxBufferTail;
    } else {
        count = port->serialPort.rxBufferSize + port->serialPort.rxBufferHead - port->serialPort.rxBufferTail;
    }

    return count;
}

uint32_t websocketTotalTxBytesFree(const serialPort_t *instance)
{
    UNUSED(instance);
    return WEBSOCKET_MAX_PACKET_SIZE;
}

bool isWebsocketTransmitBufferEmpty(const serialPort_t *instance)
{
    UNUSED(instance);
    return true;
}

bool websocketIsConnected(const serialPort_t *instance)
{
    return ((websocketPort_t*)instance)->isConnected;
}

void websocketSetBaudRate(serialPort_t *instance, uint32_t baudRate)
{
    UNUSED(instance);
    UNUSED(baudRate);
}

void websocketSetMode(serialPort_t *instance, portMode_t mode)
{
    UNUSED(instance);
    UNUSED(mode);
}

void websocketSetOptions(serialPort_t *instance, portOptions_t options)
{
    UNUSED(instance);
    UNUSED(options);
}

static const struct serialPortVTable websocketVTable[] = {
    {
        .serialWrite = websocketWrite,
        .serialTotalRxWaiting = websocketTotalRxBytesWaiting,
        .serialTotalTxFree = websocketTotalTxBytesFree,
        .serialRead = websocketRead,
        .serialSetBaudRate = websocketSetBaudRate,
        .isSerialTransmitBufferEmpty = isWebsocketTransmitBufferEmpty,
        .setMode = websocketSetMode,
        .setOptions = websocketSetOptions,
        .isConnected = websocketIsConnected,
        .writeBuf = websocketWriteBuf,
        .beginWrite = NULL,
        .endWrite = NULL,
        .isIdle = NULL,
    }
};

#endif
