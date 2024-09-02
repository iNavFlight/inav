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
/**   User Specific                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */
/*                                                                        */
/*    lx_user.h                                           PORTABLE C      */
/*                                                           6.1.7        */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains user defines for configuring LevelX in specific  */
/*    ways. This file will have an effect only if the application and     */
/*    LevelX library are built with LX_INCLUDE_USER_DEFINE_FILE defined.  */
/*    Note that all the defines in this file may also be made on the      */
/*    command line when building LevelX library and application objects.  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  11-09-2020     William E. Lamie         Initial Version 6.1.2         */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s), and      */
/*                                            added standalone support,   */
/*                                            resulting in version 6.1.7  */
/*                                                                        */
/**************************************************************************/

#ifndef LX_USER_H
#define LX_USER_H



/* Defined, this option bypasses the NOR flash driver read routine in favor or reading 
   the NOR memory directly, resulting in a significant performance increase. 
*/
/*
#define LX_DIRECT_READ
*/


/* Defined, this causes the LevelX NOR instance open logic to verify free NOR 
   sectors are all ones.
*/
/*
#define LX_FREE_SECTOR_DATA_VERIFY 
*/

/* By default this value is 128 and defines the logical sector mapping cache size. 
   Large values improve performance, but cost memory. The minimum size is 8 and 
   all values must be a power of 2.
*/
/*
#define LX_NAND_SECTOR_MAPPING_CACHE_SIZE   128
*/

/* Defined, this creates a direct mapping cache, such that there are no cache misses. 
   It also requires that LX_NAND_SECTOR_MAPPING_CACHE_SIZE represents the exact number 
   of total pages in your flash device. 
*/
/* 
#define LX_NAND_FLASH_DIRECT_MAPPING_CACHE
*/


/* Defined, this disabled the extended NOR cache.  */
/*
#define LX_NOR_DISABLE_EXTENDED_CACHE
*/

/* By default this value is 8, which represents a maximum of 8 sectors that 
   can be cached in a NOR instance.
*/
/*
#define LX_NOR_EXTENDED_CACHE_SIZE   8 
*/


/* By default this value is 16 and defines the logical sector mapping cache size. 
   Large values improve performance, but cost memory. The minimum size is 8 and all 
   values must be a power of 2.
*/
/*
#define LX_NOR_SECTOR_MAPPING_CACHE_SIZE   16
*/

/* Defined, this makes LevelX thread-safe by using a ThreadX mutex object 
   throughout the API.
*/
/*
#define LX_THREAD_SAFE_ENABLE
*/

/* Defined, LevelX will be used in standalone mode (without Azure RTOS ThreadX) */

/* #define LX_STANDALONE_ENABLE */



#endif

