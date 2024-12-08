#making a global variable to know if we are on linux, windows, or macosx.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(WINDOWS TRUE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
    #on Linux, enable valgrind
    #these commands (MEMORYCHECK...) need to apear BEFORE include(CTest) or they will not have any effect
    find_program(MEMORYCHECK_COMMAND valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --error-exitcode=1")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(MACOSX TRUE)
endif()


include(CheckSymbolExists)
function(detect_architecture symbol arch)
    if (NOT DEFINED ARCHITECTURE OR ARCHITECTURE STREQUAL "")
        set(CMAKE_REQUIRED_QUIET 1)
        check_symbol_exists("${symbol}" "" ARCHITECTURE_${arch})
        unset(CMAKE_REQUIRED_QUIET)

        # The output variable needs to be unique across invocations otherwise
        # CMake's crazy scope rules will keep it defined
        if (ARCHITECTURE_${arch})
            set(ARCHITECTURE "${arch}" PARENT_SCOPE)
            set(ARCHITECTURE_${arch} 1 PARENT_SCOPE)
            add_definitions(-DARCHITECTURE_${arch}=1)
        endif()
    endif()
endfunction()
if (MSVC)
    detect_architecture("_M_AMD64" x86_64)
    detect_architecture("_M_IX86" x86)
    detect_architecture("_M_ARM" ARM)
else()
    detect_architecture("__x86_64__" x86_64)
    detect_architecture("__i386__" x86)
    detect_architecture("__arm__" ARM)
endif()
if (NOT DEFINED ARCHITECTURE OR ARCHITECTURE STREQUAL "")
    set(ARCHITECTURE "GENERIC")
endif()
message(STATUS "target architecture: ${ARCHITECTURE}")

#if any compiler has a command line switch called "OFF" then it will need special care
if(NOT "${compileOption_C}" STREQUAL "OFF")
    set(CMAKE_C_FLAGS "${compileOption_C} ${CMAKE_C_FLAGS}")
endif()

function(usePermissiveRulesForSamplesAndTests)
    if (NOT MSVC)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-variable  -Wno-unused-function -Wno-missing-braces -Wno-strict-aliasing")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" PARENT_SCOPE)
    endif()
endfunction()

function(add_exec_target _TARGET_NAME)

    set(one_value_arguments
    )

    set(multi_value_arguments
        SOURCES
        COMPILE_OPTIONS
        MOCK_FUNCTIONS
        PRIVATE_ACCESS
        LINK_TARGETS
    )

    cmake_parse_arguments(_add_exec_target
        ""
        "${one_value_arguments}"
        "${multi_value_arguments}"
        ${ARGN}
    )

    if (NOT DEFINED _add_exec_target_SOURCES)
        message(FATAL_ERROR "No sources provided for target ${_TARGET_NAME}")
    endif()

    add_executable(${_TARGET_NAME} ${_add_exec_target_SOURCES})

    if (DEFINED _add_exec_target_COMPILE_OPTIONS)
        target_compile_options(${_TARGET_NAME}
            PRIVATE ${_add_exec_target_COMPILE_OPTIONS}
        )
    endif()

    if (DEFINED _add_exec_target_LINK_TARGETS)
        target_link_libraries(${_TARGET_NAME}
            PRIVATE
                ${_add_exec_target_LINK_TARGETS}
        )
    endif()

    if (DEFINED _add_exec_target_MOCK_FUNCTIONS)
        set(functions "")
        foreach(function ${_add_exec_target_MOCK_FUNCTIONS})
            set(functions "${functions} -Wl,--wrap=${function}")
        endforeach()

        set_target_properties(${_TARGET_NAME}
            PROPERTIES LINK_FLAGS
            ${functions}
        )

        # Used for apply mocks in Azure RTOS
        get_target_property(libs ${_TARGET_NAME} LINK_LIBRARIES)
        foreach(function ${_add_exec_target_MOCK_FUNCTIONS})
            list(INSERT libs 0
                -Wl,-wrap,${function}
            )
        endforeach()
        set_target_properties(${TARGET_NAME} PROPERTIES LINK_LIBRARIES "${libs}")
    endif()

    setTargetCompileOptions(${TARGET_NAME})
    compileTargetAsC99(${_TARGET_NAME})

endfunction (add_exec_target)

