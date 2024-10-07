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
/**   Thread                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define TX_SOURCE_CODE
#define TX_THREAD_SMP_SOURCE_CODE


/* Include necessary system files.  */

#include "tx_api.h"
#include "tx_thread.h"


#ifdef TX_DISABLE_INLINE

/* Define the routine to calculate the lowest set bit.  */

UINT  _tx_thread_lowest_set_bit_calculate(ULONG map)
{
UINT    bit_set;

    if ((map & ((ULONG) 0x1)) != ((ULONG) 0))
    {
        bit_set = ((UINT) 0);
    }
    else
    {
        map =  map & (ULONG) ((~map) + ((ULONG) 1));
        if (map < ((ULONG) 0x100))
        {
            bit_set = ((UINT) 1);
        }
        else if (map < ((ULONG) 0x10000))
        {
            bit_set =  ((UINT) 9);
            map =  map >> ((UINT) 8);
        }
        else if (map < ((ULONG) 0x01000000))
        {
            bit_set = ((UINT) 17);
            map = map >> ((UINT) 16);
        }
        else
        {
            bit_set = ((UINT) 25);
            map = map >> ((UINT) 24);
        }
        if (map >= ((ULONG) 0x10))
        {
            map = map >> ((UINT) 4);
            bit_set = bit_set + ((UINT) 4);
        }
        if (map >= ((ULONG) 0x4))
        {
            map = map >> ((UINT) 2);
            bit_set = bit_set + ((UINT) 2);
        }
        bit_set = bit_set - (UINT) (map & (ULONG) 0x1);
    }

    return(bit_set);
}


/* Define the next priority macro. Note, that this may be overridden
   by a port specific definition.  */

#if TX_MAX_PRIORITIES > 32

UINT _tx_thread_smp_next_priority_find(UINT priority)
{
ULONG           map_index;
ULONG           local_priority_map_active;
ULONG           local_priority_map;
ULONG           priority_bit;
ULONG           first_bit_set;
ULONG           found_priority;

    found_priority =  ((UINT) TX_MAX_PRIORITIES);
    if (priority < ((UINT) TX_MAX_PRIORITIES))
    {
        map_index =  priority/((UINT) 32);
        local_priority_map =  _tx_thread_priority_maps[map_index];
        priority_bit =        (((ULONG) 1) << (priority % ((UINT) 32)));
        local_priority_map =  local_priority_map & ~(priority_bit - ((UINT)1));
        if (local_priority_map != ((ULONG) 0))
        {
            TX_LOWEST_SET_BIT_CALCULATE(local_priority_map, first_bit_set)
            found_priority =  (map_index * ((UINT) 32)) + first_bit_set;
        }
        else
        {
            /* Move to next map index.  */
            map_index++;
            if (map_index < (((UINT) TX_MAX_PRIORITIES)/((UINT) 32)))
            {
                priority_bit =               (((ULONG) 1) << (map_index));
                local_priority_map_active =  _tx_thread_priority_map_active & ~(priority_bit - ((UINT) 1));
                if (local_priority_map_active != ((ULONG) 0))
                {
                    TX_LOWEST_SET_BIT_CALCULATE(local_priority_map_active, map_index)
                    local_priority_map =  _tx_thread_priority_maps[map_index];
                    TX_LOWEST_SET_BIT_CALCULATE(local_priority_map, first_bit_set)
                    found_priority =  (map_index * ((UINT) 32)) + first_bit_set;
                }
            }
        }
    }
    return(found_priority);
}
#else

UINT _tx_thread_smp_next_priority_find(UINT priority)
{
UINT            first_bit_set;
ULONG           local_priority_map;
UINT            next_priority;

    local_priority_map =  _tx_thread_priority_maps[0];
    local_priority_map =  local_priority_map >> priority;
    next_priority =  priority;
    if (local_priority_map == ((ULONG) 0))
    {
        next_priority =  ((UINT) TX_MAX_PRIORITIES);
    }
    else
    {
        if (next_priority >= ((UINT) TX_MAX_PRIORITIES))
        {
            next_priority =  ((UINT) TX_MAX_PRIORITIES);
        }
        else
        {
            TX_LOWEST_SET_BIT_CALCULATE(local_priority_map, first_bit_set)
            next_priority =  priority + first_bit_set;
        }
    }

    return(next_priority);
}
#endif


void  _tx_thread_smp_schedule_list_clear(void)
{
#if TX_THREAD_SMP_MAX_CORES > 6
UINT    i;
#endif


    /* Clear the schedule list.  */
    _tx_thread_smp_schedule_list[0] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 1
    _tx_thread_smp_schedule_list[1] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 2
    _tx_thread_smp_schedule_list[2] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 3
    _tx_thread_smp_schedule_list[3] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 4
    _tx_thread_smp_schedule_list[4] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 5
    _tx_thread_smp_schedule_list[5] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 6

    /* Loop to clear the remainder of the schedule list.  */
    i =  ((UINT) 6);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

    while (i < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (i < _tx_thread_smp_max_cores)
#endif
    {
        /* Clear entry in schedule list.  */
        _tx_thread_smp_schedule_list[i] =  TX_NULL;

        /* Move to next index.  */
        i++;
    }
#endif
#endif
#endif
#endif
#endif
#endif
}

VOID  _tx_thread_smp_execute_list_clear(void)
{
#if TX_THREAD_SMP_MAX_CORES > 6
UINT    j;
#endif

    /* Clear the execute list.  */
    _tx_thread_execute_ptr[0] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 1
    _tx_thread_execute_ptr[1] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 2
    _tx_thread_execute_ptr[2] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 3
    _tx_thread_execute_ptr[3] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 4
    _tx_thread_execute_ptr[4] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 5
    _tx_thread_execute_ptr[5] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 6

    /* Loop to clear the remainder of the execute list.  */
    j =  ((UINT) 6);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

    while (j < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (j < _tx_thread_smp_max_cores)
#endif
    {

        /* Clear entry in execute list.  */
        _tx_thread_execute_ptr[j] =  TX_NULL;

        /* Move to next index.  */
        j++;
    }
#endif
#endif
#endif
#endif
#endif
#endif
}


VOID  _tx_thread_smp_schedule_list_setup(void)
{
#if TX_THREAD_SMP_MAX_CORES > 6
UINT    j;
#endif

    _tx_thread_smp_schedule_list[0] =  _tx_thread_execute_ptr[0];
#if TX_THREAD_SMP_MAX_CORES > 1
    _tx_thread_smp_schedule_list[1] =  _tx_thread_execute_ptr[1];
#if TX_THREAD_SMP_MAX_CORES > 2
    _tx_thread_smp_schedule_list[2] =  _tx_thread_execute_ptr[2];
#if TX_THREAD_SMP_MAX_CORES > 3
    _tx_thread_smp_schedule_list[3] =  _tx_thread_execute_ptr[3];
#if TX_THREAD_SMP_MAX_CORES > 4
    _tx_thread_smp_schedule_list[4] =  _tx_thread_execute_ptr[4];
#if TX_THREAD_SMP_MAX_CORES > 5
    _tx_thread_smp_schedule_list[5] =  _tx_thread_execute_ptr[5];
#if TX_THREAD_SMP_MAX_CORES > 6

    /* Loop to setup the remainder of the schedule list.  */
    j =  ((UINT) 6);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
    while (j < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (j < _tx_thread_smp_max_cores)
#endif
    {

        /* Setup entry in schedule list.  */
        _tx_thread_smp_schedule_list[j] =  _tx_thread_execute_ptr[j];

        /* Move to next index.  */
        j++;
    }
#endif
#endif
#endif
#endif
#endif
#endif
}


#ifdef TX_THREAD_SMP_INTER_CORE_INTERRUPT
VOID  _tx_thread_smp_core_interrupt(TX_THREAD *thread_ptr, UINT current_core, UINT target_core)
{

TX_THREAD   *current_thread;


    /* Make sure this is a different core, since there is no need to interrupt the current core for
       a scheduling change.  */
    if (current_core != target_core)
    {

        /* Yes, a different core is present.  */

        /* Pickup the currently executing thread.  */
        current_thread =  _tx_thread_current_ptr[target_core];

        /* Determine if they are the same.  */
        if ((current_thread != TX_NULL) && (thread_ptr != current_thread))
        {

            /* Not the same and not NULL... determine if the core is running at thread level.  */
            if (_tx_thread_system_state[target_core] < TX_INITIALIZE_IN_PROGRESS)
            {

                /* Preempt the mapped thread.  */
                _tx_thread_smp_core_preempt(target_core);
            }
        }
    }
}
#endif


#ifdef TX_THREAD_SMP_WAKEUP_LOGIC
VOID  _tx_thread_smp_core_wakeup(UINT current_core, UINT target_core)
{

    /* Determine if the core specified is not the current core - no need to wakeup the
       current core.  */
    if (target_core != current_core)
    {

        /* Wakeup based on application's macro.  */
        TX_THREAD_SMP_WAKEUP(target_core);
    }
}
#endif


VOID  _tx_thread_smp_execute_list_setup(UINT core_index)
{

TX_THREAD   *schedule_thread;
UINT        i;


    /* Loop to copy the schedule list into the execution list.  */
    i =  ((UINT) 0);
#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

    while (i < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (i < _tx_thread_smp_max_cores)
#endif
    {

        /* Pickup the thread to schedule.  */
        schedule_thread =  _tx_thread_smp_schedule_list[i];

        /* Copy the schedule list into the execution list.  */
        _tx_thread_execute_ptr[i] =  schedule_thread;

        /* If necessary, interrupt the core with the new thread to schedule.  */
        _tx_thread_smp_core_interrupt(schedule_thread, core_index, i);

#ifdef TX_THREAD_SMP_WAKEUP_LOGIC

        /* Does this need to be waked up?  */
        if ((i != core_index) && (schedule_thread != TX_NULL))
        {

            /* Wakeup based on application's macro.  */
            TX_THREAD_SMP_WAKEUP(i);
        }
#endif
        /* Move to next index.  */
        i++;
    }
}


ULONG  _tx_thread_smp_available_cores_get(void)
{

#if TX_THREAD_SMP_MAX_CORES > 6
UINT    j;
#endif
ULONG   available_cores;

    available_cores =  ((ULONG) 0);
    if (_tx_thread_execute_ptr[0] == TX_NULL)
    {
        available_cores =  ((ULONG) 1);
    }
#if TX_THREAD_SMP_MAX_CORES > 1
    if (_tx_thread_execute_ptr[1] == TX_NULL)
    {
        available_cores =  available_cores | ((ULONG) 2);
    }
#if TX_THREAD_SMP_MAX_CORES > 2
    if (_tx_thread_execute_ptr[2] == TX_NULL)
    {
        available_cores =  available_cores | ((ULONG) 4);
    }
#if TX_THREAD_SMP_MAX_CORES > 3
    if (_tx_thread_execute_ptr[3] == TX_NULL)
    {
        available_cores =  available_cores | ((ULONG) 8);
    }
#if TX_THREAD_SMP_MAX_CORES > 4
    if (_tx_thread_execute_ptr[4] == TX_NULL)
    {
        available_cores =  available_cores | ((ULONG) 0x10);
    }
#if TX_THREAD_SMP_MAX_CORES > 5
    if (_tx_thread_execute_ptr[5] == TX_NULL)
    {
        available_cores =  available_cores | ((ULONG) 0x20);
    }
#if TX_THREAD_SMP_MAX_CORES > 6

    /* Loop to setup the remainder of the schedule list.  */
    j =  ((UINT) 6);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
    while (j < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (j < _tx_thread_smp_max_cores)
#endif
    {

        /* Determine if this core is available.  */
        if (_tx_thread_execute_ptr[j] == TX_NULL)
        {
            available_cores =  available_cores | (((ULONG) 1) << j);
        }

        /* Move to next core.  */
        j++;
    }
#endif
#endif
#endif
#endif
#endif
#endif
    return(available_cores);
}


ULONG  _tx_thread_smp_possible_cores_get(void)
{

#if TX_THREAD_SMP_MAX_CORES > 6
UINT    j;
#endif
ULONG       possible_cores;
TX_THREAD   *thread_ptr;

    possible_cores =  ((ULONG) 0);
    thread_ptr =  _tx_thread_execute_ptr[0];
    if (thread_ptr != TX_NULL)
    {
        possible_cores =  thread_ptr -> tx_thread_smp_cores_allowed;
    }
#if TX_THREAD_SMP_MAX_CORES > 1
    thread_ptr =  _tx_thread_execute_ptr[1];
    if (thread_ptr != TX_NULL)
    {
        possible_cores =  possible_cores | thread_ptr -> tx_thread_smp_cores_allowed;
    }
#if TX_THREAD_SMP_MAX_CORES > 2
    thread_ptr =  _tx_thread_execute_ptr[2];
    if (thread_ptr != TX_NULL)
    {
        possible_cores =  possible_cores | thread_ptr -> tx_thread_smp_cores_allowed;
    }
#if TX_THREAD_SMP_MAX_CORES > 3
    thread_ptr =  _tx_thread_execute_ptr[3];
    if (thread_ptr != TX_NULL)
    {
        possible_cores =  possible_cores | thread_ptr -> tx_thread_smp_cores_allowed;
    }
#if TX_THREAD_SMP_MAX_CORES > 4
    thread_ptr =  _tx_thread_execute_ptr[4];
    if (thread_ptr != TX_NULL)
    {
        possible_cores =  possible_cores | thread_ptr -> tx_thread_smp_cores_allowed;
    }
#if TX_THREAD_SMP_MAX_CORES > 5
    thread_ptr =  _tx_thread_execute_ptr[5];
    if (thread_ptr != TX_NULL)
    {
        possible_cores =  possible_cores | thread_ptr -> tx_thread_smp_cores_allowed;
    }
#if TX_THREAD_SMP_MAX_CORES > 6

    /* Loop to setup the remainder of the schedule list.  */
    j =  ((UINT) 6);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
    while (j < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (j < _tx_thread_smp_max_cores)
#endif
    {

        /* Determine if this core is available.  */
        thread_ptr =  _tx_thread_execute_ptr[j];
        if (thread_ptr != TX_NULL)
        {
            possible_cores =  possible_cores | thread_ptr -> tx_thread_smp_cores_allowed;
        }

        /* Move to next core.  */
        j++;
    }
#endif
#endif
#endif
#endif
#endif
#endif
    return(possible_cores);
}


UINT  _tx_thread_smp_lowest_priority_get(void)
{

#if TX_THREAD_SMP_MAX_CORES > 6
UINT    j;
#endif
TX_THREAD   *thread_ptr;
UINT        lowest_priority;

    lowest_priority =  ((UINT) 0);
    thread_ptr =  _tx_thread_execute_ptr[0];
    if (thread_ptr != TX_NULL)
    {
        if (thread_ptr -> tx_thread_priority > lowest_priority)
        {
            lowest_priority =  thread_ptr -> tx_thread_priority;
        }
    }
#if TX_THREAD_SMP_MAX_CORES > 1
    thread_ptr =  _tx_thread_execute_ptr[1];
    if (thread_ptr != TX_NULL)
    {
        if (thread_ptr -> tx_thread_priority > lowest_priority)
        {
            lowest_priority =  thread_ptr -> tx_thread_priority;
        }
    }
#if TX_THREAD_SMP_MAX_CORES > 2
    thread_ptr =  _tx_thread_execute_ptr[2];
    if (thread_ptr != TX_NULL)
    {
        if (thread_ptr -> tx_thread_priority > lowest_priority)
        {
            lowest_priority =  thread_ptr -> tx_thread_priority;
        }
    }
#if TX_THREAD_SMP_MAX_CORES > 3
    thread_ptr =  _tx_thread_execute_ptr[3];
    if (thread_ptr != TX_NULL)
    {
        if (thread_ptr -> tx_thread_priority > lowest_priority)
        {
            lowest_priority =  thread_ptr -> tx_thread_priority;
        }
    }
#if TX_THREAD_SMP_MAX_CORES > 4
    thread_ptr =  _tx_thread_execute_ptr[4];
    if (thread_ptr != TX_NULL)
    {
        if (thread_ptr -> tx_thread_priority > lowest_priority)
        {
            lowest_priority =  thread_ptr -> tx_thread_priority;
        }
    }
#if TX_THREAD_SMP_MAX_CORES > 5
    thread_ptr =  _tx_thread_execute_ptr[5];
    if (thread_ptr != TX_NULL)
    {
        if (thread_ptr -> tx_thread_priority > lowest_priority)
        {
            lowest_priority =  thread_ptr -> tx_thread_priority;
        }
    }
#if TX_THREAD_SMP_MAX_CORES > 6

    /* Loop to setup the remainder of the schedule list.  */
    j =  ((UINT) 6);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
    while (j < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (j < _tx_thread_smp_max_cores)
#endif
    {

        /* Determine if this core has a thread scheduled.  */
        thread_ptr =  _tx_thread_execute_ptr[j];
        if (thread_ptr != TX_NULL)
        {

            /* Is this the new lowest priority?  */
            if (thread_ptr -> tx_thread_priority > lowest_priority)
            {
                lowest_priority =  thread_ptr -> tx_thread_priority;
            }
        }

        /* Move to next core.  */
        j++;
    }
#endif
#endif
#endif
#endif
#endif
#endif
    return(lowest_priority);
}


UINT  _tx_thread_smp_remap_solution_find(TX_THREAD *schedule_thread, ULONG available_cores, ULONG thread_possible_cores, ULONG test_possible_cores)
{

UINT            core;
UINT            previous_core;
ULONG           test_cores;
ULONG           last_thread_cores;
UINT            queue_first, queue_last;
UINT            core_queue[TX_THREAD_SMP_MAX_CORES-1];
TX_THREAD       *thread_ptr;
TX_THREAD       *last_thread;
TX_THREAD       *thread_remap_list[TX_THREAD_SMP_MAX_CORES];


    /* Clear the last thread cores in the search.  */
    last_thread_cores =  ((ULONG) 0);

    /* Set the last thread pointer to NULL.  */
    last_thread =  TX_NULL;

    /* Setup the core queue indices.  */
    queue_first =  ((UINT) 0);
    queue_last =   ((UINT) 0);

    /* Build a list of possible cores for this thread to execute on, starting
       with the previously mapped core.  */
    core =  schedule_thread -> tx_thread_smp_core_mapped;
    if ((thread_possible_cores & (((ULONG) 1) << core)) != ((ULONG) 0))
    {

        /* Remember this potential mapping.  */
        thread_remap_list[core] =   schedule_thread;
        core_queue[queue_last] =    core;

        /* Move to next slot.  */
        queue_last++;

        /* Clear this core.  */
        thread_possible_cores =  thread_possible_cores & ~(((ULONG) 1) << core);
    }

    /* Loop to add additional possible cores.  */
    while (thread_possible_cores != ((ULONG) 0))
    {

        /* Determine the first possible core.  */
        test_cores =  thread_possible_cores;
        TX_LOWEST_SET_BIT_CALCULATE(test_cores, core)

        /* Clear this core.  */
        thread_possible_cores =  thread_possible_cores & ~(((ULONG) 1) << core);

        /* Remember this potential mapping.  */
        thread_remap_list[core] =  schedule_thread;
        core_queue[queue_last] =   core;

        /* Move to next slot.  */
        queue_last++;
    }

    /* Loop to evaluate the potential thread mappings, against what is already mapped.  */
    do
    {

        /* Pickup the next entry.  */
        core = core_queue[queue_first];

        /* Move to next slot.  */
        queue_first++;

        /* Retrieve the thread from the current mapping.  */
        thread_ptr =  _tx_thread_smp_schedule_list[core];

        /* Determine if there is a thread currently mapped to this core.  */
        if (thread_ptr != TX_NULL)
        {

            /* Determine the cores available for this thread.  */
            thread_possible_cores =  thread_ptr -> tx_thread_smp_cores_allowed;
            thread_possible_cores =  test_possible_cores & thread_possible_cores;

            /* Are there any possible cores for this thread?  */
            if (thread_possible_cores != ((ULONG) 0))
            {

                /* Determine if there are cores available for this thread.  */
                if ((thread_possible_cores & available_cores) != ((ULONG) 0))
                {

                    /* Yes, remember the final thread and cores that are valid for this thread.  */
                    last_thread_cores =  thread_possible_cores & available_cores;
                    last_thread =        thread_ptr;

                    /* We are done - get out of the loop!  */
                    break;
                }
                else
                {

                    /* Remove cores that will be added to the list.  */
                    test_possible_cores =  test_possible_cores & ~(thread_possible_cores);

                    /* Loop to add this thread to the potential mapping list.  */
                    do
                    {

                        /* Calculate the core.  */
                        test_cores =  thread_possible_cores;
                        TX_LOWEST_SET_BIT_CALCULATE(test_cores, core)

                        /* Clear this core.  */
                        thread_possible_cores =  thread_possible_cores & ~(((ULONG) 1) << core);

                        /* Remember this thread for remapping.  */
                        thread_remap_list[core] =  thread_ptr;

                        /* Remember this core.  */
                        core_queue[queue_last] =  core;

                        /* Move to next slot.  */
                        queue_last++;

                    } while (thread_possible_cores != ((ULONG) 0));
                }
            }
        }
    } while (queue_first != queue_last);

    /* Was a remapping solution found?  */
    if (last_thread != TX_NULL)
    {

        /* Pickup the core of the last thread to remap.  */
        core =  last_thread -> tx_thread_smp_core_mapped;

        /* Pickup the thread from the remapping list.  */
        thread_ptr =  thread_remap_list[core];

        /* Loop until we arrive at the thread we have been trying to map.  */
        while (thread_ptr != schedule_thread)
        {

            /* Move this thread in the schedule list.  */
            _tx_thread_smp_schedule_list[core] =  thread_ptr;

            /* Remember the previous core.  */
            previous_core =  core;

            /* Pickup the core of thread to remap.  */
            core =  thread_ptr -> tx_thread_smp_core_mapped;

            /* Save the new core mapping for this thread.  */
            thread_ptr -> tx_thread_smp_core_mapped =  previous_core;

            /* Move the next thread.  */
            thread_ptr =  thread_remap_list[core];
        }

        /* Save the remaining thread in the updated schedule list.  */
        _tx_thread_smp_schedule_list[core] =  thread_ptr;

        /* Update this thread's core mapping.  */
        thread_ptr -> tx_thread_smp_core_mapped =  core;

        /* Finally, setup the last thread in the remapping solution.  */
        test_cores =  last_thread_cores;
        TX_LOWEST_SET_BIT_CALCULATE(test_cores, core)

        /* Setup the last thread.  */
        _tx_thread_smp_schedule_list[core] =     last_thread;

        /* Remember the core mapping for this thread.  */
        last_thread -> tx_thread_smp_core_mapped =  core;
    }
    else
    {

        /* Set core to the maximum value in order to signal a remapping solution was not found.  */
        core =  ((UINT) TX_THREAD_SMP_MAX_CORES);
    }

    /* Return core to the caller.  */
    return(core);
}


ULONG  _tx_thread_smp_preemptable_threads_get(UINT priority, TX_THREAD *possible_preemption_list[])
{

UINT        i, j, k;
TX_THREAD   *thread_ptr;
TX_THREAD   *next_thread;
TX_THREAD   *search_thread;
TX_THREAD   *list_head;
ULONG       possible_cores =  ((ULONG) 0);


    /* Clear the possible preemption list.  */
    possible_preemption_list[0] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 1
    possible_preemption_list[1] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 2
    possible_preemption_list[2] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 3
    possible_preemption_list[3] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 4
    possible_preemption_list[4] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 5
    possible_preemption_list[5] =  TX_NULL;
#if TX_THREAD_SMP_MAX_CORES > 6

    /* Loop to clear the remainder of the possible preemption list.  */
    j =  ((UINT) 6);

#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX

    while (j < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (j < _tx_thread_smp_max_cores)
#endif
    {

        /* Clear entry in possible preemption list.  */
        possible_preemption_list[j] =  TX_NULL;

        /* Move to next core.  */
        j++;
    }
#endif
#endif
#endif
#endif
#endif
#endif

    /* Loop to build a list of threads of less priority.  */
    i =  ((UINT) 0);
    j =  ((UINT) 0);
#ifndef TX_THREAD_SMP_DYNAMIC_CORE_MAX
    while (i < ((UINT) TX_THREAD_SMP_MAX_CORES))
#else

    while (i < _tx_thread_smp_max_cores)
#endif
    {

        /* Pickup the currently mapped thread.  */
        thread_ptr =  _tx_thread_execute_ptr[i];

        /* Is there a thread scheduled for this core?  */
        if (thread_ptr != TX_NULL)
        {

            /* Update the possible cores bit map.  */
            possible_cores =  possible_cores | thread_ptr -> tx_thread_smp_cores_allowed;

            /* Can this thread be preempted?  */
            if (priority < thread_ptr -> tx_thread_priority)
            {

                /* Thread that can be added to the preemption possible list.  */

                /* Yes, this scheduled thread is lower priority, so add it to the preemption possible list.  */
                possible_preemption_list[j] =  thread_ptr;

                /* Move to next entry in preemption possible list.  */
                j++;
            }
        }

        /* Move to next core.  */
        i++;
    }

    /* Check to see if there are more than 2 threads that can be preempted.  */
    if (j > ((UINT) 1))
    {

        /* Yes, loop through the preemption possible list and sort by priority.  */
        i =  ((UINT) 0);
        do
        {

            /* Pickup preemptable thread.  */
            thread_ptr =  possible_preemption_list[i];

            /* Initialize the search index.  */
            k =  i + ((UINT) 1);

            /* Loop to get the lowest priority thread at the front of the list.  */
            while (k < j)
            {

                /* Pickup the next thread to evaluate.  */
                next_thread =  possible_preemption_list[k];

                /* Is this thread lower priority?  */
                if (next_thread -> tx_thread_priority > thread_ptr -> tx_thread_priority)
                {

                    /* Yes, swap the threads.  */
                    possible_preemption_list[i] =  next_thread;
                    possible_preemption_list[k] =  thread_ptr;
                    thread_ptr =  next_thread;
                }
                else
                {

                    /* Compare the thread priorities.  */
                    if (next_thread -> tx_thread_priority == thread_ptr -> tx_thread_priority)
                    {

                        /* Equal priority threads...  see which is in the ready list first.  */
                        search_thread =   thread_ptr -> tx_thread_ready_next;

                        /* Pickup the list head.  */
                        list_head =  _tx_thread_priority_list[thread_ptr -> tx_thread_priority];

                        /* Now loop to see if the next thread is after the current thread preemption.  */
                        while (search_thread != list_head)
                        {

                            /* Have we found the next thread?  */
                            if (search_thread == next_thread)
                            {

                                /* Yes, swap the threads.  */
                                possible_preemption_list[i] =  next_thread;
                                possible_preemption_list[k] =  thread_ptr;
                                thread_ptr =  next_thread;
                                break;
                            }

                            /* Move to the next thread.  */
                            search_thread =  search_thread -> tx_thread_ready_next;
                        }
                    }

                    /* Move to examine the next possible preemptable thread.  */
                    k++;
                }
            }

            /* We have found the lowest priority thread to preempt, now find the next lowest.  */
            i++;
        }
        while (i < (j-((UINT) 1)));
    }

    /* Return the possible cores.  */
    return(possible_cores);
}

VOID  _tx_thread_smp_simple_priority_change(TX_THREAD *thread_ptr, UINT new_priority)
{

UINT            priority;
ULONG           priority_bit;
TX_THREAD       *head_ptr;
TX_THREAD       *tail_ptr;
#if TX_MAX_PRIORITIES > 32
UINT            map_index;
#endif

    /* Pickup the priority.  */
    priority =  thread_ptr -> tx_thread_priority;

    /* Determine if there are other threads at this priority that are
       ready.  */
    if (thread_ptr -> tx_thread_ready_next != thread_ptr)
    {

        /* Yes, there are other threads at this priority ready.  */

        /* Just remove this thread from the priority list.  */
        (thread_ptr -> tx_thread_ready_next) -> tx_thread_ready_previous =    thread_ptr -> tx_thread_ready_previous;
        (thread_ptr -> tx_thread_ready_previous) -> tx_thread_ready_next =    thread_ptr -> tx_thread_ready_next;

        /* Determine if this is the head of the priority list.  */
        if (_tx_thread_priority_list[priority] == thread_ptr)
        {

            /* Update the head pointer of this priority list.  */
            _tx_thread_priority_list[priority] =  thread_ptr -> tx_thread_ready_next;
        }
    }
    else
    {

        /* This is the only thread at this priority ready to run.  Set the head
           pointer to NULL.  */
        _tx_thread_priority_list[priority] =    TX_NULL;

#if TX_MAX_PRIORITIES > 32

        /* Calculate the index into the bit map array.  */
        map_index =  priority/((UINT) 32);
#endif

        /* Clear this priority bit in the ready priority bit map.  */
        TX_MOD32_BIT_SET(priority, priority_bit)
        _tx_thread_priority_maps[MAP_INDEX] =  _tx_thread_priority_maps[MAP_INDEX] & (~(priority_bit));

#if TX_MAX_PRIORITIES > 32

        /* Determine if there are any other bits set in this priority map.  */
        if (_tx_thread_priority_maps[MAP_INDEX] == ((ULONG) 0))
        {

            /* No, clear the active bit to signify this priority map has nothing set.  */
            TX_DIV32_BIT_SET(priority, priority_bit)
            _tx_thread_priority_map_active =  _tx_thread_priority_map_active & (~(priority_bit));
        }
#endif
    }

    /* Determine if the actual thread priority should be setup, which is the
       case if the new priority is higher than the priority inheritance.  */
    if (new_priority < thread_ptr -> tx_thread_inherit_priority)
    {

        /* Change thread priority to the new user's priority.  */
        thread_ptr -> tx_thread_priority =           new_priority;
        thread_ptr -> tx_thread_preempt_threshold =  new_priority;
    }
    else
    {

        /* Change thread priority to the priority inheritance.  */
        thread_ptr -> tx_thread_priority =           thread_ptr -> tx_thread_inherit_priority;
        thread_ptr -> tx_thread_preempt_threshold =  thread_ptr -> tx_thread_inherit_priority;
    }

    /* Now, place the thread at the new priority level.  */

    /* Determine if there are other threads at this priority that are
       ready.  */
    head_ptr =  _tx_thread_priority_list[new_priority];
    if (head_ptr != TX_NULL)
    {

        /* Yes, there are other threads at this priority already ready.  */

        /* Just add this thread to the priority list.  */
        tail_ptr =                                 head_ptr -> tx_thread_ready_previous;
        tail_ptr -> tx_thread_ready_next =         thread_ptr;
        head_ptr -> tx_thread_ready_previous =     thread_ptr;
        thread_ptr -> tx_thread_ready_previous =   tail_ptr;
        thread_ptr -> tx_thread_ready_next =       head_ptr;
    }
    else
    {

        /* First thread at this priority ready.  Add to the front of the list.  */
        _tx_thread_priority_list[new_priority] =   thread_ptr;
        thread_ptr -> tx_thread_ready_next =       thread_ptr;
        thread_ptr -> tx_thread_ready_previous =   thread_ptr;

#if TX_MAX_PRIORITIES > 32

        /* Calculate the index into the bit map array.  */
        map_index =  new_priority/((UINT) 32);

        /* Set the active bit to remember that the priority map has something set.  */
        TX_DIV32_BIT_SET(new_priority, priority_bit)
        _tx_thread_priority_map_active =  _tx_thread_priority_map_active | priority_bit;
#endif

        /* Or in the thread's priority bit.  */
        TX_MOD32_BIT_SET(new_priority, priority_bit)
        _tx_thread_priority_maps[MAP_INDEX] =  _tx_thread_priority_maps[MAP_INDEX] | priority_bit;
    }
}

#endif


