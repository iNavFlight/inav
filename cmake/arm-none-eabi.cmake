set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(WIN32)
    set(TOOL_EXECUTABLE_SUFFIX ".exe")
endif()

set(TARGET_TRIPLET "arm-none-eabi")
set(gcc "${TARGET_TRIPLET}-gcc${TOOL_EXECUTABLE_SUFFIX}")

find_program(GCC "${gcc}")
if (NOT GCC)
    message(FATAL_ERROR "Could not find ${gcc}")
endif()

set(ARM_NONE_EABI_GCC_VERSION 9.2.1)

execute_process(COMMAND "${GCC}" -dumpversion
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE GCC_VERSION)

if (NOT ${ARM_NONE_EABI_GCC_VERSION} STREQUAL ${GCC_VERSION})
    # TODO: Show how to override on cmdline or install builtin compiler
    message(FATAL_ERROR "Expecting gcc version ${ARM_NONE_EABI_GCC_VERSION}, but found ${GCC_VERSION}")
endif()

get_filename_component(TOOLCHAIN_BIN_DIR "${GCC}" DIRECTORY)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-gcc${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "asm compiler")
set(CMAKE_C_COMPILER "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-gcc${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "c compiler")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-g++${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "c++ compiler")
set(CMAKE_OBJCOPY "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-objcopy${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "objcopy tool")
set(CMAKE_OBJDUMP "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-objdump${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "objdump tool")
set(CMAKE_SIZE "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-size${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "size tool")
set(CMAKE_DEBUGER "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-gdb${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "debuger")
set(CMAKE_CPPFILT "${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-c++filt${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "c++filt")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)
endif()
set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build Type" FORCE)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})

set(arm_none_eabi_debug "-Og -g")
set(arm_none_eabi_release "-Os -DNDEBUG -flto -fuse-linker-plugin")
set(arm_none_eabi_relwithdebinfo "-ggdb3 ${arm_none_eabi_release}")

SET(CMAKE_C_FLAGS_DEBUG ${arm_none_eabi_debug} CACHE INTERNAL "c compiler flags debug")
SET(CMAKE_CXX_FLAGS_DEBUG ${arm_none_eabi_debug} CACHE INTERNAL "c++ compiler flags debug")
SET(CMAKE_ASM_FLAGS_DEBUG ${arm_none_eabi_debug} CACHE INTERNAL "asm compiler flags debug")

SET(CMAKE_C_FLAGS_RELEASE ${arm_none_eabi_release} CACHE INTERNAL "c compiler flags release")
SET(CMAKE_CXX_FLAGS_RELEASE ${arm_none_eabi_release} CACHE INTERNAL "cxx compiler flags release")
SET(CMAKE_ASM_FLAGS_RELEASE ${arm_none_eabi_release} CACHE INTERNAL "asm compiler flags release")

SET(CMAKE_C_FLAGS_RELWITHDEBINFO ${arm_none_eabi_relwithdebinfo} CACHE INTERNAL "c compiler flags release")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${arm_none_eabi_relwithdebinfo} CACHE INTERNAL "cxx compiler flags release")
SET(CMAKE_ASM_FLAGS_RELWITHDEBINFO ${arm_none_eabi_relwithdebinfo} CACHE INTERNAL "asm compiler flags release")
