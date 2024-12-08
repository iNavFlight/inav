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
/** ThreadX Component                                                     */
/**                                                                       */
/**   Module Manager                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE

#include "tx_api.h"
#include "txm_module.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_power_of_two_block_size                        Cortex-M4       */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates a power of two size at or immediately above*/
/*    the input size and returns it to the caller.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    size                              Block size                        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    calculated size                   Rounded up to power of two        */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _txm_module_manager_alignment_adjust  Adjust alignment for Cortex-M */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-15-2021      Scott Larson            Initial Version 6.1.9         */
/*                                                                        */
/**************************************************************************/
ULONG  _txm_power_of_two_block_size(ULONG size)
{
    /* Check for 0 size. */
    if(size == 0)
        return 0;
    
    /* Minimum MPU block size is 32. */
    if(size <= 32)
        return 32;
    
    /* Bit twiddling trick to round to next high power of 2
       (if original size is power of 2, it will return original size. Perfect!) */
    size--;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    size++;
    
    /* Return a power of 2 size at or above the input size.  */
    return(size);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_alignment_adjust                Cortex-M4       */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adjusts the alignment and size of the code and data   */
/*    section for a given module implementation.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_preamble                   Pointer to module preamble        */
/*    code_size                         Size of the code area (updated)   */
/*    code_alignment                    Code area alignment (updated)     */
/*    data_size                         Size of data area (updated)       */
/*    data_alignment                    Data area alignment (updated)     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _txm_power_of_two_block_size      Calculate power of two size       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Initial thread stack frame                                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-15-2021      Scott Larson            Initial Version 6.1.9         */
/*                                                                        */
/**************************************************************************/
VOID  _txm_module_manager_alignment_adjust(TXM_MODULE_PREAMBLE *module_preamble,
                                           ULONG *code_size,
                                           ULONG *code_alignment,
                                           ULONG *data_size,
                                           ULONG *data_alignment)
{
#ifdef TXM_MODULE_MANAGER_16_MPU
ULONG   local_code_size;
ULONG   local_code_alignment;
ULONG   local_data_size;
ULONG   local_data_alignment;
ULONG   code_size_accum;
ULONG   data_size_accum;

    /* Copy the input parameters into local variables for ease of use.  */
    local_code_size =       *code_size;
    local_code_alignment =  *code_alignment;
    local_data_size =       *data_size;
    local_data_alignment =  *data_alignment;

    /* Determine code block sizes. Minimize the alignment requirement.
       There are 4 MPU code entries available. The following is how the code size
       will be distributed:
       1. 1/4 of the largest power of two that is greater than or equal to code size.
       2. 1/4 of the largest power of two that is greater than or equal to code size.
       3. Largest power of 2 that fits in the remaining space.
       4. Smallest power of 2 that exceeds the remaining space, minimum 32.  */
    local_code_alignment =  _txm_power_of_two_block_size(local_code_size) >> 2;
    code_size_accum =  local_code_alignment + local_code_alignment;
    code_size_accum =  code_size_accum + (_txm_power_of_two_block_size(local_code_size - code_size_accum) >> 1);
    code_size_accum =  code_size_accum + _txm_power_of_two_block_size(local_code_size - code_size_accum);
    local_code_size =  code_size_accum;
    
    /* Determine data block sizes. Minimize the alignment requirement.
       There are 4 MPU data entries available. The following is how the data size
       will be distributed:
       1. 1/4 of the largest power of two that is greater than or equal to data size.
       2. 1/4 of the largest power of two that is greater than or equal to data size.
       3. Largest power of 2 that fits in the remaining space.
       4. Smallest power of 2 that exceeds the remaining space, minimum 32.  */
    local_data_alignment =  _txm_power_of_two_block_size(local_data_size) >> 2;
    data_size_accum =  local_data_alignment + local_data_alignment;
    data_size_accum =  data_size_accum + (_txm_power_of_two_block_size(local_data_size - data_size_accum) >> 1);
    data_size_accum =  data_size_accum + _txm_power_of_two_block_size(local_data_size - data_size_accum);
    local_data_size =  data_size_accum;
    
    /* Return all the information to the caller.  */
    *code_size =        local_code_size;
    *code_alignment =   local_code_alignment;
    *data_size =        local_data_size;
    *data_alignment =   local_data_alignment;

#else

ULONG   local_code_size;
ULONG   local_code_alignment;
ULONG   local_data_size;
ULONG   local_data_alignment;
ULONG   code_block_size;
ULONG   data_block_size;
ULONG   code_size_accum;
ULONG   data_size_accum;

    /* Copy the input parameters into local variables for ease of use.  */
    local_code_size =       *code_size;
    local_code_alignment =  *code_alignment;
    local_data_size =       *data_size;
    local_data_alignment =  *data_alignment;


    /* Test for external memory enabled in preamble.  */
    if(module_preamble -> txm_module_preamble_property_flags & TXM_MODULE_SHARED_EXTERNAL_MEMORY_ACCESS)
    {
        /* External/shared memory enabled. TXM_MODULE_MANAGER_CODE_MPU_ENTRIES-1 code entries will be used.  */
        if (local_code_size <= (32*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 32 is best.   */
            code_block_size =  32;
        }
        else if (local_code_size <= (64*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 64 is best.   */
            code_block_size =  64;
        }
        else if (local_code_size <= (128*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 128 is best.   */
            code_block_size =  128;
        }
        else if (local_code_size <= (256*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 256 is best.   */
            code_block_size =  256;
        }
        else if (local_code_size <= (512*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 512 is best.   */
            code_block_size =  512;
        }
        else if (local_code_size <= (1024*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 1024 is best.   */
            code_block_size =  1024;
        }
        else if (local_code_size <= (2048*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 2048 is best.   */
            code_block_size =  2048;
        }
        else if (local_code_size <= (4096*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 4096 is best.   */
            code_block_size =  4096;
        }
        else if (local_code_size <= (8192*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 8192 is best.   */
            code_block_size =  8192;
        }
        else if (local_code_size <= (16384*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 16384 is best.   */
            code_block_size =  16384;
        }
        else if (local_code_size <= (32768*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 32768 is best.   */
            code_block_size =  32768;
        }
        else if (local_code_size <= (65536*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 65536 is best.   */
            code_block_size =  65536;
        }
        else if (local_code_size <= (131072*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 131072 is best.   */
            code_block_size =  131072;
        }
        else if (local_code_size <= (262144*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 262144 is best.   */
            code_block_size =  262144;
        }
        else if (local_code_size <= (524288*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 524288 is best.   */
            code_block_size =  524288;
        }
        else if (local_code_size <= (1048576*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 1048576 is best.   */
            code_block_size =  1048576;
        }
        else if (local_code_size <= (2097152*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 2097152 is best.   */
            code_block_size =  2097152;
        }
        else if (local_code_size <= (4194304*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1)))
        {
            /* Block size of 4194304 is best.   */
            code_block_size =  4194304;
        }
        else
        {
            /* Just set block size to 32MB just to create an allocation error!  */
            code_block_size =  33554432;
        }
        
        /* Calculate the new code size.  */
        local_code_size =  code_block_size*(TXM_MODULE_MANAGER_CODE_MPU_ENTRIES - 1);
        
        /* Determine if the code block size is greater than the current alignment. If so, use block size
           as the alignment.  */
        if (code_block_size > local_code_alignment)
            local_code_alignment = code_block_size;
        
    }
    else
    {
        /* Determine code block sizes. Minimize the alignment requirement.
           There are 4 MPU code entries available. The following is how the code size
           will be distributed:
           1. 1/4 of the largest power of two that is greater than or equal to code size.
           2. 1/4 of the largest power of two that is greater than or equal to code size.
           3. Largest power of 2 that fits in the remaining space.
           4. Smallest power of 2 that exceeds the remaining space, minimum 32.  */
        local_code_alignment =  _txm_power_of_two_block_size(local_code_size) >> 2;
        code_size_accum =  local_code_alignment + local_code_alignment;
        code_size_accum =  code_size_accum + (_txm_power_of_two_block_size(local_code_size - code_size_accum) >> 1);
        code_size_accum =  code_size_accum + _txm_power_of_two_block_size(local_code_size - code_size_accum);
        local_code_size =  code_size_accum;
    }
    
    /* Determine the best data block size, which in our case is the minimal alignment.  */
    if (local_data_size <= (32*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 32 is best.   */
        data_block_size =  32;
    }
    else if (local_data_size <= (64*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 64 is best.   */
        data_block_size =  64;
    }
    else if (local_data_size <= (128*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 128 is best.   */
        data_block_size =  128;
    }
    else if (local_data_size <= (256*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 256 is best.   */
        data_block_size =  256;
    }
    else if (local_data_size <= (512*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 512 is best.   */
        data_block_size =  512;
    }
    else if (local_data_size <= (1024*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 1024 is best.   */
        data_block_size =  1024;
    }
    else if (local_data_size <= (2048*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 2048 is best.   */
        data_block_size =  2048;
    }
    else if (local_data_size <= (4096*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 4096 is best.   */
        data_block_size =  4096;
    }
    else if (local_data_size <= (8192*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 8192 is best.   */
        data_block_size =  8192;
    }
    else if (local_data_size <= (16384*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 16384 is best.   */
        data_block_size =  16384;
    }
    else if (local_data_size <= (32768*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 32768 is best.   */
        data_block_size =  32768;
    }
    else if (local_data_size <= (65536*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 65536 is best.   */
        data_block_size =  65536;
    }
    else if (local_data_size <= (131072*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 131072 is best.   */
        data_block_size =  131072;
    }
    else if (local_data_size <= (262144*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 262144 is best.   */
        data_block_size =  262144;
    }
    else if (local_data_size <= (524288*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 524288 is best.   */
        data_block_size =  524288;
    }
    else if (local_data_size <= (1048576*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 1048576 is best.   */
        data_block_size =  1048576;
    }
    else if (local_data_size <= (2097152*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 2097152 is best.   */
        data_block_size =  2097152;
    }
    else if (local_data_size <= (4194304*TXM_MODULE_MANAGER_DATA_MPU_ENTRIES))
    {
        /* Block size of 4194304 is best.   */
        data_block_size =  4194304;
    }
    else
    {
        /* Just set data block size to 32MB just to create an allocation error!  */
        data_block_size =  33554432;
    }

    /* Calculate the new data size.  */
    data_size_accum = data_block_size;
    while(data_size_accum < local_data_size)
    {
        data_size_accum += data_block_size;
    }
    local_data_size = data_size_accum;
    
    /* Determine if the data block size is greater than the current alignment. If so, use block size
       as the alignment.  */
    if (data_block_size > local_data_alignment)
    {
        local_data_alignment =  data_block_size;
    }

    /* Return all the information to the caller.  */
    *code_size =        local_code_size;
    *code_alignment =   local_code_alignment;
    *data_size =        local_data_size;
    *data_alignment =   local_data_alignment;

#endif
}
