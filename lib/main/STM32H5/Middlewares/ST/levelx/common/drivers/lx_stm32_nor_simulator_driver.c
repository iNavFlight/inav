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

/* Include necessary files.  */
#include "lx_stm32_nor_simulator_driver.h"

static UINT  lx_nor_simulator_read(ULONG *flash_address, ULONG *destination, ULONG words);
static UINT  lx_nor_simulator_write(ULONG *flash_address, ULONG *source, ULONG words);

static UINT  lx_nor_simulator_block_erase(ULONG block, ULONG erase_count);
static UINT  lx_nor_simulator_block_erased_verify(ULONG block);

#ifndef LX_DIRECT_READ
static ULONG  nor_sector_memory[LX_NOR_SIMULATOR_SECTOR_SIZE];
#endif

static ULONG words_per_block = (LX_NOR_SIMULATOR_SECTOR_SIZE * LX_NOR_SIMULATOR_SECTORS_PER_BLOCK) / sizeof(ULONG);

static UINT is_erased = 0;

static void mem_set(void* s, int c, size_t sz)
{
    UCHAR *p = (UCHAR*)s;
    UCHAR x = c & 0xff;

    while (sz--)
        *p++ = x;
}

UINT  lx_stm32_nor_simulator_initialize(LX_NOR_FLASH *nor_flash)
{
    /* Setup the base address of the flash memory.  */
    nor_flash->lx_nor_flash_base_address = (ULONG *) LX_NOR_SIMULATOR_FLASH_BASE_ADDRESS;

    if (is_erased == 0)
    {
        mem_set(nor_flash->lx_nor_flash_base_address, 0xff, LX_NOR_SIMULATOR_FLASH_SIZE);
        is_erased = 1;
    }
    /* Setup geometry of the flash.  */
    nor_flash->lx_nor_flash_total_blocks = (LX_NOR_SIMULATOR_FLASH_SIZE / (LX_NOR_SIMULATOR_SECTOR_SIZE * LX_NOR_SIMULATOR_SECTORS_PER_BLOCK));
    nor_flash->lx_nor_flash_words_per_block = words_per_block;

    /* Setup function pointers for the NOR flash services.  */
    nor_flash->lx_nor_flash_driver_read = lx_nor_simulator_read;
    nor_flash->lx_nor_flash_driver_write = lx_nor_simulator_write;

    nor_flash->lx_nor_flash_driver_block_erase = lx_nor_simulator_block_erase;
    nor_flash->lx_nor_flash_driver_block_erased_verify = lx_nor_simulator_block_erased_verify;

#ifndef LX_DIRECT_READ
    /* Setup local buffer for NOR flash operation. This buffer must be the sector size of the NOR flash memory.  */
    nor_flash->lx_nor_flash_sector_buffer =  &nor_sector_memory[0];
#endif
    /* Return success.  */
    return(LX_SUCCESS);
}


static UINT  lx_nor_simulator_read(ULONG *flash_address, ULONG *destination, ULONG words)
{

    memcpy((VOID *)destination, (VOID *)flash_address, words * sizeof(ULONG));

    return(LX_SUCCESS);
}


static UINT  lx_nor_simulator_write(ULONG *flash_address, ULONG *source, ULONG words)
{

    memcpy((VOID *)flash_address, (VOID *)source, words * sizeof(ULONG));

    return(LX_SUCCESS);
}

static UINT  lx_nor_simulator_block_erase(ULONG block, ULONG erase_count)
{

    ULONG   *pointer;

    LX_PARAMETER_NOT_USED(erase_count);

    /* Setup pointer.  */

    pointer = (ULONG *) (LX_NOR_SIMULATOR_FLASH_BASE_ADDRESS + block * (LX_NOR_SIMULATOR_SECTOR_SIZE * LX_NOR_SIMULATOR_SECTORS_PER_BLOCK));

    /* Loop to erase block.  */

    mem_set((VOID *) pointer, 0xff, words_per_block * sizeof(ULONG));
    return(LX_SUCCESS);
}

static UINT  lx_nor_simulator_block_erased_verify(ULONG block)
{
    ULONG *word_ptr;
    ULONG words;

    /* Determine if the block is completely erased.  */

    word_ptr = (ULONG *) (LX_NOR_SIMULATOR_FLASH_BASE_ADDRESS + block * (LX_NOR_SIMULATOR_SECTOR_SIZE * LX_NOR_SIMULATOR_SECTORS_PER_BLOCK));

    /* Calculate the number of words in a block.  */
    words =  words_per_block;

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

