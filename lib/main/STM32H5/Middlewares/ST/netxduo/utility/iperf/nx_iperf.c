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
/** NetX Utility                                                          */
/**                                                                       */
/**   NetX Duo IPerf Test Program                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include   "tx_api.h"
#include   "nx_api.h"
#include   "nx_iperf.h"
#ifndef NX_WEB_HTTP_NO_FILEX
#include   "fx_api.h"
#else
#include   "filex_stub.h"
#endif

#include   "nx_web_http_server.h"

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
#include   "tx_execution_profile.h"
#endif /* TX_EXECUTION_PROFILE_ENABLE */

/* Define the counters used in the demo application...  */

NX_WEB_HTTP_SERVER nx_iperf_web_server;
FX_MEDIA           nx_iperf_ram_disk;
NX_IP             *nx_iperf_test_ip;
NX_PACKET_POOL    *nx_iperf_test_pool;
UCHAR             *nx_iperf_stack_area;
ULONG              nx_iperf_stack_area_size;
ULONG              nx_iperf_test_error_counter;
ctrl_info          nx_iperf_ctrl_info;

static NXD_ADDRESS udp_tx_ip_address;
static NXD_ADDRESS tcp_tx_ip_address;

static ULONG       udp_tx_port = NX_IPERF_DESTINATION_PORT;
static ULONG       tcp_tx_port = NX_IPERF_DESTINATION_PORT;

static UINT        udp_tx_packet_size = 1470;
static UINT        udp_tx_test_time = 10;
static UINT        udp_rx_test_time = 10;
static UINT        tcp_tx_test_time = 10;
static UINT        tcp_rx_test_time = 10;

static ULONG       error_counter;

NX_TCP_SOCKET      tcp_server_socket;
NX_TCP_SOCKET      tcp_client_socket;
NX_UDP_SOCKET      udp_server_socket;
NX_UDP_SOCKET      udp_client_socket;
ULONG              thread_tcp_rx_counter;
ULONG              thread_tcp_tx_counter;
ULONG              thread_udp_rx_counter;
ULONG              thread_udp_tx_counter;
static TX_THREAD   thread_tcp_rx_iperf;
static TX_THREAD   thread_tcp_tx_iperf;
static TX_THREAD   thread_udp_rx_iperf;
static TX_THREAD   thread_udp_tx_iperf;

extern ULONG       _tx_timer_system_clock;
#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
EXECUTION_TIME     thread_time = 0;
EXECUTION_TIME     isr_time = 0;
EXECUTION_TIME     idle_time = 0;
extern TX_THREAD  *_tx_thread_created_ptr;
#endif


static void nx_iperf_send_image(NX_WEB_HTTP_SERVER *server_ptr, UCHAR *img, UINT imgsize);
static void nx_iperf_print_tcp_rx_results(NX_WEB_HTTP_SERVER *server_ptr, NXD_ADDRESS *peer_ip_address);
static void nx_iperf_print_tcp_tx_results(NX_WEB_HTTP_SERVER *server_ptr, NXD_ADDRESS *peer_ip_address);
static void nx_iperf_print_udp_rx_results(NX_WEB_HTTP_SERVER *server_ptr, NXD_ADDRESS *peer_ip_address);
static void nx_iperf_print_udp_tx_results(NX_WEB_HTTP_SERVER *server_ptr, NXD_ADDRESS *peer_ip_address);

void        nx_iperf_tcp_rx_test(UCHAR *, ULONG);
void        nx_iperf_tcp_tx_test(UCHAR *, ULONG);
void        nx_iperf_udp_rx_test(UCHAR *, ULONG);
void        nx_iperf_udp_tx_test(UCHAR *, ULONG);

void        nx_iperf_tcp_rx_cleanup(void);
void        nx_iperf_tcp_tx_cleanup(void);
void        nx_iperf_udp_rx_cleanup(void);
void        nx_iperf_udp_tx_cleanup(void);

char       *nx_iperf_get_ip_addr_string(NXD_ADDRESS *ip_address, UINT *string_length);
void        nx_iperf_test_info_parse(ctrl_info *iperf_ctrlInfo_ptr);
void        nx_iperf_tcp_rx_connect_received(NX_TCP_SOCKET *socket_ptr, UINT port);
void        nx_iperf_tcp_rx_disconnect_received(NX_TCP_SOCKET *socket_ptr);
UINT        nx_iperf_authentication_check(struct NX_WEB_HTTP_SERVER_STRUCT *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm);
UINT        nx_iperf_get_notify(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr);
void        nx_iperf_entry(NX_PACKET_POOL *pool_ptr, NX_IP *ip_ptr, UCHAR *http_stack, ULONG http_stack_size, UCHAR *iperf_stack, ULONG iperf_stack_size);

void        nx_iperf_thread_tcp_rx_entry(ULONG thread_input);
void        nx_iperf_thread_tcp_tx_entry(ULONG thread_input);
void        nx_iperf_thread_udp_tx_entry(ULONG thread_input);
void        nx_iperf_thread_udp_rx_entry(ULONG thread_input);

void    nx_iperf_entry(NX_PACKET_POOL *pool_ptr, NX_IP *ip_ptr, UCHAR *http_stack, ULONG http_stack_size, UCHAR *iperf_stack, ULONG iperf_stack_size)
{
UINT status;

    /* Create the HTTP Server.  */
    status =  nx_web_http_server_create(&nx_iperf_web_server, "My HTTP Server", ip_ptr, NX_WEB_HTTP_SERVER_PORT, &nx_iperf_ram_disk, http_stack, http_stack_size, pool_ptr, nx_iperf_authentication_check, nx_iperf_get_notify);

    /* Check the status.  */
    if (status)
    {

        /* Update the error counter and return.  */
        nx_iperf_test_error_counter++;
        return;
    }

    /* Set the iPerf Stack and Size.  */
    nx_iperf_stack_area = iperf_stack;
    nx_iperf_stack_area_size = iperf_stack_size;

    /* Set the IP instance and Packet Pool.  */
    nx_iperf_test_ip = ip_ptr;
    nx_iperf_test_pool = pool_ptr;

    /* Start the HTTP Server.  */
    status =  nx_web_http_server_start(&nx_iperf_web_server);

    /* Check the status.  */
    if (status)
    {

        /* Update the error counter and return.  */
        nx_iperf_test_error_counter++;
        return;
    }
}

UINT    nx_iperf_authentication_check(struct NX_WEB_HTTP_SERVER_STRUCT *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm)
{


#ifdef NX_IPERF_AUTH_ENABLE

    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(request_type);
    NX_PARAMETER_NOT_USED(resource);
    *name =  "PlaceholderName";
    *password = "PlaceholderPassword";
    *realm =  "test.htm";

    return(NX_WEB_HTTP_BASIC_AUTHENTICATE);
#else
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(request_type);
    NX_PARAMETER_NOT_USED(resource);
    NX_PARAMETER_NOT_USED(name);
    NX_PARAMETER_NOT_USED(password);
    NX_PARAMETER_NOT_USED(realm);

    return(NX_SUCCESS);
#endif
}

static CHAR device_ip_addr_string[40];
char *nx_iperf_get_ip_addr_string(NXD_ADDRESS *ip_address, UINT *string_length)
{
UINT length = 0;
#ifdef FEATURE_NX_IPV6
UINT i;
#endif

    memset(device_ip_addr_string, 0, sizeof(device_ip_addr_string));

#ifdef FEATURE_NX_IPV6
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {
        for (i = 0; i < 4; i ++)
        {
            length += _nx_utility_uint_to_string((UINT)ip_address -> nxd_ip_address.v6[i] >> 16, 16, &device_ip_addr_string[length], sizeof(device_ip_addr_string) - length);
            device_ip_addr_string[length++] = ':';
            length += _nx_utility_uint_to_string((UINT)ip_address -> nxd_ip_address.v6[i] & 0xFFFF, 16, &device_ip_addr_string[length], sizeof(device_ip_addr_string) - length);
            if (i != 3)
            {
                device_ip_addr_string[length++] = ':';
            }
        }
    }
    else
#endif
    {
#ifndef NX_DISABLE_IPV4
        length = _nx_utility_uint_to_string(ip_address -> nxd_ip_address.v4 >> 24, 10, device_ip_addr_string, sizeof(device_ip_addr_string) - length);
        device_ip_addr_string[length++] = '.';
        length += _nx_utility_uint_to_string(((ip_address -> nxd_ip_address.v4 >> 16) & 0xFF), 10, &device_ip_addr_string[length], sizeof(device_ip_addr_string) - length);
        device_ip_addr_string[length++] = '.';
        length += _nx_utility_uint_to_string(((ip_address -> nxd_ip_address.v4 >> 8) & 0xFF), 10, &device_ip_addr_string[length], sizeof(device_ip_addr_string) - length);
        device_ip_addr_string[length++] = '.';
        length += _nx_utility_uint_to_string((ip_address -> nxd_ip_address.v4 & 0xFF), 10, &device_ip_addr_string[length], sizeof(device_ip_addr_string) - length);
#endif
    }

    *string_length = length;
    return device_ip_addr_string;
}

/* This function takes the token/value pair, and stores the information in the ctrl_info_ptr. */
/* For example, a token/value pair can be: "TestType"="TC_Rx", and the ctrl_info_ptr stores the information. */
static void nx_iperf_check_token_value(char *token, char *value_ptr, ctrl_info *ctrl_info_ptr)
{
UINT val;
UINT i;
UINT token_length = 0;
UINT value_length = 0;
UINT status;

    /* Check string length of token.  */
    status = _nx_utility_string_length_check(token, &token_length, NX_MAX_STRING_LENGTH);
    if (status)
    {
        return;
    }

    /* Check string length of value.  */
    if (value_ptr)
    {
        status = _nx_utility_string_length_check(value_ptr, &value_length, NX_MAX_STRING_LENGTH);
        if (status)
        {
            return;
        }
    }

    /* Check for name. */
    if ((token_length == sizeof("TestType") - 1 ) &&
        (memcmp(token, "TestType", token_length) == 0))
    {
        /* Check for value. */
        ctrl_info_ptr -> ctrl_sign = UNKNOWN_TEST;
        if (value_ptr)
        {
            if ((value_length == sizeof(TCP_Rx) - 1 ) &&
                (memcmp(value_ptr, TCP_Rx, value_length) == 0))
            {
                ctrl_info_ptr -> ctrl_sign = TCP_RX_START;
            }
            else if ((value_length == sizeof(TCP_Tx) - 1 ) &&
                     (memcmp(value_ptr, TCP_Tx, value_length) == 0))
            {
                ctrl_info_ptr -> ctrl_sign = TCP_TX_START;
            }
            else if ((value_length == sizeof(UDP_Rx) - 1 ) &&
                     (memcmp(value_ptr, UDP_Rx, value_length) == 0))
            {
                ctrl_info_ptr -> ctrl_sign = UDP_RX_START;
            }
            else if ((value_length == sizeof(UDP_Tx) - 1 ) &&
                     (memcmp(value_ptr, UDP_Tx, value_length) == 0))
            {
                ctrl_info_ptr -> ctrl_sign = UDP_TX_START;
            }
        }
    }
    else if ((token_length == (sizeof("ip") - 1)) &&
             (memcmp(token, "ip", token_length) == 0))
    {
    char *ptr = value_ptr;
    int   colon_sum, colon_count;
        ctrl_info_ptr -> ip = 0;
        val = 0;
        colon_sum = 0;
        colon_count = 0;
        if (value_ptr == 0)
        {
            ctrl_info_ptr -> version = NX_IP_VERSION_V4;
        }
        while (ptr && (*ptr != 0))
        {
            if (*ptr == '.')
            {
                ctrl_info_ptr -> version = NX_IP_VERSION_V4;
                while (value_ptr && (*value_ptr != 0))
                {
                    if (*value_ptr == '.')
                    {
                        ctrl_info_ptr -> ip = (ctrl_info_ptr -> ip << 8) + val;
                        val = 0;
                    }
                    else
                    {
                        val = val * 10 + ((UINT)(*value_ptr - '0'));
                    }
                    value_ptr++;
                }
                ctrl_info_ptr -> ip = (ctrl_info_ptr -> ip << 8) + val;
                break;
            }
            else if (*ptr == '%')
            {
                if ((*(++ptr) == '3') && (*(++ptr) == 'A'))
                {
                    ctrl_info_ptr -> version = NX_IP_VERSION_V6;
                    colon_sum++;
                }
            }
            ptr++;
        }
        while (value_ptr && (*value_ptr != 0) && (colon_sum != 0))
        {
            if (*value_ptr == '%')
            {
                if ((*(++value_ptr) == '3') && (*(++value_ptr) == 'A'))
                {
                    ctrl_info_ptr -> ipv6[colon_count / 2] = (ctrl_info_ptr -> ipv6[colon_count / 2] << 16) + val;
                    colon_count++;

                    if (*(value_ptr + 1) == '%')
                    {
                        value_ptr++;
                        if ((*(++value_ptr) == '3') && (*(++value_ptr) == 'A'))
                        {
                            for (i = 0; i <= (UINT)(7 - colon_sum); i++)
                            {
                                ctrl_info_ptr -> ipv6[colon_count / 2] = ctrl_info_ptr -> ipv6[colon_count / 2] << 16;
                                colon_count++;
                            }
                        }
                    }
                    val = 0;
                }
            }
            else
            {
                if (*value_ptr >= '0' && *value_ptr <= '9')
                {
                    val = val * 16 + ((UINT)(*value_ptr - '0'));
                }
                else if (*value_ptr >= 'a' && *value_ptr <= 'f')
                {
                    val = val * 16 + ((UINT)(*value_ptr - 'a')) + 10;
                }
                else if (*value_ptr >= 'A' && *value_ptr <= 'F')
                {
                    val = val * 16 + ((UINT)(*value_ptr - 'A')) + 10;
                }
            }
            value_ptr++;
        }
        if (ctrl_info_ptr -> version == NX_IP_VERSION_V6)
        {
            ctrl_info_ptr -> ipv6[3] = (ctrl_info_ptr -> ipv6[3] << 16) + val;
        }
    }
    else if ((token_length == (sizeof("test_time") - 1)) && 
             (memcmp(token, "test_time", token_length) == 0))
    {
        ctrl_info_ptr -> TestTime = 0;
        while (value_ptr && (*value_ptr != 0))
        {
            ctrl_info_ptr -> TestTime = ctrl_info_ptr -> TestTime * 10 + ((UINT)(*value_ptr - '0'));
            value_ptr++;
        }
        ctrl_info_ptr -> TestTime = ctrl_info_ptr -> TestTime * NX_IP_PERIODIC_RATE;
    }
    else if ((token_length == (sizeof("rate") - 1)) && 
             (memcmp(token, "rate", token_length) == 0))
    {
        ctrl_info_ptr -> Rate = 0;
        while (value_ptr && (*value_ptr != 0))
        {
            ctrl_info_ptr -> Rate = ctrl_info_ptr -> Rate * 10 + ((UINT)(*value_ptr - '0'));
            value_ptr++;
        }
    }
    else if ((token_length == (sizeof("size") - 1)) && 
             (memcmp(token, "size", token_length) == 0))
    {
        ctrl_info_ptr -> PacketSize = 0;
        while (value_ptr && (*value_ptr != 0))
        {
            ctrl_info_ptr -> PacketSize = ctrl_info_ptr -> PacketSize * 10 + ((UINT)(*value_ptr - '0'));
            value_ptr++;
        }
    }
    else if ((token_length == (sizeof("port") - 1)) && 
             (memcmp(token, "port", token_length) == 0))
    {
        ctrl_info_ptr -> port = 0;
        while (value_ptr && (*value_ptr != 0))
        {
            ctrl_info_ptr -> port = ctrl_info_ptr -> port * 10 + ((UINT)(*value_ptr - '0'));
            value_ptr++;
        }
    }
}

/* This function parses the incoming HTTP command line.  For each token/value pair, it
   invokes the nx_iperf_check_token_value routine to parse the values. */
static void nx_iperf_parse_command(NX_PACKET *packet_ptr, ctrl_info *ctrl_info_ptr)
{
UCHAR *cmd_string = packet_ptr -> nx_packet_prepend_ptr;
UCHAR *token = NX_NULL;
UCHAR *end_cmd;
UCHAR *next_token;
UCHAR *value_ptr;

    /* At this point, cmd_string points to the beginning of the HTTP request,
       which takes the form of:
       "GET /test.htm?TestType=xxxxx&ip=....&rxed_pkts=xxxx&test_time=xxxx&tputs=xxxx" */

    /* First skip the "Get /test.html?" string. */
    cmd_string += (sizeof("GET /test.htm?") - 1);

    /* Find the end of the cmd string, */
    end_cmd = cmd_string;
    while (end_cmd < packet_ptr -> nx_packet_append_ptr)
    {
        if (*end_cmd == ' ')
        {
            break;
        }
        end_cmd++;
    }
    *end_cmd = 0;

    /* The first token starts from cmd_string. */
    token = cmd_string;
    next_token = cmd_string;
    while (next_token < end_cmd)
    {
        /* Find the next token .*/
        while (next_token < end_cmd)
        {
            if (*next_token == '=')
            {
                break;
            }
            next_token++;
        }

        if (*next_token == '=')
        {
            /* Find a name=value pair. Now we need to find the "=" sign. */
            *next_token = 0;
            value_ptr = next_token + 1;
            next_token++;

            while (next_token < end_cmd)
            {
                if (*next_token == '&')
                {
                    *next_token = 0;

                    break;
                }
                next_token++;
            }

            if (value_ptr == next_token)
            {
                /* There is no value string.  */
                value_ptr = NX_NULL;
            }

            /* Move next_token beyond the NULL terminator. */
            next_token++;


            nx_iperf_check_token_value((char *)token, (char *)value_ptr, ctrl_info_ptr);
        }
        token = next_token;
    }

    /* Finished parsing the whole command. */
}

static CHAR mytempstring[30];
static VOID nx_iperf_print_main_test_window(NX_WEB_HTTP_SERVER *server_ptr)
{
NX_PACKET  *resp_packet_ptr;
UINT        status;
NXD_ADDRESS server_ip;
UINT        length = 0;
CHAR       *ip_addr_string;

#ifdef FEATURE_NX_IPV6
UINT  address_index;
ULONG prefix_length;
UINT  interface_index;
#endif

    status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr, &resp_packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);

    /* write HTML code into the packet */
    /* htmlwrite(p,s,l)  (nx_packet_data_append(p,s,l, server_ptr-> nx_web_http_server_packet_pool_ptr,NX_WAIT_FOREVER)) */

    status += htmlwrite(resp_packet_ptr, outtermosttable, sizeof(outtermosttable) - 1);
    status += htmlwrite(resp_packet_ptr, maintabletag, sizeof(maintabletag) - 1);

    /* print the IP address line. */
    status += htmlwrite(resp_packet_ptr, h1line1, sizeof(h1line1) - 1);

#ifndef NX_DISABLE_IPV4
    server_ip.nxd_ip_version = NX_IP_VERSION_V4;
    server_ip.nxd_ip_address.v4 = nx_iperf_test_ip -> nx_ip_interface[0].nx_interface_ip_address;
    ip_addr_string = nx_iperf_get_ip_addr_string(&server_ip, &length);
    status += htmlwrite(resp_packet_ptr, ip_addr_string, length);
    status += htmlwrite(resp_packet_ptr, "\n", sizeof("\n") - 1);
#endif

#ifdef FEATURE_NX_IPV6
    address_index = 0;

    /* Loop to output the IPv6 address.  */
    while (1)
    {

        /* Get the IPv6 address.  */
        if (nxd_ipv6_address_get(nx_iperf_test_ip, address_index, &server_ip, &prefix_length, &interface_index) == NX_SUCCESS)
        {
            ip_addr_string = nx_iperf_get_ip_addr_string(&server_ip, &length);
            status += htmlwrite(resp_packet_ptr, ip_addr_string, length);
            status += htmlwrite(resp_packet_ptr, "\n", sizeof("\n") - 1);
            address_index++;
        }
        else
        {
            break;
        }
    }
#endif

    status += htmlwrite(resp_packet_ptr, h1line2, sizeof(h1line2) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);
    status += nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);

    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        nx_iperf_test_error_counter++;
    }

    status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr, &resp_packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);
    status += htmlwrite(resp_packet_ptr, udptxsubmittag1, sizeof(udptxsubmittag1) - 1);
    ip_addr_string = nx_iperf_get_ip_addr_string(&udp_tx_ip_address, &length);
    status += htmlwrite(resp_packet_ptr, ip_addr_string, length);
    status += htmlwrite(resp_packet_ptr, udptxsubmittag2, sizeof(udptxsubmittag2) - 1);
    length = _nx_utility_uint_to_string(udp_tx_port, 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, udptxsubmittag3, sizeof(udptxsubmittag3) - 1);
    length = _nx_utility_uint_to_string(udp_tx_test_time, 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, udptxsubmittag4, sizeof(udptxsubmittag4) - 1);
    length = _nx_utility_uint_to_string(udp_tx_packet_size, 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, udptxsubmittag5, sizeof(udptxsubmittag5) - 1);
    status += nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);

    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        nx_iperf_test_error_counter++;
    }

    status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr, &resp_packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);
    status += htmlwrite(resp_packet_ptr, udprxsubmittag1, sizeof(udprxsubmittag1) - 1);
    length = _nx_utility_uint_to_string(udp_rx_test_time, 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, udprxsubmittag2, sizeof(udprxsubmittag2) - 1);
    status += nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);

    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        nx_iperf_test_error_counter++;
    }

    status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr, &resp_packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);
    status += htmlwrite(resp_packet_ptr, tcptxsubmittag1, sizeof(tcptxsubmittag1) - 1);
    ip_addr_string = nx_iperf_get_ip_addr_string(&tcp_tx_ip_address, &length);
    status += htmlwrite(resp_packet_ptr, ip_addr_string, length);
    status += htmlwrite(resp_packet_ptr, tcptxsubmittag2, sizeof(tcptxsubmittag2) - 1);
    length = _nx_utility_uint_to_string(tcp_tx_port, 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, tcptxsubmittag3, sizeof(tcptxsubmittag3) - 1);
    length = _nx_utility_uint_to_string(tcp_tx_test_time, 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, tcptxsubmittag4, sizeof(tcptxsubmittag4) - 1);
    status += nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);

    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        nx_iperf_test_error_counter++;
    }

    status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr, &resp_packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);
    status += htmlwrite(resp_packet_ptr, tcprxsubmittag1, sizeof(tcprxsubmittag1) - 1);
    length = _nx_utility_uint_to_string(tcp_rx_test_time, 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, tcprxsubmittag2, sizeof(tcprxsubmittag2) - 1);
    status += nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);

    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        nx_iperf_test_error_counter++;
    }

    status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr, &resp_packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);
    status += htmlwrite(resp_packet_ptr, tableendtag, sizeof(tableendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);

    status += nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);

    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        nx_iperf_test_error_counter++;
    }
}

static void nx_iperf_print_end_of_page(NX_WEB_HTTP_SERVER *server_ptr)
{
UINT       status;
NX_PACKET *resp_packet_ptr;

    status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr, &resp_packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);

    /* End of the page. */
    status += htmlwrite(resp_packet_ptr, tableendtag, sizeof(tableendtag) - 1); /* outtermost table. */
    status += htmlwrite(resp_packet_ptr, doublebr, sizeof(doublebr) - 1);
    status += htmlwrite(resp_packet_ptr, centerendtag, sizeof(centerendtag) - 1);
    status += htmlwrite(resp_packet_ptr, bodyendtag, sizeof(bodyendtag) - 1);
    status += htmlwrite(resp_packet_ptr, htmlendtag, sizeof(htmlendtag) - 1);

    status += nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);

    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        nx_iperf_test_error_counter++;
    }
}

static void nx_iperf_print_header(NX_WEB_HTTP_SERVER *server_ptr)
{
NX_PACKET *resp_packet_ptr;
UINT       status;

    status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr,
                                &resp_packet_ptr,
                                NX_TCP_PACKET,
                                NX_WAIT_FOREVER);


    /* write HTML code into the packet */
    /* htmlwrite(p,s,l)  (nx_packet_data_append(p,s,l, server_ptr-> nx_web_http_server_packet_pool_ptr,NX_WAIT_FOREVER)) */

    status += htmlwrite(resp_packet_ptr, htmlresponse, sizeof(htmlresponse) - 1);
    status += htmlwrite(resp_packet_ptr, htmltag, sizeof(htmltag) - 1);
    status += htmlwrite(resp_packet_ptr, titleline, sizeof(titleline) - 1);
    status += htmlwrite(resp_packet_ptr, bodytag, sizeof(bodytag) - 1);
    status += htmlwrite(resp_packet_ptr, logo_area, sizeof(logo_area) - 1);
    status += htmlwrite(resp_packet_ptr, hrline, sizeof(hrline) - 1);
    status += htmlwrite(resp_packet_ptr, centertag, sizeof(centertag) - 1);

    status += nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);

    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        nx_iperf_test_error_counter++;
    }
}


static void nx_iperf_print_test_inprogress(NX_WEB_HTTP_SERVER *server_ptr, char *msg)
{
NX_PACKET *resp_packet_ptr;
UINT       status;
UINT       length = 0;

    status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr,
                                &resp_packet_ptr,
                                NX_TCP_PACKET,
                                NX_WAIT_FOREVER);
    status += htmlwrite(resp_packet_ptr, tdcentertag, sizeof(tdcentertag) - 1);
    status += htmlwrite(resp_packet_ptr, fontcolortag, sizeof(fontcolortag) - 1);
    _nx_utility_string_length_check(msg, &length, NX_MAX_STRING_LENGTH);
    status += htmlwrite(resp_packet_ptr, msg, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);

    status += nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);

    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        nx_iperf_test_error_counter++;
    }
}

static void nx_iperf_print_empty_line(NX_WEB_HTTP_SERVER *server_ptr)
{
NX_PACKET *resp_packet_ptr;
UINT       status = 0;

    status += nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr, &resp_packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);
    status += htmlwrite(resp_packet_ptr, choosetesttag, sizeof(choosetesttag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);
    status += nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);

    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        nx_iperf_test_error_counter++;
    }
}

static void nx_iperf_send_test_result_info(NX_WEB_HTTP_SERVER *server_ptr)
{
char       *response_string = NX_NULL;
NXD_ADDRESS peer_ip_address;

    if (nx_iperf_ctrl_info.TestStatus == 0)
    {
        nx_iperf_print_empty_line(server_ptr);

        /* No test is running.  Do nothing. */
        return;
    }
    else if (nx_iperf_ctrl_info.TestStatus == 1)
    {
        switch (nx_iperf_ctrl_info.ctrl_sign)
        {
        case UDP_RX_START:
            response_string = "UDP Receive Test started.  After the iperf test finishes, click <a href=\"/\">here</a> to get results.";
            break;
        case TCP_RX_START:
            response_string = "TCP Receive Test started.  After the iperf test finishes, click <a href=\"/\">here</a> to get results.";
            break;
        case UDP_TX_START:
            response_string = "UDP Transmit Test starts in 2 seconds.  After iperf test is done, click <a href=\"/\">here</a> to get results.";
            break;
        case TCP_TX_START:
            response_string = "TCP Transmit test starts in 2 seconds.  After iperf test is done, click <a href=\"/\">here</a> to get results.";
            break;
        }
        nx_iperf_print_test_inprogress(server_ptr, response_string);
    }
    else if (nx_iperf_ctrl_info.TestStatus == 2)
    {

        /* Check the ThroughPut value and StartTime,
           if the StartTime is zero means no connection.
           if throughput value is zero means maybe user interrupt the test.
           recalculate the ThroughPut before Interrupt.  */
        if ((!nx_iperf_ctrl_info.ThroughPut) && (nx_iperf_ctrl_info.StartTime))
        {

            /* Calculate the run time and Throughput(Mbps).  */
            nx_iperf_ctrl_info.RunTime = tx_time_get() - nx_iperf_ctrl_info.StartTime;

            /* Check the run time.  */
            if (nx_iperf_ctrl_info.RunTime > nx_iperf_ctrl_info.TestTime)
            {
                nx_iperf_ctrl_info.RunTime = nx_iperf_ctrl_info.TestTime;
            }

            /* Calculate Throughput(Mbps).  */
            nx_iperf_ctrl_info.ThroughPut = (nx_iperf_ctrl_info.BytesTxed + nx_iperf_ctrl_info.BytesRxed) / nx_iperf_ctrl_info.RunTime * NX_IP_PERIODIC_RATE / 125000;
        }

        peer_ip_address.nxd_ip_version = nx_iperf_ctrl_info.version;

#ifdef FEATURE_NX_IPV6
        if (peer_ip_address.nxd_ip_version == NX_IP_VERSION_V6)
        {
            peer_ip_address.nxd_ip_address.v6[0] = nx_iperf_ctrl_info.ipv6[0];
            peer_ip_address.nxd_ip_address.v6[1] = nx_iperf_ctrl_info.ipv6[1];
            peer_ip_address.nxd_ip_address.v6[2] = nx_iperf_ctrl_info.ipv6[2];
            peer_ip_address.nxd_ip_address.v6[3] = nx_iperf_ctrl_info.ipv6[3];
        }
        else
#endif
        {
#ifndef NX_DISABLE_IPV4
            peer_ip_address.nxd_ip_address.v4 = nx_iperf_ctrl_info.ip;
#endif
        }

        switch (nx_iperf_ctrl_info.ctrl_sign)
        {
        case UDP_RX_START:
            nx_iperf_print_udp_rx_results(server_ptr, &peer_ip_address);
            break;
        case TCP_RX_START:
            nx_iperf_print_tcp_rx_results(server_ptr, &peer_ip_address);
            break;
        case UDP_TX_START:
            nx_iperf_print_udp_tx_results(server_ptr, &peer_ip_address);
            break;
        case TCP_TX_START:
            nx_iperf_print_tcp_tx_results(server_ptr, &peer_ip_address);
            break;
        }
        memset(&nx_iperf_ctrl_info, 0, sizeof(nx_iperf_ctrl_info));
    }
}

UINT    nx_iperf_get_notify(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr)
{
ctrl_info new_cmd;
UINT      status;
ULONG     port;
UINT      length;

    NX_PARAMETER_NOT_USED(request_type);

    memset(&new_cmd, 0, sizeof(ctrl_info));

    /* Get peer IP address.  */
    status = nxd_tcp_socket_peer_info_get(&server_ptr -> nx_web_http_server_current_session_ptr -> nx_tcp_session_socket, &udp_tx_ip_address, &port);
    if (status)
    {
        return(status);
    }
    tcp_tx_ip_address = udp_tx_ip_address;

    /* Disconnect to mark the end of respond. */
#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
    server_ptr -> nx_web_http_server_keepalive = NX_FALSE;
#endif

    _nx_utility_string_length_check(resource, &length, NX_WEB_HTTP_MAX_RESOURCE);
    if (((length == sizeof("/test.htm") - 1) && (memcmp(resource, "/test.htm", length) == 0)) ||
        ((length == 1) && *resource == '/'))
    {

        /* Printer the header.  */
        nx_iperf_print_header(server_ptr);

        /* Parse the command.  */
        nx_iperf_parse_command(packet_ptr, &new_cmd);

        /* If the current test is still running, and we have a new command,
           we need to clean up the current one. */
        if (new_cmd.ctrl_sign)
        {
            if ((nx_iperf_ctrl_info.TestStatus != 0) || ((nx_iperf_ctrl_info.ctrl_sign & NX_IPERF_CLEAN_UP_MASK) == 1))
            {

                switch (nx_iperf_ctrl_info.ctrl_sign)
                {
                case TCP_RX_START:
                    nx_iperf_tcp_rx_cleanup();
                    break;
                case TCP_TX_START:
                    nx_iperf_tcp_tx_cleanup();
                    break;
                case UDP_RX_START:
                    nx_iperf_udp_rx_cleanup();
                    break;
                case UDP_TX_START:
                    nx_iperf_udp_tx_cleanup();
                    break;
                default:
                    break;
                }
                memset(&nx_iperf_ctrl_info, 0, sizeof(nx_iperf_ctrl_info));
            }

            memcpy(&nx_iperf_ctrl_info, &new_cmd, sizeof(ctrl_info)); /* Use case of memcpy is verified. */

            /* Create the test thread and run the test.  */
            nx_iperf_test_info_parse(&nx_iperf_ctrl_info);

            /* Update the TestStatus.  */
            nx_iperf_ctrl_info.TestStatus = 1;
        }

        /* Check the status, set the default value.  */
        if (nx_iperf_ctrl_info.TestStatus == 0)
        {

            /* Check the IP version.  */
            if (udp_tx_ip_address.nxd_ip_version == NX_IP_VERSION_V4)
            {
                udp_tx_packet_size = 1470;
            }
            else
            {
                udp_tx_packet_size = 1450;
            }
        }

        /* Print the main window.  */
        nx_iperf_print_main_test_window(server_ptr);

        /* If there is a new command, show the result of launching the command. */
        nx_iperf_send_test_result_info(server_ptr);

        nx_iperf_print_end_of_page(server_ptr);

        /* Update the TestStatus.  */
        if (nx_iperf_ctrl_info.TestStatus == 1)
        {
            nx_iperf_ctrl_info.TestStatus = 2;
        }

        return(NX_WEB_HTTP_CALLBACK_COMPLETED);
    }
    /* send the logo files */
    if ((length == sizeof("/nxlogo.png") - 1) && (memcmp(resource, "/nxlogo.png", length) == 0))
    {
        nx_iperf_send_image(server_ptr, (UCHAR *)nxlogo_png, nxlogo_png_size);
        return(NX_WEB_HTTP_CALLBACK_COMPLETED);
    }

    if ((length == sizeof("/mslogo.jpg") - 1) && (memcmp(resource, "/mslogo.jpg", length) == 0))
    {
        nx_iperf_send_image(server_ptr, (UCHAR *)mslogo_jpg, mslogo_jpg_size);
        return(NX_WEB_HTTP_CALLBACK_COMPLETED);
    }

    return(NX_SUCCESS);
}

static void nx_iperf_send_image(NX_WEB_HTTP_SERVER *server_ptr, UCHAR *img, UINT imgsize)
{

UINT       remaining;
UCHAR     *position;
UINT       max_size;
UINT       status;
NX_PACKET *resp_packet_ptr;

    /* Generate HTTP header.  */
    status = nx_web_http_server_callback_generate_response_header(server_ptr,
                                                                  &resp_packet_ptr, NX_WEB_HTTP_STATUS_OK, imgsize, "image/jpeg", NX_NULL);
    if (status)
    {
        return;
    }

    status = nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);
    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        return;
    }

    status = nx_tcp_socket_mss_get(&(server_ptr -> nx_web_http_server_current_session_ptr -> nx_tcp_session_socket), (ULONG *)&max_size);
    if (status)
    {
        return;
    }

    remaining = imgsize;
    position = img;
    while (remaining)
    {
        if (remaining > max_size)
        {
            nx_web_http_server_callback_data_send(server_ptr, (void *)position, max_size);
            position += max_size;
            remaining -= max_size;
        }
        else
        {
            nx_web_http_server_callback_data_send(server_ptr, (void *)position, remaining);
            remaining = 0;
        }
    }
}


void  nx_iperf_test_info_parse(ctrl_info *iperf_ctrlInfo_ptr)
{

    /* Check the sign and set the related parameters.  */
    switch ((iperf_ctrlInfo_ptr -> ctrl_sign) & NX_IPERF_CTRL_SIGN_MASK)
    {

    case TCP_RX_START:
    {
        if (iperf_ctrlInfo_ptr -> TestTime == 0)
        {
            iperf_ctrlInfo_ptr -> TestTime = 10 * NX_IP_PERIODIC_RATE;
        }
        tcp_rx_test_time = (UINT)((iperf_ctrlInfo_ptr -> TestTime) / NX_IP_PERIODIC_RATE);

        nx_iperf_tcp_rx_test(nx_iperf_stack_area, nx_iperf_stack_area_size);
        break;
    }

    case TCP_TX_START:
    {

        if (iperf_ctrlInfo_ptr -> TestTime == 0)
        {
            iperf_ctrlInfo_ptr -> TestTime = 10 * NX_IP_PERIODIC_RATE;
        }
        tcp_tx_test_time = (UINT)((iperf_ctrlInfo_ptr -> TestTime) / NX_IP_PERIODIC_RATE);

        /* Set the transmit ip address.  */
        if (iperf_ctrlInfo_ptr -> version == NX_IP_VERSION_V4)
        {
#ifndef NX_DISABLE_IPV4
            tcp_tx_ip_address.nxd_ip_version = NX_IP_VERSION_V4;
            tcp_tx_ip_address.nxd_ip_address.v4 = iperf_ctrlInfo_ptr -> ip;
#else
            break;
#endif
        }

#ifdef FEATURE_NX_IPV6
        else
        {
            tcp_tx_ip_address.nxd_ip_version = NX_IP_VERSION_V6;
            tcp_tx_ip_address.nxd_ip_address.v6[0] = iperf_ctrlInfo_ptr -> ipv6[0];
            tcp_tx_ip_address.nxd_ip_address.v6[1] = iperf_ctrlInfo_ptr -> ipv6[1];
            tcp_tx_ip_address.nxd_ip_address.v6[2] = iperf_ctrlInfo_ptr -> ipv6[2];
            tcp_tx_ip_address.nxd_ip_address.v6[3] = iperf_ctrlInfo_ptr -> ipv6[3];
        }
#endif
        tcp_tx_port = iperf_ctrlInfo_ptr -> port;

        nx_iperf_tcp_tx_test(nx_iperf_stack_area, nx_iperf_stack_area_size);
        break;
    }

    case UDP_RX_START:
    {
        if (iperf_ctrlInfo_ptr -> TestTime == 0)
        {
            iperf_ctrlInfo_ptr -> TestTime = 10 * NX_IP_PERIODIC_RATE;
        }
        udp_rx_test_time = (UINT)((iperf_ctrlInfo_ptr -> TestTime) / NX_IP_PERIODIC_RATE);

        nx_iperf_udp_rx_test(nx_iperf_stack_area, nx_iperf_stack_area_size);
        break;
    }

    case UDP_TX_START:
    {
        if (iperf_ctrlInfo_ptr -> TestTime == 0)
        {
            iperf_ctrlInfo_ptr -> TestTime =  10 * NX_IP_PERIODIC_RATE;
        }
        udp_tx_test_time = (UINT)((iperf_ctrlInfo_ptr -> TestTime) / NX_IP_PERIODIC_RATE);

        if ((iperf_ctrlInfo_ptr -> PacketSize == 0) || (iperf_ctrlInfo_ptr -> PacketSize > 1470))
        {
            iperf_ctrlInfo_ptr -> PacketSize = 1470;
        }
        udp_tx_packet_size = (UINT)(iperf_ctrlInfo_ptr -> PacketSize);
        udp_tx_port = iperf_ctrlInfo_ptr -> port;

        /* Set the transmit ip address.  */
        if (iperf_ctrlInfo_ptr -> version == NX_IP_VERSION_V4)
        {
#ifndef NX_DISABLE_IPV4
            udp_tx_ip_address.nxd_ip_version = NX_IP_VERSION_V4;
            udp_tx_ip_address.nxd_ip_address.v4 = iperf_ctrlInfo_ptr -> ip;
#else
            break;
#endif
        }

#ifdef FEATURE_NX_IPV6
        else
        {
            udp_tx_ip_address.nxd_ip_version = NX_IP_VERSION_V6;
            udp_tx_ip_address.nxd_ip_address.v6[0] = iperf_ctrlInfo_ptr -> ipv6[0];
            udp_tx_ip_address.nxd_ip_address.v6[1] = iperf_ctrlInfo_ptr -> ipv6[1];
            udp_tx_ip_address.nxd_ip_address.v6[2] = iperf_ctrlInfo_ptr -> ipv6[2];
            udp_tx_ip_address.nxd_ip_address.v6[3] = iperf_ctrlInfo_ptr -> ipv6[3];
            if (udp_tx_packet_size > 1450)
            {
                udp_tx_packet_size = 1450;
            }
        }
#endif

        if (iperf_ctrlInfo_ptr -> Rate == 0)
        {
            iperf_ctrlInfo_ptr -> Rate = 10;
        }

        nx_iperf_udp_tx_test(nx_iperf_stack_area, nx_iperf_stack_area_size);
        break;
    }

    default:
    {
        break;
    }
    }
}

static void nx_iperf_print_tcp_rx_results(NX_WEB_HTTP_SERVER *server_ptr, NXD_ADDRESS *peer_ip_address)
{

UINT       status;
NX_PACKET *resp_packet_ptr;
UINT       length = 0;
CHAR      *ip_addr_string;

    status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr,
                                &resp_packet_ptr,
                                NX_TCP_PACKET,
                                NX_WAIT_FOREVER);

    htmlwrite(resp_packet_ptr, toptdtag, sizeof(toptdtag) - 1);
    htmlwrite(resp_packet_ptr, tabletag, sizeof(tabletag) - 1);
    status +=  htmlwrite(resp_packet_ptr, rightspanline, sizeof(rightspanline) - 1);
    htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    htmlwrite(resp_packet_ptr, "TCP Receive Test Done:", sizeof("TCP Receive Test Done:") - 1);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status +=  htmlwrite(resp_packet_ptr, rightspanline, sizeof(rightspanline) - 1);
    status +=  htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Source IP Address: ", sizeof("Source IP Address: ") - 1);
    ip_addr_string = nx_iperf_get_ip_addr_string(peer_ip_address, &length);
    status += htmlwrite(resp_packet_ptr, ip_addr_string, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Test Time(milliseconds): ", sizeof("Test Time(milliseconds): ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.RunTime * 1000 / NX_IP_PERIODIC_RATE), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Number of Packets Received: ", sizeof("Number of Packets Received: ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.PacketsRxed), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Number of Bytes Received: ", sizeof("Number of Bytes Received: ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.BytesRxed), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Throughput(Mbps): ", sizeof("Throughput(Mbps): ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.ThroughPut), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    length = _nx_utility_uint_to_string(nx_iperf_ctrl_info.idleTime, 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, "Idle Time: ", sizeof("Idle Time: ") - 1);
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, "%", sizeof("%") - 1);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);
#endif

    htmlwrite(resp_packet_ptr, tableendtag, sizeof(tableendtag) - 1);
    htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);

    status += nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);

    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        nx_iperf_test_error_counter++;
    }

    /* Delete the receive thread. */
    nx_iperf_tcp_rx_cleanup();
}

static void nx_iperf_print_tcp_tx_results(NX_WEB_HTTP_SERVER *server_ptr, NXD_ADDRESS *peer_ip_address)
{
UINT       status;
NX_PACKET *resp_packet_ptr;
UINT       length = 0;
CHAR      *ip_addr_string;

    status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr,
                                &resp_packet_ptr,
                                NX_TCP_PACKET,
                                NX_WAIT_FOREVER);

    htmlwrite(resp_packet_ptr, toptdtag, sizeof(toptdtag) - 1);
    htmlwrite(resp_packet_ptr, tabletag, sizeof(tabletag) - 1);
    status +=  htmlwrite(resp_packet_ptr, rightspanline, sizeof(rightspanline) - 1);
    htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    htmlwrite(resp_packet_ptr, "TCP Transmit Test Done:", sizeof("TCP Transmit Test Done:") - 1);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, rightspanline, sizeof(rightspanline) - 1);
    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Destination IP Address: ", sizeof("Destination IP Address: ") - 1);
    ip_addr_string = nx_iperf_get_ip_addr_string(peer_ip_address, &length);
    status += htmlwrite(resp_packet_ptr, ip_addr_string, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Destination Port: ", sizeof("Destination Port: ") - 1);
    length = _nx_utility_uint_to_string(nx_iperf_ctrl_info.port, 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Test Time(milliseconds): ", sizeof("Test Time(milliseconds): ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.RunTime * 1000 / NX_IP_PERIODIC_RATE), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Number of Packets Transmitted: ", sizeof("Number of Packets Transmitted: ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.PacketsTxed), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Number of Bytes Transmitted: ", sizeof("Number of Bytes Transmitted: ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.BytesTxed), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Throughput(Mbps): ", sizeof("Throughput(Mbps): ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.ThroughPut), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    length = _nx_utility_uint_to_string(nx_iperf_ctrl_info.idleTime, 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, "Idle Time: ", sizeof("Idle Time: ") - 1);
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, "%", sizeof("%") - 1);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);
#endif

    status += htmlwrite(resp_packet_ptr, tableendtag, sizeof(tableendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);

    status += nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);

    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        nx_iperf_test_error_counter++;
    }

    /* Delete the receive thread. */
    nx_iperf_tcp_tx_cleanup();
}

static void nx_iperf_print_udp_rx_results(NX_WEB_HTTP_SERVER *server_ptr, NXD_ADDRESS *peer_ip_address)
{
UINT       status;
NX_PACKET *resp_packet_ptr;
UINT       length = 0;
CHAR      *ip_addr_string;

    status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr,
                                &resp_packet_ptr,
                                NX_TCP_PACKET,
                                NX_WAIT_FOREVER);

    /* now send the data back to the client.  */
    htmlwrite(resp_packet_ptr, toptdtag, sizeof(toptdtag) - 1);
    htmlwrite(resp_packet_ptr, tabletag, sizeof(tabletag) - 1);
    status +=  htmlwrite(resp_packet_ptr, rightspanline, sizeof(rightspanline) - 1);
    htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    htmlwrite(resp_packet_ptr, "UDP Receive Test Done:", sizeof("UDP Receive Test Done:") - 1);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, rightspanline, sizeof(rightspanline) - 1);
    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Source IP Address: ", sizeof("Source IP Address: ") - 1);
    ip_addr_string = nx_iperf_get_ip_addr_string(peer_ip_address, &length);
    status += htmlwrite(resp_packet_ptr, ip_addr_string, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Test Time(milliseconds): ", sizeof("Test Time(milliseconds): ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.RunTime * 1000 / NX_IP_PERIODIC_RATE), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Number of Packets Received: ", sizeof("Number of Packets Received: ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.PacketsRxed), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Number of Bytes Received: ", sizeof("Number of Bytes Received: ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.BytesRxed), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Throughput(Mbps): ", sizeof("Throughput(Mbps): ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.ThroughPut), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.idleTime), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, "Idle Time: ", sizeof("Idle Time: ") - 1);
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, "%", sizeof("%") - 1);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);
#endif

    status += htmlwrite(resp_packet_ptr, tableendtag, sizeof(tableendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);

    status += nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);

    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        nx_iperf_test_error_counter++;
    }

    nx_iperf_udp_rx_cleanup();
}

static void nx_iperf_print_udp_tx_results(NX_WEB_HTTP_SERVER *server_ptr, NXD_ADDRESS *peer_ip_address)
{
UINT       status;
NX_PACKET *resp_packet_ptr;
UINT       length = 0;
CHAR      *ip_addr_string;

    status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr,
                                &resp_packet_ptr,
                                NX_TCP_PACKET,
                                NX_WAIT_FOREVER);

    /* now send the data back to the client.  */
    htmlwrite(resp_packet_ptr, toptdtag, sizeof(toptdtag) - 1);
    htmlwrite(resp_packet_ptr, tabletag, sizeof(tabletag) - 1);
    status +=  htmlwrite(resp_packet_ptr, rightspanline, sizeof(rightspanline) - 1);
    htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    htmlwrite(resp_packet_ptr, "UDP Transmit Test Done:", sizeof("UDP Transmit Test Done:") - 1);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, rightspanline, sizeof(rightspanline) - 1);
    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Destination IP Address: ", sizeof("Destination IP Address: ") - 1);
    ip_addr_string = nx_iperf_get_ip_addr_string(peer_ip_address, &length);
    status += htmlwrite(resp_packet_ptr, ip_addr_string, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Destination Port: ", sizeof("Destination Port: ") - 1);
    length = _nx_utility_uint_to_string(nx_iperf_ctrl_info.port, 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Test Time(milliseconds): ", sizeof("Test Time(milliseconds): ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.RunTime * 1000 / NX_IP_PERIODIC_RATE), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Number of Packets Transmitted: ", sizeof("Number of Packets Transmitted: ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.PacketsTxed), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Number of Bytes Transmitted: ", sizeof("Number of Bytes Transmitted: ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.BytesTxed), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    status += htmlwrite(resp_packet_ptr, "Throughput(Mbps): ", sizeof("Throughput(Mbps): ") - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.ThroughPut), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    status += htmlwrite(resp_packet_ptr, trtag, sizeof(trtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdtag, sizeof(tdtag) - 1);
    status += htmlwrite(resp_packet_ptr, fonttag, sizeof(fonttag) - 1);
    length = _nx_utility_uint_to_string((UINT)(nx_iperf_ctrl_info.idleTime), 10, mytempstring, sizeof(mytempstring));
    status += htmlwrite(resp_packet_ptr, "Idle Time: ", sizeof("Idle Time: ") - 1);
    status += htmlwrite(resp_packet_ptr, mytempstring, length);
    status += htmlwrite(resp_packet_ptr, "%", sizeof("%") - 1);
    status += htmlwrite(resp_packet_ptr, fontendtag, sizeof(fontendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);
    status += htmlwrite(resp_packet_ptr, trendtag, sizeof(trendtag) - 1);
#endif

    status += htmlwrite(resp_packet_ptr, tableendtag, sizeof(tableendtag) - 1);
    status += htmlwrite(resp_packet_ptr, tdendtag, sizeof(tdendtag) - 1);

    status += nx_web_http_server_callback_packet_send(server_ptr, resp_packet_ptr);

    if (status)
    {
        nx_packet_release(resp_packet_ptr);
        nx_iperf_test_error_counter++;
    }

    nx_iperf_udp_tx_cleanup();
}

void    nx_iperf_thread_tcp_rx_entry(ULONG thread_input)
{
UINT        status;
NX_PACKET  *packet_ptr;
ULONG       actual_status;
ULONG       expire_time;
ctrl_info  *ctrlInfo_ptr;
NXD_ADDRESS ip_address;
ULONG       port;

    /* Set the pointer.  */
    ctrlInfo_ptr = (ctrl_info *)thread_input;

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    /* Update the time.  */
    thread_time = 0;
    isr_time = 0;
    idle_time = 0;
#endif

    /* Update the test result.  */
    ctrlInfo_ptr -> PacketsRxed = 0;
    ctrlInfo_ptr -> BytesRxed = 0;
    ctrlInfo_ptr -> ThroughPut = 0;
    ctrlInfo_ptr -> StartTime = 0;
    ctrlInfo_ptr -> RunTime = 0;
    ctrlInfo_ptr -> ErrorCode = 0;

    /* Ensure the IP instance has been initialized.  */
    status =  nx_ip_status_check(nx_iperf_test_ip, NX_IP_INITIALIZE_DONE, &actual_status, NX_IP_PERIODIC_RATE);

    /* Check status...  */
    if (status != NX_SUCCESS)
    {
        error_counter++;
        return;
    }

    /* Create a socket.  */
    status =  nx_tcp_socket_create(nx_iperf_test_ip, &tcp_server_socket, "TCP Server Socket",
                                   NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 32 * 1024,
                                   NX_NULL, nx_iperf_tcp_rx_disconnect_received);

    /* Check for error.  */
    if (status)
    {
        error_counter++;
        return;
    }

    /* Setup this thread to listen.  */
    status =  nx_tcp_server_socket_listen(nx_iperf_test_ip, NX_IPERF_TCP_RX_PORT, &tcp_server_socket, 5, nx_iperf_tcp_rx_connect_received);

    /* Check for error.  */
    if (status)
    {
        nx_tcp_socket_delete(&tcp_server_socket);
        error_counter++;
        return;
    }

    /* Increment thread tcp rx's counter.  */
    thread_tcp_rx_counter++;

    /* Accept a client socket connection.  */
    status =  nx_tcp_server_socket_accept(&tcp_server_socket, NX_WAIT_FOREVER);

    /* Check for error.  */
    if (status)
    {
        nx_tcp_server_socket_unlisten(nx_iperf_test_ip, NX_IPERF_TCP_RX_PORT);
        nx_tcp_socket_delete(&tcp_server_socket);
        error_counter++;
        return;
    }

    /*Get source ip address*/
    status = nxd_tcp_socket_peer_info_get(&tcp_server_socket, &ip_address, &port);
    if (status)
    {
        nx_tcp_server_socket_unaccept(&tcp_server_socket);
        nx_tcp_server_socket_unlisten(nx_iperf_test_ip, NX_IPERF_TCP_RX_PORT);
        nx_tcp_socket_delete(&tcp_server_socket);
        error_counter++;
        return;
    }

    ctrlInfo_ptr -> version = ip_address.nxd_ip_version;
    if (ctrlInfo_ptr -> version == NX_IP_VERSION_V4)
    {
#ifndef NX_DISABLE_IPV4
        ctrlInfo_ptr -> ip = ip_address.nxd_ip_address.v4;
#endif
    }
#ifdef FEATURE_NX_IPV6
    else if (ctrlInfo_ptr -> version == NX_IP_VERSION_V6)
    {
        memcpy(ctrlInfo_ptr -> ipv6, ip_address.nxd_ip_address.v6, sizeof(ULONG) * 4); /* Use case of memcpy is verified. */
    }
#endif

    /* Set the test start time.  */
    ctrlInfo_ptr -> StartTime = tx_time_get();
    expire_time = (ULONG)(ctrlInfo_ptr -> StartTime + (ctrlInfo_ptr -> TestTime) + 20);

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    _tx_execution_thread_total_time_reset();
    _tx_execution_isr_time_reset();
    _tx_execution_idle_time_reset();
#endif

    while (tx_time_get() < expire_time)
    {
        /* Receive a TCP message from the socket.  */
        status =  nx_tcp_socket_receive(&tcp_server_socket, &packet_ptr, NX_WAIT_FOREVER);

        /* Check for error.  */
        if (status)
        {
            error_counter++;
            break;
        }

        /* Update the counter.  */
        ctrlInfo_ptr -> PacketsRxed++;
        ctrlInfo_ptr -> BytesRxed += packet_ptr -> nx_packet_length;

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
    }

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    _tx_execution_thread_total_time_get(&thread_time);
    _tx_execution_isr_time_get(&isr_time);
    _tx_execution_idle_time_get(&idle_time);
#endif

    /* Calculate the test time and Throughput(Mbps).  */
    ctrlInfo_ptr -> RunTime = tx_time_get() - ctrlInfo_ptr -> StartTime;
    ctrlInfo_ptr -> ThroughPut = ctrlInfo_ptr -> BytesRxed / ctrlInfo_ptr -> RunTime * NX_IP_PERIODIC_RATE / 125000;

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    ctrlInfo_ptr -> idleTime = (ULONG)((unsigned long long)idle_time * 100 / ((unsigned long long)thread_time + (unsigned long long)isr_time + (unsigned long long)idle_time));
#endif

    /* Disconnect the server socket.  */
    status =  nx_tcp_socket_disconnect(&tcp_server_socket, 10);

    /* Check for error.  */
    if (status)
    {
        error_counter++;
    }

    /* Unaccept the server socket.  */
    status =  nx_tcp_server_socket_unaccept(&tcp_server_socket);
    status += nx_tcp_server_socket_unlisten(nx_iperf_test_ip, NX_IPERF_TCP_RX_PORT);

    /* Check for error.  */
    if (status)
    {
        error_counter++;
    }

    if (error_counter)
    {
        ctrlInfo_ptr -> ErrorCode = error_counter;
    }

    /* Delete the socket.  */
    nx_tcp_socket_delete(&tcp_server_socket);
}

void  nx_iperf_tcp_rx_connect_received(NX_TCP_SOCKET *socket_ptr, UINT port)
{
    /* Check for the proper socket and port.  */
    if ((socket_ptr != &tcp_server_socket) || (port != NX_IPERF_TCP_RX_PORT))
    {
        error_counter++;
    }
}

void  nx_iperf_tcp_rx_disconnect_received(NX_TCP_SOCKET *socket)
{
    /* Check for proper disconnected socket.  */
    if (socket != &tcp_server_socket)
    {
        error_counter++;
    }
}

void nx_iperf_tcp_rx_cleanup(void)
{
    nx_tcp_socket_disconnect(&tcp_server_socket, NX_NO_WAIT);
    nx_tcp_server_socket_unaccept(&tcp_server_socket);
    nx_tcp_server_socket_unlisten(nx_iperf_test_ip, NX_IPERF_TCP_RX_PORT);
    nx_tcp_socket_delete(&tcp_server_socket);

    tx_thread_terminate(&thread_tcp_rx_iperf);
    tx_thread_delete(&thread_tcp_rx_iperf);
}

void nx_iperf_tcp_rx_test(UCHAR *stack_space, ULONG stack_size)
{

UINT status;

    status = tx_thread_create(&thread_tcp_rx_iperf, "thread tcp rx",
                              nx_iperf_thread_tcp_rx_entry,
                              (ULONG)&nx_iperf_ctrl_info,
                              stack_space, stack_size, NX_IPERF_THREAD_PRIORITY, NX_IPERF_THREAD_PRIORITY,
                              TX_NO_TIME_SLICE, TX_AUTO_START);

    if (status)
    {
        nx_iperf_ctrl_info.ErrorCode = 1;
    }
    return;
}

void    nx_iperf_thread_tcp_tx_entry(ULONG thread_input)
{
UINT       status;
UINT       is_first = NX_TRUE;
NX_PACKET *my_packet = NX_NULL;
#ifndef NX_DISABLE_PACKET_CHAIN
NX_PACKET  *packet_ptr;
NX_PACKET  *last_packet;
ULONG       remaining_size;
#endif /* NX_DISABLE_PACKET_CHAIN */
ULONG       expire_time;
ctrl_info  *ctrlInfo_ptr;
ULONG       packet_size;
NXD_ADDRESS server_ip;

    /* Set the pointer.  */
    ctrlInfo_ptr = (ctrl_info *)thread_input;

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    thread_time = 0;
    isr_time = 0;
    idle_time = 0;
#endif

    ctrlInfo_ptr -> PacketsTxed = 0;
    ctrlInfo_ptr -> BytesTxed = 0;
    ctrlInfo_ptr -> ThroughPut = 0;
    ctrlInfo_ptr -> StartTime = 0;
    ctrlInfo_ptr -> RunTime = 0;
    ctrlInfo_ptr -> ErrorCode = 0;

    server_ip.nxd_ip_version = ctrlInfo_ptr -> version;

#ifdef FEATURE_NX_IPV6
    if (ctrlInfo_ptr -> version == NX_IP_VERSION_V6)
    {
        server_ip.nxd_ip_address.v6[0] = ctrlInfo_ptr -> ipv6[0];
        server_ip.nxd_ip_address.v6[1] = ctrlInfo_ptr -> ipv6[1];
        server_ip.nxd_ip_address.v6[2] = ctrlInfo_ptr -> ipv6[2];
        server_ip.nxd_ip_address.v6[3] = ctrlInfo_ptr -> ipv6[3];
    }
    else if (ctrlInfo_ptr -> version == NX_IP_VERSION_V4)
#endif
    {
#ifndef NX_DISABLE_IPV4
        server_ip.nxd_ip_address.v4 = ctrlInfo_ptr -> ip;
#endif
    }

    /* TCP Transmit Test Starts in 2 seconds.  */
    tx_thread_sleep(200);

    /* Create the socket.  */
    status =  nx_tcp_socket_create(nx_iperf_test_ip, &tcp_client_socket, "TCP Client Socket",
                                   NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 32 * 1024,
                                   NX_NULL, NX_NULL);

    /* Check for error.  */
    if (status)
    {
        error_counter++;
        return;
    }

    /* Bind the socket.  */
    status =  nx_tcp_client_socket_bind(&tcp_client_socket, NX_ANY_PORT, NX_WAIT_FOREVER);

    /* Check for error.  */
    if (status)
    {
        nx_tcp_socket_delete(&tcp_client_socket);
        error_counter++;
        return;
    }

    /* Attempt to connect the socket.  */
    status =  nxd_tcp_client_socket_connect(&tcp_client_socket, &server_ip, ctrlInfo_ptr -> port, NX_WAIT_FOREVER);

    /* Check for error.  */
    if (status)
    {
        nx_tcp_client_socket_unbind(&tcp_client_socket);
        nx_tcp_socket_delete(&tcp_client_socket);
        error_counter++;
        return;
    }

    /* Set the test start time.  */
    ctrlInfo_ptr -> StartTime = tx_time_get();
    expire_time = (ULONG)(ctrlInfo_ptr -> StartTime + (ctrlInfo_ptr -> TestTime));

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    _tx_execution_thread_total_time_reset();
    _tx_execution_isr_time_reset();
    _tx_execution_idle_time_reset();
#endif

    /* Set the packet size.  */
    status = nx_tcp_socket_mss_get(&tcp_client_socket, &packet_size);

    /* Check for error.  */
    if (status)
    {
        nx_tcp_socket_disconnect(&tcp_client_socket, NX_NO_WAIT);
        nx_tcp_client_socket_unbind(&tcp_client_socket);
        nx_tcp_socket_delete(&tcp_client_socket);
        error_counter++;
        return;
    }

    /* Loop to transmit the packet.  */
    while (tx_time_get() < expire_time)
    {

        /* Allocate a packet.  */
        status =  nx_packet_allocate(nx_iperf_test_pool, &my_packet, NX_TCP_PACKET, NX_WAIT_FOREVER);

        /* Check status.  */
        if (status != NX_SUCCESS)
        {
            break;
        }

        /* Write ABCs into the packet payload!  */
        /* Adjust the write pointer.  */
        if (my_packet -> nx_packet_prepend_ptr + packet_size <= my_packet -> nx_packet_data_end)
        {
            my_packet -> nx_packet_append_ptr =  my_packet -> nx_packet_prepend_ptr + packet_size;
#ifndef NX_DISABLE_PACKET_CHAIN
            remaining_size = 0;
#endif /* NX_DISABLE_PACKET_CHAIN */
        }
        else
        {
#ifdef NX_DISABLE_PACKET_CHAIN
            packet_size = (ULONG)(my_packet -> nx_packet_data_end - my_packet -> nx_packet_prepend_ptr);
            my_packet -> nx_packet_append_ptr =  my_packet -> nx_packet_prepend_ptr + packet_size;
#else
            my_packet -> nx_packet_append_ptr = my_packet -> nx_packet_data_end;
            remaining_size = packet_size - (ULONG)(my_packet -> nx_packet_append_ptr - my_packet -> nx_packet_prepend_ptr);
            last_packet = my_packet;
#endif /* NX_DISABLE_PACKET_CHAIN */
        }
        my_packet -> nx_packet_length =  packet_size;

#ifndef NX_DISABLE_PACKET_CHAIN
        while (remaining_size)
        {

            /* Allocate a packet.  */
            status =  nx_packet_allocate(nx_iperf_test_pool, &packet_ptr, NX_TCP_PACKET, NX_WAIT_FOREVER);

            /* Check status.  */
            if (status != NX_SUCCESS)
            {
                break;
            }

            last_packet -> nx_packet_next = packet_ptr;
            last_packet = packet_ptr;
            if (remaining_size < (ULONG)(packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_prepend_ptr))
            {
                packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_prepend_ptr + remaining_size;
            }
            else
            {
                packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_data_end;
            }
            remaining_size = remaining_size - (ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr);
        }
#endif /* NX_DISABLE_PACKET_CHAIN */

        if (is_first)
        {
            memset(my_packet -> nx_packet_prepend_ptr, 0, (UINT)(my_packet -> nx_packet_data_end - my_packet -> nx_packet_prepend_ptr));
            is_first = NX_FALSE;
        }

        /* Send the packet out!  */
        status =  nx_tcp_socket_send(&tcp_client_socket, my_packet, NX_WAIT_FOREVER);

        /* Determine if the status is valid.  */
        if (status)
        {
            error_counter++;
            nx_packet_release(my_packet);
            break;
        }
        else
        {

            /* Update the counter.  */
            ctrlInfo_ptr -> PacketsTxed++;
            ctrlInfo_ptr -> BytesTxed += packet_size;
        }
    }

    /* Calculate the test time and Throughput(Mbps).  */
    ctrlInfo_ptr -> RunTime = tx_time_get() - ctrlInfo_ptr -> StartTime;
    ctrlInfo_ptr -> ThroughPut = ctrlInfo_ptr -> BytesTxed / ctrlInfo_ptr -> RunTime * NX_IP_PERIODIC_RATE / 125000;

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    _tx_execution_thread_total_time_get(&thread_time);
    _tx_execution_isr_time_get(&isr_time);
    _tx_execution_idle_time_get(&idle_time);
#endif

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    ctrlInfo_ptr -> idleTime = (ULONG)((unsigned long long)idle_time * 100 / ((unsigned long long)thread_time + (unsigned long long)isr_time + (unsigned long long)idle_time));
#endif

    /* Disconnect this socket.  */
    status =  nx_tcp_socket_disconnect(&tcp_client_socket, NX_NO_WAIT);

    /* Determine if the status is valid.  */
    if (status)
    {
        error_counter++;
    }

    /* Unbind the socket.  */
    status =  nx_tcp_client_socket_unbind(&tcp_client_socket);

    /* Check for error.  */
    if (status)
    {
        error_counter++;
    }

    if (error_counter)
    {
        ctrlInfo_ptr -> ErrorCode = error_counter;
    }

    /* Delete the socket.  */
    nx_tcp_socket_delete(&tcp_client_socket);
}

void nx_iperf_tcp_tx_cleanup(void)
{
    nx_tcp_socket_disconnect(&tcp_client_socket, NX_NO_WAIT);
    nx_tcp_client_socket_unbind(&tcp_client_socket);
    nx_tcp_socket_delete(&tcp_client_socket);

    tx_thread_terminate(&thread_tcp_tx_iperf);
    tx_thread_delete(&thread_tcp_tx_iperf);
}

void  nx_iperf_tcp_tx_test(UCHAR *stack_space, ULONG stack_size)
{
UINT status;

    status = tx_thread_create(&thread_tcp_tx_iperf, "thread tcp tx",
                              nx_iperf_thread_tcp_tx_entry,
                              (ULONG)&nx_iperf_ctrl_info,
                              stack_space, stack_size, NX_WEB_HTTP_SERVER_PRIORITY + 1, NX_WEB_HTTP_SERVER_PRIORITY + 1,
                              TX_NO_TIME_SLICE, TX_AUTO_START);

    if (status)
    {
        nx_iperf_ctrl_info.ErrorCode = 1;
    }
    return;
}


void   nx_iperf_thread_udp_rx_entry(ULONG thread_input)
{
UINT        status;
ULONG       expire_time;
NX_PACKET  *my_packet;
ctrl_info  *ctrlInfo_ptr;
int         packetID = 0;
UINT        sender_port;
ULONG       tmp;
NXD_ADDRESS source_ip_address;

    /* Set the pointer.  */
    ctrlInfo_ptr = (ctrl_info *)thread_input;

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    thread_time = 0;
    isr_time = 0;
    idle_time = 0;
#endif

    /* Update the test result.  */
    ctrlInfo_ptr -> PacketsRxed = 0;
    ctrlInfo_ptr -> BytesRxed = 0;
    ctrlInfo_ptr -> ThroughPut = 0;
    ctrlInfo_ptr -> StartTime = 0;
    ctrlInfo_ptr -> RunTime = 0;
    ctrlInfo_ptr -> ErrorCode = 0;

    /* Create a UDP socket.  */
    status = nx_udp_socket_create(nx_iperf_test_ip, &udp_server_socket, "UDP Server Socket", NX_IP_NORMAL, NX_FRAGMENT_OKAY, 0x80, 5);

    /* Check status.  */
    if (status)
    {
        error_counter++;
        return;
    }

    /* Bind the UDP socket to the IP port.  */
    status = nx_udp_socket_bind(&udp_server_socket, NX_IPERF_UDP_RX_PORT, TX_WAIT_FOREVER);

    /* Check status.  */
    if (status)
    {
        nx_udp_socket_delete(&udp_server_socket);
        error_counter++;
        return;
    }

    /* Disable checksum for UDP. */
    nx_udp_socket_checksum_disable(&udp_server_socket);

    /* Receive a UDP packet.  */
    status = nx_udp_socket_receive(&udp_server_socket, &my_packet, TX_WAIT_FOREVER);

    /* Check status.  */
    if (status)
    {
        nx_udp_socket_unbind(&udp_server_socket);
        nx_udp_socket_delete(&udp_server_socket);
        error_counter++;
        return;
    }

    /* Get source ip address*/
    nxd_udp_source_extract(my_packet, &source_ip_address, &sender_port);

    /* Set the IP address Version.  */
    ctrlInfo_ptr -> version = source_ip_address.nxd_ip_version;

#ifndef NX_DISABLE_IPV4
    if (ctrlInfo_ptr -> version == NX_IP_VERSION_V4)
    {
        ctrlInfo_ptr -> ip = source_ip_address.nxd_ip_address.v4;
    }
#endif
#ifdef FEATURE_NX_IPV6
    if (ctrlInfo_ptr -> version == NX_IP_VERSION_V6)
    {
        ctrlInfo_ptr -> ipv6[0] = source_ip_address.nxd_ip_address.v6[0];
        ctrlInfo_ptr -> ipv6[1] = source_ip_address.nxd_ip_address.v6[1];
        ctrlInfo_ptr -> ipv6[2] = source_ip_address.nxd_ip_address.v6[2];
        ctrlInfo_ptr -> ipv6[3] = source_ip_address.nxd_ip_address.v6[3];
    }
#endif

    /* Release the packet.  */
    nx_packet_release(my_packet);

    /* Set the test start time.  */
    ctrlInfo_ptr -> StartTime = tx_time_get();
    expire_time = (ULONG)(ctrlInfo_ptr -> StartTime + (ctrlInfo_ptr -> TestTime) + 5);   /* Wait 5 more ticks to synchronize. */

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    _tx_execution_thread_total_time_reset();
    _tx_execution_isr_time_reset();
    _tx_execution_idle_time_reset();
#endif

    while (tx_time_get() < expire_time)
    {

        /* Receive a UDP packet.  */
        status =  nx_udp_socket_receive(&udp_server_socket, &my_packet, TX_WAIT_FOREVER);

        /* Check status.  */
        if (status != NX_SUCCESS)
        {
            error_counter++;
            break;
        }

        /* Update the counter.  */
        ctrlInfo_ptr -> PacketsRxed++;
        ctrlInfo_ptr -> BytesRxed += my_packet -> nx_packet_length;

        /* Detect the end of the test signal. */
        packetID = *(int *)(my_packet -> nx_packet_prepend_ptr);

        tmp = (ULONG)packetID;
        NX_CHANGE_ULONG_ENDIAN(tmp);
        packetID = (int)tmp;


        /* Check the packet ID.  */
        if (packetID < 0)
        {

            /* Test has finished. */
#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
            _tx_execution_thread_total_time_get(&thread_time);
            _tx_execution_isr_time_get(&isr_time);
            _tx_execution_idle_time_get(&idle_time);
#endif

            /* Calculate the test time and Throughput.  */
            ctrlInfo_ptr -> RunTime = tx_time_get() - ctrlInfo_ptr -> StartTime;
            ctrlInfo_ptr -> ThroughPut = ctrlInfo_ptr -> BytesRxed / ctrlInfo_ptr -> RunTime * NX_IP_PERIODIC_RATE / 125000;

            /* received end of the test signal */

            /* Send the UDP packet.  */
            status = nxd_udp_socket_send(&udp_server_socket, my_packet, &source_ip_address, sender_port);

            /* Check the status.  */
            if (status)
            {

                /* Release the packet.  */
                nx_packet_release(my_packet);
            }
            else
            {

                /* Loop to receive the end of the test signal.  */
                while (1)
                {

                    /* Receive a UDP packet.  */
                    status =  nx_udp_socket_receive(&udp_server_socket, &my_packet, 20);

                    /* Check the status.  */
                    if (status)
                    {
                        break;
                    }

                    /* Send the UDP packet.  */
                    status = nxd_udp_socket_send(&udp_server_socket, my_packet, &source_ip_address, sender_port);

                    /* Check the status.  */
                    if (status)
                    {

                        /* Release the packet.  */
                        nx_packet_release(my_packet);
                    }
                }
            }
            break;
        }
        else
        {

            /* Release the packet.  */
            nx_packet_release(my_packet);
        }
    }

    if (packetID >= 0)
    {

        /* Test is not synchronized. */
#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
        _tx_execution_thread_total_time_get(&thread_time);
        _tx_execution_isr_time_get(&isr_time);
        _tx_execution_idle_time_get(&idle_time);
#endif

        /* Calculate the test time and Throughput.  */
        ctrlInfo_ptr -> RunTime = tx_time_get() - ctrlInfo_ptr -> StartTime;
        ctrlInfo_ptr -> ThroughPut = ctrlInfo_ptr -> BytesRxed / ctrlInfo_ptr -> RunTime * NX_IP_PERIODIC_RATE / 125000;
    }

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    ctrlInfo_ptr -> idleTime = (ULONG)((unsigned long long)idle_time * 100 / ((unsigned long long)thread_time + (unsigned long long)isr_time + (unsigned long long)idle_time));
#endif

    /* Unbind and Delete the socket.  */
    nx_udp_socket_unbind(&udp_server_socket);
    nx_udp_socket_delete(&udp_server_socket);

    /* Check error counter.  */
    if (error_counter)
    {
        ctrlInfo_ptr -> ErrorCode = error_counter;
    }
}

void nx_iperf_udp_rx_cleanup(void)
{
    nx_udp_socket_unbind(&udp_server_socket);
    nx_udp_socket_delete(&udp_server_socket);

    tx_thread_terminate(&thread_udp_rx_iperf);
    tx_thread_delete(&thread_udp_rx_iperf);
}

void  nx_iperf_udp_rx_test(UCHAR *stack_space, ULONG stack_size)
{
UINT status;

    status = tx_thread_create(&thread_udp_rx_iperf, "thread udp rx",
                              nx_iperf_thread_udp_rx_entry,
                              (ULONG)&nx_iperf_ctrl_info,
                              stack_space, stack_size, NX_WEB_HTTP_SERVER_PRIORITY + 1, NX_WEB_HTTP_SERVER_PRIORITY + 1,
                              TX_NO_TIME_SLICE, TX_AUTO_START);

    if (status)
    {
        nx_iperf_ctrl_info.ErrorCode = 1;
    }
    return;
}

static void nx_iperf_send_udp_packet(int udp_id, ctrl_info *ctrlInfo_ptr)
{

UINT         status;
NX_PACKET   *my_packet = NX_NULL;
udp_payload *payload_ptr;
ULONG        tmp;

NXD_ADDRESS  server_ip;

    server_ip.nxd_ip_version = ctrlInfo_ptr -> version;

#ifdef FEATURE_NX_IPV6
    if (ctrlInfo_ptr -> version == NX_IP_VERSION_V6)
    {
        server_ip.nxd_ip_address.v6[0] = ctrlInfo_ptr -> ipv6[0];
        server_ip.nxd_ip_address.v6[1] = ctrlInfo_ptr -> ipv6[1];
        server_ip.nxd_ip_address.v6[2] = ctrlInfo_ptr -> ipv6[2];
        server_ip.nxd_ip_address.v6[3] = ctrlInfo_ptr -> ipv6[3];
    }
    else if (ctrlInfo_ptr -> version == NX_IP_VERSION_V4)
#endif
    {
#ifndef NX_DISABLE_IPV4
        server_ip.nxd_ip_address.v4 = ctrlInfo_ptr -> ip;
#endif
    }

    /* Send the end of test indicator. */
    nx_packet_allocate(nx_iperf_test_pool, &my_packet, NX_UDP_PACKET, TX_WAIT_FOREVER);
    my_packet -> nx_packet_append_ptr =  my_packet -> nx_packet_prepend_ptr + ctrlInfo_ptr -> PacketSize;

    payload_ptr = (udp_payload *)my_packet -> nx_packet_prepend_ptr;
    payload_ptr -> udp_id = udp_id;
    payload_ptr -> tv_sec = _tx_timer_system_clock / NX_IP_PERIODIC_RATE;
    payload_ptr -> tv_usec = _tx_timer_system_clock / NX_IP_PERIODIC_RATE * 1000000;

    tmp = (ULONG)payload_ptr -> udp_id;
    NX_CHANGE_ULONG_ENDIAN(tmp);
    payload_ptr -> udp_id = (int)tmp;

    NX_CHANGE_ULONG_ENDIAN(payload_ptr -> tv_sec);
    NX_CHANGE_ULONG_ENDIAN(payload_ptr -> tv_usec);

    /* Adjust the write pointer.  */
    my_packet -> nx_packet_length = (UINT)(ctrlInfo_ptr -> PacketSize);

    /* Send the UDP packet.  */
    status = nxd_udp_socket_send(&udp_client_socket, my_packet, &server_ip, ctrlInfo_ptr -> port);

    /* Check the status.  */
    if (status)
    {

        /* Release the packet.  */
        nx_packet_release(my_packet);
        return;
    }
}

void  nx_iperf_thread_udp_tx_entry(ULONG thread_input)
{
UINT       status;
ULONG      expire_time;
ctrl_info *ctrlInfo_ptr;
NX_PACKET *my_packet;
int        i;
long       udp_id;

    /* Initialize the value.  */
    udp_id = 0;
    ctrlInfo_ptr = (ctrl_info *)thread_input;

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    thread_time = 0;
    isr_time = 0;
    idle_time = 0;
#endif

    ctrlInfo_ptr -> PacketsTxed = 0;
    ctrlInfo_ptr -> BytesTxed = 0;
    ctrlInfo_ptr -> ThroughPut = 0;
    ctrlInfo_ptr -> StartTime = 0;
    ctrlInfo_ptr -> RunTime = 0;
    ctrlInfo_ptr -> ErrorCode = 0;

    /* UDP Transmit Test Starts in 2 seconds.  */
    tx_thread_sleep(200);

    /* Create a UDP socket.  */
    status = nx_udp_socket_create(nx_iperf_test_ip, &udp_client_socket, "UDP Client Socket", NX_IP_NORMAL, NX_FRAGMENT_OKAY, 0x80, 5);

    /* Check status.  */
    if (status)
    {
        error_counter++;
        return;
    }

    /* Bind the UDP socket to the IP port.  */
    status =  nx_udp_socket_bind(&udp_client_socket, NX_ANY_PORT, TX_WAIT_FOREVER);

    /* Check status.  */
    if (status)
    {
        nx_udp_socket_delete(&udp_client_socket);
        error_counter++;
        return;
    }

    /* Do not calculate checksum for UDP. */
    nx_udp_socket_checksum_disable(&udp_client_socket);

    /* Set the test start time.  */
    ctrlInfo_ptr -> StartTime = tx_time_get();
    expire_time = (ULONG)(ctrlInfo_ptr -> StartTime + (ctrlInfo_ptr -> TestTime));

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    _tx_execution_thread_total_time_reset();
    _tx_execution_isr_time_reset();
    _tx_execution_idle_time_reset();
#endif

    while (tx_time_get() < expire_time)
    {

        /* Write ABCs into the packet payload!  */
        nx_iperf_send_udp_packet(udp_id, ctrlInfo_ptr);

        /* Update the ID.  */
        udp_id = (udp_id + 1) & 0x7FFFFFFF;

        /* Update the counter.  */
        ctrlInfo_ptr -> PacketsTxed++;
        ctrlInfo_ptr -> BytesTxed += ctrlInfo_ptr -> PacketSize;
    }

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    _tx_execution_thread_total_time_get(&thread_time);
    _tx_execution_isr_time_get(&isr_time);
    _tx_execution_idle_time_get(&idle_time);
#endif

    /* Calculate the test time and Throughput.  */
    ctrlInfo_ptr -> RunTime = tx_time_get() - ctrlInfo_ptr -> StartTime;
    ctrlInfo_ptr -> ThroughPut = ctrlInfo_ptr -> BytesTxed / ctrlInfo_ptr -> RunTime * NX_IP_PERIODIC_RATE / 125000;

#if defined(TX_ENABLE_EXECUTION_CHANGE_NOTIFY) || defined(TX_EXECUTION_PROFILE_ENABLE)
    ctrlInfo_ptr -> idleTime = (ULONG)((unsigned long long)idle_time * 100 / ((unsigned long long)thread_time + (unsigned long long)isr_time + (unsigned long long)idle_time));
#endif

    if (error_counter)
    {
        ctrlInfo_ptr -> ErrorCode = error_counter;
    }

    ctrlInfo_ptr -> PacketSize = 100;
    for (i = 0; i < 10; i++)
    {
        /* Send the end of the test signal*/
        nx_iperf_send_udp_packet((0 - udp_id), ctrlInfo_ptr);

        /* Receive the packet.  s*/
        if (nx_udp_socket_receive(&udp_client_socket, &my_packet, 10) == NX_SUCCESS)
        {
            nx_packet_release(my_packet);
            break;
        }
    }

    /* Unbind and Delete the socket.  */
    nx_udp_socket_unbind(&udp_client_socket);
    nx_udp_socket_delete(&udp_client_socket);
}

void nx_iperf_udp_tx_cleanup(void)
{
    nx_udp_socket_unbind(&udp_client_socket);
    nx_udp_socket_delete(&udp_client_socket);
    tx_thread_terminate(&thread_udp_tx_iperf);
    tx_thread_delete(&thread_udp_tx_iperf);
}

void  nx_iperf_udp_tx_test(UCHAR *stack_space, ULONG stack_size)
{
UINT status;

    status = tx_thread_create(&thread_udp_tx_iperf, "thread udp tx",
                              nx_iperf_thread_udp_tx_entry,
                              (ULONG)&nx_iperf_ctrl_info,
                              stack_space, stack_size, NX_WEB_HTTP_SERVER_PRIORITY + 1, NX_WEB_HTTP_SERVER_PRIORITY + 1,
                              TX_NO_TIME_SLICE, TX_AUTO_START);

    if (status)
    {
        nx_iperf_ctrl_info.ErrorCode = 1;
    }
    return;
}

