set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(THREADX_ARCH "win32")
set(THREADX_TOOLCHAIN "vs_2019")

set(WIN32_FLAGS "")

set(CMAKE_C_FLAGS   "${WIN32_FLAGS} " CACHE INTERNAL "c compiler flags")
set(CMAKE_CXX_FLAGS "${WIN32_FLAGS} -fno-rtti -fno-exceptions" CACHE INTERNAL "cxx compiler flags")
set(CMAKE_ASM_FLAGS "${WIN32_FLAGS} -x assembler-with-cpp" CACHE INTERNAL "asm compiler flags")
set(CMAKE_EXE_LINKER_FLAGS "${WIN32_FLAGS} ${LD_FLAGS}" CACHE INTERNAL "exe link flags")

# this makes the test compiles use static library option so that we don't need to pre-set linker flags and scripts
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
