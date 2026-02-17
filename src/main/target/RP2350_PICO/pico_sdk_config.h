/*
 * INAV RP2350 Pico SDK Configuration
 *
 * Replaces the auto-generated pico/config_autogen.h.
 * Referenced via -DPICO_CONFIG_HEADER=pico_sdk_config.h
 *
 * Platform defines (PICO_RP2350, PICO_RP2350A, etc.) are set via
 * cmake compiler flags in cmake/rp2350.cmake.
 */

#ifndef _INAV_PICO_SDK_CONFIG_H
#define _INAV_PICO_SDK_CONFIG_H

// Board identifier
#define PICO_BOARD "pico2"

// SDK library flags — tells SDK which components are available
#define LIB_CMSIS_CORE 1
#define LIB_PICO_PLATFORM_PANIC 1
#define LIB_PICO_STDIO 1
#define LIB_PICO_STDIO_USB 1
#define LIB_PICO_PRINTF 1
#define LIB_PICO_PRINTF_PICO 1
#define LIB_PICO_RUNTIME 1
#define LIB_PICO_RUNTIME_INIT 1
#define LIB_PICO_STANDARD_BINARY_INFO 1
#define LIB_PICO_SYNC 1
#define LIB_PICO_TIME 1
#define LIB_PICO_UTIL 1
#define LIB_PICO_MALLOC 0
#define LIB_PICO_MEM_OPS 0
#define LIB_PICO_ATOMIC 1
#define LIB_PICO_MULTICORE 0
#define LIB_PICO_BOOTROM 1
#define LIB_PICO_STDLIB 1

// USB CDC configuration
#define PICO_STDIO_USB_CONNECT_WAIT_TIMEOUT_MS 0
#define PICO_RP2040_USB_DEVICE_ENUMERATION_FIX 0
#define PICO_RP2040_USB_DEVICE_UFRAME_FIX 0

// No C++ exceptions
#define PICO_CXX_ENABLE_EXCEPTIONS 0

// Mem ops — use compiler builtins
#define PICO_USE_OPTIMIZED_ROM_MEMSET 0
#define PICO_USE_OPTIMIZED_ROM_MEMCPY 0

// CMSIS rename exceptions (maps SDK IRQ names to CMSIS names)
#include "cmsis/rename_exceptions.h"

#endif /* _INAV_PICO_SDK_CONFIG_H */
