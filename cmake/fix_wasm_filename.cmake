# Fix WebAssembly filename references in generated .js file
# This script replaces hardcoded .wasm filename references with the correct versioned filename
#
# Usage: cmake -P fix_wasm_filename.cmake --arg <js_file> <wasm_basename>
# Example: cmake -P fix_wasm_filename.cmake --arg inav_9.0.0_WASM.js inav_9.0.0_WASM

# Parse arguments (skip the script name and --arg flag)
set(argc ${CMAKE_ARGC})
set(argv ${CMAKE_ARGV})

if(argc LESS 5)
    message(FATAL_ERROR "fix_wasm_filename.cmake requires: js_file and wasm_basename")
endif()

# Arguments are at positions 4 and 5 (0-indexed: argv4, argv5)
set(js_file "${CMAKE_ARGV4}")
set(wasm_basename "${CMAKE_ARGV5}")

if(NOT EXISTS "${js_file}")
    message(FATAL_ERROR "JavaScript file not found: ${js_file}")
endif()

message(STATUS "Fixing WASM filename references in: ${js_file}")
message(STATUS "Target basename: ${wasm_basename}")

# Read the entire .js file
file(READ "${js_file}" js_content)

# Use regex to replace ANY .wasm filename with the correct one
# CMake regex doesn't support non-greedy matching, so use simpler pattern
# Pattern: return locateFile('***.wasm') - match anything up to .wasm
string(REGEX REPLACE "return locateFile\\('[^']+\\.wasm'\\)" "return locateFile('${wasm_basename}.wasm')" js_content "${js_content}")
string(REGEX REPLACE "return locateFile\\(\"[^\"]+\\.wasm\"\\)" "return locateFile(\"${wasm_basename}.wasm\")" js_content "${js_content}")

# Also handle direct locateFile calls without return
string(REGEX REPLACE "locateFile\\('[^']+\\.wasm'\\)" "locateFile('${wasm_basename}.wasm')" js_content "${js_content}")
string(REGEX REPLACE "locateFile\\(\"[^\"]+\\.wasm\"\\)" "locateFile(\"${wasm_basename}.wasm\")" js_content "${js_content}")

# Write the modified content back
file(WRITE "${js_file}" "${js_content}")

message(STATUS "Successfully updated WASM filename references to: ${wasm_basename}.wasm")
