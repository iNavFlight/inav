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
/**   Transmission Control Protocol (TCP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_tcp.h"


/* Bring in externs for caller checking code.  */
NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_tcp_info_get                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the TCP information get          */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to the IP instance    */
/*    tcp_packets_sent                      Destination for number of     */
/*                                            packets sent                */
/*    tcp_bytes_sent                        Destination for number of     */
/*                                            bytes sent                  */
/*    tcp_packets_received                  Destination for number of     */
/*                                            packets received            */
/*    tcp_bytes_received                    Destination for number of     */
/*                                            bytes received              */
/*    tcp_invalid_packets                   Destination for number of     */
/*                                            invalid packets             */
/*    tcp_receive_packets_dropped           Destination for number of     */
/*                                            receive packets dropped     */
/*    tcp_checksum_errors                   Destination for number of     */
/*                                            checksum errors             */
/*    tcp_connections                       Destination for number of     */
/*                                            connections                 */
/*    tcp_disconnections                    Destination for number of     */
/*                                            disconnections              */
/*    tcp_connections_dropped               Destination for number of     */
/*                                            connections dropped         */
/*    tcp_retransmit_packets                Destination for number of     */
/*                                            retransmit packets          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_info_get                      Actual TCP information get    */
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
UINT  _nxe_tcp_info_get(NX_IP *ip_ptr, ULONG *tcp_packets_sent, ULONG *tcp_bytes_sent,
                        ULONG *tcp_packets_received, ULONG *tcp_bytes_received,
                        ULONG *tcp_invalid_packets, ULONG *tcp_receive_packets_dropped,
                        ULONG *tcp_checksum_errors, ULONG *tcp_connections,
                        ULONG *tcp_disconnections, ULONG *tcp_connections_dropped,
                        ULONG *tcp_retransmit_packets)
{

UINT status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check to see if TCP is enabled.  */
    if (!ip_ptr -> nx_ip_tcp_packet_receive)
    {
        return(NX_NOT_ENABLED);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual TCP information get function.  */
    status =  _nx_tcp_info_get(ip_ptr, tcp_packets_sent, tcp_bytes_sent, tcp_packets_received,
                               tcp_bytes_received, tcp_invalid_packets, tcp_receive_packets_dropped,
                               tcp_checksum_errors, tcp_connections, tcp_disconnections,
                               tcp_connections_dropped, tcp_retransmit_packets);

    /* Return completion status.  */
    return(status);
}

