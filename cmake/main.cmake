set(MAIN_INCLUDE_DIRS
    "${MAIN_LIB_DIR}"
    "${MAIN_SRC_DIR}"
    "${MAIN_LIB_DIR}/main/MAVLink"
)

set(MAIN_DEFINITIONS
    __FORKNAME__=inav
    __REVISION__="${GIT_SHORT_HASH}"
)

set(MAIN_COMPILE_OPTIONS
    -Wall
    -Wextra
    -Wunsafe-loop-optimizations
    -Wdouble-promotion
    -Wstrict-prototypes
    -Werror=switch
)

macro(main_sources var) # list-var src-1...src-n
    set(${var} ${ARGN})
    list(TRANSFORM ${var} PREPEND "${MAIN_SRC_DIR}/")
endmacro()

macro(exclude_basenames) # list-var excludes-var
    set(_filtered "")
    foreach(item ${${ARGV0}})
        get_filename_component(basename ${item} NAME)
        if (NOT ${basename} IN_LIST ${ARGV1})
            list(APPEND _filtered ${item})
        endif()
    endforeach()
    set(${ARGV0} ${_filtered})
endmacro()

macro(glob_except) # var-name pattern excludes-var
    file(GLOB ${ARGV0} ${ARGV1})
    exclude_basenames(${ARGV0} ${ARGV2})
endmacro()

function(setup_firmware_target name)
    target_compile_options(${name} PRIVATE ${MAIN_COMPILE_OPTIONS})
    target_include_directories(${name} PRIVATE ${MAIN_INCLUDE_DIRS})
    target_compile_definitions(${name} PRIVATE ${MAIN_DEFINITIONS} __TARGET__="${name}" ${name})
    enable_settings(${name})
    # XXX: Don't make SETTINGS_GENERATED_C part of the build,
    # since it's compiled via #include in settings.c. This will
    # change once we move off PGs
    target_sources(${name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/${name}/${SETTINGS_GENERATED_H}")
    set_target_properties(${name} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
    get_property(targets GLOBAL PROPERTY VALID_TARGETS)
    set_property(GLOBAL PROPERTY VALID_TARGETS "${targets} ${name}")
    setup_openocd(${name})
endfunction()

function(exclude_from_all target)
    set_property(TARGET ${target} PROPERTY
        TARGET_MESSAGES OFF
        EXCLUDE_FROM_ALL 1
        EXCLUDE_FROM_DEFAULT_BUILD 1)
endfunction()

function(collect_targets)
    get_property(targets GLOBAL PROPERTY VALID_TARGETS)
    list(SORT targets)
    set(list_target_name "targets")
    add_custom_target(${list_target_name}
        COMMAND cmake -E echo "Valid targets: ${targets}")
    exclude_from_all(${list_target_name})
endfunction()
