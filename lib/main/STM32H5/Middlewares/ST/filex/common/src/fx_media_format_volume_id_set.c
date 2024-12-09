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


/* Define external reference to media volume ID.  */

extern ULONG   _fx_media_format_volume_id;
UINT  fx_media_format_volume_id_set(ULONG new_volume_id);

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _fx_media_format_media_volume_id_set                PORTABLE C      */ 
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function modifies the default media volume ID for all          */ 
/*    formatting.                                                         */ 
/*                                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    new_volume_id                         New media volume Id value     */ 
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
UINT  fx_media_format_volume_id_set(ULONG new_volume_id)
{

    /* Simply copy the new media volume ID into the default location.  */
    _fx_media_format_volume_id =  new_volume_id;

    /* Return success.  */
    return(FX_SUCCESS);
}
