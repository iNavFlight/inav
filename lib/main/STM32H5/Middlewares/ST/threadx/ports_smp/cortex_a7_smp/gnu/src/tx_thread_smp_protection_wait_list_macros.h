@/**************************************************************************/
@/*                                                                        */
@/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
@/*                                                                        */
@/*       This software is licensed under the Microsoft Software License   */
@/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
@/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
@/*       and in the root directory of this software.                      */
@/*                                                                        */
@/**************************************************************************/
@
@
@/**************************************************************************/
@/**************************************************************************/
@/**                                                                       */
@/** ThreadX Component                                                     */
@/**                                                                       */
@/**   Thread - Low Level SMP Support                                      */
@/**                                                                       */
@/**************************************************************************/
@/**************************************************************************/

    .macro _tx_thread_smp_protect_lock_got
@
@    /* Set the currently owned core.  */
@    _tx_thread_smp_protection.tx_thread_smp_protect_core = this_core;
@
    STR     r1, [r2, #8]                        @ Store this core
@
@    /* Increment the protection count. */
@    _tx_thread_smp_protection.tx_thread_smp_protect_count++;
@
    LDR     r3, [r2, #12]                       @ Pickup ownership count
    ADD     r3, r3, #1                          @ Increment ownership count
    STR     r3, [r2, #12]                       @ Store ownership count
    DMB

#ifdef TX_MPCORE_DEBUG_ENABLE
    LSL     r3, r1, #2                          @ Build offset to array indexes
    LDR     r4, =_tx_thread_current_ptr         @ Pickup start of the current thread array
    ADD     r4, r3, r4                          @ Build index into the current thread array
    LDR     r3, [r4]                            @ Pickup current thread for this core
    STR     r3, [r2, #4]                        @ Save current thread pointer
    STR     LR, [r2, #16]                       @ Save caller's return address
    STR     r0, [r2, #20]                       @ Save CPSR
#endif

    .endm

    .macro _tx_thread_smp_protect_remove_from_front_of_list
@
@    /* Remove ourselves from the list.  */
@    _tx_thread_smp_protect_wait_list[_tx_thread_smp_protect_wait_list_head++] =  0xFFFFFFFF;
@
    MOV     r3, #0xFFFFFFFF                     @ Build the invalid core value
    LDR     r4, =_tx_thread_smp_protect_wait_list_head @ Get the address of the head
    LDR     r5, [r4]                            @ Get the value of the head
    LDR     r6, =_tx_thread_smp_protect_wait_list @ Get the address of the list
    STR     r3, [r6, r5, LSL #2]                @ Store the invalid core value
    ADD     r5, r5, #1                          @ Increment the head
@
@    /* Did we wrap?  */
@    if (_tx_thread_smp_protect_wait_list_head == TX_THREAD_SMP_MAX_CORES + 1)
@    {
@
    LDR     r3, =_tx_thread_smp_protect_wait_list_size @ Load address of core list size
    LDR     r3, [r3]                            @ Load the max cores value
    CMP     r5, r3                              @ Compare the head to it
    BNE     _store_new_head\@                   @ Are we at the max?
@
@    _tx_thread_smp_protect_wait_list_head = 0;
@
    EOR     r5, r5, r5                          @ We're at the max. Set it to zero
@
@    }
@
_store_new_head\@:

    STR     r5, [r4]                            @ Store the new head
@
@    /* We have the lock!  */
@    return;
@
    .endm


    .macro _tx_thread_smp_protect_wait_list_lock_get
@VOID  _tx_thread_smp_protect_wait_list_lock_get()
@{
@    /* We do this until we have the lock.  */
@    while (1)
@    {
@
_tx_thread_smp_protect_wait_list_lock_get__try_to_get_lock\@:
@
@    /* Is the list lock available?  */
@    _tx_thread_smp_protect_wait_list_lock_protect_in_force = load_exclusive(&_tx_thread_smp_protect_wait_list_lock_protect_in_force);
@
    LDR     r1, =_tx_thread_smp_protect_wait_list_lock_protect_in_force
    LDREX   r2, [r1]                            @ Pickup the protection flag
@
@    if (protect_in_force == 0)
@    {
@
    CMP     r2, #0
    BNE     _tx_thread_smp_protect_wait_list_lock_get__try_to_get_lock\@ @ No, protection not available
@
@    /* Try to get the list.  */
@    int status = store_exclusive(&_tx_thread_smp_protect_wait_list_lock_protect_in_force, 1);
@
    MOV     r2, #1                              @ Build lock value
    STREX   r3, r2, [r1]                        @ Attempt to get the protection
@
@    if (status == SUCCESS)
@
    CMP     r3, #0
    BNE     _tx_thread_smp_protect_wait_list_lock_get__try_to_get_lock\@ @ Did it fail? If so, try again.
@
@    /* We have the lock!  */
@    return;
@
    .endm


    .macro _tx_thread_smp_protect_wait_list_add
@VOID  _tx_thread_smp_protect_wait_list_add(UINT new_core)
@{
@
@    /* We're about to modify the list, so get the list lock.  */
@    _tx_thread_smp_protect_wait_list_lock_get();
@
    PUSH    {r1-r2}

    _tx_thread_smp_protect_wait_list_lock_get

    POP     {r1-r2}
@
@    /* Add this core.  */
@    _tx_thread_smp_protect_wait_list[_tx_thread_smp_protect_wait_list_tail++] = new_core;
@
    LDR     r3, =_tx_thread_smp_protect_wait_list_tail @ Get the address of the tail
    LDR     r4, [r3]                            @ Get the value of tail
    LDR     r5, =_tx_thread_smp_protect_wait_list @ Get the address of the list
    STR     r1, [r5, r4, LSL #2]                @ Store the new core value
    ADD     r4, r4, #1                          @ Increment the tail
@
@    /* Did we wrap?  */
@    if (_tx_thread_smp_protect_wait_list_tail == _tx_thread_smp_protect_wait_list_size)
@    {
@
    LDR     r5, =_tx_thread_smp_protect_wait_list_size @ Load max cores address
    LDR     r5, [r5]                            @ Load max cores value
    CMP     r4, r5                              @ Compare max cores to tail
    BNE     _tx_thread_smp_protect_wait_list_add__no_wrap\@ @ Did we wrap?
@
@    _tx_thread_smp_protect_wait_list_tail = 0;
@
    MOV     r4, #0
@
@    }
@
_tx_thread_smp_protect_wait_list_add__no_wrap\@:

    STR     r4, [r3]                            @ Store the new tail value.
@
@    /* Release the list lock.  */
@    _tx_thread_smp_protect_wait_list_lock_protect_in_force = 0;
@
    MOV     r3, #0                              @ Build lock value
    LDR     r4, =_tx_thread_smp_protect_wait_list_lock_protect_in_force
    STR     r3, [r4]                            @ Store the new value

    .endm


    .macro _tx_thread_smp_protect_wait_list_remove
@VOID _tx_thread_smp_protect_wait_list_remove(UINT core)
@{
@
@    /* Get the core index.  */
@    UINT core_index;
@    for (core_index = 0;; core_index++)
@
    EOR     r1, r1, r1                          @ Clear for 'core_index'
    LDR     r2, =_tx_thread_smp_protect_wait_list @ Get the address of the list
@
@    {
@
_tx_thread_smp_protect_wait_list_remove__check_cur_core\@:
@
@    /* Is this the core?  */
@    if (_tx_thread_smp_protect_wait_list[core_index] == core)
@    {
@        break;
@
    LDR     r3, [r2, r1, LSL #2]                @ Get the value at the current index
    CMP     r3, r10                             @ Did we find the core?
    BEQ     _tx_thread_smp_protect_wait_list_remove__found_core\@
@
@    }
@
    ADD     r1, r1, #1                          @ Increment cur index
    B       _tx_thread_smp_protect_wait_list_remove__check_cur_core\@ @ Restart the loop
@
@    }
@
_tx_thread_smp_protect_wait_list_remove__found_core\@:
@
@    /* We're about to modify the list. Get the lock. We need the lock because another
@       core could be simultaneously adding (a core is simultaneously trying to get
@       the inter-core lock) or removing (a core is simultaneously being preempted,
@       like what is currently happening).  */
@    _tx_thread_smp_protect_wait_list_lock_get();
@
    PUSH    {r1}

    _tx_thread_smp_protect_wait_list_lock_get

    POP     {r1}
@
@    /* We remove by shifting.  */
@    while (core_index != _tx_thread_smp_protect_wait_list_tail)
@    {
@
_tx_thread_smp_protect_wait_list_remove__compare_index_to_tail\@:

    LDR     r2, =_tx_thread_smp_protect_wait_list_tail @ Load tail address
    LDR     r2, [r2]                            @ Load tail value
    CMP     r1, r2                              @ Compare cur index and tail
    BEQ     _tx_thread_smp_protect_wait_list_remove__removed\@
@
@    UINT next_index = core_index + 1;
@
    MOV     r2, r1                              @ Move current index to next index register
    ADD     r2, r2, #1                          @ Add 1
@
@    if (next_index == _tx_thread_smp_protect_wait_list_size)
@    {
@
    LDR     r3, =_tx_thread_smp_protect_wait_list_size
    LDR     r3, [r3]
    CMP     r2, r3
    BNE     _tx_thread_smp_protect_wait_list_remove__next_index_no_wrap\@
@
@    next_index = 0;
@
    MOV     r2, #0
@
@    }
@
_tx_thread_smp_protect_wait_list_remove__next_index_no_wrap\@:
@
@    list_cores[core_index] = list_cores[next_index];
@
    LDR     r0, =_tx_thread_smp_protect_wait_list @ Get the address of the list
    LDR     r3, [r0, r2, LSL #2]                @ Get the value at the next index
    STR     r3, [r0, r1, LSL #2]                @ Store the value at the current index
@
@    core_index = next_index;
@
    MOV     r1, r2

    B       _tx_thread_smp_protect_wait_list_remove__compare_index_to_tail\@
@
@    }
@
_tx_thread_smp_protect_wait_list_remove__removed\@:
@
@    /* Now update the tail.  */
@    if (_tx_thread_smp_protect_wait_list_tail == 0)
@    {
@
    LDR     r0, =_tx_thread_smp_protect_wait_list_tail @ Load tail address
    LDR     r1, [r0]                            @ Load tail value
    CMP     r1, #0
    BNE     _tx_thread_smp_protect_wait_list_remove__tail_not_zero\@
@
@    _tx_thread_smp_protect_wait_list_tail = _tx_thread_smp_protect_wait_list_size;
@
    LDR     r2, =_tx_thread_smp_protect_wait_list_size
    LDR     r1, [r2]
@
@    }
@
_tx_thread_smp_protect_wait_list_remove__tail_not_zero\@:
@
@    _tx_thread_smp_protect_wait_list_tail--;
@
    SUB     r1, r1, #1
    STR     r1, [r0]                            @ Store new tail value
@
@    /* Release the list lock.  */
@    _tx_thread_smp_protect_wait_list_lock_protect_in_force = 0;
@
    MOV     r0, #0                              @ Build lock value
    LDR     r1, =_tx_thread_smp_protect_wait_list_lock_protect_in_force @ Load lock address
    STR     r0, [r1]                            @ Store the new value
@
@    /* We're no longer waiting. Note that this should be zero since, again,
@       this function is only called when a thread preemption is occurring.  */
@    _tx_thread_smp_protect_wait_counts[core]--;
@
    LDR     r1, =_tx_thread_smp_protect_wait_counts @ Load wait list counts
    LDR     r2, [r1, r10, LSL #2]               @ Load waiting value
    SUB     r2, r2, #1                          @ Subtract 1
    STR     r2, [r1, r10, LSL #2]               @ Store new waiting value

    .endm

