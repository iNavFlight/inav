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
/**   Directory                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_system.h"
#include "fx_directory.h"
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_directory_name_extract                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function extracts the file name from the supplied input        */
/*    string.  If there is nothing left after the extracted name, a NULL  */
/*    is returned to the caller.  Otherwise, if something is left, a      */
/*    pointer to it is returned.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    source_ptr                           Source string pointer          */
/*    dest_ptr                             Destination string pointer     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Pointer to Next Name                 (if multiple directories)      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    FileX System Functions                                              */
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
CHAR  *_fx_directory_name_extract(CHAR *source_ptr, CHAR *dest_ptr)
{

UINT i;


    /* Set the destination string to NULL.  */
    dest_ptr[0] = 0;

    /* Is a backslash present?  */
    if ((*source_ptr == '\\') || (*source_ptr == '/'))
    {

        /* Advance the string pointer.  */
        source_ptr++;
    }

    /* Loop to remove any leading spaces.  */
    while (*source_ptr == ' ')
    {

        /* Position past leading space.  */
        source_ptr++;
    }

    /* Loop to extract the name.  */
    i = 0;
    while (*source_ptr)
    {

        /* If another backslash is present, break the loop.  */
        if ((*source_ptr == '\\') || (*source_ptr == '/'))
        {
            break;
        }

        /* Long name can be at most 255 characters, but are further limited by the
           FX_MAX_LONG_NAME_LEN define.  */
        if (i == FX_MAX_LONG_NAME_LEN - 1)
        {
            break;
        }

        /* Store the character.  */
        dest_ptr[i] =  *source_ptr++;

        /* Increment the character counter.  */
        i++;
    }

    /* NULL-terminate the string.  */
    dest_ptr[i] =  0;

    /* Determine if we can backup to the previous character.  */
    if (i)
    {

        /* Yes, we can move backwards.  */
        i--;
    }

    /* Get rid of trailing blanks in the destination string.  */
    while (dest_ptr[i] == ' ')
    {

        /* Set this entry to NULL.  */
        dest_ptr[i] =  0;

        /* Backup to the next character. Since leading spaces have been removed,
           we know that the index is always greater than 1.  */
        i--;
    }

    /* Determine if the source string is now at the end.  */
    if (*source_ptr == 0)
    {

        /* Yes, return a NULL pointer.  */
        source_ptr = FX_NULL;
    }

    /* Return the last pointer position in the source.  */
    return(source_ptr);
}

