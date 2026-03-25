# Fix WebAssembly filename references in generated files
# This script updates both the .js file and HTML file with correct versioned filenames
#
# Usage: cmake -P fix_wasm_build_files.cmake --arg <js_file> <html_file> <wasm_basename> <js_filename> [<firmware_version>]
# Example: cmake -P fix_wasm_build_files.cmake --arg inav_9.0.0_WASM.js index.html inav_9.0.0_WASM inav_9.0.0_WASM.js 9.0.0

# Parse arguments (skip the script name and --arg flag)
set(argc ${CMAKE_ARGC})
set(argv ${CMAKE_ARGV})

if(argc LESS 7)
    message(FATAL_ERROR "fix_wasm_build_files.cmake requires: js_file, html_file, wasm_basename, and js_filename")
endif()

# Arguments are at positions 4, 5, 6, and 7 (0-indexed)
set(js_file "${CMAKE_ARGV4}")
set(html_file "${CMAKE_ARGV5}")
set(wasm_basename "${CMAKE_ARGV6}")
set(js_filename "${CMAKE_ARGV7}")
# Optional firmware version at position 8
if(argc GREATER 7)
    set(firmware_version "${CMAKE_ARGV8}")
else()
    set(firmware_version "")
endif()

if(NOT EXISTS "${js_file}")
    message(FATAL_ERROR "JavaScript file not found: ${js_file}")
endif()

if(NOT EXISTS "${html_file}")
    message(FATAL_ERROR "HTML file not found: ${html_file}")
endif()

message(STATUS "Fixing WASM filenames in: ${js_file}")
message(STATUS "Fixing WASM HTML in: ${html_file}")
message(STATUS "Target WASM basename: ${wasm_basename}")
message(STATUS "Target JS filename: ${js_filename}")
if(firmware_version)
    message(STATUS "Firmware version: ${firmware_version}")
endif()

# ==========================================
# Part 1: Fix JavaScript file
# ==========================================

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

# Handle new URL patterns for wasm file
string(REGEX REPLACE "new URL\\('[^']+\\.wasm'" "new URL('${wasm_basename}.wasm'" js_content "${js_content}")
string(REGEX REPLACE "new URL\\(\"[^\"]+\\.wasm\"" "new URL(\"${wasm_basename}.wasm\"" js_content "${js_content}")

# Handle new URL patterns for worker js file  
string(REGEX REPLACE "new Worker\\(new URL\\('[^']+\\.js'" "new Worker(new URL('${js_filename}'" js_content "${js_content}")
string(REGEX REPLACE "new Worker\\(new URL\\(\"[^\"]+\\.js\"" "new Worker(new URL(\"${js_filename}\"" js_content "${js_content}")

# Write the modified content back
file(WRITE "${js_file}" "${js_content}")

message(STATUS "Successfully updated WASM filename references to: ${wasm_basename}.wasm")

# ==========================================
# Part 2: Fix HTML file
# ==========================================

# Read the entire HTML file
file(READ "${html_file}" html_content)

# Replace the script src to load the correct WASM JS file
# Pattern: script.src = 'inav_WASM.js?t=' or ./inav_WASM.js with cache-busting
# Matches: wasmScriptTag.src = 'inav_WASM.js?t=' + Date.now(); or import('./inav_WASM.js?t=' + Date.now()); etc.
# Preserve the ./ prefix if it exists by using group 2
string(REGEX REPLACE "(['\"])(\\./)?(inav_)[^'\"]*\\.js" "\\1\\2${js_filename}" html_content "${html_content}")

# Update the version in the footer if firmware_version is provided
if(firmware_version)
    string(REGEX REPLACE "INAV [0-9]+\\.[0-9]+\\.[0-9]+" "INAV ${firmware_version}" html_content "${html_content}")
endif()

# Write the modified content back
file(WRITE "${html_file}" "${html_content}")

message(STATUS "Successfully updated HTML to load: ${js_filename}")
if(firmware_version)
    message(STATUS "Updated version to: INAV ${firmware_version}")
endif()
