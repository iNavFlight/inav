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
#include "fx_media.h"


/* Define external reference to OEM name string.  */

extern UCHAR   _fx_media_format_oem_name[8];
UINT  fx_media_format_oem_name_set(UCHAR new_oem_name[8]);

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _fx_media_format_oem_name_set                       PORTABLE C      */ 
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function modifies the default OEM name for all formatting. The */ 
/*    new OEM name must be 8 characters long - blank padded if necessary. */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    new_oem_name                          Pointer to new OEM name       */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
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
/*  03-02-2021     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/
UINT  fx_media_format_oem_name_set(UCHAR new_oem_name[8])
{

UINT    i;


    /* Simply copy the new OEM name into the default location.  */
    for (i = 0; i < 8; i++)
    {

        /* Copy one character of the new OEM name.  */
        _fx_media_format_oem_name[i] =  new_oem_name[i];
    }

    /* Return success.  */
    return(FX_SUCCESS);
}

