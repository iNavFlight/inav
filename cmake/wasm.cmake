# WebAssembly (WASM) Build Configuration
# Provides wasm-specific compilation and linking for INAV

# Only include this file for WASM builds
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    # Stub function for non-WASM builds to avoid errors in target subdirectories
    function(target_wasm name)
        # WASM target - skipped for this build
    endfunction()
    return()
endif()

# WebAssembly-specific source excludes and includes
main_sources(WASM_COMMON_SRC_EXCLUDES
    build/atomic.h
    drivers/system.c
    drivers/time.c
    drivers/timer.c
    drivers/rcc.c
    drivers/persistent.c
    drivers/accgyro/accgyro_mpu.c
    drivers/display_ug2864hsweg01.c
    drivers/pwm_output.c
    drivers/pwm_mapping.c
    drivers/pwm_esc_detect.c
    io/displayport_oled.c
    io/serial_uart.c
)

# WASM-specific sources for simulation
main_sources(WASM_SRC
    config/config_streamer_file.c
    drivers/serial_ex.c
    drivers/serial_tcp.c
    drivers/serial_tcp.h
    target/SITL/sim/xplane.c
    target/SITL/sim/realFlight.c
    target/SITL/sim/simple_soap_client.c
    target/SITL/sim/simHelper.c
)

# Emscripten linkage options
set(WASM_LINK_OPTIONS
    -sWASM=1
    -sINVOKE_RUN=0
    -sWASMFS=1
    -lwebsocket.js
    -sFORCE_FILESYSTEM=1
    -sASSERTIONS=0
    -sASYNCIFY=1
    -pthread
    -lwebsocket.js
    -sPROXY_POSIX_SOCKETS
    -sPROXY_TO_PTHREAD
    -sALLOW_TABLE_GROWTH=1
    -sPTHREAD_POOL_SIZE=10
    -sEXPORTED_FUNCTIONS=['_main','_wasmExit','_malloc','_free','_fcScheduler','_inavSerialExSend','_inavSerialExConnect','_inavSerialExDisconnect','_serialExHasMessages','_serialExGetMessage']
    -sEXPORTED_RUNTIME_METHODS=['cwrap','ccall','callMain','addFunction','HEAP8','HEAPU8','HEAPU16','wasmMemory','FS']
)


set(WASM_COMPILE_OPTIONS
    -Wno-format
    -funsigned-char
    -fPIC
)

 set(WASM_DEFINITIONS
    WASM_BUILD
)

# Memory settings for WASM runtime
set(WASM_MEMORY_SIZE 16777216)  # 16MB initial memory
if(WASM_CUSTOM_MEMORY)
    set(WASM_MEMORY_SIZE ${WASM_CUSTOM_MEMORY})
endif()

list(APPEND WASM_LINK_OPTIONS "-s TOTAL_MEMORY=${WASM_MEMORY_SIZE}")

function (target_wasm name)
    exclude(COMMON_SRC "${WASM_COMMON_SRC_EXCLUDES}")

    set(target_sources)
    list(APPEND target_sources ${WASM_SRC})
    
    file(GLOB target_c_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
    file(GLOB target_h_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    list(APPEND target_sources ${target_c_sources} ${target_h_sources})

    set(target_definitions ${COMMON_COMPILE_DEFINITIONS})
    set(hse_mhz 8)  # Dummy value for WASM
    math(EXPR hse_value "${hse_mhz} * 1000000")
    list(APPEND target_definitions "HSE_VALUE=${hse_value}")

    if (MSP_UART)
        list(APPEND target_definitions "MSP_UART=${MSP_UART}")
    endif()

    string(TOLOWER ${PROJECT_NAME} lowercase_project_name)
    set(binary_name ${lowercase_project_name}_${FIRMWARE_VERSION}_${name})
    if(DEFINED BUILD_SUFFIX AND NOT "" STREQUAL "${BUILD_SUFFIX}")
        set(binary_name "${binary_name}_${BUILD_SUFFIX}")
    endif()

    list(APPEND target_definitions ${WASM_DEFINITIONS})

    # Create main executable with unique name
    set(exe_target ${name}_exe)
    add_executable(${exe_target})
    target_sources(${exe_target} PRIVATE ${target_sources} ${COMMON_SRC})
    target_include_directories(${exe_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(${exe_target} PRIVATE ${target_definitions})
    
    # Set SUFFIX to empty and PREFIX to empty for Emscripten to generate proper .js file
    set_target_properties(${exe_target} PROPERTIES SUFFIX ".js" PREFIX "")
    
    # Set explicit output directory and names for Emscripten
    set_target_properties(${exe_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        RUNTIME_OUTPUT_NAME "${name}"
    )

    if(WARNINGS_AS_ERRORS)
        target_compile_options(${exe_target} PRIVATE -Werror)
    endif()

    target_compile_options(${exe_target} PRIVATE ${WASM_COMPILE_OPTIONS})
    target_link_options(${exe_target} PRIVATE ${WASM_LINK_OPTIONS})
    
    # Output files
    set(wasm_filename ${CMAKE_BINARY_DIR}/${binary_name}.js)
    set(wasm_wasm_file ${CMAKE_BINARY_DIR}/${binary_name}.wasm)
    set(emscripten_output_js ${CMAKE_BINARY_DIR}/bin/${name}.js)
    set(emscripten_output_wasm ${CMAKE_BINARY_DIR}/bin/${name}.wasm)

    # Custom target to manage WebAssembly output
    # Emscripten generates both .js and .wasm files from the same source
    # We need to fix the .wasm filename reference in the .js file
    set(wasm_basename ${binary_name})
    set(html_file "${CMAKE_SOURCE_DIR}/src/main/target/SITL/wasm/inav_WASM.html")
    set(output_index_html "${CMAKE_BINARY_DIR}/index.html")

    
    add_custom_command(
        OUTPUT ${wasm_filename} ${wasm_wasm_file} ${output_index_html}
        COMMAND ${CMAKE_COMMAND} -E copy ${emscripten_output_js} ${wasm_filename}
        COMMAND ${CMAKE_COMMAND} -E copy ${emscripten_output_wasm} ${wasm_wasm_file}
        COMMAND ${CMAKE_COMMAND} -E copy ${html_file} ${output_index_html}
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/fix_wasm_filename.cmake --arg ${wasm_filename} ${wasm_basename}
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SOURCE_DIR}/cmake/fix_wasm_html.cmake --arg ${output_index_html} ${wasm_basename}.js ${FIRMWARE_VERSION}
        DEPENDS ${exe_target}
        COMMENT "Processing WebAssembly output: ${name}"
    )
    
    add_custom_target(${name} ALL
        DEPENDS ${wasm_filename} ${wasm_wasm_file} ${output_index_html}
        COMMENT "WebAssembly module ready: ${name}"
    )

    setup_firmware_target(${exe_target} ${name} ${ARGN})
endfunction()
