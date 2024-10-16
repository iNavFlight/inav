macro(compileTargetAs32)
  message(STATUS "Building target ${TARGET_NAME} as 32 bit")
  set_target_properties(${TARGET_NAME} PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} -m32" LINK_FLAGS "${LINK_FLAGS} -m32")
endmacro()

macro(compileAsC99)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set (CMAKE_C_FLAGS "--std=c99 ${CMAKE_C_FLAGS}")
    endif()
  else()
    set (CMAKE_C_STANDARD 99)
  endif()
endmacro(compileAsC99)

function(setTargetCompileOptions target)
  message(STATUS "setTargetCompileOptions for target ${target}")
  if((WIN32) AND (NOT(MINGW)))
    #windows needs this define
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    # Make warning as error
    # For more information see https://docs.microsoft.com/en-us/cpp/error-messages/compiler-errors-1/c-cpp-build-errors?view=vs-2019
    target_compile_options(${target} PRIVATE
      /WX
      /Wall
      /wd4061
      /wd4100
      /wd4189
      /wd4204
      /wd4221
      /wd4255
      /wd4668
      /wd4706
      /wd4710
      /wd4820
      /wd5031
      /wd5045
    )
  elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
      # Make warning as error
      target_compile_options(${target}
        PRIVATE
          "-Wall"
          "-Werror"
          "-Wextra"
          "-Wunused"
          "-Wuninitialized"
          "-Wconversion"
          "-Wpointer-arith"
          "-Wlogical-op"
          "-Wfloat-equal"
          "-Wmissing-declarations"
          "-Wno-unused-parameter"
          "-Wjump-misses-init"
      )
      
      if(${build_pedantic})
          message(STATUS "Using '-Wpedantic' flag for compilation as warning only for target ${target}")
          target_compile_options(${target} PRIVATE "-Wpedantic" "-Wshadow")
          #Temopary hack
          target_compile_options(${target} PRIVATE "-Wno-error=overlength-strings")
      endif()

      set(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
      if (${build_as_32})
        compileTargetAs32()
      endif ()
  endif()
endfunction()

function(compileTargetAsC99 target)
  if (CMAKE_VERSION VERSION_LESS "3.1")
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
      set_target_properties(${target} PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS} --std=c99")
    endif()
  else()
    set_target_properties(${target} PROPERTIES C_STANDARD 99)
  endif()
endfunction()