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

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ipv6.h"
#include "nx_nd_cache.h"




#ifdef FEATURE_NX_IPV6
/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** NetX Component                                                        */
/**                                                                       */
/**   Internet Protocol version 6 Prefix Table (IPv6 prefix table)        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/




/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ipv6_prefix_list_add_entry                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This is an internal function.  It adds a prefix into the prefix     */
/*    table.                                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    prefix                                128-bit IPv6 prefix.          */
/*    prefix_length                         Length of the prefix.         */
/*    valid_lifetime                        Life time of this entry.      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                NX_SUCCESS: The entry has been*/
/*                                             added to the prefix table. */
/*                                          NX_OVERFLOW: The prefix table */
/*                                             is full.                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_process_ra                 ICMPv6 Router Advertisement   */
/*                                          process routine.              */
/*                                                                        */
/*  NOTE                                                                  */
/*                                                                        */
/*    This function acquires the nx_ipv6_protection mutex.                */
/*    Make sure the mutex is not acquired before calling this function.   */
/*                                                                        */
/*    This function cannot be called from an ISR.                         */
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
UINT  _nx_ipv6_prefix_list_add_entry(NX_IP *ip_ptr, ULONG *prefix,
                                     ULONG prefix_length, ULONG valid_lifetime)
{

INT                   invalid_bits;
INT                   i;
ULONG                 invalid_mask;
NX_IPV6_PREFIX_ENTRY *new_entry;      /* Pointer to the new entry. */
NX_IPV6_PREFIX_ENTRY *current;        /* Pointer to the location where the
                                         new entry is gonig to be inserted before it. */
NX_IPV6_PREFIX_ENTRY *prev = NX_NULL; /* Pointer to the location where the
                                         new entry is gonig to be inserted after it. */


    /*
       Insert the entry based on the prefix_length.
       Maintain longest-match-first.
     */

    /* Start with the head of the list. */
    current = ip_ptr -> nx_ipv6_prefix_list_ptr;

    /* Find where we should insert the new element. */
    while (current)
    {

        /* Search for the right location based on longest-prefix-match. */
        if (prefix_length > current -> nx_ipv6_prefix_entry_prefix_length)
        {

            /* Find the right location. */
            break;
        }

        /* Check if the entry matches or not.  We start by checking the
           length of the prefix.  This is a quick test. */
        if (prefix_length == current -> nx_ipv6_prefix_entry_prefix_length)
        {

            /* If the prefix_length is the same, check whether these
               two prefixes are the same.  If they are the same, we
               don't need to create a new entry. */
            if (CHECK_IPV6_ADDRESSES_SAME(prefix, current -> nx_ipv6_prefix_entry_network_address))
            {

                /* We have the same entry.  Just update the
                   valid_lifetime field, according to 5.5.3(e) in RFC 4862. */
                if ((valid_lifetime > 2 * 60 * 60) || /* if received lifetime is greater than 2 hours */
                    (valid_lifetime > current -> nx_ipv6_prefix_entry_valid_lifetime))
                {

                    current -> nx_ipv6_prefix_entry_valid_lifetime = valid_lifetime;
                }
                else if (current -> nx_ipv6_prefix_entry_valid_lifetime <= 2 * 60 * 60)
                {

                    /* Do nothing. */
                }
                else
                {
                    current -> nx_ipv6_prefix_entry_valid_lifetime = 2 * 60 * 60;
                }

                /* The entry is already in the table and is still valid.
                   No need to update.  Just return it.*/

                return(NX_DUPLICATED_ENTRY);
            }
        }

        /* Move to next entry. */
        prev = current;
        current = current -> nx_ipv6_prefix_entry_next;
    }

    /* We fall into this case if prefix length is greater than
       the prefix length of the current entry.  So we
       need to insert it in front of it. */

    /* Make sure the list is not full. */
    if (ip_ptr -> nx_ipv6_prefix_entry_free_list == NX_NULL)
    {
        return(NX_OVERFLOW);
    }

    /* Get a new entry from the free list. */
    new_entry = ip_ptr -> nx_ipv6_prefix_entry_free_list;

    /* Move free list to the next element. */
    ip_ptr -> nx_ipv6_prefix_entry_free_list = new_entry -> nx_ipv6_prefix_entry_next;

    if (ip_ptr -> nx_ipv6_prefix_entry_free_list)
    {
        ip_ptr -> nx_ipv6_prefix_entry_free_list -> nx_ipv6_prefix_entry_prev = NX_NULL;
    }

    /* Fill information into the new entry */
    COPY_IPV6_ADDRESS(prefix, new_entry -> nx_ipv6_prefix_entry_network_address);
    new_entry -> nx_ipv6_prefix_entry_prefix_length = prefix_length;
    new_entry -> nx_ipv6_prefix_entry_valid_lifetime = valid_lifetime;

    /* Zero out the bits in the prefix after the prefix length */
    invalid_bits = (INT)(128 - prefix_length);
    for (i = 3; i >= 0; i--)
    {

        if (invalid_bits <= 0)
        {
            break;
        }
        if (invalid_bits < 32)
        {
            invalid_mask = (ULONG)(~(((ULONG)1 << invalid_bits) - 1));
        }
        else
        {
            invalid_mask = 0;
        }

        new_entry -> nx_ipv6_prefix_entry_network_address[i] &= invalid_mask;

        invalid_bits -= 32;
    }

    /* Insert the new entry between prev and current. */
    new_entry -> nx_ipv6_prefix_entry_prev = prev;
    new_entry -> nx_ipv6_prefix_entry_next = current;
    if (current)
    {
        current -> nx_ipv6_prefix_entry_prev = new_entry;
    }
    if (prev)
    {
        prev -> nx_ipv6_prefix_entry_next = new_entry;
    }
    else
    {
        ip_ptr -> nx_ipv6_prefix_list_ptr = new_entry;
    }

    /* All done. */
    return(NX_SUCCESS);
}


#endif /* FEATURE_NX_IPV6 */

