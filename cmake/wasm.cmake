# Emscripten/WebAssembly toolchain for SITL
# This toolchain allows INAV SITL to compile to WebAssembly for browser-based simulation

# Set build type if not specified
if(NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# Find Emscripten
if(NOT DEFINED ENV{EMSDK})
    if(EXISTS "$ENV{HOME}/emsdk/emsdk_env.sh")
        message(STATUS "EMSDK not set, trying to use ~/emsdk")
        set(ENV{EMSDK} "$ENV{HOME}/emsdk")
    else()
        message(FATAL_ERROR "EMSDK environment variable not set. Please run: source ~/emsdk/emsdk_env.sh")
    endif()
endif()

# Set Emscripten compilers
set(CMAKE_SYSTEM_NAME Emscripten)
set(CMAKE_SYSTEM_VERSION 1)

set(CMAKE_C_COMPILER "emcc" CACHE INTERNAL "c compiler")
set(CMAKE_CXX_COMPILER "em++" CACHE INTERNAL "c++ compiler")
set(CMAKE_AR "emar" CACHE INTERNAL "ar")
set(CMAKE_RANLIB "emranlib" CACHE INTERNAL "ranlib")

# Build type flags
set(debug_options "-O0 -g")
set(release_options "-O2 -DNDEBUG")
set(relwithdebinfo_options "-g ${release_options}")

set(CMAKE_C_FLAGS_DEBUG ${debug_options} CACHE INTERNAL "c compiler flags debug")
set(CMAKE_CXX_FLAGS_DEBUG ${debug_options} CACHE INTERNAL "c++ compiler flags debug")

set(CMAKE_C_FLAGS_RELEASE ${release_options} CACHE INTERNAL "c compiler flags release")
set(CMAKE_CXX_FLAGS_RELEASE ${release_options} CACHE INTERNAL "cxx compiler flags release")

set(CMAKE_C_FLAGS_RELWITHDEBINFO ${relwithdebinfo_options} CACHE INTERNAL "c compiler flags relwithdebinfo")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${relwithdebinfo_options} CACHE INTERNAL "cxx compiler flags relwithdebinfo")

# Mark as configured for Emscripten
set(CMAKE_CROSSCOMPILING_EMULATOR "\${CMAKE_CURRENT_BINARY_DIR}/node_modules/.bin/node" CACHE FILEPATH "Node.js for running WebAssembly")
