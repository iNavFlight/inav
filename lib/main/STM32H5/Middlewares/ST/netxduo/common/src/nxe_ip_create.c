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
#include "nx_packet.h"

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_ip_create                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the IP instance create           */
/*    function call.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    name                                  Name of this IP instance      */
/*    ip_address                            Internet address for this IP  */
/*    network_mask                          Network mask for IP address   */
/*    default_pool                          Default packet pool           */
/*    ip_link_driver                        User supplied IP link driver  */
/*    memory_ptr                            Pointer memory area for IP    */
/*    memory_size                           Size of IP memory area        */
/*    priority                              Priority of IP helper thread  */
/*    ip_control_block_size                 Size of NX_IP structure       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_create                         Actual IP instance create     */
/*                                            function                    */
/*    tx_thread_identify                    Get current thread pointer    */
/*    tx_thread_preemption_change           Change preemption for thread  */
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
UINT  _nxe_ip_create(NX_IP *ip_ptr, CHAR *name, ULONG ip_address, ULONG network_mask,
                     NX_PACKET_POOL *default_pool, VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT *),
                     VOID *memory_ptr, ULONG memory_size, UINT priority, UINT ip_control_block_size)
{

UINT       status;
UINT       old_threshold = 0;
NX_IP     *created_ip;
ULONG      created_count;
UCHAR     *end_stack;
TX_THREAD *current_thread;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (default_pool == NX_NULL) ||
        (default_pool -> nx_packet_pool_id != NX_PACKET_POOL_ID) || (ip_link_driver == NX_NULL) ||
        (memory_ptr == NX_NULL) || (ip_control_block_size != (UINT)sizeof(NX_IP)))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for a memory size error.  */
    if (memory_size < TX_MINIMUM_STACK)
    {
        return(NX_SIZE_ERROR);
    }

    /* Check the priority specified.  */
    if (priority >= TX_MAX_PRIORITIES)
    {
        return(NX_OPTION_ERROR);
    }

    /* Calculate the end of the stack area.  */
    end_stack =  ((UCHAR *)memory_ptr) + (memory_size - 1);

    /* Pickup current thread pointer.  */
    current_thread =  tx_thread_identify();

    /* Disable preemption temporarily.  */
    if (current_thread)
    {
        tx_thread_preemption_change(current_thread, 0, &old_threshold);
    }

    /* Loop to check for the IP instance already created.  */
    created_ip =     _nx_ip_created_ptr;
    created_count =  _nx_ip_created_count;
    while (created_count--)
    {

        /* Is the new ip already created?  */
        /*lint -e{946} suppress pointer subtraction, since it is necessary. */
        if ((ip_ptr == created_ip) ||
            ((memory_ptr >= created_ip -> nx_ip_thread.tx_thread_stack_start) && (memory_ptr < created_ip -> nx_ip_thread.tx_thread_stack_end)) ||
            ((((VOID *)end_stack)  >= created_ip -> nx_ip_thread.tx_thread_stack_start) && (((VOID *)end_stack)  < created_ip -> nx_ip_thread.tx_thread_stack_end)))
        {

            /* Restore preemption.  */
            if (current_thread)
            {

                /*lint -e{644} suppress variable might not be initialized, since "old_threshold" was initialized by previous tx_thread_preemption_change. */
                tx_thread_preemption_change(current_thread, old_threshold, &old_threshold);
            }

            /* Duplicate ip created, return an error!  */
            return(NX_PTR_ERROR);
        }

        /* Move to next entry.  */
        created_ip =  created_ip -> nx_ip_created_next;
    }

    /* Restore preemption.  */
    if (current_thread)
    {

        /*lint -e{644} suppress variable might not be initialized, since "old_threshold" was initialized by previous tx_thread_preemption_change. */
        tx_thread_preemption_change(current_thread, old_threshold, &old_threshold);
    }

    /* Check for invalid IP address.  Note that Interface with DHCP enabled
       would start with 0.0.0.0.  Therefore the 0 IP address is allowed. */
    if ((ip_address != 0) &&
        ((ip_address & NX_IP_CLASS_A_MASK) != NX_IP_CLASS_A_TYPE) &&
        ((ip_address & NX_IP_CLASS_B_MASK) != NX_IP_CLASS_B_TYPE) &&
        ((ip_address & NX_IP_CLASS_C_MASK) != NX_IP_CLASS_C_TYPE))
    {
        return(NX_IP_ADDRESS_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual IP instance create function.  */
    status =  _nx_ip_create(ip_ptr, name, ip_address, network_mask, default_pool, ip_link_driver,
                            memory_ptr, memory_size, priority);

    /* Return completion status.  */
    return(status);
}

