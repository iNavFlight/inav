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
#include <arpa/inet.h>

#define BASE_IP_ADDRESS 5760
#define TCP_BUFFER_SIZE 2048
#define TCP_MAX_PACKET_SIZE 65535

typedef struct 
{
    serialPort_t serialPort;

    uint8_t rxBuffer[TCP_BUFFER_SIZE];

    uint8_t id;
    bool isInitalized;
    pthread_mutex_t receiveMutex;
    pthread_t receiveThread;
    int socketFd;
    int clientSocketFd;
    struct sockaddr_in sockAddress;
    struct sockaddr clientAddress;
    bool isClientConnected;
} tcpPort_t;


serialPort_t *tcpOpen(USART_TypeDef *USARTx, serialReceiveCallbackPtr callback, void *rxCallbackData, uint32_t baudRate, portMode_t mode, portOptions_t options);

void tcpSend(tcpPort_t *port);
int tcpReceive(tcpPort_t *port);
