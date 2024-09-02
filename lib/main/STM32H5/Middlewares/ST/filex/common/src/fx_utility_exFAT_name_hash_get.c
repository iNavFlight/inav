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
/*    _fx_utility_exFAT_name_hash_get                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns hash of ASCII file name.                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    name                                  ASCII file name               */
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
USHORT  _fx_utility_exFAT_name_hash_get(CHAR *name)
{

USHORT hash;


    /* Initialize hash to 0. */
    hash = 0;

    /* Is there a name?  */
    if (!name)
    {

        /* No, just return 0.  */
        return(0);
    }

    /* Create hash for name.  */
    while (*name)
    {

        /* Compute hash.  */
        hash = (USHORT)(((hash >> 1) | (hash << 15)) + _fx_utility_exFAT_upcase_get((USHORT)*name));
        hash = (USHORT)((hash >> 1) | (hash << 15));

        /* Move to next character of name.  */
        name++;
    }

    /* Return the hash.  */
    return(hash);
}

#endif /* FX_ENABLE_EXFAT */

