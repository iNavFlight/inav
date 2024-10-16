
target_sources(${PROJECT_NAME}
    PRIVATE
    # {{BEGIN_TARGET_SOURCES}}
	${CMAKE_CURRENT_LIST_DIR}/src/tx_initialize_low_level.c
	${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_context_restore.c
	${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_context_save.c
	${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_interrupt_control.c
	${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_schedule.c
	${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_stack_build.c
	${CMAKE_CURRENT_LIST_DIR}/src/tx_thread_system_return.c
	${CMAKE_CURRENT_LIST_DIR}/src/tx_timer_interrupt.c

    # {{END_TARGET_SOURCES}}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/inc
)

target_compile_definitions(${PROJECT_NAME} PUBLIC "-D_GNU_SOURCE -DTX_LINUX_DEBUG_ENABLE")