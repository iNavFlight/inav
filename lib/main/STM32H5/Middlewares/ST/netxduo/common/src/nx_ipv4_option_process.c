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
#include "nx_icmpv4.h"

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ipv4_option_process                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function goes through IPv4 option fields.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ipv4_packet_receive               Main IPv4 packet receive      */
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
UINT  _nx_ipv4_option_process(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{


NX_IPV4_HEADER *ip_header_ptr;
UCHAR          *option_ptr;
ULONG           ip_option_length;
#ifndef NX_DISABLE_ICMPV4_ERROR_MESSAGE
ULONG           ip_normal_length = 20;
#endif /* NX_DISABLE_ICMPV4_ERROR_MESSAGE */
UINT            index = 0;
UCHAR           op_type;
UCHAR           op_length;
UCHAR           op_timestamp_offset;
UCHAR           op_timestamp_overflow;
UCHAR           op_timestamp_flags;
UINT            op_timestamp_counter = 0;

    /* Set the IPv4 header and IPv4 option pointer.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ip_header_ptr = (NX_IPV4_HEADER *)(packet_ptr -> nx_packet_prepend_ptr);
    option_ptr = packet_ptr -> nx_packet_prepend_ptr + sizeof(NX_IPV4_HEADER);

    /* Calculate the IPv4 option length.  */
    ip_option_length = ((((ip_header_ptr -> nx_ip_header_word_0 & NX_IP_LENGTH_MASK) >> 24) - NX_IP_NORMAL_LENGTH) & 0xFF) * (ULONG)sizeof(ULONG);

    /* Loop to process the IPv4 option.  */
    while (index < ip_option_length)
    {

        /* Get the option type.  */
        op_type = *option_ptr;

        /* Process the option type. */
        switch (op_type)
        {

        case NX_IP_OPTION_END:
        {

            /* End option.  */
            return(NX_TRUE);
        }
        case NX_IP_OPTION_NO_OPERATION:
        {

            /* No opeartion.  */

            /* Update the Option pointer and index.  */
            option_ptr++;
            index++;
            continue;
        }
        case NX_IP_OPTION_INTERNET_TIMESTAMP:
        {

            /* Timestamp option. RFC781.  */

            /* Update the counter;  */
            op_timestamp_counter++;

            /* Check the counter.  */
            if (op_timestamp_counter > 1)
            {
#ifndef NX_DISABLE_ICMPV4_ERROR_MESSAGE
                /* Option length error, send a Parameter Problem Message .  */
                /*lint -e{835} -e{845} suppress operating on zero. */
                NX_ICMPV4_SEND_PARAMETER_PROBLEM(ip_ptr, packet_ptr, NX_ICMP_ZERO_CODE, (ip_normal_length + index + 2));
#endif
                /* Return NX_FALSE.  */
                return(NX_FALSE);
            }

            /* Get the option length.  */
            op_length = *(option_ptr + 1);

            /* Get the option offset.  */
            op_timestamp_offset = *(option_ptr + 2);

            /* Get the option overflow and flag.  */
            op_timestamp_overflow = (*(option_ptr + 3)) >> 4;
            op_timestamp_flags = (*(option_ptr + 3)) & 0xF;

            /* Only check the option errors.  */

            /* Check the option length error.  */
            if ((op_length < 8) || (op_length > 40) || ((op_length % 4) != 0))
            {

#ifndef NX_DISABLE_ICMPV4_ERROR_MESSAGE
                /* Option length error, send a Parameter Problem Message .  */
                /*lint -e{835} -e{845} suppress operating on zero. */
                NX_ICMPV4_SEND_PARAMETER_PROBLEM(ip_ptr, packet_ptr, NX_ICMP_ZERO_CODE, (ip_normal_length + index + 2));
#endif

                /* Return NX_FALSE.  */
                return(NX_FALSE);
            }

            /* Check the option offset error, offset must be greater than 5, and offset must be an odd number.  */
            if ((op_timestamp_offset < 5) || ((op_timestamp_offset % 2) == 0))
            {

#ifndef NX_DISABLE_ICMPV4_ERROR_MESSAGE
                /* Option offset error, send a Parameter Problem Message .  */
                /*lint -e{835} -e{845} suppress operating on zero. */
                NX_ICMPV4_SEND_PARAMETER_PROBLEM(ip_ptr, packet_ptr, NX_ICMP_ZERO_CODE, (ip_normal_length + index + 3));
#endif

                /* Return NX_FALSE.  */
                return(NX_FALSE);
            }

            /* Check the option overflow error.  */
            if (op_timestamp_overflow == 15)
            {

#ifndef NX_DISABLE_ICMPV4_ERROR_MESSAGE
                /* Option overflow error, send a Parameter Problem Message .  */
                /*lint -e{835} -e{845} suppress operating on zero. */
                NX_ICMPV4_SEND_PARAMETER_PROBLEM(ip_ptr, packet_ptr, NX_ICMP_ZERO_CODE, (ip_normal_length + index + 4));
#endif

                /* Return NX_FALSE.  */
                return(NX_FALSE);
            }

            /* Check the option flags error.  */
            if ((op_timestamp_flags != 0) && (op_timestamp_flags != 1) && (op_timestamp_flags != 3))
            {

#ifndef NX_DISABLE_ICMPV4_ERROR_MESSAGE
                /* Option flags error, send a Parameter Problem Message .  */
                /*lint -e{835} -e{845} suppress operating on zero. */
                NX_ICMPV4_SEND_PARAMETER_PROBLEM(ip_ptr, packet_ptr, NX_ICMP_ZERO_CODE, (ip_normal_length + index + 4));
#endif

                /* Return NX_FALSE.  */
                return(NX_FALSE);
            }
            break;
        }
        default:
            break;
        }

        /* Get the option length.  */
        op_length = *(option_ptr + 1);

        /* Check for invalid option length.
           RFC 791: The option-length octet counts the option-type octet and the 
           option-length octet as well as the option-data octets.  */
        if ((op_length < 2) || ((index + op_length) > ip_option_length))
        {
            return(NX_FALSE);
        }

        /* Move to the next top level option. */
        option_ptr += op_length;

        /* Update the index.  */
        index += op_length;
    }

    /* Return NX_TRUE.  */
    return(NX_TRUE);
}
#endif /* !NX_DISABLE_IPV4  */

