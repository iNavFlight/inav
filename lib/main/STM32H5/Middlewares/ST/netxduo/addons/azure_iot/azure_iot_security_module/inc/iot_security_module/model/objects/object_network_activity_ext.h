/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

#ifndef OBJECT_NETWORK_ACTIVITY_EXT_H
#define OBJECT_NETWORK_ACTIVITY_EXT_H

#include <asc_config.h>

#ifdef __cplusplus
extern   "C" {
#endif

#include "asc_security_core/utils/collection/hashset.h"
#include "asc_security_core/model/objects/network_activity.h"

#define IPV4_HASHSET_SIZE (10 * ASC_COLLECTOR_NETWORK_ACTIVITY_MAX_IPV4_OBJECTS_IN_CACHE / 7)
#define IPV6_HASHSET_SIZE (10 * ASC_COLLECTOR_NETWORK_ACTIVITY_MAX_IPV6_OBJECTS_IN_CACHE / 7)

#define IPV6_ADDRESS_HASH(address) \
    ((address)[0] ^ (address)[1] ^ (address)[2] ^ (address)[3])

HASHSET_DECLARATIONS(network_activity_ipv4_t)
network_activity_ipv4_t *network_activity_ipv4_init();
void network_activity_ipv4_deinit(network_activity_ipv4_t *network_activity_ipv4, void *context);

#ifndef NX_DISABLE_IPV6
HASHSET_DECLARATIONS(network_activity_ipv6_t)
network_activity_ipv6_t *network_activity_ipv6_init();
void network_activity_ipv6_deinit(network_activity_ipv6_t *network_activity_ipv6, void *context);
#endif /* NX_DISABLE_IPV6 */

#ifdef __cplusplus
}
#endif

#endif /* OBJECT_NETWORK_ACTIVITY_EXT_H */
