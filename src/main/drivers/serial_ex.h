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

/*
 * SERIAL_EX - Extended Serial Interface for WebAssembly
 *
 * This module provides an extended serial communication interface that emulates
 * serial port functionality through function calls and callbacks, making serial
 * communication available within WebAssembly (WASM) builds where traditional
 * serial hardware access is not possible.
 *
 * Key Features:
 * - Emulates serial port behavior via callback-based API
 * - Integrates with INAV's serial port abstraction layer
 * - Provides bidirectional communication: send() and receive()
 * - Can be extended with JavaScript bridges for web-based connectivity
 * - Optional proxy mechanism for tunneling to real TCP connections
 *
 * Usage Pattern:
 * 1. Application calls serialExInit() to create a virtual serial port
 * 2. Data transmission: inavSerialExSend() pushes data out
 * 3. Data reception: inavSerialExReceive() injects received data (called from JS)
 * 4. Callbacks: Received data triggers the registered rxCallback for processing
 *
 * WebAssembly Integration:
 * - JavaScript layer can register receive callbacks via cwrap/ccall
 * - Allows web-based UI to exchange data with the flight controller
 * - Can interface with proxy scripts (e.g., Node.js) for TCP connectivity
 *
 * Proxy Architecture (Optional):
 * INAV WASM <-> JavaScript bridge <-> Proxy script (Node.js/Python) <-> Real TCP/Serial
 * This enables remote connectivity without requiring direct hardware access in the browser.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "drivers/serial.h"

#define SERIAL_EX_BUFFER_SIZE 2048
#define SERIAL_EX_MAX_PACKET_SIZE 65535

typedef struct 
{
    serialPort_t serialPort;

    uint8_t rxBuffer[SERIAL_EX_BUFFER_SIZE];
    uint8_t id;
    bool isInitalized;
    bool isConnected;
} exPort_t;

serialPort_t *serialExInit(USART_TypeDef *USARTx, serialReceiveCallbackPtr callback, void *rxCallbackData, uint32_t baudRate, portMode_t mode, portOptions_t options);

bool inavSerialExSend(int portIndex, const uint8_t* buffer, int recvSize );
bool inavSerialExConnect(int portIndex);
bool inavSerialExDisconnect(int portIndex);
