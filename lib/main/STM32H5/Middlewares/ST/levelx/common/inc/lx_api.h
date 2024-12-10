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
/** LevelX Component                                                      */ 
/**                                                                       */
/**   Application Interface (API)                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    lx_api.h                                            PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    high-performance LevelX. All service prototypes and data structure  */
/*    definitions are defined in this file.                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1    */
/*  11-09-2020     William E. Lamie         Modified comment(s), and      */
/*                                            added support for lx_user.h */
/*                                            so user can specify values, */
/*                                            resulting in version 6.1.2  */
/*  12-31-2020     William E. Lamie         Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1.3  */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            added standalone support,   */
/*                                            resulting in version 6.1.7  */
/*  08-02-2021     William E. Lamie         Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1.8  */
/*  10-15-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            updated configuration for   */
/*                                            nand flash                  */
/*                                            resulting in version 6.1.9  */
/*  01-31-2022     Bhupendra Naphade        Modified comment(s),          */
/*                                            updated include order for   */
/*                                            standalone mode,            */
/*                                            resulting in version 6.1.10 */
/*  04-25-2022     William E. Lamie         Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022     William E. Lamie         Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022     Xiuwen Cai               Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/

#ifndef LX_API_H
#define LX_API_H


/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Determine if the optional LevelX user define file should be used.  */

#ifdef LX_INCLUDE_USER_DEFINE_FILE


/* Yes, include the user defines in lx_user.h. The defines in this file may 
   alternately be defined on the command line.  */

#include "lx_user.h"
#endif

/* Include the ThreadX api file.  */

#ifndef LX_STANDALONE_ENABLE
#include "tx_api.h"
#endif


#ifdef LX_STANDALONE_ENABLE

/* Define compiler library include files.  */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef VOID
#define VOID                                    void
typedef char                                    CHAR;
typedef char                                    BOOL;
typedef unsigned char                           UCHAR;
typedef int                                     INT;
typedef unsigned int                            UINT;
typedef long                                    LONG;
typedef unsigned long                           ULONG;
typedef short                                   SHORT;
typedef unsigned short                          USHORT;
#endif

#ifndef ULONG64_DEFINED
#define ULONG64_DEFINED
typedef unsigned long long                      ULONG64;
#endif

/* Define basic alignment type used in block and byte pool operations. This data type must
   be at least 32-bits in size and also be large enough to hold a pointer type.  */

#ifndef ALIGN_TYPE_DEFINED
#define ALIGN_TYPE_DEFINED
#define ALIGN_TYPE                              ULONG
#endif

/* Define the LX_MEMSET macro to the standard library function, if not already defined.  */
#ifndef LX_MEMSET
#define LX_MEMSET(a,b,c)                        memset((a),(b),(c))
#endif

/* Disable usage of ThreadX mutex in standalone mode */
#ifdef LX_THREAD_SAFE_ENABLE
#undef LX_THREAD_SAFE_ENABLE
#endif

#define LX_INTERRUPT_SAVE_AREA
#define LX_DISABLE
#define LX_RESTORE

#else

#define LX_MEMSET                               TX_MEMSET

#define LX_INTERRUPT_SAVE_AREA                  TX_INTERRUPT_SAVE_AREA
#define LX_DISABLE                              TX_DISABLE
#define LX_RESTORE                              TX_RESTORE

#endif


/* Disable warning of parameter not used. */
#ifndef LX_PARAMETER_NOT_USED
#define LX_PARAMETER_NOT_USED(p) ((void)(p))
#endif /* LX_PARAMETER_NOT_USED */


/* Define basic constants for the LevelX Stack.  */
#define AZURE_RTOS_LEVELX
#define LEVELX_MAJOR_VERSION                        6
#define LEVELX_MINOR_VERSION                        2
#define LEVELX_PATCH_VERSION                        0


/* Define general LevelX Constants.  */

#define LX_FALSE                                    0
#define LX_TRUE                                     1
#define LX_NULL                                     0
#define LX_BLOCK_ERASE_COUNT_MASK                   0x7FFFFFFF
#define LX_BLOCK_ERASE_COUNT_MAX                    0x7FFFFFFF
#define LX_BLOCK_ERASED                             0x80000000
#define LX_BLOCK_ERASE_STARTED                      0
#define LX_ALL_ONES                                 0xFFFFFFFF


/* Define error codes.  */

#define LX_SUCCESS                                  0
#define LX_ERROR                                    1
#define LX_NO_SECTORS                               2
#define LX_SECTOR_NOT_FOUND                         3
#define LX_NO_PAGES                                 4
#define LX_INVALID_WRITE                            5
#define LX_NAND_ERROR_CORRECTED                     6
#define LX_NAND_ERROR_NOT_CORRECTED                 7
#define LX_NO_MEMORY                                8
#define LX_DISABLED                                 9
#define LX_SYSTEM_INVALID_FORMAT                    90
#define LX_SYSTEM_INVALID_BLOCK                     91
#define LX_SYSTEM_ALLOCATION_FAILED                 92
#define LX_SYSTEM_MUTEX_CREATE_FAILED               93
#define LX_SYSTEM_INVALID_SECTOR_MAP                94 


/* Define NOR flash constants.  */

#define LX_NOR_FLASH_OPENED                         ((ULONG) 0x4E4F524F)
#define LX_NOR_FLASH_CLOSED                         ((ULONG) 0x4E4F5244)
#define LX_NOR_SECTOR_SIZE                          (512/sizeof(ULONG))
#define LX_NOR_FLASH_MIN_LOGICAL_SECTOR_OFFSET      1
#define LX_NOR_FLASH_MAX_LOGICAL_SECTOR_OFFSET      2
#ifndef LX_NOR_FLASH_MAX_ERASE_COUNT_DELTA
#define LX_NOR_FLASH_MAX_ERASE_COUNT_DELTA          4
#endif
#define LX_NOR_SECTOR_MAPPING_CACHE_DEPTH           4
#ifndef LX_NOR_SECTOR_MAPPING_CACHE_SIZE
#define LX_NOR_SECTOR_MAPPING_CACHE_SIZE            16          /* Minimum value of 8, all sizes must be a power of 2.  */
#endif
#ifndef LX_NOR_EXTENDED_CACHE_SIZE
#define LX_NOR_EXTENDED_CACHE_SIZE                  8           /* Maximum number of extended cache sectors.            */
#endif


/* Define the mask for the hash index into the sector mapping cache table.  The sector mapping cache is divided 
   into 4 entry pieces that are indexed by the formula:  
   
            index =  (sector & LX_NOR_SECTOR_MAPPING_CACHE_HASH_MASK) * LX_NOR_SECTOR_MAPPING_CACHE_DEPTH

   The LX_NOR_SECTOR_MAPPING_CACHE_DEPTH define must not be changed unless the related source code is also changed.  */

#define LX_NOR_SECTOR_MAPPING_CACHE_HASH_MASK       ((LX_NOR_SECTOR_MAPPING_CACHE_SIZE/LX_NOR_SECTOR_MAPPING_CACHE_DEPTH)-1)
#define LX_NOR_SECTOR_MAPPING_CACHE_ENTRY_MASK      0x7FFFFFFF
#define LX_NOR_SECTOR_MAPPING_CACHE_ENTRY_VALID     0x80000000

#define LX_NOR_PHYSICAL_SECTOR_VALID                0x80000000
#define LX_NOR_PHYSICAL_SECTOR_SUPERCEDED           0x40000000
#define LX_NOR_PHYSICAL_SECTOR_MAPPING_NOT_VALID    0x20000000
#define LX_NOR_LOGICAL_SECTOR_MASK                  0x1FFFFFFF
#define LX_NOR_PHYSICAL_SECTOR_FREE                 0xFFFFFFFF


/* Define NAND flash constants.  */

#define LX_NAND_GOOD_BLOCK                          0xFF
#define LX_NAND_BAD_BLOCK                           0x00
#define LX_NAND_FLASH_OPENED                        ((ULONG) 0x4E4F524F)
#define LX_NAND_FLASH_CLOSED                        ((ULONG) 0x4E4F5244)
#ifndef LX_NAND_FLASH_MAX_ERASE_COUNT_DELTA
#define LX_NAND_FLASH_MAX_ERASE_COUNT_DELTA         64
#endif

/* Define the NAND sector mapping cache.  */

#define LX_NAND_SECTOR_MAPPING_CACHE_DEPTH          4           /* Not required if LX_NAND_FLASH_DIRECT_MAPPING_CACHE is defined.  */   
#ifndef LX_NAND_SECTOR_MAPPING_CACHE_SIZE
#define LX_NAND_SECTOR_MAPPING_CACHE_SIZE           128         /* Minimum value of 8, all sizes must be a power of 2, unless direct
                                                                   mapping is defined, in which there is no power of 2 requirement.  */
#endif
#ifndef LX_NAND_ERASE_COUNT_WRITE_SIZE
#define LX_NAND_ERASE_COUNT_WRITE_SIZE              (nand_flash -> lx_nand_flash_pages_per_block + 1)
#endif  

#ifndef LX_NAND_FLASH_MAPPING_LIST_UPDATE_DISABLE
#define LX_NAND_FLASH_MAPPING_LIST_UPDATE_DISABLE
#endif 


/* Define the mask for the hash index into the NAND sector mapping cache table.  The sector mapping cache is divided 
   into 4 entry pieces that are indexed by the formula:  
   
            index =  (sector & LX_NAND_SECTOR_MAPPING_CACHE_HASH_MASK) * LX_NAND_SECTOR_MAPPING_CACHE_DEPTH

   The LX_NAND_SECTOR_MAPPING_CACHE_DEPTH define must not be changed unless the related source code is also changed.  */

#define LX_NAND_SECTOR_MAPPING_CACHE_HASH_MASK      ((LX_NAND_SECTOR_MAPPING_CACHE_SIZE/LX_NAND_SECTOR_MAPPING_CACHE_DEPTH)-1)
#define LX_NAND_SECTOR_MAPPING_CACHE_ENTRY_MASK     0x7FFFFFFF
#define LX_NAND_SECTOR_MAPPING_CACHE_ENTRY_VALID    0x80000000

#define LX_NAND_BLOCK_VALID                         0x80000000
#define LX_NAND_BLOCK_EMPTY                         0x40000000
#define LX_NAND_BLOCK_FULL                          0x20000000
#define LX_NAND_PAGE_VALID                          0x80000000
#define LX_NAND_PAGE_SUPERCEDED                     0x40000000
#define LX_NAND_PAGE_MAPPING_NOT_VALID              0x20000000
#define LX_NAND_LOGICAL_SECTOR_MASK                 0x1FFFFFFF
#define LX_NAND_PAGE_FREE                           0xFFFFFFFF
#define LX_NAND_PAGE_LIST_VALID                     0xF0F0F0F0


/* Define the NAND flash extra byte structure. This will be set in the spare area of each mapped physical page.  */

typedef struct LX_NAND_PAGE_EXTRA_INFO_STRUCT
{
    ULONG                           lx_nand_page_extra_info_logical_sector;
} LX_NAND_PAGE_EXTRA_INFO;


/* Define the NAND flash logical sector cache entry structure.  */

typedef struct LX_NAND_SECTOR_MAPPING_CACHE_ENTRY_STRUCT
{
#ifndef LX_NAND_FLASH_DIRECT_MAPPING_CACHE
    ULONG                           lx_nand_sector_mapping_cache_logical_sector;
#endif
    USHORT                          lx_nand_sector_mapping_cache_block; 
    USHORT                          lx_nand_sector_mapping_cache_page;
} LX_NAND_SECTOR_MAPPING_CACHE_ENTRY;


/* Define the NAND flash control block structure.  */

typedef struct LX_NAND_FLASH_STRUCT
{
    ULONG                           lx_nand_flash_state;
    ULONG                           lx_nand_flash_total_blocks;
    ULONG                           lx_nand_flash_pages_per_block;
    ULONG                           lx_nand_flash_bytes_per_page;
    ULONG                           lx_nand_flash_words_per_block;
    ULONG                           lx_nand_flash_words_per_page;
    ULONG                           lx_nand_flash_total_pages;

    ULONG                           lx_nand_flash_bad_blocks;
    ULONG                           lx_nand_flash_free_pages;
    ULONG                           lx_nand_flash_mapped_pages;
    ULONG                           lx_nand_flash_obsolete_pages;
    ULONG                           lx_nand_flash_minimum_erase_count;
    ULONG                           lx_nand_flash_maximum_erase_count;
    ULONG                           lx_nand_flash_free_block_search;
    ULONG                           lx_nand_flash_found_block_search;
    ULONG                           lx_nand_flash_found_page_search;   

    ULONG                           lx_nand_flash_max_mapped_sector;
    ULONG                           lx_nand_flash_page_corrections;
    ULONG                           lx_nand_flash_last_block_correction;
    ULONG                           lx_nand_flash_last_page_correction;

    ULONG                           lx_nand_flash_diagnostic_system_errors;
    ULONG                           lx_nand_flash_diagnostic_system_error;
    ULONG                           lx_nand_flash_diagnostic_sector_write_requests;
    ULONG                           lx_nand_flash_diagnostic_sector_read_requests;
    ULONG                           lx_nand_flash_diagnostic_sector_release_requests;
    ULONG                           lx_nand_flash_diagnostic_page_allocates;
    ULONG                           lx_nand_flash_diagnostic_page_allocate_errors;

    ULONG                           lx_nand_flash_diagnostic_sector_mapping_cache_hits;
    ULONG                           lx_nand_flash_diagnostic_sector_mapping_cache_misses;
    ULONG                           lx_nand_flash_diagnostic_page_extra_bytes_cache_hits;
    ULONG                           lx_nand_flash_diagnostic_page_extra_bytes_cache_misses;
    ULONG                           lx_nand_flash_diagnostic_page_0_cache_hits;
    ULONG                           lx_nand_flash_diagnostic_page_0_cache_misses;
    ULONG                           lx_nand_flash_diagnostic_block_status_cache_hits;
    ULONG                           lx_nand_flash_diagnostic_block_status_cache_misses;    
       
    ULONG                           lx_nand_flash_diagnostic_block_reclaim_attempts;
    ULONG                           lx_nand_flash_diagnostic_block_erases;
    ULONG                           lx_nand_flash_diagnostic_block_status_gets;
    ULONG                           lx_nand_flash_diagnostic_block_status_sets;
    ULONG                           lx_nand_flash_diagnostic_page_extra_bytes_sets;
    ULONG                           lx_nand_flash_diagnostic_page_writes;
    ULONG                           lx_nand_flash_diagnostic_page_extra_bytes_gets;
    ULONG                           lx_nand_flash_diagnostic_page_reads;
    ULONG                           lx_nand_flash_diagnostic_moved_pages;
    ULONG                           lx_nand_flash_diagnostic_block_erased_verifies;
    ULONG                           lx_nand_flash_diagnostic_page_erased_verifies;
    
    ULONG                           lx_nand_flash_diagnostic_initial_format;
    ULONG                           lx_nand_flash_diagnostic_erased_block;
    ULONG                           lx_nand_flash_diagnostic_re_erase_block;
    ULONG                           lx_nand_flash_diagnostic_page_being_obsoleted;
    ULONG                           lx_nand_flash_diagnostic_page_obsoleted;
    ULONG                           lx_nand_flash_diagnostic_mapping_invalid;
    ULONG                           lx_nand_flash_diagnostic_mapping_write_interrupted;
    ULONG                           lx_nand_flash_diagnostic_page_not_free;
    ULONG                           lx_nand_flash_diagnostic_page_data_not_free;

    UINT                            (*lx_nand_flash_driver_read)(ULONG block, ULONG page, ULONG *destination, ULONG words);
    UINT                            (*lx_nand_flash_driver_write)(ULONG block, ULONG page, ULONG *source, ULONG words);
    UINT                            (*lx_nand_flash_driver_block_erase)(ULONG block, ULONG erase_count);
    UINT                            (*lx_nand_flash_driver_block_erased_verify)(ULONG block);
    UINT                            (*lx_nand_flash_driver_page_erased_verify)(ULONG block, ULONG page);
    UINT                            (*lx_nand_flash_driver_block_status_get)(ULONG block, UCHAR *bad_block_flag);
    UINT                            (*lx_nand_flash_driver_block_status_set)(ULONG block, UCHAR bad_block_flag);
    UINT                            (*lx_nand_flash_driver_extra_bytes_get)(ULONG block, ULONG page, UCHAR *destination, UINT size);
    UINT                            (*lx_nand_flash_driver_extra_bytes_set)(ULONG block, ULONG page, UCHAR *source, UINT size);
    UINT                            (*lx_nand_flash_driver_system_error)(UINT error_code, ULONG block, ULONG page);
    
    ULONG                           *lx_nand_flash_page_buffer;
    UINT                            lx_nand_flash_sector_mapping_cache_enabled;
    LX_NAND_SECTOR_MAPPING_CACHE_ENTRY   
                                    lx_nand_flash_sector_mapping_cache[LX_NAND_SECTOR_MAPPING_CACHE_SIZE];


    /* Define the extended cache structures for block status, page extra bytes, and block page 0 caches. The memory for these cache
       extensions is optionally supplied by the application after the lx_nand_flash_open call has been made.  */
       
    UCHAR                           *lx_nand_flash_block_status_cache;
    LX_NAND_PAGE_EXTRA_INFO         *lx_nand_flash_page_extra_bytes_cache;
    ULONG                           *lx_nand_flash_page_0_cache;

#ifdef LX_THREAD_SAFE_ENABLE

    /* When this conditional is used, the LevelX code utilizes a ThreadX mutex for thread
       safe operation. Generally, this is not required since FileX ensures thread safe operation at
       a higher layer.  */
    TX_MUTEX                        lx_nand_flash_mutex;
#endif
    
    /* Define the NAND flash control block open next/previous pointers.  */
    struct LX_NAND_FLASH_STRUCT     *lx_nand_flash_open_next,
                                    *lx_nand_flash_open_previous;
    
} LX_NAND_FLASH;



/* Define the NOR flash sector cache entry structure.  */

typedef struct LX_NOR_SECTOR_MAPPING_CACHE_ENTRY_STRUCT
{
    ULONG                           lx_nor_sector_mapping_cache_logical_sector;
    ULONG                           *lx_nor_sector_mapping_cache_physical_sector_map_entry; 
    ULONG                           *lx_nor_sector_mapping_cache_physical_sector_address;
} LX_NOR_SECTOR_MAPPING_CACHE_ENTRY;


/* Define the NOR flash extended cache entry structure.  */

typedef struct LX_NOR_FLASH_EXTENDED_CACHE_ENTRY_STRUCT
{
    ULONG                           *lx_nor_flash_extended_cache_entry_sector_address; 
    ULONG                           *lx_nor_flash_extended_cache_entry_sector_memory;
    ULONG                           lx_nor_flash_extended_cache_entry_access_count;
} LX_NOR_FLASH_EXTENDED_CACHE_ENTRY;


/* Define the NOR flash control block structure.  */

typedef struct LX_NOR_FLASH_STRUCT
{
    ULONG                           lx_nor_flash_state;
    ULONG                           lx_nor_flash_total_blocks;
    ULONG                           lx_nor_flash_words_per_block;
    ULONG                           lx_nor_flash_total_physical_sectors;
    ULONG                           lx_nor_flash_physical_sectors_per_block;

    ULONG                           *lx_nor_flash_base_address;
    ULONG                           lx_nor_flash_block_free_bit_map_offset;
    ULONG                           lx_nor_flash_block_bit_map_words;
    ULONG                           lx_nor_flash_block_bit_map_mask;
    ULONG                           lx_nor_flash_block_physical_sector_mapping_offset;
    ULONG                           lx_nor_flash_block_physical_sector_offset;

    ULONG                           lx_nor_flash_free_physical_sectors;
    ULONG                           lx_nor_flash_mapped_physical_sectors;
    ULONG                           lx_nor_flash_obsolete_physical_sectors;
    ULONG                           lx_nor_flash_minimum_erase_count;
    ULONG                           lx_nor_flash_maximum_erase_count;

    ULONG                           lx_nor_flash_free_block_search;
    ULONG                           lx_nor_flash_found_block_search;
    ULONG                           lx_nor_flash_found_sector_search;   

    ULONG                           lx_nor_flash_write_requests;
    ULONG                           lx_nor_flash_read_requests;
    ULONG                           lx_nor_flash_sector_mapping_cache_hits;
    ULONG                           lx_nor_flash_sector_mapping_cache_misses;
    ULONG                           lx_nor_flash_physical_block_allocates;
    ULONG                           lx_nor_flash_physical_block_allocate_errors;
    ULONG                           lx_nor_flash_diagnostic_system_errors;
    ULONG                           lx_nor_flash_diagnostic_system_error;
    ULONG                           lx_nor_flash_diagnostic_initial_format;
    ULONG                           lx_nor_flash_diagnostic_erased_block;
    ULONG                           lx_nor_flash_diagnostic_re_erase_block;
    ULONG                           lx_nor_flash_diagnostic_sector_being_obsoleted;
    ULONG                           lx_nor_flash_diagnostic_sector_obsoleted;
    ULONG                           lx_nor_flash_diagnostic_mapping_invalidated;
    ULONG                           lx_nor_flash_diagnostic_mapping_write_interrupted;
    ULONG                           lx_nor_flash_diagnostic_sector_not_free;
    ULONG                           lx_nor_flash_diagnostic_sector_data_not_free;

    UINT                            (*lx_nor_flash_driver_read)(ULONG *flash_address, ULONG *destination, ULONG words);
    UINT                            (*lx_nor_flash_driver_write)(ULONG *flash_address, ULONG *source, ULONG words);
    UINT                            (*lx_nor_flash_driver_block_erase)(ULONG block, ULONG erase_count);
    UINT                            (*lx_nor_flash_driver_block_erased_verify)(ULONG block);
    UINT                            (*lx_nor_flash_driver_system_error)(UINT error_code);

    ULONG                           *lx_nor_flash_sector_buffer;
    UINT                            lx_nor_flash_sector_mapping_cache_enabled;
    LX_NOR_SECTOR_MAPPING_CACHE_ENTRY   
                                    lx_nor_flash_sector_mapping_cache[LX_NOR_SECTOR_MAPPING_CACHE_SIZE];

#ifndef LX_NOR_DISABLE_EXTENDED_CACHE

    UINT                            lx_nor_flash_extended_cache_entries;
    LX_NOR_FLASH_EXTENDED_CACHE_ENTRY
                                    lx_nor_flash_extended_cache[LX_NOR_EXTENDED_CACHE_SIZE];
    ULONG                           lx_nor_flash_extended_cache_hits;
    ULONG                           lx_nor_flash_extended_cache_misses;
#endif

#ifdef LX_THREAD_SAFE_ENABLE

    /* When this conditional is used, the LevelX code utilizes a ThreadX mutex for thread
       safe operation. Generally, this is not required since FileX ensures thread safe operation at
       a higher layer.  */
    TX_MUTEX                        lx_nor_flash_mutex;
#endif
    
    /* Define the NOR flash control block open next/previous pointers.  */
    struct LX_NOR_FLASH_STRUCT      *lx_nor_flash_open_next,
                                    *lx_nor_flash_open_previous;
    
} LX_NOR_FLASH;


/* Each physical NOR block has the following structure at the beginning of the block:

    Offset              Meaning
    
    0           Erase count
    4           Minimum logical sector number - only when the block is full
    8           Maximum logical sector number - only when the block is full
    12          Free physical sector bit map, where a value of 1 indicates a 
                free physical sector. The bit map is evenly divisible by 4
    .           Array of physical sector mapping information (4 bytes per entry, 
                one entry for each physical sector in block). Each entry looks
                like the following:
                
                        Bit(s)                 Meaning
                        
                        0-29                Logical sector mapped if not 0x3FFFFFFF
                        30                  If 0, entry is being superceded
                        31                  If 1, entry is valid
                          
             Array of physical sectors, with each of size LX_NOR_SECTOR_SIZE
*/    


typedef struct LX_NOR_FLASH_BLOCK_HEADER_STRUCT
{
    ULONG                           lx_nor_flash_block_erase_count;              /* Bit 31: 1 -> not used, 0 -> used      */
    ULONG                           lx_nor_flash_block_min_logical_sector;       /* On full block, minimum valid sector   */
    ULONG                           lx_nor_flash_block_max_logical_sector;       /* On full block, maximum valid sector   */
} LX_NOR_FLASH_BLOCK_HEADER;


/* Define external structure references.   */

extern LX_NAND_FLASH                                    *_lx_nand_flash_opened_ptr;
extern ULONG                                            _lx_nand_flash_opened_count;
extern LX_NOR_FLASH                                     *_lx_nor_flash_opened_ptr;
extern ULONG                                            _lx_nor_flash_opened_count;


/* Map internal functions.  */

#ifndef LX_SOURCE_CODE
#define lx_nand_flash_close                             _lx_nand_flash_close
#define lx_nand_flash_defragment                        _lx_nand_flash_defragment
#define lx_nand_flash_partial_defragment                _lx_nand_flash_partial_defragment
#define lx_nand_flash_extended_cache_enable             _lx_nand_flash_extended_cache_enable
#define lx_nand_flash_initialize                        _lx_nand_flash_initialize
#define lx_nand_flash_open                              _lx_nand_flash_open
#define lx_nand_flash_page_ecc_check                    _lx_nand_flash_page_ecc_check
#define lx_nand_flash_page_ecc_compute                  _lx_nand_flash_page_ecc_compute
#define lx_nand_flash_sector_read                       _lx_nand_flash_sector_read
#define lx_nand_flash_sector_release                    _lx_nand_flash_sector_release
#define lx_nand_flash_sector_write                      _lx_nand_flash_sector_write
#define lx_nand_flash_256byte_ecc_check                 _lx_nand_flash_256byte_ecc_check
#define lx_nand_flash_256byte_ecc_compute               _lx_nand_flash_256byte_ecc_compute

#define lx_nor_flash_close                              _lx_nor_flash_close
#define lx_nor_flash_defragment                         _lx_nor_flash_defragment
#define lx_nor_flash_partial_defragment                 _lx_nor_flash_partial_defragment
#define lx_nor_flash_extended_cache_enable              _lx_nor_flash_extended_cache_enable
#define lx_nor_flash_initialize                         _lx_nor_flash_initialize
#define lx_nor_flash_open                               _lx_nor_flash_open
#define lx_nor_flash_sector_read                        _lx_nor_flash_sector_read
#define lx_nor_flash_sector_release                     _lx_nor_flash_sector_release
#define lx_nor_flash_sector_write                       _lx_nor_flash_sector_write
#endif


/* External LevelX API prototypes.  */

UINT    _lx_nand_flash_close(LX_NAND_FLASH *nand_flash);
UINT    _lx_nand_flash_defragment(LX_NAND_FLASH *nand_flash);
UINT    _lx_nand_flash_initialize(void);
UINT    _lx_nand_flash_extended_cache_enable(LX_NAND_FLASH  *nand_flash, VOID *memory, ULONG size);
UINT    _lx_nand_flash_open(LX_NAND_FLASH  *nand_flash, CHAR *name, UINT (*nand_driver_initialize)(LX_NAND_FLASH *));
UINT    _lx_nand_flash_page_ecc_check(LX_NAND_FLASH *nand_flash, UCHAR *page_buffer, UCHAR *ecc_buffer);
UINT    _lx_nand_flash_page_ecc_compute(LX_NAND_FLASH *nand_flash, UCHAR *page_buffer, UCHAR *ecc_buffer);
UINT    _lx_nand_flash_partial_defragment(LX_NAND_FLASH *nand_flash, UINT max_blocks);
UINT    _lx_nand_flash_sector_read(LX_NAND_FLASH *nand_flash, ULONG logical_sector, VOID *buffer);
UINT    _lx_nand_flash_sector_release(LX_NAND_FLASH *nand_flash, ULONG logical_sector);
UINT    _lx_nand_flash_sector_write(LX_NAND_FLASH *nand_flash, ULONG logical_sector, VOID *buffer);

UINT    _lx_nor_flash_close(LX_NOR_FLASH *nor_flash);
UINT    _lx_nor_flash_defragment(LX_NOR_FLASH *nor_flash);
UINT    _lx_nor_flash_extended_cache_enable(LX_NOR_FLASH *nor_flash, VOID *memory, ULONG size);
UINT    _lx_nor_flash_initialize(void);
UINT    _lx_nor_flash_open(LX_NOR_FLASH  *nor_flash, CHAR *name, UINT (*nor_driver_initialize)(LX_NOR_FLASH *));
UINT    _lx_nor_flash_partial_defragment(LX_NOR_FLASH *nor_flash, UINT max_blocks);
UINT    _lx_nor_flash_sector_read(LX_NOR_FLASH *nor_flash, ULONG logical_sector, VOID *buffer);
UINT    _lx_nor_flash_sector_release(LX_NOR_FLASH *nor_flash, ULONG logical_sector);
UINT    _lx_nor_flash_sector_write(LX_NOR_FLASH *nor_flash, ULONG logical_sector, VOID *buffer);


/* Internal LevelX prototypes.  */

UINT    _lx_nand_flash_block_full_update(LX_NAND_FLASH *nand_flash, ULONG block, ULONG erase_count);
VOID    _lx_nand_flash_block_obsoleted_check(LX_NAND_FLASH *nand_flash, ULONG block);
UINT    _lx_nand_flash_block_reclaim(LX_NAND_FLASH *nand_flash);

UINT    _lx_nand_flash_driver_read(LX_NAND_FLASH *nand_flash, ULONG block, ULONG page, ULONG *destination, ULONG words);
UINT    _lx_nand_flash_driver_write(LX_NAND_FLASH *nand_flash, ULONG block, ULONG page, ULONG *source, ULONG words);
UINT    _lx_nand_flash_driver_block_erase(LX_NAND_FLASH *nand_flash, ULONG block, ULONG erase_count);
UINT    _lx_nand_flash_driver_block_erased_verify(LX_NAND_FLASH *nand_flash, ULONG block);
UINT    _lx_nand_flash_driver_page_erased_verify(LX_NAND_FLASH *nand_flash, ULONG block, ULONG page);
UINT    _lx_nand_flash_driver_block_status_get(LX_NAND_FLASH *nand_flash, ULONG block, UCHAR *bad_block_flag);
UINT    _lx_nand_flash_driver_block_status_set(LX_NAND_FLASH *nand_flash, ULONG block, UCHAR bad_block_flag);
UINT    _lx_nand_flash_driver_extra_bytes_get(LX_NAND_FLASH *nand_flash, ULONG block, ULONG page, UCHAR *destination, UINT size);
UINT    _lx_nand_flash_driver_extra_bytes_set(LX_NAND_FLASH *nand_flash, ULONG block, ULONG page, UCHAR *source, UINT size);

VOID    _lx_nand_flash_internal_error(LX_NAND_FLASH *nand_flash, ULONG error_code);
UINT    _lx_nand_flash_logical_sector_find(LX_NAND_FLASH *nand_flash, ULONG logical_sector, ULONG superceded_check, ULONG *block, ULONG *page);
UINT    _lx_nand_flash_next_block_to_erase_find(LX_NAND_FLASH *nand_flash, ULONG *return_erase_block, ULONG *return_erase_count, ULONG *return_mapped_pages, ULONG *return_obsolete_pages);
UINT    _lx_nand_flash_physical_page_allocate(LX_NAND_FLASH *nand_flash, ULONG *block, ULONG *page, ULONG *erase_count);
VOID    _lx_nand_flash_sector_mapping_cache_invalidate(LX_NAND_FLASH *nand_flash, ULONG logical_sector);
VOID    _lx_nand_flash_system_error(LX_NAND_FLASH *nand_flash, UINT error_code, ULONG block, ULONG page);
UINT    _lx_nand_flash_256byte_ecc_check(UCHAR *page_buffer, UCHAR *ecc_buffer);
UINT    _lx_nand_flash_256byte_ecc_compute(UCHAR *page_buffer, UCHAR *ecc_buffer);

UINT    _lx_nor_flash_block_reclaim(LX_NOR_FLASH *nor_flash);
UINT    _lx_nor_flash_driver_block_erase(LX_NOR_FLASH *nor_flash, ULONG block, ULONG erase_count);
UINT    _lx_nor_flash_driver_read(LX_NOR_FLASH *nor_flash, ULONG *flash_address, ULONG *destination, ULONG words);
UINT    _lx_nor_flash_driver_write(LX_NOR_FLASH *nor_flash, ULONG *flash_address, ULONG *source, ULONG words);
VOID    _lx_nor_flash_internal_error(LX_NOR_FLASH *nor_flash, ULONG error_code);
UINT    _lx_nor_flash_logical_sector_find(LX_NOR_FLASH *nor_flash, ULONG logical_sector, ULONG superceded_check, ULONG **physical_sector_map_entry, ULONG **physical_sector_address);
UINT    _lx_nor_flash_next_block_to_erase_find(LX_NOR_FLASH *nor_flash, ULONG *return_erase_block, ULONG *return_erase_count, ULONG *return_mapped_sectors, ULONG *return_obsolete_sectors);
UINT    _lx_nor_flash_physical_sector_allocate(LX_NOR_FLASH *nor_flash, ULONG logical_sector, ULONG **physical_sector_map_entry, ULONG **physical_sector_address);
VOID    _lx_nor_flash_sector_mapping_cache_invalidate(LX_NOR_FLASH *nor_flash, ULONG logical_sector);
VOID    _lx_nor_flash_system_error(LX_NOR_FLASH *nor_flash, UINT error_code);


#ifdef __cplusplus
}
#endif

#endif 

