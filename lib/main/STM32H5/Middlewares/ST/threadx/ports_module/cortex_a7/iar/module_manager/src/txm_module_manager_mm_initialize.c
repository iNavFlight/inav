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

#define CACHE_DISABLED              0x12
#define SDRAM_START                 0x00000000
#define SDRAM_END                   0x1fffffff
#define CACHE_WRITEBACK             0x1e

#define SECTION_DESCRIPTOR          0x00000002
#define DACR_CLIENT_MODE            0x55555555


/*** Page table attributes TTBR0 ***********************************************
* IRGN = 01 - Normal memory, Inner Write-Back Write-Allocate Cacheable
* S - non-shareable
* RGN = 01 - Normal memory, Outer Write-Back Write-Allocate Cacheable
* NOS - outer-shareable
*******************************************************************************/
#define TTBR0_ATTRIBUTES    0x48



/* ASID table, index is ASID number and contents hold pointer to module.  */
TXM_MODULE_INSTANCE *_txm_asid_table[TXM_ASID_TABLE_LENGTH];

/* Master page table, 2^14 (16kB) alignment.
 * First table is the master level 1 table, the rest are for each module.  */
#pragma data_alignment=16384
ULONG _txm_ttbr1_page_table[TXM_MAXIMUM_MODULES][TXM_MASTER_PAGE_TABLE_ENTRIES];

/* Module start and end level 2 page tables, 2^10 (1kB) alignment.
 * First set of 4 tables are the master level 2 tables, the rest are for each module. 
 * Each module needs two L2 tables for code and two L2 tables for data. */
#pragma data_alignment=1024 
ULONG _txm_level2_module_page_table[TXM_MAXIMUM_MODULES * 4][TXM_LEVEL_2_PAGE_TABLE_ENTRIES];

/* Module external memory level 2 page tables, 2^10 (1kB) alignment.  */
#pragma data_alignment=1024 
ULONG _txm_level2_external_page_pool[TXM_LEVEL2_EXTERNAL_POOL_PAGES][TXM_LEVEL_2_PAGE_TABLE_ENTRIES];

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _txm_module_manager_mm_initialize                Cortex-A7/MMU/IAR  */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Scott Larson, Microsoft Corporation                                 */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This function performs the initial set up of the the A7 MMU.        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    None                                                                */ 
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
/*    Application code                                                    */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  09-30-2020      Scott Larson            Initial Version 6.1           */
/*                                                                        */
/**************************************************************************/
UINT  _txm_module_manager_mm_initialize(VOID)
{

#ifdef TXM_MODULE_MEMORY_PROTECTION_ENABLED
    UINT    i;
    ULONG   cp15reg;
    UINT    user_mode_index;
    UINT    counter_limit;
    
    /* Clear ASID table.  */
    for (i = 0; i < TXM_ASID_TABLE_LENGTH; i++)
    {
        _txm_asid_table[i] = 0;
    }
    _txm_asid_table[0] = (TXM_MODULE_INSTANCE *)TXM_ASID_RESERVED;
    
    
    /********************************************************************************/
    /* This is an example showing how to set up the cache attributes.               */
    /********************************************************************************/

/*******************************************************************************
* PAGE TABLE generation 
* Generate the page tables 
* Build a flat translation table for the whole address space. 
* ie: Create 4096 1MB sections from 0x000xxxxx to 0xFFFxxxxx 
*  31           20|19 18|17|16| 15|14 12|11 10|9|8    5|4 |3 2|1 0|
* |base address   | 0  0|nG| S|AP2|TEX  |AP   |P|Domain|XN|CB |1 0|
*
* Bits[31:20]   - Top 12 bits of VA is pointer into table 
* nG[17]=0      - Non global, enables matching against ASID in the TLB when set.
* S[16]=0       - Indicates normal memory is shared when set.
* AP2[15]=0  
* TEX[14:12]=000
* AP[11:10]=11  - Configure for full read/write access in all modes 
* IMPP[9]=0     - Ignored
* Domain[5:8]=1111   - Set all pages to use domain 15
* XN[4]=0       - Execute never disabled
* CB[3:2]= 00   - Set attributes to Strongly-ordered memory. 
*                 (except for the descriptor where code segment is based, 
*                  see below) 
* Bits[1:0]=10  - Indicate entry is a 1MB section 
*******************************************************************************/

/* ---- Parameter setting to level1 descriptor (bits 19:0) ---- */
/* setting for Strongly-ordered memory     
   B-00000000000000000000010111100010 */
#define TTB_PARA_STRGLY             0x05E2

/* setting for Outer and inner not cache normal memory 
   B-00000000000000000001010111100010 */
#define TTB_PARA_NORMAL_NOT_CACHE   0x15E2

/* setting for Outer and inner write back, write allocate normal memory 
   (Cacheable) 
   B-00000000000000000001010111101110 */
#define TTB_PARA_NORMAL_CACHE       0x15EE  //0x15EE

/* In this chip (RZA1) there are the following 12 sections with the defined memory size (MB) */
#define M_SIZE_NOR      128     /* [Area00] CS0, CS1 area (for NOR flash) */
#define M_SIZE_SDRAM    128     /* [Area01] CS2, CS3 area (for SDRAM) */
#define M_SIZE_CS45     128     /* [Area02] CS4, CS5 area */
#define M_SIZE_SPI      128     /* [Area03] SPI, SP2 area (for Serial flash) */
#define M_SIZE_RAM      10      /* [Area04] Internal RAM */
#define M_SIZE_IO_1     502     /* [Area05] I/O area 1 */
#define M_SIZE_NOR_M    128     /* [Area06] CS0, CS1 area (for NOR flash) (mirror) */
#define M_SIZE_SDRAM_M  128     /* [Area07] CS2, CS3 area (for SDRAM) (mirror) */
#define M_SIZE_CS45_M   128     /* [Area08] CS4, CS5 area (mirror) */
#define M_SIZE_SPI_M    128     /* [Area09] SPI, SP2 area (for Serial flash) (mirror) */
#define M_SIZE_RAM_M    10      /* [Area10] Internal RAM (mirror) */
#define M_SIZE_IO_2     2550    /* [Area11] I/O area 2 */
/* Should add to:       4096 */
    
    counter_limit = M_SIZE_NOR;
    for (i = 0; i < counter_limit; i++)
    {
        _txm_ttbr1_page_table[TXM_MASTER_PAGE_TABLE_INDEX][i] = (i << TXM_MMU_LEVEL1_PAGE_SHIFT) | TTB_PARA_NORMAL_CACHE;
    }
    
    counter_limit += M_SIZE_SDRAM;
    for (; i < counter_limit; i++)
    {
        _txm_ttbr1_page_table[TXM_MASTER_PAGE_TABLE_INDEX][i] = (i << TXM_MMU_LEVEL1_PAGE_SHIFT) | TTB_PARA_NORMAL_CACHE;
    }
    
    counter_limit += M_SIZE_CS45;
    for (; i < counter_limit; i++)
    {
        _txm_ttbr1_page_table[TXM_MASTER_PAGE_TABLE_INDEX][i] = (i << TXM_MMU_LEVEL1_PAGE_SHIFT) | TTB_PARA_STRGLY;
    }
    
    counter_limit += M_SIZE_SPI;
    for (; i < counter_limit; i++)
    {
        _txm_ttbr1_page_table[TXM_MASTER_PAGE_TABLE_INDEX][i] = (i << TXM_MMU_LEVEL1_PAGE_SHIFT) | TTB_PARA_NORMAL_CACHE;
    }
    
    counter_limit += M_SIZE_RAM;
    for (; i < counter_limit; i++)
    {
        _txm_ttbr1_page_table[TXM_MASTER_PAGE_TABLE_INDEX][i] = (i << TXM_MMU_LEVEL1_PAGE_SHIFT) | TTB_PARA_NORMAL_CACHE;
    }
    
    counter_limit += M_SIZE_IO_1;
    for (; i < counter_limit; i++)
    {
        _txm_ttbr1_page_table[TXM_MASTER_PAGE_TABLE_INDEX][i] = (i << TXM_MMU_LEVEL1_PAGE_SHIFT) | TTB_PARA_STRGLY;
    }
    
    counter_limit += M_SIZE_NOR_M;
    for (; i < counter_limit; i++)
    {
        _txm_ttbr1_page_table[TXM_MASTER_PAGE_TABLE_INDEX][i] = (i << TXM_MMU_LEVEL1_PAGE_SHIFT) | TTB_PARA_NORMAL_NOT_CACHE;
    }
    
    counter_limit += M_SIZE_SDRAM_M;
    for (; i < counter_limit; i++)
    {
        _txm_ttbr1_page_table[TXM_MASTER_PAGE_TABLE_INDEX][i] = (i << TXM_MMU_LEVEL1_PAGE_SHIFT) | TTB_PARA_NORMAL_NOT_CACHE;
    }
    
    counter_limit += M_SIZE_CS45_M;
    for (; i < counter_limit; i++)
    {
        _txm_ttbr1_page_table[TXM_MASTER_PAGE_TABLE_INDEX][i] = (i << TXM_MMU_LEVEL1_PAGE_SHIFT) | TTB_PARA_STRGLY;
    }
    
    counter_limit += M_SIZE_SPI_M;
    for (; i < counter_limit; i++)
    {
        _txm_ttbr1_page_table[TXM_MASTER_PAGE_TABLE_INDEX][i] = (i << TXM_MMU_LEVEL1_PAGE_SHIFT) | TTB_PARA_NORMAL_NOT_CACHE;
    }
    
    counter_limit += M_SIZE_RAM_M;
    for (; i < counter_limit; i++)
    {
        _txm_ttbr1_page_table[TXM_MASTER_PAGE_TABLE_INDEX][i] = (i << TXM_MMU_LEVEL1_PAGE_SHIFT) | TTB_PARA_NORMAL_NOT_CACHE;
    }
    
    counter_limit += M_SIZE_IO_2;
    for (; i < counter_limit; i++)
    {
        _txm_ttbr1_page_table[TXM_MASTER_PAGE_TABLE_INDEX][i] = (i << TXM_MMU_LEVEL1_PAGE_SHIFT) | TTB_PARA_STRGLY;
    }
    
    /********************************************************************************/
    /* This is the end of the example showing how to set up the cache attributes.   */
    /********************************************************************************/
    
    
    /* Clear ASID.  */
    cp15reg = 0;
    asm volatile ("mcr p15, 0, %0, c13, c0, 1" : : "r"(cp15reg) : );
    asm("isb");
    
    /* Put the page table address in TTBR.  */
    cp15reg = (int)(VOID*)_txm_ttbr1_page_table;
    cp15reg |= TTBR0_ATTRIBUTES;
    asm volatile ("mcr p15, 0, %0, c2, c0, 0" : : "r"(cp15reg) : );
    
    /* Set the domain to client mode.  */
    cp15reg = DACR_CLIENT_MODE;
    asm volatile ("mcr p15, 0, %0, c3, c0, 0" : : "r"(cp15reg) : );
    

/* Level 2 small page attributes: normal memory, cache & buffer enabled, priviledged access. */
#define TTB_LEVEL2_NORMAL_CACHE         0x05E

/* Level 2 clear AP attributes mask. */
#define TTB_LEVEL2_AP_CLEAR_MASK        0xFFFFFFCF

/* Attributes for user mode table entry in level 2 table. */
#define TTB_LEVEL2_USER_MODE_ENTRY      0x06E
    
    /* Set up Level 2 table for user to kernel mode entry trampoline.  */
    /* Find which table entry _txm_module_manager_user_mode_entry is in.  */
    user_mode_index = (ULONG)_txm_module_manager_user_mode_entry >> TXM_MMU_LEVEL1_PAGE_SHIFT;
    /* Fill table.  */
    for (i = 0; i < TXM_LEVEL_2_PAGE_TABLE_ENTRIES; i++)
    {
        _txm_level2_module_page_table[TXM_MASTER_PAGE_TABLE_INDEX][i] = ((ULONG)_txm_module_manager_user_mode_entry & TXM_MMU_LEVEL1_MASK) | (i << TXM_MMU_LEVEL2_PAGE_SHIFT) | TTB_LEVEL2_NORMAL_CACHE;
    }
    
    /* Enter Level 2 table in to master table.  */
    _txm_ttbr1_page_table[TXM_MASTER_PAGE_TABLE_INDEX][user_mode_index] = ((ULONG)_txm_level2_module_page_table & TXM_MMU_LEVEL1_SECOND_MASK) | TXM_MMU_LEVEL1_SECOND_ATTRIBUTES;
    
    /* Find level 2 entry that holds _txm_module_manager_user_mode_entry.  */
    user_mode_index = ((ULONG)_txm_module_manager_user_mode_entry & ~TXM_MMU_LEVEL1_MASK) >> TXM_MMU_LEVEL2_PAGE_SHIFT;
    
    /* Set attribute bits for the user mode entry page.  */
    _txm_level2_module_page_table[TXM_MASTER_PAGE_TABLE_INDEX][user_mode_index] = (_txm_level2_module_page_table[TXM_MASTER_PAGE_TABLE_INDEX][user_mode_index] & TTB_LEVEL2_AP_CLEAR_MASK) | TTB_LEVEL2_USER_MODE_ENTRY;
    
    /* Enable the MMU.  */
    asm volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r"(cp15reg) : : );
    cp15reg |= 0x1;
    asm volatile ("mcr p15, 0, %0, c1, c0, 0" : : "r"(cp15reg) : );
    
    return(TX_SUCCESS);

#else
    return(TX_FEATURE_NOT_ENABLED);
#endif
}
