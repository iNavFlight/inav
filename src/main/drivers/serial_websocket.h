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

#include <stdbool.h>
#include <stdint.h>

#include "drivers/serial.h"

#define WEBSOCKET_BUFFER_SIZE 2048
#define WEBSOCKET_MAX_PACKET_SIZE 65535

typedef struct
{
    serialPort_t serialPort;

    uint8_t rxBuffer[WEBSOCKET_BUFFER_SIZE];

    uint8_t id;
    bool isInitialized;
    bool isConnected;
    int wsHandle;  // WebSocket handle from JavaScript
} websocketPort_t;

serialPort_t *websocketOpen(USART_TypeDef *USARTx, serialReceiveCallbackPtr callback, void *rxCallbackData, uint32_t baudRate, portMode_t mode, portOptions_t options);

// Called from JavaScript when WebSocket receives data
extern void websocketReceiveBytes(int portIndex, const uint8_t* buffer, size_t recvSize);
