if(MSVC)
  add_compile_options(
      $<$<CONFIG:>:/MT> #---------|
      $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
      $<$<CONFIG:Release>:/MT> #--|
  )
endif()

if(ADDRESS_SANITIZER)
  add_compile_options(-fsanitize=address)
  if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_link_options(-fsanitize=address)
  endif()
endif()

# Turn on strict compiler flags only for testing to allow better compatability with diverse platforms.
if(UNIT_TESTING)
  if(MSVC)
    if(WARNINGS_AS_ERRORS)
      set(WARNINGS_AS_ERRORS_FLAG "/WX")
      # Linker warnings 4300 and 4302 warn that seeing asan metadata disables incremental linking
      # and that the experience of debugging asan builds may be better if you pass /debug to the linker
      # Explicitly disabling incremental linking is annoying because cmake adds it implicitly as part of language
      # setup, so you need to sift through CMAKE_<type>_LINKER_FLAGS_<config> to turn it off. We could just append
      # /INCREMENTAL:NO but then we get a warning about conflicting linker flags (telling us the last one wins)
      # We do run asan in configurations with /debug turned on, so ensuring we turn it on in release mode as well
      # isn't that valuable
      set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /WX /ignore:4300 /ignore:4302")
    endif()
    # C5072 warns that you might want debug info with asan, we know we're in a release build, that warning is not helpful
    add_compile_options(/W4 ${WARNINGS_AS_ERRORS_FLAG} /wd5031 /wd4668 /wd4820 /wd4255 /wd4710 /wd5072 /analyze)
  elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    if(WARNINGS_AS_ERRORS)
      set(WARNINGS_AS_ERRORS_FLAG "-Werror")
    endif()

    add_compile_options(-Xclang -Wall -Wextra -pedantic  ${WARNINGS_AS_ERRORS_FLAG} -Wdocumentation -Wdocumentation-unknown-command -fcomment-block-commands=retval -Wcast-qual -Wunused -Wuninitialized -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wfloat-equal)
  elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
    if(WARNINGS_AS_ERRORS)
      set(WARNINGS_AS_ERRORS_FLAG "-Werror")
    endif()

    add_compile_options(-Wall -Wextra -pedantic  ${WARNINGS_AS_ERRORS_FLAG} -Wcast-qual -Wunused -Wuninitialized -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Wfloat-equal)
  else()
    message(WARNING "Using an unsupported compiler. Disabling stricter compiler flags.")
  endif()
endif()
