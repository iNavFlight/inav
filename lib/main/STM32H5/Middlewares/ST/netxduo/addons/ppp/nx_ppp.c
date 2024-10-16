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
/**   Point-to-Point Protocol (PPP)                                       */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_PPP_SOURCE_CODE


/* Force error checking to be disabled in this module */
#include "tx_port.h"

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

#ifndef TX_SAFETY_CRITICAL
#ifndef TX_DISABLE_ERROR_CHECKING
#define TX_DISABLE_ERROR_CHECKING
#endif
#endif


/* Include necessary system files.  */

#include "nx_api.h"
#ifndef NX_DISABLE_IPV4
#include "nx_ppp.h"
#ifndef NX_PPP_DISABLE_CHAP
#include "nx_md5.h"
#endif

/* Define global PPP variables and data structures.  */

/* Define the PPP created list head pointer and count.  */

NX_PPP  *_nx_ppp_created_ptr =       NX_NULL;
ULONG    _nx_ppp_created_count =     0;


/* Define the CRC lookup table.  This is used to improve the 
   CRC calculation performance.  */

const USHORT _nx_ppp_crc_table[256] = 
{
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};


/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_thread_entry                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function handles all PPP processing, including assembly of     */ 
/*    PPP requests and dispatching them.                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_addr                              Address of PPP instance       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_get                    Get PPP event flags           */ 
/*    nx_packet_release                     Release packet                */ 
/*    _nx_ppp_chap_state_machine_update     Update CHAP state machine     */ 
/*    _nx_ppp_lcp_state_machine_update      Update LCP state machine      */ 
/*    _nx_ppp_process_deferred_ip_packet_send                             */ 
/*                                          Process deferred IP packet    */ 
/*                                            send                        */ 
/*    _nx_ppp_process_deferred_raw_string_send                            */ 
/*                                          Process deferred raw string   */ 
/*                                            send                        */ 
/*    _nx_ppp_receive_packet_get            Get PPP receive packet        */ 
/*    _nx_ppp_receive_packet_process        Process received PPP packet   */ 
/*    _nx_ppp_timeout                       Process PPP timeout           */ 
/*    [_nx_ppp_debug_log_capture]           Optional PPP debug log        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX                                                             */ 
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
void _nx_ppp_thread_entry(ULONG ppp_addr)
{

TX_INTERRUPT_SAVE_AREA

NX_PPP      *ppp_ptr;
ULONG       ppp_events;
NX_PACKET   *packet_ptr;
NX_PACKET   *next_packet_ptr;
ULONG       count;


    /* Setup the PPP pointer.  */
    ppp_ptr =  (NX_PPP *) ppp_addr;

    /* Loop to continue processing incoming bytes.  */
    while(NX_FOREVER)    
    {


#ifdef NX_PPP_DEBUG_LOG_PRINT_PROTOCOL
        /* Display debug output of PPP protocol status.   */
        _nx_ppp_debug_log_capture_protocol(ppp_ptr);
#endif

        /* Wait for PPP event(s). The timeout on the wait will drive PPP timeout processing
           as well.  */
        tx_event_flags_get(&(ppp_ptr -> nx_ppp_event), (ULONG) 0xFFFFFFFF, TX_OR_CLEAR, &ppp_events, NX_WAIT_FOREVER);

        /* Check for PPP stop event.  */
        if (ppp_events & NX_PPP_EVENT_STOP)
        {

            /* Is PPP already stopped?  */
            if (ppp_ptr -> nx_ppp_state != NX_PPP_STOPPED)
            {
            
                /* No, stop this PPP instance an prepare for the next start operation.  */
                
                /* Set state to stopped.  */
                ppp_ptr -> nx_ppp_state =  NX_PPP_STOPPED;
                
                /* Clean up resources and prepare for next PPP start.  */

                /* Release any packets queued up for the PPP instance.  */

                /* Release any partial receive packets.  */
                if (ppp_ptr -> nx_ppp_receive_partial_packet)
                    nx_packet_release(ppp_ptr -> nx_ppp_receive_partial_packet);

                /* Clear the saved receive packet.  */
                ppp_ptr -> nx_ppp_receive_partial_packet =  NX_NULL;
                                
                /* Setup the receive buffer processing.  */
                ppp_ptr -> nx_ppp_serial_buffer_write_index =  0;
                ppp_ptr -> nx_ppp_serial_buffer_read_index =   0;
                ppp_ptr -> nx_ppp_serial_buffer_byte_count =   0;

#ifdef NX_PPP_PPPOE_ENABLE

                /* Loop to release all packets in the deferred processing queue..  */
                while (ppp_ptr -> nx_ppp_deferred_received_packet_head)
                {

                    /* Restore interrupts.  */
                    TX_RESTORE

                    /* Pickup the packet.  */
                    packet_ptr =  ppp_ptr -> nx_ppp_deferred_received_packet_head;

                    /* Move the head pointer to the next packet.  */
                    ppp_ptr -> nx_ppp_deferred_received_packet_head =  packet_ptr -> nx_packet_queue_next;

                    /* Check for end of deferred processing queue.  */
                    if (ppp_ptr -> nx_ppp_deferred_received_packet_head == NX_NULL)
                    {

                        /* Yes, the queue is empty.  Set the tail pointer to NULL.  */
                        ppp_ptr -> nx_ppp_deferred_received_packet_tail =  NX_NULL;
                    }

                    /* Release all packets in the raw packet queue.  */ 
                    TX_DISABLE

                    /* Release packet.  */
                    nx_packet_release(packet_ptr);
                }
#endif /* NX_PPP_PPPOE_ENABLE  */

                /* Release all packets in the IP packet queue.  */ 
                TX_DISABLE
                
                /* Pickup the head pointer and count.  */
                packet_ptr =  ppp_ptr -> nx_ppp_ip_packet_queue_head;
                
                /* Set the list head and tail pointers to NULL.  */
                ppp_ptr -> nx_ppp_ip_packet_queue_head =  NX_NULL;
                ppp_ptr -> nx_ppp_ip_packet_queue_tail =  NX_NULL;                
                
                /* Pickup the count.  */
                count =  ppp_ptr -> nx_ppp_ip_packet_queue_count;

                /* Clear the count.  */
                ppp_ptr -> nx_ppp_ip_packet_queue_count =  0;
                
                /* Restore interrupts.  */
                TX_RESTORE
                
                /* Loop to release all packets.  */
                while (count)
                {
                    /* Pickup next packet.  */
                    next_packet_ptr =  packet_ptr -> nx_packet_queue_next;
                    
                    /* Release packet.  */
                    nx_packet_release(packet_ptr);
                    
                    /* Move to next packet.  */
                    packet_ptr =  next_packet_ptr;
                    
                    /* Decrement the count.  */
                    count--;
                }
              
                /* Release all packets in the raw packet queue.  */ 
                TX_DISABLE
                
                /* Pickup the head pointer and count.  */
                packet_ptr =  ppp_ptr -> nx_ppp_raw_packet_queue_head;
                
                /* Set the list head and tail pointers to NULL.  */
                ppp_ptr -> nx_ppp_raw_packet_queue_head =  NX_NULL;
                ppp_ptr -> nx_ppp_raw_packet_queue_tail =  NX_NULL;                
                
                /* Pickup the count.  */
                count =  ppp_ptr -> nx_ppp_raw_packet_queue_count;

                /* Clear the count.  */
                ppp_ptr -> nx_ppp_raw_packet_queue_count =  0;
                
                /* Restore interrupts.  */
                TX_RESTORE
                
                /* Loop to release all packets.  */
                while (count)
                {
                    /* Pickup next packet.  */
                    next_packet_ptr =  packet_ptr -> nx_packet_queue_next;
                    
                    /* Release packet.  */
                    nx_packet_release(packet_ptr);
                    
                    /* Move to next packet.  */
                    packet_ptr =  next_packet_ptr;
                    
                    /* Decrement the count.  */
                    count--;
                }

                /* Reset all state machines.  */
                ppp_ptr -> nx_ppp_lcp_state =   NX_PPP_LCP_INITIAL_STATE;
                ppp_ptr -> nx_ppp_pap_state =   NX_PPP_PAP_INITIAL_STATE;
                ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_INITIAL_STATE;
                ppp_ptr -> nx_ppp_ipcp_state =  NX_PPP_IPCP_INITIAL_STATE;

                /* Determine how to setup the initial authenticated field.  */
                if ((ppp_ptr -> nx_ppp_pap_verify_login) || 
                    (ppp_ptr -> nx_ppp_pap_generate_login) || 
                    (ppp_ptr -> nx_ppp_chap_get_verification_values))
                {
                    ppp_ptr -> nx_ppp_authenticated =  NX_FALSE;
                }
                else
                    ppp_ptr -> nx_ppp_authenticated =  NX_TRUE;

                /* Clear the IP address.  */
                nx_ip_interface_address_set(ppp_ptr -> nx_ppp_ip_ptr, ppp_ptr -> nx_ppp_interface_index, 0, 0);

                /* Clear the local information for PPP client.  */
                if (ppp_ptr -> nx_ppp_server == NX_FALSE)
                {

                    /* Clear the IP addresses.  */
                    ppp_ptr -> nx_ppp_ipcp_local_ip[0] = 0;
                    ppp_ptr -> nx_ppp_ipcp_local_ip[1] = 0;
                    ppp_ptr -> nx_ppp_ipcp_local_ip[2] = 0;
                    ppp_ptr -> nx_ppp_ipcp_local_ip[3] = 0;
                    ppp_ptr -> nx_ppp_ipcp_peer_ip[0] = 0;
                    ppp_ptr -> nx_ppp_ipcp_peer_ip[1] = 0;
                    ppp_ptr -> nx_ppp_ipcp_peer_ip[2] = 0;
                    ppp_ptr -> nx_ppp_ipcp_peer_ip[3] = 0;

                    /* Clear the DNS addresses.  */
                    ppp_ptr -> nx_ppp_primary_dns_address =  0;
                    ppp_ptr -> nx_ppp_secondary_dns_address =  0;
                }
            }
        }
        
        /* Check for PPP start event.  */
        if (ppp_events & NX_PPP_EVENT_START)
        {
        
            /* Is PPP in a stopped state?  */
            if (ppp_ptr -> nx_ppp_state == NX_PPP_STOPPED)
            {

                /* Update the PPP state.  */
                ppp_ptr -> nx_ppp_state =  NX_PPP_STARTED;
                
                /* Yes, move to the initial LCP state.  */
                ppp_ptr -> nx_ppp_lcp_state =  NX_PPP_LCP_START_STATE;

                /* Initiate the PPP protocol.  */
                _nx_ppp_lcp_state_machine_update(ppp_ptr, NX_NULL);
            }
        }

        /* Check for PPP raw packet send request.  */
        if (ppp_events & NX_PPP_EVENT_RAW_STRING_SEND)
        {

            /* Process deferred raw string send requests.  */
            _nx_ppp_process_deferred_raw_string_send(ppp_ptr);
        }

        /* Check for deferred IP transmit requests.  */
        if (ppp_events & NX_PPP_EVENT_IP_PACKET_SEND)
        {

            /* Process deferred IP transmit packets.  */
            _nx_ppp_process_deferred_ip_packet_send(ppp_ptr);
        }

#ifndef NX_PPP_DISABLE_CHAP

        /* Check for CHAP challenge event.  */
        if (ppp_events & NX_PPP_EVENT_CHAP_CHALLENGE)
        {

            /* Determine if CHAP is in a completed state.  */
            if (ppp_ptr -> nx_ppp_chap_state == NX_PPP_CHAP_COMPLETED_STATE)
            {
            
                /* Move the state to the new challenge state.  */
                ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_COMPLETED_NEW_STATE;
                
                /* Now update the CHAP state machine in order to force the 
                   new CHAP challenge out.  */
                _nx_ppp_chap_state_machine_update(ppp_ptr, NX_NULL);                
            }
        }
#endif

        /* Now check for timeout processing.  */
        if (ppp_events & NX_PPP_EVENT_TIMEOUT)
        {

            /* Timeout occurred. Is there are PPP timeout active?   */
            if (ppp_ptr -> nx_ppp_timeout)
            {
                
                /* Decrement the timeout count.  */
                ppp_ptr -> nx_ppp_timeout--;
                
                /* Has the timeout expired?  */
                if (ppp_ptr -> nx_ppp_timeout == 0)
                {
                
                    /* Process timeout request.  */
                    _nx_ppp_timeout(ppp_ptr);
                }
            }
            
            /* Increment the receive timeout processing.   */
            ppp_ptr -> nx_ppp_receive_timeouts++;
        }

        /* Check for PPP Packet receive event.  */
        if (ppp_events & NX_PPP_EVENT_PACKET_RECEIVE)
        { 

            /* Pickup the next PPP packet from serial port to process. This is called whether an event was set or not
               simply to handle the case when a non-PPP frame is received.  */
            _nx_ppp_receive_packet_get(ppp_ptr, &packet_ptr);

            /* Now determine if there is a packet to process.  */
            if (packet_ptr)
            {

#ifdef NX_PPP_DEBUG_LOG_ENABLE

                /* Insert an entry into the PPP frame debug log.  */
                _nx_ppp_debug_log_capture(ppp_ptr, 'R', packet_ptr);
#endif

                /* Yes, call the PPP packet processing routine.  */
                _nx_ppp_receive_packet_process(ppp_ptr, packet_ptr);
            }
        }

#ifdef NX_PPP_PPPOE_ENABLE

        /* Check for PPP Packet receive event.  */
        if (ppp_events & NX_PPP_EVENT_PPPOE_PACKET_RECEIVE)
        {

            /* Loop to process all deferred packet requests.  */
            while (ppp_ptr -> nx_ppp_deferred_received_packet_head)
            {

                /* Remove the first packet and process it!  */

                /* Disable interrupts.  */
                TX_DISABLE

                /* Pickup the first packet.  */
                packet_ptr =  ppp_ptr -> nx_ppp_deferred_received_packet_head;

                /* Move the head pointer to the next packet.  */
                ppp_ptr -> nx_ppp_deferred_received_packet_head =  packet_ptr -> nx_packet_queue_next;

                /* Check for end of deferred processing queue.  */
                if (ppp_ptr -> nx_ppp_deferred_received_packet_head == NX_NULL)
                {

                    /* Yes, the queue is empty.  Set the tail pointer to NULL.  */
                    ppp_ptr -> nx_ppp_deferred_received_packet_tail =  NX_NULL;
                }       

                /* Restore interrupts.  */
                TX_RESTORE

#ifdef NX_PPP_DEBUG_LOG_ENABLE

                /* Insert an entry into the PPP frame debug log.  */
                _nx_ppp_debug_log_capture(ppp_ptr, 'R', packet_ptr);
#endif

                /* Yes, call the PPP packet processing routine.  */
                _nx_ppp_receive_packet_process(ppp_ptr, packet_ptr);
            }
        }
#endif /* NX_PPP_PPPOE_ENABLE  */
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_driver                                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function provides the basic interface to the NetX TCP/IP       */ 
/*    network stack.                                                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    driver_req_ptr                        NetX driver request structure */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_transmit_release            Release transmit packet       */ 
/*    tx_event_flags_set                    Set PPP events                */ 
/*    tx_thread_resume                      Resume PPP thread             */ 
/*    tx_timer_activate                     Activate PPP timer            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    NetX                                                                */ 
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
void  _nx_ppp_driver(NX_IP_DRIVER *driver_req_ptr)
{

TX_INTERRUPT_SAVE_AREA

NX_IP           *ip_ptr;
NX_INTERFACE    *interface_ptr;
NX_PPP          *ppp_ptr;
NX_PACKET       *packet_ptr;
UINT            i;


    /* Setup the IP pointer from the driver request.  */
    ip_ptr =  driver_req_ptr -> nx_ip_driver_ptr;

    /* Pickup the interface pointer.  */
    interface_ptr =  driver_req_ptr -> nx_ip_driver_interface;

    /* Default to successful return.  */
    driver_req_ptr -> nx_ip_driver_status =  NX_SUCCESS;

    /* Process according to the driver request type in the driver control 
       block.  */
    switch (driver_req_ptr -> nx_ip_driver_command)
    {

        case NX_LINK_INTERFACE_ATTACH:
        {
        
            /* First, we need to find the PPP instance that is mapped to 
               this IP instance and that doesn't have its interface attach
               complete yet. This requires that PPP is created prior to 
               IP creation.  */
            ppp_ptr =  _nx_ppp_created_ptr; 
            i =  0;
            while (i < _nx_ppp_created_count)
            {

                /* Does this PPP instance point to the current IP instance?  */
                if ((ppp_ptr -> nx_ppp_ip_ptr == ip_ptr) && (ppp_ptr -> nx_ppp_interface_ptr == NX_NULL))
                {

                    /* Yes, we found an un-attached PPP instance for this IP interface instance. Get out of
                       the search loop.  */ 
                    break;
                }

                /* Move the next PPP instance.  */
                ppp_ptr =  ppp_ptr -> nx_ppp_created_next;
                
                /* Increment counter. */
                i++;
            }

            /* Determine if a PPP instance was found.  */
            if (i >= _nx_ppp_created_count)
            {

                /* Return an error to NetX.  */
                driver_req_ptr -> nx_ip_driver_status =  NX_IP_INTERNAL_ERROR;
                return;
            }
            
            /* Now find interface index.  */
            i =  0;
            while (i < NX_MAX_PHYSICAL_INTERFACES)
            {
            
                /* Is this the interface index?  */
                if (&(ip_ptr -> nx_ip_interface[i]) == interface_ptr)
                {
                
                    /* Found the index, get out of the loop.  */
                    break;
                }                        
                
                /* Move to next interface.  */
                i++;
            }
            
            /* Determine if the interface index was found.  */
            if (i >= NX_MAX_PHYSICAL_INTERFACES)
            {

                /* Return an error to NetX.  */
                driver_req_ptr -> nx_ip_driver_status =  NX_IP_INTERNAL_ERROR;
                return;
            }
            
            /* Otherwise, we have everything we need to perform the attachment.  */
                                   
            /* Remember the PPP instance pointer in the IP interface pointer.  */
            interface_ptr -> nx_interface_additional_link_info = (void *) ppp_ptr;

            /* Remember the interface pointer and index in the PPP structure.  */
            ppp_ptr -> nx_ppp_interface_ptr =    interface_ptr;
            ppp_ptr -> nx_ppp_interface_index =  i;
            break;
        }

        case NX_LINK_INITIALIZE:
        {

            /* Process driver initialization.  */


            /* Otherwise, we have found the PPP instance, continue initialization.  */

            /* Pickup the associated PPP instance address.  */
            ppp_ptr =  (NX_PPP *)  interface_ptr-> nx_interface_additional_link_info;

            /* Setup the link maximum transfer unit.  */
            interface_ptr -> nx_interface_ip_mtu_size =  NX_PPP_MRU;

            /* Clear the hardware physical address, since there is no 
               such thing in PPP.  */
            interface_ptr -> nx_interface_physical_address_msw =  0;
            interface_ptr -> nx_interface_physical_address_lsw =  0;

            /* Indicate to the IP software that IP to physical mapping
               is not required in PPP.  */
            interface_ptr -> nx_interface_address_mapping_needed =  NX_FALSE;

            /* Set the link up to false. */            
            interface_ptr -> nx_interface_link_up =  NX_FALSE;

            /* Resume the PPP thread.  */
            tx_thread_resume(&(ppp_ptr -> nx_ppp_thread));

            /* Activate the PPP timer.  */
            tx_timer_activate(&(ppp_ptr -> nx_ppp_timer));
            break;
        }

   
        case NX_LINK_ENABLE:
        {

            /* Pickup the associated PPP instance address.  */
            ppp_ptr =  (NX_PPP *)  interface_ptr-> nx_interface_additional_link_info;

            /* Now set event flag to start PPP.  */
            tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_START, TX_OR);
            break;
        }

        case NX_LINK_DISABLE:
        {

            /* Process driver link disable.  */             

            /* Pickup the associated PPP instance address.  */
            ppp_ptr =  (NX_PPP*)  interface_ptr-> nx_interface_additional_link_info;

            /* Now set event flag to start PPP.  */
            tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_STOP, TX_OR);
            break;
        }

        case NX_LINK_PACKET_SEND:
        {

            /* Pickup the associated PPP instance address.  */
            ppp_ptr =  (NX_PPP*)  interface_ptr -> nx_interface_additional_link_info;

            /* Pickup packet pointer.   */
            packet_ptr =  driver_req_ptr -> nx_ip_driver_packet;
            
            /* Determine if the interface link is still up.  */
            if (interface_ptr -> nx_interface_link_up == NX_FALSE)
            {
                
#ifndef NX_PPP_DISABLE_INFO

                /* Increment the transmit frames dropped counter.  */
                ppp_ptr -> nx_ppp_transmit_frames_dropped++;
#endif
                /* No, release the packet.  */
                nx_packet_transmit_release(packet_ptr);
                break;
            }

            /* Disable interrupts.   */
            TX_DISABLE
        
            /* Determine if the transmit queue is empty.  */
            if (ppp_ptr -> nx_ppp_ip_packet_queue_count++)
            {
        
                /* Not empty, simply link the new packet to the tail and update the tail.  */
                (ppp_ptr -> nx_ppp_ip_packet_queue_tail) -> nx_packet_queue_next =  packet_ptr;
                ppp_ptr -> nx_ppp_ip_packet_queue_tail =                            packet_ptr;
            }
            else
            {
        
                /* List is empty, set the head and tail to this packet.  */
                ppp_ptr -> nx_ppp_ip_packet_queue_head =  packet_ptr;
                ppp_ptr -> nx_ppp_ip_packet_queue_tail =  packet_ptr;
            }
        
            /* Restore interrupts.  */
            TX_RESTORE
        
            /* Set event flag to wake up PPP thread for processing.  */
            tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_IP_PACKET_SEND, TX_OR);
            break;
        }

        case NX_LINK_GET_STATUS:
        {

            /* Return the link status in the supplied return pointer.  */
            *(driver_req_ptr -> nx_ip_driver_return_ptr) =  interface_ptr -> nx_interface_link_up;
            break;
        }

        case NX_LINK_UNINITIALIZE:
        {
        
            break;
        }

        default:
        {

            /* Invalid driver request.  */

            /* Return the unhandled command status.  */
            driver_req_ptr -> nx_ip_driver_status =  NX_UNHANDLED_COMMAND;
        }
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_receive_packet_get                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function assembles the PPP frame, which involves moving bytes  */ 
/*    from the serial buffer, converting the escape sequences, and        */
/*    returning the formed PPP packet to the caller.                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    return_packet_ptr                     Return packet pointer         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_check_crc                     Check HDLC frame CRC          */ 
/*    nx_packet_allocate                    Allocate packet               */ 
/*    nx_packet_release                     Release packet                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX                                                             */ 
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
void  _nx_ppp_receive_packet_get(NX_PPP *ppp_ptr, NX_PACKET **return_packet_ptr)
{

TX_INTERRUPT_SAVE_AREA
NX_PACKET   *packet_head_ptr;
#ifndef NX_DISABLE_PACKET_CHAIN
NX_PACKET   *next_packet_ptr;
#endif
UCHAR       byte;
UCHAR       *buffer_ptr;      
ULONG       buffer_size;
ULONG       timeouts;
NX_PACKET   *packet_ptr;
UINT        status;


    /* Default the return packet pointer NULL.  */
    *return_packet_ptr =  NX_NULL;
    
    /* Pickup the current working packet.  */
    packet_ptr =  ppp_ptr -> nx_ppp_receive_partial_packet;

    /* Determine if there is a working receive packet.  */
    if (packet_ptr == NX_NULL)
    {
    
        /* No, setup new working receive packet.  */
    
        /* Allocate a packet for the PPP packet.  */
        status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_RECEIVE_PACKET, NX_PPP_TIMEOUT);

        /* Determine if the packet was allocated successfully.  */
        if (status != NX_SUCCESS)
        {

#ifndef NX_PPP_DISABLE_INFO

            /* Increment the number of packet allocation timeouts.  */ 
            ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

            /* An error was detected, simply return a NULL pointer.  */
            return;
        }
    
        /* Save the working receive packet pointer.  */
        ppp_ptr -> nx_ppp_receive_partial_packet =  packet_ptr;

        /* Setup the packet head pointer.  */
        packet_head_ptr =  packet_ptr;
        ppp_ptr -> nx_ppp_head_packet = packet_head_ptr;

        /* Setup packet payload pointer.  */
        buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

        /* Setup packet size.  */
        buffer_size =  0;

        /* Initialize the timeout counter.  */
        timeouts =  0;
    }
    else
    {
    
        /* Pickup saved buffer ptr, size and timeout counter.  */
        buffer_ptr =   ppp_ptr -> nx_ppp_receive_buffer_ptr;
        buffer_size =  ppp_ptr -> nx_ppp_receive_buffer_size;
        
        /* Pickup saved timeout count.  */
        timeouts =  ppp_ptr -> nx_ppp_receive_timeouts;
        /* Setup the packet head pointer.  */
        packet_head_ptr = ppp_ptr -> nx_ppp_head_packet;
    }

    /* Loop to drain the serial buffer.  */
    do
    {

        /* Disable interrupts.  */
        TX_DISABLE

        /* Determine if there are any characters in the serial receive buffer.  */
        if (ppp_ptr -> nx_ppp_serial_buffer_byte_count)
        {

            /* Yes, there are one or more characters.  */

            /* Pickup the character.  */
            byte =  ppp_ptr -> nx_ppp_serial_buffer[ppp_ptr -> nx_ppp_serial_buffer_read_index];

            /* Now determine if there is an escape sequence character and we don't have the following
               character.  */
            if ((byte == 0x7d) && (ppp_ptr -> nx_ppp_serial_buffer_byte_count == 1))
            {

                /* Restore interrupts.  */
                TX_RESTORE

                /* We need to wait for another character.  */
                break;
            }
        }
        else
        {
    
            /* Nothing is left in the buffer.  */

            /* Restore interrupts.  */
            TX_RESTORE

            /* Determine if the timeout count has been exceeded.  */
            if (timeouts > NX_PPP_RECEIVE_TIMEOUTS)
            {

                /* Timeout count has been exceeded.  */

                /* Determine if the packet is non-PPP. If so, we should dispatch it to 
                   the non-PPP packet handler.  */
                if ((buffer_size) && (packet_head_ptr -> nx_packet_prepend_ptr[0] != 0x7e))
                {

                    /* Determine if there is a user handler for non-PPP packets.  */
                    if (ppp_ptr -> nx_ppp_non_ppp_packet_handler)
                    {
                    
                        /* Adjust the packet parameters.  */
                        packet_head_ptr -> nx_packet_length =  buffer_size;
                        packet_ptr -> nx_packet_append_ptr =  buffer_ptr;

                        /* Dispatch packet to user's handler for non-PPP packets.  */
                        (ppp_ptr -> nx_ppp_non_ppp_packet_handler)(packet_head_ptr);
                    }
                    else
                    {
                    
                        /* Release the current packet.  */
                        nx_packet_release(packet_head_ptr);
                    }
                }
                else
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the frame timeout counter.  */
                    ppp_ptr -> nx_ppp_frame_timeouts++;
#endif

                    /* Release the current packet.  */
                    nx_packet_release(packet_head_ptr);
                }

                /* Clear the saved receive packet.  */
                ppp_ptr -> nx_ppp_receive_partial_packet =  NX_NULL;

                /* Allocate a packet for the PPP packet.  */
                status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_RECEIVE_PACKET, NX_PPP_TIMEOUT);

                /* Determine if the packet was allocated successfully.  */
                if (status != NX_SUCCESS)
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the number of packet allocation timeouts.  */ 
                    ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

                    /* An error was detected, simply return a NULL pointer.  */
                    return;
                }

                /* Save the new receive packet.  */
                ppp_ptr -> nx_ppp_receive_partial_packet =  packet_ptr;

                /* Setup the packet head pointer.  */
                packet_head_ptr =  packet_ptr;
                ppp_ptr -> nx_ppp_head_packet = packet_head_ptr;

                /* Setup packet payload pointer.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Setup packet size.  */
                buffer_size =  0;

                /* Initialize the timeout counter.  */
                timeouts =  0;
            }

            /* We need to wait for another character.  */
            break;
        }

        /* At this point, we have a character. Adjust the serial buffer information.  */
        ppp_ptr -> nx_ppp_serial_buffer_read_index++;

        /* Check for serial buffer wrap-around condition.  */
        if (ppp_ptr -> nx_ppp_serial_buffer_read_index >= NX_PPP_SERIAL_BUFFER_SIZE)
        {

            /* Reset the buffer read index.  */
            ppp_ptr -> nx_ppp_serial_buffer_read_index =  0;
        }

        /* Adjust the serial buffer count.  */
        ppp_ptr -> nx_ppp_serial_buffer_byte_count--;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Clear the timeouts counter.  */
        timeouts =  0;

        /* Determine if we are at the beginning of the packet.  */
        if (buffer_size == 0)
        {

            /* Determine if we should add a PPP frame header.  */
            if (byte == 0xFF)  
            {

                /* Yes, a packet is present without a leading 0x7e. Simply place it in the buffer.  */
                *buffer_ptr++ =  0x7e;

                /* Increment the buffer size.  */
                buffer_size++;
            }
        }

        /* Determine if we are at the end of the PPP frame.  */
        else if (byte == 0x7e)
        {

            /* Yes, we are at the end of the PPP frame.  */

            /* Determine if a non-PPP frame preceded this end of frame marker.  */
            if (packet_head_ptr -> nx_packet_prepend_ptr[0] != 0x7e)
            {

                /* Determine if there is a handler for non-PPP packets.  */
                if (ppp_ptr -> nx_ppp_non_ppp_packet_handler)
                {
                
                    /* Adjust the packet parameters.  */
                    packet_head_ptr -> nx_packet_length =  buffer_size;
                    packet_ptr -> nx_packet_append_ptr =  buffer_ptr;

                    /* Dispatch packet to user's handler for non-PPP packets.  */
                    (ppp_ptr -> nx_ppp_non_ppp_packet_handler)(packet_head_ptr);
                }
                else
                {
                
                    /* Release the current packet.  */
                    nx_packet_release(packet_head_ptr);
                }

                /* Clear the saved receive packet.  */
                ppp_ptr -> nx_ppp_receive_partial_packet =  NX_NULL;

                /* Allocate a packet for the PPP packet.  */
                status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_RECEIVE_PACKET, NX_PPP_TIMEOUT);

                /* Determine if the packet was allocated successfully.  */
                if (status != NX_SUCCESS)
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the number of packet allocation timeouts.  */ 
                    ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

                    /* An error was detected, simply return a NULL pointer.  */
                    return;
                }

                /* Save the current receive packet.  */
                ppp_ptr -> nx_ppp_receive_partial_packet =  packet_ptr;

                /* Setup the packet head pointer.  */
                packet_head_ptr =  packet_ptr;
                ppp_ptr -> nx_ppp_head_packet = packet_head_ptr;

                /* Setup packet payload pointer.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Setup packet size.  */
                buffer_size =  0;

                /* Initialize the timeout counter.  */
                timeouts =  0;

                /* Continue to get the next byte.  */
                continue;
            }

            /* A PPP frame is present. Put the 0x7e into the frame.  */

            /* Yes, place the final 0x7e in the buffer.  */
            *buffer_ptr++ =  0x7e;

            /* Increment the buffer size.  */
            buffer_size++;

            /* Adjust the packet parameters.  */
            packet_head_ptr -> nx_packet_length =  buffer_size;
            packet_ptr -> nx_packet_append_ptr =  buffer_ptr;

            /* Check the CRC of the packet.  */
            status =  _nx_ppp_check_crc(packet_head_ptr);

            /* Determine if there was an error.  */
            if (status != NX_SUCCESS)
            {

                /* CRC error is present, just give up on the current frame.  */

#ifndef NX_PPP_DISABLE_INFO

                /* Increment the frame CRC error counter.  */
                ppp_ptr -> nx_ppp_frame_crc_errors++;
#endif

                /* Release the current packet.  */
                nx_packet_release(packet_head_ptr);

                /* Clear the saved receive packet.  */
                ppp_ptr -> nx_ppp_receive_partial_packet =  NX_NULL;

                /* Allocate a packet for the PPP packet.  */
                status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_RECEIVE_PACKET, NX_WAIT_FOREVER);

                /* Determine if the packet was allocated successfully.  */
                if (status != NX_SUCCESS)
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the number of packet allocation timeouts.  */ 
                    ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

                    /* An error was detected, simply return a NULL pointer.  */
                    return;
                }

                /* Save the current receive packet.  */
                ppp_ptr -> nx_ppp_receive_partial_packet =  packet_ptr;

                /* Setup the packet head pointer.  */
                packet_head_ptr =  packet_ptr;
                ppp_ptr -> nx_ppp_head_packet = packet_head_ptr;

                /* Setup packet payload pointer.  */
                buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                /* Setup packet size.  */
                buffer_size =  0;

                /* Initialize the timeout counter.  */
                timeouts =  0;

                break;
            }

            /* Remove the FCS (2 bytes) and Flag (1 byte).  */
            packet_head_ptr -> nx_packet_length =  packet_head_ptr -> nx_packet_length - 3;  
            packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_append_ptr - 3;

#ifndef NX_DISABLE_PACKET_CHAIN
            /* Check for the condition where there is essentially no data in the last packet. */
            if (packet_ptr -> nx_packet_append_ptr <= packet_ptr -> nx_packet_prepend_ptr)
            {
                ULONG diff = (ULONG)(packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_append_ptr);

                packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr;

                if (packet_head_ptr != packet_ptr)
                {


                    NX_PACKET *tmp_packet_ptr = packet_head_ptr;  
                    NX_PACKET *prev_packet_ptr = tmp_packet_ptr;
                    while (tmp_packet_ptr -> nx_packet_next != 0x0)
                    {
                        prev_packet_ptr = tmp_packet_ptr;
                        tmp_packet_ptr = tmp_packet_ptr -> nx_packet_next;
                    }

                    nx_packet_release(tmp_packet_ptr);
                    prev_packet_ptr -> nx_packet_next = NX_NULL;
                    prev_packet_ptr -> nx_packet_append_ptr -= diff;
                }
            }
#endif  /* NX_DISABLE_PACKET_CHAIN */

            /* Remove the HDLC header.  */
            packet_head_ptr -> nx_packet_prepend_ptr += 3;
            packet_head_ptr -> nx_packet_length -= 3;

            /* Return the pointer.  */
            *return_packet_ptr =  packet_head_ptr;

            /* Set the receive packet to NULL to indicate there is no partial packet being 
               processed.  */
            ppp_ptr -> nx_ppp_receive_partial_packet =  NX_NULL;

            /* Return to caller.  */
            return;
        }

        /* Determine if an escape byte is present.  */
        if (byte == 0x7d)
        {

            /* Disable interrupts.  */
            TX_DISABLE

            /* Pickup the character - we know there is one because of earlier logic.  */
            byte =  ppp_ptr -> nx_ppp_serial_buffer[ppp_ptr -> nx_ppp_serial_buffer_read_index++];

            /* Check for serial buffer wrap-around condition.  */
            if (ppp_ptr -> nx_ppp_serial_buffer_read_index >= NX_PPP_SERIAL_BUFFER_SIZE)
            {

                /* Reset the buffer read index.  */
                ppp_ptr -> nx_ppp_serial_buffer_read_index =  0;
            }

            /* Adjust the serial buffer count.  */
            ppp_ptr -> nx_ppp_serial_buffer_byte_count--;

            /* Restore interrupts.  */
            TX_RESTORE

            /* Convert the escape sequence character.  */
            byte =  byte ^ 0x20;
        }

        /* Determine if there is room in the payload.  */
        if (buffer_ptr < (packet_ptr -> nx_packet_data_end - 4))
        {

            /* Put the character in the packet payload.  */
            *buffer_ptr++ =  byte;

            /* Increment the buffer size.  */
            buffer_size++;
        }

#ifndef NX_DISABLE_PACKET_CHAIN

        /* We need to perform packet chaining at this point, but only for IP data frames. Non PPP data and
           the other PPP protocol packets must fit within the payload of one packet.  */
        else if ((packet_head_ptr -> nx_packet_prepend_ptr[0] == 0x7e) && 
                 (packet_head_ptr -> nx_packet_prepend_ptr[1] == 0xff) &&
                 (packet_head_ptr -> nx_packet_prepend_ptr[2] == 0x03) &&
                 (packet_head_ptr -> nx_packet_prepend_ptr[3] == 0x00) &&
                 (packet_head_ptr -> nx_packet_prepend_ptr[4] == 0x21)) /* 0x0021 is NX_PPP_DATA */
        {

            /* We need to move to the next packet and chain them.  */
            packet_ptr -> nx_packet_append_ptr =  buffer_ptr;

            /* Allocate a new packet to for packet chaining.  */
            status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &next_packet_ptr, NX_RECEIVE_PACKET, NX_PPP_TIMEOUT);

            /* Determine if the packet was allocated successfully.  */
            if (status != NX_SUCCESS)
            {

#ifndef NX_PPP_DISABLE_INFO

                /* Increment the number of packet allocation timeouts.  */ 
                ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

                /* Clear the partial receive packet pointer.  */
                ppp_ptr -> nx_ppp_receive_partial_packet =  NX_NULL;

                /* Release the current packet.  */
                nx_packet_release(packet_head_ptr);

                /* An error was detected, simply return a NULL pointer.  */
                return;
            }

            /* Adjust the next packet pointer.  */
            packet_ptr -> nx_packet_next =  next_packet_ptr;

            /* Setup the last pointer.  */
            packet_head_ptr -> nx_packet_last =  next_packet_ptr;

            /* Use the packet pointer.  */
            packet_ptr =  next_packet_ptr;
            ppp_ptr -> nx_ppp_receive_partial_packet = packet_ptr;

            /* Setup buffer pointer.  */
            buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

            /* Put the character in the packet payload.  */
            *buffer_ptr++ =  byte;

            /* Increment the buffer size.  */
            buffer_size++;
        }
#endif /* NX_DISABLE_PACKET_CHAIN */
        else
        {

            /* At this point we know we have a buffer full of non-PPP characters, in 
               most cases this is simply noise.  */

            /* Determine if there is a non-PPP handler.  */
            if (ppp_ptr -> nx_ppp_non_ppp_packet_handler)
            {

                /* Adjust the packet parameters.  */
                packet_head_ptr -> nx_packet_length =  buffer_size;
                packet_ptr -> nx_packet_append_ptr =  buffer_ptr;

                /* Dispatch packet to user's handler for non-PPP packets.  */
                (ppp_ptr -> nx_ppp_non_ppp_packet_handler)(packet_head_ptr);
            }
            else
            {
            
                /* Release the current packet.  */
                nx_packet_release(packet_head_ptr);
            }
            
            /* Clear the partial receive packet pointer.  */
            ppp_ptr -> nx_ppp_receive_partial_packet =  NX_NULL;

            /* Allocate a packet for the PPP packet.  */
            status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_RECEIVE_PACKET, NX_PPP_TIMEOUT);

            /* Determine if the packet was allocated successfully.  */
            if (status != NX_SUCCESS)
            {

#ifndef NX_PPP_DISABLE_INFO

                /* Increment the number of packet allocation timeouts.  */ 
                ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

                /* An error was detected, simply return a NULL pointer.  */
                return;
            }

            /* Save the current receive packet.  */
            ppp_ptr -> nx_ppp_receive_partial_packet =  packet_ptr;

            /* Setup the packet head pointer.  */
            packet_head_ptr =  packet_ptr;
            ppp_ptr -> nx_ppp_head_packet = packet_head_ptr;

            /* Setup packet payload pointer.  */
            buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

            /* Setup packet size.  */
            buffer_size =  0;

            /* Initialize the timeout counter.  */
            timeouts =  0;
        }
    } while(ppp_ptr -> nx_ppp_serial_buffer_byte_count);

    /* Save the buffer size, buffer pointer, and timeout count.  */
    ppp_ptr -> nx_ppp_receive_buffer_size =  buffer_size;
    ppp_ptr -> nx_ppp_receive_buffer_ptr =   buffer_ptr;
    ppp_ptr -> nx_ppp_receive_timeouts =     timeouts;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_receive_packet_process                      PORTABLE C      */ 
/*                                                           6.1.2        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes a received PPP request and dispatches it    */ 
/*    to the appropriate protocol or to NetX.                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    packet_ptr                            Pointer to PPP packet         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                     Release packet                */ 
/*    _nx_ppp_lcp_state_machine_update      Process LCP message           */ 
/*    _nx_ppp_pap_state_machine_update      Process PAP message           */ 
/*    _nx_ppp_chap_state_machine_update     Process CHAP message          */ 
/*    _nx_ppp_ipcp_state_machine_update     Process IPCP message          */ 
/*    _nx_ppp_netx_packet_transfer          Transfer packet to NetX       */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_thread_entry                  PPP thread entry              */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  11-09-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            improved packet length      */
/*                                            verification,               */
/*                                            resulting in version 6.1.2  */
/*                                                                        */
/**************************************************************************/
void  _nx_ppp_receive_packet_process(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr)
{

UINT        protocol;
UINT        ppp_ipcp_state;
UINT        code;
UINT        length;


#ifndef NX_PPP_DISABLE_INFO

    /* Increment the number of IP frames sent.  */
    ppp_ptr -> nx_ppp_total_frames_received++;
#endif

    /* Check for valid packet length for Protocol (2 bytes).  */
    if (packet_ptr -> nx_packet_length < 2)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        /* Return.  */
        return;
    }

    /* Pickup the protocol type.  */
    protocol =  (((UINT) packet_ptr -> nx_packet_prepend_ptr[0]) << 8) | ((UINT) packet_ptr -> nx_packet_prepend_ptr[1]);

    /* Check protocol.  */
    if (protocol != NX_PPP_DATA)
    {

#ifndef NX_DISABLE_PACKET_CHAIN

        /* Discard the chained packets.  */
        if (packet_ptr -> nx_packet_next)
        {

            /* Release the packet. */
            nx_packet_release(packet_ptr);

            /* Return.  */
            return;
        }
#endif

        /* Other Protocols must also have Code:1 byte, Identifier:1 bytes, Length: 2 bytes.  */
        if (packet_ptr -> nx_packet_length < 6)
        {

            /* Release the packet. */
            nx_packet_release(packet_ptr);

            /* Return.  */
            return;
        }

        /* Get the message length.  */
        length = (((UINT) packet_ptr -> nx_packet_prepend_ptr[4]) << 8) | ((UINT) packet_ptr -> nx_packet_prepend_ptr[5]);

        /* Check if the packet length is equal to message length plus 2 bytes protocal type.  */
        if ((length + 2) != packet_ptr -> nx_packet_length)
        {

            /* Release the packet. */
            nx_packet_release(packet_ptr);

            /* Return.  */
            return;
        }
    }

    /* Determine if the packet is LCP.  */
    if (protocol == NX_PPP_LCP_PROTOCOL) 
    {

        /* Pickup the type of received LCP code.  */
        code =  packet_ptr -> nx_packet_prepend_ptr[2];

        /* Process the LCP message, updating the LCP state machine.  */
        _nx_ppp_lcp_state_machine_update(ppp_ptr, packet_ptr);

        /* Determine if the LCP is now complete.  */
        if (ppp_ptr -> nx_ppp_lcp_state ==  NX_PPP_LCP_COMPLETED_STATE)
        {

#ifndef NX_PPP_DISABLE_PAP

            /* Determine if the PAP state machine should be started. */
            if ((ppp_ptr -> nx_ppp_generate_authentication_protocol == NX_PPP_PAP_PROTOCOL) ||
                (ppp_ptr -> nx_ppp_verify_authentication_protocol == NX_PPP_PAP_PROTOCOL))
            {

                /* Check if PAP already start.  */
                if (ppp_ptr -> nx_ppp_pap_state == NX_PPP_PAP_INITIAL_STATE)
                {

                    /* Yes, change state to PAP start state and update the PAP state machine.  */
                    ppp_ptr -> nx_ppp_pap_state = NX_PPP_PAP_START_STATE;

                    /* Process the PAP message.  */
                    _nx_ppp_pap_state_machine_update(ppp_ptr, NX_NULL);
                }
            }
#endif

#ifndef NX_PPP_DISABLE_CHAP

            /* Determine if the CHAP state machine should be started. */
            if ((ppp_ptr -> nx_ppp_generate_authentication_protocol == NX_PPP_CHAP_PROTOCOL) ||
                (ppp_ptr -> nx_ppp_verify_authentication_protocol == NX_PPP_CHAP_PROTOCOL))
            {

                /* Check if CHAP already start.  */
                if (ppp_ptr -> nx_ppp_chap_state == NX_PPP_CHAP_INITIAL_STATE)
                {

                    /* Yes, change state to CHAP start state and update the CHAP state machine.  */
                    ppp_ptr -> nx_ppp_chap_state = NX_PPP_CHAP_START_STATE;

                    /* Process the CHAP message.  */
                    _nx_ppp_chap_state_machine_update(ppp_ptr, NX_NULL);
                }
            }
#endif

            /* Do not set the IPCP state to INIT on receipt of an LCP echo request or reply! */
            if ((code != NX_PPP_LCP_ECHO_REQUEST) && (code != NX_PPP_LCP_ECHO_REPLY))
            {

                /* Check for authentication.  */
                if (ppp_ptr -> nx_ppp_authenticated)
                {

                    /* Check if IPCP already start.  */
                    if (ppp_ptr -> nx_ppp_ipcp_state == NX_PPP_IPCP_INITIAL_STATE)
                    {

                        /* Yes, change state to IPCP start state and update the PAP state machine.  */
                        ppp_ptr -> nx_ppp_ipcp_state = NX_PPP_IPCP_START_STATE;

                        /* Process the IPCP message.  */
                        _nx_ppp_ipcp_state_machine_update(ppp_ptr, NX_NULL);
                    }
                }
            }

            /* Check if the packet is Echo Request.  */
            if (code == NX_PPP_LCP_ECHO_REQUEST)
            {

                /* Clear the pointer since it was sent out as Echo Reply.  */
                packet_ptr =  NX_NULL;
            }
        }
    }


#ifndef NX_PPP_DISABLE_PAP

    /* Determine if the packet is PAP.  */
    else if (protocol == NX_PPP_PAP_PROTOCOL)
    {        

        /* Process the PAP message.  */
        _nx_ppp_pap_state_machine_update(ppp_ptr, packet_ptr);

        /* Check for authentication.  */
        if (ppp_ptr -> nx_ppp_authenticated)
        {

            /* Check if IPCP already start.  */
            if (ppp_ptr -> nx_ppp_ipcp_state == NX_PPP_IPCP_INITIAL_STATE)
            {

                /* Yes, change state to IPCP start state and update the IPCP state machine.  */
                ppp_ptr -> nx_ppp_ipcp_state = NX_PPP_IPCP_START_STATE;

                /* Process the IPCP message.  */
                _nx_ppp_ipcp_state_machine_update(ppp_ptr, NX_NULL);
            }
        }
    }
#endif

#ifndef NX_PPP_DISABLE_CHAP

    /* Determine if the packet is CHAP.  */
    else if (protocol == NX_PPP_CHAP_PROTOCOL) 
    {        

        /* Process the CHAP message.  */
        _nx_ppp_chap_state_machine_update(ppp_ptr, packet_ptr);

        /* Check for authentication.  */
        if (ppp_ptr -> nx_ppp_authenticated)
        {

            /* Check if IPCP already start.  */
            if (ppp_ptr -> nx_ppp_ipcp_state == NX_PPP_IPCP_INITIAL_STATE)
            {

                /* Yes, change state to IPCP start state and update the IPCP state machine.  */
                ppp_ptr -> nx_ppp_ipcp_state = NX_PPP_IPCP_START_STATE;

                /* Process the IPCP message.  */
                _nx_ppp_ipcp_state_machine_update(ppp_ptr, NX_NULL);
            }
        }
    }    
#endif

    /* Determine if packet is IPCP.  */
    else if (protocol == NX_PPP_IPCP_PROTOCOL)
    { 

        /* Process the IPCP message.  */
        ppp_ipcp_state = ppp_ptr -> nx_ppp_ipcp_state;

        _nx_ppp_ipcp_state_machine_update(ppp_ptr, packet_ptr);

        /* Check for IPCP completion... when this happens, the link is up!  */
        if (ppp_ptr -> nx_ppp_ipcp_state ==  NX_PPP_IPCP_COMPLETED_STATE)
        {
        
            /* Set the PPP state machine to indicate it is complete.  */
            ppp_ptr -> nx_ppp_state =  NX_PPP_ESTABLISHED;
            
            /* Mark the IP interface instance as link up.  */
            (ppp_ptr -> nx_ppp_interface_ptr) -> nx_interface_link_up =  NX_TRUE;

            /* Determine if the application has registered a link up notification
               callback.  */
            if ((ppp_ptr -> nx_ppp_link_up_callback) && (ppp_ipcp_state!=NX_PPP_IPCP_COMPLETED_STATE))
            {

                /* Yes, call the application's callback function.  */
                (ppp_ptr -> nx_ppp_link_up_callback)(ppp_ptr);
            }
        }
    }

    /* Check for normal data packet.  */
    else if (protocol == NX_PPP_DATA)
    { 

        /* Transfer the packet to NetX.  */
        _nx_ppp_netx_packet_transfer(ppp_ptr, packet_ptr);

        /* Clear the pointer since it was passed to NetX.  */
        packet_ptr =  NX_NULL;
    }
#ifndef NX_PPP_DISABLE_INFO

    else
        /* Increment the number of frames dropped.  */
        ppp_ptr -> nx_ppp_receive_frames_dropped++;
#endif

    /* Determine if the packet needs to be released.  */
    if (packet_ptr)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_timeout                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes PPP time-out events, which includes         */ 
/*    updating the responsible state machine.                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_state_machine_update      PPP state machine update      */ 
/*    _nx_ppp_pap_state_machine_update      PPP PAP state machine update  */ 
/*    _nx_ppp_chap_state_machine_update     PPP CHAP state machine update */ 
/*    _nx_ppp_ipcp_state_machine_update     PPP IPCP state machine update */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX                                                             */ 
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
void _nx_ppp_timeout(NX_PPP *ppp_ptr)
{

    /* Determine if the LCP state machine needs updating.  */
    if ((ppp_ptr -> nx_ppp_lcp_state >= NX_PPP_LCP_START_STATE) && 
         (ppp_ptr -> nx_ppp_lcp_state < NX_PPP_LCP_COMPLETED_STATE))      
    {

        /* Update the PPP state machine.  */    
        _nx_ppp_lcp_state_machine_update(ppp_ptr, NX_NULL);
    }

#ifndef NX_PPP_DISABLE_PAP

    /* Determine if the PAP state machine needs updating.  */
    if ((ppp_ptr -> nx_ppp_pap_state >= NX_PPP_PAP_START_STATE) && 
         (ppp_ptr -> nx_ppp_pap_state < NX_PPP_PAP_COMPLETED_STATE))      
    {

         /* Update the PAP state machine.  */
         _nx_ppp_pap_state_machine_update(ppp_ptr, NX_NULL);
    }
#endif

#ifndef NX_PPP_DISABLE_CHAP

    /* Determine if the CHAP state machine needs updating.  */
    if ((ppp_ptr -> nx_ppp_chap_state >= NX_PPP_CHAP_START_STATE) && 
         (ppp_ptr -> nx_ppp_chap_state < NX_PPP_CHAP_COMPLETED_STATE))      
    {

         /* Update the CHAP state machine.  */
         _nx_ppp_chap_state_machine_update(ppp_ptr, NX_NULL);
    }
#endif

    /* Determine if the IPCP state machine needs updating.  */
    if ((ppp_ptr -> nx_ppp_ipcp_state >= NX_PPP_IPCP_START_STATE) && 
         (ppp_ptr -> nx_ppp_ipcp_state < NX_PPP_IPCP_COMPLETED_STATE))      
    {

        /* Update the IPCP state machine.  */
        _nx_ppp_ipcp_state_machine_update(ppp_ptr, NX_NULL);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_timer_entry                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes PPP time-outs, which sets a time-out event  */ 
/*    that wakes up the PPP processing thread.                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Set timeout event flag        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX                                                             */ 
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
void _nx_ppp_timer_entry(ULONG id)
{

NX_PPP  *ppp_ptr;


    /* Pickup the PPP pointer.  */
    ppp_ptr =  (NX_PPP *) id;

    /* Make sure the PPP pointer is good.  */
    if ((ppp_ptr != NX_NULL) && (ppp_ptr -> nx_ppp_id == NX_PPP_ID))
    {    

        /* Set the timeout event for the PPP thread.  */
        tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_TIMEOUT, TX_OR);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_netx_packet_transfer                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function removes the PPP header and passes the packet to       */ 
/*    NetX.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ip_packet_deferred_receive        Deferred IP packet receive    */ 
/*    _nx_ip_packet_receive                 IP packet receive             */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_receive_packet_process        Receive packet processing     */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memmove use cases, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
void _nx_ppp_netx_packet_transfer(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr)
{

ULONG   offset;

#ifndef NX_PPP_DISABLE_INFO
    /* Increment the number of IP frames received.  */
    ppp_ptr -> nx_ppp_ip_frames_received++;
#endif

    /* Add the incoming interface pointer.  */
    packet_ptr -> nx_packet_ip_interface =  ppp_ptr -> nx_ppp_interface_ptr;

    /* Remove the PPP header [00,21] in the front of the IP packet.  */
    packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr + 2;

    /* Adjust the packet length.  */
    packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length - 2;

    /* Calculate the offset for four byte alignment.  */
    offset = (((ULONG)packet_ptr -> nx_packet_prepend_ptr) & 3);

    /* Move the data to keep four byte alignment for first packet.  */
    if (offset)
    {
        memmove(packet_ptr -> nx_packet_prepend_ptr - offset, packet_ptr -> nx_packet_prepend_ptr, /* Use case of memmove is verified.  */
                (UINT)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr));
        packet_ptr -> nx_packet_prepend_ptr -= offset;
        packet_ptr -> nx_packet_append_ptr -= offset;
    }

    /* Transfer the receive packet to NetX.  */
#ifdef NX_DIRECT_ISR_CALL
    _nx_ip_packet_receive(ppp_ptr -> nx_ppp_ip_ptr, packet_ptr);
#else
    _nx_ip_packet_deferred_receive(ppp_ptr -> nx_ppp_ip_ptr, packet_ptr);
#endif
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_process_deferred_raw_string_send            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function takes all the raw string packets from the deferred raw*/ 
/*    string packet queue and then sends them out.                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    (nx_ppp_byte_send)                    User's byte output routine    */ 
/*    nx_packet_release                     Release packet                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_thread_entry                  PPP processing thread         */ 
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
void _nx_ppp_process_deferred_raw_string_send(NX_PPP *ppp_ptr)
{

TX_INTERRUPT_SAVE_AREA
  
NX_PACKET       *packet_ptr;
ULONG           i;
#ifdef NX_PPP_PPPOE_ENABLE
UINT            release_packet;
#endif /* NX_PPP_PPPOE_ENABLE  */


    /* Loop to process all queued packets.  */
    do
    {
    
        /* Disable interrupts.  */
        TX_DISABLE
        
        /* Pickup the number of queued packets.   */
        if (ppp_ptr -> nx_ppp_raw_packet_queue_count)
        {
        
            /* Pickup the oldest packet.  */
            packet_ptr =  ppp_ptr -> nx_ppp_raw_packet_queue_head;
            
            /* Update the head pointer to the next packet. */
            ppp_ptr -> nx_ppp_raw_packet_queue_head =  packet_ptr -> nx_packet_queue_next;
            
            /* Decrement the number of queued packets.  */
            ppp_ptr -> nx_ppp_raw_packet_queue_count--;
        }
        else
        {
        
            /* No packet just set the packet pointer to NULL.  */
            packet_ptr =  NX_NULL;
        }    

        /* Restore interrupts.  */
        TX_RESTORE

        /* Is there a packet to process?  */
        if (packet_ptr == NX_NULL)
        {
        
            /* No, just get out of the loop!  */
            break;
        }

#ifdef NX_PPP_PPPOE_ENABLE
        release_packet = NX_TRUE;
#endif /* NX_PPP_PPPOE_ENABLE  */

        if (ppp_ptr -> nx_ppp_byte_send)
        {

            /* Loop to send all the bytes of the packet out.  */
            for (i = 0; i < packet_ptr -> nx_packet_length; i++)
            {

                /* Send each byte out.  */
                (ppp_ptr -> nx_ppp_byte_send)(packet_ptr -> nx_packet_prepend_ptr[i]);
            }
        }

#ifdef NX_PPP_PPPOE_ENABLE

        /* Check the PPPoE packet send function.  */
        if (ppp_ptr -> nx_ppp_packet_send)
        {

            /* Send the packet out.  */
            (ppp_ptr -> nx_ppp_packet_send)(packet_ptr);

            /* Update the flag since this packet should been released in PPPoE.  */
            release_packet = NX_FALSE;
        }

        /* Check if need to release the packet.  */
        if (release_packet == NX_TRUE)
#endif /* NX_PPP_PPPOE_ENABLE  */

            nx_packet_release(packet_ptr);

    } while (1);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_process_deferred_ip_packet_send             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function takes all the packets from the deferred IP packet     */ 
/*    queue, packages them in an PPP frame, and then sends them out.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_packet_transmit               Send AHDLC packet             */ 
/*    nx_packet_transmit_release            Release NetX packet           */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_thread_entry                  PPP processing thread         */ 
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
void _nx_ppp_process_deferred_ip_packet_send(NX_PPP *ppp_ptr)
{

TX_INTERRUPT_SAVE_AREA
  
NX_PACKET       *packet_ptr;


    /* Loop to process all queued packets.  */
    do
    {
    
        /* Disable interrupts.  */
        TX_DISABLE
        
        /* Pickup the number of queued packets.   */
        if (ppp_ptr -> nx_ppp_ip_packet_queue_count)
        {
        
            /* Pickup the oldest packet.  */
            packet_ptr =  ppp_ptr -> nx_ppp_ip_packet_queue_head;
            
            /* Update the head pointer to the next packet. */
            ppp_ptr -> nx_ppp_ip_packet_queue_head =  packet_ptr -> nx_packet_queue_next;
            
            /* Decrement the number of queued packets.  */
            ppp_ptr -> nx_ppp_ip_packet_queue_count--;
        }
        else
        {
        
            /* No packet just set the packet pointer to NULL.  */
            packet_ptr =  NX_NULL;
        }    

        /* Restore interrupts.  */
        TX_RESTORE

        /* Is there a packet to process?  */
        if (packet_ptr == NX_NULL)
        {
        
            /* No, just get out of the loop!  */
            break;
        }

        /* Determine if there is room in the front of the packet for the PPP header.  */
        if ((packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_data_start) < 2)
        {

            /* Error, there is no room at the front of the packet to prepend the PPP header.  */

#ifndef NX_PPP_DISABLE_INFO

            /* Increment the internal error counter.  */
            ppp_ptr -> nx_ppp_internal_errors++;
#endif

            /* Release the NetX packet.  */
            nx_packet_transmit_release(packet_ptr);

            /* An error was detected, simply return a NULL pointer.  */
            return;
        }

        /* Otherwise, backup the prepend pointer.  */
        packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr - 2;

        /* Place the PPP header in the packet.  */
        packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_DATA & 0xFF00) >> 8;
        packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_DATA & 0xFF;

        /* Adjust the length of the packet.  */
        packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length + 2;

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of IP frames sent.  */
        ppp_ptr -> nx_ppp_ip_frames_sent++;
#endif

        /* Send the packet!  */
        _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);
    } while (1);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_state_machine_update                    PORTABLE C      */ 
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes LCP messages and updates the LCP state      */ 
/*    machine.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    packet_ptr                            Pointer to LCP packet. If the */ 
/*                                            packet is NULL, a timeout is*/ 
/*                                            present                     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_code_reject               Reject received code          */ 
/*    _nx_ppp_lcp_configuration_retrieve    Retrieve configuration info   */ 
/*    _nx_ppp_lcp_configure_reply_send      Send configure reply          */ 
/*    _nx_ppp_lcp_configure_request_send    Send configure request        */ 
/*    _nx_ppp_lcp_nak_configure_list        Get options naked             */ 
/*    _nx_ppp_lcp_terminate_ack_send        Send terminate ack            */ 
/*    _nx_ppp_lcp_terminate_request_send    Send terminate request        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_receive_packet_process        Receive packet processing     */ 
/*    _nx_ppp_timeout                       PPP timeout                   */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  11-09-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            improved packet length      */
/*                                            verification,               */
/*                                            resulting in version 6.1.2  */
/*  08-02-2021     Yuxin Zhou               Modified comment(s), fixed    */
/*                                            the logic of retransmission,*/
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
void  _nx_ppp_lcp_state_machine_update(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr)
{
     
UINT    configure_status;
UCHAR   *lcp_message_ptr;
UCHAR   code;
UINT    status;

    /* Determine if a packet is present. If so, derive the event from the packet.  */
    if (packet_ptr)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of LCP frames received.  */
        ppp_ptr -> nx_ppp_lcp_frames_received++;
#endif

        /* Setup a pointer to the LCP pointer.  */
        lcp_message_ptr =  packet_ptr -> nx_packet_prepend_ptr;

        /* Pickup the type of received LCP code.  */
        code =  packet_ptr -> nx_packet_prepend_ptr[2];

#ifndef NX_PPP_DISABLE_INFO

        /* Count the number of specific LCP requests.  */
        switch (code)
        {
        
        case NX_PPP_LCP_CONFIGURE_REQUEST:
        
            /* Increment the number of LCP configure requests received.  */
            ppp_ptr -> nx_ppp_lcp_configure_requests_received++;
            break;
        
        case NX_PPP_LCP_CONFIGURE_ACK:
        
            /* Increment the number of LCP configure ACKs received.  */
            ppp_ptr -> nx_ppp_lcp_configure_acks_received++;
            break;

        case NX_PPP_LCP_CONFIGURE_NAK:

            /* Increment the number of LCP configure NAKs received.  */
            ppp_ptr -> nx_ppp_lcp_configure_naks_received++;        
            break;

        case NX_PPP_LCP_CONFIGURE_REJECT:
        
            /* Increment the number of LCP configure rejects received.  */
            ppp_ptr -> nx_ppp_lcp_configure_rejects_received++;
            break;
        
        case NX_PPP_LCP_TERMINATE_REQUEST:
        
            /* Increment the number of LCP terminate requests received.  */
            ppp_ptr -> nx_ppp_lcp_terminate_requests_received++;
            break;
        
        case NX_PPP_LCP_TERMINATE_ACK:
     
            /* Increment the number of LCP terminate ACKs received.  */
            ppp_ptr -> nx_ppp_lcp_terminate_acks_received++;  
            break;

        case NX_PPP_LCP_CODE_REJECT:
        
            /* Increment the number of LCP code rejects received.  */
            ppp_ptr -> nx_ppp_lcp_code_rejects_received++;
            break;

        case NX_PPP_LCP_PROTOCOL_REJECT:
        
            /* Increment the number of LCP protocol rejects received.  */
            ppp_ptr -> nx_ppp_lcp_protocol_rejects_received++;
            break;

        case NX_PPP_LCP_ECHO_REQUEST:
        
            /* Increment the number of LCP echo requests received.  */
            ppp_ptr -> nx_ppp_lcp_echo_requests_received++;

            break;
       
        case NX_PPP_LCP_ECHO_REPLY:

                /* Increment the number of LCP echo replies received.  */
                break;

        case NX_PPP_LCP_DISCARD_REQUEST:
        
            /* Increment the number of LCP discard requests received.  */
            ppp_ptr -> nx_ppp_lcp_discard_requests_received++;
            break;

        default: 
        
            /* Increment the number of LCP unknown (unhandled) requests received.  */
            ppp_ptr -> nx_ppp_lcp_unknown_requests_received++;
        }
#endif

        /* Remember receive id.  */
        ppp_ptr -> nx_ppp_receive_id =  packet_ptr -> nx_packet_prepend_ptr[3];

        /* Is the code supported by PPP?  */
        if ((code < NX_PPP_LCP_CONFIGURE_REQUEST) ||
            (code > NX_PPP_LCP_DISCARD_REQUEST))
        {
        
            /* No, this code is not supported. Reject the code.  */
            _nx_ppp_lcp_code_reject(ppp_ptr, lcp_message_ptr);
            
            /* Return.  */
            return;
        }
    }
    else
    {

        /* Set the LCP pointer to NULL.  */
        lcp_message_ptr =  NX_NULL;
        
        /* Set the code to timeout to indicate a timeout has occurred.  */
        code =  NX_PPP_LCP_TIMEOUT;

#ifndef NX_PPP_DISABLE_INFO

        /* Determine if we are in the initial state. If so, this really isn't a timeout.  */
        if (ppp_ptr -> nx_ppp_lcp_state != NX_PPP_LCP_START_STATE)
        {

            /* Increment the LCP timeout counter.  */
            ppp_ptr -> nx_ppp_lcp_state_machine_timeouts++;
        }
#endif
    }

    /* Process relative to the current state.  */
    switch (ppp_ptr -> nx_ppp_lcp_state)
    {
    
        case NX_PPP_LCP_START_STATE:
        {
    
            /* Initial LCP state.  */
 
            /* Initialize the NAK and rejected lists.  */
#ifndef NX_PPP_DNS_OPTION_DISABLE

            ppp_ptr -> nx_ppp_naked_list[0] =       0;
#else
            ppp_ptr -> nx_ppp_naked_list[0] =       1;
#endif
            ppp_ptr -> nx_ppp_peer_naked_list[0] =  0;
            ppp_ptr -> nx_ppp_rejected_list[0] =    0;

            /* Setup the retry counter.  */
            ppp_ptr -> nx_ppp_protocol_retry_counter =  0;
 
            /* Setup the timeout.  */
            ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;

            /* Send configuration request to peer.  */
            _nx_ppp_lcp_configure_request_send(ppp_ptr);

            /* Move to the next state.  */
            ppp_ptr -> nx_ppp_lcp_state =  NX_PPP_LCP_CONFIGURE_REQUEST_SENT_STATE;
            break;
        }

        case NX_PPP_LCP_CONFIGURE_REQUEST_SENT_STATE:
        {
    
            /* In this state, we have sent a configuration request but had not received an ACK
               or a configuration request from the peer.  */
    
            /* Process relative to the incoming code.  */
            if (code ==  NX_PPP_LCP_CONFIGURE_ACK)
            {

                /* Determine if the ID matches our last transmit ID. If not, just discard it.  */
                if (ppp_ptr -> nx_ppp_transmit_id != ppp_ptr -> nx_ppp_receive_id)
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the invalid frame ID counter.  */
                    ppp_ptr -> nx_ppp_invalid_frame_id++;
#endif

                    /* Discard this request by simply returning!  */
                    return;
                }

                /* The peer has ACKed our configuration request. Move to the 
                   configure request ACKed state.  */
                ppp_ptr -> nx_ppp_lcp_state =  NX_PPP_LCP_CONFIGURE_REQUEST_ACKED_STATE;
            
                /* Turn off the timeout for the configuration request.  */
                ppp_ptr -> nx_ppp_timeout =  0;
            }

            else if (code ==  NX_PPP_LCP_CONFIGURE_NAK)
            {
        
                /* Determine if the ID matches our last transmit ID. If not, just discard it.  */
                if (ppp_ptr -> nx_ppp_transmit_id != ppp_ptr -> nx_ppp_receive_id)
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the invalid frame ID counter.  */
                    ppp_ptr -> nx_ppp_invalid_frame_id++;
#endif

                    /* Discard this request by simply returning!  */
                    return;
                }

                /* Figure out what options were naked.  */
                _nx_ppp_lcp_nak_configure_list(ppp_ptr, ppp_ptr -> nx_ppp_naked_list);

                /* Send new configure request.  */
                _nx_ppp_lcp_configure_request_send(ppp_ptr);
            }

            else if ((code == NX_PPP_LCP_CONFIGURE_REQUEST) && (packet_ptr))
            {
        
                /* The peer has sent a configuration request.  */ 
            
                /* Retrieve configuration.  */
                status = _nx_ppp_lcp_configuration_retrieve(ppp_ptr, packet_ptr, ppp_ptr -> nx_ppp_peer_naked_list, ppp_ptr -> nx_ppp_rejected_list, &configure_status); 
 
                /* Discard invalid packet.  */
                if (status)
                {
                    return;
                }

                /* Determine if the configuration request is fine or needs to be negotiated further.  */
                if (configure_status == 0)
                {
            
                    /* The peer's request is acceptable, move into peer configuration request ACKed state.  */
                    ppp_ptr -> nx_ppp_lcp_state =  NX_PPP_LCP_PEER_CONFIGURE_REQUEST_ACKED_STATE;
                }
                        
                /* Send configuration reply.  */
                _nx_ppp_lcp_configure_reply_send(ppp_ptr, configure_status, lcp_message_ptr, ppp_ptr -> nx_ppp_peer_naked_list, ppp_ptr -> nx_ppp_rejected_list);                       
            }
            else if (code == NX_PPP_LCP_TIMEOUT)
            {

                /* Increment the retry counter.  */
                ppp_ptr -> nx_ppp_protocol_retry_counter++;
 
                /* Determine if the LCP retry counter has been exceeded.  */
                if (ppp_ptr -> nx_ppp_protocol_retry_counter < NX_PPP_MAX_LCP_PROTOCOL_RETRIES)
                {
 
                    /* Setup the timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;
        
                    /* Timeout, send configuration request again.  */
                    _nx_ppp_lcp_configure_request_send(ppp_ptr); 
                }
                else
                {

                    /* Retry counter exceeded.  */

                    /* Enter LCP failed state.  PPP must be stopped and started to try again.  */
                    ppp_ptr -> nx_ppp_lcp_state =  NX_PPP_LCP_FAILED_STATE;

                    /* Determine if the application has registered a link down notification
                       callback.  */
                    if (ppp_ptr -> nx_ppp_link_down_callback)
                    {

                        /* Yes, call the application's callback function.  */
                        (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                    }
                }
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_lcp_state_machine_unhandled_requests++;
            }
#endif
            break;    
        }

        case NX_PPP_LCP_CONFIGURE_REQUEST_ACKED_STATE:
        {
    
            /* In this state, we have received the ACK for our configuration request, but have not yet
               received a configuration request from the peer.  */
    
            /* Process relative to the incoming code.  */
            if ((code == NX_PPP_LCP_CONFIGURE_REQUEST) && (packet_ptr))
            {
        
                /* The peer has sent a configuration request.  */ 
                
                /* Retrieve configuration.  */
                status = _nx_ppp_lcp_configuration_retrieve(ppp_ptr, packet_ptr, ppp_ptr -> nx_ppp_peer_naked_list, ppp_ptr -> nx_ppp_rejected_list, &configure_status); 
 
                /* Discard invalid packet.  */
                if (status)
                {
                    return;
                }
                        
                /* Determine if the configuration request is fine or needs to be negotiated further.  */
                if (configure_status == 0)
                {
            
                    /* The peer's request is acceptable, move into the LCP done state.  */
                    ppp_ptr -> nx_ppp_lcp_state =  NX_PPP_LCP_COMPLETED_STATE;

                    /* Determine if we need to update the IP's MTU.  */
                    if (ppp_ptr -> nx_ppp_mru > (ppp_ptr -> nx_ppp_interface_ptr) -> nx_interface_ip_mtu_size)
                    {

                        /* Yes, the peer can accept larger messages than the default.  */
                        (ppp_ptr -> nx_ppp_interface_ptr) -> nx_interface_ip_mtu_size =  ppp_ptr -> nx_ppp_mru;
                    }
                    
                    /* Disable the LCP timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  0;
                }
                        
                /* Send configuration reply.  */
                _nx_ppp_lcp_configure_reply_send(ppp_ptr, configure_status, lcp_message_ptr, ppp_ptr -> nx_ppp_peer_naked_list, ppp_ptr -> nx_ppp_rejected_list);                       
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_lcp_state_machine_unhandled_requests++;
            }
#endif
            break;    
        }
    
        case NX_PPP_LCP_PEER_CONFIGURE_REQUEST_ACKED_STATE:
        {

            /* In this state, we have sent our configuration request, but haven't received an ACK. We have also received 
               a peer configuration request and have ACKed that request.  */
           
            /* Process relative to the incoming code.  */
            if (code ==  NX_PPP_LCP_CONFIGURE_ACK)
            {
        
                /* Determine if the ID matches our last transmit ID. If not, just discard it.  */
                if (ppp_ptr -> nx_ppp_transmit_id != ppp_ptr -> nx_ppp_receive_id)
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the invalid frame ID counter.  */
                    ppp_ptr -> nx_ppp_invalid_frame_id++;
#endif

                    /* Discard this request by simply returning!  */
                    return;
                }

                /* The peer has ACKed our configuration request. Move to the 
                   LCP completed state.  */
                ppp_ptr -> nx_ppp_lcp_state =  NX_PPP_LCP_COMPLETED_STATE;
            
                /* Determine if we need to update the IP's MTU.  */
                if (ppp_ptr -> nx_ppp_mru > (ppp_ptr -> nx_ppp_interface_ptr) -> nx_interface_ip_mtu_size)
                {

                    /* Yes, the peer can accept larger messages than the default.  */
                    (ppp_ptr -> nx_ppp_interface_ptr) -> nx_interface_ip_mtu_size =  ppp_ptr -> nx_ppp_mru;
                }

                /* Turn off the timeout for the configuration request.  */
                ppp_ptr -> nx_ppp_timeout =  0;
            }

            else if (code ==  NX_PPP_LCP_CONFIGURE_NAK)
            {
        
                /* Determine if the ID matches our last transmit ID. If not, just discard it.  */
                if (ppp_ptr -> nx_ppp_transmit_id != ppp_ptr -> nx_ppp_receive_id)
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the invalid frame ID counter.  */
                    ppp_ptr -> nx_ppp_invalid_frame_id++;
#endif

                    /* Discard this request by simply returning!  */
                    return;
                }

                /* Figure out what options were naked.  */
                _nx_ppp_lcp_nak_configure_list(ppp_ptr, ppp_ptr -> nx_ppp_naked_list);

                /* Send new configure request.  */
                _nx_ppp_lcp_configure_request_send(ppp_ptr);
            }

            else if (code == NX_PPP_LCP_TIMEOUT)
            {

                /* Increment the retry counter.  */
                ppp_ptr -> nx_ppp_protocol_retry_counter++;
 
                /* Determine if the LCP retry counter has been exceeded.  */
                if (ppp_ptr -> nx_ppp_protocol_retry_counter < NX_PPP_MAX_LCP_PROTOCOL_RETRIES)
                {

                    /* Setup the timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;

                    /* Timeout, send configuration request again.  */
                    _nx_ppp_lcp_configure_request_send(ppp_ptr); 
                }
                else
                {
                
                    /* Retry counter exceeded.  */

                    /* Enter LCP failed state.  PPP must be stopped and started to try again.  */
                    ppp_ptr -> nx_ppp_lcp_state =  NX_PPP_LCP_FAILED_STATE;

                    /* Determine if the application has registered a link down notification
                       callback.  */
                    if (ppp_ptr -> nx_ppp_link_down_callback)
                    {

                        /* Yes, call the application's callback function.  */
                        (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                    }
                }
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_lcp_state_machine_unhandled_requests++;
            }
#endif
            break;    
        }

        case NX_PPP_LCP_COMPLETED_STATE:
        {
    
            /* PPP is up and operational at this point.  */
            
            /* Process relative to incoming code.  */
            if (code == NX_PPP_LCP_TERMINATE_REQUEST)
            {

                /* ACK the terminate request.  */
                _nx_ppp_lcp_terminate_ack_send(ppp_ptr);

                /* Move to stopped state.  */
                ppp_ptr -> nx_ppp_lcp_state =  NX_PPP_LCP_STOPPED_STATE;

                /* Set the event to stop the PPP instance.  */
                tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_STOP, TX_OR);

                /* Determine if the application has registered a link down notification
                   callback.  */
                if (ppp_ptr -> nx_ppp_link_down_callback)
                {

                    /* Yes, call the application's callback function.  */
                    (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                }
            }
            else if ((code == NX_PPP_LCP_ECHO_REQUEST) && (packet_ptr))
            {
              
                /* Respond to the echo request. */
                _nx_ppp_lcp_ping_reply(ppp_ptr, packet_ptr);
            }

            else if ((code == NX_PPP_LCP_ECHO_REPLY) && (packet_ptr))
            {

                /* Check if the PPP instance is waiting on an echo reply. */
                if (ppp_ptr -> nx_ppp_lcp_echo_reply_id)
                {

                    /* It is. Check if this is a valid reply. */
                    _nx_ppp_lcp_ping_process_echo_reply(ppp_ptr, packet_ptr);
                }

                break;
            }

            /* In this state, we have received the ACK for our configuration request/completed LCP, but are expecting
               a configuration request from the peer.  */
    
            /* Process relative to the incoming code.  */
            else if ((code == NX_PPP_LCP_CONFIGURE_REQUEST) && (packet_ptr))
            {
        
                /* The peer has sent a configuration request.  */ 

                /* Retrieve configuration.  */
                status = _nx_ppp_lcp_configuration_retrieve(ppp_ptr, packet_ptr, ppp_ptr -> nx_ppp_peer_naked_list, ppp_ptr -> nx_ppp_rejected_list, &configure_status); 
 
                /* Discard invalid packet.  */
                if (status)
                {
                    return;
                }
                        
                /* Determine if the configuration request is fine or needs to be negotiated further.  */
                if (configure_status == 0)
                {


                    /* The peer's request is acceptable, move into the LCP done state.  */
                    ppp_ptr -> nx_ppp_lcp_state =  NX_PPP_LCP_COMPLETED_STATE;

                    /* Determine if we need to update the IP's MTU.  */
                    if (ppp_ptr -> nx_ppp_mru > (ppp_ptr -> nx_ppp_interface_ptr) -> nx_interface_ip_mtu_size)
                    {

                        /* Yes, the peer can accept larger messages than the default.  */
                        (ppp_ptr -> nx_ppp_interface_ptr) -> nx_interface_ip_mtu_size =  ppp_ptr -> nx_ppp_mru;
                    }
                }

                /* Send configuration reply.  */
                _nx_ppp_lcp_configure_reply_send(ppp_ptr, configure_status, lcp_message_ptr, ppp_ptr -> nx_ppp_peer_naked_list, ppp_ptr -> nx_ppp_rejected_list);    
            }
#ifndef NX_PPP_DISABLE_INFO
            else 
            {

                /* Increment the number of unhandled state machine events. */
                ppp_ptr -> nx_ppp_lcp_state_machine_unhandled_requests++;
            }
#endif
            break;
        }
        
        case NX_PPP_LCP_STOPPING_STATE:
        {
    
            /* We received a terminate request from the other side and just need to get the ACK.  */
            
            /* Process relative to incoming code.  */
            if (code == NX_PPP_LCP_TERMINATE_ACK)
            {

                /* Move to stopped state.  */
                ppp_ptr -> nx_ppp_lcp_state =  NX_PPP_LCP_STOPPED_STATE;

                /* Set the event to stop the PPP instance.  */
                tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_STOP, TX_OR);

                /* Determine if the application has registered a link down notification
                    callback.  */
                if (ppp_ptr -> nx_ppp_link_down_callback)
                {

                    /* Yes, call the application's callback function.  */
                    (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                }
            }        
            else if (code == NX_PPP_LCP_TIMEOUT)
            {

                /* Increment the retry counter.  */
                ppp_ptr -> nx_ppp_protocol_retry_counter++;
 
                /* Determine if the LCP retry counter has been exceeded.  */
                if (ppp_ptr -> nx_ppp_protocol_retry_counter < NX_PPP_MAX_LCP_PROTOCOL_RETRIES)
                {
 
                    /* Setup the timeout.  */
                   ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;
        
                    /* Send terminate request.  */
                    _nx_ppp_lcp_terminate_request_send(ppp_ptr);
                }
                else
                {
                
                    /* Retry counter exceeded.  */

                    /* Enter LCP failed state.  */
                    ppp_ptr -> nx_ppp_lcp_state =  NX_PPP_LCP_FAILED_STATE;

                    /* Set the event to stop the PPP instance.  */
                    tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_STOP, TX_OR);

                    /* Determine if the application has registered a link down notification
                       callback.  */
                    if (ppp_ptr -> nx_ppp_link_down_callback)
                    {

                        /* Yes, call the application's callback function.  */
                        (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                    }
                }
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

#ifndef NX_PPP_DISABLE_INFO
                if (code == NX_PPP_LCP_ECHO_REQUEST)
                {
                

                    ppp_ptr -> nx_ppp_lcp_echo_requests_dropped++;
                }
#endif

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_lcp_state_machine_unhandled_requests++;
            }
#endif
            break;
        }
        
        default:
        {

#ifndef NX_PPP_DISABLE_INFO
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_lcp_state_machine_unhandled_requests++;
            }
#endif
            break;
        }
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_code_reject                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds a code reject message and sends it out.        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    lcp_ptr                               Pointer to LCP message        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a packet for sending */ 
/*    _nx_ppp_packet_transmit               Send PPP packet               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_state_machine_update      LCP state machine processing  */ 
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
void  _nx_ppp_lcp_code_reject(NX_PPP *ppp_ptr, UCHAR *lcp_ptr)
{

UINT        i;
UINT        status;
UINT        length;
NX_PACKET   *packet_ptr;


    /* Allocate a packet for the PPP packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* An error was detected, simply return.  */
        return;
    }

    /* Build the configuration request.  */
    packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_LCP_PROTOCOL & 0xFF00) >> 8;
    packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_LCP_PROTOCOL & 0xFF;
    packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_LCP_CODE_REJECT;
    packet_ptr -> nx_packet_prepend_ptr[3] =  ppp_ptr -> nx_ppp_receive_id;

    /* Setup the length.  */
    packet_ptr -> nx_packet_prepend_ptr[4] =  lcp_ptr[4];
    packet_ptr -> nx_packet_prepend_ptr[5] =  lcp_ptr[5];

    length =  (((UINT) lcp_ptr[4]) << 8) | ((UINT) lcp_ptr[5]);

    /* Check if out of boundary.  */
    if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < (length + 2))
     {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Send the options that were received with RCR.  */
    for(i = 0; i < (length - 4); i++) 
    {

        /* Copy option byte into new packet.  */
        packet_ptr -> nx_packet_prepend_ptr[6+i] =  lcp_ptr[6+i];
    }

    /* Setup the packet length and append pointer.  */
    packet_ptr -> nx_packet_length =  length + 2;
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

#ifndef NX_PPP_DISABLE_INFO

    /* Increment the number of LCP frames sent.  */
    ppp_ptr -> nx_ppp_lcp_frames_sent++;
    
    /* Increment code rejects sent counter.  */
    ppp_ptr -> nx_ppp_lcp_code_rejects_sent++;
#endif

    /* Send code reject message.  */
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_configure_reply_send                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds and sends a configuration reply.               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    configure_status                      Status from configure request */ 
/*    lcp_ptr                               Pointer to LCP message        */ 
/*    naked_list                            List of NAKed options         */ 
/*    rejected_list                         List of rejected options      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a packet for sending */ 
/*    _nx_ppp_packet_transmit               Send PPP packet               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_state_machine_update      LCP state machine processing  */ 
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
void  _nx_ppp_lcp_configure_reply_send(NX_PPP *ppp_ptr, UINT configure_status, UCHAR *lcp_ptr, UCHAR *naked_list, UCHAR *rejected_list) 
{

UINT        i;
UINT        status;
UINT        length;
NX_PACKET   *packet_ptr;


    /* Allocate a packet for the PPP packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* An error was detected, simply return a NULL pointer.  */
        return;
    }

    /* Build the configuration reply.  */
    packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_LCP_PROTOCOL & 0xFF00) >> 8;
    packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_LCP_PROTOCOL & 0xFF;

    /* Process relative to the supplied status.  */
    if (configure_status == 0)
    {
        /* Send ACK.  */
        packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_LCP_CONFIGURE_ACK;

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of LCP ACKs sent.  */
        ppp_ptr -> nx_ppp_lcp_configure_acks_sent++;
#endif
    }
    else if (configure_status & 2)
    {
        /* Send reject.  */
        packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_LCP_CONFIGURE_REJECT;

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of LCP rejects sent.  */
        ppp_ptr -> nx_ppp_lcp_configure_rejects_sent++;
#endif
    }
    else
    {
        /* Send NAK.  */
        packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_LCP_CONFIGURE_NAK;

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of LCP NAKs sent.  */
        ppp_ptr -> nx_ppp_lcp_configure_naks_sent++;
#endif
    }
    
    /* Insert the id.  */
    packet_ptr -> nx_packet_prepend_ptr[3] =  ppp_ptr -> nx_ppp_receive_id;

    /* Process again according to the status.  */
    if (configure_status == 0)
    {

        /* Setup the options list.  */

        /* Setup the length.  */
        packet_ptr -> nx_packet_prepend_ptr[4] =  lcp_ptr[4];
        packet_ptr -> nx_packet_prepend_ptr[5] =  lcp_ptr[5];

        length =  (((UINT) lcp_ptr[4]) << 8) | ((UINT) lcp_ptr[5]);

        /* Check if out of boundary.  */
        if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < (length + 2))
         {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);
            return;
        }

        /* Send the options that were received with the request.  */
        for(i = 0; i < (length - 4); i++) 
        {

            /* Copy option byte into new packet.  */
            packet_ptr -> nx_packet_prepend_ptr[6+i] =  lcp_ptr[6+i];
        }

        /* Setup the packet length and append pointer.  */
        packet_ptr -> nx_packet_length =  length + 2;
        packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;
    }
    else if (configure_status & 2) 
    {

        /* Rejected options.  */

        /* Setup the length.  */
        packet_ptr -> nx_packet_prepend_ptr[4] =  0;
        packet_ptr -> nx_packet_prepend_ptr[5] =  (UCHAR)(rejected_list[0] + 4);

        /* Check if out of boundary.  */
        if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < (UINT)(rejected_list[0] + 6))
         {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);
            return;
        }

        /* Load entire rejected list.  */
        for(i = 1; i < (UCHAR)(rejected_list[0] + 1); i++)
        {
            /* Copy option byte into new packet.  */
            packet_ptr -> nx_packet_prepend_ptr[6+i-1] =  rejected_list[i];
        }

        /* Setup the packet length and append pointer.  */
        packet_ptr -> nx_packet_length =  (ULONG)(rejected_list[0] + 6);
        packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;
    }
    else 
    {
    
        /* NAKed options.  */

        /* Setup the length.  */
        packet_ptr -> nx_packet_prepend_ptr[4] =  0;
        packet_ptr -> nx_packet_prepend_ptr[5] =  (UCHAR)(naked_list[0] + 4);

        /* Check if out of boundary.  */
        if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < (UINT)(naked_list[0] + 6))
         {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);
            return;
        }

        /* Load entire naked list.  */
        for(i = 1; i < (UCHAR)(naked_list[0] + 1); i++)
        {
            /* Copy option byte into new packet.  */
            packet_ptr -> nx_packet_prepend_ptr[6+i-1] =  naked_list[i];
        }

        /* Setup the packet length and append pointer.  */
        packet_ptr -> nx_packet_length =  (ULONG)(naked_list[0] + 6);
        packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;
    }

#ifndef NX_PPP_DISABLE_INFO

    /* Increment the number of LCP frames sent.  */
    ppp_ptr -> nx_ppp_lcp_frames_sent++;
#endif

    /* Send the reply out.  */
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_configure_request_send                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds and sends a LCP configuration request.         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate PPP packet           */ 
/*    _nx_ppp_packet_transmit               Send LCP packet               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_state_machine_update      LCP state machine processing  */ 
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
void  _nx_ppp_lcp_configure_request_send(NX_PPP *ppp_ptr)
{
    
UINT        status;
NX_PACKET   *packet_ptr;


    /* Allocate a packet for the PPP packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* An error was detected, simply return a NULL pointer.  */
        return;
    }

    /* Increment the transmit ID.  */
    ppp_ptr -> nx_ppp_transmit_id++;

    /* Build the configuration request.  */
    packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_LCP_PROTOCOL & 0xFF00) >> 8;
    packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_LCP_PROTOCOL & 0xFF;
    packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_LCP_CONFIGURE_REQUEST;
    packet_ptr -> nx_packet_prepend_ptr[3] =  ppp_ptr -> nx_ppp_transmit_id;
    packet_ptr -> nx_packet_prepend_ptr[4] =  0;

    /* Load the MRU.  */
    packet_ptr -> nx_packet_prepend_ptr[6] =  1;
    packet_ptr -> nx_packet_prepend_ptr[7] =  4;
    packet_ptr -> nx_packet_prepend_ptr[8] =  (UCHAR) ((NX_PPP_MRU) >> 8);
    packet_ptr -> nx_packet_prepend_ptr[9] =  (UCHAR) ((NX_PPP_MRU) & 0xff);

    /* Load the authentication protocol type. */
    if ((ppp_ptr -> nx_ppp_verify_authentication_protocol == NX_PPP_PAP_PROTOCOL) && (ppp_ptr -> nx_ppp_pap_verify_login))
    {

        /* Set the length for PAP authentication protocol.  */
        packet_ptr -> nx_packet_prepend_ptr[5] =   12;

        packet_ptr -> nx_packet_prepend_ptr[10] =  3;
        packet_ptr -> nx_packet_prepend_ptr[11] =  4;
        packet_ptr -> nx_packet_prepend_ptr[12] =  (NX_PPP_PAP_PROTOCOL & 0xFF00) >> 8;
        packet_ptr -> nx_packet_prepend_ptr[13] =  NX_PPP_PAP_PROTOCOL & 0xFF;
    }
    else if ((ppp_ptr -> nx_ppp_verify_authentication_protocol == NX_PPP_CHAP_PROTOCOL) && 
             (ppp_ptr -> nx_ppp_chap_get_challenge_values) && (ppp_ptr -> nx_ppp_chap_get_verification_values))
    {

        /* Set the length for CHAP authentication protocol.  */
        packet_ptr -> nx_packet_prepend_ptr[5] =   13;

        packet_ptr -> nx_packet_prepend_ptr[10] =  3;
        packet_ptr -> nx_packet_prepend_ptr[11] =  5;
        packet_ptr -> nx_packet_prepend_ptr[12] =  (NX_PPP_CHAP_PROTOCOL & 0xFF00) >> 8;
        packet_ptr -> nx_packet_prepend_ptr[13] =   NX_PPP_CHAP_PROTOCOL & 0xFF;
        packet_ptr -> nx_packet_prepend_ptr[14] =  0x05;
    }
    else
    {

        /* Set the length for no authentication protocol.  */
        packet_ptr -> nx_packet_prepend_ptr[5] =  8;
    }

    /* Setup the append pointer and the packet length (LCP length + PPP header).  */
    packet_ptr -> nx_packet_length =  (ULONG)(packet_ptr -> nx_packet_prepend_ptr[5] + 2);
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

#ifndef NX_PPP_DISABLE_INFO

    /* Increment the number of LCP frames sent.  */
    ppp_ptr -> nx_ppp_lcp_frames_sent++;
    
    /* Increment the number of LCP configure requests sent.  */
    ppp_ptr -> nx_ppp_lcp_configure_requests_sent++;
#endif

    /* Send the configure request packet.  */
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_configuration_retrieve                  PORTABLE C      */ 
/*                                                           6.1.2        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function pickup the configuration options. Unhandled options   */ 
/*    are placed in NAKed or Rejected lists.                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    naked_list                            List of NAKed options         */ 
/*    rejected_list                         List of rejected options      */ 
/*    configure_status                      Returned configration status: */
/*                                            0 -> Success                */ 
/*                                            1 -> NAKed options          */ 
/*                                            2 -> Rejected options       */ 
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
/*    _nx_ppp_lcp_state_machine_update      LCP state machine processing  */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  11-09-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            improved packet length      */
/*                                            verification,               */
/*                                            resulting in version 6.1.2  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ppp_lcp_configuration_retrieve(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr, UCHAR *naked_list, UCHAR *rejected_list, UINT *configure_status)
{

UINT    option_index, nak_list_index, rejected_list_index;
UINT    len;
UINT    type;
UINT    counter;
ULONG   authentication_protocol;
UCHAR   *option_data;


    /* Initialize the configure status.  */
    *configure_status = 0;

    /* Clear both the NAKed and rejected list length.   */
    naked_list[0] =     0; 
    rejected_list[0] =  0;
 
    /* Start indexes at 1, since byte 0 contains length.  */
    nak_list_index =       1;
    rejected_list_index =  1;

    /* Process the options in the LCP message.  */
    for (option_index = 6; (option_index + 2) <= packet_ptr -> nx_packet_length; )
    {

        /* Pickup the option type - section 6 of RFC1661.  */
        type = packet_ptr -> nx_packet_prepend_ptr[option_index++];

        /* Get the length of the option. The length also includes the type and length fields.  */
        len =  packet_ptr -> nx_packet_prepend_ptr[option_index++];

        /* Check if the length is valid.  */
        if ((len < 2) || (len > (packet_ptr -> nx_packet_length - (option_index - 2))))
        {
            return(NX_PPP_BAD_PACKET);
        }

        /* Set a pointer to option data.  */
        option_data = &packet_ptr -> nx_packet_prepend_ptr[option_index];

        /* Advance the index to next option.  */
        option_index +=  len - 2;

        /* Process relative to the option type.  */
        switch (type)
        {

        case 1: 

            /* Maximum Receive Unit (MRU) option.  */
            ppp_ptr -> nx_ppp_mru =  (ULONG)((((USHORT) option_data[0]) << 8) | ((USHORT) option_data[1]));
    
            /* Determine if the MRU is too small.  */
            if (ppp_ptr -> nx_ppp_mru < NX_PPP_MINIMUM_MRU)
            {
                *configure_status |=  1;

                /* Default the MRU.  */
                ppp_ptr -> nx_ppp_mru =  NX_PPP_MRU;

                /* Check if out of boundary.  */
                if ((nak_list_index + len) > NX_PPP_OPTION_MESSAGE_LENGTH)
                    break;

                /* Yes, enter it in the NAK list.  */
                naked_list[nak_list_index++] =  (UCHAR)type; 
                naked_list[nak_list_index++] =  (UCHAR)len; 
                for (counter = 0; counter < (len-2); counter++) 
                    naked_list[nak_list_index++] = option_data[counter]; 
                naked_list[0] = (UCHAR)(nak_list_index - 1);
            }
            break;
            
        case 3:
        
            /* Authentication protocol selection.  */

            /* Pickup the authentication protocol.  */
            authentication_protocol =  (((UINT) option_data[0]) << 8) | ((UINT) option_data[1]);

            /* Determine if the authentication protocol specified by the peer is PAP and we have a generate
               routine defined.  */
            if ((authentication_protocol == NX_PPP_PAP_PROTOCOL) && (ppp_ptr -> nx_ppp_pap_generate_login))
            {
            
                /* Yes, enable the PAP protocol.  */
                ppp_ptr -> nx_ppp_generate_authentication_protocol =  NX_PPP_PAP_PROTOCOL;
            }

            /* Determine if the authentication protocol specified by the peer is CHAP and we have a generate
               response routine defined.  */
            if ((authentication_protocol == NX_PPP_CHAP_PROTOCOL) && (ppp_ptr -> nx_ppp_chap_get_responder_values))
            {
            
                /* Yes, enable the CHAP protocol.  */
                ppp_ptr -> nx_ppp_generate_authentication_protocol =  NX_PPP_CHAP_PROTOCOL;
            }
            
            /* Determine if the required authentication at the peer is the same as what this peer can generate
               login information for.  */
            if (ppp_ptr -> nx_ppp_generate_authentication_protocol != authentication_protocol)
            {

                /* We do not support the requested authentication by the peer.  */

                /* Check to see if we don't have any authentication protocols enabled.  */
                if (ppp_ptr -> nx_ppp_generate_authentication_protocol == 0)
                {
                    *configure_status |=  2; 

                    /* Check if out of boundary.  */
                    if ((rejected_list_index + len) > NX_PPP_OPTION_MESSAGE_LENGTH)
                        break;

                    /* No authentication is supported, simply include requested authentication 
                       option in the rejected list.  */
                    rejected_list[rejected_list_index++] =  (UCHAR)type; 
                    rejected_list[rejected_list_index++] =  (UCHAR)len; 
                    for (counter = 0; counter < (len-2); counter++) 
                        rejected_list[rejected_list_index++] = option_data[counter]; 
                    rejected_list[0] = (UCHAR)(rejected_list_index - 1);
                }

                /* Determine if this peer has PAP enabled.  */
                if (ppp_ptr -> nx_ppp_generate_authentication_protocol == NX_PPP_PAP_PROTOCOL)
                {
                    *configure_status |=  1; 

                    /* Check if out of boundary.  */
                    if ((nak_list_index + 4) > NX_PPP_OPTION_MESSAGE_LENGTH)
                        break;

                    /* PAP enabled but something different is requested by the peer, build NAK entry with PAP hint.  */
                    naked_list[nak_list_index++] =  3; 
                    naked_list[nak_list_index++] =  NX_PPP_PAP_AUTHENTICATE_NAK; 
                    naked_list[nak_list_index++] =  (NX_PPP_PAP_PROTOCOL & 0xFF00) >> 8;
                    naked_list[nak_list_index++] =  NX_PPP_PAP_PROTOCOL & 0xFF;
                    naked_list[0] = (UCHAR)(nak_list_index - 1);

                    /* Clear authentication protocol.  */
                    ppp_ptr -> nx_ppp_verify_authentication_protocol =  0;
                }

                /* Determine if this peer has CHAP enabled.  */
                if (ppp_ptr -> nx_ppp_generate_authentication_protocol == NX_PPP_CHAP_PROTOCOL)
                {
                    *configure_status |=  1;

                    /* Check if out of boundary.  */
                    if ((nak_list_index + 5) > NX_PPP_OPTION_MESSAGE_LENGTH)
                        break;

                    /* CHAP enabled but something different is requested by the peer, build NAK entry with CHAP hint.  */
                    naked_list[nak_list_index++] =  3; 
                    naked_list[nak_list_index++] =  5; 
                    naked_list[nak_list_index++] =  (NX_PPP_CHAP_PROTOCOL & 0xFF00) >> 8;
                    naked_list[nak_list_index++] =  NX_PPP_CHAP_PROTOCOL & 0xFF;
                    naked_list[nak_list_index++] =  0x05; 
                    naked_list[0] = (UCHAR)(nak_list_index - 1);
                }
            }
            else
            {
            
                /* Matching authentication protocol.  */
            
                /* Check to see if we have the CHAP authentication protocol enabled.  */
                if (ppp_ptr -> nx_ppp_generate_authentication_protocol == NX_PPP_CHAP_PROTOCOL)
                {

                    /* Now determine if something other than CHAP MD5 was requested.  */
                    if (option_data[2] != 0x05)
                    {
                        *configure_status |=  1; 

                        /* Check if out of boundary.  */
                        if ((nak_list_index + 5) > NX_PPP_OPTION_MESSAGE_LENGTH)
                            break;

                        /* CHAP enabled but something different is requested by the peer, build NAK entry with CHAP hint.  */
                        naked_list[nak_list_index++] =  3; 
                        naked_list[nak_list_index++] =  5; 
                        naked_list[nak_list_index++] =  (NX_PPP_CHAP_PROTOCOL & 0xFF00) >> 8;
                        naked_list[nak_list_index++] =  NX_PPP_CHAP_PROTOCOL & 0xFF;
                        naked_list[nak_list_index++] =  0x05; 
                        naked_list[0] = (UCHAR)(nak_list_index - 1);
                    }
                    else
                    {
                    
                        /* Clear the authenticated flag since peer requires CHAP authentication.  */
                        ppp_ptr -> nx_ppp_authenticated =  NX_FALSE;        
                    }
                }
                else
                {
                
                    /* Clear the authenticated flag since peer requires PAP authentication.  */
                    ppp_ptr -> nx_ppp_authenticated =  NX_FALSE;        
                }
            }
            break;

        case 2: /* ACCM, Not implemented.  */
        case 5: /* Magic number - just acknowledge, we do not send out magic number.  */
        case 7: /* The sender can receive protocol compression.  */
        case 8: /* The sender can receive address compression.  */

            /* Skip these options, since we don't care to object.  */              
            break;
            
        default: 

            *configure_status |=  2; 

            /* Check if out of boundary.  */
            if ((rejected_list_index + len) > NX_PPP_OPTION_MESSAGE_LENGTH)
                break;

            /* All other options we must reject since we do not negotiate.  */
            rejected_list[rejected_list_index++] =  (UCHAR)type; 
            rejected_list[rejected_list_index++] =  (UCHAR)len; 
            for (counter = 0; counter < (len-2); counter++) 
                rejected_list[rejected_list_index++] = option_data[counter];
            rejected_list[0] = (UCHAR)(rejected_list_index - 1);
            break;
        }
    }

    /* Check if packet length is valid.  */
    if (option_index != packet_ptr -> nx_packet_length)
    {
        return(NX_PPP_BAD_PACKET);
    }

    /* Return status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_nak_configure_list                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes the NAKed option list.                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    naked_list                            List of NAKed options         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_state_machine_update      LCP state machine processing  */ 
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
void  _nx_ppp_lcp_nak_configure_list(NX_PPP *ppp_ptr, UCHAR *naked_list)
{

UINT    i;

    NX_PARAMETER_NOT_USED(ppp_ptr);

    /* Currently, nothing needs to be done in this routine since the options
       we are asking for are basic and must be supported. If additional options
       are added, we will need to process the NAK list.  */

    /* Just clear the naked list for now.  */
    for(i = 0; i < NX_PPP_OPTION_MESSAGE_LENGTH; i++)
        naked_list[i] = 0; 
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_terminate_ack_send                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds a terminate ACK LCP message and sends it out.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a packet for sending */ 
/*    _nx_ppp_packet_transmit               Send AHDLC packet             */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_state_machine_update      LCP state machine processing  */ 
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
void  _nx_ppp_lcp_terminate_ack_send(NX_PPP *ppp_ptr)
{
    
UINT        status;
NX_PACKET   *packet_ptr;


    /* Allocate a packet for the PPP packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* An error was detected, simply return a NULL pointer.  */
        return;
    }

    /* Build terminate ACK LCP message.  */
    packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_LCP_PROTOCOL & 0xFF00) >> 8;
    packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_LCP_PROTOCOL & 0xFF;
    packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_LCP_TERMINATE_ACK;
    packet_ptr -> nx_packet_prepend_ptr[3] =  ppp_ptr -> nx_ppp_receive_id;
    packet_ptr -> nx_packet_prepend_ptr[4] =  0;
    packet_ptr -> nx_packet_prepend_ptr[5] =  4;

    /* Setup the append pointer and the packet length.  */
    packet_ptr -> nx_packet_length =  6;
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

#ifndef NX_PPP_DISABLE_INFO

    /* Increment the number of LCP frames sent.  */
    ppp_ptr -> nx_ppp_lcp_frames_sent++;

    /* Increment the number of LCP terminate ACKs sent.  */
    ppp_ptr -> nx_ppp_lcp_terminate_acks_sent++;
#endif

    /* Send the packet out.  */
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_terminate_request_send                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds a LCP terminate message and sends it out.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a packet for sending */ 
/*    _nx_ppp_packet_transmit               Send PPP packet               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_state_machine_update      LCP state machine processing  */ 
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
void  _nx_ppp_lcp_terminate_request_send(NX_PPP *ppp_ptr)
{

UINT        status;
NX_PACKET   *packet_ptr;

    /* Allocate a packet for the PPP packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* An error was detected, simply return a NULL pointer.  */
        return;
    }

    /* Build terminate request message.  */
    packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_LCP_PROTOCOL & 0xFF00) >> 8;
    packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_LCP_PROTOCOL & 0xFF;
    packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_LCP_TERMINATE_REQUEST;
    packet_ptr -> nx_packet_prepend_ptr[3] =  ppp_ptr -> nx_ppp_transmit_id;
    packet_ptr -> nx_packet_prepend_ptr[4] =  0;
    packet_ptr -> nx_packet_prepend_ptr[5] =  4;

    /* Setup the append pointer and the packet length.  */
    packet_ptr -> nx_packet_length =  6;
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

#ifndef NX_PPP_DISABLE_INFO

    /* Increment the number of LCP frames sent.  */
    ppp_ptr -> nx_ppp_lcp_frames_sent++;

    /* Increment the number of LCP terminate requests sent.  */
    ppp_ptr -> nx_ppp_lcp_terminate_requests_sent++;
#endif

    /* Send terminate request.  */
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);
}


#ifndef NX_PPP_DISABLE_PAP
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_pap_state_machine_update                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes events and state changes in the PPP PAP     */ 
/*    state machine.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    packet_ptr                            Pointer to PAP packet         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_pap_login_valid               Check for valid peer login    */ 
/*    _nx_ppp_pap_authentication_request    Send authentication request   */ 
/*    _nx_ppp_pap_authentication_ack        Send authentication ACK       */ 
/*    _nx_ppp_pap_authentication_nak        Send authentication NAK       */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_receive_packet_process        Receive PPP packet processing */ 
/*    _nx_ppp_timeout                       PPP timeout                   */ 
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
void _nx_ppp_pap_state_machine_update(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr)
{

UCHAR       code;
UINT        valid;


    /* Determine if a packet is present. If so, drive the event from the packet.  */
    if (packet_ptr)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of PAP frames received.  */
        ppp_ptr -> nx_ppp_pap_frames_received++;
#endif

        /* Pickup the type of received PAP code.  */
        code =  packet_ptr -> nx_packet_prepend_ptr[2];

        /* Remember receive id.  */
        ppp_ptr -> nx_ppp_receive_id =  packet_ptr -> nx_packet_prepend_ptr[3];

        /* Check the incoming code.  */
        if ((code < NX_PPP_PAP_AUTHENTICATE_REQUEST) || (code > NX_PPP_PAP_AUTHENTICATE_NAK))
        {

#ifndef NX_PPP_DISABLE_INFO

            /* Increment the number of internal errors.  */
            ppp_ptr -> nx_ppp_internal_errors++;
    
            /* Increment the PAP unknown requests counter.  */
            ppp_ptr -> nx_ppp_pap_unknown_requests_received++;
#endif
            return;
        }
    }
    else
    {
    
        /* Set the code to timeout.  */
        code =   NX_PPP_PAP_AUTHENTICATE_TIMEOUT;
    }

#ifndef NX_PPP_DISABLE_INFO

    /* Increment the appropriate counter.  */
    switch (code)
    {
    
    case NX_PPP_PAP_AUTHENTICATE_REQUEST:
    
        /* Increment the authenticate requests counter.  */
        ppp_ptr -> nx_ppp_pap_authenticate_requests_received++;
        break;
        
    case NX_PPP_PAP_AUTHENTICATE_ACK:
    
        /* Increment the authenticate acks counter.  */
        ppp_ptr -> nx_ppp_pap_authenticate_acks_received++;
        break;
        
    case NX_PPP_PAP_AUTHENTICATE_NAK:
    
        /* Increment the authenticate naks counter.  */
        ppp_ptr -> nx_ppp_pap_authenticate_naks_received++;
        break;
    
    case NX_PPP_PAP_AUTHENTICATE_TIMEOUT:
    
        /* Determine if we are in the initial state. If so, this really isn't a timeout.  */
        if (ppp_ptr -> nx_ppp_pap_state != NX_PPP_PAP_START_STATE)
        {

            /* Increment the authenticate timeout counter.  */
            ppp_ptr -> nx_ppp_pap_state_machine_timeouts++;
        }
        break;
    }
#endif

    /* Process relative to the current state.  */
    switch (ppp_ptr -> nx_ppp_pap_state)
    {
    
        /* Starting state.  */
        case NX_PPP_PAP_START_STATE:
        {

            /* Determine if we need to generate a login for the peer to verify.  */
            if (ppp_ptr -> nx_ppp_generate_authentication_protocol)
            {

                /* Setup the retry counter.  */
                ppp_ptr -> nx_ppp_protocol_retry_counter =  0;
 
                /* Setup the timeout.  */
                ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;

                /* Generate PAP login request and send to the peer.  */
                _nx_ppp_pap_authentication_request(ppp_ptr);         
                
                /* Move to the authenticate request sent state.  */
                ppp_ptr -> nx_ppp_pap_state =  NX_PPP_PAP_AUTHENTICATE_REQUEST_SENT_STATE;
            }
            else
            {
            
                /* Move to the authenticate wait state.  */
                ppp_ptr -> nx_ppp_pap_state =  NX_PPP_PAP_AUTHENTICATE_REQUEST_WAIT_STATE;
            }
            break;
        }

        case NX_PPP_PAP_AUTHENTICATE_REQUEST_SENT_STATE:
        {

            /* In this state, this peer has sent an authentication request and is waiting for 
               an ACK or NAK from the peer.  */
            if ((code == NX_PPP_PAP_AUTHENTICATE_REQUEST) && (packet_ptr))
            {

                /* Determine if the login information is valid.  */
                valid =  _nx_ppp_pap_login_valid(ppp_ptr, packet_ptr);          
                
                /* Is the login valid?  */
                if (valid == NX_TRUE)
                {
                
                    /* Send an ACK message.  */
                    _nx_ppp_pap_authentication_ack(ppp_ptr); 
                    
                    /* Now determine if we need to authenticate ourselves.  */
                    if (ppp_ptr -> nx_ppp_generate_authentication_protocol != NX_PPP_PAP_PROTOCOL)
                    {
                        
                        /* We do not need to authenticate ourselves, so we can move into PAP completed state.  */
                        ppp_ptr ->  nx_ppp_pap_state =  NX_PPP_PAP_COMPLETED_STATE;
                        
                        /* Mark the PPP instance as authenticated.  */
                        ppp_ptr -> nx_ppp_authenticated =  NX_TRUE;

                        /* Turn off the timeout.  */
                        ppp_ptr -> nx_ppp_timeout =  0;
                    }
                }
                else
                {
                
                    /* Send NAK to tell peer the authentication failed.  */
                    _nx_ppp_pap_authentication_nak(ppp_ptr); 
                    
                    /* No state change, just stay here!  */
                }
            }
            
            /* Was a NAK received?  */
            else if (code == NX_PPP_PAP_AUTHENTICATE_NAK)
            {

                /* Determine if there is an application NAK callback.  */
                if (ppp_ptr -> nx_ppp_nak_authentication_notify)
                {

                    /* Yes, call the application's authentication NAK notify callback function.  */
                    (ppp_ptr -> nx_ppp_nak_authentication_notify)();
                }

                /* Generate a new PAP login request and send to the peer.  */
                _nx_ppp_pap_authentication_request(ppp_ptr);         
            }

            /* Was an ACK received?  */
            else if (code == NX_PPP_PAP_AUTHENTICATE_ACK)
            {
            
                /* Determine if this peer requires authentication.  */
                if (ppp_ptr -> nx_ppp_pap_verify_login)
                {
                
                    /* Yes, we require PAP verification so move to the request wait state.  */
                    ppp_ptr -> nx_ppp_pap_state =  NX_PPP_PAP_AUTHENTICATE_REQUEST_WAIT_STATE;
                }
                else
                {
                
                    /* Otherwise, we have completed the PAP authentication. Move to the completed
                       state. */
                    ppp_ptr -> nx_ppp_pap_state =  NX_PPP_PAP_COMPLETED_STATE;

                    /* Mark the PPP instance as authenticated.  */
                    ppp_ptr -> nx_ppp_authenticated =  NX_TRUE;

                    /* Turn off the timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  0;
                }
            }
            
            /* Was a timeout received?  */
            else if (code == NX_PPP_PAP_AUTHENTICATE_TIMEOUT)
            {
            
                /* Increment the retry counter.  */
                ppp_ptr -> nx_ppp_protocol_retry_counter++;
 
                /* Determine if the PAP retry counter has been exceeded.  */
                if (ppp_ptr -> nx_ppp_protocol_retry_counter < NX_PPP_MAX_PAP_PROTOCOL_RETRIES)
                {

                    /* Setup the timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;

                    /* Generate a new PAP login request and send to the peer.  */
                    _nx_ppp_pap_authentication_request(ppp_ptr);         
                }
                else
                {

                    /* Retry counter exceeded.  */

                    /* Enter PAP failed state.  PPP must be stopped and started to try again.  */
                    ppp_ptr -> nx_ppp_pap_state =  NX_PPP_PAP_FAILED_STATE;

                    /* Determine if the application has registered a link down notification
                       callback.  */
                    if (ppp_ptr -> nx_ppp_link_down_callback)
                    {

                        /* Yes, call the application's callback function.  */
                        (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                    }
                }
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_pap_state_machine_unhandled_requests++;
            }
#endif

            break;
        }

        case NX_PPP_PAP_AUTHENTICATE_REQUEST_WAIT_STATE:
        {

            /* In this state, this peer must send an authenticate request.  There is no
               authentication required by the peer.  */
            if ((code == NX_PPP_PAP_AUTHENTICATE_REQUEST) && (packet_ptr))
            {

                /* Determine if the login information is valid.  */
                valid =  _nx_ppp_pap_login_valid(ppp_ptr, packet_ptr);          
                
                /* Is the login valid?  */
                if (valid == NX_TRUE)
                {
                
                    /* Send an ACK message.  */
                    _nx_ppp_pap_authentication_ack(ppp_ptr); 
                    
                    /* No authentication is required by peer, so we can move into PAP completed state.  */
                    ppp_ptr ->  nx_ppp_pap_state =  NX_PPP_PAP_COMPLETED_STATE;

                    /* Mark the PPP instance as authenticated.  */
                    ppp_ptr -> nx_ppp_authenticated =  NX_TRUE;

                    /* Turn off the timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  0;
                }
                else
                {
                
                    /* Send NAK to tell peer the authentication failed.  */
                    _nx_ppp_pap_authentication_nak(ppp_ptr); 
                    
                    /* No state change, just stay here!  */
                }
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_pap_state_machine_unhandled_requests++;
            }
#endif
        break;
    }
    
    default:
#ifndef NX_PPP_DISABLE_INFO
        {

            /* Increment the number of unhandled state machine events.   */
            ppp_ptr -> nx_ppp_pap_state_machine_unhandled_requests++;
        }
#endif
        break;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_pap_authentication_request                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds and sends an authentication request message    */ 
/*    the peer. The peer will then either ACK or NAK the request.         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet               */ 
/*    (nx_ppp_pap_generate_login)           Pickup name and password      */ 
/*    _nx_ppp_packet_transmit               Send PAP response             */ 
/*    _nx_utility_string_length_check       Check string length           */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_pap_state_machine_update      PPP state machine update      */ 
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
void _nx_ppp_pap_authentication_request(NX_PPP *ppp_ptr)
{

UINT        length, password_length, i, j;
UCHAR       name[NX_PPP_NAME_SIZE + 1];
UCHAR       password[NX_PPP_PASSWORD_SIZE + 1];
UINT        status;
NX_PACKET   *packet_ptr;


    /* Initialize name and password.  */
    name[0] =  0;
    password[0] =  0;

    /* Determine if there is a login name/password generation routine.  */
    if (ppp_ptr -> nx_ppp_pap_generate_login)
    {

        /* Get the name and password */
        ppp_ptr -> nx_ppp_pap_generate_login((CHAR *) name, (CHAR *) password);
    }

    /* Allocate a packet for the PPP PAP response packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* An error was detected, simply return a NULL pointer.  */
        return;
    }

    /* Calculate the size of the name and password.  */            
    if (_nx_utility_string_length_check((CHAR *)name, &length, NX_PPP_NAME_SIZE) ||
        _nx_utility_string_length_check((CHAR *)password, &password_length, NX_PPP_PASSWORD_SIZE))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Check if out of boundary.  */
    if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < (UINT)(length + 1 + password_length + 1 + 6))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Set PAP authentication request, ID, and length.  */
    packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_PAP_PROTOCOL & 0xFF00) >> 8;
    packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_PAP_PROTOCOL & 0xFF;
    packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_PAP_AUTHENTICATE_REQUEST;
    packet_ptr -> nx_packet_prepend_ptr[3] =  ppp_ptr -> nx_ppp_transmit_id; /* ID */

    /* Setup the length.  */
    packet_ptr -> nx_packet_prepend_ptr[4] =  (UCHAR) (((length) + 1 + (password_length) + 1 + 4) >> 8);
    packet_ptr -> nx_packet_prepend_ptr[5] =  (UCHAR) (((length) + 1 + (password_length) + 1 + 4) & 0xFF);

    /* Store the PAP name.  */
    packet_ptr -> nx_packet_prepend_ptr[6] =  (UCHAR)(length & 0xFF);
    for (i = 0; i < length; i++)
    {

        /* Store byte of name.  */
        packet_ptr -> nx_packet_prepend_ptr[i+7] =  name[i];
    }

    /* Store PAP password.  */
    packet_ptr -> nx_packet_prepend_ptr[i+7] =  (password_length & 0xFF);
    for (j = 0; j < password_length; j++)
    {

        /* Store byte of name.  */
        packet_ptr -> nx_packet_prepend_ptr[j+i+8] =  password[j];
    }

    /* Setup the append pointer and the packet length.  */
    packet_ptr -> nx_packet_length =  (length) + 1 + (password_length) + 1 + 4 + 2;
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

#ifndef NX_PPP_DISABLE_INFO
    
    /* Increment the number of PAP frames sent.  */
    ppp_ptr -> nx_ppp_pap_frames_sent++;
    
    /* Increment the authentication requests sent counter.  */
    ppp_ptr -> nx_ppp_pap_authenticate_requests_sent++;
#endif
            
    /* Send the PAP request.  */
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_pap_login_valid                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function validates the login information supplied by the       */ 
/*    peer.                                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    packet_ptr                            Pointer to PAP packet         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_TRUE                               Valid login                   */ 
/*    NX_FALSE                              Invalid login                 */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    (nx_ppp_pap_verify_login)             Verify name and password      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_pap_state_machine_update      PAP state machine update      */ 
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
UINT _nx_ppp_pap_login_valid(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr)
{

UCHAR   length, password_length, i, j;
UCHAR   name[NX_PPP_NAME_SIZE + 1];
UCHAR   password[NX_PPP_PASSWORD_SIZE + 1];
UINT    status;


    /* Get the name length.  */
    length =  packet_ptr -> nx_packet_prepend_ptr[6];

    /* Check for valid packet length.  */
    if ((ULONG)(length + 7) > packet_ptr -> nx_packet_length)
    {
        return(NX_FALSE);
    }

    /* Determine if the length is greater than the name size.  */
    if (length > NX_PPP_NAME_SIZE)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of internal errors.  */
        ppp_ptr -> nx_ppp_internal_errors++;
#endif
        return(NX_FALSE);
    }

    /* Get the name.  */
    for(i = 0; i < length; i++)
    {

        /* Get a character of the name.  */
        name[i] =  packet_ptr -> nx_packet_prepend_ptr[i+7];
    }

    /* Null terminate the name.  */
    name[i] = 0;

    /* Get length of password.  */
    password_length =  packet_ptr -> nx_packet_prepend_ptr[i+7];

    /* Check for valid packet length.  */
    if ((ULONG)(password_length + i + 8) > packet_ptr -> nx_packet_length)
    {
        return(NX_FALSE);
    }

    /* Determine if the length is greater than the password size.  */
    if (password_length > NX_PPP_PASSWORD_SIZE)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of internal errors.  */
        ppp_ptr -> nx_ppp_internal_errors++;
#endif

        return(NX_FALSE);
    }
        
    /* Get the password.  */
    for(j = 0; j < password_length; j++)
    {

        /* Get a character of the password.  */
        password[j] =  packet_ptr -> nx_packet_prepend_ptr[j+i+8];
    }

    /* Null terminate the password.  */
    password[j] = 0;

    /* Determine if there is an authentication routine.  */
    if (ppp_ptr -> nx_ppp_pap_verify_login)
    {

        /* Call the user supplied PAP authentication routine.  */
        status =  ppp_ptr -> nx_ppp_pap_verify_login((CHAR *) name, (CHAR *) password);
        
        /* Check to see if it is valid.  */
        if (status)
        {
        
            /* Return NX_FALSE, which indicates the password is invalid.  */
            return (NX_FALSE);
        }
    }

    /* Return NX_TRUE.  */
    return(NX_TRUE);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_pap_authentication_ack                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds and sends an authentication ACK message to     */ 
/*    the peer in response to the peer's authentication request.          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet               */ 
/*    _nx_ppp_packet_transmit               Send PAP response             */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_pap_state_machine_update      PPP state machine update      */ 
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
void  _nx_ppp_pap_authentication_ack(NX_PPP *ppp_ptr)
{

UINT        status;
NX_PACKET   *packet_ptr;


    /* Allocate a packet for the PPP PAP response packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* An error was detected, simply return a NULL pointer.  */
        return;
    }

    /* Build the PAP ACK response message.  */
    packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_PAP_PROTOCOL & 0xFF00) >> 8;
    packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_PAP_PROTOCOL & 0xFF;
    packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_PAP_AUTHENTICATE_ACK;
    packet_ptr -> nx_packet_prepend_ptr[3] =  ppp_ptr -> nx_ppp_receive_id;
    packet_ptr -> nx_packet_prepend_ptr[4] =  0;
    packet_ptr -> nx_packet_prepend_ptr[5] =  5;
    packet_ptr -> nx_packet_prepend_ptr[6] =  0;

    /* Setup the append pointer and the packet length.  */
    packet_ptr -> nx_packet_length =  7;
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

    /* Set the authenticated flag to true.  */
    ppp_ptr -> nx_ppp_authenticated =  NX_TRUE;

#ifndef NX_PPP_DISABLE_INFO
    
    /* Increment the number of PAP frames sent.  */
    ppp_ptr -> nx_ppp_pap_frames_sent++;
    
    /* Increment the number of authentication ACKs sent counter.  */
    ppp_ptr -> nx_ppp_pap_authenticate_acks_sent++;
#endif

    /* Send the ACK message out.  */
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_pap_authentication_nak                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds and sends an authentication NAK message to     */ 
/*    the peer in response to the peer's authentication request.          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet               */ 
/*    _nx_ppp_packet_transmit               Send PAP response             */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_pap_state_machine_update      PPP state machine update      */ 
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
void _nx_ppp_pap_authentication_nak(NX_PPP *ppp_ptr)
{

UINT        status;
NX_PACKET   *packet_ptr;


    /* Allocate a packet for the PPP PAP response packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* An error was detected, simply return a NULL pointer.  */
        return;
    }

    /* Build the PAP ACK response message.  */
    packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_PAP_PROTOCOL & 0xFF00) >> 8;
    packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_PAP_PROTOCOL & 0xFF;
    packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_PAP_AUTHENTICATE_NAK;  
    packet_ptr -> nx_packet_prepend_ptr[3] =  ppp_ptr -> nx_ppp_receive_id;
    packet_ptr -> nx_packet_prepend_ptr[4] =  0;
    packet_ptr -> nx_packet_prepend_ptr[5] =  5;
    packet_ptr -> nx_packet_prepend_ptr[6] =  0;

    /* Setup the append pointer and the packet length.  */
    packet_ptr -> nx_packet_length =  7;
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;


#ifndef NX_PPP_DISABLE_INFO
    
    /* Increment the number of PAP frames sent.  */
    ppp_ptr -> nx_ppp_pap_frames_sent++;
    
    /* Increment the authentication NAKs sent counter.  */
    ppp_ptr -> nx_ppp_pap_authenticate_naks_sent++;
#endif

    /* Send the authentication NAK message out.  */
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);
}

#endif


#ifndef NX_PPP_DISABLE_CHAP
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_chap_state_machine_update                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes events and state changes in the PPP CHAP    */ 
/*    state machine.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    packet_ptr                            CHAP packet pointer           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_chap_challenge_send           Send CHAP challenge to peer   */ 
/*    _nx_ppp_chap_challenge_respond        Respond to challenge from peer*/ 
/*    _nx_ppp_chap_challenge_validate       Validate challenge response   */ 
/*                                            from peer                   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_receive_packet_process        Receive PPP packet processing */ 
/*    _nx_ppp_timeout                       PPP timeout                   */ 
/*    _nx_ppp_thread_entry                  PPP processing thread         */ 
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
void _nx_ppp_chap_state_machine_update(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr)
{

UCHAR       code;
UINT        valid;


    /* Determine if a packet is present. If so, derive the event from the packet.  */
    if (packet_ptr)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of CHAP frames received.  */
        ppp_ptr -> nx_ppp_chap_frames_received++;
#endif

        /* Pickup the type of received CHAP code.  */
        code =  packet_ptr -> nx_packet_prepend_ptr[2];

        /* Remember receive id.  */
        ppp_ptr -> nx_ppp_receive_id =  packet_ptr -> nx_packet_prepend_ptr[3];

        /* Check the incoming code.  */
        if ((code < NX_PPP_CHAP_CHALLENGE_REQUEST) || (code > NX_PPP_CHAP_CHALLENGE_FAILURE))
        {

#ifndef NX_PPP_DISABLE_INFO

            /* Increment the number of internal errors.  */
            ppp_ptr -> nx_ppp_internal_errors++;

            /* Increment the number of unknown CHAP requests received.  */
            ppp_ptr -> nx_ppp_chap_unknown_requests_received++;
#endif
            return;
        }
    }
    else
    {
    
        /* Set the code to timeout.  */
        code =   NX_PPP_CHAP_CHALLENGE_TIMEOUT;
    }

#ifndef NX_PPP_DISABLE_INFO

    /* Update receive counters.  */
    switch (code)
    {
    
    case NX_PPP_CHAP_CHALLENGE_REQUEST:
    
        /* Increment the number of CHAP challenge requests received.  */
        ppp_ptr -> nx_ppp_chap_challenge_requests_received++;
        break;
        
    case NX_PPP_CHAP_CHALLENGE_RESPONSE:
    
        /* Increment the number of CHAP challenge responses received.  */
        ppp_ptr -> nx_ppp_chap_challenge_responses_received++;
        break;
        
    case NX_PPP_CHAP_CHALLENGE_SUCCESS:
    
        /* Increment the number of CHAP challenge successful notifications received.  */
        ppp_ptr -> nx_ppp_chap_challenge_successes_received++;
        break;
        
    case NX_PPP_CHAP_CHALLENGE_FAILURE:
    
        /* Increment the number of CHAP challenge failures received.  */
        ppp_ptr -> nx_ppp_chap_challenge_failures_received++;
        break;
        
    case NX_PPP_CHAP_CHALLENGE_TIMEOUT:
    

        /* Determine if we are in the initial state. If so, this really isn't a timeout.  */
        if (ppp_ptr -> nx_ppp_chap_state != NX_PPP_CHAP_START_STATE)
        {

            /* Increment the number of CHAP timeouts received.  */
            ppp_ptr -> nx_ppp_chap_state_machine_timeouts++;
        }
        break;
    }
#endif

    /* Process relative to the current state.  */
    switch (ppp_ptr -> nx_ppp_chap_state)
    {
        /* Starting state.  */
        case NX_PPP_CHAP_START_STATE:
        {

            /* Determine if we need to challenge the peer.  */
            if (ppp_ptr -> nx_ppp_verify_authentication_protocol)
            {

                /* Setup the retry counter.  */
                ppp_ptr -> nx_ppp_protocol_retry_counter =  0;
 
                /* Setup the timeout.  */
                ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;

                /* Send challenge request to peer.  */
                _nx_ppp_chap_challenge_send(ppp_ptr);
                
                /* Move to the challenge request sent state.  */
                ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_REQUEST_SENT_STATE;
            }
            else if (ppp_ptr -> nx_ppp_generate_authentication_protocol)
            {
            
                /* Move to the challenge wait state, since the peer must challenge.  */
                ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_REQUEST_WAIT_STATE;
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_chap_state_machine_unhandled_requests++;
            }
#endif
            break;
        }

        case NX_PPP_CHAP_CHALLENGE_REQUEST_SENT_STATE:
        {
        
            /* In this state, this peer has sent a challenge request and is waiting for 
               response from the peer.  */
            if ((code == NX_PPP_CHAP_CHALLENGE_REQUEST) && (packet_ptr))
            {

                /* Generate a challenge response and send it to the peer.  */
                _nx_ppp_chap_challenge_respond(ppp_ptr, packet_ptr);

                /* Move to the sent both challenge and response sent state.  */
                ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_REQUEST_SENT_BOTH_STATE;
            }
            
            else if ((code == NX_PPP_CHAP_CHALLENGE_RESPONSE) && (packet_ptr))
            {
            
                /* Determine if the challenge response is valid. Note this function all sends the Success or
                   Failure to the peer.  */
                valid =  _nx_ppp_chap_challenge_validate(ppp_ptr, packet_ptr);          
                
                /* Is the login valid?  */
                if (valid == NX_TRUE)
                {
                
                    /* Does the peer need to perform an initial challenge?  */
                    if (ppp_ptr -> nx_ppp_generate_authentication_protocol)
                    {
            
                        /* Move to the challenge wait state, since the peer must challenge.  */
                        ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_REQUEST_WAIT_STATE;
                    }
                    else
                    {
                    
                        /* Since the peer does not need to challenge, the initial CHAP protocol is complete.  */
                        ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_COMPLETED_STATE;

                        /* Mark the PPP instance as authenticated.  */
                        ppp_ptr -> nx_ppp_authenticated =  NX_TRUE;

                        /* Turn off the timeout.  */
                        ppp_ptr -> nx_ppp_timeout =  0;
                    }
                }
                else
                {
                
                    /* Determine if there is an application NAK callback.  */
                    if (ppp_ptr -> nx_ppp_nak_authentication_notify)
                    {

                        /* Yes, call the application's authentication NAK notify callback function.  */
                        (ppp_ptr -> nx_ppp_nak_authentication_notify)();
                    }

                    /* Enter into a failed state since challenge failed.  */
                    ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_FAILED_STATE;

                    /* Determine if the application has registered a link down notification
                       callback.  */
                    if (ppp_ptr -> nx_ppp_link_down_callback)
                    {

                        /* Yes, call the application's callback function.  */
                        (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                    }
                }
            }
            
            /* Was a timeout received?  */
            else if (code == NX_PPP_CHAP_CHALLENGE_TIMEOUT)
            {
            
                /* Increment the retry counter.  */
                ppp_ptr -> nx_ppp_protocol_retry_counter++;
 
                /* Determine if the CHAP retry counter has been exceeded.  */
                if (ppp_ptr -> nx_ppp_protocol_retry_counter < NX_PPP_MAX_CHAP_PROTOCOL_RETRIES)
                {

                    /* Setup the timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;
            
                     /* Send challenge request to peer.  */
                    _nx_ppp_chap_challenge_send(ppp_ptr);         
                }
                else
                {
                
                    /* Retry counter exceeded.  */

                    /* Enter CHAP failed state.  PPP must be stopped and started to try again.  */
                    ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_FAILED_STATE;

                    /* Determine if the application has registered a link down notification
                       callback.  */
                    if (ppp_ptr -> nx_ppp_link_down_callback)
                    {

                        /* Yes, call the application's callback function.  */
                        (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                    }
                }
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_chap_state_machine_unhandled_requests++;
            }
#endif
            break;
        }

        case NX_PPP_CHAP_CHALLENGE_REQUEST_SENT_BOTH_STATE:
        {
        
            /* In this state, this peer has sent a challenge request and a challenge response and is waiting for 
               response from the peer.  */
            if (code == NX_PPP_CHAP_CHALLENGE_SUCCESS)
            {

                /* Move to the challenge sent received response state, since the original challenge is still pending.  */
                ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_REQUEST_SENT_RESPONDED_STATE;
            }

            else if (code == NX_PPP_CHAP_CHALLENGE_FAILURE)
            {
            
                /* Move to the failed state since our response failed to satisfy the peer's challenge.  */
                ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_FAILED_STATE;

                /* Determine if the application has registered a link down notification
                   callback.  */
                if (ppp_ptr -> nx_ppp_link_down_callback)
                {

                    /* Yes, call the application's callback function.  */
                    (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                }
            }            
            
            else if ((code == NX_PPP_CHAP_CHALLENGE_RESPONSE) && (packet_ptr))
            {
            
                /* Determine if the challenge response is valid. Note this function all sends the Success or
                   Failure to the peer.  */
                valid =  _nx_ppp_chap_challenge_validate(ppp_ptr, packet_ptr);          
                
                /* Is the login valid?  */
                if (valid == NX_TRUE)
                {
                
                    /* Does the peer need to perform an initial challenge?  */
                    if (ppp_ptr -> nx_ppp_generate_authentication_protocol)
                    {
            
                        /* Move to the challenge wait state, since the peer must challenge.  */
                        ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_RESPONSE_WAIT_STATE;
                    }
                    else
                    {
                    
                        /* Since the peer does not need to challenge, the initial CHAP protocol is complete.  */
                        ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_COMPLETED_STATE;

                        /* Mark the PPP instance as authenticated.  */
                        ppp_ptr -> nx_ppp_authenticated =  NX_TRUE;

                        /* Turn off the timeout.  */
                        ppp_ptr -> nx_ppp_timeout =  0;
                    }
                }
                else
                {
                
                    /* Enter into a failed state since challenge failed.  */
                    ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_FAILED_STATE;

                    /* Determine if the application has registered a link down notification
                       callback.  */
                    if (ppp_ptr -> nx_ppp_link_down_callback)
                    {

                        /* Yes, call the application's callback function.  */
                        (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                    }
                }
            }
            
            /* Was a timeout received?  */
            else if (code == NX_PPP_CHAP_CHALLENGE_TIMEOUT)
            {
            
                /* Increment the retry counter.  */
                ppp_ptr -> nx_ppp_protocol_retry_counter++;
 
                /* Determine if the CHAP retry counter has been exceeded.  */
                if (ppp_ptr -> nx_ppp_protocol_retry_counter < NX_PPP_MAX_CHAP_PROTOCOL_RETRIES)
                {

                    /* Setup the timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;
            
                     /* Send challenge request to peer.  */
                    _nx_ppp_chap_challenge_send(ppp_ptr);         
                }
                else
                {
                
                    /* Retry counter exceeded.  */

                    /* Enter CHAP failed state.  PPP must be stopped and started to try again.  */
                    ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_FAILED_STATE;

                    /* Determine if the application has registered a link down notification
                       callback.  */
                    if (ppp_ptr -> nx_ppp_link_down_callback)
                    {

                        /* Yes, call the application's callback function.  */
                        (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                    }
                }
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_chap_state_machine_unhandled_requests++;
            }
#endif
            break;
        }

        case NX_PPP_CHAP_CHALLENGE_REQUEST_SENT_RESPONDED_STATE:
        {

            /* In this state, we have already successfully responded to a challenge from the peer
               but are still waiting for the initial reply from the peer to our challenge request.  */
            if ((code == NX_PPP_CHAP_CHALLENGE_RESPONSE) && (packet_ptr))
            {
            
                /* Determine if the challenge response is valid. Note this function all sends the Success or
                   Failure to the peer.  */
                valid =  _nx_ppp_chap_challenge_validate(ppp_ptr, packet_ptr);          
                
                /* Is the login valid?  */
                if (valid == NX_TRUE)
                {
                
                    /* Since the peer does not need to challenge, the initial CHAP protocol is complete.  */
                    ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_COMPLETED_STATE;

                    /* Mark the PPP instance as authenticated.  */
                    ppp_ptr -> nx_ppp_authenticated =  NX_TRUE;

                    /* Turn off the timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  0;
                }
                else
                {
                
                    /* Enter into a failed state since challenge failed.  */
                    ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_FAILED_STATE;

                    /* Determine if the application has registered a link down notification
                       callback.  */
                    if (ppp_ptr -> nx_ppp_link_down_callback)
                    {

                        /* Yes, call the application's callback function.  */
                        (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                    }
                }
            }
            
            /* Was a timeout received?  */
            else if (code == NX_PPP_CHAP_CHALLENGE_TIMEOUT)
            {
            
                /* Increment the retry counter.  */
                ppp_ptr -> nx_ppp_protocol_retry_counter++;
 
                /* Determine if the CHAP retry counter has been exceeded.  */
                if (ppp_ptr -> nx_ppp_protocol_retry_counter < NX_PPP_MAX_CHAP_PROTOCOL_RETRIES)
                {

                    /* Setup the timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;
            
                     /* Send challenge request to peer.  */
                    _nx_ppp_chap_challenge_send(ppp_ptr);         
                }
                else
                {
                
                    /* Retry counter exceeded.  */

                    /* Enter CHAP failed state.  PPP must be stopped and started to try again.  */
                    ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_FAILED_STATE;

                    /* Determine if the application has registered a link down notification
                       callback.  */
                    if (ppp_ptr -> nx_ppp_link_down_callback)
                    {

                        /* Yes, call the application's callback function.  */
                        (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                    }
                }
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_chap_state_machine_unhandled_requests++;
            }
#endif
            break;
        }

        case NX_PPP_CHAP_CHALLENGE_REQUEST_WAIT_STATE:
        {

            /* In this state we only need the challenge from peer, either this peer doesn't challenge or 
               the initial challenge has already successfully completed.  */
            if ((code == NX_PPP_CHAP_CHALLENGE_REQUEST) && (packet_ptr))
            {

                /* Generate a challenge response and send it to the peer.  */
                _nx_ppp_chap_challenge_respond(ppp_ptr, packet_ptr);

                /* Move to the wait for challenge response state.  */
                ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_RESPONSE_WAIT_STATE;
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_chap_state_machine_unhandled_requests++;
            }
#endif
            break;
        }

        case NX_PPP_CHAP_CHALLENGE_RESPONSE_WAIT_STATE:
        {

            /* In this state, this peer has sent a challenge response and is waiting for 
               response from the peer.  */
            if (code == NX_PPP_CHAP_CHALLENGE_SUCCESS)
            {

                /* Since the peer does not need to challenge, the initial CHAP protocol is complete.  */
                ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_COMPLETED_STATE;

                /* Mark the PPP instance as authenticated.  */
                ppp_ptr -> nx_ppp_authenticated =  NX_TRUE;

                /* Turn off the timeout.  */
                ppp_ptr -> nx_ppp_timeout =  0;
            }

            else if (code == NX_PPP_CHAP_CHALLENGE_FAILURE)
            {
            
                /* Move to the failed state since our response failed to satisfy the peer's challenge.  */
                ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_FAILED_STATE;

                /* Determine if the application has registered a link down notification
                   callback.  */
                if (ppp_ptr -> nx_ppp_link_down_callback)
                {

                    /* Yes, call the application's callback function.  */
                    (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                }
            }            
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_chap_state_machine_unhandled_requests++;
            }
#endif
            break;
        }

        case NX_PPP_CHAP_COMPLETED_STATE:
        {

            /* In this state the initial challenge(s) have been processed... This state is used to field
               additional challenges during the life of the link.  */
            if ((code == NX_PPP_CHAP_CHALLENGE_REQUEST) && (packet_ptr))
            {

                /* Generate a challenge response and send it to the peer.  */
                _nx_ppp_chap_challenge_respond(ppp_ptr, packet_ptr);

                /* Stay in this state.  */
            }
            else if (code == NX_PPP_CHAP_CHALLENGE_FAILURE)
            {
            
                /* Move to the failed state since our response failed to satisfy the peer's challenge.  */
                ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_FAILED_STATE;

                /* Determine if the application has registered a link down notification
                   callback.  */
                if (ppp_ptr -> nx_ppp_link_down_callback)
                {

                    /* Yes, call the application's callback function.  */
                    (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                }
            }            
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_chap_state_machine_unhandled_requests++;
            }
#endif
            break;
        }

        case NX_PPP_CHAP_COMPLETED_NEW_STATE:
        {

            /* In this state the new challenge(s) is issued... */

            /* Setup the retry counter.  */
            ppp_ptr -> nx_ppp_protocol_retry_counter =  0;
 
            /* Setup the timeout.  */
            ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;

            /* Send challenge request to peer.  */
            _nx_ppp_chap_challenge_send (ppp_ptr);         
                
            /* Move to the new challenge request sent state.  */
            ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_COMPLETED_NEW_SENT_STATE;
            break;
        }

        case NX_PPP_CHAP_COMPLETED_NEW_SENT_STATE:
        {

            /* In this state the initial challenge(s) have been processed... This state is used to field
               additional challenges during the life of the link.  */
            if ((code == NX_PPP_CHAP_CHALLENGE_REQUEST) && (packet_ptr))
            {

                /* Generate a challenge response and send it to the peer.  */
                _nx_ppp_chap_challenge_respond(ppp_ptr, packet_ptr);

                /* Stay in this state.  */
            }

            else if (code == NX_PPP_CHAP_CHALLENGE_FAILURE)
            {
            
                /* Determine if there is an application NAK callback.  */
                if (ppp_ptr -> nx_ppp_nak_authentication_notify)
                {

                    /* Yes, call the application's authentication NAK notify callback function.  */
                    (ppp_ptr -> nx_ppp_nak_authentication_notify)();
                }

                /* Move to the failed state since our response failed to satisfy the peer's challenge.  */
                ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_FAILED_STATE;

                /* Mark the PPP instance as not authenticated.  */
                ppp_ptr -> nx_ppp_authenticated =  NX_FALSE;

                /* Determine if the application has registered a link down notification
                   callback.  */
                if (ppp_ptr -> nx_ppp_link_down_callback)
                {

                    /* Yes, call the application's callback function.  */
                    (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                }
            }            

            else if ((code == NX_PPP_CHAP_CHALLENGE_RESPONSE) && (packet_ptr))
            {
            
                /* Determine if the challenge response is valid. Note this function all sends the Success or
                   Failure to the peer.  */
                valid =  _nx_ppp_chap_challenge_validate(ppp_ptr, packet_ptr);          
                
                /* Is the login valid?  */
                if (valid == NX_FALSE)
                {
                
                    /* No: Determine if there is an application NAK callback.  */
                    if (ppp_ptr -> nx_ppp_nak_authentication_notify)
                    {

                        /* Yes, call the application's authentication NAK notify callback function.  */
                        (ppp_ptr -> nx_ppp_nak_authentication_notify)();
                    }

                    /* Enter into a failed state since challenge failed.  */
                    ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_FAILED_STATE;
                    
                    /* Mark the PPP instance as not authenticated.  */
                    ppp_ptr -> nx_ppp_authenticated =  NX_FALSE;

                    /* Determine if the application has registered a link down notification
                       callback.  */
                    if (ppp_ptr -> nx_ppp_link_down_callback)
                    {

                        /* Yes, call the application's callback function.  */
                        (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                    }
                }
                else
                {
                    
                    /* Simply move back to the completed state.  */
                    ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_COMPLETED_STATE;

                    /* Turn off the timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  0;
                }
            }
            /* Was a timeout received?  */
            else if (code == NX_PPP_CHAP_CHALLENGE_TIMEOUT)
            {
            
                /* Increment the retry counter.  */
                ppp_ptr -> nx_ppp_protocol_retry_counter++;
 
                /* Determine if the CHAP retry counter has been exceeded.  */
                if (ppp_ptr -> nx_ppp_protocol_retry_counter < NX_PPP_MAX_CHAP_PROTOCOL_RETRIES)
                {

                    /* Setup the timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;
            
                     /* Send challenge request to peer.  */
                    _nx_ppp_chap_challenge_send (ppp_ptr);         
                }
                else
                {
                
                    /* Retry counter exceeded.  */

                    /* Enter CHAP failed state.  PPP must be stopped and started to try again.  */
                    ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_CHALLENGE_FAILED_STATE;

                    /* Determine if the application has registered a link down notification
                       callback.  */
                    if (ppp_ptr -> nx_ppp_link_down_callback)
                    {

                        /* Yes, call the application's callback function.  */
                        (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                    }
                }
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_chap_state_machine_unhandled_requests++;
            }
#endif
            break;
        }
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_chap_challenge_send                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends a CHAP challenge to the peer.                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet               */ 
/*    (nx_ppp_chap_get_challenge_values)    Get values for challenge      */ 
/*    _nx_ppp_packet_transmit               Send PAP response             */ 
/*    _nx_utility_string_length_check       Check string length           */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_chap_state_machine_update     Update CHAP state machine     */ 
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
void _nx_ppp_chap_challenge_send(NX_PPP *ppp_ptr)
{

UCHAR       id;
UINT        length, length1, i, j;
NX_PACKET   *packet_ptr;
UINT        status;


    /* Determine if there is a challenge generation routine.  */
    if (ppp_ptr -> nx_ppp_chap_get_challenge_values)
    {

        /* Yes, get the challenge values.  */
        ppp_ptr -> nx_ppp_chap_get_challenge_values(ppp_ptr -> nx_ppp_chap_random_value, 
                                                (CHAR *) &id, ppp_ptr -> nx_ppp_chap_challenger_name);
    }
    else
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the internal error counter.  */
        ppp_ptr -> nx_ppp_internal_errors++;
#endif

        /* An error was detected, simply return.  */
        return;
    }

    /* Allocate a packet for the PPP CHAP challenge packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* An error was detected, simply return a NULL pointer.  */
        return;
    }

    /* Calculate the lengths of the challenger name and random value.  */           
    if (_nx_utility_string_length_check(ppp_ptr -> nx_ppp_chap_challenger_name, &length1, NX_PPP_NAME_SIZE) ||
        _nx_utility_string_length_check(ppp_ptr -> nx_ppp_chap_random_value, &length, NX_PPP_VALUE_SIZE))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Check if out of boundary.  */
    if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < (UINT)(length + 1 + length1 + 6))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Build CHAP challenge message.  */
    packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_CHAP_PROTOCOL & 0xFF00) >> 8;
    packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_CHAP_PROTOCOL & 0xFF;
    packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_CHAP_CHALLENGE_REQUEST;
    packet_ptr -> nx_packet_prepend_ptr[3] =  id; /* ID */

    /* Setup the length.  */
    packet_ptr -> nx_packet_prepend_ptr[4] =  (UCHAR) ((((UINT) length) + 1 + ((UINT) length1) + 4) >> 8);
    packet_ptr -> nx_packet_prepend_ptr[5] =  (UCHAR) ((((UINT) length) + 1 + ((UINT) length1) + 4) & 0xFF);

    /* Store the CHAP random value.  */
    packet_ptr -> nx_packet_prepend_ptr[6] =  (UCHAR)length;
    for (i = 0; i < length; i++)
    {

        /* Store byte of name.  */
        packet_ptr -> nx_packet_prepend_ptr[i+7] =  (UCHAR)(ppp_ptr -> nx_ppp_chap_random_value[i]);
    }

    /* Store CHAP challenge name.  */
    for (j = 0; j < length1; j++)
    {

        /* Store byte of name.  */
        packet_ptr -> nx_packet_prepend_ptr[j+i+7] =  (UCHAR)(ppp_ptr -> nx_ppp_chap_challenger_name[j]);
    }

    /* Setup the append pointer and the packet length.  */
    packet_ptr -> nx_packet_length =  ((UINT) length) + 1 + ((UINT) length1) + 4 + 2;
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

#ifndef NX_PPP_DISABLE_INFO

    /* Increment the number of CHAP frames sent.  */
    ppp_ptr -> nx_ppp_chap_frames_sent++;
    
    /* Increment the number of CHAP challenge requests.  */
    ppp_ptr -> nx_ppp_chap_challenge_requests_sent++;
#endif

    /* Send challenge message.  */
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_chap_challenge_respond                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes the CHAP challenge request from the peer.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    packet_ptr                            Pointer to CHAP challenge     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet               */ 
/*    (nx_ppp_chap_get_responder_values)    Get responder values for reply*/ 
/*    _nx_ppp_hash_generator                Generate MD5 hash             */ 
/*    _nx_ppp_packet_transmit               Send PAP response             */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_chap_state_machine_update     Update CHAP state machine     */ 
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
void _nx_ppp_chap_challenge_respond(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr)
{

UCHAR       name[NX_PPP_NAME_SIZE + 1], secret[NX_PPP_NAME_SIZE + 1]; 
UCHAR       value[NX_PPP_VALUE_SIZE + 1], hvalue[NX_PPP_HASHED_VALUE_SIZE + 1];
UCHAR       id;
UINT        length, length1, i, j;
NX_PACKET   *response_packet_ptr;
UINT        status;
UINT        name_length;


    /* Pickup the id and length.  */
    id =  packet_ptr -> nx_packet_prepend_ptr[3];

    /* Pickup the length.  */
    length =  (UINT) packet_ptr -> nx_packet_prepend_ptr[4];
    length1 = (UINT) packet_ptr -> nx_packet_prepend_ptr[5];
    length1 = (length << 8) | (length1) ;

    /* Check for valid packet length.  */
    if ((length1 + 2) > packet_ptr -> nx_packet_length)
    {
        return;
    }

    /* Pickup the length of the random value.  */
    length =  (UINT) packet_ptr -> nx_packet_prepend_ptr[6];

    /* Check for valid packet length.  */
    if ((length == 0) || ((ULONG)(length + 7) > packet_ptr -> nx_packet_length))
    {
        return;
    }

    /* Determine if the length is greater than the destination.  */
    name_length = length1 - length - 5;
    if ((length > NX_PPP_VALUE_SIZE) ||
        (name_length > NX_PPP_NAME_SIZE))
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of internal errors.  */
        ppp_ptr -> nx_ppp_internal_errors++;
#endif
        return;
    }

    /* Pickup the random value.  */
    for(i = 0; i < length; i++)
    {

        /* Pickup a byte of the random value.  */
        value[i] =  packet_ptr -> nx_packet_prepend_ptr[i+7];
    }

    /* Now pickup the challenge name.  */
    for(j = 0; j < name_length; j++)
    {

        /* Pickup a byte of the challenger name.  */
        name[j] =  packet_ptr -> nx_packet_prepend_ptr[i+j+7];
    }

    /* Null terminate it.  */
    name[j] =  0;

    /* Determine if there is a challenge get responder values routine.  */
    if (ppp_ptr -> nx_ppp_chap_get_responder_values)
    {

        /* Get name and password for this server   */
        ppp_ptr -> nx_ppp_chap_get_responder_values((CHAR*) name, (CHAR *) name, (CHAR *) secret);
    }
    else
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the internal error counter.  */
        ppp_ptr -> nx_ppp_internal_errors++;
#endif

        /* An error was detected, simply return a NULL pointer.  */
        return;
    }

    /* Generate hash value.  */
    _nx_ppp_hash_generator(hvalue, id, secret, value, length);

    /* Allocate a packet for the PPP CHAP response packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &response_packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* An error was detected, simply return a NULL pointer.  */
        return;
    }

    /* Calculate length of the name  */
    for(length = 0; name[length]; length++);

    /* Check if out of boundary.  */
    if ((UINT)(response_packet_ptr -> nx_packet_data_end - response_packet_ptr -> nx_packet_prepend_ptr) < (UINT)(length + NX_PPP_HASHED_VALUE_SIZE + 7))
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return;
    }

    /* Build CHAP challenge response message.  */
    response_packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_CHAP_PROTOCOL & 0xFF00) >> 8;
    response_packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_CHAP_PROTOCOL & 0xFF;
    response_packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_CHAP_CHALLENGE_RESPONSE;
    response_packet_ptr -> nx_packet_prepend_ptr[3] =  id; /* ID */

    /* Setup the length.  */
    response_packet_ptr -> nx_packet_prepend_ptr[4] =  (UCHAR) ((((UINT) length) + NX_PPP_HASHED_VALUE_SIZE + 5) >> 8);
    response_packet_ptr -> nx_packet_prepend_ptr[5] =  (UCHAR) ((((UINT) length) + NX_PPP_HASHED_VALUE_SIZE + 5) & 0xFF);

    /* Set the length of the hashed response value.  */
    response_packet_ptr -> nx_packet_prepend_ptr[6] =  (UCHAR) NX_PPP_HASHED_VALUE_SIZE;

    /* Loop to insert the hashed value.  */
    for(i = 0; i < NX_PPP_HASHED_VALUE_SIZE; i++)
    {

        /* Copy one byte of the hashed value.  */
        response_packet_ptr -> nx_packet_prepend_ptr[i+7] =  hvalue[i];
    }

    /* Loop to insert the name.  */
    for(j = 0; j < length; j++)
    {

        /* Copy one byte of the name.  */
        response_packet_ptr -> nx_packet_prepend_ptr[i+j+7] =  name[j];
    }

    /* Setup the append pointer and the packet length.  */
    response_packet_ptr -> nx_packet_length =   length + NX_PPP_HASHED_VALUE_SIZE + 5 + 2;
    response_packet_ptr -> nx_packet_append_ptr =  response_packet_ptr -> nx_packet_prepend_ptr + response_packet_ptr -> nx_packet_length;

#ifndef NX_PPP_DISABLE_INFO

    /* Increment the number of CHAP frames sent.  */
    ppp_ptr -> nx_ppp_chap_frames_sent++;

    /* Increment the number of CHAP responses sent.  */
    ppp_ptr -> nx_ppp_chap_challenge_responses_sent++;
#endif

    /* Transmit response message.   */
    _nx_ppp_packet_transmit(ppp_ptr, response_packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_chap_challenge_validate                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function validates the CHAP response from the peer.            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    packet_ptr                            Pointer to CHAP challenge     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                NX_TRUE - valid login         */ 
/*                                          NX_FALSE - invalid login      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet               */ 
/*    (nx_ppp_chap_get_verification_values) Get verification values       */ 
/*    _nx_ppp_hash_generator                Generate MD5 hash             */ 
/*    _nx_ppp_packet_transmit               Send PAP response             */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_chap_state_machine_update     Update CHAP state machine     */ 
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
UINT  _nx_ppp_chap_challenge_validate(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr)
{

UCHAR       name[NX_PPP_NAME_SIZE + 1], secret[NX_PPP_NAME_SIZE + 1]; 
UCHAR       hvalue[NX_PPP_HASHED_VALUE_SIZE + 1];
UCHAR       hvalue1[NX_PPP_HASHED_VALUE_SIZE + 1];
UCHAR       id;
UINT        length, length1, i, j;
NX_PACKET   *response_packet_ptr;
UINT        status;
UINT        name_length;


    /* Pickup the id and length.  */
    id =  packet_ptr -> nx_packet_prepend_ptr[3];

    /* Pickup the length.  */
    length =  (UINT) packet_ptr -> nx_packet_prepend_ptr[4];
    length1 = (UINT) packet_ptr -> nx_packet_prepend_ptr[5];
    length1 = (length << 8) | (length1) ;

    /* Check for valid packet length.  */
    if ((length1 + 2) > packet_ptr -> nx_packet_length)
    {
        return(NX_FALSE);
    }

    /* Pickup the length of the hashed value.  */
    length =  (UINT) packet_ptr -> nx_packet_prepend_ptr[6];

    /* Check for valid packet length.  */
    if ((length == 0) || ((ULONG)(length + 7) > packet_ptr -> nx_packet_length))
    {
        return(NX_FALSE);
    }

    /* Determine if the length is greater than the destination.  */
    name_length = length1 - length - 5;
    if ((length > NX_PPP_HASHED_VALUE_SIZE) ||
        (name_length > NX_PPP_NAME_SIZE))
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of internal errors.  */
        ppp_ptr -> nx_ppp_internal_errors++;
#endif

        /* Return false in case of internal error.  */
        return(NX_FALSE);
    }

    /* Pickup the hashed value.  */
    for(i = 0; i < length; i++)
    {

        /* Pickup one byte of the hashed value.  */
        hvalue[i] =  packet_ptr -> nx_packet_prepend_ptr[i+7];
    }

    /* Pickup the name.  */
    for(j = 0; j < name_length; j++)
    {

        /* Pickup one byte of the hashed value.  */
        name[j] =  packet_ptr -> nx_packet_prepend_ptr[i+j+7];   
    }

    /* Null terminate the name.  */
    name[j] = 0;            

    /* Determine if there is a challenge get verification values routine.  */
    if (ppp_ptr -> nx_ppp_chap_get_verification_values)
    {

        /* Get name and password for this server   */
        ppp_ptr -> nx_ppp_chap_get_verification_values(ppp_ptr -> nx_ppp_chap_challenger_name, (CHAR *) name, (CHAR *) secret);
    }
    else
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the internal error counter.  */
        ppp_ptr -> nx_ppp_internal_errors++;
#endif

        /* Return false in case of internal error.  */
        return(NX_FALSE);
    }

    /* Generate hash value.  */
    _nx_ppp_hash_generator(hvalue1, id, secret, (UCHAR *) ppp_ptr -> nx_ppp_chap_random_value, 0);

    /* Compare the computed hash value with the hash value received.  */
    for(i = 0; i < NX_PPP_HASHED_VALUE_SIZE; i++)
    {

        /* Are the hash values equal?  */
        if (hvalue[i] != hvalue1[i])
        { 
            /* No, get out of the loop.  */
            break;
        }
    }


    /* Allocate a packet for the PPP CHAP response packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &response_packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* Return false in case of internal error.  */
        return(NX_FALSE);
    }

    /* Determine if challenge was successful.  */
    if (i == NX_PPP_HASHED_VALUE_SIZE)
    {

        /* Build CHAP ACK message.  */
        response_packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_CHAP_PROTOCOL & 0xFF00) >> 8;
        response_packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_CHAP_PROTOCOL & 0xFF;
        response_packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_CHAP_CHALLENGE_SUCCESS;
        response_packet_ptr -> nx_packet_prepend_ptr[3] =  id; /* ID */

        /* Setup the length.  */
        response_packet_ptr -> nx_packet_prepend_ptr[4] =  (UCHAR) 0;
        response_packet_ptr -> nx_packet_prepend_ptr[5] =  (UCHAR) 4;

        /* Setup the append pointer and the packet length.  */
        response_packet_ptr -> nx_packet_length = 6;
        response_packet_ptr -> nx_packet_append_ptr =  response_packet_ptr -> nx_packet_prepend_ptr + response_packet_ptr -> nx_packet_length;

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of CHAP frames sent.  */
        ppp_ptr -> nx_ppp_chap_frames_sent++;

        /* Increment the number of CHAP success notifications sent.  */
        ppp_ptr -> nx_ppp_chap_challenge_successes_sent++;
#endif

        /* Transmit the message.  */
        _nx_ppp_packet_transmit(ppp_ptr, response_packet_ptr);
        
        /* Return true to indicate the response was valid.  */
        return(NX_TRUE);
    }
    else 
    {

        /* Build CHAP NAK message.  */
        response_packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_CHAP_PROTOCOL & 0xFF00) >> 8;
        response_packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_CHAP_PROTOCOL & 0xFF;
        response_packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_CHAP_CHALLENGE_FAILURE;
        response_packet_ptr -> nx_packet_prepend_ptr[3] =  id; /* ID */

        /* Setup the length.  */
        response_packet_ptr -> nx_packet_prepend_ptr[4] =  (UCHAR) 0;
        response_packet_ptr -> nx_packet_prepend_ptr[5] =  (UCHAR) 4;

        /* Setup the append pointer and the packet length.  */
        response_packet_ptr -> nx_packet_length = 6;
        response_packet_ptr -> nx_packet_append_ptr =  response_packet_ptr -> nx_packet_prepend_ptr + response_packet_ptr -> nx_packet_length;

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of CHAP frames sent.  */
        ppp_ptr -> nx_ppp_chap_frames_sent++;

        /* Increment the number of CHAP failure notifications sent.  */
        ppp_ptr -> nx_ppp_chap_challenge_failures_sent++;
#endif

        /* Transmit the message.  */
        _nx_ppp_packet_transmit(ppp_ptr, response_packet_ptr);

        /* Return false to indicate the response was invalid.  */
        return(NX_FALSE);
    }
}
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_ipcp_state_machine_update                   PORTABLE C      */ 
/*                                                           6.1.2        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes events and state changes in the PPP IPCP    */ 
/*    state machine.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    packet_ptr                            Packet pointer                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_ipcp_configure_check          Check configuration info      */ 
/*    _nx_ppp_ipcp_configure_request_send   Send configuration request    */ 
/*    _nx_ppp_ipcp_response_extract         Extract info from response    */ 
/*    _nx_ppp_ipcp_response_send            Send response                 */ 
/*    _nx_ppp_ipcp_terminate_send           Send terminate                */ 
/*    _nx_ppp_ipcp_terminate_ack_send       Send terminate ack            */ 
/*    nx_ip_interface_address_set           Set interface IP address      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_receive_packet_process        Receive PPP packet processing */ 
/*    _nx_ppp_timeout                       PPP timeout                   */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  11-09-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            corrected the NAKed list    */
/*                                            pointer,                    */
/*                                            resulting in version 6.1.2  */
/*                                                                        */
/**************************************************************************/
void _nx_ppp_ipcp_state_machine_update(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr)
{    

UINT    acceptable_configuration;
UCHAR   good_data[NX_PPP_OPTION_MESSAGE_LENGTH];
UCHAR   code;


    /* Determine if a packet is present. If so, derive the event from the packet.  */
    if (packet_ptr)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of IPCP frames received.  */
        ppp_ptr -> nx_ppp_ipcp_frames_received++;
#endif

        /* Pickup the type of received IPCP code.  */
        code =  packet_ptr -> nx_packet_prepend_ptr[2];

        /* Remember receive id.  */
        ppp_ptr -> nx_ppp_receive_id =  packet_ptr -> nx_packet_prepend_ptr[3];

        /* Is the code supported by PPP?  */
        if ((code < NX_PPP_IPCP_CONFIGURE_REQUEST) ||
            (code > NX_PPP_IPCP_TERMINATE_ACK))
        {
        
#ifndef NX_PPP_DISABLE_INFO

            /* Increment the number of internal errors.  */
            ppp_ptr -> nx_ppp_internal_errors++;
            
            /* Increment the number of unhandled requests.  */
            ppp_ptr -> nx_ppp_ipcp_state_machine_unhandled_requests++;
#endif
            return;
        }
    }
    else
    {
        
        /* Set the code to timeout to indicate a timeout has occurred.  */
        code =  NX_PPP_IPCP_TIMEOUT;
    }

#ifndef NX_PPP_DISABLE_INFO

    /* Increment the corresponding request counter.  */
    switch (code)
    {
    
    case NX_PPP_IPCP_CONFIGURE_REQUEST:

        /* Increment the number of IPCP configure requests received.  */
        ppp_ptr -> nx_ppp_ipcp_configure_requests_received++;
        break;

    case NX_PPP_IPCP_CONFIGURE_ACK:

        /* Increment the number of IPCP configure ACKs received.  */
        ppp_ptr -> nx_ppp_ipcp_configure_acks_received++;
        break;

    case NX_PPP_IPCP_CONFIGURE_NAK:

        /* Increment the number of IPCP configure NAKs received.  */
        ppp_ptr -> nx_ppp_ipcp_configure_naks_received++;
        break;

    case NX_PPP_IPCP_CONFIGURE_REJECT:

        /* Increment the number of IPCP configure rejects received.  */
        ppp_ptr -> nx_ppp_ipcp_configure_rejects_received++;
        break;

    case NX_PPP_IPCP_TERMINATE_REQUEST:

        /* Increment the number of IPCP terminate requests received.  */
        ppp_ptr -> nx_ppp_ipcp_terminate_requests_received++;
        break;

    case NX_PPP_IPCP_TERMINATE_ACK:

        /* Increment the number of IPCP terminate ACKs received.  */
        ppp_ptr -> nx_ppp_ipcp_terminate_acks_received++;
        break;

    case NX_PPP_IPCP_TIMEOUT:

        /* Determine if we are in the initial state. If so, this really isn't a timeout.  */
        if (ppp_ptr -> nx_ppp_ipcp_state != NX_PPP_IPCP_START_STATE)
        {
        
            /* Increment the number of IPCP state machine timeouts.  */
            ppp_ptr -> nx_ppp_ipcp_state_machine_timeouts++;
        }
        break;
    }
#endif

    /* Process relative to the current state.  */
    switch (ppp_ptr -> nx_ppp_ipcp_state)
    {
    
        case NX_PPP_IPCP_START_STATE:
        {
    
            /* Initial IPCP state.  */
 
            /* Initialize the NAK and rejected list.  */
#ifndef NX_PPP_DNS_OPTION_DISABLE
            ppp_ptr -> nx_ppp_naked_list[0] =       0;
            ppp_ptr -> nx_ppp_naked_list[1] =       0;
#else
            ppp_ptr -> nx_ppp_naked_list[0] =       1;
            ppp_ptr -> nx_ppp_naked_list[1] =       1;
#endif

            ppp_ptr -> nx_ppp_peer_naked_list[0] =  0;
            ppp_ptr -> nx_ppp_rejected_list[0] =    0;

            /* Setup the retry counter.  */
            ppp_ptr -> nx_ppp_protocol_retry_counter =  0;
 
            /* Setup the timeout.  */
            ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;
    
            /* Send the configuration request.  */
            _nx_ppp_ipcp_configure_request_send(ppp_ptr, ppp_ptr -> nx_ppp_naked_list);

            /* Move into the request sent state.  */
            ppp_ptr -> nx_ppp_ipcp_state = NX_PPP_IPCP_CONFIGURE_REQUEST_SENT_STATE;
            break;
        }

        case NX_PPP_IPCP_CONFIGURE_REQUEST_SENT_STATE:
        {
    
            /* In this state, we have sent a configuration request but had not received an ACK
               or a configuration request from the peer.  */
    
            /* Process relative to the incoming code.  */
            if (code ==  NX_PPP_IPCP_CONFIGURE_ACK)
            {
        
                /* Determine if the ID matches our last transmit ID. If not, just discard it.  */
                if (ppp_ptr -> nx_ppp_transmit_id != ppp_ptr -> nx_ppp_receive_id)
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the invalid frame ID counter.  */
                    ppp_ptr -> nx_ppp_invalid_frame_id++;
#endif

                    /* Discard this request by simply returning!  */
                    return;
                }

                /* The peer has ACKed our configuration request. Move to the 
                   configure request ACKed state.  */
                ppp_ptr -> nx_ppp_ipcp_state =  NX_PPP_IPCP_CONFIGURE_REQUEST_ACKED_STATE;
            
                /* Turn off the timeout for the configuration request.  */
                ppp_ptr -> nx_ppp_timeout =  0;
            }

            else if (code == NX_PPP_IPCP_CONFIGURE_REJECT)
            {

                /* Determine if the ID matches our last transmit ID. If not, just discard it.  */
                if (ppp_ptr -> nx_ppp_transmit_id != ppp_ptr -> nx_ppp_receive_id)
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the invalid frame ID counter.  */
                    ppp_ptr -> nx_ppp_invalid_frame_id++;
#endif

                    /* Discard this request by simply returning!  */
                    return;
                }

                /* Set the NAK option list, just to indicate to the configure request that 
                   the IPCP request without DNS option is what we want.  */
                ppp_ptr -> nx_ppp_naked_list[0] =  1;

                /* Send the configuration request.  */
                _nx_ppp_ipcp_configure_request_send(ppp_ptr, ppp_ptr -> nx_ppp_naked_list);
            }

            else if ((code ==  NX_PPP_IPCP_CONFIGURE_NAK) && (packet_ptr))
            {

                /* Determine if the ID matches our last transmit ID. If not, just discard it.  */
                if (ppp_ptr -> nx_ppp_transmit_id != ppp_ptr -> nx_ppp_receive_id)
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the invalid frame ID counter.  */
                    ppp_ptr -> nx_ppp_invalid_frame_id++;
#endif

                    /* Discard this request by simply returning!  */
                    return;
                }

                /* Extract the information from response... looking for IP address and possibly DNS server IP address.  */
                _nx_ppp_ipcp_response_extract(ppp_ptr, packet_ptr);

                /* Send the configuration request.  */
                _nx_ppp_ipcp_configure_request_send(ppp_ptr, ppp_ptr -> nx_ppp_naked_list);
            }

            else if ((code == NX_PPP_IPCP_CONFIGURE_REQUEST) && (packet_ptr))
            {

                /* The peer has sent a configuration request.  */ 
            
                /* Check configuration information.  */
                acceptable_configuration =  _nx_ppp_ipcp_configure_check(ppp_ptr, packet_ptr, ppp_ptr -> nx_ppp_peer_naked_list, 
                                                                         ppp_ptr -> nx_ppp_rejected_list, 
                                                                         good_data);

                /* Check for acceptable configuration.  */
                if (acceptable_configuration == NX_TRUE) 
                {

                    /* Yes, send an ack.  */
                    _nx_ppp_ipcp_response_send(ppp_ptr, NX_PPP_IPCP_CONFIGURE_ACK, good_data, good_data[1], packet_ptr); 

                    /* Move to the state where the peer's configuration has been ACKed.  */
                    ppp_ptr -> nx_ppp_ipcp_state =  NX_PPP_IPCP_PEER_CONFIGURE_REQUEST_ACKED_STATE;
                }
                else 
                {
            
                    /* Otherwise, configuration is unacceptable, send a nak.  */

                    /* Check rejected list.  */
                    if (ppp_ptr -> nx_ppp_rejected_list[0] != 0)
                    {

                        /* Yes, there are rejected options so send a configure reject.  */
                        _nx_ppp_ipcp_response_send(ppp_ptr, NX_PPP_IPCP_CONFIGURE_REJECT, 
                                                   &ppp_ptr -> nx_ppp_rejected_list[1], 
                                                   ppp_ptr -> nx_ppp_rejected_list[0], NX_NULL);
                        
                    }
                    else if (ppp_ptr -> nx_ppp_peer_naked_list[0] != 0)
                    {

                        /* Yes, there are naked options so send a new request.  */
                        _nx_ppp_ipcp_response_send(ppp_ptr, NX_PPP_IPCP_CONFIGURE_NAK, 
                                                   &ppp_ptr -> nx_ppp_peer_naked_list[1], 
                                                   ppp_ptr -> nx_ppp_peer_naked_list[0], NX_NULL);
                        
                    }
                }
            }
            else if (code == NX_PPP_IPCP_TIMEOUT)
            {

                /* Increment the retry counter.  */
                ppp_ptr -> nx_ppp_protocol_retry_counter++;
 
                /* Determine if the IPCP retry counter has been exceeded.  */
                if (ppp_ptr -> nx_ppp_protocol_retry_counter < NX_PPP_MAX_IPCP_PROTOCOL_RETRIES)
                {

                    /* Setup the timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;
        
                    /* Timeout, send the configuration request.  */
                    _nx_ppp_ipcp_configure_request_send(ppp_ptr, ppp_ptr -> nx_ppp_naked_list); 
                }
                else
                {
                
                    /* Retry counter exceeded.  */

                    /* Enter IPCP failed state.  PPP must be stopped and started to try again.  */
                    ppp_ptr -> nx_ppp_ipcp_state =  NX_PPP_IPCP_FAILED_STATE;

                    /* Determine if the application has registered a link down notification
                       callback.  */
                    if (ppp_ptr -> nx_ppp_link_down_callback)
                    {

                        /* Yes, call the application's callback function.  */
                        (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                    }
                }
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_ipcp_state_machine_unhandled_requests++;
            }
#endif
            break;    
        }

        case NX_PPP_IPCP_CONFIGURE_REQUEST_ACKED_STATE:
        {
    
            /* In this state, we have received the ACK for our configuration request, but have not yet
               received a configuration request from the peer.  */
    
            /* Process relative to the incoming code.  */
            if ((code == NX_PPP_IPCP_CONFIGURE_REQUEST) && (packet_ptr))
            {
        
                /* The peer has sent a configuration request.  */ 
                
                /* Check configuration information.  */
                acceptable_configuration =  _nx_ppp_ipcp_configure_check(ppp_ptr, packet_ptr, ppp_ptr -> nx_ppp_peer_naked_list, ppp_ptr -> nx_ppp_rejected_list, good_data);

                /* Check for acceptable configuration.  */
                if (acceptable_configuration == NX_TRUE) 
                {

                    /* Yes, send an ack.  */
                    _nx_ppp_ipcp_response_send(ppp_ptr, NX_PPP_IPCP_CONFIGURE_ACK, good_data, good_data[1], packet_ptr);

                    /* Move to the state where the peer's configuration has been ACKed.  */
                    ppp_ptr -> nx_ppp_ipcp_state =  NX_PPP_IPCP_COMPLETED_STATE;

                    /* Set the IP address for the specific interface from the configuration.   */
                    nx_ip_interface_address_set(ppp_ptr -> nx_ppp_ip_ptr, ppp_ptr -> nx_ppp_interface_index, 
                                                IP_ADDRESS(ppp_ptr -> nx_ppp_ipcp_local_ip[0], 
                                                           ppp_ptr -> nx_ppp_ipcp_local_ip[1], 
                                                           ppp_ptr -> nx_ppp_ipcp_local_ip[2], 
                                                           ppp_ptr -> nx_ppp_ipcp_local_ip[3]),
                                                           0xffffffff);

                    /* Set gateway address.  */
                    (ppp_ptr -> nx_ppp_ip_ptr) -> nx_ip_gateway_address =   (ppp_ptr -> nx_ppp_interface_ptr) -> nx_interface_ip_address;
                    (ppp_ptr -> nx_ppp_ip_ptr) -> nx_ip_gateway_interface =  ppp_ptr -> nx_ppp_interface_ptr;

                    /* Turn off the timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  0;
                }
                else 
                {
            
                    /* Otherwise, configuration is unacceptable, send a NAK.  */

                    /* Check rejected list.  */
                    if (ppp_ptr -> nx_ppp_rejected_list[0] != 0)
                    {

                        /* Yes, there are rejected options so send a new request.  */
                        _nx_ppp_ipcp_response_send(ppp_ptr, NX_PPP_IPCP_CONFIGURE_REJECT, &ppp_ptr -> nx_ppp_rejected_list[1], ppp_ptr -> nx_ppp_rejected_list[0], NX_NULL);
                    }
                    else if (ppp_ptr -> nx_ppp_peer_naked_list[0] != 0)
                    {

                        /* Yes, there are naked options so send a new request.  */
                        _nx_ppp_ipcp_response_send(ppp_ptr, NX_PPP_IPCP_CONFIGURE_NAK, &ppp_ptr -> nx_ppp_peer_naked_list[1], ppp_ptr -> nx_ppp_peer_naked_list[0], NX_NULL);
                    }
                }
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_ipcp_state_machine_unhandled_requests++;
            }
#endif
            break;    
        }
    
        case NX_PPP_IPCP_PEER_CONFIGURE_REQUEST_ACKED_STATE:
        {

            /* In this state, we have sent our configuration request, but haven't received an ACK. We have also received 
               a peer configuration request and have ACKed that request.  */
           
            /* Process relative to the incoming code.  */
            if (code ==  NX_PPP_IPCP_CONFIGURE_ACK)
            {
        
                /* Determine if the ID matches our last transmit ID. If not, just discard it.  */
                if (ppp_ptr -> nx_ppp_transmit_id != ppp_ptr -> nx_ppp_receive_id)
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the invalid frame ID counter.  */
                    ppp_ptr -> nx_ppp_invalid_frame_id++;
#endif

                    /* Discard this request by simply returning!  */
                    return;
                }

                /* The peer has ACKed our configuration request. Move to the 
                   IPCP completed state.  */
                ppp_ptr -> nx_ppp_ipcp_state =  NX_PPP_IPCP_COMPLETED_STATE;
            
                /* Turn off the timeout for the configuration request.  */
                ppp_ptr -> nx_ppp_timeout =  0;

                /* Set the IP address for the specific interface from the configuration.   */
                nx_ip_interface_address_set(ppp_ptr -> nx_ppp_ip_ptr, ppp_ptr -> nx_ppp_interface_index, 
                                            IP_ADDRESS(ppp_ptr -> nx_ppp_ipcp_local_ip[0], ppp_ptr -> nx_ppp_ipcp_local_ip[1], 
                                                       ppp_ptr -> nx_ppp_ipcp_local_ip[2], ppp_ptr -> nx_ppp_ipcp_local_ip[3]),
                                                       0xffffffff);

                /* Set gateway address.  */
                (ppp_ptr -> nx_ppp_ip_ptr) -> nx_ip_gateway_address =    (ppp_ptr -> nx_ppp_interface_ptr) -> nx_interface_ip_address;
                (ppp_ptr -> nx_ppp_ip_ptr) -> nx_ip_gateway_interface =  ppp_ptr -> nx_ppp_interface_ptr; 

                /* Turn off the timeout.  */
                ppp_ptr -> nx_ppp_timeout =  0;
            }

            else if (code == NX_PPP_IPCP_CONFIGURE_REJECT)
            {

                /* Determine if the ID matches our last transmit ID. If not, just discard it.  */
                if (ppp_ptr -> nx_ppp_transmit_id != ppp_ptr -> nx_ppp_receive_id)
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the invalid frame ID counter.  */
                    ppp_ptr -> nx_ppp_invalid_frame_id++;
#endif

                    /* Discard this request by simply returning!  */
                    return;
                }

                /* Set the NAK option list, just to indicate to the configure request that 
                   the IPCP request without DNS option is what we want.  */
                ppp_ptr -> nx_ppp_naked_list[0] =  1;

                /* Send the configuration request.  */
                _nx_ppp_ipcp_configure_request_send(ppp_ptr, ppp_ptr -> nx_ppp_naked_list);
            }

            else if ((code ==  NX_PPP_IPCP_CONFIGURE_NAK) && (packet_ptr))
            {

                /* Determine if the ID matches our last transmit ID. If not, just discard it.  */
                if (ppp_ptr -> nx_ppp_transmit_id != ppp_ptr -> nx_ppp_receive_id)
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the invalid frame ID counter.  */
                    ppp_ptr -> nx_ppp_invalid_frame_id++;
#endif

                    /* Discard this request by simply returning!  */
                    return;
                }

                /* Extract the information from response... looking for IP address and possibly DNS server IP address.  */
                _nx_ppp_ipcp_response_extract(ppp_ptr, packet_ptr);

                /* Send the configuration request.  */
                _nx_ppp_ipcp_configure_request_send(ppp_ptr, ppp_ptr -> nx_ppp_naked_list);
            }

            else if (code == NX_PPP_IPCP_TIMEOUT)
            {

                /* Increment the retry counter.  */
                ppp_ptr -> nx_ppp_protocol_retry_counter++;
 
                /* Setup the timeout.  */
                ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;

                /* Timeout, send the configuration request.  */
                _nx_ppp_ipcp_configure_request_send(ppp_ptr, ppp_ptr -> nx_ppp_naked_list); 
            }

#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_ipcp_state_machine_unhandled_requests++;
            }
#endif
            break;    
        }

        case NX_PPP_IPCP_COMPLETED_STATE:
        {
    
            /* IPCP is up and operational at this point.  */
            
            /* Process relative to incoming code.  */
            if (code == NX_PPP_IPCP_TERMINATE_REQUEST)
            {

                /* Increment the retry counter.  */
                ppp_ptr -> nx_ppp_protocol_retry_counter++;
 
                /* Setup the timeout.  */
                ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;

                /* ACK the terminate request.  */
                _nx_ppp_ipcp_terminate_ack_send(ppp_ptr);
            
                /* Send terminate request.  */
                _nx_ppp_ipcp_terminate_send(ppp_ptr);

                /* Move into the IPCP stopping state.  */
                ppp_ptr -> nx_ppp_ipcp_state =  NX_PPP_IPCP_STOPPING_STATE;
            }
            else if ((code == NX_PPP_IPCP_CONFIGURE_REQUEST) && (packet_ptr))
            {
                /* The peer has sent a configuration request.  */ 
            
                /* Check configuration information.  */
                acceptable_configuration =  _nx_ppp_ipcp_configure_check(ppp_ptr, packet_ptr, ppp_ptr -> nx_ppp_peer_naked_list, ppp_ptr -> nx_ppp_rejected_list, good_data);

                /* Check for acceptable configuration.  */
                if (acceptable_configuration == NX_TRUE) 
                {

                    /* Yes, send an ack.  */
                    _nx_ppp_ipcp_response_send(ppp_ptr, NX_PPP_IPCP_CONFIGURE_ACK, good_data, good_data[1], packet_ptr);
                }
                else 
                {
                    /* Otherwise, configuration is unacceptable, send a nak.  */

                    /* Check rejected list.  */
                    if (ppp_ptr -> nx_ppp_rejected_list[0] != 0)
                    {

                        /* Yes, there are rejected options so send a new request.  */
                        _nx_ppp_ipcp_response_send(ppp_ptr, NX_PPP_IPCP_CONFIGURE_REJECT, &ppp_ptr -> nx_ppp_rejected_list[1], ppp_ptr -> nx_ppp_rejected_list[0], NX_NULL);
                    }
                    else if (ppp_ptr -> nx_ppp_peer_naked_list[0] != 0)
                    {

                        /* Yes, there are naked options so send a new request.  */
                        _nx_ppp_ipcp_response_send(ppp_ptr, NX_PPP_IPCP_CONFIGURE_NAK, &ppp_ptr -> nx_ppp_peer_naked_list[1], ppp_ptr -> nx_ppp_peer_naked_list[0], NX_NULL);
                    }
                }
            }

#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_ipcp_state_machine_unhandled_requests++;
            }
#endif
            break;
        }
        
        case NX_PPP_LCP_STOPPING_STATE:
        {
    
            /* We received a terminate request from the other side and just need to get the ACK.  */
            
            /* Process relative to incoming code.  */
            if (code == NX_PPP_IPCP_TERMINATE_ACK)
            {

                /* Move the the stopped state.  */
                ppp_ptr -> nx_ppp_ipcp_state =  NX_PPP_IPCP_STOPPED_STATE;
            }        
            else if (code == NX_PPP_IPCP_TIMEOUT)
            {

                /* Increment the retry counter.  */
                ppp_ptr -> nx_ppp_protocol_retry_counter++;
 
                /* Determine if the IPCP retry counter has been exceeded.  */
                if (ppp_ptr -> nx_ppp_protocol_retry_counter < NX_PPP_MAX_IPCP_PROTOCOL_RETRIES)
                {

                    /* Setup the timeout.  */
                    ppp_ptr -> nx_ppp_timeout =  NX_PPP_PROTOCOL_TIMEOUT;
        
                    /* Send terminate request.  */
                    _nx_ppp_ipcp_terminate_send(ppp_ptr);
                }
                else
                {
                
                    /* Retry counter exceeded.  */

                    /* Enter IPCP failed state.  PPP must be stopped and started to try again.  */
                    ppp_ptr -> nx_ppp_ipcp_state =  NX_PPP_IPCP_FAILED_STATE;

                    /* Determine if the application has registered a link down notification
                       callback.  */
                    if (ppp_ptr -> nx_ppp_link_down_callback)
                    {

                        /* Yes, call the application's callback function.  */
                        (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
                    }
                }
            }
#ifndef NX_PPP_DISABLE_INFO
            else
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_ipcp_state_machine_unhandled_requests++;
            }
#endif
            break;
        }

        default:
        {

#ifndef NX_PPP_DISABLE_INFO
            {

                /* Increment the number of unhandled state machine events.   */
                ppp_ptr -> nx_ppp_ipcp_state_machine_unhandled_requests++;
            }
#endif
            break;
        }
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_ipcp_configure_check                        PORTABLE C      */ 
/*                                                           6.1.2        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes the IPCP message options.                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    packet_ptr                            IPCP packet pointer           */ 
/*    naked_list                            NAKed option list             */ 
/*    rejected_list                         Rejected option list          */ 
/*    good_data                             Good options                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                                              */ 
/*       NX_TRUE  1 indicates no errors, address obtained                 */
/*       NX_FALSE 0 indicates an error or unknown option or address       */  
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_ipcp_state_machine_update     IPCP state machine processing */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  11-09-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            improved packet length      */
/*                                            verification,               */
/*                                            resulting in version 6.1.2  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ppp_ipcp_configure_check(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr, UCHAR *naked_list, UCHAR *rejected_list, UCHAR *good_data)
{

UINT    length, ip_stat, status;
UINT    i, w, m; 
UINT    nak_length = 0;
UINT    rej_length = 0;
UINT    good_index = 0;
UINT    nak_index = 0;
UINT    rej_index = 0;
UCHAR   option;   


    /* Extract the length. */
    length = ((UINT)(packet_ptr -> nx_packet_prepend_ptr[4] << 8) | (UINT)packet_ptr -> nx_packet_prepend_ptr[5]);

    /* Subtract 4 to remove the code, id, and length bytes from the length.  */
    length = length - 4;

    /* Initialize the rejected and naked lists. */
    rejected_list[0] =  naked_list[0] =  good_data[0] =  0;

    /* Loop to process the IPCP configuration options.  */
    for(w = 0, rej_index = 1, status = 1, nak_index = 1; (w + 2) <= length;)
    {

        UINT opt_length;

        /* Pickup the IPCP option.  */
        option =  packet_ptr -> nx_packet_prepend_ptr[w + 6];

        /* Pickup the IPCP option length.  */
        opt_length =  (UINT) packet_ptr -> nx_packet_prepend_ptr[w + 7];

        /* Determine if the length is valid.  */
        if ((opt_length < 2) || (opt_length > (length - w)))
        {

#ifndef NX_PPP_DISABLE_INFO
          
            /* Increment the internal overflow counter.  */
            ppp_ptr -> nx_ppp_packet_overflow++;
#endif

            /* Something is wrong, just get out of the loop!  */
            return(status);
        }

        /* Adjust the option length.  */
        opt_length -= 2;
         
        /* Process relative to the option.  */
        switch (option)
        {
                 
            case NX_PPP_IP_ADDRESS_OPTION:      
            {

                /* Check if out of boundary.  */
                if ((opt_length != 4) || ((good_index + 6) > NX_PPP_OPTION_MESSAGE_LENGTH))
                    return(NX_FALSE);

                /* IP address option.  */
                good_data[good_index++] =  NX_PPP_IP_ADDRESS_OPTION;
                good_data[good_index++] =  6;

                ip_stat =  0;
                for(i = 0; i < 4; i++)
                {
                    
                    /* Pick up each byte of the IP address.  */
                    good_data[i + good_index] =  packet_ptr -> nx_packet_prepend_ptr[w + i + 8];
    
                    /* OR in the IP address bytes to make it easy to see if we got something.  */
                    ip_stat |=  good_data[i + good_index];
                }          

                /* Adjust the main index.  */
                w += 6; 
    
                /* Check if we really have an IP address.  */
                if (!ip_stat)
                {
    
                    /* Copy default IP address.  */            
                    for(i = 0; i < 4; i++)
                    {
                        good_data[i + good_index] =  ppp_ptr -> nx_ppp_ipcp_peer_ip[i];
                    }

                    /* Setup NAK list so we can get the IP address hint.  */
                    for(i = 0; i < 6; i++)
                    {
                        naked_list[nak_index++] =  good_data[i]; 
                    }

                    nak_length = nak_index - 1;
    
                    /* Clear status.  */
                    status =  0;
                }
                else
                {
       
                    /* We did get an IP address for the peer. Remember it!  */
                    for (i = 0; i < 4; i++)
                    {
                        
                        /* Remember each byte of peer's IP address.  */
                        ppp_ptr -> nx_ppp_ipcp_peer_ip[i] =  good_data[i + good_index];
                    }                      
                }

                good_index += opt_length;

                break;
             }

             case NX_PPP_IP_COMPRESSION_OPTION:
             {
         
                /* IP Compression Option.  Since this is the receiver telling us what they 
                   support it is not important to us since we don't support compression.  */
    
                /* Simply adjust the main index into the option list.  */
                w += (opt_length + 2); /* skip */
                break;
             }

            case NX_PPP_DNS_SERVER_OPTION:
             {

                /* Check if out of boundary.  */
                if ((opt_length != 4) || ((good_index + 6) > NX_PPP_OPTION_MESSAGE_LENGTH))
                    return(NX_FALSE);

                 /* Only request a hint if we don't have already have a dns address . */
                 good_data[good_index++] =  NX_PPP_DNS_SERVER_OPTION;
                 good_data[good_index++] =  6;
                 ip_stat =  0;
                 for(i = 0; i < 4; i++)
                 {

                     /* Pick up each byte of the primary DNS address.  */
                     good_data[i + good_index] =  packet_ptr -> nx_packet_prepend_ptr[w + i + 8];

                     /* Or in the IP address bytes to make it easy to see if we got something.  */
                     ip_stat |=  good_data[i + good_index];
                 }
    
                 /* Adjust the main index.  */
                 w += 6; 

                 /* Check if we really have an primary DNS address.  */
                 if (!ip_stat)
                 {


                     /* Update the number of retries on secondary  address requests. */
                     ppp_ptr -> nx_ppp_dns_address_retries++;

                     /* Check if we have reached the limit on retries. */
                     if (ppp_ptr -> nx_ppp_dns_address_retries >= NX_PPP_DNS_ADDRESS_MAX_RETRIES)
                     {
                         /* We have. Return successful result and let IPCP complete. */
                         break;
                     }

                    /* Copy default DNS IP address.  */            
                     good_data[good_index] =  (UCHAR) (ppp_ptr -> nx_ppp_primary_dns_address >> 24);
                     good_data[good_index + 1] =  (UCHAR) ((ppp_ptr -> nx_ppp_primary_dns_address >> 16) & 0xff);
                     good_data[good_index + 2] = (UCHAR) ((ppp_ptr -> nx_ppp_primary_dns_address >> 8) & 0xff);
                     good_data[good_index + 3]= (UCHAR) (ppp_ptr -> nx_ppp_primary_dns_address & 0xff);

                    /* Setup NAK list so we can get the DNS IP address hint.  */
                    for(i = 0; i < 6; i++)
                    {
                        naked_list[nak_index++] =  good_data[good_index + i - 2]; 
                    }

                     /* Clear status.  */
                     status =  0;

                     /* Update the size of the nak and reject data. */
                     nak_length = nak_index - 1;
                 }
                 else 
                 {

                     /* Copy good IP address.  */
                     ppp_ptr -> nx_ppp_primary_dns_address = ((ULONG)good_data[good_index] << 24) & 0xFF000000;
                     ppp_ptr -> nx_ppp_primary_dns_address |= (good_data[good_index + 1] << 16) & 0x00FF0000;
                     ppp_ptr -> nx_ppp_primary_dns_address |= (good_data[good_index + 2] << 8) & 0x0000FF00;
                     ppp_ptr -> nx_ppp_primary_dns_address |= good_data[good_index + 3] & 0x000000FF;
                 }

                 good_index += opt_length;

                 break;
             }

             case NX_PPP_DNS_SECONDARY_SERVER_OPTION:
             {

                /* Check if out of boundary.  */
                if ((opt_length != 4) || ((good_index + 6) > NX_PPP_OPTION_MESSAGE_LENGTH))
                    return(NX_FALSE);

                 /* Only request a hint if we don't have already have a dns address . */
                 good_data[good_index++] =  NX_PPP_DNS_SECONDARY_SERVER_OPTION;
                 good_data[good_index++] =  6;
                 ip_stat =  0;
                 for(i = 0; i < 4; i++)
                 {

                     /* Pick up each byte of the secondary DNS address.  */
                     good_data[i + good_index] =  packet_ptr -> nx_packet_prepend_ptr[w + i + 8];

                     /* Or in the IP address bytes to make it easy to see if we got something.  */
                     ip_stat |=  good_data[i + good_index];
                 }
    
                 /* Adjust the main index.  */
                 w += 6; 

                 /* Check if we really have an primary DNS address.  */
                 if (!ip_stat)
                 {

                     /* Update the number of retries on primary address requests. */
                     ppp_ptr -> nx_ppp_secondary_dns_address_retries++;

                     /* Check if we have reached the limit on retries. */
                     if (ppp_ptr -> nx_ppp_secondary_dns_address_retries >= NX_PPP_DNS_ADDRESS_MAX_RETRIES)
                     {
                         /* We have. Return successful result and let IPCP complete. */
                         break;
                     }

                    /* Copy the primary DNS IP address from the packet data.  */            
                     good_data[good_index] =  (UCHAR) (ppp_ptr -> nx_ppp_secondary_dns_address >> 24);
                     good_data[good_index + 1] =  (UCHAR) ((ppp_ptr -> nx_ppp_secondary_dns_address >> 16) & 0xff);
                     good_data[good_index + 2] = (UCHAR) ((ppp_ptr -> nx_ppp_secondary_dns_address >> 8) & 0xff);
                     good_data[good_index + 3]= (UCHAR) (ppp_ptr -> nx_ppp_secondary_dns_address & 0xff);

                    /* Setup NAK list so we can get the DNS IP address hint.  */
                    for(i = 0; i < 6; i++)
                    {
                        naked_list[nak_index++] =  good_data[good_index + i - 2]; 
                    }

                     /* Clear status.  */
                     status =  0;

                     /* Update the size of the nak and reject data. */
                     nak_length = nak_index - 1;
                 }

                 else 
                 {

                     /* Copy good IP address.  */
                     ppp_ptr -> nx_ppp_secondary_dns_address = ((ULONG)good_data[good_index] << 24) & 0xFF000000;
                     ppp_ptr -> nx_ppp_secondary_dns_address |= (good_data[good_index + 1] << 16) & 0x00FF0000;
                     ppp_ptr -> nx_ppp_secondary_dns_address |= (good_data[good_index + 2] << 8) & 0x0000FF00;
                     ppp_ptr -> nx_ppp_secondary_dns_address |= good_data[good_index + 3] & 0x000000FF;
                 }

                 good_index += opt_length;

                 break;
             }

            default:                             
            {
            
                /* All other options get NAKed. */
                status = 0;        
    
                /* Determine if the option will fit.  */
                if ((rej_index + opt_length + 2) >= NX_PPP_OPTION_MESSAGE_LENGTH)
                {
    
#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the internal overflow counter.  */
                    ppp_ptr -> nx_ppp_packet_overflow++;
#endif
                
                    /* Return status.  */
                    return(status);
                }
    
                /* Place the option in the rejected list.  */
                rejected_list[rej_index++] = option;
                rejected_list[rej_index++] = (UCHAR)(opt_length + 2);
    
                /* Loop to copy the rest of the options into the rejected list.  */
                for (m = 0; m < opt_length; m++)
                {
            
                    /* Copy option byte into the rejected option list.  */
                    rejected_list[rej_index] =  packet_ptr -> nx_packet_prepend_ptr[w + m + 8];
                    rej_index++;
                }

                rej_length = rej_index - 1;
    
                /* Adjust the main index into the option list.  */
                w += (opt_length + 2); /* skip */
                break;        
            }
        }
    }

    /* Setup the length in the naked and rejected lists.  */
    naked_list[0] = (UCHAR)nak_length; 
    rejected_list[0] = (UCHAR)rej_length; 

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_ipcp_configure_request_send                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds a configure request message and sends it out.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    negotiate_list                        List of options to negotiate  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a packet for sending */ 
/*    _nx_ppp_packet_transmit               Send IPCP packet              */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_ipcp_state_machine_update     IPCP state machine processing */ 
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
void  _nx_ppp_ipcp_configure_request_send(NX_PPP *ppp_ptr, UCHAR *negotiate_list)
{

UINT        status, i;
NX_PACKET   *packet_ptr;
UINT        index;


    /* Allocate a packet for the IPCP packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* An error was detected, simply return a NULL pointer.  */
        return;
    }

    /* Clear the packet memory.*/
    memset(packet_ptr -> nx_packet_prepend_ptr, 0, (UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr));

    /* Adjust the transmit ID.  */
    ppp_ptr -> nx_ppp_transmit_id++;

    /* Build IPCP request message.  */
    packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_IPCP_PROTOCOL & 0xFF00) >> 8;
    packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_IPCP_PROTOCOL & 0xFF;
    packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_IPCP_CONFIGURE_REQUEST;
    packet_ptr -> nx_packet_prepend_ptr[3] =  ppp_ptr -> nx_ppp_transmit_id;
    packet_ptr -> nx_packet_prepend_ptr[4] =  0x00;
    packet_ptr -> nx_packet_prepend_ptr[5] =  0x0A;  /* Size of the IPCP data */
    packet_ptr -> nx_packet_prepend_ptr[6] =  0x03;
    packet_ptr -> nx_packet_prepend_ptr[7] =  0x06;

    /* Load up the default IP address.  */
    for(i = 0; i < 4; i++)
    {

        /* Copy byte of IP address.  */
        packet_ptr -> nx_packet_prepend_ptr[i+8] =  ppp_ptr -> nx_ppp_ipcp_local_ip[i];
    }

    index = i + 8;

    /* Determine if we want the DNS option ...  */
    if (negotiate_list[0] == 0)
    {        

        /* Yes (because it is not obvious from the code style), let's try to get the DNS server.  */

        /* Update the IPCP length for DNS address.  */  
        packet_ptr -> nx_packet_prepend_ptr[5] =  (UCHAR)(packet_ptr -> nx_packet_prepend_ptr[5] + 6);

        /* Add the option for retrieving the DNS server as well.  */
        packet_ptr -> nx_packet_prepend_ptr[index] = NX_PPP_DNS_SERVER_OPTION;
        packet_ptr -> nx_packet_prepend_ptr[index+1] = 0x06;
        packet_ptr -> nx_packet_prepend_ptr[index+2] = (UCHAR) (ppp_ptr -> nx_ppp_primary_dns_address >> 24);
        packet_ptr -> nx_packet_prepend_ptr[index+3] = (UCHAR) ((ppp_ptr -> nx_ppp_primary_dns_address >> 16) & 0xff);
        packet_ptr -> nx_packet_prepend_ptr[index+4] = (UCHAR) ((ppp_ptr -> nx_ppp_primary_dns_address >> 8) & 0xff);
        packet_ptr -> nx_packet_prepend_ptr[index+5] = (UCHAR) (ppp_ptr -> nx_ppp_primary_dns_address & 0xff);
        index += 6;

        /* Also check for a secondary DNS address request. */
        if (negotiate_list[1] == 0)
        {

            /* Update the IPCP length for secondary DNS address.  */  
            packet_ptr -> nx_packet_prepend_ptr[5] =  (UCHAR)(packet_ptr -> nx_packet_prepend_ptr[5] + 6);

            /* Add the option for retrieving the DNS server as well.  */
            packet_ptr -> nx_packet_prepend_ptr[index] = NX_PPP_DNS_SECONDARY_SERVER_OPTION;  
            packet_ptr -> nx_packet_prepend_ptr[index+1] = 0x06;
            packet_ptr -> nx_packet_prepend_ptr[index+2] = (UCHAR) (ppp_ptr -> nx_ppp_secondary_dns_address >> 24);
            packet_ptr -> nx_packet_prepend_ptr[index+3] = (UCHAR) ((ppp_ptr -> nx_ppp_secondary_dns_address >> 16) & 0xff);
            packet_ptr -> nx_packet_prepend_ptr[index+4] = (UCHAR) ((ppp_ptr -> nx_ppp_secondary_dns_address >> 8) & 0xff);
            packet_ptr -> nx_packet_prepend_ptr[index+5] = (UCHAR) (ppp_ptr -> nx_ppp_secondary_dns_address & 0xff);
            index += 6;
        }
    }

    /* Setup the append pointer and the packet length.  */
    packet_ptr -> nx_packet_length =  index;
    
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

#ifndef NX_PPP_DISABLE_INFO

    /* Increment the number of IPCP frames sent.  */
    ppp_ptr -> nx_ppp_ipcp_frames_sent++;

    /* Increment the number of IPCP configure requests sent.  */
    ppp_ptr -> nx_ppp_ipcp_configure_requests_sent++;
#endif

    /* Send IPCP configure request packet.  */
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_ipcp_response_extract                       PORTABLE C      */ 
/*                                                           6.1.2        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts information from the IPCP response           */ 
/*    message.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    packet_ptr                            Pointer to IPCP packet        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_ipcp_state_machine_update     IPCP state machine processing */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  11-09-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            improved packet length      */
/*                                            verification,               */
/*                                            resulting in version 6.1.2  */
/*                                                                        */
/**************************************************************************/
void  _nx_ppp_ipcp_response_extract(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr)
{

UINT    i, j;    
ULONG   length;
    
    
   
    /* Search for IP address and DNS address in response.  */
    
    /* Setup the packet length.  */
    length =  (((ULONG) packet_ptr -> nx_packet_prepend_ptr[4]) << 8) | ((ULONG) packet_ptr -> nx_packet_prepend_ptr[5]);
    
    /* Subtract the request type, id, and length.  */
    if (length >= 4)
        length =  length - 4;
    else
        length =  0;

    /* Loop to parse the options to look for primary DNS address.  */
    i =  6;
    while (length)
    {
    
        /* Subtract the length.  */
        if (length >= ((ULONG) packet_ptr -> nx_packet_prepend_ptr[i+1]))
            length =  length - ((ULONG) packet_ptr -> nx_packet_prepend_ptr[i+1]);
        else
            length =  0;

        /* Check for the IP address.  */
        if (packet_ptr -> nx_packet_prepend_ptr[i] == NX_PPP_IP_ADDRESS_OPTION)
        {


            /* Copy the IP address bytes into the PPP storage.  */
            for(j = 0; j < 4; j++)
            {

                /* Copy a byte of the IP address. */
                ppp_ptr -> nx_ppp_ipcp_local_ip[j] =  packet_ptr -> nx_packet_prepend_ptr[i+j+2];
            }
        }

        /* Check for the primary DNS option. */
        else if (packet_ptr -> nx_packet_prepend_ptr[i] == NX_PPP_DNS_SERVER_OPTION)
        {
    
            /* Yes, we have a primary DNS address.  */
            
            /* Let's copy it into the PPP structure in case we need it later.  */
            ppp_ptr -> nx_ppp_primary_dns_address =  ((((ULONG) packet_ptr -> nx_packet_prepend_ptr[i+2]) << 24) |
                                                     (((ULONG) packet_ptr -> nx_packet_prepend_ptr[i+3]) << 16) |
                                                     (((ULONG) packet_ptr -> nx_packet_prepend_ptr[i+4]) << 8)  |
                                                      ((ULONG) packet_ptr -> nx_packet_prepend_ptr[i+5]));

        }

        /* Check for the primary DNS option. */
        else if (packet_ptr -> nx_packet_prepend_ptr[i] == NX_PPP_DNS_SECONDARY_SERVER_OPTION)
        {
    
            /* Yes, we have a secondary DNS address.  */

            /* Let's copy it into the PPP structure in case we need it later.  */
            ppp_ptr -> nx_ppp_secondary_dns_address =  ((((ULONG) packet_ptr -> nx_packet_prepend_ptr[i+2]) << 24) |
                                                     (((ULONG) packet_ptr -> nx_packet_prepend_ptr[i+3]) << 16) |
                                                     (((ULONG) packet_ptr -> nx_packet_prepend_ptr[i+4]) << 8)  |
                                                      ((ULONG) packet_ptr -> nx_packet_prepend_ptr[i+5]));


            /* Break out of the loop.  */
            break;
        }
        
        /* Otherwise move to the next option. */
        if (length >= ((ULONG) packet_ptr -> nx_packet_prepend_ptr[i+1]))
        {
        
            /* Yes, there is another option.  */
            
            /* Position to the next option.  */
            i =  i + (((UINT) packet_ptr -> nx_packet_prepend_ptr[i+1]));
        }
        else
        {
        
            /* Make sure the length is 0.  */
            length =  0;
        }
    } 
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_ipcp_response_send                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds and sends an IPCP response message.            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    type                                  ACK or NAK selection          */ 
/*    data                                  Previous IPCP message         */ 
/*    length                                Length of previous IPCP msg   */ 
/*    packet_ptr                            Request packet pointer        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a packet for sending */ 
/*    _nx_ppp_packet_transmit               Send frame                    */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_ipcp_state_machine_update     IPCP state machine processing */ 
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
void  _nx_ppp_ipcp_response_send(NX_PPP *ppp_ptr, UCHAR type, UCHAR *data, UCHAR length, NX_PACKET *cfg_packet_ptr)
{

UINT        status, i;
NX_PACKET   *packet_ptr;


    /* Allocate a packet for the IPCP packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* An error was detected, simply return a NULL pointer.  */
        return;
    }

    /* Determine if an ACK is requested.  */
    if (type == NX_PPP_IPCP_CONFIGURE_ACK) 
    {

        /* Check if out of boundary.  */
        if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < (UINT)(cfg_packet_ptr -> nx_packet_length))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);
            return;
        }

        /* Yes, an ACK is requested.  Simply copy the config packet into the new packet and change 
           the type. */
        for (i = 0; i < cfg_packet_ptr -> nx_packet_length; i++)
        {

            /* Determine if the copy exceeds the payload.  */
            if (&packet_ptr -> nx_packet_prepend_ptr[i] >= packet_ptr -> nx_packet_data_end)
            {

#ifndef NX_PPP_DISABLE_INFO

                /* Increment the internal error counter.  */
                ppp_ptr -> nx_ppp_internal_errors++;
#endif

                /* Release the packet.  */
                nx_packet_release(packet_ptr);

                /* Simply return.  */
                return;
            }

            /* Copy one byte.  */
            packet_ptr -> nx_packet_prepend_ptr[i] =  cfg_packet_ptr -> nx_packet_prepend_ptr[i];
        }

        /* Adjust the type of the new packet to represent an ACK.  */
        packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_IPCP_CONFIGURE_ACK;

        /* Adjust the length and append pointers of the packet.  */
        packet_ptr -> nx_packet_length =  cfg_packet_ptr -> nx_packet_length;
        packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;
    }
    else
    {

        /* Check if out of boundary.  */
        if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < (UINT)(length + 6))
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);
            return;
        }

        /* Build IPCP response message.  */
        packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_IPCP_PROTOCOL & 0xFF00) >> 8;
        packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_IPCP_PROTOCOL & 0xFF;
        packet_ptr -> nx_packet_prepend_ptr[2] =  type;
        packet_ptr -> nx_packet_prepend_ptr[3] =  ppp_ptr -> nx_ppp_receive_id;
        packet_ptr -> nx_packet_prepend_ptr[4] =  0x00;
        packet_ptr -> nx_packet_prepend_ptr[5] =  (UCHAR)(length + 4);

        /* Load up the default IP address.  */
        for(i = 0; i < length; i++)
        {

            /* Copy byte of IP address.  */
            packet_ptr -> nx_packet_prepend_ptr[i+6] =  data[i];
        }

        /* Setup the append pointer and the packet length.  */
        packet_ptr -> nx_packet_length =  (UCHAR)(length + 4 + 2);
        packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;
    }

#ifndef NX_PPP_DISABLE_INFO

    /* Increment the number of IPCP frames sent.  */
    ppp_ptr -> nx_ppp_ipcp_frames_sent++;

    /* Increment counters based on type of response.  */
    if (type == NX_PPP_IPCP_CONFIGURE_ACK)
    {
    
        /* Increment the number of IPCP configure ACKs sent.  */
        ppp_ptr -> nx_ppp_ipcp_configure_acks_sent++;
    }
    else if (type == NX_PPP_IPCP_CONFIGURE_NAK)
    {
    
        /* Increment the number of IPCP configure NAKs sent.  */
        ppp_ptr -> nx_ppp_ipcp_configure_naks_sent++;
    }
    else if (type == NX_PPP_IPCP_CONFIGURE_REJECT)
    {
    
        /* Increment the number of IPCP configure rejects sent.  */
        ppp_ptr -> nx_ppp_ipcp_configure_rejects_sent++;
    }
#endif

    /* Send IPCP response.  */
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);               
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_ipcp_terminate_send                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds a terminate request message and sends          */ 
/*    it out.                                                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a packet for sending */ 
/*    _nx_ppp_packet_transmit               Send AHDLC packet             */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_ipcp_state_machine_update     IPCP state machine processing */ 
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
void  _nx_ppp_ipcp_terminate_send(NX_PPP *ppp_ptr)
{

UINT        status;
NX_PACKET   *packet_ptr;


    /* Allocate a packet for the IPCP packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* An error was detected, simply return a NULL pointer.  */
        return;
    }

    /* Build IPCP terminate message.  */
    packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_IPCP_PROTOCOL & 0xFF00) >> 8;
    packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_IPCP_PROTOCOL & 0xFF;
    packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_IPCP_TERMINATE_REQUEST;
    packet_ptr -> nx_packet_prepend_ptr[3] =  ppp_ptr -> nx_ppp_transmit_id;
    packet_ptr -> nx_packet_prepend_ptr[4] =  0x00;
    packet_ptr -> nx_packet_prepend_ptr[5] =  0x04;

    /* Setup the append pointer and the packet length.  */
    packet_ptr -> nx_packet_length = 6;
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

#ifndef NX_PPP_DISABLE_INFO

    /* Increment the number of IPCP frames sent.  */
    ppp_ptr -> nx_ppp_ipcp_frames_sent++;

    /* Increment the number of IPCP terminate sent.  */
    ppp_ptr -> nx_ppp_ipcp_terminate_requests_sent++;
#endif

    /* Send IPCP terminate packet.  */    
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_ipcp_terminate_ack_send                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds a terminate ACK request message and sends      */ 
/*    it out.                                                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a packet for sending */ 
/*    _nx_ppp_packet_transmit               Send AHDLC packet             */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_ipcp_state_machine_update     IPCP state machine processing */ 
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
void  _nx_ppp_ipcp_terminate_ack_send(NX_PPP *ppp_ptr)
{

UINT        status;
NX_PACKET   *packet_ptr;


    /* Allocate a packet for the IPCP packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

#ifndef NX_PPP_DISABLE_INFO

        /* Increment the number of packet allocation timeouts.  */ 
        ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

        /* An error was detected, simply return a NULL pointer.  */
        return;
    }

    /* Build IPCP terminate ACK message.  */
    packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_IPCP_PROTOCOL & 0xFF00) >> 8;
    packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_IPCP_PROTOCOL & 0xFF;
    packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_IPCP_TERMINATE_ACK;
    packet_ptr -> nx_packet_prepend_ptr[3] =  ppp_ptr -> nx_ppp_receive_id;
    packet_ptr -> nx_packet_prepend_ptr[4] =  0x00;
    packet_ptr -> nx_packet_prepend_ptr[5] =  0x04;

    /* Setup the append pointer and the packet length.  */
    packet_ptr -> nx_packet_length = 6;
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

#ifndef NX_PPP_DISABLE_INFO

    /* Increment the number of IPCP frames sent.  */
    ppp_ptr -> nx_ppp_ipcp_frames_sent++;

    /* Increment the number of IPCP terminate ACKs sent.  */
    ppp_ptr -> nx_ppp_ipcp_terminate_acks_sent++;
#endif

    /* Send IPCP terminate ACK packet.  */    
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_packet_transmit                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds a CRC to the current transmit buffer and then    */ 
/*    sends the buffer via the user supplied byte output routine.         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    packet_ptr                            Pointer to PPP packet         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    [_nx_ppp_debug_log_capture]           Put send frame in debug log   */ 
/*    _nx_ppp_crc_append                    Add PPP CRC                   */ 
/*    (nx_ppp_byte_send)                    User supplied byte send       */ 
/*    nx_packet_transmit_release            Release the transmit packet   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_process_deferred_ip_packet_send  Process deferred IP packet */ 
/*    _nx_ppp_lcp_code_reject               Send LCP code reject message  */ 
/*    _nx_ppp_lcp_configure_reply_send      Send LCP configure reply      */ 
/*    _nx_ppp_lcp_configure_request_send    Send LCP configure request    */ 
/*    _nx_ppp_lcp_terminate_ack_send        Send LCP terminate ACK        */ 
/*    _nx_ppp_lcp_terminate_request_send    Send LCP terminate request    */ 
/*    _nx_ppp_pap_authentication_request    PAP authentication request    */ 
/*    _nx_ppp_pap_authentication_ack        PAP authentication ACK        */ 
/*    _nx_ppp_pap_authentication_nak        PAP authentication NAK        */ 
/*    _nx_ppp_chap_challenge_send           CHAP challenge send           */ 
/*    _nx_ppp_chap_challenge_respond        CHAP challenge respond        */ 
/*    _nx_ppp_chap_challenge_validate       CHAP challenge validate       */ 
/*    _nx_ppp_ipcp_configure_request_send   Send IPCP configure request   */ 
/*    _nx_ppp_ipcp_response_send            Send IPCP response            */ 
/*    _nx_ppp_ipcp_terminate_send           Send IPCP terminate           */ 
/*    _nx_ppp_ipcp_terminate_ack_send       Send IPCP terminate ACK       */ 
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
void  _nx_ppp_packet_transmit(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr)
{

NX_PACKET   *current_packet_ptr;
UINT        i;
UCHAR       byte; 
UCHAR       *buffer_ptr;
UCHAR       hdlc[3] = {0x7e, 0xff, 0x03};
UCHAR       crc[2];
#ifdef NX_PPP_PPPOE_ENABLE
UINT        release_packet = NX_TRUE;
#endif /* NX_PPP_PPPOE_ENABLE  */


#ifdef NX_PPP_DEBUG_LOG_ENABLE

    /* Place the outgoing frame into the optional PPP debug log.  */
    _nx_ppp_debug_log_capture(ppp_ptr, 'S', packet_ptr);
#endif

    /* Check for send function.  */
    if (ppp_ptr -> nx_ppp_byte_send)
    {

        /* Step1. Send the HDLC bytes.  */
        for (i = 0; i < 3; i++)
        {

            /* Pickup the byte.  */
            byte =  hdlc[i];

            /* Determine if it needs an escape sequence.  */
            if ((byte < ((UCHAR) 0x20)) || ((byte == ((UCHAR) 0x7e)) && (i != 0)) || (byte == ((UCHAR) 0x7d)))
            {

                /* Yes, this byte needs an escape sequence.  */

                /* Change the byte.  */
                byte =  byte ^ 0x20;

#ifndef NX_PPP_DISABLE_INFO

                /* Increment the number of bytes sent.  */
                ppp_ptr -> nx_ppp_bytes_sent++;
#endif

                /* Send the escape sequence byte via the user supplied output routine.  */
                ppp_ptr -> nx_ppp_byte_send(0x7d);
            }

#ifndef NX_PPP_DISABLE_INFO

            /* Increment the number of bytes sent.  */
            ppp_ptr -> nx_ppp_bytes_sent++;
#endif

            /* Call user supplied byte output routine.  */
            ppp_ptr -> nx_ppp_byte_send(byte);
        }

        /* Step2. Send the data.  */
        /* Initialize the current packet pointer to the packet pointer.  */
        current_packet_ptr =  packet_ptr;

        /* Setup the initial buffer pointer.  */
        buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

        /* Loop through the buffer to send each byte out.  */
        for(i = 0; i < packet_ptr -> nx_packet_length; i++)
        {

            /* Determine if we need to move to any additional packets.  */
            if (buffer_ptr >= current_packet_ptr -> nx_packet_append_ptr)
            {

#ifndef NX_DISABLE_PACKET_CHAIN

                /* Yes, we need to move to the next packet.  */
                current_packet_ptr =  current_packet_ptr -> nx_packet_next;

                /* Determine if there is a next packet.  */
                if (current_packet_ptr == NX_NULL)
                {

#ifndef NX_PPP_DISABLE_INFO

                    /* Increment the internal error counter.  */
                    ppp_ptr -> nx_ppp_internal_errors++; 
#endif

                    /* Get out of the loop.  */
                    break;
                }

                /* Otherwise setup new buffer pointer.  */
                buffer_ptr =  current_packet_ptr -> nx_packet_prepend_ptr;
#else
                break;

#endif /* NX_DISABLE_PACKET_CHAIN */

            }

            /* Pickup the next byte to be sent.  */
            byte =  *buffer_ptr++;

            /* Determine if this byte needs an escape sequence.  */
            if ((byte < ((UCHAR) 0x20)) || (byte == 0x7e) || (byte == 0x7d))
            {

                /* Yes, this byte needs an escape sequence.  */

                /* Change the byte.  */
                byte =  byte ^ 0x20;

#ifndef NX_PPP_DISABLE_INFO

                /* Increment the number of bytes sent.  */
                ppp_ptr -> nx_ppp_bytes_sent++;
#endif

                /* Send the escape sequence byte via the user supplied output routine.  */
                ppp_ptr -> nx_ppp_byte_send(0x7d);
            }

#ifndef NX_PPP_DISABLE_INFO

            /* Increment the number of bytes sent.  */
            ppp_ptr -> nx_ppp_bytes_sent++;
#endif

            /* Call user supplied byte output routine.  */
            ppp_ptr -> nx_ppp_byte_send(byte);
        }

        /* Step3. Send CRC.  */
        /* Place a CRC at the end of the transmit buffer.  */
         _nx_ppp_crc_append(packet_ptr, crc);

        /* Now send the CRC and end-of-frame bytes.  */
        for (i = 0; i < 2; i++)
        {

            /* Pickup the byte.  */
            byte =  crc[i];

            /* Determine if it needs an escape sequence.  */
            if ((byte < ((UCHAR) 0x20)) || (byte == ((UCHAR) 0x7e)) || (byte == ((UCHAR) 0x7d)))
            {

                /* Yes, this byte needs an escape sequence.  */

                /* Change the byte.  */
                byte =  byte ^ 0x20;

#ifndef NX_PPP_DISABLE_INFO

                /* Increment the number of bytes sent.  */
                ppp_ptr -> nx_ppp_bytes_sent++;
#endif

                /* Send the escape sequence byte via the user supplied output routine.  */
                ppp_ptr -> nx_ppp_byte_send(0x7d);
            }

#ifndef NX_PPP_DISABLE_INFO

            /* Increment the number of bytes sent.  */
            ppp_ptr -> nx_ppp_bytes_sent++;
#endif

            /* Call user supplied byte output routine.  */
            ppp_ptr -> nx_ppp_byte_send(byte);
        }

        /* Step4. Finally, send an end-of-frame character.  */
        ppp_ptr -> nx_ppp_byte_send((UCHAR) 0x7e);
     }
    
#ifdef NX_PPP_PPPOE_ENABLE

     /* Check the PPPoE packet send function.  */
     if (ppp_ptr -> nx_ppp_packet_send)
     {

         /* Send the packet out.  */
         (ppp_ptr -> nx_ppp_packet_send)(packet_ptr);

         /* Update the flag since this packet should been released in PPPoE.  */
         release_packet = NX_FALSE;
         
     }

     /* Check if need to release the packet.  */
     if (release_packet == NX_TRUE)
     {
#endif /* NX_PPP_PPPOE_ENABLE  */

         /* Remove the PPP header.  */
         packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr + 2;
         packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length - 2;

         /* Release the packet.  */
         nx_packet_transmit_release(packet_ptr);

#ifdef NX_PPP_PPPOE_ENABLE
     }
#endif /* NX_PPP_PPPOE_ENABLE  */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_check_crc                                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes the CRC of the AHDLC packet in the packet   */ 
/*    payload. If the CRC is correct, the CRC is removed from the receive */ 
/*    buffer.                                                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    packet_ptr                            Address of PPP instance       */ 
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
/*    _nx_ppp_receive_packet_get            Receive PPP packet processing */ 
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
UINT  _nx_ppp_check_crc(NX_PACKET *packet_ptr)
{

NX_PACKET   *current_packet_ptr;   
UCHAR       byte;
UCHAR       *buffer_ptr;
USHORT      i;
USHORT      crc_value;
   

    /* Set initial CRC value.  */
    crc_value = 0xffff;
   
    /* Setup buffer pointer.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr + 1;

    /* Setup the current packet pointer.  */
    current_packet_ptr =  packet_ptr;

    /* Loop to process the CRC for the receive buffer.  */
    for(i = 1; i < (packet_ptr -> nx_packet_length - 1); i++)
    {

        /* Determine if the buffer pointer is inside the current packet.  */
        if (buffer_ptr >= current_packet_ptr -> nx_packet_append_ptr)
        {

#ifndef NX_DISABLE_PACKET_CHAIN

            /* We have exhausted this packet and must move to the next.  */
            current_packet_ptr =  current_packet_ptr -> nx_packet_next;

            /* Determine if there is a next packet.  */
            if (current_packet_ptr == NX_NULL)
                break;

            /* Otherwise, we have a valid next packet. Setup the buffer pointer
               to the first byte in the next packet.  */
            buffer_ptr =  current_packet_ptr -> nx_packet_prepend_ptr;
#else
            break;
#endif /* NX_DISABLE_PACKET_CHAIN */

        }

        /* Pickup character from buffer.  */
        byte =  *buffer_ptr++; 

        /* Update the CRC.  */
        crc_value =  (( crc_value >> 8 ) ^ _nx_ppp_crc_table[ ( crc_value & 0xFF ) ^ byte ]);
    }

    /* At this point, the CRC should be 0xf0b8.  */
    if (crc_value != 0xf0b8)
    {

        /* CRC failed, not a valid PPP packet. */
       return(NX_PPP_BAD_PACKET);
    }

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_crc_append                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calculates and appends the two byte CRC to the        */ 
/*    frame. It also adds the end of frame marker after the CRC.          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    packet_ptr                            Pointer to PPP packet         */ 
/*    crc                                   Destination for CRC           */ 
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
/*    _nx_ppp_packet_transmit               PPP packet transmit routine   */ 
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
UINT  _nx_ppp_crc_append(NX_PACKET *packet_ptr, UCHAR crc[2])
{

NX_PACKET   *current_packet_ptr;   
UCHAR       *buffer_ptr;
UINT        i;
USHORT      crc_value;
UCHAR       byte;
UCHAR       address = 0xff;
UCHAR       control = 0x03;


    /* The FCS field is calculated over all bits of the Address, Control,
       Protocol, Information and Padding fields, not including any start
       and stop bits (asynchronous) nor any bits (synchronous) or octets
       (asynchronous or synchronous) inserted for transparency.  This
       also does not include the Flag Sequences nor the FCS field itself.
       RFC1662, Section3.1, Page6.  */

    /* Initialize CRC value.  */
    crc_value = 0xffff;

    /* Step1. Calculate address and control.  */
    crc_value = ( crc_value >> 8 ) ^ _nx_ppp_crc_table[ ( crc_value & 0xFF ) ^ address ];
    crc_value = ( crc_value >> 8 ) ^ _nx_ppp_crc_table[ ( crc_value & 0xFF ) ^ control ];

    /* Step2. Calculate protocol, information and padding fiedls.  */

    /* Initialize the current packet pointer to the packet pointer.  */
    current_packet_ptr =  packet_ptr;

    /* Setup the initial buffer pointer.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Loop to calculate CRC.  */
    for(i = 0; i < packet_ptr -> nx_packet_length; i++)
    {

        /* Determine if we need to move to any additional packets.  */
        if (buffer_ptr >= current_packet_ptr -> nx_packet_append_ptr)
        {

#ifndef NX_DISABLE_PACKET_CHAIN

            /* Yes, we need to move to the next packet.  */
            current_packet_ptr =  current_packet_ptr -> nx_packet_next;

            /* Determine if there is a next packet.  */
            if (current_packet_ptr == NX_NULL)
            {

                /* Get out of the loop.  */
                break;
            }

            /* Otherwise setup new buffer pointer.  */
            buffer_ptr =  current_packet_ptr -> nx_packet_prepend_ptr;
#else
            break;
#endif /* NX_DISABLE_PACKET_CHAIN */

        }

        /* Pickup the next byte to be sent.  */
        byte =  *buffer_ptr++;

        /* Update the CRC.  */
        crc_value = ( crc_value >> 8 ) ^ _nx_ppp_crc_table[ ( crc_value & 0xFF ) ^ byte ];
    }

    /* Now complement the CRC value.  */
    crc_value = crc_value ^ 0xffff; 

    /* Store first byte of the CRC.  */
    crc[0] =  (UCHAR) (crc_value & 0xff);

    /* Store second byte of the CRC.  */
    crc[1] =  (UCHAR) ((crc_value >> 8) & 0xff);

    /* Return a successful completion.  */
    return(NX_SUCCESS);
}


#ifdef NX_PPP_DEBUG_LOG_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_debug_log_capture                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function places the current frame into the circular debug      */ 
/*    log.                                                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP structure      */ 
/*    packet_type                           Either a send or receive      */ 
/*    packet_ptr                            Pointer to PPP packet         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_time_get                           Get time                      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_packet_transmit               Transmit packet processing    */ 
/*    _nx_ppp_thread_entry                  Receive thread processing     */ 
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
void  _nx_ppp_debug_log_capture(NX_PPP *ppp_ptr, UCHAR packet_type, NX_PACKET *packet_ptr)
{

UINT                i;
ULONG               time;
NX_PPP_DEBUG_ENTRY *entry_ptr;

    /* Check for a NULL pointer.  */
    if (packet_ptr == NX_NULL)
        return;

    /* Pickup current time.  */
    time =  tx_time_get();

#ifdef NX_PPP_DEBUG_LOG_PRINT_ENABLE 
    /* Print out the current time stamp.  */
    printf("Time: %lu, ", time);        

    /* Print out the PPP name.  */
    printf("PPP Name: %s, ", ppp_ptr -> nx_ppp_name);

    /* Print out the current PPP state.  */
    printf("PPP State: %u, ", ppp_ptr -> nx_ppp_state);

    /* Print out the current PPP LCP state.  */
    printf("PPP LCP State: %u, ", ppp_ptr -> nx_ppp_lcp_state);

    /* Print out the current PPP PAP state.  */
    printf("PPP PAP State: %u, ", ppp_ptr -> nx_ppp_pap_state);

    /* Print out the current PPP CHAP state.  */
    printf("PPP CHAP State: %u, ", ppp_ptr -> nx_ppp_chap_state);

    /* Print out the current IPCP state.  */
    printf("PPP IPCP State: %u, ", ppp_ptr -> nx_ppp_ipcp_state);

     /* Print out the authentication flag.  */
     if (ppp_ptr -> nx_ppp_authenticated)
         printf("Authenticated, ");
     else
         printf("Not Authenticated, ");

     /* Determine if the packet is a receive packet or a send packet.  */
     if (packet_type == 'R')
         printf("Received Packet Length: %lu, Packet: ", packet_ptr -> nx_packet_length);
     else
         printf("Send Packet Length: %lu, Packet: ", packet_ptr -> nx_packet_length);

    /* Dump out at least part of the packet payload.  */
    for (i = 0; i < packet_ptr -> nx_packet_length; i++)
    {

        /* Check for packet spanning multiple packets.  */
        if ((packet_ptr -> nx_packet_prepend_ptr+i) >= packet_ptr -> nx_packet_append_ptr)
            break;

        /* Check for maximum dump size.  */
        if (i >= NX_PPP_DEBUG_FRAME_SIZE)
            break;

        /* Print one character.  */
        printf("%02x ", packet_ptr -> nx_packet_prepend_ptr[i]);
    }

    printf("\n");
#endif /* NX_PPP_DEBUG_LOG_PRINT_ENABLE */

    /* Now load the internal PPP log.  */

    /* Setup the debug log entry pointer.  */
    entry_ptr =  &(ppp_ptr -> nx_ppp_debug_log[ppp_ptr -> nx_ppp_debug_log_oldest_index++]);

    /* Check for wrap-around of the index.  */
    if (ppp_ptr -> nx_ppp_debug_log_oldest_index >= NX_PPP_DEBUG_LOG_SIZE)
        ppp_ptr -> nx_ppp_debug_log_oldest_index =  0;

    /* Setup the debug log entries.  */
    entry_ptr -> nx_ppp_debug_entry_time_stamp =  time;
    entry_ptr -> nx_ppp_debug_ppp_state =         (UCHAR) ppp_ptr -> nx_ppp_state;
    entry_ptr -> nx_ppp_debug_lcp_state =         (UCHAR) ppp_ptr -> nx_ppp_lcp_state;
    entry_ptr -> nx_ppp_debug_pap_state =         (UCHAR) ppp_ptr -> nx_ppp_pap_state;
    entry_ptr -> nx_ppp_debug_chap_state =        (UCHAR) ppp_ptr -> nx_ppp_chap_state;
    entry_ptr -> nx_ppp_debug_ipcp_state =        (UCHAR) ppp_ptr -> nx_ppp_ipcp_state;
    entry_ptr -> nx_ppp_debug_authenticated =     ppp_ptr -> nx_ppp_authenticated;
    entry_ptr -> nx_ppp_debug_frame_type =        packet_type;
    entry_ptr -> nx_ppp_debug_packet_length =     packet_ptr -> nx_packet_length;

    /* Store at least part of the packet payload.  */
    for (i = 0; i < packet_ptr -> nx_packet_length; i++)
    {

        /* Check for packet spanning multiple packets.  */
        if ((packet_ptr -> nx_packet_prepend_ptr+i) >= packet_ptr -> nx_packet_append_ptr)
            break;

        /* Check for maximum dump size.  */
        if (i >= NX_PPP_DEBUG_FRAME_SIZE)
            break;

        /* Store one character.  */
        entry_ptr -> nx_ppp_debug_frame[i] =  packet_ptr -> nx_packet_prepend_ptr[i];
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_debug_log_capture_protocol                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function outputs the status of various protocols and timeouts  */
/*    of the specified PPP instance.                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP structure      */ 
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
/*    _nx_ppp_thread_entry                  PPP event processing          */ 
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
void  _nx_ppp_debug_log_capture_protocol(NX_PPP *ppp_ptr)
{


    /* Print out the PPP name.  */
    printf("%s: ", ppp_ptr -> nx_ppp_name);

    /* Print out the current PPP state.  */
    printf("State: %u, ", ppp_ptr -> nx_ppp_state);

    /* Print out the current PPP LCP state.  */
    printf("LCP State: %u, ", ppp_ptr -> nx_ppp_lcp_state);

    /* Print out the current IPCP state.  */
    printf("IPCP State: %u, ", ppp_ptr -> nx_ppp_ipcp_state);

    if ((ppp_ptr -> nx_ppp_chap_enabled) || (ppp_ptr -> nx_ppp_pap_enabled))
    {
    
         /* Print out the authentication flag.  */
         if (ppp_ptr -> nx_ppp_authenticated)
             printf("Authenticated, ");
         else
         {
         
             printf("Not Authenticated, ");

             /* Print out the current PPP PAP state.  */
             printf("PAP State: %u, ", ppp_ptr -> nx_ppp_pap_state);

             /* Print out the current PPP CHAP state.  */
             printf("CHAP State: %u, ", ppp_ptr -> nx_ppp_chap_state);
         }
    }

    /* Print out the current PPP LCP state.  */
    printf("Time remaining: %lu, ", ppp_ptr -> nx_ppp_timeout);
    printf("Timeouts: %lu, ", ppp_ptr -> nx_ppp_receive_timeouts);
    printf("Protocol retries: %lu, ", ppp_ptr -> nx_ppp_protocol_retry_counter);

    printf("\n");
}
#endif /* NX_PPP_DEBUG_LOG_ENABLE */


#ifndef NX_PPP_DISABLE_CHAP
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_hash_generator                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calculates the MD5 hash value for CHAP authentication */ 
/*    processing.  The input challenge "rand_value" is a stream of octets,*/
/*    including 0x0, so the length option ensures the rand_value length is*/
/*    set correctly.                                                      */
/*                                                                        */ 
/*    If the caller is certain there is no 0x0 within the rand_value it   */
/*    can send a zero length arguement and this function will compute the */
/*    rand_value length.                                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    hvalue                                Hash value return string      */ 
/*    id                                    PPP message ID                */ 
/*    secret                                Secret input string           */ 
/*    rand_value                            Random value input string     */ 
/*    length                                Length of random string,      */
/*                                           if zero, assume null         */
/*                                           terminated                   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_chap_challenge_respond        CHAP challenge response       */ 
/*    _nx_ppp_chap_challenge_validate       CHAP validate response        */ 
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
void _nx_ppp_hash_generator(unsigned char *hvalue,  unsigned char id, 
                            unsigned char *secret,  unsigned char *rand_value, UINT length)
{

UINT    slen, rlen;
NX_MD5  context;


    /* Compute the length of the secret. */
    for(slen = 0; secret[slen]; slen++);

    /* Compute the length of the rand_value if no length is specified. */
    if(length==0)
    {
        for(rlen = 0; rand_value[rlen]; rlen++);   
    }
    else
    {
        rlen = length;
    }  
 
    /* Initialize the MD5 digest calculation.  */
    _nx_md5_initialize(&context);
 
    /* Update the digest.  */
    _nx_md5_update(&context, &id, 1);
    _nx_md5_update(&context, secret, slen);
    _nx_md5_update(&context, rand_value, rlen);

    /* Finally, calculate the digest.  */
    _nx_md5_digest_calculate(&context, hvalue);
}
#endif /* NX_PPP_DISABLE_CHAP */


/* Define externally available API functions.  */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_byte_receive                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the PPP byte receive             */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    byte                                  Newly received byte           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_byte_receive                  Actual PPP byte receive       */ 
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
UINT  _nxe_ppp_byte_receive(NX_PPP *ppp_ptr, UCHAR byte)
{

UINT    status;


    /* Check for an invalid input pointer.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID))
        return(NX_PTR_ERROR);

    /* Call actual byte receive function.  */
    status =  _nx_ppp_byte_receive(ppp_ptr, byte);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_byte_receive                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function receives a byte from the application (usually an      */ 
/*    application ISR), buffers it, and notifies the PPP receive thread.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    byte                                  Newly received byte           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Notify PPP receiving thread   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code (Including ISRs)                                   */ 
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
UINT  _nx_ppp_byte_receive(NX_PPP *ppp_ptr, UCHAR byte)
{

TX_INTERRUPT_SAVE_AREA


    /* Disable interrupts.  */
    TX_DISABLE

    /* Check for no longer active PPP instance.  */
    if (ppp_ptr -> nx_ppp_id != NX_PPP_ID)
    {

        /* PPP is no longer active.  */

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return an error.  */
        return(NX_PTR_ERROR);
    }

    /* Check for a stopped PPP instance.  */
    if (ppp_ptr -> nx_ppp_state == NX_PPP_STOPPED)
    {

        /* Silently discard byte.  */
        
        /* Restore interrupts.  */
        TX_RESTORE

        /* Return success.  */
        return(NX_SUCCESS);
    }

    /* Determine if there is enough room in the serial buffer.  */
    if (ppp_ptr -> nx_ppp_serial_buffer_byte_count >= NX_PPP_SERIAL_BUFFER_SIZE)
    {

#ifndef NX_PPP_DISABLE_INFO
        /* Increment the number of bytes dropped.  */
        ppp_ptr -> nx_ppp_bytes_dropped++;
#endif

        /* Restore interrupts.  */
        TX_RESTORE

        /* No, return an error.  */
        return(NX_PPP_BUFFER_FULL);
    }

    /* Otherwise, PPP is active and there is room in the buffer!  */

    /* Place the byte in the buffer.  */
    ppp_ptr -> nx_ppp_serial_buffer[ppp_ptr -> nx_ppp_serial_buffer_write_index++] =  byte;

    /* Check for a wrap-around of the serial buffer.  */
    if (ppp_ptr -> nx_ppp_serial_buffer_write_index >= NX_PPP_SERIAL_BUFFER_SIZE)
    {

        /* Yes, buffer wrap-around is present. Reset the write pointer to the beginning of the
           buffer.  */
        ppp_ptr -> nx_ppp_serial_buffer_write_index =  0;
    }

    /* Increment the byte count.  */
    ppp_ptr -> nx_ppp_serial_buffer_byte_count++;

#ifndef NX_PPP_DISABLE_INFO
    /* Increment the number of bytes received.  */
    ppp_ptr -> nx_ppp_bytes_received++;
#endif

    /* Restore interrupts.  */
    TX_RESTORE

    /* Determine if the PPP receive thread needs to be alerted.  */
    if ((ppp_ptr -> nx_ppp_serial_buffer_byte_count >= NX_PPP_SERIAL_BUFFER_ALERT_THRESHOLD) ||
        (byte == 0x7e))
    {

        /* Yes, alert the receiving thread that a byte is available for processing.  */
        tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_PACKET_RECEIVE, TX_OR);
    }

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_create                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the PPP create instance          */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*    name                                  Pointer to instance name      */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    stack_memory_ptr                      Pointer to thread stack       */ 
/*    stack_size                            Pointer to thread stack size  */
/*    thread_priority                       Priority of thread(s) created */ 
/*                                            for PPP                     */ 
/*    pool_ptr                              Default packet pool pointer   */ 
/*    ppp_non_ppp_packet_handler            Non PPP packet handler        */ 
/*                                            provided by the application */ 
/*    ppp_byte_send                         Byte output function provided */ 
/*                                            by the application          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_create                        Actual PPP create function    */ 
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
UINT  _nxe_ppp_create(NX_PPP *ppp_ptr, CHAR *name, NX_IP *ip_ptr, 
               VOID *stack_memory_ptr, ULONG stack_size, UINT thread_priority, 
               NX_PACKET_POOL *pool_ptr,
               void (*ppp_non_ppp_packet_handler)(NX_PACKET *packet_ptr),
               void (*ppp_byte_send)(UCHAR byte))
{

UINT    status;


    /* Check for an invalid input pointer.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id == NX_PPP_ID) || 
        (ip_ptr == NX_NULL) || (stack_memory_ptr == NX_NULL) || (pool_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual PPP create function.  */
    status =  _nx_ppp_create(ppp_ptr, name, ip_ptr, stack_memory_ptr, stack_size, thread_priority, 
                                pool_ptr, ppp_non_ppp_packet_handler, ppp_byte_send);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_create                                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a PPP instance for the specified IP.          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*    name                                  Pointer to instance name      */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    stack_memory_ptr                      Pointer to thread stack       */ 
/*    stack_size                            Pointer to thread stack size  */
/*    thread_priority                       Priority of thread(s) created */ 
/*                                            for PPP                     */ 
/*    pool_ptr                              Default packet pool pointer   */ 
/*    ppp_non_ppp_packet_handler            Non PPP packet handler        */ 
/*                                            provided by the application */ 
/*    ppp_byte_send                         Byte output function provided */ 
/*                                            by the application          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_create                 Create PPP event flags group  */ 
/*    tx_thread_create                      Create PPP helper thread      */ 
/*    tx_time_get                           Get time for IDs              */ 
/*    tx_timer_create                       Create PPP timer              */ 
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
UINT  _nx_ppp_create(NX_PPP *ppp_ptr, CHAR *name, NX_IP *ip_ptr, 
               VOID *stack_memory_ptr, ULONG stack_size, UINT thread_priority, 
               NX_PACKET_POOL *pool_ptr,
               void (*ppp_non_ppp_packet_handler)(NX_PACKET *packet_ptr),
               void (*ppp_byte_send)(UCHAR byte))
{

TX_INTERRUPT_SAVE_AREA

NX_PPP      *tail_ptr;


    /* Check the supplied packet pool for minimum required payload length.  */
    if (pool_ptr -> nx_packet_pool_payload_size < NX_PPP_MIN_PACKET_PAYLOAD)
    {
        return(NX_PPP_BAD_PACKET);
    }

    /* Initialize the PPP control block to zero.  */
    memset((void *) ppp_ptr, 0, sizeof(NX_PPP));

    /* Save the PPP name.  */
    ppp_ptr -> nx_ppp_name =                    name;

    /* Save the IP pointer.  */
    ppp_ptr -> nx_ppp_ip_ptr =                  ip_ptr;

    /* Save the packet pool pointer.  */
    ppp_ptr -> nx_ppp_packet_pool_ptr =         pool_ptr;

    /* Save the byte output routine specified by the user.  */
    ppp_ptr -> nx_ppp_byte_send =               ppp_byte_send;
    ppp_ptr -> nx_ppp_non_ppp_packet_handler =  ppp_non_ppp_packet_handler;

    /* Setup the initial state.  */
    ppp_ptr -> nx_ppp_state =                   NX_PPP_STOPPED;

    /* Setup the Maximum Receive Unit (MRU).  */
    ppp_ptr -> nx_ppp_mru =                     NX_PPP_MRU;

    /* Setup receive and transmit IDs.  */
    ppp_ptr -> nx_ppp_transmit_id =             (UCHAR) tx_time_get();

    /* Default the authenticated field to true.  */
    ppp_ptr -> nx_ppp_authenticated =           NX_TRUE;

    /* Create event flag group to control the PPP processing thread.  */
    tx_event_flags_create(&(ppp_ptr -> nx_ppp_event), "PPP EVENTS") ;

    /* Create the PPP processing thread. Note that this thread does not run until the PPP driver is
       initialized during the IP create.  */
    tx_thread_create(&(ppp_ptr -> nx_ppp_thread), "PPP THREAD", _nx_ppp_thread_entry, (ULONG) ppp_ptr,  
            stack_memory_ptr, stack_size, thread_priority, thread_priority, NX_PPP_THREAD_TIME_SLICE, TX_DONT_START);

    /* Create the PPP timeout timer.  */
    tx_timer_create(&(ppp_ptr -> nx_ppp_timer), "PPP TIMER", _nx_ppp_timer_entry, (ULONG) ppp_ptr, NX_PPP_BASE_TIMEOUT, NX_PPP_BASE_TIMEOUT, TX_NO_ACTIVATE);

    /* Otherwise, the PPP initialization was successful.  Place the
       PPP control block on the list of created PPP instances.  */
    TX_DISABLE

    /* Load the PPP ID field in the PPP control block.  */
    ppp_ptr -> nx_ppp_id =  NX_PPP_ID;

    /* Place the new PPP control block on the list of created IPs.  First,
       check for an empty list.  */
    if (_nx_ppp_created_ptr)
    {

        /* Pickup tail pointer.  */
        tail_ptr =  _nx_ppp_created_ptr -> nx_ppp_created_previous;

        /* Place the new PPP control block in the list.  */
        _nx_ppp_created_ptr -> nx_ppp_created_previous =  ppp_ptr;
        tail_ptr -> nx_ppp_created_next =  ppp_ptr;

        /* Setup this PPP's created links.  */
        ppp_ptr -> nx_ppp_created_previous =  tail_ptr;
        ppp_ptr -> nx_ppp_created_next =      _nx_ppp_created_ptr; 
    }
    else
    {

        /* The created PPP list is empty.  Add PPP control block to empty list.  */
        _nx_ppp_created_ptr =                ppp_ptr;
        ppp_ptr -> nx_ppp_created_next =     ppp_ptr;
        ppp_ptr -> nx_ppp_created_previous = ppp_ptr;
    }

    /* Increment the created PPP counter.  */
    _nx_ppp_created_count++;

    /* Restore previous interrupt posture.  */
    TX_RESTORE

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_delete                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the PPP delete instance          */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_delete                        Actual PPP delete function    */ 
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
UINT  _nxe_ppp_delete(NX_PPP *ppp_ptr)
{

UINT    status;


    /* Check for an invalid input pointer.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual PPP delete function.  */
    status =  _nx_ppp_delete(ppp_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_delete                                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes a PPP instance for the specified IP.          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_delete                 Delete PPP event flags group  */ 
/*    tx_event_flags_set                    Set PPP event flag to stop PPP*/ 
/*    tx_thread_delete                      Create PPP helper threads     */ 
/*    tx_thread_identify                    Identify current thread       */ 
/*    tx_thread_sleep                       Sleep for a small time        */ 
/*    tx_timer_deactivate                   Timer deactivate              */ 
/*    tx_timer_delete                       Delete timer                  */ 
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
UINT  _nx_ppp_delete(NX_PPP *ppp_ptr)
{

TX_INTERRUPT_SAVE_AREA


    /* Determine if the caller is the PPP thread itself. This is not allowed since
       a thread cannot delete itself in ThreadX.  */
    if (&ppp_ptr -> nx_ppp_thread == tx_thread_identify())
    {

        /* Invalid caller of this routine, return an error!  */
        return(NX_CALLER_ERROR);
    }
    
    /* Set the event to close the PPP thread.  */
    tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_STOP, TX_OR);

    /* Now wait until the PPP thread goes to a closed state before proceeding with the delete. 
       This will give PPP the opportunity to properly release all packets being worked on.  */
    while (ppp_ptr -> nx_ppp_state != NX_PPP_STOPPED)
    {

        /* Sleep for a tick.  */
        tx_thread_sleep(1);
    }

    /* Disable interrupts.  */
    TX_DISABLE

    /* Clear the PPP ID to show that it is no longer valid.  */
    ppp_ptr -> nx_ppp_id =  0;

    /* Decrement the number of PPP instances created.  */
    _nx_ppp_created_count--;

    /* See if the PPP instance is the only one on the list.  */
    if (ppp_ptr == ppp_ptr -> nx_ppp_created_next)
    {

        /* Only created PPP instance, just set the created list to NULL.  */
        _nx_ppp_created_ptr =  TX_NULL;
    }
    else
    {

        /* Link-up the neighbors.  */
        (ppp_ptr -> nx_ppp_created_next) -> nx_ppp_created_previous =
                                            ppp_ptr -> nx_ppp_created_previous;
        (ppp_ptr -> nx_ppp_created_previous) -> nx_ppp_created_next =
                                            ppp_ptr -> nx_ppp_created_next;

        /* See if we have to update the created list head pointer.  */
        if (_nx_ppp_created_ptr == ppp_ptr)
            
            /* Yes, move the head pointer to the next link. */
            _nx_ppp_created_ptr =  ppp_ptr -> nx_ppp_created_next; 
    }

    /* Restore previous interrupt posture.  */
    TX_RESTORE

    /* Terminate the thread.  */
    tx_thread_terminate(&(ppp_ptr -> nx_ppp_thread));

    /* Delete the PPP thread.  */
    tx_thread_delete(&(ppp_ptr -> nx_ppp_thread));

    /* Deactivate the PPP timer.  */
    tx_timer_deactivate(&(ppp_ptr -> nx_ppp_timer));

    /* Delete the PPP timer.  */
    tx_timer_delete(&(ppp_ptr -> nx_ppp_timer));
    
    /* Delete the event flag group.  */
    tx_event_flags_delete(&(ppp_ptr -> nx_ppp_event));

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_link_up_notify                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function checks for errors in the PPP link up notify callback  */ 
/*    notify set function call.                                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*    ppp_link_up_callback                  Pointer to application        */ 
/*                                            callback function           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_link_up_notify                Actual PPP link up notify set */ 
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
UINT  _nxe_ppp_link_up_notify(NX_PPP *ppp_ptr, VOID (*ppp_link_up_callback)(NX_PPP *ppp_ptr))
{

UINT    status;

    /* Check for an invalid input pointer.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING
                                                
    /* Call actual PPP link up notify set function.  */
    status =  _nx_ppp_link_up_notify(ppp_ptr, ppp_link_up_callback);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_link_up_notify                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function registers an application callback for the link up     */ 
/*    event.                                                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*    ppp_link_up_callback                  Pointer to application        */ 
/*                                            callback function           */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ppp_link_up_notify(NX_PPP *ppp_ptr, VOID (*ppp_link_up_callback)(NX_PPP *ppp_ptr))
{

    /* Valid PPP pointer, now setup the notification callback.  Note that supplying
       a NULL for the callback will disable notification.  */
    ppp_ptr -> nx_ppp_link_up_callback =  ppp_link_up_callback;

    /* Return success.  */
    return(NX_SUCCESS);
}
 

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_link_down_notify                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function checks for errors in the PPP link down notify callback*/ 
/*    notify set function call.                                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*    ppp_link_down_callback                Pointer to application        */ 
/*                                            callback function           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_link_down_notify              Actual PPP link down notify   */ 
/*                                            set function                */ 
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
UINT  _nxe_ppp_link_down_notify(NX_PPP *ppp_ptr, VOID (*ppp_link_down_callback)(NX_PPP *ppp_ptr))
{

UINT    status;

    /* Check for an invalid input pointer.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING
                                                
    /* Call actual PPP link down notify set function.  */
    status =  _nx_ppp_link_down_notify(ppp_ptr, ppp_link_down_callback);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_link_down_notify                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function registers an application callback for the link down   */ 
/*    event.                                                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*    ppp_link_down_callback                Pointer to application        */ 
/*                                            callback function           */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ppp_link_down_notify(NX_PPP *ppp_ptr, VOID (*ppp_link_down_callback)(NX_PPP *ppp_ptr))
{

    /* Valid PPP pointer, now setup the notification callback.  Note that supplying
       a NULL for the callback will disable notification.  */
    ppp_ptr -> nx_ppp_link_down_callback =  ppp_link_down_callback;

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_pap_enable                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the PPP PAP enable               */ 
/*    function call.                                                      */
/*                                                                        */   
/*    Note: The first string lengths of name and password are limited by  */
/*    internal buffer size. The second string of name and password are    */
/*    NULL terminated.                                                    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*    generate_login                        Pointer to login function     */ 
/*    verify_login                          Pointer to verify function    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_pap_enable                    Actual PPP PAP enable function*/ 
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
UINT  _nxe_ppp_pap_enable(NX_PPP *ppp_ptr, UINT (*generate_login)(CHAR *name, CHAR *password),
                        UINT (*verify_login)(CHAR *name, CHAR *password))
{

UINT    status;


    /* Check for an invalid input pointer... including not being in the closed state for this configuration API.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID) || (ppp_ptr -> nx_ppp_state != NX_PPP_STOPPED) ||
        ((generate_login == NX_NULL) && (verify_login == NX_NULL)))
        return(NX_PTR_ERROR);
        
    /* Check for valid caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual PPP PAP enable function.  */
    status =  _nx_ppp_pap_enable(ppp_ptr, generate_login, verify_login);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_pap_enable                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function enables PAP for the specified PPP instance.           */
/*                                                                        */   
/*    Note: The first string lengths of name and password are limited by  */
/*    internal buffer size. The second string of name and password are    */
/*    NULL terminated.                                                    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    generate_login                        Pointer to application        */ 
/*                                            function that generates     */ 
/*                                            name and password for login */ 
/*    verify_login                          Pointer to application        */ 
/*                                            function that verifies the  */ 
/*                                            supplied name and password  */ 
/*                                            are valid                   */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ppp_pap_enable(NX_PPP *ppp_ptr, UINT (*generate_login)(CHAR *name, CHAR *password),
                         UINT (*verify_login)(CHAR *name, CHAR *password))
{

#ifdef NX_PPP_DISABLE_PAP

    /* Return the non implemented error.  */
    return(NX_NOT_IMPLEMENTED);

#else

    /* Setup the PAP information.  */   
    if (generate_login)
    {
    
        /* Setup login generation information.  */
        ppp_ptr -> nx_ppp_pap_generate_login =  generate_login;        

        /* The authenticated flag is not cleared for the generate login, since the
           peer may choose not to require PAP. */
    }
    
    /* Determine if the generate login is provided.  */
    if (verify_login)
    {
        /* Setup login verification information.  */
        ppp_ptr -> nx_ppp_pap_verify_login =                  verify_login;
        ppp_ptr -> nx_ppp_verify_authentication_protocol =    NX_PPP_PAP_PROTOCOL;

        /* Authentication is needed, clear the flag.  */
        ppp_ptr -> nx_ppp_authenticated =  NX_FALSE;        
    }

    /* Show that the PAP is enabled.  */
    ppp_ptr -> nx_ppp_pap_enabled =  NX_TRUE;

    /* Set the initial PAP state.  */
    ppp_ptr -> nx_ppp_pap_state =  NX_PPP_PAP_INITIAL_STATE;

    /* Return success.  */
    return(NX_SUCCESS);
#endif
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_chap_challenge                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the PPP CHAP challenge           */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_chap_challenge                Actual PPP CHAP challenge     */ 
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
UINT  _nxe_ppp_chap_challenge(NX_PPP *ppp_ptr)
{

UINT    status;


    /* Check for an invalid input pointer.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual PPP CHAP challenge function.  */
    status =  _nx_ppp_chap_challenge(ppp_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_chap_challenge                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function generates a CHAP challenge for the specified PPP      */ 
/*    instance.                                                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Notify PPP receiving thread   */ 
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
UINT  _nx_ppp_chap_challenge(NX_PPP *ppp_ptr)
{

#ifdef NX_PPP_DISABLE_CHAP

    /* Return the non implemented error.  */
    return(NX_NOT_IMPLEMENTED);

#else

    /* Determine if the CHAP of the PPP instance is enabled and able to challenge.  */
    if ((ppp_ptr -> nx_ppp_chap_enabled) &&
        (ppp_ptr -> nx_ppp_chap_get_challenge_values) &&
        (ppp_ptr -> nx_ppp_chap_get_verification_values))
    {

        /* Check for the appropriate CHAP state. If the initial CHAP has not yet 
           completed, simply discard this request.  */
        if (ppp_ptr -> nx_ppp_chap_state == NX_PPP_CHAP_COMPLETED_STATE)
        {
                
            /* Yes, the CHAP protocol is in an open state so another challenge is legal.  */
        
            /* Initiate CHAP challenge by setting the appropriate event flag to wakeup the PPP thread.  */
            tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_CHAP_CHALLENGE, TX_OR);
        }
        
        /* Return successful completion.  */
        return(NX_SUCCESS);
    }
    else
    {

        /* CHAP is either not enabled or is setup only for CHAP response - not a challenge. Return an error!  */
        return(NX_PPP_FAILURE);
    }
#endif
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_chap_enable                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the PPP CHAP enable              */ 
/*    function call.                                                      */
/*                                                                        */   
/*    Note: The string lengths of rand_value, name, system and secret are */
/*    limited by internal buffer size.                                    */
/*                                                                        */  
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*    get_challenge_values                  Pointer to application        */ 
/*                                            function that retrieves     */ 
/*                                            values required to make     */ 
/*                                            a challenge                 */ 
/*    get_responder_values                  Pointer to application        */ 
/*                                            function that retrieves     */ 
/*                                            values required to respond  */ 
/*                                            to a challenge              */ 
/*    get_verification_values               Pointer to application        */ 
/*                                            function that retrieves     */ 
/*                                            values required to verify   */ 
/*                                            a challenge                 */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_chap_enable                   Actual PPP CHAP enable        */ 
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
UINT  _nxe_ppp_chap_enable(NX_PPP *ppp_ptr, 
                           UINT (*get_challenge_values)(CHAR *rand_value, CHAR *id, CHAR *name),
                           UINT (*get_responder_values)(CHAR *system, CHAR *name, CHAR *secret),
                           UINT (*get_verification_values)(CHAR *system, CHAR *name, CHAR *secret))
{

UINT    status;


    /* Check for an invalid input pointer... including not being in the closed state for this configuration API.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID) || (ppp_ptr -> nx_ppp_state != NX_PPP_STOPPED) ||
        ((get_challenge_values == NX_NULL) && (get_responder_values == NX_NULL) && (get_verification_values == NX_NULL)))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual PPP CHAP enable function.  */
    status =  _nx_ppp_chap_enable(ppp_ptr, get_challenge_values, get_responder_values, get_verification_values);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_chap_enable                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function enables CHAP for the specified PPP instance.          */ 
/*                                                                        */   
/*    Note: The string lengths of rand_value, name, system and secret are */
/*    limited by internal buffer size.                                    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    get_challenge_values                  Pointer to application        */ 
/*                                            function that retrieves     */ 
/*                                            values required to make     */ 
/*                                            a challenge                 */ 
/*    get_responder_values                  Pointer to application        */ 
/*                                            function that retrieves     */ 
/*                                            values required to respond  */ 
/*                                            to a challenge              */ 
/*    get_verification_values               Pointer to application        */ 
/*                                            function that retrieves     */ 
/*                                            values required to verify   */ 
/*                                            a challenge response        */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ppp_chap_enable(NX_PPP *ppp_ptr, 
                          UINT (*get_challenge_values)(CHAR *rand_value, CHAR *id, CHAR *name),
                          UINT (*get_responder_values)(CHAR *system, CHAR *name, CHAR *secret),
                          UINT (*get_verification_values)(CHAR *system, CHAR *name, CHAR *secret))
{

#ifdef NX_PPP_DISABLE_CHAP

    /* Return the non implemented error.  */
    return(NX_NOT_IMPLEMENTED);

#else

    /* Setup CHAP information.  */
    if (get_responder_values)
    {
    
        /* Setup challenge response callback.  */
        ppp_ptr -> nx_ppp_chap_get_responder_values =     get_responder_values;

        /* The authenticated flag is not cleared for the generate login, since the
           peer may choose not to require CHAP. */
    }

    /* Determine if we are going to challenge the peer.  */
    if ((get_challenge_values) && (get_verification_values))
    {

        /* Setup the CHAP information.  */
        ppp_ptr -> nx_ppp_chap_get_challenge_values =     get_challenge_values;
        ppp_ptr -> nx_ppp_chap_get_verification_values =  get_verification_values;

        /* Yes, we must generate a challenge.   */
        ppp_ptr -> nx_ppp_verify_authentication_protocol =  NX_PPP_CHAP_PROTOCOL;
        ppp_ptr -> nx_ppp_authenticated =                   NX_FALSE;                
    }

    /* Show that the CHAP is enabled.  */
    ppp_ptr -> nx_ppp_chap_enabled =  NX_TRUE;

    /* Set the initial CHAP state.  */
    ppp_ptr -> nx_ppp_chap_state =  NX_PPP_CHAP_INITIAL_STATE;

    /* Return success.  */
    return(NX_SUCCESS);
#endif
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_dns_address_get                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function checks for errors in the DNS address of the PPP       */ 
/*    instance get function call.                                         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    dns_address_ptr                       Destination for DNS address   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_dns_address_get               Actual PPP DNS address get    */ 
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
UINT  _nxe_ppp_dns_address_get(NX_PPP *ppp_ptr, ULONG *dns_address_ptr)
{

UINT    status;

    /* Check for an invalid input pointer...  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID) ||
        (dns_address_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                                                
    /* Call actual PPP DNS address get function.  */
    status =  _nx_ppp_dns_address_get(ppp_ptr, dns_address_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_dns_address_get                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the DNS address of the PPP instance.        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    dns_address_ptr                       Destination for DNS address   */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ppp_dns_address_get(NX_PPP *ppp_ptr, ULONG *dns_address_ptr)
{

    /* Determine if the PPP instance is in an established state.  */
    if (ppp_ptr -> nx_ppp_state == NX_PPP_ESTABLISHED)
    {

        if (ppp_ptr -> nx_ppp_primary_dns_address == 0x0)
        {
            return NX_PPP_ADDRESS_NOT_ESTABLISHED;
        }

        /* Return the DNS address ptr.  */
        *dns_address_ptr =  ppp_ptr -> nx_ppp_primary_dns_address;
            
        /* Return success.  */
        return(NX_SUCCESS);
    }
    else
    {
    
        /* Set the DNS address to 0.  */
        *dns_address_ptr =  0;
        
        /* The PPP connection has not yet been established.  */
        return(NX_PPP_NOT_ESTABLISHED);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_dns_address_set                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function checks for errors in the DNS address of the PPP       */ 
/*    instance set function call.                                         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    dns_address                           Primary DNS address           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_dns_address_set               Actual PPP DNS address set    */ 
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
UINT  _nxe_ppp_dns_address_set(NX_PPP *ppp_ptr, ULONG dns_address)
{

UINT    status;

    /* Check for an invalid input pointer...  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID))
        return(NX_PTR_ERROR);
         
    /* Check for invalid address. */
    if (dns_address == 0x0)
        return(NX_PPP_INVALID_PARAMETER);

    /* Check for valid caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual PPP DNS address set function.  */
    status =  _nx_ppp_dns_address_set(ppp_ptr, dns_address);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_dns_address_set                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the primary DNS address for the PPP device to    */
/*   supply if it receives a DNS option request (129) in the configure    */ 
/*   request NAKed list.                                                  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    dns_address                           Primary DNS address           */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ppp_dns_address_set(NX_PPP *ppp_ptr, ULONG dns_address)
{

    /* Set the primary DNS address.  */
    ppp_ptr -> nx_ppp_primary_dns_address = dns_address;

    /* Return success.  */
    return NX_SUCCESS;
}
 

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_secondary_dns_address_get                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function checks for errors in the secondary DNS address of the */ 
/*    PPP instance get function call.                                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    secondary_dns_address_ptr             Secondary DNS address         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_ppp_secondary_dns_address_get     Actual PPP secondary DNS      */ 
/*                                            address get function        */
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
UINT  _nxe_ppp_secondary_dns_address_get(NX_PPP *ppp_ptr, ULONG *secondary_dns_address_ptr)
{

UINT    status;

    /* Check for an invalid input pointer...  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID) ||
        (secondary_dns_address_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                                                
    /* Call actual PPP secondary DNS address get function.  */
    status =  _nx_ppp_secondary_dns_address_get(ppp_ptr, secondary_dns_address_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_secondary_dns_address_get                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the secondary DNS address of the PPP        */ 
/*    instance.                                                           */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    secondary_dns_address_ptr             Secondary DNS address         */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ppp_secondary_dns_address_get(NX_PPP *ppp_ptr, ULONG *secondary_dns_address_ptr)
{

    /* Determine if the PPP instance is in an established state.  */
    if (ppp_ptr -> nx_ppp_state == NX_PPP_ESTABLISHED)
    {

        if (ppp_ptr -> nx_ppp_secondary_dns_address == 0x0)
        {
            return(NX_PPP_ADDRESS_NOT_ESTABLISHED);
        }
    
        /* Return the DNS address ptr.  */
        *secondary_dns_address_ptr =  ppp_ptr -> nx_ppp_secondary_dns_address;
            
        /* Return success.  */
        return(NX_SUCCESS);
    }
    else
    {
    
        /* Set the DNS address to 0.  */
        *secondary_dns_address_ptr =  0;
        
        /* The PPP connection has not yet been established.  */
        return(NX_PPP_NOT_ESTABLISHED);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_secondary_dns_address_set                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function checks for errors in the secondary DNS address of the */ 
/*    PPP instance set function call.                                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    secondary_dns_address_ptr             Secondary DNS address         */  
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_ppp_secondary_dns_address_set     Actual PPP secondary DNS      */ 
/*                                            address set function        */ 
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
UINT  _nxe_ppp_secondary_dns_address_set(NX_PPP *ppp_ptr, ULONG secondary_dns_address)
{

UINT    status;

    /* Check for an invalid input pointer...  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID))
        return(NX_PTR_ERROR);
         
    /* Check for invalid address. */
    if (secondary_dns_address == 0x0)
        return(NX_PPP_INVALID_PARAMETER);

    /* Check for valid caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual PPP secondary DNS address set function.  */
    status =  _nx_ppp_secondary_dns_address_set(ppp_ptr, secondary_dns_address);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_secondary_dns_address_set                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the secondary DNS address for the PPP device to  */
/*   supply if it receives a DNS option request (131) in the configure    */ 
/*   request NAKed list.                                                  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    secondary_dns_address_ptr             Secondary DNS address         */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ppp_secondary_dns_address_set(NX_PPP *ppp_ptr, ULONG secondary_dns_address)
{

    /* Set the secondary DNS address.  */
    ppp_ptr -> nx_ppp_secondary_dns_address = secondary_dns_address;
        
    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_interface_index_get                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the PPP interface index get      */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    index_ptr                             Index of associated interface */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nxe_ppp_interface_index_get         Actual PPP interface index get */ 
/*                                           function                     */ 
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
UINT  _nxe_ppp_interface_index_get(NX_PPP *ppp_ptr, UINT *index_ptr)
{

UINT    status;

    /* Check for an invalid input pointer.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                                                
    /* Call actual PPP interface idnex get function.  */
    status =  _nx_ppp_interface_index_get(ppp_ptr, index_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_interface_index_get                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the interface index with a particular PPP   */ 
/*    instance.                                                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    index_ptr                             Index of associated interface */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ppp_interface_index_get(NX_PPP *ppp_ptr, UINT *index_ptr)
{

    /* Return the interface index.  */
    *index_ptr =  ppp_ptr -> nx_ppp_interface_index;
    
    /* Determine if the interface is actually setup.  */
    if (ppp_ptr -> nx_ppp_interface_ptr)
    {
        
        /* Return success.  */
        return(NX_SUCCESS);
    }
    else
    {
        /* Return an error. The IP thread has not executed yet which means the
           interface has not be setup.  */
        return(NX_IN_PROGRESS);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_ip_address_assign                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the PPP IP address assign        */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*    local_ip_address                      Local IP address              */ 
/*    peer_ip_address                       Peer IP address               */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_ip_address_assign             Actual PPP IP address assign  */ 
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
UINT  _nxe_ppp_ip_address_assign(NX_PPP *ppp_ptr, ULONG local_ip_address, ULONG peer_ip_address)
{

UINT    status;


    /* Check for an invalid input pointer... including not being in the closed state for this configuration API.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID) || (ppp_ptr -> nx_ppp_state != NX_PPP_STOPPED))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING

    /* Call actual PPP IP address assign function.  */
    status =  _nx_ppp_ip_address_assign(ppp_ptr, local_ip_address, peer_ip_address);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_ip_address_assign                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function assigns a local and peer IP address for the           */ 
/*    specified PPP instance.                                             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    local_ip_address                      Local IP address              */ 
/*    peer_ip_address                       Peer IP address               */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ppp_ip_address_assign(NX_PPP *ppp_ptr, ULONG local_ip_address, ULONG peer_ip_address)
{

UINT    i;


    /* Check if set the local IP address.  */
    if (local_ip_address != 0)
    {

        /* If true, this PPP instance is a Server. Update the flag.   */
        ppp_ptr -> nx_ppp_server = NX_TRUE;
    }

    /* Load the IP addresses into the PPP IPCP arrays.  */
    i =  4;
    do
    {

        /* Load the IP values into the array.  */
        ppp_ptr -> nx_ppp_ipcp_local_ip[i-1] =  (UCHAR) (local_ip_address & 0xFF);
        ppp_ptr -> nx_ppp_ipcp_peer_ip[i-1] = (UCHAR) (peer_ip_address & 0xFF);

        /* Shift the IP address down.  */
        local_ip_address =  local_ip_address >> 8;
        peer_ip_address = peer_ip_address >> 8;

        /* Decrement the index.  */
        i--;

    } while (i);

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_nak_authentication_notify                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function checks for errors in the PP NAK authentication notify */ 
/*    set function call.                                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    nak_authentication_notify             Authentication NAK callback   */ 
/*                                            function                    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_ppp_nak_authentication_notify     Actual PPP NAK authentication */ 
/*                                            notify set function         */  
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
UINT  _nxe_ppp_nak_authentication_notify(NX_PPP *ppp_ptr, void (*nak_authentication_notify)(void))
{

UINT    status;

    /* Check for an invalid input pointer... including not being in the closed state for this configuration API.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID) || (ppp_ptr -> nx_ppp_state != NX_PPP_STOPPED))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_INIT_AND_THREADS_CALLER_CHECKING
                                                
    /* Call actual PPP NAK authentication notify set function.  */
    status =  _nx_ppp_nak_authentication_notify(ppp_ptr, nak_authentication_notify);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_nak_authentication_notify                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function assigns an application callback function to be        */ 
/*    called if an authentication NAK is received from the peer.          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    nak_authentication_notify             Authentication NAK callback   */ 
/*                                            function                    */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ppp_nak_authentication_notify(NX_PPP *ppp_ptr, void (*nak_authentication_notify)(void))
{

    /* Store the callback function pointer in the PPP structure.  */
    ppp_ptr -> nx_ppp_nak_authentication_notify =  nak_authentication_notify;

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_raw_string_send                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the PPP raw string send          */ 
/*    function call.                                                      */
/*                                                                        */   
/*    Note: The string length of string_ptr is limited by the packet      */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*    string_ptr                            Pointer to ASCII string       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_raw_string_send               Actual PPP raw string send    */ 
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
UINT  _nxe_ppp_raw_string_send(NX_PPP *ppp_ptr, CHAR *string_ptr)
{

UINT    status;


    /* Check for an invalid input pointer.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID) || (string_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual PPP raw string send function.  */
    status =  _nx_ppp_raw_string_send(ppp_ptr, string_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_raw_string_send                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends a raw ASCII string to through PPP without any   */ 
/*    PPP framing. This is useful to respond to modem commands in some    */ 
/*    applications.                                                       */
/*                                                                        */   
/*    Note: The string length of string_ptr is limited by the packet      */
/*    payload size.                                                       */
/*                                                                        */  
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    string_ptr                            Pointer to ASCII string       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a packet             */ 
/*    tx_event_flags_set                    Alert PPP thread to process   */ 
/*                                            raw packet                  */ 
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
UINT  _nx_ppp_raw_string_send(NX_PPP *ppp_ptr, CHAR *string_ptr)
{

TX_INTERRUPT_SAVE_AREA

UINT        i, j;
UINT        status;
NX_PACKET   *packet_ptr;


    /* Initialize the string index.  */
    i =  0;
    
    /* Loop to process the raw string send request.  */
    while (string_ptr[i])
    {

        /* Allocate a packet to defer the raw string send to the PPP thread.  */
        status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, NX_PPP_TIMEOUT);

        /* Determine if there was an error allocating a packet.  */
        if (status != NX_SUCCESS)
        {
        
#ifndef NX_PPP_DISABLE_INFO

            /* Increment the number of packet allocation timeouts.  */ 
            ppp_ptr -> nx_ppp_packet_allocate_timeouts++;
#endif

            /* Just return an error.  */
            return(NX_NO_PACKET);          
        }

        /* Initialize the packet index.  */
        j =  0;
        
        /* Loop through the string copying the characters into the packet.  */
        do
        {

            /* Copy character into the packet.  */
            packet_ptr -> nx_packet_prepend_ptr[j++] =  (UCHAR)(string_ptr[i++]);
            
        } while ((string_ptr[i]) && (&(packet_ptr -> nx_packet_prepend_ptr[j]) < packet_ptr -> nx_packet_data_end));

        /* Update the append pointer.  */
        packet_ptr -> nx_packet_append_ptr =  &(packet_ptr -> nx_packet_prepend_ptr[j]);

        /* Setup the packet length.  */
        packet_ptr -> nx_packet_length =  j;

        /* Now place the packet on the raw packet queue.  */

        /* Disable interrupts.   */
        TX_DISABLE
        
        /* Determine if the raw transmit queue is empty.  */
        if (ppp_ptr -> nx_ppp_raw_packet_queue_count++)
        {
        
            /* Not empty, simply link the new packet to the tail and update the tail.  */
            (ppp_ptr -> nx_ppp_raw_packet_queue_tail) -> nx_packet_queue_next =  packet_ptr;
            ppp_ptr -> nx_ppp_raw_packet_queue_tail =                            packet_ptr;
        }
        else
        {
        
            /* List is empty, set the head and tail to this packet.  */
            ppp_ptr -> nx_ppp_raw_packet_queue_head =  packet_ptr;
            ppp_ptr -> nx_ppp_raw_packet_queue_tail =  packet_ptr;
        }
        
        /* Restore interrupts.  */
        TX_RESTORE
        
        /* Set event flag to wake up PPP thread for processing.  */
        tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_RAW_STRING_SEND, TX_OR);
    }

    /* Return success.  */
    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_start                                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function checks for errors in the PPP start function call.     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_ppp_start                         Actual PPP start function     */ 
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
UINT  _nxe_ppp_start(NX_PPP *ppp_ptr)
{

UINT    status;

    /* Check for an invalid input pointer.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                                                
    /* Call actual PPP start function.  */
    status =  _nx_ppp_start(ppp_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_start                                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function starts the PPP instance.                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Set PPP event flag to         */ 
/*                                            start PPP                   */ 
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
UINT  _nx_ppp_start(NX_PPP *ppp_ptr)
{

    /* Determine the current state.  */
    if (ppp_ptr -> nx_ppp_state != NX_PPP_STOPPED)
    {

        /* Already started.  */
        return(NX_PPP_ALREADY_STARTED);
    }
    else
    {

        /* Now restart the PPP instance.  */
        tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_START, TX_OR);
    }

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_stop                                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function checks for errors in the PPP stop function call.      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_ppp_stop                          Actual PPP stop function      */ 
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
UINT  _nxe_ppp_stop(NX_PPP *ppp_ptr)
{

UINT    status;

    /* Check for an invalid input pointer.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual PPP stop function.  */
    status =  _nx_ppp_stop(ppp_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_stop                                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function stops the PPP instance.                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Set PPP event flag to stop PPP*/ 
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
UINT  _nx_ppp_stop(NX_PPP *ppp_ptr)
{


    /* Determine the current state.  */
    if (ppp_ptr -> nx_ppp_state == NX_PPP_STOPPED)
    {

        /* Already stopped.  */
        return(NX_PPP_ALREADY_STOPPED);
    }
    else
    {

        /* Check if LCP connection is already established or in negotiation.  */
        if ((ppp_ptr -> nx_ppp_lcp_state == NX_PPP_LCP_COMPLETED_STATE) ||
            (ppp_ptr -> nx_ppp_lcp_state == NX_PPP_LCP_CONFIGURE_REQUEST_SENT_STATE) ||
            (ppp_ptr -> nx_ppp_lcp_state == NX_PPP_LCP_CONFIGURE_REQUEST_ACKED_STATE) ||
            (ppp_ptr -> nx_ppp_lcp_state == NX_PPP_LCP_PEER_CONFIGURE_REQUEST_ACKED_STATE))
        {

            /* Setup the retry counter.  */
            ppp_ptr -> nx_ppp_protocol_retry_counter = 0;

            /* Setup the timeout.  */
            ppp_ptr -> nx_ppp_timeout = NX_PPP_PROTOCOL_TIMEOUT;

            /* Move into the stopping to wait for terminate ack.  */
            ppp_ptr -> nx_ppp_lcp_state =  NX_PPP_LCP_STOPPING_STATE;

            /* Send terminate request.  */
            _nx_ppp_lcp_terminate_request_send(ppp_ptr);
        }
        else
        {

            /* Set the event to stop the PPP instance.  */
            tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_STOP, TX_OR);

            /* Determine if the application has registered a link down notification
               callback.  */
            if (ppp_ptr -> nx_ppp_link_down_callback)
            {

                /* Yes, call the application's callback function.  */
                (ppp_ptr -> nx_ppp_link_down_callback)(ppp_ptr);
            }
        }
    }

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_restart                                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function checks for errors in the PPP restart function call.   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_ppp_restart                       Actual PPP restart function   */ 
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
UINT  _nxe_ppp_restart(NX_PPP *ppp_ptr)
{

UINT    status;

    /* Check for an invalid input pointer.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                                                
    /* Call actual PPP restart function.  */
    status =  _nx_ppp_restart(ppp_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_restart                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function restarts the PPP instance. This is typically done     */ 
/*    after a link down situation.                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to instance           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Set PPP event flag to stop PPP*/ 
/*    tx_thread_sleep                       Sleep for a small time        */ 
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
UINT  _nx_ppp_restart(NX_PPP *ppp_ptr)
{

    /* Set the event to stop the PPP instance.  */
    tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_STOP, TX_OR);

    /* Now restart the PPP instance.  */
    tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_START, TX_OR);

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_status_get                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */  
/*    This function checks for errors in the PPP status get function call.*/ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    status_ptr                            Pointer to destination for    */ 
/*                                            PPP status                  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_ppp_status_get                    Actual PPP status get function*/  
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
UINT  _nxe_ppp_status_get(NX_PPP *ppp_ptr, UINT *status_ptr)
{

UINT    status;

    /* Check for an invalid input pointer.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID) || (status_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for valid caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                                                
    /* Call actual PPP status get function.  */
    status =  _nx_ppp_status_get(ppp_ptr, status_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_status_get                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the current status of the PPP instance.     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    status_ptr                            Pointer to destination for    */ 
/*                                            PPP status                  */ 
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_ppp_status_get(NX_PPP *ppp_ptr, UINT *status_ptr)
{

    /* Now determine the current state.  */
    if (ppp_ptr -> nx_ppp_state == NX_PPP_ESTABLISHED)
    {
   
        /* PPP is up, return established state.  */
        *status_ptr =  NX_PPP_STATUS_ESTABLISHED;

        /* Return success!  */
        return(NX_SUCCESS);
    }
    else if ((ppp_ptr -> nx_ppp_lcp_state > NX_PPP_LCP_INITIAL_STATE) && (ppp_ptr -> nx_ppp_lcp_state < NX_PPP_LCP_COMPLETED_STATE))
    {

        /* Is LCP in a failed state?  */
        if (ppp_ptr -> nx_ppp_lcp_state == NX_PPP_LCP_FAILED_STATE)
        {
        
            /* Return LCP failed state.  */
            *status_ptr =  NX_PPP_STATUS_LCP_FAILED;

            /* Return success.  */
            return(NX_SUCCESS);            
        }
        else
        {
        
            /* Return LCP in-progress state.  */
            *status_ptr =  NX_PPP_STATUS_LCP_IN_PROGRESS;

            /* Return success.  */            
            return(NX_SUCCESS);            
        }
    }
    else if ((ppp_ptr -> nx_ppp_pap_state > NX_PPP_PAP_INITIAL_STATE) && (ppp_ptr -> nx_ppp_pap_state < NX_PPP_PAP_COMPLETED_STATE))
    {
    
        /* Is PAP in a failed state?  */
        if (ppp_ptr -> nx_ppp_pap_state == NX_PPP_PAP_FAILED_STATE)
        {
        
            /* Return PAP failed state.  */
            *status_ptr =  NX_PPP_STATUS_PAP_FAILED;

            /* Return success.  */
            return(NX_SUCCESS);            
        }
        else
        {
        
            /* Return PAP in-progress state.  */
            *status_ptr =  NX_PPP_STATUS_PAP_IN_PROGRESS;

            /* Return success.  */            
            return(NX_SUCCESS);            
        }
    }
    else if ((ppp_ptr -> nx_ppp_chap_state > NX_PPP_CHAP_INITIAL_STATE) && (ppp_ptr -> nx_ppp_chap_state < NX_PPP_CHAP_COMPLETED_STATE))
    {
    
        /* Is CHAP in a failed state?  */
        if (ppp_ptr -> nx_ppp_chap_state == NX_PPP_CHAP_CHALLENGE_FAILED_STATE)
        {
        
            /* Return CHAP failed state.  */
            *status_ptr =  NX_PPP_STATUS_CHAP_FAILED;
            
            /* Return success.  */
            return(NX_SUCCESS);            
        }
        else
        {
        
            /* Return CHAP in-progress state.  */
            *status_ptr =  NX_PPP_STATUS_CHAP_IN_PROGRESS;
            
            /* Return success.  */
            return(NX_SUCCESS);            
        }
    }
    else 
    {
    
        /* Is IPCP in a failed state?  */
        if (ppp_ptr -> nx_ppp_ipcp_state == NX_PPP_IPCP_FAILED_STATE)
        {
        
            /* Return IPCP failed state.  */
            *status_ptr =  NX_PPP_STATUS_IPCP_FAILED;

            /* Return success.  */
            return(NX_SUCCESS);            
        }
        else
        {
        
            /* Return IPCP in-progress state.  */
            *status_ptr =  NX_PPP_STATUS_IPCP_IN_PROGRESS;
            
            /* Return success.  */
            return(NX_SUCCESS);            
        }
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_ping_reply                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/* This function is called when the PPP instance detects an LCP echo      */
/* request is received, and retransmits a response.                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    packet_ptr                            Pointer to echo request packet*/ 
/*                                             received                   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                  Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_packet_transmit               Transmit the PPP data         */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_state_machine_update      Processes events and updates  */
/*                                            the PPP in the LCP state    */
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
void  _nx_ppp_lcp_ping_reply(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr) 
{


    /* Change the coding to indicate an Echo reply. */
    packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_LCP_ECHO_REPLY;

    /* Set the Magic number as zero.  */
    packet_ptr -> nx_packet_prepend_ptr[6] = 0;
    packet_ptr -> nx_packet_prepend_ptr[7] = 0;
    packet_ptr -> nx_packet_prepend_ptr[8] = 0;
    packet_ptr -> nx_packet_prepend_ptr[9] = 0;

#ifndef NX_PPP_DISABLE_INFO

    ppp_ptr -> nx_ppp_lcp_echo_replies_sent++;


    /* Increment the number of LCP frames sent.  */
    ppp_ptr -> nx_ppp_lcp_frames_sent++;
#endif

    /* Send the reply out.  */
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_ping_process_echo_reply                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*  This function is called when the PPP instance detects an LCP echo     */
/*  reply is received.  It checks if the reply has a matching ID with the */
/*  previous echo request sent out and if so clears the                   */
/*  nx_ppp_lcp_echo_reply_id to indicate the device received a valid      */
/*  reply.                                                                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    packet_ptr                            Pointer to echo request packet*/ 
/*                                             received                   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                  Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_ppp_lcp_state_machine_update      Processes events and updates  */
/*                                            the PPP in the LCP state    */
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
void   _nx_ppp_lcp_ping_process_echo_reply(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr)
{

    /* Verify the PPP ID matches. */
    if (packet_ptr -> nx_packet_prepend_ptr[3] == ppp_ptr -> nx_ppp_lcp_echo_reply_id)
    {

        /* Successful ping received. */
#ifndef NX_PPP_DISABLE_INFO
        /* Increment the number of LCP echo replies received.  */
        ppp_ptr -> nx_ppp_lcp_echo_replies_received++;
#endif

        /* Clear the echo ID to indicate we received an LCP echo reply 
           matching the one we just sent out. */
        ppp_ptr -> nx_ppp_lcp_echo_reply_id = 0;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_ping_request                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the nx_ppp_ping_request   */
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    data_ptr                              Pointer to data in LCP echo   */ 
/*    data_size                             Size of data in LCP echo      */ 
/*    wait_option                           Wait option to transmit echo  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */ 
/*    NX_PPP_INVALID_PARAMETER              Invalid non pointer input     */
/*    NX_SUCCESS                            Successful echo request sent  */
/*    status                                Actual completion status      */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_ping_request                  Actual LCP echo service       */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nxe_ppp_ping_request(NX_PPP *ppp_ptr, CHAR *data_ptr, ULONG data_size, ULONG wait_option)
{

UINT status;

    if ((ppp_ptr == NX_NULL) || (data_ptr == NX_NULL))
    {
        return NX_PTR_ERROR;
    }

    if (data_size == 0)
    {
        return NX_PPP_INVALID_PARAMETER;
    }

    status = _nx_ppp_ping_request(ppp_ptr, data_ptr, data_size, wait_option);

    return status;

}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_ping_request                                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function builds and sends an LCP echo request, and sets the    */
/*    echo ID to the PPP nx_ppp_transmit_id. The caller waits for the echo*/
/*    ID (nx_ppp_lcp_echo_reply_id) to be reset to zero to indicate a     */
/*    matching echo reply was received.                                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               PPP instance pointer          */ 
/*    data_ptr                              Pointer to data in LCP echo   */ 
/*    data_size                             Size of data in LCP echo      */ 
/*    wait_option                           Wait option to transmit echo  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Echo request successfully sent*/ 
/*    status                                Actual completion status      */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a packet for sending */ 
/*    _nx_ppp_packet_transmit               Send PPP packet               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_ppp_ping_request(NX_PPP *ppp_ptr, CHAR *data_ptr, ULONG data_size, ULONG wait_option)
{

UINT        status;
UINT        i;
CHAR        *work_ptr;
NX_PACKET   *packet_ptr;


    /* Echo request may only be sent in the LCP completed state..  */
    if (ppp_ptr -> nx_ppp_lcp_state !=  NX_PPP_LCP_COMPLETED_STATE)
    {

         return NX_PPP_NOT_ESTABLISHED;
    }

    /* Allocate a packet for the PPP packet.  */
    status =  nx_packet_allocate(ppp_ptr -> nx_ppp_packet_pool_ptr, &packet_ptr, NX_PPP_PACKET, wait_option);

    /* Determine if the packet was allocated successfully.  */
    if (status != NX_SUCCESS)
    {

        /* An error was detected, simply return a NULL pointer.  */
        return status;
    }

    /* Check if out of boundary.  */
    if ((UINT)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < (UINT)(data_size + 10))
     {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
        return(NX_PPP_BAD_PACKET);
    }

    /* Increment the transmit ID.  */
    ppp_ptr -> nx_ppp_transmit_id++;

    /* Build the configuration request.  */
    packet_ptr -> nx_packet_prepend_ptr[0] =  (NX_PPP_LCP_PROTOCOL & 0xFF00) >> 8;
    packet_ptr -> nx_packet_prepend_ptr[1] =  NX_PPP_LCP_PROTOCOL & 0xFF;
    packet_ptr -> nx_packet_prepend_ptr[2] =  NX_PPP_LCP_ECHO_REQUEST;
    packet_ptr -> nx_packet_prepend_ptr[3] =  ppp_ptr -> nx_ppp_transmit_id;

    /* Indicate the PPP instance is waiting on an ECHO reply with this id: */
    ppp_ptr -> nx_ppp_lcp_echo_reply_id = ppp_ptr -> nx_ppp_transmit_id;

    /* Set up the length.  */
    packet_ptr -> nx_packet_prepend_ptr[4] =  (UCHAR)(((UINT)(data_size + 8)) >> 8);
    packet_ptr -> nx_packet_prepend_ptr[5] =  (UCHAR)(((UINT)(data_size + 8)) & 0xFF);

    /* Magic number will be all zeroes. */
    packet_ptr -> nx_packet_prepend_ptr[6] =   0;
    packet_ptr -> nx_packet_prepend_ptr[7] =  0;
    packet_ptr -> nx_packet_prepend_ptr[8] =  0;
    packet_ptr -> nx_packet_prepend_ptr[9] =  0;

    /* Load the data */
    work_ptr = data_ptr;

    /* Loop through the input buffer.  */
    for(i = 1; i <= data_size; i++)
    {

        /* Copy byte of IP address.  */
        packet_ptr -> nx_packet_prepend_ptr[i+9] =  (UCHAR)(*work_ptr);
        work_ptr++ ;
    }

    packet_ptr -> nx_packet_length =  i + 9;
    packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + packet_ptr -> nx_packet_length;

#ifndef NX_PPP_DISABLE_INFO

    /* Increment the number of LCP echo requests sent.  */
    ppp_ptr -> nx_ppp_lcp_echo_requests_sent++;

    /* Increment the number of LCP frames sent.  */
    ppp_ptr -> nx_ppp_lcp_frames_sent++;
#endif

    /* Send the configure request packet.  */
    _nx_ppp_packet_transmit(ppp_ptr, packet_ptr);

    return NX_SUCCESS;
}



#ifdef NX_PPP_PPPOE_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_packet_receive                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the PPP packet receive           */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    packet_ptr                            Pointer to packet to receive  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_ppp_packet_receive                Actual PPP packet receive     */ 
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
UINT  _nxe_ppp_packet_receive(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr)
{

UINT    status;


    /* Check for an invalid input pointer.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID))
        return(NX_PTR_ERROR);

    /* Check for packet pointer.  */
    if (packet_ptr == NX_NULL)
        return(NX_PTR_ERROR);

    /* Call actual packet receive function.  */
    status =  _nx_ppp_packet_receive(ppp_ptr, packet_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_packet_receive                              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function receives packet from the application (usually an      */ 
/*    application ISR), buffers it, and notifies the PPP receive thread.  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    packet_ptr                            Pointer to packet to receive  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Notify PPP receiving thread   */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code (Including ISRs)                                   */ 
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
UINT  _nx_ppp_packet_receive(NX_PPP *ppp_ptr, NX_PACKET *packet_ptr)
{

TX_INTERRUPT_SAVE_AREA 

    /* Disable interrupts.  */
    TX_DISABLE

    /* Check for no longer active PPP instance.  */
    if (ppp_ptr -> nx_ppp_id != NX_PPP_ID)
    {

        /* PPP is no longer active.  */

        /* Restore interrupts.  */
        TX_RESTORE

        /* Return an error.  */
        return(NX_PTR_ERROR);
    }

    /* Check for a stopped PPP instance.  */
    if (ppp_ptr -> nx_ppp_state == NX_PPP_STOPPED)
    {

        /* Silently discard byte.  */
        
        /* Restore interrupts.  */
        TX_RESTORE

        /* Return success.  */
        return(NX_SUCCESS);
    }

    /* Check to see if the deferred processing queue is empty.  */
    if (ppp_ptr -> nx_ppp_deferred_received_packet_head)
    {

        /* Not empty, just place the packet at the end of the queue.  */
        (ppp_ptr -> nx_ppp_deferred_received_packet_tail) -> nx_packet_queue_next = packet_ptr;
        packet_ptr -> nx_packet_queue_next =  NX_NULL;
        ppp_ptr -> nx_ppp_deferred_received_packet_tail = packet_ptr;

        /* Restore interrupts.  */
        TX_RESTORE
    }
    else
    {

        /* Empty deferred receive processing queue.  Just setup the head pointers and
           set the event flags to ensure the PPP helper thread looks at the deferred processing
           queue.  */
        ppp_ptr -> nx_ppp_deferred_received_packet_head = packet_ptr;
        ppp_ptr -> nx_ppp_deferred_received_packet_tail = packet_ptr;
        packet_ptr -> nx_packet_queue_next = NX_NULL;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Wakeup PPP helper thread to process the PPP deferred receive.  */  
        tx_event_flags_set(&(ppp_ptr -> nx_ppp_event), NX_PPP_EVENT_PPPOE_PACKET_RECEIVE, TX_OR);
    }

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_ppp_packet_send_set                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the PPP set packet send          */ 
/*    function call.                                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    nx_ppp_packet_send                    Routine to send PPP packet    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxe_ppp_packet_send_set              Actual PPP packet send set    */ 
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
UINT  _nxe_ppp_packet_send_set(NX_PPP *ppp_ptr, VOID (*nx_ppp_packet_send)(NX_PACKET *packet_ptr))
{

UINT    status;


    /* Check for an invalid input pointer.  */
    if ((ppp_ptr == NX_NULL) || (ppp_ptr -> nx_ppp_id != NX_PPP_ID))
        return(NX_PTR_ERROR);

    /* Check for packet pointer.  */
    if (nx_ppp_packet_send == NX_NULL)
        return(NX_PTR_ERROR);

    /* Call actual packet send set function.  */
    status =  _nx_ppp_packet_send_set(ppp_ptr, nx_ppp_packet_send);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_ppp_packet_send_set                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function set the PPP packet send function.                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ppp_ptr                               Pointer to PPP instance       */ 
/*    nx_ppp_packet_send                    Routine to send PPP packet    */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    none                                                                */ 
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
UINT  _nx_ppp_packet_send_set(NX_PPP *ppp_ptr, VOID (*nx_ppp_packet_send)(NX_PACKET *packet_ptr))
{


    /* Set the PPP packet send function.  */
    ppp_ptr -> nx_ppp_packet_send = nx_ppp_packet_send;

    /* Return successful completion.  */
    return(NX_SUCCESS);
}
#endif /*  NX_PPP_PPPOE_ENABLE  */
#endif /* NX_DISABLE_IPV4 */
