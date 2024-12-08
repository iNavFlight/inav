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
/**   Transmission Control Protocol (TCP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_tcp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_deferred_cleanup_check                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for deferred cleanup processing, which is the  */
/*    case for all TCP socket timeout processing. This is done so that    */
/*    mutex protection can be obtained prior to processing the timeout.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_client_bind_cleanup           Bind cleanup                  */
/*    _nx_tcp_connect_cleanup               Connect cleanup               */
/*    _nx_tcp_disconnect_cleanup            Disconnect cleanup            */
/*    _nx_tcp_receive_cleanup               Receive cleanup               */
/*    _nx_tcp_transmit_cleanup              Transmit cleanup              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_thread_entry                   IP helper thread              */
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
VOID  _nx_tcp_deferred_cleanup_check(NX_IP *ip_ptr)
{

ULONG          created_sockets;
ULONG          suspended_threads;
NX_TCP_SOCKET *socket_ptr;
TX_THREAD     *thread_ptr;


    /* Pickup the first socket and the created count.  */
    socket_ptr =       ip_ptr -> nx_ip_tcp_created_sockets_ptr;
    created_sockets =  ip_ptr -> nx_ip_tcp_created_sockets_count;

    /* Loop through all created TCP sockets on the IP instance.  */
    while (created_sockets--)
    {

        /* Check the socket for deferred bind cleanup.  */
        suspended_threads =  socket_ptr -> nx_tcp_socket_bind_suspended_count;
        if (suspended_threads)
        {

            /* Pickup the socket pointer.  */
            thread_ptr =  socket_ptr -> nx_tcp_socket_bind_suspension_list;

            /* Loop through the suspended threads for the bind operation to determine if there
               is a timeout.  */
            do
            {

                /* Determine if this thread has deferred the timeout processing.  */
                if (thread_ptr -> tx_thread_suspend_cleanup == _nx_tcp_cleanup_deferred)
                {

                    /* Yes, call the cleanup routine again!  */
                    _nx_tcp_client_bind_cleanup(thread_ptr NX_CLEANUP_ARGUMENT);
                }

                /* Move to next suspended thread.  */
                thread_ptr =  thread_ptr -> tx_thread_suspended_next;
            } while (--suspended_threads);
        }

        /* Check the socket for deferred connect cleanup.  */
        thread_ptr =  socket_ptr -> nx_tcp_socket_connect_suspended_thread;
        if (thread_ptr)
        {

            /* Determine if this thread has deferred the timeout processing.  */
            if (thread_ptr -> tx_thread_suspend_cleanup == _nx_tcp_cleanup_deferred)
            {

                /* Yes, call the cleanup routine again!  */
                _nx_tcp_connect_cleanup(thread_ptr NX_CLEANUP_ARGUMENT);
            }
        }

        /* Check the socket for deferred disconnect cleanup.  */
        thread_ptr =  socket_ptr -> nx_tcp_socket_disconnect_suspended_thread;
        if (thread_ptr)
        {

            /* Determine if this thread has deferred the timeout processing.  */
            if (thread_ptr -> tx_thread_suspend_cleanup == _nx_tcp_cleanup_deferred)
            {

                /* Yes, call the cleanup routine again!  */
                _nx_tcp_disconnect_cleanup(thread_ptr NX_CLEANUP_ARGUMENT);
            }
        }

        /* Check the socket for deferred receive cleanup.  */
        suspended_threads =  socket_ptr -> nx_tcp_socket_receive_suspended_count;
        if (suspended_threads)
        {

            /* Pickup the socket pointer.  */
            thread_ptr =  socket_ptr -> nx_tcp_socket_receive_suspension_list;

            /* Loop through the suspended threads for the receive operation to determine if there
               is a timeout.  */
            do
            {

                /* Determine if this thread has deferred the timeout processing.  */
                if (thread_ptr -> tx_thread_suspend_cleanup == _nx_tcp_cleanup_deferred)
                {

                    /* Yes, call the cleanup routine again!  */
                    _nx_tcp_receive_cleanup(thread_ptr NX_CLEANUP_ARGUMENT);
                }

                /* Move to next suspended thread.  */
                thread_ptr =  thread_ptr -> tx_thread_suspended_next;
            } while (--suspended_threads);
        }

        /* Check the socket for deferred transmit cleanup.  */
        suspended_threads =  socket_ptr -> nx_tcp_socket_transmit_suspended_count;
        if (suspended_threads)
        {

            /* Pickup the socket pointer.  */
            thread_ptr =  socket_ptr -> nx_tcp_socket_transmit_suspension_list;

            /* Loop through the suspended threads for the transmit operation to determine if there
               is a timeout.  */
            do
            {

                /* Determine if this thread has deferred the timeout processing.  */
                if (thread_ptr -> tx_thread_suspend_cleanup == _nx_tcp_cleanup_deferred)
                {

                    /* Yes, call the cleanup routine again!  */
                    _nx_tcp_transmit_cleanup(thread_ptr NX_CLEANUP_ARGUMENT);
                }

                /* Move to next suspended thread.  */
                thread_ptr =  thread_ptr -> tx_thread_suspended_next;
            } while (--suspended_threads);
        }

        /* Move to next created TCP socket.  */
        socket_ptr =  socket_ptr -> nx_tcp_socket_created_next;
    }
}

