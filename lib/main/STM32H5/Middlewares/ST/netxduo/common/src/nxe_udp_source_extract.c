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
/**   User Datagram Protocol (UDP)                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_udp.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_udp_source_extract                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the extract UDP source call.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to UDP packet pointer */
/*    ip_address                            Pointer to packet source IP   */
/*                                            address                     */
/*    port                                  Pointer to source UDP port    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Actual completion status      */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_PACKET                     Malformed packet to process   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_udp_source_extract                Actual UDP source extract     */
/*                                            function                    */
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
UINT  _nxe_udp_source_extract(NX_PACKET *packet_ptr, ULONG *ip_address, UINT *port)
{

#ifndef NX_DISABLE_IPV4
UINT status;


    /* Check for invalid input pointers.  */
    if ((packet_ptr == NX_NULL) || (ip_address == NX_NULL) || (port == NX_NULL))
    {

        return(NX_PTR_ERROR);
    }

    /* Check for invalid packet pointer.  */
    if (packet_ptr -> nx_packet_ip_header == NX_NULL)
    {

        return(NX_INVALID_PACKET);
    }


    if (packet_ptr -> nx_packet_ip_version != NX_IP_VERSION_V4)
    {

        return(NX_INVALID_PACKET);
    }

    /* Check to see if the packet has enough room in front for backing up.  */
    /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
    if ((UINT)(packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_data_start) <
        (sizeof(NX_UDP_HEADER) + sizeof(NX_IPV4_HEADER)))
    {

        return(NX_INVALID_PACKET);
    }

    /* Call actual UDP source extract function.  */
    status =  _nx_udp_source_extract(packet_ptr, ip_address, port);

    /* Return completion status.  */
    return(status);
#else
    NX_PARAMETER_NOT_USED(packet_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(port);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

