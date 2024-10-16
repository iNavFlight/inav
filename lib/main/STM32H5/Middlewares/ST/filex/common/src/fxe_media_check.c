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


FX_CALLER_CHECKING_EXTERNS


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fxe_media_check                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the media check call.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to a previously       */
/*                                            opened media                */
/*    scratch_memory_ptr                    Pointer to memory area for    */
/*                                            check disk to use (as       */
/*                                            mentioned above)            */
/*    scratch_memory_size                   Size of the scratch memory    */
/*    error_correction_option               Specifies which - if any -    */
/*                                            errors are corrected by     */
/*                                            the check disk function.    */
/*                                            Setting the following bit   */
/*                                            causes that error to be     */
/*                                            corrected:                  */
/*                                                                        */
/*                                            0x01 -> Fix FAT Chain Errors*/
/*                                            0x02 -> Fix Directory Entry */
/*                                                      Errors            */
/*                                            0x04 -> Fix Lost Clusters   */
/*                                                                        */
/*    errors_detected                       Specifies the destination     */
/*                                            ULONG to place the error    */
/*                                            report from check disk.     */
/*                                            This has a similar bit map  */
/*                                            as before:                  */
/*                                                                        */
/*                                            0x01 -> FAT Chain Error(s)  */
/*                                            0x02 -> Directory Entry     */
/*                                                      Error(s)          */
/*                                            0x04 -> Lost Cluster(s)     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    FX_SUCCESS                            Check disk performed its      */
/*                                            operation successfully.     */
/*                                            This does not mean that     */
/*                                            there were no errors. The   */
/*                                            errors_detected variable    */
/*                                            needs to be examined.       */
/*    FX_MEDIA_NOT_OPEN                     The media was not open.       */
/*    FX_NOT_ENOUGH_MEMORY                  The scratch memory was not    */
/*                                            large enough or the nesting */
/*                                            depth was greater than the  */
/*                                            maximum specified.          */
/*    FX_IO_ERROR                           I/O Error reading/writing to  */
/*                                            the media.                  */
/*    FX_ERROR_NOT_FIXED                    Fundamental problem with      */
/*                                            media that couldn't be fixed*/
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_media_check                       Actual media check service    */
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
UINT  _fxe_media_check(FX_MEDIA *media_ptr, UCHAR *scratch_memory_ptr, ULONG scratch_memory_size, ULONG error_correction_option, ULONG *errors_detected)
{

UINT status;


    /* Check for a NULL media or scratch pointer.  */
    if ((media_ptr == FX_NULL) || (scratch_memory_ptr == FX_NULL))
    {
        return(FX_PTR_ERROR);
    }

    /* Check for a valid caller.  */
    FX_CALLER_CHECKING_CODE

    /* Call actual media check service.  */
    status =  _fx_media_check(media_ptr, scratch_memory_ptr, scratch_memory_size, error_correction_option, errors_detected);

    /* Return status to the caller.  */
    return(status);
}

