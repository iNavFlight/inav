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
/**   Unicode                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_unicode.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_unicode_length_get_extended                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the length of the supplied unicode name.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    unicode_name                          Pointer to unicode name       */
/*    buffer_length                         Buffer length for unicode name*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    length                                                              */
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
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
ULONG  _fx_unicode_length_get_extended(UCHAR *unicode_name, UINT buffer_length)
{

ULONG i;
ULONG length;


    i =  0;
    length =  0;
    while (i < buffer_length)
    {

        /* See if we are at the end of the unicode string...  This assumes that there is a NULL at the end of the string.  */
        if ((unicode_name[i] == 0) && (unicode_name[i + 1] == 0))
        {
            break;
        }

        /* Increment index.  */
        i =  i + 2;

        /* Increment the length.  */
        length++;
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_UNICODE_LENGTH_GET, unicode_name, length, 0, 0, FX_TRACE_FILE_EVENTS, 0, 0)

    /* Return length.  */
    return(length);
}

