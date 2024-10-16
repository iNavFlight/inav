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
/**   Fault Tolerant                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    fx_fault_tolerant.h                                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the FileX Fault Tolerant component constants,     */
/*    data definitions, and external references.  It is assumed that      */
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

#ifndef _FX_FAULT_TOLERANT_H_
#define _FX_FAULT_TOLERANT_H_

#ifdef FX_ENABLE_FAULT_TOLERANT

#ifndef FX_FAULT_TOLERANT
#error "FX_FAULT_TOLERANT must be defined with FX_ENABLE_FAULT_TOLERANT"
#endif /* FX_FAULT_TOLERANT */

/* Define ID of fault tolerant. */
#define FX_FAULT_TOLERANT_ID                      0x46544C52

/* Define fault tolerant version. */
#define FX_FAULT_TOLERANT_VERSION_MAJOR           0x01
#define FX_FAULT_TOLERANT_VERSION_MINOR           0x00

/* Define byte offset in boot sector where the cluster number of the Fault Tolerant Log file is.
   Note that this field (byte 116 to 119) is marked as reserved by FAT 12/16/32/exFAT specification. */
#ifndef FX_FAULT_TOLERANT_BOOT_INDEX
#define FX_FAULT_TOLERANT_BOOT_INDEX              116
#endif /* FX_FAULT_TOLERANT_BOOT_INDEX */

/* Define the extension invoked when fault tolerant log is recovered or applied during enable API. */
#ifndef FX_FAULT_TOLERANT_ENABLE_EXTENSION
#define FX_FAULT_TOLERANT_ENABLE_EXTENSION
#endif /* FX_FAULT_TOLERANT_ENABLE_EXTENSION */

/* Define the extension invoked when fault tolerant log is applied. */
#ifndef FX_FAULT_TOLERANT_APPLY_LOGS_EXTENSION
#define FX_FAULT_TOLERANT_APPLY_LOGS_EXTENSION
#endif /* FX_FAULT_TOLERANT_APPLY_LOGS_EXTENSION */

/* If not-defined, default port-specific processing extensions to white space.  */
#ifndef FX_FAULT_TOLERANT_WRITE_LOG_FILE_EXTENSION
#define FX_FAULT_TOLERANT_WRITE_LOG_FILE_EXTENSION
#endif /* FX_FAULT_TOLERANT_WRITE_LOG_FILE_EXTENSION */

/* Define the maximum size of log file. */
#define FX_FAULT_TOLERANT_MAXIMUM_LOG_FILE_SIZE   3072

/* For backward compatibility, map the symbols deprecated to FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE. */
#ifndef FX_FAULT_TOLERANT_MINIMAL_CLUSTER
#define FX_FAULT_TOLERANT_MINIMAL_CLUSTER         FX_FAULT_TOLERANT_MAXIMUM_LOG_FILE_SIZE
#endif /* FX_FAULT_TOLERANT_MINIMAL_CLUSTER */
#ifndef FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE
#define FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE     FX_FAULT_TOLERANT_MAXIMUM_LOG_FILE_SIZE
#endif /* FX_FAULT_TOLERANT_MINIMAL_BUFFER_SIZE */

/* Define the states of the fault tolerant module. */
#define FX_FAULT_TOLERANT_STATE_IDLE              0x00u
#define FX_FAULT_TOLERANT_STATE_STARTED           0x01u
#define FX_FAULT_TOLERANT_STATE_SET_FAT_CHAIN     0x02u

/* Define types of logs. */
#define FX_FAULT_TOLERANT_FAT_LOG_TYPE            1
#define FX_FAULT_TOLERANT_DIR_LOG_TYPE            2
#define FX_FAULT_TOLERANT_BITMAP_LOG_TYPE         3

/* Define operations of FAT chain. */
#define FX_FAULT_TOLERANT_FAT_CHAIN_RECOVER       0     /* Recover new FAT chain. */
#define FX_FAULT_TOLERANT_FAT_CHAIN_CLEANUP       1     /* Cleanup old FAT chain. */

/* Define flags for FAT chain log. */
#define FX_FAULT_TOLERANT_FLAG_FAT_CHAIN_VALID    0x01
#define FX_FAULT_TOLERANT_FLAG_BITMAP_USED        0x02

/* Define size of each section in log file. */
#define FX_FAULT_TOLERANT_LOG_HEADER_SIZE         sizeof(FX_FAULT_TOLERANT_LOG_HEADER)
#define FX_FAULT_TOLERANT_FAT_CHAIN_SIZE          sizeof(FX_FAULT_TOLERANT_FAT_CHAIN)
#define FX_FAULT_TOLERANT_LOG_CONTENT_HEADER_SIZE sizeof(FX_FAULT_TOLERANT_LOG_CONTENT)

/* Define offset of each section in log file. */
#define FX_FAULT_TOLERANT_FAT_CHAIN_OFFSET        (FX_FAULT_TOLERANT_LOG_HEADER_SIZE)
#define FX_FAULT_TOLERANT_LOG_CONTENT_OFFSET      (FX_FAULT_TOLERANT_LOG_HEADER_SIZE + FX_FAULT_TOLERANT_FAT_CHAIN_SIZE)

/* Define size of log entries. */
/* The total size of DIR log entry is variable. 16 is the fixed size of DIR log entry. */
#define FX_FAULT_TOLERANT_FAT_LOG_ENTRY_SIZE      sizeof(FX_FAULT_TOLERANT_FAT_LOG)
#define FX_FAULT_TOLERANT_BITMAP_LOG_ENTRY_SIZE   sizeof(FX_FAULT_TOLERANT_BITMAP_LOG)
#define FX_FAULT_TOLERANT_DIR_LOG_ENTRY_SIZE      sizeof(FX_FAULT_TOLERANT_DIR_LOG)

#ifdef FX_FAULT_TOLERANT_TRANSACTION_FAIL_FUNCTION
#define FX_FAULT_TOLERANT_TRANSACTION_FAIL _fx_fault_tolerant_transaction_fail
#else
/* Define transaction fail for fault tolerant. */
#define FX_FAULT_TOLERANT_TRANSACTION_FAIL(m)                            \
    if ((m) -> fx_media_fault_tolerant_enabled == FX_TRUE)               \
    {                                                                    \
        (m) -> fx_media_fault_tolerant_transaction_count--;              \
        if ((m) -> fx_media_fault_tolerant_transaction_count == 0)       \
        {                                                                \
            _fx_fault_tolerant_recover(m);                               \
            _fx_fault_tolerant_reset_log_file(m);                        \
        }                                                                \
    }

#endif /* FX_FAULT_TOLERANT_TRANSACTION_FAIL_FUNCTION */

/* Log file format
 * The entire log file fits into one cluster.
 *
 *                      1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +                                ID                             +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +   Total Log Size (in Bytes)    +        Header Checksum       +      Log Header
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +  version major + version minor +           reserved           +
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +      Checksum of FAT chain     +     Flag      |   Reserved   +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +                   Insertion Point - Front                     +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +            Head cluster of newly created FAT chain            +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++      FAT Chain
 * +                 Head cluster of original FAT chain            +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +                   Insertion Point - Back                      +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +                     Next Deletion Point                       +
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +     checksum of log content    +      count of log entries    +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * + (log 0)                        .                              +
 * +                                .                              +
 * +                                .                              +
 * +                                .                              +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * + (log 1)                        .                              +
 * +                                .                              +      Log Entries
 * +                                .                              +
 * +                                .                              +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +                               ...                             +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * + (log N)                        .                              +
 * +                                .                              +
 * +                                .                              +
 * +                                .                              +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 *
 * Log Header
 *
 *  * ID: It is fixed value FX_FAULT_TOLERANT_ID.
 *  * Total Size: Total size (in bytes) of the log file, including Log Header, FAT Chain and Log Entries
 *  * Header Checksum: It is the checksum of all data in log header.
 *  * Version Major: The major version number.
 *  * Version Minor: The minor version number.
 *  * Reserved: Reserved for future use.
 *
 *  FAT Chain
 *
 *  * Checksum of FAT chain: It is the checksum of all data in the FAT Chain section.
 *  * Flag: 8 bits. The 1-6 bits are not used (from left to right).
 *      bit 7: When set, bitmap is used in old FAT chain.
 *      bit 8: When set, FAT chain is valid.
 *  * Reserved: Reserved for future use.
 *  * Insertion Point - Front:  The cluster where the newly created chain is attached to
 *  * Head cluster of new FAT chain. It is the head cluster of new FAT chain.
 *  * Head cluster of oritinal FAT chain. It is the head cluster of the original FAT chain,
 *        which will be removed.
 *  * Inswertion Point - Back:  The original cluster right after the newly created chain is inserted.
 *  * Next Deletion Point: next session of cluster to delete. It is used during deleting FAT chain.
 *      The deltion is carried out in stages for perofmrance reasons.  When delting a FAT chain, the
 *      Next Deletion Point is chosen, and the chain between head and the Next Deletion Point is deleted.
 *
 *  Log Entries:
 *
 *  * Checksum of log entries: It is the checksum of all data in Log Entries
 *  * Counter of log entries: It is the counter of all log entries in Log content.
 *  * log 0 ... log N: Log entries. The formats are described below.
 */

/* Defined structure of log header.


 */
typedef struct FX_FAULT_TOLERANT_LOG_HEADER_STRUCT
{
    ULONG  fx_fault_tolerant_log_header_id;
    USHORT fx_fault_tolerant_log_header_total_size;
    USHORT fx_fault_tolerant_log_header_checksum;
    UCHAR  fx_fault_tolerant_log_header_version_major;
    UCHAR  fx_fault_tolerant_log_header_version_minor;
    USHORT fx_fault_tolerant_log_header_reserved;
} FX_FAULT_TOLERANT_LOG_HEADER;

/* Define structure of FAT chain. */
typedef struct FX_FAULT_TOLERANT_FAT_CHAIN_STRUCT
{
    USHORT fx_fault_tolerant_FAT_chain_checksumm;
    UCHAR  fx_fault_tolerant_FAT_chain_flag;
    UCHAR  fx_fault_tolerant_FAT_chain_reserved;

    ULONG  fx_fault_tolerant_FAT_chain_insertion_front;
    ULONG  fx_fault_tolerant_FAT_chain_head_new;
    ULONG  fx_fault_tolerant_FAT_chain_head_original;
    ULONG  fx_fault_tolerant_FAT_chain_insertion_back;
    ULONG  fx_fault_tolerant_FAT_chain_next_deletion;
} FX_FAULT_TOLERANT_FAT_CHAIN;


/* Define structure of Log content. */
typedef struct FX_FAULT_TOLERANT_LOG_CONTENT_STRUCT
{
    USHORT fx_fault_tolerant_log_content_checksum;
    USHORT fx_fault_tolerant_log_content_count;
} FX_FAULT_TOLERANT_LOG_CONTENT;


/* 3 types of log entries are defined. */
/* FAT log format
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +              type              +           size               +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +                             cluster                           +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +                              value                            +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Description:
 *
 * Type:     FX_FAULT_TOLERANT_FAT_LOG.
 * Size:     The size of this log entry.
 * Cluster:  The cluster number in FAT.
 * Value:    The value of cluster in FAT.
 *
 */
typedef struct FX_FAULT_TOLERANT_FAT_LOG_STRUCT
{
    USHORT fx_fault_tolerant_FAT_log_type;
    USHORT fx_fault_tolerant_FAT_log_size;
    ULONG  fx_fault_tolerant_FAT_log_cluster;
    ULONG  fx_fault_tolerant_FAT_log_value;
} FX_FAULT_TOLERANT_FAT_LOG;

/* DIR log format
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +              type              +           size               +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +                         sector offset                         +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +                           log sector                          +
 * +                                                               +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +                           log data                            +
 * +                              .                                +
 * +                              .                                +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Fields description,
 *
 * Type:          FX_FAULT_TOLERANT_DIRECTORY_LOG
 * Size:          The size of this log entry.
 * Sector Offset: The offset in original sector this DIR is trying to write to.
 * Log Sector:    The original sector this DIR is trying to write to.
 * Log Data:      Content of DIR entries.
 *
 */
typedef struct FX_FAULT_TOLERANT_DIR_LOG_STRUCT
{
    USHORT  fx_fault_tolerant_dir_log_type;
    USHORT  fx_fault_tolerant_dir_log_size;
    ULONG   fx_fault_tolerant_dir_log_offset;
    ULONG64 fx_fault_tolerant_dir_log_sector;
} FX_FAULT_TOLERANT_DIR_LOG;

/* Bitmap log format
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +              type              +           size               +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +                             cluster                           +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * +                              value                            +
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Description,
 *
 * Type:     FX_FAULT_TOLERANT_BITMAP_LOG.
 * Size:     The size of this log entry.
 * Cluster:  The cluster number in FAT.
 * Value:    The value of cluster in FAT.
 */
typedef struct FX_FAULT_TOLERANT_BITMAP_LOG_STRUCT
{
    USHORT fx_fault_tolerant_bitmap_log_type;
    USHORT fx_fault_tolerant_bitmap_log_size;
    ULONG  fx_fault_tolerant_bitmap_log_cluster;
    ULONG  fx_fault_tolerant_bitmap_log_value;
} FX_FAULT_TOLERANT_BITMAP_LOG;


/* This function checks whether or not the log file exists, and creates the log file if it does not exist. */
UINT _fx_fault_tolerant_enable(FX_MEDIA *media_ptr, VOID *memory_buffer, UINT memory_size);
UINT _fxe_fault_tolerant_enable(FX_MEDIA *media_ptr, VOID *memory_buffer, UINT memory_size);

/* This function resets statistics and marks the start of a transaction. */
UINT _fx_fault_tolerant_transaction_start(FX_MEDIA *media_ptr);

/* This function marks the end of a transaction and completes the log file.
 * Then it applies all data in the log file to the file system.
 * Finally it resets the log header section. */
UINT _fx_fault_tolerant_transaction_end(FX_MEDIA *media_ptr);

/* This function resets the log file. */
UINT _fx_fault_tolerant_reset_log_file(FX_MEDIA *media_ptr);

/* This function applies the log file to the file system. */
UINT _fx_fault_tolerant_apply_logs(FX_MEDIA *media_ptr);

/* This function recovers FAT chain. */
UINT _fx_fault_tolerant_recover(FX_MEDIA *media_ptr);

/* This function adds a FAT log entry. */
UINT _fx_fault_tolerant_add_FAT_log(FX_MEDIA *media_ptr, ULONG cluster, ULONG value);

/* This function adds a directory log entry. */
UINT _fx_fault_tolerant_add_dir_log(FX_MEDIA *media_ptr, ULONG64 logical_sector, ULONG offset,
                                    UCHAR *data, ULONG data_size);
#ifdef FX_ENABLE_EXFAT
/* This function adds a bitmap log entry. */
UINT _fx_fault_tolerant_add_bitmap_log(FX_MEDIA *media_ptr, ULONG cluster, ULONG value);

/* This function adds a checksum log entry. */
UINT _fx_fault_tolerant_add_checksum_log(FX_MEDIA *media_ptr, ULONG64 logical_sector, ULONG offset, USHORT checksum);
#endif /* FX_ENABLE_EXFAT */

/* This function sets the FAT chain. */
UINT _fx_fault_tolerant_set_FAT_chain(FX_MEDIA *media_ptr, UINT use_bitmap, ULONG insertion_front,
                                      ULONG new_head_cluster, ULONG original_head_cluster, ULONG insertion_back);

/* This function reads a FAT entry from log file. If it doesn't exist in log file, return and read from FAT entry directly. */
UINT _fx_fault_tolerant_read_FAT(FX_MEDIA *media_ptr, ULONG cluster, ULONG *value, ULONG log_type);

/* This function reads directory sector from log file. */
UINT _fx_fault_tolerant_read_directory_sector(FX_MEDIA *media_ptr, ULONG64 logical_sector,
                                              VOID *buffer_ptr, ULONG64 sectors);

/* This function calculates the checksum of the log header. */
USHORT _fx_fault_tolerant_calculate_checksum(UCHAR *data, UINT len);

/* This function cleans up FAT chain. */
UINT _fx_fault_tolerant_cleanup_FAT_chain(FX_MEDIA *media_ptr, UINT operation);

/* This function reads log file from file system to memory buffer. */
UINT _fx_fault_tolerant_read_log_file(FX_MEDIA *media_ptr);

/* This function writes data of one sector from memory to log file in file system. */
UINT _fx_fault_tolerant_write_log_file(FX_MEDIA *media_ptr, ULONG relative_sector);

/* This function creates log file. */
UINT _fx_fault_tolerant_create_log_file(FX_MEDIA *media_ptr);

#ifdef FX_FAULT_TOLERANT_TRANSACTION_FAIL_FUNCTION
/* This function cleans up resources created by fault tolerant when transaction fails. */
UINT _fx_fault_tolerant_transaction_fail(FX_MEDIA *media_ptr);
#endif /* FX_FAULT_TOLERANT_TRANSACTION_FAIL_FUNCTION */


#endif /* FX_ENABLE_FAULT_TOLERANT */

#endif /* _FX_FAULT_TOLERANT_H_ */

