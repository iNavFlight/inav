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


extern TXM_MODULE_INSTANCE *_txm_asid_table[TXM_ASID_TABLE_LENGTH];
extern ULONG _txm_level2_module_page_table[TXM_MAXIMUM_MODULES][TXM_LEVEL_2_PAGE_TABLE_ENTRIES];
extern ULONG _txm_ttbr1_page_table[TXM_MAXIMUM_MODULES][TXM_MASTER_PAGE_TABLE_ENTRIES];


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_inside_data_check           Cortex-A7/MMU/GNU   */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function determines if pointer is within the module's data or  */
/*    shared memory.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    pointer                           Data pointer                      */
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
/*    TXM_MODULE_MANAGER_DATA_POINTER_CHECK                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx      Scott Larson            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
UINT _txm_module_manager_inside_data_check(ULONG pointer)
{

ULONG translation;

    /* ATS1CUR operation on address supplied in pointer, Stage 1 unprivileged read.  */
    __asm volatile ("MCR p15, 0, %0, c7, c8, 2" : : "r"(pointer) : );
    __asm volatile ("ISB");                                       /* Ensure completion of the MCR write to CP15.  */
    __asm volatile ("MRC p15, 0, %0, c7, c4, 0" : "=r"(translation) : : ); /* Read result from 32-bit PAR into translation.  */

    if (translation & TXM_ADDRESS_TRANSLATION_FAULT_BIT)
    {
        return(TX_FALSE);
    }

    return(TX_TRUE);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_assign_asid                 Cortex-A7/MMU/GNU   */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function assigns an Application Specific ID (ASID) to a        */
/*    module.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                   Pointer to module instance        */
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
/*    _txm_module_manager_mm_register_setup                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx      Scott Larson            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
UINT    _txm_module_manager_assign_asid(TXM_MODULE_INSTANCE *module_instance)
{
UINT    i = 1;

    /* Find first non-zero ASID, starting at index 1.  */
    while(i < TXM_ASID_TABLE_LENGTH)
    {
        if(_txm_asid_table[i] != 0)
        {
            i++;
        }
        else
        {
            module_instance -> txm_module_instance_asid = i;
            _txm_asid_table[i] = module_instance;
            return(TX_SUCCESS);
        }
    }

    return(TXM_MODULE_ASID_ERROR);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_remove_asid                 Cortex-A7/MMU/GNU   */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function removes a module from the ASID list.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                   Pointer to module instance        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    TXM_MODULE_MANAGER_MODULE_UNLOAD                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx      Scott Larson            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
VOID    _txm_module_manager_remove_asid(TXM_MODULE_INSTANCE *module_instance)
{
    if(module_instance -> txm_module_instance_asid)
    {
        _txm_asid_table[module_instance -> txm_module_instance_asid] = 0;
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_mm_register_setup           Cortex-A7/MMU/GNU   */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up the Cortex-A7 MMU register definitions based  */
/*    on the module's memory characteristics.                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                   Pointer to module instance        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _txm_module_manager_assign_asid                                     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    TXM_MODULE_MANAGER_MODULE_SETUP                                     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx      Scott Larson            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
VOID  _txm_module_manager_mm_register_setup(TXM_MODULE_INSTANCE *module_instance)
{
#ifdef TXM_MODULE_MEMORY_PROTECTION_ENABLED

ULONG   start_address;
ULONG   end_address;
ULONG   mmu_l1_entries;
ULONG   mmu_l2_entries = 0;
ULONG   level1_index;
ULONG   level2_index;
ULONG   temp_index;
ULONG   temp_address;
ULONG   l2_address;
ULONG   attributes = 0;
ULONG   asid;
UINT    i;


    /* Assign an ASID to this module.  */
    _txm_module_manager_assign_asid(module_instance);

    asid = module_instance -> txm_module_instance_asid;

    /* Copy master level 1 page table to module's page table. */
    for(i = 0; i < TXM_MASTER_PAGE_TABLE_ENTRIES; i++)
    {
        _txm_ttbr1_page_table[asid][i] = _txm_ttbr1_page_table[TXM_MASTER_PAGE_TABLE_INDEX][i];
    }

    /* Clear level 2 tables.  */
    for(i = 0; i < TXM_LEVEL_2_PAGE_TABLE_ENTRIES; i++)
    {
        _txm_level2_module_page_table[asid + TXM_MODULE_CODE_PAGE_TABLE_START_OFFSET][i] = 0;
        _txm_level2_module_page_table[asid + TXM_MODULE_CODE_PAGE_TABLE_END_OFFSET][i]   = 0;
        _txm_level2_module_page_table[asid + TXM_MODULE_DATA_PAGE_TABLE_START_OFFSET][i] = 0;
        _txm_level2_module_page_table[asid + TXM_MODULE_DATA_PAGE_TABLE_END_OFFSET][i]   = 0;
    }

    /* Get code start and end addresses.  */
    start_address = (ULONG)module_instance -> txm_module_instance_code_start;
    /* Extend end address to end of page (TXM_MODULE_MEMORY_ALIGNMENT-1). */
    end_address =  ((((ULONG)module_instance -> txm_module_instance_code_end) + TXM_MODULE_MEMORY_ALIGNMENT-1) & ~((ULONG)TXM_MODULE_MEMORY_ALIGNMENT-1)) - 1;

    /* How many level 1 table entries does code span?  */
    mmu_l1_entries = (end_address >> TXM_MMU_LEVEL1_PAGE_SHIFT) - (start_address >> TXM_MMU_LEVEL1_PAGE_SHIFT) + 1;

    /* Add 1 to align. */
    end_address++;

    /* How many level 2 table entries does code need?
     * 0: start and end addresses both aligned.
     * 1: either start or end address aligned.
     * 2: start and end addresses both not aligned. */
    if(start_address & ~TXM_MMU_LEVEL1_MASK)
    {
        /* If start address is not aligned, increment.  */
        mmu_l2_entries++;
    }
    if(end_address & ~TXM_MMU_LEVEL1_MASK)
    {
        /* If end address is not aligned, increment.  */
        mmu_l2_entries++;
    }

    /* Get index into L1 table.  */
    level1_index = (start_address >> TXM_MMU_LEVEL1_PAGE_SHIFT);

    /* Set up level 1 table.  */
    /* Do start and end entries need level 2 pages?  */
    if(mmu_l2_entries > 0)
    {
        /* If start_address is not aligned, we need a L2 page.  */
        if(start_address & ~TXM_MMU_LEVEL1_MASK)
        {
            /* Is there already a pointer to an L2 page in the L1 table? If bit 0 is set, there is.  */
            if(_txm_ttbr1_page_table[asid][level1_index] & 0x01)
            {
                /* Get L2 page address from L1 table.  */
                l2_address = _txm_ttbr1_page_table[asid][level1_index] & TXM_MMU_LEVEL1_SECOND_MASK;

                /* Copy the existing L2 page into the module L2 page.  */
                for(i = 0; i < TXM_LEVEL_2_PAGE_TABLE_ENTRIES; i++)
                {
                    _txm_level2_module_page_table[asid + TXM_MODULE_CODE_PAGE_TABLE_START_OFFSET][i] = ((ULONG *) l2_address)[i] | TXM_MMU_LEVEL2_USER_ATTRIBUTE_BASE;
                }
            }
            else
            {
                /* Translate attributes from L1 entry to an L2 entry.  */
                attributes = (((_txm_ttbr1_page_table[asid][level1_index] & TXM_MMU_LEVEL1_ATTRIBUTE_XN_MASK)   >> TXM_MMU_LEVEL1_ATTRIBUTE_XN_SHIFT)   << TXM_MMU_LEVEL2_ATTRIBUTE_XN_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][level1_index] & TXM_MMU_LEVEL1_ATTRIBUTE_B_MASK)    >> TXM_MMU_LEVEL1_ATTRIBUTE_B_SHIFT)    << TXM_MMU_LEVEL2_ATTRIBUTE_B_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][level1_index] & TXM_MMU_LEVEL1_ATTRIBUTE_C_MASK)    >> TXM_MMU_LEVEL1_ATTRIBUTE_C_SHIFT)    << TXM_MMU_LEVEL2_ATTRIBUTE_C_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][level1_index] & TXM_MMU_LEVEL1_ATTRIBUTE_AP_MASK)   >> TXM_MMU_LEVEL1_ATTRIBUTE_AP_SHIFT)   << TXM_MMU_LEVEL2_ATTRIBUTE_AP_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][level1_index] & TXM_MMU_LEVEL1_ATTRIBUTE_TEX_MASK)  >> TXM_MMU_LEVEL1_ATTRIBUTE_TEX_SHIFT)  << TXM_MMU_LEVEL2_ATTRIBUTE_TEX_SHIFT) |
                             TXM_MMU_LEVEL2_USER_ATTRIBUTE_BASE;

                /* Build L2 page with attributes inherited from L1 entry.  */
                for(i = 0; i < TXM_LEVEL_2_PAGE_TABLE_ENTRIES; i++)
                {
                    _txm_level2_module_page_table[asid + TXM_MODULE_CODE_PAGE_TABLE_START_OFFSET][i] = ((ULONG)start_address & TXM_MMU_LEVEL1_MASK) | (i << TXM_MMU_LEVEL2_PAGE_SHIFT) | attributes;
                }
            }

            /* Put L2 page in L1 table. */
            _txm_ttbr1_page_table[asid][level1_index] = ((ULONG)_txm_level2_module_page_table[asid + TXM_MODULE_CODE_PAGE_TABLE_START_OFFSET] & TXM_MMU_LEVEL1_SECOND_MASK) | TXM_MMU_LEVEL1_SECOND_ATTRIBUTES;

            /* Decrement number of L1 entries remaining.  */
            mmu_l1_entries--;

            /* Set up L2 start table.  */
            /* Determine how many entries in L2 table.  */
            if((end_address >> TXM_MMU_LEVEL1_PAGE_SHIFT) - (start_address >> TXM_MMU_LEVEL1_PAGE_SHIFT))
            {
                /* End address goes to next L1 page (or beyond).  */
                temp_address = ((start_address >> TXM_MMU_LEVEL1_PAGE_SHIFT) + 1) << (TXM_MMU_LEVEL1_PAGE_SHIFT);
                mmu_l2_entries = (temp_address - start_address) >> TXM_MMU_LEVEL2_PAGE_SHIFT;
            }
            else
            {
                /* End address is on the same L1 page.  */
                mmu_l2_entries = (end_address >> TXM_MMU_LEVEL2_PAGE_SHIFT) - (start_address >> TXM_MMU_LEVEL2_PAGE_SHIFT);
            }

            /* Insert module settings into start table.  */
            level2_index = ((start_address & ~TXM_MMU_LEVEL1_MASK) >> TXM_MMU_LEVEL2_PAGE_SHIFT);
            for(i = 0; i < mmu_l2_entries; i++, level2_index++)
            {
                _txm_level2_module_page_table[asid + TXM_MODULE_CODE_PAGE_TABLE_START_OFFSET][level2_index] = ((ULONG)start_address & TXM_MMU_LEVEL1_MASK) | (level2_index << TXM_MMU_LEVEL2_PAGE_SHIFT) | TXM_MMU_LEVEL2_CODE_ATTRIBUTES;
            }

            level1_index++;
        }

        /* Does last entry need a level 2 page?  */
        /* If end_address is not aligned, we need a L2 page.  */
        if((end_address & ~TXM_MMU_LEVEL1_MASK) && (mmu_l1_entries != 0))
        {
            /* Get index into L1 table.  */
            temp_index = (end_address >> TXM_MMU_LEVEL1_PAGE_SHIFT);

            /* Is there already a pointer to an L2 page in the L1 table? If bit 0 is set, there is.  */
            if(_txm_ttbr1_page_table[asid][temp_index] & 0x01)
            {
                /* Get L2 page address from L1 table.  */
                l2_address = _txm_ttbr1_page_table[asid][temp_index] & TXM_MMU_LEVEL1_SECOND_MASK;

                /* Copy the existing L2 page into the module L2 page.  */
                for(i = 0; i < TXM_LEVEL_2_PAGE_TABLE_ENTRIES; i++)
                {
                    _txm_level2_module_page_table[asid + TXM_MODULE_CODE_PAGE_TABLE_END_OFFSET][i] = ((ULONG *) l2_address)[i] | TXM_MMU_LEVEL2_USER_ATTRIBUTE_BASE;
                }
            }
            else
            {
                /* Translate attributes from L1 entry to an L2 entry.  */
                attributes = (((_txm_ttbr1_page_table[asid][temp_index] & TXM_MMU_LEVEL1_ATTRIBUTE_XN_MASK)     >> TXM_MMU_LEVEL1_ATTRIBUTE_XN_SHIFT)   << TXM_MMU_LEVEL2_ATTRIBUTE_XN_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][temp_index] & TXM_MMU_LEVEL1_ATTRIBUTE_B_MASK)      >> TXM_MMU_LEVEL1_ATTRIBUTE_B_SHIFT)    << TXM_MMU_LEVEL2_ATTRIBUTE_B_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][temp_index] & TXM_MMU_LEVEL1_ATTRIBUTE_C_MASK)      >> TXM_MMU_LEVEL1_ATTRIBUTE_C_SHIFT)    << TXM_MMU_LEVEL2_ATTRIBUTE_C_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][temp_index] & TXM_MMU_LEVEL1_ATTRIBUTE_AP_MASK)     >> TXM_MMU_LEVEL1_ATTRIBUTE_AP_SHIFT)   << TXM_MMU_LEVEL2_ATTRIBUTE_AP_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][temp_index] & TXM_MMU_LEVEL1_ATTRIBUTE_TEX_MASK)    >> TXM_MMU_LEVEL1_ATTRIBUTE_TEX_SHIFT)  << TXM_MMU_LEVEL2_ATTRIBUTE_TEX_SHIFT) |
                             TXM_MMU_LEVEL2_USER_ATTRIBUTE_BASE;

                /* Build L2 page with attributes inherited from L1 entry.  */
                for(i = 0; i < TXM_LEVEL_2_PAGE_TABLE_ENTRIES; i++)
                {
                    _txm_level2_module_page_table[asid + TXM_MODULE_CODE_PAGE_TABLE_END_OFFSET][i] = ((ULONG)start_address & TXM_MMU_LEVEL1_MASK) | (i << TXM_MMU_LEVEL2_PAGE_SHIFT) | attributes;
                }
            }

            /* Put L2 page in L1 table. */
            _txm_ttbr1_page_table[asid][temp_index] = ((ULONG)_txm_level2_module_page_table[asid + TXM_MODULE_CODE_PAGE_TABLE_END_OFFSET] & TXM_MMU_LEVEL1_SECOND_MASK) | TXM_MMU_LEVEL1_SECOND_ATTRIBUTES;

            /* Decrement number of L1 entries remaining.  */
            mmu_l1_entries--;

            /* Determine how many entries in L2 table.  */
            mmu_l2_entries = ((end_address & ~TXM_MMU_LEVEL1_MASK) >> TXM_MMU_LEVEL2_PAGE_SHIFT);

            /* Set up L2 end table.  */
            for(i = 0; i < mmu_l2_entries; i++)
            {
                _txm_level2_module_page_table[asid + TXM_MODULE_CODE_PAGE_TABLE_END_OFFSET][i] = ((ULONG)end_address & TXM_MMU_LEVEL1_MASK) | (i << TXM_MMU_LEVEL2_PAGE_SHIFT) | TXM_MMU_LEVEL2_CODE_ATTRIBUTES;
            }
        }
    }

    /* Fill any L1 entries between start and end pages of module code range.  */
    for(i = 0; i < mmu_l1_entries; i++, level1_index++)
    {
        /* Place address and attributes in table.  */
        _txm_ttbr1_page_table[asid][level1_index] = (level1_index << TXM_MMU_LEVEL1_PAGE_SHIFT) | TXM_MMU_LEVEL1_CODE_ATTRIBUTES;
    }

    /**************************************************************************/
    /* At this point, code protection is set up.  */
    /* Data protection is set up below.  */
    /**************************************************************************/

    /* Get data start and end addresses.  */
    start_address = (ULONG)module_instance -> txm_module_instance_data_start;
    end_address =   (ULONG)module_instance -> txm_module_instance_data_end;

    /* How many level 1 table entries does data span?  */
    mmu_l1_entries = (end_address >> TXM_MMU_LEVEL1_PAGE_SHIFT) - (start_address >> TXM_MMU_LEVEL1_PAGE_SHIFT) + 1;

    /* Add 1 to align. */
    end_address++;

    /* How many level 2 table entries does data need?
     * 0: start and end addresses both aligned.
     * 1: either start or end address aligned.
     * 2: start and end addresses both not aligned. */
    if(start_address & ~TXM_MMU_LEVEL1_MASK)
    {
        /* If start address is not aligned, increment.  */
        mmu_l2_entries++;
    }
    if(end_address & ~TXM_MMU_LEVEL1_MASK)
    {
        /* If end address is not aligned, increment.  */
        mmu_l2_entries++;
    }

    /* Get index into L1 table.  */
    level1_index = (start_address >> TXM_MMU_LEVEL1_PAGE_SHIFT);

    /* Set up level 1 table.  */
    /* Do start and end entries need level 2 pages?  */
    if(mmu_l2_entries > 0)
    {
        /* If start_address is not aligned, we need a L2 page.  */
        if(start_address & ~TXM_MMU_LEVEL1_MASK)
        {
            /* Is there already a pointer to an L2 page in the L1 table? If bit 0 is set, there is.  */
            if(_txm_ttbr1_page_table[asid][level1_index] & 0x01)
            {
                /* Get L2 page address from L1 table.  */
                l2_address = _txm_ttbr1_page_table[asid][level1_index] & TXM_MMU_LEVEL1_SECOND_MASK;

                /* Copy the existing L2 page into the module L2 page.  */
                for(i = 0; i < TXM_LEVEL_2_PAGE_TABLE_ENTRIES; i++)
                {
                    _txm_level2_module_page_table[asid + TXM_MODULE_DATA_PAGE_TABLE_START_OFFSET][i] = ((ULONG *) l2_address)[i] | TXM_MMU_LEVEL2_USER_ATTRIBUTE_BASE;
                }
            }
            else
            {
                /* Translate attributes from L1 entry to an L2 entry.  */
                attributes = (((_txm_ttbr1_page_table[asid][level1_index] & TXM_MMU_LEVEL1_ATTRIBUTE_XN_MASK)   >> TXM_MMU_LEVEL1_ATTRIBUTE_XN_SHIFT)   << TXM_MMU_LEVEL2_ATTRIBUTE_XN_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][level1_index] & TXM_MMU_LEVEL1_ATTRIBUTE_B_MASK)    >> TXM_MMU_LEVEL1_ATTRIBUTE_B_SHIFT)    << TXM_MMU_LEVEL2_ATTRIBUTE_B_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][level1_index] & TXM_MMU_LEVEL1_ATTRIBUTE_C_MASK)    >> TXM_MMU_LEVEL1_ATTRIBUTE_C_SHIFT)    << TXM_MMU_LEVEL2_ATTRIBUTE_C_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][level1_index] & TXM_MMU_LEVEL1_ATTRIBUTE_AP_MASK)   >> TXM_MMU_LEVEL1_ATTRIBUTE_AP_SHIFT)   << TXM_MMU_LEVEL2_ATTRIBUTE_AP_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][level1_index] & TXM_MMU_LEVEL1_ATTRIBUTE_TEX_MASK)  >> TXM_MMU_LEVEL1_ATTRIBUTE_TEX_SHIFT)  << TXM_MMU_LEVEL2_ATTRIBUTE_TEX_SHIFT) |
                             TXM_MMU_LEVEL2_USER_ATTRIBUTE_BASE;

                /* Build L2 page with attributes inherited from L1 entry.  */
                for(i = 0; i < TXM_LEVEL_2_PAGE_TABLE_ENTRIES; i++)
                {
                    _txm_level2_module_page_table[asid + TXM_MODULE_DATA_PAGE_TABLE_START_OFFSET][i] = ((ULONG)start_address & TXM_MMU_LEVEL1_MASK) | (i << TXM_MMU_LEVEL2_PAGE_SHIFT) | attributes;
                }
            }

            /* Put L2 page in L1 table. */
            _txm_ttbr1_page_table[asid][level1_index] = ((ULONG)_txm_level2_module_page_table[asid + TXM_MODULE_DATA_PAGE_TABLE_START_OFFSET] & TXM_MMU_LEVEL1_SECOND_MASK) | TXM_MMU_LEVEL1_SECOND_ATTRIBUTES;

            /* Decrement number of L1 entries remaining.  */
            mmu_l1_entries--;

            /* Set up L2 start table.  */
            /* Determine how many entries in L2 table.  */
            if((end_address >> TXM_MMU_LEVEL1_PAGE_SHIFT) - (start_address >> TXM_MMU_LEVEL1_PAGE_SHIFT))
            {
                /* End address goes to next L1 page (or beyond).  */
                temp_address = ((start_address >> TXM_MMU_LEVEL1_PAGE_SHIFT) + 1) << (TXM_MMU_LEVEL1_PAGE_SHIFT);
                mmu_l2_entries = (temp_address - start_address) >> TXM_MMU_LEVEL2_PAGE_SHIFT;
            }
            else
            {
                /* End address is on the same L1 page.  */
                mmu_l2_entries = (end_address >> TXM_MMU_LEVEL2_PAGE_SHIFT) - (start_address >> TXM_MMU_LEVEL2_PAGE_SHIFT);
            }

            /* Insert module settings into start table.  */
            level2_index = ((start_address & ~TXM_MMU_LEVEL1_MASK) >> TXM_MMU_LEVEL2_PAGE_SHIFT);
            for(i = 0; i < mmu_l2_entries; i++, level2_index++)
            {
                _txm_level2_module_page_table[asid + TXM_MODULE_DATA_PAGE_TABLE_START_OFFSET][level2_index] = ((ULONG)start_address & TXM_MMU_LEVEL1_MASK) | (level2_index << TXM_MMU_LEVEL2_PAGE_SHIFT) | TXM_MMU_LEVEL2_DATA_ATTRIBUTES;
            }

            level1_index++;
        }

        /* Does last entry need a level 2 page?  */
        /* If end_address is not aligned, we need a L2 page.  */
        if((end_address & ~TXM_MMU_LEVEL1_MASK) && (mmu_l1_entries != 0))
        {
            /* Get index into L1 table.  */
            temp_index = (end_address >> TXM_MMU_LEVEL1_PAGE_SHIFT);

            /* Is there already a pointer to an L2 page in the L1 table? If bit 0 is set, there is.  */
            if(_txm_ttbr1_page_table[asid][temp_index] & 0x01)
            {
                /* Get L2 page address from L1 table.  */
                l2_address = _txm_ttbr1_page_table[asid][temp_index] & TXM_MMU_LEVEL1_SECOND_MASK;

                /* Copy the existing L2 page into the module L2 page.  */
                for(i = 0; i < TXM_LEVEL_2_PAGE_TABLE_ENTRIES; i++)
                {
                    _txm_level2_module_page_table[asid + TXM_MODULE_DATA_PAGE_TABLE_END_OFFSET][i] = ((ULONG *) l2_address)[i] | TXM_MMU_LEVEL2_USER_ATTRIBUTE_BASE;
                }
            }
            else
            {
                /* Translate attributes from L1 entry to an L2 entry.  */
                attributes = (((_txm_ttbr1_page_table[asid][temp_index] & TXM_MMU_LEVEL1_ATTRIBUTE_XN_MASK)     >> TXM_MMU_LEVEL1_ATTRIBUTE_XN_SHIFT)   << TXM_MMU_LEVEL2_ATTRIBUTE_XN_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][temp_index] & TXM_MMU_LEVEL1_ATTRIBUTE_B_MASK)      >> TXM_MMU_LEVEL1_ATTRIBUTE_B_SHIFT)    << TXM_MMU_LEVEL2_ATTRIBUTE_B_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][temp_index] & TXM_MMU_LEVEL1_ATTRIBUTE_C_MASK)      >> TXM_MMU_LEVEL1_ATTRIBUTE_C_SHIFT)    << TXM_MMU_LEVEL2_ATTRIBUTE_C_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][temp_index] & TXM_MMU_LEVEL1_ATTRIBUTE_AP_MASK)     >> TXM_MMU_LEVEL1_ATTRIBUTE_AP_SHIFT)   << TXM_MMU_LEVEL2_ATTRIBUTE_AP_SHIFT) |
                             (((_txm_ttbr1_page_table[asid][temp_index] & TXM_MMU_LEVEL1_ATTRIBUTE_TEX_MASK)    >> TXM_MMU_LEVEL1_ATTRIBUTE_TEX_SHIFT)  << TXM_MMU_LEVEL2_ATTRIBUTE_TEX_SHIFT) |
                             TXM_MMU_LEVEL2_USER_ATTRIBUTE_BASE;

                /* Build L2 page with attributes inherited from L1 entry.  */
                for(i = 0; i < TXM_LEVEL_2_PAGE_TABLE_ENTRIES; i++)
                {
                    _txm_level2_module_page_table[asid + TXM_MODULE_DATA_PAGE_TABLE_END_OFFSET][i] = ((ULONG)start_address & TXM_MMU_LEVEL1_MASK) | (i << TXM_MMU_LEVEL2_PAGE_SHIFT) | attributes;
                }
            }

            /* Put L2 page in L1 table. */
            _txm_ttbr1_page_table[asid][temp_index] = ((ULONG)_txm_level2_module_page_table[asid + TXM_MODULE_DATA_PAGE_TABLE_END_OFFSET] & TXM_MMU_LEVEL1_SECOND_MASK) | TXM_MMU_LEVEL1_SECOND_ATTRIBUTES;

            /* Decrement number of L1 entries remaining.  */
            mmu_l1_entries--;

            /* Determine how many entries in L2 table.  */
            mmu_l2_entries = ((end_address & ~TXM_MMU_LEVEL1_MASK) >> TXM_MMU_LEVEL2_PAGE_SHIFT);

            /* Set up L2 end table.  */
            for(i = 0; i < mmu_l2_entries; i++)
            {
                _txm_level2_module_page_table[asid + TXM_MODULE_DATA_PAGE_TABLE_END_OFFSET][i] = ((ULONG)end_address & TXM_MMU_LEVEL1_MASK) | (i << TXM_MMU_LEVEL2_PAGE_SHIFT) | TXM_MMU_LEVEL2_DATA_ATTRIBUTES;
            }
        }
    }

    /* Fill any L1 entries between start and end pages of module data range.  */
    for(i = 0; i < mmu_l1_entries; i++, level1_index++)
    {
        /* Place address and attributes in table.  */
        _txm_ttbr1_page_table[asid][level1_index] = (level1_index << TXM_MMU_LEVEL1_PAGE_SHIFT) | TXM_MMU_LEVEL1_DATA_ATTRIBUTES;
    }

#endif
}
