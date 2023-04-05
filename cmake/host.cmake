
if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

if(WIN32)
    set(TOOL_EXECUTABLE_SUFFIX ".exe")
endif()

set(CMAKE_ASM_COMPILER "gcc${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "asm compiler")
set(CMAKE_C_COMPILER "gcc${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "c compiler")
set(CMAKE_CXX_COMPILER "g++${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "c++ compiler")
set(CMAKE_OBJCOPY "objcopy${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "objcopy tool")
set(CMAKE_OBJDUMP "objdump${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "objdump tool")
set(CMAKE_SIZE "size${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "size tool")
set(CMAKE_DEBUGGER "gdb${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "debugger")
set(CMAKE_CPPFILT "c++filt${TOOL_EXECUTABLE_SUFFIX}" CACHE INTERNAL "c++filt")

set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Build Type" FORCE)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})

set(debug_options "-Og -O0 -g")
set(release_options "-Os -DNDEBUG")
set(relwithdebinfo_options "-ggdb3 ${release_options}")

set(CMAKE_C_FLAGS_DEBUG ${debug_options} CACHE INTERNAL "c compiler flags debug")
set(CMAKE_CXX_FLAGS_DEBUG ${debug_options} CACHE INTERNAL "c++ compiler flags debug")
set(CMAKE_ASM_FLAGS_DEBUG ${debug_options} CACHE INTERNAL "asm compiler flags debug")

set(CMAKE_C_FLAGS_RELEASE ${release_options} CACHE INTERNAL "c compiler flags release")
set(CMAKE_CXX_FLAGS_RELEASE ${release_options} CACHE INTERNAL "cxx compiler flags release")
set(CMAKE_ASM_FLAGS_RELEASE ${release_options} CACHE INTERNAL "asm compiler flags release")

set(CMAKE_C_FLAGS_RELWITHDEBINFO ${relwithdebinfo_options} CACHE INTERNAL "c compiler flags release")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${relwithdebinfo_options} CACHE INTERNAL "cxx compiler flags release")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO ${relwithdebinfo_options} CACHE INTERNAL "asm compiler flags release")
