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
/** FileX Component                                                       */
/**                                                                       */
/**   Utility                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"


#ifdef FX_ENABLE_EXFAT
#include "fx_directory_exFAT.h"
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_exFAT_unicode_name_hash_get             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns hash of Unicode file name.                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    unicode_name                          Unicode file name             */
/*    unicode_length                        Unicode name length           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return name hash                                                    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_directory_exFAT_entry_write                                     */
/*    _fx_directory_rename                                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
USHORT  _fx_utility_exFAT_unicode_name_hash_get(CHAR *unicode_name, ULONG unicode_length)
{

USHORT hash;
USHORT upcased_char;

    /* Initialize hash to 0. */
    hash = 0;

    /* Is there a name?  */
    if (!unicode_name)
    {

        /* No, just return 0.  */
        return(0);
    }

    /* Create hash for name.  */
    while (unicode_length)
    {

        /* Get up-cased character.  */
        upcased_char = (USHORT)(_fx_utility_exFAT_upcase_get((USHORT)(*unicode_name | (*(unicode_name + 1) << 8))));

        /* Compute hash.  */
        hash = (USHORT)(((hash >> 1) | (hash << 15)) + (upcased_char & 0xFF));
        hash = (USHORT)(((hash >> 1) | (hash << 15)) + (upcased_char >> 8));

        /* Move to next character of the Unicode name.  */
        unicode_name = unicode_name + 2;

        /* Decrement length.  */
        unicode_length--;
    }

    /* Return the hash.  */
    return(hash);
}

#endif /* FX_ENABLE_EXFAT */

