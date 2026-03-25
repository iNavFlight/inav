# Emscripten WebAssembly Toolchain Configuration
# This toolchain enables building INAV as WebAssembly (WASM) using Emscripten

set(CMAKE_SYSTEM_NAME Emscripten)
set(CMAKE_SYSTEM_PROCESSOR wasm)

# Find Emscripten compiler
find_program(EMCC emcc DOC "Emscripten C compiler")
find_program(EMXX em++ DOC "Emscripten C++ compiler")

if(NOT EMCC OR NOT EMXX)
    message(FATAL_ERROR 
        "Emscripten not found. Please install Emscripten SDK:\n"
        "  https://emscripten.org/docs/getting_started/downloads.html\n"
        "And activate the environment:\n"
        "  source ~/emsdk/emsdk_env.sh  (Linux/macOS)\n"
        "  Or use the Windows batch file"
    )
endif()

# Set compiler
set(CMAKE_C_COMPILER ${EMCC} CACHE INTERNAL "Emscripten C compiler")
set(CMAKE_CXX_COMPILER ${EMXX} CACHE INTERNAL "Emscripten C++ compiler")

# Disable C++ extensions for consistency
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type defaults
if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Build Type" FORCE)
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS ${CMAKE_CONFIGURATION_TYPES})

# Compiler flags for different build types
set(emscripten_debug "-O0 -g -DDEBUG")
set(emscripten_release "-O3 -DNDEBUG")

set(CMAKE_C_FLAGS_DEBUG ${emscripten_debug} CACHE INTERNAL "emscripten C compiler flags debug")
set(CMAKE_CXX_FLAGS_DEBUG ${emscripten_debug} CACHE INTERNAL "emscripten C++ compiler flags debug")

set(CMAKE_C_FLAGS_RELEASE ${emscripten_release} CACHE INTERNAL "emscripten C compiler flags release")
set(CMAKE_CXX_FLAGS_RELEASE ${emscripten_release} CACHE INTERNAL "emscripten C++ compiler flags release")

# Generic flags for all builds
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# Link flags
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMSCRIPTEN_LINK_FLAGS}")

# File extension for binaries
set(CMAKE_EXECUTABLE_SUFFIX ".js")

# Skip compiler checks that don't work with Emscripten
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# Disable ASM support
set(CMAKE_ASM_COMPILER_WORKS 1)
