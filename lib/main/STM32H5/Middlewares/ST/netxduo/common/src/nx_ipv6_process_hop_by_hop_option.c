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
#include "nx_ipv6.h"
#include "nx_icmpv6.h"

#ifdef FEATURE_NX_IPV6


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ipv6_process_hop_by_hop_option                  PORTABLE C      */
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the Hop by Hop and the Destination headers. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to process  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion         */
/*    NX_OPTION_HEADER_ERROR                Error parsing packet options  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ipv6_option_error                Handle errors in IPv6 option   */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ipv6_dispatch_process            Process IPv6 optional header   */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer read overflow check, */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
UINT _nx_ipv6_process_hop_by_hop_option(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

INT                        header_length;
UINT                       offset_base, offset;
UINT                       rv;
NX_IPV6_HOP_BY_HOP_OPTION *option;


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /*  Make sure there's no OOB when reading Hdr Ext Len from the packet buffer. */
    if ((UINT)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr) < 2)
    {

        /* return an error code. */
        return(NX_OPTION_HEADER_ERROR);
    }

    /* Read the Hdr Ext Len field. */
    header_length = *(packet_ptr -> nx_packet_prepend_ptr + 1);

    /* Calculate the the true header length: (n + 1) * 8 */
    header_length = (header_length + 1) << 3;

    /* The 1st option starts from the 3rd byte. */
    offset = 2;

    /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
    /*lint -e{737} suppress loss of sign, since nx_packet_append_ptr is assumed to be larger than nx_packet_ip_header. */
    offset_base = (UINT)((ULONG)(packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_ip_header) - (ULONG)sizeof(NX_IPV6_HEADER));
    header_length = header_length - (INT)offset;

    /* Sanity check; does the header length data go past the end of the end of the packet buffer? */
    /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
    if ((UINT)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr) <
        ((UINT)header_length + offset))
    {

        /* Yes, handle the error as indicated by the option type 2 msb's. */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        option = (NX_IPV6_HOP_BY_HOP_OPTION *)(packet_ptr -> nx_packet_prepend_ptr + offset);

        _nx_ipv6_option_error(ip_ptr, packet_ptr, option -> nx_ipv6_hop_by_hop_option_type, offset_base + offset);
        return(NX_OPTION_HEADER_ERROR);
    }

    while (header_length > 0)
    {

        /* Get a pointer to the options. */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        option = (NX_IPV6_HOP_BY_HOP_OPTION *)(packet_ptr -> nx_packet_prepend_ptr + offset);

        switch (option -> nx_ipv6_hop_by_hop_option_type)
        {

        case 0:

            /* Pad1 option.  This option indicates the size of the padding is one.
               So we skip one byte. */
            offset++;
            header_length--;
            break;

        case 1:

            /* PadN option. Skip N+2 bytes. */
            offset += ((UINT)(option -> nx_ipv6_hop_by_hop_length) + 2);
            header_length -= ((INT)(option -> nx_ipv6_hop_by_hop_length) + 2);
            break;

#ifdef NX_ENABLE_THREAD
        case 109:

            /* RFC 7731.  */

            /* Skip N+2 bytes.  */
            offset += ((UINT)(option -> nx_ipv6_hop_by_hop_length) + 2);
            header_length -= ((INT)(option -> nx_ipv6_hop_by_hop_length) + 2);
            break;
#endif /* NX_ENABLE_THREAD  */

        default:

            /* Unknown option.  */
            rv = _nx_ipv6_option_error(ip_ptr, packet_ptr, option -> nx_ipv6_hop_by_hop_option_type, offset_base + offset);

            /* If no errors, just skip this option and move onto the next option.*/
            if (rv == NX_SUCCESS)
            {

                /* Skip this option and continue processing the rest of the header. */
                offset += ((UINT)(option -> nx_ipv6_hop_by_hop_length) + 2);
                header_length -= ((INT)(option -> nx_ipv6_hop_by_hop_length) + 2);
                break;
            }
            else
            {

                /* Return value indicates an error status: we need to drop the entire packet. */
                return(rv); /* Drop this packet. */
            }
        }
    }

    /* Successful processing of option header. */
    return(NX_SUCCESS);
}

#endif /*  FEATURE_NX_IPV6 */

