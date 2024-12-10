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
/**   Directory                                                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    fx_directory_exFAT.h                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the exFAT Directory entry component constants,    */
/*    data definitions, and external references.                          */
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

#ifndef FX_DIRECTORY_EXFAT_H
#define FX_DIRECTORY_EXFAT_H

#include "fx_api.h"

#define FX_EXFAT_DIR_ENTRY_TYPE_ALLOCATION_BITMAP      0x81
#define FX_EXFAT_DIR_ENTRY_TYPE_UP_CASE_TABLE          0x82
#define FX_EXFAT_DIR_ENTRY_TYPE_VOLUME_LABEL           0x83
#define FX_EXFAT_DIR_ENTRY_TYPE_FILE_DIRECTORY         0x85
#define FX_EXFAT_DIR_ENTRY_TYPE_VOLUME_GUID            0xA0
#define FX_EXFAT_DIR_ENTRY_TYPE_STREAM_EXTENSION       0xC0
#define FX_EXFAT_DIR_ENTRY_TYPE_FILE_NAME              0xC1
#define FX_EXFAT_DIR_ENTRY_TYPE_CONTINUOUS_INFO_MANAGE 0xE0
#define FX_EXFAT_DIR_ENTRY_TYPE_CONTINUOUS_INFO        0xE1
#define FX_EXFAT_DIR_ENTRY_TYPE_END_MARKER             0x00
#define FX_EXFAT_DIR_ENTRY_TYPE_FREE                   0xFF


/* Define the directory entry size.  */

#define FX_EXFAT_DIR_ENTRY_SIZE                        32


/* Define bit masks. */

#define FX_EXFAT_ENTRY_TYPE_CODE_MASK                  0x1F
#define FX_EXFAT_ENTRY_TYPE_IMPORTANCE_MASK            0x20
#define FX_EXFAT_ENTRY_TYPE_CATEGORY_MASK              0x40
#define FX_EXFAT_ENTRY_TYPE_IN_USE_MASK                0x80
#define FX_EXFAT_SECOND_FLAG_ALLOCATION_POSSIBLE_MASK  0x01


/* Define common primary and secondary directory entry structure.  */

#define FX_EXFAT_ENTRY_TYPE                            0     /* Size 1 byte */


/* Define primary-specific directory entry structure.  */

#define FX_EXFAT_SECOND_COUNT                          1     /* Size 1 byte */
#define FX_EXFAT_CHECK_SUM                             2     /* Size 2 bytes */
#define FX_EXFAT_PRIM_FLAG                             4     /* Size 2 bytes */


/* Define secondary-specific directory entry structure.  */

#define FX_EXFAT_SECOND_FLAG                           1     /* Size 1 byte */


/* Define common primary and secondary directory entry structure.  */

#define FX_EXFAT_FIRST_CLUSTER                         20    /* Size 4 bytes */
#define FX_EXFAT_DATA_LENGTH                           24    /* Size 8 bytes */


/* Define Allocation Bitmap directory entry.  */

#define FX_EXFAT_BIT_MAP_FLAGS                         1     /* Size 1 byte */


/* Define UP-case Table Entry specific.  */

#define FX_EXFAT_UP_CASE_TABLE_CHECK_SUM               4     /* Size 4 bytes */


/* Define Volume Label Entry specific.  */

#define FX_EXFAT_CHAR_COUNT                            1     /* Size 1 byte */
#define FX_EXFAT_VOLUME_LABEL                          2     /* Size 22 bytes */


/* Define File directory entry specific. This is a primary item.  */

#define FX_EXFAT_FILE_ATTR                             4     /* Size 2 bytes */
#define FX_EXFAT_CREATE_TIME                           8     /* Size 4 bytes */
#define FX_EXFAT_LAST_MODIFIED_TIME                    12    /* Size 4 bytes */
#define FX_EXFAT_LAST_ACCESSED_TIME                    16    /* Size 4 bytes */
#define FX_EXFAT_LAST_CREATE_10MS_INC                  20    /* Size 1 byte */
#define FX_EXFAT_LAST_MODIFIED_10MS_INC                21    /* Size 1 byte */
#define FX_EXFAT_CREATE_UTC_OFFSET                     22    /* Size 1 byte */
#define FX_EXFAT_LAST_MODIFIED_UTC_OFFSET              23    /* Size 1 byte */
#define FX_EXFAT_LAST_ACCESSED_UTC_OFFSET              24    /* Size 1 byte */


/* Define Volume GUID directory entry specific. This is a primary item.  */

#define FX_EXFAT_VOLUME_GUID                           6     /* Size 16 bytes */


/* Define Stream extension directory entry specific. This is a secondary item.  */

#define FX_EXFAT_NAME_LENGTH                           3     /* Size 1 byte */
#define FX_EXFAT_NAME_HASH                             4     /* Size 2 bytes */
#define FX_EXFAT_VALID_DATA_LENGTH                     8     /* Size 8 bytes */


/* Define File Name directory entry specific. This is a secondary item.  */

#define FX_EXFAT_FILE_NAME                             2     /* Size 30 bytes */


/* Define Continuous Information Manage directory entry specific. This is a secondary item.  */

#define FX_EXFAT_VENDOR_GUID                           2     /* Size 16 bytes */
#define FX_EXFAT_FAT_CHECKSUM                          20    /* Size 4  bytes */


/* Define Continuous Information directory entry specific. This is a secondary item.  */

#define FX_EXFAT_SET_CHECKSUM                          18    /* Size 2  bytes */

#endif

