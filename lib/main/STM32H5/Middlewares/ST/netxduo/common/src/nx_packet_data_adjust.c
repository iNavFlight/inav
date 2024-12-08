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
/*    _nx_packet_data_adjust                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adjusts the packet data to fill the specified header. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to the source packet  */
/*    header_size                           The size of header            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_allocate                   Allocate data packet          */
/*    _nx_packet_data_append                Packet data append service    */
/*    _nx_packet_release                    Release data packet           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_forward_packet_process         Forward IP packet             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            verified memmove use cases, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_packet_data_adjust(NX_PACKET *packet_ptr, ULONG header_size)
{

ULONG           available_size;
ULONG           shift_size;
#ifndef NX_DISABLE_PACKET_CHAIN
UINT            status;
ULONG           append_size;
UCHAR          *data_start;
NX_PACKET      *work_ptr;
#endif /* !NX_DISABLE_PACKET_CHAIN  */


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* The header must be filled in one packet. */
    if (((ALIGN_TYPE)packet_ptr -> nx_packet_data_end - (ALIGN_TYPE)packet_ptr -> nx_packet_data_start) < header_size)
    {
        return(NX_UNDERFLOW);
    }

    /* 1. Check if there is enough space to add header.  */
    if (((ALIGN_TYPE)packet_ptr -> nx_packet_prepend_ptr - (ALIGN_TYPE)packet_ptr -> nx_packet_data_start) >= header_size)
    {

        /* Yes. Just return success. */
        return(NX_SUCCESS);
    }

    /* Compute the total avilable size in this packet.  */
    available_size = (ULONG)(((ALIGN_TYPE)packet_ptr -> nx_packet_prepend_ptr - (ALIGN_TYPE)packet_ptr -> nx_packet_data_start) +
                             ((ALIGN_TYPE)packet_ptr -> nx_packet_data_end - (ALIGN_TYPE)packet_ptr -> nx_packet_append_ptr));

    /* 2. Would the header fit into the available space?  */
    if (available_size >= header_size)
    {

        /* Yes, adjust the data.  */

        /* Calculate the shift data size. */
        shift_size = (ULONG)((ALIGN_TYPE)packet_ptr -> nx_packet_append_ptr - (ALIGN_TYPE)packet_ptr -> nx_packet_prepend_ptr);

        /* Move the data, */
        memmove(packet_ptr -> nx_packet_data_start + header_size, packet_ptr -> nx_packet_prepend_ptr, shift_size); /* Use case of memmove is verified.  */

        /* Update the prepend and append pointer.  */
        packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_data_start + header_size;
        packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + shift_size;
    }
    else
    {

#ifndef NX_DISABLE_PACKET_CHAIN

        /* 3. Current packet does not have enough space to fill the header.
              Allocate a new packet to fill the overflowing data and chain the packet.  */

        /* Odd value of header_size is not supported now.
         * In _nx_ip_checksum_compute(), nx_packet_append_ptr must not be odd value. */
        if (header_size & 1)
        {
            return(NX_NOT_SUPPORTED);
        }

        /* Calculate the append data size. */
        append_size = header_size - available_size;

        /* Set the append data pointer.  */
        data_start = packet_ptr -> nx_packet_append_ptr - append_size;

        /* Allocate a packet.  */
        status = _nx_packet_allocate(packet_ptr -> nx_packet_pool_owner, &work_ptr, 0, NX_NO_WAIT);

        /* Check status.  */
        if (status)
        {
            return(status);
        }

        /* Firstly, append the overflowing data to the new packet.. */
        memcpy(work_ptr -> nx_packet_prepend_ptr, data_start, append_size); /* Use case of memcpy is verified.  lgtm[cpp/banned-api-usage-required-any] */
        work_ptr -> nx_packet_append_ptr = (UCHAR *)((ALIGN_TYPE)work_ptr -> nx_packet_prepend_ptr + append_size);

        /* Secondly, calculate the shift data size.  */
        shift_size = (ULONG)(((ALIGN_TYPE)packet_ptr -> nx_packet_append_ptr - (ALIGN_TYPE)packet_ptr -> nx_packet_prepend_ptr) - append_size);

        /* Move the data.  */
        memmove(packet_ptr -> nx_packet_data_start + header_size, packet_ptr -> nx_packet_prepend_ptr, shift_size); /* Use case of memmove is verified.  */

        /* Update the prepend and append pointer.  */
        packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_data_start + header_size;
        packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + shift_size;

        /* At this point, all the necessary packets have been allocated.
           We need to link this new packet to the next of the supplied packet.  */
        work_ptr -> nx_packet_next = packet_ptr -> nx_packet_next;
        packet_ptr -> nx_packet_next = work_ptr;

        /* Update the last packet pointer.  */
        if (packet_ptr -> nx_packet_last == NX_NULL)
        {
            packet_ptr -> nx_packet_last = work_ptr;
        }
#else /* NX_DISABLE_PACKET_CHAIN  */

        /* Return error code.  */
        return(NX_UNDERFLOW);
#endif /* !NX_DISABLE_PACKET_CHAIN  */
    }

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

