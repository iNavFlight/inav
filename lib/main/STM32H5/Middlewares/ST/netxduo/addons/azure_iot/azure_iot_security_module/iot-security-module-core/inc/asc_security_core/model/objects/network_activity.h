/*******************************************************************************/
/*                                                                             */
/* Copyright (c) Microsoft Corporation. All rights reserved.                   */
/*                                                                             */
/* This software is licensed under the Microsoft Software License              */
/* Terms for Microsoft Azure Defender for IoT. Full text of the license can be */
/* found in the LICENSE file at https://aka.ms/AzureDefenderForIoT_EULA        */
/* and in the root directory of this software.                                 */
/*                                                                             */
/*******************************************************************************/

#ifndef OBJECT_NETWORK_ACTIVITY_H
#define OBJECT_NETWORK_ACTIVITY_H

#include <stdint.h>

#include <asc_config.h>

#include "asc_security_core/model/objects/transport_protocol.h"
#include "asc_security_core/utils/collection/collection.h"

typedef struct {
    uint32_t bytes_in;
    uint32_t bytes_out;
    uint16_t local_port;
    uint16_t remote_port;
    transport_protocol_t transport_protocol;
} network_activity_common_t;


typedef struct network_activity_ipv4_t {
    /* This macro must be first in object */
    COLLECTION_INTERFACE(struct network_activity_ipv4_t);
    network_activity_common_t common;
    uint32_t local_address;
    uint32_t remote_address;
} network_activity_ipv4_t;


typedef struct network_activity_ipv6_t {
    /* This macro must be first in object */
    COLLECTION_INTERFACE(struct network_activity_ipv6_t);
    network_activity_common_t common;
    uint32_t local_address[4];
    uint32_t remote_address[4];
} network_activity_ipv6_t;


#endif /* OBJECT_NETWORK_ACTIVITY_H */
