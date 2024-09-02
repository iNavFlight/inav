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
/**   ThreadX MISRA Compliance                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        #define SHT_PROGBITS 0x1

        EXTERN __aeabi_memset
        EXTERN _tx_thread_current_ptr
        EXTERN _tx_thread_interrupt_disable
        EXTERN _tx_thread_interrupt_restore
        EXTERN _tx_thread_stack_analyze
        EXTERN _tx_thread_stack_error_handler
        EXTERN _tx_thread_system_state
#ifdef TX_ENABLE_EVENT_TRACE
        EXTERN _tx_trace_buffer_current_ptr
        EXTERN _tx_trace_buffer_end_ptr
        EXTERN _tx_trace_buffer_start_ptr
        EXTERN _tx_trace_event_enable_bits
        EXTERN _tx_trace_full_notify_function
        EXTERN _tx_trace_header_ptr
#endif

        PUBLIC _tx_misra_always_true
        PUBLIC _tx_misra_block_pool_to_uchar_pointer_convert
        PUBLIC _tx_misra_byte_pool_to_uchar_pointer_convert
        PUBLIC _tx_misra_char_to_uchar_pointer_convert
        PUBLIC _tx_misra_const_char_to_char_pointer_convert
#ifdef TX_ENABLE_EVENT_TRACE
        PUBLIC _tx_misra_entry_to_uchar_pointer_convert
#endif
        PUBLIC _tx_misra_indirect_void_to_uchar_pointer_convert
        PUBLIC _tx_misra_memset
        PUBLIC _tx_misra_message_copy
#ifdef TX_ENABLE_EVENT_TRACE
        PUBLIC _tx_misra_object_to_uchar_pointer_convert
#endif
        PUBLIC _tx_misra_pointer_to_ulong_convert
        PUBLIC _tx_misra_status_get
        PUBLIC _tx_misra_thread_stack_check
#ifdef TX_ENABLE_EVENT_TRACE
        PUBLIC _tx_misra_time_stamp_get
#endif
        PUBLIC _tx_misra_timer_indirect_to_void_pointer_convert
        PUBLIC _tx_misra_timer_pointer_add
        PUBLIC _tx_misra_timer_pointer_dif
#ifdef TX_ENABLE_EVENT_TRACE
        PUBLIC _tx_misra_trace_event_insert
#endif
        PUBLIC _tx_misra_uchar_pointer_add
        PUBLIC _tx_misra_uchar_pointer_dif
        PUBLIC _tx_misra_uchar_pointer_sub
        PUBLIC _tx_misra_uchar_to_align_type_pointer_convert
        PUBLIC _tx_misra_uchar_to_block_pool_pointer_convert
#ifdef TX_ENABLE_EVENT_TRACE
        PUBLIC _tx_misra_uchar_to_entry_pointer_convert
        PUBLIC _tx_misra_uchar_to_header_pointer_convert
#endif
        PUBLIC _tx_misra_uchar_to_indirect_byte_pool_pointer_convert
        PUBLIC _tx_misra_uchar_to_indirect_uchar_pointer_convert
#ifdef TX_ENABLE_EVENT_TRACE
        PUBLIC _tx_misra_uchar_to_object_pointer_convert
#endif
        PUBLIC _tx_misra_uchar_to_void_pointer_convert
        PUBLIC _tx_misra_ulong_pointer_add
        PUBLIC _tx_misra_ulong_pointer_dif
        PUBLIC _tx_misra_ulong_pointer_sub
        PUBLIC _tx_misra_ulong_to_pointer_convert
        PUBLIC _tx_misra_ulong_to_thread_pointer_convert
        PUBLIC _tx_misra_user_timer_pointer_get
        PUBLIC _tx_misra_void_to_block_pool_pointer_convert
        PUBLIC _tx_misra_void_to_byte_pool_pointer_convert
        PUBLIC _tx_misra_void_to_event_flags_pointer_convert
        PUBLIC _tx_misra_void_to_indirect_uchar_pointer_convert
        PUBLIC _tx_misra_void_to_mutex_pointer_convert
        PUBLIC _tx_misra_void_to_queue_pointer_convert
        PUBLIC _tx_misra_void_to_semaphore_pointer_convert
        PUBLIC _tx_misra_void_to_thread_pointer_convert
        PUBLIC _tx_misra_void_to_uchar_pointer_convert
        PUBLIC _tx_misra_void_to_ulong_pointer_convert
        PUBLIC _tx_misra_ipsr_get
        PUBLIC _tx_misra_control_get
        PUBLIC _tx_misra_control_set
#ifdef __ARMVFP__
        PUBLIC _tx_misra_fpccr_get
        PUBLIC _tx_misra_vfp_touch
#endif

        PUBLIC _tx_misra_event_flags_group_not_used
        PUBLIC _tx_misra_event_flags_set_notify_not_used
        PUBLIC _tx_misra_queue_not_used
        PUBLIC _tx_misra_queue_send_notify_not_used
        PUBLIC _tx_misra_semaphore_not_used
        PUBLIC _tx_misra_semaphore_put_notify_not_used
        PUBLIC _tx_misra_thread_entry_exit_notify_not_used
        PUBLIC _tx_misra_thread_not_used

#ifdef TX_MISRA_ENABLE
        PUBLIC _tx_version_id

        SECTION `.data`:DATA:REORDER:NOROOT(2)
        DATA
//   51 CHAR  _tx_version_id[100] =  "Copyright (c) Microsoft Corporation. All rights reserved. * ThreadX 6.1 MISRA C Compliant *";
_tx_version_id:
        DC8 43H, 6FH, 70H, 79H, 72H, 69H, 67H, 68H
        DC8 74H, 20H, 28H, 63H, 29H, 20H, 31H, 39H
        DC8 39H, 36H, 2DH, 32H, 30H, 31H, 38H, 20H
        DC8 45H, 78H, 70H, 72H, 65H, 73H, 73H, 20H
        DC8 4CH, 6FH, 67H, 69H, 63H, 20H, 49H, 6EH
        DC8 63H, 2EH, 20H, 2AH, 20H, 54H, 68H, 72H
        DC8 65H, 61H, 64H, 58H, 20H, 36H, 2EH, 31H
        DC8 20H, 4DH, 49H, 53H, 52H, 41H, 20H, 43H
        DC8 20H, 43H, 6FH, 6DH, 70H, 6CH, 69H, 61H
        DC8 6EH, 74H, 20H, 2AH, 0
        DC8 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
#endif //TX_MISRA_ENABLE

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  VOID  _tx_misra_memset(VOID *ptr, UINT value, UINT size);            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_memset:
        PUSH     {R4,LR}
        MOVS     R4,R0
        MOVS     R0,R2
        MOVS     R2,R1
        MOVS     R1,R0
        MOVS     R0,R4
        BL       __aeabi_memset
        POP      {R4,PC}          // return

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  UCHAR  *_tx_misra_uchar_pointer_add(UCHAR *ptr, ULONG amount);       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_uchar_pointer_add:
        ADD      R0,R0,R1
        BX       LR               // return


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  UCHAR  *_tx_misra_uchar_pointer_sub(UCHAR *ptr, ULONG amount);       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_uchar_pointer_sub:
        RSBS     R1,R1,#+0
        ADD      R0,R0,R1
        BX       LR               // return


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  ULONG  _tx_misra_uchar_pointer_dif(UCHAR *ptr1, UCHAR *ptr2);        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_uchar_pointer_dif:
        SUBS     R0,R0,R1
        BX       LR               // return


/************************************************************************************************************************************/
/************************************************************************************************************************************/
/**                                                                                                                                 */
/**  This single function serves all of the below prototypes.                                                                       */
/**                                                                                                                                 */
/**  ULONG  _tx_misra_pointer_to_ulong_convert(VOID *ptr);                                                                          */
/**  VOID  *_tx_misra_ulong_to_pointer_convert(ULONG input);                                                                        */
/**  UCHAR  **_tx_misra_indirect_void_to_uchar_pointer_convert(VOID **return_ptr);                                                  */
/**  UCHAR  **_tx_misra_uchar_to_indirect_uchar_pointer_convert(UCHAR *pointer);                                                    */
/**  UCHAR  *_tx_misra_block_pool_to_uchar_pointer_convert(TX_BLOCK_POOL *pool);                                                    */
/**  TX_BLOCK_POOL  *_tx_misra_void_to_block_pool_pointer_convert(VOID *pointer);                                                   */
/**  UCHAR  *_tx_misra_void_to_uchar_pointer_convert(VOID *pointer);                                                                */
/**  TX_BLOCK_POOL *_tx_misra_uchar_to_block_pool_pointer_convert(UCHAR *pointer);                                                  */
/**  UCHAR  **_tx_misra_void_to_indirect_uchar_pointer_convert(VOID *pointer);                                                      */
/**  TX_BYTE_POOL  *_tx_misra_void_to_byte_pool_pointer_convert(VOID *pointer);                                                     */
/**  UCHAR  *_tx_misra_byte_pool_to_uchar_pointer_convert(TX_BYTE_POOL *pool);                                                      */
/**  ALIGN_TYPE  *_tx_misra_uchar_to_align_type_pointer_convert(UCHAR *pointer);                                                    */
/**  TX_BYTE_POOL  **_tx_misra_uchar_to_indirect_byte_pool_pointer_convert(UCHAR *pointer);                                         */
/**  TX_EVENT_FLAGS_GROUP  *_tx_misra_void_to_event_flags_pointer_convert(VOID *pointer);                                           */
/**  ULONG  *_tx_misra_void_to_ulong_pointer_convert(VOID *pointer);                                                                */
/**  TX_MUTEX  *_tx_misra_void_to_mutex_pointer_convert(VOID *pointer);                                                             */
/**  TX_QUEUE  *_tx_misra_void_to_queue_pointer_convert(VOID *pointer);                                                             */
/**  TX_SEMAPHORE  *_tx_misra_void_to_semaphore_pointer_convert(VOID *pointer);                                                     */
/**  VOID  *_tx_misra_uchar_to_void_pointer_convert(UCHAR *pointer);                                                                */
/**  TX_THREAD  *_tx_misra_ulong_to_thread_pointer_convert(ULONG value);                                                            */
/**  VOID  *_tx_misra_timer_indirect_to_void_pointer_convert(TX_TIMER_INTERNAL **pointer);                                          */
/**  CHAR  *_tx_misra_const_char_to_char_pointer_convert(const char *pointer);                                                      */
/**  TX_THREAD  *_tx_misra_void_to_thread_pointer_convert(void *pointer);                                                           */
/**  UCHAR  *_tx_misra_object_to_uchar_pointer_convert(TX_TRACE_OBJECT_ENTRY *pointer);                                             */
/**  TX_TRACE_OBJECT_ENTRY  *_tx_misra_uchar_to_object_pointer_convert(UCHAR *pointer);                                             */
/**  TX_TRACE_HEADER  *_tx_misra_uchar_to_header_pointer_convert(UCHAR *pointer);                                                   */
/**  TX_TRACE_BUFFER_ENTRY  *_tx_misra_uchar_to_entry_pointer_convert(UCHAR *pointer);                                              */
/**  UCHAR  *_tx_misra_entry_to_uchar_pointer_convert(TX_TRACE_BUFFER_ENTRY *pointer);                                              */
/**  UCHAR  *_tx_misra_char_to_uchar_pointer_convert(CHAR *pointer);                                                                */
/**  VOID    _tx_misra_event_flags_group_not_used(TX_EVENT_FLAGS_GROUP *group_ptr);                                                 */
/**  VOID    _tx_misra_event_flags_set_notify_not_used(VOID (*events_set_notify)(TX_EVENT_FLAGS_GROUP *notify_group_ptr));          */
/**  VOID    _tx_misra_queue_not_used(TX_QUEUE *queue_ptr);                                                                         */
/**  VOID    _tx_misra_queue_send_notify_not_used(VOID (*queue_send_notify)(TX_QUEUE *notify_queue_ptr));                           */
/**  VOID    _tx_misra_semaphore_not_used(TX_SEMAPHORE *semaphore_ptr);                                                             */
/**  VOID    _tx_misra_semaphore_put_notify_not_used(VOID (*semaphore_put_notify)(TX_SEMAPHORE *notify_semaphore_ptr));             */
/**  VOID    _tx_misra_thread_not_used(TX_THREAD *thread_ptr);                                                                      */
/**  VOID    _tx_misra_thread_entry_exit_notify_not_used(VOID (*thread_entry_exit_notify)(TX_THREAD *notify_thread_ptr, UINT id));  */
/**                                                                                                                                 */
/************************************************************************************************************************************/
/************************************************************************************************************************************/
        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_pointer_to_ulong_convert:
_tx_misra_ulong_to_pointer_convert:
_tx_misra_indirect_void_to_uchar_pointer_convert:
_tx_misra_uchar_to_indirect_uchar_pointer_convert:
_tx_misra_block_pool_to_uchar_pointer_convert:
_tx_misra_void_to_block_pool_pointer_convert:
_tx_misra_void_to_uchar_pointer_convert:
_tx_misra_uchar_to_block_pool_pointer_convert:
_tx_misra_void_to_indirect_uchar_pointer_convert:
_tx_misra_void_to_byte_pool_pointer_convert:
_tx_misra_byte_pool_to_uchar_pointer_convert:
_tx_misra_uchar_to_align_type_pointer_convert:
_tx_misra_uchar_to_indirect_byte_pool_pointer_convert:
_tx_misra_void_to_event_flags_pointer_convert:
_tx_misra_void_to_ulong_pointer_convert:
_tx_misra_void_to_mutex_pointer_convert:
_tx_misra_void_to_queue_pointer_convert:
_tx_misra_void_to_semaphore_pointer_convert:
_tx_misra_uchar_to_void_pointer_convert:
_tx_misra_ulong_to_thread_pointer_convert:
_tx_misra_timer_indirect_to_void_pointer_convert:
_tx_misra_const_char_to_char_pointer_convert:
_tx_misra_void_to_thread_pointer_convert:
#ifdef TX_ENABLE_EVENT_TRACE
_tx_misra_object_to_uchar_pointer_convert:
_tx_misra_uchar_to_object_pointer_convert:
_tx_misra_uchar_to_header_pointer_convert:
_tx_misra_uchar_to_entry_pointer_convert:
_tx_misra_entry_to_uchar_pointer_convert:
#endif
_tx_misra_char_to_uchar_pointer_convert:
_tx_misra_event_flags_group_not_used:
_tx_misra_event_flags_set_notify_not_used:
_tx_misra_queue_not_used:
_tx_misra_queue_send_notify_not_used:
_tx_misra_semaphore_not_used:
_tx_misra_semaphore_put_notify_not_used:
_tx_misra_thread_entry_exit_notify_not_used:
_tx_misra_thread_not_used:

        BX       LR               // return


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  ULONG  *_tx_misra_ulong_pointer_add(ULONG *ptr, ULONG amount);       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_ulong_pointer_add:
        ADD      R0,R0,R1, LSL #+2
        BX       LR               // return


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  ULONG  *_tx_misra_ulong_pointer_sub(ULONG *ptr, ULONG amount);       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_ulong_pointer_sub:
        MVNS     R2,#+3
        MULS     R1,R2,R1
        ADD      R0,R0,R1
        BX       LR               // return


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  ULONG  _tx_misra_ulong_pointer_dif(ULONG *ptr1, ULONG *ptr2);        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_ulong_pointer_dif:
        SUBS     R0,R0,R1
        ASRS     R0,R0,#+2
        BX       LR               // return


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  VOID  _tx_misra_message_copy(ULONG **source, ULONG **destination,    */
/**                                                           UINT size); */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_message_copy:
        PUSH     {R4,R5}
        LDR      R3,[R0, #+0]
        LDR      R4,[R1, #+0]
        LDR      R5,[R3, #+0]
        STR      R5,[R4, #+0]
        ADDS     R4,R4,#+4
        ADDS     R3,R3,#+4
        CMP      R2,#+2
        BCC.N    ??_tx_misra_message_copy_0
        SUBS     R2,R2,#+1
        B.N      ??_tx_misra_message_copy_1
??_tx_misra_message_copy_2:
        LDR      R5,[R3, #+0]
        STR      R5,[R4, #+0]
        ADDS     R4,R4,#+4
        ADDS     R3,R3,#+4
        SUBS     R2,R2,#+1
??_tx_misra_message_copy_1:
        CMP      R2,#+0
        BNE.N    ??_tx_misra_message_copy_2
??_tx_misra_message_copy_0:
        STR      R3,[R0, #+0]
        STR      R4,[R1, #+0]
        POP      {R4,R5}
        BX       LR               // return


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  ULONG  _tx_misra_timer_pointer_dif(TX_TIMER_INTERNAL **ptr1,         */
/**                                           TX_TIMER_INTERNAL **ptr2);  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_timer_pointer_dif:
        SUBS     R0,R0,R1
        ASRS     R0,R0,#+2
        BX       LR               // return


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  TX_TIMER_INTERNAL **_tx_misra_timer_pointer_add(TX_TIMER_INTERNAL    */
/**                                                  **ptr1, ULONG size); */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_timer_pointer_add:
        ADD      R0,R0,R1, LSL #+2
        BX       LR               // return


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  VOID  _tx_misra_user_timer_pointer_get(TX_TIMER_INTERNAL             */
/**                              *internal_timer, TX_TIMER **user_timer); */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_user_timer_pointer_get:
        SUBS    R0,#8
        STR     R0,[R1, #+0]
        BX      LR               // return


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  VOID  _tx_misra_thread_stack_check(TX_THREAD *thread_ptr,            */
/**                                              VOID **highest_stack);   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_thread_stack_check:
        PUSH     {R3-R5,LR}
        MOVS     R4,R0
        MOVS     R5,R1
        BL       _tx_thread_interrupt_disable
        CMP      R4,#+0
        BEQ.N    ??_tx_misra_thread_stack_check_0
        LDR      R1,[R4, #+0]
        LDR.N    R2,??DataTable2  // 0x54485244
        CMP      R1,R2
        BNE.N    ??_tx_misra_thread_stack_check_0
        LDR      R1,[R4, #+8]
        LDR      R2,[R5, #+0]
        CMP      R1,R2
        BCS.N    ??_tx_misra_thread_stack_check_1
        LDR      R1,[R4, #+8]
        STR      R1,[R5, #+0]
??_tx_misra_thread_stack_check_1:
        LDR      R1,[R4, #+12]
        LDR      R1,[R1, #+0]
        CMP      R1,#-269488145
        BNE.N    ??_tx_misra_thread_stack_check_2
        LDR      R1,[R4, #+16]
        LDR      R1,[R1, #+1]
        CMP      R1,#-269488145
        BNE.N    ??_tx_misra_thread_stack_check_2
        LDR      R1,[R5, #+0]
        LDR      R2,[R4, #+12]
        CMP      R1,R2
        BCS.N    ??_tx_misra_thread_stack_check_3
??_tx_misra_thread_stack_check_2:
        BL       _tx_thread_interrupt_restore
        MOVS     R0,R4
        BL       _tx_thread_stack_error_handler
        BL       _tx_thread_interrupt_disable
??_tx_misra_thread_stack_check_3:
        LDR      R1,[R5, #+0]
        LDR      R1,[R1, #-4]
        CMP      R1,#-269488145
        BEQ.N    ??_tx_misra_thread_stack_check_0
        BL       _tx_thread_interrupt_restore
        MOVS     R0,R4
        BL       _tx_thread_stack_analyze
        BL       _tx_thread_interrupt_disable
??_tx_misra_thread_stack_check_0:
        BL       _tx_thread_interrupt_restore
        POP      {R0,R4,R5,PC}    // return

#ifdef TX_ENABLE_EVENT_TRACE

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  VOID  _tx_misra_trace_event_insert(ULONG event_id,                   */
/**           VOID *info_field_1, ULONG info_field_2, ULONG info_field_3, */
/**           ULONG info_field_4, ULONG filter, ULONG time_stamp);        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_trace_event_insert:
        PUSH     {R3-R7,LR}
        LDR.N    R4,??DataTable2_1
        LDR      R4,[R4, #+0]
        CMP      R4,#+0
        BEQ.N    ??_tx_misra_trace_event_insert_0
        LDR.N    R5,??DataTable2_2
        LDR      R5,[R5, #+0]
        LDR      R6,[SP, #+28]
        TST      R5,R6
        BEQ.N    ??_tx_misra_trace_event_insert_0
        LDR.N    R5,??DataTable2_3
        LDR      R5,[R5, #+0]
        LDR.N    R6,??DataTable2_4
        LDR      R6,[R6, #+0]
        CMP      R5,#+0
        BNE.N    ??_tx_misra_trace_event_insert_1
        LDR      R5,[R6, #+44]
        LDR      R7,[R6, #+60]
        LSLS     R7,R7,#+16
        ORRS     R7,R7,#0x80000000
        ORRS     R5,R7,R5
        B.N      ??_tx_misra_trace_event_insert_2
??_tx_misra_trace_event_insert_1:
        CMP      R5,#-252645136
        BCS.N    ??_tx_misra_trace_event_insert_3
        MOVS     R5,R6
        MOVS     R6,#-1
        B.N      ??_tx_misra_trace_event_insert_2
??_tx_misra_trace_event_insert_3:
        MOVS     R6,#-252645136
        MOVS     R5,#+0
??_tx_misra_trace_event_insert_2:
        STR      R6,[R4, #+0]
        STR      R5,[R4, #+4]
        STR      R0,[R4, #+8]
        LDR      R0,[SP, #+32]
        STR      R0,[R4, #+12]
        STR      R1,[R4, #+16]
        STR      R2,[R4, #+20]
        STR      R3,[R4, #+24]
        LDR      R0,[SP, #+24]
        STR      R0,[R4, #+28]
        ADDS     R4,R4,#+32
        LDR.N    R0,??DataTable2_5
        LDR      R0,[R0, #+0]
        CMP      R4,R0
        BCC.N    ??_tx_misra_trace_event_insert_4
        LDR.N    R0,??DataTable2_6
        LDR      R4,[R0, #+0]
        LDR.N    R0,??DataTable2_1
        STR      R4,[R0, #+0]
        LDR.N    R0,??DataTable2_7
        LDR      R0,[R0, #+0]
        STR      R4,[R0, #+32]
        LDR.N    R0,??DataTable2_8
        LDR      R0,[R0, #+0]
        CMP      R0,#+0
        BEQ.N    ??_tx_misra_trace_event_insert_0
        LDR.N    R0,??DataTable2_7
        LDR      R0,[R0, #+0]
        LDR.N    R1,??DataTable2_8
        LDR      R1,[R1, #+0]
        BLX      R1
        B.N      ??_tx_misra_trace_event_insert_0
??_tx_misra_trace_event_insert_4:
        LDR.N    R0,??DataTable2_1
        STR      R4,[R0, #+0]
        LDR.N    R0,??DataTable2_7
        LDR      R0,[R0, #+0]
        STR      R4,[R0, #+32]
??_tx_misra_trace_event_insert_0:
        POP      {R0,R4-R7,PC}    // return


        SECTION `.text`:CODE:NOROOT(2)
        SECTION_TYPE SHT_PROGBITS, 0
        DATA
??DataTable2_1:
        DC32     _tx_trace_buffer_current_ptr

        SECTION `.text`:CODE:NOROOT(2)
        SECTION_TYPE SHT_PROGBITS, 0
        DATA
??DataTable2_2:
        DC32     _tx_trace_event_enable_bits

        SECTION `.text`:CODE:NOROOT(2)
        SECTION_TYPE SHT_PROGBITS, 0
        DATA
??DataTable2_5:
        DC32     _tx_trace_buffer_end_ptr

        SECTION `.text`:CODE:NOROOT(2)
        SECTION_TYPE SHT_PROGBITS, 0
        DATA
??DataTable2_6:
        DC32     _tx_trace_buffer_start_ptr

        SECTION `.text`:CODE:NOROOT(2)
        SECTION_TYPE SHT_PROGBITS, 0
        DATA
??DataTable2_7:
        DC32     _tx_trace_header_ptr

        SECTION `.text`:CODE:NOROOT(2)
        SECTION_TYPE SHT_PROGBITS, 0
        DATA
??DataTable2_8:
        DC32     _tx_trace_full_notify_function


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  ULONG  _tx_misra_time_stamp_get(VOID);                               */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_time_stamp_get:
        MOVS     R0,#+0
        BX       LR               // return

#endif

        SECTION `.text`:CODE:NOROOT(2)
        SECTION_TYPE SHT_PROGBITS, 0
        DATA
??DataTable2:
        DC32     0x54485244

        SECTION `.text`:CODE:NOROOT(2)
        SECTION_TYPE SHT_PROGBITS, 0
        DATA
??DataTable2_3:
        DC32     _tx_thread_system_state

        SECTION `.text`:CODE:NOROOT(2)
        SECTION_TYPE SHT_PROGBITS, 0
        DATA
??DataTable2_4:
        DC32     _tx_thread_current_ptr


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  UINT  _tx_misra_always_true(void);                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_always_true:
        MOVS     R0,#+1
        BX       LR               // return


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/**  UINT  _tx_misra_status_get(UINT status);                             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_status_get:
        MOVS     R0,#+0
        BX       LR               // return


/***********************************************************************************************/
/***********************************************************************************************/
/**                                                                                            */
/**  ULONG  _tx_misra_ipsr_get(void);                                                          */
/**                                                                                            */
/***********************************************************************************************/
/***********************************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_ipsr_get:
        MRS      R0, IPSR
        BX       LR               // return


/***********************************************************************************************/
/***********************************************************************************************/
/**                                                                                            */
/**  ULONG  _tx_misra_control_get(void);                                                       */
/**                                                                                            */
/***********************************************************************************************/
/***********************************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_control_get:
        MRS      R0, CONTROL
        BX       LR               // return

        
/***********************************************************************************************/
/***********************************************************************************************/
/**                                                                                            */
/**  void   _tx_misra_control_set(ULONG value);                                                */
/**                                                                                            */
/***********************************************************************************************/
/***********************************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_control_set:
        MSR      CONTROL, R0
        BX       LR               // return

        
#ifdef __ARMVFP__

/***********************************************************************************************/
/***********************************************************************************************/
/**                                                                                            */
/**  ULONG  _tx_misra_fpccr_get(void);                                                         */
/**                                                                                            */
/***********************************************************************************************/
/***********************************************************************************************/

        SECTION `.text`:CODE:NOROOT(2)
        THUMB
_tx_misra_fpccr_get:
        LDR      r0, =0xE000EF34  // Build FPCCR address
        LDR      r0, [r0]         // Load FPCCR value
        BX       LR               // return
        
        
/***********************************************************************************************/
/***********************************************************************************************/
/**                                                                                            */
/**  void   _tx_misra_vfp_touch(void);                                                         */
/**                                                                                            */
/***********************************************************************************************/
/***********************************************************************************************/

        SECTION `.text`:CODE:NOROOT(1)
        THUMB
_tx_misra_vfp_touch:
        vmov.f32 s0, s0
        BX       LR               // return
        
#endif
        
        
        SECTION `.iar_vfe_header`:DATA:NOALLOC:NOROOT(2)
        SECTION_TYPE SHT_PROGBITS, 0
        DATA
        DC32 0

        END
