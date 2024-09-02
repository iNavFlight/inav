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
/** NetX Secure Component                                                 */
/**                                                                       */
/**    Datagram Transport Layer Security (DTLS)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_dtls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_secure_dtls_server_delete                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors when deleting a DTLS server.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            DTLS server control block     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_server_delete         Actual function call          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nxe_secure_dtls_server_delete(NX_SECURE_DTLS_SERVER *server_ptr)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status = NX_SUCCESS;
NX_SECURE_DTLS_SERVER *temp_server;

    /* Check pointer */
    if(server_ptr == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    temp_server = _nx_secure_dtls_server_created_ptr;
    for (;;)
    {
        if (temp_server == NX_NULL)
        {

            /* No session created. */
            return(NX_PTR_ERROR);
        }

        if (temp_server == server_ptr)
        {

            /* Found the server to delete. */
            break;
        }

        temp_server = temp_server -> nx_dtls_server_created_next;

        if (temp_server == _nx_secure_dtls_server_created_ptr)
        {

            /* This server is not created. */
            return(NX_PTR_ERROR);
        }
    }

    /* Call actual function. */
    status = _nx_secure_dtls_server_delete(server_ptr);
    
    return(status);
#else
    NX_PARAMETER_NOT_USED(server_ptr);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

