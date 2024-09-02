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
#include <stdlib.h>

#include <asc_config.h>

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_ipv6.h"

#include "asc_security_core/object_pool.h"

#include "iot_security_module/model/objects/object_network_activity_ext.h"

OBJECT_POOL_DECLARATIONS(network_activity_ipv4_t)
OBJECT_POOL_DEFINITIONS(network_activity_ipv4_t, ASC_COLLECTOR_NETWORK_ACTIVITY_MAX_IPV4_OBJECTS_IN_CACHE)
HASHSET_DEFINITIONS(network_activity_ipv4_t, IPV4_HASHSET_SIZE)


network_activity_ipv4_t *network_activity_ipv4_init()
{
    return object_pool_get(network_activity_ipv4_t);
}

void network_activity_ipv4_deinit(network_activity_ipv4_t *network_activity_ipv4, void *context)
{
    object_pool_free(network_activity_ipv4_t, network_activity_ipv4);
}


int hashset_network_activity_ipv4_t_equals(network_activity_ipv4_t *a, network_activity_ipv4_t *b)
{
    if (a == b)
    {
        return 1;
    }

    if (a == NULL || b == NULL)
    {
        return 0;
    }

    return (
            a->common.local_port == b->common.local_port &&
            a->common.remote_port == b->common.remote_port &&
            a->common.transport_protocol == b->common.transport_protocol &&
            a->local_address == b->local_address &&
            a->remote_address == b->remote_address
        );
}


unsigned int hashset_network_activity_ipv4_t_hash(network_activity_ipv4_t *data_ptr)
{
    unsigned int result = data_ptr->local_address;
    result ^= data_ptr->remote_address;
    result ^= (unsigned int)(((data_ptr->common.local_port << 16) | data_ptr->common.remote_port));
    result ^= data_ptr->common.transport_protocol;

    return result;
}


void hashset_network_activity_ipv4_t_update(network_activity_ipv4_t *old_data, network_activity_ipv4_t *new_data)
{
    old_data->common.bytes_in += new_data->common.bytes_in;
    old_data->common.bytes_out += new_data->common.bytes_out;
    object_pool_free(network_activity_ipv4_t, new_data);
}


#ifndef NX_DISABLE_IPV6
OBJECT_POOL_DECLARATIONS(network_activity_ipv6_t)
OBJECT_POOL_DEFINITIONS(network_activity_ipv6_t, ASC_COLLECTOR_NETWORK_ACTIVITY_MAX_IPV6_OBJECTS_IN_CACHE)
HASHSET_DEFINITIONS(network_activity_ipv6_t, IPV6_HASHSET_SIZE)

network_activity_ipv6_t *network_activity_ipv6_init()
{
    return object_pool_get(network_activity_ipv6_t);
}


void network_activity_ipv6_deinit(network_activity_ipv6_t *network_activity_ipv6, void *context)
{
    object_pool_free(network_activity_ipv6_t, network_activity_ipv6);
}


int hashset_network_activity_ipv6_t_equals(network_activity_ipv6_t *a, network_activity_ipv6_t *b)
{
    if (a == b)
    {
        return 1;
    }

    if (a == NULL || b == NULL)
    {
        return 0;
    }

    return (
            a->common.local_port == b->common.local_port &&
            a->common.remote_port == b->common.remote_port &&
            a->common.transport_protocol == b->common.transport_protocol &&
            CHECK_IPV6_ADDRESSES_SAME((ULONG *)a->local_address, (ULONG *)b->local_address) &&
            CHECK_IPV6_ADDRESSES_SAME((ULONG *)a->remote_address, (ULONG *)b->remote_address)
        );
}


unsigned int hashset_network_activity_ipv6_t_hash(network_activity_ipv6_t *data_ptr)
{
    unsigned int result = IPV6_ADDRESS_HASH(data_ptr->local_address);
    result ^= IPV6_ADDRESS_HASH(data_ptr->remote_address);
    result ^= (unsigned int)(((data_ptr->common.local_port << 16) | data_ptr->common.remote_port));
    result ^= data_ptr->common.transport_protocol;

    return result;
}


void hashset_network_activity_ipv6_t_update(network_activity_ipv6_t *old_data, network_activity_ipv6_t *new_data)
{
    old_data->common.bytes_in += new_data->common.bytes_in;
    old_data->common.bytes_out += new_data->common.bytes_out;
    object_pool_free(network_activity_ipv6_t, new_data);
}
#endif /* NX_DISABLE_IPV6 */