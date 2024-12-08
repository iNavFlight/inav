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
/**   Application Interface                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    fx_api.h                                            PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the basic Application Interface (API) to the      */
/*    high-performance FileX FAT compatible embedded file system.         */
/*    All service prototypes and data structure definitions are defined   */
/*    in this file.                                                       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            and added conditionals to   */
/*                                            disable few declarations    */
/*                                            for code size reduction,    */
/*                                            resulting in version 6.1    */
/*  11-09-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1.2  */
/*  12-31-2020     William E. Lamie         Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1.3  */
/*  03-02-2021     William E. Lamie         Modified comment(s), and      */
/*                                            added standalone support,   */
/*                                            resulting in version 6.1.5  */
/*  04-02-2021     William E. Lamie         Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1.6  */
/*  06-02-2021     William E. Lamie         Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1.7  */
/*  08-02-2021     William E. Lamie         Modified comment(s), and      */
/*                                            updated product constants,  */
/*                                            resulting in version 6.1.8  */
/*  01-31-2022     Bhupendra Naphade        Modified comment(s), and      */
/*                                            removed fixed sector        */
/*                                            size in exFAT, fixed        */
/*                                            errors without cache,       */
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

#ifndef FX_API_H
#define FX_API_H


/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif


/* Disable warning of parameter not used. */
#ifndef FX_PARAMETER_NOT_USED
#define FX_PARAMETER_NOT_USED(p) ((void)(p))
#endif /* FX_PARAMETER_NOT_USED */


/* Disable ThreadX error checking for internal FileX source code.  */

#ifdef FX_SOURCE_CODE
#ifndef TX_DISABLE_ERROR_CHECKING
#define TX_DISABLE_ERROR_CHECKING
#endif
#endif


/* Include the FileX port-specific file.  */

#include "fx_port.h"

/* Define compiler library include files */
 
#ifdef FX_STANDALONE_ENABLE
#include   "string.h"
#endif

/* Define the major/minor version information that can be used by the application
   and the FileX source as well.  */

#define AZURE_RTOS_FILEX
#define FILEX_MAJOR_VERSION     6
#define FILEX_MINOR_VERSION     2
#define FILEX_PATCH_VERSION     0

/* Define the following symbols for backward compatibility */
#define EL_PRODUCT_FILEX

#ifdef FX_STANDALONE_ENABLE

/* FileX will be used without Azure RTOS ThreadX */

#ifndef FX_SINGLE_THREAD
#define FX_SINGLE_THREAD
#endif /* !FX_SINGLE_THREAD */


/* FileX will be used with local path logic disabled */

#ifndef FX_NO_LOCAL_PATH
#define FX_NO_LOCAL_PATH
#endif /* !FX_NO_LOCAL_PATH */


/* FileX is built without update to the time parameters. */

#ifndef FX_NO_TIMER
#define FX_NO_TIMER
#endif /* !FX_NO_TIMER */

#endif


/* Override the interrupt protection provided in FileX port files to simply use ThreadX protection,
   which is often in-line assembly.  */

#ifndef FX_LEGACY_INTERRUPT_PROTECTION
#undef  FX_INT_SAVE_AREA
#undef  FX_DISABLE_INTS
#undef  FX_RESTORE_INTS
#define FX_INT_SAVE_AREA    TX_INTERRUPT_SAVE_AREA
#define FX_DISABLE_INTS     TX_DISABLE
#define FX_RESTORE_INTS     TX_RESTORE
#endif


/* Define the update rate of the system timer.  These values may also be defined at the command
   line when compiling the fx_system_initialize.c module in the FileX library build.  Alternatively, they can
   be modified in this file.  Note: the update rate must be an even number of seconds greater
   than or equal to 2, which is the minimal update rate for FAT time. */

#ifndef FX_UPDATE_RATE_IN_SECONDS
#define FX_UPDATE_RATE_IN_SECONDS       10     /* Update time at 10 second intervals */
#endif

#ifndef FX_UPDATE_RATE_IN_TICKS
#define FX_UPDATE_RATE_IN_TICKS         1000   /* Same update rate, but in ticks  */
#endif


/* Determine if fault tolerance is selected. If so, turn on the old fault tolerant options -
   if they are not defined already.  */

#ifdef FX_ENABLE_FAULT_TOLERANT
#ifndef FX_FAULT_TOLERANT_DATA
#define FX_FAULT_TOLERANT_DATA
#endif
#ifndef FX_FAULT_TOLERANT
#define FX_FAULT_TOLERANT
#endif
#endif


/* Determine if cache is disabled. If so, disable direct read sector cache.  */
#ifdef FX_DISABLE_CACHE
#ifndef FX_DISABLE_DIRECT_DATA_READ_CACHE_FILL
#define FX_DISABLE_DIRECT_DATA_READ_CACHE_FILL
#endif
#endif


/* Determine if local paths are enabled and if the local path setup code has not been defined.
   If so, define the default local path setup code for files that reference the local path.  */

#ifndef FX_NO_LOCAL_PATH
#ifndef FX_LOCAL_PATH_SETUP
#ifndef FX_SINGLE_THREAD
#define FX_LOCAL_PATH_SETUP     extern TX_THREAD *_tx_thread_current_ptr;
#else
#define FX_NO_LOCAL_PATH
#endif
#endif
#endif

/* Determine if tracing is enabled.  */

#if defined(TX_ENABLE_EVENT_TRACE) && !defined(FX_STANDALONE_ENABLE)


/* Trace is enabled. Remap calls so that interrupts can be disabled around the actual event logging.  */

#include "tx_trace.h"


/* Define the object types in FileX, if not defined.  */

#ifndef FX_TRACE_OBJECT_TYPE_MEDIA
#define FX_TRACE_OBJECT_TYPE_MEDIA                      9               /* P1 = FAT cache size, P2 = sector cache size       */
#define FX_TRACE_OBJECT_TYPE_FILE                       10              /* none                                              */
#endif


/* Define event filters that can be used to selectively disable certain events or groups of events.  */

#define FX_TRACE_ALL_EVENTS                             0x00007800      /* All FileX events                          */
#define FX_TRACE_INTERNAL_EVENTS                        0x00000800      /* FileX internal events                     */
#define FX_TRACE_MEDIA_EVENTS                           0x00001000      /* FileX media events                        */
#define FX_TRACE_DIRECTORY_EVENTS                       0x00002000      /* FileX directory events                    */
#define FX_TRACE_FILE_EVENTS                            0x00004000      /* FileX file events                         */


/* Define the trace events in FileX, if not defined.  */

/* Define FileX Trace Events, along with a brief description of the additional information fields,
   where I1 -> Information Field 1, I2 -> Information Field 2, etc.  */

/* Define the FileX internal events first.  */

#ifndef FX_TRACE_INTERNAL_LOG_SECTOR_CACHE_MISS
#define FX_TRACE_INTERNAL_LOG_SECTOR_CACHE_MISS         201             /* I1 = media ptr, I2 = sector, I3 = total misses, I4 = cache size          */
#define FX_TRACE_INTERNAL_DIR_CACHE_MISS                202             /* I1 = media ptr, I2 = total misses                                        */
#define FX_TRACE_INTERNAL_MEDIA_FLUSH                   203             /* I1 = media ptr, I2 = dirty sectors                                       */
#define FX_TRACE_INTERNAL_DIR_ENTRY_READ                204             /* I1 = media ptr                                                           */
#define FX_TRACE_INTERNAL_DIR_ENTRY_WRITE               205             /* I1 = media ptr                                                           */
#define FX_TRACE_INTERNAL_IO_DRIVER_READ                206             /* I1 = media ptr, I2 = sector, I3 = number of sectors, I4 = buffer         */
#define FX_TRACE_INTERNAL_IO_DRIVER_WRITE               207             /* I1 = media ptr, I2 = sector, I3 = number of sectors, I4 = buffer         */
#define FX_TRACE_INTERNAL_IO_DRIVER_FLUSH               208             /* I1 = media ptr                                                           */
#define FX_TRACE_INTERNAL_IO_DRIVER_ABORT               209             /* I1 = media ptr                                                           */
#define FX_TRACE_INTERNAL_IO_DRIVER_INIT                210             /* I1 = media ptr                                                           */
#define FX_TRACE_INTERNAL_IO_DRIVER_BOOT_READ           211             /* I1 = media ptr, I2 = buffer                                              */
#define FX_TRACE_INTERNAL_IO_DRIVER_RELEASE_SECTORS     212             /* I1 = media ptr, I2 = sector, I3 = number of sectors                      */
#define FX_TRACE_INTERNAL_IO_DRIVER_BOOT_WRITE          213             /* I1 = media ptr, I2 = buffer                                              */
#define FX_TRACE_INTERNAL_IO_DRIVER_UNINIT              214             /* I1 = media ptr                                                           */


/* Define the FileX API events.  */

#define FX_TRACE_DIRECTORY_ATTRIBUTES_READ              220             /* I1 = media ptr, I2 = directory name, I3 = attributes                     */
#define FX_TRACE_DIRECTORY_ATTRIBUTES_SET               221             /* I1 = media ptr, I2 = directory name, I3 = attributes                     */
#define FX_TRACE_DIRECTORY_CREATE                       222             /* I1 = media ptr, I2 = directory name                                      */
#define FX_TRACE_DIRECTORY_DEFAULT_GET                  223             /* I1 = media ptr, I2 = return path name                                    */
#define FX_TRACE_DIRECTORY_DEFAULT_SET                  224             /* I1 = media ptr, I2 = new path name                                       */
#define FX_TRACE_DIRECTORY_DELETE                       225             /* I1 = media ptr, I2 = directory name                                      */
#define FX_TRACE_DIRECTORY_FIRST_ENTRY_FIND             226             /* I1 = media ptr, I2 = directory name                                      */
#define FX_TRACE_DIRECTORY_FIRST_FULL_ENTRY_FIND        227             /* I1 = media ptr, I2 = directory name                                      */
#define FX_TRACE_DIRECTORY_INFORMATION_GET              228             /* I1 = media ptr, I2 = directory name                                      */
#define FX_TRACE_DIRECTORY_LOCAL_PATH_CLEAR             229             /* I1 = media ptr                                                           */
#define FX_TRACE_DIRECTORY_LOCAL_PATH_GET               230             /* I1 = media ptr, I2 = return path name                                    */
#define FX_TRACE_DIRECTORY_LOCAL_PATH_RESTORE           231             /* I1 = media ptr, I2 = local path ptr                                      */
#define FX_TRACE_DIRECTORY_LOCAL_PATH_SET               232             /* I1 = media ptr, I2 = local path ptr, I3 = new path name                  */
#define FX_TRACE_DIRECTORY_LONG_NAME_GET                233             /* I1 = media ptr, I2 = short file name, I3 = long file name                */
#define FX_TRACE_DIRECTORY_NAME_TEST                    234             /* I1 = media ptr, I2 = directory name                                      */
#define FX_TRACE_DIRECTORY_NEXT_ENTRY_FIND              235             /* I1 = media ptr, I2 = directory name                                      */
#define FX_TRACE_DIRECTORY_NEXT_FULL_ENTRY_FIND         236             /* I1 = media ptr, I2 = directory name                                      */
#define FX_TRACE_DIRECTORY_RENAME                       237             /* I1 = media ptr, I2 = old directory name, I3 = new directory name         */
#define FX_TRACE_DIRECTORY_SHORT_NAME_GET               238             /* I1 = media ptr, I2 = long file name, I3 = short file name                */
#define FX_TRACE_FILE_ALLOCATE                          239             /* I1 = file ptr, I2 = size I3 = previous size, I4 = new size               */
#define FX_TRACE_FILE_ATTRIBUTES_READ                   240             /* I1 = media ptr, I2 = file name, I3 = attributes                          */
#define FX_TRACE_FILE_ATTRIBUTES_SET                    241             /* I1 = media ptr, I2 = file name, I3 = attributes                          */
#define FX_TRACE_FILE_BEST_EFFORT_ALLOCATE              242             /* I1 = file ptr, I2 = size, I3 = actual_size_allocated                     */
#define FX_TRACE_FILE_CLOSE                             243             /* I1 = file ptr, I3 = file size                                            */
#define FX_TRACE_FILE_CREATE                            244             /* I1 = media ptr, I2 = file name                                           */
#define FX_TRACE_FILE_DATE_TIME_SET                     245             /* I1 = media ptr, I2 = file name, I3 = year, I4 = month                    */
#define FX_TRACE_FILE_DELETE                            246             /* I1 = media ptr, I2 = file name                                           */
#define FX_TRACE_FILE_OPEN                              247             /* I1 = media ptr, I2 = file ptr, I3 = file name, I4 = open type            */
#define FX_TRACE_FILE_READ                              248             /* I1 = file ptr, I2 = buffer ptr, I3 = request size I4 = actual size       */
#define FX_TRACE_FILE_RELATIVE_SEEK                     249             /* I1 = file ptr, I2 = byte offset, I3 = seek from, I4 = previous offset    */
#define FX_TRACE_FILE_RENAME                            250             /* I1 = media ptr, I2 = old file name, I3 = new file name                   */
#define FX_TRACE_FILE_SEEK                              251             /* I1 = file ptr, I2 = byte offset, I3 = previous offset                    */
#define FX_TRACE_FILE_TRUNCATE                          252             /* I1 = file ptr, I2 = size, I3 = previous size, I4 = new size              */
#define FX_TRACE_FILE_TRUNCATE_RELEASE                  253             /* I1 = file ptr, I2 = size, I3 = previous size, I4 = new size              */
#define FX_TRACE_FILE_WRITE                             254             /* I1 = file ptr, I2 = buffer ptr, I3 = size, I4 = bytes written            */
#define FX_TRACE_MEDIA_ABORT                            255             /* I1 = media ptr                                                           */
#define FX_TRACE_MEDIA_CACHE_INVALIDATE                 256             /* I1 = media ptr                                                           */
#define FX_TRACE_MEDIA_CHECK                            257             /* I1 = media ptr, I2 = scratch memory, I3 = scratch memory size, I4 =errors*/
#define FX_TRACE_MEDIA_CLOSE                            258             /* I1 = media ptr                                                           */
#define FX_TRACE_MEDIA_FLUSH                            259             /* I1 = media ptr                                                           */
#define FX_TRACE_MEDIA_FORMAT                           260             /* I1 = media ptr, I2 = root entries, I3 = sectors, I4 = sectors per cluster*/
#define FX_TRACE_MEDIA_OPEN                             261             /* I1 = media ptr, I2 = media driver, I3 = memory ptr, I4 = memory size     */
#define FX_TRACE_MEDIA_READ                             262             /* I1 = media ptr, I2 = logical sector, I3 = buffer ptr, I4 = bytes read    */
#define FX_TRACE_MEDIA_SPACE_AVAILABLE                  263             /* I1 = media ptr, I2 = available bytes ptr, I3 = available clusters        */
#define FX_TRACE_MEDIA_VOLUME_GET                       264             /* I1 = media ptr, I2 = volume name, I3 = volume source                     */
#define FX_TRACE_MEDIA_VOLUME_SET                       265             /* I1 = media ptr, I2 = volume name                                         */
#define FX_TRACE_MEDIA_WRITE                            266             /* I1 = media ptr, I2 = logical_sector, I3 = buffer_ptr, I4 = byte written  */
#define FX_TRACE_SYSTEM_DATE_GET                        267             /* I1 = year, I2 = month, I3 = day                                          */
#define FX_TRACE_SYSTEM_DATE_SET                        268             /* I1 = year, I2 = month, I3 = day                                          */
#define FX_TRACE_SYSTEM_INITIALIZE                      269             /* None                                                                     */
#define FX_TRACE_SYSTEM_TIME_GET                        270             /* I1 = hour, I2 = minute, I3 = second                                      */
#define FX_TRACE_SYSTEM_TIME_SET                        271             /* I1 = hour, I2 = minute, I3 = second                                      */
#define FX_TRACE_UNICODE_DIRECTORY_CREATE               272             /* I1 = media ptr, I2 = source unicode, I3 = source length, I4 = short_name */
#define FX_TRACE_UNICODE_DIRECTORY_RENAME               273             /* I1 = media ptr, I2 = source unicode, I3 = source length, I4 = new_name   */
#define FX_TRACE_UNICODE_FILE_CREATE                    274             /* I1 = media ptr, I2 = source unicode, I3 = source length, I4 = short name */
#define FX_TRACE_UNICODE_FILE_RENAME                    275             /* I1 = media ptr, I2 = source unicode, I3 = source length, I4 = new name   */
#define FX_TRACE_UNICODE_LENGTH_GET                     276             /* I1 = unicode name, I2 = length                                           */
#define FX_TRACE_UNICODE_NAME_GET                       277             /* I1 = media ptr, I2 = source short name, I3 = unicode name, I4 = length   */
#define FX_TRACE_UNICODE_SHORT_NAME_GET                 278             /* I1 = media ptr, I2 = source unicode name, I3 = length, I4 =  short name  */
#endif


/* Map the trace macros to internal FileX versions so we can get interrupt protection.  */

#ifdef FX_SOURCE_CODE

#define FX_TRACE_OBJECT_REGISTER(t, p, n, a, b)         _fx_trace_object_register(t, (VOID *)p, (CHAR *)n, (ULONG)a, (ULONG)b);
#define FX_TRACE_OBJECT_UNREGISTER(o)                   _fx_trace_object_unregister((VOID *)o);
#define FX_TRACE_IN_LINE_INSERT(i, a, b, c, d, f, g, h) _fx_trace_event_insert((ULONG)i, (ULONG)a, (ULONG)b, (ULONG)c, (ULONG)d, (ULONG)f, g, h);
#define FX_TRACE_EVENT_UPDATE(e, t, i, a, b, c, d)      _fx_trace_event_update((TX_TRACE_BUFFER_ENTRY *)e, (ULONG)t, (ULONG)i, (ULONG)a, (ULONG)b, (ULONG)c, (ULONG)d);


/* Define FileX trace prototypes.  */

VOID _fx_trace_object_register(UCHAR object_type, VOID *object_ptr, CHAR *object_name, ULONG parameter_1, ULONG parameter_2);
VOID _fx_trace_object_unregister(VOID *object_ptr);
VOID _fx_trace_event_insert(ULONG event_id, ULONG info_field_1, ULONG info_field_2, ULONG info_field_3, ULONG info_field_4, ULONG filter, TX_TRACE_BUFFER_ENTRY **current_event, ULONG *current_timestamp);
VOID _fx_trace_event_update(TX_TRACE_BUFFER_ENTRY *event, ULONG timestamp, ULONG event_id, ULONG info_field_1, ULONG info_field_2, ULONG info_field_3, ULONG info_field_4);

#endif

#else
#define FX_TRACE_OBJECT_REGISTER(t, p, n, a, b)
#define FX_TRACE_OBJECT_UNREGISTER(o)
#define FX_TRACE_IN_LINE_INSERT(i, a, b, c, d, f, g, h)
#define FX_TRACE_EVENT_UPDATE(e, t, i, a, b, c, d)
#endif


/* Define basic constants for FileX.  */

#define FX_MEDIA_ID                            ((ULONG)0x4D454449)
#define FX_MEDIA_CLOSED_ID                     ((ULONG)0x4D454443)
#define FX_MEDIA_ABORTED_ID                    ((ULONG)0x4D454441)

#define FX_FILE_ID                             ((ULONG)0x46494C45)
#define FX_FILE_CLOSED_ID                      ((ULONG)0x46494C43)
#define FX_FILE_ABORTED_ID                     ((ULONG)0x46494C41)


/* The maximum path includes the entire path and the file name.  */

#ifndef FX_MAXIMUM_PATH
#define FX_MAXIMUM_PATH                        256
#endif


/* Define directory entry name sizes. Directory entries are found in the
   root directory and in directory files (sub-directories). Each directory
   entry has a short file name which is a fixed maximum of 13 characters and
   and optional 255 character long file name.  */

#ifndef FX_MAX_LONG_NAME_LEN
#define FX_MAX_LONG_NAME_LEN                   256      /* Minimum value is 13, Maximum value is 256.  */
#endif

#define FX_MAX_SHORT_NAME_LEN                  13       /* Only allowed value is 13.  */

#ifndef FX_MAX_LAST_NAME_LEN
#define FX_MAX_LAST_NAME_LEN                   256      /* Should be as large or larger than FX_MAX_LONG_NAME_LEN.  */
#endif


/* Define constants related to the logical sector cache. These constants represent the maximum
   number of logical sectors that can be cached. The actual number of logical sectors in the
   cache is determined by the amount of memory supplied to fx_media_open for the FX_MEDIA
   instance.  */

#ifndef FX_MAX_SECTOR_CACHE
#define FX_MAX_SECTOR_CACHE                    256      /* Maximum size of logical sector cache,
                                                           minimum value of 2 all other values must
                                                           be a power of 2.  */
#endif

/* Define the mask for the hash index into the logical sector cache.  If the logical sector cache
   determined by the amount of memory supplied by the application to fx_media_open is larger than
   FX_SECTOR_CACHE_HASH_ENABLE (usually 16) and can satisfy FX_MAX_SECTOR_CACHE sectors, the cache
   is divided into 4 entry pieces (FX_SECTOR_CACHE_DEPTH) that are indexed by the formula:

        index =  (cluster & media_ptr -> fx_media_sector_cache_hash_mask) * FX_SECTOR_CACHE_LEVELS

   The FX_SECTOR_CACHE_DEPTH define must not be changed unless the logical sector read/write
   code is also changed.  */

#define FX_SECTOR_CACHE_HASH_ENABLE            16
#define FX_SECTOR_CACHE_DEPTH                  4

#ifndef FX_FAT_MAP_SIZE
#define FX_FAT_MAP_SIZE                        128  /* Minimum 1, maximum any. This represents how many 32-bit words used for the written FAT sector bit map. */
#endif

#ifndef FX_MAX_FAT_CACHE
#define FX_MAX_FAT_CACHE                       16   /* Minimum value is 8, all values must be a power of 2.  */
#endif


/* Define the size of fault tolerant cache, which is used when freeing FAT chain. */

#ifndef FX_FAULT_TOLERANT_CACHE_SIZE
#define FX_FAULT_TOLERANT_CACHE_SIZE           1024
#endif /* FX_FAULT_TOLERANT_CACHE_SIZE */


/* Define the mask for the hash index into the FAT table.  The FAT cache is divided into 4 entry pieces
   that are indexed by the formula:

            index =  (cluster & FX_FAT_CACHE_HASH_MASK) * FX_FAT_CACHE_LEVELS

   The FX_FAT_CACHE_DEPTH define must not be changed unless the FAT entry read/write code is also changed.  */

#define FX_FAT_CACHE_DEPTH                     4
#define FX_FAT_CACHE_HASH_MASK                 ((FX_MAX_FAT_CACHE / FX_FAT_CACHE_DEPTH) - 1)


/* FileX API input parameters and general constants.  */

#define FX_TRUE                                1
#define FX_FALSE                               0
#define FX_NULL                                0
#define FX_OPEN_FOR_READ                       0
#define FX_OPEN_FOR_WRITE                      1
#define FX_OPEN_FOR_READ_FAST                  2

#define FX_12_BIT_FAT_SIZE                     4086
#define FX_16_BIT_FAT_SIZE                     65525
#define FX_MAX_12BIT_CLUST                     0x0FF0
#define FX_SIGN_EXTEND                         0xF000
#define FX_12BIT_SIZE                          3    /* 2 FAT entries per 3 bytes  */

#define FX_FAT_ENTRY_START                     2
#define FX_DIR_ENTRY_SIZE                      32
#define FX_DIR_NAME_SIZE                       8
#define FX_DIR_EXT_SIZE                        3
#define FX_DIR_RESERVED                        8
#define FX_DIR_ENTRY_FREE                      0xE5
#define FX_DIR_ENTRY_DONE                      0x00
#define FX_READ_ONLY                           0x01
#define FX_HIDDEN                              0x02
#define FX_SYSTEM                              0x04
#define FX_VOLUME                              0x08
#define FX_DIRECTORY                           0x10
#define FX_ARCHIVE                             0x20
#define FX_LONG_NAME                           (FX_READ_ONLY | FX_HIDDEN | FX_SYSTEM | FX_VOLUME)
#define FX_LONG_NAME_ENTRY_LEN                 13


/* Define FAT FAT entry values.  */

#define FX_FREE_CLUSTER                        0x0000
#define FX_NOT_USED                            0x0001
#define FX_RESERVED_1                          0xFFF0
#define FX_RESERVED_2                          0xFFF6
#define FX_BAD_CLUSTER                         0xFFF7
#define FX_LAST_CLUSTER_1                      0xFFF8
#define FX_LAST_CLUSTER_2                      0xFFFF

#define FX_RESERVED_1_32                       0x0FFFFFF0
#define FX_RESERVED_2_32                       0x0FFFFFF6
#define FX_BAD_CLUSTER_32                      0x0FFFFFF7
#define FX_LAST_CLUSTER_1_32                   0x0FFFFFF8
#define FX_LAST_CLUSTER_2_32                   0x0FFFFFFF


/* Define time/date FAT constants.  */

#define FX_YEAR_SHIFT                          9
#define FX_MONTH_SHIFT                         5
#define FX_HOUR_SHIFT                          11
#define FX_MINUTE_SHIFT                        5
#define FX_YEAR_MASK                           0x7F
#define FX_MONTH_MASK                          0x0F
#define FX_DAY_MASK                            0x1F
#define FX_HOUR_MASK                           0x1F
#define FX_MINUTE_MASK                         0x3F
#define FX_SECOND_MASK                         0x1F
#define FX_BASE_YEAR                           1980
#define FX_MAXIMUM_YEAR                        2107
#define FX_MAXIMUM_MONTH                       12
#define FX_MAXIMUM_HOUR                        23
#define FX_MAXIMUM_MINUTE                      59
#define FX_MAXIMUM_SECOND                      59
#define FX_INITIAL_DATE                        0x4A21   /* 01-01-2017 */
#define FX_INITIAL_TIME                        0x0000   /* 12:00 am   */

/* FileX API return values.  */

#define FX_SUCCESS                             0x00
#define FX_BOOT_ERROR                          0x01
#define FX_MEDIA_INVALID                       0x02
#define FX_FAT_READ_ERROR                      0x03
#define FX_NOT_FOUND                           0x04
#define FX_NOT_A_FILE                          0x05
#define FX_ACCESS_ERROR                        0x06
#define FX_NOT_OPEN                            0x07
#define FX_FILE_CORRUPT                        0x08
#define FX_END_OF_FILE                         0x09
#define FX_NO_MORE_SPACE                       0x0A
#define FX_ALREADY_CREATED                     0x0B
#define FX_INVALID_NAME                        0x0C
#define FX_INVALID_PATH                        0x0D
#define FX_NOT_DIRECTORY                       0x0E
#define FX_NO_MORE_ENTRIES                     0x0F
#define FX_DIR_NOT_EMPTY                       0x10
#define FX_MEDIA_NOT_OPEN                      0x11
#define FX_INVALID_YEAR                        0x12
#define FX_INVALID_MONTH                       0x13
#define FX_INVALID_DAY                         0x14
#define FX_INVALID_HOUR                        0x15
#define FX_INVALID_MINUTE                      0x16
#define FX_INVALID_SECOND                      0x17
#define FX_PTR_ERROR                           0x18
#define FX_INVALID_ATTR                        0x19
#define FX_CALLER_ERROR                        0x20
#define FX_BUFFER_ERROR                        0x21
#define FX_NOT_IMPLEMENTED                     0x22
#define FX_WRITE_PROTECT                       0x23
#define FX_INVALID_OPTION                      0x24
#define FX_SECTOR_INVALID                      0x89
#define FX_IO_ERROR                            0x90
#define FX_NOT_ENOUGH_MEMORY                   0x91
#define FX_ERROR_FIXED                         0x92
#define FX_ERROR_NOT_FIXED                     0x93
#define FX_NOT_AVAILABLE                       0x94
#define FX_INVALID_CHECKSUM                    0x95
#define FX_READ_CONTINUE                       0x96
#define FX_INVALID_STATE                       0x97


/* FileX driver interface constants.  */

#define FX_DRIVER_READ                         0
#define FX_DRIVER_WRITE                        1
#define FX_DRIVER_FLUSH                        2
#define FX_DRIVER_ABORT                        3
#define FX_DRIVER_INIT                         4
#define FX_DRIVER_BOOT_READ                    5
#define FX_DRIVER_RELEASE_SECTORS              6
#define FX_DRIVER_BOOT_WRITE                   7
#define FX_DRIVER_UNINIT                       8


/* Define relative seek constants.  */

#define FX_SEEK_BEGIN                          0
#define FX_SEEK_END                            1
#define FX_SEEK_FORWARD                        2
#define FX_SEEK_BACK                           3


/* Define types for logical sectors. This information is passed to the driver
   as additional information.  */

#define FX_UNKNOWN_SECTOR                      0
#define FX_BOOT_SECTOR                         1
#define FX_FAT_SECTOR                          2
#define FX_DIRECTORY_SECTOR                    3
#define FX_DATA_SECTOR                         4


/* Define media diagnostic constants.  */

#define FX_FAT_CHAIN_ERROR                     0x01
#define FX_DIRECTORY_ERROR                     0x02
#define FX_LOST_CLUSTER_ERROR                  0x04
#define FX_FILE_SIZE_ERROR                     0x08


/* Define boot record offset constants.  */

#define FX_JUMP_INSTR                          0x000
#define FX_OEM_NAME                            0x003
#define FX_BYTES_SECTOR                        0x00B
#define FX_SECTORS_CLUSTER                     0x00D
#define FX_RESERVED_SECTORS                    0x00E
#define FX_NUMBER_OF_FATS                      0x010
#define FX_ROOT_DIR_ENTRIES                    0x011
#define FX_SECTORS                             0x013
#define FX_MEDIA_TYPE                          0x015
#define FX_SECTORS_PER_FAT                     0x016
#define FX_SECTORS_PER_TRK                     0x018
#define FX_HEADS                               0x01A
#define FX_HIDDEN_SECTORS                      0x01C
#define FX_HUGE_SECTORS                        0x020
#define FX_DRIVE_NUMBER                        0x024
#define FX_RESERVED                            0x025
#define FX_BOOT_SIG                            0x026
#define FX_BOOT_SIG_32                         0x042
#define FX_VOLUME_ID                           0x027
#define FX_VOLUME_ID_32                        0x043
#define FX_VOLUME_LABEL                        0x02B
#define FX_VOLUME_LABEL_32                     0x047
#define FX_FILE_SYSTEM_TYPE                    0x036
#define FX_SIG_OFFSET                          0x1FE
#define FX_ROOT_CLUSTER_32                     0x02C
#define FX_SECTORS_PER_FAT_32                  0x024
#define FX_SIG_BYTE_1                          0x55
#define FX_SIG_BYTE_2                          0xAA


/* If not-defined, default port-specific processing extensions to white space.  */

#ifndef FX_FILE_OPEN_EXTENSION
#define FX_FILE_OPEN_EXTENSION
#endif

#ifndef FX_DIRECTORY_ENTRY_READ_EXTENSION
#define FX_DIRECTORY_ENTRY_READ_EXTENSION
#endif

#ifndef FX_DIRECTORY_ENTRY_WRITE_EXTENSION
#define FX_DIRECTORY_ENTRY_WRITE_EXTENSION
#endif

#ifndef FX_UTILITY_FAT_ENTRY_READ_EXTENSION
#define FX_UTILITY_FAT_ENTRY_READ_EXTENSION
#endif

#ifndef FX_UTILITY_FAT_ENTRY_WRITE_EXTENSION
#define FX_UTILITY_FAT_ENTRY_WRITE_EXTENSION
#endif

#ifndef FX_UTILITY_LOGICAL_SECTOR_FLUSH_EXTENSION
#define FX_UTILITY_LOGICAL_SECTOR_FLUSH_EXTENSION
#endif

#ifndef FX_UTILITY_LOGICAL_SECTOR_READ_EXTENSION
#define FX_UTILITY_LOGICAL_SECTOR_READ_EXTENSION
#endif

#ifndef FX_UTILITY_LOGICAL_SECTOR_READ_EXTENSION_1
#define FX_UTILITY_LOGICAL_SECTOR_READ_EXTENSION_1
#endif

#ifndef FX_UTILITY_LOGICAL_SECTOR_WRITE_EXTENSION
#define FX_UTILITY_LOGICAL_SECTOR_WRITE_EXTENSION
#endif


#ifdef FX_ENABLE_EXFAT


/* Define exFAT specific constants.  */

#define FX_EF_MUST_BE_ZERO                     11       /* Must be Zero Area                  053 bytes */
#define FX_EF_PARTITION_OFFSET                 64       /* Partition Offset                   008 bytes */
#define FX_EF_VOLUME_LENGTH                    72       /* Number of sectors on the Partition 008 bytes */
#define FX_EF_FAT_OFFSET                       80       /* Number of sectors before exFAT     004 bytes */
#define FX_EF_FAT_LENGTH                       84       /* Each FAT length in Sectors         004 bytes */
#define FX_EF_CLUSTER_HEAP_OFFSET              88       /* Number of Sectors Before Cluster   004 bytes
                                                           Heap */
#define FX_EF_CLUSTER_COUNT                    92       /* Number of Clusters in the Cluster  004 bytes
                                                           Heap */
#define FX_EF_FIRST_CLUSTER_OF_ROOT_DIR        96       /* Cluster index of the First         004 bytes
                                                           Cluster of the ROOT Directory                */
#define FX_EF_VOLUME_SERIAL_NUMBER             100      /* Volume Serial Number               004 bytes */
#define FX_EF_FILE_SYSTEM_REVISION             104      /* File System  Revision              002 bytes */
#define FX_EF_VOLUME_FLAGS                     106      /* Status of exFAT structures         002 bytes */
#define FX_EF_BYTE_PER_SECTOR_SHIFT            108      /* log2(N),where N is the Number of   001 byte
                                                           Bytes per Sector */
#define FX_EF_SECTOR_PER_CLUSTER_SHIFT         109      /* log2(N),where N is the Number of   001 byte
                                                           Sectors per Cluster */
#define FX_EF_NUMBER_OF_FATS                   110      /* Number of FATs                     001 byte */
#define FX_EF_DRIVE_SELECT                     111      /* Extended INT13h driver number      001 byte */
#define FX_EF_PERCENT_IN_USE                   112      /* Percentage of Allocated Clusters   001 byte */
#define FX_EF_RESERVED                         113      /* Reserved                           007 bytes */
#define FX_EF_BOOT_CODE                        120      /* Boot code area                     390 bytes */

#define FX_RESERVED_1_exFAT                    0xFFFFFFF8
#define FX_RESERVED_2_exFAT                    0xFFFFFFFE
#define FX_BAD_CLUSTER_exFAT                   0xFFFFFFF7
#define FX_LAST_CLUSTER_exFAT                  0xFFFFFFFF

#ifndef FX_MAX_EX_FAT_NAME_LEN
#define FX_MAX_EX_FAT_NAME_LEN                 255      /* Only allowed value is 255 */
#endif

#ifndef BITS_PER_BYTE
#define BITS_PER_BYTE                          8
#endif
#define BITS_PER_BYTE_SHIFT                    3


#define FX_EXFAT_MAX_DIRECTORY_SIZE            (256 * 1024 * 1024)  /* 256 MB */
#define FX_EXFAT_SIZE_OF_FAT_ELEMENT_SHIFT     2

#define FX_EXFAT_BIT_MAP_NUM_OF_CACHED_SECTORS 1

#define FX_EXFAT_BITMAP_CLUSTER_FREE           0
#define FX_EXFAT_BITMAP_CLUSTER_OCCUPIED       1

#ifndef FX_EXFAT_MAX_CACHE_SIZE
#define FX_EXFAT_MAX_CACHE_SIZE                512
#endif
#define FX_EXFAT_BITMAP_CACHE_SIZE             FX_EXFAT_MAX_CACHE_SIZE

/* exFAT System Area Layout */

#define FX_EXFAT_FAT_MAIN_SYSTEM_AREA_SIZE     12

#define FX_EXFAT_FAT_MAIN_BOOT_SECTOR_OFFSET   0
#define FX_EXFAT_FAT_BACKUP_BOOT_SECTOR_OFFSET FX_EXFAT_FAT_MAIN_SYSTEM_AREA_SIZE

#define FX_EXFAT_FAT_EXT_BOOT_SECTOR_OFFSET    1
#define FX_EXFAT_FAT_OEM_PARAM_OFFSET          9
#define FX_EXFAT_FAT_CHECK_SUM_OFFSET          11

#define FX_EXFAT_FAT_NUM_OF_SYSTEM_AREAS       2


/* Define exFAT format parameters.  */

#define  EXFAT_MIN_NUM_OF_RESERVED_SECTORS     1
#define  EXFAT_BOOT_REGION_SIZE                24
#define  EXFAT_FAT_BITS                        32
#define  EXFAT_FAT_FILE_SYS_REVISION           0x100
#define  EXFAT_FAT_VOLUME_FLAG                 0x000
#define  EXFAT_FAT_NUM_OF_FATS                 0x001
#define  EXFAT_FAT_DRIVE_SELECT                0x080
#define  EXFAT_FAT_VOLUME_NAME_FIELD_SIZE      11
#define  EXFAT_BIT_MAP_FIRST_TABLE             0
#define  EXFAT_LAST_CLUSTER_MASK               0xFFFFFFFF
#define  EXFAT_DEFAULT_BOUNDARY_UNIT           128
#define  EXFAT_NUM_OF_DIR_ENTRIES              2


#define DIVIDE_TO_CEILING(a, b)                (((a) + (b) - 1) / (b))
#define ALIGN_UP(a, b)                         (DIVIDE_TO_CEILING(a, b) * (b))

#define FX_FAT12                               0x01
#define FX_FAT16                               0x04
#define FX_BIGDOS                              0x06
#define FX_exFAT                               0x07
#define FX_FAT32                               0x0B
#define FX_NO_FAT                              0xFF

#endif /* FX_ENABLE_EXFAT */


/* Define the control block definitions for all system objects.  */


/* Define a single entry in the FAT cache.  The FAT cache is used to reduce the
   number of times the actual FAT sectors need to be accessed, thereby improving
   performance.  */

typedef struct FX_FAT_CACHE_ENTRY_STRUCT
{
    ULONG fx_fat_cache_entry_cluster;
    ULONG fx_fat_cache_entry_value;
    ULONG fx_fat_cache_entry_dirty;
} FX_FAT_CACHE_ENTRY;


/* Define the directory entry structure that contains information about a specific
   directory entry.  */

typedef struct FX_DIR_ENTRY_STRUCT
{

    CHAR   *fx_dir_entry_name;
    CHAR    fx_dir_entry_short_name[FX_MAX_SHORT_NAME_LEN];                 /* Short file name, if LFN is present                */
    UINT    fx_dir_entry_long_name_present;                                 /* 0 (default) => LFN not present; 1 => LFN present  */
    UINT    fx_dir_entry_long_name_shorted;                                 /* LFN too large, file name was made shorter         */
    UCHAR   fx_dir_entry_attributes;                                        /* Directory entry attributes                        */
    UCHAR   fx_dir_entry_reserved;                                          /* NT reserved, write 0 at create, otherwise ignore  */
    UCHAR   fx_dir_entry_created_time_ms;                                   /* Create time in milliseconds, always write 0       */
    UINT    fx_dir_entry_created_time;                                      /* Created time                                      */
    UINT    fx_dir_entry_created_date;                                      /* Created date                                      */
    UINT    fx_dir_entry_last_accessed_date;                                /* Last accessed date                                */
    UINT    fx_dir_entry_time;                                              /* Modified time                                     */
    UINT    fx_dir_entry_date;                                              /* Modified cluster                                  */
    ULONG   fx_dir_entry_cluster;                                           /* File/directory's starting cluster                 */
    ULONG64 fx_dir_entry_file_size;                                         /* Size of the file in bytes                         */
    ULONG64 fx_dir_entry_log_sector;                                        /* Logical sector of this directory entry            */
    ULONG   fx_dir_entry_byte_offset;                                       /* Offset in logical sector of this directory entry  */
    ULONG   fx_dir_entry_number;                                            /* Index into the directory                          */
    ULONG   fx_dir_entry_last_search_cluster;                               /* Last cluster searched                             */
    ULONG   fx_dir_entry_last_search_relative_cluster;                      /* Last relative cluster searched                    */
    ULONG64 fx_dir_entry_last_search_log_sector;                            /* Last logical sector searched                      */
    ULONG   fx_dir_entry_last_search_byte_offset;                           /* Last offset in logical sector searched            */
    ULONG64 fx_dir_entry_next_log_sector;

#ifdef FX_ENABLE_EXFAT
    /* for exFAT */
    CHAR    fx_dir_entry_dont_use_fat;                                      /* 0 bit - for current, 1st bit - for parent         */
    UCHAR   fx_dir_entry_type;
    ULONG64 fx_dir_entry_available_file_size;
    ULONG   fx_dir_entry_secondary_count;
#endif /* FX_ENABLE_EXFAT */
} FX_DIR_ENTRY;

typedef FX_DIR_ENTRY  *FX_DIR_ENTRY_PTR;


/* Define the path data structure.  This structure will contain
   the current path and the information for performing directory
   entry operations.  */

typedef struct FX_PATH_STRUCT
{
    /* Define the path information.  */
    FX_DIR_ENTRY fx_path_directory;
    CHAR         fx_path_string[FX_MAXIMUM_PATH];
    CHAR         fx_path_name_buffer[FX_MAX_LONG_NAME_LEN];
    ULONG        fx_path_current_entry;
} FX_PATH;

typedef FX_PATH FX_LOCAL_PATH;


/* Define the cache control data structure.  There are FX_MAX_SECTOR_CACHE
   of these structures defined inside the FX_MEDIA structure.  Each entry
   maintains a cache representation of a media sector.  */

typedef struct FX_CACHED_SECTOR_STRUCT
{

    /* Define the buffer pointer associated with this cache entry.  */
    UCHAR               *fx_cached_sector_memory_buffer;

    /* Define the sector number that is cached.  */
    ULONG64             fx_cached_sector;

    /* Define the flag that indicates whether or not the cached sector
       has been modified and needs to be written to the media.  */
    UCHAR               fx_cached_sector_buffer_dirty;

    /* Define the valid flag that indicates whether or not this entry is
       still valid.  */
    UCHAR               fx_cached_sector_valid;

    /* Define the sector type, which indicates what type of sector is present.  */
    UCHAR               fx_cached_sector_type;

    /* Define a reserved byte, reserved for future use.  */
    UCHAR               fx_cached_sector_reserved;

    /* Define the next cached sector pointer.  This is used to implement
       the "last used" algorithm when looking for cache entry to swap out to
       the physical media.  */
    struct FX_CACHED_SECTOR_STRUCT
                        *fx_cached_sector_next_used;

} FX_CACHED_SECTOR;


/* Determine if the media control block has an extension defined. If not, 
   define the extension to whitespace.  */

#ifndef FX_MEDIA_MODULE_EXTENSION
#define FX_MEDIA_MODULE_EXTENSION
#endif


/* Define the media control block.  All information about each open
   media device are maintained in by the FX_MEDIA data type.  */

typedef struct FX_MEDIA_STRUCT
{

    /* Define the media ID used for error checking.  */
    ULONG               fx_media_id;

    /* Define the media's name.  */
    CHAR                *fx_media_name;

    /* Remember the memory buffer area.  */
    UCHAR               *fx_media_memory_buffer;
    ULONG               fx_media_memory_size;

#ifdef FX_DISABLE_CACHE
    ULONG64             fx_media_memory_buffer_sector;
#else

    /* Define the flag that indicates whether the logical cache utilizes
       a hash function or is a linear search. If set, the logical cache
       is accessed via a hash function on the requested sector.  */
    UINT                fx_media_sector_cache_hashed;

    /* Define the number of sectors that can actually be cached based on the
       user supplied buffer at media open time.  */
    ULONG               fx_media_sector_cache_size;

    /* Define the end of the cache area.  This is used to determine
       if the I/O is for the internal memory of the media.  */
    UCHAR               *fx_media_sector_cache_end;

    /* Define the list head of the cached sector entries.  This
       pointer points to the most recently used cache sector.  */
    struct FX_CACHED_SECTOR_STRUCT
                        *fx_media_sector_cache_list_ptr;

    /* Define the bit map that represents the hashed cache sectors that are
       valid. This bit map will help optimize the invalidation of the hashed
       sector cache.  */
    ULONG               fx_media_sector_cache_hashed_sector_valid;

    /* Define the outstanding dirty sector counter. This is used to optimize
       the searching of sectors to flush to the media.  */
    ULONG               fx_media_sector_cache_dirty_count;
#endif /* FX_DISABLE_CACHE */

    /* Define the basic information about the associated media.  */
    UINT                fx_media_bytes_per_sector;
    UINT                fx_media_sectors_per_track;
    UINT                fx_media_heads;

    ULONG64             fx_media_total_sectors;
    ULONG               fx_media_total_clusters;

#ifdef FX_ENABLE_EXFAT
    /* Define exFAT media information.  */
    ULONG               fx_media_exfat_volume_serial_number;
    UINT                fx_media_exfat_file_system_revision;
    UINT                fx_media_exfat_volume_flag;
    USHORT              fx_media_exfat_drive_select;
    USHORT              fx_media_exfat_percent_in_use;
    UINT                fx_media_exfat_bytes_per_sector_shift;
    UINT                fx_media_exfat_sector_per_clusters_shift;

    /* exFAT: Bitmap cache */
    /* Pointer to Bitmap cache */
    UCHAR               fx_media_exfat_bitmap_cache[FX_EXFAT_BITMAP_CACHE_SIZE];

    /* Define beginning sector of Bitmap table.  */
    ULONG               fx_media_exfat_bitmap_start_sector;

    /* Define the cache size in sectors. Used for flash operation.  */
    ULONG               fx_media_exfat_bitmap_cache_size_in_sectors;

    /* Define the number of first cached cluster.  */
    ULONG               fx_media_exfat_bitmap_cache_start_cluster;

    /* Define the number of last cached cluster.  */
    ULONG               fx_media_exfat_bitmap_cache_end_cluster;

    /* Define how many clusters mapped in one sector.  */
    UINT                fx_media_exfat_bitmap_clusters_per_sector_shift;

    /* Define is Bitmap table was changed or not.  */
    UINT                fx_media_exfat_bitmap_cache_dirty;
#endif /* FX_ENABLE_EXFAT */

    UINT                fx_media_reserved_sectors;
    UINT                fx_media_root_sector_start;
    UINT                fx_media_root_sectors;
    UINT                fx_media_data_sector_start;
    UINT                fx_media_sectors_per_cluster;
    UINT                fx_media_sectors_per_FAT;
    UINT                fx_media_number_of_FATs;
    UINT                fx_media_12_bit_FAT;
    UINT                fx_media_32_bit_FAT;
    ULONG               fx_media_FAT32_additional_info_sector;
    UINT                fx_media_FAT32_additional_info_last_available;
#ifdef FX_DRIVER_USE_64BIT_LBA
    ULONG64             fx_media_hidden_sectors;
#else
    ULONG               fx_media_hidden_sectors;
#endif
    ULONG               fx_media_root_cluster_32;
    UINT                fx_media_root_directory_entries;
    ULONG               fx_media_available_clusters;
    ULONG               fx_media_cluster_search_start;

    /* Define the information pertinent to the I/O driver interface.  */

    VOID                *fx_media_driver_info;
    UINT                fx_media_driver_request;
    UINT                fx_media_driver_status;
    UCHAR               *fx_media_driver_buffer;
#ifdef FX_DRIVER_USE_64BIT_LBA
    ULONG64             fx_media_driver_logical_sector;
#else
    ULONG               fx_media_driver_logical_sector;
#endif
    ULONG               fx_media_driver_sectors;
    ULONG               fx_media_driver_physical_sector;
    UINT                fx_media_driver_physical_track;
    UINT                fx_media_driver_physical_head;
    UINT                fx_media_driver_write_protect;      /* The driver sets this to FX_TRUE when media is write protected.  */
    UINT                fx_media_driver_free_sector_update; /* The driver sets this to FX_TRUE when it needs to know freed clusters.  */
    UINT                fx_media_driver_system_write;
    UINT                fx_media_driver_data_sector_read;
    UINT                fx_media_driver_sector_type;

    /* Define the driver entry point.  */
    VOID                (*fx_media_driver_entry)(struct FX_MEDIA_STRUCT *);

    /* Define notify function called when media is open. */
    VOID                (*fx_media_open_notify)(struct FX_MEDIA_STRUCT *);

    /* Define notify function called when media is closed. */
    VOID                (*fx_media_close_notify)(struct FX_MEDIA_STRUCT *);

    /* Define the head pointer for the open files of this media.  */
    struct FX_FILE_STRUCT
                        *fx_media_opened_file_list;

    /* Define the counter for keeping track of how many open files are
       present.  */
    ULONG               fx_media_opened_file_count;

    /* Define the next and previous link pointers for the open media list.  */
    struct FX_MEDIA_STRUCT
                        *fx_media_opened_next,
                        *fx_media_opened_previous;

#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Define various statistics for this media instance. This information
       should be useful in performance tuning and optimizing the application.  */

    ULONG               fx_media_directory_attributes_reads;
    ULONG               fx_media_directory_attributes_sets;
    ULONG               fx_media_directory_creates;
    ULONG               fx_media_directory_default_gets;
    ULONG               fx_media_directory_default_sets;
    ULONG               fx_media_directory_deletes;
    ULONG               fx_media_directory_first_entry_finds;
    ULONG               fx_media_directory_first_full_entry_finds;
    ULONG               fx_media_directory_information_gets;
    ULONG               fx_media_directory_local_path_clears;
    ULONG               fx_media_directory_local_path_gets;
    ULONG               fx_media_directory_local_path_restores;
    ULONG               fx_media_directory_local_path_sets;
    ULONG               fx_media_directory_name_tests;
    ULONG               fx_media_directory_next_entry_finds;
    ULONG               fx_media_directory_next_full_entry_finds;
    ULONG               fx_media_directory_renames;
    ULONG               fx_media_file_allocates;
    ULONG               fx_media_file_attributes_reads;
    ULONG               fx_media_file_attributes_sets;
    ULONG               fx_media_file_best_effort_allocates;
    ULONG               fx_media_file_closes;
    ULONG               fx_media_file_creates;
    ULONG               fx_media_file_deletes;
    ULONG               fx_media_file_opens;
    ULONG               fx_media_file_reads;
    ULONG               fx_media_file_relative_seeks;
    ULONG               fx_media_file_renames;
    ULONG               fx_media_file_seeks;
    ULONG               fx_media_file_truncates;
    ULONG               fx_media_file_truncate_releases;
    ULONG               fx_media_file_writes;
    ULONG               fx_media_aborts;
    ULONG               fx_media_flushes;
    ULONG               fx_media_reads;
    ULONG               fx_media_writes;
    ULONG               fx_media_directory_entry_reads;
    ULONG               fx_media_directory_entry_writes;
    ULONG               fx_media_directory_searches;
    ULONG               fx_media_directory_free_searches;
    ULONG               fx_media_fat_entry_reads;
    ULONG               fx_media_fat_entry_writes;
    ULONG               fx_media_fat_entry_cache_read_hits;
    ULONG               fx_media_fat_entry_cache_read_misses;
    ULONG               fx_media_fat_entry_cache_write_hits;
    ULONG               fx_media_fat_entry_cache_write_misses;
    ULONG               fx_media_fat_cache_flushes;
    ULONG               fx_media_fat_sector_reads;
    ULONG               fx_media_fat_sector_writes;
    ULONG               fx_media_logical_sector_reads;
    ULONG               fx_media_logical_sector_writes;
    ULONG               fx_media_logical_sector_cache_read_hits;
    ULONG               fx_media_logical_sector_cache_read_misses;
    ULONG               fx_media_driver_read_requests;
    ULONG               fx_media_driver_write_requests;
    ULONG               fx_media_driver_boot_read_requests;
    ULONG               fx_media_driver_boot_write_requests;
    ULONG               fx_media_driver_release_sectors_requests;
    ULONG               fx_media_driver_flush_requests;
#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE
    ULONG               fx_media_directory_search_cache_hits;
#endif
#endif

    /* Define the media's protection object, which is a ThreadX mutex.
       Only one thread is allowed to access any media or associated files
       at a time.  If FX_SINGLE_THREAD is defined, the FileX services are
       going to be called from only one thread, hence the protection is
       not needed.  */
#ifndef FX_SINGLE_THREAD
    TX_MUTEX            fx_media_protect;
#endif

#ifndef FX_MEDIA_DISABLE_SEARCH_CACHE

    /* Define the information used to remember the last directory entry found through
       searching or walking the directory via directory entry next. This information
       will be used to eliminate multiple searches for the same directory entry if
       the accesses are done sequentially.  */
    UINT                fx_media_last_found_directory_valid;
    FX_DIR_ENTRY        fx_media_last_found_directory;
    FX_DIR_ENTRY        fx_media_last_found_entry;
    CHAR                fx_media_last_found_file_name[FX_MAX_LONG_NAME_LEN];
    CHAR                fx_media_last_found_name[FX_MAX_LAST_NAME_LEN];
#endif

    /* Define the current directory information for the media.  */
    FX_PATH             fx_media_default_path;

    /* Define FAT entry cache and the variable used to index the cache.  */
    FX_FAT_CACHE_ENTRY  fx_media_fat_cache[FX_MAX_FAT_CACHE];

    /* Define the FAT secondary update map.  This will be used on flush and
       close to update sectors of any secondary FATs in the media.  */
    UCHAR               fx_media_fat_secondary_update_map[FX_FAT_MAP_SIZE];

    /* Define a variable for the application's use.  */
    ALIGN_TYPE          fx_media_reserved_for_user;

    /* Define an area to allocate long file names so that local storage on
       calling thread's stack is not used for long file names.  This helps
       reduce the amount of thread stack space needed when using FileX.  */
    CHAR                fx_media_name_buffer[4*FX_MAX_LONG_NAME_LEN];

#ifdef FX_RENAME_PATH_INHERIT

    /* Define the file and directory rename buffer that will be used to prepend
       paths when necessary to the target file name.  */
    CHAR                fx_media_rename_buffer[FX_MAXIMUM_PATH];
#endif

#ifndef FX_DISABLE_CACHE
    /* Define the sector cache control structures for this media.  */
    struct FX_CACHED_SECTOR_STRUCT
                        fx_media_sector_cache[FX_MAX_SECTOR_CACHE];

    /* Define the sector cache hash mask so that the hash algorithm can be used with
       any power of 2 number of cache sectors.  */
    ULONG               fx_media_sector_cache_hash_mask;
#endif /* FX_DISABLE_CACHE */

    /* Define a variable to disable burst cache. This is used by the underlying
       driver.  */
    ULONG               fx_media_disable_burst_cache;

#ifdef FX_ENABLE_FAULT_TOLERANT

    /* Fault tolerant information */
    /* Indicate whether fault tolerant is enabled. */

    UCHAR               fx_media_fault_tolerant_enabled;

    /* State of fault tolerant operation. */
    UCHAR               fx_media_fault_tolerant_state;

    /* Transaction recursive count. */
    USHORT              fx_media_fault_tolerant_transaction_count;

    /* Start cluster of log. */
    ULONG               fx_media_fault_tolerant_start_cluster;

    /* Count of consecutive clusters of log. */
    ULONG               fx_media_fault_tolerant_clusters;

    /* Count of total logs. */
    ULONG               fx_media_fault_tolerant_total_logs;

    /* Pointer to the memory buffer area used for fault tolerant operations.  */
    UCHAR              *fx_media_fault_tolerant_memory_buffer;

    /* Size of memory buffer area used for fault tolerant operations. */
    ULONG               fx_media_fault_tolerant_memory_buffer_size;

    /* Size of log file. */
    ULONG               fx_media_fault_tolerant_file_size;

    /* Memory space used during the release of FAT list. */
    ULONG               fx_media_fault_tolerant_cache[FX_FAULT_TOLERANT_CACHE_SIZE >> 2];

    /* Sector number of cached FAT entries. */
    ULONG               fx_media_fault_tolerant_cached_FAT_sector;
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Reserved value of FAT table. */
    ULONG               fx_media_fat_reserved;

    /* Last value of FAT table. */
    ULONG               fx_media_fat_last;

    /* Media geometry structure */
    UCHAR               fx_media_FAT_type;

    /* Define the module port extension in the media control block. This 
       is typically defined to whitespace in fx_port.h.  */
    FX_MEDIA_MODULE_EXTENSION

} FX_MEDIA;

typedef FX_MEDIA *      FX_MEDIA_PTR;


/* Determine if the file control block has an extension defined. If not, 
   define the extension to whitespace.  */

#ifndef FX_FILE_MODULE_EXTENSION
#define FX_FILE_MODULE_EXTENSION
#endif


/* Define the FileX file control block.  All information about open
   files are found in this data type.  */

typedef struct FX_FILE_STRUCT
{

    /* Define the file ID used for error checking.  */
    ULONG               fx_file_id;

    /* Define the file's name.  */
    CHAR                *fx_file_name;

    /* Define the open mode request.  */
    ULONG               fx_file_open_mode;

    /* Define the file modified field.  */
    UCHAR               fx_file_modified;

    /* Define the data storage parameters.  */
    ULONG               fx_file_total_clusters;
    ULONG               fx_file_first_physical_cluster;
    ULONG               fx_file_consecutive_cluster;
    ULONG               fx_file_last_physical_cluster;
    ULONG               fx_file_current_physical_cluster;
    ULONG64             fx_file_current_logical_sector;
    ULONG               fx_file_current_logical_offset;
    ULONG               fx_file_current_relative_cluster;
    ULONG               fx_file_current_relative_sector;
    ULONG64             fx_file_current_file_offset;
    ULONG64             fx_file_current_file_size;
    ULONG64             fx_file_current_available_size;
#ifdef FX_ENABLE_FAULT_TOLERANT
    ULONG64             fx_file_maximum_size_used;
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* Remember the media that is associated with this file. */
    FX_MEDIA            *fx_file_media_ptr;

    /* Define the pointers necessary to maintain the open file on
       the list of open files.  */
    struct FX_FILE_STRUCT
                        *fx_file_opened_next,
                        *fx_file_opened_previous;

    /* Define the complete directory entry structure.  */
    FX_DIR_ENTRY        fx_file_dir_entry;
    CHAR                fx_file_name_buffer[FX_MAX_LONG_NAME_LEN];

    /* Define a variable for the application's use */
    ULONG               fx_file_disable_burst_cache;

    /* Define a notify function called when file is written to. */
    VOID               (*fx_file_write_notify)(struct FX_FILE_STRUCT *);

    /* Define the module port extension in the file control block. This 
       is typically defined to whitespace in fx_port.h.  */
    FX_FILE_MODULE_EXTENSION

} FX_FILE;

typedef FX_FILE  *FX_FILE_PTR;


/* Define the FileX API mappings based on the error checking
   selected by the user.  Note: this section is only applicable to
   application source code, hence the conditional that turns off this
   stuff when the include file is processed by the FileX source. */

#ifndef FX_SOURCE_CODE

#ifdef FX_DISABLE_ONE_LINE_FUNCTION
#define fx_file_seek(f, b)                    fx_file_extended_seek(f, (ULONG64)b)
#define fx_file_allocate(f, s)                fx_file_extended_allocate(f, (ULONG64)s);
#define fx_file_truncate(f, s)                fx_file_extended_truncate(f, (ULONG64)s);
#define fx_file_relative_seek(f, b, sf)       fx_file_extended_relative_seek(f, (ULONG64)b, sf);
#define fx_file_truncate_release(f, s)        fx_file_extended_truncate_release(f, (ULONG64)s);
#endif /* FX_DISABLE_ONE_LINE_FUNCTION */

/* Determine if error checking is desired.  If so, map API functions
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work.
   Note: error checking is enabled by default.  */

#ifdef FX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define fx_directory_attributes_read          _fx_directory_attributes_read
#define fx_directory_attributes_set           _fx_directory_attributes_set
#define fx_directory_create                   _fx_directory_create
#define fx_directory_default_get              _fx_directory_default_get
#define fx_directory_default_get_copy         _fx_directory_default_get_copy
#define fx_directory_default_set              _fx_directory_default_set
#define fx_directory_delete                   _fx_directory_delete
#define fx_directory_first_entry_find         _fx_directory_first_entry_find
#define fx_directory_first_full_entry_find    _fx_directory_first_full_entry_find
#define fx_directory_information_get          _fx_directory_information_get
#define fx_directory_local_path_clear         _fx_directory_local_path_clear
#define fx_directory_local_path_get           _fx_directory_local_path_get
#define fx_directory_local_path_get_copy      _fx_directory_local_path_get_copy
#define fx_directory_local_path_restore       _fx_directory_local_path_restore
#define fx_directory_local_path_set           _fx_directory_local_path_set
#define fx_directory_long_name_get            _fx_directory_long_name_get
#define fx_directory_long_name_get_extended   _fx_directory_long_name_get_extended
#define fx_directory_name_test                _fx_directory_name_test
#define fx_directory_next_entry_find          _fx_directory_next_entry_find
#define fx_directory_next_full_entry_find     _fx_directory_next_full_entry_find
#define fx_directory_rename                   _fx_directory_rename
#define fx_directory_short_name_get           _fx_directory_short_name_get
#define fx_directory_short_name_get_extended  _fx_directory_short_name_get_extended

#ifndef FX_DISABLE_ONE_LINE_FUNCTION
#define fx_file_allocate                      _fx_file_allocate
#endif /* FX_DISABLE_ONE_LINE_FUNCTION */
#define fx_file_attributes_read               _fx_file_attributes_read
#define fx_file_attributes_set                _fx_file_attributes_set
#define fx_file_best_effort_allocate          _fx_file_best_effort_allocate
#define fx_file_close                         _fx_file_close
#define fx_file_create                        _fx_file_create
#define fx_file_date_time_set                 _fx_file_date_time_set
#define fx_file_delete                        _fx_file_delete
#define fx_file_open                          _fx_file_open
#define fx_file_read                          _fx_file_read
#ifndef FX_DISABLE_ONE_LINE_FUNCTION
#define fx_file_relative_seek                 _fx_file_relative_seek
#endif /* FX_DISABLE_ONE_LINE_FUNCTION */
#define fx_file_rename                        _fx_file_rename
#ifndef FX_DISABLE_ONE_LINE_FUNCTION
#define fx_file_seek                          _fx_file_seek
#define fx_file_truncate                      _fx_file_truncate
#define fx_file_truncate_release              _fx_file_truncate_release
#endif /* FX_DISABLE_ONE_LINE_FUNCTION */
#define fx_file_write                         _fx_file_write
#define fx_file_write_notify_set              _fx_file_write_notify_set
#define fx_file_extended_allocate             _fx_file_extended_allocate
#define fx_file_extended_best_effort_allocate _fx_file_extended_best_effort_allocate
#define fx_file_extended_relative_seek        _fx_file_extended_relative_seek
#define fx_file_extended_seek                 _fx_file_extended_seek
#define fx_file_extended_truncate             _fx_file_extended_truncate
#define fx_file_extended_truncate_release     _fx_file_extended_truncate_release

#define fx_media_abort                        _fx_media_abort
#define fx_media_cache_invalidate             _fx_media_cache_invalidate
#define fx_media_check                        _fx_media_check
#define fx_media_close                        _fx_media_close
#define fx_media_flush                        _fx_media_flush
#define fx_media_format                       _fx_media_format
#ifdef FX_ENABLE_EXFAT
#define fx_media_exFAT_format                 _fx_media_exFAT_format
#endif /* FX_ENABLE_EXFAT */
#define fx_media_open                         _fx_media_open
#define fx_media_read                         _fx_media_read
#define fx_media_space_available              _fx_media_space_available
#define fx_media_volume_get                   _fx_media_volume_get
#define fx_media_volume_get_extended          _fx_media_volume_get_extended
#define fx_media_volume_set                   _fx_media_volume_set
#define fx_media_write                        _fx_media_write
#define fx_media_open_notify_set              _fx_media_open_notify_set
#define fx_media_close_notify_set             _fx_media_close_notify_set
#define fx_media_extended_space_available     _fx_media_extended_space_available

#define fx_unicode_directory_create           _fx_unicode_directory_create
#define fx_unicode_directory_rename           _fx_unicode_directory_rename
#define fx_unicode_file_create                _fx_unicode_file_create
#define fx_unicode_file_rename                _fx_unicode_file_rename
#define fx_unicode_length_get                 _fx_unicode_length_get
#define fx_unicode_length_get_extended        _fx_unicode_length_get_extended
#define fx_unicode_name_get                   _fx_unicode_name_get
#define fx_unicode_name_get_extended          _fx_unicode_name_get_extended
#define fx_unicode_short_name_get             _fx_unicode_short_name_get
#define fx_unicode_short_name_get_extended    _fx_unicode_short_name_get_extended

#define fx_system_date_get                    _fx_system_date_get
#define fx_system_date_set                    _fx_system_date_set
#define fx_system_time_get                    _fx_system_time_get
#define fx_system_time_set                    _fx_system_time_set
#define fx_system_initialize                  _fx_system_initialize

#ifdef FX_ENABLE_FAULT_TOLERANT
#define fx_fault_tolerant_enable              _fx_fault_tolerant_enable
#endif /* FX_ENABLE_FAULT_TOLERANT */

#else

/* Services with error checking.  */

#define fx_directory_attributes_read          _fxe_directory_attributes_read
#define fx_directory_attributes_set           _fxe_directory_attributes_set
#define fx_directory_create                   _fxe_directory_create
#define fx_directory_default_get              _fxe_directory_default_get
#define fx_directory_default_get_copy         _fxe_directory_default_get_copy
#define fx_directory_default_set              _fxe_directory_default_set
#define fx_directory_delete                   _fxe_directory_delete
#define fx_directory_first_entry_find         _fxe_directory_first_entry_find
#define fx_directory_first_full_entry_find    _fxe_directory_first_full_entry_find
#define fx_directory_information_get          _fxe_directory_information_get
#define fx_directory_local_path_clear         _fxe_directory_local_path_clear
#define fx_directory_local_path_get           _fxe_directory_local_path_get
#define fx_directory_local_path_get_copy      _fxe_directory_local_path_get_copy
#define fx_directory_local_path_restore       _fxe_directory_local_path_restore
#define fx_directory_local_path_set(m, l, n)  _fxe_directory_local_path_set(m, l, n, sizeof(FX_LOCAL_PATH))
#define fx_directory_long_name_get            _fxe_directory_long_name_get
#define fx_directory_long_name_get_extended   _fxe_directory_long_name_get_extended
#define fx_directory_name_test                _fxe_directory_name_test
#define fx_directory_next_entry_find          _fxe_directory_next_entry_find
#define fx_directory_next_full_entry_find     _fxe_directory_next_full_entry_find
#define fx_directory_rename                   _fxe_directory_rename
#define fx_directory_short_name_get           _fxe_directory_short_name_get
#define fx_directory_short_name_get_extended  _fxe_directory_short_name_get_extended

#ifndef FX_DISABLE_ONE_LINE_FUNCTION
#define fx_file_allocate                      _fxe_file_allocate
#endif  /* FX_DISABLE_ONE_LINE_FUNCTION */
#define fx_file_attributes_read               _fxe_file_attributes_read
#define fx_file_attributes_set                _fxe_file_attributes_set
#define fx_file_best_effort_allocate          _fxe_file_best_effort_allocate
#define fx_file_close                         _fxe_file_close
#define fx_file_create                        _fxe_file_create
#define fx_file_date_time_set                 _fxe_file_date_time_set
#define fx_file_delete                        _fxe_file_delete
#define fx_file_open(m, f, n, t)              _fxe_file_open(m, f, n, t, sizeof(FX_FILE))
#define fx_file_read                          _fxe_file_read
#ifndef FX_DISABLE_ONE_LINE_FUNCTION
#define fx_file_relative_seek                 _fxe_file_relative_seek
#endif /* FX_DISABLE_ONE_LINE_FUNCTION */
#define fx_file_rename                        _fxe_file_rename
#ifndef FX_DISABLE_ONE_LINE_FUNCTION
#define fx_file_seek                          _fxe_file_seek
#define fx_file_truncate                      _fxe_file_truncate
#define fx_file_truncate_release              _fxe_file_truncate_release
#endif /* FX_DISABLE_ONE_LINE_FUNCTION */
#define fx_file_write                         _fxe_file_write
#define fx_file_write_notify_set              _fxe_file_write_notify_set
#define fx_file_extended_allocate             _fxe_file_extended_allocate
#define fx_file_extended_best_effort_allocate _fxe_file_extended_best_effort_allocate
#define fx_file_extended_relative_seek        _fxe_file_extended_relative_seek
#define fx_file_extended_seek                 _fxe_file_extended_seek
#define fx_file_extended_truncate             _fxe_file_extended_truncate
#define fx_file_extended_truncate_release     _fxe_file_extended_truncate_release

#define fx_media_abort                        _fxe_media_abort
#define fx_media_cache_invalidate             _fxe_media_cache_invalidate
#define fx_media_check                        _fxe_media_check
#define fx_media_close                        _fxe_media_close
#define fx_media_flush                        _fxe_media_flush
#define fx_media_format                       _fxe_media_format
#ifdef FX_ENABLE_EXFAT
#define fx_media_exFAT_format                 _fxe_media_exFAT_format
#endif /* FX_ENABLE_EXFAT */
#define fx_media_open(m, n, d, i, p, s)       _fxe_media_open(m, n, d, i, p, s, sizeof(FX_MEDIA))
#define fx_media_read                         _fxe_media_read
#define fx_media_space_available              _fxe_media_space_available
#define fx_media_volume_get                   _fxe_media_volume_get
#define fx_media_volume_get_extended          _fxe_media_volume_get_extended
#define fx_media_volume_set                   _fxe_media_volume_set
#define fx_media_write                        _fxe_media_write
#define fx_media_open_notify_set              _fxe_media_open_notify_set
#define fx_media_close_notify_set             _fxe_media_close_notify_set
#define fx_media_extended_space_available     _fxe_media_extended_space_available

#define fx_unicode_directory_create           _fxe_unicode_directory_create
#define fx_unicode_directory_rename           _fxe_unicode_directory_rename
#define fx_unicode_file_create                _fxe_unicode_file_create
#define fx_unicode_file_rename                _fxe_unicode_file_rename
#define fx_unicode_length_get                 _fx_unicode_length_get
#define fx_unicode_length_get_extended        _fx_unicode_length_get_extended
#define fx_unicode_name_get                   _fxe_unicode_name_get
#define fx_unicode_name_get_extended          _fxe_unicode_name_get_extended
#define fx_unicode_short_name_get             _fxe_unicode_short_name_get
#define fx_unicode_short_name_get_extended    _fxe_unicode_short_name_get_extended

#define fx_system_date_get                    _fxe_system_date_get
#define fx_system_date_set                    _fxe_system_date_set
#define fx_system_time_get                    _fxe_system_time_get
#define fx_system_time_set                    _fxe_system_time_set
#define fx_system_initialize                  _fx_system_initialize

#ifdef FX_ENABLE_FAULT_TOLERANT
#define fx_fault_tolerant_enable              _fxe_fault_tolerant_enable
#endif /* FX_ENABLE_FAULT_TOLERANT */

#endif

/* Define the function prototypes of the FileX API.  */

UINT fx_directory_attributes_read(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes_ptr);
UINT fx_directory_attributes_set(FX_MEDIA *media_ptr, CHAR *directory_name, UINT attributes);
UINT fx_directory_create(FX_MEDIA *media_ptr, CHAR *directory_name);
UINT fx_directory_default_get(FX_MEDIA *media_ptr, CHAR **return_path_name);
UINT fx_directory_default_get_copy(FX_MEDIA *media_ptr, CHAR *return_path_name_buffer, UINT return_path_name_buffer_size);
UINT fx_directory_default_set(FX_MEDIA *media_ptr, CHAR *new_path_name);
UINT fx_directory_delete(FX_MEDIA *media_ptr, CHAR *directory_name);
UINT fx_directory_first_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name);
UINT fx_directory_first_full_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes,
                                        ULONG *size, UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute, UINT *second);
UINT fx_directory_information_get(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes, ULONG *size,
                                  UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute, UINT *second);
UINT fx_directory_local_path_clear(FX_MEDIA *media_ptr);
UINT fx_directory_local_path_get(FX_MEDIA *media_ptr, CHAR **return_path_name);
UINT fx_directory_local_path_get_copy(FX_MEDIA *media_ptr, CHAR *return_path_name_buffer, UINT return_path_name_buffer_size);
UINT fx_directory_local_path_restore(FX_MEDIA *media_ptr, FX_LOCAL_PATH *local_path_ptr);
#ifdef FX_DISABLE_ERROR_CHECKING
UINT _fx_directory_local_path_set(FX_MEDIA *media_ptr, FX_LOCAL_PATH *local_path_ptr, CHAR *new_path_name);
#else
UINT _fxe_directory_local_path_set(FX_MEDIA *media_ptr, FX_LOCAL_PATH *local_path_ptr, CHAR *new_path_name, UINT local_path_control_block_size);
#endif
UINT fx_directory_long_name_get(FX_MEDIA *media_ptr, CHAR *short_file_name, CHAR *long_file_name);
UINT fx_directory_long_name_get_extended(FX_MEDIA* media_ptr, CHAR* short_file_name, CHAR* long_file_name, UINT long_file_name_buffer_length);
UINT fx_directory_name_test(FX_MEDIA *media_ptr, CHAR *directory_name);
UINT fx_directory_next_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name);
UINT fx_directory_next_full_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes,
                                       ULONG *size, UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute, UINT *second);
UINT fx_directory_rename(FX_MEDIA *media_ptr, CHAR *old_directory_name, CHAR *new_directory_name);
UINT fx_directory_short_name_get(FX_MEDIA *media_ptr, CHAR *long_file_name, CHAR *short_file_name);
UINT fx_directory_short_name_get_extended(FX_MEDIA* media_ptr, CHAR* long_file_name, CHAR* short_file_name, UINT short_file_name_length);

#ifndef FX_DISABLE_ONE_LINE_FUNCTION
UINT fx_file_allocate(FX_FILE *file_ptr, ULONG size);
#endif /* FX_DISABLE_ONE_LINE_FUNCTION*/
UINT fx_file_attributes_read(FX_MEDIA *media_ptr, CHAR *file_name, UINT *attributes_ptr);
UINT fx_file_attributes_set(FX_MEDIA *media_ptr, CHAR *file_name, UINT attributes);
UINT fx_file_best_effort_allocate(FX_FILE *file_ptr, ULONG size, ULONG *actual_size_allocated);
UINT fx_file_close(FX_FILE *file_ptr);
UINT fx_file_create(FX_MEDIA *media_ptr, CHAR *file_name);
UINT fx_file_date_time_set(FX_MEDIA *media_ptr, CHAR *file_name,
                           UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second);
UINT fx_file_delete(FX_MEDIA *media_ptr, CHAR *file_name);
#ifdef FX_DISABLE_ERROR_CHECKING
UINT _fx_file_open(FX_MEDIA *media_ptr, FX_FILE *file_ptr, CHAR *file_name,
                   UINT open_type);
#else
UINT _fxe_file_open(FX_MEDIA *media_ptr, FX_FILE *file_ptr, CHAR *file_name,
                    UINT open_type, UINT file_control_block_size);
#endif
UINT fx_file_read(FX_FILE *file_ptr, VOID *buffer_ptr, ULONG request_size, ULONG *actual_size);
#ifndef FX_DISABLE_ONE_LINE_FUNCTION
UINT fx_file_relative_seek(FX_FILE *file_ptr, ULONG byte_offset, UINT seek_from);
#endif /* FX_DISABLE_ONE_LINE_FUNCTION */
UINT fx_file_rename(FX_MEDIA *media_ptr, CHAR *old_file_name, CHAR *new_file_name);
#ifndef FX_DISABLE_ONE_LINE_FUNCTION
UINT fx_file_seek(FX_FILE *file_ptr, ULONG byte_offset);
UINT fx_file_truncate(FX_FILE *file_ptr, ULONG size);
UINT fx_file_truncate_release(FX_FILE *file_ptr, ULONG size);
#endif /* FX_DISABLE_ONE_LINE_FUNCTION */
UINT fx_file_write(FX_FILE *file_ptr, VOID *buffer_ptr, ULONG size);
UINT fx_file_write_notify_set(FX_FILE *file_ptr, VOID (*file_write_notify)(FX_FILE *));
UINT fx_file_extended_allocate(FX_FILE *file_ptr, ULONG64 size);
UINT fx_file_extended_best_effort_allocate(FX_FILE *file_ptr, ULONG64 size, ULONG64 *actual_size_allocated);
UINT fx_file_extended_relative_seek(FX_FILE *file_ptr, ULONG64 byte_offset, UINT seek_from);
UINT fx_file_extended_seek(FX_FILE *file_ptr, ULONG64 byte_offset);
UINT fx_file_extended_truncate(FX_FILE *file_ptr, ULONG64 size);
UINT fx_file_extended_truncate_release(FX_FILE *file_ptr, ULONG64 size);

UINT fx_media_abort(FX_MEDIA *media_ptr);
UINT fx_media_cache_invalidate(FX_MEDIA *media_ptr);
UINT fx_media_check(FX_MEDIA *media_ptr, UCHAR *scratch_memory_ptr, ULONG scratch_memory_size, ULONG error_correction_option, ULONG *errors_detected);
UINT fx_media_close(FX_MEDIA *media_ptr);
UINT fx_media_flush(FX_MEDIA *media_ptr);
UINT fx_media_format(FX_MEDIA *media_ptr, VOID (*driver)(FX_MEDIA *media), VOID *driver_info_ptr, UCHAR *memory_ptr, UINT memory_size,
                     CHAR *volume_name, UINT number_of_fats, UINT directory_entries, UINT hidden_sectors,
                     ULONG total_sectors, UINT bytes_per_sector, UINT sectors_per_cluster,
                     UINT heads, UINT sectors_per_track);
#ifdef FX_ENABLE_EXFAT
UINT fx_media_exFAT_format(FX_MEDIA *media_ptr, VOID (*driver)(FX_MEDIA *media), VOID *driver_info_ptr, UCHAR *memory_ptr, UINT memory_size,
                           CHAR *volume_name, UINT number_of_fats, ULONG64 hidden_sectors, ULONG64 total_sectors,
                           UINT bytes_per_sector, UINT sectors_per_cluster, UINT volume_serial_number, UINT boundary_unit);
#endif /* FX_ENABLE_EXFAT */
#ifdef FX_DISABLE_ERROR_CHECKING
UINT _fx_media_open(FX_MEDIA *media_ptr, CHAR *media_name,
                    VOID (*media_driver)(FX_MEDIA *), VOID *driver_info_ptr,
                    VOID *memory_ptr, ULONG memory_size);
#else
UINT _fxe_media_open(FX_MEDIA *media_ptr, CHAR *media_name,
                     VOID (*media_driver)(FX_MEDIA *), VOID *driver_info_ptr,
                     VOID *memory_ptr, ULONG memory_size, UINT media_control_block_size);
#endif
UINT fx_media_read(FX_MEDIA *media_ptr, ULONG logical_sector, VOID *buffer_ptr);
UINT fx_media_space_available(FX_MEDIA *media_ptr, ULONG *available_bytes_ptr);
UINT fx_media_volume_get(FX_MEDIA *media_ptr, CHAR *volume_name, UINT volume_source);
UINT fx_media_volume_get_extended(FX_MEDIA *media_ptr, CHAR *volume_name, UINT volume_name_buffer_length, UINT volume_source);
UINT fx_media_volume_set(FX_MEDIA *media_ptr, CHAR *volume_name);
UINT fx_media_write(FX_MEDIA *media_ptr, ULONG logical_sector, VOID *buffer_ptr);
UINT fx_media_open_notify_set(FX_MEDIA *media_ptr, VOID (*media_open_notify)(FX_MEDIA *));
UINT fx_media_close_notify_set(FX_MEDIA *media_ptr, VOID (*media_close_notify)(FX_MEDIA *));
UINT fx_media_extended_space_available(FX_MEDIA *media_ptr, ULONG64 *available_bytes_ptr);

UINT fx_system_date_get(UINT *year, UINT *month, UINT *day);
UINT fx_system_date_set(UINT year, UINT month, UINT day);
VOID fx_system_initialize(VOID);
UINT fx_system_time_get(UINT *hour, UINT *minute, UINT *second);
UINT fx_system_time_set(UINT hour, UINT minute, UINT second);

UINT fx_unicode_directory_create(FX_MEDIA *media_ptr,
                                 UCHAR *source_unicode_name, ULONG source_unicode_length,
                                 CHAR *short_name);
UINT fx_unicode_directory_rename(FX_MEDIA *media_ptr,
                                 UCHAR *old_unicode_name, ULONG old_unicode_length,
                                 UCHAR *new_unicode_name, ULONG new_unicode_length,
                                 CHAR *new_short_name);
UINT fx_unicode_file_create(FX_MEDIA *media_ptr,
                            UCHAR *source_unicode_name, ULONG source_unicode_length,
                            CHAR *short_name);
UINT  fx_unicode_file_rename(FX_MEDIA *media_ptr, UCHAR *old_unicode_name, ULONG old_unicode_length,
                             UCHAR *new_unicode_name, ULONG new_unicode_length, CHAR *new_short_name);
ULONG fx_unicode_length_get(UCHAR *unicode_name);
ULONG fx_unicode_length_get_extended(UCHAR *unicode_name, UINT buffer_length);
UINT  fx_unicode_name_get(FX_MEDIA *media_ptr, CHAR *source_short_name,
                          UCHAR *destination_unicode_name, ULONG *destination_unicode_length);
UINT  fx_unicode_name_get_extended(FX_MEDIA *media_ptr, CHAR *source_short_name,
                          UCHAR *destination_unicode_name, ULONG *destination_unicode_length, ULONG unicode_name_buffer_length);
UINT  fx_unicode_short_name_get(FX_MEDIA *media_ptr,
                                UCHAR *source_unicode_name, ULONG source_unicode_length,
                                CHAR *destination_short_name);
UINT  fx_unicode_short_name_get_extended(FX_MEDIA *media_ptr,
                                UCHAR *source_unicode_name, ULONG source_unicode_length,
                                CHAR *destination_short_name, ULONG short_name_buffer_length);

#ifdef FX_ENABLE_FAULT_TOLERANT
UINT fx_fault_tolerant_enable(FX_MEDIA *media_ptr, VOID *memory_buffer, UINT memory_size);
#endif /* FX_ENABLE_FAULT_TOLERANT */


/* Define prototype for utility services commonly used by FileX I/O Drivers.  This eliminates the
   need to include internal FileX component files in I/O drivers.  */

UINT    _fx_utility_16_unsigned_read(UCHAR *source_ptr);
VOID    _fx_utility_16_unsigned_write(UCHAR *dest_ptr, UINT value);
ULONG   _fx_utility_32_unsigned_read(UCHAR *source_ptr);
VOID    _fx_utility_32_unsigned_write(UCHAR *dest_ptr, ULONG value);
ULONG64 _fx_utility_64_unsigned_read(UCHAR *source_ptr);
VOID    _fx_utility_64_unsigned_write(UCHAR *dest_ptr, ULONG64 value);
VOID    _fx_utility_memory_copy(UCHAR *source_ptr, UCHAR *dest_ptr, ULONG size);
VOID    _fx_utility_memory_set(UCHAR *dest_ptr, UCHAR value, ULONG size);

#endif

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef __cplusplus
}
#endif

#endif

