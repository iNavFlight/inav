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
/**   Utility                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    fx_utility.h                                        PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the FileX Utility component constants, data       */
/*    definitions, and external references.  It is assumed that fx_api.h  */
/*    (and fx_port.h) have already been included.                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Bhupendra Naphade        Modified comment(s), and      */
/*                                            changed prototype for       */
/*                                            exFAT size calculate        */
/*                                            utility function,           */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/

#ifndef FX_UTILITY_H
#define FX_UTILITY_H


/* Define the internal Utility component function prototypes.  */

UINT    _fx_utility_16_unsigned_read(UCHAR *source_ptr);
VOID    _fx_utility_16_unsigned_write(UCHAR *dest_ptr, UINT value);
ULONG   _fx_utility_32_unsigned_read(UCHAR *source_ptr);
VOID    _fx_utility_32_unsigned_write(UCHAR *dest_ptr, ULONG value);
ULONG64 _fx_utility_64_unsigned_read(UCHAR *source_ptr);
VOID    _fx_utility_64_unsigned_write(UCHAR *dest_ptr, ULONG64 value);
VOID    _fx_utility_memory_copy(UCHAR *source_ptr, UCHAR *dest_ptr, ULONG size);
VOID    _fx_utility_memory_set(UCHAR *dest_ptr, UCHAR value, ULONG size);
FX_CACHED_SECTOR
       *_fx_utility_logical_sector_cache_entry_read(FX_MEDIA *media_ptr, ULONG64 logical_sector, FX_CACHED_SECTOR **previous_cache_entry);
UINT    _fx_utility_logical_sector_read(FX_MEDIA *media_ptr, ULONG64 logical_sector,
                                        VOID *buffer_ptr, ULONG sectors, UCHAR sector_type);
UINT    _fx_utility_logical_sector_write(FX_MEDIA *media_ptr, ULONG64 logical_sector,
                                         VOID *buffer_ptr, ULONG sectors, UCHAR sector_type);
UINT    _fx_utility_logical_sector_flush(FX_MEDIA *media_ptr, ULONG64 starting_sector, ULONG64 sectors, UINT invalidate);
UINT    _fx_utility_FAT_entry_read(FX_MEDIA *media_ptr, ULONG cluster, ULONG *entry_ptr);
UINT    _fx_utility_FAT_entry_write(FX_MEDIA *media_ptr, ULONG cluster, ULONG next_cluster);
UINT    _fx_utility_FAT_flush(FX_MEDIA *media_ptr);
UINT    _fx_utility_FAT_map_flush(FX_MEDIA *media_ptr);
ULONG   _fx_utility_FAT_sector_get(FX_MEDIA *media_ptr, ULONG cluster);
UINT    _fx_utility_string_length_get(CHAR *string, UINT max_length);


#ifdef FX_ENABLE_EXFAT

/* Define the exFAT Utility function prototypes.  */

UINT _fx_utility_exFAT_cluster_free(FX_MEDIA *media_ptr, UCHAR *work_ptr);
UINT _fx_utility_exFAT_geometry_check(FX_MEDIA *media_ptr, UCHAR *sector_buffer);
UINT _fx_utility_exFAT_system_area_checksum_verify(FX_MEDIA_PTR media_ptr,
                                                   UCHAR *sector_buffer, ULONG boot_sector_offset,
                                                   ULONG *calculated_checksum);
VOID _fx_utility_exFAT_size_calculate(UINT bytes_per_sector, ULONG boundary_unit, ULONG64 size_in_sectors, ULONG sectors_per_cluster,
                                      ULONG *sectors_per_fat_ptr, ULONG *fat_offset_ptr, ULONG *cluster_heap_offset_ptr);
UINT _fx_utility_exFAT_system_area_format(FX_MEDIA *media_ptr, ULONG boundary_unit,
                                          ULONG *system_area_checksum_ptr, UCHAR *memory_ptr, UINT memory_size);
UINT _fx_utility_exFAT_system_sector_write(FX_MEDIA *media_ptr, UCHAR *data_buffer, ULONG64 logical_sector,
                                           ULONG sector_count, ULONG sector_type);
UINT _fx_utility_exFAT_system_area_checksum_write(FX_MEDIA *media_ptr, UCHAR *sector_buffer,
                                                  ULONG *system_area_checksum_ptr);


/* Define the exFAT utility function prototypes.  */

UINT   _fx_utility_exFAT_bitmap_start_sector_get(FX_MEDIA *media_ptr, ULONG *start_sector);
UINT   _fx_utility_exFAT_bitmap_cache_update(FX_MEDIA *media_ptr, ULONG cluster);
UINT   _fx_utility_exFAT_bitmap_flush(FX_MEDIA *media_ptr);
UINT   _fx_utility_exFAT_bitmap_initialize(FX_MEDIA *media_ptr);
UINT   _fx_utility_exFAT_bitmap_cache_prepare(FX_MEDIA *media_ptr, ULONG cluster);
UINT   _fx_utility_exFAT_cluster_state_get(FX_MEDIA *media_ptr, ULONG cluster, UCHAR *cluster_state);
UINT   _fx_utility_exFAT_cluster_state_set(FX_MEDIA *media_ptr, ULONG cluster, UCHAR new_cluster_state);
UINT   _fx_utility_exFAT_bitmap_free_cluster_find(FX_MEDIA *media_ptr, ULONG start, ULONG *free_cluster);
USHORT _fx_utility_exFAT_upcase_get(USHORT character);
USHORT _fx_utility_exFAT_name_hash_get(CHAR *name);
USHORT _fx_utility_exFAT_unicode_name_hash_get(CHAR *unicode_name, ULONG unicode_length);
USHORT _fx_utility_exFAT_upcase_get(USHORT character);
UINT   _fx_utility_absolute_path_get(CHAR *base_path, CHAR *new_path, CHAR *absolute_path);
UINT   _fx_utility_token_length_get(CHAR *path);
UINT   _fx_utility_exFAT_allocate_new_cluster(FX_MEDIA *media_ptr, FX_DIR_ENTRY *directory_ptr, ULONG *last_cluster, ULONG *cluster);


#endif /* FX_ENABLE_EXFAT */


#endif

