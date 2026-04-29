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

#pragma once

#include <pthread.h>

typedef struct {
    char host[256];
    char port[16];
    char path[256];
    int timeout_ms;
    pthread_mutex_t lock;
} soap_client_t;

int soap_client_init(soap_client_t* c, const char* host, const char* port, const char* path, int timeout_ms);
void soap_client_destroy(soap_client_t* c);

// Sends raw XML as content of <soap:Body> and returns raw XML content of response <soap:Body>.
// Caller must free(*response_body_xml) on success.
int soap_client_call_raw_body(soap_client_t* c,
                              const char* soap_action,
                              const char* request_body_xml,
                              char** response_body_xml,
                              int* http_status);