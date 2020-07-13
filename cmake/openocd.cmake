set(OPENOCD "" CACHE STRING "path to openocd (default: search for it)")
set(OPENOCD_CFG "" CACHE STRING "path to openocd configuration (default: generate automatically)")
set(OPENOCD_INTERFACE "" CACHE STRING "openocd interface name (default: automatic depending on target)")

if (OPENOCD)
    set(OPENOCD_PATH ${OPENOCD})
else()
    find_program(OPENOCD_FOUND_PATH NAMES openocd openocd.exe)
    if (NOT OPENOCD_FOUND_PATH)
        message(STATUS "Could not find openocd, debugging won't be available")
    else()
        set(OPENOCD_PATH ${OPENOCD_FOUND_PATH})
    endif()
endif()

if(OPENOCD_PATH)
    # Retrieve version number as a sanity check
    execute_process(COMMAND ${OPENOCD_PATH} -v
        OUTPUT_QUIET
        ERROR_VARIABLE OPENOCD_HELP
        RESULT_VARIABLE OPENOCD_RESULT)

    string(REPLACE "\n" ";" OPENOCD_HELP_LINES ${OPENOCD_HELP})
    list(GET OPENOCD_HELP_LINES 0 OPENOCD_FIRST_HELP_LINE)
    string(REPLACE "\r" "" OPENOCD_HELP_LINE ${OPENOCD_FIRST_HELP_LINE})
    if (NOT OPENOCD_RESULT EQUAL 0)
        # User provided an incorrect path
        message(FATAL_ERROR "error executing ${OPENOCD_PATH} (${OPENOCD_RESULT})")
    endif()
    message(STATUS "using openocd: ${OPENOCD_HELP_LINE}")
    add_custom_target(openocd ${OPENOCD_PATH} -f ${OPENOCD_CFG}
        COMMENT "Run openocd using OPENOCD_CFG=(${OPENOCD_CFG}) as configuration"
        USES_TERMINAL
    )
endif()

function(setup_openocd target_name)
    if(OPENOCD_INTERFACE)
        set(openocd_interface ${OPENOCD_INTERFACE})
    else()
        get_property(openocd_interface TARGET ${target_name} PROPERTY OPENOCD_DEFAULT_INTERFACE)
    endif()
    get_property(openocd_target TARGET ${target_name} PROPERTY OPENOCD_TARGET)
    if(OPENOCD_CFG OR (openocd_target AND openocd_interface))
        set(openocd_run_target "openocd_${target_name}")
        if (OPENOCD_CFG AND NOT OPENOCD_CFG STREQUAL "")
            get_filename_component(openocd_cfg_path ${OPENOCD_CFG}
                ABSOLUTE
                BASE_DIR ${CMAKE_BINARY_DIR})
        else()
            set(openocd_cfg_path ${CMAKE_BINARY_DIR}/openocd/${target_name}.cfg)
            add_custom_command(
                OUTPUT ${openocd_cfg_path}
                COMMENT "Generating openocd configuration for ${openocd_target} via ${openocd_interface}"
                COMMAND ${CMAKE_COMMAND} -P ${MAIN_DIR}/cmake/openocd_cfg.cmake
                    ${openocd_target} ${openocd_interface} ${openocd_cfg_path}
            )
        endif()

        # Target for openocd configuration
        set(openocd_cfg_target "openocd_cfg_${target_name}")
        add_custom_target(${openocd_cfg_target} DEPENDS ${openocd_cfg_path})
        exclude_from_all(${openocd_cfg_target})

        # Target for running openocd
        add_custom_target(${openocd_run_target} ${OPENOCD_PATH} -f ${openocd_cfg_path}
            COMMENT "Running openocd for target ${target_name} via ${openocd_interface}"
            DEPENDS ${openocd_cfg_path}
            USES_TERMINAL
        )
        exclude_from_all(${openocd_run_target})
        # Target for flashing via openocd
        set(openocd_flash_target "openocd_flash_${target_name}")
        add_custom_target(${openocd_flash_target} ${CMAKE_COMMAND} -E env
            OPENOCD_CMD=${OPENOCD_PATH}
            ${MAIN_UTILS_DIR}/openocd_flash.py -f
            ${openocd_cfg_path} $<TARGET_FILE:${target_name}>

            COMMENT "Flashing ${target_name} with openocd"
            DEPENDS ${openocd_cfg_path} ${target_name}
        )
        exclude_from_all(${openocd_flash_target})
    endif()
endfunction()
