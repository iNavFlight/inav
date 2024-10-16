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

#define FX_SOURCE_CODE


/* Locate FileX control component data in this file.  */

#define FX_SYSTEM_INIT


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_system.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_system_initialize                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the various control data structures for   */
/*    the FileX System component.                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_timer_create                       Create system timer           */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Initialization                                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s), and      */
/*                                            added conditional to        */
/*                                            disable build options,      */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _fx_system_initialize(VOID)
{

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_SYSTEM_INITIALIZE, 0, 0, 0, 0, FX_TRACE_INTERNAL_EVENTS, 0, 0)

    /* Initialize the head pointer of the opened media list and the
       number of opened media.  */
    _fx_system_media_opened_ptr =       FX_NULL;
    _fx_system_media_opened_count =     0;

    /* Initialize the time and date fields with their default values.  */
    _fx_system_date =   FX_INITIAL_DATE;
    _fx_system_time =   FX_INITIAL_TIME;

    /* Initialize the sector and FAT cache sizes.  */
    _fx_system_media_max_sector_cache =  FX_MAX_SECTOR_CACHE;
    _fx_system_media_max_fat_cache =     FX_MAX_FAT_CACHE;

    /* Create the FileX system timer.  This is responsible for updating
       the specified date and time at the rate specified by
       FX_UPDATE_RATE_IN_TICKS.  Note that the timer is not necessary for
       regular FileX operation - it is only needed for accurate system
       date and time stamps on files.  */

#ifndef FX_NO_TIMER
    tx_timer_create(&_fx_system_timer, "FileX System Timer", _fx_system_timer_entry, FX_TIMER_ID,
                    FX_UPDATE_RATE_IN_TICKS, FX_UPDATE_RATE_IN_TICKS, TX_AUTO_ACTIVATE);
#endif

#ifndef FX_DISABLE_BUILD_OPTIONS
    /* Setup the build options variables.  */

    /* Setup the first build options variable.  */
    if (FX_MAX_LONG_NAME_LEN > 0xFF)
    {
        _fx_system_build_options_1 =  _fx_system_build_options_1 | (((ULONG)0xFF) << 24);
    }
    else
    {
        _fx_system_build_options_1 =  _fx_system_build_options_1 | (((ULONG)(FX_MAX_LONG_NAME_LEN & 0xFF)) << 24);
    }
    if (FX_MAX_LAST_NAME_LEN > 0xFF)
    {
        _fx_system_build_options_1 =  _fx_system_build_options_1 | (((ULONG)0xFF) << 16);
    }
    else
    {
        _fx_system_build_options_1 =  _fx_system_build_options_1 | (((ULONG)(FX_MAX_LAST_NAME_LEN & 0xFF)) << 24);
    }

#ifdef FX_NO_TIMER
    _fx_system_build_options_1 = _fx_system_build_options_1 | (((ULONG)1) << 10);
#endif
#ifdef FX_SINGLE_THREAD
    _fx_system_build_options_1 = _fx_system_build_options_1 | (((ULONG)1) << 9);
#endif
#ifdef FX_DONT_UPDATE_OPEN_FILES
    _fx_system_build_options_1 = _fx_system_build_options_1 | (((ULONG)1) << 8);
#endif
#ifdef FX_MEDIA_DISABLE_SEARCH_CACHE
    _fx_system_build_options_1 = _fx_system_build_options_1 | (((ULONG)1) << 7);
#endif
#ifdef FX_MEDIA_STATISTICS_DISABLE
    _fx_system_build_options_1 = _fx_system_build_options_1 | (((ULONG)1) << 6);
#endif

#ifdef FX_SINGLE_OPEN_LEGACY
    _fx_system_build_options_1 = _fx_system_build_options_1 | (((ULONG)1) << 4);
#endif
#ifdef FX_RENAME_PATH_INHERIT
    _fx_system_build_options_1 = _fx_system_build_options_1 | (((ULONG)1) << 3);
#endif
#ifdef FX_NO_LOCAL_PATH
    _fx_system_build_options_1 = _fx_system_build_options_1 | (((ULONG)1) << 2);
#endif
#ifdef FX_FAULT_TOLERANT_DATA
    _fx_system_build_options_1 = _fx_system_build_options_1 | (((ULONG)1) << 1);
#endif
#ifdef FX_FAULT_TOLERANT
    _fx_system_build_options_1 = _fx_system_build_options_1 | ((ULONG)1);
#endif

    /* Setup the second build options variable.  */
    if (FX_MAX_SECTOR_CACHE > ((ULONG)0xFFFF))
    {
        _fx_system_build_options_2 =  _fx_system_build_options_2 | (((ULONG)0xFFFF) << 16);
    }
    else
    {
        _fx_system_build_options_2 =  _fx_system_build_options_2 | (((ULONG)FX_MAX_SECTOR_CACHE) << 16);
    }
    if (FX_FAT_MAP_SIZE > 0xFF)
    {
        _fx_system_build_options_2 =  _fx_system_build_options_2 | (((ULONG)0xFF) << 8);
    }
    else
    {
        _fx_system_build_options_2 =  _fx_system_build_options_2 | (((ULONG)FX_FAT_MAP_SIZE) << 8);
    }
    if (FX_MAX_FAT_CACHE > 0xFF)
    {
        _fx_system_build_options_2 =  _fx_system_build_options_2 | ((ULONG)0xFF);
    }
    else
    {
        _fx_system_build_options_2 =  _fx_system_build_options_2 | ((ULONG)FX_MAX_FAT_CACHE);
    }

    /* Setup the third build options variable.  */
    if (FX_UPDATE_RATE_IN_SECONDS > 0xFF)
    {
        _fx_system_build_options_3 =  _fx_system_build_options_3 | (((ULONG)0xFF) << 16);
    }
    else
    {
        _fx_system_build_options_3 =  _fx_system_build_options_3 | (((ULONG)FX_UPDATE_RATE_IN_SECONDS) << 16);
    }
    if (FX_UPDATE_RATE_IN_TICKS > ((ULONG)0xFFFF))
    {
        _fx_system_build_options_3 =  _fx_system_build_options_3 | ((ULONG)0xFFFF);
    }
    else
    {
        _fx_system_build_options_3 =  _fx_system_build_options_3 | ((ULONG)FX_UPDATE_RATE_IN_TICKS);
    }
#endif /* FX_DISABLE_BUILD_OPTIONS */
}

