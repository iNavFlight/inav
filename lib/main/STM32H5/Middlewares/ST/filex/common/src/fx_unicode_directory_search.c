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

#define FX_SOURCE_CODE


/* Include necessary system files.  */

#include "fx_api.h"
#include "fx_unicode.h"
#include "fx_utility.h"

#ifndef FX_NO_LOCAL_PATH
FX_LOCAL_PATH_SETUP
#endif


/* Define several Unicode working arrays...  This keeps the data structures
   off the local stack.  */

UCHAR _fx_unicode_temp_long_file_name[FX_MAX_LONG_NAME_LEN];
UCHAR _fx_unicode_search_name[FX_MAX_LONG_NAME_LEN * 2];


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _fx_unicode_directory_search                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function searches for the specified unicode or short name.     */
/*                                                                        */
/*    Note: The buffer of short_name and unicode_name must be valid to    */
/*    fill search result. When short_name is a zero length string, search */
/*    is based on unicode string (terminated with NULL). If it's found    */
/*    the short name is written back to buffer of short_name. In this case*/
/*    unicode_name_buffer_length is ignored and short_name_buffer_length  */
/*    must not be zero to specify the buffer length. If buffer is too     */
/*    smallfor the result, overflow characters and NULL-terminator are cut*/
/*    off. When short_name is a valid string, search is based on          */
/*    short_name (terminated with NULL). If it's found the unicode name is*/
/*    written back to buffer of unicode_name. In this case                */
/*    short_name_buffer_length is ignored and unicode_name_buffer_length  */
/*    must not be zero to specify the unicode buffer length. If buffer is */
/*    too small for the result, overflow characters are cut off but       */
/*    NULL-terminator is kept.                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    media_ptr                             Pointer to media              */
/*    short_name                            Pointer to short name         */
/*    short_name_buffer_length              Buffer length for short name  */
/*    unicode_name                          Pointer to Unicode name       */
/*    unicode_name_length                   Unicode name length           */
/*    unicode_name_buffer_length            Buffer length for Unicode name*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Completion Status                                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _fx_unicode_directory_entry_read      Read a full unicode directory */
/*                                            entry                       */
/*    _fx_utility_FAT_entry_read            Read a FAT entry              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Unicode Utilities                                                   */
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
UINT  _fx_unicode_directory_search(FX_MEDIA *media_ptr, FX_DIR_ENTRY *entry_ptr,
                                   UCHAR *short_name, ULONG short_name_buffer_length, 
                                   UCHAR *unicode_name, ULONG *unicode_name_length, ULONG unicode_name_buffer_length)
{

ULONG         i, j;
UINT          status, found;
ULONG         cluster, next_cluster = 0;
ULONG         directory_size;
FX_DIR_ENTRY  search_dir;
FX_DIR_ENTRY *search_dir_ptr;
ULONG         unicode_search_length;
ULONG         local_unicode_name_length;
CHAR          unicode_to_short_name[13];
CHAR         *short_name_ptr;


    /* Setup temp unicode name length.  */
    local_unicode_name_length =  *unicode_name_length;

#ifndef FX_MEDIA_STATISTICS_DISABLE

    /* Increment the number of directory search requests.  */
    media_ptr -> fx_media_directory_searches++;
#endif

    /* Set the initial search directory to the current working
       directory - if there is one.  */

    /* First check for a local path pointer stored in the thread control block.  This
       is only available in ThreadX Version 4 and above.  */
#ifndef FX_NO_LOCAL_PATH
    if (_tx_thread_current_ptr -> tx_thread_filex_ptr)
    {

        /* Determine if the local directory is not the root directory.  */
        if (((FX_PATH *)_tx_thread_current_ptr -> tx_thread_filex_ptr) -> fx_path_directory.fx_dir_entry_name[0])
        {

            /* Start at the current working directory of the media.  */
            search_dir =   ((FX_PATH *)_tx_thread_current_ptr -> tx_thread_filex_ptr) -> fx_path_directory;

            /* Set the internal pointer to the search directory as well.  */
            search_dir_ptr =  &search_dir;
        }
        else
        {

            /* We are searching in the root directory.  */
            search_dir_ptr =  FX_NULL;
        }
    }
    else
#endif
    if (media_ptr -> fx_media_default_path.fx_path_directory.fx_dir_entry_name[0])
    {

        /* Start at the current working directory of the media.  */
        search_dir =  media_ptr -> fx_media_default_path.fx_path_directory;

        /* Set the internal pointer to the search directory as well.  */
        search_dir_ptr =  &search_dir;
    }
    else
    {

        /* The current default directory is the root so just set the
           search directory pointer to NULL.  */
        search_dir_ptr =  FX_NULL;
    }

    /* Calculate the directory size.  */
    if (search_dir_ptr)
    {

        /* Ensure that the search directory's last search cluster is cleared.  */
        search_dir_ptr -> fx_dir_entry_last_search_cluster =  0;

        /* Calculate the directory size by counting the allocated
           clusters for it.  */
        i =        0;
        cluster =  search_dir_ptr -> fx_dir_entry_cluster;
        while ((cluster >= FX_FAT_ENTRY_START) && (cluster < media_ptr -> fx_media_fat_reserved))
        {

            /* Increment the cluster count.  */
            i++;

            /* Read the next FAT entry.  */
            status =  _fx_utility_FAT_entry_read(media_ptr, cluster, &next_cluster);

            /* Check the return status.  */
            if (status != FX_SUCCESS)
            {

                /* Return the bad status.  */
                return(status);
            }

            /* Check for error situation.  */
            if ((cluster == next_cluster) || (i > media_ptr -> fx_media_total_clusters))
            {

                /* Return the bad status.  */
                return(FX_FAT_READ_ERROR);
            }

            cluster = next_cluster;
        }

        /* Now we can calculate the directory size.  */
        directory_size =  (((ULONG)media_ptr -> fx_media_bytes_per_sector) *
                           ((ULONG)media_ptr -> fx_media_sectors_per_cluster) * i) /
                           (ULONG)FX_DIR_ENTRY_SIZE;

        /* Also save this in the directory entry so we don't have to
           calculate it later.  */
        search_dir_ptr -> fx_dir_entry_file_size =  directory_size;
    }
    else
    {

        /* Directory size is the number of entries in the root directory.  */
        directory_size =  (ULONG)media_ptr -> fx_media_root_directory_entries;
    }

    /* Determine if we are searching for a short file name or a unicode file name.  */
    if (short_name[0] == 0)
    {

        /* If the unicode name fit into short name length, covert the Unicode to ASCII if possible.  */
        if (local_unicode_name_length <= 13)
        {
            for (j = 0; j < local_unicode_name_length; j++)
            {
                if ((unicode_name[j * 2] <= 0x7F) && (unicode_name[j * 2 + 1] == 0))
                {

                    unicode_to_short_name[j] = (CHAR)unicode_name[j * 2];
                    if ((unicode_to_short_name[j] >= 'a') && (unicode_to_short_name[j] <= 'z'))
                    {

                        /* Lower case, convert to upper case!  */
                        unicode_to_short_name[j] =  (CHAR)((INT)unicode_to_short_name[j] - 0x20);
                    }
                }
                else
                {
                    unicode_to_short_name[0] = 0;
                    break;
                }
            }
        }
        else
        {
            unicode_to_short_name[0] = 0;
        }
    }
    else
    {
        unicode_to_short_name[0] = 0;
    }

    /* Loop through entries in the directory.  Yes, this is a
       linear search!  */
    i =      0;
    do
    {

        /* Read an entry from the directory.  */
        status =  _fx_unicode_directory_entry_read(media_ptr, search_dir_ptr, &i, entry_ptr, &_fx_unicode_search_name[0], &unicode_search_length);
        i++;

        /* Check for error status.  */
        if (status != FX_SUCCESS)
        {
            return(status);
        }

        /* Determine if this is an empty entry.  */
        if (((UCHAR)entry_ptr -> fx_dir_entry_name[0] == (UCHAR)FX_DIR_ENTRY_FREE) && (entry_ptr -> fx_dir_entry_short_name[0] == 0))
        {
            continue;
        }

        /* Determine if this is the last directory entry.  */
        if ((UCHAR)entry_ptr -> fx_dir_entry_name[0] == (UCHAR)FX_DIR_ENTRY_DONE)
        {
            break;
        }

        /* Determine if there is a short name to match.  */
        if (unicode_to_short_name[0])
        {

            /* Get the short name pointer.  */
            if (entry_ptr -> fx_dir_entry_short_name[0])
            {
                short_name_ptr =  entry_ptr -> fx_dir_entry_short_name;
            }
            else
            {
                short_name_ptr =  entry_ptr -> fx_dir_entry_name;
            }

            for (j = 0; j < local_unicode_name_length; j++)
            {

                /* Compare characters.  */
                if (short_name_ptr[j] != unicode_to_short_name[j])
                {
                    break;
                }
            }
            if (j == local_unicode_name_length)
            {

                /* Only the 1st 13 bytes or the buffer length is copied, whichever is smaller.  */
                if (short_name_buffer_length > 13)
                {
                    short_name_buffer_length = 13;
                }

                /* The names match, copy the short name into the destination.  */
                for (j = 0; j < short_name_buffer_length; j++)
                {
                    short_name[j] = (UCHAR)short_name_ptr[j];
                }

                /* Return success to caller.  */
                return(FX_SUCCESS);
            }
        }

        /* Determine if this is not a unicode name.  */
        if (unicode_search_length == 0)
        {
            continue;
        }

        /* Determine if we are searching for a short file name or a unicode file name.  */
        if (short_name[0])
        {

            /* We have a short name and need a unicode name.  Compare the short name against the short name in
               the directory entry for a match.  */
            found =  FX_TRUE;
            for (j = 0; j < 12; j++)
            {

                /* Compare characters...  */
                if (entry_ptr -> fx_dir_entry_short_name[0])
                {

                    /* Yes, the return name is in the short name field... compare against it!  */
                    if (short_name[j] != (UCHAR)entry_ptr -> fx_dir_entry_short_name[j])
                    {

                        found = FX_FALSE;
                        break;
                    }
                }
                else
                {

                    /* No, the return name is in the name field... compare against it!  */
                    if (short_name[j] != (UCHAR)entry_ptr -> fx_dir_entry_name[j])
                    {

                        found = FX_FALSE;
                        break;
                    }
                }

                /* Are we done?  */
                if (short_name[j] == (UCHAR)FX_NULL)
                {
                    break;
                }
            }

            /* One final compare to see if we have a match.  */
            if ((found == FX_FALSE) || ((j == 12) && (short_name[12] != 0)))
            {
                continue;
            }

            /* A match was found so copy the unicode name and length and return.  */
            /* Copy the length.  */
            *unicode_name_length =  unicode_search_length;
            
            /* Check if the name fit in the buffer.  */
            if (unicode_name_buffer_length < (unicode_search_length + 1) * 2)
            {
                unicode_search_length = (unicode_name_buffer_length - 2) / 2;
            }

            /* Copy the unicode name.   */
            for (j = 0; j < unicode_search_length * 2; j++)
            {

                /* Copy one unicode character to the destination...  */
                unicode_name[j] =  _fx_unicode_search_name[j];
            }

            /* Make sure there is a NULL in the destination.  */
            unicode_name[j] =    (UCHAR)0;
            unicode_name[j + 1] =  (UCHAR)0;



            /* Return successful completion.  */
            return(FX_SUCCESS);
        }
        else
        {

            /* Determine if this is the correct unicode name.  */
            if (unicode_search_length != local_unicode_name_length)
            {
                continue;
            }

            /* Compare the unicode search name with the requested unicode name.  */
            for (j = 0; j < (local_unicode_name_length * 2); j = j + 2)
            {

                /* Compare bytes of each unicode name.  */
                if (unicode_name[j] != _fx_unicode_search_name[j])
                {

                    /* Not match, Check if the character is in ASCII range.  */
                    if ((_fx_unicode_search_name[j + 1] == 0) && (unicode_name[j + 1] == 0))
                    {

                        /* Check if it is case mismatch.  */
                        if ((unicode_name[j]) >= 'a' && (unicode_name[j] <= 'z'))
                        {
                            if ((unicode_name[j] - 0x20) == _fx_unicode_search_name[j])
                            {
                                continue;
                            }
                        }
                        if ((_fx_unicode_search_name[j]) >= 'a' && (_fx_unicode_search_name[j] <= 'z'))
                        {
                            if ((_fx_unicode_search_name[j] - 0x20) == unicode_name[j])
                            {
                                continue;
                            }
                        }
                    }

                    break;
                }

                /* Compare the next byte.  */
                if (unicode_name[j + 1] != _fx_unicode_search_name[j + 1])
                {
                    break;
                }
            }

            /* Determine if the names do not match.  */
            if (j != (local_unicode_name_length * 2))
            {
                continue;
            }

            /* Otherwise, the names match, copy the short name into the destination.  */
            /* Only the 1st 13 bytes or the buffer length is copied, whichever is smaller.  */
            if (short_name_buffer_length > 13)
            {
                short_name_buffer_length = 13;
            }
            for (j = 0; j < short_name_buffer_length; j++)
            {

                /* Copy a character.  */
                if (entry_ptr -> fx_dir_entry_short_name[0])
                {
                    short_name[j] =  (UCHAR)entry_ptr -> fx_dir_entry_short_name[j];
                }
                else
                {
                    short_name[j] =  (UCHAR)entry_ptr -> fx_dir_entry_name[j];
                }
            }

            /* Return success to caller.  */
            return(FX_SUCCESS);
        }
    } while (i < directory_size);

    /* Return not found.  */
    return(FX_NOT_FOUND);
}

