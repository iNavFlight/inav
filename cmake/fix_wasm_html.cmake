# Fix WebAssembly filename reference in HTML file
# This script updates the HTML to load the correct versioned WASM JS file
# and updates the version string in the footer
#
# Usage: cmake -P fix_wasm_html.cmake --arg <html_file> <js_filename> [<firmware_version>]
# Example: cmake -P fix_wasm_html.cmake --arg index.html inav_9.0.0_WASM.js 9.0.0

# Parse arguments (skip the script name and --arg flag)
set(argc ${CMAKE_ARGC})
set(argv ${CMAKE_ARGV})

if(argc LESS 5)
    message(FATAL_ERROR "fix_wasm_html.cmake requires: html_file and js_filename")
endif()

# Arguments are at positions 4 and 5 (0-indexed: argv4, argv5)
set(html_file "${CMAKE_ARGV4}")
set(js_filename "${CMAKE_ARGV5}")
# Optional firmware version at position 6
if(argc GREATER 5)
    set(firmware_version "${CMAKE_ARGV6}")
else()
    set(firmware_version "")
endif()

if(NOT EXISTS "${html_file}")
    message(FATAL_ERROR "HTML file not found: ${html_file}")
endif()

message(STATUS "Fixing WASM HTML: ${html_file}")
message(STATUS "Target JS filename: ${js_filename}")
if(firmware_version)
    message(STATUS "Firmware version: ${firmware_version}")
endif()

# Read the entire HTML file
file(READ "${html_file}" html_content)

# Replace the script src to load the correct WASM JS file
# Pattern: script.src = 'inav_WASM.js?t=' or similar with cache-busting
# Matches: wasmScriptTag.src = 'inav_WASM.js?t=' + Date.now(); etc.
string(REGEX REPLACE "(['\"])inav_[^'\"]*\\.js" "\\1${js_filename}" html_content "${html_content}")

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
