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

#ifndef PROTOCOL_PRIVATE_H
#define PROTOCOL_PRIVATE_H
#include <asc_config.h>

#include "asc_security_core/model/objects/transport_protocol.h"
#include "asc_security_core/model/schema/protocol_builder.h"

static inline AzureIoTSecurity_Protocol_enum_t protocol_serialize_enum(transport_protocol_t protocol)
{
    switch (protocol) {
        case TRANSPORT_PROTOCOL_TCP:
            return AzureIoTSecurity_Protocol_TCP;
        case TRANSPORT_PROTOCOL_UDP:
            return AzureIoTSecurity_Protocol_UDP;
        case TRANSPORT_PROTOCOL_ICMP:
            return AzureIoTSecurity_Protocol_ICMP;
        default:
            return -1;
    }
}

#endif /* PROTOCOL_PRIVATE_H */
