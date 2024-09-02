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
/**   Internet Protocol version 6 Default Router Table (IPv6 router)      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
#define NX_SOURCE_CODE


#include "nx_api.h"
#include "nx_ipv6.h"

#ifdef FEATURE_NX_IPV6

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_find_max_prefix_length                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds the longest matching prefix between two IPv6    */
/*      addresses.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    addr1                                 IPv6 address 1                */
/*    addr2                                 IPv6 address 2                */
/*    max_length                            Maximum length to match       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Number of matching bits                                             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ipv6_source_selection                                           */
/*                                                                        */
/*  NOTE                                                                  */
/*                                                                        */
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
UINT _nxd_ipv6_find_max_prefix_length(ULONG *addr1, ULONG *addr2, UINT max_length)
{
UINT length = 0;
UINT i, j, bit, time;

    for (i = 0; i < 4; i++)
    {
        if (addr1[i] == addr2[i])
        {
            length += 32;
        }
        /* Length shall not exceed max_length. Stop compare. */
        else if (length + 31 < max_length)
        {
            break;
        }
        else
        {
            bit = 16;
            time = 16;
            for (j = 0; j < 5; j++)
            {
                time = time / 2;
                if (addr1[i] >> bit == addr2[i] >> bit)
                {
                    bit -= time;
                    if (time == 0)
                    {
                        length += (32 - bit);
                    }
                }
                else if (j == 4)
                {
                    length += (31 - bit);
                    break;
                }
                else
                {
                    bit += time;
                }
            }
            break;
        }
    }


    return(length);
}

#endif /* FEATURE_NX_IPV6 */

