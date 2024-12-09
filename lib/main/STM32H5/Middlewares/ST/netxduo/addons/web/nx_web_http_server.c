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
/** NetX Web Component                                                    */
/**                                                                       */
/**   Hypertext Transfer Protocol (HTTP)                                  */
/**   Hypertext Transfer Protocol Secure (HTTPS using TLS)                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_WEB_HTTP_SERVER_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif

/* Include necessary system files.  */

#include    "nx_api.h"
#include    "nx_ip.h"
#include    "nx_web_http_server.h"

#include    "stdio.h"
#include    "string.h"
#ifdef NX_WEB_HTTP_DIGEST_ENABLE    
#include   "nx_md5.h"
#endif

#if defined(NX_WEB_HTTP_SERVER_OMIT_CONTENT_LENGTH) && !defined(NX_WEB_HTTP_KEEPALIVE_DISABLE)
#error "Content length needed when keepalive is enabled"
#endif /* NX_WEB_HTTP_SERVER_OMIT_CONTENT_LENGTH */

#ifdef  NX_WEB_HTTP_DIGEST_ENABLE

/* Use for mapping random nonces to printable characters.  */
static CHAR _nx_web_http_server_base64_array[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
#endif

/* Define date strings. */
const CHAR _nx_web_http_server_weekday[][3] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const CHAR _nx_web_http_server_month[][3] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/* Define basic MIME maps. */
static NX_WEB_HTTP_SERVER_MIME_MAP _nx_web_http_server_mime_maps[] =
{
    {"html",     "text/html"},
    {"htm",      "text/html"},
    {"txt",      "text/plain"},
    {"gif",      "image/gif"},
    {"jpg",      "image/jpeg"},
    {"ico",      "image/x-icon"},
};


/* Create two arrays to hold encoded and decoded username/password data. */
static CHAR  authorization_request[NX_WEB_HTTP_MAX_STRING + 1];
static CHAR  authorization_decoded[NX_WEB_HTTP_MAX_NAME + NX_WEB_HTTP_MAX_PASSWORD + 2];

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_content_get                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP content get call.       */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    packet_ptr                            Pointer to HTTP request packet*/
/*    byte_offset                           Byte offset into content      */
/*    destination_ptr                       Pointer to content destination*/
/*    destination_size                      Maximum size of destination   */
/*    actual_size                           Actual amount of content      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_content_get       Actual server content get call*/
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
UINT  _nxe_web_http_server_content_get(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset, CHAR *destination_ptr, UINT destination_size, UINT *actual_size)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) ||
        (packet_ptr == NX_NULL) || (destination_ptr == NX_NULL) || (actual_size == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual server content get function.  */
    status =  _nx_web_http_server_content_get(server_ptr, packet_ptr, byte_offset, destination_ptr, destination_size, actual_size);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_content_get                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the user's specified portion of the HTTP      */
/*    content. Content is typically included in POST and PUT requests     */
/*    from the client. This routine is designed to be called from the     */
/*    application's request notify callback.                              */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    packet_ptr                            Pointer to HTTP request packet*/
/*    byte_offset                           Byte offset into content      */
/*    destination_ptr                       Pointer to content destination*/
/*    destination_size                      Maximum size of destination   */
/*    actual_size                           Actual amount of content      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_content_get_extended                            */
/*                                          Get content data              */
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
UINT  _nx_web_http_server_content_get(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset, CHAR *destination_ptr, UINT destination_size, UINT *actual_size)
{
UINT status;

    /* Get content data.  */
    status = _nx_web_http_server_content_get_extended(server_ptr, packet_ptr, byte_offset, destination_ptr, destination_size, actual_size);

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_packet_content_find            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP packet content find     */
/*    call.                                                               */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    packet_ptr                            Pointer to pointer to the     */
/*                                            packet that contains the    */
/*                                            HTTP header                 */
/*    content_length                        Pointer to content length     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_packet_content_find                             */
/*                                          Actual server packet content  */
/*                                            find call                   */
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
UINT  _nxe_web_http_server_packet_content_find(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, ULONG *content_length)
{


    /* Check for invalid packet pointer.  */
    if((server_ptr == NX_NULL) || (server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) ||
       (packet_ptr == NX_NULL) || (*packet_ptr == NX_NULL) || (content_length == NX_NULL))
        return(NX_PTR_ERROR);

    /* Call actual server content length get function.  */
    return(_nx_web_http_server_packet_content_find(server_ptr, packet_ptr, content_length));

}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_packet_content_find             PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds the content length specified in the HTTP        */
/*    header, and move the nx_packet_prepend_ptr to the beginning of      */
/*    the content.  If the beginning of the content is not in the packet, */
/*    this function attempts to read the HTTP server socket for the       */
/*    next packet.                                                        */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    packet_ptr                            Pointer to pointer to the     */
/*                                            packet that contains the    */
/*                                            HTTP header                 */
/*    content_length                        Pointer to content length     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_content_length_get                              */
/*                                          Get content length            */
/*    _nx_web_http_server_packet_get        Receive another packet        */
/*    _nx_web_http_server_calculate_content_offset                        */
/*                                          Pickup content offset         */
/*    nx_packet_allocate                    Allocate a new packet         */
/*    nx_packet_release                     Release the packet            */
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
/*  06-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_web_http_server_packet_content_find(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, ULONG *content_length)
{

ULONG       get_offset;
UINT        status;
ULONG       length = 0;
UINT        packet_chunked;
NX_PACKET  *header_packet_ptr;
NX_PACKET  *new_packet_ptr;
ULONG       temp_offset;

    /* Check if the packet is chunked.  */
    packet_chunked = _nx_web_http_server_chunked_check(*packet_ptr);

    /* If the packet isn't chunked.  */
    if (!packet_chunked)
    {

        /* Get the content length header information. */
        status = _nx_web_http_server_content_length_get(*packet_ptr, &length);

        if (status != NX_SUCCESS)
        {
            return(status);
        }
    }

    /* Get the content offset.  */
    get_offset = (ULONG)_nx_web_http_server_calculate_content_offset(*packet_ptr);

    /* Check packet length.  */
    if((*packet_ptr) -> nx_packet_length == get_offset)
    {

        /* If the packet doesn't contain any content, need to receive a new packet.  */

        /* If the received request packet is chunked.  */
        if (server_ptr -> nx_web_http_server_request_chunked)
        {

            /* Initialize the request packet pointer and remaining size for processing chunked packet.  */
            server_ptr -> nx_web_http_server_request_packet = NX_NULL;
            server_ptr -> nx_web_http_server_chunked_request_remaining_size = 0;
        }

        /* Make the receive call to obtain the next packet. */
        status = _nx_web_http_server_packet_get(server_ptr, packet_ptr);

        if(status != NX_SUCCESS)
        {
            return(NX_WEB_HTTP_TIMEOUT);
        }

    }
    else
    {

        /* Separate the HTTP header and content. Copy the content to a new packet.  */

        /* Remember the packet pointer and content offset.  */
        header_packet_ptr = *packet_ptr;
        temp_offset = get_offset;

#ifndef NX_DISABLE_PACKET_CHAIN

        /* If the packet is chained, loop to find the end of the HTTP header.  */
        while ((*packet_ptr) && (temp_offset > (ULONG)((*packet_ptr) -> nx_packet_append_ptr - (*packet_ptr) -> nx_packet_prepend_ptr)))
        {
            temp_offset -= (ULONG)((*packet_ptr) -> nx_packet_append_ptr - (*packet_ptr) -> nx_packet_prepend_ptr);
            (*packet_ptr) = (*packet_ptr) -> nx_packet_next;
        }

        /* Chack if packet is valid.  */
        if ((*packet_ptr) == NX_NULL)
        {
            return(NX_WEB_HTTP_BAD_PACKET_LENGTH);
        }

        /* If this packet contain no content, set next packet as the first packet of the content.  */
        if (temp_offset == (ULONG)((*packet_ptr) -> nx_packet_append_ptr - (*packet_ptr) -> nx_packet_prepend_ptr))
        {
            new_packet_ptr = (*packet_ptr) -> nx_packet_next;
        }
        else
        {
#endif

            /* Allocate a new packet to store content.  */
            status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr, 
                                        &new_packet_ptr, 0, NX_WAIT_FOREVER);
            if(status)
            {
                return(status);
            }

            /* Copy the content part to the new packet.  */
            nx_packet_data_append(new_packet_ptr,
                                  (*packet_ptr) -> nx_packet_prepend_ptr + temp_offset,
                                  (ULONG)((*packet_ptr) -> nx_packet_append_ptr - (*packet_ptr) -> nx_packet_prepend_ptr) - temp_offset,
                                  server_ptr -> nx_web_http_server_packet_pool_ptr,
                                  NX_WAIT_FOREVER);

            /* Update the append pointer of the header part.  */
            (*packet_ptr) -> nx_packet_append_ptr = (*packet_ptr) -> nx_packet_prepend_ptr + temp_offset;

#ifndef NX_DISABLE_PACKET_CHAIN

            /* Set the next packet of the content packet.  */
            new_packet_ptr -> nx_packet_next = (*packet_ptr) -> nx_packet_next;
        }

        /* Set the last packet of the content and header packet.  */
        new_packet_ptr -> nx_packet_last = header_packet_ptr -> nx_packet_last;
        header_packet_ptr -> nx_packet_last = (*packet_ptr);
        (*packet_ptr) -> nx_packet_next = NX_NULL;
#endif

        /* Update the length of the header and content packet.  */
        new_packet_ptr -> nx_packet_length = header_packet_ptr -> nx_packet_length - get_offset;
        header_packet_ptr -> nx_packet_length = get_offset;

        /* Return the content packet.  */
        if (server_ptr -> nx_web_http_server_request_chunked)
        {

            /* If the packet is chunked, set for processing chunked packet.  */
            server_ptr -> nx_web_http_server_request_packet = new_packet_ptr;
            server_ptr -> nx_web_http_server_chunked_request_remaining_size = new_packet_ptr -> nx_packet_length;

            /* Get the processed chunked packet. Set this packet as the returned packet.  */
            status = _nx_web_http_server_packet_get(server_ptr, packet_ptr);
            if (status)
            {
                return(status);
            }
        }
        else
        {

            /* Set the returned packet.  */
            *packet_ptr = new_packet_ptr;
        }
    }

    /* Determine if there is content in this HTTP request.  */
    if(content_length)
    {

        if (packet_chunked)
        {

            /* If the packet is chunked, set content length as packet length.  */
            *content_length = (*packet_ptr) -> nx_packet_length;
        }
        else
        {

            /* Set the content length as the length in HTTP header.  */
            *content_length =  length;
        }
    }

    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_packet_get                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP server packet get call. */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    packet_ptr                            Pointer to the packet to      */
/*                                            be returned.                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_packet_get        Actual server packet get call */
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
UINT  _nxe_web_http_server_packet_get(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr)
{


    /* Check for invalid packet pointer.  */
    if((server_ptr == NX_NULL) || (server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) ||
       (packet_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Call actual server packet get function.  */
    return(_nx_web_http_server_packet_get(server_ptr, packet_ptr));

}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_packet_get                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function obtains the next packet from the HTTP server socket.  */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    packet_ptr                            Pointer to the packet to      */
/*                                            be returned.                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_request_chunked_get                             */
/*                                          Get chunked request packet    */
/*    _nx_web_http_server_receive           Receive another packet        */
/*    nx_packet_release                     Packet release                */
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
UINT _nx_web_http_server_packet_get(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr)
{
NX_PACKET *new_packet_ptr;
UINT       status; 

    if (server_ptr -> nx_web_http_server_request_chunked)
    {

        /* If the request packet is chunked, remove the chunk header and get the packet which contain the chunk data.  */
        status = _nx_web_http_server_request_chunked_get(server_ptr, &new_packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_RECEIVE);
    }
    else
    {

        /* Receive another packet from client.  */
        status = _nx_web_http_server_receive(server_ptr, &new_packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_RECEIVE);
    }

    /* Check the return status.  */
    if (status != NX_SUCCESS)
    {

        if (server_ptr -> nx_web_http_server_request_chunked)
        {

            /* Reset the chunked info.  */
            nx_packet_release(server_ptr -> nx_web_http_server_request_packet);
            server_ptr -> nx_web_http_server_request_packet = NX_NULL;
            server_ptr -> nx_web_http_server_chunked_request_remaining_size = 0;
            server_ptr -> nx_web_http_server_request_chunked = NX_FALSE;
            return(status);
        }

        /* Error, return to caller.  */
        return(NX_WEB_HTTP_TIMEOUT);
    }
    
    *packet_ptr = new_packet_ptr;

    return(NX_SUCCESS);

}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_content_get_extended           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP extended get content    */
/*    service.                                                            */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    packet_ptr                            Pointer to HTTP request packet*/
/*    byte_offset                           Byte offset into content      */
/*    destination_ptr                       Pointer to content destination*/
/*    destination_size                      Maximum size of destination   */
/*    actual_size                           Actual amount of content      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_content_get_extended                            */
/*                                          Actual extended get content   */
/*                                              get service               */
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
UINT  _nxe_web_http_server_content_get_extended(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset,
                                            CHAR *destination_ptr, UINT destination_size, UINT *actual_size)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) ||
        (packet_ptr == NX_NULL) || (destination_ptr == NX_NULL) || (actual_size == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual server content get function.  */
    status =  _nx_web_http_server_content_get_extended(server_ptr, packet_ptr, byte_offset, destination_ptr, destination_size, actual_size);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_content_get_extended            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the user's specified portion of the HTTP      */
/*    content. Content is typically included in POST and PUT requests     */
/*    from the client. This routine is designed to be called from the     */
/*    application's request notify callback.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    packet_ptr                            Pointer to HTTP request packet*/
/*    byte_offset                           Byte offset into content      */
/*    destination_ptr                       Pointer to content destination*/
/*    destination_size                      Maximum size of destination   */
/*    actual_size                           Actual amount of content      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_chunked_check     Check if the packet is chunked*/
/*    _nx_web_http_server_packet_get        Receive another packet        */
/*    _nx_web_http_server_content_length_get                              */
/*                                          Get content length with error */
/*                                          status returned separately    */
/*                                          content length value.         */
/*    _nx_web_http_server_calculate_content_offset                        */
/*                                          Pickup content offset         */
/*    _nx_web_http_server_packet_content_find                             */
/*                                          Get the content packet        */
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
UINT  _nx_web_http_server_content_get_extended(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG byte_offset,
                                               CHAR *destination_ptr, UINT destination_size, UINT *actual_size)
{

ULONG       content_length;
ULONG       offset;
UINT        status;
UINT        remaining_bytes;
NX_PACKET   *new_packet_ptr;
CHAR        *buffer_ptr;
#ifndef NX_DISABLE_PACKET_CHAIN
NX_PACKET   *header_packet_ptr = NX_NULL;
#endif

    /* Default the actual size to zero.  */
    *actual_size =  0;

    if (_nx_web_http_server_chunked_check(packet_ptr) == NX_FALSE)
    {

        /* Determine if there is content in this HTTP request.  */
        status =  _nx_web_http_server_content_length_get(packet_ptr, &content_length);

        if (status != NX_SUCCESS)
        {
            return status;
        }

        /* Make sure the content length is at least greater than the byte offset supplied.  */
        if (content_length <= byte_offset)
        {

            /* No more data in this buffer.  */
            return(NX_WEB_HTTP_DATA_END);
        }

        /* Determine if the destination size is greater than the content length.  */
        if (destination_size > content_length)
        {

            /* Yes, make the destination size equal to the content length.  */
            destination_size = content_length;
        }

        /* Add the byte offset with content offset.  */
        byte_offset +=  ((ULONG) _nx_web_http_server_calculate_content_offset(packet_ptr));
    }
    else
    {
#ifdef NX_DISABLE_PACKET_CHAIN

        NX_PARAMETER_NOT_USED(server_ptr);

        return NX_WEB_HTTP_ERROR;
#else

        /* If never processed the chunked packet, need to separate the HTTP header and content.  */
        if (!server_ptr -> nx_web_http_server_expect_receive_bytes)
        {

            /* Remember the header packet.  */
            header_packet_ptr = packet_ptr;

            /* Separate the HTTP header and get the first content packet.  */
            status = _nx_web_http_server_packet_content_find(server_ptr, &packet_ptr, NX_NULL);
            if (status)
            {
                return(status);
            }
        }
        else
        {

            /* If the HTTP header and content is already separated, process this packet as normal packet.  */
            byte_offset +=  ((ULONG) _nx_web_http_server_calculate_content_offset(packet_ptr));
        }
#endif
    }

    /* Determine if we need to read one or more additional packets.  */
    while (byte_offset >= packet_ptr -> nx_packet_length)
    {

#ifdef NX_DISABLE_PACKET_CHAIN
        NX_PARAMETER_NOT_USED(server_ptr);

        return NX_WEB_HTTP_ERROR;
#else
        /* Read another packet because the offset reaches past the current packet
           length.  */
        status = _nx_web_http_server_packet_get(server_ptr, &new_packet_ptr);

        /* Check the return status.  */
        if (status != NX_SUCCESS)
        {

            /* Error, return to caller.  */
            return(NX_WEB_HTTP_TIMEOUT);
        }

        /* Otherwise, we have a new packet to append to the head packet.  */

        /* Determine if the current packet is already chained.  */
        if (packet_ptr -> nx_packet_next)
        {

            /* Yes, link the current last packet to the new packet.  */
            (packet_ptr -> nx_packet_last) -> nx_packet_next =  new_packet_ptr;
        }
        else
        {

            /* Otherwise, this is our first chained packet.  Link to the head.  */
            packet_ptr -> nx_packet_next =  new_packet_ptr;
        }

        /* Is the new packet chained?  */
        if (new_packet_ptr -> nx_packet_next)
        {

            /* Yes, there is a last packet pointer.  Point to this from the head packet.  */
            packet_ptr -> nx_packet_last =  new_packet_ptr -> nx_packet_last;
        }
        else
        {

            /* No, there isn't a last packet pointer.  Point to new packet from the head packet.  */
            packet_ptr -> nx_packet_last =  new_packet_ptr;
        }

        /* Update the packet length.  */
        packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length + new_packet_ptr -> nx_packet_length;
#endif /* NX_DISABLE_PACKET_CHAIN */
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    if (header_packet_ptr)
    {

        /* Append the content packet to the header packet.  */
        if (header_packet_ptr -> nx_packet_next)
        {
            (header_packet_ptr -> nx_packet_last) -> nx_packet_next =  packet_ptr;
        }
        else
        {
            header_packet_ptr -> nx_packet_next =  packet_ptr;
        }

        /* Update the last packet.  */
        if (packet_ptr -> nx_packet_next)
        {
            header_packet_ptr -> nx_packet_last =  packet_ptr -> nx_packet_last;
        }
        else
        {
            header_packet_ptr -> nx_packet_last =  packet_ptr;
        }

        /* Update the packet length.  */
        header_packet_ptr -> nx_packet_length =  header_packet_ptr -> nx_packet_length + packet_ptr -> nx_packet_length;
    }
#endif

    /* Default the buffer pointer to NULL.  */
    buffer_ptr =  NX_NULL;

    /* Now find the packet that contains the offset.  */
    offset =  0;
    new_packet_ptr =  packet_ptr;
    do
    {

        /* Determine if the get request falls into this packet.  */
        if (byte_offset < (offset + (ULONG)(new_packet_ptr -> nx_packet_append_ptr - new_packet_ptr -> nx_packet_prepend_ptr)))
        {

            /* Yes, the get offset is in this packet.  */

            /* Setup the starting byte pointer.  */
            buffer_ptr =  ((CHAR *) new_packet_ptr -> nx_packet_prepend_ptr) + (byte_offset - offset);
            break;
        }

        /* Otherwise update the offset.  */
        offset =  offset + (ULONG)(new_packet_ptr -> nx_packet_append_ptr - new_packet_ptr -> nx_packet_prepend_ptr);

#ifdef NX_DISABLE_PACKET_CHAIN
        new_packet_ptr =  NX_NULL;
#else
        /* Move to next packet in the chain.  */
        new_packet_ptr =  new_packet_ptr -> nx_packet_next;
#endif /* NX_DISABLE_PACKET_CHAIN */

    } while (new_packet_ptr);

    /* Determine if an error occurred in the search.  */
    if ((buffer_ptr == (CHAR *) NX_NULL) || (new_packet_ptr == NX_NULL))
        return(NX_WEB_HTTP_ERROR);

    /* Now determine if the maximum buffer size has to be adjusted.  */
    if (destination_size > (UINT) (packet_ptr -> nx_packet_length - byte_offset))
    {

        /* Adjust the destination size downward.  */
        destination_size =  (UINT) (packet_ptr -> nx_packet_length - byte_offset);
    }

    /* Initialize the remaining bytes.  */
    remaining_bytes =  destination_size;

    /* Otherwise copy the bytes from the offset to the end of the packet pointer.  */
    while (remaining_bytes--)
    {

#ifndef NX_DISABLE_PACKET_CHAIN

        /* If the packet is chained, copy the data in the chain.  */
        if (buffer_ptr == (CHAR *)new_packet_ptr -> nx_packet_append_ptr)
        {
            new_packet_ptr = new_packet_ptr -> nx_packet_next;
            buffer_ptr = (CHAR *)new_packet_ptr -> nx_packet_prepend_ptr;
        }
#endif /* NX_DISABLE_PACKET_CHAIN */

        /* Copy a byte to the destination.  */
        *destination_ptr++ =  *buffer_ptr++;
    }

    /* Successful completion.  */
    *actual_size =  destination_size;
    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_content_length_get             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP content length service. */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to HTTP packet        */
/*    content_length                        Pointer for returning content */
/*                                             length value               */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_content_length_get                              */
/*                                          Actual get content length call*/
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
UINT  _nxe_web_http_server_content_length_get(NX_PACKET *packet_ptr, ULONG *content_length)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((packet_ptr == NX_NULL) || (content_length == NX_NULL))
        return(NX_PTR_ERROR);

    /* Call actual server content length get function.  */
    status =  _nx_web_http_server_content_length_get(packet_ptr, content_length);

    /* Return completion status. */
    return status;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_content_length_get              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function find a valid content length field and returns a       */
/*    successful completion status (NX_SUCCESS) if so. Otherwise it       */
/*    it returns an error status.  A valid content length value is        */
/*    zero or more and matches the size of the message body.              */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to HTTP request packet*/
/*    content_length                        Pointer to a valid Content    */
/*                                            Length value                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Content length extracted      */
/*    status                                Invalid content length        */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Threads                                                             */
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
UINT  _nx_web_http_server_content_length_get(NX_PACKET *packet_ptr, ULONG *length)
{

CHAR    *buffer_ptr;


    /* Default the content length to no data.  */
    *length =  0;

    /* Setup pointer to buffer.  */
    buffer_ptr =  (CHAR *) packet_ptr -> nx_packet_prepend_ptr;

    /* Find the "Content-length: " token first.  */
    while (((buffer_ptr+17) < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0))
    {

        /* Check for the Content-length token.  */
        if (((*buffer_ptr ==      'c') || (*buffer_ptr ==      'C')) &&
            ((*(buffer_ptr+1) ==  'o') || (*(buffer_ptr+1) ==  'O')) &&
            ((*(buffer_ptr+2) ==  'n') || (*(buffer_ptr+2) ==  'N')) &&
            ((*(buffer_ptr+3) ==  't') || (*(buffer_ptr+3) ==  'T')) &&
            ((*(buffer_ptr+4) ==  'e') || (*(buffer_ptr+4) ==  'E')) &&
            ((*(buffer_ptr+5) ==  'n') || (*(buffer_ptr+5) ==  'N')) &&
            ((*(buffer_ptr+6) ==  't') || (*(buffer_ptr+6) ==  'T')) &&
            (*(buffer_ptr+7) ==  '-') &&
            ((*(buffer_ptr+8) ==  'l') || (*(buffer_ptr+8) ==  'L')) &&
            ((*(buffer_ptr+9) ==  'e') || (*(buffer_ptr+9) ==  'E')) &&
            ((*(buffer_ptr+10) == 'n') || (*(buffer_ptr+10) == 'N')) &&
            ((*(buffer_ptr+11) == 'g') || (*(buffer_ptr+11) == 'G')) &&
            ((*(buffer_ptr+12) == 't') || (*(buffer_ptr+12) == 'T')) &&
            ((*(buffer_ptr+13) == 'h') || (*(buffer_ptr+13) == 'H')) &&
            (*(buffer_ptr+14) == ':') &&
            (*(buffer_ptr+15) == ' '))
        {

            /* Move the pointer up to the length token.  */
            buffer_ptr =  buffer_ptr + 16;
            break;
        }

        /* Move the pointer up to the next character.  */
        buffer_ptr++;
    }

    /* Now convert the length into a numeric value.  */
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr >= '0') && (*buffer_ptr <= '9'))
    {

        /* Update the content length.  */
        *length =  *length * 10;
        *length =  *length + (((UINT) (*buffer_ptr)) - 0x30);

        /* Move the buffer pointer forward.  */
        buffer_ptr++;
    }

     /* Determine if the content length was picked up properly.  */
     if ((buffer_ptr >= (CHAR *) packet_ptr -> nx_packet_append_ptr) ||
         ((*buffer_ptr != ' ') && (*buffer_ptr != (CHAR) 13)))
     {

         /* Error, set the length to zero.  */
         return NX_WEB_HTTP_INCOMPLETE_PUT_ERROR;
     }

    /* Return successful completion status to the caller.  */
    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_create                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP server create call.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    http_server_name                      Name of HTTP server           */
/*    server_port                           Port to use for HTTP server   */
/*    ip_ptr                                Pointer to IP instance        */
/*    media_ptr                             Pointer to media structure    */
/*    stack_ptr                             Server thread's stack pointer */
/*    stack_size                            Server thread's stack size    */
/*    pool_ptr                              Pointer to packet pool        */
/*    authentication_check                  Pointer to application's      */
/*                                            authentication checking     */
/*    request_notify                        Pointer to application's      */
/*                                            request notify service      */
/*    http_server_size                      Size of HTTP server           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_create            Actual server create call     */
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
UINT  _nxe_web_http_server_create(NX_WEB_HTTP_SERVER *http_server_ptr, CHAR *http_server_name, NX_IP *ip_ptr, UINT server_port, FX_MEDIA *media_ptr, VOID *stack_ptr, ULONG stack_size, NX_PACKET_POOL *pool_ptr,
                                UINT (*authentication_check)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, CHAR **password, CHAR **realm),
                                UINT (*request_notify)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr), UINT http_server_size)
{

NX_PACKET   *packet_ptr;
UINT        status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) ||
        (http_server_ptr == NX_NULL) || (http_server_ptr -> nx_web_http_server_id == NX_WEB_HTTP_SERVER_ID) ||
        (stack_ptr == NX_NULL) || (pool_ptr == NX_NULL) ||
        (http_server_size != sizeof(NX_WEB_HTTP_SERVER)))
        return(NX_PTR_ERROR);

    /* Pickup a packet from the supplied packet pool.  */
    packet_ptr =  pool_ptr -> nx_packet_pool_available_list;

    /* Determine if the packet payload is equal to or greater than the maximum HTTP header supported.  */
    if (((packet_ptr -> nx_packet_data_end - packet_ptr -> nx_packet_data_start) - NX_PHYSICAL_TRAILER)
                                                                                            < NX_WEB_HTTP_SERVER_MIN_PACKET_SIZE)
        return(NX_WEB_HTTP_POOL_ERROR);

    /* Call actual server create function.  */
    status =  _nx_web_http_server_create(http_server_ptr, http_server_name, ip_ptr, server_port, media_ptr, stack_ptr, stack_size, pool_ptr,
                            authentication_check, request_notify);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_create                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a HTTP server on the specified IP. In doing   */
/*    so this function creates an TCP socket for subsequent HTTP          */
/*    transfers and a thread for the HTTP server.                         */
/*                                                                        */
/*    Note: The string resource in callback functions authentication_check*/
/*    and request_notify is built by internal logic and always            */
/*    NULL-terminated.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    http_server_name                      Name of HTTP server           */
/*    server_port                           Port for HTTP server          */
/*    ip_ptr                                Pointer to IP instance        */
/*    media_ptr                             Pointer to media structure    */
/*    stack_ptr                             Server thread's stack pointer */
/*    stack_size                            Server thread's stack size    */
/*    pool_ptr                              Pointer to packet pool        */
/*    authentication_check                  Pointer to application's      */
/*                                            authentication checking     */
/*    request_notify                        Pointer to application's      */
/*                                            request notify service      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_tcp_socket_create                  Create HTTP server socket     */
/*    nx_tcp_socket_delete                  Delete the HTTP server socket */
/*    tx_thread_create                      Create the HTTP server thread */
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
UINT _nx_web_http_server_create(NX_WEB_HTTP_SERVER *http_server_ptr,
                               CHAR *http_server_name, NX_IP *ip_ptr,
                               UINT server_port, FX_MEDIA *media_ptr,
                               VOID *stack_ptr, ULONG stack_size,
                               NX_PACKET_POOL *pool_ptr,
                               UINT (*authentication_check)(NX_WEB_HTTP_SERVER *server_ptr,
                                                            UINT request_type, CHAR *resource,
                                                            CHAR **name, CHAR **password, CHAR **realm),
                              UINT (*request_notify)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type,
                                                     CHAR *resource, NX_PACKET *packet_ptr))
{

UINT        status;


    /* Clear the HTTP server structure.  */
    memset((void *) http_server_ptr, 0, sizeof(NX_WEB_HTTP_SERVER));

    /* Create the Server's TCP socket.  */
    status = nx_tcpserver_create(ip_ptr, &http_server_ptr -> nx_web_http_server_tcpserver, http_server_name,
                                 NX_WEB_HTTP_TYPE_OF_SERVICE,  NX_WEB_HTTP_FRAGMENT_OPTION, NX_WEB_HTTP_TIME_TO_LIVE,
                                 NX_WEB_HTTP_SERVER_WINDOW_SIZE, NX_NULL,
                                 _nx_web_http_server_receive_data, _nx_web_http_server_connection_end,
                                 _nx_web_http_server_connection_timeout, NX_WEB_HTTP_SERVER_TIMEOUT / NX_IP_PERIODIC_RATE,
                                 stack_ptr, stack_size, http_server_ptr -> nx_web_http_server_session_buffer,
                                 NX_WEB_HTTP_SERVER_SESSION_BUFFER_SIZE, NX_WEB_HTTP_SERVER_PRIORITY, NX_WEB_HTTP_SERVER_TIMEOUT_ACCEPT);

    /* Determine if an error occurred.   */
    if (status != NX_SUCCESS)
    {

        /* Yes, return error code.  */
        return(status);
    }

    /* Store server ptr. */
    http_server_ptr -> nx_web_http_server_tcpserver.nx_tcpserver_reserved = (ULONG)http_server_ptr;

    /* Save the Server name.  */
    http_server_ptr -> nx_web_http_server_name =  http_server_name;

    /* Save the IP pointer address.  */
    http_server_ptr -> nx_web_http_server_ip_ptr =  ip_ptr;

    /* Save the packet pool pointer.  */
    http_server_ptr -> nx_web_http_server_packet_pool_ptr =  pool_ptr;

    /* Save the media pointer address.  */
    http_server_ptr -> nx_web_http_server_media_ptr =  media_ptr;

    /* Save the user-supplied routines, if specified.  */
    http_server_ptr -> nx_web_http_server_authentication_check =  authentication_check;
    http_server_ptr -> nx_web_http_server_request_notify =        request_notify;

    /* Set the server ID to indicate the HTTP server is ready.  */
    http_server_ptr -> nx_web_http_server_id =  NX_WEB_HTTP_SERVER_ID;

    /* Set the port to the default HTTP listening port (80). */
    http_server_ptr -> nx_web_http_server_listen_port = server_port;

    /* Return successful completion.  */
    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_delete                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP server delete call.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_delete            Actual server delete call     */
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
UINT  _nxe_web_http_server_delete(NX_WEB_HTTP_SERVER *http_server_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((http_server_ptr == NX_NULL) || (http_server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual server delete function.  */
    status =  _nx_web_http_server_delete(http_server_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_delete                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes a previously created HTTP server on the       */
/*    specified IP.                                                       */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_tcp_socket_delete                  Delete the HTTP server socket */
/*    nx_tcp_socket_disconnect              Disconnect HTTP server socket */
/*    nx_tcp_socket_unaccept                Unaccept HTTP server connect  */
/*    nx_tcp_socket_unlisten                Unlisten on the HTTP port     */
/*    tx_thread_delete                      Delete the HTTP server thread */
/*    tx_thread_suspend                     Suspend the HTTP server thread*/
/*    tx_thread_terminate                   Terminate the HTTP server     */
/*                                            thread                      */
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
UINT  _nx_web_http_server_delete(NX_WEB_HTTP_SERVER *http_server_ptr)
{
UINT status;
NX_TCPSERVER *tcpserver_ptr = &(http_server_ptr -> nx_web_http_server_tcpserver);


#ifdef NX_WEB_HTTPS_ENABLE
UINT i;
    
    /* Stop TLS session if using HTTPS. */
    if(http_server_ptr -> nx_web_http_is_https_server)
    {
        for(i = 0; i < tcpserver_ptr -> nx_tcpserver_sessions_count; i++)
        {
            nx_secure_tls_session_end(&(tcpserver_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_tls_session), NX_WAIT_FOREVER);
            nx_secure_tls_session_delete(&(tcpserver_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_tls_session));
        }
    }

#endif

    /* Delete the TCP server.  */
    status = nx_tcpserver_delete(tcpserver_ptr);

    /* Clear the server ID to indicate the HTTP server is no longer ready.  */
    http_server_ptr -> nx_web_http_server_id =  0;

    /* Return status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_param_get                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP parameter get call.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to HTTP request packet*/
/*    param_number                          Parameter number (start at 0) */
/*    param_ptr                             Pointer to destination        */
/*    max_param_size                        Maximum size of destination   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_param_get         Actual server parameter get   */
/*                                            call                        */
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
UINT  _nxe_web_http_server_param_get(NX_PACKET *packet_ptr, UINT param_number, CHAR *param_ptr, UINT *param_size, UINT max_param_size)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((packet_ptr == NX_NULL) || (param_ptr == NX_NULL) || (param_size == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual server parameter get function.  */
    status =  _nx_web_http_server_param_get(packet_ptr, param_number, param_ptr, param_size, max_param_size);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_param_get                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the requested parameter specified in the URL  */
/*    requested by the client. If the specified parameter is not present, */
/*    a not found error is returned to the caller.                        */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to HTTP request packet*/
/*    param_number                          Parameter number (start at 0) */
/*    param_ptr                             Pointer to destination        */
/*    max_param_size                        Maximum size of destination   */
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
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_web_http_server_param_get(NX_PACKET *packet_ptr, UINT param_number, CHAR *param_ptr, UINT *param_size, UINT max_param_size)
{

UINT    i;
UINT    current_param;
CHAR    *buffer_ptr;


    /* Set the destination string to NULL.  */
    param_ptr[0] =  (CHAR) NX_NULL;
    *param_size = 0;

    /* Set current parameter to 0.  */
    current_param =  0;

    /* Setup a pointer to the HTTP buffer.  */
    buffer_ptr =  (CHAR *) packet_ptr -> nx_packet_prepend_ptr;

    /* Position to the start of the URL.  */
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != '/'))
    {

        /* Move the buffer pointer.  */
        buffer_ptr++;
    }

    /* Not find URL.  */
    if (buffer_ptr >= (CHAR *) packet_ptr -> nx_packet_append_ptr)
    {
        return(NX_WEB_HTTP_NOT_FOUND);
    }

    /* Loop through the buffer to search for the specified parameter.  */
    do
    {

        /* Determine if the character is a semicolon, indicating a parameter
           is present.  */
        if (*buffer_ptr == ';')
        {

            /* Yes, a parameter is present.  */

            /* Move the buffer pointer forward.  */
            buffer_ptr++;

            /* Is this the parameter requested?  */
            if (current_param == param_number)
            {


                /* Yes, we have found the parameter.  */
                for (i = 0; i < max_param_size; i++)
                {

                    /* Check if reach the end of the packet data.  */
                    if (buffer_ptr >= (CHAR *)packet_ptr -> nx_packet_append_ptr)
                    {
                        return(NX_WEB_HTTP_NOT_FOUND);
                    }

                    /* Check for end of parameter.  */
                    if ((*buffer_ptr == ';') || (*buffer_ptr == '?') ||
                        (*buffer_ptr == '&') || (*buffer_ptr == ' ') ||
                        (*buffer_ptr == (CHAR) 13))
                    {

                        /* Yes, we are finished and need to get out of the loop.  */
                        break;
                    }

                    /* Otherwise, store the character in the destination.  */
                    param_ptr[i] =  *buffer_ptr++;
                }

                /* NULL terminate the parameter.  */
                if (i < max_param_size)
                {
                    param_ptr[i] =  (CHAR) NX_NULL;
                }

                /* Return to caller.  */
                if (param_ptr[i] == (CHAR) NX_NULL)
                {
                    *param_size = i;
                    return(NX_SUCCESS);
                }
                else
                {
                    return(NX_WEB_HTTP_IMPROPERLY_TERMINATED_PARAM);
                }
            }
            else
            {

                /* Increment the current parameter.  */
                current_param++;
            }
        }
        else
        {

            /* Check for any other character that signals the end of the param list.  */
            if ((*buffer_ptr == '?') || (*buffer_ptr == ' ') || (*buffer_ptr == '&'))
                break;

            /* Update the buffer pointer.  */
            buffer_ptr++;
        }

    } while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 13));

    /* Return a not found error.  */
    return(NX_WEB_HTTP_NOT_FOUND);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_query_get                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP query get call.         */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to HTTP request packet*/
/*    query_number                          Query number (start at 0)     */
/*    query_ptr                             Pointer to destination        */
/*    query_size                            Pointer to data size return   */
/*    max_query_size                        Maximum size of destination   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_query_get         Actual server query get       */
/*                                            call                        */
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
UINT  _nxe_web_http_server_query_get(NX_PACKET *packet_ptr, UINT query_number, CHAR *query_ptr, UINT *query_size, UINT max_query_size)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((packet_ptr == NX_NULL) || (query_ptr == NX_NULL) || (query_size == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual server query get function.  */
    status =  _nx_web_http_server_query_get(packet_ptr, query_number, query_ptr, query_size, max_query_size);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_query_get                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the requested query specified in the URL      */
/*    requested by the client. If the specified query is not present,     */
/*    a not found error is returned to the caller.                        */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to HTTP request packet*/
/*    query_number                          Query number (start at 0)     */
/*    query_ptr                             Pointer to destination        */
/*    query_size                            Pointer to data size return   */
/*    max_query_size                        Maximum size of destination   */
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
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_web_http_server_query_get(NX_PACKET *packet_ptr, UINT query_number, CHAR *query_ptr, UINT *query_size, UINT max_query_size)
{

UINT    i;
UINT    current_query;
CHAR    *buffer_ptr;


    /* Set the destination string to NULL.  */
    query_ptr[0] =  (CHAR) NX_NULL;
    *query_size = 0;

    /* Set current query number to 0.  */
    current_query =  0;

    /* Setup a pointer to the HTTP buffer.  */
    buffer_ptr =  (CHAR *) packet_ptr -> nx_packet_prepend_ptr;

    /* Position to the start of the URL.  */
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != '/'))
    {

        /* Move the buffer pointer.  */
        buffer_ptr++;
    }

    /* Not find URL.  */
    if (buffer_ptr >= (CHAR *) packet_ptr -> nx_packet_append_ptr)
    {
        return(NX_WEB_HTTP_NOT_FOUND);
    }

    /* Loop through the buffer to search for the specified query instance.  */
    do
    {

        /* Determine if the character is a '?' or a '&', indicating a query
           is present.  */
        if (((*buffer_ptr == '?') && (current_query == 0)) ||
            ((*buffer_ptr == '&') && (current_query != 0)))
        {

            /* Yes, a query is present.  */

            /* Move the buffer pointer forward.  */
            buffer_ptr++;

            /* Is this the query requested?  */
            if (current_query == query_number)
            {


                /* Yes, we have found the query.  */
                for (i = 0; i < max_query_size; i++)
                {

                    /* Check if reach the end of the packet data.  */
                    if (buffer_ptr >= (CHAR *)packet_ptr -> nx_packet_append_ptr)
                    {
                        return(NX_WEB_HTTP_NOT_FOUND);
                    }

                    /* Check for end of query.  */
                    if ((*buffer_ptr == ';') || (*buffer_ptr == '?') ||
                        (*buffer_ptr == '&') || (*buffer_ptr == ' ') ||
                        (*buffer_ptr == (CHAR) 13))
                    {

                        /* Yes, we are finished and need to get out of the loop.  */
                        break;
                    }

                    /* Otherwise, store the character in the destination.  */
                    query_ptr[i] =  *buffer_ptr++;
                }

                /* NULL terminate the query.  */
                query_ptr[i] =  (CHAR) NX_NULL;

                /* Return to caller.  */
                if (i)
                {
                    *query_size = i;
                    return(NX_SUCCESS);
                }
                else
                {
                    return(NX_WEB_HTTP_NO_QUERY_PARSED);
                }
            }
            else
            {

                /* Increment the current query.  */
                current_query++;
            }
        }
        else
        {

            /* Check for any other character that signals the end of the query list.  */
            if ((*buffer_ptr == '?') || (*buffer_ptr == ' ') || (*buffer_ptr == ';'))
                break;

            /* Update the buffer pointer.  */
            buffer_ptr++;
        }

    } while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 13));

    /* Return a not found error.  */
    return(NX_WEB_HTTP_NOT_FOUND);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_start                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP server start call.      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_start             Actual server start call      */
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
UINT  _nxe_web_http_server_start(NX_WEB_HTTP_SERVER *http_server_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((http_server_ptr == NX_NULL) || (http_server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID))
        return(NX_PTR_ERROR);

    /* Call actual server start function.  */
    status =  _nx_web_http_server_start(http_server_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_start                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function starts a previously created HTTP server on the        */
/*    specified IP.                                                       */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_tcp_server_socket_listen           Start listening on HTTP port  */
/*    tx_thread_resume                      Resume the HTTP server thread */
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
UINT  _nx_web_http_server_start(NX_WEB_HTTP_SERVER *http_server_ptr)
{

UINT    status;
UINT    port;

    port = http_server_ptr -> nx_web_http_server_listen_port;

    /* Start listening on the HTTP server port.  */
    status =  nx_tcpserver_start(&http_server_ptr -> nx_web_http_server_tcpserver, port, NX_WEB_HTTP_SERVER_MAX_PENDING);

    /* Determine if the listen was unsuccessful.  */
    if (status != NX_SUCCESS)
    {
        /* Just return an HTTP error.  */
        return(status);
    }

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

#ifdef NX_WEB_HTTPS_ENABLE

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_secure_configure               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTPS configuration call.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    crypto_table                          TLS cryptographic routines    */
/*    metadata_buffer                       Cryptographic metadata buffer */
/*    metadata_size                         Size of metadata buffer       */
/*    packet_buffer                         TLS packet buffer             */
/*    packet_size                           Size of packet buffer         */
/*    identity_certificate                  TLS server certificate        */
/*    trusted_certificates                  TLS trusted certificates      */
/*    trusted_certs_num                     Number of trusted certs       */
/*    remote_certificates                   Remote certificates array     */
/*    remote_certs_num                      Number of remote certificates */
/*    remote_certificate_buffer             Buffer for remote certs       */
/*    remote_cert_buffer_size               Size of remote cert buffer    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_secure_configure Actual server configure call   */
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
UINT  _nxe_web_http_server_secure_configure(NX_WEB_HTTP_SERVER *http_server_ptr, const NX_SECURE_TLS_CRYPTO *crypto_table,
                                            VOID *metadata_buffer, ULONG metadata_size,
                                            UCHAR* packet_buffer, UINT packet_buffer_size,
                                            NX_SECURE_X509_CERT *identity_certificate,
                                            NX_SECURE_X509_CERT *trusted_certificates[],
                                            UINT trusted_certs_num,
                                            NX_SECURE_X509_CERT *remote_certificates[],
                                            UINT remote_certs_num,
                                            UCHAR *remote_certificate_buffer,
                                            UINT remote_cert_buffer_size)
{

UINT    status;


    /* Check for invalid input pointers. Note that the remote certificates array and buffer
     * may be NX_NULL for the HTTPS server since remote certificates are optional for TLS Server.*/
    if ((http_server_ptr == NX_NULL) || (http_server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) ||
         crypto_table == NX_NULL || metadata_buffer == NX_NULL || packet_buffer == NX_NULL ||
         identity_certificate == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual server start function.  */
    status =  _nx_web_http_server_secure_configure(http_server_ptr, crypto_table, metadata_buffer, metadata_size,
                                                   packet_buffer, packet_buffer_size, identity_certificate,
                                                   trusted_certificates, trusted_certs_num,
                                                   remote_certificates, remote_certs_num, remote_certificate_buffer,
                                                   remote_cert_buffer_size);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_secure_configure                PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function configures a previously created NetX Web HTTP server  */
/*    instance to use TLS for secure HTTPS communications. The parameters */
/*    are used to configure all the possible TLS sessions with identical  */
/*    state so that each incoming HTTPS client experiences consistent     */
/*    behavior. The number of TLS sessions is controlled using the macro  */
/*    NX_WEB_HTTP_SERVER_SESSION_MAX.                                     */
/*                                                                        */
/*    The cryptographic routine table (ciphersuite table) is shared       */
/*    between all TLS sessions as it just contains function pointers.     */
/*                                                                        */
/*    The metadata buffer and packet reassembly buffer are divided        */
/*    equally between all TLS sessions. If the buffer size is not evenly  */
/*    divisible by the number of sessions the remainder will be unused.   */
/*                                                                        */
/*    The passed-in identity certificate is used by all sessions. During  */
/*    TLS operation the server identity certificate is only read from so  */
/*    copies are not needed for each session.                             */
/*                                                                        */
/*    The trusted certificates are added to the trusted store for each    */
/*    TLS session in the server. This is used for client certificate      */
/*    verification which is enabled if remote certificates are provided.  */
/*                                                                        */
/*    The remote certificate array and buffer is shared by default        */
/*    between all TLS sessions. This does mean that some sessions may     */
/*    block during certificate validation.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    crypto_table                          TLS cryptographic routines    */
/*    metadata_buffer                       Cryptographic metadata buffer */
/*    metadata_size                         Size of metadata buffer       */
/*    packet_buffer                         TLS packet buffer             */
/*    packet_size                           Size of packet buffer         */
/*    identity_certificate                  TLS server certificate        */
/*    trusted_certificates                  TLS trusted certificates      */
/*    trusted_certs_num                     Number of trusted certs       */
/*    remote_certificates                   Remote certificates array     */
/*    remote_certs_num                      Number of remote certificates */
/*    remote_certificate_buffer             Buffer for remote certs       */
/*    remote_cert_buffer_size               Size of remote cert buffer    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_tcpserver_tls_setup                Socket server TLS configure   */
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
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_web_http_server_secure_configure(NX_WEB_HTTP_SERVER *http_server_ptr, const NX_SECURE_TLS_CRYPTO *crypto_table,
                                            VOID *metadata_buffer, ULONG metadata_size,
                                            UCHAR* packet_buffer, UINT packet_buffer_size,
                                            NX_SECURE_X509_CERT *identity_certificate,
                                            NX_SECURE_X509_CERT *trusted_certificates[],
                                            UINT trusted_certs_num,
                                            NX_SECURE_X509_CERT *remote_certificates[],
                                            UINT remote_certs_num,
                                            UCHAR *remote_certificate_buffer,
                                            UINT remote_cert_buffer_size)
{

UINT    status;

    /* This is a secure HTTP server, so set HTTPS flag. */
    http_server_ptr -> nx_web_http_is_https_server = NX_TRUE;

    /* Configure TLS for the socket server. */
    status = nx_tcpserver_tls_setup(&http_server_ptr->nx_web_http_server_tcpserver,
                                    crypto_table, metadata_buffer, metadata_size,
                                    packet_buffer, packet_buffer_size, identity_certificate,
                                    trusted_certificates, trusted_certs_num,
                                    remote_certificates, remote_certs_num,
                                    remote_certificate_buffer,remote_cert_buffer_size);
    
    /* Return result of TLS setup. */
    return(status);
}

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_secure_ecc_configure           PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTPS ECC configuration call.*/
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    supported_groups                      List of supported groups      */
/*    supported_group_count                 Number of supported groups    */
/*    curves                                List of curve methods         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_secure_ecc_configure                            */
/*                                          Actual ECC configuration call */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-25-2022     Yuxin Zhou               Initial Version 6.1.11        */
/*                                                                        */
/**************************************************************************/
UINT _nxe_web_http_server_secure_ecc_configure(NX_WEB_HTTP_SERVER *http_server_ptr,
                                               const USHORT *supported_groups, USHORT supported_group_count,
                                               const NX_CRYPTO_METHOD **curves)
{
UINT status;

    /* Check for invalid input pointers. */
    if ((http_server_ptr == NX_NULL) || (http_server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) ||
        (supported_groups == NX_NULL) || (supported_group_count == 0) || (curves == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual ECC configuration function.  */
    status = _nx_web_http_server_secure_ecc_configure(http_server_ptr, supported_groups, supported_group_count, curves);
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_secure_ecc_configure            PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function configures supported curve lists for NetX Web HTTP    */
/*    server instance using TLS.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    supported_groups                      List of supported groups      */
/*    supported_group_count                 Number of supported groups    */
/*    curves                                List of curve methods         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_tcpserver_tls_ecc_setup            Socket server ECC configure   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-25-2022     Yuxin Zhou               Initial Version 6.1.11        */
/*                                                                        */
/**************************************************************************/
UINT _nx_web_http_server_secure_ecc_configure(NX_WEB_HTTP_SERVER *http_server_ptr,
                                              const USHORT *supported_groups, USHORT supported_group_count,
                                              const NX_CRYPTO_METHOD **curves)
{
UINT status;

    /* Configure TLS ECC for the socket server. */
    status = nx_tcpserver_tls_ecc_setup(&http_server_ptr -> nx_web_http_server_tcpserver, 
                                        supported_groups, supported_group_count, curves);
    return(status);
}
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

#endif /* NX_WEB_HTTPS_ENABLE */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_stop                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP server stop call.       */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_stop              Actual server stop call       */
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
UINT  _nxe_web_http_server_stop(NX_WEB_HTTP_SERVER *http_server_ptr)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((http_server_ptr == NX_NULL) || (http_server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual server stop function.  */
    status =  _nx_web_http_server_stop(http_server_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_stop                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function stops a previously started HTTP server on the         */
/*    specified IP.                                                       */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_tcpserver_stop                     Suspend the HTTP server thread*/
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
UINT  _nx_web_http_server_stop(NX_WEB_HTTP_SERVER *http_server_ptr)
{
UINT status;
NX_TCPSERVER *tcpserver_ptr = &(http_server_ptr -> nx_web_http_server_tcpserver);

#ifdef NX_WEB_HTTPS_ENABLE
UINT i;

    /* Stop TLS session if using HTTPS. */
    if(http_server_ptr -> nx_web_http_is_https_server)
    {
        for(i = 0; i < tcpserver_ptr -> nx_tcpserver_sessions_count; i++)
        {
            nx_secure_tls_session_end(&(tcpserver_ptr -> nx_tcpserver_sessions[i].nx_tcp_session_tls_session), NX_WAIT_FOREVER);
        }
    }
#endif

    /* Suspend the HTTP server thread.  */
    status = nx_tcpserver_stop(tcpserver_ptr);

    /* Return status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_callback_data_send             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the data send call.              */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    data_ptr                              Pointer to data to send       */
/*    data_length                           Length of data to send        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_callback_data_send                              */
/*                                          Actual function call          */
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
UINT  _nxe_web_http_server_callback_data_send(NX_WEB_HTTP_SERVER *server_ptr, VOID *data_ptr, ULONG data_length)
{
UINT        status;

    /* Check pointers. */
    if(server_ptr == NX_NULL || data_ptr == NX_NULL || server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID)
    {
        return(NX_PTR_ERROR);
    }

    /* Actual function call. */
    status = _nx_web_http_server_callback_data_send(server_ptr, data_ptr, data_length);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_callback_data_send              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends data to the client from the application         */
/*    callback function. This is typically done to satisfy a GET or       */
/*    POST request that is processed completely by the application        */
/*    callback function.                                                  */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    data_ptr                              Pointer to data to send       */
/*    data_length                           Length of data to send        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_response_packet_allocate                        */
/*                                          Allocate a resonse packet     */
/*    nx_packet_data_append                 Append data to packet         */
/*    nx_tcp_socket_send                    Send TCP data                 */
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
UINT  _nx_web_http_server_callback_data_send(NX_WEB_HTTP_SERVER *server_ptr, VOID *data_ptr, ULONG data_length)
{

NX_PACKET   *new_packet_ptr;
UINT        status;


    /* Allocate a new packet for data.  */
    status =  _nx_web_http_server_response_packet_allocate(server_ptr, &new_packet_ptr, NX_WAIT_FOREVER);

    /* Determine if an error is present.  */
    if (status != NX_SUCCESS)
    {

        /* Indicate an allocation error occurred.  */
        server_ptr -> nx_web_http_server_allocation_errors++;

        /* Error, return to caller.  */
        return(NX_WEB_HTTP_ERROR);
    }

    /* Now append the data to the packet.  */
    status =  nx_packet_data_append(new_packet_ptr, data_ptr, data_length,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

    /* Determine if an error is present.  */
    if (status != NX_SUCCESS)
    {

        /* Indicate an allocation error occurred.  */
        server_ptr -> nx_web_http_server_allocation_errors++;

        /* Release the initial packet.  */
        nx_packet_release(new_packet_ptr);

        /* Error, return to caller.  */
        return(status);
    }

    /* Send the data back to the client.  */
    status = _nx_web_http_server_send(server_ptr, new_packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_SEND);

    /* Determine if this is successful.  */
    if (status != NX_SUCCESS)
    {
        /* Release the packet.  */
        nx_packet_release(new_packet_ptr);
    }

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_callback_response_send         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the response send call.          */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    status_code                           Status-code and reason-phrase */
/*    information                           Pointer to HTTP info string   */
/*    additional_information                Pointer to additional HTTP    */
/*                                            information                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxe_web_http_server_callback_response_send                         */
/*                                          Actual function call          */
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
UINT  _nxe_web_http_server_callback_response_send(NX_WEB_HTTP_SERVER *server_ptr, CHAR *status_code, CHAR *information, CHAR *additional_info)
{
UINT status;

    if(server_ptr == NX_NULL || status_code == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Actual function call.  */
    status = _nx_web_http_server_callback_response_send(server_ptr, status_code, information, additional_info);

    /* Return successful completion.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_callback_response_send          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends the completion response to the client from the  */
/*    application callback function. This is typically done to satisfy a  */
/*    GET or POST request that is processed completely by the application */
/*    callback function.                                                  */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    status_code                           Status-code and reason-phrase */
/*    information                           Pointer to HTTP info string   */
/*    additional_information                Pointer to additional HTTP    */
/*                                            information                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_response_send     Send HTTP response            */
/*    _nx_utility_string_length_check       Check string length           */
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
UINT  _nx_web_http_server_callback_response_send(NX_WEB_HTTP_SERVER *server_ptr, CHAR *status_code, CHAR *information, CHAR *additional_info)
{

UINT status_code_length = 0;
UINT information_length = 0;
UINT additional_info_length = 0;
UINT status;


    /* Check length of header, information and additional information.  */
    if (_nx_utility_string_length_check(status_code, &status_code_length, NX_MAX_STRING_LENGTH) ||
        (information && _nx_utility_string_length_check(information, &information_length, NX_MAX_STRING_LENGTH)) ||
        (additional_info && _nx_utility_string_length_check(additional_info, &additional_info_length, NX_MAX_STRING_LENGTH)))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* Call the internal HTTP response send function.  */
    status = _nx_web_http_server_response_send(server_ptr, status_code, status_code_length, information,
                                               information_length, additional_info, additional_info_length);

    /* Return status.  */
    return(status);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_callback_response_send_extended                */
/*                                                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the response send call.          */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    status_code                           Status-code and reason-phrase */
/*    status_code_length                    Length of status-code         */
/*    information                           Pointer to HTTP info string   */
/*    information_length                    Length of information         */
/*    additional_information                Pointer to additional HTTP    */
/*                                            information                 */
/*    additional_information_length         Length of additional          */
/*                                            information                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxe_web_http_server_callback_response_send_extended                */
/*                                          Actual function call          */
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
UINT _nxe_web_http_server_callback_response_send_extended(NX_WEB_HTTP_SERVER *server_ptr, CHAR *status_code,
                                                          UINT status_code_length, CHAR *information,
                                                          UINT information_length, CHAR *additional_info,
                                                          UINT additional_information_length)
{
UINT status;

    if(server_ptr == NX_NULL || status_code == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Actual function call.  */
    status = _nx_web_http_server_callback_response_send_extended(server_ptr, status_code, status_code_length,
                                                                 information, information_length,
                                                                 additional_info, additional_information_length);

    /* Return successful completion.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_callback_response_send_extended PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends the completion response to the client from the  */
/*    application callback function. This is typically done to satisfy a  */
/*    GET or POST request that is processed completely by the application */
/*    callback function.                                                  */
/*                                                                        */
/*    Note: The strings of status_code, information and                   */
/*    additional_information must be NULL-terminated and length of each   */
/*    string matches the length specified in the argument list.           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    status_code                           Status-code and reason-phrase */
/*    status_code_length                    Length of status-code         */
/*    information                           Pointer to HTTP info string   */
/*    information_length                    Length of information         */
/*    additional_information                Pointer to additional HTTP    */
/*                                            information                 */
/*    additional_information_length         Length of additional          */
/*                                            information                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_response_send     Send HTTP response            */
/*    _nx_utility_string_length_check       Check string length           */
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
UINT _nx_web_http_server_callback_response_send_extended(NX_WEB_HTTP_SERVER *server_ptr, CHAR *status_code,
                                                         UINT status_code_length, CHAR *information,
                                                         UINT information_length, CHAR *additional_info,
                                                         UINT additional_info_length)
{
UINT temp_status_code_length = 0;
UINT temp_information_length = 0;
UINT temp_additional_info_length = 0;

    /* Check length of status_code, information and additional information.  */
    if (_nx_utility_string_length_check(status_code, &temp_status_code_length, NX_MAX_STRING_LENGTH) ||
        (information && _nx_utility_string_length_check(information, &temp_information_length, NX_MAX_STRING_LENGTH)) ||
        (additional_info && _nx_utility_string_length_check(additional_info, &temp_additional_info_length, NX_MAX_STRING_LENGTH)))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* Validate string length.  */
    if ((temp_status_code_length != status_code_length) ||
        (information && (temp_information_length != information_length)) ||
        (additional_info && (temp_additional_info_length != additional_info_length)))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* Call the internal HTTP response send function.  */
    return(_nx_web_http_server_response_send(server_ptr, status_code, status_code_length, information,
                                             information_length, additional_info, additional_info_length));
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_connection_end                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is an internal callback used by the TCP server when a */
/*    TCP connection is ready to close.                                   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tcpserver_ptr                         Pointer to TCP server         */
/*    session_ptr                           Pointer to TCP session        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_connection_disconnect                           */
/*                                          Disconnect TCP or TLS         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Duo                                                            */ 
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
VOID        _nx_web_http_server_connection_end(NX_TCPSERVER *tcpserver_ptr, NX_TCP_SESSION *session_ptr)
{
NX_WEB_HTTP_SERVER *server_ptr;


    /* Get the HTTP server pointer.  */
    server_ptr =  (NX_WEB_HTTP_SERVER *) tcpserver_ptr -> nx_tcpserver_reserved;

    /* Disconnect the TCP server (and end TLS if used). */
    _nx_web_http_server_connection_disconnect(server_ptr, session_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_DISCONNECT);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_connection_timeout              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is an internal callback used by the TCP server when a */
/*    TCP connection times out.                                           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tcpserver_ptr                         Pointer to TCP server         */
/*    session_ptr                           Pointer to TCP session        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_connection_disconnect                           */
/*                                          Disconnect TCP or TLS         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Duo                                                            */ 
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
VOID        _nx_web_http_server_connection_timeout(NX_TCPSERVER *tcpserver_ptr, NX_TCP_SESSION *session_ptr)
{
NX_WEB_HTTP_SERVER *server_ptr;


    /* Get the HTTP server pointer.  */
    server_ptr =  (NX_WEB_HTTP_SERVER *) tcpserver_ptr -> nx_tcpserver_reserved;

    /* Disconnect the TCP server (and end TLS if used). */
    _nx_web_http_server_connection_disconnect(server_ptr, session_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_DISCONNECT);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_receive_data                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is the callback function executed whenever a client   */
/*    connection appears on the HTTP Server port.  It is responsible for  */
/*    waking up the HTTP Server thread.                                   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to HTTP Server socket */
/*    port                                  HTTP Server port              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_get_client_request                              */
/*                                          Get request from client       */
/*    _nx_web_http_server_connection_disconnect                           */
/*                                          Disconnect connection         */
/*    _nx_web_http_server_get_client_keepalive                            */
/*                                          Get keepalive flag            */
/*    _nx_web_http_server_chunked_check     Check if the packet is chunked*/
/*    _nx_web_http_server_get_process       Process GET request           */
/*    _nx_web_http_server_put_process       Process PUT request           */
/*    _nx_web_http_server_delete_process    Process DELETE request        */
/*    _nx_web_http_server_response_packet_allocate                        */
/*                                          Allocate a response packet    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX                                                                */
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
VOID  _nx_web_http_server_receive_data(NX_TCPSERVER *tcpserver_ptr, NX_TCP_SESSION *session_ptr)
{

NX_WEB_HTTP_SERVER      *server_ptr;
NX_PACKET               *packet_ptr;
UCHAR                   *buffer_ptr;
UINT                    status;
NX_PACKET               *response_pkt;


    /* Set the HTTP server pointer.  */
    server_ptr =  (NX_WEB_HTTP_SERVER *) tcpserver_ptr -> nx_tcpserver_reserved;

    /* Store the current session. */
    server_ptr -> nx_web_http_server_current_session_ptr = session_ptr;

    /* Get the complete HTTP client request.  */
    status =  _nx_web_http_server_get_client_request(server_ptr, &packet_ptr);

    /* Check if the HTTP request is valid.  */
    if (status == NX_SUCCESS)
    {
        if (packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr < 7)
        {
            nx_packet_release(packet_ptr);
            status = NX_WEB_HTTP_ERROR;
        }
    }

    /* Determine if it was successful.  */
    if (status != NX_SUCCESS)
    {

        if(status != NX_NO_PACKET)
        {

            /* Increment the number of invalid HTTP requests.  */
            server_ptr -> nx_web_http_server_invalid_http_headers++;
        }

        /* Disconnect from the current connection.  */
        _nx_web_http_server_connection_disconnect(server_ptr, session_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_DISCONNECT);

        /* Return.  */
        return;
    }

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
    /* Check whether keepalive is necessary. */
    _nx_web_http_server_get_client_keepalive(server_ptr, packet_ptr);
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */

    /* Check if the packet is chunked.  */
    server_ptr -> nx_web_http_server_request_chunked = (UCHAR)_nx_web_http_server_chunked_check(packet_ptr);
    server_ptr -> nx_web_http_server_expect_receive_bytes = 0;
    server_ptr -> nx_web_http_server_actual_bytes_received = 0;

    /* Otherwise, we have received an HTTP client request successfully.  */

    /* Setup a pointer to packet buffer area.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Determine what type of request is present.  */

    /* Check for a GET request.  */
    if ((buffer_ptr[0] == 'G') && (buffer_ptr[1] == 'E') && (buffer_ptr[2] == 'T') && (buffer_ptr[3] == ' '))
    {

        /* We have a HTTP GET request to get a resource from this HTTP Server.  */
        server_ptr -> nx_web_http_server_request_type = NX_WEB_HTTP_SERVER_GET_REQUEST;

        /* Increment the number of GET requests.  */
        server_ptr -> nx_web_http_server_get_requests++;

        /* Process the GET request.  */
        _nx_web_http_server_get_process(server_ptr, NX_WEB_HTTP_SERVER_GET_REQUEST, packet_ptr);
    }

    /* Check for a PUT request.  */
    else if ((buffer_ptr[0] == 'P') && (buffer_ptr[1] == 'U') && (buffer_ptr[2] == 'T') && (buffer_ptr[3] == ' '))
    {

        /* We have a HTTP PUT request to store a resource on this HTTP Server.  */
        server_ptr -> nx_web_http_server_request_type = NX_WEB_HTTP_SERVER_PUT_REQUEST;

        /* Increment the number of PUT requests.  */
        server_ptr -> nx_web_http_server_put_requests++;

        /* Process the PUT request.  */
        _nx_web_http_server_put_process(server_ptr, packet_ptr);
    }

    /* Check for a DELETE request.  */
    else if ((buffer_ptr[0] == 'D') && (buffer_ptr[1] == 'E') && (buffer_ptr[2] == 'L') && (buffer_ptr[3] == 'E') && 
             (buffer_ptr[4] == 'T') && (buffer_ptr[5] == 'E') && (buffer_ptr[6] == ' '))
    {

        /* We have a HTTP DELETE request to delete a resource from this HTTP Server.  */
        server_ptr -> nx_web_http_server_request_type = NX_WEB_HTTP_SERVER_DELETE_REQUEST;

        /* Increment the number of DELETE requests.  */
        server_ptr -> nx_web_http_server_delete_requests++;

        /* Process the Delete request.  */
        _nx_web_http_server_delete_process(server_ptr, packet_ptr);
    }

    /* Check for a POST request.  */
    else if ((buffer_ptr[0] == 'P') && (buffer_ptr[1] == 'O') && (buffer_ptr[2] == 'S') && 
             (buffer_ptr[3] == 'T') && (buffer_ptr[4] == ' '))
    {

        /* We have a HTTP POST request to send data to this HTTP Server for processing.  Note that the POST
           request is nearly identical to the GET request, except the parameter/query data is found in the
           content rather than as part of the URL (resource). */
        server_ptr -> nx_web_http_server_request_type = NX_WEB_HTTP_SERVER_POST_REQUEST;

        /* Increment the number of POST requests.  */
        server_ptr -> nx_web_http_server_post_requests++;

#ifdef NX_WEB_HTTP_MULTIPART_ENABLE

        /* Reset last received multipart packet. */
        server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_last_packet = NX_NULL;
#endif /* NX_WEB_HTTP_MULTIPART_ENABLE */

        /* Process the POST request.  */
        _nx_web_http_server_get_process(server_ptr, NX_WEB_HTTP_SERVER_POST_REQUEST, packet_ptr);

#ifdef NX_WEB_HTTP_MULTIPART_ENABLE
        /* Restore the packet to release. */
        if (server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_last_packet)
            packet_ptr = server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_last_packet;
#endif /* NX_WEB_HTTP_MULTIPART_ENABLE */
    }

    /* Check for a HEAD request.  */
    else if ((buffer_ptr[0] == 'H') && (buffer_ptr[1] == 'E') && (buffer_ptr[2] == 'A') && 
             (buffer_ptr[3] == 'D') && (buffer_ptr[4] == ' '))
    {

        /* We have a HTTP HEAD request to get a resource header from this HTTP Server.  Note that the HEAD
           request is nearly identical to the GET request, except the requested content is not returned to
           the client.  */
        server_ptr -> nx_web_http_server_request_type = NX_WEB_HTTP_SERVER_HEAD_REQUEST;

        /* Increment the number of HEAD requests.  */
        server_ptr -> nx_web_http_server_head_requests++;

        /* Process the HEAD request.  */
        _nx_web_http_server_get_process(server_ptr, NX_WEB_HTTP_SERVER_HEAD_REQUEST, packet_ptr);
    }

    /* Unhandled request.  */
    else
    {

        /* We have an unhandled HTTP request.  */
        server_ptr -> nx_web_http_server_request_type = NX_WEB_HTTP_SERVER_UNKNOWN_REQUEST;

        /* Send response back to HTTP Client.  */
        _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_NOT_IMPLEMENTED,
                                          sizeof(NX_WEB_HTTP_STATUS_NOT_IMPLEMENTED) - 1,
                                          "NetX HTTP Request Not Implemented",
                                          sizeof("NetX HTTP Request Not Implemented") - 1, NX_NULL, 0);

        /* Increment the number of unhandled requests.  */
        server_ptr -> nx_web_http_server_unknown_requests++;
    }

    /* Release the packet.  */
    nx_packet_release(packet_ptr);

    /* Release the unprocessed packet.  */
    if (server_ptr -> nx_web_http_server_request_packet)
    {
        nx_packet_release(server_ptr -> nx_web_http_server_request_packet);
        server_ptr -> nx_web_http_server_request_packet = NX_NULL;
    }

    /* If there are no more data to send, append the last chunk.  */
    if (server_ptr -> nx_web_http_server_response_chunked)
    {

        /* Allocate a packet to send the chunk end.  */
        status = _nx_web_http_server_response_packet_allocate(server_ptr, &response_pkt, NX_WAIT_FOREVER);
        if (status)
        {

            /* Disconnect from the current connection.  */
            _nx_web_http_server_connection_disconnect(server_ptr, session_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_DISCONNECT);
            return;
        }

        nx_packet_data_append(response_pkt, "0\r\n\r\n", 5, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Send internal. */
        status = _nx_web_http_server_send(server_ptr, response_pkt, NX_WEB_HTTP_SERVER_TIMEOUT_SEND);
        if (status)
        {

            /* Disconnect from the current connection.  */
            _nx_web_http_server_connection_disconnect(server_ptr, session_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_DISCONNECT);

            nx_packet_release(response_pkt);
            return;
        }

        server_ptr -> nx_web_http_server_response_chunked = NX_FALSE;
    }

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
    if(server_ptr -> nx_web_http_server_keepalive == NX_FALSE)
    {
#endif
        /* Disconnect from the current connection.  */
        _nx_web_http_server_connection_disconnect(server_ptr, session_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_DISCONNECT);

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
    }
#endif

    return;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_get_client_request              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves the complete HTTP client request in a single*/ 
/*    packet.  Doing this makes the other parsing and searching           */
/*    routines simple.                                                    */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    packet_ptr                            Destination for request       */
/*                                            packet pointer              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_copy                        Copy packet                   */
/*    nx_packet_data_append                 Move data into packet         */
/*    nx_packet_release                     Release packet                */
/*    nx_tcp_socket_receive                 Receive an HTTP request packet*/
/*                                                                        */
/*  CALLED BY                                                             */
/*    _nx_web_http_server_receive_data                                    */
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
UINT  _nx_web_http_server_get_client_request(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr)
{

NX_PACKET   *head_packet_ptr;
NX_PACKET   *new_packet_ptr;
CHAR        *buffer_ptr;
UINT        status;
NX_PACKET   *work_ptr;
UINT        crlf_found = 0;
NX_PACKET   *tmp_ptr;


    /* Default the return packet pointer to NULL.  */
    *packet_ptr =  NX_NULL;

    /* Wait for a request on the HTTP TCP well known port 80, or if encryption is enabled 443.  */
    status = _nx_web_http_server_receive(server_ptr, &head_packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_RECEIVE);

    /* At this point, the server has a connection with an HTTP Client  */

    /* Check the return status.  */
    if (status != NX_SUCCESS)
    {

        /* Return an error condition.  */
        return(status);
    }

    /* Setup pointer to start of buffer.  */
    buffer_ptr =  (CHAR *) head_packet_ptr -> nx_packet_prepend_ptr;

    /* Determine if the packet is an HTTP request.  */
    if ((buffer_ptr[0] != 'G') && (buffer_ptr[0] != 'g') && (buffer_ptr[0] != 'P') && (buffer_ptr[0] != 'p') &&
        (buffer_ptr[0] != 'D') && (buffer_ptr[0] != 'd') && (buffer_ptr[0] != 'H') && (buffer_ptr[0] != 'h') &&
        (buffer_ptr[0] != 'T') && (buffer_ptr[0] != 't'))
    {

        /* Invalid first packet for HTTP request.  */

        /* Release the packet.  */
        nx_packet_release(head_packet_ptr);

        /* Return an error.  */
        return(NX_WEB_HTTP_ERROR);
    }

    crlf_found = 0;
    work_ptr = head_packet_ptr;
    
    /* Build a pointer to the buffer area.  */
    buffer_ptr =  (CHAR *) work_ptr -> nx_packet_prepend_ptr; 
    
    do
    {
    
        /* See if there is a blank line present in the buffer.  */
        /* Search the buffer for a cr/lf pair.  */
        while (buffer_ptr < (CHAR *) work_ptr -> nx_packet_append_ptr)
        {
            if (!(crlf_found & 1) && (*buffer_ptr == (CHAR)13))
            {

                /* Found CR. */
                crlf_found++;
            }
            else if((crlf_found & 1) && (*buffer_ptr == (CHAR)10))
            {

                /* Found LF. */
                crlf_found++;
            }
            else
            {

                /* Reset the CRLF marker. */
                crlf_found = 0;
            }

            if (crlf_found == 4)
            {
    
                /* Yes, we have found the end of the HTTP request header.  */
    
                /* Set the return packet pointer.  */
                *packet_ptr =  head_packet_ptr;
    
                /* Return a successful completion.  */
                return(NX_SUCCESS);
            }
    
            /* Move the buffer pointer up.  */
            buffer_ptr++;
        }

        /* Determine if the packet has already overflowed into another packet.  */

#ifndef NX_DISABLE_PACKET_CHAIN

        if (work_ptr -> nx_packet_next != NX_NULL)
        {
            /* Get the next packet in the chain. */
            work_ptr  = work_ptr -> nx_packet_next;
            buffer_ptr =  (CHAR *) work_ptr -> nx_packet_prepend_ptr; 
        }
        else
           
#endif         
        {
            /* Receive another packet from the HTTP server port.  */            
            status = _nx_web_http_server_receive(server_ptr, &new_packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_RECEIVE);
            
            /* Check the return status.  */
            if (status != NX_SUCCESS)
            {
    
                /* Release the current head packet.  */
                nx_packet_release(head_packet_ptr);
            
                /* Return an error condition.  */
                return(status);
            }      
    
            /* Successfully received another packet.  Its contents now need to be placed in the head packet.  */
            tmp_ptr = new_packet_ptr;
#ifndef NX_DISABLE_PACKET_CHAIN
            while (tmp_ptr)
            {
#endif /* NX_DISABLE_PACKET_CHAIN */
    
                /* Copy the contents of the current packet into the head packet.  */
                status =  nx_packet_data_append(head_packet_ptr, (VOID *) tmp_ptr -> nx_packet_prepend_ptr,
                                                (ULONG)(tmp_ptr -> nx_packet_append_ptr - tmp_ptr -> nx_packet_prepend_ptr),
                                                server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WEB_HTTP_SERVER_TIMEOUT);

                /* Determine if an error occurred.  */
                if (status != NX_SUCCESS)
                {
        
                    /* Yes, an error is present.  */
        
                    /* Release both packets.  */
                    nx_packet_release(head_packet_ptr);
                    nx_packet_release(new_packet_ptr);
        
                    /* Return an error condition.  */
                    return(status);
                }

#ifndef NX_DISABLE_PACKET_CHAIN
                tmp_ptr = tmp_ptr -> nx_packet_next;
            }
#endif /* NX_DISABLE_PACKET_CHAIN */

            /* Release the new packet. */
            nx_packet_release(new_packet_ptr);
        }

    } while (status == NX_SUCCESS);

    /* Release the packet.  */
    nx_packet_release(head_packet_ptr);

    return NX_WEB_HTTP_ERROR;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_get_process                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the GET, POST, and HEAD HTTP client         */
/*    requests.                                                           */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    request_type                          Type of request (GET, POST,   */
/*                                            or HEAD)                    */
/*    packet_ptr                            Request packet pointer        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_basic_authenticate                              */
/*                                          Process basic authentication  */
/*    _nx_web_http_server_digest_authenticate                             */
/*                                          Process digest authentication */
/*    _nx_web_http_server_calculate_content_offset                        */
/*                                          Retrieve content offset       */
/*    _nx_web_http_server_content_length_get                              */
/*                                          Retrieve content length       */
/*    _nx_web_http_server_response_send     Send response back to client  */
/*    _nx_web_http_server_retrieve_resource Retrieve resource from request*/
/*    _nx_web_http_server_type_get_extended Derive file type              */
/*    _nx_web_http_server_packet_get        Receive another packet        */
/*    _nx_web_http_server_field_value_get   Get field value               */
/*    nx_tcp_socket_transmit_configure      Configure the server socket   */
/*    fx_directory_information_get          Get length of resource file   */
/*    fx_file_close                         Close resource file           */
/*    fx_file_open                          Open resource file            */
/*    fx_file_read                          Read data from resource file  */
/*    _nx_web_http_server_reponse_packet_allocate                         */
/*                                          Allocate a response packet    */
/*    nx_packet_release                     Release packet                */
/*    _nx_utility_string_length_check       Check string length           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_receive_data      HTTP receive processing       */
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
VOID  _nx_web_http_server_get_process(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, NX_PACKET *packet_ptr)
{

ULONG       length = 0;
UINT        file_type_length;
UINT        status;
NX_PACKET   *new_packet_ptr;
CHAR        *name_ptr;
CHAR        *password_ptr;
CHAR        *realm_ptr;
ULONG       temp;
CHAR        temp_string[30];
UINT        auth_request_present = NX_FALSE;
NX_TCP_SOCKET *socket_ptr = &server_ptr -> nx_web_http_server_current_session_ptr -> nx_tcp_session_socket;
UINT        offset = 0;
UINT        resource_length;
UINT        name_length = 0;
UINT        password_length = 0;
UINT        realm_length = 0;
UINT        temp_name_length = 0;
UINT        temp_password_length = 0;
UINT        temp_realm_length = 0;


    /* Pickup the URL (resource) from the request.  */
    status =  _nx_web_http_server_retrieve_resource(server_ptr, packet_ptr, server_ptr -> nx_web_http_server_request_resource, NX_WEB_HTTP_MAX_RESOURCE + 1);

    /* Determine if the resource was extracted successfully.  */
    if (status != NX_SUCCESS)
    {

        /* Send response back to HTTP Client.  */
        _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_BAD_REQUEST,
                                          sizeof(NX_WEB_HTTP_STATUS_BAD_REQUEST) - 1,
                                          "NetX HTTP URL Bad", sizeof("NetX HTTP URL Bad") - 1, NX_NULL, 0);

        /* Error, return to caller.  */
        return;
    }

    /* Determine if a POST is present.  */
    if (request_type == NX_WEB_HTTP_SERVER_POST_REQUEST)
    {

        if (!server_ptr -> nx_web_http_server_request_chunked)
        {

            /* It is. Check for a valid content-length field. */
            status = _nx_web_http_server_content_length_get(packet_ptr, &length);

            /* Check for errors.  */
            if ( status != NX_SUCCESS)
            {

                /* Send response back to HTTP Client.  */
                _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_BAD_REQUEST,
                                                  sizeof(NX_WEB_HTTP_STATUS_BAD_REQUEST) - 1,
                                                  "NetX HTTP Invalid Content Length",
                                                  sizeof("NetX HTTP Invalid Content Length") - 1, NX_NULL, 0);

                /* Error, return to caller.  */
                return;
            }
        }

        /* Check for an invalid content offset .  */
        offset = _nx_web_http_server_calculate_content_offset(packet_ptr);
        if (offset == 0)
        {

            /* Send response back to HTTP Client.  */
            _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_BAD_REQUEST,
                                              sizeof(NX_WEB_HTTP_STATUS_BAD_REQUEST) - 1,
                                              "NetX HTTP Invalid Content Offset", 
                                              sizeof("NetX HTTP Invalid Content Offset") - 1, NX_NULL, 0);

            /* Error, return to caller.  */
            return;
        }

#ifdef  NX_WEB_HTTP_MULTIPART_ENABLE
        /* Cleanup the multipart field. */
        memset(&server_ptr -> nx_web_http_server_multipart, 0, sizeof(NX_WEB_HTTP_SERVER_MULTIPART));
#endif /* NX_WEB_HTTP_MULTIPART_ENABLE */
    }

    /* Determine if the application has specified an authentication function for this server.  */
    if (server_ptr -> nx_web_http_server_authentication_check ||
        server_ptr -> nx_web_http_server_authentication_check_extended)
    {

        /* Determine if authentication is required for the specified resource.  */
        if (server_ptr -> nx_web_http_server_authentication_check_extended)
        {
            status =  (server_ptr -> nx_web_http_server_authentication_check_extended)(server_ptr, request_type, server_ptr -> nx_web_http_server_request_resource,
                                                                                       &name_ptr, &name_length, &password_ptr, &password_length, &realm_ptr, &realm_length);
        }
        else
        {
            status =  (server_ptr -> nx_web_http_server_authentication_check)(server_ptr, request_type, server_ptr -> nx_web_http_server_request_resource,
                                                                            &name_ptr, &password_ptr, &realm_ptr);
        }

        if ((status == NX_WEB_HTTP_BASIC_AUTHENTICATE) ||
            (status == NX_WEB_HTTP_DIGEST_AUTHENTICATE))
        {

            /* Check name, password and realm string.  */
            if (_nx_utility_string_length_check(name_ptr, &temp_name_length, NX_WEB_HTTP_MAX_NAME) ||
                _nx_utility_string_length_check(password_ptr, &temp_password_length, NX_WEB_HTTP_MAX_PASSWORD) ||
                _nx_utility_string_length_check(realm_ptr, &temp_realm_length, NX_MAX_STRING_LENGTH))
            {

                /* Error, return to caller.  */
                return;
            }

            /* Validate string length. */
            if (server_ptr -> nx_web_http_server_authentication_check_extended &&
                ((realm_length != temp_realm_length) ||
                 (name_length != temp_name_length) ||
                 (password_length != temp_password_length)))
            {
                return;
            }
        }

        /* Determine what kind - if any - authentication is requested for this resource.  */
        if (status == NX_WEB_HTTP_BASIC_AUTHENTICATE)
        {

            /* Process basic authentication request.  */
            status =  _nx_web_http_server_basic_authenticate(server_ptr, packet_ptr, name_ptr, password_ptr, realm_ptr, &auth_request_present);
        }
#ifdef  NX_WEB_HTTP_DIGEST_ENABLE
        else if (status == NX_WEB_HTTP_DIGEST_AUTHENTICATE)
        {

            /* Process digest authentication request.  */
            status =  _nx_web_http_server_digest_authenticate(server_ptr, packet_ptr, name_ptr, password_ptr, realm_ptr, &auth_request_present);
        }
#endif

        /* Determine if the authentication failed.  */
        if ((status != NX_WEB_HTTP_DONT_AUTHENTICATE) && (status != NX_SUCCESS))
        {

            /*  Yes it did. Inform the HTTP server application about the failed authentication. */
            if (server_ptr -> nx_web_http_server_invalid_username_password_callback && auth_request_present)
            {
            NXD_ADDRESS client_nxd_address;
            ULONG       client_port;

                /* Get the IP address of the client:  */
                status =   nxd_tcp_socket_peer_info_get(socket_ptr, &client_nxd_address , &client_port);

                if (status == NX_SUCCESS)
                {
                    /* Send this information to the host application. */
                    (server_ptr -> nx_web_http_server_invalid_username_password_callback)(server_ptr -> nx_web_http_server_request_resource, &client_nxd_address, request_type);
                }
            }

            return;
        }
    }

    /* Check whether a full response is necessary. */
    if((server_ptr -> nx_web_http_server_cache_info_get) &&
       (server_ptr -> nx_web_http_server_request_type == NX_WEB_HTTP_SERVER_GET_REQUEST))
    {

        /* Searching for "If-Modified-Since" header. */
        if(_nx_web_http_server_field_value_get(packet_ptr, (UCHAR *)"if-modified-since", 17, (UCHAR *)temp_string, sizeof(temp_string)) == NX_SUCCESS)
        {
        UINT max_age;
        NX_WEB_HTTP_SERVER_DATE date;
        CHAR date_string[30];

            /* Get last modified date of this resource. */
            if(server_ptr -> nx_web_http_server_cache_info_get(server_ptr -> nx_web_http_server_request_resource, &max_age, &date) == NX_TRUE)
            {

                /* Convert date to string, the return length is always 29. */
                temp = _nx_web_http_server_date_to_string(&date, date_string);
                date_string[temp] = 0;

                /* Check the last modified date with if-modified-since. */
                if(memcmp(temp_string, date_string, temp + 1) == 0)
                {

                    /* Send not modified.  */
                    _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_NOT_MODIFIED,
                                                      sizeof(NX_WEB_HTTP_STATUS_NOT_MODIFIED) - 1,
                                                      NX_NULL, 0, NX_NULL, 0);

                    /* Return to caller.  */
                    return;
                }
            }
        }
    }

    /* Setup the server socket with a specific packet transmit retry logic.  */
    nx_tcp_socket_transmit_configure(socket_ptr, 
                                    NX_WEB_HTTP_SERVER_TRANSMIT_QUEUE_DEPTH,
                                    NX_WEB_HTTP_SERVER_RETRY_SECONDS*NX_IP_PERIODIC_RATE,
                                    NX_WEB_HTTP_SERVER_RETRY_MAX, 
                                    NX_WEB_HTTP_SERVER_RETRY_SHIFT);

    /* At this point, either there isn't any required authentication for this resource or the authentication is
       complete.  */

    /* If the HTTP server receives an empty POST request (no message body, content length = 0, it is the
       responsibility of the request notify callback to send a response (if any) to the HTTP Client. 
       The HTTP server will process the request no further. */

    /* Determine if a user supplied get function has been specified.  */
    if (server_ptr -> nx_web_http_server_request_notify)
    {

        /* Call the user supplied function to notify the user of the get request.  */
        status =  (server_ptr -> nx_web_http_server_request_notify)(server_ptr, request_type, server_ptr -> nx_web_http_server_request_resource, packet_ptr);

#ifdef  NX_WEB_HTTP_MULTIPART_ENABLE
        /* Release the packet that is not processed by callback function. */
        if(server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_next_packet)
            nx_packet_release(server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_next_packet);
#endif /* NX_WEB_HTTP_MULTIPART_ENABLE */

        /* Determine if the user supplied routine is requesting the get should be aborted.  */
        if (status != NX_SUCCESS)
        {

            /* Determine if the user callback routine successfully completed the request processing.  */
            if (status == NX_WEB_HTTP_CALLBACK_COMPLETED)
            {

                /* User callback routine already sent success response back to HTTP Client.  */
                return;
            }

            /* Send response back to HTTP Client.  */
            _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR,
                                              sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1,
                                              "NetX HTTP Request Aborted",
                                              sizeof("NetX HTTP Request Aborted") - 1, NX_NULL, 0);

            /* Yes, error was detected. Abort the remainder of the get processing.  */
            return;
        }
    }

    /* If it's POST request, process the message body.  */
    if (request_type == NX_WEB_HTTP_SERVER_POST_REQUEST)
    {

        /* Was there a message body in the request? */
        if (server_ptr -> nx_web_http_server_request_chunked)
        {

            /* Find the first chunk of the content.  */
            status = _nx_web_http_server_packet_content_find(server_ptr, &packet_ptr, NX_NULL);
            if (status)
            {

                /* No. Regardless if a reply was sent via the request notify callback, we are done with this packet. */
                return;
            }

            nx_packet_release(packet_ptr);
        }
        else
        {
            if (length == 0)
            {

                /* No. Regardless if a reply was sent via the request notify callback, we are done with this packet. */
                return;
            }

            length -= (ULONG)(packet_ptr -> nx_packet_length - offset);
        }

        /* If necessary, receive more packets from the TCP socket.  */
        while (length || server_ptr -> nx_web_http_server_request_chunked)
        {

            /* Wait for more packets.  */
            status = _nx_web_http_server_packet_get(server_ptr, &new_packet_ptr);

            /* Check the return status.  */
            if (status != NX_SUCCESS)
            {

                if (status == NX_WEB_HTTP_GET_DONE)
                {
                    break;
                }

                /* Send response back to HTTP Client.  */
                _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR,
                                                  sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1,
                                                  "NetX HTTP Receive Timeout",
                                                  sizeof("NetX HTTP Receive Timeout") - 1, NX_NULL, 0);

                /* Error, return to caller.  */
                return;
            }

            /* Update the length.  */
            if (!server_ptr -> nx_web_http_server_request_chunked)
            {
                length -= new_packet_ptr -> nx_packet_length;
            }

            nx_packet_release(new_packet_ptr);
        }
    }


    /* Get the length of request resource.  */
    if (_nx_utility_string_length_check(server_ptr -> nx_web_http_server_request_resource,  &resource_length, 
                                        sizeof(server_ptr -> nx_web_http_server_request_resource) - 1))
    {
        return;
    }

    /* Open the specified file for reading.  */
    status =  fx_file_open(server_ptr -> nx_web_http_server_media_ptr, &(server_ptr -> nx_web_http_server_file), server_ptr -> nx_web_http_server_request_resource, FX_OPEN_FOR_READ);

    /* Check for error condition.  */
    if (status != NX_SUCCESS)
    {

        /* Send response back to HTTP Client.  */
        _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_NOT_FOUND,
                                          sizeof(NX_WEB_HTTP_STATUS_NOT_FOUND) - 1,
                                          "NetX HTTP Server unable to find file: ",
                                          sizeof("NetX HTTP Server unable to find file: ") - 1,
                                          server_ptr -> nx_web_http_server_request_resource,
                                          resource_length);

        /* Error, return to caller.  */
        return;
    }

    /* Calculate the size of the file.  */
    length =  0;
    fx_directory_information_get(server_ptr -> nx_web_http_server_media_ptr, server_ptr -> nx_web_http_server_request_resource, FX_NULL,
                            &length, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL, FX_NULL);

    /* Derive the file type. We use whatever value is returned since if there is no
       match this function will return a default value.  */
    _nx_web_http_server_type_get_extended(server_ptr, server_ptr -> nx_web_http_server_request_resource, resource_length, temp_string, sizeof(temp_string), &file_type_length);

    temp = file_type_length;
    temp_string[temp] = 0;

    /* Now build a response header.  */
    status = _nx_web_http_server_generate_response_header(server_ptr, &new_packet_ptr, NX_WEB_HTTP_STATUS_OK,
                                                          sizeof(NX_WEB_HTTP_STATUS_OK) - 1, length, temp_string,
                                                          file_type_length, NX_NULL, 0);
    if(status)
    {

        /* Send response back to HTTP Client.  */
        _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR,
                                          sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1,
                                          "NetX HTTP Request Aborted",
                                          sizeof("NetX HTTP Request Aborted") - 1, NX_NULL, 0);

        /* Close the file.  */
        fx_file_close(&(server_ptr -> nx_web_http_server_file));

        /* Error, return to caller.  */
        return;
    }

    /* Check to see if only a response is required.  */
    if ((!length) || (request_type == NX_WEB_HTTP_SERVER_HEAD_REQUEST))
    {

        /* Yes, only a response is required... send it!  */
        status = _nx_web_http_server_send(server_ptr, new_packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_SEND);
        
        /* Determine if this is successful.  */
        if (status != NX_SUCCESS)
        {
    
            /* Release the packet.  */
            nx_packet_release(new_packet_ptr);
            
            /* Force zero length.  */
            length = 0;
        }
    }
    
    /* Get length of packet */
    temp = new_packet_ptr -> nx_packet_length;

    /* Now, loop to send the data contents.  If the request type is "HEAD", skip this loop to output the 
       file contents.  */
    while ((length) && (request_type != NX_WEB_HTTP_SERVER_HEAD_REQUEST))
    {

        /* Determine if we need to allocate a new packet for data.  */
        if (!temp)
        {

            /* Yes, allocate a new packet.  */
            status =  _nx_web_http_server_response_packet_allocate(server_ptr, &new_packet_ptr, NX_WAIT_FOREVER);

            /* Determine if an error is present.  */
            if (status != NX_SUCCESS)
            {

                /* Indicate an allocation error occurred.  */
                server_ptr -> nx_web_http_server_allocation_errors++;

                /* Error, return to caller.  */
                break;
            }
        }

        /* Calculate the maximum length.  */
        temp =  ((ULONG) (new_packet_ptr -> nx_packet_data_end - new_packet_ptr -> nx_packet_append_ptr)) - NX_PHYSICAL_TRAILER;

        /* Determine if this exceeds the MSS of the peer.  */
        if (temp > socket_ptr -> nx_tcp_socket_connect_mss)
        {

            /* Yes, reduce the maximum size to the mss size.  */
            temp =  socket_ptr -> nx_tcp_socket_connect_mss;
        }

        /* Read data from the file.  */
        status =  fx_file_read(&(server_ptr -> nx_web_http_server_file), new_packet_ptr -> nx_packet_append_ptr,
                                        temp, &temp);
        
        /* Check for an error.  */
        if (status != NX_SUCCESS)
        {

            /* Release the packet.  */
            nx_packet_release(new_packet_ptr);

            /* Error, return.  */
            break;
        }

        /* Update the packet information with the data read.  */
        new_packet_ptr -> nx_packet_length =  new_packet_ptr -> nx_packet_length + temp;
        new_packet_ptr -> nx_packet_append_ptr =  new_packet_ptr -> nx_packet_append_ptr + temp;

        /* Send the packet out.  */
        status =  _nx_web_http_server_send(server_ptr, new_packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_SEND);

        /* Check for success.  */
        if (status != NX_SUCCESS)
        {

            /* Release the packet.  */
            nx_packet_release(new_packet_ptr);

            /* Return to caller.  */
            break;
        }

        /* Increment the bytes sent count.  */
        server_ptr -> nx_web_http_server_total_bytes_sent =  server_ptr -> nx_web_http_server_total_bytes_sent + temp;

        /* Adjust the file length based on what we have sent.  */
        length =  length - temp;
        
        /* Indicate new packet needed */
        temp = 0;
    }

    /* Close the file.  */
    fx_file_close(&(server_ptr -> nx_web_http_server_file));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_put_process                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the PUT HTTP client requests.               */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    packet_ptr                            Request packet pointer        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_basic_authenticate                              */
/*                                          Process basic authentication  */
/*    _nx_web_http_server_digest_authenticate                             */
/*                                          Process digest authentication */
/*    _nx_web_http_server_calculate_content_offset                        */
/*                                          Retrieve content offset       */
/*    _nx_web_http_server_content_length_get                              */
/*                                          Retrieve content length       */
/*    _nx_web_http_server_response_send     Send response back to client  */
/*    _nx_web_http_server_retrieve_resource Retrieve resource from request*/
/*    _nx_web_http_server_packet_get        Receive another packet        */
/*    fx_file_close                         Close resource file           */
/*    fx_file_create                        Create resource file          */
/*    fx_file_open                          Open resource file            */
/*    fx_file_write                         Write data to resource file   */
/*    nx_packet_release                     Release packet                */
/*    _nx_utility_string_length_check       Check string length           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_content_get       Content get processing        */
/*    _nx_web_http_server_get_process       GET request processing        */
/*    _nx_web_http_server_put_process       PUT request processing        */
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
VOID  _nx_web_http_server_put_process(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{

UINT        status;
ULONG       length = 0;
UINT        offset;
CHAR        *name_ptr;
CHAR        *password_ptr;
CHAR        *realm_ptr;
NX_PACKET   *data_packet_ptr;
NX_PACKET   *next_packet_ptr;
UINT        auth_request_present = NX_FALSE;
NX_TCP_SOCKET *socket_ptr = &server_ptr -> nx_web_http_server_current_session_ptr -> nx_tcp_session_socket;
UINT        name_length = 0;
UINT        password_length = 0;
UINT        realm_length = 0;
UINT        temp_name_length = 0;
UINT        temp_password_length = 0;
UINT        temp_realm_length = 0;


    /* Pickup the URL (resource) from the request.  */
    status =  _nx_web_http_server_retrieve_resource(server_ptr, packet_ptr, server_ptr -> nx_web_http_server_request_resource, NX_WEB_HTTP_MAX_RESOURCE + 1);

    /* Determine if the resource was extracted successfully.  */
    if (status != NX_SUCCESS)
    {

        /* Send response back to HTTP Client.  */
        _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_BAD_REQUEST,
                                          sizeof(NX_WEB_HTTP_STATUS_BAD_REQUEST) - 1,
                                          "NetX HTTP URL Bad",
                                          sizeof("NetX HTTP URL Bad") - 1, NX_NULL, 0);

        /* Error, return to caller.  */
        return;
    }

    if (!server_ptr -> nx_web_http_server_request_chunked)
    {

        /* Calculate the content length from the request.  */
        status =  _nx_web_http_server_content_length_get(packet_ptr, &length);

        /* Determine if the content length is valid.  */
        if (status != NX_SUCCESS)
        {

            /* Send response back to HTTP Client.  */
            _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_BAD_REQUEST,
                                              sizeof(NX_WEB_HTTP_STATUS_BAD_REQUEST) - 1,
                                              "NetX HTTP Invalid Content Length",
                                              sizeof("NetX HTTP Invalid Content Length") - 1, NX_NULL, 0);

            /* Error, return to caller.  */
            return;
        }
    }

    /* Calculate the offset to the content (the two Cr LF tokens) in the request.  */
    offset =  _nx_web_http_server_calculate_content_offset(packet_ptr);

    /* Determine if the offset to the content is valid.  */
    if (offset == 0)
    {

        /* Send response back to HTTP Client.  */
        _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_BAD_REQUEST,
                                          sizeof(NX_WEB_HTTP_STATUS_BAD_REQUEST) - 1,
                                          "NetX HTTP Invalid Content Offset",
                                          sizeof("NetX HTTP Invalid Content Offset") - 1, NX_NULL, 0);

        /* Error, return to caller.  */
        return;
    }

    /* Determine if the application has specified an authentication function for this server.  */
    if (server_ptr -> nx_web_http_server_authentication_check ||
        server_ptr -> nx_web_http_server_authentication_check_extended)
    {

        /* Determine if authentication is required for the specified resource.  */
        if (server_ptr -> nx_web_http_server_authentication_check_extended)
        {
            status =  (server_ptr -> nx_web_http_server_authentication_check_extended)(server_ptr, NX_WEB_HTTP_SERVER_PUT_REQUEST, server_ptr -> nx_web_http_server_request_resource,
                                                                                       &name_ptr, &name_length, &password_ptr, &password_length, &realm_ptr, &realm_length);
        }
        else
        {
            status =  (server_ptr -> nx_web_http_server_authentication_check)(server_ptr, NX_WEB_HTTP_SERVER_PUT_REQUEST, server_ptr -> nx_web_http_server_request_resource,
                                                                            &name_ptr, &password_ptr, &realm_ptr);
        }

        if ((status == NX_WEB_HTTP_BASIC_AUTHENTICATE) ||
            (status == NX_WEB_HTTP_DIGEST_AUTHENTICATE))
        {

            /* Check name, password and realm string.  */
            if (_nx_utility_string_length_check(name_ptr, &temp_name_length, NX_WEB_HTTP_MAX_NAME) ||
                _nx_utility_string_length_check(password_ptr, &temp_password_length, NX_WEB_HTTP_MAX_PASSWORD) ||
                _nx_utility_string_length_check(realm_ptr, &temp_realm_length, NX_MAX_STRING_LENGTH))
            {

                /* Error, return to caller.  */
                return;
            }

            /* Validate string length. */
            if (server_ptr -> nx_web_http_server_authentication_check_extended &&
                ((realm_length != temp_realm_length) ||
                 (name_length != temp_name_length) ||
                 (password_length != temp_password_length)))
            {
                return;
            }
        }

        /* Determine what kind - if any - authentication is requested for this resource.  */
        if (status == NX_WEB_HTTP_BASIC_AUTHENTICATE)
        {

            /* Process basic authentication request.  */
            status =  _nx_web_http_server_basic_authenticate(server_ptr, packet_ptr, name_ptr, password_ptr, realm_ptr, &auth_request_present);
        }
#ifdef  NX_WEB_HTTP_DIGEST_ENABLE
        else if (status == NX_WEB_HTTP_DIGEST_AUTHENTICATE)
        {

            /* Process digest authentication request.  */
            status =  _nx_web_http_server_digest_authenticate(server_ptr, packet_ptr, name_ptr, password_ptr, realm_ptr, &auth_request_present);
        }
#endif

        /* Determine if the authentication is currently in progress.  */
        if ((status != NX_WEB_HTTP_DONT_AUTHENTICATE) && (status != NX_SUCCESS))
        {

            /* Yes, authentication is in progress.  The HTTP 401 has already been sent.  */
            if (server_ptr -> nx_web_http_server_invalid_username_password_callback && auth_request_present)
            {
            NXD_ADDRESS client_nxd_address;
            ULONG       client_port;

                /* Get the IP address of the client:  */
                status =   nxd_tcp_socket_peer_info_get(socket_ptr, &client_nxd_address , &client_port);

                if (status == NX_SUCCESS)
                {
                    /* Send this information to the host application. */
                    (server_ptr -> nx_web_http_server_invalid_username_password_callback)(server_ptr -> nx_web_http_server_request_resource, &client_nxd_address, NX_WEB_HTTP_SERVER_PUT_REQUEST);
                }
            }

            return;
        }
    }

    /* At this point, either there isn't any required authentication for this resource or the authentication is
       complete.  */

    /* If the request is empty (content-length = 0) the request notify callback is where a response is generated 
       to return to the Client.  NetX HTTP Server will not process an empty request further. */

    /* Determine if a user supplied get function has been specified.  */
    if (server_ptr -> nx_web_http_server_request_notify)
    {

        /* Call the user supplied function to notify the user of the put request.  */
        status =  (server_ptr -> nx_web_http_server_request_notify)(server_ptr, NX_WEB_HTTP_SERVER_PUT_REQUEST, server_ptr -> nx_web_http_server_request_resource, packet_ptr);

        /* Determine if the user supplied routine is requesting the put should be aborted.  */
        if (status != NX_SUCCESS)
        {

            /* Determine if the user callback routine successfully completed the request processing.  */
            if (status == NX_WEB_HTTP_CALLBACK_COMPLETED)
            {

                /* Send success response back to HTTP Client.  */
                _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_OK,
                                                  sizeof(NX_WEB_HTTP_STATUS_OK) - 1, NX_NULL, 0, NX_NULL, 0);
                return;
            }

            /* No, an error was detected. Abort the remainder of the get processing.  */
            _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR,
                                              sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1,
                                              "NetX HTTP Request Aborted",
                                              sizeof("NetX HTTP Request Aborted") - 1, NX_NULL, 0);
            return;
        }
    }

    /* Was there a message body in the request? */
    if (server_ptr -> nx_web_http_server_request_chunked)
    {

        /* Find the first chunk of the content.  */
        status = _nx_web_http_server_packet_content_find(server_ptr, &packet_ptr, NX_NULL);
        if (status)
        {

            /* No. Regardless if a reply was sent via the request notify callback, we are done with this packet. */
            return;
        }

        offset = 0;
        length = packet_ptr -> nx_packet_length;
    }
    else if (length == 0)
    {

        /* No. Regardless if a reply was sent via the request notify callback, we are done with this packet. */
        return;
    }

    /* Otherwise, everything is okay...  complete the request.  */

    /* Create the specified file.  */
    status = fx_file_create(server_ptr -> nx_web_http_server_media_ptr, server_ptr -> nx_web_http_server_request_resource);


    if (status != NX_SUCCESS)
    {

        /* Send response back to HTTP Client.  */
        _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR,
                                          sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1,
                                          "NetX HTTP File Create Failed",
                                          sizeof("NetX HTTP File Create Failed") - 1, NX_NULL, 0);

        /* Release first chunked packet.  */
        if (server_ptr -> nx_web_http_server_request_chunked)
        {
            nx_packet_release(packet_ptr);
        }

        /* Error, return to caller.  */
        return;
    }

    /* Open the specified file for writing.  */
    status =  fx_file_open(server_ptr -> nx_web_http_server_media_ptr, &(server_ptr -> nx_web_http_server_file), server_ptr -> nx_web_http_server_request_resource, FX_OPEN_FOR_WRITE);

    /* Check for error condition.  */
    if (status != NX_SUCCESS)
    {

        /* Send response back to HTTP Client.  */
        _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR,
                                          sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1,
                                          "NetX HTTP File Open Failed",
                                          sizeof("NetX HTTP File Open Failed") - 1, NX_NULL, 0);

        /* Release first chunked packet.  */
        if (server_ptr -> nx_web_http_server_request_chunked)
        {
            nx_packet_release(packet_ptr);
        }

        /* Error, return to caller.  */
        return;
    }

    /* Determine if there is any content in the first packet.  */
    if ((UINT)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr) > offset)
    {

        /* Write the content found in this packet.  */
        status =  fx_file_write(&(server_ptr -> nx_web_http_server_file), (packet_ptr -> nx_packet_prepend_ptr + offset),
                                ((ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr) - offset));

        /* Check the status.  */
        if (status != NX_SUCCESS)
        {

            /* Send response back to HTTP Client.  */
            _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR,
                                              sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1,
                                              "NetX HTTP File Write Failed",
                                              sizeof("NetX HTTP File Write Failed") - 1, NX_NULL, 0);

            /* Release first chunked packet.  */
            if (server_ptr -> nx_web_http_server_request_chunked)
            {
                nx_packet_release(packet_ptr);
            }

            /* Error, return to caller.  */
            return;
        }

        /* Update the length.  */
        length =  length - ((ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr) - offset);

        /* Increment the bytes received count.  */
        server_ptr -> nx_web_http_server_total_bytes_received =  server_ptr -> nx_web_http_server_total_bytes_received +
                                                             ((ULONG)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr) - offset);
    }

#ifndef NX_DISABLE_PACKET_CHAIN
    /* Loop to write chained packets out to the file.  */
    next_packet_ptr =  packet_ptr -> nx_packet_next;
    while ((length) && (next_packet_ptr))
    {

        /* Write the content of the next packet.  */
        status =  fx_file_write(&(server_ptr -> nx_web_http_server_file), next_packet_ptr -> nx_packet_prepend_ptr,
                                (ULONG)(next_packet_ptr -> nx_packet_append_ptr - next_packet_ptr -> nx_packet_prepend_ptr));

        /* Check the status.  */
        if (status != NX_SUCCESS)
        {

            /* Send response back to HTTP Client.  */
            _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR,
                                              sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1,
                                              "NetX HTTP File Write Failed",
                                              sizeof("NetX HTTP File Write Failed") - 1, NX_NULL, 0);

            /* Release first chunked packet.  */
            if (server_ptr -> nx_web_http_server_request_chunked)
            {
                nx_packet_release(packet_ptr);
            }

            /* Error, return to caller.  */
            return;
        }

        /* Update the length.  */
        length =  length - (ULONG)(next_packet_ptr -> nx_packet_append_ptr - next_packet_ptr -> nx_packet_prepend_ptr);

        /* Increment the bytes received count.  */
        server_ptr -> nx_web_http_server_total_bytes_received =  server_ptr -> nx_web_http_server_total_bytes_received +
                                        (ULONG)(next_packet_ptr -> nx_packet_append_ptr - next_packet_ptr -> nx_packet_prepend_ptr);

        /* Move to the next pointer.  */
        next_packet_ptr =  next_packet_ptr -> nx_packet_next;
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Release first chunked packet.  */
    if (server_ptr -> nx_web_http_server_request_chunked)
    {
        nx_packet_release(packet_ptr);
    }

    /* If necessary, receive more packets from the TCP socket to complete the write request.  */
    while (length || server_ptr -> nx_web_http_server_request_chunked)
    {

        /* Wait for a request.  */
        status = _nx_web_http_server_packet_get(server_ptr, &data_packet_ptr);

        /* Check the return status.  */
        if (status != NX_SUCCESS)
        {

            if (status == NX_WEB_HTTP_GET_DONE)
            {
                break;
            }

            /* Send response back to HTTP Client.  */
            _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR,
                                              sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1,
                                              "NetX HTTP Receive Timeout",
                                              sizeof("NetX HTTP Receive Timeout") - 1, NX_NULL, 0);

            /* Error, return to caller.  */
            return;
        }

        if (server_ptr -> nx_web_http_server_request_chunked)
        {
            length = data_packet_ptr -> nx_packet_length;
        }

        /* Loop to write the packet chain out to the file.  */
        next_packet_ptr =  data_packet_ptr;
        do
        {

            /* Write the content of this packet.  */
            status =  fx_file_write(&(server_ptr -> nx_web_http_server_file), next_packet_ptr -> nx_packet_prepend_ptr,
                                               (ULONG)(next_packet_ptr -> nx_packet_append_ptr - next_packet_ptr -> nx_packet_prepend_ptr));

            /* Check the status.  */
            if (status != NX_SUCCESS)
            {

                /* Send response back to HTTP Client.  */
                _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR,
                                                  sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1,
                                                  "NetX HTTP File Write Failed",
                                                  sizeof("NetX HTTP File Write Failed") - 1, NX_NULL, 0);

                /* Release the previous data packet.  */
                nx_packet_release(data_packet_ptr);

                /* Error, return to caller.  */
                return;
            }

            /* Update the length.  */
            length =  length - (UINT)(next_packet_ptr -> nx_packet_append_ptr - next_packet_ptr -> nx_packet_prepend_ptr);

            /* Increment the bytes received count.  */
            server_ptr -> nx_web_http_server_total_bytes_received =  server_ptr -> nx_web_http_server_total_bytes_received +
                                        (ULONG)(next_packet_ptr -> nx_packet_append_ptr - next_packet_ptr -> nx_packet_prepend_ptr);

#ifdef NX_DISABLE_PACKET_CHAIN
            next_packet_ptr =  NX_NULL;
#else
            /* Move to the next pointer.  */
            next_packet_ptr =  next_packet_ptr -> nx_packet_next;
#endif /* NX_DISABLE_PACKET_CHAIN */

        } while ((length) && (next_packet_ptr));

        /* Release the previous data packet.  */
        nx_packet_release(data_packet_ptr);
    }

    /* Success, at this point close the file and prepare a successful response for the client.  */
    fx_file_close(&(server_ptr -> nx_web_http_server_file));


    /* Now build a response header.  */
    status = _nx_web_http_server_generate_response_header(server_ptr, &data_packet_ptr, NX_WEB_HTTP_STATUS_OK,
                                                          sizeof(NX_WEB_HTTP_STATUS_OK) - 1, 0,
                                                          NX_NULL, 0, NX_NULL, 0);
    if (status == NX_SUCCESS)
    {

        /* Send the response back to the client.  */
        status = _nx_web_http_server_send(server_ptr, data_packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_SEND);

        /* Check for an error.  */
        if (status != NX_SUCCESS)
        {

            /* Just release the packet.  */
            nx_packet_release(data_packet_ptr);
        }
    }

}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_delete_process                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the DELETE HTTP client requests.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    packet_ptr                            Request packet pointer        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_basic_authenticate                              */
/*                                          Process basic authentication  */
/*    _nx_web_http_server_digest_authenticate                             */
/*                                          Process digest authentication */
/*    _nx_web_http_server_response_send     Send response back to client  */
/*    _nx_web_http_server_retrieve_resource Retrieve resource from request*/
/*    fx_file_delete                        Delete resource file          */
/*    nx_packet_release                     Release packet                */
/*    _nx_web_http_server_send              Send packet to client         */
/*    _nx_utility_string_length_check       Check string length           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_receive_data      HTTP Server receive processing*/
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
VOID  _nx_web_http_server_delete_process(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{

UINT        status;
CHAR        *name_ptr;
CHAR        *password_ptr;
CHAR        *realm_ptr;
NX_PACKET   *response_ptr;
UINT        auth_request_present = NX_FALSE;
NX_TCP_SOCKET *socket_ptr = &server_ptr -> nx_web_http_server_current_session_ptr -> nx_tcp_session_socket;
UINT        name_length = 0;
UINT        password_length = 0;
UINT        realm_length = 0;
UINT        temp_name_length = 0;
UINT        temp_password_length = 0;
UINT        temp_realm_length = 0;


    /* Pickup the URL (resource) from the request.  */
    status =  _nx_web_http_server_retrieve_resource(server_ptr, packet_ptr, server_ptr -> nx_web_http_server_request_resource, NX_WEB_HTTP_MAX_RESOURCE + 1);

    /* Determine if the resource was extracted successfully.  */
    if (status != NX_SUCCESS)
    {

        /* Send response back to HTTP Client.  */
        _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_BAD_REQUEST,
                                          sizeof(NX_WEB_HTTP_STATUS_BAD_REQUEST) - 1,
                                          "NetX HTTP URL Bad",
                                          sizeof("NetX HTTP URL Bad") - 1, NX_NULL, 0);

        /* Error, return to caller.  */
        return;
    }

    /* Determine if the application has specified an authentication function for this server.  */
    if (server_ptr -> nx_web_http_server_authentication_check ||
        server_ptr -> nx_web_http_server_authentication_check_extended)
    {

        /* Determine if authentication is required for the specified resource.  */
        if (server_ptr -> nx_web_http_server_authentication_check_extended)
        {
            status =  (server_ptr -> nx_web_http_server_authentication_check_extended)(server_ptr, NX_WEB_HTTP_SERVER_DELETE_REQUEST, server_ptr -> nx_web_http_server_request_resource,
                                                                                       &name_ptr, &name_length, &password_ptr, &password_length, &realm_ptr, &realm_length);
        }
        else
        {
            status =  (server_ptr -> nx_web_http_server_authentication_check)(server_ptr, NX_WEB_HTTP_SERVER_DELETE_REQUEST, server_ptr -> nx_web_http_server_request_resource,
                                                                            &name_ptr, &password_ptr, &realm_ptr);
        }

        if ((status == NX_WEB_HTTP_BASIC_AUTHENTICATE) ||
            (status == NX_WEB_HTTP_DIGEST_AUTHENTICATE))
        {

            /* Check name, password and realm string.  */
            if (_nx_utility_string_length_check(name_ptr, &temp_name_length, NX_WEB_HTTP_MAX_NAME) ||
                _nx_utility_string_length_check(password_ptr, &temp_password_length, NX_WEB_HTTP_MAX_PASSWORD) ||
                _nx_utility_string_length_check(realm_ptr, &temp_realm_length, NX_MAX_STRING_LENGTH))
            {

                /* Error, return to caller.  */
                return;
            }

            /* Validate string length. */
            if (server_ptr -> nx_web_http_server_authentication_check_extended &&
                ((realm_length != temp_realm_length) ||
                 (name_length != temp_name_length) ||
                 (password_length != temp_password_length)))
            {
                return;
            }
        }

        /* Determine what kind - if any - authentication is requested for this resource.  */
        if (status == NX_WEB_HTTP_BASIC_AUTHENTICATE)
        {

            /* Process basic authentication request.  */
            status =  _nx_web_http_server_basic_authenticate(server_ptr, packet_ptr, name_ptr, password_ptr, realm_ptr, &auth_request_present);
        }
#ifdef  NX_WEB_HTTP_DIGEST_ENABLE
        else if (status == NX_WEB_HTTP_DIGEST_AUTHENTICATE)
        {

            /* Process digest authentication request.  */
            status =  _nx_web_http_server_digest_authenticate(server_ptr, packet_ptr, name_ptr, password_ptr, realm_ptr, &auth_request_present);
        }
#endif

        /* Determine if the authentication is currently in progress.  */
        if ((status != NX_WEB_HTTP_DONT_AUTHENTICATE) && (status != NX_SUCCESS))
        {

            /* Yes, authentication is in progress.  The HTTP 401 has already been sent.  */
            if (server_ptr -> nx_web_http_server_invalid_username_password_callback && auth_request_present)
            {
            NXD_ADDRESS client_nxd_address;
            ULONG       client_port;

                /* Get the IP address of the client:  */
                status =   nxd_tcp_socket_peer_info_get(socket_ptr, &client_nxd_address , &client_port);

                if (status == NX_SUCCESS)
                {
                    /* Send this information to the host application. */
                    (server_ptr -> nx_web_http_server_invalid_username_password_callback)(server_ptr -> nx_web_http_server_request_resource, &client_nxd_address, NX_WEB_HTTP_SERVER_DELETE_REQUEST);
                }
            }

            return;
        }
    }

    /* Determine if a user supplied notify function has been specified.  */
    if (server_ptr -> nx_web_http_server_request_notify)
    {

        /* Call the user supplied function to notify the user of the delete request.  */
        status =  (server_ptr -> nx_web_http_server_request_notify)(server_ptr, NX_WEB_HTTP_SERVER_DELETE_REQUEST, server_ptr -> nx_web_http_server_request_resource, packet_ptr);

        /* Determine if the user supplied routine is requesting the delete should be aborted.  */
        if (status != NX_SUCCESS)
        {

            /* Determine if the user callback routine successfully completed the request processing.  */
            if (status == NX_WEB_HTTP_CALLBACK_COMPLETED)
            {

                /* Send success response back to HTTP Client.  */
                _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_OK,
                                                  sizeof(NX_WEB_HTTP_STATUS_OK) - 1, NX_NULL, 0, NX_NULL, 0);
                return;
            }

            /* Send response back to HTTP Client.  */
            _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR,
                                              sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1,
                                              "NetX HTTP Request Aborted",
                                              sizeof("NetX HTTP Request Aborted") - 1, NX_NULL, 0);

            /* Yes, error was detected. Abort the remainder of the delete processing.  */
            return;
        }
    }

    /* Otherwise, everything is okay...  complete the request.  */

    /* Delete the specified file.  */
    status =  fx_file_delete(server_ptr -> nx_web_http_server_media_ptr, server_ptr -> nx_web_http_server_request_resource);

    /* Check for error condition.  */
    if (status != NX_SUCCESS)
    {

        /* Send response back to HTTP Client.  */
        _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR,
                                          sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1,
                                          "NetX HTTP File Delete Failed",
                                          sizeof("NetX HTTP File Delete Failed") - 1, NX_NULL, 0);

        /* Error, return to caller.  */
        return;
    }

    /* Now build a response header.  */
    status = _nx_web_http_server_generate_response_header(server_ptr, &response_ptr, NX_WEB_HTTP_STATUS_OK,
                                                          sizeof(NX_WEB_HTTP_STATUS_OK) - 1, 0, NX_NULL, 0, NX_NULL, 0);
    if (status == NX_SUCCESS)
    {

        /* Send the response back to the client.  */
        status = _nx_web_http_server_send(server_ptr, response_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_SEND);

        /* Check for an error.  */
        if (status != NX_SUCCESS)
        {

            /* Just release the packet.  */
            nx_packet_release(response_ptr);
        }
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_invalid_userpassword_notify_set                */
/*                                                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs error checking for service to set invalid    */
/*    username password callback function.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    invalid_username_password_callback    Pointer to application's      */
/*                                          invalid username password     */
/*                                          callback function             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
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
UINT _nxe_web_http_server_invalid_userpassword_notify_set(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                    UINT (*invalid_username_password_callback)(CHAR *resource, NXD_ADDRESS *client_nxd_address, UINT request_type)) 
{

UINT status;


    if ((http_server_ptr == NX_NULL) || (invalid_username_password_callback == NX_NULL))
    {
        return NX_PTR_ERROR;
    }

    status = _nx_web_http_server_invalid_userpassword_notify_set(http_server_ptr, invalid_username_password_callback);

    return status;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_invalid_userpassword_notify_set PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets invalid username password callback function      */
/*                                                                        */
/*    Note: The string resource in callback function                      */
/*    invalid_username_password_callback is built by internal logic and   */
/*    always NULL-terminated.                                             */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    invalid_username_password_callback    Pointer to application's      */
/*                                          invalid username password     */
/*                                          callback function             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
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
UINT _nx_web_http_server_invalid_userpassword_notify_set(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                         UINT (*invalid_username_password_callback)(CHAR *resource, NXD_ADDRESS *client_nxd_address, UINT request_type)) 
{

    http_server_ptr -> nx_web_http_server_invalid_username_password_callback = invalid_username_password_callback;

    return NX_SUCCESS;
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_response_send                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends the specified Server response to the requesting */
/*    HTTP client.                                                        */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    status_code                           Status-code and reason-phrase */
/*    status_code_length                    Length of status-code         */
/*    information                           Pointer to HTTP info string   */
/*    information_length                    Length of information         */
/*    additional_information                Pointer to additional HTTP    */
/*                                            information                 */
/*    additional_information_length         Length of additional          */
/*                                            information                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_generate_response_header                        */
/*                                          Generate response header      */
/*    nx_packet_data_append                 Append information to response*/
/*    nx_packet_release                     Release packet                */
/*    _nx_web_http_server_send              Send HTTP Server response     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_receive               HTTP receive processing   */
/*    _nx_web_http_server_get_process           Process GET request       */
/*    _nx_web_http_server_put_process           Process PUT request       */
/*    _nx_web_http_server_delete_process        Process DELETE request    */
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
UINT  _nx_web_http_server_response_send(NX_WEB_HTTP_SERVER *server_ptr, CHAR *status_code,
                                        UINT status_code_length, CHAR *information,
                                        UINT information_length, CHAR *additional_information,
                                        UINT additional_information_length)
{

UINT        status;
UINT        length;
NX_PACKET   *packet_ptr;

    /* Determine if there is additional information.  */
    if(information)
    {

        /* Calculate the size of the information field.  */
        length = information_length;

        /* Determine if there is additional information.  */
        if (additional_information)
        {

            /* Update the length with it as well.  */
            length = length + additional_information_length;
        }
    }
    else
        length = 0;

    /* Generate response header. */
    status = _nx_web_http_server_generate_response_header(server_ptr, &packet_ptr, status_code,
                                                          status_code_length, length, NX_NULL, 0, NX_NULL, 0);

    /* Determine if an error occurred.  */
    if (status != NX_SUCCESS)
    {

        /* Just return.  */
        return(status);
    }

    /* Determine if there is additional information.  */
    if (information)
    {

        /* Place the first informational field.  */
        status = nx_packet_data_append(packet_ptr, information, information_length,
                                       server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* If there is additional information, place it in the buffer as well.  */
        if (additional_information)
        {

            /* Now, place the additional informational field.  */
            status += nx_packet_data_append(packet_ptr, additional_information, additional_information_length,
                                            server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
        }
    }

    /* Check for an error.  */
    if (status != NX_SUCCESS)
    {

        /* Just release the packet and return.  */
        nx_packet_release(packet_ptr);
        return(status);
    }

    /* Send the response back to the client.  */
    status = _nx_web_http_server_send(server_ptr, packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_SEND);

    /* Check for an error.  */
    if (status != NX_SUCCESS)
    {

        /* Just release the packet.  */
        nx_packet_release(packet_ptr);
    }

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_basic_authenticate              PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for basic (name & password) authentication.    */
/*    If found and correct, it returns a success.  Otherwise, it returns  */
/*    an authentication request to the requesting client.                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    packet_ptr                            Request packet pointer        */
/*    name_ptr                              Pointer to name string        */
/*    password_ptr                          Pointer to password string    */
/*    realm_ptr                             Pointer to realm string       */
/*    auth_request_present                  Indicate if authentication    */
/*                                                  must be performed     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nx_web_http_server_retrieve_basic_authorization                     */
/*                                          Pickup authorization          */
/*   _nx_web_http_base64_decode             Decode authorization          */
/*    _nx_web_http_server_response_packet_allocate                        */
/*                                          Allocate a response packet    */
/*    nx_packet_data_append                 Append information to response*/
/*    nx_packet_release                     Release packet                */
/*    _nx_web_http_server_send              Send HTTP Server response     */
/*    _nx_utility_string_length_check       Check string length           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_get_process       Process GET request           */
/*    _nx_web_http_server_put_process       Process PUT request           */
/*    _nx_web_http_server_delete_process    Process DELETE request        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved the logic of       */
/*                                            parsing base64,             */
/*                                            resulting in version 6.1.6  */
/*  10-31-2022     Yuxin Zhou               Modified comment(s), fixed    */
/*                                            the issue of processing     */
/*                                            empty password,             */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_web_http_server_basic_authenticate(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, CHAR *name_ptr, CHAR *password_ptr, CHAR *realm_ptr, UINT *auth_request_present)
{

UINT        i, j;
UINT        status, status1;
CHAR        quote[2] = {0x22, 0};
CHAR        crlf[2] = {13,10};
UINT        match;
UINT        realm_length;
UINT        length;
UINT        authorization_decoded_size;

    /* Default to no authentication request detected. */
    *auth_request_present = NX_FALSE;

    /* Default the status to authenticate.  */
    status =  NX_WEB_HTTP_BASIC_AUTHENTICATE;

    memset(&authorization_request[0], 0, sizeof(authorization_request));

    memset(&authorization_decoded[0], 0, sizeof(authorization_decoded));

    /* Is the authorization request present?  */
    length = _nx_web_http_server_retrieve_basic_authorization(packet_ptr, authorization_request);
    if (length)
    {

        /* Yes, an authorization request is present.  */
        *auth_request_present = NX_TRUE;

        /* Convert the request from Base64 representation to ASCII.  */
        _nx_utility_base64_decode((UCHAR *)authorization_request, length, (UCHAR *)authorization_decoded, sizeof(authorization_decoded), &authorization_decoded_size);

        /* See if it is valid.  */

        /* Compare the name.  */
        i =  0;
        match = NX_TRUE;
        while (name_ptr[i] && (i < authorization_decoded_size))
        {

            /* Is there a mismatch?  */
            if (name_ptr[i] != authorization_decoded[i])
            {

                /* Name mismatch. Continue to avoid timing attack. */
                match = NX_FALSE;
            }

            /* Move to next character.  */
            i++;
        }

        /* Determine if everything matches.  */
        if (match && (i < authorization_decoded_size) && (authorization_decoded[i] == ':'))
        {

            /* Move the authorization index past the semicolon.  */
            i++;

            /* Now compare the passwords.  */
            j =  0;
            match = NX_TRUE;
            while (password_ptr[j] && (i < authorization_decoded_size))
            {

                /* Is there a mismatch?  */
                if (password_ptr[j] != authorization_decoded[i])
                {

                    /* Password mismatch. Continue to avoid timing attack. */
                    match = NX_FALSE;
                }

                /* Move to next character.  */
                i++;
                j++;
            }

            /* Determine if we have a match.  */
            if (match && (i == authorization_decoded_size) && 
                (authorization_decoded[i] == (CHAR) NX_NULL) && 
                (password_ptr[j] == (CHAR) NX_NULL))
            {

                /* Yes, we have successful authorization!!  */
                status =  NX_SUCCESS;
            }
        }
    }

    /* Determine if we need to send back an unauthorized request.  */
    if (status == NX_WEB_HTTP_BASIC_AUTHENTICATE)
    {

        /* We need authorization so build the HTTP 401 Unauthorized message to send to the server.  */

        if (_nx_utility_string_length_check(realm_ptr, &realm_length, NX_MAX_STRING_LENGTH))
        {
            return(NX_WEB_HTTP_ERROR);
        }

        /* Allocate a packet for sending the response back.  */
        status1 =  _nx_web_http_server_response_packet_allocate(server_ptr, &packet_ptr, NX_WAIT_FOREVER);

        /* Determine if an error occurred in the packet allocation.  */
        if (status1 != NX_SUCCESS)
        {

            /* Indicate an allocation error occurred.  */
            server_ptr -> nx_web_http_server_allocation_errors++;

            /* Just return.  */
            return(status1);
        }

        /* Insert the response header.  */
        nx_packet_data_append(packet_ptr, NX_WEB_HTTP_VERSION, sizeof(NX_WEB_HTTP_VERSION) - 1,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
        nx_packet_data_append(packet_ptr, " ", 1, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
        nx_packet_data_append(packet_ptr, NX_WEB_HTTP_STATUS_UNAUTHORIZED, sizeof(NX_WEB_HTTP_STATUS_UNAUTHORIZED) - 1,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place the <cr,lf> into the buffer.  */
        nx_packet_data_append(packet_ptr, crlf, 2,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Insert the type of authentication requested.  */
        nx_packet_data_append(packet_ptr, "WWW-Authenticate: Basic realm=", 30,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Insert the double quote.  */
        nx_packet_data_append(packet_ptr, quote, 1,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place the realm string into the buffer.  */
        nx_packet_data_append(packet_ptr, realm_ptr, realm_length,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Insert the double quote.  */
        nx_packet_data_append(packet_ptr, quote, 1,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place the <cr,lf> into the buffer.  */
        nx_packet_data_append(packet_ptr, crlf, 2,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place another <cr,lf> into the buffer to signal end of FULL HTTP response.  */
        nx_packet_data_append(packet_ptr, crlf, 2,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Send the response back to the client.  */
        status1 = _nx_web_http_server_send(server_ptr, packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_SEND);

        /* Check for an error.  */
        if (status1)
        {

            /* Return the internal NetX error. */
            status = status1;

            /* Just release the packet.  */
            nx_packet_release(packet_ptr);
        }
    }

    /* Return the result of the authentication request.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_retrieve_basic_authorization    PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves basic authentication information from the   */
/*    HTTP request packet.                                                */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Request packet pointer        */
/*    authorization_request_ptr             Pointer to destination for    */
/*                                            authorization string        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   None                                                                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_basic_authenticate                              */
/*                                          Basic authenticate processing */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Yuxin Zhou               Modified comment(s),  fixed   */
/*                                            the HTTP Server state issue */
/*                                            with basic authorization,   */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_web_http_server_retrieve_basic_authorization(NX_PACKET *packet_ptr, CHAR *authorization_request_ptr)
{

UINT    length;
UINT    found;
CHAR    *buffer_ptr;


    /* Set the found flag to false.  */
    found =  NX_FALSE;

    /* Default the authorization request to zero.  */
    length =  0;

    /* Set the authorization request string to NULL.  */
    authorization_request_ptr[0] =  NX_NULL;

    /* Setup pointer to buffer.  */
    buffer_ptr =  (CHAR *) packet_ptr -> nx_packet_prepend_ptr;


    /* Find the "Authorization: " token first.  */
    while (((buffer_ptr+15) < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0))
    {

        /* Check for the Authorization: token.  */
        if (((*buffer_ptr ==      'a') || (*buffer_ptr ==      'A')) &&
            ((*(buffer_ptr+1) ==  'u') || (*(buffer_ptr+1) ==  'U')) &&
            ((*(buffer_ptr+2) ==  't') || (*(buffer_ptr+2) ==  'T')) &&
            ((*(buffer_ptr+3) ==  'h') || (*(buffer_ptr+3) ==  'H')) &&
            ((*(buffer_ptr+4) ==  'o') || (*(buffer_ptr+4) ==  'O')) &&
            ((*(buffer_ptr+5) ==  'r') || (*(buffer_ptr+5) ==  'R')) &&
            ((*(buffer_ptr+6) ==  'i') || (*(buffer_ptr+6) ==  'I')) &&
            ((*(buffer_ptr+7) ==  'z') || (*(buffer_ptr+7) ==  'Z')) &&
            ((*(buffer_ptr+8) ==  'a') || (*(buffer_ptr+8) ==  'A')) &&
            ((*(buffer_ptr+9) ==  't') || (*(buffer_ptr+9) ==  'T')) &&
            ((*(buffer_ptr+10) == 'i') || (*(buffer_ptr+10) == 'I')) &&
            ((*(buffer_ptr+11) == 'o') || (*(buffer_ptr+11) == 'O')) &&
            ((*(buffer_ptr+12) == 'n') || (*(buffer_ptr+12) == 'N')) &&
            (*(buffer_ptr+13) == ':') &&
            (*(buffer_ptr+14) == ' '))
        {

            /* Move the pointer up to the length token.  */
            buffer_ptr =  buffer_ptr + 15;

            /* Set the found flag.  */
            found =  NX_TRUE;
            break;
        }

        /* Move the pointer up to the next character.  */
        buffer_ptr++;
    }

    /* Determine if the first token was found.  */
    if (!found)
    {

        /* No, authorization is not present.  Return a zero length.  */
        return(length);
    }

    /* Set the found flag back to false.  */
    found =  NX_FALSE;

    /* Now remove any extra blanks.  */
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr == ' '))
    {

        /* Move the pointer up one character.  */
        buffer_ptr++;
    }

    /* Now check for the "Basic " token.  */
    while (((buffer_ptr+6) < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0))
    {

        /* Check for the Basic token.  */
        if (((*buffer_ptr ==      'b') || (*buffer_ptr ==      'B')) &&
            ((*(buffer_ptr+1) ==  'a') || (*(buffer_ptr+1) ==  'A')) &&
            ((*(buffer_ptr+2) ==  's') || (*(buffer_ptr+2) ==  'S')) &&
            ((*(buffer_ptr+3) ==  'i') || (*(buffer_ptr+3) ==  'I')) &&
            ((*(buffer_ptr+4) ==  'c') || (*(buffer_ptr+4) ==  'C')) &&
            (*(buffer_ptr+5) == ' '))
        {

            /* Move the pointer up to the actual authorization string.  */
            buffer_ptr =  buffer_ptr + 6;

            /* Set the found flag.  */
            found =  NX_TRUE;
            break;
        }

        /* Move the pointer up to the next character.  */
        buffer_ptr++;
    }

    /* Determine if the first token was found.  */
    if (!found)
    {

        /* No, authorization is not present.  Return a zero length.  */
        return(length);
    }

    /* Now remove any extra blanks.  */
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr == ' '))
    {

        /* Move the pointer up one character.  */
        buffer_ptr++;
    }

    /* Now pickup the authorization string.  */
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0) && (*buffer_ptr != ' ') && (*buffer_ptr != (CHAR) 13) && (length < NX_WEB_HTTP_MAX_STRING))
    {

        /* Copy a character of the authorization string into the destination.  */
        authorization_request_ptr[length] =  *buffer_ptr++;
        length++;
    }

    /* Return the length to the caller.  */
    return(length);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_retrieve_resource               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves the resource (URL) portion of the request   */
/*    and places it in the destination.                                   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    packet_ptr                            Request packet pointer        */
/*    destination                           Destination for resource      */
/*    max_size                              Maximum size of destination   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_get_process       Process GET request           */
/*    _nx_web_http_server_put_process       Process PUT request           */
/*    _nx_web_http_server_delete_process    Process DELETE request        */
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
UINT  _nx_web_http_server_retrieve_resource(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, CHAR *destination, UINT max_size)
{

UINT    i;
CHAR    *buffer_ptr;

#ifndef NX_WEB_HTTPS_ENABLE
    NX_PARAMETER_NOT_USED(server_ptr);
#endif /* NX_WEB_HTTPS_ENABLE */

    /* Set the destination to NULL.  */
    destination[0] =  (CHAR) 0;

    /* Setup pointer to buffer.  */
    buffer_ptr =  (CHAR *) packet_ptr -> nx_packet_prepend_ptr;

    /* Find the first space which is the start position of URI. */
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != ' '))
    {

        /* Move the pointer up to the next character.  */
        buffer_ptr++;
    }

    /* Check for an error condition.  */
    if (*buffer_ptr != ' ')
        return(NX_WEB_HTTP_ERROR);

    buffer_ptr++;

#ifdef NX_WEB_HTTPS_ENABLE

    if (server_ptr -> nx_web_http_is_https_server)
    {
        if ((buffer_ptr + 8 < (CHAR *)packet_ptr->nx_packet_append_ptr) && (*buffer_ptr != '/'))
        {

            /* Check whether it is an absoluteURI*/
            if (((*buffer_ptr == 'h') || (*buffer_ptr == 'H')) &&
                ((*(buffer_ptr + 1) == 't') || (*(buffer_ptr + 1) == 'T')) &&
                ((*(buffer_ptr + 2) == 't') || (*(buffer_ptr + 2) == 'T')) &&
                ((*(buffer_ptr + 3) == 'p') || (*(buffer_ptr + 3) == 'P')) &&
                ((*(buffer_ptr + 4) == 's') || (*(buffer_ptr + 4) == 'S')) &&
                (*(buffer_ptr + 5) == ':') &&
                (*(buffer_ptr + 6) == '/') &&
                (*(buffer_ptr + 7) == '/'))
            {

                /* Yes it is. Find the absolute path. */
                buffer_ptr += 8;

                /* Find the first slash character.  The first slash marks the beginning of the URL.  */
                while ((buffer_ptr < (CHAR *)packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR)0) && (*buffer_ptr != '/'))
                {

                    /* Move the pointer up to the next character.  */
                    buffer_ptr++;
                }
            }
        }

    }
    else
#endif
    {
        if ((buffer_ptr + 7 < (CHAR *)packet_ptr->nx_packet_append_ptr) && (*buffer_ptr != '/'))
        {

            /* Check whether it is an absoluteURI*/
            if (((*buffer_ptr == 'h') || (*buffer_ptr == 'H')) &&
                ((*(buffer_ptr + 1) == 't') || (*(buffer_ptr + 1) == 'T')) &&
                ((*(buffer_ptr + 2) == 't') || (*(buffer_ptr + 2) == 'T')) &&
                ((*(buffer_ptr + 3) == 'p') || (*(buffer_ptr + 3) == 'P')) &&
                (*(buffer_ptr + 4) == ':') &&
                (*(buffer_ptr + 5) == '/') &&
                (*(buffer_ptr + 6) == '/'))
            {

                /* Yes it is. Find the absolute path. */
                buffer_ptr += 7;

                /* Find the first slash character.  The first slash marks the beginning of the URL.  */
                while ((buffer_ptr < (CHAR *)packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR)0) && (*buffer_ptr != '/'))
                {

                    /* Move the pointer up to the next character.  */
                    buffer_ptr++;
                }
            }
        }
    }

    /* Check for an error condition.  */
    if ((buffer_ptr >= (CHAR *) packet_ptr -> nx_packet_append_ptr) || (*buffer_ptr != '/'))
        return(NX_WEB_HTTP_ERROR);

    /* Copy the rest of the resource to the destination.  Space, semicolon, and question mark characters signal the
       end of the resource.  */
    i =  0;
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != ' ') && (*buffer_ptr != ';') && (*buffer_ptr != '?') && (i < (max_size-1)))
    {

        /* Check escape characters. */
        if(*buffer_ptr == '%')
        {

            /* It is an escape character. */
            if((buffer_ptr + 2) < (CHAR *)packet_ptr -> nx_packet_append_ptr)
            {

                /* Convert the HEX number. */
                buffer_ptr++;
                if((*buffer_ptr >= '0') && (*buffer_ptr <= '9'))
                    destination[i] = (CHAR)(*buffer_ptr - '0');
                else if((*buffer_ptr >= 'a') && (*buffer_ptr <= 'f'))
                    destination[i] = (CHAR)(*buffer_ptr - 'a' + 10);
                else if((*buffer_ptr >= 'A') && (*buffer_ptr <= 'F'))
                    destination[i] = (CHAR)(*buffer_ptr - 'A' + 10);
                else
                {

                    /* Error picking up the resource.  */
                    destination[0] =  (CHAR) 0;
                    return(NX_WEB_HTTP_ERROR);
                }
                destination[i] = (CHAR)(destination[i] << 4);

                /* Convert the HEX number. */
                buffer_ptr++;
                if((*buffer_ptr >= '0') && (*buffer_ptr <= '9'))
                    destination[i] = (CHAR)(destination[i] + (*buffer_ptr - '0'));
                else if((*buffer_ptr >= 'a') && (*buffer_ptr <= 'f'))
                    destination[i] = (CHAR)(destination[i] + (*buffer_ptr - 'a' + 10));
                else if((*buffer_ptr >= 'A') && (*buffer_ptr <= 'F'))
                    destination[i] = (CHAR)(destination[i] + (*buffer_ptr - 'A' + 10));
                else
                {

                    /* Error picking up the resource.  */
                    destination[0] =  (CHAR) 0;
                    return(NX_WEB_HTTP_ERROR);
                }

                /* Move to the next byte. */
                i++;
                buffer_ptr++;
            }
            else
            {

                /* Error picking up the resource.  */
                destination[0] =  (CHAR) 0;
                return(NX_WEB_HTTP_ERROR);
            }
        }
        else
        {

            /* Copy the URL name into the destination.  */
            destination[i++] =  *buffer_ptr++;
        }
    }

    /* Determine if the resource was retrieved.  */
    if ((destination[0] == (CHAR) 0) || (buffer_ptr >= (CHAR *)packet_ptr -> nx_packet_append_ptr) || ((*buffer_ptr != ' ') && (*buffer_ptr != '?') && (*buffer_ptr != ';')))
    {

        /* Error picking up the resource.  */
        destination[0] =  (CHAR) 0;
        return(NX_WEB_HTTP_ERROR);
    }

    /* Everything is okay, place a NULL at the end of the resource.  */
    destination[i] =  (CHAR) 0;

    /* Return success.  */
    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_calculate_content_offset        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates the byte offset to the start of the        */
/*    HTTP request content area.  This area immediately follows the HTTP  */
/*    request header (which ends with a blank line).                      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to request packet     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Byte Offset                           (0 implies no content)        */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_content_get       Application content get       */
/*    _nx_web_http_server_get_process       Process GET request           */
/*    _nx_web_http_server_put_process       Process PUT request           */
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
UINT  _nx_web_http_server_calculate_content_offset(NX_PACKET *packet_ptr)
{

UINT       offset;
CHAR      *buffer_ptr;
UINT       crlf_found = 0;


    /* Default the content offset to zero.  */
    offset =  0;

    /* Find the "cr,lf,cr,lf" token.  */
#ifndef NX_DISABLE_PACKET_CHAIN
    while(packet_ptr)
    {
#endif

        /* Setup pointer to buffer.  */
        buffer_ptr =  (CHAR *) packet_ptr -> nx_packet_prepend_ptr;

        while(buffer_ptr < (CHAR *)packet_ptr -> nx_packet_append_ptr)
        {
            if (!(crlf_found & 1) && (*buffer_ptr == (CHAR)13))
            {

                /* Found CR.  */
                crlf_found++;
            }
            else if ((crlf_found & 1) && (*buffer_ptr == (CHAR)10))
            {

                /* Found LF.  */
                crlf_found++;
            }
            else
            {

                /* Reset the CRLF marker.  */
                crlf_found = 0;
            }

            offset++;
            buffer_ptr++;

            if (crlf_found == 4)
            {

                /* Return the offset to the caller.  */
                return(offset);
            }
        }

#ifndef NX_DISABLE_PACKET_CHAIN
        packet_ptr = packet_ptr -> nx_packet_next;
    }
#endif

    /* Not found the "cr,lf,cr,lf".  */
    return(0);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_type_get                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks error for deriving the type of the resource.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    name                                  Name string                   */
/*    http_type_string                      Destination HTTP type string  */
/*    string_size                           Return HTTP type string size  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Size                                  Number of bytes in string     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_type_get                                        */
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
UINT  _nxe_web_http_server_type_get(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name, CHAR *http_type_string, UINT *string_size)
{
UINT status;


    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) ||
        (name == NX_NULL)       || (http_type_string == NX_NULL) || (string_size == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    status = _nx_web_http_server_type_get(server_ptr, name, http_type_string, string_size);

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_type_get                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function derives the type of the resource.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    name                                  Name string                   */
/*    http_type_string                      Destination HTTP type string  */
/*    string_size                           Return HTTP type string size  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Size                                  Number of bytes in string     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_memicmp                                         */
/*    _nx_utility_string_length_check                                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_type_get_extended Get MIME type                 */
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
UINT  _nx_web_http_server_type_get(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name, CHAR *http_type_string, UINT *string_size)
{
UINT    temp_name_length;

    /* Check name length.  */
    if (_nx_utility_string_length_check(name, &temp_name_length, NX_MAX_STRING_LENGTH))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* Call actual service type get function. */
    return(_nx_web_http_server_type_get_extended(server_ptr, name, temp_name_length,
                                                 http_type_string, NX_MAX_STRING_LENGTH + 1,
                                                 string_size));
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_type_get_extended              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks error for deriving the type of the resource.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    name                                  Name string                   */
/*    name_length                           Length of name string         */
/*    http_type_string                      Destination HTTP type string  */
/*    http_type_string_max_size             Size of the destination string*/
/*    string_size                           Return HTTP type string size  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Web HTTP status code          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_type_get_extended Get MIME type                 */
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
UINT  _nxe_web_http_server_type_get_extended(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name, UINT name_length,
                                             CHAR *http_type_string, UINT http_type_string_max_size,
                                             UINT *string_size)
{
UINT status;


    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) ||
        (name == NX_NULL)       || (http_type_string == NX_NULL) || (string_size == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    status = _nx_web_http_server_type_get_extended(server_ptr, name, name_length, http_type_string,
                                                   http_type_string_max_size, string_size);

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_type_get_extended               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function derives the type of the resource.                     */
/*                                                                        */
/*    Note: The string of name must be NULL-terminated and length of each */
/*    string matches the length specified in the argument list.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    name                                  Name string                   */
/*    name_length                           Length of name string         */
/*    http_type_string                      Destination HTTP type string  */
/*    http_type_string_max_size             Size of the destination string*/
/*    string_size                           Return HTTP type string size  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Web HTTP status code          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_memicmp           Compare ignore case           */
/*    _nx_utility_string_length_check       String length check           */
/*    memcpy                                Copy memory data              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_get_process       Process GET request           */
/*    Application Code                                                    */
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
UINT  _nx_web_http_server_type_get_extended(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name, UINT name_length,
                                            CHAR *http_type_string, UINT http_type_string_max_size,
                                            UINT *string_size)
{

UINT    i;
CHAR   *ch;
UINT    ext_length;
UINT    map_ext_length;
UINT    map_type_length;
UINT    temp_name_length;

    /* Check name length.  */
    if (_nx_utility_string_length_check(name, &temp_name_length, name_length))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* Validate string length. */
    if (name_length != temp_name_length)
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* First find the end of the string.  */    
    ch = name + name_length;

    /* Now look backwards to find the last period that signals the
       file extension.  */
    ext_length = 0;
    while ((ch >= name) && (*ch != '.') &&(*ch != '/'))
    {
        ch--;
        ext_length++;
    }

    if(*ch != '.')
    {

        /* No extension is found. Return the default mime type. */
        if(http_type_string_max_size < (sizeof(NX_WEB_HTTP_SERVER_DEFAULT_MIME)))
        {
            /* NX_HTTP_SERVER_DEFAULT_MIME does not fit into 
               the caller-supplied http_type_string. */
            return (NX_WEB_HTTP_ERROR);
        }


        /* No extension is found. Return the default mime type. */
        memcpy(http_type_string, NX_WEB_HTTP_SERVER_DEFAULT_MIME, sizeof(NX_WEB_HTTP_SERVER_DEFAULT_MIME)); /* Use case of memcpy is verified. */
        *string_size = sizeof(NX_WEB_HTTP_SERVER_DEFAULT_MIME) - 1;
        return(NX_WEB_HTTP_EXTENSION_MIME_DEFAULT);
    }

    /* Position forward again, past the period.  */
    ch++;
    ext_length--;

    /* Now see what HTTP file type to return.  */
    /* Search user defined MIME maps first. */
    if(server_ptr -> nx_web_http_server_mime_maps_additional &&
       (server_ptr -> nx_web_http_server_mime_maps_additional_num > 0))
    {
        for(i = 0; i < server_ptr -> nx_web_http_server_mime_maps_additional_num; i++)
        {

            /* Check map extension and type length.  */
            if (_nx_utility_string_length_check(server_ptr -> nx_web_http_server_mime_maps_additional[i].nx_web_http_server_mime_map_extension,
                                                &map_ext_length, ext_length) ||
                _nx_utility_string_length_check(server_ptr -> nx_web_http_server_mime_maps_additional[i].nx_web_http_server_mime_map_type,
                                                &map_type_length, http_type_string_max_size - 1))
            {
                continue;
            }

            if(_nx_web_http_server_memicmp((UCHAR *)ch, ext_length,
                                           (UCHAR *)server_ptr -> nx_web_http_server_mime_maps_additional[i].nx_web_http_server_mime_map_extension,
                                           map_ext_length) == NX_SUCCESS)
            {

                /* Find the extension. Return the mapped MIME type. */
                memcpy(http_type_string, server_ptr -> nx_web_http_server_mime_maps_additional[i].nx_web_http_server_mime_map_type, map_type_length + 1); /* Use case of memcpy is verified. */
                *string_size = map_type_length;
                return(NX_SUCCESS);
            }
        }
    }

    /* Search default MIME maps. */
    for(i = 0; i < sizeof(_nx_web_http_server_mime_maps) / sizeof(NX_WEB_HTTP_SERVER_MIME_MAP); i++)
    {

        /* Check map extension and type length.  */
        if (_nx_utility_string_length_check(_nx_web_http_server_mime_maps[i].nx_web_http_server_mime_map_extension,
                                            &map_ext_length, ext_length) ||
            _nx_utility_string_length_check(_nx_web_http_server_mime_maps[i].nx_web_http_server_mime_map_type,
                                            &map_type_length, http_type_string_max_size - 1))
        {
            continue;
        }

        if(_nx_web_http_server_memicmp((UCHAR *)ch, ext_length,
                                   (UCHAR *)_nx_web_http_server_mime_maps[i].nx_web_http_server_mime_map_extension,
                                   map_ext_length) == NX_SUCCESS)
        {

            /* Find the extension. Return the mapped MIME type. */
            memcpy(http_type_string, _nx_web_http_server_mime_maps[i].nx_web_http_server_mime_map_type, map_type_length + 1); /* Use case of memcpy is verified. */
            *string_size = map_type_length;
            return(NX_SUCCESS);
        }
    }

    /* No extension matches. Return the default mime type. */

    if(http_type_string_max_size < (sizeof(NX_WEB_HTTP_SERVER_DEFAULT_MIME)))
    {
        /* NX_HTTP_SERVER_DEFAULT_MIME does not fit into 
           the caller-supplied http_type_string. */
        return (NX_WEB_HTTP_ERROR);
    }

    memcpy(http_type_string, NX_WEB_HTTP_SERVER_DEFAULT_MIME, sizeof(NX_WEB_HTTP_SERVER_DEFAULT_MIME)); /* Use case of memcpy is verified. */
    *string_size = sizeof(NX_WEB_HTTP_SERVER_DEFAULT_MIME) - 1;
    return(NX_WEB_HTTP_EXTENSION_MIME_DEFAULT);
}


#ifdef NX_WEB_HTTP_DIGEST_ENABLE

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_nonce_allocate                  PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocate a new nonce for digest authentication.       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    nonce_ptr                             Allocated nonce pointer       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_time_get                           Get system time               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_digest_authenticate                             */
/*                                          Digest authentication         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Yuxin Zhou               Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nx_web_http_server_nonce_allocate(NX_WEB_HTTP_SERVER *server_ptr, NX_WEB_HTTP_SERVER_NONCE **nonce_ptr)
{
UINT i;
UCHAR random_value;
NX_WEB_HTTP_SERVER_NONCE *nonces_list = server_ptr -> nx_web_http_server_nonces;


    /* Search if there is free entry for new nonce.  */
    for (i = 0; i < NX_WEB_HTTP_SERVER_NONCE_MAX; i++)
    {
        if (nonces_list[i].nonce_state == NX_WEB_HTTP_SERVER_NONCE_INVALID)
        {
            *nonce_ptr = &(nonces_list[i]);
            break;
        }
    }

    if (i == NX_WEB_HTTP_SERVER_NONCE_MAX)
    {

        /* If no free entry, check the timeout of allocated nonces.  */
        for (i = 0; i < NX_WEB_HTTP_SERVER_NONCE_MAX; i++)
        {
            if (nonces_list[i].nonce_state == NX_WEB_HTTP_SERVER_NONCE_VALID)
            {
                if (tx_time_get() > nonces_list[i].nonce_timestamp + NX_WEB_HTTP_SERVER_NONCE_TIMEOUT)
                {

                    /* If this nonce is timed out, free up this entry for new nonce.  */
                    *nonce_ptr = &(nonces_list[i]);
                    break;
                }
            }
        }

        /* If no entry can be allocated, return error.  */
        if (i == NX_WEB_HTTP_SERVER_NONCE_MAX)
        {
            return(NX_NOT_FOUND);
        }
    }

    /* Generate new nonce for digest authentication. */
    for (i = 0; i < NX_WEB_HTTP_SERVER_NONCE_SIZE; i++)
    {
        random_value = (UCHAR)NX_RAND() % (sizeof(_nx_web_http_server_base64_array) - 1);
        (*nonce_ptr) -> nonce_buffer[i] = (UCHAR)_nx_web_http_server_base64_array[random_value];
    }

    /* Reset the timestamp and state for the new nonce.  */
    (*nonce_ptr) -> nonce_timestamp = tx_time_get();
    (*nonce_ptr) -> nonce_state = NX_WEB_HTTP_SERVER_NONCE_VALID;

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_digest_authenticate             PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for digest (MD5 only) authentication.          */
/*    If the digest is correct, it returns a success.  Otherwise, it      */
/*    returns an authentication request to the requesting client.         */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    packet_ptr                            Request packet pointer        */
/*    name_ptr                              Pointer to name string        */
/*    password_ptr                          Pointer to password string    */
/*    realm_ptr                             Pointer to realm string       */
/*    auth_request_present                  Indicate if authentication    */
/*                                            must be performed           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nx_web_http_server_retrieve_digest_authorization                    */
/*                                          Pickup authorization          */
/*   _nx_web_http_server_digest_response_calculate                        */
/*                                          Calculate the digest          */
/*    _nx_web_http_server_response_packet_allocate                        */
/*                                          Allocate a response packet    */
/*    nx_packet_data_append                 Append information to response*/
/*    nx_packet_release                     Release packet                */
/*    _nx_web_http_server_send              Send HTTP Server response     */
/*    _nx_utility_string_length_check       Check string length           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_get_process       Process GET request           */
/*    _nx_web_http_server_put_process       Process PUT request           */
/*    _nx_web_http_server_delete_process    Process DELETE request        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported random nonce,     */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_web_http_server_digest_authenticate(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, CHAR *name_ptr, CHAR *password_ptr, CHAR *realm_ptr, UINT *auth_request_present)
{

CHAR        authorization_response[NX_WEB_HTTP_MAX_ASCII_MD5 + 1];
CHAR        calculated_response[NX_WEB_HTTP_MAX_ASCII_MD5 + 1];
CHAR        authorization_uri[NX_WEB_HTTP_MAX_RESOURCE + 1];
CHAR        method[8];
CHAR        quote[2] = {0x22, 0};
CHAR        *buffer_ptr;
UINT        i;
UINT        status, status1, callback_status;
CHAR        crlf[2] = {13,10};
CHAR        authorization_nc[NX_WEB_HTTP_MAX_RESOURCE + 1];
CHAR        authorization_cnonce[NX_WEB_HTTP_MAX_RESOURCE + 1];
UINT        realm_length;
NX_WEB_HTTP_SERVER_NONCE *nonce_ptr = NX_NULL;

    /* Default to no authentication request detected. */
    *auth_request_present =  NX_FALSE;

    /* Default the status to authenticate.  */
    status =  NX_WEB_HTTP_DIGEST_AUTHENTICATE;

    /* Is the authorization request present?  */
    if (_nx_web_http_server_retrieve_digest_authorization(server_ptr, packet_ptr, authorization_response, authorization_uri, authorization_nc, authorization_cnonce, &nonce_ptr))
    {

        /* Yes, an authorization request is present.  */
        *auth_request_present =  NX_TRUE;

        /* Pickup method from the packet.  */
        buffer_ptr =  (CHAR *) packet_ptr -> nx_packet_prepend_ptr;
        i = 0;
        while (((buffer_ptr + i) < (CHAR *)packet_ptr -> nx_packet_append_ptr) && (buffer_ptr[i] != ' ') && (i < (sizeof(method) - 1)))
        {

            /* Copy bytes of method. */
            method[i] =  buffer_ptr[i];
            i++;
        }

        /* Null terminate method.  */
        method[i] =  (CHAR) NX_NULL;

        /* If the digest authenticate callback function is set, invoke the callback function. */
        if(server_ptr -> nx_web_http_server_digest_authenticate_callback)
        {
            callback_status = (server_ptr -> nx_web_http_server_digest_authenticate_callback)(server_ptr, name_ptr, realm_ptr, password_ptr, method, authorization_uri, authorization_nc, authorization_cnonce);
        }
        else
        {

            /* If the digest authenticate callback is not set, assume it is success, for backward
               compatibility reasons. */
            callback_status = NX_SUCCESS;
        }

        /* Calculate what the MD5 should be.  */
        _nx_web_http_server_digest_response_calculate(server_ptr, name_ptr, realm_ptr, password_ptr, (CHAR *)(nonce_ptr -> nonce_buffer), method, authorization_uri, authorization_nc, authorization_cnonce, calculated_response);

        /* Determine if the calculated response is the same as the received response.  */
        i =  0;
        status = NX_SUCCESS;
        while (i < NX_WEB_HTTP_MAX_ASCII_MD5 + 1)
        {
            /* Is there a mismatch?  */
            if (calculated_response[i] != authorization_response[i])
            {

                /* Authorization mismatch. Continue to avoid timing attack. */
                status = NX_WEB_HTTP_DIGEST_AUTHENTICATE;
            }

            /* Otherwise, look at next character.  */
            i++;
        }

        /* If the response is authenticated, mark the nonce as accepted.  */
        if (status == NX_SUCCESS)
        {

            /* If another session uses the same nonce, don't accept it.  */
            if (nonce_ptr -> nonce_state == NX_WEB_HTTP_SERVER_NONCE_ACCEPTED)
            {
                if (nonce_ptr -> nonce_session_ptr != server_ptr -> nx_web_http_server_current_session_ptr)
                {
                    status = NX_WEB_HTTP_DIGEST_AUTHENTICATE;
                }
            }
            else
            {

                /* Update nonce state and set the session pointer for mapping in disconnection.  */
                nonce_ptr -> nonce_state = NX_WEB_HTTP_SERVER_NONCE_ACCEPTED;
                nonce_ptr -> nonce_session_ptr = server_ptr -> nx_web_http_server_current_session_ptr;
            }
        }
        else
        {
            nonce_ptr -> nonce_state = NX_WEB_HTTP_SERVER_NONCE_INVALID;
        }

        /* If digest authenticate callback function returns non-success value, the request is 
           considered unauthenticated. */
        if(callback_status != NX_SUCCESS)
            status = NX_WEB_HTTP_DIGEST_AUTHENTICATE;
    }

    /* Determine if we need to send back an unauthorized request.  */
    if (status == NX_WEB_HTTP_DIGEST_AUTHENTICATE)
    {

        /* Allocate a new nonce for digest authentication.  */
        status1 = _nx_web_http_server_nonce_allocate(server_ptr, &nonce_ptr);

        /* Determine if an error occurred in the packet allocation.  */
        if (status1)
        {

            /* Send response back to HTTP Client.  */
            _nx_web_http_server_response_send(server_ptr, NX_WEB_HTTP_STATUS_INTERNAL_ERROR, sizeof(NX_WEB_HTTP_STATUS_INTERNAL_ERROR) - 1, 
                                              "NetX HTTP Server Internal Error", sizeof("NetX HTTP Server Internal Error") - 1, NX_NULL, 0);

            /* Indicate an allocation error occurred.  */
            server_ptr -> nx_web_http_server_allocation_errors++;

            /* Return the internal NetX error.  */
            return(status1);
        }

        /* We need authorization so build the HTTP 401 Unauthorized message to send to the server.  */

        /* Check string length.  */
        if (_nx_utility_string_length_check(realm_ptr, &realm_length, NX_MAX_STRING_LENGTH))
        {
            return(NX_WEB_HTTP_ERROR);
        }

        /* Allocate a packet for sending the response back.  */
        status1 =  _nx_web_http_server_response_packet_allocate(server_ptr, &packet_ptr, NX_WAIT_FOREVER);

        /* Determine if an error occurred in the packet allocation.  */
        if (status1 != NX_SUCCESS)
        {

            /* Indicate an allocation error occurred.  */
            server_ptr -> nx_web_http_server_allocation_errors++;

            /* Return the internal NetX error.  */
            return(status1);
        }

        /* Insert the response header.  */
        nx_packet_data_append(packet_ptr, NX_WEB_HTTP_VERSION, sizeof(NX_WEB_HTTP_VERSION) - 1,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
        nx_packet_data_append(packet_ptr, " ", 1, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
        nx_packet_data_append(packet_ptr, NX_WEB_HTTP_STATUS_UNAUTHORIZED, sizeof(NX_WEB_HTTP_STATUS_UNAUTHORIZED) - 1,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place the <cr,lf> into the buffer.  */
        nx_packet_data_append(packet_ptr, crlf, 2,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Insert the type of authentication requested.  */
        nx_packet_data_append(packet_ptr, "WWW-Authenticate: Digest realm=", 31,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Insert the double quote.  */
        nx_packet_data_append(packet_ptr, quote, 1,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place the realm string into the buffer.  */
        nx_packet_data_append(packet_ptr, realm_ptr, realm_length,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Insert the double quote.  */
        nx_packet_data_append(packet_ptr, quote, 1,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place a comma into the buffer.  */
        nx_packet_data_append(packet_ptr, ",", 1,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Insert the algorithm into the buffer.  */
        nx_packet_data_append(packet_ptr, " algorithm=", 11,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Insert the double quote.  */
        nx_packet_data_append(packet_ptr, quote, 1,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Insert the md5 tag into the buffer.  */
        nx_packet_data_append(packet_ptr, "md5", 3,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Insert the double quote.  */
        nx_packet_data_append(packet_ptr, quote, 1,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place a comma into the buffer.  */
        nx_packet_data_append(packet_ptr, ",", 1,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Insert the nonce into the buffer.  */
        nx_packet_data_append(packet_ptr, " nonce=", 7,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Insert the double quote.  */
        nx_packet_data_append(packet_ptr, quote, 1,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place the nonce string into the buffer.  */
        nx_packet_data_append(packet_ptr, nonce_ptr -> nonce_buffer, NX_WEB_HTTP_SERVER_NONCE_SIZE,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Insert the double quote.  */
        nx_packet_data_append(packet_ptr, quote, 1,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place the qop="auth" parameter string into the buffer.  */
        nx_packet_data_append(packet_ptr, ", qop=\"auth\"", 12,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place the <cr,lf> into the buffer.  */
        nx_packet_data_append(packet_ptr, crlf, 2,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Set Content-Length as 0.  */
        nx_packet_data_append(packet_ptr, "Content-Length: 0", 17,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place the <cr,lf> into the buffer.  */
        nx_packet_data_append(packet_ptr, crlf, 2,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place another <cr,lf> into the buffer to signal end of FULL HTTP response.  */
        nx_packet_data_append(packet_ptr, crlf, 2,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Send the response back to the client.  */
        status1 = _nx_web_http_server_send(server_ptr, packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_SEND);

        /* Check for an error.  */
        if (status1)
        {

            /* Set the internal NetX error as the status return. */
            status = status1;

            /* Just release the packet.  */
            nx_packet_release(packet_ptr);
        }
    }

    /* Return the result of the authentication request.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_digest_response_calculate       PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function computes the MD5 digest based on the supplied input   */
/*    parameters.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    username                              Username string               */
/*    realm                                 Realm string                  */
/*    password                              Password string               */
/*    nonce                                 Authentication nonce string   */
/*    method                                Request method string         */
/*    uri                                   Resource request string       */
/*    nc                                    Nonce count string            */
/*    cnonce                                Client nonce string           */
/*    result                                Computed digest string        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nx_web_http_server_hex_ascii_convert  Convert hex to ASCII          */
/*   _nx_md5_initialize                     Initialize MD5 algorithm      */
/*   _nx_md5_update                         Update MD5 digest             */
/*   _nx_md5_digest_calculate               Complete the MD5 algorithm    */
/*    _nx_utility_string_length_check       Check string length           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_digest_authenticate                             */
/*                                          Digest authentication         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported random nonce,     */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
VOID  _nx_web_http_server_digest_response_calculate(NX_WEB_HTTP_SERVER *server_ptr, CHAR *username, CHAR *realm, CHAR *password, CHAR *nonce, CHAR *method, CHAR *uri, CHAR *nc, CHAR *cnonce, CHAR *result)
{

CHAR    md5_binary[NX_WEB_HTTP_MAX_BINARY_MD5];
CHAR    ha1_string[NX_WEB_HTTP_MAX_ASCII_MD5 + 1];
CHAR    ha2_string[NX_WEB_HTTP_MAX_ASCII_MD5 + 1];
UINT    username_length;
UINT    password_length;
UINT    realm_length;
UINT    method_length;
UINT    uri_length;
UINT    nc_length;
UINT    cnonce_length;

    /* Check string length.  */
    if (_nx_utility_string_length_check(username, &username_length, NX_WEB_HTTP_MAX_NAME) ||
        _nx_utility_string_length_check(password, &password_length, NX_WEB_HTTP_MAX_PASSWORD) ||
        _nx_utility_string_length_check(realm, &realm_length, NX_MAX_STRING_LENGTH) ||
        _nx_utility_string_length_check(method, &method_length, 7) ||
        _nx_utility_string_length_check(uri, &uri_length, NX_WEB_HTTP_MAX_RESOURCE) ||
        _nx_utility_string_length_check(nc, &nc_length, NX_WEB_HTTP_MAX_RESOURCE) ||
        _nx_utility_string_length_check(cnonce, &cnonce_length, NX_WEB_HTTP_MAX_RESOURCE))
    {
        return;
    }


    /* Calculate the H(A1) portion of the digest.  */
    _nx_md5_initialize(&(server_ptr -> nx_web_http_server_md5data));
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) username, username_length);
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) ":", 1);
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) realm, realm_length);
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) ":", 1);
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) password, password_length);
    _nx_md5_digest_calculate(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) md5_binary);

    /* Convert this H(A1) portion to ASCII Hex representation.  */
    _nx_web_http_server_hex_ascii_convert(md5_binary, NX_WEB_HTTP_MAX_BINARY_MD5, ha1_string);

    /* Make the H(A2) portion of the digest.  */
    _nx_md5_initialize(&(server_ptr -> nx_web_http_server_md5data));
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) method, method_length);
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) ":", 1);
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) uri, uri_length);
    _nx_md5_digest_calculate(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) md5_binary);

    /* Convert this H(A2) portion to ASCII Hex representation.  */
    _nx_web_http_server_hex_ascii_convert(md5_binary, NX_WEB_HTTP_MAX_BINARY_MD5, ha2_string);

    /* Now make the final MD5 digest.  */
    _nx_md5_initialize(&(server_ptr -> nx_web_http_server_md5data));
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) ha1_string, sizeof(ha1_string) - 1);
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) ":", 1);
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) nonce, NX_WEB_HTTP_SERVER_NONCE_SIZE);

    /* Start of Internet Explorer bug work-around.  */
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) ":", 1);
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) nc, nc_length);
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) ":", 1);
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) cnonce, cnonce_length);
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) ":auth", 5);
    /* End of Internet Explorer bug work-around.  */

    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) ":", 1);
    _nx_md5_update(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) ha2_string, sizeof(ha2_string) - 1);
    _nx_md5_digest_calculate(&(server_ptr -> nx_web_http_server_md5data), (unsigned char *) md5_binary);

    /* Finally, convert the response back to an ASCII string and place in
       the destination.  */
    _nx_web_http_server_hex_ascii_convert(md5_binary, NX_WEB_HTTP_MAX_BINARY_MD5, result);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_retrieve_digest_authorization   PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves the MD5 digest parameters from the          */
/*    supplied request packet.                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            HTTP Server pointer           */
/*    packet_ptr                            Request packet pointer        */
/*    response                              Digest response pointer       */
/*    uri                                   URI from response pointer     */
/*    nc                                    Nonce count string            */
/*    cnonce                                Client nonce string           */
/*    nonce_ptr                             Server nonce pointer          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    length                                Length of response (should be */
/*                                            32). A value of 0 indicates */
/*                                            an error is present         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_digest_authenticate                             */
/*                                          Digest authentication         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported random nonce,     */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_web_http_server_retrieve_digest_authorization(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, CHAR *response, CHAR *uri, CHAR *nc, CHAR *cnonce, NX_WEB_HTTP_SERVER_NONCE **nonce_ptr)
{

UINT    length;
UINT    found;
CHAR    *buffer_ptr;
CHAR    *saved_buffer_ptr;
UCHAR   *nonce_buffer;
UINT    i;


    /* Set the found flag to false.  */
    found =  NX_FALSE;

    /* Default the authorization request to zero.  */
    length =  0;

    /* Set the response and uri strings to NULL.  */
    response[0] =  NX_NULL;
    uri[0] =       NX_NULL;

    /* Internet Explorer bug work-around.  */
    nc[0] =        NX_NULL;
    cnonce[0] =    NX_NULL;

    /* Setup pointer to buffer.  */
    buffer_ptr =  (CHAR *) packet_ptr -> nx_packet_prepend_ptr;

    /* Find the "Authorization: " token first.  */
    while (((buffer_ptr+15) < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0))
    {

        /* Check for the Authorization: token.  */
        if (((*buffer_ptr ==      'a') || (*buffer_ptr ==      'A')) &&
            ((*(buffer_ptr+1) ==  'u') || (*(buffer_ptr+1) ==  'U')) &&
            ((*(buffer_ptr+2) ==  't') || (*(buffer_ptr+2) ==  'T')) &&
            ((*(buffer_ptr+3) ==  'h') || (*(buffer_ptr+3) ==  'H')) &&
            ((*(buffer_ptr+4) ==  'o') || (*(buffer_ptr+4) ==  'O')) &&
            ((*(buffer_ptr+5) ==  'r') || (*(buffer_ptr+5) ==  'R')) &&
            ((*(buffer_ptr+6) ==  'i') || (*(buffer_ptr+6) ==  'I')) &&
            ((*(buffer_ptr+7) ==  'z') || (*(buffer_ptr+7) ==  'Z')) &&
            ((*(buffer_ptr+8) ==  'a') || (*(buffer_ptr+8) ==  'A')) &&
            ((*(buffer_ptr+9) ==  't') || (*(buffer_ptr+9) ==  'T')) &&
            ((*(buffer_ptr+10) == 'i') || (*(buffer_ptr+10) == 'I')) &&
            ((*(buffer_ptr+11) == 'o') || (*(buffer_ptr+11) == 'O')) &&
            ((*(buffer_ptr+12) == 'n') || (*(buffer_ptr+12) == 'N')) &&
            (*(buffer_ptr+13) == ':') &&
            (*(buffer_ptr+14) == ' '))
        {

            /* Move the pointer up to the length token.  */
            buffer_ptr =  buffer_ptr + 15;

            /* Set the found flag.  */
            found =  NX_TRUE;
            break;
        }

        /* Move the pointer up to the next character.  */
        buffer_ptr++;
    }

    /* Determine if the first token was found.  */
    if (!found)
    {

        /* No, authorization is not present.  Return a zero length.  */
        return(length);
    }

    /* Set the found flag back to false.  */
    found =  NX_FALSE;

    /* Now remove any extra blanks.  */
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr == ' '))
    {

        /* Move the pointer up one character.  */
        buffer_ptr++;
    }

    /* Now check for the "Digest " token.  */
    while (((buffer_ptr+7) < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0))
    {

        /* Check for the Digest token.  */
        if (((*buffer_ptr ==      'd') || (*buffer_ptr ==      'D')) &&
            ((*(buffer_ptr+1) ==  'i') || (*(buffer_ptr+1) ==  'I')) &&
            ((*(buffer_ptr+2) ==  'g') || (*(buffer_ptr+2) ==  'G')) &&
            ((*(buffer_ptr+3) ==  'e') || (*(buffer_ptr+3) ==  'E')) &&
            ((*(buffer_ptr+4) ==  's') || (*(buffer_ptr+4) ==  'S')) &&
            ((*(buffer_ptr+5) ==  't') || (*(buffer_ptr+5) ==  'T')) &&
            (*(buffer_ptr+6) == ' '))
        {

            /* Move the pointer up to the actual authorization string.  */
            buffer_ptr =  buffer_ptr + 7;

            /* Set the found flag.  */
            found =  NX_TRUE;
            break;
        }

        /* Move the pointer up to the next character.  */
        buffer_ptr++;
    }

    /* Determine if the second token was found.  */
    if (!found)
    {

        /* No, digest is not present.  Return a zero length.  */
        return(length);
    }

    /* Now remove any extra blanks.  */
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr == ' '))
    {

        /* Move the pointer up one character.  */
        buffer_ptr++;
    }

    /* Start of Internet Explorer bug work-around (Parses nc and cnonce parameters).  */

    /* Save current buffer pointer, so each parameter search always starts from here.  */
    saved_buffer_ptr =  buffer_ptr;

    while (((buffer_ptr + 6) < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0))
    {

        /* Check for the uri token.  */
        if (((*(buffer_ptr) ==  'n') || (*(buffer_ptr) ==  'N')) &&
            ((*(buffer_ptr+1) ==  'o') || (*(buffer_ptr+1) ==  'O')) &&
            ((*(buffer_ptr+2) ==  'n') || (*(buffer_ptr+2) ==  'N')) &&
            ((*(buffer_ptr+3) ==  'c') || (*(buffer_ptr+3) ==  'C')) &&
            ((*(buffer_ptr+4) ==  'e') || (*(buffer_ptr+4) ==  'E')) &&
            (*(buffer_ptr+5) == '='))
        {

            /* Move the pointer up to the actual nonce string.  */
            buffer_ptr =  buffer_ptr + 6;
            found = NX_TRUE;

            break;
        }

        /* Move the pointer up to the next character.  */
        buffer_ptr++;
    }

    /* Check if nonce is found.  */
    if (!found)
    {
        return(0);
    }

    /* Now remove any extra blanks and quotes.  */
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && ((*buffer_ptr == ' ') || (*buffer_ptr == (CHAR) 0x22)))
    {

        /* Move the pointer up one character.  */
        buffer_ptr++;
    }

    /* Now pickup the nonce string.  */
    length =  0;
    nonce_buffer = (UCHAR *)buffer_ptr;
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0) && (*buffer_ptr != ' ') && (*buffer_ptr != (CHAR) 13))
    {

        /* Determine if the ending quote is present.  */
        if (*buffer_ptr == (CHAR) 0x22)
        {
            break;
        }

        /* Increase the length.  */
        length++;
        buffer_ptr++;
    }

    /* Check the nonce size.  */
    if (length != NX_WEB_HTTP_SERVER_NONCE_SIZE)
    {
        return(0);
    }

    /* Check if the nonce is valid.  */
    for (i = 0; i < NX_WEB_HTTP_SERVER_NONCE_MAX; i++)
    {
        if ((server_ptr -> nx_web_http_server_nonces[i].nonce_state != NX_WEB_HTTP_SERVER_NONCE_INVALID) &&
            (memcmp(server_ptr -> nx_web_http_server_nonces[i].nonce_buffer, nonce_buffer, NX_WEB_HTTP_SERVER_NONCE_SIZE) == 0)) /* Use case of memcmp is verified. */
        {
            *nonce_ptr = &(server_ptr -> nx_web_http_server_nonces[i]);
            break;
        }
    }

    /* If the nonca is invalid, just return.  */
    if (i == NX_WEB_HTTP_SERVER_NONCE_MAX)
    {
        return(0);
    }

    /* Get saved buffer pointer.  */
    buffer_ptr =  saved_buffer_ptr;

    /* Now look for the nc in the digest response.  */
    while (((buffer_ptr+3) < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0))
    {

        /* Check for the nc token.  */
        if (((*buffer_ptr ==      'n') || (*buffer_ptr ==      'N')) &&
            ((*(buffer_ptr+1) ==  'c') || (*(buffer_ptr+1) ==  'C')) &&
            (*(buffer_ptr+2) == '='))
        {

            /* Move the pointer up to the actual authorization string.  */
            buffer_ptr =  buffer_ptr + 3;

            break;
        }

        /* Move the pointer up to the next character.  */
        buffer_ptr++;
    }

    /* Now remove any extra blanks and quotes (should be no starting quote).  */
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && ((*buffer_ptr == ' ') || (*buffer_ptr == (CHAR) 0x22)))
    {

        /* Move the pointer up one character.  */
        buffer_ptr++;
    }

    /* Now pickup the nc string (should be 8 hex characters; should not be quoted).  */
    length =  0;
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0) && (*buffer_ptr != ' ') && (*buffer_ptr != (CHAR) 13) && (length < NX_WEB_HTTP_MAX_RESOURCE))
    {

        /* Determine if the ending quote or comma is present (should be no ending quote).  */
        if ((*buffer_ptr == (CHAR) 0x22) || (*buffer_ptr == ','))
        {

            break;
        }

        /* Copy a character of the authorization string into the destination.  */
        nc[length++] =  *buffer_ptr++;
    }

    /* Null terminate the NC.  */
    nc[length] =  (CHAR) NX_NULL;


    /* Get saved buffer pointer.  */
    buffer_ptr =  saved_buffer_ptr;

    /* Now look for the cnonce in the digest response.  */
    while (((buffer_ptr+7) < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0))
    {

        /* Check for the uri token.  */
        if (((*buffer_ptr ==      'c') || (*buffer_ptr ==      'C')) &&
            ((*(buffer_ptr+1) ==  'n') || (*(buffer_ptr+1) ==  'N')) &&
            ((*(buffer_ptr+2) ==  'o') || (*(buffer_ptr+2) ==  'O')) &&
            ((*(buffer_ptr+3) ==  'n') || (*(buffer_ptr+3) ==  'N')) &&
            ((*(buffer_ptr+4) ==  'c') || (*(buffer_ptr+4) ==  'C')) &&
            ((*(buffer_ptr+5) ==  'e') || (*(buffer_ptr+5) ==  'E')) &&
            (*(buffer_ptr+6) == '='))
        {

            /* Move the pointer up to the actual authorization string.  */
            buffer_ptr =  buffer_ptr + 7;

            break;
        }

        /* Move the pointer up to the next character.  */
        buffer_ptr++;
    }

    /* Now remove any extra blanks and quotes.  */
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && ((*buffer_ptr == ' ') || (*buffer_ptr == (CHAR) 0x22)))
    {

        /* Move the pointer up one character.  */
        buffer_ptr++;
    }

    /* Now pickup the cnonce string.  */
    length =  0;
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0) && (*buffer_ptr != ' ') && (*buffer_ptr != (CHAR) 13) && (length < NX_WEB_HTTP_MAX_RESOURCE))
    {

        /* Determine if the ending quote is present.  */
        if (*buffer_ptr == (CHAR) 0x22)
        {

            break;
        }

        /* Copy a character of the authorization string into the destination.  */
        cnonce[length++] =  *buffer_ptr++;
    }

    /* Null terminate the CNONCE.  */
    cnonce[length] =  (CHAR) NX_NULL;

    /* End of Internet Explorer bug work-around.  */

    /* Get saved buffer pointer.  */
    buffer_ptr =  saved_buffer_ptr;

    /* Now look for the uri in the digest response.  */
    while (((buffer_ptr+4) < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0))
    {

        /* Check for the uri token.  */
        if (((*buffer_ptr ==      'u') || (*buffer_ptr ==      'U')) &&
            ((*(buffer_ptr+1) ==  'r') || (*(buffer_ptr+1) ==  'R')) &&
            ((*(buffer_ptr+2) ==  'i') || (*(buffer_ptr+2) ==  'I')) &&
            (*(buffer_ptr+3) == '='))
        {

            /* Move the pointer up to the actual authorization string.  */
            buffer_ptr =  buffer_ptr + 4;

            break;
        }

        /* Move the pointer up to the next character.  */
        buffer_ptr++;
    }

    /* Now remove any extra blanks and quotes.  */
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && ((*buffer_ptr == ' ') || (*buffer_ptr == (CHAR) 0x22)))
    {

        /* Move the pointer up one character.  */
        buffer_ptr++;
    }

    /* Now pickup the uri string.  */
    length =  0;
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0) && (*buffer_ptr != ' ') && (*buffer_ptr != (CHAR) 13) && (length < NX_WEB_HTTP_MAX_RESOURCE))
    {

        /* Determine if the ending quote is present.  */
        if (*buffer_ptr == (CHAR) 0x22)
        {

            break;
        }

        /* Copy a character of the authorization string into the destination.  */
        uri[length++] =  *buffer_ptr++;
    }

    /* Null terminate the URI.  */
    uri[length] =  (CHAR) NX_NULL;

    /* Get saved buffer pointer.  */
    buffer_ptr =  saved_buffer_ptr;

    /* Now look for the digest response.  */
    while (((buffer_ptr+9) < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0))
    {

        /* Check for the uri token.  */
        if (((*buffer_ptr ==      'r') || (*buffer_ptr ==      'R')) &&
            ((*(buffer_ptr+1) ==  'e') || (*(buffer_ptr+1) ==  'E')) &&
            ((*(buffer_ptr+2) ==  's') || (*(buffer_ptr+2) ==  'S')) &&
            ((*(buffer_ptr+3) ==  'p') || (*(buffer_ptr+3) ==  'P')) &&
            ((*(buffer_ptr+4) ==  'o') || (*(buffer_ptr+4) ==  'O')) &&
            ((*(buffer_ptr+5) ==  'n') || (*(buffer_ptr+5) ==  'N')) &&
            ((*(buffer_ptr+6) ==  's') || (*(buffer_ptr+6) ==  'S')) &&
            ((*(buffer_ptr+7) ==  'e') || (*(buffer_ptr+7) ==  'E')) &&
            (*(buffer_ptr+8) == '='))
        {

            /* Move the pointer up to the actual authorization string.  */
            buffer_ptr =  buffer_ptr + 9;

            break;
        }

        /* Move the pointer up to the next character.  */
        buffer_ptr++;
    }

    /* Now remove any extra blanks and leading quote.  */
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && ((*buffer_ptr == ' ') || (*buffer_ptr == (CHAR) 0x22)))
    {

        /* Move the pointer up one character.  */
        buffer_ptr++;
    }

    /* Finally, pickup the response.  */
    length =  0;
    while ((buffer_ptr < (CHAR *) packet_ptr -> nx_packet_append_ptr) && (*buffer_ptr != (CHAR) 0) && (*buffer_ptr != ' ') && (*buffer_ptr != (CHAR) 13) && (*buffer_ptr != (CHAR) 0x3A) && (length < NX_WEB_HTTP_MAX_ASCII_MD5))
    {

        /* Determine if the ending quote is present.  */
        if (*buffer_ptr == (CHAR) 0x22)
        {

            break;
        }

        /* Copy a character of the response string into the destination.  */
        response[length++] =  *buffer_ptr++;
    }

    /* Null terminate response.  */
    response[length] =  (CHAR) NX_NULL;

    /* Check for an error.  */
    if ((length != 32) || (buffer_ptr >= (CHAR *) packet_ptr -> nx_packet_append_ptr) || (*buffer_ptr != (CHAR) 0x22))
    {

        /* Error, clear the length and the response string.  */
        length =  0;
        response[0] =  (CHAR) NX_NULL;
    }

    /* Return the length to the caller.  */
    return(length);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_hex_ascii_convert               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts hexadecimal characters into an ASCII string. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    source                                Source hex string             */
/*    source_length                         Length of source string       */
/*    destination                           Pointer to destination string */
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
/*    _nx_web_http_server_digest_response_calculate                       */
/*                                              Digest authentication     */
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
VOID  _nx_web_http_server_hex_ascii_convert(CHAR *source, UINT source_length, CHAR *destination)
{

UINT    i,j;
CHAR    digit;


    /* Setup destination index.  */
    j =  0;

    /* Loop to process the entire source string.  */
    for (i = 0; i < source_length; i++)
    {

        /* Pickup the first nibble.  */
        digit =  (source[i] >> 4) & 0xF;

        /* Convert to ASCII and store.  */
        if (digit <= 9)
            destination[j++] =  (CHAR)(digit + '0');
        else
            destination[j++] =  (CHAR)(digit + 'a' - 10);

        /* Pickup the second nibble.  */
        digit =  source[i] & 0xF;

        /* Convert to ASCII and store.  */
        if (digit <= 9)
            destination[j++] =  (CHAR)(digit + '0');
        else
            destination[j++] =  (CHAR)(digit + 'a' - 10);
    }

    /* Finally, place a NULL in the destination string.  */
    destination[j] =  (CHAR) NX_NULL;
}
#endif


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_server_get_entity_header              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks error for getting the entity header.           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    packet_pptr                           Pointer to packet             */ 
/*    entity_header_buffer                  Buffer to get entity header   */ 
/*    buffer_size                           Size of buffer                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_get_entity_header                               */
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
UINT  _nxe_web_http_server_get_entity_header(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, UCHAR *entity_header_buffer, ULONG buffer_size)
{

#ifdef  NX_WEB_HTTP_MULTIPART_ENABLE
    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) ||
        (packet_pptr == NX_NULL) || (entity_header_buffer == NX_NULL))
        return NX_PTR_ERROR;

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    return _nx_web_http_server_get_entity_header(server_ptr, packet_pptr, entity_header_buffer, buffer_size);
#else
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(packet_pptr);
    NX_PARAMETER_NOT_USED(entity_header_buffer);
    NX_PARAMETER_NOT_USED(buffer_size);

    return NX_WEB_HTTP_ERROR;
#endif /* NX_WEB_HTTP_MULTIPART_ENABLE */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_get_entity_header               PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function gets the entity header and skip header.               */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    packet_pptr                           Pointer to packet             */ 
/*    entity_header_buffer                  Buffer to get entity header   */ 
/*    buffer_size                           Size of buffer                */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_field_value_get                                 */
/*    _nx_web_http_server_memicmp                                         */
/*    _nx_web_http_server_boundary_find                                   */
/*    nx_packet_release                                                   */
/*    nx_tcp_socket_receive                                               */
/*    _nx_utility_string_length_check                                     */
/*    memmove                                                             */
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed write underflow,      */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memmove use cases, */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_web_http_server_get_entity_header(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, UCHAR *entity_header_buffer, ULONG buffer_size)
{

#ifdef  NX_WEB_HTTP_MULTIPART_ENABLE
NX_WEB_HTTP_SERVER_MULTIPART   *multipart_ptr;
UINT                        status;
UINT                        offset;
NX_PACKET                  *available_packet;
UINT                        crlf_match_count = 0;
UINT                        end_boundary_count = 2;
UINT                        skip_count = 2;
UCHAR                      *ch;
UINT                        index;

    /* Get multipart context. */
    multipart_ptr = &server_ptr -> nx_web_http_server_multipart;

    /* Is the multipart context initialized? */
    if(multipart_ptr -> nx_web_http_server_multipart_boundary[0] == 0)
    {
    ULONG   field_length;
    UINT    quotation_index;
    UINT    i;

        /* No, it is not initialized. */

        /* Get content type. */
        status = _nx_web_http_server_field_value_get(*packet_pptr, (UCHAR *)"content-type", 12,
                                                 multipart_ptr -> nx_web_http_server_multipart_boundary, NX_WEB_HTTP_MAX_HEADER_FIELD + 1);
        if(status)
            return status;

        /* Is the content type "multipart"? */
        /* Check the length. The length of "multipart/" is 10. */
        if (_nx_utility_string_length_check((CHAR *)multipart_ptr -> nx_web_http_server_multipart_boundary, (UINT *)&field_length, NX_WEB_HTTP_MAX_HEADER_FIELD))
            return NX_WEB_HTTP_ERROR;
        if(field_length < 10)
            return NX_WEB_HTTP_NOT_FOUND;

        /* Check the data. */
        if(_nx_web_http_server_memicmp(multipart_ptr -> nx_web_http_server_multipart_boundary, 10,
                                   (UCHAR *)"multipart/", 10) != NX_SUCCESS) 
            return NX_WEB_HTTP_NOT_FOUND;

        /* Find the boundary. The length of "boundary=" is 9. */
        index = 0;
        while((index + 9) < field_length)
        {
            if(_nx_web_http_server_memicmp(&multipart_ptr -> nx_web_http_server_multipart_boundary[index], 9,
                                       (UCHAR *)"boundary=", 9) == NX_SUCCESS)
            {

                break;
            }

            /* Move the pointer up to the next character.  */
            index++;
        }

        /* Move the pointer up to the boundary value. */
        index += 9;

        /* Is boundary string found? */
        if((field_length - index) <= 1)
        {

            /* Boundary not found or no boundary value. */
            return NX_WEB_HTTP_NOT_FOUND;
        }

        /* Boundary is started with "CRLF--". */
        multipart_ptr -> nx_web_http_server_multipart_boundary[0] = 13;
        multipart_ptr -> nx_web_http_server_multipart_boundary[1] = 10;
        multipart_ptr -> nx_web_http_server_multipart_boundary[2] = '-';
        multipart_ptr -> nx_web_http_server_multipart_boundary[3] = '-';

        /* Remove quotation. */
        if(multipart_ptr -> nx_web_http_server_multipart_boundary[index] == '"')
        {

            /* Find the right quotation. */
            index++;
            for(quotation_index = field_length; quotation_index > index; quotation_index--)
            {
                if(multipart_ptr -> nx_web_http_server_multipart_boundary[quotation_index] == '"')
                    break;
            }

            /* Is the right quotation found? */
            if(quotation_index > index)
                multipart_ptr -> nx_web_http_server_multipart_boundary[quotation_index] = 0;
            else
            {

                /* Boundary not valid. */
                return NX_WEB_HTTP_NOT_FOUND;
            }

            /* Leave boundary string only. */
            memmove(&multipart_ptr -> nx_web_http_server_multipart_boundary[4],
                    &multipart_ptr -> nx_web_http_server_multipart_boundary[index],
                    quotation_index - index + 1); /* Use case of memmove is verified.  */
        }
        else
        {

            /* Directly copy boundary string to the head field. */
            i = 4;
            index -= 4;
            do
            {
                multipart_ptr -> nx_web_http_server_multipart_boundary[i] = multipart_ptr -> nx_web_http_server_multipart_boundary[index + i];
                i++;
            }while((multipart_ptr -> nx_web_http_server_multipart_boundary[i - 1] != 0) &&
                   (multipart_ptr -> nx_web_http_server_multipart_boundary[i - 1] != ' '));

            /* Set the terminal character. */
            multipart_ptr -> nx_web_http_server_multipart_boundary[i - 1] = 0;
        }
    }

    /* If the received request is chunked.  */
    if (server_ptr -> nx_web_http_server_request_chunked)
    {

        /* If never processed the chunked packet, need to separate the HTTP header and content.  */
        if (!server_ptr -> nx_web_http_server_expect_receive_bytes)
        {

            available_packet = *packet_pptr;

            /* Separate the HTTP header and get the first content packet.  */
            status = _nx_web_http_server_packet_content_find(server_ptr, &available_packet, NX_NULL);
            if (status)
            {
                return(status);
            }

            /* Append '\r\n' to the start of the packet.  */
            available_packet -> nx_packet_prepend_ptr -= 2;
            available_packet -> nx_packet_prepend_ptr[0] = 13;
            available_packet -> nx_packet_prepend_ptr[1] = 10;

            /* Update packet length.  */
            available_packet -> nx_packet_length += 2;

            /* Store the packet. */
            nx_packet_release(*packet_pptr);
            *packet_pptr = available_packet;
            server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_last_packet = available_packet;

            /* Reset the offset. */
            multipart_ptr -> nx_web_http_server_multipart_available_offset = 0;
        }
    }

    /* Find the boundary. */
    while(multipart_ptr -> nx_web_http_server_multipart_boundary_find == NX_FALSE)
    {
        if(_nx_web_http_server_boundary_find(server_ptr, packet_pptr) != NX_SUCCESS)
            break;
    }

    /* Whether the boundary is found? */
    if(multipart_ptr -> nx_web_http_server_multipart_boundary_find == NX_FALSE)
        return NX_WEB_HTTP_NOT_FOUND;

    /* Update the packet and offset. */
    _nx_web_http_server_boundary_find(server_ptr, packet_pptr);

    /* Get offset of available data in the packet.  */
    offset = multipart_ptr -> nx_web_http_server_multipart_available_offset;

    /* Skip data that are not header. */
    available_packet = *packet_pptr;

    if(offset >= available_packet -> nx_packet_length)
    {

        /* Release the packet that has been processed. */
        nx_packet_release(*packet_pptr);

        /* All data has been processed. Get a new packet. */
        if(_nx_web_http_server_packet_get(server_ptr, &available_packet) != NX_SUCCESS)
        {

            /* Error, return to caller.  */
            return(NX_WEB_HTTP_TIMEOUT);
        }

        /* Store the packet. */
        *packet_pptr = available_packet;
        server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_last_packet = available_packet;
    }
#ifndef NX_DISABLE_PACKET_CHAIN
    else
    {
        while((UINT)(available_packet -> nx_packet_append_ptr - available_packet -> nx_packet_prepend_ptr) <= offset)
        {
            offset -= (UINT)(available_packet -> nx_packet_append_ptr - available_packet -> nx_packet_prepend_ptr);

            /* Get next packet. */
            available_packet = available_packet -> nx_packet_next;
        }
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Get entity header. */
    ch = available_packet -> nx_packet_prepend_ptr + offset;
    index = 0;
    while((entity_header_buffer == NX_NULL) || (index < buffer_size))
    {

        /* Calculate valid CRLF. */
        if((*ch == 13) && ((crlf_match_count & 1) == 0))
            crlf_match_count++;
        else if((*ch == 10) && ((crlf_match_count & 1) == 1))
            crlf_match_count++;
        else if(*ch == '-')
        {

            /* Check whether the last boundary is found. */
            if(end_boundary_count < 3)
                end_boundary_count--;

            if(end_boundary_count == 0)
                break;
        }
        else
        {
            end_boundary_count = 0xFF;
            crlf_match_count = 0;
        }

        /* Copy data to buffer. */
        if(skip_count >0)
            skip_count--;
        else
        {
            if(entity_header_buffer)
                entity_header_buffer[index] = *ch;
            index++;
        }
        multipart_ptr -> nx_web_http_server_multipart_available_offset++;

        /* Check whether two CRLFs are found. */
        if(crlf_match_count == 4)
            break;

        /* Get next character. */
        if((ch + 1) >= available_packet -> nx_packet_append_ptr)
        {

            /* ch points to the end of this packet. */
#ifndef NX_DISABLE_PACKET_CHAIN
            if(available_packet -> nx_packet_next)
            {

                /* Copy data from next packet. */
                available_packet = available_packet -> nx_packet_next;
            }
            else
#endif /* NX_DISABLE_PACKET_CHAIN */
            {

                /* Release the packet that has been processed. */
                nx_packet_release(*packet_pptr);

                /* Get a new packet and copy data from it. */
                status = _nx_web_http_server_packet_get(server_ptr, &available_packet);

                /* Check the return status.  */
                if (status != NX_SUCCESS)
                {

                    /* Error, return to caller.  */
                    return(NX_WEB_HTTP_TIMEOUT);
                }

                /* Store the packet. */
                *packet_pptr = available_packet;
                server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_last_packet = available_packet;

                /* Reset the offset. */
                multipart_ptr -> nx_web_http_server_multipart_available_offset = 0;
            }

            ch = available_packet -> nx_packet_prepend_ptr;
        }
        else
            ch++;

    }

    /* Check whether two CRLFs are found. */
    if(crlf_match_count == 4)
    {

        /* Set terminal 0. */
        if(entity_header_buffer)
        {
            if (index >= 4)
            {
                entity_header_buffer[index - 4] = 0;
            }
            else
            {
                entity_header_buffer[0] = 0;
            }
        }
        return NX_SUCCESS;
    }
    else
        return NX_WEB_HTTP_NOT_FOUND;
#else
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(packet_pptr);
    NX_PARAMETER_NOT_USED(entity_header_buffer);
    NX_PARAMETER_NOT_USED(buffer_size);

    return NX_WEB_HTTP_ERROR;
#endif /* NX_WEB_HTTP_MULTIPART_ENABLE */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_server_get_entity_content             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks error for getting the entity content.          */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    packet_pptr                           Pointer to packet             */ 
/*    available_offset                      Return the offset             */ 
/*    available_length                      Return the length             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_get_entity_content                              */
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
UINT  _nxe_web_http_server_get_entity_content(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, ULONG *available_offset, ULONG *available_length)
{

#ifdef  NX_WEB_HTTP_MULTIPART_ENABLE
    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) ||
        (packet_pptr == NX_NULL) || (available_offset == NX_NULL) || (available_length == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check whether boundary is found. */
    if(server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_boundary[0] == 0)
        return(NX_WEB_HTTP_ERROR);

    return _nx_web_http_server_get_entity_content(server_ptr, packet_pptr, available_offset, available_length);
#else
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(packet_pptr);
    NX_PARAMETER_NOT_USED(available_offset);
    NX_PARAMETER_NOT_USED(available_length);

    return NX_WEB_HTTP_ERROR;
#endif /* NX_WEB_HTTP_MULTIPART_ENABLE */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_get_entity_content              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function gets the entity content.                              */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    packet_pptr                           Pointer to packet             */ 
/*    available_offset                      Return the offset             */ 
/*    available_length                      Return the length             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_boundary_find                                   */
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
UINT  _nx_web_http_server_get_entity_content(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, ULONG *available_offset, ULONG *available_length)
{

#ifdef  NX_WEB_HTTP_MULTIPART_ENABLE
UINT    status;

    /* Whether the boundary is found? */
    if(server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_boundary_find == NX_TRUE)
        return NX_WEB_HTTP_BOUNDARY_ALREADY_FOUND;

    status = _nx_web_http_server_boundary_find(server_ptr, packet_pptr);
    if(status)
        return status;

    /* Set the offset and length. */
    *available_offset = server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_available_offset;
    *available_length = server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_available_length;

    return NX_SUCCESS;
#else
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(packet_pptr);
    NX_PARAMETER_NOT_USED(available_offset);
    NX_PARAMETER_NOT_USED(available_length);

    return NX_WEB_HTTP_ERROR;
#endif /* NX_WEB_HTTP_MULTIPART_ENABLE */
}


#ifdef  NX_WEB_HTTP_MULTIPART_ENABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_boundary_find                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function finds boundary in packet even if it is chained.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    packet_pptr                           Pointer to packet             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                                                   */
/*    nx_tcp_socket_receive                                               */
/*    _nx_web_http_server_match_string                                    */
/*    _nx_utility_string_length_check                                     */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_web_http_server_get_entity_header                               */
/*    _nx_web_http_server_get_entity_content                              */
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
UINT  _nx_web_http_server_boundary_find(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr)
{

NX_WEB_HTTP_SERVER_MULTIPART   *multipart_ptr;
NX_PACKET                  *available_packet;
UINT                        offset;
ULONG                       match_count = 0;
UCHAR                      *match_end_ptr;
UINT                        boundary_length;

    /* Get multipart context. */
    multipart_ptr = &server_ptr -> nx_web_http_server_multipart;

    /* Is boundary already found? */
    if(multipart_ptr -> nx_web_http_server_multipart_boundary_find == NX_TRUE)
    {

        /* Yes. Update the packet and offset. */
        if(multipart_ptr -> nx_web_http_server_multipart_next_packet != NX_NULL)
        {
            nx_packet_release(*packet_pptr);
            *packet_pptr = multipart_ptr -> nx_web_http_server_multipart_next_packet;
            server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_last_packet = multipart_ptr -> nx_web_http_server_multipart_next_packet;
        }

        multipart_ptr -> nx_web_http_server_multipart_available_offset = multipart_ptr -> nx_web_http_server_multipart_next_available_offset;

        /* Reset next packet and available offset. */
        multipart_ptr -> nx_web_http_server_multipart_next_packet = NX_NULL;
        multipart_ptr -> nx_web_http_server_multipart_next_available_offset = 0;

        /* Reset boundary find. */
        multipart_ptr -> nx_web_http_server_multipart_boundary_find = NX_FALSE;

        return NX_SUCCESS;
    }

    /* Is the boundary in the next packet? */
    if(multipart_ptr -> nx_web_http_server_multipart_next_packet)
    {

        /* Yes, we're done with this packet. */
        nx_packet_release(*packet_pptr);

        /* Update the packet and multipart offset. */
        *packet_pptr = multipart_ptr -> nx_web_http_server_multipart_next_packet;
        server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_last_packet = multipart_ptr -> nx_web_http_server_multipart_next_packet;
        multipart_ptr -> nx_web_http_server_multipart_available_offset = 0;
    }
    else if(multipart_ptr -> nx_web_http_server_multipart_next_available_offset)
    {

        /* Find boundary in this packet. */
        /* Update the offset. */
        multipart_ptr -> nx_web_http_server_multipart_available_offset = multipart_ptr -> nx_web_http_server_multipart_next_available_offset;
    }
    else
    {

        /* Find boundary from this packet. */
        /* Initialize the offset. */
        multipart_ptr -> nx_web_http_server_multipart_next_available_offset = multipart_ptr -> nx_web_http_server_multipart_available_offset;
    }

    /* Reset next packet. */
    multipart_ptr -> nx_web_http_server_multipart_next_packet = NX_NULL;

    /* Reset boundary find. */
    multipart_ptr -> nx_web_http_server_multipart_boundary_find = NX_FALSE;


    /* Get offset of available data in the packet.  */
    offset = multipart_ptr -> nx_web_http_server_multipart_available_offset;

    /* Skip data that are not header. */
    available_packet = *packet_pptr;

    if(offset >= available_packet -> nx_packet_length)
    {

        /* Release the packet that has been processed. */
        nx_packet_release(*packet_pptr);

        /* All data has been processed. Get a new packet. */
        if(_nx_web_http_server_packet_get(server_ptr, &available_packet) != NX_SUCCESS)
        {

            /* Error, return to caller.  */
            return(NX_WEB_HTTP_TIMEOUT);
        }

        /* Store the packet. */
        *packet_pptr = available_packet;
        server_ptr -> nx_web_http_server_multipart.nx_web_http_server_multipart_last_packet = available_packet;

        /* Reset the offset. */
        multipart_ptr -> nx_web_http_server_multipart_available_offset = 0;
        multipart_ptr -> nx_web_http_server_multipart_next_available_offset = 0;
        offset = 0;
    }
    else
    {
#ifndef NX_DISABLE_PACKET_CHAIN
        while((UINT)(available_packet -> nx_packet_append_ptr - available_packet -> nx_packet_prepend_ptr) <= offset)
        {
            offset -= (UINT)(available_packet -> nx_packet_append_ptr - available_packet -> nx_packet_prepend_ptr);

            /* Get next packet. */
            available_packet = available_packet -> nx_packet_next;
        }
#endif /* NX_DISABLE_PACKET_CHAIN */
    }

    /* Check boundary length.  */
    if (_nx_utility_string_length_check((CHAR *)multipart_ptr -> nx_web_http_server_multipart_boundary, &boundary_length, NX_WEB_HTTP_MAX_HEADER_FIELD))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* Look for the boundary string in the (next) packet. */
    while(_nx_web_http_server_match_string(available_packet -> nx_packet_prepend_ptr + offset,
                                       available_packet -> nx_packet_append_ptr,
                                       multipart_ptr -> nx_web_http_server_multipart_boundary,
                                       boundary_length,
                                       &match_count, &match_end_ptr) != NX_SUCCESS)
    {

        /* Match string in the next packet. */
#ifndef NX_DISABLE_PACKET_CHAIN
        if(available_packet -> nx_packet_next)
        {
            multipart_ptr -> nx_web_http_server_multipart_next_available_offset += (UINT)((UINT)(available_packet -> nx_packet_append_ptr - available_packet -> nx_packet_prepend_ptr) - offset);
            available_packet = available_packet -> nx_packet_next;
            offset = 0;
        }
        else
#endif /* NX_DISABLE_PACKET_CHAIN */
            break;
    }

    /* Is boundary matched? */
    if(match_end_ptr)
    {

        /* Yes. Found a match. */
        multipart_ptr -> nx_web_http_server_multipart_next_available_offset += (UINT)((UINT)(match_end_ptr - available_packet -> nx_packet_prepend_ptr) - offset);
        multipart_ptr -> nx_web_http_server_multipart_available_length = multipart_ptr -> nx_web_http_server_multipart_next_available_offset - multipart_ptr -> nx_web_http_server_multipart_available_offset - match_count;
        multipart_ptr -> nx_web_http_server_multipart_boundary_find = NX_TRUE;

    }
    else if(match_count)
    {

        /* Set the available length to all remaining data. */
        multipart_ptr -> nx_web_http_server_multipart_available_length = (*packet_pptr) -> nx_packet_length - multipart_ptr -> nx_web_http_server_multipart_available_offset;

        /* Partial match. Get another packet. */
        if(_nx_web_http_server_packet_get(server_ptr, &available_packet) != NX_SUCCESS)
        {

            /* Error, return to caller.  */
            return(NX_WEB_HTTP_TIMEOUT);
        }

        /* Match the boundary again. */
        if(_nx_web_http_server_match_string(available_packet -> nx_packet_prepend_ptr,
                                        available_packet -> nx_packet_append_ptr,
                                        multipart_ptr -> nx_web_http_server_multipart_boundary,
                                        boundary_length,
                                        &match_count, &match_end_ptr) == NX_SUCCESS)
        {

            /* Yes. Find a match. */
            if((ULONG)(match_end_ptr - available_packet -> nx_packet_prepend_ptr) < match_count)
            {

                /* The boundary is between two packets. */
                multipart_ptr -> nx_web_http_server_multipart_next_available_offset = (UINT)(match_end_ptr - available_packet -> nx_packet_prepend_ptr);
                multipart_ptr -> nx_web_http_server_multipart_available_length -= boundary_length - multipart_ptr -> nx_web_http_server_multipart_next_available_offset;
                multipart_ptr -> nx_web_http_server_multipart_boundary_find = NX_TRUE;
            }
            else
            {

                /* The boundary is in the next packet. */
                multipart_ptr -> nx_web_http_server_multipart_next_available_offset = 0;
            }
        }
        else
        {

            /* The boundary is not found. */
            multipart_ptr -> nx_web_http_server_multipart_next_available_offset = 0;
        }

        /* Store the packet. */
        multipart_ptr -> nx_web_http_server_multipart_next_packet = available_packet;

    }
    else
    {

        /* Set the available length to all remaining data. */
        multipart_ptr -> nx_web_http_server_multipart_available_length = (*packet_pptr) -> nx_packet_length - multipart_ptr -> nx_web_http_server_multipart_available_offset;
        multipart_ptr -> nx_web_http_server_multipart_next_available_offset = (*packet_pptr) -> nx_packet_length;
    }

    return NX_SUCCESS;
}
#endif /* NX_WEB_HTTP_MULTIPART_ENABLE */


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_match_string                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function compares a target string in specified memory area.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    src_start                             Pointer to start of source    */ 
/*    src_end                               Pointer to end of source      */ 
/*    target                                Pointer to target             */ 
/*    target_length                         Length to target              */ 
/*    match_count                           Return the match count        */ 
/*    match_end_ptr                         Return the end match position */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    memcmp                                Compare memory data           */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_web_http_server_boundary_find                                   */
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
UINT  _nx_web_http_server_match_string(UCHAR *src_start, UCHAR *src_end, UCHAR *target, ULONG target_length, ULONG *match_count, UCHAR **match_end_ptr)
{

UCHAR  *ch;
ULONG   pre_match = *match_count;
ULONG   remain_match;

    /* Initialize. */
    *match_end_ptr = NX_NULL;

    /* Process part match. */
    while(pre_match)
    {

        /* Compare pre-match. */
        if(memcmp(target, target + (*match_count - pre_match), pre_match) == 0)
        {

            /* Calculate the remain match. */
            remain_match = target_length - pre_match;

            /* The pre-match is the same. */
            if((ULONG)(src_end - src_start) < remain_match)
                remain_match = (ULONG)(src_end - src_start);

            if(memcmp(target + pre_match, src_start, remain_match) == 0)
            {

                /* Remain part matches. */
                *match_count = pre_match + remain_match;

                if(*match_count < target_length)
                {

                    /* Part match. */
                    return NX_WEB_HTTP_NOT_FOUND;
                }

                /* Full match. */
                *match_end_ptr = src_start + remain_match;
                return NX_SUCCESS;
            }
        }

        pre_match--;
    }

    ch = src_start;

    /* Perform full match. */
    *match_count = target_length;
    while(ch + *match_count <= src_end)
    {
        if(memcmp(ch, target, *match_count) == 0)
        {

            /* Find a match. */
            *match_end_ptr = ch + *match_count;
            return NX_SUCCESS;
        }

        ch++;
    }

    /* Perform part match. */
    for(*match_count = target_length - 1; *match_count > 0; *match_count = *match_count - 1)
    {
        if(ch + *match_count <= src_end)
        {
            if(memcmp(ch, target, *match_count) == 0)
            {

                /* Find a match. */
                return NX_WEB_HTTP_NOT_FOUND;
            }

            ch++;
        }
    }

    return NX_WEB_HTTP_NOT_FOUND;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_field_value_get                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function gets field value by field name from HTTP header.      */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    packet_ptr                            Pointer to packet             */ 
/*    field_name                            Name of field in header       */ 
/*    name_length                           Length of name field          */ 
/*    field_value                           Return of field value         */ 
/*    field_value_size                      Size of field value           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_memicmp           Compare ignore case           */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_web_http_server_get_process                                     */
/*    _nx_web_http_server_get_entity_header                               */
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
UINT  _nx_web_http_server_field_value_get(NX_PACKET *packet_ptr, UCHAR *field_name, ULONG name_length, UCHAR *field_value, ULONG field_value_size)
{

UCHAR  *ch;
UINT    index;


    /* Initialize. */
    ch = packet_ptr -> nx_packet_prepend_ptr;

    /* Loop to find field name. */
    while(ch + name_length < packet_ptr -> nx_packet_append_ptr)
    {
        if(_nx_web_http_server_memicmp(ch, name_length, field_name, name_length) == NX_SUCCESS)
        {

            /* Field name found. */
            break;
        }

        /* Move the pointer up to the next character.  */
        ch++;
    }

    /* Skip field name and ':'. */
    ch += name_length + 1;

    /* Is field name found? */
    if(ch >= packet_ptr -> nx_packet_append_ptr)
        return NX_WEB_HTTP_NOT_FOUND;

    /* Skip white spaces. */
    while(*ch == ' ')
    {
        if(ch >= packet_ptr -> nx_packet_append_ptr)
            return NX_WEB_HTTP_NOT_FOUND;

        /* Get next character. */
        ch++;
    }

    /* Copy field value. */
    index = 0;
    while(index < field_value_size)
    {

        /* Whether the end of line CRLF is not found? */
        if(ch + 2 > packet_ptr -> nx_packet_append_ptr)
            return NX_WEB_HTTP_NOT_FOUND;

        /* Compare CRLF. */ 
        if((*ch == 13) && (*(ch + 1) == 10))
            break;

        /* Copy data. */
        field_value[index++] = *ch++;
    }

    /* Buffer overflow? */
    if(index == field_value_size)
        return NX_WEB_HTTP_NOT_FOUND;

    /* Trim white spaces. */
    while((index > 0) && (field_value[index - 1] == ' '))
        index--;

    /* Append terminal 0. */
    field_value[index] = 0;


    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_memicmp                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function compares two pieces of memory case insensitive.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    src                                   Pointer to source             */ 
/*    src_length                            Length of source              */ 
/*    dest                                  Pointer to destination        */ 
/*    dest_length                           Length of destination         */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_web_http_server_get_entity_header                               */
/*    _nx_web_http_server_field_value_get                                 */
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
UINT  _nx_web_http_server_memicmp(UCHAR *src, ULONG src_length, UCHAR *dest, ULONG dest_length)
{
UCHAR   ch;

    /* Compare the length. */
    if(src_length != dest_length)
        return NX_WEB_HTTP_FAILED;

    while(src_length)
    {

        /* Is src lowercase? */
        if((*src >= 'a') && (*src <= 'z'))
            ch = (UCHAR)(*src - 'a' + 'A');

        /* Is src uppercase? */
        else if((*src >= 'A') && (*src <= 'Z'))
            ch = (UCHAR)(*src - 'A' + 'a');
        else
            ch = *src;

        /* Compare case insensitive. */
        if((*src != *dest) && (ch != *dest))
            return NX_WEB_HTTP_FAILED;

        /* Pickup next character. */
        src_length--;
        src++;
        dest++;
    }

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_generate_response_header        PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function generates HTTP response header.                       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    packet_pptr                           Pointer to packet             */ 
/*    status_code                           Status-code and reason-phrase */
/*    status_code_length                    Length of status-code         */
/*    content_length                        Length of content             */ 
/*    content_type                          Type of content               */ 
/*    content_type_length                   Length of content type        */ 
/*    additional_header                     Other HTTP headers            */ 
/*    additional_header_length              Length of other HTTP headers  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_response_packet_allocate                        */
/*                                          Allocate response packet      */
/*    nx_packet_data_append                 Append packet data            */
/*    memcmp                                Compare string                */
/*    _nx_utility_uint_to_string            Convert number to ASCII       */
/*    _nx_web_http_server_date_to_string    Convert data to string        */
/*    nx_packet_release                     Release packet                */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_web_http_server_get_process                                     */
/*    _nx_web_http_server_put_process                                     */
/*    _nx_web_http_server_delete_process                                  */
/*    _nx_web_http_server_response_send                                   */
/*    _nx_web_http_server_callback_generate_response_header               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            improved the logic of       */
/*                                            converting number to string,*/
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_web_http_server_generate_response_header(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr,
                                                   CHAR *status_code, UINT status_code_length,
                                                   UINT content_length, CHAR *content_type,
                                                   UINT content_type_length, CHAR* additional_header,
                                                   UINT additional_header_length)
{

UINT        status;
UINT        temp;
CHAR        temp_string[30];
CHAR        crlf[2] = {13,10};
NX_PACKET  *packet_ptr;
CHAR        status_code_ok;
CHAR        status_code_not_modified;


    /* Allocate a packet for sending the response back.  */
    status = _nx_web_http_server_response_packet_allocate(server_ptr, packet_pptr, NX_WAIT_FOREVER);

    /* Determine if an error occurred in the packet allocation.  */
    if(status != NX_SUCCESS)
    {

        /* Indicate an allocation error occurred.  */
        server_ptr -> nx_web_http_server_allocation_errors++;

        /* Just return.  */
        return status;
    }
    else
        packet_ptr = *packet_pptr;

    /* Check status code. */
    if(memcmp(status_code, NX_WEB_HTTP_STATUS_OK, sizeof(NX_WEB_HTTP_STATUS_OK)) == 0)
        status_code_ok = NX_TRUE;
    else
        status_code_ok = NX_FALSE;

    if(memcmp(status_code, NX_WEB_HTTP_STATUS_NOT_MODIFIED, sizeof(NX_WEB_HTTP_STATUS_NOT_MODIFIED)) == 0)
        status_code_not_modified = NX_TRUE;
    else
        status_code_not_modified = NX_FALSE;

    /* Insert the status line.  */
    /* Place HTTP-Version. */
    status = nx_packet_data_append(packet_ptr, NX_WEB_HTTP_VERSION, sizeof(NX_WEB_HTTP_VERSION) - 1,  server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

    /* Insert space. */
    status += nx_packet_data_append(packet_ptr, " ", 1,  server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

    status += nx_packet_data_append(packet_ptr, status_code, status_code_length,  server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

    /* Place the <cr,lf> into the buffer.  */
    status += nx_packet_data_append(packet_ptr, crlf, 2, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

    /* Determine if there is content_type.  */
    if(content_type)
    {

        /* Insert the content type.  */
        status += nx_packet_data_append(packet_ptr, "Content-Type: ", 14,  server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
        status += nx_packet_data_append(packet_ptr, content_type, content_type_length,  server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place the <cr,lf> into the buffer.  */
        status += nx_packet_data_append(packet_ptr, crlf, 2, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
    }

#ifndef NX_WEB_HTTP_SERVER_OMIT_CONTENT_LENGTH
    /* Determine if there is content_length.  */
    if(content_length || (status_code_not_modified == NX_TRUE))
    {
#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE

        /* Place the "Connection" field in the header.  */
        if(server_ptr -> nx_web_http_server_keepalive == NX_TRUE)
        {

            /* Keepalive. */
            status += nx_packet_data_append(packet_ptr, "Connection: keep-alive", 22,
                    server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
        }
        else
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */
        {

        /* Close. */
        status += nx_packet_data_append(packet_ptr, "Connection: Close", 17,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
        }

        /* Place the <cr,lf> into the buffer.  */
        status += nx_packet_data_append(packet_ptr, crlf, 2, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        if(content_length)
        {

            /* Convert the content_length to ASCII representation.  */
            temp = _nx_utility_uint_to_string(content_length, 10, temp_string, sizeof(temp_string));

            /* Place the "Content-Length" field in the header.  */
            status += nx_packet_data_append(packet_ptr, "Content-Length: ", 16,
                    server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
            status += nx_packet_data_append(packet_ptr, temp_string, temp,
                    server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

            /* Place the <cr,lf> into the buffer.  */
            status += nx_packet_data_append(packet_ptr, crlf, 2, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
        }

    }
    else
#endif /* NX_WEB_HTTP_SERVER_OMIT_CONTENT_LENGTH */
    {

        /* Place the "Connection" field in the header.  */

        /* Force close the connection since there's no content-length. */
        status += nx_packet_data_append(packet_ptr, "Connection: Close", 17,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
        server_ptr -> nx_web_http_server_keepalive = NX_FALSE;
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */

        /* Place the <cr,lf> into the buffer.  */
        status += nx_packet_data_append(packet_ptr, crlf, 2, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
    }

    /* Insert Date header if callback function is set. */
    if(server_ptr -> nx_web_http_server_gmt_get)
    {
    NX_WEB_HTTP_SERVER_DATE date;

        /* Get current date. */
        server_ptr -> nx_web_http_server_gmt_get(&date);

        /* Convert date to string. */
        temp = _nx_web_http_server_date_to_string(&date, temp_string);

        /* Place the "Date" field in the header.  */
        status += nx_packet_data_append(packet_ptr, "Date: ", 6,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Insert date. */
        status += nx_packet_data_append(packet_ptr, temp_string, temp, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Place the <cr,lf> into the buffer.  */
        status += nx_packet_data_append(packet_ptr, crlf, 2, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

        /* Set Cache Control if callback function is set. */
        if((server_ptr -> nx_web_http_server_cache_info_get) &&
           ((status_code_ok == NX_TRUE) || (status_code_not_modified == NX_TRUE)) &&
           (server_ptr -> nx_web_http_server_request_type == NX_WEB_HTTP_SERVER_GET_REQUEST))
        {
        UINT max_age;

            /* Get cache infomation. */
            if(server_ptr -> nx_web_http_server_cache_info_get(server_ptr -> nx_web_http_server_request_resource, &max_age, &date) == NX_TRUE)
            {

                /* Place "Cache-control" header. */
                status += nx_packet_data_append(packet_ptr, "Cache-Control: max-age=", 23,
                                                server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

                /* Convert the max-age to ASCII representation.  */
                temp = _nx_utility_uint_to_string(max_age, 10, temp_string, sizeof(temp_string));
                status += nx_packet_data_append(packet_ptr, temp_string, temp,
                                                server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

                /* Place the <cr,lf> into the buffer.  */
                status += nx_packet_data_append(packet_ptr, crlf, 2, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

                if(status_code_ok == NX_TRUE)
                {

                    /* Convert date to string. */
                    temp = _nx_web_http_server_date_to_string(&date, temp_string);

                    /* Place the "Last-Modified" field in the header.  */
                    status += nx_packet_data_append(packet_ptr, "Last-Modified: ", 15,
                                                    server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

                    /* Insert date. */
                    status += nx_packet_data_append(packet_ptr, temp_string, temp, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

                    /* Place the <cr,lf> into the buffer.  */
                    status += nx_packet_data_append(packet_ptr, crlf, 2, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
                }
            }
        }
    }

    /* Insert additional header. */
    if(additional_header)
    {
        status += nx_packet_data_append(packet_ptr, additional_header, additional_header_length,
                                        server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
    }

    /* Place the <cr,lf> into the buffer.  */
    status += nx_packet_data_append(packet_ptr, crlf, 2, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);

    if (status != NX_SUCCESS)
    {

        /* Just release the packet.  */
        nx_packet_release(packet_ptr);
    }

    return status;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_request_read                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the next data in byte.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    Server_ptr                            Pointer to HTTP server        */
/*    data                                  Returned data                 */
/*    wait_option                           Suspension option             */
/*    current_packet_pptr                   Pointer to the packet being   */
/*                                            processed                   */
/*    current_data_ptr                      Pointer to the data will be   */
/*                                            processed                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_receive           Receive another packet        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_request_byte_expect                             */ 
/*                                          Checks next byte is  expected */
/*    _nx_web_http_server_chunked_size_get  Gets chunk size of request    */
/*                                          packet chunk                  */
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
UINT _nx_web_http_server_request_read(NX_WEB_HTTP_SERVER *server_ptr, UCHAR *data, ULONG wait_option, 
                                      NX_PACKET **current_packet_pptr, UCHAR **current_data_pptr)
{
UINT status;

    /* If there is no remaining data.  */
    while (server_ptr -> nx_web_http_server_chunked_request_remaining_size == 0)
    {

        /* Release the previous received packet.  */
        if (server_ptr -> nx_web_http_server_request_packet)
        {
            nx_packet_release(server_ptr -> nx_web_http_server_request_packet);
            server_ptr -> nx_web_http_server_request_packet = NX_NULL;
        }

        /* Receive another packet.  */
        status = _nx_web_http_server_receive(server_ptr, &(server_ptr -> nx_web_http_server_request_packet), wait_option);
        if (status)
        {
            return(status);
        }

        /* Update the packet pointer, data pointer and remaining size.  */
        (*current_packet_pptr) = server_ptr -> nx_web_http_server_request_packet;
        (*current_data_pptr) = server_ptr -> nx_web_http_server_request_packet -> nx_packet_prepend_ptr;
        server_ptr -> nx_web_http_server_chunked_request_remaining_size = server_ptr -> nx_web_http_server_request_packet -> nx_packet_length;
    }

    /* Process the packet chain.  */
    if ((*current_data_pptr) == (*current_packet_pptr) -> nx_packet_append_ptr)
    {
#ifndef NX_DISABLE_PACKET_CHAIN
        if ((*current_packet_pptr) -> nx_packet_next == NX_NULL)
        {
            return(NX_INVALID_PACKET);
        }

        (*current_packet_pptr) = (*current_packet_pptr) -> nx_packet_next;
        (*current_data_pptr) = (*current_packet_pptr) -> nx_packet_prepend_ptr;

        /* Release the processed packet in the packet chain.  */
        if (server_ptr -> nx_web_http_server_request_packet)
        {
            server_ptr -> nx_web_http_server_request_packet -> nx_packet_next = NX_NULL;
            nx_packet_release(server_ptr -> nx_web_http_server_request_packet);
            server_ptr -> nx_web_http_server_request_packet = (*current_packet_pptr);
        }
#else
        return(NX_INVALID_PACKET);
#endif /* NX_DISABLE_PACKET_CHAIN */
    }

    /* Set the returned data.  */
    *data = *(*current_data_pptr);

    /* Update the data pointer and remaining size.  */
    (*current_data_pptr)++;
    server_ptr -> nx_web_http_server_chunked_request_remaining_size--;

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_request_byte_expect             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks if next byte is the expected data.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    data                                  Expected data                 */
/*    wait_option                           Suspension option             */
/*    current_packet_pptr                   Pointer to the packet being   */
/*                                            processed                   */
/*    current_data_ptr                      Pointer to the data will be   */
/*                                            processed                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Compare result                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_request_read      Get next data                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_chunked_size_get  Gets chunk size of request    */
/*    _nx_web_http_server_request_chunked_get                             */
/*                                          Get chunk data from request   */
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
UINT _nx_web_http_server_request_byte_expect(NX_WEB_HTTP_SERVER *server_ptr, UCHAR data, ULONG wait_option, 
                                             NX_PACKET **current_packet_pptr, UCHAR **current_data_pptr)
{
UINT status;
UCHAR tmp;

    /* Get next data.  */
    status = _nx_web_http_server_request_read(server_ptr, &tmp, wait_option, current_packet_pptr, current_data_pptr);

    if (status)
    {
        return(status);
    }

    /* Return the compare result.  */
    if (tmp != data)
    {
        return(NX_NOT_FOUND);
    }

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_chunked_size_get                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the chunk size of the request packet chunk.      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    chunk_size                            Returned chunk size           */
/*    wait_option                           Suspension option             */
/*    current_packet_pptr                   Pointer to the packet being   */
/*                                            processed                   */
/*    current_data_ptr                      Pointer to the data will be   */
/*                                            processed                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_chunked_size_get  Get chunk size                */
/*    _nx_web_http_server_request_byte_expect                             */
/*                                          Check if next byte is expected*/
/*    nx_packet_release                     Release packet                */
/*    nx_packet_append                      Append packet data            */
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
UINT _nx_web_http_server_chunked_size_get(NX_WEB_HTTP_SERVER *server_ptr, UINT *chunk_size, ULONG wait_option, 
                                          NX_PACKET **current_packet_pptr, UCHAR **current_data_pptr)
{
UINT status;
UINT size = 0;
UCHAR tmp;
UINT  chunk_extension = 0;

    if (server_ptr -> nx_web_http_server_actual_bytes_received < server_ptr -> nx_web_http_server_expect_receive_bytes)
    {

        /* If there are bytes need to receive, set the size need to receive as chunk size.  */
        *chunk_size = server_ptr -> nx_web_http_server_expect_receive_bytes - server_ptr -> nx_web_http_server_actual_bytes_received;
    }
    else
    {

        /* Get the chunk size.  */
        while (1)
        {

            /* Read next byte from request packet.  */
            status = _nx_web_http_server_request_read(server_ptr, &tmp, wait_option, current_packet_pptr, current_data_pptr);
            if (status)
            {
                return(status);
            }

            /* Skip the chunk extension.  */
            if (chunk_extension && (tmp != '\r'))
            {
                continue;
            }

            /* Calculate the size.  */
            if ((tmp >= 'a') && (tmp <= 'f'))
            {
                size = (size << 4) + 10 + (UINT)(tmp - 'a');
            }
            else if ((tmp >= 'A') && (tmp <= 'F'))
            {
                size = (size << 4) + 10 + (UINT)(tmp - 'A');
            }
            else if ((tmp >= '0') && (tmp <= '9'))
            {
                size = (size << 4) + (UINT)(tmp - '0');
            }
            else if (tmp == '\r')
            {

                /* Find the end of chunk header.  */
                break;
            }
            else if (tmp == ';')
            {

                /* Find chunk extension.  */
                chunk_extension = 1;
            }
            else
            {
                return(NX_NOT_FOUND);
            }
        }

        /* Expect '\n'.  */
        status = _nx_web_http_server_request_byte_expect(server_ptr, '\n', wait_option, current_packet_pptr, current_data_pptr);
        if (status)
        {
            return(status);
        }

        *chunk_size = size;
    }

    /* If there is no remaining data, receive another packet.  */
    while (server_ptr -> nx_web_http_server_chunked_request_remaining_size == 0)
    {
        if (server_ptr -> nx_web_http_server_request_packet)
        {
            nx_packet_release(server_ptr -> nx_web_http_server_request_packet);
        }

        status = _nx_web_http_server_receive(server_ptr, &(server_ptr -> nx_web_http_server_request_packet), wait_option);
        if (status)
        {
            return(status);
        }

        /* Update the current request packet, data pointer and remaining size.  */
        (*current_packet_pptr) = server_ptr -> nx_web_http_server_request_packet;
        (*current_data_pptr) = server_ptr -> nx_web_http_server_request_packet -> nx_packet_prepend_ptr;
        server_ptr -> nx_web_http_server_chunked_request_remaining_size = server_ptr -> nx_web_http_server_request_packet -> nx_packet_length;
    }

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_request_chunked_get             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets the chunk data from chunked request.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    packet_pptr                           Pointer to the packet         */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_chunked_size_get  Get chunk size                */
/*    _nx_web_http_server_request_byte_expect                             */
/*                                          Check if next byte is expected*/
/*    nx_packet_allocate                    Allocate packet               */
/*    nx_packet_append                      Append packet data            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_packet_get        Get next packet from HTTP     */
/*                                          server socket                 */ 
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
UINT _nx_web_http_server_request_chunked_get(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr, ULONG wait_option)
{
UINT       status;
UINT       chunk_size = 0, length = 0, temp_size = 0;
UINT       remaining_size = 0;
NX_PACKET *packet_ptr;
NX_PACKET *current_packet_ptr = NX_NULL;
UCHAR     *current_data_ptr = NX_NULL;

    /* Set pointer for processing packet data.  */
    current_packet_ptr = server_ptr -> nx_web_http_server_request_packet;

    if (current_packet_ptr)
    {

        /* Set current data pointer.  */
        current_data_ptr = current_packet_ptr -> nx_packet_prepend_ptr;
    }

    /* Get chunk size.  */
    status = _nx_web_http_server_chunked_size_get(server_ptr, &chunk_size, wait_option, &current_packet_ptr, &current_data_ptr);
    if (status)
    {
        return(status);
    }

    /* Check if it's the end.  */
    if (chunk_size == 0)
    {

        /* Read CRLF.  */
        status = _nx_web_http_server_request_byte_expect(server_ptr, '\r', wait_option, &current_packet_ptr, &current_data_ptr);
        if (status)
        {
            return(status);
        }

        status = _nx_web_http_server_request_byte_expect(server_ptr, '\n', wait_option, &current_packet_ptr, &current_data_ptr);
        if (status)
        {
            return(status);
        }

        *packet_pptr = NX_NULL;

        return(NX_WEB_HTTP_GET_DONE);
    }

    /* Set the return packet.  */
    *packet_pptr = server_ptr -> nx_web_http_server_request_packet;
    server_ptr -> nx_web_http_server_request_packet = NX_NULL;
    packet_ptr = *packet_pptr;
    packet_ptr -> nx_packet_prepend_ptr = current_data_ptr;
    remaining_size = server_ptr -> nx_web_http_server_chunked_request_remaining_size;

    /* Process the chunk data.  */
    if (chunk_size <= remaining_size)
    {

        /* One or more chunk in the remaining data.  */
        temp_size = chunk_size;

        /* Check if the chunk data is all in this packet.  */
        while (temp_size > (UINT)(current_packet_ptr -> nx_packet_append_ptr - current_data_ptr))
        {

#ifndef NX_DISABLE_PACKET_CHAIN
            if (current_packet_ptr -> nx_packet_next == NX_NULL)
            {
                return(NX_INVALID_PACKET);
            }

            temp_size -= (UINT)(current_packet_ptr -> nx_packet_append_ptr - current_data_ptr);
            current_packet_ptr = current_packet_ptr -> nx_packet_next;
            current_data_ptr = current_packet_ptr -> nx_packet_prepend_ptr;
#else
            return(NX_INVALID_PACKET);
#endif /* NX_DISABLE_PACKET_CHAIN */
        }

        /* Skip the chunk data.  */
        current_data_ptr += temp_size;
        server_ptr -> nx_web_http_server_chunked_request_remaining_size -= chunk_size;
        length = chunk_size;

        /* Read CRLF.  */
        status = _nx_web_http_server_request_byte_expect(server_ptr, '\r', wait_option, &current_packet_ptr, &current_data_ptr);
        if (status)
        {
            return(status);
        }

        status = _nx_web_http_server_request_byte_expect(server_ptr, '\n', wait_option, &current_packet_ptr, &current_data_ptr);
        if (status)
        {
            return(status);
        }

        /* Check the remaining data.  */
        if (server_ptr -> nx_web_http_server_chunked_request_remaining_size)
        {
            if (server_ptr -> nx_web_http_server_request_packet)
            {

                /* If received new packet, adjust the prepend pointer of this packet.  */
                server_ptr -> nx_web_http_server_request_packet -> nx_packet_prepend_ptr = current_data_ptr;
            }
            else
            {

                /* Copy the remaining data to a new packet.  */
                /* Allocate a packet.  */
                status = nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr, 
                                            &(server_ptr -> nx_web_http_server_request_packet), 
                                            0, wait_option);
                if (status)
                {
                    return(status);
                }

                /* Copy the remaining data in current packet to the new packet.   */
                temp_size = (UINT)(current_packet_ptr -> nx_packet_append_ptr - current_data_ptr);
                status = nx_packet_data_append(server_ptr -> nx_web_http_server_request_packet, 
                                               current_data_ptr, 
                                               temp_size, 
                                               server_ptr -> nx_web_http_server_packet_pool_ptr, 
                                               wait_option);
                if (status)
                {
                    return(status);
                }

                /* Check if any remaining data not in current packet.  */
                if (server_ptr -> nx_web_http_server_chunked_request_remaining_size > temp_size)
                {
#ifndef NX_DISABLE_PACKET_CHAIN

                    /* If there are chained packets, append the packets to the new packet.  */
                    if (current_packet_ptr -> nx_packet_next)
                    {
                        server_ptr -> nx_web_http_server_request_packet -> nx_packet_next = current_packet_ptr -> nx_packet_next;
                        server_ptr -> nx_web_http_server_request_packet -> nx_packet_last = current_packet_ptr -> nx_packet_last;
                        current_packet_ptr -> nx_packet_next = NX_NULL;
                    }
                    else
#endif /* NX_DISABLE_PACKET_CHAIN */
                    {
                        return(NX_INVALID_PACKET);
                    }
                }
            }

            /* Update the packet length.  */
            server_ptr -> nx_web_http_server_request_packet -> nx_packet_length = server_ptr -> nx_web_http_server_chunked_request_remaining_size;
        }
    }
    else
    {

        /* All the remaining data is in this chunk.  */
        server_ptr -> nx_web_http_server_chunked_request_remaining_size = 0;
        length = remaining_size;
    }

    /* Set the received bytes.  */
    server_ptr -> nx_web_http_server_expect_receive_bytes = chunk_size;
    server_ptr -> nx_web_http_server_actual_bytes_received = length;

#ifndef NX_DISABLE_PACKET_CHAIN
    /* Set length of the packet chain header.  */
    (*packet_pptr) -> nx_packet_length = length;

    /* Find the last packet.  */
    while (packet_ptr -> nx_packet_next && 
           (length > (UINT)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr)))
    {
        length -= (UINT)(packet_ptr -> nx_packet_append_ptr - packet_ptr -> nx_packet_prepend_ptr);
        packet_ptr = packet_ptr -> nx_packet_next;
    }

    /* Set the last packet.  */
    (*packet_pptr) -> nx_packet_last = packet_ptr;

    /* Maybe the '\r\n' is in another packet, release this packet.  */
    if (packet_ptr -> nx_packet_next)
    {
        nx_packet_release(packet_ptr -> nx_packet_next);
        packet_ptr -> nx_packet_next = NX_NULL;
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Set packet length and append pointer.  */
    /* If the packet is chained, set the length and append pointer of the last packet.  */
    packet_ptr -> nx_packet_length = length;
    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr + length;

    return(NX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_response_chunked_set           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the HTTP response chunked set    */
/*    API call.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    chunk_size                            Size of the chunk             */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_response_chunked_set                            */
/*                                          Actual response chunked set   */
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
UINT _nxe_web_http_server_response_chunked_set(NX_WEB_HTTP_SERVER *server_ptr, UINT chunk_size, NX_PACKET *packet_ptr)
{
UINT        status;

    /* Make sure the client instance makes sense. */
    if ((server_ptr == NX_NULL) || (packet_ptr == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Call actual response chunked set function.  */
    status =  _nx_web_http_server_response_chunked_set(server_ptr, chunk_size, packet_ptr);

    /* Return success to the caller.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_response_chunked_set            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the chunked information of chunked response.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    chunk_size                            Size of the chunk             */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_packet_data_append                 Append the packet data        */
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
UINT _nx_web_http_server_response_chunked_set(NX_WEB_HTTP_SERVER *server_ptr, UINT chunk_size, NX_PACKET *packet_ptr)
{
UINT        status;
UINT        temp_size, i, j;
CHAR        temp_string[10];
CHAR        crlf[2] = {13,10};

    /* Covert the size to ASCII.  */
    temp_size = chunk_size;
    i = 0;

    while (temp_size)
    {

        for (j = i; j > 0; j--)
        {
            temp_string[j] = temp_string[j - 1];
        }

        if ((temp_size & 0x0F) < 10)
        {
            temp_string[0] = (CHAR)((temp_size & 0x0F) + '0');
        }
        else
        {
            temp_string[0] = (CHAR)((temp_size & 0x0F) - 10 + 'a');
        }
        temp_size = temp_size >> 4;
        i++;
    }

    if (i == 0)
    {
        temp_string[i++] = '0';
    }

    /* Place the chunk size in the packet.  */
    status = nx_packet_data_append(packet_ptr, temp_string, i, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
    if (status)
    {
        return(status);
    }

    /* Place the CRLF to signal the end of the chunk size.  */
    status = nx_packet_data_append(packet_ptr, crlf, 2, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
    if (status)
    {
        return(status);
    }

    /* Set the request chunked flag.  */
    server_ptr -> nx_web_http_server_response_chunked = NX_TRUE;

    /* The total bytes need to transfer is chunk size plus chunk header length.  */
    server_ptr -> nx_web_http_server_expect_transfer_bytes = chunk_size + i + 2;
    server_ptr -> nx_web_http_server_actual_bytes_transferred = 0;

    return(NX_SUCCESS);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_chunked_check                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks if the packet is chunked.                      */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    packet_ptr                            Pointer to the packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                The packet is chunked or not  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_field_value_get   Get filed value               */
/*    _nx_web_http_server_memicmp           Compare two strings           */
/*    _nx_utility_string_length_check       Check string length           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_packet_content_find                             */
/*                                          Find content length in the    */
/*                                          HTTP header                   */ 
/*    _nx_web_http_server_content_get_extended                            */
/*                                          Get user specified portion of */
/*                                          HTTP header                   */
/*    _nx_web_http_server_receive_data      HTTP Server receive processing*/
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
UINT _nx_web_http_server_chunked_check(NX_PACKET *packet_ptr)
{
UINT status;
CHAR temp_string[20];
UINT temp_string_length;

    /* Check if the packet is chunked.  */
    status = _nx_web_http_server_field_value_get(packet_ptr, (UCHAR *)"Transfer-Encoding", 17, (UCHAR *)temp_string, sizeof(temp_string));

    if ((status == NX_SUCCESS) && 
        (_nx_utility_string_length_check(temp_string, &temp_string_length, sizeof(temp_string) - 1) == NX_SUCCESS) &&
        (_nx_web_http_server_memicmp((UCHAR *)temp_string, temp_string_length, (UCHAR *)"chunked", 7) == 0))
    {
        return(NX_TRUE);
    }
    
    return(NX_FALSE);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_response_packet_allocate       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the call to allocate packets for */
/*    an HTTP(S) server session, as used for PUT and POST operations.     */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP client        */
/*    packet_ptr                            Pointer to allocated packet   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxe_web_http_server_response_packet_allocate                       */
/*                                          Actual packet allocation call */
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
UINT _nxe_web_http_server_response_packet_allocate(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr,
                                                   UINT wait_option)
{
UINT status;

    /* Check our pointers. */
    if(server_ptr == NX_NULL || packet_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Call the actual function. */
    status = _nx_web_http_server_response_packet_allocate(server_ptr, packet_ptr, wait_option);

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_response_packet_allocate        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates a packet for Server HTTP(S) responses so    */
/*    that servers needing to send large amounts of data (such as for     */
/*    multi-part file upload) can allocated the needed.                   */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    packet_ptr                            Pointer to allocated packet   */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_packet_allocate         Allocate a TLS packet         */
/*    nx_packet_allocate                    Allocate a TCP packet         */
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
UINT _nx_web_http_server_response_packet_allocate(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr,
                                                  UINT wait_option)
{
UINT status;

    /* Allocate a packet.  */
#ifdef NX_WEB_HTTPS_ENABLE
    if(server_ptr -> nx_web_http_is_https_server)
    {
        status = nx_secure_tls_packet_allocate(&(server_ptr -> nx_web_http_server_current_session_ptr -> nx_tcp_session_tls_session),
                                               server_ptr -> nx_web_http_server_packet_pool_ptr,
                                               packet_ptr, wait_option);
    }
    else
#endif
    {
        status =  nx_packet_allocate(server_ptr -> nx_web_http_server_packet_pool_ptr,
                                     packet_ptr,
                                     NX_TCP_PACKET, wait_option);
    }

    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_server_callback_generate_response_header              */
/*                                                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks error for generating HTTP response header.     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    packet_pptr                           Pointer to packet             */ 
/*    status_code                           Status-code and reason-phrase */
/*    content_length                        Length of content             */ 
/*    content_type                          Type of content               */ 
/*    additional_header                     Other HTTP headers            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_callback_generate_response_header               */
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
UINT  _nxe_web_http_server_callback_generate_response_header(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr,
                                                         CHAR *status_code, UINT content_length, CHAR *content_type,
                                                         CHAR* additional_header)
{

    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) ||
        (packet_pptr == NX_NULL) || (status_code == NX_NULL))
        return(NX_PTR_ERROR);

    return _nx_web_http_server_callback_generate_response_header(server_ptr, packet_pptr, status_code, content_length, content_type, additional_header);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_callback_generate_response_header               */
/*                                                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is used in callback function to generate HTTP         */
/*    response header.                                                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    packet_pptr                           Pointer to packet             */ 
/*    status_code                           Status-code and reason-phrase */
/*    content_length                        Length of content             */ 
/*    content_type                          Type of content               */ 
/*    additional_header                     Other HTTP headers            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_generate_response_header                        */
/*                                          Generate response header      */
/*    _nx_utility_string_length_check       Check string length           */
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
UINT  _nx_web_http_server_callback_generate_response_header(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_pptr,
                                                            CHAR *status_code, UINT content_length, CHAR *content_type,
                                                            CHAR* additional_header)
{
UINT status_code_length = 0;
UINT content_type_length = 0;
UINT additional_header_length = 0;

    /* Check status code, content type and additional header length.  */
    if (_nx_utility_string_length_check(status_code, &status_code_length, NX_MAX_STRING_LENGTH) ||
        (content_type && _nx_utility_string_length_check(content_type, &content_type_length, NX_MAX_STRING_LENGTH)) ||
        (additional_header && _nx_utility_string_length_check(additional_header, &additional_header_length, NX_MAX_STRING_LENGTH)))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* Call the internal HTTP response send function.  */
    return _nx_web_http_server_generate_response_header(server_ptr, packet_pptr, status_code,
                                                        status_code_length, content_length,
                                                        content_type, content_type_length,
                                                        additional_header, additional_header_length);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_server_callback_generate_response_header_extended     */
/*                                                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks error for generating HTTP response header.     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    packet_pptr                           Pointer to packet             */ 
/*    status_code                           Status-code and reason-phrase */
/*    status_code_length                    Length of status-code         */
/*    content_length                        Length of content             */ 
/*    content_type                          Type of content               */
/*    content_type_length                   Length of content type        */ 
/*    additional_header                     Other HTTP headers            */
/*    additional_header_length              Length of other HTTP headers  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_callback_generate_response_header_extended      */
/*                                          Actual generate header        */
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
UINT  _nxe_web_http_server_callback_generate_response_header_extended(NX_WEB_HTTP_SERVER *server_ptr,
                                                                      NX_PACKET **packet_pptr, CHAR *status_code,
                                                                      UINT status_code_length, UINT content_length,
                                                                      CHAR *content_type, UINT content_type_length,
                                                                      CHAR *additional_header, UINT additional_header_length)
{

    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) ||
        (packet_pptr == NX_NULL) || (status_code == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    return _nx_web_http_server_callback_generate_response_header_extended(server_ptr, packet_pptr, status_code,
                                                                          status_code_length, content_length,
                                                                          content_type, content_type_length,
                                                                          additional_header, additional_header_length);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_callback_generate_response_header_extended      */
/*                                                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is used in callback function to generate HTTP         */
/*    response header.                                                    */ 
/*                                                                        */
/*    Note: The strings of status_code, content_type and additional_header*/
/*    must be NULL-terminated and length of each string matches the       */
/*    length specified in the argument list.                              */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    packet_pptr                           Pointer to packet             */ 
/*    status_code                           Status-code and reason-phrase */
/*    status_code_length                    Length of status-code         */
/*    content_length                        Length of content             */ 
/*    content_type                          Type of content               */
/*    content_type_length                   Length of content type        */ 
/*    additional_header                     Other HTTP headers            */
/*    additional_header_length              Length of other HTTP headers  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_generate_response_header                        */
/*                                          Generate response header      */
/*    _nx_utility_string_length_check       Check string length           */
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
UINT  _nx_web_http_server_callback_generate_response_header_extended(NX_WEB_HTTP_SERVER *server_ptr,
                                                                     NX_PACKET **packet_pptr, CHAR *status_code,
                                                                     UINT status_code_length, UINT content_length,
                                                                     CHAR *content_type, UINT content_type_length,
                                                                     CHAR *additional_header, UINT additional_header_length)
{
UINT temp_status_code_length = 0;
UINT temp_content_type_length = 0;
UINT temp_add_header_length = 0;

    /* Check status code, content type and additional header length.  */
    if (_nx_utility_string_length_check(status_code, &temp_status_code_length, status_code_length) ||
        (content_type && _nx_utility_string_length_check(content_type, &temp_content_type_length, content_type_length)) ||
        (additional_header &&  _nx_utility_string_length_check(additional_header, &temp_add_header_length, additional_header_length)))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* Validate string length.  */
    if ((status_code_length != temp_status_code_length) ||
        (content_type && content_type_length != temp_content_type_length) ||
        (additional_header && additional_header_length != temp_add_header_length))
    {
        return(NX_WEB_HTTP_ERROR);
    }

    /* Call the internal HTTP response send function.  */
    return _nx_web_http_server_generate_response_header(server_ptr, packet_pptr, status_code,
                                                        status_code_length, content_length,
                                                        content_type, content_type_length,
                                                        additional_header, additional_header_length);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_server_callback_packet_send           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks error for sending a packet.                    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    packet_ptr                            Pointer to packet             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_callback_packet_send                            */
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
UINT  _nxe_web_http_server_callback_packet_send(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{

    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) || (packet_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    return _nx_web_http_server_callback_packet_send(server_ptr, packet_ptr);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_callback_packet_send            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function is used in callback function to send a packet.        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    packet_ptr                            Pointer to packet             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_send              Send packet to client         */
/*    nx_packet_data_append                 Append packet data            */
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
UINT  _nx_web_http_server_callback_packet_send(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{
UINT status;
CHAR crlf[2] = {13,10};
UINT length = packet_ptr -> nx_packet_length;

    /* If the request is chunked, add CRLF at the end of the chunk.  */
    if (server_ptr -> nx_web_http_server_response_chunked)
    {

        /* Check the packet length.  */
        if (server_ptr -> nx_web_http_server_expect_transfer_bytes < (server_ptr -> nx_web_http_server_actual_bytes_transferred + length))
        {
            return(NX_INVALID_PACKET);
        }
        
        if (server_ptr -> nx_web_http_server_expect_transfer_bytes == (server_ptr -> nx_web_http_server_actual_bytes_transferred + length))
        {

            /* Place an extra CRLF to signal the end of the chunk.  */
            nx_packet_data_append(packet_ptr, crlf, 2, server_ptr -> nx_web_http_server_packet_pool_ptr, NX_WAIT_FOREVER);
        }

        /* Update the transferred bytes.  */
        server_ptr -> nx_web_http_server_actual_bytes_transferred += length;
    }

    /* Send internal. */
    status = _nx_web_http_server_send(server_ptr, packet_ptr, NX_WEB_HTTP_SERVER_TIMEOUT_SEND);
    if (status)
    {
        return(status);
    }

    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_server_gmt_callback_set               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks error for setting gmt callback function.       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    gmt_get                               Pointer to application's      */ 
/*                                            GMT time function           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_gmt_callback_set                                */
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
UINT  _nxe_web_http_server_gmt_callback_set(NX_WEB_HTTP_SERVER *server_ptr, VOID (*gmt_get)(NX_WEB_HTTP_SERVER_DATE *))
{

    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) || (gmt_get == NX_NULL))
        return(NX_PTR_ERROR);

    /* Send internal. */
    return _nx_web_http_server_gmt_callback_set(server_ptr, gmt_get);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_gmt_callback_set                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets gmt callback function.                           */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    gmt_get                               Pointer to application's      */ 
/*                                            GMT time function           */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
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
UINT  _nx_web_http_server_gmt_callback_set(NX_WEB_HTTP_SERVER *server_ptr, VOID (*gmt_get)(NX_WEB_HTTP_SERVER_DATE *))
{

TX_INTERRUPT_SAVE_AREA
    
    /* Lockout interrupts.  */
    TX_DISABLE

    /* Set the function pointer. */
    server_ptr -> nx_web_http_server_gmt_get = gmt_get;
    
    /* Restore interrupts.  */
    TX_RESTORE

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_server_cache_info_callback_set        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks error for setting cache info callback function.*/ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    cache_info_get                        Pointer to application's      */ 
/*                                            cache information function  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_cache_info_callback_set                         */
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
UINT  _nxe_web_http_server_cache_info_callback_set(NX_WEB_HTTP_SERVER *server_ptr, UINT (*cache_info_get)(CHAR *, UINT *, NX_WEB_HTTP_SERVER_DATE *))
{

    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) || (cache_info_get == NX_NULL))
        return(NX_PTR_ERROR);

    /* Send internal. */
    return _nx_web_http_server_cache_info_callback_set(server_ptr, cache_info_get);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_cache_info_callback_set         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets cache info callback function.                    */ 
/*                                                                        */
/*    Note: The string resource in callback functions authentication_check*/
/*    and request_notify is built by internal logic and always            */
/*    NULL-terminated.                                                    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    cache_info_get                        Pointer to application's      */ 
/*                                            cache information function  */ 
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
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_web_http_server_cache_info_callback_set(NX_WEB_HTTP_SERVER *server_ptr, UINT (*cache_info_get)(CHAR *, UINT *, NX_WEB_HTTP_SERVER_DATE *))
{

TX_INTERRUPT_SAVE_AREA
    
    /* Lockout interrupts.  */
    TX_DISABLE

    /* Set the function pointer. */
    server_ptr -> nx_web_http_server_cache_info_get = cache_info_get;
    
    /* Restore interrupts.  */
    TX_RESTORE

    return NX_SUCCESS;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_date_to_string                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts from date to string.                         */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    date                                  Pointer to HTTP date          */ 
/*    string                                Pointer to output buffer      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_date_convert                                    */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_web_http_server_get_process                                     */
/*    _nx_web_http_server_generate_response_header                        */
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
UINT  _nx_web_http_server_date_to_string(NX_WEB_HTTP_SERVER_DATE *date, CHAR *string)
{

UINT index = 0;

    /* RDC1122-date. */
    /* Append weekday. */
    string[index++] = _nx_web_http_server_weekday[date -> nx_web_http_server_weekday][0];
    string[index++] = _nx_web_http_server_weekday[date -> nx_web_http_server_weekday][1];
    string[index++] = _nx_web_http_server_weekday[date -> nx_web_http_server_weekday][2];
    string[index++] = ',';
    string[index++] = ' ';

    /* Convert and append day. */
    _nx_web_http_server_date_convert(date -> nx_web_http_server_day, 2, &string[index]);
    index += 2;
    string[index++] = ' ';

    /* Append month. */
    string[index++] = _nx_web_http_server_month[date -> nx_web_http_server_month][0];
    string[index++] = _nx_web_http_server_month[date -> nx_web_http_server_month][1];
    string[index++] = _nx_web_http_server_month[date -> nx_web_http_server_month][2];
    string[index++] = ' ';

    /* Convert and append year. */
    _nx_web_http_server_date_convert(date -> nx_web_http_server_year, 4, &string[index]);
    index += 4;
    string[index++] = ' ';

    /* Convert and append hour. */
    _nx_web_http_server_date_convert(date -> nx_web_http_server_hour, 2, &string[index]);
    index += 2;
    string[index++] = ':';

    /* Convert and append minute. */
    _nx_web_http_server_date_convert(date -> nx_web_http_server_minute, 2, &string[index]);
    index += 2;
    string[index++] = ':';

    /* Convert and append second. */
    _nx_web_http_server_date_convert(date -> nx_web_http_server_second, 2, &string[index]);
    index += 2;
    string[index++] = ' ';
    string[index++] = 'G';
    string[index++] = 'M';
    string[index++] = 'T';

    return index;
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_date_convert                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function converts a date from number to string.                */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    date                                  The number to convert         */ 
/*    count                                 Digital count for string      */ 
/*    string                                Pointer to output buffer      */ 
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
/*    _nx_web_http_server_date_to_string                                  */
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
VOID  _nx_web_http_server_date_convert(UINT date, UINT count, CHAR *string)
{

UINT    digit;

    /* Initialize all 4 bytes to digit 0. */
    *((ULONG*)string) = 0x30303030;
    string[count] = 0;

    /* Loop to convert the number to ASCII.  */
    while(count > 0)
    {
        count--;

        /* Compute the next decimal digit.  */
        digit =  date % 10;

        /* Update the input date.  */
        date =  date / 10;

        /* Store the new digit in ASCII form.  */
        string[count] =  (CHAR) (digit + 0x30);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_server_mime_maps_additional_set       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks error for setting additional MIME maps.        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    mime_maps                             Pointer MIME map array        */ 
/*    mime_maps_num                         Number of MIME map array      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_mime_maps_additional_set                        */
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
UINT  _nxe_web_http_server_mime_maps_additional_set(NX_WEB_HTTP_SERVER *server_ptr, NX_WEB_HTTP_SERVER_MIME_MAP *mime_maps, UINT mime_maps_num)
{

    /* Check for invalid input pointers.  */
    if ((server_ptr == NX_NULL) || (server_ptr -> nx_web_http_server_id != NX_WEB_HTTP_SERVER_ID) || (mime_maps == NX_NULL))
        return(NX_PTR_ERROR);

    /* Send internal. */
    return _nx_web_http_server_mime_maps_additional_set(server_ptr, mime_maps, mime_maps_num);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_mime_maps_additional_set        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets additional MIME maps.                            */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    mime_maps                             Pointer MIME map array        */ 
/*    mime_maps_num                         Number of MIME map array      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
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
UINT  _nx_web_http_server_mime_maps_additional_set(NX_WEB_HTTP_SERVER *server_ptr, NX_WEB_HTTP_SERVER_MIME_MAP *mime_maps, UINT mime_maps_num)
{

TX_INTERRUPT_SAVE_AREA
    
    /* Lockout interrupts.  */
    TX_DISABLE

    /* Set the mime maps. */
    server_ptr -> nx_web_http_server_mime_maps_additional = mime_maps;
    server_ptr -> nx_web_http_server_mime_maps_additional_num = mime_maps_num;
    
    /* Restore interrupts.  */
    TX_RESTORE

    return NX_SUCCESS;
}

#ifndef NX_WEB_HTTP_KEEPALIVE_DISABLE
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_get_client_keepalive            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function indicates whether this connection will keep alive.    */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    server_ptr                            Pointer to HTTP server        */ 
/*    packet_ptr                            Pointer to packet             */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    None                                                                */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_field_value_get   Get field value               */
/*    _nx_web_http_server_memicmp           Compare two strings           */
/*    _nx_utility_string_length_check       Check string length           */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    _nx_web_http_server_receive_data                                    */
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
VOID  _nx_web_http_server_get_client_keepalive(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr)
{
UCHAR   *ch;
UINT    version = 0;
UCHAR   connection_value[16];
UINT    connection_value_length;


    /* Locate to HTTP version. */
    ch = packet_ptr -> nx_packet_prepend_ptr;
    while(ch + 11 < packet_ptr -> nx_packet_append_ptr)
    {
        if((*ch == ' ') &&
           (*(ch + 1) == 'H') &&
           (*(ch + 2) == 'T') &&
           (*(ch + 3) == 'T') &&
           (*(ch + 4) == 'P') &&
           (*(ch + 5) == '/'))
        {

            /* Find the HTTP version. */
            ch += 6;
            if((*ch >= '0') && 
               (*ch <= '9') &&
               (*(ch + 1) == '.') &&
               (*(ch + 2) >= '0') &&
               (*(ch + 2) <= '9'))
            {

                /* Get the version. */
                version = (UCHAR)((*ch - '0') << 4) | (UCHAR)(*(ch + 2) - '0');
            }

            break;
        }
        else if((*ch == 13) && (*(ch + 1) == 10))
        {

            /* Find the end of line. */
            break;
        }

        /* Move to the next byte. */
        ch++;
    }

    /* Is a valid HTTP version found? */
    if(version == 0)
    {
        server_ptr -> nx_web_http_server_keepalive = NX_FALSE;
        return;
    }

    /* Initialize the keepalive flag. */
    if(version > 0x10)
        server_ptr -> nx_web_http_server_keepalive = NX_TRUE;
    else
        server_ptr -> nx_web_http_server_keepalive = NX_FALSE;

    /* Searching for 'connection' header. */
    if(_nx_web_http_server_field_value_get(packet_ptr, (UCHAR *)"connection", 10, connection_value, sizeof(connection_value)) == NX_SUCCESS)
    {
        _nx_utility_string_length_check((CHAR *)connection_value, &connection_value_length, sizeof(connection_value) - 1);

        if(_nx_web_http_server_memicmp(connection_value, connection_value_length, (UCHAR *)"keep-alive", 10) == NX_SUCCESS)
            server_ptr -> nx_web_http_server_keepalive = NX_TRUE;
        else if(_nx_web_http_server_memicmp(connection_value, connection_value_length, (UCHAR *)"close", 5) == NX_SUCCESS)
            server_ptr -> nx_web_http_server_keepalive = NX_FALSE;
    }
}
#endif /* NX_WEB_HTTP_KEEPALIVE_DISABLE */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_send                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends data to the remote host using either the TCP    */
/*    socket for plain HTTP or the TLS session for HTTPS.                 */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    packet_ptr                            Pointer to packet to send.    */
/*    wait_option                           Indicates behavior if packet  */
/*                                             cannot be sent.            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_session_send            Send data using TLS session   */
/*    nx_secure_tcp_socket_send             Send data using TCP socket    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
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
UINT  _nx_web_http_server_send(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET *packet_ptr, ULONG wait_option)
{

UINT        status;
NX_TCP_SOCKET *tcp_socket;
#ifdef NX_WEB_HTTPS_ENABLE
NX_SECURE_TLS_SESSION *tls_session;
#endif

    tcp_socket = &(server_ptr -> nx_web_http_server_current_session_ptr -> nx_tcp_session_socket);

#ifdef NX_WEB_HTTPS_ENABLE

    tls_session = &(server_ptr -> nx_web_http_server_current_session_ptr -> nx_tcp_session_tls_session);

    /* End TLS session if using HTTPS. */
    if(server_ptr -> nx_web_http_is_https_server)
    {
        status = nx_secure_tls_session_send(tls_session, packet_ptr, wait_option);
    }
    else
#endif
    {
        status = nx_tcp_socket_send(tcp_socket, packet_ptr, wait_option);
    }

    /* Return status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_receive                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives data from the remote host using either the   */
/*    TCP socket for plain HTTP or the TLS session for HTTPS.             */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    packet_ptr                            Pointer to packet to send.    */
/*    wait_option                           Indicates behavior if packet  */
/*                                             cannot be received         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_secure_tls_session_receive            Receive data using TLS     */
/*    nx_secure_tcp_socket_receive             Receive data using TCP     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_web_http_server_packet_get       Get next packet from HTTP      */
/*                                         server socket.                 */  
/*    _nx_web_http_server_get_client_request                              */
/*                                         Get complete HTTP request      */
/*    _nx_web_http_server_request_read     Get the next data in byte      */
/*    _nx_web_http_server_chunked_size_get Get chunk size of the request  */
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
UINT  _nx_web_http_server_receive(NX_WEB_HTTP_SERVER *server_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

UINT        status;
NX_TCP_SOCKET *tcp_socket;
#ifdef NX_WEB_HTTPS_ENABLE
NX_SECURE_TLS_SESSION *tls_session;
#endif

    tcp_socket = &(server_ptr -> nx_web_http_server_current_session_ptr -> nx_tcp_session_socket);

#ifdef NX_WEB_HTTPS_ENABLE
    tls_session = &(server_ptr -> nx_web_http_server_current_session_ptr -> nx_tcp_session_tls_session);

    /* End TLS session if using HTTPS. */
    if(server_ptr -> nx_web_http_is_https_server)
    {
        status = nx_secure_tls_session_receive(tls_session, packet_ptr, wait_option);
    }
    else
#endif
    {
        status = nx_tcp_socket_receive(tcp_socket, packet_ptr, wait_option);
    }

    /* Return status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_connection_reset                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used whenever the HTTP server needs to close the   */
/*    network connection and reset to receive new requests. If the server */
/*    is using TLS for HTTPS, then it also ends the TLS session.          */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    session_ptr                           Pointer to session            */
/*    error_number                          Error status in caller        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_web_http_server_connection_disconnect                           */
/*                                          End TLS session, and calls    */ 
/*                                          Disconnect and unaccept on the*/
/*                                          socket                        */
/*    _nx_tcp_server_socket_relisten        Reset socket to accept another*/
/*                                          connection                    */
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
VOID _nx_web_http_server_connection_reset(NX_WEB_HTTP_SERVER *server_ptr, NX_TCP_SESSION *session_ptr, UINT wait_option)
{
NX_TCP_SOCKET *tcp_socket;


    tcp_socket = &(session_ptr -> nx_tcp_session_socket);

    /* Disconnect the server connection. */
    _nx_web_http_server_connection_disconnect(server_ptr, session_ptr, wait_option);

    /* Relisten on the HTTP(S) Server port.  */
    nx_tcp_server_socket_relisten(server_ptr -> nx_web_http_server_ip_ptr, server_ptr -> nx_web_http_server_listen_port, tcp_socket);

}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_connection_disconnect           PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is used whenever the HTTP server needs to close the   */
/*    network connection completely. If the server is using TLS for HTTPS */
/*    then it also ends the TLS session.                                  */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            Pointer to HTTP server        */
/*    session_ptr                           Pointer to session            */
/*    wait_option                           Wait option for TCP socket    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_session_end            End TLS session               */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported random nonce,     */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
VOID _nx_web_http_server_connection_disconnect(NX_WEB_HTTP_SERVER *server_ptr, NX_TCP_SESSION *session_ptr, UINT wait_option)
{
NX_TCP_SOCKET *tcp_socket;
#ifdef NX_WEB_HTTPS_ENABLE
NX_SECURE_TLS_SESSION *tls_session;
#endif /* NX_WEB_HTTPS_ENABLE */
#ifdef NX_WEB_HTTP_DIGEST_ENABLE
UINT i;
#endif /* NX_WEB_HTTP_DIGEST_ENABLE  */

#ifndef NX_WEB_HTTPS_ENABLE
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(wait_option);
#endif /* NX_WEB_HTTPS_ENABLE */

#ifdef NX_WEB_HTTP_DIGEST_ENABLE

    /* Once the nonce has been accepted, set the state as invalid. */
    for (i = 0; i < NX_WEB_HTTP_SERVER_NONCE_MAX; i++)
    {
        if ((server_ptr -> nx_web_http_server_nonces[i].nonce_state == NX_WEB_HTTP_SERVER_NONCE_ACCEPTED) &&
            (server_ptr -> nx_web_http_server_nonces[i].nonce_session_ptr == session_ptr))
        {
            server_ptr -> nx_web_http_server_nonces[i].nonce_state = NX_WEB_HTTP_SERVER_NONCE_INVALID;
            break;
        }
    }
#endif /* NX_WEB_HTTP_DIGEST_ENABLE  */

    tcp_socket = &(session_ptr -> nx_tcp_session_socket);

#ifdef NX_WEB_HTTPS_ENABLE

    tls_session = &(session_ptr -> nx_tcp_session_tls_session);

    /* End TLS session if using HTTPS. */
    if(server_ptr -> nx_web_http_is_https_server)
    {
        _nx_secure_tls_session_end(tls_session, wait_option);
    }
#endif

    /* Disconnect from the current connection.  */
    nx_tcp_socket_disconnect(tcp_socket, NX_WEB_HTTP_SERVER_TIMEOUT_DISCONNECT);

    /* Unaccept the connection.  */
    nx_tcp_server_socket_unaccept(tcp_socket);

}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_web_http_server_digest_authenticate_notify_set PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs error checking for service to set digest     */ 
/*    authenticate callback function.                                     */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    http_server_ptr                       Pointer to HTTP server        */ 
/*    digest_authenticate_callback          Pointer to application's      */ 
/*                                          digest authenticate callback  */ 
/*                                          function                      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_web_http_server_digest_authenticate_notify_set                  */
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
UINT _nxe_web_http_server_digest_authenticate_notify_set(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                         UINT (*digest_authenticate_callback)(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name_ptr,
                                                         CHAR *realm_ptr, CHAR *password_ptr, CHAR *method,
                                                         CHAR *authorization_uri, CHAR *authorization_nc,
                                                         CHAR *authorization_cnonce)) 
{

    /* Check for invalid input pointers.  */
    if ((http_server_ptr == NX_NULL) || (digest_authenticate_callback == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    return(_nx_web_http_server_digest_authenticate_notify_set(http_server_ptr, digest_authenticate_callback));
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_web_http_server_digest_authenticate_notify_set  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets digest authenticate callback function.           */ 
/*                                                                        */
/*    Note: The strings name_ptr, realm_ptr, password_ptr, method,        */
/*    authorization_uri, authorization_nc and authorization_cnonce in     */
/*    callback function digest_authenticate_callback is built by internal */
/*    logic and always NULL-terminated.                                   */
/*                                                                        */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    http_server_ptr                       Pointer to HTTP server        */ 
/*    digest_authenticate_callback          Pointer to application's      */ 
/*                                          digest authenticate callback  */ 
/*                                          function                      */ 
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
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_web_http_server_digest_authenticate_notify_set(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                        UINT (*digest_authenticate_callback)(NX_WEB_HTTP_SERVER *server_ptr, CHAR *name_ptr,
                                                        CHAR *realm_ptr, CHAR *password_ptr, CHAR *method,
                                                        CHAR *authorization_uri, CHAR *authorization_nc,
                                                        CHAR *authorization_cnonce)) 
{

#ifdef  NX_WEB_HTTP_DIGEST_ENABLE
    http_server_ptr -> nx_web_http_server_digest_authenticate_callback = digest_authenticate_callback;

    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(http_server_ptr);
    NX_PARAMETER_NOT_USED(digest_authenticate_callback);

    return(NX_NOT_SUPPORTED);
#endif
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_web_http_server_authentication_check_set       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks errors for setting authentication checking     */
/*    callback function.                                                  */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    authentication_check_extended         Pointer to application's      */
/*                                            authentication checking     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_http_server_authentication_check_set                            */
/*                                          Set authentication callback   */
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
UINT  _nxe_web_http_server_authentication_check_set(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                    UINT (*authentication_check_extended)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, UINT *name_length, CHAR **password, UINT *password_length, CHAR **realm, UINT *realm_length))
{
UINT status;

    /* Check for invalid input pointers.  */
    if ((http_server_ptr == NX_NULL) || (authentication_check_extended == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call the actual authentication checking set function.  */
    status= _nx_web_http_server_authentication_check_set(http_server_ptr, authentication_check_extended);

    /* Return staus.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_web_http_server_authentication_check_set        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the callback function of authentication checking.*/
/*                                                                        */
/*    Note: The strings of name, password and realm must be               */
/*    NULL-terminated and length of each string matches the length        */
/*    specified in the argument list. The string resource in callback     */
/*    function authentication_check and request_notify is built by        */
/*    internal logic and always NULL-terminated.                          */
/*                                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    http_server_ptr                       Pointer to HTTP server        */
/*    authentication_check_extended         Pointer to application's      */
/*                                            authentication checking     */
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
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_web_http_server_authentication_check_set(NX_WEB_HTTP_SERVER *http_server_ptr,
                                                  UINT (*authentication_check_extended)(NX_WEB_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, CHAR **name, UINT *name_length, CHAR **password, UINT *password_length, CHAR **realm, UINT *realm_length))
{

    /* Set the extended authentication checking function.  */
    http_server_ptr -> nx_web_http_server_authentication_check_extended = authentication_check_extended;

    /* Return success.  */
    return(NX_SUCCESS);
}