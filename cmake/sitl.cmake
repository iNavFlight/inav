
main_sources(SITL_COMMON_SRC_EXCLUDES
    build/atomic.h
    drivers/system.c
    drivers/time.c
    drivers/timer.c
    drivers/rcc.c
    drivers/persistent.c
    drivers/accgyro/accgyro_mpu.c
    drivers/display_ug2864hsweg01.c
    io/displayport_oled.c
)

main_sources(SITL_SRC
    drivers/serial_websocket.c
    drivers/serial_websocket.h
)

# Only include TCP server and simulator code for non-WASM builds
if(NOT ${TOOLCHAIN} STREQUAL "wasm")
    # File-based config storage for native SITL
    main_sources(SITL_SRC
        config/config_streamer_file.c
    )
    main_sources(SITL_SRC
        drivers/serial_tcp.c
        drivers/serial_tcp.h
        target/SITL/sim/realFlight.c
        target/SITL/sim/realFlight.h
        target/SITL/sim/simHelper.c
        target/SITL/sim/simHelper.h
        target/SITL/sim/simple_soap_client.c
        target/SITL/sim/simple_soap_client.h
        target/SITL/sim/xplane.c
        target/SITL/sim/xplane.h
    )
else()
    # WASM-specific: Manual PG registry (linker script not supported)
    # RAM-based config storage (no file I/O in browser)
    main_sources(SITL_SRC
        config/config_streamer_ram.c
        target/SITL/wasm_pg_registry.c
        target/SITL/wasm_pg_runtime.c
        target/SITL/wasm_pg_runtime.h
        target/SITL/wasm_stubs.c
        target/SITL/wasm_msp_bridge.c
        target/SITL/serial_wasm.c
        target/SITL/serial_wasm.h
    )
endif()


if(CMAKE_HOST_APPLE)
  set(MACOSX ON)
endif()

set(SITL_LINK_OPTIONS
    -Wl,-L${STM32_LINKER_DIR}
)

if(${CYGWIN})
    set(SITL_LINK_OPTIONS ${SITL_LINK_OPTIONS} "-static-libgcc")
endif()

set(SITL_LINK_LIBRARIES
    -lpthread
    -lm
    -lc
)

if(NOT MACOSX)
    set(SITL_LINK_LIBRARIES ${SITL_LINK_LIBRARIES} -lrt)
endif()

set(SITL_COMPILE_OPTIONS
    -Wno-format #Fixme: Compile for 32bit, but settings.rb has to be adjusted
    -funsigned-char
)

if(DEBUG)
    message(STATUS "Debug mode enabled. Adding -g to SITL_COMPILE_OPTIONS.")
    list(APPEND SITL_COMPILE_OPTIONS -g)
endif()

if(NOT MACOSX)
    set(SITL_COMPILE_OPTIONS ${SITL_COMPILE_OPTIONS}
        -Wno-return-local-addr
        -Wno-error=maybe-uninitialized
        -fsingle-precision-constant
    )
    # Temporarily disabled - ld version may not support this flag
    # if (CMAKE_COMPILER_IS_GNUCC AND NOT CMAKE_C_COMPILER_VERSION VERSION_LESS 12.0)
    #     set(SITL_LINK_OPTIONS ${SITL_LINK_OPTIONS} "-Wl,--no-warn-rwx-segments")
    # endif()
else()
    set(SITL_COMPILE_OPTIONS ${SITL_COMPILE_OPTIONS}
    )
endif()

set(SITL_DEFINITIONS
    SITL_BUILD
)

# WebAssembly-specific settings
if(${TOOLCHAIN} STREQUAL "wasm")
    # Disable simulator for WASM builds
    list(APPEND SITL_DEFINITIONS SKIP_SIMULATOR=1)
    # Use RAM-based config storage (no file I/O in browser)
    list(APPEND SITL_DEFINITIONS CONFIG_IN_RAM)

    # Emscripten-specific compile options
    set(SITL_COMPILE_OPTIONS ${SITL_COMPILE_OPTIONS}
        # Phase 5 MVP: Disable pthreads
        # -pthread
        -funsigned-char
        -g                          # Debug symbols for browser DevTools
    )

    # Emscripten linker options
    set(SITL_LINK_OPTIONS
        # Phase 5 MVP: Disable pthreads to avoid COOP/COEP header requirements
        # -pthread
        # -sUSE_PTHREADS=1
        # -sPTHREAD_POOL_SIZE=8
        -sALLOW_MEMORY_GROWTH=1
        # ASYNCIFY allows WASM to unwind the call stack when exiting from EM_ASM callbacks.
        # Without this, emscripten_force_exit() called from within EM_ASM (in systemReset)
        # would freeze the JS event loop, preventing the reload IPC message from being processed.
        -sASYNCIFY=1
        -sWEBSOCKET_URL="ws://localhost:5771"
        -sFORCE_FILESYSTEM=1
        -sEXPORTED_FUNCTIONS=_main,_serialWriteByte,_serialReadByte,_serialAvailable,_serialGetRxDroppedBytes,_serialGetTxDroppedBytes,_malloc,_free
        -sEXPORTED_RUNTIME_METHODS=ccall,cwrap,UTF8ToString,stringToUTF8,lengthBytesUTF8,getValue,setValue
        -gsource-map                                      # Generate .wasm.map for browser debugging
        -lidbfs.js
    )

    # Override libraries for WASM (no system libs needed)
    set(SITL_LINK_LIBRARIES "")
endif()

function (target_sitl name)
    if(CMAKE_VERSION VERSION_GREATER 3.22)
        set(CMAKE_C_STANDARD 17)
    endif()

    # Accept both host and wasm toolchains for SITL builds
    if(NOT ${TOOLCHAIN} STREQUAL "host" AND NOT ${TOOLCHAIN} STREQUAL "wasm")
        return()
    endif()

    exclude(COMMON_SRC "${SITL_COMMON_SRC_EXCLUDES}")

    set(target_sources)
    list(APPEND target_sources ${SITL_SRC})
    file(GLOB target_c_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.c")
    file(GLOB target_h_sources "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    list(APPEND target_sources ${target_c_sources} ${target_h_sources})

    set(target_definitions ${COMMON_COMPILE_DEFINITIONS})

    set(hse_mhz ${STM32_DEFAULT_HSE_MHZ})
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

    list(APPEND target_definitions ${SITL_DEFINITIONS})
    set(exe_target ${name}.elf)
    add_executable(${exe_target})
    target_sources(${exe_target} PRIVATE ${target_sources} ${COMMON_SRC})
    target_include_directories(${exe_target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_compile_definitions(${exe_target} PRIVATE ${target_definitions})


    if(WARNINGS_AS_ERRORS)
        target_compile_options(${exe_target} PRIVATE -Werror)
    endif()

    target_compile_options(${exe_target} PRIVATE ${SITL_COMPILE_OPTIONS})

    target_link_libraries(${exe_target} PRIVATE ${SITL_LINK_LIBRARIES})
    target_link_options(${exe_target} PRIVATE ${SITL_LINK_OPTIONS})

    # Only use linker script for non-WASM builds
    if(NOT ${TOOLCHAIN} STREQUAL "wasm")
        set(script_path ${MAIN_SRC_DIR}/target/link/sitl.ld)
        if(NOT EXISTS ${script_path})
            message(FATAL_ERROR "linker script ${script_path} doesn't exist")
        endif()
        set_target_properties(${exe_target} PROPERTIES LINK_DEPENDS ${script_path})
        if(NOT MACOSX)
            target_link_options(${exe_target} PRIVATE -T${script_path})
        endif()
    endif()

    if(${CYGWIN})
        set(exe_filename ${CMAKE_BINARY_DIR}/${binary_name}.exe)
    else()
        set(exe_filename ${CMAKE_BINARY_DIR}/${binary_name})
    endif()

    add_custom_target(${name} ALL
        cmake -E copy $<TARGET_FILE:${exe_target}> ${exe_filename}
    )

    setup_firmware_target(${exe_target} ${name} ${ARGN})
    #clean_<target>
    set(generator_cmd "")
    if (CMAKE_GENERATOR STREQUAL "Unix Makefiles")
        set(generator_cmd "make")
    elseif(CMAKE_GENERATOR STREQUAL "Ninja")
        set(generator_cmd "ninja")
    endif()
    if (NOT generator_cmd STREQUAL "")
        set(clean_target "clean_${name}")
        add_custom_target(${clean_target}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${generator_cmd} clean
            COMMENT "Removing intermediate files for ${name}")
        set_property(TARGET ${clean_target} PROPERTY
            EXCLUDE_FROM_ALL 1
            EXCLUDE_FROM_DEFAULT_BUILD 1)
    endif()
endfunction()
