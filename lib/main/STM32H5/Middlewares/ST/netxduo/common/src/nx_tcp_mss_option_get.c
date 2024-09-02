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
#include "nx_tcp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_mss_option_get                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function searches for the Maximum Segment Size (MSS) option.   */
/*    If found, first check the option length, if option length is not    */
/*    valid, it returns NX_FALSE to the caller, else it set the mss value */
/*    and returns NX_TRUE to the caller. Otherwise, NX_TRUE is returned.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    option_ptr                            Pointer to option area        */
/*    option_area_size                      Size of option area           */
/*    mss                                   Max segment size              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_FALSE                              TCP option is invalid         */
/*    NX_TRUE                               TCP option is valid           */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_packet_process                TCP packet processing         */
/*    _nx_tcp_server_socket_relisten        Socket relisten processing    */
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
UINT  _nx_tcp_mss_option_get(UCHAR *option_ptr, ULONG option_area_size, ULONG *mss)
{

ULONG option_length;

    /* Initialize the value.  */
    *mss = 0;

    /* Loop through the option area looking for the MSS.  */
    while (option_area_size >= 4)
    {

        /* Is the current character the MSS type?  */
        if (*option_ptr == NX_TCP_MSS_KIND)
        {

            /* Yes, we found it!  */

            /* Move the pointer forward by one.  */
            option_ptr++;

            /* Check the option length, if option length is not equal to 4, return NX_FALSE.  */
            if (*option_ptr++ != 4)
            {
                return(NX_FALSE);
            }

            /* Build the mss size.  */
            *mss = (ULONG)*option_ptr++;

            /* Get the LSB of the MSS.  */
            *mss = (*mss << 8) | (ULONG)*option_ptr;

            /* Finished, get out of the loop!  */
            break;
        }

        /* Otherwise, process relative to the option type.  */

        /* Check for end of list.  */
        if (*option_ptr == NX_TCP_EOL_KIND)
        {

            /* Yes, end of list, get out!  */
            break;
        }

        /* Check for NOP.  */
        if (*option_ptr++ == NX_TCP_NOP_KIND)
        {

            /* One character option!  */
            option_area_size--;
        }
        else
        {

            /* Derive the option length.  */
            option_length =  ((ULONG)*option_ptr);

            /* Return when option length is invalid. */
            if (option_length == 0)
            {
                return(NX_FALSE);
            }

            /* Move the option pointer forward.  */
            option_ptr =  option_ptr + (option_length - 1);

            /* Determine if this is greater than the option area size.  */
            if (option_length > option_area_size)
            {
                return(NX_FALSE);
            }
            else
            {
                option_area_size =  option_area_size - option_length;
            }
        }
    }

    /* Return.  */
    return(NX_TRUE);
}

