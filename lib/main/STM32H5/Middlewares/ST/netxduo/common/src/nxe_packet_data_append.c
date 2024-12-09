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
/**   Packet Pool Management (Packet)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_packet.h"

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_packet_data_append                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the packet data append           */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to packet to append   */
/*    data_start                            Pointer to start of the data  */
/*    data_size                             Number of bytes to append     */
/*    pool_ptr                              Pool to allocate the packet   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_data_append                Actual packet data append     */
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
UINT  _nxe_packet_data_append(NX_PACKET *packet_ptr, VOID *data_start, ULONG data_size,
                              NX_PACKET_POOL *pool_ptr, ULONG wait_option)
{

UINT status;


    /* Check for invalid input pointers.  */
    if ((pool_ptr == NX_NULL) || (pool_ptr -> nx_packet_pool_id != NX_PACKET_POOL_ID) ||
        (packet_ptr == NX_NULL) || (data_start == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for an invalid size of data to append.  */
    if (!data_size)
    {
        return(NX_SIZE_ERROR);
    }

    /* Check for an invalid packet prepend pointer.  */
    /*lint -e{946} suppress pointer subtraction, since it is necessary. */
    if (packet_ptr -> nx_packet_prepend_ptr < packet_ptr -> nx_packet_data_start)
    {
        return(NX_UNDERFLOW);
    }

    /* Check for an invalid packet append pointer.  */
    /*lint -e{946} suppress pointer subtraction, since it is necessary. */
    if (packet_ptr -> nx_packet_append_ptr > packet_ptr -> nx_packet_data_end)
    {
        return(NX_OVERFLOW);
    }

    /* Check for appropriate caller.  */
    NX_THREAD_WAIT_CALLER_CHECKING

    /* Call actual packet data append function.  */
    status =  _nx_packet_data_append(packet_ptr, data_start, data_size, pool_ptr, wait_option);

    /* Return completion status.  */
    return(status);
}

