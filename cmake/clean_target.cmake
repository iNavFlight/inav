# Removes the build artefacts of a single firmware target. Run at build time
# by the clean_<target> targets added by add_clean_target():
#
#   cmake -P cmake/clean_target.cmake <path> [<path>...]
#
# Every path that is a directory is treated as the object directory of an
# executable and only the compiler output below it is removed. The directory
# itself is kept, because the Makefile generator stores the build rules of the
# target (build.make, DependInfo.cmake, ...) next to the object files and
# removing those breaks the next "make <target>" until CMake is run again.
# Every other path is removed as a file, missing files are ignored.

if(CMAKE_ARGC GREATER 3)
    math(EXPR last_argument "${CMAKE_ARGC} - 1")
    foreach(argument RANGE 3 ${last_argument})
        set(path "${CMAKE_ARGV${argument}}")
        if(IS_DIRECTORY "${path}")
            file(GLOB_RECURSE objects "${path}/*.o" "${path}/*.obj" "${path}/*.d")
            if(objects)
                file(REMOVE ${objects})
            endif()
        else()
            file(REMOVE "${path}")
        endif()
    endforeach()
endif()
