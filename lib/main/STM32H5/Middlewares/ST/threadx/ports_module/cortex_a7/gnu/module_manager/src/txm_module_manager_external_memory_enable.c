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
#include "tx_mutex.h"
#include "tx_queue.h"
#include "tx_thread.h"
#include "txm_module.h"

/* External page tables.  */
extern ULONG _txm_level2_external_page_pool[TXM_LEVEL2_EXTERNAL_POOL_PAGES][TXM_LEVEL_2_PAGE_TABLE_ENTRIES];
extern ULONG _txm_ttbr1_page_table[TXM_MAXIMUM_MODULES][TXM_MASTER_PAGE_TABLE_ENTRIES];


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_level2_page_get                            Cortex-A7/MMU/GNU   */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets an available L2 page table and places it in the  */
/*    module external page table list.                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                       Module instance pointer       */
/*    page_addr                             Address of L2 page            */
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
/*    _txm_module_manager_external_memory_enable                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx      Scott Larson            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
ULONG _txm_level2_page_get(TXM_MODULE_INSTANCE *module_instance, ULONG *page_addr)
{

UINT    i;
UINT    status;
UINT    table_index;
UINT    pool_index;

    /* Default status to success.  */
    status =  TX_SUCCESS;

    /* Find first free table slot in module control block.  */
    for(i = 0; i < TXM_MODULE_LEVEL2_EXTERNAL_PAGES; i++)
    {
        if(module_instance->txm_external_page_table[i] == TX_NULL)
        {
            table_index = i;
            break;
        }
    }

    if(i >= TXM_MODULE_LEVEL2_EXTERNAL_PAGES)
    {
        status = TXM_MODULE_EXTERNAL_MEMORY_ENABLE_ERROR;
    }

    else
    {
        /* Find first free table in pool.  */
        for(i = 0; i < TXM_LEVEL2_EXTERNAL_POOL_PAGES; i++)
        {
            if(_txm_level2_external_page_pool[i][0] == (ULONG) TX_NULL)
            {
                pool_index = i;
                break;
            }
        }

        if(i >= TXM_LEVEL2_EXTERNAL_POOL_PAGES)
        {
            status = TXM_MODULE_EXTERNAL_MEMORY_ENABLE_ERROR;
        }
    }


    if(status == TX_SUCCESS)
    {
        /* Place page address in table slot. Return page address. */
        module_instance->txm_external_page_table[table_index] = _txm_level2_external_page_pool[pool_index];
        *page_addr = (ULONG)_txm_level2_external_page_pool[pool_index];
    }

    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_level2_page_clear                          Cortex-A7/MMU/GNU   */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function clears the first entry in a L2 page table and clears  */
/*    the table entry from the module external page table list.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                       Module instance pointer       */
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
VOID    _txm_level2_page_clear(TXM_MODULE_INSTANCE *module_instance)
{
UINT    i;

    /* Clear table slots and zero out L2 entry.  */
    for(i = 0; i < TXM_MODULE_LEVEL2_EXTERNAL_PAGES; i++)
    {
        if(module_instance->txm_external_page_table[i])
        {
            *(ULONG *)module_instance->txm_external_page_table[i] = (ULONG)TX_NULL;
            module_instance->txm_external_page_table[i] = TX_NULL;
        }
    }
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _txm_module_manager_external_memory_enable      Cortex-A7/MMU/GNU   */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates an entry in the MMU table for a shared        */
/*    memory space.                                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    module_instance                       Module instance pointer       */
/*    start_address                         Start address of memory       */
/*    length                                Length of external memory     */
/*    attributes                            Memory attributes             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _tx_mutex_get                         Get protection mutex          */
/*    _tx_mutex_put                         Release protection mutex      */
/*    TX_MEMSET                             Fill memory with constant     */
/*    _txm_level2_page_get                  Get L2 page table             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx      Scott Larson            Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
UINT  _txm_module_manager_external_memory_enable(   TXM_MODULE_INSTANCE *module_instance,
                                                    VOID *start_address,
                                                    ULONG length,
                                                    UINT attributes)
{

ULONG   start_addr = (ULONG) start_address;
ULONG   end_addr;
ULONG   mmu_l1_entries;
ULONG   mmu_l2_entries = 0;
ULONG   level1_index;
ULONG   level2_index;
ULONG   temp_index;
ULONG   temp_addr;
ULONG   page_addr;
ULONG   asid;
ULONG   level1_attributes;
ULONG   level2_attributes;
UINT    status;
UINT    i;

    /* Determine if the module manager has not been initialized yet.  */
    if (_txm_module_manager_ready != TX_TRUE)
    {
        /* Module manager has not been initialized.  */
        return(TX_NOT_AVAILABLE);
    }

    /* Determine if the module is valid.  */
    if (module_instance == TX_NULL)
    {
        /* Invalid module pointer.  */
        return(TX_PTR_ERROR);
    }

    /* Get module manager protection mutex.  */
    _tx_mutex_get(&_txm_module_manager_mutex, TX_WAIT_FOREVER);

    /* Determine if the module instance is valid.  */
    if (module_instance -> txm_module_instance_id != TXM_MODULE_ID)
    {
        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Invalid module pointer.  */
        return(TX_PTR_ERROR);
    }

    /* Determine if the module instance is in the loaded state.  */
    if (module_instance -> txm_module_instance_state != TXM_MODULE_LOADED)
    {
        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Return error if the module is not ready.  */
        return(TX_START_ERROR);
    }

    /* Determine if the module instance is memory protected.  */
    if (module_instance -> txm_module_instance_asid == 0)
    {
        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Return error if the module is not protected.  */
        return(TXM_MODULE_EXTERNAL_MEMORY_ENABLE_ERROR);
    }

    /* Start address must be aligned to MMU block size (4 kB).
       Length will be rounded up to 4 kB alignment.  */
    if(start_addr & ~TXM_MMU_LEVEL2_MASK)
    {
        /* Release the protection mutex.  */
        _tx_mutex_put(&_txm_module_manager_mutex);

        /* Return alignment error.  */
        return(TXM_MODULE_ALIGNMENT_ERROR);
    }

    /**************************************************************************/
    /* At this point, we have a valid address. Set up MMU.  */
    /**************************************************************************/

    /* Round length up to 4 kB alignment.  */
    if(length & ~TXM_MMU_LEVEL2_MASK)
    {
        length = ((length + TXM_MODULE_MEMORY_ALIGNMENT - 1)/TXM_MODULE_MEMORY_ALIGNMENT) * TXM_MODULE_MEMORY_ALIGNMENT;
    }

    /* Get end address.  */
    end_addr = start_addr + length - 1;

    /* How many level 1 table entries does data span?  */
    mmu_l1_entries = (end_addr >> TXM_MMU_LEVEL1_PAGE_SHIFT) - (start_addr >> TXM_MMU_LEVEL1_PAGE_SHIFT) + 1;

    /* Add 1 to align. */
    end_addr++;

    /* How many level 2 table entries does data need?
     * 0: start and end addresses both aligned.
     * 1: either start or end address aligned.
     * 2: start and end addresses both not aligned. */
    if(start_addr & ~TXM_MMU_LEVEL1_MASK)
    {
        /* If start address is not aligned, increment.  */
        mmu_l2_entries++;
    }
    if(end_addr & ~TXM_MMU_LEVEL1_MASK)
    {
        /* If end address is not aligned, increment.  */
        mmu_l2_entries++;
    }

    /* Get index into L1 table.  */
    level1_index = (start_addr >> TXM_MMU_LEVEL1_PAGE_SHIFT);

    /* Get module ASID.  */
    asid = module_instance -> txm_module_instance_asid;

    /* Do start and end entries need level 2 pages?  */
    if(mmu_l2_entries > 0)
    {
        /* Build L2 attributes.  */
        level2_attributes = ((attributes & TXM_MMU_ATTRIBUTE_XN)    << TXM_MMU_LEVEL2_USER_ATTRIBUTE_XN_SHIFT)  |
                            ((attributes & TXM_MMU_ATTRIBUTE_B)     << TXM_MMU_LEVEL2_USER_ATTRIBUTE_B_SHIFT)   |
                            ((attributes & TXM_MMU_ATTRIBUTE_C)     << TXM_MMU_LEVEL2_USER_ATTRIBUTE_C_SHIFT)   |
                            ((attributes & TXM_MMU_ATTRIBUTE_AP)    << TXM_MMU_LEVEL2_USER_ATTRIBUTE_AP_SHIFT)  |
                            ((attributes & TXM_MMU_ATTRIBUTE_TEX)   << TXM_MMU_LEVEL2_USER_ATTRIBUTE_TEX_SHIFT) |
                            TXM_MMU_LEVEL2_USER_ATTRIBUTE_BASE;

        /* If start_addr is not aligned, we need a L2 page.  */
        if(start_addr & ~TXM_MMU_LEVEL1_MASK)
        {

            /* Is there already an L2 page in the L1 table?  */
            if((_txm_ttbr1_page_table[asid][level1_index] & ~TXM_MMU_LEVEL1_SECOND_MASK) == TXM_MMU_LEVEL1_SECOND_ATTRIBUTES)
            {
                page_addr = _txm_ttbr1_page_table[asid][level1_index] & TXM_MMU_LEVEL1_SECOND_MASK;
            }
            else
            {
                /* Get L2 table from pool.  */
                status = _txm_level2_page_get(module_instance, &page_addr);

                if(status != TX_SUCCESS)
                {
                    /* Release the protection mutex.  */
                    _tx_mutex_put(&_txm_module_manager_mutex);

                    return(TXM_MODULE_EXTERNAL_MEMORY_ENABLE_ERROR);
                }

                /* Clear L2 table.  */
                TX_MEMSET((void *)page_addr, 0, TXM_LEVEL_2_PAGE_TABLE_ENTRIES);

                /* Put L2 page in L1 table. */
                _txm_ttbr1_page_table[asid][level1_index] = (page_addr & TXM_MMU_LEVEL1_SECOND_MASK) | TXM_MMU_LEVEL1_SECOND_ATTRIBUTES;
            }

            /* Decrement number of L1 entries remaining.  */
            mmu_l1_entries--;

            /* Set up L2 start table.  */
            /* Determine how many entries in L2 table.  */
            if((end_addr >> TXM_MMU_LEVEL1_PAGE_SHIFT) - (start_addr >> TXM_MMU_LEVEL1_PAGE_SHIFT))
            {
                /* End address goes to next L1 page (or beyond).  */
                temp_addr = ((start_addr >> TXM_MMU_LEVEL1_PAGE_SHIFT) + 1) << (TXM_MMU_LEVEL1_PAGE_SHIFT);
                mmu_l2_entries = (temp_addr - start_addr) >> TXM_MMU_LEVEL2_PAGE_SHIFT;
            }
            else
            {
                /* End address is on the same L1 page.  */
                mmu_l2_entries = (end_addr >> TXM_MMU_LEVEL2_PAGE_SHIFT) - (start_addr >> TXM_MMU_LEVEL2_PAGE_SHIFT);
            }

            /* Insert module settings into start table.  */
            level2_index = ((start_addr & ~TXM_MMU_LEVEL1_MASK) >> TXM_MMU_LEVEL2_PAGE_SHIFT);
            for(i = 0; i < mmu_l2_entries; i++, level2_index++)
            {
                ((ULONG *) page_addr)[level2_index] = (start_addr & TXM_MMU_LEVEL1_MASK) | (level2_index << TXM_MMU_LEVEL2_PAGE_SHIFT) | level2_attributes;
            }

            level1_index++;
        }

        /* Does last entry need a level 2 page?  */
        /* If end_address is not aligned, we need a L2 page.  */
        if((end_addr & ~TXM_MMU_LEVEL1_MASK) && (mmu_l1_entries != 0))
        {
            /* Get index into L1 table.  */
            temp_index = (end_addr >> TXM_MMU_LEVEL1_PAGE_SHIFT);

            /* Is there already an L2 page in the L1 table?  */
            if((_txm_ttbr1_page_table[asid][temp_index] & ~TXM_MMU_LEVEL1_SECOND_MASK) == TXM_MMU_LEVEL1_SECOND_ATTRIBUTES)
            {
                page_addr = _txm_ttbr1_page_table[asid][temp_index] & TXM_MMU_LEVEL1_SECOND_MASK;
            }
            else
            {
                /* Get L2 table from pool.  */
                status = _txm_level2_page_get(module_instance, &page_addr);

                if(status != TX_SUCCESS)
                {
                    /* Release the protection mutex.  */
                    _tx_mutex_put(&_txm_module_manager_mutex);

                    return(TXM_MODULE_EXTERNAL_MEMORY_ENABLE_ERROR);
                }

                /* Clear L2 table.  */
                TX_MEMSET((void *)page_addr, 0, TXM_LEVEL_2_PAGE_TABLE_ENTRIES);

                /* Put L2 page in L1 table. */
                _txm_ttbr1_page_table[asid][temp_index] = (page_addr & TXM_MMU_LEVEL1_SECOND_MASK) | TXM_MMU_LEVEL1_SECOND_ATTRIBUTES;
            }

            /* Decrement number of L1 entries remaining.  */
            mmu_l1_entries--;

            /* Determine how many entries in L2 table.  */
            mmu_l2_entries = ((end_addr & ~TXM_MMU_LEVEL1_MASK) >> TXM_MMU_LEVEL2_PAGE_SHIFT);

            /* Set up L2 end table.  */
            for(i = 0; i < mmu_l2_entries; i++)
            {
                ((ULONG *) page_addr)[i] = (end_addr & TXM_MMU_LEVEL1_MASK) | (i << TXM_MMU_LEVEL2_PAGE_SHIFT) | level2_attributes;
            }
        }
    }

    /* Fill any L1 entries between start and end pages of module data range.  */
    for(i = 0; i < mmu_l1_entries; i++, level1_index++)
    {
        /* Build L1 attributes.  */
        level1_attributes = ((attributes & TXM_MMU_ATTRIBUTE_XN)    << TXM_MMU_LEVEL1_USER_ATTRIBUTE_XN_SHIFT)  |
                            ((attributes & TXM_MMU_ATTRIBUTE_B)     << TXM_MMU_LEVEL1_USER_ATTRIBUTE_B_SHIFT)   |
                            ((attributes & TXM_MMU_ATTRIBUTE_C)     << TXM_MMU_LEVEL1_USER_ATTRIBUTE_C_SHIFT)   |
                            ((attributes & TXM_MMU_ATTRIBUTE_AP)    << TXM_MMU_LEVEL1_USER_ATTRIBUTE_AP_SHIFT)  |
                            ((attributes & TXM_MMU_ATTRIBUTE_TEX)   << TXM_MMU_LEVEL1_USER_ATTRIBUTE_TEX_SHIFT) |
                            TXM_MMU_LEVEL1_USER_ATTRIBUTE_BASE;

        /* Place address and attributes in table.  */
        _txm_ttbr1_page_table[asid][level1_index] = (level1_index << TXM_MMU_LEVEL1_PAGE_SHIFT) | level1_attributes;
    }


    /* Release the protection mutex.  */
    _tx_mutex_put(&_txm_module_manager_mutex);

    /* Return success.  */
    return(TX_SUCCESS);
}
