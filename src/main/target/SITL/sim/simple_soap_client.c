/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/select.h>

#include "simple_soap_client.h"

#define REC_BUF_SIZE 6000
char recBuffer[REC_BUF_SIZE];

bool soapClientConnect(soap_client_t *client, const char *address, int port)
{
    client->sockedFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client->sockedFd < 0) {
        return false;
    }

    int one = 1;
    if (setsockopt(client->sockedFd, SOL_SOCKET, SO_REUSEADDR, &one,sizeof(one)) < 0) {
        return false;
    }
    
    client->socketAddr.sin_family = AF_INET;
    client->socketAddr.sin_port = htons(port);
    client->socketAddr.sin_addr.s_addr = inet_addr(address);

    if (connect(client->sockedFd, (struct sockaddr*)&client->socketAddr, sizeof(client->socketAddr)) < 0) {
        return false;
    }

    client->isConnected = true;
    client->isInitalised = true;

    return true;
}

void soapClientClose(soap_client_t *client)
{
    close(client->sockedFd);
    memset(client, 0, sizeof(soap_client_t));
}

void soapClientSendRequestVa(soap_client_t *client, const char* action, const char *fmt, va_list va)
{
     if (!client->isConnected) {
        return;
    }

    char* requestBody;  
    vasprintf(&requestBody, fmt, va);
    
    char* request;
    asprintf(&request, "POST / HTTP/1.1\r\nsoapaction: %s\r\ncontent-length: %u\r\ncontent-type: text/xml;charset='UTF-8'\r\nConnection: Keep-Alive\r\n\r\n<soap:Envelope xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/soap/envelope/\"><soap:Body>%s</soap:Body></soap:Envelope>",
         action, (unsigned)strlen(requestBody), requestBody);
    
    send(client->sockedFd, request, strlen(request), 0);
}

void soapClientSendRequest(soap_client_t *client, const char* action, const char *fmt, ...)
{
    va_list va;
    
    va_start(va, fmt);
    soapClientSendRequestVa(client, action, fmt, va);   
    va_end(va);
}

static bool soapClientPoll(soap_client_t *client, uint32_t timeout_ms)
{
    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(client->sockedFd, &fds);

    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000UL;

    if (select(client->sockedFd + 1, &fds, NULL, NULL, &tv) != 1) {
        return false;
    }
    return true;
}


char* soapClientReceive(soap_client_t *client)
{
     if (!client->isInitalised){
        return false;
    }

    if (!soapClientPoll(client, 1000)) {
        return false;
    }

    ssize_t size = recv(client->sockedFd, recBuffer, REC_BUF_SIZE, 0);

    if (size <= 0) {
        return NULL;
    }

    char* pos = strstr(recBuffer, "Content-Length: ");
    if (!pos) {
       return NULL;
    }

    uint32_t contentLength = strtoul(pos + 16, NULL, 10);
    char *body = strstr(pos, "\r\n\r\n");
    if (!body) {
        return NULL;
    }

    body += 4;

    ssize_t expectedLength = contentLength + body - recBuffer;
    if ((unsigned)expectedLength >= sizeof(recBuffer)) {
        return NULL;
    }

    while (size < expectedLength){
        ssize_t size2 = recv(client->sockedFd, &recBuffer[size], sizeof(recBuffer - size + 1), 0);
        if (size2 <= 0) {
            return NULL;
        }
        size += size2;       
    }

    recBuffer[size] = '\0';
    return strdup(body);
}

