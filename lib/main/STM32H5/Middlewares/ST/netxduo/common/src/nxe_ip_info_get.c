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

/* Bring in externs for caller checking code.  */
NX_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_ip_info_get                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the IP information get           */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    ip_total_packets_sent                 Destination for number of     */
/*                                            packets sent                */
/*    ip_total_bytes_sent                   Destination for number of     */
/*                                            bytes sent                  */
/*    ip_total_packets_received             Destination for number of     */
/*                                            packets received            */
/*    ip_total_bytes_received               Destination for number of     */
/*                                            bytes received              */
/*    ip_invalid_packets                    Destination for number of     */
/*                                            invalid packets             */
/*    ip_receive_packets_dropped            Destination for number of     */
/*                                            packets dropped             */
/*    ip_receive_checksum_errors            Destination for number of     */
/*                                            checksum errors             */
/*    ip_send_packets_dropped               Destination for number of     */
/*                                            send packets dropped        */
/*    ip_total_fragments_sent               Destination for number of     */
/*                                            fragments sent              */
/*    ip_total_fragments_received           Destination for number of     */
/*                                            fragments received          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_info_get                       Actual IP information get     */
/*                                            function                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
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
UINT  _nxe_ip_info_get(NX_IP *ip_ptr, ULONG *ip_total_packets_sent, ULONG *ip_total_bytes_sent,
                       ULONG *ip_total_packets_received, ULONG *ip_total_bytes_received,
                       ULONG *ip_invalid_packets, ULONG *ip_receive_packets_dropped,
                       ULONG *ip_receive_checksum_errors, ULONG *ip_send_packets_dropped,
                       ULONG *ip_total_fragments_sent, ULONG *ip_total_fragments_received)
{

UINT status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual IP information get function.  */
    status =  _nx_ip_info_get(ip_ptr, ip_total_packets_sent, ip_total_bytes_sent, ip_total_packets_received,
                              ip_total_bytes_received, ip_invalid_packets, ip_receive_packets_dropped,
                              ip_receive_checksum_errors, ip_send_packets_dropped,
                              ip_total_fragments_sent, ip_total_fragments_received);

    /* Return completion status.  */
    return(status);
}

