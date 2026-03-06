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

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "drivers/serial.h"

// WebSocket uses base port + 10 (e.g., TCP is 5761, WebSocket is 5771)
#define WS_BASE_PORT_DEFAULT 5770
#define WS_BUFFER_SIZE 2048
#define WS_MAX_PACKET_SIZE 65535

// WebSocket frame opcodes
#define WS_OPCODE_CONTINUATION 0x0
#define WS_OPCODE_TEXT         0x1
#define WS_OPCODE_BINARY       0x2
#define WS_OPCODE_CLOSE        0x8
#define WS_OPCODE_PING         0x9
#define WS_OPCODE_PONG         0xA

typedef struct
{
    serialPort_t serialPort;

    uint8_t rxBuffer[WS_BUFFER_SIZE];

    uint8_t id;
    bool isInitialized;
    bool isHandshakeComplete;
    pthread_mutex_t receiveMutex;
    pthread_t receiveThread;
    int socketFd;
    int clientSocketFd;
    struct sockaddr_storage sockAddress;
    struct sockaddr_storage clientAddress;
    bool isClientConnected;

    // WebSocket frame assembly buffer
    uint8_t frameBuffer[WS_MAX_PACKET_SIZE];
    size_t frameBufferLen;
} wsPort_t;

serialPort_t *wsOpen(USART_TypeDef *USARTx, serialReceiveCallbackPtr callback, void *rxCallbackData, uint32_t baudRate, portMode_t mode, portOptions_t options);
void wsReceiveBytesEx(int portIndex, const uint8_t* buffer, ssize_t recvSize);
uint32_t wsRXBytesFree(int portIndex);
