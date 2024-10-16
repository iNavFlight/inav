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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_packet_data_retrieve                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function copies data from a NetX packet (or packet chain) into */
/*    the supplied user buffer.                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to the source packet  */
/*    buffer_start                          Pointer to destination area   */
/*    bytes_copied                          Number of bytes copied        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_packet_data_retrieve(NX_PACKET *packet_ptr, VOID *buffer_start, ULONG *bytes_copied)
{

ULONG  remaining_bytes;
UCHAR *destination_ptr;
ULONG  bytes_to_copy;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_PACKET_DATA_RETRIEVE, packet_ptr, buffer_start, bytes_copied, 0, NX_TRACE_PACKET_EVENTS, 0, 0);

    /* Setup the destination pointer.  */
    destination_ptr =  buffer_start;

    /* Pickup the amount of bytes to copy.  */
    *bytes_copied =  packet_ptr -> nx_packet_length;

    /* Setup the remaining bytes.  */
    remaining_bytes =  packet_ptr -> nx_packet_length;

#ifndef NX_DISABLE_PACKET_CHAIN
    /* Loop to copy bytes from packet(s).  */
    while (packet_ptr)
    {
#endif /* NX_DISABLE_PACKET_CHAIN */

        /* Calculate the bytes to copy in this packet. */
        /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
        bytes_to_copy = (ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr);

        /* Copy data to destination. */
        /* Note: The buffer size must be not less than packet_ptr -> nx_packet_length.  */
        memcpy(destination_ptr, packet_ptr -> nx_packet_prepend_ptr, bytes_to_copy); /* Use case of memcpy is verified. The buffer is provided by user.  lgtm[cpp/banned-api-usage-required-any] */

        remaining_bytes -= bytes_to_copy;
        destination_ptr += bytes_to_copy;

#ifndef NX_DISABLE_PACKET_CHAIN
        /* Move to next packet.  */
        packet_ptr =  packet_ptr -> nx_packet_next;
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Determine if the packet chain was valid.  */
    if (remaining_bytes)
    {

        /* Invalid packet chain.  Calculate the actual number of bytes
           copied.  */
        *bytes_copied =  *bytes_copied - remaining_bytes;

        /* Return an error.  */
        return(NX_INVALID_PACKET);
    }

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

