target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/txe_thread_secure_stack_allocate.c
    ${CMAKE_CURRENT_LIST_DIR}/src/txe_thread_secure_stack_free.c
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_initialize_low_level.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_context_restore.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_context_save.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_interrupt_control.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_interrupt_disable.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_interrupt_restore.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_schedule.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_secure_stack.c
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_secure_stack_allocate.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_secure_stack_free.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_stack_build.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_system_return.S
    ${CMAKE_CURRENT_LIST_DIR}/src/tx_timer_interrupt.S
)

target_include_directories(${PROJECT_NAME} PUBLIC
    inc
)
