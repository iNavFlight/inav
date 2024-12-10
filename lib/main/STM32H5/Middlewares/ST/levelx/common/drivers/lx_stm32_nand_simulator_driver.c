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
/**   NAND Flash Simulator                                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary files.  */

#include "lx_stm32_nand_simulator_driver.h"


/* Define constants for the NAND flash simulation. */



/* Definition of the spare area is relative to the page size of the NAND part and perhaps manufactures of the NAND part. 
   Here are some common definitions:
   
   256 Byte page
   
        Bytes               Meaning
        
        0,1,2           ECC bytes
        3,4,6,7         Extra
        5               Bad block flag
        
    512 Byte page
    
        Bytes               Meaning
        
        0,1,2,3,6,7     ECC bytes
        8-15            Extra
        5               Bad block flag
    
    2048 Byte page
    
        Bytes               Meaning
        
        0               Bad block flag
        2-39            Extra
        40-63           ECC bytes
*/


typedef struct PHYSICAL_PAGE_STRUCT
{
    unsigned long memory[WORDS_PER_PHYSICAL_PAGE];
    unsigned char spare[SPARE_BYTES_PER_PAGE];
} PHYSICAL_PAGE;

typedef struct NAND_BLOCK_DIAG_STRUCT
{
    unsigned long erases;
    unsigned long page_writes[PHYSICAL_PAGES_PER_BLOCK];
    unsigned long max_page_writes[PHYSICAL_PAGES_PER_BLOCK];
} NAND_BLOCK_DIAG;




typedef struct NAND_FLASH_BLOCK_STRUCT
{
    PHYSICAL_PAGE       physical_pages[PHYSICAL_PAGES_PER_BLOCK];
} NAND_FLASH_BLOCK;

NAND_FLASH_BLOCK   nand_memory_area[TOTAL_BLOCKS];

NAND_BLOCK_DIAG    nand_block_diag[TOTAL_BLOCKS];


/* Define NAND flash buffer for LevelX.  */

ULONG  nand_flash_simulator_buffer[WORDS_PER_PHYSICAL_PAGE];
ULONG  *nand_flash_memory;


UINT  lx_nand_simulator_initialize(LX_NAND_FLASH *nand_flash);
UINT  lx_nand_simulator_read(ULONG block, ULONG page, ULONG *destination, ULONG words);
UINT  lx_nand_simulator_write(ULONG block, ULONG page, ULONG *source, ULONG words);
UINT  lx_nand_simulator_block_erase(ULONG block, ULONG erase_count);
UINT  lx_nand_simulator_block_erased_verify(ULONG block);
UINT  lx_nand_simulator_page_erased_verify(ULONG block, ULONG page);
UINT  lx_nand_simulator_erase_all(VOID);
UINT  lx_nand_simulator_block_status_get(ULONG block, UCHAR *bad_block_byte);
UINT  lx_nand_simulator_block_status_set(ULONG block, UCHAR bad_block_byte);
UINT  lx_nand_simulator_extra_bytes_get(ULONG block, ULONG page, UCHAR *destination, UINT size);
UINT  lx_nand_simulator_extra_bytes_set(ULONG block, ULONG page, UCHAR *source, UINT size);
UINT  lx_nand_simulator_system_error(UINT error_code, ULONG block, ULONG page);



UINT  lx_stm32_nand_simulator_initialize(LX_NAND_FLASH *nand_flash)
{

    /* Setup the buffer pointer.  */
    nand_flash_memory = (ULONG *) &nand_memory_area[0];

    /* Setup geometry of the NAND flash.  */
    nand_flash -> lx_nand_flash_total_blocks =                  TOTAL_BLOCKS;
    nand_flash -> lx_nand_flash_pages_per_block =               PHYSICAL_PAGES_PER_BLOCK;
    nand_flash -> lx_nand_flash_bytes_per_page =                BYTES_PER_PHYSICAL_PAGE;

    /* Setup function pointers for the NAND flash services.  */
    nand_flash -> lx_nand_flash_driver_read =                   lx_nand_simulator_read;
    nand_flash -> lx_nand_flash_driver_write =                  lx_nand_simulator_write;
    nand_flash -> lx_nand_flash_driver_block_erase =            lx_nand_simulator_block_erase;
    nand_flash -> lx_nand_flash_driver_block_erased_verify =    lx_nand_simulator_block_erased_verify;
    nand_flash -> lx_nand_flash_driver_page_erased_verify =     lx_nand_simulator_page_erased_verify;
    nand_flash -> lx_nand_flash_driver_block_status_get =       lx_nand_simulator_block_status_get;
    nand_flash -> lx_nand_flash_driver_block_status_set =       lx_nand_simulator_block_status_set;
    nand_flash -> lx_nand_flash_driver_extra_bytes_get =        lx_nand_simulator_extra_bytes_get;
    nand_flash -> lx_nand_flash_driver_extra_bytes_set =        lx_nand_simulator_extra_bytes_set;
    nand_flash -> lx_nand_flash_driver_system_error =           lx_nand_simulator_system_error;

    /* Setup local buffer for NAND flash operation. This buffer must be the page size of the NAND flash memory.  */
    nand_flash -> lx_nand_flash_page_buffer =  &nand_flash_simulator_buffer[0];

    /* Return success.  */
    return(LX_SUCCESS);
}


UINT  lx_nand_simulator_read(ULONG block, ULONG page, ULONG *destination, ULONG words)
{

ULONG   *flash_address;


    /* Pickup the flash address.  */
    flash_address =  &(nand_memory_area[block].physical_pages[page].memory[0]);

    /* Loop to read flash.  */
    while (words--)
    {
        /* Copy word.  */
        *destination++ =  *flash_address++;
    }

    return(LX_SUCCESS);
}


UINT  lx_nand_simulator_write(ULONG block, ULONG page, ULONG *source, ULONG words)
{

ULONG   *flash_address;

  
    /* Increment the diag info.  */
    nand_block_diag[block].page_writes[page]++;
    if (nand_block_diag[block].page_writes[page] > nand_block_diag[block].max_page_writes[page])
        nand_block_diag[block].max_page_writes[page] =  nand_block_diag[block].page_writes[page];
  
    /* Pickup the flash address.  */
    flash_address =  &(nand_memory_area[block].physical_pages[page].memory[0]);

    /* Loop to write flash.  */
    while (words--)
    {

        /* Can the word be written?  We can clear new bits, but just can't unclear
           in a NAND device.  */
        if ((*source & *flash_address) != *source)
           return(LX_INVALID_WRITE);
      
        /* Copy word.  */
        *flash_address++ =  *source++;
    }

    return(LX_SUCCESS);
}

UINT  lx_nand_simulator_block_erase(ULONG block, ULONG erase_count)
{

ULONG   *pointer;
ULONG   words;
UINT    i;

    LX_PARAMETER_NOT_USED(erase_count);

    /* Increment the diag info.  */
    nand_block_diag[block].erases++;
    for (i = 0; i < PHYSICAL_PAGES_PER_BLOCK;i++)
        nand_block_diag[block].page_writes[i] = 0;

    /* Setup pointer.  */
    pointer =  (ULONG *) &nand_memory_area[block];

    /* Loop to erase block.  */
    words =  sizeof(NAND_FLASH_BLOCK)/sizeof(ULONG);
    while (words--)
    {
        
        /* Erase word of block.  */
        *pointer++ =  (ULONG) 0xFFFFFFFF;
    }

    return(LX_SUCCESS);
}


UINT  lx_nand_simulator_erase_all(VOID)
{

ULONG   *pointer;
ULONG   words;
UINT    i, j;


    /* Increment the diag info.  */
    for (i = 0; i < TOTAL_BLOCKS; i++)
    {
    nand_block_diag[i].erases =  0;
    for (j = 0; j < PHYSICAL_PAGES_PER_BLOCK;j++)
        nand_block_diag[i].page_writes[j] = 0;
    }
    
    /* Setup pointer.  */
    pointer =  (ULONG *) &nand_memory_area[0];

    /* Loop to erase block.  */
    words =  (ULONG)(sizeof(nand_memory_area))/sizeof(ULONG);
    while (words--)
    {
        
        /* Erase word of block.  */
        *pointer++ =  (ULONG) 0xFFFFFFFF;
    }

    return(LX_SUCCESS);
}


UINT  lx_nand_simulator_block_erased_verify(ULONG block)
{

ULONG   *word_ptr;
ULONG   words;

    /* Determine if the block is completely erased.  */
    
    /* Pickup the pointer to the first word of the block.  */
    word_ptr =  (ULONG *) &nand_memory_area[block];
    
    /* Calculate the number of words in a block.  */
    words =  sizeof(NAND_FLASH_BLOCK)/sizeof(ULONG);
    
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


UINT  lx_nand_simulator_page_erased_verify(ULONG block, ULONG page)
{

ULONG   *word_ptr;
ULONG   words;

    /* Determine if the block is completely erased.  */
    
    /* Pickup the pointer to the first word of the block's page.  */
    word_ptr =  (ULONG *) &nand_memory_area[block].physical_pages[page];
    
    /* Calculate the number of words in a block.  */
    words =  WORDS_PER_PHYSICAL_PAGE;
    
    /* Loop to check if the page is erased.  */
    while (words--)
    {
    
        /* Is this word erased?  */
        if (*word_ptr++ != 0xFFFFFFFF)
            return(LX_ERROR);
    }
    
    /* Return success.  */
    return(LX_SUCCESS);
}


UINT  lx_nand_simulator_block_status_get(ULONG block, UCHAR *bad_block_byte)
{

    /* Pickup the bad block byte and return it.  */
    *bad_block_byte =  nand_memory_area[block].physical_pages[0].spare[BAD_BLOCK_POSITION];
    
    /* Return success.  */
    return(LX_SUCCESS);
}

UINT  lx_nand_simulator_block_status_set(ULONG block, UCHAR bad_block_byte)
{

    /* Set the bad block byte.  */
    nand_memory_area[block].physical_pages[0].spare[BAD_BLOCK_POSITION] =  bad_block_byte;
    
    /* Return success.  */
    return(LX_SUCCESS);
}



UINT  lx_nand_simulator_extra_bytes_get(ULONG block, ULONG page, UCHAR *destination, UINT size)
{

UCHAR   *source;

    
    /* Setup source pointer in the spare area.  */
    source =  (UCHAR *) &(nand_memory_area[block].physical_pages[page].spare[EXTRA_BYTE_POSITION]);
    
    /* Loop to return the extra bytes requested.  */
    while (size--)
    {
    
        /* Retrieve an extra byte from the spare area.  */
        *destination++ =  *source++;
    }

    /* Return success.  */
    return(LX_SUCCESS);
}


UINT  lx_nand_simulator_extra_bytes_set(ULONG block, ULONG page, UCHAR *source, UINT size)
{

UCHAR   *destination;

    
    /* Increment the diag info.  */
    nand_block_diag[block].page_writes[page]++;
    if (nand_block_diag[block].page_writes[page] > nand_block_diag[block].max_page_writes[page])
        nand_block_diag[block].max_page_writes[page] =  nand_block_diag[block].page_writes[page];
    
    /* Setup destination pointer in the spare area.  */
    destination =  (UCHAR *) &(nand_memory_area[block].physical_pages[page].spare[EXTRA_BYTE_POSITION]);
    
    /* Loop to set the extra bytes.  */
    while (size--)
    {
    
        /* Set an extra byte in the spare area.  */
        *destination++ =  *source++;
    }

    /* Return success.  */
    return(LX_SUCCESS);
}

UINT  lx_nand_simulator_system_error(UINT error_code, ULONG block, ULONG page)
{
    LX_PARAMETER_NOT_USED(error_code);
    LX_PARAMETER_NOT_USED(block);
    LX_PARAMETER_NOT_USED(page);

    /* Custom processing goes here...  all errors except for LX_NAND_ERROR_CORRECTED are fatal.  */
    return(LX_ERROR);
}



