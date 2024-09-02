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
/**   Packet Pool Mangement (Packet)                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */
#include "nx_api.h"
#include "nx_ip.h"

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_ip_max_payload_size_find                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the max IP payload computation   */
/*    service.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    src_address                           Packet Source Address         */
/*    dest_address                          Packet Destination Address    */
/*    protocol                              Protocol type                 */
/*    src_port                              Source port number, in host   */
/*                                            byte order.                 */
/*    dest_port                             Destination port number,      */
/*                                            in host byte order.         */
/*    start_offset_ptr                      Pointer to storage space that */
/*                                            contains amount of offset   */
/*                                            for payload.                */
/*    payload_length_ptr                    Pointer to storage space that */
/*                                            indicates the amount of     */
/*                                            payload data that would not */
/*                                            cause IP fragmentation.     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion Code.              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_max_payload_size_get              The acutal function that      */
/*                                            computes the max payload    */
/*                                            size.                       */
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

UINT _nxe_ip_max_payload_size_find(NX_IP *ip_ptr,
                                   NXD_ADDRESS *dest_address,
                                   UINT if_index,
                                   UINT src_port,
                                   UINT dest_port,
                                   ULONG protocol,
                                   ULONG *start_offset_ptr,
                                   ULONG *payload_length_ptr)
{

    /* Check for valid pointer to an IP instance.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Destination address must be valid. */
    if (dest_address == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    if ((dest_address -> nxd_ip_version != NX_IP_VERSION_V4) &&
        (dest_address -> nxd_ip_version != NX_IP_VERSION_V6))
    {
        return(NX_IP_ADDRESS_ERROR);
    }

    if ((protocol != NX_PROTOCOL_UDP) &&
        (protocol != NX_PROTOCOL_TCP))
    {
        return(NX_NOT_SUPPORTED);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    return(_nx_ip_max_payload_size_find(ip_ptr, dest_address, if_index, src_port, dest_port,
                                        protocol, start_offset_ptr, payload_length_ptr));
}

