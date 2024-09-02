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
#include "fx_system.h"
#include "fx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_utility_64_unsigned_read                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function reads (with endian awareness) a 64-bit unsigned data  */
/*    from the specified source and returns the value to the caller.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    source_ptr                            Source memory pointer         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    ULONG64 value                                                       */
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
ULONG64  _fx_utility_64_unsigned_read(UCHAR *source_ptr)
{

ULONG64 value;
ULONG   upper_portion;
ULONG   lower_portion;


    lower_portion = _fx_utility_32_unsigned_read(source_ptr);
    upper_portion = _fx_utility_32_unsigned_read(source_ptr + sizeof(ULONG));

    value =  ((ULONG64)upper_portion) << 32;
    value |= (ULONG64)lower_portion;

    /* Return 64-bit value to caller.  */
    return(value);
}

