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
/**   Internet Control Message Protocol (ICMP)                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ipv6.h"
#include "nx_icmpv6.h"

#ifdef FEATURE_NX_IPV6

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv6_validate_options                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function validates ICMPv6 additional options.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    option                       Pointer to ICMPv6 option               */
/*    length                       Length of the Option field             */
/*    additional_check             Whether or not caller wishes to        */
/*                                    perform additoinal verification     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                   Options are valid                      */
/*    NX_NOT_SUCCESS               Options are invalid                    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_validate_neighbor_messages                               */
/*    _nx_icmpv6_validate_ra                                              */
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
UINT _nx_icmpv6_validate_options(NX_ICMPV6_OPTION *option, INT length, INT additional_check)
{

UINT option_len;

    /* Parse all option headers from the ICMPv6 header. */
    while (length > 0)
    {
        /* Verify that the option length is not zero. */
        if (option -> nx_icmpv6_option_length == 0)
        {
            return(NX_NOT_SUCCESSFUL);
        }

        /* Also check for NO SOURCE LINK LAYER ADDRESS.  */
        if ((additional_check == NX_NO_SLLA) &&
            (option -> nx_icmpv6_option_type == 1))
        {

            return(NX_NOT_SUCCESSFUL);
        }

        /* Get the next option. */
        option_len = ((UINT)option -> nx_icmpv6_option_length) << 3;
        length -= (INT)option_len;

        /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
        option = (NX_ICMPV6_OPTION *)NX_UCHAR_POINTER_ADD(option, option_len);
    }

    if (length < 0)
    {

        /* Invalid packet length. */
        return(NX_NOT_SUCCESSFUL);
    }

    return(NX_SUCCESS);
}

#endif /* FEATURE_NX_IPV6 */

