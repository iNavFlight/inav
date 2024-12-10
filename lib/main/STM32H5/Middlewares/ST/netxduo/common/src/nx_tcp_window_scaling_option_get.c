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

#ifdef NX_ENABLE_TCP_WINDOW_SCALING

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_window_scaling_option_get                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function searches for the Window Scale option.        */
/*    If found, first check the option length, if option length is not    */
/*    valid, it returns NX_FALSE to the caller, else it set the window    */
/*    scale size and returns NX_TRUE to the caller. Otherwise,            */
/*    NX_TRUE is returned.                                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    option_ptr                            Pointer to option area        */
/*    option_area_size                      Size of option area           */
/*    window_scale                          window scale size             */
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
UINT  _nx_tcp_window_scaling_option_get(UCHAR *option_ptr, ULONG option_area_size, ULONG *window_scale)
{

ULONG option_length;


    /* Set invalid window scaling, in case the SYN message does not contain Window Scaling feature. */
    *window_scale = 0xFF;

    /* Loop through the option area looking for the window scaling option.  */
    while (option_area_size >= 3)
    {

        /* Is the current character the window scaling type?  */
        if (*option_ptr == NX_TCP_RWIN_KIND)
        {

            /* Yes, we found it!  */

            /* Move the pointer forward by one.  */
            option_ptr++;

            /* Check the option length, if option length is not equal to 3, return NX_FALSE.  */
            if (*option_ptr++ != 3)
            {
                return(NX_FALSE);
            }

            /* Get the window scale size.  */
            *window_scale =  (ULONG)*option_ptr;

            if (*window_scale > 14)
            {

                /* Make sure window scale is limited to 14, per RFC 1323 pp.11 */
                *window_scale = 14;
            }

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
        if (*option_ptr == NX_TCP_NOP_KIND)
        {
            /* One character option!  Skip this option and move to the next entry. */
            option_ptr++;

            option_area_size--;
        }
        else
        {

            /* Derive the option length.  All options *fields* area 32-bits,
               but the options themselves may be padded by NOP's.   Determine
               the option size based on alignment of the option ptr */
            option_length = *(option_ptr + 1);

            if (option_length == 0)
            {
                /* Illegal option length. */
                return(NX_FALSE);
            }

            /* Move the option pointer forward.  */
            option_ptr =  option_ptr + option_length;

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
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */

