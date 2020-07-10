set(SETTINGS_GENERATED "settings_generated")
set(SETTINGS_GENERATED_C "${SETTINGS_GENERATED}.c")
set(SETTINGS_GENERATED_H "${SETTINGS_GENERATED}.h")
set(SETTINGS_FILE "${MAIN_SRC_DIR}/fc/settings.yaml")
set(SETTINGS_GENERATOR "${MAIN_UTILS_DIR}/settings.rb")

function(enable_settings target)
    set(dir "${CMAKE_CURRENT_BINARY_DIR}/${target}")
    target_include_directories(${target} PRIVATE ${dir})
    get_target_property(options ${target} COMPILE_OPTIONS)
    get_target_property(includes ${target} INCLUDE_DIRECTORIES)
    list(TRANSFORM includes PREPEND "-I")
    get_target_property(defs ${target} COMPILE_DEFINITIONS)
    list(TRANSFORM defs PREPEND "-D")
    list(APPEND cflags ${options})
    list(APPEND cflags ${includes})
    list(APPEND cflags ${defs})
    add_custom_command(
        OUTPUT ${dir}/${SETTINGS_GENERATED_H} ${dir}/${SETTINGS_GENERATED_C}
        COMMAND
            ${CMAKE_COMMAND} -E env CFLAGS="${cflags}" TARGET=${target}
            ${RUBY_EXECUTABLE} ${SETTINGS_GENERATOR} ${MAIN_DIR} ${SETTINGS_FILE} -o "${dir}"
        DEPENDS ${SETTINGS_GENERATOR} ${SETTINGS_FILE}
    )
endfunction()
