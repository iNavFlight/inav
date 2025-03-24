include(gcc)
set(arm_none_eabi_triplet "arm-none-eabi")

# Keep version in sync with the distribution files below
set(arm_none_eabi_gcc_version "13.3.1")
# This is the output directory "pretty" name and URI name prefix
set(base_dir_name "arm-gnu-toolchain-13.3.rel1")
# This is the name inside the archive, which is no longer evincible from URI, alas
set(archive_base_dir_name "arm-gnu-toolchain-13.3.rel1")
set(arm_none_eabi_base_url "https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/${base_dir_name}")
# suffix and checksum
set(arm_none_eabi_win32 "mingw-w64-i686-arm-none-eabi.zip" 39d9882ca0eb475e81170ae826c1435d)
set(arm_none_eabi_linux_amd64 "x86_64-arm-none-eabi.tar.xz" 0601a9588bc5b9c99ad2b56133b7f118)
set(arm_none_eabi_linux_aarch64 "aarch64-arm-none-eabi.tar.xz" 303102d97b877ebbeb36b3158994b218)
set(arm_none_eabi_darwin_amd64 "darwin-x86_64-arm-none-eabi.tar.xz" 4bb141e44b831635fde4e8139d470f1f)
set(arm_none_eabi_darwin_aarch64 "darwin-arm64-arm-none-eabi.tar.xz" f1c18320bb3121fa89dca11399273f4e)

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
        host_uname_machine(machine)
        if(machine STREQUAL "x86_64" OR machine STREQUAL "amd64")
            set(dist ${arm_none_eabi_darwin_amd64})
        elseif(machine STREQUAL "aarch64" OR machine STREQUAL "arm64")
            set(dist ${arm_none_eabi_darwin_aarch64})
        else()
            message("-- no precompiled ${arm_none_eabi_triplet} toolchain for machine ${machine}")
        endif()
    endif()

    if(dist STREQUAL "")
        message(FATAL_ERROR "could not install ${arm_none_eabi_triplet}-gcc automatically")
    endif()
    list(GET dist 0 dist_suffix)
    list(GET dist 1 dist_checksum)
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
        EXPECTED_HASH MD5=${dist_checksum}
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
    string(REPLACE "." ";" url_parts ${dist_suffix})
    list(GET url_parts 0 host_dir_name)
    set(dir_name "${archive_base_dir_name}-${host_dir_name}")
    file(REMOVE_RECURSE "${TOOLS_DIR}/${base_dir_name}")
    file(RENAME  "${TOOLS_DIR}/${dir_name}" "${TOOLS_DIR}/${base_dir_name}")
    # This is **somewhat ugly**
    # the newlib distributed by ARM generates suprious warnings from re-entrant POSIX functions
    # that INAV doesn't use. These "harmless" warnings can be surpressed by removing the
    # errant section from the only libnosys used by INAV ...
    # So look the other way ... while this is "fixed"
    execute_process(COMMAND arm-none-eabi-objcopy -w -R .gnu.warning.* "${TOOLS_DIR}/${base_dir_name}/arm-none-eabi/lib/thumb/v7e-m+fp/hard/libnosys.a"
      RESULT_VARIABLE status
      WORKING_DIRECTORY ${TOOLS_DIR}
    )
    if(NOT status EQUAL 0)
        message(FATAL_ERROR "error fixing libnosys.a: ${status}")
    endif()
endfunction()

function(arm_none_eabi_gcc_add_path)
    set(gcc_path "${TOOLS_DIR}/${base_dir_name}/bin")
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
