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
/**   Dynamic Host Configuration Protocol over IPv6 (DHCPv6 Client)       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_DHCPV6_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* Include necessary system files.  */

#include    <stdio.h> 
#include    "nx_api.h"
#include    "nx_system.h"
#include    "nx_ip.h"
#include    "nx_ipv6.h"
#include    "nx_udp.h"
#include    "nxd_dhcpv6_client.h"
#include    "tx_timer.h"

#ifdef FEATURE_NX_IPV6

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/* Define the address of All DHCP Relay Agents and Servers as per RFC3315.  */
static NXD_ADDRESS  All_DHCPv6_Relay_Servers_Address;

/* Keep the DHCPv6 instance for DAD callback notify.  */
static NX_DHCPV6    *_nx_dhcpv6_DAD_ptr;


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_client_duid                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the client DUID to the Client request packet     */
/*    based on the Client DUID on record, checking to make sure it does   */
/*    not go past the end of the packet payload.                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    received_buffer                   Pointer to server reply           */
/*    length                            Size of server reply buffer       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD                               */
/*                                      Not enough room for the Client    */ 
/*                                        DUID in the packet payload      */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                            Clears specified area of memory   */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_send_request           Compiles and sends the Client     */
/*                                            DHCPv6 request              */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcpv6_add_client_duid(NX_DHCPV6 *dhcpv6_ptr, UCHAR *buffer_ptr, UINT *index)
{

ULONG message_word;
ULONG available_payload;
UCHAR mac[8];
UINT  i = 0;


    /* Compute the available payload in the packet buffer. */
    available_payload = dhcpv6_ptr -> nx_dhcpv6_pool_ptr -> nx_packet_pool_payload_size - 
                        (ULONG)sizeof(NX_IPV6_HEADER) - (ULONG)sizeof(NX_UDP_HEADER) - *index;

    /* Check if the largest possible client DUID will fit in the packet buffer. */
    if ((available_payload - 4) < ((ULONG)(dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_option_length))) 
    {

        /* Hmmm... not enough! Can't do it. */
        return NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD;
    }

    /* Build the header from the client DUID. */
    message_word = (((ULONG)(dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_op_code)) << 16);
    message_word |= (dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_option_length);

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Copy first half of the Client DUID option to packet buffer. */
    memcpy((buffer_ptr + (*index)), &message_word, sizeof(UINT)); /* Use case of memcpy is verified. */
    (*index) += (ULONG)sizeof(UINT);

    /* Build the DUID type and hardware type.  */
    message_word = (((ULONG)(dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_duid_type)) << 16);
    message_word |= dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_hardware_type;

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Set up the DUID type and hardware type. */
    memcpy((buffer_ptr + (*index)), &message_word, sizeof(UINT)); /* Use case of memcpy is verified. */
    (*index) += (ULONG)sizeof(UINT);

    /* Include the 'time' field if this is a Link layer time DUID type. */
    if (dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_duid_type == NX_DHCPV6_DUID_TYPE_LINK_TIME)
    {

        /* Build the time.  */
        message_word = dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_duid_time;

        /* Adjust for endianness. */
        NX_CHANGE_ULONG_ENDIAN(message_word);

        /* Set up the time.  */
        memcpy((buffer_ptr + (*index)), &message_word, sizeof(ULONG)); /* Use case of memcpy is verified. */
        (*index) += (ULONG)sizeof(ULONG);
    }

    /* Build the link layer address.  */
    if (dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_hardware_type == NX_DHCPV6_CLIENT_HARDWARE_TYPE_ETHERNET)
    {

        /* The length of link layer address is 48 bit.  */

        /* Build the MSW.  */
        mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_msw >> 8);
        mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_msw);
    }
    else if(dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_hardware_type == NX_DHCPV6_CLIENT_HARDWARE_TYPE_EUI_64)
    {

        /* The length of link layer address is 64 bits.  */

        /* Build the MSW.  */
        mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_msw >> 24);
        mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_msw >> 16);
        mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_msw >> 8);
        mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_msw);
    }

    /* Build the LSW.  */
    mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_lsw >> 24);
    mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_lsw >> 16);
    mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_lsw >> 8);
    mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_lsw);

    /* Set up the link layer address.  */
    memcpy((buffer_ptr + (*index)), mac, i); /* Use case of memcpy is verified. */
    (*index) += i;

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_elapsed_time                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the elapsed time option to the Client request    */
/*    packet using the session duration time tracked by the DHCPv6 Client */
/*    thread, and checks to make sure it does not go past the end of the  */ 
/*    packet payload.                                                     */ 
/*                                                                        */
/*    Note that the elapsed time is the time from which the Client        */
/*    initiates the request to the time the it gets and processes a valid */
/*    server reply.  Its accuracy depends on the resolution of the session*/
/*    timer.                                                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    buffer_ptr                        Pointer to request packet buffer  */
/*    index                             Location into buffer to write data*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD                               */
/*                                      Not enough room for the elapsed   */ 
/*                                       time option in the packet payload*/
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                            Clears specified area of memory   */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_send_request           Compiles and sends the Client     */
/*                                            DHCPv6 request              */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_add_elapsed_time(NX_DHCPV6 *dhcpv6_ptr, UCHAR *buffer_ptr, UINT *index)
{

ULONG available_payload;


    /* Compute the available payload for DHCP data in the packet buffer. */
    available_payload = (dhcpv6_ptr -> nx_dhcpv6_pool_ptr -> nx_packet_pool_payload_size - 
                         (ULONG)sizeof(NX_IPV6_HEADER) - (ULONG)sizeof(NX_UDP_HEADER) - *index);

    /* Check if the data will fit in the packet buffer. */
    if (available_payload < sizeof(NX_DHCPV6_ELAPSED_TIME))
    {

        /* Hmmm... not enough! Can't do it. */
        return NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD;
    }

    /* Adjust for endianness. */
    NX_CHANGE_USHORT_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_op_code);
    NX_CHANGE_USHORT_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_op_length);
    NX_CHANGE_USHORT_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time);

    /* Set up the elapsed time option, in hundredths of seconds. */
    memcpy((buffer_ptr + (*index)), &(dhcpv6_ptr -> nx_dhcpv6_elapsed_time), sizeof(NX_DHCPV6_ELAPSED_TIME)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(NX_DHCPV6_ELAPSED_TIME);

    /* Swap bytes back. */
    NX_CHANGE_USHORT_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_op_code);
    NX_CHANGE_USHORT_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_op_length);
    NX_CHANGE_USHORT_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time);

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_ia_address                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the IA address option to the Client request      */
/*    packet using the Client IA on record.  Also checks to make sure     */
/*    adding the IA option does not go past the end of the packet payload.*/
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    buffer_ptr                        Pointer to request packet buffer  */
/*    index                             Location into buffer to write data*/
/*    ia_index                          The index of IA address           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INVALID_IA_ADDRESS      IA address on record is NULL      */
/*    NX_DHCPV6_OPTION_NO_DATA          No IA option in client record     */
/*    NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD                               */
/*                                      Not enough room for the IA address*/ 
/*                                           option in the packet payload */
/*  CALLS                                                                 */ 
/*                                                                        */
/*    memset                            Clears specified area of memory   */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_iana               Adds the IANA and optionally the  */
/*                                         IA option to Client request    */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_add_ia_address(NX_DHCPV6 *dhcpv6_ptr, UCHAR *buffer_ptr, UINT *index, UINT ia_index)
{

ULONG   message_word;
ULONG   available_payload;

    /* Set the IA option length. */
    dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_option_length = (6 * sizeof(ULONG));

    /* Compute the available payload for DHCP data in the packet buffer. */
    available_payload = (dhcpv6_ptr -> nx_dhcpv6_pool_ptr -> nx_packet_pool_payload_size - 
                         (ULONG)sizeof(NX_IPV6_HEADER) - (ULONG)sizeof(NX_UDP_HEADER) - *index);

    /* Check if the client IA address option will fit in the packet buffer. */
    if (available_payload < (((ULONG)dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_option_length) + 4)) 
    {

        /* Hmmm... not enough! Can't do it. */
        return NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD;
    }

    /* Check that the IA address option has been created. */
    if (dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_op_code != NX_DHCPV6_OP_IA_ADDRESS)
    {

        /* Missing or invalid IA option and/or data.  */
        return NX_DHCPV6_INVALID_IA_DATA;
    }

    /* Make sure the IA address block has a non zero IP address. */
    if ((dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[0] == 0) &&
        (dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[1] == 0) &&
        (dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[2] == 0) &&
        (dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[3] == 0))
    {

        return NX_DHCPV6_INVALID_IA_ADDRESS;
    }

    /* If the client has created an address option, apply it to the message. */

    /* Clear memory to make the first word of the IA option header. */
    memset(&message_word, 0, sizeof(ULONG));

    /* Extract the IA data from the client record. */
    message_word = (((ULONG)dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_op_code) << 16);
    message_word |= dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_option_length;

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Copy IA data into the packet and update the index into the buffer. */
    memcpy((buffer_ptr + (*index)), &message_word, sizeof(ULONG)); /* Use case of memcpy is verified. */
     (*index) += (ULONG)sizeof(ULONG);

     /* Add the IPv6 address.  */
    /* Adjust for endianness.  */
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[0]);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[1]);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[2]);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[3]);

    /* Copy the Client preferred IPv6 address to the packet. */
    memcpy((buffer_ptr + (*index)), &(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[0]), sizeof(ULONG)); /* Use case of memcpy is verified. */
    (*index) += (ULONG)sizeof(ULONG);
    memcpy((buffer_ptr + (*index)), &(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[1]), sizeof(ULONG)); /* Use case of memcpy is verified. */
    (*index) += (ULONG)sizeof(ULONG);
    memcpy((buffer_ptr + (*index)), &(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[2]), sizeof(ULONG)); /* Use case of memcpy is verified. */
    (*index) += (ULONG)sizeof(ULONG);
    memcpy((buffer_ptr + (*index)), &(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[3]), sizeof(ULONG)); /* Use case of memcpy is verified. */
    (*index) += (ULONG)sizeof(ULONG);

    /* Swap bytes back. */
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[0]);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[1]);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[2]);
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[3]);
    
    /* Add the preferred lifetime and valid lifetime option.  */
    if((dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_SOLICIT) ||
       (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_CONFIRM) ||
       (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_DECLINE) ||
       (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_RELEASE))
    {

        /* When sending a confirm request, DHCPv6 Client should set its T1,T2,preferred-lifetime and 
           valid-lifetime to zero, as the server will ignore these fields, RFC3315,page41.  */
        memset((buffer_ptr + (*index)), 0, sizeof(ULONG));
        (*index) += (ULONG)sizeof(ULONG);
        memset((buffer_ptr + (*index)), 0, sizeof(ULONG));
        (*index) += (ULONG)sizeof(ULONG);
    }
    else
    {

        /* Adjust for endianness. */
        NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_preferred_lifetime);
        NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_valid_lifetime);

        memcpy((buffer_ptr + (*index)), &(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_preferred_lifetime), sizeof(ULONG)); /* Use case of memcpy is verified. */
        (*index) += (ULONG)sizeof(ULONG);
        memcpy((buffer_ptr + (*index)), &(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_valid_lifetime), sizeof(ULONG)); /* Use case of memcpy is verified. */
        (*index) += (ULONG)sizeof(ULONG);

        /* Swap bytes back. */
        NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_preferred_lifetime);
        NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_valid_lifetime);

    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_iana                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the IANA option to the Client request  packet    */
/*    using the Client IANA on record.  Also checks to make sure adding   */
/*    the IANA option does not go past the end of the packet payload. If  */
/*    exist IA options, it will call the service to add the IA option     */
/*    within the IANA option data. It then updates the IANA length field  */
/*    for the embedded IA option.                                         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    buffer_ptr                        Pointer to request packet buffer  */
/*    index                             Location into buffer to write data*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD                               */
/*                                      Not enough room for the IANA      */ 
/*                                           option in the packet payload */
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_dhcpv6_add_ia_address         Adds the IA option to buffer      */
/*    memset                            Clears specified area of memory   */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_send_request           Compiles and sends the Client     */
/*                                            DHCPv6 request              */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_add_iana(NX_DHCPV6 *dhcpv6_ptr, UCHAR *buffer_ptr, UINT *index)
{

UINT    status;
UINT    temp_index;
UINT    ia_index;
UINT    ia_length;
ULONG   message_word;
ULONG   available_payload;


    /* Initialize the completion status variable. */
    status = NX_SUCCESS;
    ia_length = 0;

    /* Compute the available payload in the packet buffer. */
    available_payload = dhcpv6_ptr -> nx_dhcpv6_pool_ptr -> nx_packet_pool_payload_size - 
                        (ULONG)sizeof(NX_IPV6_HEADER) - (ULONG)sizeof(NX_UDP_HEADER) - *index;

    /* Check if the client IANA will fit in the packet buffer. */
    if (available_payload < (((ULONG)dhcpv6_ptr -> nx_dhcpv6_iana.nx_option_length) + 4)) 
    {

        /* Hmmm... not enough! Can't do it. */
        return NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD;
    }
    
    /* Save this location in the buffer. */
    temp_index = *index;

    /* Skip the option code and option lenght of IA_NA, write these option after padding the all IA option. */
    (*index) += (ULONG)sizeof(ULONG);

    /* Add the IA_NA ID.  */
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_iana.nx_IA_NA_id);
    memcpy((buffer_ptr + (*index)), &(dhcpv6_ptr -> nx_dhcpv6_iana.nx_IA_NA_id), sizeof(ULONG)); /* Use case of memcpy is verified. */
    (*index) += (ULONG)sizeof(ULONG);    
    NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_iana.nx_IA_NA_id);

    /* Add the T1 and T2 option.  */
    if((dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_SOLICIT) ||
       (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_CONFIRM) ||
       (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_DECLINE) ||
       (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_RELEASE))
    {

        /* When sending a confirm request, DHCPv6 Client should set its T1,T2,preferred-lifetime and 
           valid-lifetime to zero, as the server will ignore these fields, RFC3315,page41.  */
        memset((buffer_ptr + (*index)), 0, sizeof(ULONG)); /* Use case of memcpy is verified. */
        (*index) += (ULONG)sizeof(ULONG);
        memset((buffer_ptr + (*index)), 0, sizeof(ULONG)); /* Use case of memcpy is verified. */
        (*index) += (ULONG)sizeof(ULONG);
    }
    else
    {

        /* Adjust the data.  */
        NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_iana.nx_T1);
        NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_iana.nx_T2);

        memcpy((buffer_ptr + (*index)), &(dhcpv6_ptr -> nx_dhcpv6_iana.nx_T1), sizeof(ULONG)); /* Use case of memcpy is verified. */
        (*index) += (ULONG)sizeof(ULONG);
        memcpy((buffer_ptr + (*index)), &(dhcpv6_ptr -> nx_dhcpv6_iana.nx_T2), sizeof(ULONG)); /* Use case of memcpy is verified. */
        (*index) += (ULONG)sizeof(ULONG);

        /* Swap bytes back. */
        NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_iana.nx_T1);
        NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_iana.nx_T2);
    }

    for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
    {

        /* Check the IA address status.  */
        if(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status)
        {

            /* Yes, so add one...*/
            status = _nx_dhcpv6_add_ia_address(dhcpv6_ptr, buffer_ptr, index, ia_index);

            /* Was the IA address option get appended successfully? */
            if (status == NX_SUCCESS)
            {

                /* Update the IA address to record the IANA option length.  */
                ia_length = (UINT)(ia_length + dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_option_length + 4);
            }
        }
    }
    
    /* Clear memory to make the first word of the IA-NA header. */
    memset(&message_word, 0, sizeof(ULONG));
    
    /* Set the IA_NA option code and option length.  */
    dhcpv6_ptr -> nx_dhcpv6_iana.nx_op_code = NX_DHCPV6_OP_IA_NA;
    dhcpv6_ptr -> nx_dhcpv6_iana.nx_option_length = (USHORT)(ia_length + 12);

    /* Write the IANA opcode and data length into one word. */
    message_word = (((ULONG)(dhcpv6_ptr -> nx_dhcpv6_iana.nx_op_code)) << 16);
    message_word |= dhcpv6_ptr -> nx_dhcpv6_iana.nx_option_length;

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Copy the word into the packet buffer going to the server. */
    memcpy((buffer_ptr + temp_index), &message_word, sizeof(ULONG)); /* Use case of memcpy is verified. */
    
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_option_request                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the option request option to the Client request  */
/*    packet using the internal 'register' in the DHCPv6 Client instance  */
/*    for what DHCPv6 options have been set or cleared. Also checks to    */
/*    make sure it does not go past the end of the packet payload.        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    buffer_ptr                        Pointer to request packet buffer  */
/*    index                             Location into buffer to write data*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD                               */
/*                                      Not enough room for the option    */ 
/*                                       request in the packet payload    */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                            Clears specified area of memory   */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_send_request           Compiles and sends the Client     */
/*                                            DHCPv6 request              */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_add_option_request(NX_DHCPV6 *dhcpv6_ptr, UCHAR *buffer_ptr, UINT *index) 
{

USHORT   message_word;
USHORT   option_list;
USHORT   option_length = 0;
UCHAR    *option_length_ptr;
ULONG    available_payload;


    /* Compute the available payload for DHCP data in the packet buffer. */
    available_payload = (dhcpv6_ptr -> nx_dhcpv6_pool_ptr -> nx_packet_pool_payload_size - 
                         (ULONG)sizeof(NX_IPV6_HEADER) - (ULONG)sizeof(NX_UDP_HEADER) - *index);

    /* Check if the option request option will fit in the packet buffer. */
    if (available_payload < (((ULONG)(dhcpv6_ptr -> nx_dhcpv6_option_request.nx_option_length)) + 4))
    {

        /* Hmmm... not enough! Can't do it. */
        return NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD;
    }

    /* Set a local variable for convenience. */
    option_list = dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request;
        
    /* Update the option code.
       Adjust for endianness. */
    NX_CHANGE_USHORT_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_code);

    /* Copy the option code to the packet. */
    memcpy((buffer_ptr + (*index)), &(dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_code), sizeof(USHORT)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(USHORT);
    
    /* Swap bytes back. */
    NX_CHANGE_USHORT_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_code);

    /* Record the option length pointer and skip it, update the option length after fill the option data.  */
    option_length_ptr = buffer_ptr + (*index);
    *index += (ULONG)sizeof(USHORT);

    /* Now check to see which options we add as option data: */

    /* Is the DNS server option requested? */
    if(option_list & NX_DHCPV6_DNS_SERVER_OPTION)
    {

        /* Yes, add this option code into the option request data. */
        message_word = NX_DHCPV6_OP_DNS_SERVER;

        /* Adjust for endianness. */
        NX_CHANGE_USHORT_ENDIAN(message_word);

        memcpy((buffer_ptr + (*index)), &message_word, sizeof(SHORT)); /* Use case of memcpy is verified. */

        /* Update the location of the buffer write pointer. */
        *index += (ULONG)sizeof(SHORT);      
        option_length = (USHORT)(option_length + sizeof(SHORT));
    }

    /* Is the timeS server option requested? */
    if(option_list & NX_DHCPV6_SNTP_SERVER_OPTION)
    {

        /* Yes, add this option code into the option request data. */
        message_word = NX_DHCPV6_OP_SNTP_SERVER;

        /* Adjust for endianness. */
        NX_CHANGE_USHORT_ENDIAN(message_word);

        memcpy((buffer_ptr + (*index)), &message_word, sizeof(SHORT)); /* Use case of memcpy is verified. */

        /* Update the location of the buffer write pointer. */
        *index += (ULONG)sizeof(SHORT);
        option_length = (USHORT)(option_length + sizeof(SHORT));
    }

    /* Is the time zone option requested? */
    if(option_list & NX_DHCPV6_NEW_POSIX_TIMEZONE_OPTION)
    {

        /* Yes, add this option code into the option request data. */
        message_word = NX_DHCPV6_OP_NEW_POSIX_TIMEZONE;

        /* Adjust for endianness. */
        NX_CHANGE_USHORT_ENDIAN(message_word);

        memcpy((buffer_ptr + (*index)), &message_word, sizeof(SHORT)); /* Use case of memcpy is verified. */

        /* Update the location of the buffer write pointer. */
        *index += (ULONG)sizeof(SHORT);
        option_length = (USHORT)(option_length + sizeof(SHORT));
    }

    /* Is the domain name option requested? */
    if(option_list & NX_DHCPV6_DOMAIN_NAME_OPTION)
    {

        /* Yes, add this option code into the option request data. */
        message_word = NX_DHCPV6_OP_DOMAIN_NAME;

        /* Adjust for endianness. */
        NX_CHANGE_USHORT_ENDIAN(message_word);

        memcpy((buffer_ptr + (*index)), &message_word, sizeof(SHORT)); /* Use case of memcpy is verified. */

        /* Update the location of the buffer write pointer. */
        *index += (ULONG)sizeof(SHORT);
        option_length = (USHORT)(option_length + sizeof(SHORT));
    }
       
    /* Is the fully qualified domain name option requested? 
       A client MUST only include the Client FQDN option in SOLICIT, REQUEST, RENEW, or REBIND message.
       A client that sends the Client FQDN option MUST also include the option in the Option Requst option 
       if it expects the server to include the Client FQDN option in any responses. RFC4704, Section5, Page7.  */
    if(((dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_SOLICIT) ||
        (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_REQUEST) ||
        (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_RENEW) ||
        (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_REBIND)) &&
       (option_list & NX_DHCPV6_CLIENT_FQDN_OPTION))
    {

        /* Yes, add this option code into the option request data. */
        message_word = NX_DHCPV6_OP_CLIENT_FQDN;

        /* Adjust for endianness. */
        NX_CHANGE_USHORT_ENDIAN(message_word);

        memcpy((buffer_ptr + (*index)), &message_word, sizeof(SHORT)); /* Use case of memcpy is verified. */

        /* Update the location of the buffer write pointer. */
        *index += (ULONG)sizeof(SHORT);
        option_length = (USHORT)(option_length + sizeof(SHORT));
    }

    /* Now record and update the option lenght.  */    
    dhcpv6_ptr -> nx_dhcpv6_option_request.nx_option_length = option_length;    
    NX_CHANGE_USHORT_ENDIAN(option_length);    
    memcpy(option_length_ptr, &option_length, sizeof(SHORT)); /* Use case of memcpy is verified. */

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_server_duid                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the server DUID to the Client request packet     */
/*    based on the server DUID on record, checking to make sure it does   */
/*    not go past the end of the packet payload.                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    received_buffer                   Pointer to server reply           */
/*    length                            Size of server reply buffer       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD                               */
/*                                      Not enough room for the server    */ 
/*                                        DUID in the packet payload      */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                            Clears specified area of memory   */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_send_request           Compiles and sends the Client     */
/*                                            DHCPv6 request              */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_add_server_duid(NX_DHCPV6 *dhcpv6_ptr, UCHAR *buffer_ptr, UINT *index)
{

ULONG   message_word;
ULONG   available_payload;
UCHAR   mac[8];
UINT    i = 0;


    /* Compute the available payload for DHCP data in the packet buffer. */
    available_payload = (dhcpv6_ptr -> nx_dhcpv6_pool_ptr -> nx_packet_pool_payload_size - 
                         (ULONG)sizeof(NX_IPV6_HEADER) - (ULONG)sizeof(NX_UDP_HEADER) - *index);

    /* Check if the largest possible server DUID will fit in the packet buffer. */
    if (available_payload < (((ULONG)(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_option_length)) + 4)) 
    {

        /* Hmmm... not enough! Can't do it. */
        return NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD;
    }

    /* Make sure the Client has a valid Server DUID. */
    if (dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_op_code != NX_DHCPV6_OP_SERVER_ID)
    {

        /* Missing or invalid option data.  */
        return NX_DHCPV6_INVALID_SERVER_DUID;
    }

    /* Build the header from the client's record of its DHCPv6 Server DUID. */
    message_word = (((ULONG)(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_op_code)) << 16);
    message_word |= (dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_option_length);

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Copy first half of the Server DUID option to packet buffer. */
    memcpy((buffer_ptr + (*index)), &message_word, sizeof(ULONG)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(ULONG);

    /* Build the DUID type and hardware type.  */
    message_word = (((ULONG)(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_duid_type)) << 16);
    message_word |= dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_hardware_type;

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Set up the DUID type and hardware type. */
    memcpy((buffer_ptr + (*index)), &message_word, sizeof(UINT)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(UINT);

    /* Include the 'time' field if this is a Link layer time DUID type. */
    if (dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_duid_type == NX_DHCPV6_DUID_TYPE_LINK_TIME)
    {

        /* Build the time.  */
        message_word = dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_duid_time;

        /* Adjust for endianness. */
        NX_CHANGE_ULONG_ENDIAN(message_word);

        /* Set up the time.  */
        memcpy((buffer_ptr + (*index)), &message_word, sizeof(ULONG)); /* Use case of memcpy is verified. */
        (*index) += (ULONG)sizeof(ULONG);
    }

    /* Build the link layer address.  */
    if (dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_hardware_type == NX_DHCPV6_CLIENT_HARDWARE_TYPE_ETHERNET)
    {

        /* The length of link layer address is 48 bit.  */

        /* Build the MSW.  */
        mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_msw >> 8);
        mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_msw);
    }
    else if(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_hardware_type == NX_DHCPV6_CLIENT_HARDWARE_TYPE_EUI_64)
    {

        /* The length of link layer address is 64 bits.  */

        /* Build the MSW.  */
        mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_msw >> 24);
        mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_msw >> 16);
        mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_msw >> 8);
        mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_msw);
    }

    /* Build the LSW.  */
    mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_lsw >> 24);
    mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_lsw >> 16);
    mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_lsw >> 8);
    mac[i++] = (UCHAR)(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_lsw);

    /* Set up the link layer address.  */
    memcpy((buffer_ptr + (*index)), mac, i); /* Use case of memcpy is verified. */
    (*index) += i;

    return NX_SUCCESS;
}
     

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_client_FQDN                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the DHCPv6 Client FQDN Option to the Client      */
/*    request packet , checking to make sure it does not go past the end  */
/*    of the packet payload.                                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    received_buffer                   Pointer to server reply           */
/*    length                            Size of server reply buffer       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD                               */
/*                                      Not enough room for the server    */ 
/*                                        DUID in the packet payload      */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                            Clears specified area of memory   */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_send_request           Compiles and sends the Client     */
/*                                            DHCPv6 request              */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_add_client_FQDN(NX_DHCPV6 *dhcpv6_ptr, UCHAR *buffer_ptr, UINT *index)
{

ULONG   message_word;
ULONG   available_payload; 
UINT    domain_name_length;


    /* Compute the available payload for DHCP data in the packet buffer. */
    available_payload = (dhcpv6_ptr -> nx_dhcpv6_pool_ptr -> nx_packet_pool_payload_size - 
                         (ULONG)sizeof(NX_IPV6_HEADER) - (ULONG)sizeof(NX_UDP_HEADER) - *index);

    /* Check if the largest possible server DUID will fit in the packet buffer. */
    if (available_payload < (((ULONG)(dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_option_length)) + 4)) 
    {

        /* Hmmm... not enough! Can't do it. */
        return NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD;
    }
    
    memset(&message_word, 0, sizeof(ULONG));

    /* Set the option code and option length.  */
    message_word = (((ULONG)(dhcpv6_ptr -> nx_dhcpv6_client_FQDN.nx_op_code)) << 16);
    message_word |= (dhcpv6_ptr -> nx_dhcpv6_client_FQDN.nx_op_length);

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Add the option code and option length into packet buffer. */
    memcpy((buffer_ptr + (*index)), &message_word, sizeof(ULONG)); /* Use case of memcpy is verified. */
    *index += (ULONG)sizeof(ULONG);

    /* Add the flags into packet buffer.  */
    *(buffer_ptr + (*index)) = dhcpv6_ptr -> nx_dhcpv6_client_FQDN.nx_flags;
    *index += 1;

    /* Encode the domain name into the packet buffer.  */                                        
    domain_name_length = _nx_dhcpv6_name_string_encode(buffer_ptr + (*index), (UCHAR *)dhcpv6_ptr -> nx_dhcpv6_client_FQDN.nx_domain_name);
    *index += domain_name_length;

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_client_create                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX create dhcpv6     */
/*    client service.                                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPV6 Client        */ 
/*    ip_ptr                              Pointer to Client IP instance   */ 
/*    name_ptr                            DHCPV6 name pointer             */ 
/*    packet_pool_ptr                     Pointer to Client packet pool   */ 
/*    stack_ptr                           Pointer to free memory          */
/*    stack_size                          Size of client stack memory     */
/*    dhcpv6_state_change_notify          Client state change handler     */
/*    dhcpv6_server_error_handler         Server error status handler     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*    NX_DHCPV6_PARAM_ERROR               Invalid non pointer input       */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_client_create           Actual Client create function    */ 
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
UINT  _nxe_dhcpv6_client_create(NX_DHCPV6 *dhcpv6_ptr, NX_IP *ip_ptr, CHAR *name_ptr, 
                                NX_PACKET_POOL *packet_pool_ptr, VOID *stack_ptr, ULONG stack_size,
                                VOID (*dhcpv6_state_change_notify)(struct NX_DHCPV6_STRUCT *dhcpv6_ptr, UINT old_state, UINT new_state),
                                VOID (*dhcpv6_server_error_handler)(struct NX_DHCPV6_STRUCT *dhcpv6_ptr, UINT op_code, UINT status_code, UINT message_type))                        
{

UINT    status;


    /* Check for invalid pointer input.  */
    if (!dhcpv6_ptr ||!ip_ptr || !packet_pool_ptr || !stack_ptr)
    {
    
        return NX_PTR_ERROR;
    }

    /* Check for invalid non pointer input. */
    if ((ip_ptr -> nx_ip_id != NX_IP_ID) || (stack_size < TX_MINIMUM_STACK))
    {

        return NX_DHCPV6_PARAM_ERROR;
    }

    /* Call actual DHCPV6 create service.  */
    status =  _nx_dhcpv6_client_create(dhcpv6_ptr, ip_ptr, name_ptr, packet_pool_ptr, stack_ptr, stack_size,
                                       dhcpv6_state_change_notify, dhcpv6_server_error_handler);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_client_create                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates the DHCPv6 Client instance with a Netx packet */ 
/*    pool, processing thread, and various flag event queues, timers and  */
/*    mutexes necessary for DHCPv6 Client operations. Reocrd the DHCPv6   */ 
/*    pointer for DAD callback function process.                          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPV6 Client        */ 
/*    ip_ptr                              Pointer to Client IP instance   */ 
/*    name_ptr                            DHCPV6 name pointer             */ 
/*    packet_pool_ptr                     Pointer to Client packet pool   */ 
/*    stack_ptr                           Pointer to free memory          */
/*    stack_size                          Size of Client stack memory     */
/*    dhcpv6_state_change_notify          Client state change handler     */
/*    dhcpv6_server_error_handler         Server error status handler     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Actual completion status      */ 
/*    NX_SUCCESS                            Successful completion status  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_create                Create ThreadX flag event queue*/
/*    tx_mutex_create                      Create mutex lock on resource  */
/*    nx_packet_pool_delete                 Delete the DHCPV6 packet pool */ 
/*    nx_udp_socket_create                  Create the DHCPV6 UDP socket  */ 
/*    nx_udp_socket_delete                  Delete the DHCPV6 UDP socket  */ 
/*    tx_mutex_create                       Create DHCPV6 mutex           */ 
/*    tx_mutex_delete                       Delete DHCPV6 mutex           */ 
/*    tx_thread_create                      Create DHCPV6 thread          */ 
/*    tx_timer_create                       Create DHCPV6 timer           */ 
/*    tx_timer_delete                       Delete DHCPV6 timer           */  
/*    nxd_ipv6_address_change_notify        Set the DAD callback function */
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
UINT  _nx_dhcpv6_client_create(NX_DHCPV6 *dhcpv6_ptr, NX_IP *ip_ptr, CHAR *name_ptr, 
                               NX_PACKET_POOL *packet_pool_ptr, VOID *stack_ptr, ULONG stack_size, 
                               VOID (*dhcpv6_state_change_notify)(struct NX_DHCPV6_STRUCT *dhcpv6_ptr, UINT old_state, UINT new_state),
                               VOID (*dhcpv6_server_error_handler)(struct NX_DHCPV6_STRUCT *dhcpv6_ptr, UINT op_code, UINT status_code, UINT message_type))                        

{

UINT  status;


    /* Initialize the DHCPV6 control block to zero.  */
    memset((void *) dhcpv6_ptr, 0, sizeof(NX_DHCPV6));

    /* Initialize the DHCPV6 Server and Relay multicast address.  */
    memset(&All_DHCPv6_Relay_Servers_Address, 0, sizeof(NXD_ADDRESS));
    All_DHCPv6_Relay_Servers_Address.nxd_ip_version = NX_IP_VERSION_V6;
    All_DHCPv6_Relay_Servers_Address.nxd_ip_address.v6[0] = 0xff020000;  
    All_DHCPv6_Relay_Servers_Address.nxd_ip_address.v6[1] = 0x00000000;
    All_DHCPv6_Relay_Servers_Address.nxd_ip_address.v6[2] = 0x00000000;
    All_DHCPv6_Relay_Servers_Address.nxd_ip_address.v6[3] = 0x00010002;

    /* Set the destination address.  */
    COPY_NXD_ADDRESS(&All_DHCPv6_Relay_Servers_Address, &(dhcpv6_ptr -> nx_dhcpv6_client_destination_address));

    /* Set the DHCPv6 IP pointer.  */
    dhcpv6_ptr -> nx_dhcpv6_ip_ptr =  ip_ptr;

    /* Set the DHCPV6 name.  */
    dhcpv6_ptr -> nx_dhcpv6_name =  name_ptr;

    /* If multihome support is available, default the client DHCP network to the primary interface. */
    dhcpv6_ptr -> nx_dhcpv6_client_interface_index = 0;

    /* Set the Client packet pool for sending messages to the Server. */
    dhcpv6_ptr -> nx_dhcpv6_pool_ptr = packet_pool_ptr;

    /* Create the IP timer event flag instance.  */
    status = tx_event_flags_create(&(dhcpv6_ptr -> nx_dhcpv6_events), "DHCPv6 Client Timer Events Queue");

    /* Check for error. */
    if (status != TX_SUCCESS)
    {

        return status;
    }

    /* Create the DHCPV6 client mutex.  */
    status =  tx_mutex_create(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex), "DHCPV6 Client Process State", TX_NO_INHERIT);

    /* Determine if the mutexes creation was successful.  */
    if (status)
    {

        /* Delete the flag queue.  */
        tx_event_flags_delete(&dhcpv6_ptr -> nx_dhcpv6_events);

        /* No, return error status.  */
        return status;
    }

    /* Create the DHCPV6 timer for keeping track of the IP lease's time remaining before expiration.  */
    status =  tx_timer_create(&(dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_timer), "NetX DHCPV6 Client IP Lease timer",
                              _nx_dhcpv6_IP_lifetime_timeout_entry, (ULONG)(ALIGN_TYPE)dhcpv6_ptr,
                              (NX_DHCPV6_IP_LIFETIME_TIMER_INTERVAL * NX_DHCPV6_TICKS_PER_SECOND), 
                              (NX_DHCPV6_IP_LIFETIME_TIMER_INTERVAL * NX_DHCPV6_TICKS_PER_SECOND), 
                              TX_NO_ACTIVATE);

    NX_TIMER_EXTENSION_PTR_SET(&(dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_timer), dhcpv6_ptr)

    /* Determine if the timer creation was successful.  */
    if (status)
    {

        /* Delete the mutex. */
        tx_mutex_delete(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* Delete all the timers, since we don't know which create call fails. */
        tx_timer_delete(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));

        /* Delete the UDP socket. */
        nx_udp_socket_delete(&(dhcpv6_ptr -> nx_dhcpv6_socket));

        /* Delete the packet pool. */
        /* Delete the flag queue. */
        tx_event_flags_delete(&dhcpv6_ptr -> nx_dhcpv6_events);

        /* No, return error status. */
        return status;
    }

    /* Create the DHCPV6 timer for keeping track of the DHCPv6 Client session time.  */
    status =  tx_timer_create(&(dhcpv6_ptr -> nx_dhcpv6_session_timer), "NetX DHCPV6 Client Session Duration timer",
                             _nx_dhcpv6_session_timeout_entry, (ULONG)(ALIGN_TYPE)dhcpv6_ptr,
                             (NX_DHCPV6_SESSION_TIMER_INTERVAL * NX_DHCPV6_TICKS_PER_SECOND), 
                             (NX_DHCPV6_SESSION_TIMER_INTERVAL * NX_DHCPV6_TICKS_PER_SECOND), 
                              TX_NO_ACTIVATE);

    NX_TIMER_EXTENSION_PTR_SET(&(dhcpv6_ptr -> nx_dhcpv6_session_timer), dhcpv6_ptr)

    /* Determine if the timers creation was successful.  */
    if (status)
    {

        /* Delete the mutex. */
        tx_mutex_delete(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* Delete all the timers, since we don't know which create call fails. */
        tx_timer_delete(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));
        tx_timer_delete(&(dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_timer));

        /* Delete the UDP socket. */
        nx_udp_socket_delete(&(dhcpv6_ptr -> nx_dhcpv6_socket));

        /* Delete the packet pool. */
        /* Delete the flag queue. */
        tx_event_flags_delete(&dhcpv6_ptr -> nx_dhcpv6_events);

        /* No, return error status. */
        return status;
    }

    /* Create the DHCPV6 processing thread. */
    status =  tx_thread_create(&(dhcpv6_ptr -> nx_dhcpv6_thread), "NetX DHCPV6 Client", _nx_dhcpv6_thread_entry, 
                               (ULONG)(ALIGN_TYPE)dhcpv6_ptr, stack_ptr, stack_size, 
                               NX_DHCPV6_THREAD_PRIORITY, NX_DHCPV6_THREAD_PRIORITY, 1, TX_DONT_START);

    NX_THREAD_EXTENSION_PTR_SET(&(dhcpv6_ptr -> nx_dhcpv6_thread), dhcpv6_ptr)

    /* Determine if the thread creation was successful. */
    if (status)
    {

        /* Delete the timers.  */        
        tx_timer_delete(&(dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_timer));
        tx_timer_delete(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));

        /* Delete the mutex.  */
        tx_mutex_delete(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* Delete the flag queue.  */
        tx_event_flags_delete(&dhcpv6_ptr -> nx_dhcpv6_events);

        /* No, return error status.  */
        return status;
    }

    /* Create the DHCP socket. */
    status = nx_udp_socket_create(dhcpv6_ptr -> nx_dhcpv6_ip_ptr, &(dhcpv6_ptr -> nx_dhcpv6_socket), "NetX DHCPV6 Client",
                                  NX_DHCPV6_TYPE_OF_SERVICE, NX_DONT_FRAGMENT, 
                                  NX_DHCPV6_TIME_TO_LIVE, NX_DHCPV6_QUEUE_DEPTH);

    /* Was the socket creation successful?  */
    if (status != NX_SUCCESS)
    {

        /* Delete the timers.  */        
        tx_timer_delete(&(dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_timer));
        tx_timer_delete(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));

        /* Delete the mutex.  */
        tx_mutex_delete(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* Delete the flag queue.  */
        tx_event_flags_delete(&dhcpv6_ptr -> nx_dhcpv6_events);

        tx_thread_delete(&dhcpv6_ptr -> nx_dhcpv6_thread);

        /* No, return error status.  */
        return status;
    }

    /* Set the Client in the initial state.  */
    dhcpv6_ptr -> nx_dhcpv6_state =  NX_DHCPV6_STATE_INIT;

    /* Set the 'time accrued' on the Client global IP address at zero seconds. */
    dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued = 0;

    /* Update the DHCPv6 Client ID.  */
    dhcpv6_ptr -> nx_dhcpv6_id =  NX_DHCPV6_ID;

    /* Save the DHCPV6 instance pointer in the socket. */
    dhcpv6_ptr -> nx_dhcpv6_socket.nx_udp_socket_reserved_ptr =  (void *) dhcpv6_ptr;

    /* Set up the elapsed time for the anticipated message exchange with the DHCPv6 Server. */
    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_op_code = NX_DHCPV6_OP_ELAPSED_TIME;
    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;

    /* Set the elapsed time option data length, minus the option header. */
    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_op_length = sizeof(NX_DHCPV6_ELAPSED_TIME) - 4;

    /* Set the option request option code. Cannot set the data length as yet. */
    dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_code = NX_DHCPV6_OP_OPTION_REQUEST;

    /* Assign the various handlers (server error messages and state change). */
    dhcpv6_ptr -> nx_dhcpv6_state_change_callback =  dhcpv6_state_change_notify;
    dhcpv6_ptr -> nx_dhcpv6_server_error_handler = dhcpv6_server_error_handler;

#if !defined (NX_DISABLE_IPV6_DAD) && defined (NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY)
    /* Set the callback function to detect DAD process.
       If DAD failure, automatically set event to send DHCP decline meessage.  
       Notice: other modules should not set the address change notify function again.  */
    status = nxd_ipv6_address_change_notify(dhcpv6_ptr -> nx_dhcpv6_ip_ptr, _nx_dhcpv6_ipv6_address_DAD_notify);

    /* Was the callback notify fucntion creation successful?  */
    if (status != NX_SUCCESS)
    {

        /* Delete the timers.  */        
        tx_timer_delete(&(dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_timer));
        tx_timer_delete(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));

        /* Delete the mutex.  */
        tx_mutex_delete(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* Delete the flag queue.  */
        tx_event_flags_delete(&dhcpv6_ptr -> nx_dhcpv6_events);

        /* Delete the thread.  */
        tx_thread_delete(&dhcpv6_ptr -> nx_dhcpv6_thread);
        
        /* Delete the thread.  */
        nx_udp_socket_delete(&(dhcpv6_ptr -> nx_dhcpv6_socket));

        /* No, return error status.  */
        return status;
    }
#endif

    /* Keep the DHCPv6 instance for DAD callback notify.  */
    _nx_dhcpv6_DAD_ptr = dhcpv6_ptr;

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_client_delete                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX delete dhcpv6     */
/*    client service.                                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_client_delete            Actual DHCPV6 delete function   */ 
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
UINT  _nxe_dhcpv6_client_delete(NX_DHCPV6 *dhcpv6_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((dhcpv6_ptr == NX_NULL) || (dhcpv6_ptr -> nx_dhcpv6_id != NX_DHCPV6_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCPV6 delete service.  */
    status =  _nx_dhcpv6_client_delete(dhcpv6_ptr);

    /* Return commpletion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_client_delete                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes the DHCPV6 Client instance and releases all   */
/*    NetX Duo and ThreadX resources.                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful Completion status  */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_unbind                  Release DHCPV6 UDP socket port*/ 
/*    nx_udp_socket_delete                  Delete the DHCPV6 UDP socket  */ 
/*    tx_thread_terminate                   Terminate DHCPV6 thread       */ 
/*    tx_thread_delete                      Delete DHCPV6 thread          */ 
/*    tx_timer_delete                       Delete DHCPV6 timers          */ 
/*    tx_mutex_delete                       Delete DHCPV6 mutexes         */
/*    tx_event_flags_delete                 Delete DHCPV6 flag queue      */ 
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
UINT  _nx_dhcpv6_client_delete(NX_DHCPV6 *dhcpv6_ptr)
{

    
    /* Terminate the DHCPV6 processing thread.  */
    tx_thread_terminate(&(dhcpv6_ptr -> nx_dhcpv6_thread));

    /* Delete the DHCPV6 processing thread.  */
    tx_thread_delete(&(dhcpv6_ptr -> nx_dhcpv6_thread));

    /* Delete the flag event queue. */
    tx_event_flags_delete(&dhcpv6_ptr -> nx_dhcpv6_events);

    /* Delete the timers */
    tx_timer_delete(&(dhcpv6_ptr->nx_dhcpv6_IP_lifetime_timer));
    tx_timer_delete(&(dhcpv6_ptr->nx_dhcpv6_session_timer));

    /* Delete the mutex.  */
    tx_mutex_delete(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

    /* Release the UDP socket port. */
    nx_udp_socket_unbind(&(dhcpv6_ptr -> nx_dhcpv6_socket));

    /* Delete the UDP socket.  */
    nx_udp_socket_delete(&(dhcpv6_ptr -> nx_dhcpv6_socket));

    /* Clear the dhcpv6 structure ID. */
    dhcpv6_ptr -> nx_dhcpv6_id =  0;

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_create_client_duid                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the create Client DUID     */
/*    service.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    duid_type                         Type of DUID (link layer +/- time)*/
/*    hardware_type                     Network hardware type e.g IEEE 802*/
/*    time                              Time stamp unique DUID identifier */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Actual completion status         */ 
/*    NX_PTR_ERROR                       Invalid pointer input            */ 
/*    NX_DHCPV6_PARAM_ERROR              Invalid non pointer input        */ 
/*    NX_DHCPV6_UNSUPPORTED_DUID_TYPE    Unsupported DUID type            */
/*    NX_DHCPV6_UNSUPPORTED_DUID_HW_TYPE Unsupported DUID hardware type   */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_create_client_duid     Actual create client DUID service */ 
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
UINT    _nxe_dhcpv6_create_client_duid(NX_DHCPV6 *dhcpv6_ptr, UINT duid_type, UINT hardware_type, ULONG time)
{

UINT status;


    /* Check for invalid pointer input. */
    if (!dhcpv6_ptr)
    {

        return NX_PTR_ERROR;
    }

    /* Check for valid non pointer input. */
    if ((duid_type < NX_DHCPV6_DUID_TYPE_LINK_TIME) || (duid_type > NX_DHCPV6_DUID_TYPE_LINK_ONLY))
    {
        return NX_DHCPV6_PARAM_ERROR;
    }

    /* Ethernet is the only a hardware type supported by this API. */
    if ((hardware_type != NX_DHCPV6_CLIENT_HARDWARE_TYPE_ETHERNET) && (hardware_type != NX_DHCPV6_CLIENT_HARDWARE_TYPE_EUI_64))
    {

        /* Return any other type as an error status. */
        return NX_DHCPV6_UNSUPPORTED_DUID_HW_TYPE;
    }

    /* This DUID type is not supported by this API. */
    if (duid_type == NX_DHCPV6_DUID_TYPE_VENDOR_ASSIGNED)
    {

        /* Return as an error status. */
        return NX_DHCPV6_UNSUPPORTED_DUID_TYPE;
    }

    /* Call the actual service. */
    status = _nx_dhcpv6_create_client_duid(dhcpv6_ptr, duid_type, hardware_type, time);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_create_client_duid                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function updates the DHCPv6 client DHCP Unique Identifier      */
/*    (DUID) with the input values, sets the linklocal address while the  */
/*    Client does not have an assigned global IP address, after waiting   */
/*    for the IPv6 thread task to read its MAC address.  If the caller    */
/*    does not supply a time ID, it will generate one in place for Link   */
/*    Layer Plus Time DUIDs.                                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    duid_type                         Type of DUID (link layer +/- time)*/
/*    hardware_type                     Network hardware type e.g IEEE 802*/
/*    time                              Time stamp unique DUID identifier */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                            Clear specified area of memory    */
/*    rand                              Generate random number for time ID*/
/*    tx_thread_sleep                   Relinquish thread control (sleep) */ 
/*    nxd_ipv6_linklocal_address_set    Use MAC address as temporary IP   */ 
/*                                            source address              */
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
UINT    _nx_dhcpv6_create_client_duid(NX_DHCPV6 *dhcpv6_ptr, UINT duid_type, UINT hardware_type, ULONG time)
{

NX_INTERFACE  *interface_ptr;
USHORT         option_length;


    /* Set the interface pointer.  */
    interface_ptr = &(dhcpv6_ptr -> nx_dhcpv6_ip_ptr -> nx_ip_interface[dhcpv6_ptr -> nx_dhcpv6_client_interface_index]);

    /* Initialize the DUID to null. */
    memset(&(dhcpv6_ptr -> nx_dhcpv6_client_duid), 0, sizeof(NX_DHCPV6_DUID));

    /* Assign the DUID op code. */
    dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_op_code = NX_DHCPV6_OP_CLIENT_ID;

    /* Assign field attributes depending on the DUID type. */
    if ((duid_type == NX_DHCPV6_DUID_TYPE_LINK_TIME) || (duid_type == NX_DHCPV6_DUID_TYPE_LINK_ONLY))
    {

        /* Set the option data length minus the option header and size of the time field. */
        dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_duid_type = (USHORT)duid_type;
        dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_hardware_type = (USHORT)hardware_type;
        option_length = 4;

        /* Is the Link Layer Time DUID type? */
        if (duid_type == NX_DHCPV6_DUID_TYPE_LINK_TIME)
        {

            /* Yes; add the size of the time field to the data length. */
            option_length = (USHORT)(option_length + 4);

            /* Did the caller supplied a time?  */
            if (!time)
            {
                /* No, create a time field using the defined time constant plus a randomizing factor.*/
                dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_duid_time = (SECONDS_SINCE_JAN_1_2000_MOD_32 + (ULONG)(NX_RAND())) & 0xFFFFFFFF;
            }
            else
            {
                /* Apply the input time. */
                dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_duid_time = time;
            }
        }

        /* Set the globally unique link layer address. */
        dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_msw = interface_ptr -> nx_interface_physical_address_msw;
        dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_lsw = interface_ptr -> nx_interface_physical_address_lsw;

        /* Check the hardware type.  */
        if (hardware_type == NX_DHCPV6_CLIENT_HARDWARE_TYPE_ETHERNET)
        {

            /* The length of link layer address is 48 bits.  */
            option_length = (USHORT)(option_length + 6);
        }
        else if (hardware_type == NX_DHCPV6_CLIENT_HARDWARE_TYPE_EUI_64)
        {

            /* The length of link layer address is 64 bits.  */
            option_length = (USHORT)(option_length + 8);
        }

        /* Set the option length.  */
        dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_option_length = option_length;
    }

     return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_add_client_ia                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the add Client IA          */
/*   (Identity Association) option service, including checking for invalid*/
/*    preferred and valid lifetimes.                                      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    ipv6_address                      Requested IPv6 address            */
/*    preferred_lifetime                Requested IP lease time before    */
/*                                           address is deprecated        */
/*    valid_lifetime                    Requested IP lease time before    */
/*                                           address is expired           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */ 
/*    NX_DHCPV6_PARAM_ERROR             Invalid non pointer input         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_client_ia          Actual add client IA service      */ 
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
UINT    _nxe_dhcpv6_add_client_ia(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *ipv6_address, 
                                  ULONG preferred_lifetime, ULONG valid_lifetime) 
{

UINT status;


    /* Check for invalid pointer input. */
    if (!dhcpv6_ptr || !ipv6_address)
    {

        return NX_PTR_ERROR;
    }
    
    /* Make sure the IPv6 address valid. */
    if ((ipv6_address -> nxd_ip_address.v6[0] == 0) &&
        (ipv6_address -> nxd_ip_address.v6[1] == 0) &&
        (ipv6_address -> nxd_ip_address.v6[2] == 0) &&
        (ipv6_address -> nxd_ip_address.v6[3] == 0))
    {

        return NX_DHCPV6_INVALID_IA_ADDRESS;
    }

    /* Check for valid non pointer input. */
    if (preferred_lifetime > valid_lifetime)
    {

       /* Client preferred time must be less than valid time or 
          server will reject it. */
        return NX_DHCPV6_PARAM_ERROR;
    }

    /* Check for an empty IANA instance. */
    if (!dhcpv6_ptr -> nx_dhcpv6_iana.nx_op_code) 
    {

        return NX_DHCPV6_PARAM_ERROR;
    }

    /* Call the actual service. */
    status = _nx_dhcpv6_add_client_ia(dhcpv6_ptr, ipv6_address, preferred_lifetime, valid_lifetime); 

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_client_ia                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function adds the Client Identity Association address with     */
/*    the input values.                                                   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    ipv6_address                      Requested IPv6 address            */
/*    preferred_lifetime                Requested IP lease time before    */
/*                                           address is deprecated        */
/*    valid_lifetime                    Requested IP lease time before    */
/*                                           address is expired           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_REACHED_MAX_IA_ADDRESS  No space to record IA address     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                            Clear specified area of memory    */
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
UINT    _nx_dhcpv6_add_client_ia(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *ipv6_address, 
                                 ULONG preferred_lifetime, ULONG valid_lifetime) 
{
UINT    ia_index;


    /* Perform duplicate address detection.  */
    for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
    {
        if((dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[0] == ipv6_address->nxd_ip_address.v6[0]) && 
           (dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[1] == ipv6_address->nxd_ip_address.v6[1]) &&
           (dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[2] == ipv6_address->nxd_ip_address.v6[2]) &&
           (dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[3] == ipv6_address->nxd_ip_address.v6[3]))
        {

            /* The IA address has already exists.  */
            return (NX_DHCPV6_IA_ADDRESS_ALREADY_EXIST);
        }
    }

    /* Find one valid space to set the IA address.  */
    for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
    {

        if(!dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status)
        {

            /* Yes, got it.  */
            break;
        }
    }

    if(ia_index < NX_DHCPV6_MAX_IA_ADDRESS)
    {

        /* Yes, get it, set the IA address.  */

        /* Initialize the IA-Address to null. */
        memset(&(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index]), 0, sizeof(NX_DHCPV6_IA_ADDRESS));

        dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_op_code = NX_DHCPV6_OP_IA_ADDRESS;

        /* Set the IA option length - minus the option header. */
        dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_option_length = (7 * sizeof(ULONG)) - 4;

        /* Set the IP address parameters as a hint to the server. */
        dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_version = NX_IP_VERSION_V6;
        COPY_IPV6_ADDRESS(&(ipv6_address -> nxd_ip_address.v6[0]), &(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[0])); 

        /* Set the life time parameters as a hint to the server. */
        dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_preferred_lifetime = preferred_lifetime;
        dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_valid_lifetime = valid_lifetime;
        dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status = NX_DHCPV6_IA_ADDRESS_STATE_INITIAL;

        return NX_SUCCESS;
    }
    else
    {
        return NX_DHCPV6_REACHED_MAX_IA_ADDRESS;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_create_client_iana                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the create Client IA-NA    */
/*    option service, including checking for invalid T1 and T2 times.     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    IA_ident                          Interface Association ID          */
/*    T1                                Requested lease time with server  */
/*                                           assigning IP (renew)         */
/*    T2                                Requested lease time with any     */
/*                                           other server (rebind)        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */ 
/*    NX_DHCPV6_PARAM_ERROR             Invalid non pointer input         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_create_client_iana     Actual create client IANA service */ 
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
UINT    _nxe_dhcpv6_create_client_iana(NX_DHCPV6 *dhcpv6_ptr, UINT IA_ident, ULONG T1, ULONG T2) 
{

UINT status;


    /* Check for invalid pointer input. */
    if (!dhcpv6_ptr || !IA_ident)
    {

        return NX_PTR_ERROR;
    }

    /* Check for valid non pointer input. */
    if (T1 > T2)
    {

       /* Client preferred time must be less than valid time or server will reject it. */
        return NX_DHCPV6_PARAM_ERROR;
    }


    /* Call the actual service. */
    status = _nx_dhcpv6_create_client_iana(dhcpv6_ptr, IA_ident, T1,  T2); 

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_create_client_iana                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function updates the Client's Identity Association for Non     */
/*    Temporary Address (IANA) with the input values.                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    IA_ident                          Interface Association ID          */
/*    T1                                Requested lease time with server  */
/*                                           assigning IP (renew)         */
/*    T2                                Requested lease time with any     */
/*                                           other server (rebind)        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memset                            Clear specified area of memory    */
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
UINT    _nx_dhcpv6_create_client_iana(NX_DHCPV6 *dhcpv6_ptr, UINT IA_ident, ULONG T1, ULONG T2) 
{

    /* Initialize the IANA to null. */
    memset(&(dhcpv6_ptr -> nx_dhcpv6_iana), 0, sizeof(NX_DHCPV6_IA_NA));

    /* Assign the IANA attributes. */
    dhcpv6_ptr -> nx_dhcpv6_iana.nx_op_code = NX_DHCPV6_OP_IA_NA;
    dhcpv6_ptr -> nx_dhcpv6_iana.nx_IA_NA_id = IA_ident;

    /* Set the IANA option data length, minus the option header. */    
    dhcpv6_ptr -> nx_dhcpv6_iana.nx_option_length = 3 * sizeof(ULONG);  

    /* Apply the Client's preference for IP renew/rebind times. */
    dhcpv6_ptr -> nx_dhcpv6_iana.nx_T1 = T1;
    dhcpv6_ptr -> nx_dhcpv6_iana.nx_T2 = T2;

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_get_client_duid_time_id                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the get Client DUID time ID*/
/*    service.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    time_id                             Pointer to DUID time field      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Actual completion status        */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_client_duid_time_id  Actual get Client DUID time ID  */
/*                                          service                       */ 
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
UINT _nxe_dhcpv6_get_client_duid_time_id(NX_DHCPV6 *dhcpv6_ptr, ULONG *time_id)
{

UINT status;


    /* Check for valid pointer input. */
    if (!dhcpv6_ptr || !time_id)
    {

        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status =  _nx_dhcpv6_get_client_duid_time_id(dhcpv6_ptr, time_id);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_client_duid_time_id                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the Client DUID time ID saved to the client */
/*    record.  It does not check if the DUID is properly filled out. It is*/
/*    up to the caller to check for a zero value (invalid) time ID.       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    time_id                             Pointer to DUID time field      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Actual completion status        */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_client_duid_time_id  Actual get Client DUID time ID  */
/*                                          service                       */ 
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
UINT   _nx_dhcpv6_get_client_duid_time_id(NX_DHCPV6 *dhcpv6_ptr, ULONG *time_id)
{
         
    /* The Time field is part of the NX_DHCPV6_DUID struct in the Client record. */
    *time_id = dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_duid_time;

    /* Return successful completion. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_register_IP_address                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function registers the assigned IP address to the IP task,     */
/*    which adds the input address to the IP address table.In either case,*/
/*    the address index in the IP address table is recorded to the Client */
/*    record.It will require it if the DHCPv6 Client wants to clear the   */
/*    IPv6 address.                                                       */
/*                                                                        */
/*    Note: the host application should perform address verification      */
/*    e.g. Duplicate Address Detection to verify uniqueness of the input  */
/*    address. If duplicate address detection is enabled in NetX Duo this */
/*    is done automatically.                                              */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Actual completion status        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxd_ipv6_address_set               Add global address to IP        */
/*    tx_mutex_get                        Get the DHCPv6 mutex            */
/*    tx_mutex_put                        Put the DHCPv6 mutex            */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    nx_dhcpv6_process                   Process received packets        */ 
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
UINT _nx_dhcpv6_register_IP_address(NX_DHCPV6 *dhcpv6_ptr)
{

UINT    status;
UINT    address_index;
UINT    ia_index;

    /* Set the DHCPv6 Client IPv6 addresses.  */
    for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
    {

        if(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status == NX_DHCPV6_IA_ADDRESS_STATE_INITIAL)
        {

            /* Add the address to the IP instance address table. */
            status = nxd_ipv6_address_set(dhcpv6_ptr -> nx_dhcpv6_ip_ptr, dhcpv6_ptr -> nx_dhcpv6_client_interface_index, 
                                          &dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address, 64, &address_index);

            /* Check for successful result. */
            if (status == NX_SUCCESS)
            {

                /* Get the mutex protection for DHCP client. */
                tx_mutex_get(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex), NX_DHCPV6_MUTEX_WAIT);

#if !defined (NX_DISABLE_IPV6_DAD) && defined (NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY)

                /* Set the IPv6 address status as DAD tentative, the status will be updated in _nx_dhcpv6_ipv6_address_DAD_notify.  */
                dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status = NX_DHCPV6_IA_ADDRESS_STATE_DAD_TENTATIVE;
#else

                /* Set the IPv6 address status as valid.  */
                dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status = NX_DHCPV6_IA_ADDRESS_STATE_VALID;
#endif

                /* Record the address index in the Client record. */ 
                dhcpv6_ptr -> nx_dhcpv6_client_address_index[ia_index] = address_index;

                /* All done! Release the mutex.*/
                tx_mutex_put(&dhcpv6_ptr -> nx_dhcpv6_client_mutex);
            }

        }
    }

    /* Add the default router.  */
    nxd_ipv6_default_router_add(dhcpv6_ptr -> nx_dhcpv6_ip_ptr, &(dhcpv6_ptr-> nx_dhcpv6_server_address),
                                NX_WAIT_FOREVER, dhcpv6_ptr -> nx_dhcpv6_client_interface_index);

    return NX_SUCCESS;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_get_IP_address                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the get Client global IP   */
/*    address service.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    ip_address                          Pointer to global IP address    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_IP_address         Actual get Client global IP       */
/*                                         address service                */ 
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
UINT _nxe_dhcpv6_get_IP_address(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *ip_address)
{

UINT status;


    /* Check for valid pointer input. */
    if (!dhcpv6_ptr || !ip_address)
    {

        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status =  _nx_dhcpv6_get_IP_address(dhcpv6_ptr,  ip_address);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_IP_address                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the IP address from the DHCPv6              */
/*    Client record.  If the client address is not valid, a null IP       */
/*    address is returned and an error status is set.                     */
/*                                                                        */ 
/*    Note that if the Client has received an IP address from the DHCPv6  */
/*    server the host must register that ddress with NetX Duo by calling  */
/*    address with NetX Duo by calling the service nxd_ipv6_address_set.  */
/*    Once the address is registered, the host can confirm its address by */
/*    calling nxd_ipv6_address_get.                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    ip_address                          Pointer to NetX Duo address     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Returned non zero IP address    */
/*    NX_DHCPV6_IA_ADDRESS_NOT_VALID      Client not assigned an address  */
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*     None                                                               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT _nx_dhcpv6_get_IP_address(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *ip_address)
{

UINT status;


    /* Clear the address buffer. */
    SET_UNSPECIFIED_ADDRESS(&ip_address -> nxd_ip_address.v6[0]);

    /* Initialize the outcome as no address assigned. */  
    status = NX_DHCPV6_IA_ADDRESS_NOT_VALID;

    /* Is the client assigned a valid address? */ 
    if (dhcpv6_ptr -> nx_dhcpv6_ia[0].nx_address_status == NX_DHCPV6_IA_ADDRESS_STATE_VALID)
    {  
        /* Yes; return the assigned IP address from the DHCPv6 Client record. */
        COPY_IPV6_ADDRESS(&(dhcpv6_ptr -> nx_dhcpv6_ia[0].nx_global_address.nxd_ip_address.v6[0]), 
                          &(ip_address -> nxd_ip_address.v6[0]));

        status = NX_SUCCESS;
    }    

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_get_lease_time_data                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the get Client IP lease    */
/*    time service.                                                       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    T1                                  Pointer to T1 time              */
/*    T2                                  Pointer to T2 time              */
/*    preferred_lifetime                  Pointer to preferred lifetime   */
/*    valid_lifetime                      Pointer to valid lifetime       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_lease_time_data    Actual get Client IP lease time   */
/*                                                 service                */ 
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
UINT _nxe_dhcpv6_get_lease_time_data(NX_DHCPV6 *dhcpv6_ptr, ULONG *T1, ULONG *T2, ULONG *preferred_lifetime, 
                                    ULONG *valid_lifetime)
{

UINT status;


    /* Check for valid pointer input. */
    if (!dhcpv6_ptr || !T1 || !T2 || !preferred_lifetime || !valid_lifetime )
    {
        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status =  _nx_dhcpv6_get_lease_time_data(dhcpv6_ptr, T1, T2, preferred_lifetime, valid_lifetime);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_lease_time_data                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the Client's global IP lease times T1 and T2*/
/*    and valid and preferred lifetimes. The address_status input field   */
/*    indicates if the IP address on the Client record is valid and       */
/*    registered. If so the lease and valid lifetimes are also registered */
/*    with the server (not merely Client preference or left over from an  */
/*    expired IP address).                                                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    T1                                  Pointer to T1 time              */
/*    T2                                  Pointer to T2 time              */
/*    preferred_lifetime                  Pointer to preferred lifetime   */
/*    valid_lifetime                      Pointer to valid lifetime       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*     None                                                               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT _nx_dhcpv6_get_lease_time_data(NX_DHCPV6 *dhcpv6_ptr, ULONG *T1, ULONG *T2, ULONG *preferred_lifetime, 
                                    ULONG *valid_lifetime)
{

    /* Initialize data to zero. */
    *T1 = 0;
    *T2 = 0;
    *valid_lifetime = 0;
    *preferred_lifetime = 0;

    /* If the client has a valid IP address status, return the actual lease time data. */
    if (dhcpv6_ptr -> nx_dhcpv6_ia[0].nx_address_status == NX_DHCPV6_IA_ADDRESS_STATE_VALID)
    {
        /* Get the time data from the Client record IA. */
        *T1 = dhcpv6_ptr -> nx_dhcpv6_iana.nx_T1;
        *T2 = dhcpv6_ptr -> nx_dhcpv6_iana.nx_T2;
        *preferred_lifetime = dhcpv6_ptr -> nx_dhcpv6_ia[0].nx_preferred_lifetime;
        *valid_lifetime = dhcpv6_ptr -> nx_dhcpv6_ia[0].nx_valid_lifetime;
    }

    /* Return successful completion. */
    return NX_SUCCESS;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_get_DNS_server_address                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    index                               Index into DNS server list      */
/*    server_address                      Pointer to copy address to      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*    _nx_dhcpv6_get_DNS_server_address   Actual get DNS server address   */ 
/*                                              service                   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nxe_dhcpv6_get_DNS_server_address(NX_DHCPV6 *dhcpv6_ptr, UINT index, NXD_ADDRESS *server_address)
{

UINT status;


    /* Check for valid pointer input. */
    if (dhcpv6_ptr == NX_NULL)
    {
        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */    
    status =  _nx_dhcpv6_get_DNS_server_address(dhcpv6_ptr, index, server_address);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_DNS_server_address                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the DNS server address at the specified     */
/*    index in the DHCP Client DNS list.  The Client DNS server list is   */
/*    compiled from DHCP Server messages supplying DNS server address     */
/*    data.                                                               */ 
/*                                                                        */ 
/*    Note: the DHCPv6 Client is configured to store up to                */
/*    NX_DHCPV6_NUM_DNS_SERVERS DNS server addresses. It is up to the     */
/*    caller to supply the index to get each server in the list if there  */
/*    is more than one stored.                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    index                               Index into DNS server list      */
/*    server_address                      Pointer to copy address to      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */
/*    NX_DHCPV6_PARAM_ERROR               Invalid index supplied          */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_dhcpv6_get_DNS_server_address  Actual get DNS server address    */
/*                                            service                     */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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

UINT  _nx_dhcpv6_get_DNS_server_address(NX_DHCPV6 *dhcpv6_ptr, UINT index, NXD_ADDRESS *server_address)
{


    /* Check for a valid input. */
    if (index >= NX_DHCPV6_NUM_DNS_SERVERS)
    {

        return NX_DHCPV6_PARAM_ERROR;
    }

    /* Store the specified DNS server addresses in the DHCPv6 Client list. */
    COPY_NXD_ADDRESS(&(dhcpv6_ptr -> nx_dhcpv6_DNS_name_server_address[index]), server_address);

    return NX_SUCCESS;
}

 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_get_time_server_address                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    index                               Index into time server list     */
/*    server_address                      Pointer to copy address to      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*    _nx_dhcpv6_get_time_server_address   Actual get time server address */ 
/*                                              service                   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nxe_dhcpv6_get_time_server_address(NX_DHCPV6 *dhcpv6_ptr, UINT index, NXD_ADDRESS *server_address)
{

UINT status;


    /* Check for valid pointer input. */
    if (dhcpv6_ptr == NX_NULL)
    {
        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */    
    status =  _nx_dhcpv6_get_time_server_address(dhcpv6_ptr, index, server_address);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_time_server_address                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the time server address at the specified    */
/*    index in the DHCP Client time list. The Client time server list is  */
/*    compiled from DHCP Server messages supplying time server address    */
/*    data.                                                               */ 
/*                                                                        */ 
/*    Note: the DHCPv6 Client is configured to store up to                */
/*    NX_DHCPV6_NUM_TIME_SERVERS TIME server addresses. It is up to the   */
/*    caller to supply the index to get each server in the list if there  */
/*    is more than one stored.                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    index                               Index into time server list     */
/*    server_address                      Pointer to copy address to      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */
/*    NX_DHCPV6_PARAM_ERROR               Invalid index supplied          */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_dhcpv6_get_time_server_address  Actual get time server address  */
/*                                            service                     */ 
/*                                                                        */
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nx_dhcpv6_get_time_server_address(NX_DHCPV6 *dhcpv6_ptr, UINT index, NXD_ADDRESS *server_address)
{


    /* Check for a valid input. */
    if (index >= NX_DHCPV6_NUM_TIME_SERVERS)
    {

        return NX_DHCPV6_PARAM_ERROR;
    }

    /* Store the specified time server addresses in the DHCPv6 Client list. */
    COPY_NXD_ADDRESS(&(dhcpv6_ptr -> nx_dhcpv6_time_server_address[index]), server_address);

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_get_other_option_data                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    option_code                         Option code of requested data   */
/*    buffer                              Buffer to copy data to          */
/*    buffer_length                       Size of buffer to copy to       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*    NX_DHCPV6_PARAM_ERROR               Invalid non pointer input       */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*     _nx_dhcpv6_get_other_option_data   Actual get other option data    */ 
/*                                              service                   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT  _nxe_dhcpv6_get_other_option_data(NX_DHCPV6 *dhcpv6_ptr, UINT option_code, UCHAR *buffer,UINT buffer_length)
{

UINT status;


    /* Check for valid pointer input. */
    if (!dhcpv6_ptr || !buffer)
    {
        return NX_PTR_ERROR;
    }

    /* Check for invalid non pointer input. */
    if (!option_code || !buffer_length)
    {

        return NX_DHCPV6_PARAM_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */    
    status =  _nx_dhcpv6_get_other_option_data(dhcpv6_ptr, option_code, buffer, buffer_length);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_other_option_data                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the option information requested in the     */
/*    input parameter. Note that there are only 2 options  which the      */
/*    NetX DHCPv6 client supports. Some options have their own specific   */ 
/*    API such as _nx_dhcpv6_get_DNS_server_address.  The complete list   */
/*    of options is not supported by the NetX Duo DHCPv6 Client.          */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    option_code                         Option code of requested data   */
/*    buffer                              Buffer to copy data to          */
/*    buffer_length                       Size of buffer to copy to       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_DHCPV6_UNKNOWN_OPTION            Unknown option specified        */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*     memcpy                          Copy specified area of memory      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcpv6_get_other_option_data(NX_DHCPV6 *dhcpv6_ptr, UINT option_code, UCHAR *buffer, UINT buffer_length)
{


UINT length = 0;

    /* Check to see which option data is requested: */
    switch (option_code)
    {

        case NX_DHCPV6_NEW_POSIX_TIMEZONE_OPTION:    

            /* Update the amount of data we are copying into the buffer. */
            length += NX_DHCPV6_TIME_ZONE_BUFFER_SIZE;

            /* Check that we don't overrun the buffer. */
            if (length > buffer_length)
            {
                return NX_DHCPV6_INVALID_DATA_SIZE;
            }

            /* Copy the time zone data to the supplied buffer. */
            memcpy(buffer, dhcpv6_ptr -> nx_dhcpv6_time_zone, NX_DHCPV6_TIME_ZONE_BUFFER_SIZE);  /* Use case of memcpy is verified. */

            break;


        case NX_DHCPV6_DOMAIN_NAME_OPTION:

            /* Update the amount of data we are copying into the buffer. */
            length +=  NX_DHCPV6_DOMAIN_NAME_BUFFER_SIZE;

            /* Check that we don't overrun the buffer. */
            if (length > buffer_length)
            {
                return NX_DHCPV6_INVALID_DATA_SIZE;
            }

            /* Copy the domain name data to the supplied buffer. */
            memcpy(buffer, dhcpv6_ptr -> nx_dhcpv6_domain_name, NX_DHCPV6_DOMAIN_NAME_BUFFER_SIZE); /* Use case of memcpy is verified. */
            break;

        /* Unknown or unsupported option*/
        default:

            return NX_DHCPV6_UNKNOWN_OPTION;
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_get_time_accrued                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the get time accrued on    */
/*    Client IP lease service.                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    time_accrued                        Pointer to time since IP lease  */
/*                                            assigned (secs)             */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_time_accrued       Actual get time accrued on Client */
/*                                          IP address lease service      */ 
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
UINT _nxe_dhcpv6_get_time_accrued(NX_DHCPV6 *dhcpv6_ptr, ULONG *time_accrued)
{

UINT status;


    /* Check for valid pointer input. */
    if (!dhcpv6_ptr || !time_accrued)
    {
        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status =  _nx_dhcpv6_get_time_accrued(dhcpv6_ptr, time_accrued);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_time_accrued                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the time accrued on the assigned global IP  */
/*    address lease.  IP address to verify the address is still valid.    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    time_accrued                        Pointer to time since IP lease  */
/*                                            assigned (secs)             */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*     tx_mutex_get                      Obtain exclusive lock on Client  */
/*                                         time accrued variable          */ 
/*     tx_mutex_put                      Release exclusive lock in Client */
/*                                         time variable                  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT _nx_dhcpv6_get_time_accrued(NX_DHCPV6 *dhcpv6_ptr, ULONG *time_accrued)
{    
UINT    ia_index;
UINT    found_ia = 0;

    /* Initialize the time accrued to zero. */
    *time_accrued = 0;

    /* Check if the client currently has a valid IP address assigned. */
    for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
    {

        if(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status == NX_DHCPV6_IA_ADDRESS_STATE_VALID)
        {

            /* Yes, got it.  */
            found_ia= 1;
            break;
        }
    }

    if(found_ia == 1)
    {

        /* Get the time accrued on the IP address lease. */
        *time_accrued = dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued;
    }

    /* Return successful completion. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_get_iana_lease_time                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the get T1 and T2 value    */
/*    of IANA option.                                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    T1                                  Pointer to T1 time              */
/*    T2                                  Pointer to T2 time              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_iana_lease_time    Actual get Client IP T1/T2 time   */
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
UINT _nxe_dhcpv6_get_iana_lease_time(NX_DHCPV6 *dhcpv6_ptr, ULONG *T1, ULONG *T2)
{

UINT status;


    /* Check for valid pointer input. */
    if (!dhcpv6_ptr || !T1 || !T2)
    {
        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status =  _nx_dhcpv6_get_iana_lease_time(dhcpv6_ptr, T1, T2);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_iana_lease_time                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the T1 and T2 of Client's global IP.        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    T1                                  Pointer to T1 time              */
/*    T2                                  Pointer to T2 time              */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*     None                                                               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT _nx_dhcpv6_get_iana_lease_time(NX_DHCPV6 *dhcpv6_ptr, ULONG *T1, ULONG *T2)
{    
UINT    ia_index;
UINT    found_ia = 0;

    /* Initialize data to zero. */
    *T1 = 0;
    *T2 = 0;
    
    /* Check if the client currently has a valid IP address assigned. */
    for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
    {

        if(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status == NX_DHCPV6_IA_ADDRESS_STATE_VALID)
        {

            /* Yes, got it.  */
            found_ia= 1;
            break;
        }
    }

    if(found_ia == 1)
    {

        /* Get the time data from the Client record IA. */
        *T1 = dhcpv6_ptr -> nx_dhcpv6_iana.nx_T1;
        *T2 = dhcpv6_ptr -> nx_dhcpv6_iana.nx_T2;
    }

    /* Return successful completion. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_get_valid_ip_address_count              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the get valid IP address   */
/*    count.                                                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPv6 Client          */ 
/*    address_count                     The count of valid IP address     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_valid_ip_address_count                               */
/*                                      Actual get the valid IP count     */ 
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
UINT _nxe_dhcpv6_get_valid_ip_address_count(NX_DHCPV6 *dhcpv6_ptr, UINT *address_count)
{

UINT status;


    /* Check for valid pointer input. */
    if (!dhcpv6_ptr || !address_count)
    {
        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status =  _nx_dhcpv6_get_valid_ip_address_count(dhcpv6_ptr, address_count);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_valid_ip_address_count               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the Client's global IP, then record the     */
/*    count of valid IP address                                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    address_count                       The count of valid IP address   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*     None                                                               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT _nx_dhcpv6_get_valid_ip_address_count(NX_DHCPV6 *dhcpv6_ptr, UINT *address_count)
{    

UINT    ia_index;

    /* Initialize data to zero. */
    *address_count = 0;
    
    /* Check if the client currently has a valid IP address assigned. */
    for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
    {

        if(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status == NX_DHCPV6_IA_ADDRESS_STATE_VALID)
        {

            /* Yes, get one, update the count. */
            (*address_count)++;
        }
    }

    /* Return successful completion. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_get_valid_ip_address_lease_time         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the get valid IP address,  */
/*    the preferred lifetime and the valid lifetime by the address index. */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    address_index                       The valid address index         */
/*    ip_address                          Pointer to IP address           */
/*    preferred_lifetime                  Pointer to preferred lifetime   */
/*    valid_lifetime                      Pointer to valid lifetime       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_lvalid_ip_address_lease_time                         */
/*                                      Actual get Client IP valid address*/ 
/*                                        and the lease time of IP        */ 
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
UINT _nxe_dhcpv6_get_valid_ip_address_lease_time(NX_DHCPV6 *dhcpv6_ptr, UINT address_index, NXD_ADDRESS *ip_address,
                                                 ULONG *preferred_lifetime, ULONG *valid_lifetime)
{

UINT status;


    /* Check for valid pointer input. */
    if (!dhcpv6_ptr || !ip_address || !preferred_lifetime || !valid_lifetime )
    {
        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status =  _nx_dhcpv6_get_valid_ip_address_lease_time(dhcpv6_ptr, address_index, ip_address, preferred_lifetime, valid_lifetime);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_get_valid_ip_address_lease_time          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function retrieves the Client's valid global IP addresses,     */ 
/*    preferred lifetime and valid lifetime by address index.             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    address_index                       The valid address index         */
/*    ip_address                          Pointer to IPv6 address         */
/*    preferred_lifetime                  Pointer to preferred lifetime   */
/*    valid_lifetime                      Pointer to valid lifetime       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*     None                                                               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT _nx_dhcpv6_get_valid_ip_address_lease_time(NX_DHCPV6 *dhcpv6_ptr, UINT address_index, NXD_ADDRESS *ip_address,
                                                ULONG *preferred_lifetime, ULONG *valid_lifetime)
{
    
UINT    status;
UINT    ia_index;
UINT    valid_ia_count;


    /* Initialize data to zero. */
    valid_ia_count = 0;
    *valid_lifetime = 0;
    *preferred_lifetime = 0;
    status = NX_DHCPV6_IA_ADDRESS_NOT_VALID;
    
    /* Clear the address buffer. */
    SET_UNSPECIFIED_ADDRESS(&ip_address -> nxd_ip_address.v6[0]);        

    /* Check if the client currently has a valid IP address assigned. */
    for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
    {

        /* Skip empty or invalidated addresses. */
        if(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status == NX_DHCPV6_IA_ADDRESS_STATE_VALID)
        {

            if((valid_ia_count++) == address_index)
            {

                /* Yes, got it.  */
                break;
            }
        }
    }

    if(ia_index < NX_DHCPV6_MAX_IA_ADDRESS)
    {

        /* Yes; return the assigned IP address from the DHCPv6 Client record. */
        COPY_IPV6_ADDRESS(&(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[0]), 
                          &(ip_address -> nxd_ip_address.v6[0]));
        
        /* Set the IP version.  */
        ip_address -> nxd_ip_version = NX_IP_VERSION_V6;

        /* Return the preferred lifetime and valid lifetime.  */
        *preferred_lifetime = dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_preferred_lifetime;
        *valid_lifetime = dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_valid_lifetime;

        status = NX_SUCCESS;
    } 

    /* Return. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_IP_lifetime_timeout_entry                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is scheduled by the ThreadX scheduler on a user       */
/*    configurable time interval.  It updates the time accrued since the  */
/*    Client was assigned its global IP address so that the DHCP Client   */
/*    knows when to renew or rebind its IP address.                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr_value                      Pointer to the DHCPV6 Client  */ 
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
VOID  _nx_dhcpv6_IP_lifetime_timeout_entry(ULONG dhcpv6_ptr_value)
{

NX_DHCPV6 *dhcpv6_ptr;


    /* Setup DHCPv6 Client pointer.  */
    NX_TIMER_EXTENSION_PTR_GET(dhcpv6_ptr, NX_DHCPV6, dhcpv6_ptr_value)

    /* Check if the DHCPv6 Client has valid IP address assigned and a finite IP
       address lease (no point in waiting for an infinite lease to time out!) */
    if ((dhcpv6_ptr -> nx_dhcpv6_iana.nx_T1 != NX_DHCPV6_INFINITE_LEASE) &&
        (dhcpv6_ptr -> nx_dhcpv6_iana.nx_T1 != 0))
    {

        /* Update the time remaining on the Client IP address lease. 
           The current time expressed in units of seconds. */
        dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued += NX_DHCPV6_IP_LIFETIME_TIMER_INTERVAL;
    }

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function executes the DHCPV6 Client request.  It compiles      */
/*    and sends the DHCPv6 request message, and updates the Client record */
/*    with server data.  It maintains the DHCPv6 Client state and calls   */
/*    the state change callback if one is registered with the DHCPv6      */
/*    Client.                                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*     None                                                               */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_send_request           Send the Client DHCPv6 request    */ 
/*    _nx_dhcpv6_waiting_on_reply       Wait for valid DHCPv6 server reply*/
/*    _nx_dhcpv6_request_decline        Send the DHCPv6 DECLINE request   */
/*    _nx_dhcpv6_register_IP_address    Register Client IP address        */
/*    _nx_dhcpv6_remove_assigned_address                                  */
/*                                      Remove Client global IP address   */
/*    _nx_dhcpv6_update_retransmit_info Update the retransmit information */
/*    _nx_dhcpv6_send_solicit           Send the DHCPv6 solicit message   */
/*    _nx_dhcpv6_flush_queue_packets    Flush the queue packets           */ 
/*    tx_mutex_get                      Obtain lock on Client resource    */
/*    tx_mutex_put                      Release lock on Client resourse   */
/*    tx_timer_deactivate               Deactivate the session timer      */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_thread_entry           Processing thread for DHCPv6      */ 
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
VOID  _nx_dhcpv6_process(NX_DHCPV6 *dhcpv6_ptr)
{

UINT      status;
UCHAR     original_state;

    /* Initialize the DHCPv6 Client status as no errors. */
    dhcpv6_ptr -> nx_status_code = NX_DHCPV6_SUCCESS;

    /* Clear any previous messages from the DHCPv6 server. */
    memset(dhcpv6_ptr -> nx_status_message, 0, NX_DHCPV6_MAX_MESSAGE_SIZE);

    /* Remember the original DHCPv6 state for the changed state callback function.  */
    original_state =  dhcpv6_ptr -> nx_dhcpv6_state;

    /* Process the DHCPv6 Client for the state it is in. */
    switch (dhcpv6_ptr -> nx_dhcpv6_state)
    {

        /* The DHCPv6 remains initial status or DHCPv6 interactive process has completed.
           Just clear the timer and message ID. */ 
        case NX_DHCPV6_STATE_INIT:
        case NX_DHCPV6_STATE_BOUND_TO_ADDRESS:
        {

            /* Clear the message id.  */
            dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = 0;

            /* Deactivate the session timer of elapsed time.  */
            tx_timer_deactivate(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));     

            /* Flush the queue packets.  */
            _nx_dhcpv6_flush_queue_packets(dhcpv6_ptr);

            return;
        }
        
        /* The caller has requested to send a solicit message. */
        case NX_DHCPV6_STATE_SENDING_SOLICIT:
        {

            dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type =  NX_DHCPV6_MESSAGE_TYPE_SOLICIT;

            dhcpv6_ptr -> nx_dhcpv6_solicitations_sent++;

            /* Send the DHCPV6 Solicit message.  */
            status = _nx_dhcpv6_send_request(dhcpv6_ptr);

            /* Check for error. */
            if (status != NX_SUCCESS)
            {

                /* Return. Can't do any more now. */
                return;
            }

            /* Make sure the server DUID is cleared. Client should be accepting any server reply. */
            memset(&(dhcpv6_ptr -> nx_dhcpv6_server_duid), 0, sizeof(NX_DHCPV6_DUID));

            /* Wait for a reply from the server. This will also extract the
               server packet information and update the DHCPv6 Client record (and state). */
            status = _nx_dhcpv6_waiting_on_reply(dhcpv6_ptr);

            /* Check for processing error or error code sent by server. As per RFC 3315 section 17.1.3,
               the Client ignores status errors (e.g. No Addresses Available)  from the server, and continues
               sending Solicit messages. */
            if ((status != NX_SUCCESS) || (dhcpv6_ptr -> nx_status_code != NX_DHCPV6_SUCCESS))
            {
                          
                /* Update the retransmission information. */
                if (_nx_dhcpv6_update_retransmit_info(dhcpv6_ptr))
                {
                
                    /* The retry limit on Solicit messages is exceeded. Set the CLient back to 
                       the INIT state and stop sending solicit messages for now. */
                    dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_INIT;

                    /* Request attempt failed. The state change callback if set will be called. */
                }
            }
            else
            {

                if(dhcpv6_ptr -> nx_dhcpv6_request_solicit_mode == NX_DHCPV6_SOLICIT_NORMAL)
                {
                    /* Server has responded to and accepted the SOLICIT request with an ADVERTISE message. 
                       Move to the next state to continue the DHCPv6 protocol. */

                    /* Set the DHCP Client state to Requesting.  */
                    dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_SENDING_REQUEST;

                    /* Reset the retransmission timeout and retry. */
                    dhcpv6_ptr -> nx_dhcpv6_transmission_timeout =  NX_DHCPV6_INIT_REQ_TRANSMISSION_TIMEOUT;
                    dhcpv6_ptr -> nx_dhcpv6_retransmission_count = 0;

                    /* Set the initaial elasped time of DHCPv6 message.  */    
                    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;

                    /* Set the retransmission information(IRT, MRC, MRT, MRD). */
                    dhcpv6_ptr -> nx_dhcpv6_init_retransmission_timeout = NX_DHCPV6_INIT_REQ_TRANSMISSION_TIMEOUT;
                    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_count = NX_DHCPV6_MAX_REQ_RETRANSMISSION_COUNT;
                    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_timeout = NX_DHCPV6_MAX_REQ_RETRANSMISSION_TIMEOUT;
                    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_duration = NX_DHCPV6_MAX_REQ_RETRANSMISSION_DURATION;

                    /* The next sending packet is the new packet, so reset the message id.  */
                    dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = 0;

                    /* Increment the number of solicitation responses.  */
                    dhcpv6_ptr -> nx_dhcpv6_solicitation_responses++;
                }
                else
                {
                    
                    /* Server sending the REPLY message responded to and accepted the SOLICIT request with Rapid Commit option,
                       Move to the next state to continue the DHCPv6 protocol. */
                    
                    /* Set the initaial elasped time of DHCPv6 message.  */    
                    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;
                    
                    /* Increment the number of solicitation responses.  */
                    dhcpv6_ptr -> nx_dhcpv6_solicitation_responses++;

                    /* Clear the message id.  */
                    dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = 0;

                    /* Deactivate the session timer of elapsed time.  */
                    tx_timer_deactivate(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));

                    /* Set the DHCP Client state to Requesting.  */
                    dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_BOUND_TO_ADDRESS;   

                    /* Yes, get the IPv6 address by DHCPv6. set it.  */
                    status = _nx_dhcpv6_register_IP_address(dhcpv6_ptr);

                }
            
            }

            break;
        }


        /* The DHCP Client is in the REQUEST state; send the Request message till the server responds 
           or the number of retries exceeds the max. */
        case NX_DHCPV6_STATE_SENDING_REQUEST:  
        {
        
            /* Reset the initial timeout and zero out transmission count. */
            dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type =  NX_DHCPV6_MESSAGE_TYPE_REQUEST;                

            /* Transmit the Request message.  */
            status = _nx_dhcpv6_send_request(dhcpv6_ptr); 

            /* Check for transmission error. */
            if (status != NX_SUCCESS)
            {

                /* Return. Can't do any more now. */
                return;
            }

            dhcpv6_ptr -> nx_dhcpv6_requests_sent++;

            /* Send the message to the server. */
            status = _nx_dhcpv6_waiting_on_reply(dhcpv6_ptr);

            /* Check for socket receive error or Server error code. */
            if ((status != NX_SUCCESS) || (dhcpv6_ptr -> nx_status_code != NX_DHCPV6_SUCCESS))
            {
                
                /* Check the status.  */
                if ((status == NX_SUCCESS) && 
                    ((dhcpv6_ptr -> nx_status_code == NX_DHCPV6_NOT_ON_LINK) ||
                     (dhcpv6_ptr -> nx_status_code == NX_DHCPV6_NO_ADDRESS_AVAILABLE)))
                {

                    /* Indicate the DHCP Client process is idle. */
                    dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_TRUE;

                    /* The next sending packet is the new packet, so reset the message id.  */
                    dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = 0;

                    /* Clear the assigned IP address.  */                
                    _nx_dhcpv6_remove_assigned_address(dhcpv6_ptr,NX_DHCPV6_REMOVE_ALL_IA_ADDRESS);

                    /* When the client receives a NotOnLink status from the server in response to a Request,
                       the client can restart the DHCP server discovery process. RFC3315, Section18.1.8, Page48.  */ 
                    _nx_dhcpv6_request_solicit(dhcpv6_ptr);
                }

                else
                {

                    /* Update the retransmission information. */
                    if (_nx_dhcpv6_update_retransmit_info(dhcpv6_ptr))
                    {

                        /* The retry limit on Request messages is exceeded. Set the Client back to 
                        the INIT state and stop sending request messages for now. */
                        dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_INIT;

                        /* Request attempt failed. The state change callback if set will be called. */
                    }
                }
            }
            else
            {
            
                /* The Server responded to and accepted the REQUEST message. The Client is now bound to assigned
                   IPv6 address. The Client must still register this IPv6 address with NetX Duo to verify it is
                   unique on the network.  */
        
                /* Clear the time accrued on the new IP lease to zero. */
                dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued = 0;

                /* The next sending packet is the new packet, so reset the message id.  */
                dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = 0;
    
                /* Receive the correct response, so deactivate the session timer of elapsed time. */
                tx_timer_deactivate(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));

                /* Increment the number of Request responses received.  */
                dhcpv6_ptr -> nx_dhcpv6_request_responses++;
    
                /* Indicate the Client is bound to an assigned IPv6 address. */
                dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_BOUND_TO_ADDRESS; 

                /* Yes, get the IPv6 address by DHCPv6. set it.  */
                status = _nx_dhcpv6_register_IP_address(dhcpv6_ptr);
            }

            break;
        }

        /* The caller has requested a RENEW request message be sent. */
        case NX_DHCPV6_STATE_SENDING_RENEW:
        {
        
            dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type =  NX_DHCPV6_MESSAGE_TYPE_RENEW;

            dhcpv6_ptr -> nx_dhcpv6_renews_sent++;

            /* Send the DHCPV6 RENEW message.  */
            status = _nx_dhcpv6_send_request(dhcpv6_ptr);

            /* Check for transmission error. */
            if (status != NX_SUCCESS)
            {

                /* Return. Can't do any more now. */
                return;
            }

            /* Send the message to the server. */
            status = _nx_dhcpv6_waiting_on_reply(dhcpv6_ptr);

            /* Check for socket receive error or Server error code. */
            if ((status != NX_SUCCESS) || 
                (dhcpv6_ptr -> nx_status_code != NX_DHCPV6_SUCCESS) || 
                ((dhcpv6_ptr -> nx_dhcpv6_reply_option_flags & NX_DHCPV6_INCLUDE_IA_ADDRESS_OPTION) == 0))
            {

                /* Process the NoBinding status.  */
                if ((status == NX_SUCCESS) && (dhcpv6_ptr -> nx_status_code == NX_DHCPV6_NO_BINDING))
                {

                    /* When the client receives a Reply message in response to a Renew or Rebind message,
                       send a Request message if the IA contains a Status Code option with the NoBinding status. 
                       RFC 3315, Sectoon 18.1.8, Page 48.  */
                    dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_SENDING_REQUEST;

                    /* Reset the retransmission timeout and retry. */
                    dhcpv6_ptr -> nx_dhcpv6_transmission_timeout =  NX_DHCPV6_INIT_REQ_TRANSMISSION_TIMEOUT;
                    dhcpv6_ptr -> nx_dhcpv6_retransmission_count = 0;

                    /* Set the initaial elasped time of DHCPv6 message.  */    
                    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;

                    /* Set the retransmission information(IRT, MRC, MRT, MRD). */
                    dhcpv6_ptr -> nx_dhcpv6_init_retransmission_timeout = NX_DHCPV6_INIT_REQ_TRANSMISSION_TIMEOUT;
                    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_count = NX_DHCPV6_MAX_REQ_RETRANSMISSION_COUNT;
                    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_timeout = NX_DHCPV6_MAX_REQ_RETRANSMISSION_TIMEOUT;
                    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_duration = NX_DHCPV6_MAX_REQ_RETRANSMISSION_DURATION;

                    /* The next sending packet is the new packet, so reset the message id.  */
                    dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = 0;
                }                     
                else
                {

                    /* Update the retransmission information. */
                    if (_nx_dhcpv6_update_retransmit_info(dhcpv6_ptr))
                    {

                        /* Clear the assigned IP address.  */
                        _nx_dhcpv6_remove_assigned_address(dhcpv6_ptr, NX_DHCPV6_REMOVE_ALL_IA_ADDRESS);

                        /* The retry limit on Renew messages is exceeded. Set the Client back to 
                        the INIT state and stop sending Renew messages for now. */
                        dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_INIT;

                        /* Renew attempt failed. The state change callback if set will be called. */
                    }   
                }
            }
            else
            {
            
                /* The Server responded to and accepted our RENEW request. Update the Client to a bound
                   state and reset the lease timers.  */
    
                /* Indicate the Client is bound to an assigned IPv6 address. */
                dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_BOUND_TO_ADDRESS;
    
                /* Reset the time keeper! */
                dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued = 0;
    
                /* The next sending packet is the new packet, so reset the message id.  */
                dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = 0;

                /* Receive the correct response, so deactivate the session timer of elapsed time. */
                tx_timer_deactivate(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));

                /* Yes, get the IPv6 address by DHCPv6. set it.  */
                status = _nx_dhcpv6_register_IP_address(dhcpv6_ptr);
    
                /* Increment the number of renew responses received.  */
                dhcpv6_ptr -> nx_dhcpv6_renew_responses++;
            }

            break;
        }

        /* The caller has requested to send a rebind request message. */
        case NX_DHCPV6_STATE_SENDING_REBIND:
        {
        
            dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type =  NX_DHCPV6_MESSAGE_TYPE_REBIND;

            /* Increase the count of retries. */
            dhcpv6_ptr -> nx_dhcpv6_rebinds_sent++;

            /* Send the DHCPV6 Rebind message.  */
            status = _nx_dhcpv6_send_request(dhcpv6_ptr);

            /* Check for socket send error. */
            if (status != NX_SUCCESS)
            {

                /* Return. Can't do any more now. */
                return;
            }

            /* Make sure the server DUID is cleared. Client should be accepting any server reply. */
            memset(&(dhcpv6_ptr -> nx_dhcpv6_server_duid), 0, sizeof(NX_DHCPV6_DUID));

            /* Send the message to the server. */
            status = _nx_dhcpv6_waiting_on_reply(dhcpv6_ptr);

            /* Check for socket receive error or Server error code. */
            if ((status != NX_SUCCESS) || 
                (dhcpv6_ptr -> nx_status_code != NX_DHCPV6_SUCCESS) || 
                ((dhcpv6_ptr -> nx_dhcpv6_reply_option_flags & NX_DHCPV6_INCLUDE_IA_ADDRESS_OPTION) == 0))
            {
               
                /* Process the NoBinding status.  */
                if ((status == NX_SUCCESS) && (dhcpv6_ptr -> nx_status_code == NX_DHCPV6_NO_BINDING))
                {

                    /* When the client receives a Reply message in response to a Renew or Rebind message,
                       send a Request message if the IA contains a Status Code option with the NoBinding status. 
                       RFC 3315, Section 18.1.8, Page 48.  */
                    dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_SENDING_REQUEST;

                    /* Reset the retransmission timeout and retry. */
                    dhcpv6_ptr -> nx_dhcpv6_transmission_timeout =  NX_DHCPV6_INIT_REQ_TRANSMISSION_TIMEOUT;
                    dhcpv6_ptr -> nx_dhcpv6_retransmission_count = 0;

                    /* Set the initaial elasped time of DHCPv6 message.  */    
                    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;

                    /* Set the retransmission information(IRT, MRC, MRT, MRD). */
                    dhcpv6_ptr -> nx_dhcpv6_init_retransmission_timeout = NX_DHCPV6_INIT_REQ_TRANSMISSION_TIMEOUT;
                    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_count = NX_DHCPV6_MAX_REQ_RETRANSMISSION_COUNT;
                    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_timeout = NX_DHCPV6_MAX_REQ_RETRANSMISSION_TIMEOUT;
                    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_duration = NX_DHCPV6_MAX_REQ_RETRANSMISSION_DURATION;

                    /* The next sending packet is the new packet, so reset the message id.  */
                    dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = 0;
                }                     
                else
                {

                    /* Update the retransmission information. */
                    if (_nx_dhcpv6_update_retransmit_info(dhcpv6_ptr))
                    {

                        /* Clear the assigned IP address.  */
                        _nx_dhcpv6_remove_assigned_address(dhcpv6_ptr, NX_DHCPV6_REMOVE_ALL_IA_ADDRESS);

                        /* The retry limit on Rebind messages is exceeded. Set the Client back to 
                        the INIT state and stop sending Rebind messages for now. */
                        dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_INIT;

                        /* Rebind attempt failed. The client state change callback if set will be called. */
                    }   
                }
            }
            else
            {
            
                /* Make sure the Client state is set to Bound to an assigned IPv6 address. */
                dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_BOUND_TO_ADDRESS;

                /* Reset the time keeper! */
                dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued = 0;
    
                /* The next sending packet is the new packet, so reset the message id.  */
                dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = 0;       

                /* Yes, get the IPv6 address by DHCPv6. set it.  */
                status = _nx_dhcpv6_register_IP_address(dhcpv6_ptr);
    
                /* Increment the number of rebind responses received.  */
                dhcpv6_ptr -> nx_dhcpv6_rebind_responses++;
                
                /* Receive the correct response, so deactivate the session timer of elapsed time. */
                tx_timer_deactivate(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));
            }

            break;
        }

        case NX_DHCPV6_STATE_SENDING_CONFIRM:
        {

            /* Set the message type.  */
            dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type =  NX_DHCPV6_MESSAGE_TYPE_CONFIRM;

            /* Increase the count of retries. */
            dhcpv6_ptr -> nx_dhcpv6_confirms_sent++;

            /* Send the DHCPV6 Confirm message.  */
            status = _nx_dhcpv6_send_request(dhcpv6_ptr);

            /* Send the message until we get a reply from the server. */
            status = _nx_dhcpv6_waiting_on_reply(dhcpv6_ptr);

            /* Check for socket receive error or Server error code. */
            if ((status != NX_SUCCESS) || (dhcpv6_ptr -> nx_status_code != NX_DHCPV6_SUCCESS))
            {

                if ((status == NX_SUCCESS) && (dhcpv6_ptr -> nx_status_code == NX_DHCPV6_NOT_ON_LINK))
                {

                    /* Indicate the DHCP Client process is idle. */
                    dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_TRUE;

                    /* The next sending packet is the new packet, so reset the message id.  */
                    dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = 0;

                    /* Clear the assigned IP address.  */                
                    _nx_dhcpv6_remove_assigned_address(dhcpv6_ptr,NX_DHCPV6_REMOVE_ALL_IA_ADDRESS);

                    /* When the client receives a NotOnLink status from the server in response to a Confirm messag,
                       the client performs DHCP server solicitation. RFC3315, Section18.1.8, Page48.  */
                    _nx_dhcpv6_request_solicit(dhcpv6_ptr);
                }

                /* Retransmit the Confirm message.  */
                else
                {                

                    /* Update the retransmission information. */
                    if (_nx_dhcpv6_update_retransmit_info(dhcpv6_ptr))
                    {
                        
                        /* The retry limit on Confirm messages is exceeded. Set the Client back to 
                        the INIT state and stop sending rebind messages for now. */
                        dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_INIT;

                        /* Confirm request failed. The state change callback if set will be called. */
                    }
                }
            }
            else
            {

                /* Successful confirmation. Set the assigned address status to valid (confirmed). */

                /* Yes, get the IPv6 address by DHCPv6. set it.  */
                status = _nx_dhcpv6_register_IP_address(dhcpv6_ptr);

                /* Increment the number of responses to confirm.  */
                dhcpv6_ptr -> nx_dhcpv6_confirm_responses++;
                           
                /* Restore the Client state depending on its IANA address status. */
                dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_BOUND_TO_ADDRESS;

                /* The next sending packet is the new packet, so reset the message id.  */
                dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = 0;

                /* Receive the correct response, so deactivate the session timer of elapsed time. */
                tx_timer_deactivate(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));
            }
            
            break;

        }

        case NX_DHCPV6_STATE_SENDING_RELEASE:
        {
        
            dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type =  NX_DHCPV6_MESSAGE_TYPE_RELEASE;

            /* Increase the count of releases. */
            dhcpv6_ptr -> nx_dhcpv6_releases_sent++;

            /* Send the DHCPV6 Release message.  */
            status = _nx_dhcpv6_send_request(dhcpv6_ptr);

            /* Check for socket error. */
            if (status != NX_SUCCESS)
            {
            
                /* Return. Can't do any more now. */
                return;
            }

            /* Send the message to the server. */
            status = _nx_dhcpv6_waiting_on_reply(dhcpv6_ptr);
            
            /* Check for socket receive error.  For the release message, consider the release
               request completed if the server responds, regardless of Server error code. RFC 3315 Section 18.1.8 */
            if (status != NX_SUCCESS)
            {
                                
                /* Update the retransmission information. */
                if (_nx_dhcpv6_update_retransmit_info(dhcpv6_ptr))
                {

                    /* Clear global ip address and lifetime parameters from the Client record. */                
                    _nx_dhcpv6_remove_assigned_address(dhcpv6_ptr, NX_DHCPV6_REMOVE_ALL_IA_ADDRESS);

                    /* The retry limit on release messages is exceeded. Set the Client back to 
                       the INIT state and stop sending rebind messages for now. */
                    dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_INIT;

                    /* No response to release message from server. The state change callback if set will be called. */
                }
            }
            else
            {
                /* Increment the number of responses to release request.  */
                dhcpv6_ptr -> nx_dhcpv6_release_responses++;
                
                /* The next sending packet is the new packet, so reset the message id.  */
                dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = 0;
                
                /* Receive the correct response, so deactivate the session timer of elapsed time. */
                tx_timer_deactivate(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));
                
                /* Clear global ip address and lifetime parameters from the Client record. */                 
                _nx_dhcpv6_remove_assigned_address(dhcpv6_ptr, NX_DHCPV6_REMOVE_ALL_IA_ADDRESS);

                /* Update the Client state to not bound. */
                dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_INIT; 
            }

            break;
        }

        case NX_DHCPV6_STATE_SENDING_DECLINE:
        {           

            dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type =  NX_DHCPV6_MESSAGE_TYPE_DECLINE;

            /* Increase the count of declines. */
            dhcpv6_ptr -> nx_dhcpv6_declines_sent++;

            /* Send the DHCPV6 Decline message.  */
            status = _nx_dhcpv6_send_request(dhcpv6_ptr);

            /* Check for interal error. */
            if (status != NX_SUCCESS)
            {
            
                /* Return. Can't do any more now. */
                return;
            }

            /* Send the message to the server. */
            status = _nx_dhcpv6_waiting_on_reply(dhcpv6_ptr);

            /* Check for socket receive error.  For the decline message, consider the release
               request completed if the server responds, regardless of Server error code. RFC3315,Section 18.1.7 */
            if (status != NX_SUCCESS)
            {
                
                /* Update the retransmission information. */
                if (_nx_dhcpv6_update_retransmit_info(dhcpv6_ptr))
                {

                    /* Clear global ip address and lifetime parameters from the Client record. */                 
                    _nx_dhcpv6_remove_assigned_address(dhcpv6_ptr, NX_DHCPV6_REMOVE_ALL_IA_ADDRESS);

                    /* The retry limit on decline messages is exceeded. Set the Client back to 
                       the INIT state and stop sending rebind messages for now. */
                    dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_INIT;

                    /* No response to decline message from server. The state change callback if set will be called. */
                }
            }
            else
            {

                /* Increment the number of server responses to Decline request.  */
                dhcpv6_ptr -> nx_dhcpv6_decline_responses++;
                
                /* The next sending packet is the new packet, so reset the message id.  */
                dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = 0;
                
                /* Receive the correct response, so deactivate the session timer of elapsed time. */
                tx_timer_deactivate(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));

                /* Clear global ip address and lifetime parameters from the Client record. */                
                status = _nx_dhcpv6_remove_assigned_address(dhcpv6_ptr, NX_DHCPV6_REMOVE_ALL_IA_ADDRESS);

                /* Change the DHCPv6 Client to the unbound state. */
                dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_INIT;
            }            

            break;
        }

        case NX_DHCPV6_STATE_SENDING_INFORM_REQUEST:
        {
        
            dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type =  NX_DHCPV6_MESSAGE_TYPE_INFORM_REQUEST;

            /* Increase the count of declines. */
            dhcpv6_ptr -> nx_dhcpv6_inform_req_sent++;
            
            /* Send the Inform Request message.  */
            status = _nx_dhcpv6_send_request(dhcpv6_ptr);

            /* Check for transmission error. */
            if (status != NX_SUCCESS)
            {

                /* Return. Nothing more to do. */
                return;
            }

            /* Wait for a reply from the server. This will also extract the
               server packet information and update the DHCPv6 Client state. */
            status = _nx_dhcpv6_waiting_on_reply(dhcpv6_ptr);

            /* Check for socket receive error or Server error code. */
            if ((status != NX_SUCCESS) || (dhcpv6_ptr -> nx_status_code != NX_DHCPV6_SUCCESS))
            {
                
                /* Update the retransmission information. */
                if (_nx_dhcpv6_update_retransmit_info(dhcpv6_ptr))
                {

                    /* The retry limit on decline messages is exceeded. Set the Client back to 
                       the INIT state and stop sending rebind messages for now. */
                    dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_INIT;
                }
            }
            else
            {
                /* Increment the number of responses to Inform Request received.  */
                dhcpv6_ptr -> nx_dhcpv6_inform_req_responses++;
                
                /* The next sending packet is the new packet, so reset the message id.  */
                dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = 0;
                
                /* Receive the correct response, so deactivate the session timer of elapsed time. */
                tx_timer_deactivate(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));

                /* Restore the Client state depending on its IANA address status. */
                if (dhcpv6_ptr -> nx_dhcpv6_ia[0].nx_address_status == NX_DHCPV6_IA_ADDRESS_STATE_VALID)
                {
                    dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_BOUND_TO_ADDRESS;
                }
                else
                {
                    dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_INIT;
                }
            }

            break;
        }

        default:
        {
        
            /* Unknown or unsupported DHCPv6 request (state). */
            return;
        }
    }

    /* Is there a state change callback registered with the Client?  */
    if ((dhcpv6_ptr -> nx_dhcpv6_state_change_callback) && (original_state != (dhcpv6_ptr -> nx_dhcpv6_state)))
    {

        /* Yes, call the the state change callback with the original and new state.  */
        (dhcpv6_ptr -> nx_dhcpv6_state_change_callback)(dhcpv6_ptr, original_state, dhcpv6_ptr -> nx_dhcpv6_state);
    }

    return;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_client_duid                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the client DUID from the server reply and    */
/*    verifies that it matches the Client DUID on record.                 */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    option_data                       Pointer to option data            */
/*    option_length                     Size of option data               */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INVALID_CLIENT_DUID     Client DUID is missing data or    */
/*                                       does not match the DUID on record*/ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_utility_get_data       Parses data from specified buffer */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_extract_packet_information                               */ 
/*                                      Extracts server reply from packet */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_process_client_duid(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length)
{

ULONG   data;
UINT    index = 0;


    /* The Client should already have its own DUID on record. So just parse the 
       data and compare each DUID field with the Client's DUID. */

    /* Check option length for DUID type and hardware type.  */
    if (option_length < 4)
    {
        return(NX_DHCPV6_INVALID_CLIENT_DUID);
    }

    /* Extract the DUID type which should be the next 2 bytes.  */
    _nx_dhcpv6_utility_get_data((option_data + index), 2, &data);

    /* Does this match the Client DUID type on record? */
    if (data != (dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_duid_type))
    {

        /* No, return the error status to reject this packet. */
        return NX_DHCPV6_INVALID_CLIENT_DUID;
    }

    index += 2;

    /* Extract the hardware type which should be the next 2 bytes.  */
    _nx_dhcpv6_utility_get_data((option_data + index), 2, &data);

    /* Does this match the Client DUID hardware type on record? */
    if (data != (dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_hardware_type))
    {

        /* No, return the error status to reject this packet. */
        return NX_DHCPV6_INVALID_CLIENT_DUID;
    }

    index += 2;

    /* IS this a link layer plus time DUID type? */
    if ((dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_duid_type) == NX_DHCPV6_DUID_TYPE_LINK_TIME)
    {

        /* Check option length for time.  */
        if (index + 4 > option_length)
        {
            return(NX_DHCPV6_INVALID_CLIENT_DUID);
        }

        /* Yes; Extract the time which should be the next 4 bytes.  */
        _nx_dhcpv6_utility_get_data((option_data + index), 4, &data);
    
        /* Does this match the Client DUID time on record? */
        if (data != (dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_duid_time))
        {

            /* No, return the error status to reject this packet. */
            return NX_DHCPV6_INVALID_CLIENT_DUID;
        }

        index += 4;
    }

    /* Check the hardware type to extract the MSW.  */
    if (dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_hardware_type == NX_DHCPV6_CLIENT_HARDWARE_TYPE_ETHERNET)
    {

        /* The length of link layer address is 48 bits.  */
 
        /* Check option length for 48 bits mac address.  */
        if (index + 6 > option_length)
        {
            return(NX_DHCPV6_INVALID_CLIENT_DUID);
        }

        /* Yes; Extract the link local address msw which should be the next 2 bytes.  */
        _nx_dhcpv6_utility_get_data((option_data + index), 2, &data);
        index += 2;
    }
    else if (dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_hardware_type == NX_DHCPV6_CLIENT_HARDWARE_TYPE_EUI_64)
    {

        /* The length of link layer address is 64 bits.  */

        /* Check option length for 64 bits mac address.  */
        if (index + 8 > option_length)
        {
            return(NX_DHCPV6_INVALID_CLIENT_DUID);
        }

        /* Yes; Extract the link local address msw which should be the next 4 bytes.  */
        _nx_dhcpv6_utility_get_data((option_data + index), 4, &data);
        index += 4;
    }

    /* Does this match the Client DUID link layer most significant bytes on record? */
    if (data != (dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_link_layer_address_msw))
    {

        /* No, return the error status to reject this packet. */
        return NX_DHCPV6_INVALID_CLIENT_DUID;
    }

    /* Yes; Extract the link local address lsw which should be the next 4 bytes.  */
    _nx_dhcpv6_utility_get_data((option_data + index), 4, &data);
    index += 4;

    /* Does this match the Client DUID link layer least significant bytes on record? */
    if (data != ((dhcpv6_ptr -> nx_dhcpv6_client_duid).nx_link_layer_address_lsw))
    {

        /* No, return the error status to reject this packet. */
        return NX_DHCPV6_INVALID_CLIENT_DUID;
    }

    /* Are we past the end of the buffer.  */
    if (index != option_length)
    {

        /* Yes, return the error status to reject this packet. */
        return NX_DHCPV6_INVALID_CLIENT_DUID;
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_domain_name                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the domain name option from the server reply.*/
/*    It will save as much of the domain name that will fit in the Client */
/*    domain name buffer. Buffer size is user configurable.               */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    packet_start                      Pointer to packet buffer          */
/*    option_data                       Pointer to option data            */
/*    option_length                     Size of option data               */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_dhcpv6_name_string_unencode   Decode the name string            */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_extract_packet_information                               */
/*                                      Extracts DHCPv6 options from      */
/*                                             server reply               */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_process_domain_name(NX_DHCPV6 *dhcpv6_ptr, UCHAR *packet_start, UCHAR *option_data, UINT option_length)
{

UINT        domain_name_length;
UINT        temp_length;
UCHAR       *domain_name_ptr;
UCHAR       *buffer_prepend_ptr;
UINT        buffer_size;
UINT        i;

    
    /* Initialize the value.  */
    domain_name_length = 0;
    temp_length = 0;
    domain_name_ptr = option_data;
    buffer_prepend_ptr = &dhcpv6_ptr -> nx_dhcpv6_domain_name[0];
    buffer_size = NX_DHCPV6_DOMAIN_NAME_BUFFER_SIZE;

    /* Process the domain search list options.  */
    while(temp_length < option_length)
    {

        /* Calculate the domain name length in the Domaim Search List option,include the null flag '\0'.  */
        for (i = 0; (i + temp_length < option_length) && (domain_name_ptr[i] != '\0'); i++);

        if (i + temp_length == option_length)
        {
            return(NX_DHCPV6_PROCESSING_ERROR);
        }
        else
        {
            temp_length += (i + 1);
        }

        /* Check buffer size.  */
        if (buffer_size < 1)
        {
            return(NX_DHCPV6_PROCESSING_ERROR);
        }

        /* Record the real domain name and return the length, one less for NULL termination. */
        domain_name_length = _nx_dhcpv6_name_string_unencode(packet_start, (UINT)(domain_name_ptr - packet_start), buffer_prepend_ptr, (UINT)((buffer_size - 1) & 0xFFFFFFFF));

        /* Check the domain name length.  */
        if(!domain_name_length)
        {

            /* Process error, return.  */
            return NX_DHCPV6_PROCESSING_ERROR;
        }

        else
        {

            /* Update the buffer value,include the null flag '\0'.  */
            domain_name_ptr += temp_length;
            buffer_prepend_ptr += (domain_name_length + 1);
            buffer_size -= (domain_name_length + 1);
        }
    }

    /* Return successful completion. */
    return NX_SUCCESS;
}

       

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_name_string_encode                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts a string containing the name as a list of    */ 
/*    labels separated by dots to the encoded list of labels specified    */ 
/*    in RFC1035 for DNS servers. This conversion doesn't handle          */ 
/*    compression and doesn't check on the lengths of the labels or the   */ 
/*    entire name.                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ptr                                   Pointer to destination        */ 
/*    name                                  Source name string            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    count                                 Count of characters encoded   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_add_client_FQDN         Add FQDN Option to DHCPv6 packet */ 
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
UINT  _nx_dhcpv6_name_string_encode(UCHAR *ptr, UCHAR *name)
{

UCHAR   *length;
UINT    count =  1;


    /* Point to the first character position in the buffer.  This will point
       to the length of the following name.  */
    length =  ptr++;

    /* Default the length to zero.  */
    *length =  0;

    /* Move through string, copying bytes and updating length.
       Whenever a "." is found, start a new string by updating the
       pointer to the length and setting the length to zero.  */
    while (*name) 
    {

        /* Is a dot been found?  */
        if (*name == '.')
        { 

            /* Yes, setup a new length pointer. */
            length =  ptr++;

            /* Default the length to zero. */
            *length =  0;
            name++;
        }
        else
        {

            /* Copy a character to the destination.  */
            *ptr++ =  *name++;

            /* Adjust the length of the current segment.  */
            (*length)++;
        }

        /* Increment the total count here.  */
        count++;
    }

    /* Add the final zero length, like a NULL terminator.  */
    *ptr =  0;

    /* Increment the total count.  */
    count++;

    /* Return the count.  */
    return(count);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_name_string_unencode                     PORTABLE C      */ 
/*                                                           6.1.2        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts from the encoded list of labels as specified */ 
/*    in RFC 1035 to a string containing the name as a list of labels     */ 
/*    separated by dots.                                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    data                                  Pointer to buffer to decode   */ 
/*    start                                 Location of start of data     */
/*    buffer                                Pointer to decoded data       */ 
/*    size                                  Size of data buffer to decode */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Size of decoded data                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_domain_name        Process the domain name       */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  11-09-2020     Yuxin Zhou               Modified comment(s), fixed    */
/*                                            domain name labelSize issue,*/
/*                                            resulting in version 6.1.2  */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_name_string_unencode(UCHAR *data, UINT start, UCHAR *buffer, UINT size)
{

UCHAR   *character =  data + start;
UINT    length = 0;

  
    /* As long as there is space in the buffer and we haven't 
       found a zero terminating label */
    while ((size > length) && (*character != '\0'))
    {

    UINT  labelSize =  *character++;

        /* Check labelSize.  */
        if (labelSize <= NX_DHCPV6_LABEL_MAX)
        {

            /* Simple count, check for space and copy the label.  */
            while ((size > length) && (labelSize > 0))
            {

                *buffer++ =  *character++;
                length++;
                labelSize--;
            }
      
            /* Now add the '.' */
            *buffer++ =  '.';
            length++;
        }
        else
        {
            /* Not defined or in compressed form. Based on section 8 of RFC 3315, 
                a domain name, or list of domain names, in DHCP MUST NOT be stored
                in compressed form, so just fail */
            return(0);
        }
    }

    /* Validate the length.  */
    if (length == 0)
    {
        return(length);
    }

    /* Done copying the data, set the last . to a trailing null */
    if (*(buffer - 1) == '.')
    {

        buffer--;
        length --;
    }
    
    /* Null terminate name.  */
    *buffer =  '\0';

    /* Return name size.  */
    return(length);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_DNS_server                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the DNS server option from the server reply. */
/*    It will save as many DNS name server addresses as are in the reply  */
/*    for which the Client is configured to save.                         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    option_data                       Pointer to option data            */
/*    option_length                     Size of option data               */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INVALID_OPTION_DATA     DNS server option missing data or */
/*                                            improperly formatted        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_extract_packet_information                               */
/*                                      Extracts DHCPv6 options from      */
/*                                             server reply               */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_process_DNS_server(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length)
{

UINT   index = 0;
UINT   w, j = 0;


    /* Loop through the length of the buffer to parse. */
    while ((index + 16) <= option_length)
    {

        /* Is the DHCPv6 Client configured to store another DNS server address? */
        if (j < NX_DHCPV6_NUM_DNS_SERVERS)
        {

            /* Set the IP version. */
            dhcpv6_ptr -> nx_dhcpv6_DNS_name_server_address[j].nxd_ip_version = NX_IP_VERSION_V6;

            /* Get the next IPv6 DNS server address. */
            for (w = 0; w <= 3; w++)
            {

                /* Yes; copy the next word into the current DNS server address. */
                memcpy(&(dhcpv6_ptr -> nx_dhcpv6_DNS_name_server_address[j].nxd_ip_address.v6[w]), /* Use case of memcpy is verified. */
                        (option_data + index), sizeof(ULONG));

                /* Adjust for endianness. */
                NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_DNS_name_server_address[j].nxd_ip_address.v6[w]);

                /* Move to the next IPv6 address word. */
                index += 4;
            }

            /* Get the next DNS server address in the reply buffer. */
            j++;
        }
        else
        {

            /* Move to the next IPv6 address. */
            index += 16;
        }
    }

    /* Is there any more data in the buffer? */
    if (index != option_length)
    {

        /* Yes, not sure what is going on with this packet. Treat as an error. */
        return NX_DHCPV6_INVALID_OPTION_DATA;
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_ia                               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the IA address option from the server reply. */
/*    It also checks that the server preferred and valid lifetimes are    */
/*    legitamate. If the IANA option contains an IA option, that will also*/
/*    be processed as well as any status code options.                    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    option_data                       Pointer to option data            */
/*    option_length                     Size of option data               */
/*    ia_index                          The index of IA address           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INVALID_IA_DATA         IA option is missing data or      */
/*                                          improperly formatted          */
/*    NX_DHCPV6_INVALID_IA_TIME         IA option lifetime data is invalid*/
/*    NX_DHCPV6_UNKNOWN_OPTION          Unkonw option type                */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_dhcpv6_utility_get_block_option_length                          */
/*                                      Get the option code and length    */ 
/*    _nx_dhcpv6_process_status         Processes STAUTS in reply buffer  */
/*    nx_dhcpv6_server_error_handler    Pointer to error code handler     */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_iana           Processes IANA in reply buffer    */
/*                                            DHCPv6 request              */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_process_ia(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length, UINT ia_index)
{

ULONG   preferred_lifetime;
ULONG   valid_lifetime;
UINT    index = 0, w;
UINT    status;
ULONG   ia_option_code, ia_option_length;


    /* Fill in the IA address code and length. Client might already have one on
       record, but use the server's data instead. */
    dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_op_code = (USHORT)NX_DHCPV6_OP_IA_ADDRESS;
    dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_option_length = (USHORT)option_length;

    /* Check option length for Ipv6 address (16 bytes), preferred-lifetime (4 bytes) and valid-lifetime (4 bytes).  */
    if (option_length < 24)
    {
        return(NX_DHCPV6_INVALID_IA_DATA);
    }

    /* Process IPv6 address.  */
    for (w = 0; w <= 3; w++)
    {

        /* Copy each IPv6 address word into the IA address. */
        memcpy(&(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[w]), /* Use case of memcpy is verified. */
               (option_data + index), sizeof(ULONG));

        /* Adjust for endianness. */
        NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[w]);

        /* Move to the next IPv6 address word. */
        index += 4;
    }

    dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_version = NX_IP_VERSION_V6;

    /* Copy the lifetime data from the reply buffer to temporary variables.*/
    memcpy(&preferred_lifetime, (option_data + index), sizeof(ULONG)); /* Use case of memcpy is verified. */
    index += 4;

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(preferred_lifetime);

    memcpy(&valid_lifetime, (option_data + index), sizeof(ULONG)); /* Use case of memcpy is verified. */
    index += 4;

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(valid_lifetime);

    /* Check for invalid lifetime data. The preferred lifetime MUST be less than
      the valid lifetime.  Note that a zero lifetime is acceptable 
      (the server is letting the Client use its own preference). */
    if (preferred_lifetime > valid_lifetime)
    {
        /* Discard this address. We cannot use it. */
        return NX_DHCPV6_INVALID_IA_TIME;
    }

    /* OK to assign time data to the client. */
    dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_preferred_lifetime = preferred_lifetime;
    dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_valid_lifetime = valid_lifetime;
    
    /* Update the address status.  */
    if(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status == NX_DHCPV6_IA_ADDRESS_STATE_INVALID)
    {
        dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status = NX_DHCPV6_IA_ADDRESS_STATE_INITIAL;
    }

    /* Update the map status, 0xFFFFFFFF indicate the IPv6 address of IA option has been processed.  */
    dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_map = 0xFFFFFFFF;

    /* Check if we're at the end of option data yet. */
    if (index == option_length)
    {

        /* Yes, all done. */
        return NX_SUCCESS;
    }

    /* Check option length for status code option.  */
    if (index + 4 > option_length)
    {
        return(NX_DHCPV6_INVALID_IA_DATA);
    }

    /* Process the status code option. */
    status = _nx_dhcpv6_utility_get_block_option_length(option_data + index, &ia_option_code, &ia_option_length);

    /* Check that the block data is valid. */
    if (status != NX_SUCCESS)
    {

        /* No, return the error status. */
        return status;
    }

    /* Skip status option code and length.  */
    index += 4;

    /* This is a double check to verify we haven't gone off the end of the packet buffer. */
    if (index + ia_option_length > option_length)
    {
        return (NX_DHCPV6_INVALID_IA_DATA);
    }

    /* Check if this is an IAaddr status option request. */
    if (ia_option_code == NX_DHCPV6_OP_STATUS_CODE)
    {
        /* The IAaddr option returned by the DHCPv6 server includes a status option. */

        /* Process the status. */
        status = _nx_dhcpv6_process_status(dhcpv6_ptr, (option_data + index), ia_option_length);

        /* Now check on the server status of the previous option in the DHCP message. */
        if ((status == NX_SUCCESS) && (dhcpv6_ptr -> nx_status_code != NX_DHCPV6_SUCCESS))
        {

            /* Does the dhcpv6 client have a server error handler? */
            if (dhcpv6_ptr -> nx_dhcpv6_server_error_handler)
            {

                /* Call the error handler with status code, option (IAaddr) and which message type it refers to. */
                dhcpv6_ptr -> nx_dhcpv6_server_error_handler(dhcpv6_ptr, NX_DHCPV6_ERROR_STATUS_CODE_IN_IA_ADDRESS, dhcpv6_ptr -> nx_status_code,
                                                             dhcpv6_ptr -> nx_dhcpv6_received_message_type);
            }
        }

        /* Check for errors processing the DHCPv6 message. */
        if (status != NX_SUCCESS)
        {

            /* Abort further processing. Return error status. */
            return status;
        }
    }

    else
    {
        return NX_DHCPV6_UNKNOWN_OPTION;
    }

    /* Keep track of how far into the packet we have parsed. */
    index += ia_option_length; 

    /* Check if we went past the reported size of IA address data. */
    if (index != option_length)
    {

        /* Return an error status. Cannot accept this reply. */
        return NX_DHCPV6_INVALID_IA_DATA;
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_iana                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the IANA option from the server reply.  It   */
/*    verifies the IANA option ID matches the IANA option ID on client    */
/*    record. It also checks that the server T1 and T2 lease times are    */
/*    legitamate. If the IANA option contains an IA option, that will also*/
/*    be processed as well as any status code options.                    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    option_data                       Pointer to option data            */
/*    option_length                     Size of option data               */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_DHCPV6_BAD_IANA_ID             IANA option ID does not match IANA*/
/*                                        option on client record         */
/*    NX_DHCPV6_INVALID_IANA_TIME       IANA T1 and T2 values are invalid */
/*    NX_DHCPV6_STATE_IP_FAILURE        Indicates server rejected IANA/IA */
/*    NX_DHCPV6_INVALID_IANA_DATA       IANA option is missing data or    */
/*                                       improperly formatted             */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_utility_get_data     Parses data from reply buffer       */
/*    _nx_dhcpv6_utility_get_block_option_length                          */ 
/*                                    Parses option header from buffer    */
/*    _nx_dhcpv6_process_ia           Process IA option in reply buffer   */
/*    _nx_dhcpv6_process_status       Process status option in buffer     */
/*    nx_dhcpv6_server_error_handler  Pointer to server error code handler*/
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_extract_packet_information                               */ 
/*                                      Extracts server reply from packet */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_process_iana(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length)
{

UINT    status;
ULONG   T1, T2, data;
UINT    index = 0;
ULONG   iana_option_code, iana_option_length;
UINT    ia_index; 
UINT    ia_count = 0;
UINT    ia_status;


    /* Check option length for IANA ID.  */
    if (option_length < 4)
    {
        return(NX_DHCPV6_INVALID_IANA_DATA);
    }

    /* Get the IANA ID. */
    memcpy(&data, (option_data + index), 4); /* Use case of memcpy is verified. */

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(data);

    /* The server response must return the correct IANA ID; this is applicable
       to clients configured with more than one IANA. */
    if (data != (dhcpv6_ptr -> nx_dhcpv6_iana.nx_IA_NA_id))
    {

        /* IANA does not match. */
        return NX_DHCPV6_BAD_IANA_ID; 
    }

    /* Update the write location index. */
    index += 4; 

    /* Check option length for T1 and T2.  */
    if (index + 8 > option_length)
    {
        return(NX_DHCPV6_INVALID_IANA_DATA);
    }

    /* Copy T1 and T2 from the buffer into temporary variables. */
    memcpy(&T1, (option_data + index), 4); /* Use case of memcpy is verified. */
    index += 4; 

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(T1);

    memcpy(&T2, (option_data + index), 4); /* Use case of memcpy is verified. */
    index += 4; 

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(T2);

    /* Check for invalid T1/T2 lifetimes. */
    if (T1 > T2)
    {

        /* Reject the server DHCPv6 response. */
        return NX_DHCPV6_INVALID_IANA_TIME; 
    }
        
    /* Record the T1 and T2.  */
    /* Yes, the server will let the Client use its own T1 time? */
    if (T1 != 0)
    {

        /* Now, assign the server T1 to the client IA-NA. */
        dhcpv6_ptr -> nx_dhcpv6_iana.nx_T1 = T1;
    }

    /* Yes,the server will let the Client use its own T2 time? */
    if (T2 != 0)
    {
        /* Now, assign the server T2 to the client IA-NA. */
        dhcpv6_ptr -> nx_dhcpv6_iana.nx_T2 = T2;
    }

    /* Check if we're at the end of option data yet. */
    if (index == option_length)
    {

        /* Yes, all done. */
        return NX_SUCCESS;
    }
    
    /* If the reply message includes IA address option, clear the record.  */
    if(dhcpv6_ptr -> nx_dhcpv6_reply_option_flags & NX_DHCPV6_INCLUDE_IA_ADDRESS_OPTION)
    {

        /* Clear the IA address table according to the address map value.   */        
        for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
        {

            /* The reply message does not include this IA option.*/
            if(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_map == 0)
            {

                /* Clear it.  */ 
                status = _nx_dhcpv6_remove_assigned_address(dhcpv6_ptr, ia_index);
            }
        }
    }

    /* Now we recurse into the options embedded in the IANA option. */
    while (index + 4 <= option_length)
    {

        /* Get the next option code and length. */
        status = _nx_dhcpv6_utility_get_block_option_length((option_data + index), &iana_option_code, &iana_option_length);
       
        /* Check that the block data is valid. */
        if (status != NX_SUCCESS)
        {
    
            /* No, return the error status. */
            return status;
        }

        /* Skip IANA sub option code and length.  */
        index += 4;

        /* This is a double check to verify we haven't gone off the end of the packet buffer. */
        if (index + iana_option_length > option_length)
        {
            return (NX_DHCPV6_INVALID_IANA_DATA);
        }

        /* Check if this is an IA address option request. */
        if (iana_option_code == NX_DHCPV6_OP_IA_ADDRESS)
        {

            /* Set the ia count to record the IA option of reply message.  */
            ia_count ++;
            ia_status = 0;

            /* First, find the same IPv6 address.  */
            for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
            {

                /* The IPv6 address is new,so find the empty space to record it.  */
                if(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_map == ia_count)
                {

                    /* Yes, find it, update the IA option.  */
                    status = _nx_dhcpv6_process_ia(dhcpv6_ptr, (option_data + index), iana_option_length, ia_index);

                    /* Check the process status. */
                    if (status != NX_SUCCESS)
                    {

                        /* No, return the error status. */
                        return status;
                    }

                    /* Yes, set success.  */
                    ia_status = 1;

                    break;
                }
            }

            /* Record the IA option failure.  */
            if(ia_status == 0)
            {

                /* Second, find one empty IA to record it.  */            
                for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
                {

                    /* The IPv6 address is new,so find the empty space to record it.  */
                    if(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_map == 0)
                    {

                        /* Yes, it is available, update the IA option.  */
                        status = _nx_dhcpv6_process_ia(dhcpv6_ptr, (option_data + index), iana_option_length, ia_index);

                        /* Check the process status. */
                        if (status != NX_SUCCESS)
                        {

                            /* No, return the error status. */
                            return status;
                        }

                        break;
                    }
                }
            }
        }

        /* Check if this is an IAaddr status option request. */
        if (iana_option_code == NX_DHCPV6_OP_STATUS_CODE)
        {
            /* The IAaddr option returned by the DHCPv6 server includes a status option. */

            /* Process the status. */
            status = _nx_dhcpv6_process_status(dhcpv6_ptr, (option_data + index), iana_option_length);

            /* Now check on the server status of the previous option in the DHCP message. */
            if ((status == NX_SUCCESS) && (dhcpv6_ptr -> nx_status_code != NX_DHCPV6_SUCCESS))
            {

                /* Does the dhcpv6 client have a server error handler? */
                if (dhcpv6_ptr -> nx_dhcpv6_server_error_handler)
                {

                    /* Call the error handler with status code, option (IA_NA) and which message type it refers to. */
                    dhcpv6_ptr -> nx_dhcpv6_server_error_handler(dhcpv6_ptr, NX_DHCPV6_ERROR_STATUS_CODE_IN_IA_NA, dhcpv6_ptr -> nx_status_code,
                                                                 dhcpv6_ptr -> nx_dhcpv6_received_message_type);
                }
            }

            /* Check for errors processing the DHCPv6 message. */
            if (status != NX_SUCCESS)
            {

                /* Abort further processing. Return error status. */
                return status;
            }
        }

        /* Keep track of how far into the packet we have parsed. */
        index += iana_option_length;
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_preference                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the preference option from the server reply. */
/*    It will save as much of the time zone that will fit in the Client   */
/*    time zone buffer. Buffer size is user configurable.                 */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    option_data                       Pointer to option data            */
/*    option_length                     Size of option data               */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INVALID_PREF_DATA       Preference option is missing data */
/*                                         or improperly formatted        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    None                                                                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_extract_packet_information                               */
/*                                      Extracts DHCPv6 options from      */
/*                                             server reply               */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_process_preference(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length)
{


    /* Fill in the option code and length. */
    dhcpv6_ptr -> nx_dhcpv6_preference.nx_op_code = NX_DHCPV6_OP_PREFERENCE;
    dhcpv6_ptr -> nx_dhcpv6_preference.nx_option_length = (USHORT)option_length;

    /* Check option length for preference value.  */
    if (option_length != 1)
    {
        return(NX_DHCPV6_INVALID_PREF_DATA);
    }

    /* Get the server preference field, and apply to the DHCPv6 Client. */
    dhcpv6_ptr -> nx_dhcpv6_preference.nx_pref_value = (USHORT)(*option_data);

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_server_duid                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the server DUID from the server reply and,   */
/*    if the Client already has a DHCPv6 server on record, verifies that  */
/*    it matches the Client's server DUID.                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    option_data                       Pointer to option data            */
/*    option_length                     Size of option data               */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*    NX_DHCPV6_INVALID_SERVER_DUID     Server DUID is missing data or    */
/*                                       does not match the DUID on record*/ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_utility_get_data       Parses data from specified buffer */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_extract_packet_information                               */ 
/*                                      Extracts server reply from packet */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_process_server_duid(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length)
{

UINT    index = 0;
ULONG   temp_type;
ULONG   temp_hardware_type;
ULONG   temp_time = 0;
ULONG   temp_msw = 0;
ULONG   temp_lsw = 0;


    /* Set the DHCPv6 option header fields for a Server DUID. */
    dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_op_code = NX_DHCPV6_OP_SERVER_ID;
    dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_option_length = (USHORT)option_length;

    /* Check option length for DUID type and hardware type.  */
    if (option_length < 4)
    {
        return(NX_DHCPV6_INVALID_SERVER_DUID);
    }

    /* Extract the DUID type which should be the next 2 bytes.  */
    _nx_dhcpv6_utility_get_data((option_data + index), 2, &temp_type); 

    /* Update the index for moving the buffer pointer forward. */
    index += 2;

    /* Extract the hardware type which should be the next 2 bytes.  */
    _nx_dhcpv6_utility_get_data((option_data + index), 2, &temp_hardware_type);

    /* Update the index for moving the buffer pointer forward. */
    index += 2;

    /* Is this a link layer plus time DUID type? */
    if (temp_type == NX_DHCPV6_DUID_TYPE_LINK_TIME)
    {

        /* Check option length for time.  */
        if (index + 4 > option_length)
        {
            return(NX_DHCPV6_INVALID_SERVER_DUID);
        }

        /* Yes; Extract the time which should be the next 4 bytes.  */
        _nx_dhcpv6_utility_get_data((option_data + index), 4,  &temp_time);

        /* Update the index for moving the buffer pointer forward. */
        index += 4;
    }

    /* Check the hardware type to extract the MSW.  */
    if (temp_hardware_type == NX_DHCPV6_CLIENT_HARDWARE_TYPE_ETHERNET)
    {
  
        /* The length of link layer address is 48 bits.  */

        /* Check option length for 48 bits mac address.  */
        if (index + 6 > option_length)
        {
            return(NX_DHCPV6_INVALID_SERVER_DUID);
        }

        /* Yes; Extract the link local address msw which should be the next 2 bytes.  */
        _nx_dhcpv6_utility_get_data((option_data + index), 2, &temp_msw); 

        /* Update the index for moving the buffer pointer forward. */
        index += 2;
    }
    else if (temp_hardware_type == NX_DHCPV6_CLIENT_HARDWARE_TYPE_EUI_64)
    {

        /* The length of link layer address is 64 bits.  */

        /* Check option length for 64 bits mac address.  */
        if (index + 8 > option_length)
        {
            return(NX_DHCPV6_INVALID_SERVER_DUID);
        }

        /* Yes; Extract the link local address msw which should be the next 4 bytes.  */
        _nx_dhcpv6_utility_get_data((option_data + index), 4, &temp_msw); 

        /* Update the index for moving the buffer pointer forward. */
        index += 4;
    }

    /* Yes; Extract the link local address lsw which should be the next 4 bytes.  */
    _nx_dhcpv6_utility_get_data((option_data + index), 4, &temp_lsw);

    /* Update the index for moving the buffer pointer forward. */
    index += 4;

    /* Check if we are past the end of the buffer. */
    if (index != option_length)
    {

        /* Yes, return the error status to reject the packet. */
        return NX_DHCPV6_INVALID_SERVER_DUID;
    }

    /* If the client state in SENDING SOLICIT/CONFRIM/REBIND/INFORM_REQUEST, record the server DUID.  */
    if (dhcpv6_ptr -> nx_dhcpv6_state == NX_DHCPV6_STATE_SENDING_SOLICIT ||
        dhcpv6_ptr -> nx_dhcpv6_state == NX_DHCPV6_STATE_SENDING_CONFIRM ||
        dhcpv6_ptr -> nx_dhcpv6_state == NX_DHCPV6_STATE_SENDING_REBIND ||
        dhcpv6_ptr -> nx_dhcpv6_state == NX_DHCPV6_STATE_SENDING_INFORM_REQUEST )
    {

        /* No previous server DUID in the Client record.  Copy it over now. */
        dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_duid_type = (USHORT)temp_type; 

        /* Is this a TIME Duid type? */
        if (temp_type == NX_DHCPV6_DUID_TYPE_LINK_TIME)
        {

            /* Yes, write in the time data. */
            dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_duid_time = temp_time;
        }

        dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_hardware_type = (USHORT)temp_hardware_type;
        dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_msw = temp_msw; 
        dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_lsw = temp_lsw;
    }
    else
    {

        /* Yes; Verify this Server DUID matches the Server DUID from the Client record. */
        if ((dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_duid_time != temp_time) ||
            (dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_msw != temp_msw) ||
            (dhcpv6_ptr -> nx_dhcpv6_server_duid.nx_link_layer_address_lsw != temp_lsw))
        {

            /* It doesn't. Return an error status to reject this packet! */
            return NX_DHCPV6_INVALID_SERVER_DUID;
        }
    }

    /* Return successful completion status. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_status                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the option status option from the server     */
/*    reply. It also attempts to save the as much of server status message*/
/*    that will fit into the status message buffer.                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    option_data                       Pointer to option data            */
/*    option_length                     Size of option data               */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_iana           Processes IANA in reply buffer    */
/*                                            DHCPv6 request              */ 
/*    _nx_dhcpv6_process_ia             Processes IA in reply buffer      */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_process_status(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length)
{

UINT  index = 0;
UINT  message_length;
ULONG status_code;


    /* Check option length for status code.  */
    if (option_length < 2)
    {
        return(NX_DHCPV6_INVALID_OPTION_DATA);
    }

    /* Get the status field itself. */  
    _nx_dhcpv6_utility_get_data((option_data + index), sizeof(USHORT), &status_code); 

    /* Update the index for moving the buffer pointer forward. */
    index += 2;  

    /* Set the status code.  */
    dhcpv6_ptr -> nx_status_code = (USHORT)status_code;

    /* Now figure out how much of the message we can save, if not all of it. */
    if ((option_length - 2) > NX_DHCPV6_MAX_MESSAGE_SIZE)
    {

        /* Store up to this much, message is truncated but possibly still useful. */
        message_length = NX_DHCPV6_MAX_MESSAGE_SIZE;
    }
    else
    {

        /* Store all of it. */
        message_length = option_length - 2;
    }

    /* Copy the status into the Client record. */
    memcpy(dhcpv6_ptr -> nx_status_message, (option_data + index), message_length); /* Use case of memcpy is verified. */

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_time_zone                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the time zone option from the server reply.  */
/*    It will save as much of the time zone that will fit in the Client   */
/*    time zone buffer. Buffer size is user configurable.                 */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    option_data                       Pointer to option data            */
/*    option_length                     Size of option data               */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    None                                                                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_extract_packet_information                               */
/*                                      Extracts DHCPv6 options from      */
/*                                             server reply               */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_process_time_zone(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length)
{

UINT i;


    /* Make sure the time zone fits in the client buffer. */
    if (option_length > NX_DHCPV6_TIME_ZONE_BUFFER_SIZE)
    {

        /* It doesn't.  Fit what we can. */
        option_length = NX_DHCPV6_TIME_ZONE_BUFFER_SIZE;
    }

    /* Copy the time zone from the buffer to the Client record. */
    for (i = 0; i < option_length; i++)
    {

        dhcpv6_ptr -> nx_dhcpv6_time_zone[i] = *(option_data + i);
    }

    /* Return completion status. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process_time_server                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function extracts the time server option from the server reply.*/
/*    It will save as many time server addresses as are in the reply for  */
/*    which the Client is configured to save.                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    option_data                       Pointer to option data            */
/*    option_length                     Size of option data               */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    memcpy                            Copies specified area of memory   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_extract_packet_information                               */
/*                                      Extracts DHCPv6 options from      */
/*                                             server reply               */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_process_time_server(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length)
{

UINT   index = 0;
UINT   w, j = 0;


    /* Loop through the length of the buffer to parse. */
    while (index + 16 <= option_length)
    {

        /* Check that the DHCPv6 Client configured to store this time server address e.g. it may
            only store one address, but the server could be sending two or more. */
        if (j < NX_DHCPV6_NUM_TIME_SERVERS)
        {

            /* Set the IP version. */
            dhcpv6_ptr -> nx_dhcpv6_time_server_address[j].nxd_ip_version = NX_IP_VERSION_V6;

            /* Get the next IPv6 time server address. */
            for (w = 0; w <= 3; w++)
            {

                /* Yes; copy each IPv6 address word into the time server address. */
                memcpy(&(dhcpv6_ptr -> nx_dhcpv6_time_server_address[j].nxd_ip_address.v6[w]), /* Use case of memcpy is verified. */
                       (option_data + index), sizeof(ULONG));

                /* Adjust for endianness. */
                NX_CHANGE_ULONG_ENDIAN(dhcpv6_ptr -> nx_dhcpv6_time_server_address[j].nxd_ip_address.v6[w]);

                /* Move to the next IPv6 address word. */
                index += 4;
            }

            /* Start parsing the next time server address, if there is one.  */
            j++;
        }
        else
        {

            /* Move to the next timer server address. */
            index += 16;
        }
    }

    /* Is there any more data in this option? */
    if (index != option_length)
    {

        /* Yes, treat as in improperly formatted packet. */
        return NX_DHCPV6_INVALID_OPTION_DATA;
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_remove_assigned_address                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function removes the DHCPv6 Client address from the Client     */
/*    record and from the IP address table. IP lease lifetimes and IP     */
/*    addresses in the Client record are also cleared.                    */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful Completion status     */ 
/*    NX_INVALID_INTERFACE               Invalid index in IP address table*/
/*    status                             Failed mutex get                 */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                       Get exclusive lock on time keeper*/ 
/*    tx_mutex_put                       Release the lock on time keeper  */ 
/*    _nx_dhcpv6_set_IP_address_NULL     Set the IP address to NULL       */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    nx_dhcpv6_process                                                    */ 
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
UINT _nx_dhcpv6_remove_assigned_address(NX_DHCPV6 *dhcpv6_ptr, UINT ia_index)
{
UINT    index;

    /* Only clear one address.  */
    if(ia_index != NX_DHCPV6_REMOVE_ALL_IA_ADDRESS)
    {

        /* Check for invalid IA index.  */
        if (ia_index >= NX_DHCPV6_MAX_IA_ADDRESS)
        {
            return(NX_DHCPV6_REACHED_MAX_IA_ADDRESS);
        }

        if((dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status) &&
           (dhcpv6_ptr -> nx_dhcpv6_client_address_index[ia_index] != 0) &&
           (dhcpv6_ptr -> nx_dhcpv6_client_address_index[ia_index] != 0xFFFFFFFF))
        {

            /* Now remove the address from the IP address table. */
            nxd_ipv6_address_delete(dhcpv6_ptr -> nx_dhcpv6_ip_ptr, dhcpv6_ptr -> nx_dhcpv6_client_address_index[ia_index]);

            /* Set the address index to invalid. */
            dhcpv6_ptr -> nx_dhcpv6_client_address_index[ia_index] = 0xFFFFFFFF;
        }
        
        /* Clear the IA data.  */
        memset(&dhcpv6_ptr -> nx_dhcpv6_ia[ia_index], 0, sizeof(NX_DHCPV6_IA_ADDRESS));

    }

    /* Clear the all addresses.  */
    else
    {
        
        /* Zero out time accrued on the current lease. */
        dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued = 0;

        /* Set all lease lifetime and time fields to zero. */
        dhcpv6_ptr -> nx_dhcpv6_iana.nx_T1 = 0;
        dhcpv6_ptr -> nx_dhcpv6_iana.nx_T2 = 0;

        /* Delete the IPv6 address of IP instance.  */
        for(index = 0; index < NX_DHCPV6_MAX_IA_ADDRESS; index++)
        {

            if((dhcpv6_ptr -> nx_dhcpv6_ia[index].nx_address_status) &&
               (dhcpv6_ptr -> nx_dhcpv6_client_address_index[index] != 0) &&
               (dhcpv6_ptr -> nx_dhcpv6_client_address_index[index] != 0xFFFFFFFF))
            {

                /* Now remove the address from the IP address table. */
                nxd_ipv6_address_delete(dhcpv6_ptr -> nx_dhcpv6_ip_ptr, dhcpv6_ptr -> nx_dhcpv6_client_address_index[index]);

                /* Set the address index to invalid. */
                dhcpv6_ptr -> nx_dhcpv6_client_address_index[index] = 0xFFFFFFFF;
            }
        }

        /* Clear the all IA data.  */
        memset(&dhcpv6_ptr -> nx_dhcpv6_ia[0], 0, NX_DHCPV6_MAX_IA_ADDRESS * sizeof(NX_DHCPV6_IA_ADDRESS));

        /* Removed the default router.  */
        if (dhcpv6_ptr -> nx_dhcpv6_state >= NX_DHCPV6_STATE_SENDING_RENEW)
        {
            nxd_ipv6_default_router_delete(dhcpv6_ptr -> nx_dhcpv6_ip_ptr, &(dhcpv6_ptr -> nx_dhcpv6_server_address));
            memset(&(dhcpv6_ptr -> nx_dhcpv6_server_address), 0, sizeof(NXD_ADDRESS));
        }
    }

    /* Clear the all address.  */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the DHCPv6 Client state with the supplied state  */ 
/*    (request). The DHCP Client thread will detect a state change and    */
/*    process the new state.                                              */
/*                                                                        */
/*    Note the return status only reflects the outcome of sending the     */
/*    message, it does not indicate whether the request was successfully  */
/*    granted. The host application is notified of "state" changes e.g.   */
/*    NX_DHCPV6_STATE_SUCCESS or NX_DHCPV6_STATE_IP_FAILURE in the        */
/*    callback function nx_dhcpv6_state_change_callback.                  */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*    dhcpv6_state                       State for making DHCPv6 request  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful completion status     */
/*    NX_DHCP_NOT_STARTED                DHCPv6 Client not started        */ 
/*    status                             Get mutex status                 */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_mutex_get                       Get DHCPv6 Client mutex          */
/*    tx_mutex_put                       Releases the Client mutex        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_solicit       Submits a SOLICIT request to Client*/ 
/*    _nx_dhcpv6_request_renew         Submits a RENEW request to Client  */ 
/*    _nx_dhcpv6_request_rebind        Submits a REBIND request to Client */ 
/*    _nx_dhcpv6_request_release       Submits a RELEASE request to Client*/ 
/*    _nx_dhcpv6_request_decline       Submits a DECLINE request to Client*/ 
/*    _nx_dhcpv6_request_confirm       Submits a CONFIRM request to Client*/ 
/*    _nx_dhcpv6_request_inform_request                                   */
/*                                     Submits an Inform Request request  */
/*                                              to Client                 */ 
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
UINT  _nx_dhcpv6_request(NX_DHCPV6 *dhcpv6_ptr, UINT dhcpv6_state)
{

UINT            status;


    /* First get an exclusive lock on the DHCPv6 Client. */
    status = tx_mutex_get(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex), NX_DHCPV6_MUTEX_WAIT);

    /* Check for error. */
    if (status != TX_SUCCESS)
    {

        /* Return the error status. */
        return status;
    }

    /* Determine if DHCP is started.  */
    if (dhcpv6_ptr -> nx_dhcpv6_started == NX_FALSE)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));


        /* DHCP is not started so it can't be stopped.  */
        return(NX_DHCPV6_NOT_STARTED);
    }    

    /* Wait for an opportunity to stop the DHCPv6 Client thread e.g. not in the midst of 
       modifying the host DHCP profile!  */
    while (dhcpv6_ptr -> nx_dhcpv6_sleep_flag != NX_TRUE)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* Sleep temporarily. */
        tx_thread_sleep(1);

        /* Get the DHCP mutex.  */
        tx_mutex_get(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex), NX_DHCPV6_MUTEX_WAIT);
    }

    /* Set the Client state to process the DHCPv6 request. */
    dhcpv6_ptr -> nx_dhcpv6_state = (UCHAR)dhcpv6_state;

    /* Release the Client mutex. */
    tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_request_confirm                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX DHCPv6 Confirm    */
/*    request service.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Completion status                */ 
/*    NX_PTR_ERROR                       Invalid pointer input            */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_confirm         Actual DHCPV6 process Confirm    */ 
/*                                                  request service       */
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
UINT  _nxe_dhcpv6_request_confirm(NX_DHCPV6 *dhcpv6_ptr)
{

UINT status;


    /* Check for invalid pointer input. */
    if (!dhcpv6_ptr)
    {

        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status = _nx_dhcpv6_request_confirm(dhcpv6_ptr);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_confirm                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends a confirm request to the server for the Client  */
/*    to know the DHCPv6 IP address already assigned to it by the server. */
/*                                                                        */ 
/*    A host application should call this service after a host has been   */
/*    disconnected from the network or powered down to verify its address */
/*    is still valid.                                                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Completion status                */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request                  Actual DHCPV6 request service   */
/*    tx_timer_activate                   Activate the session timer      */
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
UINT  _nx_dhcpv6_request_confirm(NX_DHCPV6 *dhcpv6_ptr)
{

UINT status;

    /* Set the initial and max retransmission timeouts, and max number of retries.  */
    dhcpv6_ptr -> nx_dhcpv6_transmission_timeout =  NX_DHCPV6_INIT_CONFIRM_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_retransmission_count = 0;

    /* Set the initaial elasped time of DHCPv6 message.  */    
    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;
            
    /* Set the retransmission information(IRT, MRC, MRT, MRD). */
    dhcpv6_ptr -> nx_dhcpv6_init_retransmission_timeout = NX_DHCPV6_INIT_CONFIRM_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_count = NX_DHCPV6_MAX_CONFIRM_RETRANSMISSION_COUNT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_timeout = NX_DHCPV6_MAX_CONFIRM_RETRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_duration = NX_DHCPV6_MAX_CONFIRM_RETRANSMISSION_DURATION;

    /* Activate the session timer to update the elapsed time.  */
    tx_timer_activate(&dhcpv6_ptr -> nx_dhcpv6_session_timer);

    /* Call the internal function. */
    status = _nx_dhcpv6_request(dhcpv6_ptr, NX_DHCPV6_STATE_SENDING_CONFIRM);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_decline                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends a decline request to the server for the Client  */
/*    to tell the server it does not accept the assigned address.         */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Completion status                */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request                 Actual DHCPV6 request service    */
/*    tx_timer_activate                   Activate the session timer      */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */  
/*    _nx_dhcpv6_process                 Processes server replies to      */
/*                                          DHCPv6 Client requests        */ 
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
UINT  _nx_dhcpv6_request_decline(NX_DHCPV6 *dhcpv6_ptr)
{

UINT    status;
UINT    ia_index;

    /* Set the initial and max retransmission timeout and max number of retries.  */
    dhcpv6_ptr -> nx_dhcpv6_transmission_timeout =  NX_DHCPV6_INIT_DECLINE_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_retransmission_count = 0;
    
    /* Set the initaial elasped time of DHCPv6 message.  */    
    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;
            
    /* Set the retransmission information(IRT, MRC, MRT, MRD). */
    dhcpv6_ptr -> nx_dhcpv6_init_retransmission_timeout = NX_DHCPV6_INIT_DECLINE_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_count = NX_DHCPV6_MAX_DECLINE_RETRANSMISSION_COUNT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_timeout = NX_DHCPV6_MAX_DECLINE_RETRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_duration = NX_DHCPV6_MAX_DECLINE_RETRANSMISSION_DURATION;

    /* Activate the session timer to update the elapsed time.  */
    tx_timer_activate(&dhcpv6_ptr -> nx_dhcpv6_session_timer);
        
    /* Now remove the address from the IP address table,the client MUST NOT use any of the addresses it is declining 
       as the source address in the Release message or in any subsequently transmitted message, RFC3315,section18.1.7.. */
        
    /* Set the DHCPv6 Client IPv6 addresses.  */
    for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
    {

        /* Now remove the address from the IP address table,the client MUST NOT use any of the addresses it is declining 
        as the source address in the Release message or in any subsequently transmitted message, RFC3315,section18.1.7.. */

        /* Verify we have a valid address index. */
        if(dhcpv6_ptr -> nx_dhcpv6_client_address_index[ia_index] >= NX_MAX_IPV6_ADDRESSES) 
        {

            return NX_INVALID_INTERFACE;
        }

        if(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status == NX_DHCPV6_IA_ADDRESS_STATE_DAD_FAILURE)
        {
            /* Remove the address from the IP instance address table. */

            nxd_ipv6_address_delete(dhcpv6_ptr -> nx_dhcpv6_ip_ptr, dhcpv6_ptr -> nx_dhcpv6_client_address_index[ia_index]);

            /* Record the address index in the Client record. */
            dhcpv6_ptr -> nx_dhcpv6_client_address_index[ia_index] = 0xFFFFFFFF;
        }
    }
    
    /* Call the internal function and return completion status. */
    status = _nx_dhcpv6_request(dhcpv6_ptr, NX_DHCPV6_STATE_SENDING_DECLINE);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_request_inform_request                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX DHCPv6 information*/
/*    request service.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Completion status                */ 
/*    NX_PTR_ERROR                       Invalid pointer input            */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_inform_request  Actual DHCPV6 process Inform     */ 
/*                                          Request request service       */
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
UINT  _nxe_dhcpv6_request_inform_request(NX_DHCPV6 *dhcpv6_ptr)
{

UINT status;


    /* Check for invalid pointer input. */
    if (!dhcpv6_ptr)
    {
        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status = _nx_dhcpv6_request_inform_request(dhcpv6_ptr);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_inform_request                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends a inform request to the server for the Client   */
/*    to obtain network parameters from any DHCPv6 server on the network. */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Completion status                */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request                 Actual DHCPV6 request service    */
/*    tx_timer_activate                  Activate the session timer       */
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
UINT  _nx_dhcpv6_request_inform_request(NX_DHCPV6 *dhcpv6_ptr)
{

UINT status;


    /* Set the initial and max retransmission timeout and max number of retries.  */
    dhcpv6_ptr -> nx_dhcpv6_transmission_timeout =  NX_DHCPV6_INIT_INFORM_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_retransmission_count = 0;
    
    /* Set the initaial elasped time of DHCPv6 message.  */    
    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;
                
    /* Set the retransmission information(IRT, MRC, MRT, MRD). */
    dhcpv6_ptr -> nx_dhcpv6_init_retransmission_timeout = NX_DHCPV6_INIT_INFORM_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_count = NX_DHCPV6_MAX_INFORM_RETRANSMISSION_COUNT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_timeout = NX_DHCPV6_MAX_INFORM_RETRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_duration = NX_DHCPV6_MAX_INFORM_RETRANSMISSION_DURATION;

    /* Activate the session timer to update the elapsed time.  */
    tx_timer_activate(&dhcpv6_ptr -> nx_dhcpv6_session_timer);

    /* Call the internal function and return completion status. */
    status = _nx_dhcpv6_request(dhcpv6_ptr, NX_DHCPV6_STATE_SENDING_INFORM_REQUEST);

    /* Return actual completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_request_option_DNS_server               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX submit and process*/
/*    the DNS server request option from the DHPCv6 Server service.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*    enable                             Flag indicating if the DNS server*/
/*                                          request enabled or disabled   */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Completion status                */ 
/*    NX_PTR_ERROR                       Invalid pointer input            */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_option_DNS_server                                */
/*                                       Actual DHCPV6 process DNS server */ 
/*                                                  request service       */
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
UINT  _nxe_dhcpv6_request_option_DNS_server(NX_DHCPV6 *dhcpv6_ptr, UINT enable)
{

UINT status;

    /* Check for invalid pointer input. */
    if (!dhcpv6_ptr)
    {

        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status = _nx_dhcpv6_request_option_DNS_server(dhcpv6_ptr, enable);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_option_DNS_server                PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function enables or disables the DHCPv6 DNS server option.  If */
/*    enabled this option is added to the option list in DHCPv6 messages  */
/*    the Client sends to the server.                                     */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*    enable                             Flag indicating if the DNS server*/
/*                                          request enabled or disabled   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful completion status     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_dhcpv6_request_option_DNS_server(NX_DHCPV6 *dhcpv6_ptr, UINT enable)
{

    /* Are we enabling this feature? */
    if (enable == NX_TRUE)
    {

        /* Yes, register this option request with the DHCPv6 Client. */
        dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request |= (USHORT)NX_DHCPV6_DNS_SERVER_OPTION;
    }
    else
    {

        /* Clear this option request with the DHCPv6 Client. */
        dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request &= (USHORT)(~NX_DHCPV6_DNS_SERVER_OPTION);
    }

    /* Return successful completion. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_request_option_domain_name              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX submit and process*/
/*    the domain name request option from the DHPCv6 Server service.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    enable                            Flag indicating if the domain name*/
/*                                          request enabled or disabled   */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Completion status                */ 
/*    NX_PTR_ERROR                       Invalid pointer input            */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_option_time_server                               */
/*                                       Actual DHCPV6 process domain name*/ 
/*                                                  request service       */
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
UINT  _nxe_dhcpv6_request_option_domain_name(NX_DHCPV6 *dhcpv6_ptr, UINT enable)
{

UINT status;


    /* Check for invalid pointer input. */
    if (!dhcpv6_ptr)
    {

        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status = _nx_dhcpv6_request_option_domain_name(dhcpv6_ptr, enable);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_option_domain_name               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function enables or disables the DHCPv6 domain name option.  If*/
/*    enabled this option is added to the option list in DHCPv6 messages  */
/*    the Client sends to the server.                                     */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    enable                            Flag indicating if the domain name*/
/*                                          request enabled or disabled   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful completion status     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_dhcpv6_request_option_domain_name(NX_DHCPV6 *dhcpv6_ptr, UINT enable)
{

    /* Are we enabling this feature? */
    if (enable == NX_TRUE)
    {
        /* Yes, register this option request with the DHCPv6 Client. */
        dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request |= (USHORT)NX_DHCPV6_DOMAIN_NAME_OPTION;
    }
    else
    {

        /* Clear this option request with the DHCPv6 Client. */
        dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request &= (USHORT)(~NX_DHCPV6_DOMAIN_NAME_OPTION);
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_request_option_time_server              PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX submit and process*/
/*    the time server request option from the DHPCv6 Server service.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    enable                            Flag indicating if the time server*/
/*                                          request enabled or disabled   */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Completion status                */ 
/*    NX_PTR_ERROR                       Invalid pointer input            */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_option_time_server                               */
/*                                       Actual DHCPV6 process time server*/ 
/*                                                  request service       */
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
UINT  _nxe_dhcpv6_request_option_time_server(NX_DHCPV6 *dhcpv6_ptr, UINT enable)
{

UINT status;


    /* Check for invalid pointer input. */
    if (!dhcpv6_ptr)
    {
        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status = _nx_dhcpv6_request_option_time_server(dhcpv6_ptr, enable);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_option_time_server               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function enables or disables the DHCPv6 time server option.  If*/
/*    enabled this option is added to the option list in DHCPv6 messages  */
/*    the Client sends to the server.                                     */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    enable                            Flag indicating if the time server*/
/*                                          request enabled or disabled   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Actual completion status         */ 
/*    NX_SUCCESS                         Successful completion status     */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_dhcpv6_request_option_time_server(NX_DHCPV6 *dhcpv6_ptr, UINT enable)
{

    /* Is the caller requesting the time server option? */
    if (enable == NX_TRUE)
    {
        /* Yes, register this option request with the DHCPv6 Client. */
        dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request |= (USHORT)NX_DHCPV6_SNTP_SERVER_OPTION;
    }
    else
    {

        /* Clear this option request with the DHCPv6 Client. */
        dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request &= (USHORT)(~NX_DHCPV6_SNTP_SERVER_OPTION);
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_request_option_timezone                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX submit and process*/
/*    the time zone request option from the DHPCv6 Server service.        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*    enable                            Flag indicating if the time zone  */
/*                                          request enabled or disabled   */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Completion status                */ 
/*    NX_PTR_ERROR                       Invalid pointer input            */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcpv6_request_option_time_zone  Actual DHCPV6 process time zone */ 
/*                                                  request service       */
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
UINT  _nxe_dhcpv6_request_option_timezone(NX_DHCPV6 *dhcpv6_ptr, UINT enable)
{

UINT status;


    /* Check for invalid pointer input. */
    if (!dhcpv6_ptr)
    {

        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status = _nx_dhcpv6_request_option_timezone(dhcpv6_ptr, enable);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_option_timezone                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function enables or disables the DHCPv6 time zone option.  If  */
/*    enabled this option is added to the option list in DHCPv6 messages  */
/*    the Client sends to the server.                                     */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*    enable                             Flag indicating if the time zone */
/*                                          request enabled or disabled   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful completion status     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_dhcpv6_request_option_timezone(NX_DHCPV6 *dhcpv6_ptr, UINT enable)
{

    /* Are we enabling this feature? */
    if (enable == NX_TRUE)
    {
        /* Yes, register this option request with the DHCPv6 Client. */
        dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request |= NX_DHCPV6_NEW_POSIX_TIMEZONE_OPTION;
    }
    else
    {

        /* Clear this option request with the DHCPv6 Client. */
        dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request &= (USHORT)(~NX_DHCPV6_NEW_POSIX_TIMEZONE_OPTION);
    }

    return NX_SUCCESS;
}



/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_request_option_FQDN                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the request fully qualified*/
/*    domain name option service.                                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/  
/*    domain_name                        DHCPv6 CLient domain name        */
/*    op                                 DHCPv6 Client operation          */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Completion status                */ 
/*    NX_PTR_ERROR                       Invalid pointer input            */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcpv6_request_option_FQDN       Actual DHCPv6 process FQDN      */
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
UINT  _nxe_dhcpv6_request_option_FQDN(NX_DHCPV6 *dhcpv6_ptr, CHAR *domain_name, UINT op)
{

UINT status;


    /* Check for invalid pointer input. */
    if ((!dhcpv6_ptr) || (!domain_name))
    {       
        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status = _nx_dhcpv6_request_option_FQDN(dhcpv6_ptr, domain_name, op);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_option_FQDN                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function processes the DHCPv6 Client fully qualified domain    */
/*    name. DHCPv6 Client sends this option to indicate if the server     */
/*    SHOULD or SHOULD NOT perform the AAAA RR or DNS updates.            */
/*                                                                        */
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/  
/*    domain_name                        DHCPv6 CLient domain name        */
/*    op                                 DHCPv6 Client operation          */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Successful completion status     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nx_dhcpv6_request_option_FQDN(NX_DHCPV6 *dhcpv6_ptr, CHAR *domain_name, UINT op)
{
UINT name_length;

    /* Set the Client FQDN option code.  */
    dhcpv6_ptr -> nx_dhcpv6_client_FQDN.nx_op_code = NX_DHCPV6_OP_CLIENT_FQDN;

    /* Check domain name lengrh. The total length of a domain name is restricted to 255 octets or less. RFC1035.  */
    if (_nx_utility_string_length_check(domain_name, &name_length, NX_DHCPV6_MAX_NAME_SIZE))
    {
        return(NX_DHCPV6_PARAM_ERROR);
    }

    /* Set the Client FQDN option length. 1 + length of domain name(domain name length + label + terminator) .  */
    dhcpv6_ptr -> nx_dhcpv6_client_FQDN.nx_op_length = (USHORT)(1 + name_length + 2);

    /* Set the Client fully qualified domain name.  */
    dhcpv6_ptr -> nx_dhcpv6_client_FQDN.nx_domain_name = domain_name;
    
    /* Flags Field.  
       0 1 2 3 4 5 6 7
       +-+-+-+-+-+-+-+-+
       |  MBZ    |N|O|S|
       +-+-+-+-+-+-+-+-+
    */
                         
    /* Set the flags.  */   
    if (op == NX_DHCPV6_CLIENT_DESIRES_UPDATE_AAAA_RR)
    {

        /* DHCPv6 Client chooses to update the FQDN-to-IPv6 address mapping for FQDN and address(es) used by the Cslient.
           Set the "S", "O" and "N" bits as 0.  */
        dhcpv6_ptr -> nx_dhcpv6_client_FQDN.nx_flags = 0x00;
    }
    else if (op == NX_DHCPV6_CLIENT_DESIRES_SERVER_DO_DNS_UPDATE)
    {      
        
        /* DHCPv6 Client chooses to update the FQDN-to-IPv6 address mapping for FQDN and address(es) used by the Client to the Server.      
           Set the "S" bit as 1, Set "O" and "N" bits as 0.  */
        dhcpv6_ptr -> nx_dhcpv6_client_FQDN.nx_flags = 0x01;
    }
    else
    {   

        /* DHCPv6 Client choose to request that the Server perform no DNS updates on its behalf.                     
           Set the "N" bit as 1, Set "O" and "S" bits as 0.  */
        dhcpv6_ptr -> nx_dhcpv6_client_FQDN.nx_flags = 0x04;
    }

    /* Yes, register this option request with the DHCPv6 Client. */
    dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request |= NX_DHCPV6_CLIENT_FQDN_OPTION;

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_rebind                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends a rebind request to the server for the Client to*/
/*    obtain or rebind an IP address with a DHCPv6 server on the network. */
/*                                                                        */ 
/*    Note: the Client thread task will detect when it is time to rebind  */
/*    the Client IP address and call the expired address handler if one is*/
/*    set.  The expired address handler is where the host should call this*/ 
/*    service from.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    NX_SUCCESS                         Rebind already initiated         */
/*    status                             Completion status                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request                 Actual DHCPV6 request service    */
/*    tx_timer_activate                  Activate the session timer       */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*    _nx_dhcpv6_thread_entry            DHCPv6 Client thread task        */ 
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
UINT  _nx_dhcpv6_request_rebind(NX_DHCPV6 *dhcpv6_ptr)
{

UINT    status;
UINT    ia_index;
UINT    max_valid_lifetime = 0;


    /* Is the Client state set to REBIND yet? */
    if (dhcpv6_ptr -> nx_dhcpv6_state == NX_DHCPV6_STATE_SENDING_REBIND)
    {

        /* Yes, the rebind process is already started. No need to re-request it. 
           Also must not zero out retry count or reset timeout back to starting value. */
        return NX_SUCCESS;
    }


    /* Set the initial and max retransmission timeouts and max number of retries. */
    dhcpv6_ptr -> nx_dhcpv6_transmission_timeout =  NX_DHCPV6_INIT_REBIND_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_retransmission_count = 0;
    
    /* Set the initaial elasped time of DHCPv6 message.  */    
    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;
        
    /* Set the retransmission information(IRT, MRC, MRT, MRD). */
    dhcpv6_ptr -> nx_dhcpv6_init_retransmission_timeout = NX_DHCPV6_INIT_REBIND_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_count = NX_DHCPV6_MAX_REBIND_RETRANSMISSION_COUNT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_timeout = NX_DHCPV6_MAX_REBIND_RETRANSMISSION_TIMEOUT;
    
    /* Find the max valid lifetime.  */
    for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
    {

        /* Check the address status.  */
        if((dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status == NX_DHCPV6_IA_ADDRESS_STATE_VALID) &&
           (dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_valid_lifetime > max_valid_lifetime))
        {

            /* Find the max valid lifetime of all IAs.  */
            max_valid_lifetime = dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_valid_lifetime;
        }
    }

    /* Set the max duration time.The message exchange is terminated when the valid lifetimes of all the addresses assigned to the IA expire,
       RFC3315, Section18.1.4, Page43.  */
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_duration = max_valid_lifetime;

    /* Activate the session timer to update the elapsed time.  */
    tx_timer_activate(&dhcpv6_ptr -> nx_dhcpv6_session_timer);

    /* Call the internal function and return completion status. */
    status = _nx_dhcpv6_request(dhcpv6_ptr, NX_DHCPV6_STATE_SENDING_REBIND);

    /* Return the actual completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_request_release                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX DHCPv6 Release    */
/*    request service.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Completion status                */ 
/*    NX_PTR_ERROR                       Invalid pointer input            */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_release         Actual DHCPV6 process Release    */ 
/*                                             request service            */
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
UINT  _nxe_dhcpv6_request_release(NX_DHCPV6 *dhcpv6_ptr)
{

UINT status;

    /* Check for invalid pointer input. */
    if (!dhcpv6_ptr)
    {
        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status = _nx_dhcpv6_request_release(dhcpv6_ptr);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_release                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends a release request to the server for the Client  */
/*    to release a assigned IP address back to the DHCPv6 server who      */
/*    previously assigned it.                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Actual completion status         */ 
/*    NX_DHCPV6_IA_ADDRESS_NOT_VALID     Client not assigned an address   */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request                  Actual DHCPV6 request service   */
/*    tx_timer_activate                  Activate the session timer       */
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
UINT  _nx_dhcpv6_request_release(NX_DHCPV6 *dhcpv6_ptr)
{

UINT    status;
UINT    ia_index;

    /* Check if the Client is bound to an assigned address. */
    if((dhcpv6_ptr -> nx_dhcpv6_state != NX_DHCPV6_STATE_BOUND_TO_ADDRESS) &&
       (dhcpv6_ptr -> nx_dhcpv6_state != NX_DHCPV6_STATE_SENDING_RENEW) &&
       (dhcpv6_ptr -> nx_dhcpv6_state != NX_DHCPV6_STATE_SENDING_REBIND))
    {

        /* It is not. The host application should use the nx_dhcpv6_stop if the DHCP Client
           has simply failed to obtain an IP address from the server instead. */

        /* Benign error so return successful outcome. */
        return NX_DHCPV6_IA_ADDRESS_NOT_VALID;
    }

    /* Set the initial and max retransmission timeouts and max number of retries.  */
    dhcpv6_ptr -> nx_dhcpv6_transmission_timeout =  NX_DHCPV6_INIT_RELEASE_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_retransmission_count = 0;
    
    /* Set the initaial elasped time of DHCPv6 message.  */    
    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;
         
    /* Set the retransmission information(IRT, MRC, MRT, MRD). */
    dhcpv6_ptr -> nx_dhcpv6_init_retransmission_timeout = NX_DHCPV6_INIT_RELEASE_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_count = NX_DHCPV6_MAX_RELEASE_RETRANSMISSION_COUNT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_timeout = NX_DHCPV6_MAX_RELEASE_RETRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_duration = NX_DHCPV6_MAX_RELEASE_RETRANSMISSION_DURATION;

    /* Activate the session timer to update the elapsed time.  */
    tx_timer_activate(&dhcpv6_ptr -> nx_dhcpv6_session_timer);
    
    /* Set the DHCPv6 Client IPv6 addresses.  */
    for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
    {

        /* Now remove the address from the IP address table,the client MUST NOT use any of the addresses it is declining 
        as the source address in the Release message or in any subsequently transmitted message, RFC3315,section18.1.7.. */

        /* Verify we have a valid address index. */
        if(dhcpv6_ptr -> nx_dhcpv6_client_address_index[ia_index] >= NX_MAX_IPV6_ADDRESSES) 
        {

            return NX_INVALID_INTERFACE;
        }

        if(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_op_code)
        {
            /* Remove the address from the IP instance address table. */

            nxd_ipv6_address_delete(dhcpv6_ptr -> nx_dhcpv6_ip_ptr, dhcpv6_ptr -> nx_dhcpv6_client_address_index[ia_index]);

            /* Record the address index in the Client record. */
            dhcpv6_ptr -> nx_dhcpv6_client_address_index[ia_index] = 0xFFFFFFFF;
        }
    }

    /* Call the internal function and return completion status. */
    status = _nx_dhcpv6_request(dhcpv6_ptr, NX_DHCPV6_STATE_SENDING_RELEASE);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_renew                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function calls the internal request service to renew the       */
/*    DHCPv6 IP address assigned to it by the server.                     */
/*                                                                        */ 
/*    Note: the Client thread task will detect when it is time to renew   */
/*    the Client IP address and call the deprecated address handler if one*/
/*    is set.  The deprecated address handler is where the host should    */
/*    call this service from.                                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                         Renew already initiated          */
/*    status                             Completion status                */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request                 Actual DHCPV6 request service    */
/*    tx_timer_activate                  Activate the session timer       */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*    _nx_dhcpv6_thread_entry            DHCPv6 Client thread task        */ 
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
UINT  _nx_dhcpv6_request_renew(NX_DHCPV6 *dhcpv6_ptr)
{

UINT status;


    /* Is the Client state set to RENEW yet? */
    if (dhcpv6_ptr -> nx_dhcpv6_state == NX_DHCPV6_STATE_SENDING_RENEW)
    {

        /* Yes, the renew process is already started. No need to re-request it. 
           Also must not zero out retry count or reset timeout back to starting value. */
        return NX_SUCCESS;
    }

    /* Set initial retry and timeout values for sending the Renew request. */
    dhcpv6_ptr -> nx_dhcpv6_retransmission_count = 0;
    dhcpv6_ptr -> nx_dhcpv6_transmission_timeout =  NX_DHCPV6_INIT_RENEW_TRANSMISSION_TIMEOUT;
    
    /* Set the initaial elasped time of DHCPv6 message.  */    
    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;
                
    /* Set the retransmission information(IRT, MRC, MRT, MRD). */
    dhcpv6_ptr -> nx_dhcpv6_init_retransmission_timeout = NX_DHCPV6_INIT_RENEW_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_count = NX_DHCPV6_MAX_RENEW_RETRANSMISSION_COUNT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_timeout = NX_DHCPV6_MAX_RENEW_RETRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_duration = dhcpv6_ptr -> nx_dhcpv6_iana.nx_T2;

    /* Activate the session timer to update the elapsed time.  */
    tx_timer_activate(&dhcpv6_ptr -> nx_dhcpv6_session_timer);

    /* Set the Client to the Renewing state for the DHCP Client thread to process. */
    status = _nx_dhcpv6_request(dhcpv6_ptr, NX_DHCPV6_STATE_SENDING_RENEW);

    /* Actual completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_request_solicit                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX DHCPv6 solicit    */
/*    request service.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_solicit          Actual DHCPV6 process Solicit   */ 
/*                                             request service            */
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
UINT  _nxe_dhcpv6_request_solicit(NX_DHCPV6 *dhcpv6_ptr)
{

UINT status;


    /* Check for invalid pointer input. */
    if (!dhcpv6_ptr)
    {
        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status = _nx_dhcpv6_request_solicit(dhcpv6_ptr);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_solicit                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends a solicit request for the Client to obtain an IP*/
/*    address from any DHCPv6 server on the network.                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Completion status                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request                 Actual DHCPV6 request service    */
/*    tx_timer_activate                  Activate the session timer       */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*    _nx_dhcpv6_process                 Send the DHCPv6 solicit message  */ 
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
UINT  _nx_dhcpv6_request_solicit(NX_DHCPV6 *dhcpv6_ptr)
{

UINT status;


    /* Set the mode of sending the solicit message.  Do not override a rapid option if set. */
    if (dhcpv6_ptr ->nx_dhcpv6_request_solicit_mode != NX_DHCPV6_SOLICIT_RAPID)
    {

        /* Set the mode of sending the solicit message.  */
        dhcpv6_ptr -> nx_dhcpv6_request_solicit_mode = NX_DHCPV6_SOLICIT_NORMAL;
    }

    /* Set the initial and maximum timeout and retries.  */
    dhcpv6_ptr -> nx_dhcpv6_transmission_timeout =  NX_DHCPV6_INIT_SOL_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_retransmission_count = 0;
    
    /* Set the initaial elasped time of DHCPv6 message.  */    
    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;
        
    /* Set the retransmission information(IRT, MRC, MRT, MRD). */
    dhcpv6_ptr -> nx_dhcpv6_init_retransmission_timeout = NX_DHCPV6_INIT_SOL_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_count = NX_DHCPV6_MAX_SOL_RETRANSMISSION_COUNT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_timeout = NX_DHCPV6_MAX_SOL_RETRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_duration = NX_DHCPV6_MAX_SOL_RETRANSMISSION_DURATION;

    /* Reset the preference value if the Client previously accepted an Advertise message. */
    dhcpv6_ptr -> nx_dhcpv6_preference.nx_pref_value = 0;

    /* Activate the session timer to update the elapsed time.  */
    tx_timer_activate(&dhcpv6_ptr -> nx_dhcpv6_session_timer);

    /* Set the Client to the Solicity state for the DHCP Client thread to process. */
    status = _nx_dhcpv6_request(dhcpv6_ptr, NX_DHCPV6_STATE_SENDING_SOLICIT);

    /* Return the completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_request_solicit_rapid                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX DHCPv6 solicit    */
/*    request service with rapid commit option.                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_solicit_rapid    Actual DHCPV6 process Solicit   */ 
/*                                             request service            */
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
UINT  _nxe_dhcpv6_request_solicit_rapid(NX_DHCPV6 *dhcpv6_ptr)
{

UINT status;


    /* Check for invalid pointer input. */
    if (!dhcpv6_ptr)
    {
        return NX_PTR_ERROR;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual service. */
    status = _nx_dhcpv6_request_solicit_rapid(dhcpv6_ptr);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request_solicit_rapid                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sends a solicit request with rapid commit option for  */
/*    the Client to rapidly obtain an IP address from any DHCPv6 server.  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Completion status                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_request                  Actual DHCPV6 request service   */
/*    tx_timer_activate                   Activate the session timer      */
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
UINT  _nx_dhcpv6_request_solicit_rapid(NX_DHCPV6 *dhcpv6_ptr)
{

UINT status;


    /* Set the mode of sending the solicit message.  */
    dhcpv6_ptr ->nx_dhcpv6_request_solicit_mode = NX_DHCPV6_SOLICIT_RAPID;

    /* Set the initial and maximum timeout and retries.  */
    dhcpv6_ptr -> nx_dhcpv6_transmission_timeout =  NX_DHCPV6_INIT_SOL_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_retransmission_count = 0;
    
    /* Set the initaial elasped time of DHCPv6 message.  */    
    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;
        
    /* Set the retransmission information(IRT, MRC, MRT, MRD). */
    dhcpv6_ptr -> nx_dhcpv6_init_retransmission_timeout = NX_DHCPV6_INIT_SOL_TRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_count = NX_DHCPV6_MAX_SOL_RETRANSMISSION_COUNT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_timeout = NX_DHCPV6_MAX_SOL_RETRANSMISSION_TIMEOUT;
    dhcpv6_ptr -> nx_dhcpv6_max_retransmission_duration = NX_DHCPV6_MAX_SOL_RETRANSMISSION_DURATION;

    /* Reset the preference value if the Client previously accepted an Advertise message. */
    dhcpv6_ptr -> nx_dhcpv6_preference.nx_pref_value = 0;

    /* Activate the session timer to update the elapsed time.  */
    tx_timer_activate(&dhcpv6_ptr -> nx_dhcpv6_session_timer);

    /* Set the Client to the Solicity state for the DHCP Client thread to process. */
    status = _nx_dhcpv6_request(dhcpv6_ptr, NX_DHCPV6_STATE_SENDING_SOLICIT);

    /* Return the completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_resume                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the resume NetX DHCPv6     */
/*    client task service.                                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_resume                   Actual DHCPV6 resume function   */ 
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
 UINT  _nxe_dhcpv6_resume(NX_DHCPV6 *dhcpv6_ptr)
{        

UINT    status;


    /* Check for invalid input pointer.  */
    if ((dhcpv6_ptr == NX_NULL) || (dhcpv6_ptr -> nx_dhcpv6_id != NX_DHCPV6_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCPV6 stop service.  */
    status =  _nx_dhcpv6_resume(dhcpv6_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_resume                                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function resumes DHCPV6 processing thread and updates the      */
/*    DHCPv6 client state according to nx_dhcpv6_IP_lifetime_time_accrued.*/ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                             Actual completion status         */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_dhcpv6_start                    Starts DHCPV6 thread task        */ 
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
UINT  _nx_dhcpv6_resume(NX_DHCPV6 *dhcpv6_ptr)
{

UINT      status;            
UCHAR     original_state;

    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex), NX_DHCPV6_MUTEX_WAIT);

    /* Determine if DHCPV6 has already been started.  */
    if (dhcpv6_ptr -> nx_dhcpv6_started)
    {

        /* Release the DHCPv6 Client mutex.  */
        tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* Error, the DHCPV6 client has already been started.  */
        return(NX_DHCPV6_ALREADY_STARTED);
    }

    /* Determine if Client has created its DUID and IANA yet. */
    if ((dhcpv6_ptr -> nx_dhcpv6_ip_ptr == NX_NULL)                 || 
        (dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_option_length == 0) || 
        (dhcpv6_ptr -> nx_dhcpv6_iana.nx_option_length == 0))
    {

        /* Release the DHCPv6 Client mutex.  */
        tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* No, this DHCPv6 Client is not ready to run! */
        return NX_DHCPV6_MISSING_REQUIRED_OPTIONS;
    }       
                    
    /* Remember the original DHCPv6 state for the changed state callback function.  */
    original_state =  dhcpv6_ptr -> nx_dhcpv6_state;

    /* Bind the UDP socket to the DHCPV6 Client port.  */
    status =  nx_udp_socket_bind(&(dhcpv6_ptr -> nx_dhcpv6_socket), NX_DHCPV6_CLIENT_UDP_PORT, TX_WAIT_FOREVER);

    /* Check for success */
    if (status)
    {

        /* Release the DHCPv6 Client mutex.  */
        tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* Return completion status.  */
        return(status); 
    }
            
    /* Check the DHCPv6 Client state. */
    if ((dhcpv6_ptr -> nx_dhcpv6_state != NX_DHCPV6_STATE_BOUND_TO_ADDRESS) &&
        (dhcpv6_ptr -> nx_dhcpv6_state != NX_DHCPV6_STATE_SENDING_RENEW) &&
        (dhcpv6_ptr -> nx_dhcpv6_state != NX_DHCPV6_STATE_SENDING_REBIND))
    {     

        /* Set the DHCPv6 Client state to Init.*/
        dhcpv6_ptr -> nx_dhcpv6_state =  NX_DHCPV6_STATE_INIT;

        /* Clear the session time, lease lifetime, retransmission counters. */
        dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;
        dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued = 0;
        dhcpv6_ptr -> nx_dhcpv6_transmission_timeout = 0;          
        dhcpv6_ptr -> nx_dhcpv6_retransmission_count = 0;            
    }
    else
    {

        /* Check the accrued time to upate the state.  */  
        if ((dhcpv6_ptr -> nx_dhcpv6_state == NX_DHCPV6_STATE_BOUND_TO_ADDRESS) && ((dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued >= dhcpv6_ptr -> nx_dhcpv6_iana.nx_T2)))
        {

            /* Update the status as NX_DHCPV6_STATE_SENDING_RENEW to send rebinding message in _nx_dhcpv6_thread_entry function.  */
            dhcpv6_ptr -> nx_dhcpv6_state = NX_DHCPV6_STATE_SENDING_RENEW;
        }  
    }
          

    /* Set the DHCPv6 started flag to indicate DHCPv6 Client is now running.  */
    dhcpv6_ptr -> nx_dhcpv6_started =  NX_TRUE;

    /* Activate the DHCPv6 timers.  */
    tx_timer_activate(&dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_timer);   

    /* Resume the DHCPv6 processing thread.  */
    tx_thread_resume(&(dhcpv6_ptr -> nx_dhcpv6_thread));  
                      
    /* Is there a state change callback registered with the Client?  */
    if ((dhcpv6_ptr -> nx_dhcpv6_state_change_callback) && (original_state != (dhcpv6_ptr -> nx_dhcpv6_state)))
    {

        /* Yes, call the the state change callback with the original and new state.  */
        (dhcpv6_ptr -> nx_dhcpv6_state_change_callback)(dhcpv6_ptr, original_state, dhcpv6_ptr -> nx_dhcpv6_state);
    }

    /* Release the DHCPv6 Client mutex.  */
    tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

    /* Return completion status.  */
    return(status);  
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_send_request                             PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*   This function sends a DHCPV6 request to the server.  All options     */
/*   required for each Client DHCPv6 message are compiled into the Client */
/*   request package. Since each message must have a unique transaction   */
/*   ID, it is up to the caller to set the Client transaction ID in the   */
/*   Client record before calling this service.  The server reply must    */
/*   contain the matching message transaction ID for each message type the*/ 
/*   Client sends it. This function will check for invalid message types. */ 
/*                                                                        */ 
/*   To send a message, the Client allocates a packet from its own packet */
/*   pool.  It then must set the IPv6 packet interface depending if the   */
/*   Client has a valid global IP address or is using its link local      */
/*   address till it can get a global IP address.  It can send the packet */
/*   either to all servers on its network or include server and relays    */
/*   (user configurable).                                                 */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                            Pointer to DHCPV6 Client      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Actual completion status      */ 
/*    NX_DHCPV6_ILLEGAL_MESSAGE_TYPE        Illegal message type to send  */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate a DHCPV6 packet      */ 
/*    nx_packet_release                     Release DHCPV6 packet         */ 
/*    nx_udp_socket_send                    Send DHCPV6 packet            */ 
/*   _nx_dhcpv6_add_client_duid             Add Client DUID to request    */ 
/*   _nx_dhcpv6_add_server_duid             Add server DUID to request    */
/*   _nx_dhcpv6_add_elapsed_time            Add elapsed time option       */
/*   _nx_dhcpv6_add_iana                    Add IANA option to request    */
/*   _nx_dhcpv6_add_ia_address              Add IA address option         */
/*   _nx_dhcpv6_add_option_request          Add option request option     */
/*    memcpy                                Copy specified area of memory */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process                   Process DHCPv6 client request  */ 
/*    _nx_dhcpv6_waiting_on_reply          Process replies to valid server*/ 
/*                                                 reply received         */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            verified memcpy use cases,  */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), supported*/
/*                                            adding user options,        */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcpv6_send_request(NX_DHCPV6 *dhcpv6_ptr)
{

NX_PACKET   *packet_ptr;
UCHAR       *buffer;  
UINT        status;
ULONG       message_word;
UINT        index; 
NX_INTERFACE     *interface_ptr;
NXD_IPV6_ADDRESS *ipv6_address;
ULONG             available_payload;
UCHAR            *user_option_ptr;
UINT              user_option_length;


    /* Initialize local variables. */
    index = 0;

    /* Check for invalid or illegal message type. */
    if ((dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_ADVERTISE)     || 
        (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_REPLY)         ||
        (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == NX_DHCPV6_MESSAGE_TYPE_RECONFIGURE)   ||
        (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type > NX_DHCPV6_MESSAGE_TYPE_INFORM_REQUEST) ||
        (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type == 0))
    {

        /* Return error status. */
        return NX_DHCPV6_ILLEGAL_MESSAGE_TYPE;
    }

    /* Allocate a DHCPV6 packet.  */
    status =  nx_packet_allocate(dhcpv6_ptr -> nx_dhcpv6_pool_ptr, &packet_ptr, NX_IPv6_UDP_PACKET, NX_DHCPV6_PACKET_TIME_OUT);

    /* Was the packet allocation successful?  */
    if (status != NX_SUCCESS)
    {

        /* Return status.  */
        return(status);
    }

    /* Verify packet payload. */
    if ((packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr) < 4)
    {
        nx_packet_release(packet_ptr);
        return(NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD);
    }

    /* Indicate this is an IPv6 packet. */
    packet_ptr -> nx_packet_ip_version = NX_IP_VERSION_V6;

    /* Setup the buffer pointer.  */
    buffer =  packet_ptr -> nx_packet_prepend_ptr;

    /* Use the message id value to judge whether the next packet is retransmit packet or not.  
       If the id value is 0, indicate the new dhcpv6 request, then generate a random number, else,leave the transaction  ID unchanged.
       A client MUST leave the transaction ID unchanged in retransmissions of a message,RFC3315,section 15.1,page 28.  */
    if(dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid == 0)
    {

        /* Generate "random" transaction ID using physical address and a random factor for each request
        to the Server.  The Server must include a matching transaction ID in its reply. */    
        dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid =  
            ((dhcpv6_ptr -> nx_dhcpv6_ip_ptr -> nx_ip_arp_physical_address_msw) ^ 
            (dhcpv6_ptr -> nx_dhcpv6_ip_ptr -> nx_ip_arp_physical_address_lsw) ^ 
            (ULONG)(NX_RAND()));

        /* Now reduce it to three bytes. */
        dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid = (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid & (ULONG)(0x0ffffff));
    }

    /* Clear memory to make the message header. */
    memset(&message_word, 0, sizeof(ULONG));

    /* Add the message type and transaction ID as the DHCPv6 header fields. */
    message_word = (((ULONG)(dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type)) << 24); 

    message_word |= (((ULONG)(0x0ffffff)) & (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid));

    /* Adjust for endianness. */
    NX_CHANGE_ULONG_ENDIAN(message_word);

    /* Copy the message header to the packet buffer. */
    memcpy(buffer, &message_word, 4); /* Use case of memcpy is verified. */

    /* Update the buffer 'pointer'. */
    index += (ULONG)sizeof(ULONG);

    /* Add the Client DUID to the packet buffer. */
    status = _nx_dhcpv6_add_client_duid(dhcpv6_ptr, buffer, &index);

    /* Check for error. */
    if(status != NX_SUCCESS)
    {

        nx_packet_release(packet_ptr);

        return status;
    }

    /* Determine if any additional options need to be added relative to the DHCPV6 message type.  */
    switch (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type)
    {

        case NX_DHCPV6_MESSAGE_TYPE_SOLICIT:
        {
            /* Append the IA-NA option and depending on host state. */
            status = _nx_dhcpv6_add_iana(dhcpv6_ptr, buffer, &index);

            /* Check for internal errors. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return the error status and abort. */
                return status;
            }

            /* Compute the session time so far and add an elapsed time option. */
            status = _nx_dhcpv6_add_elapsed_time(dhcpv6_ptr, buffer, &index);

            /* Check for internal errors. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return the error status and abort. */
                return status;
            }
                
            /* Did the Client ask for a DHCPv6 Client FQDN option ?  
               A client MUST only include the Client FQDN option in SOLICIT, REQUEST, RENEW, or REBIND messsage.  */
            if (dhcpv6_ptr ->nx_dhcpv6_client_FQDN.nx_op_code)
            {

                /* Add the option request. */
                status = _nx_dhcpv6_add_client_FQDN(dhcpv6_ptr, buffer, &index); 

                /* Check for internal error. */
                if (status != NX_SUCCESS)
                {

                    nx_packet_release(packet_ptr);

                    /* Return the error status and abort. */
                    return status;
                }
            }

            /* Did the Client ask for a requested option? */
            if (dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request)
            {

                /* Add the option request. */
                status = _nx_dhcpv6_add_option_request(dhcpv6_ptr, buffer, &index); 

                /* Check for internal error. */
                if (status != NX_SUCCESS)
                {

                    nx_packet_release(packet_ptr);

                    /* Return the error status and abort. */
                    return status;
                }
            }

            /* Did the Client ask for rapid commit option? */
            if (dhcpv6_ptr -> nx_dhcpv6_request_solicit_mode == NX_DHCPV6_SOLICIT_RAPID)
            {

                /* Compute the available payload for DHCP data in the packet buffer. */
                available_payload = (ULONG)(dhcpv6_ptr -> nx_dhcpv6_pool_ptr -> nx_packet_pool_payload_size -
                                            sizeof(NX_IPV6_HEADER) - sizeof(NX_UDP_HEADER) - index);

                /* Check if the data will fit in the packet buffer. */
                if (available_payload < sizeof(ULONG))
                {

                    /* Hmmm... not enough! Can't do it. */
                    return NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD;
                }

                /* Add the rapid commit option request. */
                /* Clear memory to make the message header, and add the message type.  */
                memset(&message_word, 0, sizeof(ULONG));

                /* Add the message type. */
                message_word = (((ULONG)NX_DHCPV6_OP_RAPID_COMMIT) << 16); 

                /* Adjust for endianness. */
                NX_CHANGE_ULONG_ENDIAN(message_word);

                /* Copy the message to the packet buffer. */
                memcpy((buffer + index), &message_word, sizeof(ULONG)); /* Use case of memcpy is verified. */

                /* Update the index.  */
                index += (ULONG)sizeof(ULONG);
            }

            /* Increment the number of Solicit messages sent.  */
            dhcpv6_ptr -> nx_dhcpv6_solicitations_sent++;

            break;
        }

        case NX_DHCPV6_MESSAGE_TYPE_REQUEST:
        {
            /* Compute the session time so far and add an elapsed time option. */
            status = _nx_dhcpv6_add_elapsed_time(dhcpv6_ptr, buffer, &index);

            /* Check for internal error. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return error status. */
                return status;
            }      

            /* Add the server DUID just extracted from the Server Advertise message. */
            status = _nx_dhcpv6_add_server_duid(dhcpv6_ptr, buffer, &index);

            /* Check for internal error. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return error status. */
                return status;
            }

            /* Append an IA-NA with the IA address option received from server added to the IA-NA option. */
            status = _nx_dhcpv6_add_iana(dhcpv6_ptr, buffer, &index);

            /* Check for internal error. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return error status. */
                return status;
            }
                
            /* Did the Client ask for a DHCPv6 Client FQDN option ?  
               A client MUST only include the Client FQDN option in SOLICIT, REQUEST, RENEW, or REBIND messsage.  */
            if (dhcpv6_ptr ->nx_dhcpv6_client_FQDN.nx_op_code)
            {

                /* Add the option request. */
                status = _nx_dhcpv6_add_client_FQDN(dhcpv6_ptr, buffer, &index); 

                /* Check for internal error. */
                if (status != NX_SUCCESS)
                {

                    nx_packet_release(packet_ptr);

                    /* Return the error status and abort. */
                    return status;
                }
            }
              
            /* Did the Client ask for a requested option? */
            if (dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request)
            {
                /* Add the server DUID just extracted from the Server Advertise message. */
                status = _nx_dhcpv6_add_option_request(dhcpv6_ptr, buffer, &index); 

                /* Check for internal error. */
                if (status != NX_SUCCESS)
                {

                    nx_packet_release(packet_ptr);

                    /* Return error status. */
                    return status;
                }
            }

            /* Increment the number of Request messages sent.  */
            dhcpv6_ptr -> nx_dhcpv6_requests_sent++;

            break;
        }

        case NX_DHCPV6_MESSAGE_TYPE_RENEW:
        {
            /* Compute the session time so far and add an elapsed time option. */
            status = _nx_dhcpv6_add_elapsed_time(dhcpv6_ptr, buffer, &index);

            /* Check for internal error. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return error status. */
                return status;
            }

            /* Add the server DUID just extracted from the Server Advertise message. */
            status = _nx_dhcpv6_add_server_duid(dhcpv6_ptr, buffer, &index);

            /* Check for internal error. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return error status. */
                return status;
            }

            status = _nx_dhcpv6_add_iana(dhcpv6_ptr, buffer, &index);

            /* Check for internal errors. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return the error status and abort. */
                return status;
            }        

            /* Did the Client ask for a DHCPv6 Client FQDN option ?  
               A client MUST only include the Client FQDN option in SOLICIT, REQUEST, RENEW, or REBIND messsage.  */
            if (dhcpv6_ptr ->nx_dhcpv6_client_FQDN.nx_op_code)
            {

                /* Add the option request. */
                status = _nx_dhcpv6_add_client_FQDN(dhcpv6_ptr, buffer, &index); 

                /* Check for internal error. */
                if (status != NX_SUCCESS)
                {

                    nx_packet_release(packet_ptr);

                    /* Return the error status and abort. */
                    return status;
                }
            }
            
            /* A client MAY include an Option Request option in a Solicit, Request, Renew, Rebind, Confirm or Information-request
               message to inform the server about options the client wants the server to send to the client. RFC3315, Section22.7, Page78.*/
            if (dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request)
            {
                /* Add the server DUID just extracted from the Server Advertise message. */
                status = _nx_dhcpv6_add_option_request(dhcpv6_ptr, buffer, &index); 

                /* Check for internal error. */
                if (status != NX_SUCCESS)
                {

                    nx_packet_release(packet_ptr);

                    /* Return error status. */
                    return status;
                }
            }

            dhcpv6_ptr -> nx_dhcpv6_renews_sent++;

            break;
        }

        case NX_DHCPV6_MESSAGE_TYPE_REBIND:
        {
            /* Compute the session time so far and add an elapsed time option. */
            status = _nx_dhcpv6_add_elapsed_time(dhcpv6_ptr, buffer, &index);

            /* Check for internal errors. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return the error status and abort. */
                return status;
            }

            /* Rebind message includes the current IA/IA-NA, current IA address. */
            status = _nx_dhcpv6_add_iana(dhcpv6_ptr, buffer, &index);

            /* Check for internal errors. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return the error status and abort. */
                return status;
            }
                      
            /* Did the Client ask for a DHCPv6 Client FQDN option ?  
               A client MUST only include the Client FQDN option in SOLICIT, REQUEST, RENEW, or REBIND messsage.  */
            if (dhcpv6_ptr ->nx_dhcpv6_client_FQDN.nx_op_code)
            {

                /* Add the option request. */
                status = _nx_dhcpv6_add_client_FQDN(dhcpv6_ptr, buffer, &index); 

                /* Check for internal error. */
                if (status != NX_SUCCESS)
                {

                    nx_packet_release(packet_ptr);

                    /* Return the error status and abort. */
                    return status;
                }
            }

            /* Did the Client ask for a requested option? */
            if (dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request)
            {

                /* Add the server DUID just extracted from the Server Advertise message. */
                status = _nx_dhcpv6_add_option_request(dhcpv6_ptr, buffer, &index); 

                /* Check for internal errors. */
                if (status != NX_SUCCESS)
                {

                    nx_packet_release(packet_ptr);

                    /* Return the error status and abort. */
                    return status;
                }
            }

            /* Increment the number of Solicit messages sent.  */
            dhcpv6_ptr -> nx_dhcpv6_rebinds_sent++;

            break;
        }

        case NX_DHCPV6_MESSAGE_TYPE_CONFIRM:
        {
            /* Compute the session time so far and add an elapsed time option. */
            status = _nx_dhcpv6_add_elapsed_time(dhcpv6_ptr, buffer, &index);

            /* Check for internal errors. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return the error status and abort. */
                return status;
            }

            /* Rebind message includes the current IA/IA-NA, current IA address. */
            status = _nx_dhcpv6_add_iana(dhcpv6_ptr, buffer, &index);

            /* Check for internal errors. */
            if (status != NX_SUCCESS)
            {

                /* Return the error status and abort. */
                return status;
            }
             
            /* A client MAY include an Option Request option in a Solicit, Request, Renew, Rebind, Confirm or Information-request
               message to inform the server about options the client wants the server to send to the client. RFC3315, Section22.7, Page78.*/
            if (dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request)
            {
                /* Add the server DUID just extracted from the Server Advertise message. */
                status = _nx_dhcpv6_add_option_request(dhcpv6_ptr, buffer, &index); 

                /* Check for internal error. */
                if (status != NX_SUCCESS)
                {

                    nx_packet_release(packet_ptr);

                    /* Return error status. */
                    return status;
                }
            }

            /* Increment the number of Solicit messages sent.  */
            dhcpv6_ptr -> nx_dhcpv6_confirms_sent++;

            break;
        }

        case NX_DHCPV6_MESSAGE_TYPE_DECLINE:
        {

            /* Add the server DUID just extracted from the Server Advertise message. */
            status = _nx_dhcpv6_add_server_duid(dhcpv6_ptr, buffer, &index);

            /* Check for internal error. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return error status. */
                return status;
            }

            /* Compute the session time so far and add an elapsed time option. */
            status = _nx_dhcpv6_add_elapsed_time(dhcpv6_ptr, buffer, &index);

            /* Check for internal errors. */
            if (status != NX_SUCCESS)
            {

                /* Return the error status and abort. */
                return status;
            }

            /* Decling the message in the current IA/IA-NA, current IA address. */
            status = _nx_dhcpv6_add_iana(dhcpv6_ptr, buffer, &index);
            
            /* Check for internal errors. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return the error status and abort. */
                return status;
            }

            /* Increment the number of Decline messages sent.  */
            dhcpv6_ptr -> nx_dhcpv6_declines_sent++;

            break;
        }

        case NX_DHCPV6_MESSAGE_TYPE_RELEASE:
        {

            /* Add the server DUID just extracted from the Server Advertise message. */
            status = _nx_dhcpv6_add_server_duid(dhcpv6_ptr, buffer, &index);

            /* Check for internal error. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return error status. */
                return status;
            }

            /* Compute the session time so far and add an elapsed time option. */
            status = _nx_dhcpv6_add_elapsed_time(dhcpv6_ptr, buffer, &index);

            /* Check for internal errors. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return the error status and abort. */
                return status;
            }                      

            /* Decling the message in the current IA/IA-NA, current IA address. */
            status = _nx_dhcpv6_add_iana(dhcpv6_ptr, buffer, &index);

            /* Check for internal errors. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return the error status and abort. */
                return status;
            }

            /* Increment the number of Releases messages sent.  */
            dhcpv6_ptr -> nx_dhcpv6_releases_sent++;

            break;
        }

        case NX_DHCPV6_MESSAGE_TYPE_INFORM_REQUEST:
        {

            /* Did the Client ask for a requested option? */
            if (!dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request)
            {

                /* No, so don't bother sending out the request. */

                nx_packet_release(packet_ptr);

                return NX_SUCCESS;
            }

            /* Compute the session time so far and add an elapsed time option. */
            status = _nx_dhcpv6_add_elapsed_time(dhcpv6_ptr, buffer, &index);

            /* Check for internal error. */
            if (status != NX_SUCCESS)
            {

                nx_packet_release(packet_ptr);

                /* Return error status. */
                return status;
            }

            /* Did the Client ask for a requested option? */
            if (dhcpv6_ptr -> nx_dhcpv6_option_request.nx_op_request)
            {
                /* Add the server DUID just extracted from the Server Advertise message. */
                status = _nx_dhcpv6_add_option_request(dhcpv6_ptr, buffer, &index); 

                /* Check for internal error. */
                if (status != NX_SUCCESS)
                {

                    nx_packet_release(packet_ptr);

                    /* Return error status. */
                    return status;
                }
            }

            /* Increment the number of Inform Request messages sent.  */
            dhcpv6_ptr -> nx_dhcpv6_inform_req_sent++;

            break;
        }

        default:
        {
            nx_packet_release(packet_ptr);

            return NX_DHCPV6_UNKNOWN_MSG_TYPE;
        }
    }

    /* Add any user supplied options to the buffer.  */
    if (dhcpv6_ptr -> nx_dhcpv6_user_option_add)
    {

        /* Set the pointer for adding user option.  */
        user_option_ptr = buffer + index;

        /* Calculate the available length for user options.  */
        user_option_length = (UINT)(packet_ptr -> nx_packet_data_end - user_option_ptr);

        /* Add the specific DHCP option user wanted.  */
        if (dhcpv6_ptr -> nx_dhcpv6_user_option_add(dhcpv6_ptr, dhcpv6_ptr -> nx_dhcpv6_client_interface_index, dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_type, user_option_ptr, &user_option_length) == NX_TRUE)
        {

            /* Update the index to include the user options.  */
            index += user_option_length;
        }
        else
        {

            /* Invalid user options. Release the packet.  */
            nx_packet_release(packet_ptr);
            return(NX_DHCPV6_UNKNOWN_OPTION);
        }
    }

    /* Setup the packet pointers.  */
    packet_ptr -> nx_packet_length = index;

    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + index;

    /* Select Client Source address:
       1. When a client sends a DHCP message to the All_DHCP_Relay_Agents_and_Servers address,
          the client must use a link-local address assigned to the interface for which it
          is requesting configuration as the source address.
       2. When a client sends a DHCP message directly to a server using unicast, 
          the source address must be an address assigned to the interface for which the 
          client is interested in obtaining configuration.
       RFC 3315, Section 16. Client Source Address and Interface Selection, Page 32.  */

    /* Set the interface pointer.  */
    interface_ptr = &(dhcpv6_ptr -> nx_dhcpv6_ip_ptr -> nx_ip_interface[dhcpv6_ptr ->nx_dhcpv6_client_interface_index]);

    /* Check the destination address.  */
    if (CHECK_IPV6_ADDRESSES_SAME(dhcpv6_ptr -> nx_dhcpv6_client_destination_address.nxd_ip_address.v6, All_DHCPv6_Relay_Servers_Address.nxd_ip_address.v6))
    {

        /* Find the link-local address as source adress. Get first address from interface.  */
        ipv6_address = interface_ptr -> nxd_interface_ipv6_address_list_head;

        /* Loop to check the address.  */
        while (ipv6_address)
        {

            /* Check for a valid address. */
            if (ipv6_address -> nxd_ipv6_address_state != NX_IPV6_ADDR_STATE_VALID)
            {
                ipv6_address = ipv6_address -> nxd_ipv6_address_next;
            }

            /* Check for link-local address. */
            else if (IPv6_Address_Type(ipv6_address -> nxd_ipv6_address) & IPV6_ADDRESS_LINKLOCAL)
            {

                /* Find the Link-Local Address.  */
                break;
            }
            else
            {
                ipv6_address = ipv6_address -> nxd_ipv6_address_next;
            }
        }

        /* Check if found the link-local address.  */
        if(ipv6_address == NX_NULL)
        {

            /* No valid link-local address.  */
            nx_packet_release(packet_ptr);
            return(NX_IP_ADDRESS_ERROR);
        }
    }
    else
    {

        /* Find the source address.  */
        status = _nxd_ipv6_interface_find(dhcpv6_ptr -> nx_dhcpv6_ip_ptr, dhcpv6_ptr -> nx_dhcpv6_client_destination_address.nxd_ip_address.v6,
                                          &ipv6_address, interface_ptr);

        /* Check if found the address.  */
        if (status)
        {

            /* No valid source address.  */
            nx_packet_release(packet_ptr);
            return(NX_IP_ADDRESS_ERROR);
        }
    }

    /* Send the packet out! */
    status = _nxd_udp_socket_source_send(&(dhcpv6_ptr -> nx_dhcpv6_socket), packet_ptr, 
                                         &dhcpv6_ptr -> nx_dhcpv6_client_destination_address, NX_DHCPV6_SERVER_UDP_PORT, 
                                         ipv6_address -> nxd_ipv6_address_index);

    /* If an error is detected, the packet was not sent and we have to release the packet. */
    if (status != NX_SUCCESS)
    {

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
    }

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_session_timeout_entry                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function updates the Client session duration time with the     */
/*    Server. This information is used for the Elapsed Time data in DHCPv6*/
/*    Client messages sends to the server as part of the DHCPv6 protocol. */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr_value                      Pointer to the DHCPV6 Client  */ 
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
VOID  _nx_dhcpv6_session_timeout_entry(ULONG dhcpv6_ptr_value)
{    

NX_DHCPV6 *dhcpv6_ptr;


    /* Setup DHCPv6 Client pointer.  */
    NX_TIMER_EXTENSION_PTR_GET(dhcpv6_ptr, NX_DHCPV6, dhcpv6_ptr_value)

    /* Check the elapsed time, the time is expressed in hundredths of 1 second, 65500 is the largest time value.  */
    if(dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time < 65500)
    {

        /* Update the elapsed time of current DHCP session. 
           This time is expressed in hundredths of a second.  */
        dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = (USHORT)(dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time + NX_DHCPV6_SESSION_TIMER_INTERVAL * 100);
    }
    else
    {

        /* Set the value 0xFFFF to represent any elapsed time values greater than the largest time value . */
        dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0xFFFF;
    }
    return;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_set_time_accrued                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the set time accrued on    */
/*    Client IP lease service.                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    time_accrued                        Pointer to time since IP lease  */
/*                                            assigned (secs)             */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_set_time_accrued       Actual set time accrued on Client */
/*                                          IP address lease service      */ 
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
UINT _nxe_dhcpv6_set_time_accrued(NX_DHCPV6 *dhcpv6_ptr, ULONG time_accrued)
{

UINT status;


    /* Check for valid pointer input. */
    if (!dhcpv6_ptr)
    {
        return NX_PTR_ERROR;
    }

    /* Call the actual service. */
    status =  _nx_dhcpv6_set_time_accrued(dhcpv6_ptr, time_accrued);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_set_time_accrued                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function enables the host to set the time accrued on the       */
/*    Client's IP address lease.  The caller must first suspend the       */
/*    DHCPv6 Client using the nx_dhcpv6_suspend service before using this */
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    time_accrued                        Pointer to time to set the      */ 
/*                                            lient accrued time (secs)   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT _nx_dhcpv6_set_time_accrued(NX_DHCPV6 *dhcpv6_ptr, ULONG time_accrued)
{


    /* Check if the DHCPv6 Client has been started. */
    if (dhcpv6_ptr -> nx_dhcpv6_started)
    {
        /* It has.  We cannot set the time accrued if the DHCPv6 client thread is running. */
        return NX_DHCPV6_ALREADY_STARTED;
    }

    /* Set the time accrued on the Client IP address lease. */
    dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued = time_accrued;

    /* Return successful completion. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_client_set_interface                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the set client interface   */
/*    service.                                                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    interface_index                     Index for DHCP network interface*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */ 
/*    NX_INVALID_INTERFACE              Invalid interface index input     */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_client_set_interface   Actual set interface service      */ 
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
UINT _nxe_dhcpv6_client_set_interface(NX_DHCPV6 *dhcpv6_ptr, UINT interface_index)
{

UINT status;


    /* Check for valid pointer input. */
    if (!dhcpv6_ptr)
    {
        return NX_PTR_ERROR;
    }

    /* Check for an invalid interface specified. */
    if (interface_index >= NX_MAX_PHYSICAL_INTERFACES)
    {
        return NX_INVALID_INTERFACE;
    }

    /* Call the actual service. */
    status =  _nx_dhcpv6_client_set_interface(dhcpv6_ptr, interface_index);

    /* Return completion status. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_client_set_interface                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function specifies the physical interface for DHCP             */
/*    communications with the server in NetX Duo environments that support*/
/*    mult-homed hosts.                                                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    interface_index                     Index for DHCP network interface*/
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*    NX_NOT_ENABLED                      NetXDuo version does not support*/
/*                                           multiple network interfaces  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT _nx_dhcpv6_client_set_interface(NX_DHCPV6 *dhcpv6_ptr, UINT interface_index)
{


    /* Set the client interface. */
    dhcpv6_ptr -> nx_dhcpv6_client_interface_index = interface_index;

    /* Return successful completion. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_client_set_destination_address          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the set destination        */
/*    address service.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */   
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    destination_address                 Destination address             */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*    NX_PTR_ERROR                      Invalid pointer input             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_client_set_destination_address                           */ 
/*                                      Actual set address service        */ 
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
UINT _nxe_dhcpv6_client_set_destination_address(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *destination_address)
{

UINT status;


    /* Check for invalid input pointer.  */
    if ((dhcpv6_ptr == NX_NULL) || (dhcpv6_ptr -> nx_dhcpv6_id != NX_DHCPV6_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for an invalid address type. */
    if (destination_address -> nxd_ip_version != NX_IP_VERSION_V6)
    {

        /* Invalid address type.  */
        return(NX_DHCPV6_PARAM_ERROR);
    }

    /* Check if the server address is unspecified (::). */
    if(CHECK_UNSPECIFIED_ADDRESS(destination_address -> nxd_ip_address.v6))
    {

        /* Null address input. */         
        return(NX_DHCPV6_PARAM_ERROR);
    }

    /* Call the actual service. */
    status =  _nx_dhcpv6_client_set_destination_address(dhcpv6_ptr, destination_address);

    /* Return completion status. */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_client_set_destination_address           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function set the destination address where DHCP message should */
/*    be sent. By default is All_DHCP_Relay_Agents_and_Servers(FF02::1:2).*/
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */ 
/*    destination_address                 Destination address             */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
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
UINT _nx_dhcpv6_client_set_destination_address(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *destination_address)
{


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex), NX_WAIT_FOREVER);

    /* Set the destination address.  */
    COPY_NXD_ADDRESS(destination_address, &(dhcpv6_ptr -> nx_dhcpv6_client_destination_address));

    /* Release the DHCPv6 Client mutex.  */
    tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

    /* Return successful completion. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_start                                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the NetX start the dhcpv6  */
/*    client service.                                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_start                    Actual DHCPV6 start function    */ 
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
UINT  _nxe_dhcpv6_start(NX_DHCPV6 *dhcpv6_ptr)
{

UINT    status;


    /* Check for invalid input pointer.  */
    if ((dhcpv6_ptr == NX_NULL) || (dhcpv6_ptr -> nx_dhcpv6_id != NX_DHCPV6_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCPV6 start service.  */
    status =  _nx_dhcpv6_start(dhcpv6_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_start                                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function starts the DHCPV6 processing thread and readies the   */
/*    DHCPv6 Client to initiate DHCPv6 requests.  The Client state is set */
/*    to INIT and awaits the host application requesting a DHCPv6 meesage */
/*    sent e.g. the SOLICIT message. For hosts binding (or attempting to  */
/*    obtain) a IP address in a previous session, this will clear session */
/*    parameters such as accrued lease time and message retry count.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_DHCPV6_ALREADY_STARTED           DHCPv6 Client already started   */ 
/*    NX_DHCPV6_MISSING_REQUIRED_OPTIONS  Client missing required options */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_create                  Create the Client UDP socket  */
/*    nx_udp_socket_bind                    Bind the Client socket        */ 
/*    nx_udp_socket_unbind                  Unbind the Client socket      */ 
/*    tx_timer_activate                     Activate Client timers        */
/*    tx_timer_deactivate                   Deactivate Client timers      */
/*    tx_thread_resume                      Resume Client thread task     */ 
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
UINT  _nx_dhcpv6_start(NX_DHCPV6 *dhcpv6_ptr)
{
                 
UINT        status;


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex), NX_DHCPV6_MUTEX_WAIT);

    /* Determine if DHCPV6 has already been started.  */
    if (dhcpv6_ptr -> nx_dhcpv6_started)
    {

        /* Release the DHCPv6 Client mutex.  */
        tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* Error, the DHCPV6 client has already been started.  */
        return(NX_DHCPV6_ALREADY_STARTED);
    }

    /* Determine if Client has created its DUID and IANA yet. */
    if ((dhcpv6_ptr -> nx_dhcpv6_ip_ptr == NX_NULL)                 || 
        (dhcpv6_ptr -> nx_dhcpv6_client_duid.nx_option_length == 0) || 
        (dhcpv6_ptr -> nx_dhcpv6_iana.nx_option_length == 0))
    {

        /* Release the DHCPv6 Client mutex.  */
        tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* No, this DHCPv6 Client is not ready to run! */
        return NX_DHCPV6_MISSING_REQUIRED_OPTIONS;
    }

    /* Set the DHCPv6 Client state to Init.*/
    dhcpv6_ptr -> nx_dhcpv6_state =  NX_DHCPV6_STATE_INIT;

    /* Clear the session time, lease lifetime, retransmission counters. */
    dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time = 0;
    dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued = 0;
    dhcpv6_ptr -> nx_dhcpv6_transmission_timeout = 0;          
    dhcpv6_ptr -> nx_dhcpv6_retransmission_count = 0;            

    /* Bind the UDP socket to the DHCPV6 Client port.  */
    status =  nx_udp_socket_bind(&(dhcpv6_ptr -> nx_dhcpv6_socket), NX_DHCPV6_CLIENT_UDP_PORT, TX_WAIT_FOREVER);

    /* Check for success */
    if (status == NX_SUCCESS)
    {
    
        /* Resume the DHCPv6 processing thread.  */
        status =  tx_thread_resume(&(dhcpv6_ptr -> nx_dhcpv6_thread));
    
        /* Determine if the resume was successful.  */
        if (status == NX_SUCCESS)
        {
    
            /* Set the DHCPv6 started flag to indicate DHCPv6 Client is now running.  */
            dhcpv6_ptr -> nx_dhcpv6_started =  NX_TRUE;

            /* Activate the DHCPv6 timers.  */
            tx_timer_activate(&dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_timer);
        }
        else
        {
    
            /* Error, unbind the DHCPv6 socket.  */
            nx_udp_socket_unbind(&(dhcpv6_ptr -> nx_dhcpv6_socket));
        }
    }

    /* Release the DHCPv6 Client mutex.  */
    tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

    /* Return completion status.  */
    return(status);  
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_suspend                                 PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the suspend NetX DHCPv6    */
/*    client task service.                                                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                         Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_PTR_ERROR                        Invalid pointer input           */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_suspend                  Actual DHCPV6 suspend function  */ 
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
UINT  _nxe_dhcpv6_suspend(NX_DHCPV6 *dhcpv6_ptr)
{

UINT    status;


    /* Check for invalid input pointer.  */
    if ((dhcpv6_ptr == NX_NULL) || (dhcpv6_ptr -> nx_dhcpv6_id != NX_DHCPV6_ID))
    {
    
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCPV6 stop service.  */
    status =  _nx_dhcpv6_suspend(dhcpv6_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_suspend                                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function suspends (stops) DHCPV6 processing thread, stops the  */
/*    DHCP session and IP address lease timers, and clears the started    */
/*    status of the DHCPv6 Client. It resets the DHCPv6 Client to the     */
/*    INIT.  The DHCP socket is unbound and the DHCP thread aborted.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*    NX_DHCPV6_NOT_STARTED               Task not running; can't be      */
/*                                               suspended                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_thread_suspend                     Suspend DHCPV6 thread task    */ 
/*    tx_timer_deactivate                   Deactivate timer              */ 
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
UINT  _nx_dhcpv6_suspend(NX_DHCPV6 *dhcpv6_ptr)
{

UINT status;


    status = _nx_dhcpv6_stop(dhcpv6_ptr);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_stop                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the DHCPv6 Client stop     */ 
/*    service.                                                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                            Actual completion status          */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_stop                   Actual stop DHCPV6 thread task    */ 
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
UINT  _nxe_dhcpv6_stop(NX_DHCPV6 *dhcpv6_ptr)
{        

UINT    status;


    /* Check for invalid input pointer.  */
    if ((dhcpv6_ptr == NX_NULL) || (dhcpv6_ptr -> nx_dhcpv6_id != NX_DHCPV6_ID))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual DHCPV6 stop service.  */
    status =  _nx_dhcpv6_stop(dhcpv6_ptr);

    /* Return status.  */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_stop                                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function suspends (stops) DHCPV6 processing thread, stops the  */
/*    DHCP session and IP address lease timers. The DHCP socket is unbound*/
/*    and the DHCP thread aborted.                                        */ 
/*                                                                        */ 
/*    Stopping the DHCP Client is necessary to restart the DHCP client,   */
/*    example if a previously assigned address is declined or released,   */ 
/*    and the host would like to solicit an IP address again.             */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                        Pointer to DHCPV6 Client instance */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                          Successful completion status    */ 
/*    NX_DHCPV6_NOT_STARTED               Task not running; can't be      */
/*                                               suspended                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_thread_suspend                     Suspend DHCPV6 thread task    */ 
/*    tx_timer_deactivate                   Deactivate timer              */ 
/*    tx_mutex_get                          Obtain  Client mutex          */ 
/*    tx_mutex_put                          Release Client mutex          */ 
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

UINT  _nx_dhcpv6_stop(NX_DHCPV6 *dhcpv6_ptr)
{

UINT    current_preemption;


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex), NX_DHCPV6_MUTEX_WAIT);

    /* Determine if DHCPV6 is started.  */
    if (dhcpv6_ptr -> nx_dhcpv6_started == NX_FALSE)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* DHCPV6 has not been started so we cannot stop it.  */
        return(NX_DHCPV6_NOT_STARTED);
    }

    /* Clear the DHCPV6 started flag to indicate DHCPV6 task is not running.  */
    dhcpv6_ptr -> nx_dhcpv6_started =  NX_FALSE;

    /* Disable preemption for critical section.  */
    tx_thread_preemption_change(tx_thread_identify(), 0, &current_preemption);

    /* Loop to wait for the DHCP Client thread to be in a position to be stopped.  */
    while (dhcpv6_ptr -> nx_dhcpv6_sleep_flag != NX_TRUE)
    {

        /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* Sleep temporarily. */
        tx_thread_sleep(1);

        /* Get the DHCP mutex.  */
        tx_mutex_get(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex), NX_DHCPV6_MUTEX_WAIT);
    }

    /* Clear the flag.  */
    dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_FALSE;

    /* Suspend the DHCP thread.  */
    tx_thread_suspend(&(dhcpv6_ptr -> nx_dhcpv6_thread));

    /* Abort the wait on the DHCP Client thread.  */
    tx_thread_wait_abort(&(dhcpv6_ptr -> nx_dhcpv6_thread));

    /* Unbind the port.  */
    nx_udp_socket_unbind(&(dhcpv6_ptr -> nx_dhcpv6_socket));

    /* Stop the timers. */
    tx_timer_deactivate(&(dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_timer));
    tx_timer_deactivate(&(dhcpv6_ptr -> nx_dhcpv6_session_timer));

    /* Restore preemption.  */
    tx_thread_preemption_change(tx_thread_identify(), current_preemption, &current_preemption);


    /* Release the DHCP mutex.  */
    tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

    /* Return completion status.  */
    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_dhcpv6_reinitialize                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for the reinitializes the     */
/*    DHCPv6 Client service.                                              */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                            Pointer to DHCPv6 Client      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Actual completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_reinitialize               Actual clear address service  */
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

UINT _nxe_dhcpv6_reinitialize(NX_DHCPV6 *dhcpv6_ptr)
{

UINT status;

       if (dhcpv6_ptr == NX_NULL)
       {
           return NX_PTR_ERROR;
       }

       /* Check for appropriate caller.  */
       NX_THREADS_ONLY_CALLER_CHECKING

       status = _nx_dhcpv6_reinitialize(dhcpv6_ptr);

       return status;
}
        
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_reinitialize                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function reinitializes the DHCP instance for restarting the    */
/*    DHCP client state machine and re-running the DHCPv6 protocol.  This */
/*    is not necessary if the Client has not previously obtained an       */
/*    IP address from the server.  The IP address in both the DHCP Client */ 
/*    data record and in the IP instance is cleared.                      */
/*                                                                        */ 
/*    Note: the host MUST stop the DHCPv6 client before calling this      */
/*    service with the nx_dhcpv6_stop service.                            */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                            Pointer to DHCPv6 Client      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    NX_DHCPV6_ALREADY_STARTED             DHCPv6 client is running      */
/*    status                                Actual completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_remove_assigned_address    Clears client IP address      */
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
    
UINT _nx_dhcpv6_reinitialize(NX_DHCPV6 *dhcpv6_ptr)
{

UINT status;

     
    /* Check that the DHCPv6 client is started. */
    if (dhcpv6_ptr -> nx_dhcpv6_started ==  NX_TRUE)
    {

        /* Need to stop the DHCP client before we can reinitialize it. */
        return NX_DHCPV6_ALREADY_STARTED;
    }

    /* This clears the assigned IP address from the Client record, as well as 
       removes the assigned IP address from the IP address table. */
    status = _nx_dhcpv6_remove_assigned_address(dhcpv6_ptr, NX_DHCPV6_REMOVE_ALL_IA_ADDRESS);

    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_thread_entry                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is the background processing thread for the DHCPV6    */
/*    Client.  It waits to be notified by ThreadX event flags when to     */
/*    perform certain actions.  These include updating the time remaining */
/*    on the Client IP address lease time, and maintaining the duration of*/
/*    the current Client Server session (if Client has made a request).   */
/*    It will terminate if a host application handler is called and       */
/*    indicates the Client should abort.                                  */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    info                               Pointer to DHCPV6 Client instance*/ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_get                Receive notice of event flags set */ 
/*    tx_mutex_get                      Obtain lock on Client resource    */ 
/*    tx_mutex_put                      Release lock on Client resource   */ 
/*    _nx_dhcpv6_request_renew          Initiate the RENEW request        */
/*    _nx_dhcpv6_request_rebind         Initiate the REBIND request       */
/*    _nx_dhcpv6_request_decline        Initiate the DECLINE request      */
/*    _nx_dhcpv6_remove_assigned_address                                  */
/*                                      Remove the assigned IPv6 address  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    ThreadX                                                             */ 
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
VOID  _nx_dhcpv6_thread_entry(ULONG info)
{

UINT        status;
NX_DHCPV6   *dhcpv6_ptr;
UINT        current_preemption;
ULONG       dhcpv6_events;
UINT        ia_index;
UINT        original_state;

    /* Setup the DHCPV6 Client pointer.  */
    NX_THREAD_EXTENSION_PTR_GET(dhcpv6_ptr, NX_DHCPV6, info)

    /* Process periodic DHCPv6 Client tasks. */
    do
    {

        /* Loop to get the DHCP mutex to handle error, release, and 
           stop requests properly.  */
        do
        {

            /* Get the DHCP mutex.  */
            status =  tx_mutex_get(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex), NX_DHCPV6_MUTEX_WAIT);

        } while (status != TX_SUCCESS);


        /* Check the state of the DHCP Client state for further DHCP messaging. */
        _nx_dhcpv6_process(dhcpv6_ptr);

         /* Release the DHCP mutex.  */
        tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* Disable preemption for critical section.  */
        tx_thread_preemption_change(tx_thread_identify(), 0, &current_preemption);

        /* Indicate the DHCP Client process is idle. */
        dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_TRUE;

        /* Sleep for timer interval.  */
        tx_thread_sleep(NX_DHCPV6_SESSION_TIMER_INTERVAL);

        /* Clear flag to indicate DHCP thread is not in a position to be stopped.  */
        dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_FALSE;

        /* Restore original preemption.  */
        tx_thread_preemption_change(tx_thread_identify(), current_preemption, &current_preemption);

        /* Check the expiration on valid and preferred lifetimes only if the Client is in the Bound state. */
        if (dhcpv6_ptr -> nx_dhcpv6_state == NX_DHCPV6_STATE_BOUND_TO_ADDRESS)
        {

            /* At time T1 for an IA, the client initiates a Renew message exchange to extend the lifetimes. RFC3315,page 42.*/
            if (dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued >= dhcpv6_ptr -> nx_dhcpv6_iana.nx_T1) 
            {
                
                /* Indicate the DHCP Client process is idle while the handler executes. */
                dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_TRUE;

                /* Sending the renew message.  */
                _nx_dhcpv6_request_renew(dhcpv6_ptr);

                /* Reset flag to indicate DHCP thread is not idle (not stoppable).  */
                dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_FALSE;
            }
        }

        /* Check the expiration on valid and preferred lifetimes only if the Client is in the sending renew state. */
        if (dhcpv6_ptr -> nx_dhcpv6_state == NX_DHCPV6_STATE_SENDING_RENEW)
        {

            /* At time T2 for an IA, The Client initiates a Rebind message exchanges. RFC3315, page 43. */
            if (dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued >= dhcpv6_ptr -> nx_dhcpv6_iana.nx_T2)
            {
                
                /* Indicate the DHCP Client process is idle while the handler executes. */
                dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_TRUE;

                /* Sending the rebind message.  */
                _nx_dhcpv6_request_rebind(dhcpv6_ptr);

                /* Reset flag to indicate DHCP thread is not idle (not stoppable).  */
                dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_FALSE;
            }
        }
        
        /* Check the DHCPv6 Client state. */
        if ((dhcpv6_ptr -> nx_dhcpv6_state == NX_DHCPV6_STATE_BOUND_TO_ADDRESS) ||
            (dhcpv6_ptr -> nx_dhcpv6_state == NX_DHCPV6_STATE_SENDING_RENEW) ||
            (dhcpv6_ptr -> nx_dhcpv6_state == NX_DHCPV6_STATE_SENDING_REBIND))
        {

            for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
            {

                /* Has the IP address valid lifetime expired?  */
                if ((dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status) &&
                    (dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued >= dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_valid_lifetime))
                {

                    /* Save the existing state of the DHCPv6 Client. */
                    original_state = dhcpv6_ptr -> nx_dhcpv6_state;

                    /*  Clear the assigned IP address. */                
                    _nx_dhcpv6_remove_assigned_address(dhcpv6_ptr, ia_index);

                    /* Indicate the DHCP Client process is idle while the handler executes. */
                    dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_TRUE;

                    /* Restart the Client to the solicit IP address request state. */
                    _nx_dhcpv6_request_solicit(dhcpv6_ptr);

                    /* Indicate the DHCP Client process is idle while the handler executes. */
                    dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_FALSE;

                    if (dhcpv6_ptr -> nx_dhcpv6_state_change_callback && (original_state != dhcpv6_ptr -> nx_dhcpv6_state))
                    {
                        /* Yes, call the state change callback with the original and new state. */
                        (dhcpv6_ptr -> nx_dhcpv6_state_change_callback)(dhcpv6_ptr, original_state, dhcpv6_ptr -> nx_dhcpv6_state);
                    }
                 }
            }
        }
       


        /* Pickup IP event flags.  */
        status = tx_event_flags_get(&(_nx_dhcpv6_DAD_ptr -> nx_dhcpv6_events), NX_IP_ALL_EVENTS, TX_OR_CLEAR, &dhcpv6_events, TX_NO_WAIT);
         
        /* Check for an IP receive packet event.  */
        if ((status == TX_SUCCESS) && (dhcpv6_events & NX_DHCPV6_DAD_FAILURE_EVENT))
        {

            /* Indicate the DHCP Client process is idle while the handler executes. */
            dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_TRUE;

            /* The DAD process failure, send DHCPv6 DECLINE message.  */
            _nx_dhcpv6_request_decline(dhcpv6_ptr);        
            
            dhcpv6_events = dhcpv6_events & ~(ULONG)(NX_DHCPV6_DAD_FAILURE_EVENT);

            /* Reset flag to indicate DHCP thread is not idle (not stoppable).  */
            dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_FALSE;
        }

    } while (1);

}

 
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_utility_get_block_option_length          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function parses the input buffer (assuming to be an option     */
/*    block in a server reply) for option code and length. It assumes the */
/*    buffer pointer is pointed to the first byte of the option buffer.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_                             Pointer to DHCPv6 Client        */ 
/*    option                              Option code of requested data   */
/*    buffer_ptr                          Buffer to copy data to          */
/*    length                              Size of buffer                  */
/*                                              valid                     */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Completion status               */ 
/*    NX_DHCPV6_INCOMPLETE_OPTION_BLOCK   Unknown option specified        */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_dhcpv6_utility_get_data       Parses a specific data from the  */ 
/*                                           DHCPv6 option                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_utility_get_block_option_length                          */
/*                                     Parses option code and length      */
/*    _nx_dhcpv6_extract_packet_information                               */
/*                                     Parses each option from server     */
/*                                        reply and updates Client record */
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
UINT  _nx_dhcpv6_utility_get_block_option_length(UCHAR *buffer_ptr, ULONG *option, ULONG *length)
{
       
    /* Initialize to zero. */
    *option = 0;
    *length = 0;

    /* First byte should be the op code. */
    _nx_dhcpv6_utility_get_data(buffer_ptr, 2, option);

    buffer_ptr += 2;

    /* Next byte should be the option length. */
    _nx_dhcpv6_utility_get_data(buffer_ptr, 2, length);

    /* Buffer is now pointed at the data (past the length field). */
    buffer_ptr += 2;

    /* Check for null data. */
    if (*option == 0)
    {

        return NX_DHCPV6_INCOMPLETE_OPTION_BLOCK; 
    }

    return(NX_SUCCESS);  
        
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_utility_get_data                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function parses the input buffer and returns numeric data      */
/*    specified by the size argument, up to 4 bytes long.  Note that if   */ 
/*    caller is using this utility to extract bytes from a DHCPv6 packet  */
/*    there is no need for byte swapping, as compared to using memcpy in  */
/*    which case there is for little endian processors.                   */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    buffer                            Pointer to data buffer            */ 
/*    value                             Pointer to data parsed from buffer*/
/*    size                              Size of buffer                    */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_DHCPV6_INVALID_DATA_SIZE       Requested data size too large     */
/*    NX_SUCCESS                        Successful completion status      */ 
/*                                                                        */
/*  CALLS                                                                 */ 
/*                                                                        */
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*    _nx_dhcpv6_process_server_duid   Process server duid in server reply*/ 
/*    _nx_dhcpv6_process_client_duid   Process server duid in server reply*/ 
/*    _nx_dhcpv6_utility_get_block_option_length                          */
/*                                     Parses option code and length      */
/*    _nx_dhcpv6_extract_packet_information                               */
/*                                     Parses each option from server     */
/*                                        reply and updates Client record */
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
UINT  _nx_dhcpv6_utility_get_data(UCHAR *buffer, UINT size, ULONG *value)
{


    /* Check that the size of the data fits in a ULONG. */
    if (size > sizeof(ULONG))
    {
        return NX_DHCPV6_INVALID_DATA_SIZE;
    }

    *value = 0;
    
    /* Process the data retrieval request.  */
    while (size-- > 0)
    {

        /* Build return value.  */
        *value = (((*value) << 8) | (*buffer));
        buffer++;
    }

    /* Return value.  */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_utility_time_randomize                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function returns a value of between -1 seconds and 1 second    */ 
/*    in system ticks.  It is used to randomize timeouts as required by   */ 
/*    the RFC's so as to not overload a network server after power out.   */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    ticks                                 Number of ticks between 1 & -1*/ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_convert_delay_to_ticks     Convert seconds to ticks      */ 
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
INT  _nx_dhcpv6_utility_time_randomize(void)
{

UINT temp;
UINT sign;
INT temp_signed;

    temp = (UINT)(NX_RAND());
    temp &= (UINT)(0x001F);
    sign = (temp & (UINT)(0x8));
    temp_signed = (INT)temp;

    /* Try for a number between 0 and 0x1F. */
    if (sign)
    {
        temp_signed = -temp_signed;
    }
    
    return temp_signed;

}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_waiting_on_reply                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function waits to receive a DHCPv6 server packet for the wait  */
/*    time set by the DHCPv6 client previously. If a reply is received,   */
/*    call function to process it.                                        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                       Pointer to DHCPV6 Client instance  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    status                           Actual completion status           */
/*    NX_SUCCESS                       Valid packet received              */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_receive            Receive UDP packet                 */
/*    nx_packet_release                Release packet to packet pool      */ 
/*    _nx_dhcpv6_packet_procss         Process the dhcpv6 packet          */ 
/*    _nx_dhcpv6_flush_queue_packets   Flush the queue packets            */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process               Process current client state       */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_waiting_on_reply(NX_DHCPV6 *dhcpv6_ptr)
{
                       
UINT        current_preemption;
UINT        status;
NX_PACKET   *packet_ptr;
ULONG       start_time;
ULONG       current_time;
ULONG       elapsed_time;
ULONG       receive_wait;
UINT        time_remaining; 
UINT        valid_answer;

    /* Initialize the parameters.  */
    packet_ptr = NX_NULL;
    valid_answer = NX_FALSE;
    time_remaining = dhcpv6_ptr -> nx_dhcpv6_transmission_timeout;

    /* Wait in short intervals to receive a valid packet while continuously checking for packets not intended
       for this DHCP Client to be removed from the receive queue. */
    do
    {

        /* Release the DHCP mutex while the Client will be blocked on a receive socket call.  */
        tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

        /* Disable preemption for critical section.  */
        tx_thread_preemption_change(tx_thread_identify(), 0, &current_preemption);

        /* Indicate the DHCP Client can be stopped.  */
        dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_TRUE;

        start_time = tx_time_get();

        if (time_remaining < NX_DHCPV6_TIME_INTERVAL)
        {
            receive_wait = time_remaining;
        }
        else
        {
            receive_wait = NX_DHCPV6_TIME_INTERVAL;
        }

        status = nx_udp_socket_receive(&(dhcpv6_ptr -> nx_dhcpv6_socket), &packet_ptr, receive_wait);

        /* Indicate the DHCP Client can not be stopped.  */
        dhcpv6_ptr -> nx_dhcpv6_sleep_flag =  NX_FALSE;

        /* Restore preemption/ end of critical section.  */
        tx_thread_preemption_change(tx_thread_identify(), current_preemption, &current_preemption);        

        /* Yes, receive a packet.  */
        if (status == NX_SUCCESS)
        {

            /* Get the DHCP mutex.  */
            while((tx_mutex_get(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex), NX_NO_WAIT) != TX_SUCCESS) &&
                (time_remaining > 0))
            {

                if (time_remaining < NX_DHCPV6_TIME_INTERVAL)
                {
                    tx_thread_sleep(time_remaining);
                    time_remaining = 0;
                }
                else
                {
                    tx_thread_sleep(NX_DHCPV6_TIME_INTERVAL);
                    time_remaining = (UINT)(time_remaining - NX_DHCPV6_TIME_INTERVAL);
                }
            }

            /* Process the dhcpv6 packet.  */
            status = _nx_dhcpv6_packet_process(dhcpv6_ptr, packet_ptr);

            /* Check the status.  */
            if(status == NX_SUCCESS) 
            {

                /* Yes, Get a valid advertise or reply message.  */
                valid_answer = NX_TRUE;

                /* DHCPv6 client MUST collect Advertise messages for the first RT seconds, 
                   unless it receives an Advertise message with a preference value of 255.
                   RFC 3315, Section 17.1.2, Page 34.  */
                if((dhcpv6_ptr -> nx_dhcpv6_state == NX_DHCPV6_STATE_SENDING_SOLICIT) &&
                   (dhcpv6_ptr -> nx_dhcpv6_retransmission_count == 0))
                {

                    /* Process the rapid solicit mode.  */
                    if(dhcpv6_ptr -> nx_dhcpv6_request_solicit_mode == NX_DHCPV6_SOLICIT_RAPID)
                    {

                        /* Client terminates the waiting process as soon as a reply message
                           with a Rapid Commit option is received. RFC3315, Section17.1.2, Page34. */  
                        break;
                    }

                    /* Process the normal solicit mode.  */
                    if(dhcpv6_ptr -> nx_dhcpv6_preference.nx_pref_value == 255)
                    {

                        /* If the client receives an Advertise message that includes a Preference option with
                           a preference value of 255, the client can terminates the waiting. */
                        break;
                    }
                }
                else
                {

                    /* Yes, Get the answer.  */    
                    break;
                }
            }
        }

        /* How much time has elapsed? */
        current_time = tx_time_get();

        /* Has the time wrapped? */
        if (current_time >= start_time)
        {
            /* No, simply subtract to get the elapsed time.   */
            elapsed_time =  current_time - start_time;
        }
        else
        {

            /* Yes it has. Time has rolled over the 32-bit boundary.  */
            elapsed_time =  (((ULONG) 0xFFFFFFFF) - start_time) + current_time;
        }

        /* Update the time remaining. */
        if (time_remaining > elapsed_time)
        {
            time_remaining -= elapsed_time;
        }
        else
        {
            /*  Time is up, break out of this loop. */
            time_remaining = 0;

            /* But not before restoring preemption/ end of critical section! */
            tx_thread_preemption_change(tx_thread_identify(), current_preemption, &current_preemption);

            break;
        }

    } while(time_remaining > 0);

    /* Flush the queue packets.  */
    _nx_dhcpv6_flush_queue_packets(dhcpv6_ptr);

    /* Check the valid answer flag.  */
    if (valid_answer == NX_TRUE)
    {
        return NX_SUCCESS;
    }
    else
    {
        return NX_DHCPV6_INVALID_SERVER_PACKET;
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_packet_process                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function process the receive pacet. the data is copied to      */
/*    another packet from the DHCPv6 Client packet pool,and the original  */
/*    received packet is released back to the receive pool immediately.   */
/*    Then process the reply message.                                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                       Pointer to DHCPV6 Client instance  */ 
/*    packet_ptr                          Pointer to received packet      */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    status                           Actual completion status           */
/*    NX_SUCCESS                       Process correctly                  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_extract            Get the reply information          */
/*    nx_packet_release                Release packet to packet pool      */ 
/*    nx_packet_allocate               Allocate a new packet.             */
/*    nx_packet_data_extract_offset    Store the received pacet data.     */
/*    _nx_dhcpv6_scan_packet_options   Scan and record the reply options. */
/*    _nx_dhcpv6_preprocess_packet_information                            */
/*                                     Preprocess the packet according to */ 
/*                                     the options record.                */
/*    _nx_dhcpv6_extract_packet_information                               */
/*                                     Extract server reply data          */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_waiting_on_reply      Process current client message     */ 
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
UINT _nx_dhcpv6_packet_process(NX_DHCPV6 *dhcpv6_ptr, NX_PACKET *packet_ptr)
{

UINT        status;
UINT        port;  
ULONG       bytes_copied;
NXD_ADDRESS server_IP_Address;
NX_PACKET   *new_packet_ptr;


    /* Check for valid packet length (message type and ID).  */
    if (packet_ptr -> nx_packet_length < 4)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        /* Return.  */
        return(NX_DHCPV6_INVALID_DATA_SIZE);
    }

    /* Extract the IP and port number from the packet and set the reply address.  */
    status =  nxd_udp_source_extract(packet_ptr, &server_IP_Address, &port);

    /* Check status.  */
    if (status != NX_SUCCESS)
    {

        /* Release the packet. We don't want it. */
        nx_packet_release(packet_ptr);

        /* Error extracting packet source information, return error status.  */
        return(status);
    }

    /* Copy the received packet into the packet allocated by the DHCPv6 Client.  */
    status =  nx_packet_allocate(dhcpv6_ptr -> nx_dhcpv6_pool_ptr, 
                                 &new_packet_ptr, NX_IPv6_UDP_PACKET, 
                                 NX_DHCPV6_PACKET_TIME_OUT);
    
    /* Check for errors. */
    if (status != NX_SUCCESS)
    {

        /* Release the receive packet. We can't use it. */
        nx_packet_release(packet_ptr);
    
        return status;
    }
                   
    /* Verify the incoming packet does not exceed our DHCPv6 Client packet payload. */
    if ((ULONG)(new_packet_ptr -> nx_packet_data_end - new_packet_ptr -> nx_packet_prepend_ptr) < packet_ptr -> nx_packet_length)
    {
                                             
        /* Release the packet allocated from the Client pool*/
        nx_packet_release(new_packet_ptr);

        /* Release the received packet. We can't use it. */
        nx_packet_release(packet_ptr);

        return(NX_DHCPV6_INVALID_DATA_SIZE);
    }

    /* Use a packet from the DHCP Client as a buffer to store the received packet data.
       Then we can release the received packet back to its packet pool. */
    status = nx_packet_data_extract_offset(packet_ptr, 0,(VOID *)new_packet_ptr -> nx_packet_prepend_ptr, packet_ptr -> nx_packet_length, &bytes_copied);

    /* Check status.  */
    if ((status != NX_SUCCESS) || (bytes_copied == 0))
    {

        /* Release the packet allocated from the Client pool*/
        nx_packet_release(new_packet_ptr);

        /* Release the received packet. We can't use it. */
        nx_packet_release(packet_ptr);

        /* Error extracting packet buffer, return error status.  */
        return(status);
    }

    /* Now we can release the received packet. */
    nx_packet_release(packet_ptr);

    /* Update the new packet with the bytes copied.  For chained packets, this will reflect the total
       'datagram' length. */
    new_packet_ptr -> nx_packet_length = bytes_copied;
    new_packet_ptr -> nx_packet_append_ptr = new_packet_ptr -> nx_packet_prepend_ptr + bytes_copied;

    /* Process the DHCPv6 packet.*/

    /* Scan the packet options and record it.  */
    status = _nx_dhcpv6_scan_packet_options(dhcpv6_ptr, new_packet_ptr);
    
    /* Check for errors processing the packet. */
    if (status != NX_SUCCESS)
    {

        /* Release the Client packet. We can't use it. */
        nx_packet_release(new_packet_ptr);

        /* Return error status. */
        return status;
    }

    /* Preprocess the packet options.  */
    status = _nx_dhcpv6_preprocess_packet_information(dhcpv6_ptr, new_packet_ptr);
    
    /* Check for errors processing the packet. */
    if (status != NX_SUCCESS)
    {

        /* Release the Client packet. We can't use it. */
        nx_packet_release(new_packet_ptr);

        /* Return error status. */
        return status;
    }

    /*  Extract the info from the packet buffer into the Client record. */
    status = _nx_dhcpv6_extract_packet_information(dhcpv6_ptr, new_packet_ptr);

    /* Release the Client packet. We are done with it. */
    nx_packet_release(new_packet_ptr);

    /* Store the DHCPv6 server address.  */
    if ((status == NX_SUCCESS) && (dhcpv6_ptr -> nx_dhcpv6_received_message_type == NX_DHCPV6_MESSAGE_TYPE_REPLY))
    {
        COPY_NXD_ADDRESS(&server_IP_Address, &(dhcpv6_ptr-> nx_dhcpv6_server_address));
    }

    /* Successfully processed response from the server. */
    return status;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_scan_packet_options                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function scan the packet options of DHCPv6 server response and */
/*    record the relevant options.                                        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */
/*    packet_ptr                          Pointer to received packet      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Actual completion status        */
/*    NX_SUCCESS                          Successful completion status    */ 
/*    NX_DHCPV6_UNKNOWN_OPTION            Unknown option in message       */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_utility_get_block_option_length                          */
/*                                        Extract option and length data  */
/*    memcpy                              Copies specified area of memory */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_packet_process           Process dhcpv6 packet.          */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcpv6_scan_packet_options(NX_DHCPV6 *dhcpv6_ptr, NX_PACKET *packet_ptr)
{

UINT        status;
UCHAR       *dhcpv6_option_ptr;
UINT        index;
ULONG       option_code;
ULONG       option_length;
UCHAR       *dhcpv6_iana_option_ptr;
UINT        iana_index;
ULONG       iana_option_code;
ULONG       iana_option_length;
UCHAR       *dhcpv6_ia_option_ptr;
ULONG       ia_option_index;
ULONG       ia_option_code;
ULONG       ia_option_length;
ULONG       ipv6_address[4];
USHORT      status_code;
UINT        ia_index;
UINT        w;
UINT        ia_count = 0;

    /* Set a pointer to the start of DHCPv6 options.  */
    dhcpv6_option_ptr = (packet_ptr -> nx_packet_prepend_ptr + 4);

    /* Initialize local variables. */
    index = 4;
    dhcpv6_ptr -> nx_dhcpv6_reply_option_flags = 0;
    dhcpv6_ptr -> nx_dhcpv6_reply_option_current_pref_value = 0;
        
    /* Clear the IA address map status.   */        
    for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
    {

        dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_map = 0;
    }

    /* Now parse all the DHCPv6 option blocks in the packet buffer. */
    /* 4 bytes is data pointer offset, and 4 bytes for option code and option length. */
    while (index + 4 <= packet_ptr -> nx_packet_length)
    {

        /* Get the option code and length of data of the current option block. */
        status = _nx_dhcpv6_utility_get_block_option_length(dhcpv6_option_ptr, &option_code, &option_length);

        /* Check that the block data is valid. */
        if (status != NX_SUCCESS)
        {

            /* No, return the error status. */
            return status;
        }

        /* Check if option data is in the packet.  */
        if ((dhcpv6_option_ptr + 4 + option_length) > packet_ptr -> nx_packet_append_ptr)
        {
            return(NX_DHCPV6_INCOMPLETE_OPTION_BLOCK);
        }
        
        /* Process the option code with an option specific API. */
        switch (option_code)
        {

            /* Note - these 'process' functions will not move the buffer pointer. */             
            case NX_DHCPV6_OP_CLIENT_ID:
            {
                         
                /* Yes, The message includes the Client DUID option.  */
                dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_CLIENT_ID_OPTION;
                break;
            }

            case NX_DHCPV6_OP_SERVER_ID:
            {           

                /* Yes, The message includes the Server DUID option.  */
                dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_SERVER_ID_OPTION;
                break;
            }
            case NX_DHCPV6_OP_IA_NA:
            {

                /* The minimum length of IANA option field is 12 bytes.  */
                if (option_length < 12)
                {
                    return(NX_DHCPV6_INCOMPLETE_OPTION_BLOCK);
                }

                /* Yes, The message includes the Server DUID option.  */
                dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_IA_NA_OPTION;

                /* Skip the IA_NA header data (4 bytes for IANA option code and option length, 12 bytes for IANA option field).  */
                dhcpv6_iana_option_ptr = dhcpv6_option_ptr + 4 + 12;
                iana_index = 0;

                /* Process the IA and status options embedded in the IANA option.  */
                /* 12 bytes is IA_NA header, 4 bytes for iana sub option code and option length.  */
                while(iana_index + 4 <= (option_length - 12))
                {

                    /* Get the next option code and length. */
                    status = _nx_dhcpv6_utility_get_block_option_length(dhcpv6_iana_option_ptr, &iana_option_code, &iana_option_length);

                    /* Check if the option data is in the packet.  */
                    if ((dhcpv6_iana_option_ptr + 4 + iana_option_length) > packet_ptr -> nx_packet_append_ptr)
                    {
                        return(NX_DHCPV6_INCOMPLETE_OPTION_BLOCK);
                    }

                    /* Check the IA address option embedded in the IANA option. */
                    if(iana_option_code == NX_DHCPV6_OP_IA_ADDRESS)
                    {

                        /* The minimum length of IA address option is 24 bytes.  */
                        if (iana_option_length < 24)
                        {
                            return(NX_DHCPV6_INCOMPLETE_OPTION_BLOCK);
                        }

                        /* Yes, The message includes the IA option.  */
                        dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_IA_ADDRESS_OPTION; 
                        
                        /* Increase the IA option count.  */
                        ia_count ++;

                        /* Skip the IA type and length option.  */
                        ia_option_index = 4;

                        /* Get the IPv6 address of IA option.  */
                        for (w = 0; w <= 3; w++)
                        {

                            /* Copy each IPv6 address word into the IA address. */
                            memcpy(&ipv6_address[w], (dhcpv6_iana_option_ptr + ia_option_index), sizeof(ULONG)); /* Use case of memcpy is verified. */

                            /* Adjust for endianness. */
                            NX_CHANGE_ULONG_ENDIAN(ipv6_address[w]);

                            ia_option_index += 4;
                        }

                        /* Check the IA table, find the same IPv6 address.  */
                        for(w = 0; w < NX_DHCPV6_MAX_IA_ADDRESS; w++)
                        {

                            if(!(memcmp(&(dhcpv6_ptr->nx_dhcpv6_ia[w].nx_global_address.nxd_ip_address.v6[0]), &ipv6_address[0], 16)))
                            {

                                /* Yes, it is, record the index to store the IA option, 0 means is IA address inexistence.  
                                   Then update the IA option according to the IA count in nx_dhcpv6_update_ia function.  */
                                dhcpv6_ptr -> nx_dhcpv6_ia[w].nx_address_map = ia_count;
                                break;
                            }
                        }

                        /* Check the IA option whether include the status codes or not.  */
                        /* 24 bytes is IA address option, 4 bytes for next option code and option length.  */
                        if(iana_option_length >= (24 + 4))
                        {

                            /* Yes, include status codes, skip the IA header data.  */
                            dhcpv6_ia_option_ptr = dhcpv6_iana_option_ptr + 28;

                            /* Get the status code and length. */
                            status = _nx_dhcpv6_utility_get_block_option_length(dhcpv6_ia_option_ptr, &ia_option_code, &ia_option_length);

                            /* Check if the option data is in the packet.  */
                            if ((dhcpv6_ia_option_ptr + 4 + ia_option_length) > packet_ptr -> nx_packet_append_ptr)
                            {
                                return(NX_DHCPV6_INCOMPLETE_OPTION_BLOCK);
                            }

                            if(ia_option_code == NX_DHCPV6_OP_STATUS_CODE)
                            {
                                if (ia_option_length < 2)
                                {
                                    return(NX_DHCPV6_INCOMPLETE_OPTION_BLOCK);
                                }

                                /* Yes, the message includes the STATUS option, get the code.  */
                                memcpy(&status_code, (dhcpv6_ia_option_ptr + 4), sizeof(USHORT)); /* Use case of memcpy is verified. */

                                /* Adjust for endianness. */
                                NX_CHANGE_USHORT_ENDIAN(status_code);

                                if(status_code == 0)
                                {

                                    /* Yes, The message includes the Success status option.  */
                                    dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_SUCCESS_OPTION; 
                                }
                                else if(status_code == 1)
                                {

                                    /* Yes, The message includes the UnspecFail status option.  */
                                    dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_UNSPEC_FAIL_OPTION;
                                }
                                else if(status_code == 2)
                                {

                                    /* Yes, The message includes the NoAddrsAvail status option.  */
                                    dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_NO_ADDR_AVAIL_OPTION;
                                }
                                else if(status_code == 3)
                                {

                                    /* Yes, The message includes the NoBinding status option.  */
                                    dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_NO_BIND_OPTION;
                                }
                                else if(status_code == 4)
                                {

                                    /* Yes, The message includes the NotOnLink status option.  */
                                    dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_NOT_ONLINK_OPTION;
                                }
                                else if(status_code == 5)
                                {

                                    /* Yes, The message includes the UseMulticast status option.  */
                                    dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_USE_MULTICAST_OPTION;
                                }
                                else
                                {

                                    /* This is an unsupported or unknown option. */
                                    return NX_DHCPV6_UNKNOWN_OPTION; 
                                }

                            }
                            else
                            {

                                /* This is an unsupported or unknown option. */
                                return NX_DHCPV6_UNKNOWN_OPTION; 
                            }
                        }
                    }

                    /* Check if this is an status option request. */
                    else if(iana_option_code == NX_DHCPV6_OP_STATUS_CODE)
                    {
                        if (iana_option_length < 2)
                        {
                            return(NX_DHCPV6_INCOMPLETE_OPTION_BLOCK);
                        }

                        /* Yes, the message includes the STATUS option, get the code.  */
                        memcpy(&status_code, (dhcpv6_iana_option_ptr + 4), sizeof(USHORT)); /* Use case of memcpy is verified. */

                        /* Adjust for endianness. */
                        NX_CHANGE_USHORT_ENDIAN(status_code);

                        if(status_code == 0)
                        {

                            /* Yes, The message includes the Success status option.  */
                            dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_SUCCESS_OPTION; 
                        }
                        else if(status_code == 1)
                        {

                            /* Yes, The message includes the UnspecFail status option.  */
                            dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_UNSPEC_FAIL_OPTION;
                        }
                        else if(status_code == 2)
                        {

                            /* Yes, The message includes the NoAddrsAvail status option.  */
                            dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_NO_ADDR_AVAIL_OPTION;
                        }
                        else if(status_code == 3)
                        {

                            /* Yes, The message includes the NoBinding status option.  */
                            dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_NO_BIND_OPTION;
                        }
                        else if(status_code == 4)
                        {

                            /* Yes, The message includes the NotOnLink status option.  */
                            dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_NOT_ONLINK_OPTION;
                        }
                        else if(status_code == 5)
                        {

                            /* Yes, The message includes the UseMulticast status option.  */
                            dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_USE_MULTICAST_OPTION;
                        }
                        else
                        {

                            /* This is an unsupported or unknown option. */
                            return NX_DHCPV6_UNKNOWN_OPTION; 
                        }
                    }
                    else
                    {   

                        /* This is an unsupported or unknown option. */
                        return NX_DHCPV6_UNKNOWN_OPTION; 
                    }

                    /* Move to the next top level option. */
                    dhcpv6_iana_option_ptr += iana_option_length + 4;

                    /* Update the iana_index.  */
                    iana_index += iana_option_length + 4;

                }

                break;
            }

            case NX_DHCPV6_OP_PREFERENCE:
            {

                /* The preference option length must be 1 byte.  */
                if (option_length != 1)
                {
                    return(NX_DHCPV6_INCOMPLETE_OPTION_BLOCK);
                }

                /* Yes, The message includes the preference option.  */
                dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_PREFERENCE_OPTION; 

                /* Record the preference option value of current advetise message.  */
                dhcpv6_ptr -> nx_dhcpv6_reply_option_current_pref_value = (USHORT)(*(dhcpv6_option_ptr + 4));

                break;
            }

            case NX_DHCPV6_OP_STATUS_CODE:
            {

                /* The minimum length of status option is 2 bytes.  */
                if (option_length < 2)
                {
                    return(NX_DHCPV6_INCOMPLETE_OPTION_BLOCK);
                }

                /* Yes, the message includes the STATUS option, get the code.  */
                memcpy(&status_code, (dhcpv6_option_ptr + 4), sizeof(USHORT)); /* Use case of memcpy is verified. */

                /* Adjust for endianness. */
                NX_CHANGE_USHORT_ENDIAN(status_code);

                if(status_code == 0)
                {

                    /* Yes, The message includes the Success status option.  */
                    dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_SUCCESS_OPTION; 
                }
                else if(status_code == 1)
                {

                    /* Yes, The message includes the UnspecFail status option.  */
                    dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_UNSPEC_FAIL_OPTION;
                }
                else if(status_code == 2)
                {

                    /* Yes, The message includes the NoAddrsAvail status option.  */
                    dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_NO_ADDR_AVAIL_OPTION;
                }
                else if(status_code == 3)
                {

                    /* Yes, The message includes the NoBinding status option.  */
                    dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_NO_BIND_OPTION;
                }
                else if(status_code == 4)
                {

                    /* Yes, The message includes the NotOnLink status option.  */
                    dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_NOT_ONLINK_OPTION;
                }
                else if(status_code == 5)
                {

                    /* Yes, The message includes the UseMulticast status option.  */
                    dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_STATUS_USE_MULTICAST_OPTION;
                }
                else
                {

                    /* This is an unsupported or unknown option. */
                    return NX_DHCPV6_UNKNOWN_OPTION; 
                }
                break;
            }

            case NX_DHCPV6_OP_RAPID_COMMIT:
            {

                /* Yes, The message includes the Rapid Commit option.  */
                dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_RAPID_COMMIT_OPTION; 
                break;
            }

            case NX_DHCPV6_OP_DNS_SERVER:
            {
            
                /* Yes, The message includes the DNS server option.  */
                dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_DNS_SERVER_OPTION; 
                break;
            }

            case NX_DHCPV6_OP_DOMAIN_NAME:
            {
                
                /* Yes, The message includes the domain name option.  */
                dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_DOMAIN_NAME_OPTION; 
                break;
            }

            case NX_DHCPV6_OP_SNTP_SERVER:
            {
            
                /* Yes, The message includes the timer server option.  */
                dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_SNTP_SERVER_OPTION; 
                break;
            }

            case NX_DHCPV6_OP_CLIENT_FQDN:
            {
            
                /* Yes, The message includes the Client FQDN option.  */
                dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_CLIENT_FQDN_OPTION; 
                break;
            }

            case NX_DHCPV6_OP_NEW_POSIX_TIMEZONE:
            {

                /* Yes, The message includes the time zone option.  */
                dhcpv6_ptr -> nx_dhcpv6_reply_option_flags |= NX_DHCPV6_INCLUDE_NEW_POSIX_TIIMEZONE_OPTION; 
                break;
            }

            default:
            {
                break;
            }
        }

        /* Move to the next top level option. */
         dhcpv6_option_ptr += option_length + 4;
         
        /* Keep track of how far into the packet we have parsed. */
        index += option_length + 4; 
    }

    /* Yes, scan is complete, just return success. */
    return NX_SUCCESS; 
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_preprocess_packet_information            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function preprocess packet information according to reply      */
/*    message option.                                                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */
/*    packet_ptr                          Pointer to received packet      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_DHCPv6_SUCCESS                   Successful completion status    */ 
/*    NX_DHCPV6_NO_DUID_OPTION            Message has no DUID option      */
/*    NX_DHCPV6_NO_RAPID_COMMIT_OPTION    Message has no rapid            */
/*                                             commit  option             */ 
/*    NX_DHCPV6_IA_ADDRESS_NOT_VALID      Message has no IA option        */
/*    NX_DHCPV6_EQUAL_OR_LESS_PREF_VALUE  Preference value is equal to    */
/*                                             or less than               */ 
/*    NX_DHCPV6_UNKNOWN_MSG_TYPE          Unknown message type            */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nx_dhcpv6_utility_get_data         Get the data in reply message   */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_packet_process           Process the packet.             */
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
UINT  _nx_dhcpv6_preprocess_packet_information(NX_DHCPV6 *dhcpv6_ptr, NX_PACKET *packet_ptr)
{
    
ULONG       received_message_type;
ULONG       returned_xid;

    /* Preprocess the DHCPv6 packet.  */   

    /* Extract the message type which should be the first byte.  */
    _nx_dhcpv6_utility_get_data(packet_ptr -> nx_packet_prepend_ptr, 1, &received_message_type);

    /* Check for an illegal message type. */
    if ((received_message_type != NX_DHCPV6_MESSAGE_TYPE_ADVERTISE) &&
        (received_message_type != NX_DHCPV6_MESSAGE_TYPE_REPLY) &&
        (received_message_type != NX_DHCPV6_MESSAGE_TYPE_RECONFIGURE))
    {

        /* These should only be sent to DHCPv6 servers! */
        return NX_DHCPV6_ILLEGAL_MESSAGE_TYPE;
    }

    /* Set the received message type.  */
    dhcpv6_ptr -> nx_dhcpv6_received_message_type = (UCHAR)received_message_type;

    /* This is a unicast packet (valid). Parse the transaction ID. */
    _nx_dhcpv6_utility_get_data((packet_ptr -> nx_packet_prepend_ptr + 1), 3, &returned_xid);

    /* Make sure the DHCP transaction ID matches our current session. */
    if (returned_xid != (dhcpv6_ptr -> nx_dhcpv6_message_hdr.nx_message_xid))
    {
        
        /* Return the bad transmaction ID.  */
        return NX_DHCPV6_BAD_TRANSACTION_ID;
    }

    /* Check the options.  */
    switch (dhcpv6_ptr -> nx_dhcpv6_state)
    {

        /* The DHCPv6 remains initial status or DHCPv6 interactive process has been completed.
           Just clear the timer and message ID. */
        case NX_DHCPV6_STATE_BOUND_TO_ADDRESS:   
        case NX_DHCPV6_STATE_INIT:
        {

            /* In these state, DHCPv6 Client do not expect any packet.  */
            return NX_DHCPV6_ILLEGAL_MESSAGE_TYPE;
        }

        /* The DHCPv6 CLient has sent a solicit message and expect advertise message. */
        case NX_DHCPV6_STATE_SENDING_SOLICIT:
        {
            
            /* DHCPv6 Client must discrd the received advertise messages 
               that meet the message does not inlcude SERVER ID or CLIENT ID.  */
            if(!(dhcpv6_ptr -> nx_dhcpv6_reply_option_flags & NX_DHCPV6_INCLUDE_CLIENT_ID_OPTION) ||
               !(dhcpv6_ptr -> nx_dhcpv6_reply_option_flags & NX_DHCPV6_INCLUDE_SERVER_ID_OPTION))
            {

                /* The packet does not include DUID option.*/
                return NX_DHCPV6_NO_DUID_OPTION;
            }

            /* DHcpv6 Client must ignore any advertise message that includes a status code option
               containing the value NoAddrsAvail.  */
            if(dhcpv6_ptr -> nx_dhcpv6_reply_option_flags & NX_DHCPV6_INCLUDE_STATUS_NO_ADDR_AVAIL_OPTION)
            {                
                return NX_DHCPV6_IA_ADDRESS_NOT_VALID;
            }

            if(dhcpv6_ptr -> nx_dhcpv6_request_solicit_mode == NX_DHCPV6_SOLICIT_RAPID)
            {
                if(!(dhcpv6_ptr -> nx_dhcpv6_reply_option_flags & NX_DHCPV6_INCLUDE_RAPID_COMMIT_OPTION))
                {

                    return NX_DHCPV6_NO_RAPID_COMMIT_OPTION;
                }
            }
            else
            {

                /* Check the preference value.  */
                if((dhcpv6_ptr -> nx_dhcpv6_preference.nx_pref_value != 0) &&
                   (dhcpv6_ptr -> nx_dhcpv6_reply_option_current_pref_value <= dhcpv6_ptr -> nx_dhcpv6_preference.nx_pref_value))
                {

                    /* The current preference value is equal to or less than the recorded preference valude,
                       DHCPv6 Client need not process the message, return.  */
                    return NX_DHCPV6_EQUAL_OR_LESS_PREF_VALUE;
                }
            }

            break;
        }
        
        /* The DHCPv6 CLien has sent a request message and expect reply message. */
        case NX_DHCPV6_STATE_SENDING_REQUEST:  
        case NX_DHCPV6_STATE_SENDING_RENEW:
        case NX_DHCPV6_STATE_SENDING_REBIND:
        case NX_DHCPV6_STATE_SENDING_CONFIRM:
        case NX_DHCPV6_STATE_SENDING_RELEASE:
        case NX_DHCPV6_STATE_SENDING_DECLINE:
        case NX_DHCPV6_STATE_SENDING_INFORM_REQUEST:
        {
                
            /* DHCPv6 Client must discrd the received advertise messages 
               that meet the message does not inlcude SERVE ID or CLIENT ID.  */
            if(!(dhcpv6_ptr -> nx_dhcpv6_reply_option_flags & NX_DHCPV6_INCLUDE_CLIENT_ID_OPTION) ||
               !(dhcpv6_ptr -> nx_dhcpv6_reply_option_flags & NX_DHCPV6_INCLUDE_SERVER_ID_OPTION))
            {
                
                /* The packet does not include DUID option.*/
                return NX_DHCPV6_NO_DUID_OPTION;
            }        
            break;
        }

        default:
        {

            /* Unknown or unsupported DHCPv6 request (state).  */
            return NX_DHCPV6_UNKNOWN_MSG_TYPE;
        }
    }

    return NX_DHCPV6_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_extract_packet_information               PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function parses the DHCPv6 server response to Client requests. */
/*    It checks for valid reply message types for the current Client      */
/*    request, and if all is well, then parses each option in sequential  */
/*    order and updates the server reply data to the Client record. At the*/
/*    end of the data extraction, it then verifies that the server reply  */
/*    contained both valid Client and server DUID options.                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */
/*    packet_ptr                          Pointer to received packet      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                              Actual completion status        */
/*    NX_SUCCESS                          Successful completion status    */ 
/*    NX_DHCPV6_ILLEGAL_MESSAGE_TYPE      Packet message type illegal     */
/*    NX_DHCPV6_BAD_TRANSACTION_ID        Message ID fails to match up    */
/*    NX_DHCPV6_PROCESSING_ERROR          Packet length is different than */
/*                                                   expected             */
/*    NX_DHCPV6_UNKNOWN_OPTION            Unknown option in message       */
/*    NX_DHCPV6_MISSING_IANA_OPTION       Message has no IANA option      */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_utility_get_block_option_length                          */
/*                                        Extract option header data      */
/*    _nx_dhcpv6_process_client_duid      Extract Client DUID from reply  */ 
/*    _nx_dhcpv6_process_server_duid      Extract Server DUID from reply  */ 
/*    _nx_dhcpv6_process_iana             Extract IANA option from reply  */
/*    _nx_dhcpv6_process_preference       Extract server preference       */
/*    _nx_dhcpv6_process_DNS_server       Extract DNS server from reply   */
/*    _nx_dhcpv6_process_time_server      Extract time server from reply  */
/*    _nx_dhcpv6_process_time_zone        Extract time zone from reply    */
/*    _nx_dhcpv6_process_domain_name      Extract domain name from reply  */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_packet_process           Process the dhcpv6 reply message*/
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcpv6_extract_packet_information(NX_DHCPV6 *dhcpv6_ptr, NX_PACKET *packet_ptr)
{

UINT        status;
ULONG       option_code;
ULONG       option_length;
UCHAR       *buffer_ptr;
UINT        index;

    /* The DHCPv6 header has been processed in preprocess function, so process the options.  */
    /* Set a pointer to the options of DHCPv6. */
    buffer_ptr = packet_ptr -> nx_packet_prepend_ptr + 4;

    /* Update index to the options. */
    index = 4;

    /* Now parse all the DHCPv6 option blocks in the packet buffer. */
    while (index + 4 <= packet_ptr -> nx_packet_length)
    {

        /* Get the option code and length of data of the current option block. */
        status = _nx_dhcpv6_utility_get_block_option_length(buffer_ptr, &option_code, &option_length);

        /* Check that the block data is valid. */
        if (status != NX_SUCCESS)
        {

            /* No, return the error status. */
            return status;
        }

        /* Keep track of how far into the packet we have parsed. */
        index += option_length + 4; 

        /* This is a double check to verify we haven't gone off the end of the packet buffer. */
        if (index > packet_ptr -> nx_packet_length)
        {
            return(NX_DHCPV6_INVALID_DATA_SIZE);
        }

        /* Update buffer pointer to option data.  */
        buffer_ptr += 4;

        /* Process the option code with an option specific API. */
        switch (option_code)
        {

            /* Note - these 'process' functions will not move the buffer pointer. */
             
            case NX_DHCPV6_OP_CLIENT_ID:
            {

                /* Process the Client DUID.  */
                status = _nx_dhcpv6_process_client_duid(dhcpv6_ptr, buffer_ptr, option_length);
                
                break;
            }

            case NX_DHCPV6_OP_SERVER_ID:
            {

                /* Process the Server DUID.  */
                status = _nx_dhcpv6_process_server_duid(dhcpv6_ptr, buffer_ptr, option_length);

                break;
            }
            case NX_DHCPV6_OP_IA_NA:
            {

                status = _nx_dhcpv6_process_iana(dhcpv6_ptr, buffer_ptr, option_length);

                /* Note: this API directly handles any server error codes received. */

                break;
            }

            /* This should not happen. The IA address option must be embedded in the IANA option. */
            case NX_DHCPV6_OP_IA_ADDRESS:
            {

                /* Don't process an IA address option outside of an address association (IANA). */
                status = NX_DHCPV6_MISSING_IANA_OPTION;

                break;
            }

            /* The RFC for DHCPv6 indicates any option can have a status option applied to it. So a status
               option outside of an IANA option (where it is usually found) has to be handled. */
            case NX_DHCPV6_OP_STATUS_CODE:
            {
                status = _nx_dhcpv6_process_status(dhcpv6_ptr, buffer_ptr, option_length);

                /* Now check on the server status of the previous option in the DHCP message. */
                if ((status == NX_SUCCESS) && (dhcpv6_ptr -> nx_status_code != NX_DHCPV6_SUCCESS))
                {

                    /* Does the dhcpv6 client have a server error handler? */
                    if (dhcpv6_ptr -> nx_dhcpv6_server_error_handler)
                    {

                        /* Send the status and which option the status is reported for to the handler. */
                        (dhcpv6_ptr -> nx_dhcpv6_server_error_handler)(dhcpv6_ptr, NX_DHCPV6_ERROR_STATUS_CODE_IN_OPTION_FIELD, dhcpv6_ptr -> nx_status_code,
                                                                       dhcpv6_ptr -> nx_dhcpv6_received_message_type);
                    }
                }

                break;
            }

            /* Process the preference option.  */
            case NX_DHCPV6_OP_PREFERENCE:
            {

                status = _nx_dhcpv6_process_preference(dhcpv6_ptr, buffer_ptr, option_length);
                break;
            }

            /* The remainder are RFC mandated option codes for specific options. Not all of these
               have been finalized by IANA as of this release. */

            case NX_DHCPV6_OP_DNS_SERVER:
            {

                status = _nx_dhcpv6_process_DNS_server(dhcpv6_ptr, buffer_ptr, option_length);

                break;
            }

            case NX_DHCPV6_OP_DOMAIN_NAME:
            {
                status = _nx_dhcpv6_process_domain_name(dhcpv6_ptr, packet_ptr -> nx_packet_prepend_ptr, buffer_ptr, option_length);
                break;
            }

            case NX_DHCPV6_OP_SNTP_SERVER:
            {
            
                status = _nx_dhcpv6_process_time_server(dhcpv6_ptr, buffer_ptr, option_length);
                break;
            }

            case NX_DHCPV6_OP_NEW_POSIX_TIMEZONE:
            {
                status = _nx_dhcpv6_process_time_zone(dhcpv6_ptr, buffer_ptr, option_length);
                break;
            }

            default:
            {
                break;
            }
        }

        /* Check for errors from option block processing. */
        if (status != NX_SUCCESS)
        {

            return status;
        }

        /* Move to the next top level option. */
        buffer_ptr += option_length;
    }

    /* Now, the index should be the packet length.  */
    if(index != packet_ptr -> nx_packet_length)
    {
        return(NX_DHCPV6_INVALID_DATA_SIZE);
    }

    /* Yes, the packet data processing is completed.  */
    return NX_SUCCESS;
}
   

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_flush_queue_packets                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function flush the useless queue packets.                      */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                          Pointer to DHCPv6 Client        */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_receive            Receive UDP packet                 */
/*    nx_packet_release                Release packet to packet pool      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process               Process the DHCPv6 Client request  */
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
VOID  _nx_dhcpv6_flush_queue_packets(NX_DHCPV6 *dhcpv6_ptr)
{   

UINT        status = NX_SUCCESS;
NX_PACKET   *packet_ptr;

    
    /* Loop to receive the queue packets.  */
    while(status == NX_SUCCESS)
    {

        /* Receive the queue packets and release it.  */
        status = nx_udp_socket_receive(&(dhcpv6_ptr -> nx_dhcpv6_socket), &packet_ptr, NX_NO_WAIT);

        /* Check the status.  */
        if (status == NX_SUCCESS)
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);   
        }
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_update_retransmit_info                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function process the retransmission,update the retransmission  */
/*    count and retransmission timeout according to the RFC3315,Section14.*/
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                       Pointer to DHCPV6 Client instance  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */
/*    status                           Actual completion status           */
/*    NX_SUCCESS                       Valid retransmit                   */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_dhcpv6_process               Process current client state       */ 
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
UINT _nx_dhcpv6_update_retransmit_info(NX_DHCPV6 *dhcpv6_ptr)
{
    
    /* Process the retransmission according to the RFC3315, Section 14, page26.  */
    if((dhcpv6_ptr -> nx_dhcpv6_max_retransmission_count != 0) || (dhcpv6_ptr -> nx_dhcpv6_max_retransmission_duration != 0))
    {

        if((dhcpv6_ptr -> nx_dhcpv6_max_retransmission_count != 0) &&
           (dhcpv6_ptr -> nx_dhcpv6_retransmission_count >= dhcpv6_ptr -> nx_dhcpv6_max_retransmission_count))
        {

            /* Has reached the max retransmission count, return. */
            return NX_DHCPV6_REACHED_MAX_RETRANSMISSION_COUNT;
        }
        
        if((dhcpv6_ptr -> nx_dhcpv6_max_retransmission_duration != 0) &&
           (dhcpv6_ptr -> nx_dhcpv6_elapsed_time.nx_session_time >= (dhcpv6_ptr -> nx_dhcpv6_max_retransmission_duration * 100)))
        {

            /* Has reached the max retransmission timeout, return. */
            return NX_DHCPV6_REACHED_MAX_RETRANSMISSION_TIMEOUT;
        }
    }

    /* Increase the count of retransmission. */
    dhcpv6_ptr -> nx_dhcpv6_retransmission_count++;

    /* Double the retransmission timeout for the next retransmit. */
    dhcpv6_ptr -> nx_dhcpv6_transmission_timeout = 2 * dhcpv6_ptr -> nx_dhcpv6_transmission_timeout;

    /* Is the retransmission timeout greater than the max allowable timeout, if there is one? */
    if ((dhcpv6_ptr -> nx_dhcpv6_max_retransmission_timeout != 0) &&
        (dhcpv6_ptr -> nx_dhcpv6_transmission_timeout > dhcpv6_ptr -> nx_dhcpv6_max_retransmission_timeout))
    {

        /* Limit the transmission time to the max for the next message. */
        dhcpv6_ptr -> nx_dhcpv6_transmission_timeout = dhcpv6_ptr -> nx_dhcpv6_max_retransmission_timeout;                        
    }

    /* Can retransmit the request. */
    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_user_option_add_callback_set             PORTABLE C     */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the user option add        */ 
/*    callback set service.                                               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                            Pointer to DHCPv6 instance    */ 
/*    dhcpv6_user_option_add                Pointer to application's      */ 
/*                                            option add function         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_dhcpv6_user_option_add_callback_set Actual user option callback */ 
/*                                            set service                 */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  07-29-2022     Yuxin Zhou               Initial Version 6.1.12        */
/*                                                                        */
/**************************************************************************/
UINT _nxe_dhcpv6_user_option_add_callback_set(NX_DHCPV6 *dhcpv6_ptr, UINT (*dhcpv6_user_option_add)(NX_DHCPV6 *dhcpv6_ptr, UINT interface_index, UINT message_type,
                                                                                                    UCHAR *user_option_ptr, UINT *user_option_length))
{

UINT    status;

    /* Check for invalid input. */
    if ((dhcpv6_ptr == NX_NULL) || (dhcpv6_user_option_add == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual DHCPv6 user option callback set service.  */
    status =  _nx_dhcpv6_user_option_add_callback_set(dhcpv6_ptr, dhcpv6_user_option_add);

    /* Return status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_user_option_add_callback_set             PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the user option add callback.                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                            Pointer to DHCPv6 instance    */ 
/*    dhcpv6_user_option_add                Pointer to application's      */ 
/*                                            option add function         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application Code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  07-29-2022     Yuxin Zhou               Initial Version 6.1.12        */
/*                                                                        */
/**************************************************************************/
UINT _nx_dhcpv6_user_option_add_callback_set(NX_DHCPV6 *dhcpv6_ptr, UINT (*dhcpv6_user_option_add)(NX_DHCPV6 *dhcpv6_ptr, UINT interface_index, UINT message_type,
                                                                                                   UCHAR *user_option_ptr, UINT *user_option_length))
{

    /* Obtain DHCPv6 Client protection mutex. */
    tx_mutex_get(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex), TX_WAIT_FOREVER);

    /* Set the callback.  */
    dhcpv6_ptr -> nx_dhcpv6_user_option_add = dhcpv6_user_option_add;

    /* Release the mutex.  */
    tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

    /* Return a successful status.  */
    return(NX_SUCCESS);
}


#if !defined (NX_DISABLE_IPV6_DAD) && defined (NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY)
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_ipv6_address_DAD_notify                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is the IPv6 address perform DAD notify callback.      */ 
/*    And set the DHCPv6 event to notify DHCPv6 Client thread. If this    */ 
/*    function is used in DHCPv6 function, nxd_ipv6_address_change_notify */ 
/*    function should not be used again.                                  */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    status                                DAD process status            */
/*    interface_index                       The interface index           */
/*    ipv6_addr_index                       The IPv6 address index        */
/*    ipv6_address                          Pointer to IPv6 address       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    tx_event_flags_set                    Set event flags to wake       */
/*                                          DHCPv6 thread to process      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    UDP receive callback                                                */ 
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
VOID _nx_dhcpv6_ipv6_address_DAD_notify(NX_IP *ip_ptr, UINT status, UINT interface_index, UINT ipv6_addr_index, ULONG *ipv6_address)
{
UINT    ia_index;

    NX_PARAMETER_NOT_USED(interface_index);

    /* Make sure the DHCPv6 DAD instance is normal.  */
    if((_nx_dhcpv6_DAD_ptr -> nx_dhcpv6_id != NX_DHCPV6_ID) ||        
       (!_nx_dhcpv6_DAD_ptr -> nx_dhcpv6_name) ||
       (_nx_dhcpv6_DAD_ptr -> nx_dhcpv6_ip_ptr != ip_ptr) ||
       (_nx_dhcpv6_DAD_ptr -> nx_dhcpv6_state != NX_DHCPV6_STATE_BOUND_TO_ADDRESS))
        return;


    /* Check the DAD status.  */
    if(status == NX_IPV6_ADDRESS_DAD_FAILURE)
    {

        /* Now find where this address cross references in the DHCP address list. */
        for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
        {
            
            if((_nx_dhcpv6_DAD_ptr -> nx_dhcpv6_client_address_index[ia_index] == ipv6_addr_index) &&
               (!memcmp(&(_nx_dhcpv6_DAD_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[0]), ipv6_address, 16)))
            {

                /* Set the IPv6 address status DAD failure.  */
                _nx_dhcpv6_DAD_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status = NX_DHCPV6_IA_ADDRESS_STATE_DAD_FAILURE;

                /* Wakeup DHCPv6 thread for processing DAD failure event.  */
                tx_event_flags_set(&_nx_dhcpv6_DAD_ptr -> nx_dhcpv6_events, NX_DHCPV6_DAD_FAILURE_EVENT, TX_OR);

                break;
            }
        }
    }
    
    /* Check the DAD status.  */
    if(status == NX_IPV6_ADDRESS_DAD_SUCCESSFUL)
    {

        for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
        {
            
            if((_nx_dhcpv6_DAD_ptr -> nx_dhcpv6_client_address_index[ia_index] == ipv6_addr_index) &&
               (!memcmp(&(_nx_dhcpv6_DAD_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address.nxd_ip_address.v6[0]), ipv6_address, 16)))
            {

                /* Set the IPv6 address status valid.  */
                _nx_dhcpv6_DAD_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status = NX_DHCPV6_IA_ADDRESS_STATE_VALID;

                break;
            }
        }
    }

    return;
}
#endif     


#ifdef NX_DHCPV6_CLIENT_RESTORE_STATE   
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_client_get_record                       PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the get DHCPv6 Client      */
/*    record service.                                                     */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                            Pointer to DHCPv6 instance    */ 
/*    client_record_ptr                     Pointer to memory to save     */
/*                                             Client record to           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    status                                Completion status from        */
/*                                            internal DHCP calls         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcpv6_client_create_record                                      */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nxe_dhcpv6_client_get_record(NX_DHCPV6 *dhcpv6_ptr, NX_DHCPV6_CLIENT_RECORD *client_record_ptr)
{

UINT status;
                    

    /* Check for invalid pointer input.  */
    if ((dhcpv6_ptr == NX_NULL) || (client_record_ptr == NX_NULL))
    {
        return NX_PTR_ERROR;
    }
                  
    /* Call actual DHCPv6 client get record service.  */
    status = _nx_dhcpv6_client_get_record(dhcpv6_ptr, client_record_ptr);

    return status;               
}
                   
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_client_get_record                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a DHCPv6 Client record for restoring the      */
/*    Client state between power cycles or idle (sleep) mode. It then     */
/*    copies the Client record to the supplied client record pointer. It  */
/*    should be that the DHCPv6 Client state be restored from the Client  */
/*    record saved to the input pointer client_record_ptr.                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                            Pointer to DHCPv6 instance    */ 
/*    client_record_ptr                     Pointer to memory to save     */
/*                                             Client record to           */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */
/*    NX_DHCPV6_NOT_BOUND                   DHCPv6 was not bound          */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memcpy                                Copy specified area of memory */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcpv6_client_get_record(NX_DHCPV6 *dhcpv6_ptr, NX_DHCPV6_CLIENT_RECORD *client_record_ptr)
{


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex), NX_DHCPV6_MUTEX_WAIT);

    /* Clear memory before filling with data. */
    memset(client_record_ptr, 0, sizeof(NX_DHCPV6_CLIENT_RECORD));

    /* The DHCPv6 Client is not Bound to an IP address. Cannot create a record for restoring Client 
       state if not bound to an IP lease. */
    if (dhcpv6_ptr -> nx_dhcpv6_state != NX_DHCPV6_STATE_BOUND_TO_ADDRESS)
    {
        return NX_DHCPV6_NOT_BOUND;
    }

    /* Set the DHCPv6 record.  */         
    client_record_ptr -> nx_dhcpv6_state = dhcpv6_ptr -> nx_dhcpv6_state;

    /* Set the interface and address index.  */
    client_record_ptr -> nx_dhcpv6_client_interface_index = dhcpv6_ptr -> nx_dhcpv6_client_interface_index;
    memcpy(client_record_ptr -> nx_dhcpv6_client_address_index, dhcpv6_ptr -> nx_dhcpv6_client_address_index, sizeof(UINT) * NX_DHCPV6_MAX_IA_ADDRESS); /* Use case of memcpy is verified. */

    /* Set the time since client set its IP address.  */
    client_record_ptr -> nx_dhcpv6_IP_lifetime_time_accrued = dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued;

    /* Set the Client DUID.  */                 
    memcpy(&client_record_ptr -> nx_dhcpv6_client_duid, &dhcpv6_ptr -> nx_dhcpv6_client_duid, sizeof(NX_DHCPV6_DUID)); /* Use case of memcpy is verified. */

    /* Set the Server DUID.  */                 
    memcpy(&client_record_ptr -> nx_dhcpv6_server_duid, &dhcpv6_ptr -> nx_dhcpv6_server_duid, sizeof(NX_DHCPV6_DUID)); /* Use case of memcpy is verified. */

    /* Set the IANA.  */                                           
    memcpy(&client_record_ptr -> nx_dhcpv6_iana, &dhcpv6_ptr -> nx_dhcpv6_iana, sizeof(NX_DHCPV6_IA_NA)); /* Use case of memcpy is verified. */

    /* Set the IA Address.  */      
    memcpy(&client_record_ptr -> nx_dhcpv6_ia, &dhcpv6_ptr -> nx_dhcpv6_ia, sizeof(NX_DHCPV6_IA_ADDRESS) * NX_DHCPV6_MAX_IA_ADDRESS); /* Use case of memcpy is verified. */
                                                                                                                                      
    /* Set the Option request.  */      
    memcpy(&client_record_ptr -> nx_dhcpv6_option_request, &dhcpv6_ptr -> nx_dhcpv6_option_request, sizeof(NX_DHCPV6_OPTIONREQUEST)); /* Use case of memcpy is verified. */
    
    /* Set the Client FQDN.  */      
    memcpy(&client_record_ptr -> nx_dhcpv6_client_FQDN, &dhcpv6_ptr -> nx_dhcpv6_client_FQDN, sizeof(NX_DHCPV6_CLIENT_FQDN)); /* Use case of memcpy is verified. */
    
    /* Set the dns name server address.  */      
    memcpy(&client_record_ptr -> nx_dhcpv6_DNS_name_server_address, &dhcpv6_ptr -> nx_dhcpv6_DNS_name_server_address, sizeof(NXD_ADDRESS) * NX_DHCPV6_NUM_DNS_SERVERS); /* Use case of memcpy is verified. */
    
    /* Set the time server address.  */      
    memcpy(&client_record_ptr -> nx_dhcpv6_time_server_address, &dhcpv6_ptr -> nx_dhcpv6_time_server_address, sizeof(NXD_ADDRESS) * NX_DHCPV6_NUM_TIME_SERVERS); /* Use case of memcpy is verified. */

    /* Set the domain name.  */
    memcpy(client_record_ptr -> nx_dhcpv6_domain_name, dhcpv6_ptr -> nx_dhcpv6_domain_name, NX_DHCPV6_DOMAIN_NAME_BUFFER_SIZE); /* Use case of memcpy is verified. */

    /* Set the time zone.  */
    memcpy(client_record_ptr -> nx_dhcpv6_time_zone, dhcpv6_ptr -> nx_dhcpv6_time_zone, NX_DHCPV6_TIME_ZONE_BUFFER_SIZE); /* Use case of memcpy is verified. */
           
    /* Release the DHCPv6 Client mutex.  */
    tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));

    return NX_SUCCESS;               
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_dhcpv6_client_restore_record                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking on the client restore service.*/
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                            Pointer to DHCPv6 instance    */ 
/*    client_record_ptr                     Pointer to previously saved   */
/*                                             Client record              */
/*    time_elapsed                          time input in timer ticks     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_PTR_ERRPR                          Invalid pointer input         */
/*    status                                NetX completion status        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*   _nx_dhcp_client_restore_record                                       */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
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
UINT  _nxe_dhcpv6_client_restore_record(NX_DHCPV6 *dhcpv6_ptr, NX_DHCPV6_CLIENT_RECORD *client_record_ptr, ULONG time_elapsed)
{

UINT status;           

                      
    /* Check for invalid pointer input.  */
    if ((dhcpv6_ptr == NX_NULL) || (client_record_ptr == NX_NULL))
    {
        return NX_PTR_ERROR;
    }                            

    /* Call actual DHCPv6 client get record service.  */  
    status = _nx_dhcpv6_client_restore_record(dhcpv6_ptr, client_record_ptr, time_elapsed);

    return status;
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_dhcpv6_client_restore_record                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function updates the DHCPv6 CLient state with the supplied DHCP*/
/*    Client record pointed to by the client_record_ptr pointer. It then  */
/*    updates the time out parameters of the DHCPv6 Client with the       */
/*    time_elapsed parameter (in timer ticks). It is intended for         */
/*    restoring Client state between reboots and assumes the DHCPv6 Client*/
/*    state was previously obtained and stored in NVRAM before power down.*/
/*                                                                        */ 
/*    Note: after restore the DHCPv6 Client record, should call           */
/*    nx_dhcpv6_resume to update the DHCPv6 Client state and resume the   */
/*    DHCPv6 processing thread.                                           */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    dhcpv6_ptr                            Pointer to DHCPv6 instance    */ 
/*    client_record_ptr                     Pointer to previously saved   */
/*                                             Client record              */
/*    time_elapsed                          time input in timer ticks     */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                NetX completion status        */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_ip_interface_address_set                                         */
/*    nx_ip_address_set                                                   */
/*    _nx_dhcp_client_update_time_remaining                               */ 
/*    memcpy                                                              */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_dhcpv6_client_restore_record(NX_DHCPV6 *dhcpv6_ptr, NX_DHCPV6_CLIENT_RECORD *client_record_ptr, ULONG time_elapsed)
{               

UINT    ia_index; 


    /* Get the DHCP mutex.  */
    tx_mutex_get(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex), NX_DHCPV6_MUTEX_WAIT);    

    /* Set the DHCPv6 record.  */         
    dhcpv6_ptr -> nx_dhcpv6_state = client_record_ptr -> nx_dhcpv6_state;

    /* Set the interface and address index.  */
    dhcpv6_ptr -> nx_dhcpv6_client_interface_index = client_record_ptr -> nx_dhcpv6_client_interface_index;
    memcpy(dhcpv6_ptr -> nx_dhcpv6_client_address_index, client_record_ptr -> nx_dhcpv6_client_address_index, sizeof(UINT) * NX_DHCPV6_MAX_IA_ADDRESS); /* Use case of memcpy is verified. */

    /* Set the Client DUID.  */                 
    memcpy(&dhcpv6_ptr -> nx_dhcpv6_client_duid, &client_record_ptr -> nx_dhcpv6_client_duid, sizeof(NX_DHCPV6_DUID)); /* Use case of memcpy is verified. */

    /* Set the Server DUID.  */                 
    memcpy(&dhcpv6_ptr -> nx_dhcpv6_server_duid, &client_record_ptr -> nx_dhcpv6_server_duid, sizeof(NX_DHCPV6_DUID)); /* Use case of memcpy is verified. */

    /* Set the IANA.  */                                           
    memcpy(&dhcpv6_ptr -> nx_dhcpv6_iana, &client_record_ptr -> nx_dhcpv6_iana, sizeof(NX_DHCPV6_IA_NA)); /* Use case of memcpy is verified. */

    /* Set the IA Address.  */      
    memcpy(&dhcpv6_ptr -> nx_dhcpv6_ia, &client_record_ptr -> nx_dhcpv6_ia, sizeof(NX_DHCPV6_IA_ADDRESS) * NX_DHCPV6_MAX_IA_ADDRESS); /* Use case of memcpy is verified. */
                                                                                                                                      
    /* Set the Option request.  */      
    memcpy(&dhcpv6_ptr -> nx_dhcpv6_option_request, &client_record_ptr -> nx_dhcpv6_option_request, sizeof(NX_DHCPV6_OPTIONREQUEST)); /* Use case of memcpy is verified. */
    
    /* Set the Client FQDN.  */      
    memcpy(&dhcpv6_ptr -> nx_dhcpv6_client_FQDN, &client_record_ptr -> nx_dhcpv6_client_FQDN, sizeof(NX_DHCPV6_CLIENT_FQDN)); /* Use case of memcpy is verified. */
    
    /* Set the dns name server address.  */      
    memcpy(&dhcpv6_ptr -> nx_dhcpv6_DNS_name_server_address, &client_record_ptr -> nx_dhcpv6_DNS_name_server_address, sizeof(NXD_ADDRESS) * NX_DHCPV6_NUM_DNS_SERVERS); /* Use case of memcpy is verified. */
    
    /* Set the time server address.  */      
    memcpy(&dhcpv6_ptr -> nx_dhcpv6_time_server_address, &client_record_ptr -> nx_dhcpv6_time_server_address, sizeof(NXD_ADDRESS) * NX_DHCPV6_NUM_TIME_SERVERS); /* Use case of memcpy is verified. */

    /* Set the domain name.  */
    memcpy(dhcpv6_ptr -> nx_dhcpv6_domain_name, client_record_ptr -> nx_dhcpv6_domain_name, NX_DHCPV6_DOMAIN_NAME_BUFFER_SIZE); /* Use case of memcpy is verified. */

    /* Set the time zone.  */
    memcpy(dhcpv6_ptr -> nx_dhcpv6_time_zone, client_record_ptr -> nx_dhcpv6_time_zone, NX_DHCPV6_TIME_ZONE_BUFFER_SIZE); /* Use case of memcpy is verified. */
                      
    /* Set the time since client set its IP address.  */
    dhcpv6_ptr -> nx_dhcpv6_IP_lifetime_time_accrued = (client_record_ptr -> nx_dhcpv6_IP_lifetime_time_accrued + time_elapsed);
                             
    /* Set the DHCPv6 Client IPv6 addresses.  */
    for(ia_index = 0; ia_index < NX_DHCPV6_MAX_IA_ADDRESS; ia_index++)
    {

        if(dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_address_status == NX_DHCPV6_IA_ADDRESS_STATE_VALID)
        {

            /* Add the address to the IP instance address table. */
            nxd_ipv6_address_set(dhcpv6_ptr -> nx_dhcpv6_ip_ptr, dhcpv6_ptr -> nx_dhcpv6_client_interface_index, 
                                 &dhcpv6_ptr -> nx_dhcpv6_ia[ia_index].nx_global_address, 64, &dhcpv6_ptr -> nx_dhcpv6_client_address_index[ia_index]);  
        }
    }        

    /* Release the DHCPv6 Client mutex.  */
    tx_mutex_put(&(dhcpv6_ptr -> nx_dhcpv6_client_mutex));
                         
    return NX_SUCCESS;    
}
#endif /* NX_DHCPV6_CLIENT_RESTORE_STATE */
#endif /* FEATURE_NX_IPV6 */
