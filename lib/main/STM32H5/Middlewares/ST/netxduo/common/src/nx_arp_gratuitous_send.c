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
/**   Address Resolution Protocol (ARP)                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_arp.h"
#include "nx_packet.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_arp_gratuitous_send                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function goes through all the physical interfaces to transmit  */
/*    gratuitous ARP message as long as the interface IP address is       */
/*    valid.  If a response is received at a later time, the supplied     */
/*    response handler is called.                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    response_handler                      Pointer to function to handle */
/*                                            Gratuitous ARP response     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_arp_announce_send                 Send ARP announce             */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
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
UINT  _nx_arp_gratuitous_send(NX_IP *ip_ptr, VOID (*response_handler)(NX_IP *ip_ptr, NX_PACKET *packet_ptr))
{

#ifndef NX_DISABLE_IPV4
ULONG i;

    /* Get mutex protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Save the response handler in the IP structure.  */
    ip_ptr -> nx_ip_arp_gratuitous_response_handler =  response_handler;

    /* Release mutex protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Loop to send ARP announce for all interfaces.  */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {

        /* Skip the entry the IP address is invalid */
        /*lint -e{539} suppress positive indentation.  */
        if (ip_ptr -> nx_ip_interface[i].nx_interface_ip_address == 0)
        {
            continue;
        }

        /* Send ARP announce.  */
        _nx_arp_announce_send(ip_ptr, i);
    }

    /* Return a successful completion.  */
    return(NX_SUCCESS);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(response_handler);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

