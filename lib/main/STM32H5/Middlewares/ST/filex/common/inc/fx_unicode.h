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
/**   Unicode                                                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    fx_unicode.h                                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the FileX Unicode component constants, data       */
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

#ifndef FX_UNICODE_H
#define FX_UNICODE_H


/* Define external data for the Unicode component.  */

extern UCHAR   _fx_unicode_temp_long_file_name[FX_MAX_LONG_NAME_LEN];
extern UCHAR _fx_unicode_search_name[FX_MAX_LONG_NAME_LEN * 2];


/* Define the external Unicode component function prototypes.  */

UINT  _fx_unicode_directory_create(FX_MEDIA *media_ptr, UCHAR *source_unicode_name, ULONG source_unicode_length,
                                   CHAR *short_name);
UINT  _fx_unicode_directory_rename(FX_MEDIA *media_ptr, UCHAR *old_unicode_name, ULONG old_unicode_length,
                                   UCHAR *new_unicode_name, ULONG new_unicode_length, CHAR *new_short_name);
UINT  _fx_unicode_file_create(FX_MEDIA *media_ptr, UCHAR *source_unicode_name, ULONG source_unicode_length,
                              CHAR *short_name);
ULONG _fx_unicode_length_get(UCHAR *unicode_name);
ULONG _fx_unicode_length_get_extended(UCHAR *unicode_name, UINT buffer_length);
UINT  _fx_unicode_name_get(FX_MEDIA *media_ptr, CHAR *source_short_name,
                           UCHAR *destination_unicode_name, ULONG *destination_unicode_length);
UINT  _fx_unicode_name_get_extended(FX_MEDIA *media_ptr, CHAR *source_short_name,
                           UCHAR *destination_unicode_name, ULONG *destination_unicode_length, ULONG unicode_name_buffer_length);
UINT  _fx_unicode_file_rename(FX_MEDIA *media_ptr, UCHAR *old_unicode_name, ULONG old_unicode_length,
                              UCHAR *new_unicode_name, ULONG new_unicode_length, CHAR *new_short_name);
UINT  _fx_unicode_short_name_get(FX_MEDIA *media_ptr, UCHAR *source_unicode_name, ULONG source_unicode_length,
                                 CHAR *destination_short_name);
UINT  _fx_unicode_short_name_get_extended(FX_MEDIA *media_ptr, UCHAR *source_unicode_name, ULONG source_unicode_length,
                                 CHAR *destination_short_name, ULONG short_name_buffer_length);

UINT _fxe_unicode_directory_create(FX_MEDIA *media_ptr, UCHAR *source_unicode_name, ULONG source_unicode_length,
                                   CHAR *short_name);
UINT _fxe_unicode_directory_rename(FX_MEDIA *media_ptr, UCHAR *old_unicode_name, ULONG old_unicode_length,
                                   UCHAR *new_unicode_name, ULONG new_unicode_length, CHAR *new_short_name);
UINT _fxe_unicode_file_create(FX_MEDIA *media_ptr, UCHAR *source_unicode_name, ULONG source_unicode_length,
                              CHAR *short_name);
UINT _fxe_unicode_file_rename(FX_MEDIA *media_ptr, UCHAR *old_unicode_name, ULONG old_unicode_length,
                              UCHAR *new_unicode_name, ULONG new_unicode_length, CHAR *new_short_name);
UINT _fxe_unicode_name_get(FX_MEDIA *media_ptr, CHAR *source_short_name,
                           UCHAR *destination_unicode_name, ULONG *destination_unicode_length);
UINT _fxe_unicode_name_get_extended(FX_MEDIA *media_ptr, CHAR *source_short_name,
                           UCHAR *destination_unicode_name, ULONG *destination_unicode_length, ULONG unicode_name_buffer_length);
UINT _fxe_unicode_short_name_get(FX_MEDIA *media_ptr, UCHAR *source_unicode_name, ULONG source_unicode_length,
                                 CHAR *destination_short_name);
UINT _fxe_unicode_short_name_get_extended(FX_MEDIA *media_ptr, UCHAR *source_unicode_name, ULONG source_unicode_length,
                                 CHAR *destination_short_name, ULONG short_name_buffer_length);                                 

/* Define the internal Unicode component function prototypes.  */

UINT  _fx_unicode_directory_search(FX_MEDIA *media_ptr, FX_DIR_ENTRY *entry_ptr,
                                   UCHAR *short_name, ULONG short_name_buffer_length, 
                                   UCHAR *unicode_name, ULONG *unicode_name_length, ULONG unicode_name_buffer_length);
UINT _fx_unicode_directory_entry_read(FX_MEDIA *media_ptr, FX_DIR_ENTRY *source_dir,
                                      ULONG *entry_ptr, FX_DIR_ENTRY *destination_ptr,
                                      UCHAR *unicode_name, ULONG *unicode_size);
UINT _fx_unicode_directory_entry_change(FX_MEDIA *media_ptr, FX_DIR_ENTRY *entry_ptr, UCHAR *unicode_name, ULONG unicode_name_length);


#endif

