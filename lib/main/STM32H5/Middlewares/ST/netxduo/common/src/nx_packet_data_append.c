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
/*    _nx_packet_data_append                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function copies the specified data to the end of the specified */
/*    packet.  Additional packets are allocated from the specified pool   */
/*    if needed.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to packet to append to*/
/*    data_start                            Pointer to start of the data  */
/*    data_size                             Number of bytes to append     */
/*    pool_ptr                              Pool to allocate packet from  */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_allocate                   Allocate data packet          */
/*    _nx_packet_release                    Release data packet           */
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
UINT  _nx_packet_data_append(NX_PACKET *packet_ptr, VOID *data_start, ULONG data_size,
                             NX_PACKET_POOL *pool_ptr, ULONG wait_option)
{

#ifndef NX_DISABLE_PACKET_CHAIN
UINT       status;                 /* Return status              */
NX_PACKET *new_list_ptr;           /* Head of new list pointer   */
NX_PACKET *last_packet =  NX_NULL; /* Last supplied packet       */
#endif /* NX_DISABLE_PACKET_CHAIN */
ULONG      available_bytes;        /* Number of available bytes  */
ULONG      copy_size;              /* Size for each memory copy  */
UCHAR     *source_ptr;             /* Buffer source pointer      */
NX_PACKET *work_ptr;               /* Working packet pointer     */


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_PACKET_DATA_APPEND, packet_ptr, data_start, data_size, pool_ptr, NX_TRACE_PACKET_EVENTS, 0, 0);

#ifndef NX_DISABLE_PACKET_CHAIN
    /* Calculate the number of bytes available at the end of the supplied packet.  */
    if (packet_ptr -> nx_packet_last)
    {

        /* More than one packet.  Walk the packet chain starting at the last packet
           to calculate the remaining bytes.  */
        available_bytes =  0;
        work_ptr =  packet_ptr -> nx_packet_last;
        do
        {

            /* Calculate the available bytes in this packet.  */
            /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
            /*lint -e{737} suppress loss of sign, since nx_packet_data_end is assumed to be larger than nx_packet_append_ptr. */
            available_bytes =  available_bytes +
                (ULONG)(work_ptr -> nx_packet_data_end - work_ptr -> nx_packet_append_ptr);

            /* Remember the last packet.  */
            last_packet =  work_ptr;

            /* Move to the next packet.   There typically won't be another packet, but just in
               case the logic is here for it!  */
            work_ptr =  work_ptr -> nx_packet_next;
        } while (work_ptr);
    }
    else
#endif /* NX_DISABLE_PACKET_CHAIN */
    {

        /* Just calculate the number of bytes available in the first packet.  */
        /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
        available_bytes =  (ULONG)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_append_ptr);
    }

    /* Determine if any new packets are required to satisfy this request. */
    if (available_bytes < data_size)
    {

#ifndef NX_DISABLE_PACKET_CHAIN
        /* Setup a temporary head pointer.  */
        new_list_ptr =  NX_NULL;

        /* Loop to pickup enough packets to complete the append request.  */
        while (available_bytes < data_size)
        {

            /* Allocate a new packet.  */
            status =  _nx_packet_allocate(pool_ptr, &work_ptr, 0, wait_option);

            /* Determine if an error is present.  */
            if (status)
            {

                /* Yes, an error is present.   */

                /* First release any packets that have been allocated so far.  */
                if (new_list_ptr)
                {
                    _nx_packet_release(new_list_ptr);
                }

                /* Return the error status to the caller of this service.  */
                return(status);
            }

            /* Add debug information. */
            NX_PACKET_DEBUG(__FILE__, __LINE__, work_ptr);

            /* No error is present.  Link the new packet to the temporary list being built.  */
            if (new_list_ptr)
            {

                /* Determine if there is already more than one packet on the list.  */
                if (new_list_ptr -> nx_packet_last)
                {

                    /* Yes, link up the last packet to the new packet and update the
                       last pointer.  */
                    /*lint -e{644} suppress variable might not be initialized, since "work_ptr" was initialized in _nx_packet_allocate. */
                    (new_list_ptr -> nx_packet_last) -> nx_packet_next =  work_ptr;
                    new_list_ptr -> nx_packet_last =  work_ptr;
                }
                else
                {

                    /* Second packet allocated.  Just setup the last and next in the
                       head pointer.  */
                    new_list_ptr -> nx_packet_last =  work_ptr;
                    new_list_ptr -> nx_packet_next =  work_ptr;
                }
            }
            else
            {

                /* Just setup the temporary list head.  */
                new_list_ptr =  work_ptr;
            }

            /* Adjust the number of available bytes according to how much space
               is in the new packet.  */
            /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
            /*lint -e{737} suppress loss of sign, since nx_packet_data_end is assumed to be larger than nx_packet_append_ptr. */
            /*lint -e{613} suppress possible use of null pointer, since "work_ptr" was set in _nx_packet_allocate. */
            available_bytes =  available_bytes +
                (ULONG)(work_ptr -> nx_packet_data_end - work_ptr -> nx_packet_append_ptr);
        }

        /* At this point, all the necessary packets have been allocated and are present
           on the temporary list.  We need to link this new list to the end of the supplied
           packet.  */
        if (last_packet)
        {

            /* Already more than one packet.  Add the new packet list to the end.  */
            last_packet -> nx_packet_next =  new_list_ptr;
        }
        else
        {

            /* Link the new packet list to the head packet.  */
            packet_ptr -> nx_packet_next =  new_list_ptr;
        }

        /* Clear the last packet that was used to maintain the new list.  */
        /*lint -e{613} suppress possible use of null pointer, since "new_list_ptr" was set in previous loop. */
        new_list_ptr -> nx_packet_last =  NX_NULL;
#else
        NX_PARAMETER_NOT_USED(pool_ptr);
        NX_PARAMETER_NOT_USED(wait_option);

        return(NX_SIZE_ERROR);
#endif /* NX_DISABLE_PACKET_CHAIN */
    }

    /* Setup the new data length in the packet.  */
    packet_ptr -> nx_packet_length =   packet_ptr -> nx_packet_length + data_size;

    /* Now copy the supplied data buffer at the end of the packet.  */
    source_ptr =  (UCHAR *)data_start;
#ifndef NX_DISABLE_PACKET_CHAIN
    if (packet_ptr -> nx_packet_last)
    {
        work_ptr =    packet_ptr -> nx_packet_last;
    }
    else
    {
#endif /* NX_DISABLE_PACKET_CHAIN */
        work_ptr =    packet_ptr;
#ifndef NX_DISABLE_PACKET_CHAIN
    }
    while (data_size)
    {

        /* Determine the amount of memory to copy.  */
        /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
        if (data_size < (ULONG)(work_ptr -> nx_packet_data_end - work_ptr -> nx_packet_append_ptr))
        {
            copy_size =  data_size;
        }
        else
        {

            /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
            copy_size =  (ULONG)(work_ptr -> nx_packet_data_end - work_ptr -> nx_packet_append_ptr);
        }
#else
        copy_size = data_size;
#endif /* NX_DISABLE_PACKET_CHAIN */

        /* Copy the data into the current packet buffer.  */
        memcpy(work_ptr -> nx_packet_append_ptr, source_ptr, copy_size); /* Use case of memcpy is verified.  lgtm[cpp/banned-api-usage-required-any] */

        /* Adjust the remaining data size.  */
        data_size =  data_size - copy_size;

        /* Update this packets append pointer.  */
        work_ptr -> nx_packet_append_ptr =  work_ptr -> nx_packet_append_ptr + copy_size;

#ifndef NX_DISABLE_PACKET_CHAIN
        /* Any more data left to append?  */
        if (data_size)
        {

            /* Yes, there is more to move.  Update the source pointer, move the work pointer
               to the next packet in the chain and update the last packet pointer.  */
            source_ptr =  source_ptr + copy_size;
            work_ptr =  work_ptr -> nx_packet_next;
            packet_ptr -> nx_packet_last =  work_ptr;
        }
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Return successful status.  */
    return(NX_SUCCESS);
}

