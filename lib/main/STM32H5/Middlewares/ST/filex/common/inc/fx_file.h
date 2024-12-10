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
/**   File                                                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    fx_file.h                                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the FileX File component constants, data          */
/*    definitions, and external references.  It is assumed that fx_api.h  */
/*    (and fx_port.h) have already been included.                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     William E. Lamie         Modified comment(s), and      */
/*                                            added conditional to        */
/*                                            disable one line function,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/

#ifndef FX_FILE_H
#define FX_FILE_H


/* Define the external File component function prototypes.  */

#ifndef FX_DISABLE_ONE_LINE_FUNCTION
UINT _fx_file_allocate(FX_FILE *file_ptr, ULONG size);
#else
#define _fx_file_allocate(f, s)                _fx_file_extended_allocate(f, (ULONG64)s);
#endif /* FX_DISABLE_ONE_LINE_FUNCTION */
UINT _fx_file_attributes_read(FX_MEDIA *media_ptr, CHAR *file_name, UINT *attributes_ptr);
UINT _fx_file_attributes_set(FX_MEDIA *media_ptr, CHAR *file_name, UINT attributes);
UINT _fx_file_best_effort_allocate(FX_FILE *file_ptr, ULONG size, ULONG *actual_size_allocated);
UINT _fx_file_close(FX_FILE *file_ptr);
UINT _fx_file_create(FX_MEDIA *media_ptr, CHAR *file_name);
UINT _fx_file_date_time_set(FX_MEDIA *media_ptr, CHAR *file_name,
                            UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second);
UINT _fx_file_delete(FX_MEDIA *media_ptr, CHAR *file_name);
UINT _fx_file_open(FX_MEDIA *media_ptr, FX_FILE *file_ptr, CHAR *file_name,
                   UINT open_type);
UINT _fx_file_read(FX_FILE *file_ptr, VOID *buffer_ptr, ULONG request_size, ULONG *actual_size);
#ifndef FX_DISABLE_ONE_LINE_FUNCTION
UINT _fx_file_relative_seek(FX_FILE *file_ptr, ULONG byte_offset, UINT seek_from);
#else
#define _fx_file_relative_seek(f, b, sf)       _fx_file_extended_relative_seek(f, (ULONG64)b, sf);
#endif /* FX_DISABLE_ONE_LINE_FUNCTION */
UINT _fx_file_rename(FX_MEDIA *media_ptr, CHAR *old_file_name, CHAR *new_file_name);
#ifndef FX_DISABLE_ONE_LINE_FUNCTION
UINT _fx_file_seek(FX_FILE *file_ptr, ULONG byte_offset);
UINT _fx_file_truncate(FX_FILE *file_ptr, ULONG size);
UINT _fx_file_truncate_release(FX_FILE *file_ptr, ULONG size);
#else
#define _fx_file_seek(f, b)                    _fx_file_extended_seek(f, (ULONG64)b)
#define _fx_file_truncate(f, s)                _fx_file_extended_truncate(f, (ULONG64)s);
#define _fx_file_truncate_release(f, s)        _fx_file_extended_truncate_release(f, (ULONG64)s);
#endif /* FX_DISABLE_ONE_LINE_FUNCTION */
UINT _fx_file_write(FX_FILE *file_ptr, VOID *buffer_ptr, ULONG size);
UINT _fx_file_write_notify_set(FX_FILE *file_ptr, VOID (*file_write_notify)(FX_FILE *));
UINT _fx_file_extended_allocate(FX_FILE *file_ptr, ULONG64 size);
UINT _fx_file_extended_best_effort_allocate(FX_FILE *file_ptr, ULONG64 size, ULONG64 *actual_size_allocated);
UINT _fx_file_extended_relative_seek(FX_FILE *file_ptr, ULONG64 byte_offset, UINT seek_from);
UINT _fx_file_extended_seek(FX_FILE *file_ptr, ULONG64 byte_offset);
UINT _fx_file_extended_truncate(FX_FILE *file_ptr, ULONG64 size);
UINT _fx_file_extended_truncate_release(FX_FILE *file_ptr, ULONG64 size);

UINT _fxe_file_allocate(FX_FILE *file_ptr, ULONG size);
UINT _fxe_file_attributes_read(FX_MEDIA *media_ptr, CHAR *file_name, UINT *attributes_ptr);
UINT _fxe_file_attributes_set(FX_MEDIA *media_ptr, CHAR *file_name, UINT attributes);
UINT _fxe_file_best_effort_allocate(FX_FILE *file_ptr, ULONG size, ULONG *actual_size_allocated);
UINT _fxe_file_close(FX_FILE *file_ptr);
UINT _fxe_file_create(FX_MEDIA *media_ptr, CHAR *file_name);
UINT _fxe_file_date_time_set(FX_MEDIA *media_ptr, CHAR *file_name,
                             UINT year, UINT month, UINT day, UINT hour, UINT minute, UINT second);
UINT _fxe_file_delete(FX_MEDIA *media_ptr, CHAR *file_name);
UINT _fxe_file_open(FX_MEDIA *media_ptr, FX_FILE *file_ptr, CHAR *file_name,
                    UINT open_type, UINT file_control_block_size);
UINT _fxe_file_read(FX_FILE *file_ptr, VOID *buffer_ptr, ULONG request_size, ULONG *actual_size);
UINT _fxe_file_relative_seek(FX_FILE *file_ptr, ULONG byte_offset, UINT seek_from);
UINT _fxe_file_rename(FX_MEDIA *media_ptr, CHAR *old_file_name, CHAR *new_file_name);
UINT _fxe_file_seek(FX_FILE *file_ptr, ULONG byte_offset);
UINT _fxe_file_truncate(FX_FILE *file_ptr, ULONG size);
UINT _fxe_file_truncate_release(FX_FILE *file_ptr, ULONG size);
UINT _fxe_file_write(FX_FILE *file_ptr, VOID *buffer_ptr, ULONG size);
UINT _fxe_file_write_notify_set(FX_FILE *file_ptr, VOID (*file_write_notify)(FX_FILE *));
UINT _fxe_file_extended_allocate(FX_FILE *file_ptr, ULONG64 size);
UINT _fxe_file_extended_best_effort_allocate(FX_FILE *file_ptr, ULONG64 size, ULONG64 *actual_size_allocated);
UINT _fxe_file_extended_relative_seek(FX_FILE *file_ptr, ULONG64 byte_offset, UINT seek_from);
UINT _fxe_file_extended_seek(FX_FILE *file_ptr, ULONG64 byte_offset);
UINT _fxe_file_extended_truncate(FX_FILE *file_ptr, ULONG64 size);
UINT _fxe_file_extended_truncate_release(FX_FILE *file_ptr, ULONG64 size);

#endif

