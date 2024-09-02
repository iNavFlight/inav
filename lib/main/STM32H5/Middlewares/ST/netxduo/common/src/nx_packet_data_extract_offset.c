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
#include "nx_packet.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_packet_data_extract_offset                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function copies data from a NetX packet (or packet chain) into */
/*    the supplied user buffer.   If an empty packet (no data) is         */
/*    received, zero bytes are copied, and the function returns without   */
/*    errors.                                                             */
/*                                                                        */
/*    Note that this function extracts data from a packet into user       */
/*    supplied buffer.  It does not modify packet internal state          */
/*    information.  The data being extracted is still available in the    */
/*    original packet for consumption again.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                        Pointer to the source packet      */
/*    offset                            Offset from start of data         */
/*    buffer_start                      Pointer to destination data area  */
/*    buffer_length                     Size in bytes                     */
/*    bytes_copied                      Number of bytes copied            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                            Completion status                 */
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
UINT  _nx_packet_data_extract_offset(NX_PACKET *packet_ptr, ULONG offset, VOID *buffer_start, ULONG buffer_length, ULONG *bytes_copied)
{

ULONG      remaining_bytes;
UCHAR     *source_ptr;
UCHAR     *destination_ptr;
ULONG      offset_bytes;
#ifndef NX_DISABLE_PACKET_CHAIN
ULONG      packet_fragment_length;
#endif /* NX_DISABLE_PACKET_CHAIN */
ULONG      bytes_to_copy;
NX_PACKET *working_packet_ptr;


    working_packet_ptr =  packet_ptr;

    /* Check for an invalid offset or packet length.  */
    if (offset >= working_packet_ptr -> nx_packet_length)
    {

        /* Note: A zero offset with a packet of zero length is ok. */
        if (offset == 0)
        {

            *bytes_copied = 0;
            return(NX_SUCCESS);
        }

        /* Otherwise, this is an invalid offset or packet length. */
        return(NX_PACKET_OFFSET_ERROR);
    }


    /* Initialize the source pointer to NULL.  */
    source_ptr =  NX_NULL;

    /* Traverse packet chain to offset.  */
    offset_bytes =  offset;
#ifndef NX_DISABLE_PACKET_CHAIN
    while (working_packet_ptr)
    {

        /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
        packet_fragment_length =  (ULONG)((working_packet_ptr -> nx_packet_append_ptr - working_packet_ptr -> nx_packet_prepend_ptr));

        /* Determine if we are at the offset location fragment in the packet chain  */
        if (packet_fragment_length > offset_bytes)
        {

            /* Setup loop to copy from this packet.  */
            source_ptr =  working_packet_ptr -> nx_packet_prepend_ptr + offset_bytes;

            /* Yes, get out of this  loop.  */
            break;
        }


        /* Decrement the remaining offset bytes*/
        offset_bytes = offset_bytes - packet_fragment_length;

        /* Move to next packet.  */
        working_packet_ptr =  working_packet_ptr -> nx_packet_next;
    }
#else /* NX_DISABLE_PACKET_CHAIN */
    /* Setup loop to copy from this packet.  */
    source_ptr =  working_packet_ptr -> nx_packet_prepend_ptr + offset_bytes;

#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Check for a valid source pointer.  */
    if (source_ptr == NX_NULL)
    {
        return(NX_PACKET_OFFSET_ERROR);
    }

    /* Setup the destination pointer.  */
    destination_ptr =  buffer_start;
    bytes_to_copy =   (packet_ptr -> nx_packet_length - offset);

    /* Pickup the amount of bytes to copy.  */
    if (bytes_to_copy < buffer_length)
    {
        *bytes_copied =  bytes_to_copy;     /* the amount of bytes returned to the caller */
        remaining_bytes =  bytes_to_copy;   /* for use in the copy loop */
    }
    else
    {
        *bytes_copied =  buffer_length;
        remaining_bytes =  buffer_length;
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    /* Loop to copy bytes from packet(s).  */
    while (working_packet_ptr && remaining_bytes)
    {
#endif /* NX_DISABLE_PACKET_CHAIN */

        /* Calculate bytes to copy.  */
        /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
        bytes_to_copy = (ULONG)(working_packet_ptr -> nx_packet_append_ptr - source_ptr);
        if (remaining_bytes < bytes_to_copy)
        {
            bytes_to_copy = remaining_bytes;
        }

        /* Copy data from this packet.  */
        memcpy(destination_ptr, source_ptr, bytes_to_copy); /* Use case of memcpy is verified. lgtm[cpp/banned-api-usage-required-any] */

        /* Update the pointers. */
        destination_ptr += bytes_to_copy;
        remaining_bytes -= bytes_to_copy;

#ifndef NX_DISABLE_PACKET_CHAIN
        /* Move to next packet.  */
        working_packet_ptr =  working_packet_ptr -> nx_packet_next;

        /* Check for a next packet.  */
        if (working_packet_ptr)
        {

            /* Setup new source pointer.  */
            source_ptr = working_packet_ptr -> nx_packet_prepend_ptr;
        }
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_PACKET_DATA_EXTRACT_OFFSET, packet_ptr, buffer_length, *bytes_copied, 0, NX_TRACE_PACKET_EVENTS, 0, 0);

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

