include(gcc)
set(arm_none_eabi_triplet "arm-none-eabi")

# Keep version in sync with the distribution files below
set(arm_none_eabi_gcc_version "9.2.1")
set(arm_none_eabi_base_url "https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2019q4/gcc-arm-none-eabi-9-2019-q4-major")
# suffix and sha1
set(arm_none_eabi_win32 "win32.zip" 60f364ecf2e8717a58d352db95b388eee508b727)
set(arm_none_eabi_linux_amd64 "x86_64-linux.tar.bz2" 3829ff61b2601c6cf061a5a275c2538a96a8d521)
set(arm_none_eabi_linux_aarch64 "aarch64-linux.tar.bz2" fdb6fe7058927ad897f63d2d245f825a9587a1c5)
set(arm_none_eabi_gcc_macos "mac.tar.bz2" 26fe33e0c25d9a2947c0373ea48c00ef46eacd58)

function(arm_none_eabi_gcc_distname var)
    string(REPLACE "/" ";" url_parts ${arm_none_eabi_base_url})
    list(LENGTH url_parts n)
    math(EXPR last "${n} - 1")
    list(GET url_parts ${last} basename)
    set(${var} ${basename} PARENT_SCOPE)
endfunction()

function(host_uname_machine var)
    # We need to call uname -m manually, since at the point
    # this file is included CMAKE_HOST_SYSTEM_PROCESSOR is
    # empty because we haven't called project() yet.
    execute_process(COMMAND uname -m
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE machine)

    set(${var} ${machine} PARENT_SCOPE)
endfunction()

function(arm_none_eabi_gcc_install)
    set(dist "")
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
        set(dist ${arm_none_eabi_win32})
    elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_HOST_SYSTEM_NAME STREQUAL "FreeBSD")
        if(NOT CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
            message("-- no compiler binaries available for ${CMAKE_HOST_SYSTEM_NAME}, using Linux binaries as a fallback")
        endif()
        host_uname_machine(machine)
        # Linux returns x86_64, FreeBSD returns amd64
        if(machine STREQUAL "x86_64" OR machine STREQUAL "amd64")
            set(dist ${arm_none_eabi_linux_amd64})
        elseif(machine STREQUAL "aarch64")
            set(dist ${arm_none_eabi_linux_aarch64})
        else()
            message("-- no precompiled ${arm_none_eabi_triplet} toolchain for machine ${machine}")
        endif()
    elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL "Darwin")
        set(dist ${arm_none_eabi_gcc_macos})
    endif()

    if(dist STREQUAL "")
        message(FATAL_ERROR "could not install ${arm_none_eabi_triplet}-gcc automatically")
    endif()
    list(GET dist 0 dist_suffix)
    list(GET dist 1 dist_sha1)
    set(dist_url "${arm_none_eabi_base_url}-${dist_suffix}")
    string(REPLACE "/" ";" url_parts ${dist_url})
    list(LENGTH url_parts n)
    math(EXPR last "${n} - 1")
    list(GET url_parts ${last} basename)
    set(output "${DOWNLOADS_DIR}/${basename}")
    message("-- downloading ${arm_none_eabi_triplet}-gcc ${arm_none_eabi_gcc_version} from ${dist_url}")
    file(DOWNLOAD ${dist_url} ${output}
        INACTIVITY_TIMEOUT 30
        STATUS status
        SHOW_PROGRESS
        EXPECTED_HASH SHA1=${dist_sha1}
        TLS_VERIFY ON
    )
    list(GET status 0 status_code)
    if(NOT status_code EQUAL 0)
        list(GET status 1 status_message)
        message(FATAL_ERROR "error downloading ${basename}: ${status_message}")
    endif()
    message("-- extracting ${basename}")
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${TOOLS_DIR})
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xf ${output}
        RESULT_VARIABLE status
        WORKING_DIRECTORY ${TOOLS_DIR}
    )
    if(NOT status EQUAL 0)
        message(FATAL_ERROR "error extracting ${basename}: ${status}")
    endif()
endfunction()

function(arm_none_eabi_gcc_add_path)
    arm_none_eabi_gcc_distname(dist_name)
    set(gcc_path "${TOOLS_DIR}/${dist_name}/bin")
    if(CMAKE_HOST_SYSTEM MATCHES ".*Windows.*")
        set(sep "\\;")
    else()
        set(sep ":")
    endif()
    set(ENV{PATH} "${gcc_path}${sep}$ENV{PATH}")
endfunction()

function(arm_none_eabi_gcc_check)
    gcc_get_version(version
        TRIPLET ${arm_none_eabi_triplet}
        PROGRAM_NAME prog
        PROGRAM_PATH prog_path
    )
    if(NOT version)
        message("-- could not find ${prog}")
        arm_none_eabi_gcc_install()
        return()
    endif()
    message("-- found ${prog} ${version} at ${prog_path}")
    if(COMPILER_VERSION_CHECK AND NOT arm_none_eabi_gcc_version STREQUAL version)
        message("-- expecting ${prog} version ${arm_none_eabi_gcc_version}, but got version ${version} instead")
        arm_none_eabi_gcc_install()
        return()
    endif()
endfunction()

arm_none_eabi_gcc_add_path()
arm_none_eabi_gcc_check()
