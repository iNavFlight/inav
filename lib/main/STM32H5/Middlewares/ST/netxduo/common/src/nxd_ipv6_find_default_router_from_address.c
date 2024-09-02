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
/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   Internet Protocol version 6 Default Router Table (IPv6 router)      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ipv6.h"
#include "nx_nd_cache.h"


#ifdef FEATURE_NX_IPV6
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_find_default_router_from_address          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds the specific IPv6 default router.               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    router_address                        The specific gateway address  */
/*                                            to search for.              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    default router entry                  Pointer to the default router */
/*                                            entry.                      */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain IP protection mutex    */
/*    tx_mutex_put                          Release IP protection mutex   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_icmpv6_process_na                                              */
/*                                                                        */
/*  NOTE                                                                  */
/*                                                                        */
/*    This function acquires the nx_ipv6_protection mutex.                */
/*    Make sure the mutex is not locked before entering this function.    */
/*                                                                        */
/*    This function cannot be called from ISR.                            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_IPV6_DEFAULT_ROUTER_ENTRY *_nxd_ipv6_find_default_router_from_address(NX_IP *ip_ptr, ULONG *router_address)
{

INT                           i;
NX_IPV6_DEFAULT_ROUTER_ENTRY *rt_entry;


    /* Get exclusive access to the IP task lock. */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Start the search at the top of the list...*/
    i = 0;

    /* And go through the entire table or until a match is found. */
    while (i < NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE)
    {

        /* Local pointer to table entry. */
        rt_entry = &ip_ptr -> nx_ipv6_default_router_table[i];

        /* Does this slot contain a valid router? */
        if (rt_entry -> nx_ipv6_default_router_entry_flag)
        {

            /* Yes, check if it matches the specified router address. */
            if (CHECK_IPV6_ADDRESSES_SAME(router_address, rt_entry -> nx_ipv6_default_router_entry_router_address))
            {


                /* Yes it does, we can release the IP protection. */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                /* Return the pointer to this router entry. */
                return(rt_entry);
            }
        }

        i++;
    }

    /* Release the lock. */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return a null pointer indicating no matching router found. */
    return(NX_NULL);
}

#endif /* FEATURE_NX_IPV6 */

