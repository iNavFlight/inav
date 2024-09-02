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
/**   Trivial File Transfer Protocol (TFTP) Client                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_TFTP_SOURCE_CODE


/* Force error checking to be disabled in this module */

#ifndef NX_DISABLE_ERROR_CHECKING
#define NX_DISABLE_ERROR_CHECKING
#endif


/* Include necessary system files.  */

#include    "nx_api.h"
#include    "nx_ip.h"
#include    "nx_ipv6.h"
#include    "nxd_tftp_client.h"     

/* Bring in externs for caller checking code.  */

NX_CALLER_CHECKING_EXTERNS


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */
/*    _nxde_tftp_client_create                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TFTP client create call.     */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    tftp_client_name                      Name of TFTP client           */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    pool_ptr                              Pointer to packet pool        */ 
/*    ip_type                               IP type                       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_PARAMETERS                 Invalid non pointer input     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nxd_tftp_client_create               Actual client create call     */ 
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
UINT  _nxde_tftp_client_create(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *tftp_client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, UINT ip_type)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((ip_ptr == NX_NULL) || (ip_ptr -> nx_ip_id != NX_IP_ID) ||   
        (tftp_client_ptr == NX_NULL) || (tftp_client_ptr -> nx_tftp_client_id == NXD_TFTP_CLIENT_ID) || 
        (pool_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for valid IP version*/
    if ((ip_type != NX_IP_VERSION_V4) && (ip_type != NX_IP_VERSION_V6))            
    {
        return NX_INVALID_PARAMETERS;
    }
                               
    /* Call actual client create function.  */
    status =  _nxd_tftp_client_create(tftp_client_ptr, tftp_client_name, ip_ptr, pool_ptr, ip_type);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_tftp_client_create                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function creates a TFTP client on the specified IP. In doing   */ 
/*    so this function creates an UDP socket for subsequent TFTP          */ 
/*    transfers.                                                          */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    tftp_client_name                      Name of TFTP client           */ 
/*    ip_ptr                                Pointer to IP instance        */ 
/*    pool_ptr                              Pointer to TFTP pool          */ 
/*    ip_type                               IP type                       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_bind                    Bind the UDP socket           */ 
/*    nx_udp_socket_create                  Create a UDP socket           */ 
/*    nx_udp_socket_delete                  Delete the UDP socket         */ 
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
UINT  _nxd_tftp_client_create(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *tftp_client_name, NX_IP *ip_ptr, NX_PACKET_POOL *pool_ptr, UINT ip_type)
{

UINT    status;


    /* Clear the TFTP server structure.  */
    memset((void *) tftp_client_ptr, 0, sizeof(NX_TFTP_CLIENT));

    /* Setup the TFTP client data structure.  */

    /* Save the TFTP client name.  */
    tftp_client_ptr -> nx_tftp_client_name =  tftp_client_name;

    /* Save the TFTP IP pointer.  */
    tftp_client_ptr -> nx_tftp_client_ip_ptr =  ip_ptr;

    /* Save the TFTP packet pool pointer.  */
    tftp_client_ptr -> nx_tftp_client_packet_pool_ptr =  pool_ptr;

    /* Initialize the server port number.  This can change after the open request.  */
    tftp_client_ptr -> nx_tftp_client_server_port =  NX_TFTP_SERVER_PORT;

    /* Setup the current state to not open.  */
    tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_NOT_OPEN;

    /* Default the client network interface to the primary interface. */
    tftp_client_ptr -> nx_tftp_client_interface_index = 0;

    /* Create the UDP socket.  */
    status =  nx_udp_socket_create(ip_ptr, &(tftp_client_ptr -> nx_tftp_client_socket), tftp_client_name, 
                          NX_TFTP_TYPE_OF_SERVICE,  NX_TFTP_FRAGMENT_OPTION, NX_TFTP_TIME_TO_LIVE, NX_TFTP_QUEUE_DEPTH);

    /* Determine if an error occurred.   */
    if (status)
    {

        /* Yes, return error code.  */
        return(status);
    }

    /* Now, bind the socket to a port number.  */

    /* Let NetX decide which port. */
    status =  nx_udp_socket_bind(&(tftp_client_ptr -> nx_tftp_client_socket), NX_TFTP_SOURCE_PORT, NX_WAIT_FOREVER);

    /* Determine if an error occurred.  */
    if (status)
    {

        /* Delete the UDP socket.  */
        nx_udp_socket_delete(&(tftp_client_ptr -> nx_tftp_client_socket));

        /* Yes, return error code.  */
        return(status);
    }
                                                
    /* Otherwise, all is okay.  Set the TFTP ID to indicate success.  */
    tftp_client_ptr -> nx_tftp_client_id =  NXD_TFTP_CLIENT_ID;

    NX_PARAMETER_NOT_USED(ip_type);

    /* Return success to the caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */  
/*    _nxde_tftp_client_delete                            PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TFTP client delete call.     */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxd_tftp_client_delete               Actual client delete call     */ 
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
UINT  _nxde_tftp_client_delete(NX_TFTP_CLIENT *tftp_client_ptr)
{

UINT    status;

                                         
    /* Check for invalid input pointers.  */                
    if ((tftp_client_ptr == NX_NULL) || (tftp_client_ptr -> nx_tftp_client_id != NXD_TFTP_CLIENT_ID))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                                           
    /* Call actual client delete function.  */
    status =  _nxd_tftp_client_delete(tftp_client_ptr);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_tftp_client_delete                             PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function deletes a TFTP client on the specified IP.            */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_unbind                  Unbind the UDP socket         */ 
/*    nx_udp_socket_delete                  Delete the UDP socket         */ 
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
UINT  _nxd_tftp_client_delete(NX_TFTP_CLIENT *tftp_client_ptr)
{

    /* Unbind the TFTP client UDP socket.  */
    nx_udp_socket_unbind(&(tftp_client_ptr -> nx_tftp_client_socket));

    /* Delete the TFTP client UDP socket.  */
    nx_udp_socket_delete(&(tftp_client_ptr -> nx_tftp_client_socket));

    /* Clear the TFTP client ID.  */
    tftp_client_ptr -> nx_tftp_client_id =  0;

    /* Return success to the caller.  */
    return(NX_SUCCESS);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */  
/*    _nxde_tftp_client_set_interface                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TFTP client set interface    */
/*    call.                                                               */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    if_index                              Interface for tftp messages   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */ 
/*    NX_INVALID_INTERFACE                  Invalid interface index input */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxd_tftp_client_set_interface        Actual set interface call     */ 
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
UINT  _nxde_tftp_client_set_interface(NX_TFTP_CLIENT *tftp_client_ptr, UINT if_index)
{

UINT    status;

    /* Check for invalid input pointer input.  */
    if (tftp_client_ptr == NX_NULL) 
        return(NX_PTR_ERROR);

    /* Verify a valid index as been supplied. */
    if (if_index >= NX_MAX_PHYSICAL_INTERFACES)
    {                                         
        return NX_TFTP_INVALID_INTERFACE;
    }              

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                                       
    /* Call actual client delete function.  */
    status =  _nxd_tftp_client_set_interface(tftp_client_ptr, if_index);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */   
/*    _nxd_tftp_client_set_interface                      PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function sets the physical interface which the TFTP client     */
/*    sends and receives TFTP packets.                                    */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    if_index                              Interface for tftp messages   */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */ 
/*    NX_INVALID_INTERFACE                  Invalid interface index input */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_udp_socket_unbind                  Unbind the UDP socket         */ 
/*    nx_udp_socket_delete                  Delete the UDP socket         */ 
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
UINT  _nxd_tftp_client_set_interface(NX_TFTP_CLIENT *tftp_client_ptr, UINT if_index)
{


    /* Set the TFTP client interface index.  */
    tftp_client_ptr -> nx_tftp_client_interface_index = if_index;

    /* Return success to the caller.  */
    return(NX_SUCCESS);
}



/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */     
/*    _nxde_tftp_client_error_info_get                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TFTP error information       */ 
/*    get call.                                                           */
/*                                                                        */   
/*    Note: error_string pointer points to string generated by internal   */
/*    logic and it is always NULL-terminated.                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    error_code                            Pointer to destination for    */ 
/*                                            error code                  */ 
/*    error_string                          Pointer to destination for    */ 
/*                                            the error string            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nxd_tftp_client_error_info_get        Actual get error info call   */ 
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
UINT  _nxde_tftp_client_error_info_get(NX_TFTP_CLIENT *tftp_client_ptr, UINT *error_code, CHAR **error_string)
{

UINT    status;


    /* Check for invalid input pointers.  */           
    if ((tftp_client_ptr == NX_NULL) || (tftp_client_ptr -> nx_tftp_client_id != NXD_TFTP_CLIENT_ID) ||
        (error_code == NX_NULL) || (error_string == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                                                     
    /* Call actual client error information get function.  */
    status =  _nxd_tftp_client_error_info_get(tftp_client_ptr, error_code, error_string);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_tftp_client_error_info_get                     PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function picks up the error code and error string for the      */ 
/*    specified TFTP client instance.                                     */ 
/*                                                                        */ 
/*    Note: error_string pointer points to string generated by internal   */
/*    logic and it is always NULL-terminated.                             */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    error_code                            Pointer to destination for    */ 
/*                                            error code                  */ 
/*    error_string                          Pointer to destination for    */ 
/*                                            the error string            */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                           Successful completion status   */ 
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
UINT  _nxd_tftp_client_error_info_get(NX_TFTP_CLIENT *tftp_client_ptr, UINT *error_code, CHAR **error_string)
{

    /* Return the error code and the error string.  */
    *error_code =    tftp_client_ptr -> nx_tftp_client_error_code;
    *error_string =  &(tftp_client_ptr -> nx_tftp_client_error_string[0]);

    /* Return success to the caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */  
/*    _nxde_tftp_client_file_close                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TFTP file close call.        */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */  
/*    ip_type                               IP type                       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_PARAMETERS                 Invalid non pointer input     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxd_tftp_client_file_close           Actual client file close      */ 
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
UINT  _nxde_tftp_client_file_close(NX_TFTP_CLIENT *tftp_client_ptr, UINT ip_type)
{

UINT    status;


    /* Check for invalid input pointers.  */       
    if ((tftp_client_ptr == NX_NULL) || (tftp_client_ptr -> nx_tftp_client_id != NXD_TFTP_CLIENT_ID))
        return(NX_PTR_ERROR);

    /* Check for valid IP version*/
    if ((ip_type != NX_IP_VERSION_V4) && (ip_type != NX_IP_VERSION_V6))
    {
        return NX_INVALID_PARAMETERS;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                              
    /* Call actual client file close function.  */
    status =  _nxd_tftp_client_file_close(tftp_client_ptr, ip_type);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_tftp_client_file_close                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function closes the previously opened TFTP file.               */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */   
/*    ip_type                               IP type                       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    NX_SUCCESS                            Successful completion status  */
/*    status                                Actual completion status      */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet for 0-length  */ 
/*                                            data packet                 */ 
/*    nxd_udp_socket_send                   Send UDP data packet          */ 
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
UINT  _nxd_tftp_client_file_close(NX_TFTP_CLIENT *tftp_client_ptr, UINT ip_type)
{

NX_PACKET   *packet_ptr;
UCHAR       *buffer_ptr;
UINT        status;


    /* Determine if the file is still open for writing.  */
    if (tftp_client_ptr -> nx_tftp_client_state == NX_TFTP_STATE_WRITE_OPEN)
    {

        /* Allocate a new packet for the final data message.  */
        if (ip_type == NX_IP_VERSION_V4)
        {
            status =  nx_packet_allocate(tftp_client_ptr -> nx_tftp_client_packet_pool_ptr, &packet_ptr, NX_IPv4_UDP_PACKET, NX_NO_WAIT);
        }
        else
        {
            status =  nx_packet_allocate(tftp_client_ptr -> nx_tftp_client_packet_pool_ptr, &packet_ptr, NX_IPv6_UDP_PACKET, NX_NO_WAIT);
        }

        /* Determine if an error occurred trying to allocate a packet.  */
        if (status != NX_SUCCESS)
        {

            /* Enter error state.  */
            tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

            /* Return error condition.  */
            return(status);
        }

        if (4u > ((ULONG)(packet_ptr -> nx_packet_data_end) - (ULONG)(packet_ptr -> nx_packet_append_ptr)))
        {
            /* Return the unsent packet to the packet pool. */
            nx_packet_release(packet_ptr);

            /* Enter error state.  */
            tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

            return(NX_SIZE_ERROR);
        }

        packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr;

        /* Now, build the TFTP 0-data length message.  */

        /* Setup a pointer to the packet payload.  */
        buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

        /* Initial byte is always zero.  */
        *buffer_ptr++ =  0;

        /* Set the ACK code.  */
        *buffer_ptr++ =  NX_TFTP_CODE_DATA;

        /* Put the block number in.  */
        *buffer_ptr++ =  (UCHAR) ((UCHAR) (tftp_client_ptr -> nx_tftp_client_block_number >> 8));
        *buffer_ptr =    (UCHAR) ((UCHAR) (tftp_client_ptr -> nx_tftp_client_block_number & 0xFF));

        /* Adjust the packet pointers and length.  */
        packet_ptr -> nx_packet_length =  4;
        packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_append_ptr + 4;

        /* Send the 0-length data packet out.  */
        status = nxd_udp_socket_send(&(tftp_client_ptr -> nx_tftp_client_socket), packet_ptr, 
                                     &tftp_client_ptr -> nx_tftp_client_server_ip, 
                                     tftp_client_ptr -> nx_tftp_client_server_port);

        /* Determine if an error occurred trying to sending the packet.  */
        if (status)
        {

            /* Return the unsent packet to the packet pool. */
            nx_packet_release(packet_ptr);

            /* Enter error state.  */
            tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

            /* Return error condition.  */
            return(status);
        }
    }

    /* Indicate the client is finished with the file. */
    tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_FINISHED;

    /* Return success to the caller.  */
    return(NX_SUCCESS);
}
               

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxe_tftp_client_file_open                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TFTP file open call.         */ 
/*                                                                        */   
/*    Note: The string length of file_name is limited by the packet       */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    file_name                             Pointer to file name          */ 
/*    server_ip_address                     IP address of TFTP server     */ 
/*    open_type                             Open for read or write        */ 
/*    wait_option                           Timeout for the open request  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_IP_ADDRESS_ERROR                   Invalid address supplied      */ 
/*    NX_OPTION_ERROR                       Invalid TFTP option supplied  */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_tftp_client_file_open             Actual client file open       */ 
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
UINT  _nxe_tftp_client_file_open(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *file_name, ULONG server_ip_address, UINT open_type, ULONG wait_option)
{

#ifndef NX_DISABLE_IPV4
UINT    status;


    /* Check for invalid input pointers.  */      
    if ((tftp_client_ptr == NX_NULL) || (tftp_client_ptr -> nx_tftp_client_id != NXD_TFTP_CLIENT_ID))
        return(NX_PTR_ERROR);

    /* Check for an invalid server IP address.  */
    if (server_ip_address == 0)
        return(NX_IP_ADDRESS_ERROR);

    /* Check for illegal open option type. */
    if ((open_type != NX_TFTP_OPEN_FOR_READ) && (open_type != NX_TFTP_OPEN_FOR_WRITE))
        return(NX_OPTION_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client file open function.  */
    status =  _nx_tftp_client_file_open(tftp_client_ptr, file_name, server_ip_address, open_type, wait_option);

    /* Return completion status.  */
    return(status);
#else
    NX_PARAMETER_NOT_USED(tftp_client_ptr);
    NX_PARAMETER_NOT_USED(file_name);
    NX_PARAMETER_NOT_USED(server_ip_address);
    NX_PARAMETER_NOT_USED(open_type);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_client_file_open                           PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function opens a TFTP file.                                    */
/*                                                                        */ 
/*    Note: The string length of file_name is limited by the packet       */
/*    payload size.                                                       */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    file_name                             Pointer to file name          */ 
/*    tftp_server_address                   IP address of TFTP server     */ 
/*    open_type                             Open for read or write        */ 
/*    wait_option                           Timeout for the open request  */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nx_tftp_client_file_open_internal    Actual open service           */ 
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
UINT  _nx_tftp_client_file_open(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *file_name, ULONG server_ip_address, UINT open_type, ULONG wait_option)
{
#ifndef NX_DISABLE_IPV4
UINT        status;


NXD_ADDRESS server_address;

    /* Create the IPv4 address block. Set the version to IPv4. */
    server_address.nxd_ip_version = NX_IP_VERSION_V4;
    server_address.nxd_ip_address.v4 = server_ip_address;

    /* Call the actual file open service. */
    status = _nx_tftp_client_file_open_internal(tftp_client_ptr, file_name, &server_address, open_type, wait_option, NX_IP_VERSION_V4);


    /* Return the completion status. */
    return status;
#else
    NX_PARAMETER_NOT_USED(tftp_client_ptr);
    NX_PARAMETER_NOT_USED(file_name);
    NX_PARAMETER_NOT_USED(server_ip_address);
    NX_PARAMETER_NOT_USED(open_type);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_DISABLE_IPV4 */
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_tftp_client_file_open                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TFTP file open call.         */
/*                                                                        */
/*    Note: The string length of file_name is limited by the packet       */
/*    payload size.                                                       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    file_name                             Pointer to file name          */ 
/*    server_ip_address                     IP address of TFTP server     */ 
/*    open_type                             Open for read or write        */ 
/*    wait_option                           Timeout for the open request  */ 
/*    ip_type                               IP type                       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_PARAMETERS                 Invalid non pointer input     */
/*    NX_IP_ADDRESS_ERROR                   Invalid address supplied      */ 
/*    NX_OPTION_ERROR                       Invalid TFTP option supplied  */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nxd_tftp_client_file_open            Actual client file open       */ 
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
UINT  _nxde_tftp_client_file_open(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *file_name, NXD_ADDRESS *server_ip_address, UINT open_type, ULONG wait_option, UINT ip_type)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((tftp_client_ptr == NX_NULL) || (tftp_client_ptr -> nx_tftp_client_id != NXD_TFTP_CLIENT_ID))
        return(NX_PTR_ERROR);

    /* Check for an invalid server IP address.  */
    if (!server_ip_address)
        return(NX_IP_ADDRESS_ERROR);

    /* Check for valid IP version*/
    if ((ip_type != NX_IP_VERSION_V4) && (ip_type != NX_IP_VERSION_V6))
    {
        return NX_INVALID_PARAMETERS;
    }

    /* Check for illegal open option type. */
    if ((open_type != NX_TFTP_OPEN_FOR_READ) && (open_type != NX_TFTP_OPEN_FOR_WRITE))
        return(NX_OPTION_ERROR);

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    /* Call actual client file open function.  */
    status =  _nxd_tftp_client_file_open(tftp_client_ptr, file_name, server_ip_address, open_type, wait_option, ip_type);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_tftp_client_file_open                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function opens a TFTP file received over IPv4 or IPv6 networks.*/ 
/*                                                                        */
/*    Note: The string length of file_name is limited by the packet       */
/*    payload size.                                                       */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    file_name                             Pointer to file name          */ 
/*    tftp_server_address                   IP address of TFTP server     */ 
/*    open_type                             Open for read or write        */ 
/*    wait_option                           Timeout for the open request  */ 
/*    ip_type                               IP type                       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_TFTP_INVALID_IP_VERSION            Unsupported IP protocol       */ 
/*    NX_TFTP_NO_ACK_RECEIVED               ACK not received from Server  */
/*    NX_TFTP_NOT_CLOSED                    TFTP client file already open */
/*    NX_TFTP_INVALID_SERVER_ADDRESS        Reply from unknown Server     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _nx_tftp_client_file_open_internal    Actual open service           */ 
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
UINT  _nxd_tftp_client_file_open(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *file_name, NXD_ADDRESS *server_ip_address, UINT open_type, ULONG wait_option, UINT ip_type)
{

UINT        status;

                    
    /* Call the actual file open service. */
    status = _nx_tftp_client_file_open_internal(tftp_client_ptr, file_name, server_ip_address, open_type, wait_option, ip_type);  
                         
    /* Return the completion status. */
    return status;
}    

    
/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nx_tftp_client_file_open_internal                  PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function opens a TFTP file received.                           */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    file_name                             Pointer to file name          */ 
/*    tftp_server_address                   IP address of TFTP server     */ 
/*    open_type                             Open for read or write        */ 
/*    wait_option                           Timeout for the open request  */  
/*    ip_type                               IP type                       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_TFTP_INVALID_IP_VERSION            Unsupported IP protocol       */ 
/*    NX_TFTP_NO_ACK_RECEIVED               ACK not received from Server  */
/*    NX_TFTP_NOT_CLOSED                    TFTP client file already open */
/*    NX_TFTP_INVALID_SERVER_ADDRESS        Reply from unknown Server     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet for TFTP      */ 
/*                                            request or data             */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_udp_socket_receive                 Receive UDP data packet       */ 
/*    nxd_udp_socket_send                   Send UDP data packet          */ 
/*    nxd_udp_source_extract                Extract IP and port from msg  */ 
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
UINT  _nx_tftp_client_file_open_internal(NX_TFTP_CLIENT *tftp_client_ptr, CHAR *file_name, NXD_ADDRESS *server_ip_address, UINT open_type, ULONG wait_option, UINT  ip_type)

{

UINT        status;
UINT        i;
NXD_ADDRESS ip_address;

UINT        port;
UCHAR       *buffer_ptr;
NX_PACKET   *packet_ptr;
UINT        matching = NX_FALSE;


    /* Determine if the TFTP instance is already open.  */
    if ((tftp_client_ptr -> nx_tftp_client_state == NX_TFTP_STATE_OPEN) ||
        (tftp_client_ptr -> nx_tftp_client_state == NX_TFTP_STATE_WRITE_OPEN))
    {

        /* This instance is already open, return an error.  */
        return(NX_TFTP_NOT_CLOSED);
    }

    /* Enter the open state.  */
    tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_OPEN;

    /* Save the server's IP address.  */

    if (ip_type == NX_IP_VERSION_V6)
    {
      
#ifndef FEATURE_NX_IPV6
        return NX_TFTP_INVALID_IP_VERSION;
#else        
        COPY_NXD_ADDRESS(server_ip_address, &tftp_client_ptr -> nx_tftp_client_server_ip);
#endif        
    }
    else
    {

#ifndef NX_DISABLE_IPV4
        /* If not IPv6, assume the client IP request is using IPv4 if not explicitly specified. */
        tftp_client_ptr -> nx_tftp_client_server_ip.nxd_ip_version = ip_type;
        tftp_client_ptr -> nx_tftp_client_server_ip.nxd_ip_address.v4 = server_ip_address -> nxd_ip_address.v4;
#else
        return NX_TFTP_INVALID_IP_VERSION;
#endif /* NX_DISABLE_IPV4 */
    }
    
    /* Initialize the server port number.  This can change after the open request.  */
    tftp_client_ptr -> nx_tftp_client_server_port =  NX_TFTP_SERVER_PORT;

    /* Clear the error code.  */
    tftp_client_ptr -> nx_tftp_client_error_code =    0;

    /* Specify that the first block.  */
    tftp_client_ptr -> nx_tftp_client_block_number =  1;

    /* Allocate a packet for initial open request message. Determine whether we are sending
       IPv4 or IPv6 packets.   */

    if (ip_type == NX_IP_VERSION_V4)
    {

        status =  nx_packet_allocate((tftp_client_ptr -> nx_tftp_client_ip_ptr) -> nx_ip_default_packet_pool, 
                                     &packet_ptr, NX_IPv4_UDP_PACKET, wait_option);
    }
    else
    {

        status =  nx_packet_allocate((tftp_client_ptr -> nx_tftp_client_ip_ptr) -> nx_ip_default_packet_pool, 
                                     &packet_ptr, NX_IPv6_UDP_PACKET, wait_option);
    }

    /* Determine if an error occurred trying to allocate a packet.  */
    if (status != NX_SUCCESS)
    {

        /* Enter error state.  */
        tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

        /* Return error condition.  */
        return(status);
    }

    packet_ptr -> nx_packet_append_ptr = packet_ptr -> nx_packet_prepend_ptr;

    /* Now, build the TFTP open request.  */

    /* Setup a pointer to the packet payload.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Initial byte is always zero.  */
    *buffer_ptr++ =  0;

    /* Determine the type of open requested.  */
    if (open_type == NX_TFTP_OPEN_FOR_READ)
        *buffer_ptr++ =  NX_TFTP_CODE_READ;
    else
        *buffer_ptr++ =  NX_TFTP_CODE_WRITE;

    /* Now place the file name in the buffer.  */
    i = 0;
    while (file_name[i] && (ULONG)(packet_ptr -> nx_packet_data_end - buffer_ptr) > 7u)
    {

        /* Store character of file name into request.  */
        *buffer_ptr++ =  (UCHAR) file_name[i];
        i++;
    }

    /* Place a NULL after the file name.  */
    *buffer_ptr++ =  NX_NULL;

    /* Now place the mode string.  */
    *buffer_ptr++ =  (UCHAR) 'O';
    *buffer_ptr++ =  (UCHAR) 'C';
    *buffer_ptr++ =  (UCHAR) 'T';
    *buffer_ptr++ =  (UCHAR) 'E';
    *buffer_ptr++ =  (UCHAR) 'T';

    /* Place a NULL after the mode.  */
    *buffer_ptr++ =  NX_NULL;

    /* Adjust the packet length and the append pointer.  */
    packet_ptr -> nx_packet_append_ptr =  buffer_ptr;
    packet_ptr -> nx_packet_length =  (ULONG)(buffer_ptr - packet_ptr -> nx_packet_prepend_ptr);

    /* Now send the open request to the TFTP server.  */
    status =  nxd_udp_socket_send(&(tftp_client_ptr -> nx_tftp_client_socket), packet_ptr, 
                                    server_ip_address, tftp_client_ptr -> nx_tftp_client_server_port);


    /* Check for error condition.  */
    if (status != NX_SUCCESS)
    {

        nx_packet_release(packet_ptr); 

        /* Enter error state.  */
        tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

        /* Return error condition.  */
        return(status);
    }

    /* Determine if the TFTP file was open for writing.  */
    if (open_type == NX_TFTP_OPEN_FOR_WRITE)
    {

        /* Change to the open for writing state.  */
        tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_WRITE_OPEN;

        /* Open for write request is present.  We now need to wait for the ACK of block number 0
           from the server.  */
        status =  nx_udp_socket_receive(&(tftp_client_ptr -> nx_tftp_client_socket), &packet_ptr, wait_option);

        /* Check the return status.  */
        if (status != NX_SUCCESS)
        {

            /* Enter error state.  */
            tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

            /* Return error condition.  */
            return(status);
        }

        /* Check for valid packet length (The minimum TFTP header size is ACK packet, four bytes).  */
        if (packet_ptr -> nx_packet_length < 4)
        {

            /* Release the packet. */
            nx_packet_release(packet_ptr);

            /* Return.  */
            return(NX_INVALID_PACKET);
        }

        /* Extract the source IP and port numbers.  */
        status = nxd_udp_source_extract(packet_ptr, &ip_address, &port);


        /* Check for an invalid server IP address.  */

#ifndef NX_DISABLE_IPV4
        if ((ip_type == NX_IP_VERSION_V4)&&
           (ip_address.nxd_ip_address.v4 == tftp_client_ptr -> nx_tftp_client_server_ip.nxd_ip_address.v4))
        {

            matching = NX_TRUE;
        }
        else
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
        if (ip_type == NX_IP_VERSION_V6)
        {

            if (CHECK_IPV6_ADDRESSES_SAME(&ip_address.nxd_ip_address.v6[0], &tftp_client_ptr -> nx_tftp_client_server_ip.nxd_ip_address.v6[0]))
            {
    
                matching = NX_TRUE;
            }
        }
        else
#endif
        {
            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            return NX_TFTP_INVALID_IP_VERSION;
        }

        /* Do we have a match? */
        if (!matching)
        {
            /* No, invalid IP address!  */

            /* Enter error state.  */
            tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Return error condition.  */
            return(NX_TFTP_INVALID_SERVER_ADDRESS);
        }
    
        /* Save the source port since the server can change its port.  */
        tftp_client_ptr -> nx_tftp_client_server_port =  port;

        /* Check for valid server ACK.  */

        /* Setup a pointer to the packet payload.  */
        buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

        /* Check for valid ACK message.  */
        if ((buffer_ptr[0] != 0) || (buffer_ptr[1] != NX_TFTP_CODE_ACK) || 
            (buffer_ptr[2] != 0) || (buffer_ptr[3] != 0) ||
            (packet_ptr -> nx_packet_length != 4))
        {

            UINT code;

            /* Enter error state.  */
            tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

            /* Get the error status from the server message. */
            buffer_ptr++;
            code =  *(buffer_ptr);

            /* If this is a server error, save it to the TFTP client. */
            if (code == NX_TFTP_CODE_ERROR)
            {

                buffer_ptr++;
                tftp_client_ptr -> nx_tftp_client_error_code =  ((UINT) (*buffer_ptr)) << 8;
                buffer_ptr++;
                tftp_client_ptr -> nx_tftp_client_error_code |= (UINT) (*buffer_ptr);
                buffer_ptr++;

                /* Loop to save error message to TFTP Client.  */
                tftp_client_ptr -> nx_tftp_client_error_string[sizeof(tftp_client_ptr -> nx_tftp_client_error_string) - 1] =  NX_NULL;
                for (i = 0; i < NX_TFTP_ERROR_STRING_MAX; i++)
                {

                    /* Store desired file name.  */
                    tftp_client_ptr -> nx_tftp_client_error_string[i] =  (CHAR) *buffer_ptr++;

                    /* Check for NULL character.  */
                    if (tftp_client_ptr -> nx_tftp_client_error_string[i] == NX_NULL)
                    {
                        break;
                    }
                }

                /* Release the packet.  */
                nx_packet_release(packet_ptr);

                /* Return error condition.  */
                return(NX_TFTP_CODE_ERROR);
            }
            /* Unknown code, not an error or an ACK. */
            else
            {

                /* Release the packet.  */
                nx_packet_release(packet_ptr);

                /* Return error condition.  */
                return(NX_TFTP_NO_ACK_RECEIVED);
            }
        }

        /* Release the packet.  */
        nx_packet_release(packet_ptr);
    }

    /* Otherwise, return success!  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_tftp_client_file_read                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TFTP file read call.         */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    packet_ptr                            Pointer to destination for    */ 
/*                                            return packet pointer       */ 
/*    wait_option                           Timeout for the read request  */ 
/*    ip_type                               IP type                       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_PARAMETERS                 Invalid non pointer input     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nxd_tftp_client_file_read             Actual client file read      */ 
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
UINT  _nxde_tftp_client_file_read(NX_TFTP_CLIENT *tftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option, UINT ip_type)
{

UINT    status;


    /* Check for invalid input pointers.  */        
    if ((tftp_client_ptr == NX_NULL) || (tftp_client_ptr -> nx_tftp_client_id != NXD_TFTP_CLIENT_ID) ||
        (packet_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for valid IP version*/
    if ((ip_type != NX_IP_VERSION_V4) && (ip_type != NX_IP_VERSION_V6))          
    {
        return NX_INVALID_PARAMETERS;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                            
    /* Call actual client file read function.  */
    status =  _nxd_tftp_client_file_read(tftp_client_ptr, packet_ptr, wait_option, ip_type);

    /* Return completion status.  */
    return(status);
}

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_tftp_client_file_read                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function reads a buffer from a previously opened TFTP file.    */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    packet_ptr                            Pointer to destination for    */ 
/*                                            return packet pointer       */ 
/*    wait_option                           Timeout for the read request  */  
/*    ip_type                               IP type                       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */
/*    NX_TFTP_CLIENT_NOT_OPEN               Requested file not open       */
/*    NX_TFTP_INVALID_SERVER_ADDRESS        Reply from unknown Server     */
/*    NX_TFTP_INVALID_IP_VERSION            Unsupported IP protocol       */
/*    NX_TFTP_END_OF_FILE                   No more data in file          */
/*    NX_TFTP_INVALID_BLOCK_NUMBER          Mismatching block number in   */
/*                                            received TFTP packet        */
/*    NX_TFTP_CODE_ERROR                    Unknown TFTP code received    */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet for ACK packet*/ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_udp_socket_receive                 Receive UDP data packet       */   
/*    nxd_udp_socket_send                   Send TFTP ACK packet          */ 
/*    nxd_udp_source_extract                Extract IP and port from msg  */ 
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
UINT  _nxd_tftp_client_file_read(NX_TFTP_CLIENT *tftp_client_ptr, NX_PACKET **packet_ptr, ULONG wait_option, UINT ip_type)
{

UCHAR       *buffer_ptr;
UINT        status;
NXD_ADDRESS ip_address;
UINT        port;
UINT        i;
USHORT      block_number;
UCHAR       code;
NX_PACKET   *ack_packet;
UINT         matching = NX_FALSE;
UCHAR       resend_ACK_packet = NX_FALSE;


    /* Determine if we are still in an open state.  */
    if (tftp_client_ptr -> nx_tftp_client_state != NX_TFTP_STATE_OPEN)
    {

        /* This instance is not open, return an error.  */
        if (tftp_client_ptr -> nx_tftp_client_state == NX_TFTP_STATE_END_OF_FILE)
            return(NX_TFTP_END_OF_FILE);
        else
            return(NX_TFTP_NOT_OPEN);
    }

    /* Read the next block of the file.  */
    status =  nx_udp_socket_receive(&(tftp_client_ptr -> nx_tftp_client_socket), packet_ptr, wait_option);

    /* Check the return status.  */
    if (status != NX_SUCCESS)
    {

        /* Check for the type of error.  */
        if (status != NX_NO_PACKET)
        {

            /* Serious error, enter error state.  */
            tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;
        }

        /* Return error condition.  */
        return(status);
    } 

    /* Check for valid packet length (The minimum TFTP header size is ACK packet, four bytes).  */
    if ((*packet_ptr) -> nx_packet_length < 4)
    {

        /* Release the packet. */
        nx_packet_release(*packet_ptr);

        /* Return.  */
        return(NX_INVALID_PACKET);
    }

    /* At this point, we have a block of the file from the server.  */

    /* Extract the source IP and port numbers.  */
    nxd_udp_source_extract(*packet_ptr, &ip_address, &port);

    /* Check for an invalid server IP address.  */

#ifndef NX_DISABLE_IPV4
    if (ip_type == NX_IP_VERSION_V4)
    {

        if (ip_address.nxd_ip_address.v4 == tftp_client_ptr -> nx_tftp_client_server_ip.nxd_ip_address.v4)
        {
            matching = NX_TRUE;
        }
    }
    else
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
    if (ip_type == NX_IP_VERSION_V6)
    {
        if (CHECK_IPV6_ADDRESSES_SAME(&ip_address.nxd_ip_address.v6[0], &tftp_client_ptr -> nx_tftp_client_server_ip.nxd_ip_address.v6[0]))
        {
    
            matching = NX_TRUE;
        }
    }
    else
#endif
    {

        /* Release the packet.  */
        nx_packet_release(*packet_ptr);

        return NX_TFTP_INVALID_IP_VERSION;
    }

    /* Do we have a match? */
    if (matching == NX_FALSE)
    {

        /* No, Invalid IP address!  */

        /* Enter error state.  */
        tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

        /* Release the packet.  */
        nx_packet_release(*packet_ptr);

        /* Set packet pointer to NULL.  */
        *packet_ptr =  NX_NULL;

        /* Return error condition.  */
        return(NX_TFTP_INVALID_SERVER_ADDRESS);
    }
    
    /* Save the source port since the server can change its port.  */
    tftp_client_ptr -> nx_tftp_client_server_port =  port;

    /* Setup a pointer to the buffer.  */
    buffer_ptr =  (*packet_ptr) -> nx_packet_prepend_ptr;

    /* Pickup the type of packet.  */
    buffer_ptr++;
    code =  *buffer_ptr;

    /* Is a data packet present?  */
    if (code == NX_TFTP_CODE_DATA)
    {

        /* Yes, a data packet is present.  */

        /* Pickup the block number.  */
        buffer_ptr++;
        block_number = (USHORT)((*buffer_ptr) << 8);
        buffer_ptr++;
        block_number = (USHORT)(block_number | (*buffer_ptr));

        /* Check if the server didn't receive last TFTP ACK. */
        if ((USHORT)(tftp_client_ptr -> nx_tftp_client_block_number - 1) == block_number)
        {

            /*  The Server will resend previous packet till it times out.
                set a flag to indicate we need to resend the ACK packet to 
                prevent this. */
            resend_ACK_packet = NX_TRUE;
        }
        /* Is there a block number match (greater than 1) ?  */
        else if (tftp_client_ptr -> nx_tftp_client_block_number != block_number)
        {
    
            /* Block numbers don't match.  Invalid packet or old data packet. Discard it. */

            /* Release the packet.  */
            nx_packet_release(*packet_ptr);

            /* Set packet pointer to NULL.  */
            *packet_ptr =  NX_NULL; 

            /* Return error condition.  */
            return(NX_TFTP_INVALID_BLOCK_NUMBER);
        }

        /* Valid block number, end ACK back to server.  */

        /* Allocate a new packet for the ACK message.  Determine whether we are sending
           IPv4 or IPv6 packets.   */

        if (ip_type == NX_IP_VERSION_V4)
        {

            status =  nx_packet_allocate(tftp_client_ptr -> nx_tftp_client_packet_pool_ptr, &ack_packet, NX_IPv4_UDP_PACKET, wait_option);
        }
        else
        {

            status =  nx_packet_allocate(tftp_client_ptr -> nx_tftp_client_packet_pool_ptr, &ack_packet, NX_IPv6_UDP_PACKET, wait_option);
        }

        /* Determine if an error occurred trying to allocate a packet.  */
        if (status != NX_SUCCESS)
        {

            /* Enter error state.  */
            tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

            /* Release the data packet.  */
            nx_packet_release(*packet_ptr);

            /* Set packet pointer to NULL.  */
            *packet_ptr =  NX_NULL;

            /* Return error condition.  */
            return(status);
        }

        if (4u > ((ULONG)(ack_packet -> nx_packet_data_end) - (ULONG)(ack_packet -> nx_packet_append_ptr)))
        {
            nx_packet_release(*packet_ptr); 

            nx_packet_release(ack_packet);
            return(NX_SIZE_ERROR);
        }

        ack_packet -> nx_packet_append_ptr = ack_packet -> nx_packet_prepend_ptr;

        /* Valid block number, we need to send an ACK back to server. So now 
           build the TFTP ACK message.  */

        /* Setup a pointer to the packet payload.  */
        buffer_ptr =  ack_packet -> nx_packet_prepend_ptr;

        /* Initial byte is always zero.  */
        *buffer_ptr++ =  0;

        /* Set the ACK code.  */
        *buffer_ptr++ =  NX_TFTP_CODE_ACK;

        /* Put the block number in.  */
        *buffer_ptr++ =  (UCHAR) (block_number >> 8);
        *buffer_ptr =    (UCHAR) (block_number & 0xFF);

        /* Adjust the ACK packet pointers and length.  */
        ack_packet -> nx_packet_length =  4;
        ack_packet -> nx_packet_append_ptr =  ack_packet -> nx_packet_append_ptr + 4;

        /* Send the ACK packet out.  */
        status = nxd_udp_socket_send(&(tftp_client_ptr -> nx_tftp_client_socket), ack_packet, 
                                     &tftp_client_ptr -> nx_tftp_client_server_ip, 
                                     tftp_client_ptr -> nx_tftp_client_server_port);

        /* Check error status. */
        if (status != NX_SUCCESS)
        {
            nx_packet_release(*packet_ptr); 

            nx_packet_release(ack_packet);
            return status;
        }

        /* If we received a duplicate data packet, release it here. */
        if (resend_ACK_packet == NX_TRUE)
        {

            /* ACK for previous packet sent, so we can release this duplicate packet. */
            nx_packet_release(*packet_ptr);

            /* Set packet pointer to NULL.  */
            *packet_ptr =  NX_NULL; 

            /* Do not handle as an error. Indicate the TFTP Client is still downloading a file. */
            return NX_TFTP_INVALID_BLOCK_NUMBER;
        }

        /* Adjust the packet to position past the TFTP header.  */
        (*packet_ptr) -> nx_packet_length =       (*packet_ptr) -> nx_packet_length - 4;
        (*packet_ptr) -> nx_packet_prepend_ptr =  (*packet_ptr) -> nx_packet_prepend_ptr + 4;

        /* Check for end of file condition. Anything less than 512 bytes signals an end of file.  */
        if ((*packet_ptr) -> nx_packet_length < NX_TFTP_FILE_TRANSFER_MAX)
        {

            /* End of file is present.  */
    
            /* Mark the state as end of file.  */
            tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_END_OF_FILE;

            
            /* Return EOF condition.  */
            return(NX_TFTP_END_OF_FILE);

        }
        else
        {

            /* More packets are still required.  Increment the block number.  */
            tftp_client_ptr -> nx_tftp_client_block_number++;
        }

        /* Return a successful status.  */
        return(NX_SUCCESS);
    }
    else if (code == NX_TFTP_CODE_ERROR)
    {

        /* Error code received.  */

        /* Change the state to error.  */
        tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

        /* Save the error code and message.  */
        buffer_ptr++;
        tftp_client_ptr -> nx_tftp_client_error_code =  (((UINT) (*buffer_ptr)) << 8);
        buffer_ptr++;
        tftp_client_ptr -> nx_tftp_client_error_code |=  (UINT) (*buffer_ptr);
        buffer_ptr++;

        /* Loop to save error message.  */
        for (i = 0; (i < (sizeof(tftp_client_ptr -> nx_tftp_client_error_string) -1)) && (*buffer_ptr); i++)
        {

            /* Store desired file name.  */
            tftp_client_ptr -> nx_tftp_client_error_string[i] =  (CHAR) *buffer_ptr++;
        }
        
        /* Set NULL terminator.  */
        tftp_client_ptr -> nx_tftp_client_error_string[i]  = NX_NULL;

        /* Release the packet.  */
        nx_packet_release(*packet_ptr);

        /* Return a failed status.  */
        return(NX_TFTP_CODE_ERROR);
    }
    /* This could be an ack from the previous file write. Just ignore it. */
    else if (code == NX_TFTP_CODE_ACK)
    {

        /* Release the data packet.  */
        nx_packet_release(*packet_ptr);

        /* Not an error. */
        return NX_SUCCESS;
    }
   else
    {

        /* Change the state to error.  */
        tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

        /* Release the packet.  */
        nx_packet_release(*packet_ptr);

        /* Return a failed status.  */
        return(NX_TFTP_FAILED);
    }
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */  
/*    _nxde_tftp_client_file_write                        PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TFTP file write call.        */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    packet_ptr                            Pointer to packet             */ 
/*    wait_option                           Timeout for the write request */ 
/*    ip_type                               IP type                       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_PARAMETERS                 Invalid non pointer input     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */
/*    _nxd_tftp_client_file_write           Actual client file write      */ 
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
UINT  _nxde_tftp_client_file_write(NX_TFTP_CLIENT *tftp_client_ptr, NX_PACKET *packet_ptr, ULONG wait_option, UINT ip_type)
{

UINT    status;


    /* Check for invalid input pointers.  */  
    if ((tftp_client_ptr == NX_NULL) || (tftp_client_ptr -> nx_tftp_client_id != NXD_TFTP_CLIENT_ID) ||
        (packet_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for valid IP version*/
    if ((ip_type != NX_IP_VERSION_V4) && (ip_type != NX_IP_VERSION_V6))
    {
        return NX_INVALID_PARAMETERS;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                                       
    /* Call actual client file write function.  */
    status =  _nxd_tftp_client_file_write(tftp_client_ptr, packet_ptr, wait_option, ip_type);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */    
/*    _nxd_tftp_client_file_write                         PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function writes a buffer to a previously opened TFTP file.     */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    tftp_client_ptr                       Pointer to TFTP client        */ 
/*    packet_ptr                            Pointer to packet             */ 
/*    wait_option                           Timeout for the write request */  
/*    ip_type                               IP type                       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_TFTP_INVALID_IP_VERSION            Unsupported IP protocol       */ 
/*    NX_TFTP_NO_ACK_RECEIVED               ACK not received from Server  */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_release                     Release packet                */ 
/*    nx_udp_socket_receive                 Receive UDP data packet       */  
/*    nxd_udp_socket_send                   Send TFTP ACK packet          */ 
/*    nxd_udp_source_extract                Extract IP and port           */ 
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
UINT  _nxd_tftp_client_file_write(NX_TFTP_CLIENT *tftp_client_ptr, NX_PACKET *packet_ptr, ULONG wait_option, UINT ip_type)
{

UCHAR       *buffer_ptr;
UINT        status;
ULONG       length;
NXD_ADDRESS ip_address;
UINT        port;
UINT        matching = NX_FALSE;


    /* Determine if we are still in an open state.  */
    if (tftp_client_ptr -> nx_tftp_client_state != NX_TFTP_STATE_WRITE_OPEN)
    {

        /* This instance is not open, return an error.  */
        return(NX_TFTP_NOT_OPEN);
    }

    /* Save the length of the TFTP write data.  */
    length =  packet_ptr -> nx_packet_length;

    /* Place the TFTP information in front of the data packet.  */
    packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr - 4;
    packet_ptr -> nx_packet_length =       packet_ptr -> nx_packet_length + 4;

    /* Setup a pointer to the buffer.  */
    buffer_ptr =  packet_ptr -> nx_packet_prepend_ptr;

    /* Set the TFTP data code.  */
    *buffer_ptr++ =  0;
    *buffer_ptr++ =  NX_TFTP_CODE_DATA;

    /* Set the block number.  */
    *buffer_ptr++ =  (UCHAR) (tftp_client_ptr -> nx_tftp_client_block_number >> 8);
    *buffer_ptr++ =  (UCHAR) (tftp_client_ptr -> nx_tftp_client_block_number & 0xFF);

    /* Send the data packet out.  */
    status =  nxd_udp_socket_send(&(tftp_client_ptr -> nx_tftp_client_socket), packet_ptr, 
                    &tftp_client_ptr -> nx_tftp_client_server_ip, tftp_client_ptr -> nx_tftp_client_server_port);

    /* Determine if an error occurred trying to send the packet.  */
    if (status)
    {

        /* Enter error state.  */
        tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

        /* Release the data packet.  */
        nx_packet_release(packet_ptr);

        /* Return error condition.  */
        return(status);
    }

    /* We now need to wait for the ACK for this write request.  */
    status =  nx_udp_socket_receive(&(tftp_client_ptr -> nx_tftp_client_socket), &packet_ptr, wait_option);

    /* Check the return status.  */
    if (status)
    {

        /* Return error code, the application may rebuild another packet and try again.  */

        /* Return error condition.  */
        return(NX_TFTP_TIMEOUT);
    }

    /* Check for valid packet length (The minimum TFTP header size is ACK packet, four bytes).  */
    if (packet_ptr -> nx_packet_length < 4)
    {

        /* Release the packet. */
        nx_packet_release(packet_ptr);

        /* Return.  */
        return(NX_INVALID_PACKET);
    }

    /* Extract the source IP and port numbers.  */
    nxd_udp_source_extract(packet_ptr, &ip_address, &port);

    /* Check for an invalid server IP address.  */

#ifndef NX_DISABLE_IPV4
    if (ip_type == NX_IP_VERSION_V4)
    {

        if (ip_address.nxd_ip_address.v4 == tftp_client_ptr -> nx_tftp_client_server_ip.nxd_ip_address.v4)
        {
            matching = NX_TRUE;
        }
    }
    else
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
    if (ip_type == NX_IP_VERSION_V6)
    {
        if (CHECK_IPV6_ADDRESSES_SAME(&ip_address.nxd_ip_address.v6[0], &tftp_client_ptr -> nx_tftp_client_server_ip.nxd_ip_address.v6[0]))
        {
    
            matching = NX_TRUE;
        }
    }
    else
#endif
    {
        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        return NX_TFTP_INVALID_IP_VERSION;
    }

    /* Do we have a match? */
    if (!matching)
    {

        /* No, Invalid IP address!  */

        /* Enter error state.  */
        tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

        /* Release the packet.  */
        nx_packet_release(packet_ptr);

        /* Return error condition.  */
        return(NX_TFTP_INVALID_SERVER_ADDRESS);
    }
    
    /* Setup a pointer to the buffer.  */
    buffer_ptr =  (packet_ptr) -> nx_packet_prepend_ptr;

    /* Check for valid ACK message.  */
    if ((buffer_ptr[0] != 0) || (buffer_ptr[1] != NX_TFTP_CODE_ACK) || 
        (buffer_ptr[2] != ((UCHAR) (tftp_client_ptr -> nx_tftp_client_block_number >> 8))) || 
        (buffer_ptr[3] != ((UCHAR) (tftp_client_ptr -> nx_tftp_client_block_number & 0xFF))) ||
        (packet_ptr -> nx_packet_length != 4))
    {

        UINT code;

        /* Enter error state.  */
        tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_ERROR;

        /* Get the error status from the server message. */
        buffer_ptr++;
        code =  *(buffer_ptr);

        /* Check if this is a error reported by the server. If so save it to 
           the TFTP Client. */
        if (code == NX_TFTP_CODE_ERROR)
        {
            UINT i;

            buffer_ptr++;
            tftp_client_ptr -> nx_tftp_client_error_code =  ((UINT) (*buffer_ptr)) << 8;
            buffer_ptr++;
            tftp_client_ptr -> nx_tftp_client_error_code |= (UINT) (*buffer_ptr);
            buffer_ptr++;

            /* Loop to save error message.  */
            for (i = 0; (i < (sizeof(tftp_client_ptr -> nx_tftp_client_error_string) -1)) && (*buffer_ptr); i++)
            {

                /* Store desired file name.  */
                tftp_client_ptr -> nx_tftp_client_error_string[i] =  (CHAR) *buffer_ptr++;
            }
        
            /* Set NULL terminator.  */
            tftp_client_ptr -> nx_tftp_client_error_string[i]  = NX_NULL;

            /* Release the packet.  */
            nx_packet_release(packet_ptr);
    
            /* Return error condition.  */
            return(NX_TFTP_CODE_ERROR);
        }
        /* Unknown code, not an error or an ACK. */
        else
        {

            /* Release the packet.  */
            nx_packet_release(packet_ptr);

            /* Return error condition.  */
            return(NX_TFTP_NO_ACK_RECEIVED);
        }
    }

    /* At this point, everything is okay.  */

    /* Release the ACK packet.  */
    nx_packet_release(packet_ptr);

    /* Check the length to see if we have the end of the file.  */
    if (length < NX_TFTP_FILE_TRANSFER_MAX)
    {

        /* Yes, this is the last packet.  */

        /* Enter into the finished state.  */
        tftp_client_ptr -> nx_tftp_client_state =  NX_TFTP_STATE_FINISHED;
    }
    else
    {

        /* More blocks to transfer still, increment the block number.  */
        tftp_client_ptr -> nx_tftp_client_block_number++;
    }

    /* Return success to the caller.  */
    return(NX_SUCCESS);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxde_tftp_client_packet_allocate                   PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function checks for errors in the TFTP packet allocate call.   */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    pool_ptr                              Pointer to packet pool        */ 
/*    packet_ptr                            Pointer to destination for    */ 
/*                                            pointer of newly allocated  */ 
/*                                            packet                      */ 
/*    wait_option                           Timeout for the request       */   
/*    ip_type                               IP type                       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*    NX_PTR_ERROR                          Invalid pointer input         */
/*    NX_INVALID_PARAMETERS                 Invalid non pointer input     */
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */  
/*    _nxd_tftp _client_file_write          Actual client file write      */ 
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
UINT  _nxde_tftp_client_packet_allocate(NX_PACKET_POOL *pool_ptr, NX_PACKET **packet_ptr, ULONG wait_option, UINT ip_type)
{

UINT    status;


    /* Check for invalid input pointers.  */
    if ((pool_ptr == NX_NULL) || (packet_ptr == NX_NULL))
        return(NX_PTR_ERROR);

    /* Check for valid IP version*/
    if ((ip_type != NX_IP_VERSION_V4) && (ip_type != NX_IP_VERSION_V6))
    {
        return NX_INVALID_PARAMETERS;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING
                                    
    /* Call actual client packet allocate function.  */
    status =  _nxd_tftp_client_packet_allocate(pool_ptr, packet_ptr, wait_option, ip_type);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _nxd_tftp_client_packet_allocate                    PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function allocates a packet with extra room for the TFTP       */ 
/*    control information.                                                */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */
/*    pool_ptr                              Pointer to packet pool        */ 
/*    packet_ptr                            Pointer to destination for    */ 
/*                                            pointer of newly allocated  */ 
/*                                            packet                      */ 
/*    wait_option                           Timeout for the request       */   
/*    ip_type                               IP type                       */
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    status                                Completion status             */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    nx_packet_allocate                    Allocate packet               */ 
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
UINT  _nxd_tftp_client_packet_allocate(NX_PACKET_POOL *pool_ptr, NX_PACKET **packet_ptr, ULONG wait_option, UINT ip_type)
{

UINT    status;


    /* Allocate a new packet.  Determine whether we are sending IPv4 or IPv6 packets.   */
    if (ip_type == NX_IP_VERSION_V4)
    {

        status =  nx_packet_allocate(pool_ptr, packet_ptr, NX_IPv4_UDP_PACKET, wait_option);
    }
    else
    {

        status =  nx_packet_allocate(pool_ptr, packet_ptr, NX_IPv6_UDP_PACKET, wait_option);
    }

    /* Determine if an error is present.  */
    if (status)
    {

        /* Return the error code from the allocate routine.  */
        return(status);
    }

    (*packet_ptr) -> nx_packet_append_ptr = (*packet_ptr) -> nx_packet_prepend_ptr;

    /* Successful packet allocation.  Adjust the prepend and append pointers forward to 
       accommodate the TFTP header.  */
    (*packet_ptr) -> nx_packet_prepend_ptr +=  4;
    (*packet_ptr) -> nx_packet_append_ptr  +=  4;

    /* Return success to the caller.  */
    return(NX_SUCCESS);
}

