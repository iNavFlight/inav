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
/** NetX Crypto Component                                                 */
/**                                                                       */
/**   Crypto Initialization                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
#define _NX_CRYPTO_INITIALIZE_

#include "nx_crypto.h"


#ifdef NX_CRYPTO_SELF_TEST

/* Include necessary system files.  */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_self_test_memcpy                         PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs memory copy function for the FIPS 140-2      */
/*    compliance build.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dest                                  Pointer to the destination    */
/*                                            memory                      */
/*    value                                 value (in byte) to set to the */
/*                                            memory location             */
/*    size                                  Number of bytes to copy       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    void *                                Pointer to the destination    */
/*                                            memory                      */
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
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            renamed function,           */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP VOID *_nx_crypto_self_test_memcpy(void *dest, const void *src, size_t size)
{
    char *from, *to;
    unsigned int i;

    from = (char*)src;
    to = (char*)dest;

    for(i = 0; i < size; i++)
    {
        to[i] = from[i];
    }

    return dest;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_self_test_memset                         PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs memory set function for the FIPS 140-2       */
/*    compliance build.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dest                                  Pointer to the destination    */
/*                                            memory                      */
/*    value                                 value (in byte) to set to the */
/*                                            memory location             */
/*    size                                  Number of bytes to set        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    void *                                Pointer to the destination    */
/*                                            memory                      */
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
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            renamed function,           */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP VOID *_nx_crypto_self_test_memset(void *dest, int value, size_t size)
{
    char  *to;
    unsigned int i;
    char v;

    to = (char*)dest;
    v = (char)(value & 0xFF);
    for(i = 0; i < size; i++)
    {
        to[i] = v;
    }

    return dest;
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_self_test_memcmp                         PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs memory comparison function for the           */
/*    FIPS 140-2 compliance build.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    str1                                  Pointer to the first string   */
/*                                            for the comparison.         */
/*    str2                                  Pointer to the second string  */
/*                                            for the comparison.         */
/*    size                                  Number of bytes to compare.   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Result                                0:  str1 and str2 are the     */
/*                                              same.                     */
/*                                          0:  str1 and str2 are         */
/*                                              different.                */
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
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            renamed function,           */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP int _nx_crypto_self_test_memcmp(const void *str1, const void *str2, size_t size)
{
    char *string1;
    char *string2;
    unsigned int i;

    string1 = (char*)str1;
    string2 = (char*)str2;
    for(i = 0; i < size; i++)
    {
        if(*string1++ != *string2++)
            return(1);
    }

    return(0);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_self_test_memmove                        PORTABLE C      */
/*                                                           6.1.7        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function performs memory move function for the FIPS 140-2      */
/*    compliance build.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dest                                  Pointer to the destination    */
/*                                            memory                      */
/*    src                                   Pointer to the source memory  */
/*    size                                  Number of bytes to move       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    void *                                Pointer to the destination    */
/*                                            memory                      */
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
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            renamed function,           */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP void* _nx_crypto_self_test_memmove(void *dest, const void *src, size_t size)
{
    char *from, *to;
    unsigned int i;

    from = (char*)src;
    to = (char*)dest;

    if((ULONG)dest < (ULONG)src)
    {
        for(i = 0; i < size; i++)
        {
            to[i] = from[i];
        }
    }
    else if((ULONG)dest > (ULONG)src)
    {

        for(i = size; i != 0; i--)
            to[i - 1] = from[i - 1];
    }
    return(dest);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_module_state_get                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves the current crypto library state.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                The bitmap of the current     */
/*                                            status.  Valid bits are:    */
/*                        NX_CRYPTO_LIBRARY_STATE_UNINITIALIZED           */
/*                        NX_CRYPTO_LIBRARY_STATE_POST_IN_PROGRESS        */
/*                        NX_CRYPTO_LIBRARY_STATE_POST_FAILED             */
/*                        NX_CRYPTO_LIBRARY_STATE_OPERATIONAL             */
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
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_module_state_get(VOID)
{
    return(_nx_crypto_library_state);

}

#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_crypto_initialize                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the NetX Crypto module.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                                              */
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
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
NX_CRYPTO_KEEP UINT _nx_crypto_initialize(VOID)
{


    return(NX_CRYPTO_SUCCESS);

}
