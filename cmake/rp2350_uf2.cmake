# Best-effort .uf2 generation for RP2350 targets.
#
# Invoked from cmake/rp2350.cmake as a POST_BUILD step so a .uf2 conversion
# failure can never fail the firmware build: the .uf2 is a convenience
# artifact for BOOTSEL drag-and-drop flashing, while .hex/.bin are the
# primary deliverables and are what CI uploads.  This matters on CI, where
# RP2350_PICO builds inside a `ninja ci` slice shared with ~17 other targets
# — a failed POST_BUILD would drop that slice's .hex uploads for all of them.
#
# Behaviour:
#   * If neither the ELF nor the .bin exists (no build output), do nothing —
#     the step effectively "doesn't run".
#   * If picotool is available and accepts --family rp2350-arm-s, use it.
#   * Otherwise (picotool missing, too old for RP2350, or it failed) fall back
#     to python3 + src/utils/elf2uf2.py, which is byte-identical to picotool
#     for this image (the ELF carries the RP2350 IMAGE_DEF).
#   * If every path fails, warn and exit 0.  The build is never failed here.
#
# Usage:
#   cmake -DELF=<path> -DBIN=<path> -DOUT=<path>
#         [-DPICOTOOL=<path-or-empty>] [-DPYTHON3=<path-or-empty>]
#         -P rp2350_uf2.cmake

if(NOT EXISTS "${ELF}" AND NOT EXISTS "${BIN}")
    message(STATUS "rp2350 .uf2: no ELF/bin found — skipping (nothing to convert)")
    return()
endif()

set(converter_msg "")
set(ok FALSE)

# 1) picotool first (nicer diagnostics / handles the RP2350 IMAGE_DEF block).
if(PICOTOOL)
    execute_process(
        COMMAND "${PICOTOOL}" uf2 convert "${ELF}" "${OUT}" --family rp2350-arm-s
        RESULT_VARIABLE picotool_rc
        OUTPUT_QUIET ERROR_QUIET
    )
    if(picotool_rc EQUAL 0)
        message(STATUS "rp2350 .uf2: generated ${OUT} via picotool")
        return()
    endif()
    set(converter_msg "picotool failed (rc=${picotool_rc}); ")
endif()

# 2) python3 + the in-tree elf2uf2.py chunker as a portable fallback.
if(PYTHON3 AND EXISTS "${BIN}")
    execute_process(
        COMMAND "${PYTHON3}" "${CMAKE_CURRENT_LIST_DIR}/../src/utils/elf2uf2.py" "${BIN}" "${OUT}"
        RESULT_VARIABLE elf2uf2_rc
        OUTPUT_QUIET ERROR_QUIET
    )
    if(elf2uf2_rc EQUAL 0)
        message(STATUS "rp2350 .uf2: generated ${OUT} via elf2uf2.py")
        return()
    endif()
    string(APPEND converter_msg "elf2uf2.py failed (rc=${elf2uf2_rc})")
else()
    string(APPEND converter_msg "no python3 or no .bin for elf2uf2.py fallback")
endif()

message(WARNING "rp2350 .uf2: ${converter_msg} — continuing without ${OUT} (BOOTSEL users: install picotool)")
