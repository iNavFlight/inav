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
/**   Media                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    fx_media.h                                          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the FileX Media component constants, data         */
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
/*                                                                        */
/**************************************************************************/

#ifndef FX_MEDIA_H
#define FX_MEDIA_H


/* Define the external Media component function prototypes.  */

UINT _fx_media_abort(FX_MEDIA *media_ptr);
UINT _fx_media_cache_invalidate(FX_MEDIA *media_ptr);
UINT _fx_media_check(FX_MEDIA *media_ptr, UCHAR *scratch_memory_ptr, ULONG scratch_memory_size, ULONG error_correction_option, ULONG *errors_detected);
UINT _fx_media_close(FX_MEDIA *media_ptr);
UINT _fx_media_flush(FX_MEDIA *media_ptr);
UINT _fx_media_format(FX_MEDIA *media_ptr, VOID (*driver)(FX_MEDIA *media), VOID *driver_info_ptr, UCHAR *memory_ptr, UINT memory_size,
                      CHAR *volume_name, UINT number_of_fats, UINT directory_entries, UINT hidden_sectors,
                      ULONG total_sectors, UINT bytes_per_sector, UINT sectors_per_cluster,
                      UINT heads, UINT sectors_per_track);
UINT _fx_media_exFAT_format(FX_MEDIA *media_ptr, VOID (*driver)(FX_MEDIA *media), VOID *driver_info_ptr, UCHAR *memory_ptr, UINT memory_size,
                            CHAR *volume_name, UINT number_of_fats, ULONG64 hidden_sectors, ULONG64 total_sectors, UINT bytes_per_sector, UINT sectors_per_cluster, UINT volume_serial_number, UINT boundary_unit);
UINT _fx_media_open(FX_MEDIA *media_ptr, CHAR *media_name,
                    VOID (*media_driver)(FX_MEDIA *), VOID *driver_info_ptr,
                    VOID *memory_ptr, ULONG memory_size);
UINT _fx_media_read(FX_MEDIA *media_ptr, ULONG logical_sector, VOID *buffer_ptr);
UINT _fx_media_space_available(FX_MEDIA *media_ptr, ULONG *available_bytes_ptr);
UINT _fx_media_volume_get(FX_MEDIA *media_ptr, CHAR *volume_name, UINT volume_source);
UINT _fx_media_volume_get_extended(FX_MEDIA *media_ptr, CHAR *volume_name, UINT volume_name_buffer_length, UINT volume_source);
UINT _fx_media_volume_set(FX_MEDIA *media_ptr, CHAR *volume_name);
UINT _fx_media_write(FX_MEDIA *media_ptr, ULONG logical_sector, VOID *buffer_ptr);
UINT _fx_media_open_notify_set(FX_MEDIA *media_ptr, VOID (*media_open_notify)(FX_MEDIA *));
UINT _fx_media_close_notify_set(FX_MEDIA *media_ptr, VOID (*media_close_notify)(FX_MEDIA *));
UINT _fx_media_extended_space_available(FX_MEDIA *media_ptr, ULONG64 *available_bytes_ptr);


UINT _fxe_media_abort(FX_MEDIA *media_ptr);
UINT _fxe_media_cache_invalidate(FX_MEDIA *media_ptr);
UINT _fxe_media_check(FX_MEDIA *media_ptr, UCHAR *scratch_memory_ptr, ULONG scratch_memory_size, ULONG error_correction_option, ULONG *errors_detected);
UINT _fxe_media_close(FX_MEDIA *media_ptr);
UINT _fxe_media_flush(FX_MEDIA *media_ptr);
UINT _fxe_media_format(FX_MEDIA *media_ptr, VOID (*driver)(FX_MEDIA *media), VOID *driver_info_ptr, UCHAR *memory_ptr, UINT memory_size,
                       CHAR *volume_name, UINT number_of_fats, UINT directory_entries, UINT hidden_sectors,
                       ULONG total_sectors, UINT bytes_per_sector, UINT sectors_per_cluster,
                       UINT heads, UINT sectors_per_track);
UINT _fxe_media_exFAT_format(FX_MEDIA *media_ptr, VOID (*driver)(FX_MEDIA *media), VOID *driver_info_ptr, UCHAR *memory_ptr, UINT memory_size,
                             CHAR *volume_name, UINT number_of_fats, ULONG64 hidden_sectors, ULONG64 total_sectors, UINT bytes_per_sector, UINT sectors_per_cluster, UINT volume_serial_number, UINT boundary_unit);
UINT _fxe_media_open(FX_MEDIA *media_ptr, CHAR *media_name,
                     VOID (*media_driver)(FX_MEDIA *), VOID *driver_info_ptr,
                     VOID *memory_ptr, ULONG memory_size, UINT media_control_block_size);
UINT _fxe_media_read(FX_MEDIA *media_ptr, ULONG logical_sector, VOID *buffer_ptr);
UINT _fxe_media_space_available(FX_MEDIA *media_ptr, ULONG *available_bytes_ptr);
UINT _fxe_media_volume_get(FX_MEDIA *media_ptr, CHAR *volume_name, UINT volume_source);
UINT _fxe_media_volume_get_extended(FX_MEDIA *media_ptr, CHAR *volume_name, UINT volume_name_buffer_length, UINT volume_source);
UINT _fxe_media_volume_set(FX_MEDIA *media_ptr, CHAR *volume_name);
UINT _fxe_media_write(FX_MEDIA *media_ptr, ULONG logical_sector, VOID *buffer_ptr);
UINT _fxe_media_open_notify_set(FX_MEDIA *media_ptr, VOID (*media_open_notify)(FX_MEDIA *));
UINT _fxe_media_close_notify_set(FX_MEDIA *media_ptr, VOID (*media_close_notify)(FX_MEDIA *));
UINT _fxe_media_extended_space_available(FX_MEDIA *media_ptr, ULONG64 *available_bytes_ptr);


/* Define the internal Media component function prototypes.  */

ULONG _fx_media_check_FAT_chain_check(FX_MEDIA *media_ptr, ULONG starting_cluster, ULONG *last_valid_cluster, ULONG *total_valid_clusters, UCHAR *logical_fat);
ULONG _fx_media_check_lost_cluster_check(FX_MEDIA *media_ptr, UCHAR *logical_fat, ULONG total_clusters, ULONG error_correction_option);
ULONG _fx_media_check_exFAT_lost_cluster_check(FX_MEDIA *media_ptr, UCHAR *logical_fat, ULONG total_clusters, ULONG error_correction_option);
UINT  _fx_media_boot_info_extract(FX_MEDIA *media_ptr);

#endif

