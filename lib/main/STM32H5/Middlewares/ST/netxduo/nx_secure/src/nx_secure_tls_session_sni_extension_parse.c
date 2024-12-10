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
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_session_sni_extension_parse          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses an incoming Hello extensions block, looking    */
/*    for a Server Name Indication (SNI) extension. The SNI extension     */
/*    (RFC 6066) currently contains only a single DNS Name entry, so only */
/*    a single DNS Name is returned. If new RFCs add other name types in  */
/*    the future, new API will be added.                                  */
/*                                                                        */
/*    This function is intended to be called in a TLS Server callback     */
/*    as part of processing Hello message extensions. The SNI extension   */
/*    is only sent by TLS Clients so if a Client receives an SNI          */
/*    extension it should be ignored.                                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    extensions                            Pointer to received extensions*/
/*    num_extensions                        The number of extensions      */
/*                                            received                    */
/*    dns_name                              Return the name requested by  */
/*                                            the client                  */
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
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_session_sni_extension_parse(NX_SECURE_TLS_SESSION *tls_session,
                                                NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                UINT num_extensions, NX_SECURE_X509_DNS_NAME *dns_name)
{
UINT         i;
const UCHAR *data_ptr;
UCHAR        name_type;
USHORT       list_length;
UINT         offset;

    NX_PARAMETER_NOT_USED(tls_session);

    /* Loop through the received extensions until we find SNI or hit the end. */
    for (i = 0; i < num_extensions; ++i)
    {
        if (extensions[i].nx_secure_tls_extension_id == NX_SECURE_TLS_EXTENSION_SERVER_NAME_INDICATION)
        {
            /* Extract the data from the extension. */
            /* Server Name Indication Extension structure:
             * |     2      |      2        |      1     |     2       |   <name length>   |
             * |  Ext Type  |  list length  |  name type | name length |  Host name string |
             */

            /* The generic extension parsing already handled the extension type, so this points to the
               list length field. */
            data_ptr = extensions[i].nx_secure_tls_extension_data;

            /* Extract the list length. */
            list_length = (USHORT)((data_ptr[0] << 8) + data_ptr[1]);
            offset = 2;

            /* Extract the name type. */
            name_type = data_ptr[offset];
            offset += 1;

            /* Extract the name length. */
            dns_name -> nx_secure_x509_dns_name_length = (USHORT)((data_ptr[offset] << 8) + data_ptr[offset + 1]);
            offset += 2;

            /* Check the name type and lengths. */
            if (name_type != NX_SECURE_TLS_SNI_NAME_TYPE_DNS ||
                list_length > extensions[i].nx_secure_tls_extension_data_length ||
                dns_name -> nx_secure_x509_dns_name_length > list_length)
            {
                return(NX_SECURE_TLS_SNI_EXTENSION_INVALID);
            }

            /* Make sure we don't copy over the end of the buffer. */
            if (dns_name -> nx_secure_x509_dns_name_length > NX_SECURE_X509_DNS_NAME_MAX)
            {
                dns_name -> nx_secure_x509_dns_name_length = NX_SECURE_X509_DNS_NAME_MAX;
            }

            /* Name and lengths check out, save off the name data. */
            NX_SECURE_MEMCPY(dns_name -> nx_secure_x509_dns_name, &data_ptr[offset], dns_name -> nx_secure_x509_dns_name_length); /* Use case of memcpy is verified. lgtm[cpp/banned-api-usage-required-any] */

            /* Success! */
            return(NX_SUCCESS);
        }
    }

    /* Searched through all the extensions, did not find SNI. */
    return(NX_SECURE_TLS_EXTENSION_NOT_FOUND);
}

