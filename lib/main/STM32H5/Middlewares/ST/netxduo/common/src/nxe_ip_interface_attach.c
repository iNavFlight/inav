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

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_ip_interface_attach                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the IP interface attach          */
/*    function call.                                                      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr_value                          Pointer to IP control block   */
/*    interface_name                        Name of this interface        */
/*    ip_address                            IP Address, in host byte order*/
/*    network_mask                          Network Mask, in host byte    */
/*                                             order                      */
/*    ip_link_driver                        User supplied link driver     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_interface_attach               Attaches the interface        */
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
UINT _nxe_ip_interface_attach(NX_IP *ip_ptr, CHAR *interface_name, ULONG ip_address, ULONG network_mask, VOID (*ip_link_driver)(struct NX_IP_DRIVER_STRUCT *))
{

UINT status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) || (ip_link_driver == NX_NULL) || (interface_name == NX_NULL))
    {
        return(NX_PTR_ERROR);
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

    /* Call actual IP interface attach function.  */
    status =  _nx_ip_interface_attach(ip_ptr, interface_name, ip_address, network_mask, ip_link_driver);

    /* Return completion status.  */
    return(status);
}

