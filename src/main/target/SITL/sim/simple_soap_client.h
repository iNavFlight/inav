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

#include <arpa/inet.h>

#define SOAP_REC_BUF_SIZE 256 * 1024

typedef struct {
    int sockedFd;
    struct sockaddr_in socketAddr;
    bool isInitalised;
    bool isConnected;
} soap_client_t;

typedef struct {
    soap_client_t client;
    char* content;
} send_info_t;


bool soapClientConnect(soap_client_t *client, const char *address, int port);
void soapClientClose(soap_client_t *client);
void soapClientSendRequestVa(soap_client_t *client, const char* action, const char *fmt, va_list va);
void soapClientSendRequest(soap_client_t *client, const char* action, const char *fmt, ...);
char* soapClientReceive(soap_client_t *client);
