function(gcc_get_version var)
    cmake_parse_arguments(args
        ""
        "TRIPLET;PROGRAM_NAME;PROGRAM_PATH"
        ""
        ${ARGN}
    )
    set(prog "gcc")
    if(args_TRIPLET)
        set(prog "${args_TRIPLET}-${prog}")
    endif()
    if(args_PROGRAM_NAME)
        set(${args_PROGRAM_NAME} ${prog} PARENT_SCOPE)
    endif()

    find_program(gcc ${prog})
    if (NOT gcc)
        set(${var} OFF PARENT_SCOPE)
    endif()

    if(args_PROGRAM_PATH)
        set(${args_PROGRAM_PATH} ${gcc} PARENT_SCOPE)
    endif()

    execute_process(COMMAND "${gcc}" -dumpversion
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE version)

    if("" STREQUAL version)
        set(${var} OFF PARENT_SCOPE)
    else()
        set(${var} ${version} PARENT_SCOPE)
    endif()
endfunction()
