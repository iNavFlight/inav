function(add_ext_lib _lib_name _alias)
    find_library (
        LIB_PATH
        ${_lib_name}
    )
    message("lib path: ${LIB_PATH}")    

    add_library(${_alias} SHARED IMPORTED)
    set_property(TARGET ${_alias} PROPERTY IMPORTED_LOCATION ${LIB_PATH})
endfunction()

