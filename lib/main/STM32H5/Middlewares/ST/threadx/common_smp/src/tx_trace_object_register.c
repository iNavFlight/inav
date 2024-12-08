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
/** ThreadX Component                                                     */
/**                                                                       */
/**   Trace                                                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_trace.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _tx_trace_object_register                           PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function registers a ThreadX system object in the trace        */
/*    registry area. This provides a mapping between the object pointers  */
/*    stored in each trace event to the actual ThreadX objects.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    object_type                           Type of system object         */
/*    object_ptr                            Address of system object      */
/*    object_name                           Name of system object         */
/*    parameter_1                           Supplemental parameter 1      */
/*    parameter_2                           Supplemental parameter 2      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020      William E. Lamie        Initial Version 6.0           */
/*  09-30-2020      Yuxin Zhou              Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  07-29-2022      Scott Larson            Modified comment(s),          */
/*                                            check for null name,        */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
VOID  _tx_trace_object_register(UCHAR object_type, VOID *object_ptr, CHAR *object_name, ULONG parameter_1, ULONG parameter_2)
{

#ifdef TX_ENABLE_EVENT_TRACE

UINT                            i, entries;
UINT                            found, loop_break;
TX_THREAD                       *thread_ptr;
UCHAR                           *work_ptr;
TX_TRACE_OBJECT_ENTRY           *entry_ptr;


    /* Determine if the registry area is setup.  */
    if (_tx_trace_registry_start_ptr != TX_NULL)
    {

        /* Trace buffer is enabled, proceed.  */

        /* Pickup the total entries.  */
        entries =  _tx_trace_total_registry_entries;

        /* Determine if there are available entries in the registry.  */
        if (_tx_trace_available_registry_entries != ((ULONG) 0))
        {

            /* There are more available entries, proceed.  */

            /* Initialize found to the max entries... indicating no space was found.  */
            found =       entries;
            loop_break =  TX_FALSE;

            /* Loop to find available entry.  */
            i =  _tx_trace_registry_search_start;
            do
            {

                /* Setup the registry entry pointer.  */
                work_ptr =   TX_OBJECT_TO_UCHAR_POINTER_CONVERT(_tx_trace_registry_start_ptr);
                work_ptr =   TX_UCHAR_POINTER_ADD(work_ptr, ((sizeof(TX_TRACE_OBJECT_ENTRY))*i));
                entry_ptr =  TX_UCHAR_TO_OBJECT_POINTER_CONVERT(work_ptr);

                /* Determine if this is the first pass building the registry. A NULL object value indicates this part
                   of the registry has never been used.  */
                if (entry_ptr -> tx_trace_object_entry_thread_pointer == (ULONG) 0)
                {

                    /* Set found to this index and break out of the loop.  */
                    found =  i;
                    loop_break =  TX_TRUE;
                }

                /* Determine if this entry matches the object pointer... we must reuse old entries left in the
                   registry.  */
                if (entry_ptr -> tx_trace_object_entry_thread_pointer == TX_POINTER_TO_ULONG_CONVERT(object_ptr))
                {

                    /* Set found to this index and break out of the loop.  */
                    found =  i;
                    loop_break =  TX_TRUE;
                }

                /* Determine if we should break out of the loop.  */
                if (loop_break == TX_TRUE)
                {

                    /* Yes, break out of the loop.  */
                    break;
                }

                /* Is this entry available?  */
                if (entry_ptr -> tx_trace_object_entry_available == TX_TRUE)
                {

                    /* Yes, determine if we have not already found an empty slot.  */
                    if (found == entries)
                    {
                        found =  i;
                    }
                    else
                    {

                        /* Setup a pointer to the found entry.  */
                        work_ptr =   TX_OBJECT_TO_UCHAR_POINTER_CONVERT(_tx_trace_registry_start_ptr);
                        work_ptr =   TX_UCHAR_POINTER_ADD(work_ptr, ((sizeof(TX_TRACE_OBJECT_ENTRY))*found));
                        entry_ptr =  TX_UCHAR_TO_OBJECT_POINTER_CONVERT(work_ptr);

                         if (entry_ptr -> tx_trace_object_entry_type != ((UCHAR) 0))
                         {
                            found =  i;
                         }
                    }
                }

                /* Move to the next entry.  */
                i++;

                /* Determine if we have wrapped the list.  */
                if (i >= entries)
                {

                    /* Yes, wrap to the beginning of the list.  */
                    i =  ((ULONG) 0);
                }

            } while (i != _tx_trace_registry_search_start);

            /* Now determine if an empty or reuse entry has been found.  */
            if (found < entries)
            {

                /* Decrement the number of available entries.  */
                _tx_trace_available_registry_entries--;

                /* Adjust the search index to the next entry.  */
                if ((found + ((ULONG) 1)) < entries)
                {

                    /* Start searching from the next index.  */
                    _tx_trace_registry_search_start =  found + ((ULONG) 1);
                }
                else
                {

                    /* Reset the search to the beginning of the list. */
                    _tx_trace_registry_search_start =  ((ULONG) 0);
                }

                /* Yes, an entry has been found...  */

                /* Build a pointer to the found entry.  */
                work_ptr =   TX_OBJECT_TO_UCHAR_POINTER_CONVERT(_tx_trace_registry_start_ptr);
                work_ptr =   TX_UCHAR_POINTER_ADD(work_ptr, ((sizeof(TX_TRACE_OBJECT_ENTRY))*found));
                entry_ptr =  TX_UCHAR_TO_OBJECT_POINTER_CONVERT(work_ptr);

                /* Populate the found entry!  */
                entry_ptr -> tx_trace_object_entry_available =       ((UCHAR) TX_FALSE);
                entry_ptr -> tx_trace_object_entry_type =            object_type;
                entry_ptr -> tx_trace_object_entry_thread_pointer =  TX_POINTER_TO_ULONG_CONVERT(object_ptr);
                entry_ptr -> tx_trace_object_entry_param_1 =         parameter_1;
                entry_ptr -> tx_trace_object_entry_param_2 =         parameter_2;

                /* Loop to copy the object name string...  */
                for (i = ((ULONG) 0); i < (((ULONG) TX_TRACE_OBJECT_REGISTRY_NAME)-((ULONG) 1)); i++)
                {

                    /* Setup work pointer to the object name character.  */
                    work_ptr =  TX_CHAR_TO_UCHAR_POINTER_CONVERT(object_name);
                    work_ptr =  TX_UCHAR_POINTER_ADD(work_ptr, i);

                    /* Determine if object_name (work_ptr) is null.  */
                    if (work_ptr == TX_NULL)
                    {
                        break;
                    }

                    /* Copy a character of the name.  */
                    entry_ptr -> tx_trace_object_entry_name[i] =  (UCHAR) *work_ptr;

                    /* Determine if we are at the end.  */
                    if (*work_ptr == ((UCHAR) 0))
                    {
                        break;
                    }
                }

                /* Null terminate the object string.  */
                entry_ptr -> tx_trace_object_entry_name[i] =  (UCHAR) 0;

                /* Determine if a thread object type is present.  */
                if (object_type == TX_TRACE_OBJECT_TYPE_THREAD)
                {

                    /* Yes, a thread object is present.  */

                    /* Setup a pointer to the thread.  */
                    thread_ptr =  TX_VOID_TO_THREAD_POINTER_CONVERT(object_ptr);

                    /* Store the thread's priority in the reserved bits.  */
                    entry_ptr -> tx_trace_object_entry_reserved1 =  ((UCHAR) 0x80) | ((UCHAR) (thread_ptr -> tx_thread_priority >> ((UCHAR) 8)));
                    entry_ptr -> tx_trace_object_entry_reserved2 =  (UCHAR) (thread_ptr -> tx_thread_priority & ((UCHAR) 0xFF));
                }
                else
                {

                    /* For all other objects, set the reserved bytes to 0.  */
                    entry_ptr -> tx_trace_object_entry_reserved1 =  ((UCHAR) 0);
                    entry_ptr -> tx_trace_object_entry_reserved2 =  ((UCHAR) 0);
                }
            }
        }
    }
#else

TX_INTERRUPT_SAVE_AREA


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (object_type != ((UCHAR) 0))
    {

        if (object_ptr != TX_NULL)
        {

            if (object_name != TX_NULL)
            {

                if (parameter_1 != ((ULONG) 0))
                {

                    if (parameter_2 != ((ULONG) 0))
                    {

                        /* NOP code.  */
                        TX_DISABLE
                        TX_RESTORE
                    }
                }
            }
        }
    }
#endif
}

