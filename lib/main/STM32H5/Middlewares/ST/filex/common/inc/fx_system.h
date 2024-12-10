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
/**   System                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    fx_system.h                                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the FileX system constants and global data        */
/*    definitions, including external references.  It is assumed that     */
/*    fx_api.h (and fx_port.h) have already been included.                */
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

#ifndef FX_SYSTEM_H
#define FX_SYSTEM_H


/* Define System component constants.  */

#define FX_TIMER_ID                 ((ULONG) 0x46585359)


/* Define the external System component function prototypes.  */

VOID _fx_system_initialize(VOID);
UINT _fx_system_date_set(UINT year, UINT month, UINT day);
UINT _fx_system_time_set(UINT hour, UINT minute, UINT second);
UINT _fx_system_date_get(UINT *year, UINT *month, UINT *day);
UINT _fx_system_time_get(UINT *hour, UINT *minute, UINT *second);
VOID _fx_system_timer_entry(ULONG id);

UINT _fxe_system_date_set(UINT year, UINT month, UINT day);
UINT _fxe_system_time_set(UINT hour, UINT minute, UINT second);
UINT _fxe_system_date_get(UINT *year, UINT *month, UINT *day);
UINT _fxe_system_time_get(UINT *hour, UINT *minute, UINT *second);


/* System Component data declarations follow.  */

/* Determine if the initialization function of this component is including
   this file.  If so, make the data definitions really happen.  Otherwise,
   make them extern so other functions in the component can access them.  */

#ifdef FX_SYSTEM_INIT
#define SYSTEM_DECLARE
#else
#define SYSTEM_DECLARE extern
#endif


/* Define the head pointer of the opened media list.  */

SYSTEM_DECLARE  FX_MEDIA  *_fx_system_media_opened_ptr;


/* Define the variable that holds the number of open media. */

SYSTEM_DECLARE  ULONG  _fx_system_media_opened_count;


/* Define the system date variable.  */

SYSTEM_DECLARE  UINT  _fx_system_date;


/* Define the system time variable.  */

SYSTEM_DECLARE  UINT  _fx_system_time;


/* Define the variable that holds the maximum size of the sector cache.  */

SYSTEM_DECLARE  ULONG  _fx_system_media_max_sector_cache;


/* Define the variable that holds the maximum size of the FAT cache.  */

SYSTEM_DECLARE  ULONG  _fx_system_media_max_fat_cache;


/* Define the global FileX build options variables. These variables contain a bit
   map representing how the FileX library was built. The following are the bit
   field definitions:

    _fx_system_build_options_1:

                    Bit(s)                   Meaning

                    31-24               FX_MAX_LONG_NAME_LEN
                    23-16               FX_MAX_LAST_NAME_LEN
                    15-11               Reserved
                    10                  FX_NO_TIMER defined
                    9                   FX_SINGLE_THREAD defined
                    8                   FX_DONT_UPDATE_OPEN_FILES defined
                    7                   FX_MEDIA_DISABLE_SEARCH_CACHE defined
                    6                   FX_MEDIA_STATISTICS_DISABLE defined
                    5                   Reserved
                    4                   FX_SINGLE_OPEN_LEGACY defined
                    3                   FX_RENAME_PATH_INHERIT defined
                    2                   FX_NO_LOCAL_PATH defined
                    1                   FX_FAULT_TOLERANT_DATA defined
                    0                   FX_FAULT_TOLERANT defined

    _fx_system_build_options_2:

                    Bit(s)                   Meaning

                    31-16               FX_MAX_SECTOR_CACHE
                    15-8                FX_FAT_MAP_SIZE
                    7-0                 FX_MAX_FAT_CACHE

    _fx_system_build_options_3:

                    Bit(s)                   Meaning

                    31-24               Reserved
                    23-16               FX_UPDATE_RATE_IN_SECONDS
                    15-0                FX_UPDATE_RATE_IN_TICKS

   Note that values greater than the value that can be represented in the build options
   bit field are represented as all ones in the bit field. For example, if FX_MAX_LONG_NAME_LEN
   is 256, the value in the bits 31-24 of _fx_system_build_options_1 is 0xFF, which is 255
   decimal.  */

SYSTEM_DECLARE  ULONG _fx_system_build_options_1;
SYSTEM_DECLARE  ULONG _fx_system_build_options_2;
SYSTEM_DECLARE  ULONG _fx_system_build_options_3;


/* Define system timer control block.  If accurate date/time stamps on
   files is not needed, the define FX_NO_TIMER should be used when
   compiling fx_system_initialize.c to eliminate the FileX timer.  */

#ifndef FX_NO_TIMER
SYSTEM_DECLARE  TX_TIMER _fx_system_timer;
#endif

#endif

