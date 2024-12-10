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
#include "fx_utility.h"


#ifdef FX_ENABLE_EXFAT
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_absolute_path_get                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function converts concatenating of base and new path to        */
/*    absolute path.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    base_path                             Base (current) path           */
/*    new_path                              New path                      */
/*    absolute_path                         Absolute Path                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    absolute_path                         Resulting absolute path       */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_utility_token_length_get          Get the next token length     */
/*    _fx_utility_string_length_get         Get string length             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_directory_default_set                                           */
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
UINT  _fx_utility_absolute_path_get(CHAR *base_path, CHAR *new_path, CHAR *absolute_path)
{

UINT length;
UINT absolute_position;
UINT i;


    /* Check if we got null pointers.  */
    if (!base_path || !new_path || !absolute_path)
    {

        /* Return error.  */
        return(FX_INVALID_PATH);
    }

    /* Is the new path starts from root?  */
    if ((new_path[0] == '\\') || (new_path[0] == '/'))
    {

        /* Yes, set the absolute path to root.  */
        absolute_path[0] = '\\';
        absolute_path[1] = 0;
        new_path++;
    }
    else
    {

        /* Initialize the absolute path with the base path.  */
        i =  0;
        while ((base_path[i]) && (i < FX_MAXIMUM_PATH))
        {

            /* Copy a character.  */
            absolute_path[i] =  base_path[i];

            /* Move to next character.  */
            i++;
        }

        /* Is the absolute path still NULL?  */
        if ((absolute_path[0] == 0) || (i == 0))
        {

            /* Yes, set the absolute path to root.  */
            absolute_path[0] = '\\';
            absolute_path[1] = 0;
        }
    }

    /* Loop to add new path to the absolute path.  */
    while ((length = _fx_utility_token_length_get(new_path)) > 0)
    {

        /* Check if the path is too long.  */
        if ((new_path[length] != 0) && (new_path[length] != '\\') && (new_path[length] != '/'))
        {
            /* The path is too long, return error.  */
            return(FX_INVALID_PATH);
        }

        /* Check if we have a current directory mark.  */
        if ((length == 1) && (new_path[0] == '.'))
        {

            /* The same directory, just skip it.  */
        }
        /* Do we have a parent directory mark?  */
        else if ((length == 2) && (new_path[0] == '.') && (new_path[1] == '.'))
        {

            /* Yes, we need to move the absolute path to the parent directory.  */
            /* Check absolute path length.  */
            absolute_position = _fx_utility_string_length_get(absolute_path, FX_MAXIMUM_PATH);
            if ((absolute_position < 2) || (absolute_path[absolute_position] != 0))
            {

                /* No parent directory left in the absolute path, or invalid path length, return error.  */
                return(FX_INVALID_PATH);
            }

            /* Move to the last character of the absolute path.  */
            absolute_position--;

            /* Check if we have a directory separator.  */
            if ((absolute_path[absolute_position] == '\\') || (absolute_path[absolute_position] == '/'))
            {

                /* Remove the separator.  */
                absolute_path[absolute_position] = 0;
                absolute_position--;
            }

            /* Loop to clear the character backward until a seprator or string exhaust.  */
            while ((absolute_path[absolute_position] != '\\') && (absolute_path[absolute_position] != '/') && (absolute_position > 0))
            {

                /* Remove one character.  */
                absolute_path[absolute_position] = 0;
                absolute_position--;
            }

            /* Do we have somthing left in the path?  */
            if (absolute_position > 1)
            {

                /* Yes, remove the last separator.  */
                absolute_path[absolute_position] = 0;
            }
        }
        else
        {

            /* Normal directory, add it.  */
            absolute_position = _fx_utility_string_length_get(absolute_path, FX_MAXIMUM_PATH);

            /* Check if the path is too long.  */
            if (absolute_path[absolute_position] != 0)
            {

                /* The path is too long, return error.  */
                return(FX_INVALID_PATH);
            }

            /* Check if we have something in the path.  */
            if (absolute_position)
            {

                /* Do we have a separator in the last character?  */
                if ((absolute_path[absolute_position - 1] != '\\') && (absolute_path[absolute_position - 1] != '/'))
                {

                    /* No, add a directory separator to the path string.  */
                    absolute_path[absolute_position] = '\\';
                    absolute_position++;
                }
            }

            /* Loop to copy the new path to the absolute path.  */
            i = 0;
            while ((absolute_position < FX_MAXIMUM_PATH) && (i < length))
            {

                /* Copy one character.  */
                absolute_path[absolute_position++] = new_path[i++];
            }

            /* Check if we have not exceed the maximum length.  */
            if (absolute_position < FX_MAXIMUM_PATH)
            {

                /* Terminate the string.  */
                absolute_path[absolute_position] = 0;
            }
            else
            {

                /* The path is too long, return error.  */
                return(FX_INVALID_PATH);
            }
        }

        /* Move to the next component.  */
        new_path += length;

        /* Is the next component starts with a separator?  */
        if ((new_path[0] == '\\') || (new_path[0] == '/'))
        {

            /* Yes, skip it.  */
            new_path++;
        }
    }

    return(FX_SUCCESS);
}
#endif /* FX_ENABLE_EXFAT */

