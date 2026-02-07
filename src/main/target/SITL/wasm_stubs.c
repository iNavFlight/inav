/*
 * WASM Stubs - Functions not available in WebAssembly builds
 *
 * This file provides stub implementations for functions that:
 * 1. Rely on native POSIX/Linux APIs not available in WASM
 * 2. Are excluded from WASM builds but still referenced by code
 * 3. Need minimal implementations for Phase 1 POC
 */

#ifdef __EMSCRIPTEN__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// ============================================================================
// POSIX Scheduler Stubs
// ============================================================================
// POSIX scheduler functions not available in WASM

int sched_get_priority_min(int policy) {
    (void)policy;
    return 0;  // Return minimum priority
}

// ============================================================================
// TCP Server Stubs
// ============================================================================
// TCP server is disabled for WASM (WebSocket only), but some code still
// references these functions.

// From drivers/serial_tcp.h
uint32_t tcpRXBytesFree(void *instance) {
    (void)instance;
    return 0;  // No TCP RX buffer in WASM
}

bool tcpReceiveBytesEx(void *instance, uint8_t **buffer, int count, uint32_t timeout_ms) {
    (void)instance;
    (void)buffer;
    (void)count;
    (void)timeout_ms;
    return false;  // TCP receive not supported in WASM
}

// From target/SITL/target.h
uint16_t tcpBasePort = 0;  // TCP not used in WASM builds

// ============================================================================
// Config Streamer - provided by config/config_streamer_ram.c for WASM
// ============================================================================
// Settings stored in RAM only (not persisted across page reloads)
// See cmake/sitl.cmake which includes config_streamer_ram.c for WASM builds

// ============================================================================
// WebSocket Serial Stubs
// ============================================================================
// WebSocket implementation needs Emscripten WebSocket API integration
// TODO Phase 1: Implement using Emscripten's emscripten/websocket.h

// From drivers/serial_websocket.h
void *wsOpen(int uart_index, uint16_t port) {
    (void)uart_index;
    (void)port;
    // Stub: Would open WebSocket server on specified port
    // Phase 1: Return NULL to indicate WS not yet implemented
    return NULL;
}

// ============================================================================
// EEPROM Validation Stubs
// ============================================================================

// From fc/config.h
void ensureEEPROMContainsValidData(void) {
    // WASM uses RAM-based config with defaults initialized by pgResetAll().
    // No persistent storage means no stale/corrupt EEPROM data to validate.
}

#endif // __EMSCRIPTEN__
