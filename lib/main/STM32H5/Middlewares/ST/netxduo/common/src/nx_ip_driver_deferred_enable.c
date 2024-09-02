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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_driver_deferred_enable                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enabled the deferred driver packet processing         */
/*    in NetX.  The supplied driver packet handler is called from the     */
/*    IP helper thread to fully process a received driver packet.  The    */
/*    driver's receive packet processing only makes a call to the         */
/*    _nx_ip_driver_deferred_receive processing from the ISR.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    driver_deferred_packet_handler        Function pointer to driver's  */
/*                                            deferred packet handling    */
/*                                            routine                     */
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
/*    Application Driver Initialization                                   */
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
VOID  _nx_ip_driver_deferred_enable(NX_IP *ip_ptr, VOID (*driver_deferred_packet_handler)(NX_IP *ip_ptr, NX_PACKET *packet_ptr))
{

#ifdef NX_DRIVER_DEFERRED_PROCESSING
TX_INTERRUPT_SAVE_AREA

    /* Disable interrupts.  */
    TX_DISABLE

    /* Initialize the deferred packet queue to be empty */
    ip_ptr -> nx_ip_driver_deferred_packet_head =  NX_NULL;
    ip_ptr -> nx_ip_driver_deferred_packet_tail =  NX_NULL;

    /* Setup the driver's deferred packet processing routine */
    ip_ptr ->  nx_ip_driver_deferred_packet_handler =  driver_deferred_packet_handler;

    TX_RESTORE
#else
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(driver_deferred_packet_handler);
#endif
}

