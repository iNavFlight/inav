# Each protocol handler is listed below. If you want
# to disable some / all of these, comment out the block(s) in question

if(NXD_ENABLE_FILE_SERVERS)
    target_sources(${PROJECT_NAME}
        PRIVATE
        # {{BEGIN_TARGET_SOURCES}}
        ${CMAKE_CURRENT_LIST_DIR}/iperf/nx_iperf.c

        # {{END_TARGET_SOURCES}}
    )

    target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/iperf
    )
endif()