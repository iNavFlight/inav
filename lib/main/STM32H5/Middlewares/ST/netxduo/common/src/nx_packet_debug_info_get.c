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


#ifdef NX_ENABLE_PACKET_DEBUG_INFO
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_packet_debug_info_get                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns status of packet for specified index          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                              Pool to packet pool           */
/*    packet_index                          Index of packet in pool       */
/*    packet_pptr                           Pointer to packet for output  */
/*    packet_status                         Status of packet for output   */
/*    thread_info                           Thread of packet for output   */
/*    file_info                             File of packet for output     */
/*    line                                  Line of packet for output     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
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
UINT  _nx_packet_debug_info_get(NX_PACKET_POOL *pool_ptr, UINT packet_index, NX_PACKET **packet_pptr,
                                ULONG *packet_status, CHAR **thread_info, CHAR **file_info, ULONG *line)
{
ULONG      payload_size;    /* Rounded payload size       */
ULONG      header_size;     /* Rounded header size        */
NX_PACKET *packet_ptr;

    /* Get the first packet. */
    packet_ptr = (NX_PACKET *)(pool_ptr -> nx_packet_pool_start);

    /* Calculate header size. */
    header_size = (ULONG)((ALIGN_TYPE)(packet_ptr -> nx_packet_data_start) - (ALIGN_TYPE)packet_ptr);

    /* Round the packet size up to something that helps guarantee proper alignment for header and payload.  */
    payload_size = (ULONG)(((pool_ptr -> nx_packet_pool_payload_size + header_size + NX_PACKET_ALIGNMENT  - 1) / NX_PACKET_ALIGNMENT) * NX_PACKET_ALIGNMENT - header_size);

    /* Calculate packet pointer. */
    packet_ptr = (NX_PACKET *)(pool_ptr -> nx_packet_pool_start + packet_index * (header_size + payload_size));

    /* Get packet pointer. */
    if (packet_pptr)
    {
        *packet_pptr = packet_ptr;
    }

    /* Get packet status. */
    if (packet_status)
    {
        *packet_status = (ULONG)(ALIGN_TYPE)packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;
    }

    /* Get thread info. */
    if (thread_info)
    {
        *thread_info = packet_ptr -> nx_packet_debug_thread;
    }

    /* Get file info. */
    if (file_info)
    {
        *file_info = packet_ptr -> nx_packet_debug_file;
    }

    /* Get line. */
    if (line)
    {
        *line = packet_ptr -> nx_packet_debug_line;
    }

    /* Return success. */
    return(NX_SUCCESS);
}
#endif /* NX_ENABLE_PACKET_DEBUG_INFO */

