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
 * Thread-safe message queue for inter-thread communication between:
 * - Main INAV thread (pthread/Worker)
 * - JavaScript main thread
 *
 * Supports two serial modes:
 * 1. MSP Mode: Binary protocol with structured messages (< 256 bytes)
 * 2. CLI Mode: Raw text input/output (up to 2 KB chunks)
 *
 * Architecture:
 * - INAV writes outgoing data to serialExWritBuf() (running in pthread)
 * - Data is enqueued in thread-safe circular message queue
 * - messagePendingPort flag is set to indicate availability
 * - JavaScript polls serialExGetPendingPort() every 10ms
 * - When data available, JavaScript calls serialExGetMessage() to retrieve it
 *
 * Queue Configuration:
 * - SERIAL_EX_MAX_MSG_SIZE: 256 bytes
 * - SERIAL_EX_QUEUE_SIZE: 4096 slots (efficient memory usage)
 *
 * Thread Safety:
 * - Write index only updated by C code (pthread context)
 * - Read index only updated by JavaScript (main thread context)
 * - No races: independent readers/writers
 * - Flag-based signaling (volatile atomic reads)
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

// Check for pending messages (non-blocking poll)
// Returns port index if messages available, or UINT32_MAX if none
// JavaScript polls this every 10ms to check for new messages
uint32_t serialExGetPendingPort(void);

// Retrieve next message from queue
// Called by JavaScript when serialExGetPendingPort() indicates messages available
bool serialExGetMessage(uint8_t* outPortIndex, uint8_t* outData, uint16_t* outLength);
