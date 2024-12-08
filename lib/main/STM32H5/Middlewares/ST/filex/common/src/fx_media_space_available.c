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
/**   Media                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_system.h"
#include "fx_media.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_media_space_available                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns the number of bytes available in the          */
/*    specified media.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Media control block pointer   */
/*    available_bytes_ptr                   Pointer to available bytes    */
/*                                            destination                 */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    return status                                                       */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*    _fx_media_extended_space_available    Actual space available service*/
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
UINT  _fx_media_space_available(FX_MEDIA *media_ptr, ULONG *available_bytes_ptr)
{

UINT    status;
ULONG64 available_bytes;

    /* Call actual media space available service.  */
    status = _fx_media_extended_space_available(media_ptr, &available_bytes);

    /* Check status.  */
    if (status == FX_SUCCESS)
    {

        /* Determine if more than 4GB available.  */
        if (available_bytes > 0x00000000FFFFFFFF)
        {

            /* Yes, we must have more than 4GB available... report the maximum we can fit
               in 32bits, which is 4GB.  */
            *available_bytes_ptr = 0xFFFFFFFF;
        }
        else
        {
            *available_bytes_ptr = (ULONG)(available_bytes);
        }
    }

    /* Return status to the caller.  */
    return(status);
}

