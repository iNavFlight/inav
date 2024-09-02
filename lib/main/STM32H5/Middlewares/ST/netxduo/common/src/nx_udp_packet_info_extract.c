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
#include "nx_udp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_udp_packet_info_extract                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function extracts the source IP, protocol, (the protocol is    */
/*    always UDP), port number and the incoming interface from the        */
/*    incoming packet.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to UDP packet         */
/*    ip_address                            Pointer to sender IP address  */
/*    protocol                              Pointer to packet protocol.   */
/*                                             Always 17 (UDP)            */
/*    port                                  Pointer to sender source port */
/*    incoming_interface                    Pointer to interface index    */
/*                                            packet received on          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nxd_udp_packet_info_extract           Invoke the NXD version of the */
/*                                            service.                    */
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
UINT  _nx_udp_packet_info_extract(NX_PACKET *packet_ptr, ULONG *ip_address,
                                  UINT *protocol, UINT *port, UINT *interface_index)
{

#ifndef NX_DISABLE_IPV4
NXD_ADDRESS nxd_ip_address;
UINT        status;


    /* Call the NXD version of the service. */
    status = _nxd_udp_packet_info_extract(packet_ptr, &nxd_ip_address, protocol, port, interface_index);

    if (status == NX_SUCCESS)
    {
        if (ip_address)
        {

            /*lint -e{644} suppress variable might not be initialized, since "nxd_ip_address" was initialized in _nxd_udp_packet_info_extract. */
            if (nxd_ip_address.nxd_ip_version == NX_IP_VERSION_V4)
            {
                *ip_address = nxd_ip_address.nxd_ip_address.v4;
            }
            else
            {
                return(NX_INVALID_PACKET);
            }
        }
    }

    return(status);
#else
    NX_PARAMETER_NOT_USED(packet_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(protocol);
    NX_PARAMETER_NOT_USED(port);
    NX_PARAMETER_NOT_USED(interface_index);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}

