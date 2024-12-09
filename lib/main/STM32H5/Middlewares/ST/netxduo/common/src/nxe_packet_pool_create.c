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
/*    _nxe_packet_pool_create                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the packet pool create           */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pool_ptr                              Packet Pool control block     */
/*    name_ptr                              Packet Pool string pointer    */
/*    payload_size                          Size of packet payload        */
/*    pool_start                            Starting address of pool      */
/*    pool_size                             Number of bytes in pool       */
/*    pool_control_block_size               Size of packet pool control   */
/*                                            block                       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_pool_create                Actual packet pool create     */
/*                                            function                    */
/*    tx_thread_identify                    Get current thread pointer    */
/*    tx_thread_preemption_change           Change preemption for thread  */
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
UINT  _nxe_packet_pool_create(NX_PACKET_POOL *pool_ptr, CHAR *name_ptr, ULONG payload_size,
                              VOID *pool_start, ULONG pool_size, UINT pool_control_block_size)
{

UINT            status;
ULONG           rounded_payload_size;
ULONG           rounded_pool_size;
ULONG           header_size;
UINT            old_threshold = 0;
NX_PACKET_POOL *created_pool;
ULONG           created_count;
CHAR           *end_memory;
CHAR           *created_end;
CHAR           *payload_address;
VOID           *rounded_pool_start;
TX_THREAD      *current_thread;


    /* Check for invalid input pointers.  */
    if ((pool_ptr == NX_NULL) || (pool_start == NX_NULL) || (pool_control_block_size != (UINT)sizeof(NX_PACKET_POOL)))
    {
        return(NX_PTR_ERROR);
    }

    /* Align the starting address to four bytes. */
    /*lint -e{923} suppress cast between ULONG and pointer.  */
    rounded_pool_start = (VOID *)((((ALIGN_TYPE)pool_start + NX_PACKET_ALIGNMENT  - 1) / NX_PACKET_ALIGNMENT) * NX_PACKET_ALIGNMENT);

    /* Round the pool size down to something that is evenly divisible by alignment.  */
    /*lint -e{923} suppress cast between ULONG and pointer.  */
    rounded_pool_size = (ULONG)(((pool_size - ((ALIGN_TYPE)rounded_pool_start - (ALIGN_TYPE)pool_start)) / NX_PACKET_ALIGNMENT) * NX_PACKET_ALIGNMENT);

    /* Calculate the address of payload. */
    /*lint -e{923} suppress cast between ULONG and pointer.  */
    payload_address = (CHAR *)((ALIGN_TYPE)rounded_pool_start + sizeof(NX_PACKET));

    /* Align the address of payload. */
    /*lint -e{923} suppress cast between ULONG and pointer.  */
    payload_address = (CHAR *)((((ALIGN_TYPE)payload_address + NX_PACKET_ALIGNMENT  - 1) / NX_PACKET_ALIGNMENT) * NX_PACKET_ALIGNMENT);

    /* Calculate the header size. */
    /*lint -e{923} suppress cast between ULONG and pointer.  */
    header_size = (ULONG)((ALIGN_TYPE)payload_address - (ALIGN_TYPE)rounded_pool_start);

    /* Round the packet size up to something that helps guarantee proper alignment for header and payload.  */
    rounded_payload_size =  (ULONG)(((header_size + payload_size + NX_PACKET_ALIGNMENT  - 1) / NX_PACKET_ALIGNMENT) * NX_PACKET_ALIGNMENT - header_size);

    /* Check for an invalid pool and payload size.  */
    if ((pool_size <= NX_PACKET_ALIGNMENT) || (!payload_size) ||
        ((rounded_payload_size + header_size) > rounded_pool_size))
    {
        return(NX_SIZE_ERROR);
    }

    /* Calculate the end of the pool memory area.  */
    end_memory =  ((CHAR *)pool_start) + (pool_size - 1);

    /* Pickup current thread pointer.  */
    current_thread =  tx_thread_identify();

    /* Disable preemption temporarily.  */
    if (current_thread)
    {
        tx_thread_preemption_change(current_thread, 0, &old_threshold);
    }

    /* Loop to check for the pool instance already created.  */
    created_pool =   _nx_packet_pool_created_ptr;
    created_count =  _nx_packet_pool_created_count;
    while (created_count--)
    {

        /* Calculate the created pool's end of memory.  */
        created_end =  created_pool -> nx_packet_pool_start + (created_pool -> nx_packet_pool_size - 1);

        /* Is the new pool already created?  */
        /*lint -e{946} suppress pointer subtraction, since it is necessary. */
        if ((pool_ptr == created_pool) ||
            ((pool_start >= (VOID *)created_pool -> nx_packet_pool_start) && (pool_start < (VOID *)created_end)) ||
            ((end_memory  >= created_pool -> nx_packet_pool_start) && (end_memory  < created_end)))
        {

            /* Restore preemption.  */
            if (current_thread)
            {

                /*lint -e{644} suppress variable might not be initialized, since "old_threshold" was initialized by previous tx_thread_preemption_change. */
                tx_thread_preemption_change(current_thread, old_threshold, &old_threshold);
            }

            /* Duplicate packet pool created, return an error!  */
            return(NX_PTR_ERROR);
        }

        /* Move to next entry.  */
        created_pool =  created_pool -> nx_packet_pool_created_next;
    }

    /* Restore preemption.  */
    if (current_thread)
    {
        tx_thread_preemption_change(current_thread, old_threshold, &old_threshold);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual packet pool create function.  */
    status =  _nx_packet_pool_create(pool_ptr, name_ptr, payload_size, pool_start, pool_size);

    /* Return completion status.  */
    return(status);
}

