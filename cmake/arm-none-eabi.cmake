set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(WIN32)
    set(TOOL_EXECUTABLE_SUFFIX ".exe")
endif()

set(TARGET_TRIPLET "arm-none-eabi")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_ASM_COMPILER "${TARGET_TRIPLET}-gcc${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "asm compiler")
set(CMAKE_C_COMPILER "${TARGET_TRIPLET}-gcc${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "c compiler")
set(CMAKE_CXX_COMPILER "${TARGET_TRIPLET}-g++${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "c++ compiler")
set(CMAKE_OBJCOPY "${TARGET_TRIPLET}-objcopy${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "objcopy tool")
set(CMAKE_OBJDUMP "${TARGET_TRIPLET}-objdump${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "objdump tool")
set(CMAKE_SIZE "${TARGET_TRIPLET}-size${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "size tool")
set(CMAKE_DEBUGGER "${TARGET_TRIPLET}-gdb${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "debugger")
set(CMAKE_CPPFILT "${TARGET_TRIPLET}-c++filt${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "c++filt")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Build Type" FORCE)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})

set(arm_none_eabi_debug "-Og -g")
# We set -Os or -O2 depending on the MCU family
set(arm_none_eabi_release "-DNDEBUG")
set(arm_none_eabi_relwithdebinfo "-ggdb3 ${arm_none_eabi_release}")

set(CMAKE_C_FLAGS_DEBUG ${arm_none_eabi_debug} CACHE INTERNAL "c compiler flags debug")
set(CMAKE_CXX_FLAGS_DEBUG ${arm_none_eabi_debug} CACHE INTERNAL "c++ compiler flags debug")
set(CMAKE_ASM_FLAGS_DEBUG ${arm_none_eabi_debug} CACHE INTERNAL "asm compiler flags debug")

set(CMAKE_C_FLAGS_RELEASE ${arm_none_eabi_release} CACHE INTERNAL "c compiler flags release")
set(CMAKE_CXX_FLAGS_RELEASE ${arm_none_eabi_release} CACHE INTERNAL "cxx compiler flags release")
set(CMAKE_ASM_FLAGS_RELEASE ${arm_none_eabi_release} CACHE INTERNAL "asm compiler flags release")

set(CMAKE_C_FLAGS_RELWITHDEBINFO ${arm_none_eabi_relwithdebinfo} CACHE INTERNAL "c compiler flags release")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${arm_none_eabi_relwithdebinfo} CACHE INTERNAL "cxx compiler flags release")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO ${arm_none_eabi_relwithdebinfo} CACHE INTERNAL "asm compiler flags release")
