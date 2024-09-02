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
/*    _nx_ip_interface_physical_address_set               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the physical address to driver.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP control block pointer      */
/*    interface_index                       Index of interface            */
/*    physical_msw                          Physical address MSW          */
/*    physical_lsw                          Physical address LSW          */
/*    update_driver                         Update driver or not          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
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
UINT  _nx_ip_interface_physical_address_set(NX_IP *ip_ptr, UINT interface_index,
                                            ULONG physical_msw, ULONG physical_lsw, UINT update_driver)
{
NX_INTERFACE *if_ptr = &(ip_ptr -> nx_ip_interface[interface_index]);
NX_IP_DRIVER  driver_request;

    /* Initialize the status. */
    driver_request.nx_ip_driver_status = NX_SUCCESS;

    /* Obtain the IP internal mutex before calling the driver.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    if (update_driver == NX_TRUE)
    {

        /* Set driver request. */
        driver_request.nx_ip_driver_physical_address_msw = physical_msw;
        driver_request.nx_ip_driver_physical_address_lsw = physical_lsw;
        driver_request.nx_ip_driver_ptr         = ip_ptr;
        driver_request.nx_ip_driver_command     = NX_LINK_SET_PHYSICAL_ADDRESS;
        driver_request.nx_ip_driver_interface   = if_ptr;

        /* Call the link driver to set physical address. */
        (if_ptr -> nx_interface_link_driver_entry)(&driver_request);
    }

    /* Check status. */
    if (driver_request.nx_ip_driver_status == NX_SUCCESS)
    {

        /* Set physical address to interface. */
        if_ptr -> nx_interface_physical_address_msw = physical_msw;
        if_ptr -> nx_interface_physical_address_lsw = physical_lsw;
    }

    /* Release the IP internal mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return completion status.  */
    return(driver_request.nx_ip_driver_status);
}

