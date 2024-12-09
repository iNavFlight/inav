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
/**   Internet Protocol (IP)                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_gateway_address_get                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the IP gateway address that will be used for     */
/*    sending IP packets with destination address addresses not in the    */
/*    local network.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP control block pointer      */
/*    ip_address                            Gateway IP address            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
UINT  _nx_ip_gateway_address_get(NX_IP *ip_ptr, ULONG *ip_address)
{

#ifndef NX_DISABLE_IPV4
UINT status;


    /* Initialize the return value to be NX_NOT_FOUND.  If the gateway address
       is set, the return value changes to NX_SUCCESS. */
    status = NX_NOT_FOUND;

    /* Obtain the IP internal mutex to get the Gateway IP address.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    if (ip_ptr -> nx_ip_gateway_address)
    {

        /* Setup the Gateway IP address.  */
        *ip_address = ip_ptr -> nx_ip_gateway_address;

        status = NX_SUCCESS;
    }

    /* Release the protection mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return completion status.  */
    return(status);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(ip_address);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

