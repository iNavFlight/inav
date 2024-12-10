macro(CONF_DEPENDS_OPTIONS __core_config_dir)
    # include default depends configuration
    include(${__core_config_dir}depends_configuration.cmake)
endmacro()

macro(CONF_DEFINE_BASE __config_path __base)
    # include default configuration from specific base dist
    include(${__config_path}${__base})
endmacro()

macro(CONF_IN_FILE_OPEN __core_config_dir __plat_config_dir __dist __path __ifdef __cpp)
    FILE(REMOVE ${__path})
    if(NOT "${__core_config_dir}" STREQUAL "${__plat_config_dir}")
        file(READ ${__plat_config_dir}/license __license_body)
    else()
        file(READ ${__core_config_dir}/license __license_body)
    endif()
    FILE(APPEND ${__path} ${__license_body})
    if (${__cpp})
        FILE(APPEND ${__path} "\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n")
    endif()

    FILE(APPEND ${__path} "/* Distribution ")
    FILE(APPEND ${__path} ${__dist} " */")
    FILE(APPEND ${__path} \n)
    FILE(APPEND ${__path} "#ifndef " ${__ifdef}\n)
    FILE(APPEND ${__path} "#define " ${__ifdef}\n\n)
endmacro()

macro(CONF_IN_FILE_BODY __core_config_dir __plat_config_dir __path)
    file(READ ${__core_config_dir}/asc_config.h.in __cfg_body)
    FILE(APPEND ${__path} ${__cfg_body})
    if(NOT "${__core_config_dir}" STREQUAL "${__plat_config_dir}")
        message("Merging platform asc_config.h.in")
        file(READ ${__plat_config_dir}/asc_config.h.in __cfg_body)
        FILE(APPEND ${__path} ${__cfg_body})
    endif()
    set(__cfg_body "")
endmacro()

macro(CONF_IN_FILE_CLOSE __path __def __cpp)
    FILE(APPEND ${__path} \n)
    FILE(APPEND ${__path} "#endif /* ${__def} */")
    if (${__cpp})
        FILE(APPEND ${__path} "\n\n#ifdef __cplusplus\n}\n#endif")
    endif()
endmacro()

macro(CONF_IN_CREATE __core_config_dir __plat_config_dir __dist __path __ifdef __cpp)
    CONF_IN_FILE_OPEN(${__core_config_dir} ${__plat_config_dir} ${__dist} ${__path} ${__ifdef} ${__cpp})
    CONF_IN_FILE_BODY(${__core_config_dir} ${__plat_config_dir} ${__path})
    CONF_IN_FILE_CLOSE(${__path} ${__ifdef} ${__cpp})
endmacro()

macro(CONF_CREATE_DIST __dist __core_config_dir __plat_config_dir __output __ifdef __check_dist __cpp)
    if (${__check_dist})
        if (EXISTS ${CMAKE_BINARY_DIR}/built_dist.conf)
            FILE(READ ${CMAKE_BINARY_DIR}/built_dist.conf _stored_dist)
            if(DEFINED _stored_dist)
                if ((NOT (${_stored_dist} STREQUAL "")) AND (NOT (${_stored_dist} STREQUAL ${__dist})))
                    message(FATAL_ERROR "Stored dist '${_stored_dist}' is not equal to current dist '${__dist}' - perform rebuild")
                endif()
            endif()
        endif()
    endif()
    message("Seting distribution " ${__config_dir}${__dist})
    set(g_core_config_path ${__core_config_dir})
    set(g_plat_config_path ${__plat_config_dir})

    # configure target dist
    include(${__plat_config_dir}${__dist}.dist)

    # set depends from ${__core_config_dir}depends_configuration.cmake
    CONF_DEPENDS_OPTIONS(${__core_config_dir})

    # create asc_config.h.in.tmp for target dist
    CONF_IN_CREATE(${__core_config_dir} ${__plat_config_dir} ${__dist} ${CMAKE_BINARY_DIR}/asc_config.h.in.tmp ${__ifdef} ${__cpp})
    configure_file(${CMAKE_BINARY_DIR}/asc_config.h.in.tmp ${__output} @ONLY)

    # store current dist
    if (${__check_dist})
        FILE(WRITE ${CMAKE_BINARY_DIR}/built_dist.conf ${__dist})
    endif()
    
    # clean up
    FILE(REMOVE ${CMAKE_BINARY_DIR}/asc_config.h.in.tmp)
endmacro()

macro(CONF_LOG_LEVEL)
    if(NOT DEFINED log_level)
        set(log_level NOTSET)
    endif()
    set(LOG_LEVELS NOTSET FATAL ERROR WARNING INFO DEBUG)
    list(FIND LOG_LEVELS ${log_level} log_level_index)
    if(log_level_index EQUAL -1)
        message(FATAL_ERROR "log_level must be one of ${LOG_LEVELS}")
    endif()
    set(ASC_LOG_LEVEL ${log_level_index})
endmacro()

macro(CONF_INC_CLEAN)
endmacro()