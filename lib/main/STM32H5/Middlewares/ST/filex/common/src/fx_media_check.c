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

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_system.h"
#include "fx_directory.h"
#include "fx_media.h"
#include "fx_utility.h"
#ifdef FX_ENABLE_EXFAT
#include "fx_directory_exFAT.h"
#endif /* FX_ENABLE_EXFAT */


/* Define parameters for FileX media check utility.  */

#ifndef FX_MAX_DIRECTORY_NESTING
#define FX_MAX_DIRECTORY_NESTING 20
#endif


/* Define data structures local to the FileX media check utility.  */

typedef struct CURRENT_DIRECTORY_ENTRY_STRUCT
{
    ULONG current_directory_entry;
    ULONG current_total_entries;
    ULONG current_start_cluster;
} CURRENT_DIRECTORY_ENTRY;


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_media_check                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks the specified media for basic structural       */
/*    errors, including cross linking, invalid FAT chains, and lost       */
/*    clusters. The function also provides the capability to correct      */
/*    errors.                                                             */
/*                                                                        */
/*    The algorithm is to follow all sub-directories immediately and      */
/*    check their contents. The maximum depth of sub-directories is       */
/*    determined by the constant FX_MAX_DIRECTORY_NESTING.                */
/*    By default, this is set at 20. Basically, the FAT chain of every    */
/*    file and sub-directory is traversed. If valid, clusters are marked  */
/*    in a logical FAT bit map to signal they are already in-use. The     */
/*    algorithm should detect any broken or cross linked FAT condition.   */
/*                                                                        */
/*    As for memory usage, the scratch memory supplied to the media check */
/*    function is used to hold several directory entries, a data          */
/*    structure to "stack" the current directory entry position before    */
/*    diving into the sub-directory, and finally the logical FAT bit map. */
/*    The basic data structures take from 512-1024 bytes and the logical  */
/*    FAT bit map requires as many bits as there are clusters in your     */
/*    device. For example, a device with 8000 clusters would require      */
/*    1000 bytes to represent.                                            */
/*                                                                        */
/*    On the performance end of things, the traversal is reasonably fast  */
/*    and is basically linear with the number of files and their sizes.   */
/*    The lost cluster checking at the end of media check is a bit more   */
/*    performance comprehensive. It basically checks that each unused     */
/*    cluster is actually marked as free. You might decide to bypass this */
/*    step if no other errors are found... or there might be ways to      */
/*    optimize the search by reading whole FAT sectors. You probably just */
/*    need to see how it behaves in your system.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to a previously       */
/*                                            opened media                */
/*    scratch_memory_ptr                    Pointer to memory area for    */
/*                                            media check to use (as      */
/*                                            mentioned above)            */
/*    scratch_memory_size                   Size of the scratch memory    */
/*    error_correction_option               Specifies which - if any -    */
/*                                            errors are corrected by     */
/*                                            the media check function.   */
/*                                            Setting the following bit   */
/*                                            causes that error to be     */
/*                                            corrected:                  */
/*                                                                        */
/*                                            0x01 -> Fix FAT Chain Errors*/
/*                                            0x02 -> Fix Directory Entry */
/*                                                      Errors            */
/*                                            0x04 -> Fix Lost Clusters   */
/*                                                                        */
/*    errors_detected                       Specifies the destination     */
/*                                            ULONG to place the error    */
/*                                            report from media check.    */
/*                                            This has a similar bit map  */
/*                                            as before:                  */
/*                                                                        */
/*                                            0x01 -> FAT Chain Error(s)  */
/*                                            0x02 -> Directory Entry     */
/*                                                      Error(s)          */
/*                                            0x04 -> Lost Cluster(s)     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    FX_SUCCESS                            Media check performed its     */
/*                                            operation successfully.     */
/*                                            This does not mean that     */
/*                                            there were no errors. The   */
/*                                            errors_detected variable    */
/*                                            needs to be examined.       */
/*    FX_MEDIA_NOT_OPEN                     The media was not open.       */
/*    FX_NOT_ENOUGH_MEMORY                  The scratch memory was not    */
/*                                            large enough or the nesting */
/*                                            depth was greater than the  */
/*                                            maximum specified.          */
/*    FX_IO_ERROR                           I/O Error reading/writing to  */
/*                                            the media.                  */
/*    FX_ERROR_NOT_FIXED                    Fundamental problem with      */
/*                                            media that couldn't be fixed*/
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_media_cache_invalidate            Invalidate the cache          */
/*    _fx_media_check_FAT_chain_check       Walk the supplied FAT chain   */
/*    _fx_media_check_lost_cluster_check    Check for lost clusters       */
/*    _fx_directory_entry_read              Directory entry read          */
/*    _fx_directory_entry_write             Directory entry write         */
/*    _fx_media_flush                       Flush changes to the media    */
/*    _fx_utility_FAT_entry_write           Write value to FAT entry      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
UINT  _fx_media_check(FX_MEDIA *media_ptr, UCHAR *scratch_memory_ptr, ULONG scratch_memory_size, ULONG error_correction_option, ULONG *errors_detected)
{

CURRENT_DIRECTORY_ENTRY *current_directory;
ULONG                    total_entries, last_cluster, valid_clusters;
ULONG                    bytes_per_cluster, i, current_errors;
UINT                     status, long_name_size;
UINT                     current_directory_index;
UCHAR                   *logical_fat, *working_ptr;
ALIGN_TYPE               address_mask;
FX_DIR_ENTRY            *temp_dir_ptr, *source_dir_ptr, *dir_entry_ptr;

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY   *trace_event;
ULONG                    trace_timestamp;
#endif

#ifdef FX_ENABLE_EXFAT
    current_errors = 0;
#endif

    /* Check the media to make sure it is open.  */
    if (media_ptr -> fx_media_id != FX_MEDIA_ID)
    {

        /* Return the media not opened error.  */
        return(FX_MEDIA_NOT_OPEN);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    FX_TRACE_IN_LINE_INSERT(FX_TRACE_MEDIA_CHECK, media_ptr, scratch_memory_ptr, scratch_memory_size, 0, FX_TRACE_MEDIA_EVENTS, &trace_event, &trace_timestamp)

    /* Protect against other threads accessing the media.  */
    FX_PROTECT

    /* Determine if there are any opened files.  */
    if (media_ptr -> fx_media_opened_file_count)
    {

        /* There are opened files... this is an error!  */

        /* Release protection.  */
        FX_UNPROTECT

        /* Return an error.  */
        return(FX_ACCESS_ERROR);
    }

    /* Invalidate the cache.  */
    _fx_media_cache_invalidate(media_ptr);

    /* Initialize the reported error flag.  */
    *errors_detected =  0;

    /* Calculate the long name size, rounded up to something that is evenly divisible by 4.  */
    long_name_size =  (((FX_MAX_LONG_NAME_LEN + 3) >> 2) << 2);

    /* Calculate the number of bytes per cluster.  */
    bytes_per_cluster =  media_ptr -> fx_media_sectors_per_cluster * media_ptr -> fx_media_bytes_per_sector;

    /* Setup address mask.  */
    address_mask =  sizeof(ULONG) - 1;
    address_mask =  ~address_mask;

    /* Setup working pointer.  */
    working_ptr =  scratch_memory_ptr + (sizeof(ULONG) - 1);
    working_ptr =  (UCHAR *)(((ALIGN_TYPE)working_ptr) & address_mask);

    /* Memory is set aside for two FX_DIR_ENTRY structures */
    dir_entry_ptr =  (FX_DIR_ENTRY *)working_ptr;

    /* Adjust the scratch memory pointer forward.  */
    working_ptr =  &working_ptr[sizeof(FX_DIR_ENTRY)];

    /* Setup the name buffer for the first directory entry.  */
    dir_entry_ptr -> fx_dir_entry_name =  (CHAR *)working_ptr;

    /* Adjust the scratch memory pointer forward.  */
    working_ptr =  working_ptr + long_name_size + (sizeof(ULONG) - 1);
    working_ptr =  (UCHAR *)(((ALIGN_TYPE)working_ptr) & address_mask);

    /* Setup the source directory entry.  */
    source_dir_ptr =  (FX_DIR_ENTRY *)working_ptr;

    /* Adjust the scratch memory pointer forward.  */
    working_ptr =  &working_ptr[sizeof(FX_DIR_ENTRY)];

    /* Setup the name buffer for the source directory entry.  */
    source_dir_ptr -> fx_dir_entry_name =  (CHAR *)working_ptr;

    /* Adjust the scratch memory pointer forward.  */
    working_ptr =  working_ptr + long_name_size + (sizeof(ULONG) - 1);
    working_ptr =  (UCHAR *)(((ALIGN_TYPE)working_ptr) & address_mask);

    /* Setup the current directory stack memory.  */
    current_directory =  (CURRENT_DIRECTORY_ENTRY *)working_ptr;

    /* Allocate space for the size of the directory entry stack.  This basically
       defines the maximum level of sub-directories supported.  */
    working_ptr =  &working_ptr[(FX_MAX_DIRECTORY_NESTING * sizeof(CURRENT_DIRECTORY_ENTRY))];

    /* Setup the initial current directory entry.  */
    current_directory_index =  0;

    /* Adjust the size to account for the header information.  */
    if (scratch_memory_size < (ULONG)((working_ptr - scratch_memory_ptr)))
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the not enough memory error.  */
        return(FX_NOT_ENOUGH_MEMORY);
    }

    /* Adjust the scratch memory size.  */
    scratch_memory_size =  scratch_memory_size - (ULONG)(working_ptr - scratch_memory_ptr);

    /* Memory is set aside for logical FAT - one bit per cluster */
    logical_fat =  (UCHAR *)working_ptr;

    /* Determine if there is enough memory.  */
    if (scratch_memory_size < ((media_ptr -> fx_media_total_clusters >> 3) + 1))
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* Return the not enough memory error.  */
        return(FX_NOT_ENOUGH_MEMORY);
    }

    /* Initialize the logical FAT table. */
    for (i = 0; i < ((media_ptr -> fx_media_total_clusters >> 3) + 1); i++)
    {
        /* Clear the logical FAT entry, which actually represents eight clusters.  */
        logical_fat[i] =  0;
    }

#ifdef FX_ENABLE_EXFAT

    /* Mark the clusters occupied by Allocation Bitmap table, Up-cast table and the first cluster of root directory. */
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {
    ULONG offset = 0;

        for (i = ((media_ptr -> fx_media_root_cluster_32 - FX_FAT_ENTRY_START) >> 3); i; i--)
        {
            logical_fat[offset++] = 0xff;
        }

        for (i = ((media_ptr -> fx_media_root_cluster_32 - FX_FAT_ENTRY_START) & 7); i; i--)
        {
            logical_fat[offset] = (UCHAR)((logical_fat[offset] << 1) | 0x1);
        }
    }
#endif /* FX_ENABLE_EXFAT */

#ifdef FX_ENABLE_FAULT_TOLERANT
    if (media_ptr -> fx_media_fault_tolerant_enabled)
    {
    ULONG cluster, cluster_number;

        /* Mark the cluster used by fault tolerant as valid. */
        for (cluster = media_ptr -> fx_media_fault_tolerant_start_cluster;
             cluster < media_ptr -> fx_media_fault_tolerant_start_cluster + media_ptr -> fx_media_fault_tolerant_clusters;
             cluster++)
        {

            cluster_number = cluster;

#ifdef FX_ENABLE_EXFAT

            /* For the index of the first cluster in exFAT is 2, adjust the number of clusters to fit Allocation Bitmap Table. */
            /* We will compare logical_fat with Aollcation Bitmap table later to find out lost clusters. */
            if (media_ptr -> fx_media_FAT_type == FX_exFAT)
            {
                cluster_number = cluster - FX_FAT_ENTRY_START;
            }
#endif /* FX_ENABLE_EXFAT */

            logical_fat[cluster_number >> 3] = (UCHAR)(logical_fat[cluster_number >> 3] | (1 << (cluster_number & 7)));
        }
    }
#endif /* FX_ENABLE_FAULT_TOLERANT */

    /* If FAT32 is present, determine if the root directory is coherent.  */
#ifdef FX_ENABLE_EXFAT
    if ((media_ptr -> fx_media_FAT_type == FX_FAT32) ||
        (media_ptr -> fx_media_FAT_type == FX_exFAT))
#else
    if (media_ptr -> fx_media_32_bit_FAT)
#endif /* FX_ENABLE_EXFAT */
    {

        /* A 32-bit FAT is present. We need to walk the clusters of the root directory to determine if
           it is intact. */
        current_errors =  _fx_media_check_FAT_chain_check(media_ptr, media_ptr -> fx_media_root_cluster_32,
                                                          &last_cluster, &valid_clusters, logical_fat);

        /* Make them part of the errors reported to the caller.  */
        *errors_detected =  *errors_detected | current_errors;

        /* Update the trace event with the errors detected.  */
        FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_MEDIA_CHECK, 0, 0, 0, *errors_detected)

        /* Determine if the I/O error is set.  */
        if (current_errors & FX_IO_ERROR)
        {

            /* Release media protection.  */
            FX_UNPROTECT

            /* File I/O Error.  */
            return(FX_IO_ERROR);
        }

        /* Check the status.  */
        if (*errors_detected)
        {

            /* Determine if we can fix the FAT32 root directory error.  */
            if ((valid_clusters) && (error_correction_option & FX_FAT_CHAIN_ERROR))
            {

                /* Make the chain end at the last cluster. */
                status = _fx_utility_FAT_entry_write(media_ptr, last_cluster, media_ptr -> fx_media_fat_last);

                /* Determine if the write was successful.  */
                if (status)
                {

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Return the error code.  */
                    return(status);
                }

                /* Adjust the total entries in the root directory.  */
                media_ptr -> fx_media_root_directory_entries =  (valid_clusters * bytes_per_cluster) / FX_DIR_ENTRY_SIZE;
            }
            else
            {

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return an error.  */
                return(FX_ERROR_NOT_FIXED);
            }
        }
    }

    /* Pickup total entries in the root directory.  */
    total_entries =  media_ptr -> fx_media_root_directory_entries;

    /* Set temp directory pointer to NULL.  */
    temp_dir_ptr =  FX_NULL;

    /* Put the root directory information in the entry stack */
    current_directory[current_directory_index].current_total_entries =    total_entries;
    current_directory[current_directory_index].current_start_cluster =    media_ptr -> fx_media_fat_last;
    current_directory[current_directory_index].current_directory_entry =  0;

    /* Now we shall do the checking in depth first manner. */
    do
    {

        /* Pickup the directory index.  */
        i =  current_directory[current_directory_index].current_directory_entry;

        /* Loop to process remaining directory entries.  */
        while (i < current_directory[current_directory_index].current_total_entries)
        {

            /* Read a directory entry.  */
#ifdef FX_ENABLE_EXFAT
            /* Hash value of the file name is not cared. */
            status =  _fx_directory_entry_read_ex(media_ptr, temp_dir_ptr, &i, dir_entry_ptr, 0);
#else
            status =  _fx_directory_entry_read(media_ptr, temp_dir_ptr, &i, dir_entry_ptr);
#endif /* FX_ENABLE_EXFAT */

            /* Determine if the read was successful.  */
            if (status)
            {

                /* Release media protection.  */
                FX_UNPROTECT

                /* Return the error code.  */
                return(status);
            }

            /* Check for the last entry.  */
#ifdef FX_ENABLE_EXFAT
            if (dir_entry_ptr -> fx_dir_entry_type == FX_EXFAT_DIR_ENTRY_TYPE_END_MARKER)
#else
            if (dir_entry_ptr -> fx_dir_entry_name[0] == (CHAR)FX_DIR_ENTRY_DONE)
#endif /* FX_ENABLE_EXFAT */
            {

                /* Last entry in this directory - no need to check further.  */
                break;
            }

            /* Is the entry free?  */
#ifdef FX_ENABLE_EXFAT
            if (dir_entry_ptr -> fx_dir_entry_type != FX_EXFAT_DIR_ENTRY_TYPE_FILE_DIRECTORY)
#else
            if ((dir_entry_ptr -> fx_dir_entry_name[0] == (CHAR)FX_DIR_ENTRY_FREE) && (dir_entry_ptr -> fx_dir_entry_short_name[0] == 0))
#endif /* FX_ENABLE_EXFAT */
            {

                /* A deleted entry */
                i++;
                continue;
            }

#ifdef FX_ENABLE_EXFAT

            /* Determine whether FAT chain is used. */
            if (dir_entry_ptr -> fx_dir_entry_dont_use_fat & 1)
            {
            ULONG64 remaining_size = dir_entry_ptr -> fx_dir_entry_file_size;
            ULONG   cluster = dir_entry_ptr -> fx_dir_entry_cluster, cluster_number;

                valid_clusters = 0;

                while (remaining_size)
                {
                    if (remaining_size >= bytes_per_cluster)
                    {
                        remaining_size -= bytes_per_cluster;
                    }
                    else
                    {
                        remaining_size = 0;
                        last_cluster = cluster;
                    }

                    cluster_number = cluster - FX_FAT_ENTRY_START;

                    /* Is the current cluster already marked? */
                    if ((logical_fat[cluster_number / 8] >> (cluster_number % 8)) & 0x01)
                    {
                        current_errors = FX_FILE_SIZE_ERROR;
                        last_cluster = dir_entry_ptr -> fx_dir_entry_cluster + valid_clusters;
                        break;
                    }
                    
                    /* Mark the current cluster. */
                    logical_fat[cluster_number >> 3] = (UCHAR)(logical_fat[cluster_number >> 3] | (1 << (cluster_number & 7)));

                    valid_clusters++;
                    cluster++;
                }
            }
            else
            {
#endif /* FX_ENABLE_EXFAT */

                /* Look for any cross links or errors in the FAT chain of current directory entry. */
                current_errors =  _fx_media_check_FAT_chain_check(media_ptr, dir_entry_ptr -> fx_dir_entry_cluster,
                                                                  &last_cluster, &valid_clusters, logical_fat);
#ifdef FX_ENABLE_EXFAT
            }
#endif /* FX_ENABLE_EXFAT */

            /* Make them part of the errors reported to the caller.  */
            *errors_detected =  *errors_detected | current_errors;

            /* Update the trace event with the errors detected.  */
            FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_MEDIA_CHECK, 0, 0, 0, *errors_detected)

            /* Determine if the I/O error is set.  */
            if (current_errors & FX_IO_ERROR)
            {

                /* Release media protection.  */
                FX_UNPROTECT

                /* File I/O Error.  */
                return(FX_IO_ERROR);
            }

            /* Check for errors.  */
            if (*errors_detected)
            {

                /* Determine if we can fix the FAT chain.  */
                if (error_correction_option & FX_FAT_CHAIN_ERROR)
                {

                    /* Determine if there is a valid cluster to write the EOF to.  */
                    if (valid_clusters)
                    {

                        /* Write EOF in the last FAT entry.  */
                        status =  _fx_utility_FAT_entry_write(media_ptr, last_cluster, media_ptr -> fx_media_fat_last);

                        /* Determine if the write was successful.  */
                        if (status)
                        {

                            /* Release media protection.  */
                            FX_UNPROTECT

                            /* Return the error code.  */
                            return(status);
                        }
                    }
                }
            }

            /* Determine if we need to update the size of the directory entry.  */
            if (dir_entry_ptr -> fx_dir_entry_file_size > (valid_clusters * bytes_per_cluster))
            {

                /* Yes, update the directory entry's size.  */
                dir_entry_ptr -> fx_dir_entry_file_size =  valid_clusters * bytes_per_cluster;

                /* Determine if the new file size is zero. */
                if (dir_entry_ptr -> fx_dir_entry_file_size == 0)
                {

                    /* Consider this a directory error.  */
                    *errors_detected =  *errors_detected | FX_DIRECTORY_ERROR;

                    /* Update the trace event with the errors detected.  */
                    FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_MEDIA_CHECK, 0, 0, 0, *errors_detected)

                    /* Clear the starting cluster number of this directory entry.  */
                    dir_entry_ptr -> fx_dir_entry_cluster =  0;

                    /* If directory fixing is required, delete this directory entry as well.  */
                    if (error_correction_option & FX_DIRECTORY_ERROR)
                    {

                        /* Mark the entry as deleted.  */
                        dir_entry_ptr -> fx_dir_entry_name[0] =        (CHAR)FX_DIR_ENTRY_FREE;
                        dir_entry_ptr -> fx_dir_entry_short_name[0] =  (CHAR)FX_DIR_ENTRY_FREE;
                    }
                }

                /* Only update the directory if the FAT chain was actually updated.  */
                if (error_correction_option & FX_FAT_CHAIN_ERROR)
                {

                    /* Update the directory entry.  */
                    status =  _fx_directory_entry_write(media_ptr, dir_entry_ptr);

                    /* Determine if the write was successful.  */
                    if (status)
                    {

                        /* Release media protection.  */
                        FX_UNPROTECT

                        /* Return the error code.  */
                        return(status);
                    }
                }
            }

            /* Determine if the entry is a sub-directory.  */
            if ((dir_entry_ptr -> fx_dir_entry_attributes & FX_DIRECTORY)
                 && (valid_clusters == 0))
            {

                /* Consider this a directory error.  */
                *errors_detected =  *errors_detected | FX_DIRECTORY_ERROR;

                /* Update the trace event with the errors detected.  */
                FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_MEDIA_CHECK, 0, 0, 0, *errors_detected)

                /* Determine if we can fix the error.  */
                if (error_correction_option & FX_DIRECTORY_ERROR)
                {

                    /* Yes, make the directory entry free.  */
                    dir_entry_ptr -> fx_dir_entry_name[0] =        (CHAR)FX_DIR_ENTRY_FREE;
                    dir_entry_ptr -> fx_dir_entry_short_name[0] =  (CHAR)FX_DIR_ENTRY_FREE;

                    /* Delete the sub-directory entry.  */
                    status =  _fx_directory_entry_write(media_ptr, dir_entry_ptr);

                    /* Determine if the write was successful.  */
                    if (status)
                    {

                        /* Release media protection.  */
                        FX_UNPROTECT

                        /* Return the error code.  */
                        return(status);
                    }

                    /* Move to next entry.  */
                    i++;
                    continue;
                }
            }

            /* Determine if the entry is a directory.  */
            if (dir_entry_ptr -> fx_dir_entry_attributes & FX_DIRECTORY)
            {

                /* Current entry is a directory. The algorithm is designed to follow all
                   sub-directories immediately, i.e., a depth first search.  */

                /* First, save the next entry position. */
                current_directory[current_directory_index].current_directory_entry =  i + 1;

                /* Push the current directory entry on the stack.  */
                current_directory_index++;

                /* Check for current directory stack overflow.  */
                if (current_directory_index >= FX_MAX_DIRECTORY_NESTING)
                {

                    /* Release media protection.  */
                    FX_UNPROTECT

                    /* Current directory stack overflow.  Return error.  */
                    return(FX_NOT_ENOUGH_MEMORY);
                }

                /* Otherwise, setup the new directory entry.  */
                current_directory[current_directory_index].current_total_entries =      (valid_clusters * bytes_per_cluster) / FX_DIR_ENTRY_SIZE;
                current_directory[current_directory_index].current_start_cluster =      dir_entry_ptr -> fx_dir_entry_cluster;
                current_directory[current_directory_index].current_directory_entry =    2;

                /* Setup new source directory.  */
                source_dir_ptr -> fx_dir_entry_cluster =              dir_entry_ptr -> fx_dir_entry_cluster;
                source_dir_ptr -> fx_dir_entry_file_size =            current_directory[current_directory_index].current_total_entries;
                source_dir_ptr -> fx_dir_entry_last_search_cluster =  0;
                temp_dir_ptr =                                        source_dir_ptr;

                /* Skip the first two entries of sub-directories.  */
                i =  2;

#ifdef FX_ENABLE_EXFAT

                /* For exFAT, there is no dir-entries for ".." and ".". */
                if (media_ptr -> fx_media_FAT_type == FX_exFAT)
                {
                    current_directory[current_directory_index].current_directory_entry = 0;
                    i =  0;
                }
#endif /* FX_ENABLE_EXFAT */
            }
            else
            {

                /* Regular file entry.  */

                /* Check for an invalid file size.  */
                if (((valid_clusters * bytes_per_cluster) - dir_entry_ptr -> fx_dir_entry_file_size) > bytes_per_cluster)
                {

                    /* There are more clusters allocated than needed for the file's size.  Indicate that this error
                       is present.  */
                    *errors_detected =  *errors_detected | FX_FILE_SIZE_ERROR;

                    /* Update the trace event with the errors detected.  */
                    FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_MEDIA_CHECK, 0, 0, 0, *errors_detected)

                    /* For now, don't shorten the cluster chain.  */
                }

                /* Look into the next entry in the current directory.  */
                i++;
            }
        }

        /* Once we get here, we have exhausted the current directory and need to return to the previous
           directory.  */

        /* Check for being at the root directory.  */
        if (current_directory_index == 0)
        {

            /* Yes, we have now exhausted searching the root directory so we are done!  */
            break;
        }

        /* Backup to the place we left off in the previous directory.  */
        current_directory_index--;

        /* Determine if we are now back at the root directory.  */
        if (current_directory[current_directory_index].current_start_cluster == media_ptr -> fx_media_fat_last)
        {

            /* The search directory should be NULL since it is the root directory.  */
            temp_dir_ptr =  FX_NULL;
        }
        else
        {

            /* Otherwise, we are returning to a sub-directory.  Setup the search directory
               appropriately.  */
            source_dir_ptr -> fx_dir_entry_cluster =              current_directory[current_directory_index].current_start_cluster;
            source_dir_ptr -> fx_dir_entry_file_size =            current_directory[current_directory_index].current_total_entries;
            source_dir_ptr -> fx_dir_entry_last_search_cluster =  0;
            temp_dir_ptr =                                        source_dir_ptr;
        }
    } while (1);

#ifdef FX_ENABLE_EXFAT
    if (media_ptr -> fx_media_FAT_type == FX_exFAT)
    {

        /* If exFAT is in use, compare the logical_fat with Allocation Bitmap Table directly. */
        current_errors =  _fx_media_check_exFAT_lost_cluster_check(media_ptr, logical_fat, media_ptr -> fx_media_total_clusters, error_correction_option);
    }
    else
    {
#endif /* FX_ENABLE_EXFAT */

        /* At this point, all the files and sub-directories have been examined.  We now need to check for
           lost clusters in the logical FAT.  A lost cluster is basically anything that is not reported in
           the logical FAT that has a non-zero value in the real FAT.  */
        current_errors =  _fx_media_check_lost_cluster_check(media_ptr, logical_fat, media_ptr -> fx_media_total_clusters, error_correction_option);
#ifdef FX_ENABLE_EXFAT
    }
#endif /* FX_ENABLE_EXFAT */

    /* Incorporate the error returned by the lost FAT check.  */
    *errors_detected =  *errors_detected | current_errors;

    /* Update the trace event with the errors detected.  */
    FX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, FX_TRACE_MEDIA_CHECK, 0, 0, 0, *errors_detected)

    /* Determine if the I/O error is set.  */
    if (current_errors & FX_IO_ERROR)
    {

        /* Release media protection.  */
        FX_UNPROTECT

        /* File I/O Error.  */
        return(FX_IO_ERROR);
    }

    /* Determine if there was any error and update was selected.  */
    if ((*errors_detected) && (error_correction_option))
    {

        /* Flush any unwritten items to the media.  */
        _fx_media_flush(media_ptr);
    }

    /* Release media protection.  */
    FX_UNPROTECT

    /* At this point, we have completed the diagnostic of the media, return success!  */
    return(FX_SUCCESS);
}

