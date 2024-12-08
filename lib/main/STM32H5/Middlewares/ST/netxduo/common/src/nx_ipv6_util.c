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

#define NX_SOURCE_CODE

#include "tx_api.h"
#include "nx_api.h"
#include "nx_ipv6.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    CHECK_IP_ADDRESSES_BY_PREFIX                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks whether or not two IPv6 addresses are the      */
/*    same by prefix length                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_addr1                              Two 128-bit IPv6 addresses to */
/*    ip_addr2                              be checked.                   */
/*    prefix_len                            Prefix length                 */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    0                                     The addresses are different.  */
/*    1                                     The addresses are the same.   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
INT CHECK_IP_ADDRESSES_BY_PREFIX(ULONG *ip_addr1, ULONG *ip_addr2,
                                 ULONG prefix_len)
{


ULONG high_prefix; /* Num of ULONG in prefix. */
ULONG low_prefix;  /* Remaining bits in prefix after high prefix. */
ULONG mask;


    /* Get number of ULONGs that can fit in the specified prefix length. */
    high_prefix  = prefix_len >> 5;

    /* Get the remaining bits in prefix length. */
    low_prefix  = prefix_len &  0x1f;

    /* Would the prefix length have 1 or more ULONGs? */
    if (high_prefix)
    {

        /* Yes; compare that number of ULONGS (in bytes) in each input address. */
        if (memcmp(ip_addr1, ip_addr2, high_prefix << 2))
        {
            /* A nonzero result indicates a mismatch. */
            return(0);
        }
    }

    /* Are there any bits to compare after the high order bits? */
    if (low_prefix)
    {

        /* Compare these bits between the two addresses, after masking out the upper ULONGs. */
        mask = ((0xFFFFFFFF) << (32 - low_prefix)) & 0xFFFFFFFF;

        if ((ip_addr1[high_prefix] ^ ip_addr2[high_prefix]) & mask)
        {
            return(0);
        }
    }

    return(1);
}



/*
 * Developers may define "NX_IPV6_UTIL_INLINE to make the following functions
 * inline.  Inline functions improve execution speed.  However they make
 * code size larger.
 */

#ifndef NX_IPV6_UTIL_INLINE
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    CHECK_IPV6_ADDRESSES_SAME                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks whether or not two IPv6 addresses are the      */
/*    same.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_addr1                              Two 128-bit IPv6 addresses to */
/*    ip_addr2                              be checked.                   */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    0                                     The addresses are different.  */
/*    1                                     The addresses are the same.   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
INT CHECK_IPV6_ADDRESSES_SAME(ULONG *ip_addr1, ULONG *ip_addr2)
{
#ifdef FEATURE_NX_IPV6
    return(ip_addr1[0] == ip_addr2[0] &&
           ip_addr1[1] == ip_addr2[1] &&
           ip_addr1[2] == ip_addr2[2] &&
           ip_addr1[3] == ip_addr2[3]);
#else /* FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_addr1);
    NX_PARAMETER_NOT_USED(ip_addr2);

    return(0);
#endif /* FEATURE_NX_IPV6 */
}

#ifdef NX_IPSEC_ENABLE
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    CHECK_IPV6_ADDRESS_RANGE                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks the IPv6 address whether or not between        */
/*    two IPv6 addresses.                                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_addr_start                         IPv6 address start            */
/*    ip_addr_end                           Ipv6 address end              */
/*    ip_addr                               The 128-bit IPv6 address to.  */
/*                                               be checked               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    0                                    Not between two IPv6 address.  */
/*    1                                    Between two IPv6 address.      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
INT CHECK_IPV6_ADDRESS_RANGE(ULONG *ip_addr_start, ULONG *ip_addr_end, ULONG *ip_addr)
{
#ifdef FEATURE_NX_IPV6
INT ip_addr_cmp1 = 0, ip_addr_cmp2 = 0;

    /* Check the IP address whether or not bigger than IP address start.  */
    /* compare the first 32 bit.  */
    if (ip_addr_start[0] < ip_addr[0])
    {
        ip_addr_cmp1 = 1;
    }
    else if (ip_addr_start[0] == ip_addr[0])
    {

        /* compare the second 32 bit.  */
        if (ip_addr_start[1] < ip_addr[1])
        {
            ip_addr_cmp1 = 1;
        }
        else if (ip_addr_start[1] == ip_addr[1])
        {
            /* compare the third 32 bit.  */
            if (ip_addr_start[2] < ip_addr[2])
            {
                ip_addr_cmp1 = 1;
            }
            else if (ip_addr_start[2] == ip_addr[2])
            {
                /* compare the forth 32 bit.  */
                if (ip_addr_start[3] <= ip_addr[3])
                {
                    ip_addr_cmp1 = 1;
                }
            }
        }
    }

    /* Check the IP address whether or not smaller than IP address end.  */
    /* compare the first 32 bit.  */
    if (ip_addr[0] < ip_addr_end[0])
    {
        ip_addr_cmp2 = 1;
    }
    else if (ip_addr[0] == ip_addr_end[0])
    {
        /* compare the second 32 bit.  */
        if (ip_addr[1] < ip_addr_end[1])
        {
            ip_addr_cmp2 = 1;
        }
        else if (ip_addr[1] == ip_addr_end[1])
        {
            /* compare the second 32 bit.  */
            if (ip_addr[2] < ip_addr_end[2])
            {
                ip_addr_cmp2 = 1;
            }
            else if (ip_addr[2] == ip_addr_end[2])
            {
                /* compare the second 32 bit.  */
                if (ip_addr[3] <= ip_addr_end[3])
                {
                    ip_addr_cmp2 = 1;
                }
            }
        }
    }
    if ((ip_addr_cmp1 == 1) && (ip_addr_cmp2 == 1))
    {
        return(1);
    }
    else
    {
        return(0);
    }
#else /* FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_addr_start);
    NX_PARAMETER_NOT_USED(ip_addr_end);
    NX_PARAMETER_NOT_USED(ip_addr);

    return(0);
#endif /* FEATURE_NX_IPV6 */
}
#endif /* NX_IPSEC_ENABLE */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    CHECK_UNSPECIFIED_ADDRESS                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks whether or not an address is unspecified (::)  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_address                            The 128-bit IPv6 address to   */
/*                                          be checked.  The address is   */
/*                                          in host byte order.           */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                0:The address is not zero     */
/*                                          1:The address is unspecified  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
INT CHECK_UNSPECIFIED_ADDRESS(ULONG *ip_addr)
{
#ifdef FEATURE_NX_IPV6
    return(!(ip_addr[0] || ip_addr[1] || ip_addr[2] || ip_addr[3]));
#else
    NX_PARAMETER_NOT_USED(ip_addr);
    return(0);
#endif
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    SET_UNSPECIFIED_ADDRESS                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function marks an IPv6 address as unnspecified (::).           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_address                            The 128-bit IPv6 address to   */
/*                                          be set.                       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
void SET_UNSPECIFIED_ADDRESS(ULONG *ip_addr)
{
#ifdef FEATURE_NX_IPV6
    ip_addr[0] = 0;
    ip_addr[1] = 0;
    ip_addr[2] = 0;
    ip_addr[3] = 0;
#else
    NX_PARAMETER_NOT_USED(ip_addr);
#endif /* FEATURE_NX_IPV6 */
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    COPY_IPV6_ADDRESS                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function makes a copy of an IPv6 address.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    copy_from                             The 128-bit IPv6 address to   */
/*                                          be copied.                    */
/*    copy_to                               The 128-bit IPv6 address to   */
/*                                          be filled in.                 */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
void COPY_IPV6_ADDRESS(ULONG *copy_from, ULONG *copy_to)
{
#ifdef FEATURE_NX_IPV6
    copy_to[0] = copy_from[0];
    copy_to[1] = copy_from[1];
    copy_to[2] = copy_from[2];
    copy_to[3] = copy_from[3];
#else
    NX_PARAMETER_NOT_USED(copy_from);
    NX_PARAMETER_NOT_USED(copy_to);
#endif /* FEATURE_NX_IPV6 */
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    COPY_NXD_ADDRESS                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function makes a copy of an IP address from one NXD_ADDRESS    */
/*    data to another NXD_ADDRESS, including the version.  Note that      */
/*    filling in the nxd_ip_address.v6[0] address also suffices for       */
/*    filling in the nxd_ip_address.v4 IP address field if the input      */
/*    NXD_ADDRESS argument is for an IPv4 address.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    copy_from                             The NXD address control block */
/*                                          to be copied.                 */
/*    copy_to                               The NXD address control block */
/*                                          to copy to.                   */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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

void COPY_NXD_ADDRESS(NXD_ADDRESS *copy_from, NXD_ADDRESS  *copy_to)
{
#ifdef FEATURE_NX_IPV6
    copy_to -> nxd_ip_version       = copy_from -> nxd_ip_version;
    copy_to -> nxd_ip_address.v6[0] = copy_from -> nxd_ip_address.v6[0];
    copy_to -> nxd_ip_address.v6[1] = copy_from -> nxd_ip_address.v6[1];
    copy_to -> nxd_ip_address.v6[2] = copy_from -> nxd_ip_address.v6[2];
    copy_to -> nxd_ip_address.v6[3] = copy_from -> nxd_ip_address.v6[3];
#else
    NX_PARAMETER_NOT_USED(copy_from);
    NX_PARAMETER_NOT_USED(copy_to);
#endif /* FEATURE_NX_IPV6 */
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    SET_SOLICITED_NODE_MULTICAST_ADDRESS                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the solicited-node multicast address based on    */
/*    a unicast IPv6 address.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    address                               Storage space of an IPv6      */
/*                                          solicited-node multicast      */
/*                                          address to be created.        */
/*    unicast_address                       The unicast address to use    */
/*                                          when creating the solicited-  */
/*                                          node multicast address.       */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
void SET_SOLICITED_NODE_MULTICAST_ADDRESS(ULONG *address,
                                          ULONG *unicast_address)
{
#ifdef FEATURE_NX_IPV6
    address[0] = (ULONG)0xFF020000;
    address[1] = (ULONG)0;
    address[2] = (ULONG)0x00000001;
    address[3] = (ULONG)(0xFF000000 | unicast_address[3]);
#else
    NX_PARAMETER_NOT_USED(address);
    NX_PARAMETER_NOT_USED(unicast_address);
#endif /* FEATURE_NX_IPV6 */
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    CHECK_ALL_ROUTER_MCAST_ADDRESS                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks whether or not an address is an all-router     */
/*    multicast address.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    address                               The 128-bit IPv6 address to   */
/*                                          be checked.  The address is   */
/*                                          in host byte order.           */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    0                                     The address is not an all-    */
/*                                          router mullticast address.    */
/*    1                                     The address is an all-router  */
/*                                          multicast address.            */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
INT CHECK_ALL_ROUTER_MCAST_ADDRESS(ULONG *address)
{
#ifdef FEATURE_NX_IPV6

    return(address[0] == (ULONG)0xFF020000 &&
           address[1] == (ULONG)0 &&
           address[2] == (ULONG)0 &&
           address[3] == (ULONG)0x00000002);

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(address);

    return(0);
#endif /* FEATURE_NX_IPV6 */
}

#endif /* NX_IPV6_UTIL_INLINE */



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    CHECK_IPV6_SOLICITED_NODE_MCAST_ADDRESS             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks whether or not an address is a solicited-node  */
/*    multicast address.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dest_ip                               The 128-bit IPv6 address to   */
/*                                          be checked.  The address is   */
/*                                          in host byte order.           */
/*    myip                                  The 128-bit local interface   */
/*                                          address, in host byte order.  */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    0                                     The address is not solicited- */
/*                                          node multicast address.       */
/*    1                                     The address is solicited-node */
/*                                          multicast address.            */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
INT CHECK_IPV6_SOLICITED_NODE_MCAST_ADDRESS(ULONG *dest_ip, ULONG *myip)
{
#ifdef FEATURE_NX_IPV6

INT isMulticast = 0;

    if ((dest_ip[0] == (ULONG)0xFF020000) && (dest_ip[1] == (ULONG)0x0) &&
        (dest_ip[2] == (ULONG)0x00000001) &&
        (dest_ip[3] == ((myip[3] & (ULONG)0x00FFFFFF) | (ULONG)0xFF000000)))
    {
        isMulticast = 1;
    }
    else if ((dest_ip[0] == (ULONG)0xFF020000) &&
             (dest_ip[1] == (ULONG)0x0) &&
             (dest_ip[2] == (ULONG)0x0) &&
             ((dest_ip[3] == (ULONG)0x00000001) || (dest_ip[3] == (ULONG)0x00010002)))
    {
        isMulticast = 1;
    }
#ifdef NX_ENABLE_THREAD
    else if ((dest_ip[0] == (ULONG)0xFF030000) &&
             (dest_ip[1] == (ULONG)0x0) &&
             (dest_ip[2] == (ULONG)0x0) &&
             ((dest_ip[3] == (ULONG)0x00000001)))     /* Realm-Local All nodes multicast address.     */
    {
        isMulticast = 1;
    }
#endif /* NX_ENABLE_THREAD  */
    else if ((dest_ip[0] == (ULONG)0xFF050000) &&
             (dest_ip[1] == (ULONG)0x0) &&
             (dest_ip[2] == (ULONG)0x0) &&
             (dest_ip[3] == (ULONG)0x00010003))
    {
        isMulticast = 1;
    }
    return(isMulticast);
#else
    NX_PARAMETER_NOT_USED(dest_ip);
    NX_PARAMETER_NOT_USED(myip);

    return(0);
#endif /* FEATURE_NX_IPV6 */
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    IPv6_Address_Type                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks the type of an IP address.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_address                            The 128-bit IPv6 address to   */
/*                                          be checked.  The address is   */
/*                                          in host byte order.           */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Address type                          A bit mask indicates the      */
/*                                          of the IPv6 address.          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
ULONG IPv6_Address_Type(ULONG *ip_address)
{

#ifdef FEATURE_NX_IPV6
ULONG ret;

/* Validate address type.
   ::/128                  Unspecified address
   ::1/128                 Loopback
   FF00::/8                Multicast
   FE80::/10               Link-local
   FEC0::/10               Global (Its use as Site-local has been deprecated (RFC 4291)
   Everything else         Global
 */
ULONG tmp;

    /* Is this multicast? */
    if ((ip_address[0] & (ULONG)0xFF000000) == (ULONG)0xFF000000)
    {
        /* Yes. */
        ret = IPV6_ADDRESS_MULTICAST;

        /* Determine type of multicast... */
        if (((ip_address[0] == (ULONG)0xFF010000) ||
             (ip_address[0] == (ULONG)0xFF020000)) &&
            (ip_address[1] == (ULONG)0) &&
            (ip_address[2] == (ULONG)0) &&
            (ip_address[3] == (ULONG)1))
        {
            return(ret | IPV6_ALL_NODE_MCAST);
        }


        if ((ip_address[0] == (ULONG)0xFF050000) &&     /* All DHCPv6 relay and server hosts */
            (ip_address[1] == (ULONG)0) &&
            (ip_address[2] == (ULONG)0) &&
            (ip_address[3] == (ULONG)0x00010003))
        {
            return(ret | IPV6_ALL_NODE_MCAST);
        }

        if (((ip_address[0] == (ULONG)0xFF010000) ||
             (ip_address[0] == (ULONG)0xFF020000) ||
             (ip_address[0] == (ULONG)0xFF050000)) &&
            (ip_address[1] == (ULONG)0) &&
            (ip_address[2] == (ULONG)0) &&
            (ip_address[3] == (ULONG)2))
        {
            return(ret | IPV6_ALL_ROUTER_MCAST);
        }

        if ((ip_address[0] == (ULONG)0xFF020000) &&
            (ip_address[1] == (ULONG)0) &&
            (ip_address[2] == (ULONG)1) &&
            (ip_address[3] >= (ULONG)0xFF000000))
        {
            return(ret | IPV6_SOLICITED_NODE_MCAST);
        }

        return(IPV6_ADDRESS_MULTICAST);
    }

    tmp = ip_address[0] & (0xFFC00000);

    if (tmp == (ULONG)0xFE800000)
    {
        return((ULONG)(IPV6_ADDRESS_UNICAST | IPV6_ADDRESS_LINKLOCAL));
    }
    /* Note that site local are deprecated in RFC 4291 and are
       treated as global type address. */
    if (tmp == (ULONG)0xFEC00000)
    {
        return((ULONG)(IPV6_ADDRESS_UNICAST | IPV6_ADDRESS_GLOBAL));
    }

    tmp = ip_address[0] | ip_address[1] | ip_address[2];

    if (tmp == 0)
    {
        if (ip_address[3] == 0)
        {
            return(IPV6_ADDRESS_UNSPECIFIED);
        }

        if (ip_address[3] == 1)
        {
            return(IPV6_ADDRESS_LOOPBACK);
        }
    }

    return((ULONG)(IPV6_ADDRESS_UNICAST | IPV6_ADDRESS_GLOBAL));
#else /* FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_address);

    return(0);
#endif /* FEATURE_NX_IPV6 */
}

#ifdef NX_LITTLE_ENDIAN
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ipv6_address_change_endian                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function only applies to little endian hosts.  It performs     */
/*    byte swaps on an IPv6 address.                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    address                            The 128-bit IPv6 address to be   */
/*                                          swapped.                      */
/*                                                                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
VOID _nx_ipv6_address_change_endian(ULONG *address)
{
#ifdef FEATURE_NX_IPV6
    if (address == NX_NULL)
    {
        return;
    }

    NX_CHANGE_ULONG_ENDIAN(address[0]);
    NX_CHANGE_ULONG_ENDIAN(address[1]);
    NX_CHANGE_ULONG_ENDIAN(address[2]);
    NX_CHANGE_ULONG_ENDIAN(address[3]);
#else
    NX_PARAMETER_NOT_USED(address);
#endif /* FEATURE_NX_IPV6 */
}

#endif /* NX_LITTLE_ENDIAN */

