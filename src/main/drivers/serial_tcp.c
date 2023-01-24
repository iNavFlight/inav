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

#if defined(SITL_BUILD)

#include <sys/socket.h>
#include <fcntl.h>

#include "common/utils.h"

#include "drivers/serial.h"
#include "drivers/serial_tcp.h"

static const struct serialPortVTable tcpVTable[];
static tcpPort_t tcpPorts[SERIAL_PORT_COUNT];
static bool tcpThreadRunning = false;

static void *tcpReceiveThread(void* arg)
{
    tcpPort_t *port = (tcpPort_t*)arg;

    while(tcpThreadRunning) {
        if (tcpReceive(port) < 0) {
            break;
        }   
    }

    return NULL;
}

static tcpPort_t *tcpReConfigure(tcpPort_t *port, uint32_t id)
{
    if (port->isInitalized){
        return port;
    }

    if (pthread_mutex_init(&port->receiveMutex, NULL) != 0){
        return NULL;
    }

    port->socketFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (port->socketFd < 0) {
        fprintf(stderr, "[SOCKET] Unable to create tcp socket\n");
        return NULL;
    }
    
    int one = 1;
    int err = 0;
    err = setsockopt(port->socketFd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
    err = fcntl(port->socketFd, F_SETFL, fcntl(port->socketFd, F_GETFL, 0) | O_NONBLOCK);

    if (err < 0){
        fprintf(stderr, "[SOCKET] Unable to set socket options\n");
        return NULL;
    }

    uint16_t tcpPort = BASE_IP_ADDRESS + id - 1; 
    port->sockAddress.sin_family = AF_INET;
    port->sockAddress.sin_port = htons(tcpPort);
    port->sockAddress.sin_addr.s_addr = INADDR_ANY;
    port->isClientConnected = false;
    port->id = id;
    
    if (bind(port->socketFd, (struct sockaddr*)&port->sockAddress, sizeof(port->sockAddress)) < 0) {
        fprintf(stderr, "[SOCKET] Unable to bind socket\n");
        return NULL;
    }

    if (listen(port->socketFd, 100) < 0) {
        fprintf(stderr, "[SOCKET] Unable to listen.\n");
        return NULL;
    }

    fprintf(stderr, "[SOCKET] Bind TCP port %d to UART%d\n", tcpPort, id);

    return port;    
}

static char *tcpGetAddressString(struct sockaddr *addr)
{
    return inet_ntoa(((struct sockaddr_in *)addr)->sin_addr);
}

int tcpReceive(tcpPort_t *port)
{ 
    if (!port->isClientConnected) {
        
        fd_set fds;

        FD_ZERO(&fds);
        FD_SET(port->socketFd, &fds);
        
        if (select(port->socketFd + 1, &fds, NULL, NULL, NULL) < 0) {
            fprintf(stderr, "[SOCKET] Unable to wait for connection.\n");
            return -1;
        } 

        socklen_t addrLen = sizeof(port->sockAddress);
        port->clientSocketFd = accept(port->socketFd, &port->clientAddress, &addrLen);
        if (port->clientSocketFd < 1) {
            fprintf(stderr, "[SOCKET] Can't accept connection.\n");
            return -1;
        }

        fprintf(stderr, "[SOCKET] %s connected to UART%d\n", tcpGetAddressString(&port->clientAddress), port->id);
        port->isClientConnected = true;
    }
        
    uint8_t buffer[TCP_BUFFER_SIZE];
    ssize_t recvSize = recv(port->clientSocketFd, buffer, TCP_BUFFER_SIZE, 0);

    // Disconnect
    if (port->isClientConnected && recvSize == 0)
    {
       fprintf(stderr, "[SOCKET] %s disconnected from UART%d\n", tcpGetAddressString(&port->clientAddress), port->id);
       close(port->clientSocketFd);
       memset(&port->clientAddress, 0, sizeof(port->clientAddress));
       port->isClientConnected = false;
       
       return 0;
    }

    for (ssize_t i = 0; i < recvSize; i++) {
        
        if (port->serialPort.rxCallback) {
            port->serialPort.rxCallback((uint16_t)buffer[i], port->serialPort.rxCallbackData);
        } else {
            pthread_mutex_lock(&port->receiveMutex);
            port->serialPort.rxBuffer[port->serialPort.rxBufferHead] = buffer[i];
            port->serialPort.rxBufferHead = (port->serialPort.rxBufferHead + 1) % port->serialPort.rxBufferSize;
            pthread_mutex_unlock(&port->receiveMutex);
        }
    }
 
    if (recvSize < 0) {
        recvSize = 0;
    }

    return (int)recvSize;
}

serialPort_t *tcpOpen(USART_TypeDef *USARTx, serialReceiveCallbackPtr callback, void *rxCallbackData, uint32_t baudRate, portMode_t mode, portOptions_t options)
{
    tcpPort_t *port = NULL;

#if defined(USE_UART1) || defined(USE_UART2) || defined(USE_UART3) || defined(USE_UART4) || defined(USE_UART5) || defined(USE_UART6) || defined(USE_UART7) || defined(USE_UART8)
    uint32_t id = (uintptr_t)USARTx;
    if (id < SERIAL_PORT_COUNT) {
        port = tcpReConfigure(&tcpPorts[id], id);
    }
#endif

    if (port == NULL) {
         return NULL;

    }

    port->serialPort.vTable = tcpVTable;
    port->serialPort.rxCallback = callback;
    port->serialPort.rxCallbackData = rxCallbackData;
    port->serialPort.rxBufferHead = port->serialPort.rxBufferTail = 0;
    port->serialPort.rxBufferSize = TCP_BUFFER_SIZE;
    port->serialPort.rxBuffer = port->rxBuffer;
    port->serialPort.mode = mode;
    port->serialPort.baudRate = baudRate;
    port->serialPort.options = options;

    int err = pthread_create(&port->receiveThread, NULL, tcpReceiveThread, (void*)port);
    if (err < 0){
        fprintf(stderr, "[SOCKET] Unable to create receive thread for UART%d\n", id);
        return NULL;
    }
    tcpThreadRunning = true;
    
    return (serialPort_t*)port;
}

uint8_t tcpRead(serialPort_t *instance)
{
    uint8_t ch;
    tcpPort_t *port = (tcpPort_t*)instance;
    pthread_mutex_lock(&port->receiveMutex);

    ch = port->serialPort.rxBuffer[port->serialPort.rxBufferTail];
    port->serialPort.rxBufferTail = (port->serialPort.rxBufferTail + 1) % port->serialPort.rxBufferSize;

    pthread_mutex_unlock(&port->receiveMutex);

    return ch;
}

void tcpWritBuf(serialPort_t *instance, const void *data, int count)
{
    tcpPort_t *port = (tcpPort_t*)instance;

    if (!port->isClientConnected) {
        return;
    }

    send(port->clientSocketFd, data, count, 0);
}

void tcpWrite(serialPort_t *instance, uint8_t ch)
{
    tcpWritBuf(instance, (void*)&ch, 1);
}

uint32_t tcpTotalRxBytesWaiting(const serialPort_t *instance)
{
    tcpPort_t *port = (tcpPort_t*)instance;
    uint32_t count;

    pthread_mutex_lock(&port->receiveMutex);

    if (port->serialPort.rxBufferHead >= port->serialPort.rxBufferTail) {
        count = port->serialPort.rxBufferHead - port->serialPort.rxBufferTail;
    } else {
        count = port->serialPort.rxBufferSize + port->serialPort.rxBufferHead - port->serialPort.rxBufferTail;
    }

    pthread_mutex_unlock(&port->receiveMutex);

    return count;
}

uint32_t tcpTotalTxBytesFree(const serialPort_t *instance)
{
    tcpPort_t *port = (tcpPort_t*)instance;
    
    if (port->isClientConnected) {
        return TCP_MAX_PACKET_SIZE;
    } else {
        return 0;
    }
}

bool isTcpTransmitBufferEmpty(const serialPort_t *instance)
{
    UNUSED(instance);

    return true;
}

bool tcpIsConnected(const serialPort_t *instance)
{
    return ((tcpPort_t*)instance)->isClientConnected;
}

void tcpSetBaudRate(serialPort_t *instance, uint32_t baudRate)
{
    UNUSED(instance);
    UNUSED(baudRate);
}


void tcpSetMode(serialPort_t *instance, portMode_t mode)
{
    UNUSED(instance);
    UNUSED(mode);
}

static const struct serialPortVTable tcpVTable[] = {
    {
        .serialWrite = tcpWrite,
        .serialTotalRxWaiting = tcpTotalRxBytesWaiting,
        .serialTotalTxFree = tcpTotalTxBytesFree,
        .serialRead = tcpRead,
        .serialSetBaudRate = tcpSetBaudRate,
        .isSerialTransmitBufferEmpty = isTcpTransmitBufferEmpty,
        .setMode = tcpSetMode,
        .isConnected = tcpIsConnected,
        .writeBuf = tcpWritBuf,
        .beginWrite = NULL,
        .endWrite = NULL,
        .isIdle = NULL,
    }
};

#endif
