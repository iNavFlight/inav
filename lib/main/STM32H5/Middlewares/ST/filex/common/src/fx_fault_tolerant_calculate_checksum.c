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
/**   Fault Tolerant                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define FX_SOURCE_CODE

#include "fx_api.h"
#include "fx_fault_tolerant.h"
#include "fx_utility.h"


#ifdef FX_ENABLE_FAULT_TOLERANT
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_fault_tolerant_calculate_checksum               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates checksum for consecutive data.  The size   */
/*    of the log file is required to be 4-byte aligned.  Therefore this   */
/*    checksum routine is able to perform 4-byte access.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    data                                  Pointer to data               */
/*    len                                   Data length                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Computed checksum value                                             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*     _fx_utility_32_unsigned_read         Read a UINT from memory       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _fx_fault_tolerant_enable                                           */
/*    _fx_fault_tolerant_cleanup_FAT_chain                                */
/*    _fx_fault_tolerant_reset_log_file                                   */
/*    _fx_fault_tolerant_set_FAT_chain                                    */
/*    _fx_fault_tolerant_transaction_end                                  */
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
USHORT  _fx_fault_tolerant_calculate_checksum(UCHAR *data, UINT len)
{
ULONG checksum = 0;
ULONG long_value;

    while (len >= 4)
    {

        /* Read first long value. */
        long_value = _fx_utility_32_unsigned_read(data);

        /* Calculate checksum. */
        checksum += (long_value >> 16) + (long_value & 0xFFFF);

        /* Decrease length. */
        len -= sizeof(ULONG);
        data += sizeof(ULONG);
    }

    /* Trim high 16 bits of checksum. */
    checksum = (checksum & 0xFFFF) + (checksum >> 16);
    checksum = (checksum & 0xFFFF) + (checksum >> 16);

    return((USHORT)((~checksum) & 0xFFFF));
}
#endif /* FX_ENABLE_FAULT_TOLERANT */

