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
/**   Internet Protocol version 6 (IPv6)                                  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "nx_api.h"
#include "nx_ipv6.h"


#if defined(FEATURE_NX_IPV6) && !defined(NX_DISABLE_FRAGMENTATION)

/* Define the status 'bits' of the copy flag field. */
#define PACKET_MORE_TO_COPY 1
#define PACKET_ADD_BUFFER   2
#define PACKET_COPY_DONE    4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ipv6_packet_copy                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function copies packet data between packets.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    source_pkt_head             Pointer to source packet chain.         */
/*    dest_pkt_head               Pointer to destination packet chain.    */
/*    size                        Number of bytes to copy.                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                   Successful completion                  */
/*    NX_NOT_SUCCESSFUL            Error with packet copy                 */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ipv6_fragment_process                                           */
/*                                                                        */
/*                                                                        */
/*  NOTE                                                                  */
/*                                                                        */
/*    None                                                                */
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
UINT _nx_ipv6_packet_copy(NX_PACKET *source_pkt_head, NX_PACKET *dest_pkt_head, UINT size)
{

UINT       bytes_remaining;
NX_PACKET *source_pkt, *dest_pkt;
UINT       bytes_to_copy;
ULONG     *source_ptr, *dest_ptr;
UCHAR     *source_byte, *dest_byte;
UINT       flag;


    /* Number of bytes to be copied. */
    bytes_remaining = size;

    /* Obtain points to the source and destination packets. */
    source_pkt = source_pkt_head -> nx_packet_last;
    dest_pkt = dest_pkt_head -> nx_packet_last;

    while (bytes_remaining > 0)
    {

        /* Make sure source or destination packets are valid. */
        if ((source_pkt == NX_NULL) || (dest_pkt == NX_NULL))
        {
            return(NX_NOT_SUCCESSFUL);
        }

        /*
           figure out the amount of bytes we can copy in this iteration.
           At the end of the interation, we shall be able to "close" either the
           source packet or the destination packet.
         */

        bytes_to_copy = bytes_remaining;

        flag = PACKET_COPY_DONE;

        /* Check if the source packet is running out of data. */
        /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
        if (bytes_to_copy > (UINT)(source_pkt -> nx_packet_append_ptr - source_pkt -> nx_packet_prepend_ptr))
        {

            /* It is. Set flag to PACKET_MORE_TO_COPY, indicating that there is more to be copied
               from the following buffer.  */
            /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
            bytes_to_copy = (UINT)(source_pkt -> nx_packet_append_ptr - source_pkt -> nx_packet_prepend_ptr);
            flag = PACKET_MORE_TO_COPY;
        }

        /* Check if the destination packet is running ouf of space.  */
        /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
        if (bytes_to_copy > (UINT)(dest_pkt -> nx_packet_data_end - dest_pkt -> nx_packet_append_ptr))
        {

            /* It is. Set the 2nd bit in the flag to indicate that at the the end of the
               iteration we will need to chain another buffer on the destination packet.*/
            /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
            bytes_to_copy = (UINT)(dest_pkt -> nx_packet_data_end - dest_pkt -> nx_packet_append_ptr);
            flag = PACKET_ADD_BUFFER;
        }

        /* Adjust packet pointers. */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        dest_ptr = (ULONG *)dest_pkt -> nx_packet_append_ptr;
        dest_pkt -> nx_packet_append_ptr += bytes_to_copy;
        dest_pkt -> nx_packet_length -= bytes_to_copy;

        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        source_ptr = (ULONG *)source_pkt -> nx_packet_prepend_ptr;
        source_pkt -> nx_packet_prepend_ptr += bytes_to_copy;

        while (bytes_to_copy)
        {
            /* Loop unrolling: copy 32 bytes in one iteration. */
            switch (bytes_to_copy >> 2)
            {
            default:
                *dest_ptr++ = *source_ptr++;
            /*lint -e{825} suppress fallthrough, since it is necessary.  */ /* fallthrough */
            case 7:
                *dest_ptr++ = *source_ptr++;
            /*lint -e{825} suppress fallthrough, since it is necessary.  */ /* fallthrough */
            case 6:
                *dest_ptr++ = *source_ptr++;
            /*lint -e{825} suppress fallthrough, since it is necessary.  */ /* fallthrough */
            case 5:
                *dest_ptr++ = *source_ptr++;
            /*lint -e{825} suppress fallthrough, since it is necessary.  */ /* fallthrough */
            case 4:
                *dest_ptr++ = *source_ptr++;
            /*lint -e{825} suppress fallthrough, since it is necessary.  */ /* fallthrough */
            case 3:
                *dest_ptr++ = *source_ptr++;
            /*lint -e{825} suppress fallthrough, since it is necessary.  */ /* fallthrough */
            case 2:
                *dest_ptr++ = *source_ptr++;
            /*lint -e{825} suppress fallthrough, since it is necessary.  */ /* fallthrough */
            case 1:
                *dest_ptr++ = *source_ptr++;
            }
            if (bytes_to_copy >= 32)
            {
                bytes_to_copy -= 32;
                bytes_remaining -= 32;
            }
            else
            {

                /* Copy bytes less than 4. */
                /*lint --e{928} suppress cast from pointer to pointer, since it is necessary  */
                source_byte = (UCHAR *)source_ptr;
                dest_byte = (UCHAR *)dest_ptr;
                switch (bytes_to_copy & 3)
                {
                case 3:
                    *dest_byte++ = *source_byte++;
                /*lint -e{825} suppress fallthrough, since it is necessary.  */ /* fallthrough */
                case 2:
                    *dest_byte++ = *source_byte++;
                /*lint -e{825} suppress fallthrough, since it is necessary.  */ /* fallthrough */
                case 1:
                    *dest_byte++ = *source_byte++;
                    break;
                default:
                    break;
                }

                bytes_remaining -= bytes_to_copy;
                bytes_to_copy = 0;
            }
        }

        /* Check if the flag has been set to more data to copy. */
        if (flag & PACKET_MORE_TO_COPY)
        {
            source_pkt_head -> nx_packet_last = source_pkt -> nx_packet_next;
            source_pkt = source_pkt_head -> nx_packet_last;
        }

        /* Check if we need to chain another buffer to the packet chain for more data copy. */
        if (flag & PACKET_ADD_BUFFER)
        {
            dest_pkt_head -> nx_packet_last = dest_pkt -> nx_packet_next;
            dest_pkt = dest_pkt -> nx_packet_next;
        }

        /* Check if we are done. */
        if (flag & PACKET_COPY_DONE)
        {

            /* We are. */
            break;
        }
    }

    return(NX_SUCCESS);
}


#endif /* FEATURE_NX_IPV6 && NX_DISABLE_FRAGMENTATION*/

