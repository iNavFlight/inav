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
/*    _tx_trace_object_unregister                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    William E. Lamie, Microsoft Corporation                             */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function unregisters a ThreadX system object from the trace    */
/*    registry area.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    object_pointer                        Address of system object      */
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
/*  05-19-2020     William E. Lamie         Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _tx_trace_object_unregister(VOID *object_ptr)
{

#ifdef TX_ENABLE_EVENT_TRACE

UINT                            i, entries;
UCHAR                           *work_ptr;
TX_TRACE_OBJECT_ENTRY           *entry_ptr;


    /* Determine if the registry area is setup.  */
    if (_tx_trace_registry_start_ptr != TX_NULL)
    {

        /* Registry is setup, proceed.  */

        /* Pickup the total entries.  */
        entries =  _tx_trace_total_registry_entries;

        /* Loop to find available entry.  */
        for (i = ((ULONG) 0); i < entries; i++)
        {

            /* Setup the registry entry pointer.  */
            work_ptr =   TX_OBJECT_TO_UCHAR_POINTER_CONVERT(_tx_trace_registry_start_ptr);
            work_ptr =   TX_UCHAR_POINTER_ADD(work_ptr, ((sizeof(TX_TRACE_OBJECT_ENTRY))*i));
            entry_ptr =  TX_UCHAR_TO_OBJECT_POINTER_CONVERT(work_ptr);

            /* Determine if this entry matches the object pointer... */
            if (entry_ptr -> tx_trace_object_entry_thread_pointer == TX_POINTER_TO_ULONG_CONVERT(object_ptr))
            {

                /* Mark this entry as available, but leave the other information so that old trace entries can
                   still find it - if necessary!  */
                entry_ptr -> tx_trace_object_entry_available =  ((UCHAR) TX_TRUE);

                /* Increment the number of available registry entries.  */
                _tx_trace_available_registry_entries++;

                /* Adjust the search index to this position.  */
                _tx_trace_registry_search_start =  i;

                break;
            }
        }
    }
#else

TX_INTERRUPT_SAVE_AREA


    /* Access input arguments just for the sake of lint, MISRA, etc.  */
    if (object_ptr != TX_NULL)
    {

        /* NOP code.  */
        TX_DISABLE
        TX_RESTORE
    }
#endif
}

