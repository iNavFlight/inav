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
/**   NOR Flash Simulator                                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary files.  */

#include "lx_api.h"

/* Define constants for the NOR flash simulation. */

/* This configuration is for one physical sector of overhead.  */


#define TOTAL_BLOCKS                        8
#define PHYSICAL_SECTORS_PER_BLOCK          16          /* Min value of 2, max value of 120 for 1 sector of overhead.  */
#define WORDS_PER_PHYSICAL_SECTOR           128
#define FREE_BIT_MAP_WORDS                  ((PHYSICAL_SECTORS_PER_BLOCK-1)/32)+1
#define USABLE_SECTORS_PER_BLOCK            (PHYSICAL_SECTORS_PER_BLOCK-1)
#define UNUSED_METADATA_WORDS_PER_BLOCK     (WORDS_PER_PHYSICAL_SECTOR-(3+FREE_BIT_MAP_WORDS+USABLE_SECTORS_PER_BLOCK))


typedef struct PHYSICAL_SECTOR_STRUCT
{
    unsigned long memory[WORDS_PER_PHYSICAL_SECTOR];
} PHYSICAL_SECTOR;


typedef struct FLASH_BLOCK_STRUCT
{
    unsigned long       erase_count;
    unsigned long       min_log_sector;
    unsigned long       max_log_sector;
    unsigned long       free_bit_map[FREE_BIT_MAP_WORDS];
    unsigned long       sector_metadata[USABLE_SECTORS_PER_BLOCK];
    unsigned long       unused_words[UNUSED_METADATA_WORDS_PER_BLOCK];
    PHYSICAL_SECTOR     physical_sectors[USABLE_SECTORS_PER_BLOCK];
} FLASH_BLOCK;

FLASH_BLOCK   nor_memory_area[TOTAL_BLOCKS];

ULONG         nor_sector_memory[WORDS_PER_PHYSICAL_SECTOR];

UINT  _lx_nor_flash_simulator_initialize(LX_NOR_FLASH *nor_flash);
UINT  _lx_nor_flash_simulator_read(ULONG *flash_address, ULONG *destination, ULONG words);
UINT  _lx_nor_flash_simulator_write(ULONG *flash_address, ULONG *source, ULONG words);
UINT  _lx_nor_flash_simulator_block_erase(ULONG block, ULONG erase_count);
UINT  _lx_nor_flash_simulator_block_erased_verify(ULONG block);
UINT  _lx_nor_flash_simulator_erase_all(VOID);
UINT  _lx_nor_flash_simulator_system_error(UINT error_code, ULONG block, ULONG sector);



UINT  _lx_nor_flash_simulator_initialize(LX_NOR_FLASH *nor_flash)
{

    /* Setup the base address of the flash memory.  */
    nor_flash -> lx_nor_flash_base_address =                (ULONG *) &nor_memory_area[0];

    /* Setup geometry of the flash.  */
    nor_flash -> lx_nor_flash_total_blocks =                TOTAL_BLOCKS;
    nor_flash -> lx_nor_flash_words_per_block =             sizeof(FLASH_BLOCK)/sizeof(ULONG);

    /* Setup function pointers for the NOR flash services.  */
    nor_flash -> lx_nor_flash_driver_read =                 _lx_nor_flash_simulator_read;
    nor_flash -> lx_nor_flash_driver_write =                _lx_nor_flash_simulator_write;
    nor_flash -> lx_nor_flash_driver_block_erase =          _lx_nor_flash_simulator_block_erase;
    nor_flash -> lx_nor_flash_driver_block_erased_verify =  _lx_nor_flash_simulator_block_erased_verify;

    /* Setup local buffer for NOR flash operation. This buffer must be the sector size of the NOR flash memory.  */
    nor_flash -> lx_nor_flash_sector_buffer =  &nor_sector_memory[0];

    /* Return success.  */
    return(LX_SUCCESS);
}


UINT  _lx_nor_flash_simulator_read(ULONG *flash_address, ULONG *destination, ULONG words)
{

    /* Loop to read flash.  */
    while (words--)
    {
        /* Copy word.  */
        *destination++ =  *flash_address++;
    }

    return(LX_SUCCESS);
}


UINT  _lx_nor_flash_simulator_write(ULONG *flash_address, ULONG *source, ULONG words)
{

    /* Loop to write flash.  */
    while (words--)
    {
     
        /* Copy word.  */
        *flash_address++ =  *source++;
    }

    return(LX_SUCCESS);
}

UINT  _lx_nor_flash_simulator_block_erase(ULONG block, ULONG erase_count)
{

ULONG   *pointer;
ULONG   words;

    LX_PARAMETER_NOT_USED(erase_count);

    /* Setup pointer.  */
    pointer =  (ULONG *) &nor_memory_area[block];

    /* Loop to erase block.  */
    words =  sizeof(FLASH_BLOCK)/sizeof(ULONG);
    while (words--)
    {
        
        /* Erase word of block.  */
        *pointer++ =  (ULONG) 0xFFFFFFFF;
    }

    return(LX_SUCCESS);
}


UINT  _lx_nor_flash_simulator_erase_all(VOID)
{

ULONG   *pointer;
ULONG   words;


    /* Setup pointer.  */
    pointer =  (ULONG *) &nor_memory_area[0];

    /* Loop to erase block.  */
    words =  sizeof(nor_memory_area)/(sizeof(ULONG));
    while (words--)
    {
        
        /* Erase word of block.  */
        *pointer++ =  (ULONG) 0xFFFFFFFF;
    }

    return(LX_SUCCESS);
}


UINT  _lx_nor_flash_simulator_block_erased_verify(ULONG block)
{

ULONG   *word_ptr;
ULONG   words;

    /* Determine if the block is completely erased.  */
    
    /* Pickup the pointer to the first word of the block.  */
    word_ptr =  (ULONG *) &nor_memory_area[block].erase_count;
    
    /* Calculate the number of words in a block.  */
    words =  sizeof(FLASH_BLOCK)/sizeof(ULONG);
    
    /* Loop to check if the block is erased.  */
    while (words--)
    {
    
        /* Is this word erased?  */
        if (*word_ptr++ != 0xFFFFFFFF)
            return(LX_ERROR);
    }
    
    /* Return success.  */
    return(LX_SUCCESS);
}

UINT  _lx_nor_flash_simulator_system_error(UINT error_code, ULONG block, ULONG sector)
{
    LX_PARAMETER_NOT_USED(error_code);
    LX_PARAMETER_NOT_USED(block);
    LX_PARAMETER_NOT_USED(sector);

    /* Custom processing goes here...  all errors are fatal.  */
    return(LX_ERROR);
}

