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


/* Include necessary system files.  */

#include "nx_secure_tls.h"

#ifndef NX_SECURE_RNG_CHECK_COUNT
#define NX_SECURE_RNG_CHECK_COUNT   3
#endif /* NX_SECURE_RNG_CHECK_COUNT */

UINT _nx_secure_crypto_rng_self_test(VOID);

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_crypto_rng_self_test                     PORTABLE C      */
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function runs the simple random number generator test to make  */
/*    sure the numbers generated are unique.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
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
/*                                            resulting in version 6.1    */
/*  03-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_crypto_rng_self_test(VOID)
{
#ifdef NX_SECURE_POWER_ON_SELF_TEST_MODULE_INTEGRITY_CHECK
UINT results[NX_SECURE_RNG_CHECK_COUNT];
UINT i, j;

    for (i = 0; i < NX_SECURE_RNG_CHECK_COUNT; i++)
    {
        results[i] = (UINT)NX_RAND();

        /* Make sure the random number is unique. */
        for (j = 0; j < i; j++)
        {
            if (results[i] == results[j])
            {

                /* Found duplicated random number. */
                return(1);
            }
        }
    }

    /* All random numbers generated are unique. */
    return(0);
#else
    return(0);
#endif
}
